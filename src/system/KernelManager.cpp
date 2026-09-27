#include "KernelManager.h"
#include <sys/utsname.h>
#include <sstream>
#include <regex>

namespace LinuxManager::System {

KernelManager::KernelManager(std::shared_ptr<Core::ISystemBackend> backend)
    : m_backend(std::move(backend)) {}

std::string KernelManager::getRunningKernel() {
    struct utsname uts{};
    if (uname(&uts) == 0) {
        return uts.release;
    }
    return "";
}

std::vector<KernelInfo> KernelManager::getInstalledKernels() {
    std::vector<KernelInfo> kernels;
    if (!m_backend) return kernels;

    std::string running = getRunningKernel();

    if (m_backend->isMock()) {
        KernelInfo k1{"6.8.0-40-generic", "linux-image-6.8.0-40-generic", true, false, 95ULL * 1024 * 1024};
        KernelInfo k2{"6.8.0-38-generic", "linux-image-6.8.0-38-generic", false, true, 92ULL * 1024 * 1024};
        KernelInfo k3{"6.8.0-35-generic", "linux-image-6.8.0-35-generic", false, true, 91ULL * 1024 * 1024};
        kernels.push_back(k1);
        kernels.push_back(k2);
        kernels.push_back(k3);
        return kernels;
    }

    auto res = m_backend->executeCommand("dpkg-query", {
        "-W", "-f=${Package}|${Version}|${Installed-Size}|${Status}\n", "linux-image-[0-9]*"
    });

    if (!res.success()) return kernels;

    std::istringstream stream(res.stdOut);
    std::string line;
    while (std::getline(stream, line)) {
        if (line.empty()) continue;
        std::istringstream ls(line);
        std::string pkg, ver, sizeStr, status;
        if (std::getline(ls, pkg, '|') &&
            std::getline(ls, ver, '|') &&
            std::getline(ls, sizeStr, '|') &&
            std::getline(ls, status)) {

            if (status.find("installed") == std::string::npos) continue;

            // Extract kernel release from package name
            // e.g. linux-image-6.8.0-40-generic -> 6.8.0-40-generic
            std::string kver = pkg;
            if (kver.starts_with("linux-image-")) {
                kver = kver.substr(12);
            }

            KernelInfo info;
            info.packageName = pkg;
            info.version = kver;
            info.sizeBytes = std::strtoull(sizeStr.c_str(), nullptr, 10) * 1024;
            info.isRunning = (kver == running);
            info.isRemovable = !info.isRunning;

            kernels.push_back(info);
        }
    }

    return kernels;
}

std::vector<KernelInfo> KernelManager::getOldUnusedKernels() {
    std::vector<KernelInfo> old;
    auto all = getInstalledKernels();
    for (const auto& k : all) {
        if (k.isRemovable) {
            old.push_back(k);
        }
    }
    return old;
}

Core::CommandResult KernelManager::removeKernel(const std::string& packageName) {
    if (!m_backend) return {};

    std::string running = getRunningKernel();
    if (!running.empty() && packageName.find(running) != std::string::npos) {
        Core::CommandResult res;
        res.exitCode = 1;
        res.stdErr = "Safety Guard: Cannot remove the currently running active kernel: " + running;
        return res;
    }

    return m_backend->executePrivilegedOperation(
        Security::PrivilegedOperation::makeKernelPurge(packageName)
    );
}

} // namespace LinuxManager::System

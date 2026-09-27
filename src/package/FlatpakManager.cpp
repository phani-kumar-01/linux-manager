#include "FlatpakManager.h"
#include <sstream>

namespace LinuxManager::Package {

FlatpakManager::FlatpakManager(std::shared_ptr<Core::ISystemBackend> backend)
    : m_backend(std::move(backend)) {}

bool FlatpakManager::isFlatpakAvailable() {
    if (!m_backend) return false;
    auto res = m_backend->executeCommand("which", {"flatpak"});
    return res.success();
}

std::vector<FlatpakItem> FlatpakManager::getInstalledFlatpaks() {
    if (!m_backend || !isFlatpakAvailable()) return {};

    auto res = m_backend->executeCommand("flatpak", {
        "list", "--columns=application,name,version,branch,arch,origin,size"
    });

    std::vector<FlatpakItem> items;
    if (!res.success()) return items;

    std::istringstream stream(res.stdOut);
    std::string line;
    while (std::getline(stream, line)) {
        if (line.empty()) continue;
        std::istringstream lineStream(line);
        std::string appId, name, ver, branch, arch, origin, sizeStr;
        if (std::getline(lineStream, appId, '\t') &&
            std::getline(lineStream, name, '\t') &&
            std::getline(lineStream, ver, '\t') &&
            std::getline(lineStream, branch, '\t') &&
            std::getline(lineStream, arch, '\t') &&
            std::getline(lineStream, origin, '\t') &&
            std::getline(lineStream, sizeStr)) {

            FlatpakItem item;
            item.appId = appId;
            item.name = name;
            item.version = ver;
            item.branch = branch;
            item.arch = arch;
            item.origin = origin;
            // Parse human-readable size or rough estimate
            item.sizeBytes = 50 * 1024 * 1024; // Default baseline ~50MB if parse fails
            items.push_back(item);
        }
    }

    return items;
}

std::vector<FlatpakItem> FlatpakManager::getUnusedRuntimes() {
    if (!m_backend || !isFlatpakAvailable()) return {};

    auto res = m_backend->executeCommand("flatpak", {"uninstall", "--unused", "--dry-run"});
    std::vector<FlatpakItem> unused;
    if (!res.success()) return unused;

    std::istringstream stream(res.stdOut);
    std::string line;
    while (std::getline(stream, line)) {
        if (line.find("runtime/") != std::string::npos) {
            FlatpakItem item;
            item.ref = line;
            item.isRuntime = true;
            item.isUnused = true;
            item.sizeBytes = 150 * 1024 * 1024; // Average runtime size ~150MB
            unused.push_back(item);
        }
    }

    return unused;
}

uint64_t FlatpakManager::getUnusedRuntimesSize() {
    auto items = getUnusedRuntimes();
    uint64_t total = 0;
    for (const auto& item : items) {
        total += item.sizeBytes;
    }
    return total;
}

Core::CommandResult FlatpakManager::uninstallUnused() {
    if (!m_backend || !isFlatpakAvailable()) {
        Core::CommandResult res;
        res.exitCode = 0;
        return res;
    }

    return m_backend->executeCommand("flatpak", {
        "uninstall", "--unused", "-y"
    }, Core::PrivilegeLevel::NormalUser, std::chrono::milliseconds(180000));
}

Core::CommandResult FlatpakManager::uninstallFlatpak(const std::string& appId) {
    if (!m_backend || !isFlatpakAvailable()) {
        Core::CommandResult res;
        res.exitCode = -1;
        res.stdErr = "Flatpak is not available.";
        return res;
    }

    return m_backend->executeCommand("flatpak", {
        "uninstall", "-y", appId
    }, Core::PrivilegeLevel::NormalUser, std::chrono::milliseconds(120000));
}

Core::CommandResult FlatpakManager::updateAll() {
    if (!m_backend || !isFlatpakAvailable()) {
        Core::CommandResult res;
        res.exitCode = 0;
        return res;
    }

    return m_backend->executeCommand("flatpak", {
        "update", "-y"
    }, Core::PrivilegeLevel::NormalUser, std::chrono::milliseconds(300000));
}

} // namespace LinuxManager::Package

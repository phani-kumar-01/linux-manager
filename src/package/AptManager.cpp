#include "AptManager.h"
#include <sstream>
#include <regex>
#include <iostream>
#include <algorithm>
#include <filesystem>

namespace LinuxManager::Package {

AptManager::AptManager(
    std::shared_ptr<Core::ISystemBackend> backend,
    std::shared_ptr<Security::OperationValidator> validator
) : m_backend(std::move(backend)), m_validator(std::move(validator)) {
    if (!m_validator) {
        auto policy = std::make_shared<Security::PolicyManager>();
        m_validator = std::make_shared<Security::OperationValidator>(policy);
    }
}

std::vector<PackageInfo> AptManager::parseDpkgStatus(const std::string& statusContent) {
    std::vector<PackageInfo> packages;
    std::istringstream stream(statusContent);
    std::string line;

    PackageInfo currentPkg;
    bool isInstalled = false;

    while (std::getline(stream, line)) {
        if (line.empty()) {
            if (!currentPkg.name.empty() && isInstalled) {
                packages.push_back(currentPkg);
            }
            currentPkg = PackageInfo{};
            isInstalled = false;
            continue;
        }

        if (line.starts_with("Package: ")) {
            currentPkg.name = line.substr(9);
        } else if (line.starts_with("Status: ")) {
            if (line.find("install ok installed") != std::string::npos) {
                isInstalled = true;
            }
        } else if (line.starts_with("Version: ")) {
            currentPkg.currentVersion = line.substr(9);
        } else if (line.starts_with("Section: ")) {
            currentPkg.section = line.substr(9);
        } else if (line.starts_with("Installed-Size: ")) {
            uint64_t kb = std::strtoull(line.substr(16).c_str(), nullptr, 10);
            currentPkg.installedSize = kb * 1024; // KB to Bytes
        } else if (line.starts_with("Architecture: ")) {
            currentPkg.architecture = line.substr(14);
        } else if (line.starts_with("Maintainer: ")) {
            currentPkg.maintainer = line.substr(12);
        } else if (line.starts_with("Description: ")) {
            currentPkg.summary = line.substr(13);
        }
    }

    if (!currentPkg.name.empty() && isInstalled) {
        packages.push_back(currentPkg);
    }

    return packages;
}

std::vector<PackageInfo> AptManager::getInstalledPackages() {
    if (!m_backend) return {};

    // First attempt fast /var/lib/dpkg/status parsing
    std::string statusContent = m_backend->readFileContents("/var/lib/dpkg/status");
    if (!statusContent.empty()) {
        return parseDpkgStatus(statusContent);
    }

    // Fallback to dpkg-query
    auto res = m_backend->executeCommand("dpkg-query", {
        "-W", "-f=${Package}|${Version}|${Installed-Size}|${Section}|${binary:Summary}\n"
    });

    std::vector<PackageInfo> packages;
    std::istringstream stream(res.stdOut);
    std::string line;
    while (std::getline(stream, line)) {
        if (line.empty()) continue;
        std::istringstream lineStream(line);
        std::string name, ver, sizeStr, sec, sum;
        if (std::getline(lineStream, name, '|') &&
            std::getline(lineStream, ver, '|') &&
            std::getline(lineStream, sizeStr, '|') &&
            std::getline(lineStream, sec, '|') &&
            std::getline(lineStream, sum)) {
            PackageInfo pkg;
            pkg.name = name;
            pkg.currentVersion = ver;
            pkg.installedSize = std::strtoull(sizeStr.c_str(), nullptr, 10) * 1024;
            pkg.section = sec;
            pkg.summary = sum;
            pkg.source = PackageSource::Apt;
            pkg.status = PackageStatus::Installed;
            packages.push_back(pkg);
        }
    }

    return packages;
}

std::vector<PackageInfo> AptManager::getOrphanPackages() {
    if (!m_backend) return {};

    auto res = m_backend->executeCommand("apt-get", {"--dry-run", "autoremove"});
    std::vector<PackageInfo> orphans;
    if (!res.success()) return orphans;

    std::istringstream stream(res.stdOut);
    std::string line;
    bool inRemovals = false;
    std::vector<std::string> orphanNames;

    while (std::getline(stream, line)) {
        if (line.find("The following packages will be REMOVED:") != std::string::npos) {
            inRemovals = true;
            continue;
        }
        if (inRemovals) {
            if (line.empty() || line.find(" upgraded, ") != std::string::npos || line.starts_with("WARNING")) {
                break;
            }
            std::istringstream pkgStream(line);
            std::string name;
            while (pkgStream >> name) {
                if (name.ends_with("*")) name.pop_back();
                if (!name.empty()) orphanNames.push_back(name);
            }
        }
    }

    for (const auto& name : orphanNames) {
        PackageInfo info = getPackageDetails(name);
        info.status = PackageStatus::Orphan;
        orphans.push_back(info);
    }

    return orphans;
}

std::vector<PackageInfo> AptManager::searchPackages(const std::string& query) {
    if (!m_backend || query.empty()) return {};

    auto res = m_backend->executeCommand("apt-cache", {"search", "--names-only", query});
    std::vector<PackageInfo> results;
    std::istringstream stream(res.stdOut);
    std::string line;

    while (std::getline(stream, line)) {
        if (line.empty()) continue;
        auto spacePos = line.find(" - ");
        if (spacePos != std::string::npos) {
            PackageInfo pkg;
            pkg.name = line.substr(0, spacePos);
            pkg.summary = line.substr(spacePos + 3);
            pkg.source = PackageSource::Apt;
            pkg.status = PackageStatus::Available;
            results.push_back(pkg);
        }
    }

    return results;
}

PackageInfo AptManager::getPackageDetails(const std::string& packageName) {
    PackageInfo pkg;
    pkg.name = packageName;
    pkg.source = PackageSource::Apt;
    if (!m_backend) return pkg;

    auto installed = getInstalledPackages();
    for (const auto& p : installed) {
        if (p.name == packageName) {
            return p;
        }
    }

    auto res = m_backend->executeCommand("apt-cache", {"show", packageName});
    std::istringstream stream(res.stdOut);
    std::string line;
    while (std::getline(stream, line)) {
        if (line.starts_with("Version: ") && pkg.currentVersion.empty()) {
            pkg.currentVersion = line.substr(9);
        } else if (line.starts_with("Installed-Size: ") && pkg.installedSize == 0) {
            pkg.installedSize = std::strtoull(line.substr(16).c_str(), nullptr, 10) * 1024;
        } else if (line.starts_with("Section: ") && pkg.section.empty()) {
            pkg.section = line.substr(9);
        } else if (line.starts_with("Description-en: ") || line.starts_with("Description: ")) {
            if (pkg.summary.empty()) {
                auto colon = line.find(':');
                pkg.summary = line.substr(colon + 2);
            }
        }
    }
    return pkg;
}

RemovalPreview AptManager::generateRemovalPreview(const std::string& packageName) {
    RemovalPreview preview;
    preview.targetPackage = packageName;
    if (!m_backend) return preview;

    PackageInfo info = getPackageDetails(packageName);
    preview.installedSizeBytes = info.installedSize;
    preview.totalReclaimableBytes = info.installedSize;

    std::string home = m_backend->getUserHomeDirectory();

    // Check common user config/cache leftover locations
    std::vector<std::string> possibleConfigs = {
        home + "/.config/" + packageName,
        home + "/." + packageName
    };
    for (const auto& path : possibleConfigs) {
        if (m_backend->pathExists(path)) {
            preview.configDirectories.push_back(path);
            uint64_t size = m_backend->getPathSize(path);
            preview.totalReclaimableBytes += size;
        }
    }

    std::vector<std::string> possibleCaches = {
        home + "/.cache/" + packageName
    };
    for (const auto& path : possibleCaches) {
        if (m_backend->pathExists(path)) {
            preview.cacheDirectories.push_back(path);
            uint64_t size = m_backend->getPathSize(path);
            preview.totalReclaimableBytes += size;
        }
    }

    return preview;
}

Core::CommandResult AptManager::installPackage(const std::string& packageName) {
    if (!m_backend) {
        Core::CommandResult res;
        res.exitCode = -1;
        res.stdErr = "Backend not initialized.";
        return res;
    }

    return m_backend->executePrivilegedOperation(
        Security::PrivilegedOperation::makeAptInstall({packageName})
    );
}

Core::CommandResult AptManager::removePackage(const std::string& packageName, bool purgeConfig) {
    if (!m_backend) {
        Core::CommandResult res;
        res.exitCode = -1;
        res.stdErr = "Backend not initialized.";
        return res;
    }

    return m_backend->executePrivilegedOperation(
        Security::PrivilegedOperation::makeAptRemove({packageName}, purgeConfig)
    );
}

Core::CommandResult AptManager::autoremoveOrphans() {
    if (!m_backend) {
        Core::CommandResult res;
        res.exitCode = -1;
        res.stdErr = "Backend not initialized.";
        return res;
    }

    return m_backend->executePrivilegedOperation(
        Security::PrivilegedOperation::makeAptAutoremove()
    );
}

Core::CommandResult AptManager::cleanAptCache() {
    if (!m_backend) {
        Core::CommandResult res;
        res.exitCode = -1;
        res.stdErr = "Backend not initialized.";
        return res;
    }

    return m_backend->executePrivilegedOperation(
        Security::PrivilegedOperation::makeAptClean()
    );
}

uint64_t AptManager::getAptCacheSize() {
    if (!m_backend) return 0;
    return m_backend->getPathSize("/var/cache/apt/archives");
}

} // namespace LinuxManager::Package

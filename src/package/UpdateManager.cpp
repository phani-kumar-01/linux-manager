#include "UpdateManager.h"
#include <sstream>
#include <regex>
#include <iostream>

namespace LinuxManager::Package {

UpdateManager::UpdateManager(std::shared_ptr<Core::ISystemBackend> backend)
    : m_backend(std::move(backend)) {}

Core::CommandResult UpdateManager::refreshRepositories() {
    if (!m_backend) {
        Core::CommandResult res;
        res.exitCode = -1;
        res.stdErr = "Backend not initialized.";
        return res;
    }

    return m_backend->executePrivilegedOperation(
        Security::PrivilegedOperation::makeAptUpdate()
    );
}

Core::CommandResult UpdateManager::applyAllUpdates() {
    if (!m_backend) {
        Core::CommandResult res;
        res.exitCode = -1;
        res.stdErr = "Backend not initialized.";
        return res;
    }

    return m_backend->executePrivilegedOperation(
        Security::PrivilegedOperation::makeAptUpgrade()
    );
}

Core::CommandResult UpdateManager::applySelectedUpdates(const std::vector<std::string>& packageNames) {
    if (!m_backend || packageNames.empty()) {
        Core::CommandResult res;
        res.exitCode = -1;
        res.stdErr = "No packages selected for upgrade.";
        return res;
    }

    return m_backend->executePrivilegedOperation(
        Security::PrivilegedOperation::makeAptInstall(packageNames)
    );
}

void UpdateManager::parseAptSimulateOutput(const std::string& output, UpdateSummary& summary) {
    std::istringstream stream(output);
    std::string line;

    // Pattern: Inst <pkg> [current_ver] (candidate_ver <archive> [<arch>])
    // Example: Inst libssl3 [3.0.13-0ubuntu3.4] (3.0.13-0ubuntu3.5 Ubuntu:24.04/noble-updates, Ubuntu:24.04/noble-security [amd64])
    static const std::regex instRegex(R"(^Inst\s+([^\s]+)\s+\[([^\]]+)\]\s+\(([^\s]+)\s+([^\)]+)\))");
    static const std::regex instNewRegex(R"(^Inst\s+([^\s]+)\s+\(([^\s]+)\s+([^\)]+)\))");

    while (std::getline(stream, line)) {
        std::smatch match;
        if (std::regex_search(line, match, instRegex)) {
            PackageInfo pkg;
            pkg.name = match[1].str();
            pkg.currentVersion = match[2].str();
            pkg.candidateVersion = match[3].str();
            std::string repoSource = match[4].str();

            pkg.source = PackageSource::Apt;
            pkg.status = PackageStatus::Upgradable;
            if (repoSource.find("security") != std::string::npos || repoSource.find("-sec") != std::string::npos) {
                pkg.isSecurityUpdate = true;
                summary.securityUpdates++;
            }

            summary.updates.push_back(pkg);
            summary.totalUpdates++;
        } else if (std::regex_search(line, match, instNewRegex)) {
            PackageInfo pkg;
            pkg.name = match[1].str();
            pkg.candidateVersion = match[2].str();
            std::string repoSource = match[3].str();

            pkg.source = PackageSource::Apt;
            pkg.status = PackageStatus::Upgradable;
            if (repoSource.find("security") != std::string::npos) {
                pkg.isSecurityUpdate = true;
                summary.securityUpdates++;
            }

            summary.updates.push_back(pkg);
            summary.totalUpdates++;
        }
    }
}

UpdateSummary UpdateManager::checkAvailableUpdates() {
    UpdateSummary summary;
    if (!m_backend) return summary;

    auto res = m_backend->executeCommand("apt-get", {"-s", "dist-upgrade"});
    if (res.success()) {
        parseAptSimulateOutput(res.stdOut, summary);
    }

    // Flatpak updates check if flatpak is installed
    auto flatpakCheck = m_backend->executeCommand("flatpak", {"remote-ls", "--updates"});
    if (flatpakCheck.success() && !flatpakCheck.stdOut.empty()) {
        std::istringstream stream(flatpakCheck.stdOut);
        std::string line;
        while (std::getline(stream, line)) {
            if (!line.empty()) {
                summary.flatpakUpdates++;
                summary.totalUpdates++;
            }
        }
    }

    return summary;
}

} // namespace LinuxManager::Package

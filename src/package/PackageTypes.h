#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace LinuxManager::Package {

enum class PackageSource {
    Apt,
    Flatpak,
    Snap
};

enum class PackageStatus {
    Installed,
    Upgradable,
    Available,
    Orphan,
    ResidualConfig
};

struct PackageInfo {
    std::string name;
    std::string currentVersion;
    std::string candidateVersion;
    std::string section;
    std::string summary;
    std::string description;
    uint64_t installedSize = 0; // in bytes
    uint64_t downloadSize = 0;  // in bytes
    PackageSource source = PackageSource::Apt;
    PackageStatus status = PackageStatus::Installed;
    bool isSecurityUpdate = false;
    std::string architecture;
    std::string maintainer;
    std::string installDate;
};

struct RemovalPreview {
    std::string targetPackage;
    uint64_t installedSizeBytes = 0;
    std::vector<std::string> configDirectories;
    std::vector<std::string> cacheDirectories;
    std::vector<std::string> orphanDependencies;
    uint64_t totalReclaimableBytes = 0;
};

} // namespace LinuxManager::Package

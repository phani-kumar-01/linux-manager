#pragma once

#include "PackageTypes.h"
#include "../core/ISystemBackend.h"
#include "../security/OperationValidator.h"
#include <memory>
#include <vector>
#include <string>

namespace LinuxManager::Package {

class AptManager {
public:
    AptManager(
        std::shared_ptr<Core::ISystemBackend> backend,
        std::shared_ptr<Security::OperationValidator> validator
    );

    [[nodiscard]] std::vector<PackageInfo> getInstalledPackages();
    [[nodiscard]] std::vector<PackageInfo> getOrphanPackages();
    [[nodiscard]] std::vector<PackageInfo> searchPackages(const std::string& query);
    [[nodiscard]] PackageInfo getPackageDetails(const std::string& packageName);

    [[nodiscard]] RemovalPreview generateRemovalPreview(const std::string& packageName);

    [[nodiscard]] Core::CommandResult installPackage(const std::string& packageName);
    [[nodiscard]] Core::CommandResult removePackage(const std::string& packageName, bool purgeConfig = false);
    [[nodiscard]] Core::CommandResult autoremoveOrphans();
    [[nodiscard]] Core::CommandResult cleanAptCache();
    [[nodiscard]] uint64_t getAptCacheSize();

private:
    std::shared_ptr<Core::ISystemBackend> m_backend;
    std::shared_ptr<Security::OperationValidator> m_validator;

    std::vector<PackageInfo> parseDpkgStatus(const std::string& statusContent);
};

} // namespace LinuxManager::Package

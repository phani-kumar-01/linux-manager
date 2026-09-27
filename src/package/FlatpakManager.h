#pragma once

#include "PackageTypes.h"
#include "../core/ISystemBackend.h"
#include <memory>
#include <vector>

namespace LinuxManager::Package {

struct FlatpakItem {
    std::string ref;
    std::string name;
    std::string appId;
    std::string version;
    std::string branch;
    std::string arch;
    std::string origin;
    uint64_t sizeBytes = 0;
    bool isRuntime = false;
    bool isUnused = false;
};

class FlatpakManager {
public:
    explicit FlatpakManager(std::shared_ptr<Core::ISystemBackend> backend);

    [[nodiscard]] bool isFlatpakAvailable();
    [[nodiscard]] std::vector<FlatpakItem> getInstalledFlatpaks();
    [[nodiscard]] std::vector<FlatpakItem> getUnusedRuntimes();
    [[nodiscard]] uint64_t getUnusedRuntimesSize();

    [[nodiscard]] Core::CommandResult uninstallUnused();
    [[nodiscard]] Core::CommandResult uninstallFlatpak(const std::string& appId);
    [[nodiscard]] Core::CommandResult updateAll();

private:
    std::shared_ptr<Core::ISystemBackend> m_backend;
};

} // namespace LinuxManager::Package

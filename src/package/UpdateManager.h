#pragma once

#include "PackageTypes.h"
#include "../core/ISystemBackend.h"
#include <memory>
#include <vector>

namespace LinuxManager::Package {

struct UpdateSummary {
    uint32_t totalUpdates = 0;
    uint32_t securityUpdates = 0;
    uint32_t flatpakUpdates = 0;
    uint64_t totalDownloadSizeBytes = 0;
    std::vector<PackageInfo> updates;
};

class UpdateManager {
public:
    explicit UpdateManager(std::shared_ptr<Core::ISystemBackend> backend);

    [[nodiscard]] Core::CommandResult refreshRepositories();
    [[nodiscard]] UpdateSummary checkAvailableUpdates();
    [[nodiscard]] Core::CommandResult applyAllUpdates();
    [[nodiscard]] Core::CommandResult applySelectedUpdates(const std::vector<std::string>& packageNames);

private:
    std::shared_ptr<Core::ISystemBackend> m_backend;
    void parseAptSimulateOutput(const std::string& output, UpdateSummary& summary);
};

} // namespace LinuxManager::Package

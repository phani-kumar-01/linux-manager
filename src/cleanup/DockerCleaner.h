#pragma once

#include "CleanupTypes.h"
#include "../core/ISystemBackend.h"
#include <memory>
#include <vector>

namespace LinuxManager::Cleanup {

struct DockerUsage {
    bool isAvailable = false;
    uint64_t imagesSize = 0;
    uint64_t containersSize = 0;
    uint64_t volumesSize = 0;
    uint64_t buildCacheSize = 0;
    uint64_t reclaimableImages = 0;
    uint64_t reclaimableContainers = 0;
    uint64_t reclaimableVolumes = 0;
    uint64_t reclaimableBuildCache = 0;
    uint64_t totalReclaimable = 0;
};

class DockerCleaner {
public:
    explicit DockerCleaner(std::shared_ptr<Core::ISystemBackend> backend);

    [[nodiscard]] bool isDockerAvailable();
    [[nodiscard]] DockerUsage getDockerUsage();
    [[nodiscard]] std::vector<CleanupItem> getDockerCleanupItems();

    [[nodiscard]] Core::CommandResult pruneContainers();
    [[nodiscard]] Core::CommandResult pruneImages();
    [[nodiscard]] Core::CommandResult pruneBuildCache();
    [[nodiscard]] Core::CommandResult pruneSystem(bool pruneVolumes = false);

private:
    std::shared_ptr<Core::ISystemBackend> m_backend;
    uint64_t parseSizeString(const std::string& sizeStr);
};

} // namespace LinuxManager::Cleanup

#pragma once

#include "StorageTypes.h"
#include "../core/ISystemBackend.h"
#include <memory>
#include <vector>
#include <string>

namespace LinuxManager::Storage {

class LargeFileScanner {
public:
    explicit LargeFileScanner(std::shared_ptr<Core::ISystemBackend> backend);

    [[nodiscard]] std::vector<LargeFileInfo> findLargeFiles(
        const std::string& rootDirectory,
        uint64_t minimumSizeBytes = 100ULL * 1024 * 1024, // 100 MB default
        size_t maxResults = 50
    );

private:
    std::shared_ptr<Core::ISystemBackend> m_backend;
};

} // namespace LinuxManager::Storage

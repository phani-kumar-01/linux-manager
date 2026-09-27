#pragma once

#include "StorageTypes.h"
#include "../core/ISystemBackend.h"
#include <memory>
#include <vector>
#include <string>

namespace LinuxManager::Storage {

class StorageScanner {
public:
    explicit StorageScanner(std::shared_ptr<Core::ISystemBackend> backend);

    [[nodiscard]] std::vector<DiskPartition> getPartitions();
    [[nodiscard]] StorageNode scanDirectoryTree(const std::string& rootPath, int maxDepth = 2);
    [[nodiscard]] std::vector<StorageNode> listDirectoryChildrenSorted(const std::string& dirPath);

private:
    std::shared_ptr<Core::ISystemBackend> m_backend;
    StorageNode scanNodeRecursive(const std::string& path, int currentDepth, int maxDepth);
};

} // namespace LinuxManager::Storage

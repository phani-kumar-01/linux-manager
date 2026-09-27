#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace LinuxManager::Storage {

struct StorageNode {
    std::string name;
    std::string path;
    uint64_t sizeBytes = 0;
    bool isDirectory = false;
    std::vector<StorageNode> children;
};

struct LargeFileInfo {
    std::string path;
    std::string filename;
    uint64_t sizeBytes = 0;
    std::string extension;
    std::string lastModified;
};

struct DiskPartition {
    std::string filesystem;
    std::string mountPoint;
    uint64_t totalBytes = 0;
    uint64_t usedBytes = 0;
    uint64_t availableBytes = 0;
    double usePercentage = 0.0;
};

} // namespace LinuxManager::Storage

#include "StorageScanner.h"
#include <sys/statvfs.h>
#include <filesystem>
#include <algorithm>
#include <system_error>
#include <sstream>

namespace LinuxManager::Storage {

StorageScanner::StorageScanner(std::shared_ptr<Core::ISystemBackend> backend)
    : m_backend(std::move(backend)) {}

std::vector<DiskPartition> StorageScanner::getPartitions() {
    std::vector<DiskPartition> partitions;
    if (!m_backend) return partitions;

    if (m_backend->isMock()) {
        DiskPartition p1;
        p1.filesystem = "/dev/nvme0n1p2";
        p1.mountPoint = "/";
        p1.totalBytes = 512ULL * 1024 * 1024 * 1024;
        p1.usedBytes = 184ULL * 1024 * 1024 * 1024;
        p1.availableBytes = 328ULL * 1024 * 1024 * 1024;
        p1.usePercentage = 36.0;
        partitions.push_back(p1);

        DiskPartition p2;
        p2.filesystem = "/dev/nvme0n1p1";
        p2.mountPoint = "/boot/efi";
        p2.totalBytes = 512ULL * 1024 * 1024;
        p2.usedBytes = 32ULL * 1024 * 1024;
        p2.availableBytes = 480ULL * 1024 * 1024;
        p2.usePercentage = 6.2;
        partitions.push_back(p2);

        return partitions;
    }

    auto res = m_backend->executeCommand("df", {"-B1", "-x", "tmpfs", "-x", "devtmpfs", "-x", "squashfs"});
    if (!res.success()) {
        // Fallback statvfs on root
        struct statvfs stat{};
        if (statvfs("/", &stat) == 0) {
            DiskPartition p;
            p.filesystem = "/dev/root";
            p.mountPoint = "/";
            p.totalBytes = stat.f_blocks * stat.f_frsize;
            p.availableBytes = stat.f_bavail * stat.f_frsize;
            p.usedBytes = p.totalBytes - (stat.f_bfree * stat.f_frsize);
            if (p.totalBytes > 0) {
                p.usePercentage = (static_cast<double>(p.usedBytes) / p.totalBytes) * 100.0;
            }
            partitions.push_back(p);
        }
        return partitions;
    }

    std::istringstream stream(res.stdOut);
    std::string line;
    bool first = true;
    while (std::getline(stream, line)) {
        if (first) { first = false; continue; }
        if (line.empty()) continue;

        std::istringstream ls(line);
        std::string fs, blocksStr, usedStr, availStr, usePctStr, mount;
        if (ls >> fs >> blocksStr >> usedStr >> availStr >> usePctStr >> mount) {
            DiskPartition p;
            p.filesystem = fs;
            p.mountPoint = mount;
            p.totalBytes = std::strtoull(blocksStr.c_str(), nullptr, 10);
            p.usedBytes = std::strtoull(usedStr.c_str(), nullptr, 10);
            p.availableBytes = std::strtoull(availStr.c_str(), nullptr, 10);
            if (p.totalBytes > 0) {
                p.usePercentage = (static_cast<double>(p.usedBytes) / p.totalBytes) * 100.0;
            }
            partitions.push_back(p);
        }
    }

    return partitions;
}

StorageNode StorageScanner::scanDirectoryTree(const std::string& rootPath, int maxDepth) {
    return scanNodeRecursive(rootPath, 0, maxDepth);
}

StorageNode StorageScanner::scanNodeRecursive(const std::string& path, int currentDepth, int maxDepth) {
    StorageNode node;
    node.path = path;
    std::filesystem::path p(path);
    node.name = p.filename().string();
    if (node.name.empty()) node.name = path;

    std::error_code ec;
    if (!std::filesystem::exists(p, ec)) return node;

    node.isDirectory = std::filesystem::is_directory(p, ec);
    if (!node.isDirectory) {
        node.sizeBytes = std::filesystem::file_size(p, ec);
        return node;
    }

    if (currentDepth < maxDepth) {
        for (const auto& entry : std::filesystem::directory_iterator(
                 p, std::filesystem::directory_options::skip_permission_denied, ec)) {
            if (ec) break;
            StorageNode child = scanNodeRecursive(entry.path().string(), currentDepth + 1, maxDepth);
            node.sizeBytes += child.sizeBytes;
            node.children.push_back(child);
        }
        std::sort(node.children.begin(), node.children.end(), [](const StorageNode& a, const StorageNode& b) {
            return a.sizeBytes > b.sizeBytes;
        });
    } else {
        if (m_backend) {
            node.sizeBytes = m_backend->getPathSize(path);
        }
    }

    return node;
}

std::vector<StorageNode> StorageScanner::listDirectoryChildrenSorted(const std::string& dirPath) {
    std::vector<StorageNode> children;
    std::error_code ec;
    std::filesystem::path p(dirPath);

    if (!std::filesystem::exists(p, ec) || !std::filesystem::is_directory(p, ec)) {
        return children;
    }

    for (const auto& entry : std::filesystem::directory_iterator(
             p, std::filesystem::directory_options::skip_permission_denied, ec)) {
        if (ec) break;

        StorageNode node;
        node.path = entry.path().string();
        node.name = entry.path().filename().string();
        node.isDirectory = entry.is_directory(ec);
        if (m_backend) {
            node.sizeBytes = m_backend->getPathSize(node.path);
        }
        children.push_back(node);
    }

    std::sort(children.begin(), children.end(), [](const StorageNode& a, const StorageNode& b) {
        return a.sizeBytes > b.sizeBytes;
    });

    return children;
}

} // namespace LinuxManager::Storage

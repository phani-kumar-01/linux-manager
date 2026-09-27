#include "LargeFileScanner.h"
#include <filesystem>
#include <algorithm>
#include <system_error>

namespace LinuxManager::Storage {

LargeFileScanner::LargeFileScanner(std::shared_ptr<Core::ISystemBackend> backend)
    : m_backend(std::move(backend)) {}

std::vector<LargeFileInfo> LargeFileScanner::findLargeFiles(
    const std::string& rootDirectory,
    uint64_t minimumSizeBytes,
    size_t maxResults
) {
    std::vector<LargeFileInfo> results;
    if (!m_backend) return results;

    if (m_backend->isMock()) {
        LargeFileInfo f1;
        f1.path = "/home/testuser/Downloads/ubuntu-24.04-desktop-amd64.iso";
        f1.filename = "ubuntu-24.04-desktop-amd64.iso";
        f1.sizeBytes = 5ULL * 1024 * 1024 * 1024;
        f1.extension = ".iso";
        results.push_back(f1);

        LargeFileInfo f2;
        f2.path = "/home/testuser/.cache/large_dataset.tar.gz";
        f2.filename = "large_dataset.tar.gz";
        f2.sizeBytes = 1200ULL * 1024 * 1024;
        f2.extension = ".gz";
        results.push_back(f2);

        return results;
    }

    std::error_code ec;
    std::filesystem::path root(rootDirectory);
    if (!std::filesystem::exists(root, ec) || !std::filesystem::is_directory(root, ec)) {
        return results;
    }

    for (const auto& entry : std::filesystem::recursive_directory_iterator(
             root, std::filesystem::directory_options::skip_permission_denied, ec)) {
        if (ec) break;

        if (entry.is_regular_file(ec)) {
            uint64_t size = entry.file_size(ec);
            if (!ec && size >= minimumSizeBytes) {
                LargeFileInfo info;
                info.path = entry.path().string();
                info.filename = entry.path().filename().string();
                info.sizeBytes = size;
                info.extension = entry.path().extension().string();
                results.push_back(info);
            }
        }
    }

    std::sort(results.begin(), results.end(), [](const LargeFileInfo& a, const LargeFileInfo& b) {
        return a.sizeBytes > b.sizeBytes;
    });

    if (results.size() > maxResults) {
        results.resize(maxResults);
    }

    return results;
}

} // namespace LinuxManager::Storage

#pragma once

#include "CleanupTypes.h"
#include "../core/ISystemBackend.h"
#include <memory>
#include <vector>
#include <string>

namespace LinuxManager::Cleanup {

struct DeveloperProjectItem {
    std::string projectPath;
    std::string artifactType; // "node_modules", "target", ".venv", "__pycache__", ".gradle"
    uint64_t sizeBytes = 0;
    std::string lastModified;
};

class DeveloperCleaner {
public:
    explicit DeveloperCleaner(std::shared_ptr<Core::ISystemBackend> backend);

    // Global Language Ecosystem Caches
    [[nodiscard]] std::vector<CleanupItem> scanAllDeveloperCaches();
    [[nodiscard]] CleanupItem scanNodeGlobalCache();
    [[nodiscard]] CleanupItem scanPythonGlobalCache();
    [[nodiscard]] CleanupItem scanJavaGlobalCache();
    [[nodiscard]] CleanupItem scanRustGlobalCache();
    [[nodiscard]] CleanupItem scanAndroidGlobalCache();

    // Deep Project Directory Scans (e.g. ~/Projects, ~/workspace)
    [[nodiscard]] std::vector<DeveloperProjectItem> scanProjectDirectories(
        const std::vector<std::string>& searchRoots,
        int maxDepth = 4
    );

private:
    std::shared_ptr<Core::ISystemBackend> m_backend;
    void scanDirectoryRecursive(
        const std::string& currentPath,
        int currentDepth,
        int maxDepth,
        std::vector<DeveloperProjectItem>& results
    );
};

} // namespace LinuxManager::Cleanup

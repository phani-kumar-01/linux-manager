#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace LinuxManager::Cleanup {

enum class CleanupCategory {
    AptCache,
    SystemLogs,
    ThumbnailCache,
    CrashReports,
    TempFiles,
    FlatpakUnused,
    DockerResources,
    NodeDeveloper,
    PythonDeveloper,
    JavaDeveloper,
    RustDeveloper,
    AndroidDeveloper,
    Custom
};

enum class RiskLevel {
    Safe,       // Can be cleaned with high confidence (e.g. thumbnails, apt cache, crash dumps)
    Moderate,   // Dev build artifacts or old logs (e.g. project target/, project node_modules)
    Caution     // SDK components, Docker volumes, virtualenvs (requires explicit user confirmation)
};

struct CleanupItem {
    std::string id;
    std::string title;
    std::string path;
    std::string description;
    uint64_t sizeBytes = 0;
    CleanupCategory category = CleanupCategory::Custom;
    RiskLevel risk = RiskLevel::Safe;
    bool isSelected = true;
    bool requiresElevation = false;
    std::vector<std::string> subItems;
};

struct CleanupPreview {
    uint64_t totalReclaimableBytes = 0;
    uint64_t selectedReclaimableBytes = 0;
    std::vector<CleanupItem> items;
};

struct CleanupExecutionResult {
    bool success = true;
    std::string operationId;
    uint64_t bytesCleaned = 0;
    size_t itemsCleanedCount = 0;
    std::vector<std::string> failedItems;
    std::string details;
};

} // namespace LinuxManager::Cleanup

#pragma once

#include "CleanupTypes.h"
#include "../core/ISystemBackend.h"
#include <memory>
#include <vector>

namespace LinuxManager::Cleanup {

class CacheScanner {
public:
    explicit CacheScanner(std::shared_ptr<Core::ISystemBackend> backend);

    [[nodiscard]] std::vector<CleanupItem> scanSystemCaches();
    [[nodiscard]] CleanupItem scanAptCache();
    [[nodiscard]] CleanupItem scanJournalLogs();
    [[nodiscard]] CleanupItem scanThumbnails();
    [[nodiscard]] CleanupItem scanCrashReports();
    [[nodiscard]] CleanupItem scanBrowserCaches();
    [[nodiscard]] CleanupItem scanTempFiles();

private:
    std::shared_ptr<Core::ISystemBackend> m_backend;
};

} // namespace LinuxManager::Cleanup

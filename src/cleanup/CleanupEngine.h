#pragma once

#include "CleanupTypes.h"
#include "CacheScanner.h"
#include "DeveloperCleaner.h"
#include "DockerCleaner.h"
#include "../core/ISystemBackend.h"
#include "../core/HistoryManager.h"
#include "../security/OperationValidator.h"
#include <memory>
#include <vector>

namespace LinuxManager::Cleanup {

class CleanupEngine {
public:
    CleanupEngine(
        std::shared_ptr<Core::ISystemBackend> backend,
        std::shared_ptr<Security::OperationValidator> validator,
        std::shared_ptr<Core::HistoryManager> history
    );

    [[nodiscard]] CleanupPreview scanAll();
    [[nodiscard]] CleanupPreview scanCategory(CleanupCategory category);

    [[nodiscard]] CleanupExecutionResult executeCleanup(
        const std::vector<CleanupItem>& itemsToClean,
        bool dryRun = false
    );

    [[nodiscard]] std::shared_ptr<CacheScanner> getCacheScanner() const noexcept { return m_cacheScanner; }
    [[nodiscard]] std::shared_ptr<DeveloperCleaner> getDeveloperCleaner() const noexcept { return m_devCleaner; }
    [[nodiscard]] std::shared_ptr<DockerCleaner> getDockerCleaner() const noexcept { return m_dockerCleaner; }

private:
    std::shared_ptr<Core::ISystemBackend> m_backend;
    std::shared_ptr<Security::OperationValidator> m_validator;
    std::shared_ptr<Core::HistoryManager> m_history;

    std::shared_ptr<CacheScanner> m_cacheScanner;
    std::shared_ptr<DeveloperCleaner> m_devCleaner;
    std::shared_ptr<DockerCleaner> m_dockerCleaner;

    bool cleanSingleItem(const CleanupItem& item, std::string& errorMsg);
};

} // namespace LinuxManager::Cleanup

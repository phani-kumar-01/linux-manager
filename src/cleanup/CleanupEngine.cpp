#include "CleanupEngine.h"
#include <iomanip>
#include <sstream>
#include <ctime>

namespace LinuxManager::Cleanup {

CleanupEngine::CleanupEngine(
    std::shared_ptr<Core::ISystemBackend> backend,
    std::shared_ptr<Security::OperationValidator> validator,
    std::shared_ptr<Core::HistoryManager> history
) : m_backend(std::move(backend)),
    m_validator(std::move(validator)),
    m_history(std::move(history)) {
    
    m_cacheScanner = std::make_shared<CacheScanner>(m_backend);
    m_devCleaner = std::make_shared<DeveloperCleaner>(m_backend);
    m_dockerCleaner = std::make_shared<DockerCleaner>(m_backend);
}

CleanupPreview CleanupEngine::scanAll() {
    CleanupPreview preview;
    if (!m_backend) return preview;

    auto systemCaches = m_cacheScanner->scanSystemCaches();
    for (const auto& item : systemCaches) {
        if (item.sizeBytes > 0) {
            preview.items.push_back(item);
            preview.totalReclaimableBytes += item.sizeBytes;
            if (item.isSelected) {
                preview.selectedReclaimableBytes += item.sizeBytes;
            }
        }
    }

    auto devCaches = m_devCleaner->scanAllDeveloperCaches();
    for (const auto& item : devCaches) {
        if (item.sizeBytes > 0) {
            preview.items.push_back(item);
            preview.totalReclaimableBytes += item.sizeBytes;
            if (item.isSelected) {
                preview.selectedReclaimableBytes += item.sizeBytes;
            }
        }
    }

    auto dockerItems = m_dockerCleaner->getDockerCleanupItems();
    for (const auto& item : dockerItems) {
        if (item.sizeBytes > 0) {
            preview.items.push_back(item);
            preview.totalReclaimableBytes += item.sizeBytes;
            if (item.isSelected) {
                preview.selectedReclaimableBytes += item.sizeBytes;
            }
        }
    }

    return preview;
}

bool CleanupEngine::cleanSingleItem(const CleanupItem& item, std::string& errorMsg) {
    if (!m_backend) {
        errorMsg = "Backend not initialized.";
        return false;
    }

    // Special items handled by specific commands
    if (item.id == "system-apt-cache") {
        auto res = m_backend->executePrivilegedOperation(
            Security::PrivilegedOperation::makeAptClean()
        );
        if (!res.success()) {
            errorMsg = res.stdErr;
            return false;
        }
        return true;
    }

    if (item.id == "system-journal-logs") {
        // Safe vacuum to 100M
        auto res = m_backend->executePrivilegedOperation(
            Security::PrivilegedOperation::makeJournalVacuumSize(100)
        );
        if (!res.success()) {
            errorMsg = res.stdErr;
            return false;
        }
        return true;
    }

    if (item.id == "docker-containers") {
        auto res = m_dockerCleaner->pruneContainers();
        return res.success();
    }

    if (item.id == "docker-images") {
        auto res = m_dockerCleaner->pruneImages();
        return res.success();
    }

    if (item.id == "docker-build-cache") {
        auto res = m_dockerCleaner->pruneBuildCache();
        return res.success();
    }

    std::string home = m_backend->getUserHomeDirectory();

    // Standard filesystem paths
    if (!item.subItems.empty()) {
        bool allOk = true;
        for (const auto& sub : item.subItems) {
            auto val = m_validator->validateFileDeletion(sub, home);
            if (!val) {
                errorMsg += "Path rejected by policy: " + sub + "; ";
                allOk = false;
                continue;
            }
            if (!m_backend->removePath(sub, true)) {
                allOk = false;
            }
        }
        return allOk;
    } else if (!item.path.empty()) {
        auto val = m_validator->validateFileDeletion(item.path, home);
        if (!val) {
            errorMsg = val.reason;
            return false;
        }
        return m_backend->removePath(item.path, true);
    }

    return true;
}

CleanupExecutionResult CleanupEngine::executeCleanup(
    const std::vector<CleanupItem>& itemsToClean,
    bool dryRun
) {
    CleanupExecutionResult result;
    if (m_history) {
        result.operationId = m_history->generateOperationId();
    } else {
        result.operationId = "OP-PREVIEW";
    }

    if (dryRun) {
        for (const auto& item : itemsToClean) {
            if (item.isSelected) {
                result.bytesCleaned += item.sizeBytes;
                result.itemsCleanedCount++;
            }
        }
        result.details = "Dry Run preview completed without modifying files.";
        return result;
    }

    for (const auto& item : itemsToClean) {
        if (!item.isSelected) continue;

        std::string err;
        if (cleanSingleItem(item, err)) {
            result.bytesCleaned += item.sizeBytes;
            result.itemsCleanedCount++;
        } else {
            result.failedItems.push_back(item.title + " (" + err + ")");
            result.success = false;
        }
    }

    // Record in history
    if (m_history && result.itemsCleanedCount > 0) {
        auto now = std::chrono::system_clock::now();
        std::time_t tt = std::chrono::system_clock::to_time_t(now);
        std::tm tm{};
        localtime_r(&tt, &tm);
        std::ostringstream ts;
        ts << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");

        Core::HistoryRecord rec;
        rec.operationId = result.operationId;
        rec.timestamp = ts.str();
        rec.category = "System Cleanup";
        rec.bytesReclaimed = result.bytesCleaned;
        rec.itemsCount = result.itemsCleanedCount;
        rec.success = result.success;
        std::ostringstream dss;
        dss << "Cleaned " << result.itemsCleanedCount << " items, reclaiming "
            << (result.bytesCleaned / (1024 * 1024)) << " MB";
        rec.description = dss.str();
        rec.details = result.details;

        m_history->recordOperation(rec);
    }

    return result;
}

} // namespace LinuxManager::Cleanup

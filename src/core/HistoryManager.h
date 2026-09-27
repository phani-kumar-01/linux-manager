#pragma once

#include "ISystemBackend.h"
#include <string>
#include <vector>
#include <chrono>
#include <memory>

namespace LinuxManager::Core {

struct HistoryRecord {
    std::string operationId;
    std::string timestamp;
    std::string category;
    std::string description;
    uint64_t bytesReclaimed = 0;
    size_t itemsCount = 0;
    bool success = true;
    std::string details;
};

class HistoryManager {
public:
    explicit HistoryManager(std::shared_ptr<ISystemBackend> backend);

    void recordOperation(const HistoryRecord& record);
    [[nodiscard]] std::vector<HistoryRecord> getHistory() const;
    void clearHistory();
    [[nodiscard]] std::string generateOperationId();

    [[nodiscard]] uint64_t getTotalReclaimedBytes() const;

private:
    std::shared_ptr<ISystemBackend> m_backend;
    std::string m_historyFilePath;
    void ensureStorageDirectoryExists() const;
};

} // namespace LinuxManager::Core

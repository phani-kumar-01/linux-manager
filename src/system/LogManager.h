#pragma once

#include "../core/ISystemBackend.h"
#include <memory>
#include <vector>
#include <string>

namespace LinuxManager::System {

struct JournalInfo {
    uint64_t diskSizeBytes = 0;
    std::string oldestLogDate;
    std::string newestLogDate;
};

class LogManager {
public:
    explicit LogManager(std::shared_ptr<Core::ISystemBackend> backend);

    [[nodiscard]] JournalInfo getJournalInfo();
    [[nodiscard]] Core::CommandResult vacuumBySize(uint64_t targetSizeBytes);
    [[nodiscard]] Core::CommandResult vacuumByDays(uint32_t retentionDays);
    [[nodiscard]] std::string getRecentErrors(int lines = 50);

private:
    std::shared_ptr<Core::ISystemBackend> m_backend;
};

} // namespace LinuxManager::System

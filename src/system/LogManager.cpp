#include "LogManager.h"
#include <regex>
#include <sstream>

namespace LinuxManager::System {

LogManager::LogManager(std::shared_ptr<Core::ISystemBackend> backend)
    : m_backend(std::move(backend)) {}

JournalInfo LogManager::getJournalInfo() {
    JournalInfo info;
    if (!m_backend) return info;

    if (m_backend->isMock()) {
        info.diskSizeBytes = 1800ULL * 1024 * 1024;
        info.oldestLogDate = "2026-07-10 09:30:00";
        info.newestLogDate = "2026-08-22 21:15:00";
        return info;
    }

    auto res = m_backend->executeCommand("journalctl", {"--disk-usage"});
    if (res.success()) {
        static const std::regex journalRegex(R"(take up\s+([0-9.]+)([KMGTP]?)B)");
        std::smatch match;
        if (std::regex_search(res.stdOut, match, journalRegex)) {
            double val = std::stod(match[1].str());
            std::string unit = match[2].str();
            uint64_t multiplier = 1;
            if (unit == "K") multiplier = 1024ULL;
            else if (unit == "M") multiplier = 1024ULL * 1024ULL;
            else if (unit == "G") multiplier = 1024ULL * 1024ULL * 1024ULL;
            info.diskSizeBytes = static_cast<uint64_t>(val * multiplier);
        }
    }

    return info;
}

Core::CommandResult LogManager::vacuumBySize(uint64_t targetSizeBytes) {
    if (!m_backend) return {};

    uint64_t targetMb = targetSizeBytes / (1024 * 1024);
    if (targetMb < 50) targetMb = 50;

    return m_backend->executePrivilegedOperation(
        Security::PrivilegedOperation::makeJournalVacuumSize(targetMb)
    );
}

Core::CommandResult LogManager::vacuumByDays(uint32_t retentionDays) {
    if (!m_backend) return {};

    if (retentionDays < 1) retentionDays = 1;
    return m_backend->executePrivilegedOperation(
        Security::PrivilegedOperation::makeJournalVacuumTime(retentionDays)
    );
}

std::string LogManager::getRecentErrors(int lines) {
    if (!m_backend) return "";

    if (m_backend->isMock()) {
        return "Aug 22 20:00:01 host kernel: pcieport 0000:00:1c.5: AER: Corrected error received";
    }

    auto res = m_backend->executeCommand("journalctl", {
        "-p", "3", "-n", std::to_string(lines), "--no-pager"
    });

    return res.stdOut;
}

} // namespace LinuxManager::System

#include "HistoryManager.h"
#include "MockSystemBackend.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <filesystem>
#include <atomic>

namespace LinuxManager::Core {

static std::atomic<uint32_t> s_opCounter{1};

HistoryManager::HistoryManager(std::shared_ptr<ISystemBackend> backend)
    : m_backend(std::move(backend)) {
    if (m_backend) {
        m_historyFilePath = m_backend->getUserHomeDirectory() + "/.local/share/linux-manager/history.log";
    } else {
        m_historyFilePath = "/tmp/linux-manager-history.log";
    }
}

std::string HistoryManager::generateOperationId() {
    auto now = std::chrono::system_clock::now();
    std::time_t tt = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
    gmtime_r(&tt, &tm);

    std::ostringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d-") << std::setfill('0') << std::setw(3) << (s_opCounter++);
    return ss.str();
}

void HistoryManager::ensureStorageDirectoryExists() const {
    if (m_backend && m_backend->isMock()) return;
    std::filesystem::path p(m_historyFilePath);
    std::error_code ec;
    std::filesystem::create_directories(p.parent_path(), ec);
}

void HistoryManager::recordOperation(const HistoryRecord& record) {
    if (m_backend && m_backend->isMock()) {
        std::string existing = m_backend->readFileContents(m_historyFilePath);
        std::ostringstream ss;
        ss << record.operationId << "|" << record.timestamp << "|" << record.category << "|"
           << record.description << "|" << record.bytesReclaimed << "|" << record.itemsCount << "|"
           << (record.success ? "1" : "0") << "|" << (record.details.empty() ? "-" : record.details) << "\n";
        m_backend->removePath(m_historyFilePath);
        // Save back
        std::string updated = existing + ss.str();
        auto mock = std::dynamic_pointer_cast<MockSystemBackend>(m_backend);
        if (mock) {
            mock->setMockFile(m_historyFilePath, updated);
        }
        return;
    }

    ensureStorageDirectoryExists();
    std::ofstream file(m_historyFilePath, std::ios::app);
    if (!file.is_open()) return;

    file << record.operationId << "|"
         << record.timestamp << "|"
         << record.category << "|"
         << record.description << "|"
         << record.bytesReclaimed << "|"
         << record.itemsCount << "|"
         << (record.success ? "1" : "0") << "|"
         << (record.details.empty() ? "-" : record.details) << "\n";
}

std::vector<HistoryRecord> HistoryManager::getHistory() const {
    std::vector<HistoryRecord> history;
    std::string content;

    if (m_backend && m_backend->isMock()) {
        content = m_backend->readFileContents(m_historyFilePath);
    } else {
        std::ifstream file(m_historyFilePath);
        if (!file.is_open()) return history;
        std::stringstream ss;
        ss << file.rdbuf();
        content = ss.str();
    }

    std::istringstream stream(content);
    std::string line;
    while (std::getline(stream, line)) {
        if (line.empty()) continue;
        std::istringstream lineStream(line);
        std::string opId, ts, cat, desc, bytesStr, itemsStr, succStr, det;

        if (std::getline(lineStream, opId, '|') &&
            std::getline(lineStream, ts, '|') &&
            std::getline(lineStream, cat, '|') &&
            std::getline(lineStream, desc, '|') &&
            std::getline(lineStream, bytesStr, '|') &&
            std::getline(lineStream, itemsStr, '|') &&
            std::getline(lineStream, succStr, '|')) {
            std::getline(lineStream, det);

            HistoryRecord rec;
            rec.operationId = opId;
            rec.timestamp = ts;
            rec.category = cat;
            rec.description = desc;
            rec.bytesReclaimed = std::strtoull(bytesStr.c_str(), nullptr, 10);
            rec.itemsCount = std::strtoul(itemsStr.c_str(), nullptr, 10);
            rec.success = (succStr == "1");
            rec.details = (det == "-") ? "" : det;

            history.push_back(rec);
        }
    }

    return history;
}

void HistoryManager::clearHistory() {
    if (m_backend) {
        m_backend->removePath(m_historyFilePath);
    }
}

uint64_t HistoryManager::getTotalReclaimedBytes() const {
    auto records = getHistory();
    uint64_t total = 0;
    for (const auto& rec : records) {
        if (rec.success) {
            total += rec.bytesReclaimed;
        }
    }
    return total;
}

} // namespace LinuxManager::Core

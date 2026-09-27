#pragma once

#include "../core/ISystemBackend.h"
#include <string>
#include <vector>
#include <cstdint>
#include <memory>

namespace LinuxManager::System {

struct OsInfo {
    std::string name;
    std::string version;
    std::string id;
    std::string idLike;
    std::string versionId;
    std::string prettyName;
    std::string codename;
    bool isDebianBased = true;
};

struct CpuInfo {
    std::string model;
    std::string architecture;
    uint32_t logicalCores = 1;
    uint32_t physicalCores = 1;
    double currentUsagePercentage = 0.0;
    double currentTemperatureCelsius = 0.0;
};

struct MemoryInfo {
    uint64_t totalBytes = 0;
    uint64_t usedBytes = 0;
    uint64_t availableBytes = 0;
    uint64_t swapTotalBytes = 0;
    uint64_t swapUsedBytes = 0;
    double usagePercentage = 0.0;
};

struct BatteryInfo {
    bool present = false;
    std::string status;
    double capacityPercentage = 100.0;
    bool isCharging = false;
};

struct SystemOverview {
    OsInfo os;
    CpuInfo cpu;
    MemoryInfo memory;
    BatteryInfo battery;
    std::string kernelVersion;
    std::string hostName;
    std::string uptimeFormatted;
    uint64_t uptimeSeconds = 0;
};

class SystemInfo {
public:
    explicit SystemInfo(std::shared_ptr<Core::ISystemBackend> backend);

    [[nodiscard]] OsInfo getOsInfo();
    [[nodiscard]] CpuInfo getCpuInfo();
    [[nodiscard]] MemoryInfo getMemoryInfo();
    [[nodiscard]] BatteryInfo getBatteryInfo();
    [[nodiscard]] SystemOverview getSystemOverview();

    [[nodiscard]] double sampleCpuUsage();
    [[nodiscard]] double readCpuTemperature();

private:
    std::shared_ptr<Core::ISystemBackend> m_backend;
    uint64_t m_lastTotalTime = 0;
    uint64_t m_lastIdleTime = 0;
};

} // namespace LinuxManager::System

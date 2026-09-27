#pragma once

#include "../core/ISystemBackend.h"
#include <string>
#include <vector>
#include <memory>
#include <cstdint>

namespace LinuxManager::System {

struct BatteryHealth {
    bool hasBattery = false;
    std::string deviceName = "BAT0";
    std::string modelName;
    std::string status; // "Charging", "Discharging", "Full", "Not charging"
    double currentPercent = 0.0;
    double healthPercent = 100.0; // Energy full / Energy design * 100
    uint64_t designCapacityWh = 0;
    uint64_t fullCapacityWh = 0;
    int cycleCount = 0;
    double dischargeRateWatts = 0.0;
};

struct PowerProfileInfo {
    std::string activeProfile = "balanced";
    std::vector<std::string> availableProfiles = {"power-saver", "balanced", "performance"};
    std::string daemonBackend = "power-profiles-daemon";
    bool isAvailable = false;
};

class PowerManager {
public:
    explicit PowerManager(std::shared_ptr<Core::ISystemBackend> backend);

    [[nodiscard]] BatteryHealth getBatteryHealth();
    [[nodiscard]] PowerProfileInfo getPowerProfiles();
    [[nodiscard]] Core::CommandResult setPowerProfile(const std::string& profile);

private:
    std::shared_ptr<Core::ISystemBackend> m_backend;

    [[nodiscard]] uint64_t readSysfsUint64(const std::string& path) const;
    [[nodiscard]] std::string readSysfsString(const std::string& path) const;
};

} // namespace LinuxManager::System

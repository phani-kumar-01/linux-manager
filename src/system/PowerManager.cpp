#include "PowerManager.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <cmath>

namespace LinuxManager::System {

PowerManager::PowerManager(std::shared_ptr<Core::ISystemBackend> backend)
    : m_backend(std::move(backend)) {}

uint64_t PowerManager::readSysfsUint64(const std::string& path) const {
    if (!m_backend) return 0;
    std::string s = m_backend->readFileContents(path);
    if (s.empty()) return 0;
    try {
        return std::stoull(s);
    } catch (...) {
        return 0;
    }
}

std::string PowerManager::readSysfsString(const std::string& path) const {
    if (!m_backend) return "";
    std::string s = m_backend->readFileContents(path);
    while (!s.empty() && (s.back() == '\n' || s.back() == '\r')) s.pop_back();
    return s;
}

BatteryHealth PowerManager::getBatteryHealth() {
    BatteryHealth bh;

    if (m_backend && m_backend->isMock()) {
        bh.hasBattery = true;
        bh.deviceName = "BAT0";
        bh.modelName = "Li-ion Primary Battery";
        bh.status = "Discharging";
        bh.currentPercent = 84.5;
        bh.healthPercent = 92.4;
        bh.designCapacityWh = 72;
        bh.fullCapacityWh = 66;
        bh.cycleCount = 142;
        bh.dischargeRateWatts = 11.2;
        return bh;
    }

    if (!m_backend) return bh;

    std::string base = "/sys/class/power_supply";
    std::error_code ec;
    if (!std::filesystem::exists(base, ec)) return bh;

    for (const auto& entry : std::filesystem::directory_iterator(base, ec)) {
        std::string filename = entry.path().filename().string();
        if (filename.starts_with("BAT")) {
            bh.hasBattery = true;
            bh.deviceName = filename;
            std::string batPath = entry.path().string();

            bh.modelName = readSysfsString(batPath + "/model_name");
            if (bh.modelName.empty()) bh.modelName = "Standard Battery";

            bh.status = readSysfsString(batPath + "/status");
            bh.cycleCount = static_cast<int>(readSysfsUint64(batPath + "/cycle_count"));

            // Energy or Charge (in micro-watt-hours or micro-amp-hours)
            uint64_t energyNow = readSysfsUint64(batPath + "/energy_now");
            uint64_t energyFull = readSysfsUint64(batPath + "/energy_full");
            uint64_t energyDesign = readSysfsUint64(batPath + "/energy_full_design");
            uint64_t powerNow = readSysfsUint64(batPath + "/power_now");

            if (energyFull == 0) {
                // Fallback to charge_*
                energyNow = readSysfsUint64(batPath + "/charge_now");
                energyFull = readSysfsUint64(batPath + "/charge_full");
                energyDesign = readSysfsUint64(batPath + "/charge_full_design");
                powerNow = readSysfsUint64(batPath + "/current_now");
            }

            if (energyFull > 0 && energyDesign > 0) {
                bh.healthPercent = (static_cast<double>(energyFull) / static_cast<double>(energyDesign)) * 100.0;
                if (bh.healthPercent > 100.0) bh.healthPercent = 100.0;
            }

            if (energyNow > 0 && energyFull > 0) {
                bh.currentPercent = (static_cast<double>(energyNow) / static_cast<double>(energyFull)) * 100.0;
            } else {
                bh.currentPercent = static_cast<double>(readSysfsUint64(batPath + "/capacity"));
            }

            bh.fullCapacityWh = energyFull / 1000000;
            bh.designCapacityWh = energyDesign / 1000000;
            bh.dischargeRateWatts = static_cast<double>(powerNow) / 1000000.0;

            break;
        }
    }

    return bh;
}

PowerProfileInfo PowerManager::getPowerProfiles() {
    PowerProfileInfo info;

    if (m_backend && m_backend->isMock()) {
        info.isAvailable = true;
        info.activeProfile = "balanced";
        info.availableProfiles = {"power-saver", "balanced", "performance"};
        return info;
    }

    if (!m_backend) return info;

    // Check powerprofilesctl
    auto checkRes = m_backend->executeCommand("which", {"powerprofilesctl"});
    if (checkRes.success()) {
        info.isAvailable = true;
        auto getRes = m_backend->executeCommand("powerprofilesctl", {"get"});
        if (getRes.success() && !getRes.stdOut.empty()) {
            std::string act = getRes.stdOut;
            while (!act.empty() && (act.back() == '\n' || act.back() == '\r')) act.pop_back();
            info.activeProfile = act;
        }
    } else {
        // Check system76-power (Pop!_OS)
        auto s76Check = m_backend->executeCommand("which", {"system76-power"});
        if (s76Check.success()) {
            info.isAvailable = true;
            info.daemonBackend = "system76-power";
            auto getRes = m_backend->executeCommand("system76-power", {"profile"});
            if (getRes.success() && !getRes.stdOut.empty()) {
                std::string act = getRes.stdOut;
                while (!act.empty() && (act.back() == '\n' || act.back() == '\r')) act.pop_back();
                info.activeProfile = act;
            }
        }
    }

    return info;
}

Core::CommandResult PowerManager::setPowerProfile(const std::string& profile) {
    if (!m_backend) {
        Core::CommandResult err;
        err.exitCode = -1;
        err.stdErr = "Backend not initialized.";
        return err;
    }

    if (m_backend->isMock()) {
        Core::CommandResult res;
        res.exitCode = 0;
        res.stdOut = "Mock power profile set to " + profile;
        return res;
    }

    auto profInfo = getPowerProfiles();
    if (profInfo.daemonBackend == "system76-power") {
        return m_backend->executeCommand("system76-power", {"profile", profile});
    }

    return m_backend->executeCommand("powerprofilesctl", {"set", profile});
}

} // namespace LinuxManager::System

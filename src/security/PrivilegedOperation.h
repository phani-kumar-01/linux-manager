#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace LinuxManager::Security {

enum class PrivilegedOpType {
    AptUpdate,
    AptInstall,
    AptRemove,
    AptPurge,
    AptUpgrade,
    AptAutoremove,
    AptClean,
    JournalVacuumSize,
    JournalVacuumTime,
    ServiceRestart,
    ServiceStart,
    ServiceStop,
    ServiceEnable,
    ServiceDisable,
    KernelPurge
};

struct PrivilegedOperation {
    PrivilegedOpType type;
    std::vector<std::string> packages; // For AptInstall, AptRemove, AptPurge, KernelPurge
    std::string serviceUnit;           // For Service*
    uint64_t targetSizeMb = 100;       // For JournalVacuumSize
    uint32_t retentionDays = 14;       // For JournalVacuumTime

    // Convenience constructors
    static PrivilegedOperation makeAptUpdate() {
        return PrivilegedOperation{PrivilegedOpType::AptUpdate, {}, "", 0, 0};
    }

    static PrivilegedOperation makeAptInstall(const std::vector<std::string>& pkgs) {
        return PrivilegedOperation{PrivilegedOpType::AptInstall, pkgs, "", 0, 0};
    }

    static PrivilegedOperation makeAptRemove(const std::vector<std::string>& pkgs, bool purge = false) {
        return PrivilegedOperation{
            purge ? PrivilegedOpType::AptPurge : PrivilegedOpType::AptRemove,
            pkgs, "", 0, 0
        };
    }

    static PrivilegedOperation makeAptUpgrade() {
        return PrivilegedOperation{PrivilegedOpType::AptUpgrade, {}, "", 0, 0};
    }

    static PrivilegedOperation makeAptAutoremove() {
        return PrivilegedOperation{PrivilegedOpType::AptAutoremove, {}, "", 0, 0};
    }

    static PrivilegedOperation makeAptClean() {
        return PrivilegedOperation{PrivilegedOpType::AptClean, {}, "", 0, 0};
    }

    static PrivilegedOperation makeJournalVacuumSize(uint64_t sizeMb) {
        return PrivilegedOperation{PrivilegedOpType::JournalVacuumSize, {}, "", sizeMb, 0};
    }

    static PrivilegedOperation makeJournalVacuumTime(uint32_t days) {
        return PrivilegedOperation{PrivilegedOpType::JournalVacuumTime, {}, "", 0, days};
    }

    static PrivilegedOperation makeServiceAction(PrivilegedOpType action, const std::string& unit) {
        return PrivilegedOperation{action, {}, unit, 0, 0};
    }

    static PrivilegedOperation makeKernelPurge(const std::string& kernelPkg) {
        return PrivilegedOperation{PrivilegedOpType::KernelPurge, {kernelPkg}, "", 0, 0};
    }
};

} // namespace LinuxManager::Security

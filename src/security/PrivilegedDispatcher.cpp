#include "PrivilegedDispatcher.h"
#include "../core/CommandRunner.h"
#include <unistd.h>

namespace LinuxManager::Security {

PrivilegedDispatcher::PrivilegedDispatcher(
    std::shared_ptr<Core::ISystemBackend> backend,
    std::shared_ptr<OperationValidator> validator
) : m_backend(std::move(backend)), m_validator(std::move(validator)) {
    if (!m_validator) {
        auto policy = std::make_shared<PolicyManager>();
        m_validator = std::make_shared<OperationValidator>(policy);
    }
}

Core::CommandResult PrivilegedDispatcher::executeWithPolkit(
    const std::string& trustedBin,
    const std::vector<std::string>& arguments,
    std::chrono::milliseconds timeout
) {
    if (m_backend && m_backend->isMock()) {
        return m_backend->executeCommand(trustedBin, arguments, Core::PrivilegeLevel::RootRequired, timeout);
    }

    // If already root (e.g. root container)
    if (geteuid() == 0) {
        return Core::CommandRunner::run(trustedBin, arguments, timeout);
    }

    // Execute via trusted pkexec absolute binary
    std::vector<std::string> pkexecArgs = {trustedBin};
    pkexecArgs.insert(pkexecArgs.end(), arguments.begin(), arguments.end());

    return Core::CommandRunner::run(PKEXEC_BIN, pkexecArgs, timeout);
}

Core::CommandResult PrivilegedDispatcher::dispatch(const PrivilegedOperation& op) {
    // 1. Mandatory validation through security allow-list
    auto val = m_validator->validatePrivilegedOperation(op);
    if (!val) {
        Core::CommandResult err;
        err.exitCode = -1;
        err.stdErr = "Security Policy Rejection: " + val.reason;
        return err;
    }

    if (m_backend && m_backend->isMock()) {
        return m_backend->executePrivilegedOperation(op);
    }

    // 2. Strict mapping to known trusted binaries and argument structures
    switch (op.type) {
        case PrivilegedOpType::AptUpdate: {
            return executeWithPolkit(APT_GET_BIN, {"update"}, std::chrono::milliseconds(120000));
        }

        case PrivilegedOpType::AptInstall: {
            std::vector<std::string> args = {"install", "-y"};
            args.insert(args.end(), op.packages.begin(), op.packages.end());
            return executeWithPolkit(APT_GET_BIN, args, std::chrono::milliseconds(600000));
        }

        case PrivilegedOpType::AptRemove: {
            std::vector<std::string> args = {"remove", "-y"};
            args.insert(args.end(), op.packages.begin(), op.packages.end());
            return executeWithPolkit(APT_GET_BIN, args, std::chrono::milliseconds(180000));
        }

        case PrivilegedOpType::AptPurge:
        case PrivilegedOpType::KernelPurge: {
            std::vector<std::string> args = {"purge", "-y"};
            args.insert(args.end(), op.packages.begin(), op.packages.end());
            return executeWithPolkit(APT_GET_BIN, args, std::chrono::milliseconds(180000));
        }

        case PrivilegedOpType::AptUpgrade: {
            std::vector<std::string> args = {
                "dist-upgrade", "-y",
                "-o", "Dpkg::Options::=--force-confdef",
                "-o", "Dpkg::Options::=--force-confold"
            };
            return executeWithPolkit(APT_GET_BIN, args, std::chrono::milliseconds(600000));
        }

        case PrivilegedOpType::AptAutoremove: {
            return executeWithPolkit(APT_GET_BIN, {"autoremove", "-y"}, std::chrono::milliseconds(180000));
        }

        case PrivilegedOpType::AptClean: {
            return executeWithPolkit(APT_GET_BIN, {"clean"}, std::chrono::milliseconds(60000));
        }

        case PrivilegedOpType::JournalVacuumSize: {
            std::string arg = "--vacuum-size=" + std::to_string(op.targetSizeMb) + "M";
            return executeWithPolkit(JOURNALCTL_BIN, {arg}, std::chrono::milliseconds(60000));
        }

        case PrivilegedOpType::JournalVacuumTime: {
            std::string arg = "--vacuum-time=" + std::to_string(op.retentionDays) + "d";
            return executeWithPolkit(JOURNALCTL_BIN, {arg}, std::chrono::milliseconds(60000));
        }

        case PrivilegedOpType::ServiceRestart: {
            return executeWithPolkit(SYSTEMCTL_BIN, {"restart", op.serviceUnit}, std::chrono::milliseconds(60000));
        }

        case PrivilegedOpType::ServiceStart: {
            return executeWithPolkit(SYSTEMCTL_BIN, {"start", op.serviceUnit}, std::chrono::milliseconds(60000));
        }

        case PrivilegedOpType::ServiceStop: {
            return executeWithPolkit(SYSTEMCTL_BIN, {"stop", op.serviceUnit}, std::chrono::milliseconds(60000));
        }

        case PrivilegedOpType::ServiceEnable: {
            return executeWithPolkit(SYSTEMCTL_BIN, {"enable", op.serviceUnit}, std::chrono::milliseconds(60000));
        }

        case PrivilegedOpType::ServiceDisable: {
            return executeWithPolkit(SYSTEMCTL_BIN, {"disable", op.serviceUnit}, std::chrono::milliseconds(60000));
        }
    }

    Core::CommandResult fallbackErr;
    fallbackErr.exitCode = -1;
    fallbackErr.stdErr = "Unsupported privileged operation.";
    return fallbackErr;
}

} // namespace LinuxManager::Security

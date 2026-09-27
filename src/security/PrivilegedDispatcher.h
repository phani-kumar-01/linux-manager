#pragma once

#include "PrivilegedOperation.h"
#include "OperationValidator.h"
#include "../core/ISystemBackend.h"
#include <memory>

namespace LinuxManager::Security {

class PrivilegedDispatcher {
public:
    explicit PrivilegedDispatcher(
        std::shared_ptr<Core::ISystemBackend> backend,
        std::shared_ptr<OperationValidator> validator
    );

    [[nodiscard]] Core::CommandResult dispatch(const PrivilegedOperation& operation);

private:
    std::shared_ptr<Core::ISystemBackend> m_backend;
    std::shared_ptr<OperationValidator> m_validator;

    // Trusted absolute binary paths
    static constexpr const char* APT_GET_BIN = "/usr/bin/apt-get";
    static constexpr const char* SYSTEMCTL_BIN = "/usr/bin/systemctl";
    static constexpr const char* JOURNALCTL_BIN = "/usr/bin/journalctl";
    static constexpr const char* PKEXEC_BIN = "/usr/bin/pkexec";

    [[nodiscard]] Core::CommandResult executeWithPolkit(
        const std::string& trustedBin,
        const std::vector<std::string>& arguments,
        std::chrono::milliseconds timeout = std::chrono::milliseconds(300000)
    );
};

} // namespace LinuxManager::Security

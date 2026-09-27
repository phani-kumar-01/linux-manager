#pragma once

#include "ISystemBackend.h"
#include "../security/PrivilegedOperation.h"
#include <memory>
#include <string>

namespace LinuxManager::Core {

class PermissionManager {
public:
    explicit PermissionManager(std::shared_ptr<ISystemBackend> backend);

    [[nodiscard]] bool isRunningAsRoot() const;
    [[nodiscard]] bool canElevateWithPolkit() const;

    [[nodiscard]] CommandResult runPrivilegedOperation(
        const Security::PrivilegedOperation& operation
    );

private:
    std::shared_ptr<ISystemBackend> m_backend;
};

} // namespace LinuxManager::Core

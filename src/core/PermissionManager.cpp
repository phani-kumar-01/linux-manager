#include "PermissionManager.h"
#include <unistd.h>
#include <filesystem>

namespace LinuxManager::Core {

PermissionManager::PermissionManager(std::shared_ptr<ISystemBackend> backend)
    : m_backend(std::move(backend)) {}

bool PermissionManager::isRunningAsRoot() const {
    if (!m_backend) return false;
    return m_backend->isElevated();
}

bool PermissionManager::canElevateWithPolkit() const {
    if (isRunningAsRoot()) return true;
    if (m_backend && m_backend->isMock()) return true;
    
    std::error_code ec;
    return std::filesystem::exists("/usr/bin/pkexec", ec);
}

CommandResult PermissionManager::runPrivilegedOperation(
    const Security::PrivilegedOperation& operation
) {
    if (!m_backend) {
        CommandResult res;
        res.exitCode = -1;
        res.stdErr = "No system backend configured.";
        return res;
    }

    return m_backend->executePrivilegedOperation(operation);
}

} // namespace LinuxManager::Core

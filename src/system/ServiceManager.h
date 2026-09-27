#pragma once

#include "../core/ISystemBackend.h"
#include <memory>
#include <vector>
#include <string>

namespace LinuxManager::System {

enum class ServiceState {
    Active,
    Inactive,
    Failed,
    Activating,
    Deactivating
};

struct ServiceUnit {
    std::string unitName;
    std::string loadState;
    std::string activeState;
    std::string subState;
    std::string description;
    bool isFailed = false;
    bool isProtected = false;
};

class ServiceManager {
public:
    explicit ServiceManager(std::shared_ptr<Core::ISystemBackend> backend);

    [[nodiscard]] std::vector<ServiceUnit> getAllServices();
    [[nodiscard]] std::vector<ServiceUnit> getFailedServices();
    [[nodiscard]] std::string getServiceLogs(const std::string& serviceName, int lines = 50);

    [[nodiscard]] Core::CommandResult restartService(const std::string& serviceName);
    [[nodiscard]] Core::CommandResult startService(const std::string& serviceName);
    [[nodiscard]] Core::CommandResult stopService(const std::string& serviceName);
    [[nodiscard]] Core::CommandResult enableService(const std::string& serviceName);
    [[nodiscard]] Core::CommandResult disableService(const std::string& serviceName);

    [[nodiscard]] bool isServiceProtected(const std::string& serviceName) const;

private:
    std::shared_ptr<Core::ISystemBackend> m_backend;
};

} // namespace LinuxManager::System

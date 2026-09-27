#pragma once

#include "ISystemBackend.h"
#include "PermissionManager.h"
#include "HistoryManager.h"
#include "../security/PolicyManager.h"
#include "../security/OperationValidator.h"
#include "../package/AptManager.h"
#include "../package/UpdateManager.h"
#include "../package/FlatpakManager.h"
#include "../package/RepositoryManager.h"
#include "../cleanup/CleanupEngine.h"
#include "../developer/SdkDoctor.h"
#include "../process/ProcessManager.h"
#include "../startup/StartupManager.h"
#include "../storage/StorageScanner.h"
#include "../storage/LargeFileScanner.h"
#include "../system/SystemInfo.h"
#include "../system/ServiceManager.h"
#include "../system/KernelManager.h"
#include "../system/LogManager.h"
#include "../system/PowerManager.h"

#include <memory>
#include <string>

namespace LinuxManager::Core {

struct DashboardMetrics {
    double cpuUsagePercent = 0.0;
    double memoryUsagePercent = 0.0;
    uint64_t memoryUsedBytes = 0;
    uint64_t memoryTotalBytes = 0;
    double diskUsagePercent = 0.0;
    uint64_t diskUsedBytes = 0;
    uint64_t diskTotalBytes = 0;
    double cpuTemperature = 0.0;
    uint32_t availableUpdatesCount = 0;
    uint32_t securityUpdatesCount = 0;
    uint64_t recoverableCleanupBytes = 0;
    uint32_t failedServicesCount = 0;
    std::string osPrettyName;
    std::string kernelVersion;
    std::string uptimeFormatted;
    std::string userName;
};

class Application {
public:
    explicit Application(std::shared_ptr<ISystemBackend> backend = nullptr);
    ~Application() = default;

    void initialize();

    [[nodiscard]] DashboardMetrics getDashboardMetrics();

    [[nodiscard]] std::shared_ptr<ISystemBackend> getBackend() const noexcept { return m_backend; }
    [[nodiscard]] std::shared_ptr<Security::PolicyManager> getPolicyManager() const noexcept { return m_policy; }
    [[nodiscard]] std::shared_ptr<Security::OperationValidator> getValidator() const noexcept { return m_validator; }
    [[nodiscard]] std::shared_ptr<PermissionManager> getPermissionManager() const noexcept { return m_permissions; }
    [[nodiscard]] std::shared_ptr<HistoryManager> getHistoryManager() const noexcept { return m_history; }

    [[nodiscard]] std::shared_ptr<Package::AptManager> getAptManager() const noexcept { return m_aptManager; }
    [[nodiscard]] std::shared_ptr<Package::UpdateManager> getUpdateManager() const noexcept { return m_updateManager; }
    [[nodiscard]] std::shared_ptr<Package::FlatpakManager> getFlatpakManager() const noexcept { return m_flatpakManager; }
    [[nodiscard]] std::shared_ptr<Package::RepositoryManager> getRepositoryManager() const noexcept { return m_repoManager; }

    [[nodiscard]] std::shared_ptr<Developer::SdkDoctor> getSdkDoctor() const noexcept { return m_sdkDoctor; }
    [[nodiscard]] std::shared_ptr<Process::ProcessManager> getProcessManager() const noexcept { return m_processManager; }
    [[nodiscard]] std::shared_ptr<Startup::StartupManager> getStartupManager() const noexcept { return m_startupManager; }

    [[nodiscard]] std::shared_ptr<Cleanup::CleanupEngine> getCleanupEngine() const noexcept { return m_cleanupEngine; }
    [[nodiscard]] std::shared_ptr<Storage::StorageScanner> getStorageScanner() const noexcept { return m_storageScanner; }
    [[nodiscard]] std::shared_ptr<Storage::LargeFileScanner> getLargeFileScanner() const noexcept { return m_largeFileScanner; }

    [[nodiscard]] std::shared_ptr<System::SystemInfo> getSystemInfo() const noexcept { return m_systemInfo; }
    [[nodiscard]] std::shared_ptr<System::ServiceManager> getServiceManager() const noexcept { return m_serviceManager; }
    [[nodiscard]] std::shared_ptr<System::KernelManager> getKernelManager() const noexcept { return m_kernelManager; }
    [[nodiscard]] std::shared_ptr<System::LogManager> getLogManager() const noexcept { return m_logManager; }
    [[nodiscard]] std::shared_ptr<System::PowerManager> getPowerManager() const noexcept { return m_powerManager; }

private:
    std::shared_ptr<ISystemBackend> m_backend;
    std::shared_ptr<Security::PolicyManager> m_policy;
    std::shared_ptr<Security::OperationValidator> m_validator;
    std::shared_ptr<PermissionManager> m_permissions;
    std::shared_ptr<HistoryManager> m_history;

    std::shared_ptr<Package::AptManager> m_aptManager;
    std::shared_ptr<Package::UpdateManager> m_updateManager;
    std::shared_ptr<Package::FlatpakManager> m_flatpakManager;
    std::shared_ptr<Package::RepositoryManager> m_repoManager;

    std::shared_ptr<Developer::SdkDoctor> m_sdkDoctor;
    std::shared_ptr<Process::ProcessManager> m_processManager;
    std::shared_ptr<Startup::StartupManager> m_startupManager;

    std::shared_ptr<Cleanup::CleanupEngine> m_cleanupEngine;
    std::shared_ptr<Storage::StorageScanner> m_storageScanner;
    std::shared_ptr<Storage::LargeFileScanner> m_largeFileScanner;

    std::shared_ptr<System::SystemInfo> m_systemInfo;
    std::shared_ptr<System::ServiceManager> m_serviceManager;
    std::shared_ptr<System::KernelManager> m_kernelManager;
    std::shared_ptr<System::LogManager> m_logManager;
    std::shared_ptr<System::PowerManager> m_powerManager;
};

} // namespace LinuxManager::Core

#include "Application.h"
#include "RealSystemBackend.h"

namespace LinuxManager::Core {

Application::Application(std::shared_ptr<ISystemBackend> backend)
    : m_backend(std::move(backend)) {
    if (!m_backend) {
        m_backend = std::make_shared<RealSystemBackend>();
    }
    initialize();
}

void Application::initialize() {
    m_policy = std::make_shared<Security::PolicyManager>();
    m_validator = std::make_shared<Security::OperationValidator>(m_policy);
    m_permissions = std::make_shared<PermissionManager>(m_backend);
    m_history = std::make_shared<HistoryManager>(m_backend);

    m_aptManager = std::make_shared<Package::AptManager>(m_backend, m_validator);
    m_updateManager = std::make_shared<Package::UpdateManager>(m_backend);
    m_flatpakManager = std::make_shared<Package::FlatpakManager>(m_backend);
    m_repoManager = std::make_shared<Package::RepositoryManager>(m_backend);

    m_sdkDoctor = std::make_shared<Developer::SdkDoctor>(m_backend);
    m_processManager = std::make_shared<Process::ProcessManager>(m_backend);
    m_startupManager = std::make_shared<Startup::StartupManager>(m_backend);

    m_cleanupEngine = std::make_shared<Cleanup::CleanupEngine>(m_backend, m_validator, m_history);
    m_storageScanner = std::make_shared<Storage::StorageScanner>(m_backend);
    m_largeFileScanner = std::make_shared<Storage::LargeFileScanner>(m_backend);

    m_systemInfo = std::make_shared<System::SystemInfo>(m_backend);
    m_serviceManager = std::make_shared<System::ServiceManager>(m_backend);
    m_kernelManager = std::make_shared<System::KernelManager>(m_backend);
    m_logManager = std::make_shared<System::LogManager>(m_backend);
    m_powerManager = std::make_shared<System::PowerManager>(m_backend);
}

DashboardMetrics Application::getDashboardMetrics() {
    DashboardMetrics metrics;

    auto overview = m_systemInfo->getSystemOverview();
    metrics.osPrettyName = overview.os.prettyName;
    metrics.kernelVersion = overview.kernelVersion;
    metrics.uptimeFormatted = overview.uptimeFormatted;
    metrics.userName = m_backend->getCurrentUserName();

    metrics.cpuUsagePercent = overview.cpu.currentUsagePercentage;
    metrics.cpuTemperature = overview.cpu.currentTemperatureCelsius;

    metrics.memoryTotalBytes = overview.memory.totalBytes;
    metrics.memoryUsedBytes = overview.memory.usedBytes;
    metrics.memoryUsagePercent = overview.memory.usagePercentage;

    auto partitions = m_storageScanner->getPartitions();
    if (!partitions.empty()) {
        metrics.diskTotalBytes = partitions[0].totalBytes;
        metrics.diskUsedBytes = partitions[0].usedBytes;
        metrics.diskUsagePercent = partitions[0].usePercentage;
    }

    auto failedServices = m_serviceManager->getFailedServices();
    metrics.failedServicesCount = static_cast<uint32_t>(failedServices.size());

    // Recoverable cleanup estimate
    auto cleanupPreview = m_cleanupEngine->scanAll();
    metrics.recoverableCleanupBytes = cleanupPreview.totalReclaimableBytes;

    return metrics;
}

} // namespace LinuxManager::Core

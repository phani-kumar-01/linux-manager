#include "core/Application.h"
#include "core/MockSystemBackend.h"
#include "core/RealSystemBackend.h"

#include <iostream>
#include <iomanip>
#include <memory>
#include <string>
#include <vector>

#ifdef HAVE_QT_GUI
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QFile>
#include <QUrl>
#endif

void printBanner() {
    std::cout << "\033[1;36m"
              << "╔════════════════════════════════════════════════════════════════╗\n"
              << "║                         LINUX MANAGER                          ║\n"
              << "║      Native System, Software, Cleanup & Developer Manager      ║\n"
              << "║                    Target: Ubuntu & Debian                     ║\n"
              << "╚════════════════════════════════════════════════════════════════╝\n"
              << "\033[0m" << std::endl;
}

void printHelp() {
    std::cout << "Usage: linux-manager [OPTIONS]\n\n"
              << "Options:\n"
              << "  --cli              Run in terminal interactive console mode (default when GUI not active)\n"
              << "  --dashboard        Show immediate system status and metrics\n"
              << "  --scan             Scan all system and developer caches\n"
              << "  --clean            Execute system cleanup\n"
              << "  --dry-run          Perform a simulated dry-run cleanup\n"
              << "  --updates          Check available APT and Flatpak updates\n"
              << "  --services         List systemd services and failed units\n"
              << "  --storage          Analyze disk partitions and large files\n"
              << "  --developer        Scan developer caches (Node, Python, Java, Rust)\n"
              << "  --sdk              Run Developer SDK Doctor & Toolchain diagnostics\n"
              << "  --ports            List active listening ports & process finder\n"
              << "  --autostart        List startup applications\n"
              << "  --repos            List APT repositories and PPAs\n"
              << "  --power            Inspect battery health, degradation, and power profiles\n"
              << "  --history          Display past cleanup and maintenance operations\n"
              << "  --mock             Use MockSystemBackend (safe simulated sandbox)\n"
              << "  --version          Display application version\n"
              << "  --help             Show this help message\n"
              << std::endl;
}

std::string formatBytes(uint64_t bytes) {
    if (bytes >= 1024ULL * 1024ULL * 1024ULL) {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2) << (static_cast<double>(bytes) / (1024ULL * 1024ULL * 1024ULL)) << " GB";
        return ss.str();
    } else if (bytes >= 1024ULL * 1024ULL) {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(1) << (static_cast<double>(bytes) / (1024ULL * 1024ULL)) << " MB";
        return ss.str();
    } else if (bytes >= 1024ULL) {
        std::ostringstream ss;
        ss << (bytes / 1024ULL) << " KB";
        return ss.str();
    }
    return std::to_string(bytes) + " B";
}

void showDashboard(LinuxManager::Core::Application& app) {
    auto m = app.getDashboardMetrics();
    std::cout << "\n\033[1;32mGood day, " << m.userName << "\033[0m\n\n"
              << "──────────────── System Status ────────────────\n"
              << "OS:           " << m.osPrettyName << " (" << m.kernelVersion << ")\n"
              << "Uptime:       " << m.uptimeFormatted << "\n"
              << "CPU Usage:    " << std::fixed << std::setprecision(1) << m.cpuUsagePercent << "% (" << m.cpuTemperature << "°C)\n"
              << "Memory:       " << formatBytes(m.memoryUsedBytes) << " / " << formatBytes(m.memoryTotalBytes)
              << " (" << std::fixed << std::setprecision(1) << m.memoryUsagePercent << "%)\n"
              << "Disk:         " << formatBytes(m.diskUsedBytes) << " / " << formatBytes(m.diskTotalBytes)
              << " (" << std::fixed << std::setprecision(1) << m.diskUsagePercent << "%)\n"
              << "───────────────────────────────────────────────\n"
              << "Pending Updates:    " << m.availableUpdatesCount << " available (" << m.securityUpdatesCount << " security)\n"
              << "Recoverable Cache:  \033[1;33m" << formatBytes(m.recoverableCleanupBytes) << "\033[0m\n"
              << "Failed Services:    " << (m.failedServicesCount > 0 ? "\033[1;31m" : "\033[1;32m")
              << m.failedServicesCount << "\033[0m\n"
              << "───────────────────────────────────────────────\n" << std::endl;
}

void showScan(LinuxManager::Core::Application& app) {
    std::cout << "\n\033[1;34mScanning system and developer caches...\033[0m\n";
    auto preview = app.getCleanupEngine()->scanAll();

    std::cout << "\nFound " << preview.items.size() << " cleanable targets:\n\n";
    std::cout << std::left << std::setw(32) << "Target" << std::setw(15) << "Size" << std::setw(12) << "Risk" << "Path\n";
    std::cout << std::string(75, '-') << "\n";

    for (const auto& item : preview.items) {
        std::string riskStr = (item.risk == LinuxManager::Cleanup::RiskLevel::Safe) ? "\033[32mSafe\033[0m" :
                              (item.risk == LinuxManager::Cleanup::RiskLevel::Moderate) ? "\033[33mModerate\033[0m" : "\033[31mCaution\033[0m";
        std::cout << std::left << std::setw(32) << item.title
                  << std::setw(15) << formatBytes(item.sizeBytes)
                  << std::setw(20) << riskStr
                  << item.path << "\n";
    }

    std::cout << std::string(75, '-') << "\n";
    std::cout << "\033[1;32mTotal Recoverable Space: " << formatBytes(preview.totalReclaimableBytes) << "\033[0m\n\n";
}

void showUpdates(LinuxManager::Core::Application& app) {
    std::cout << "\n\033[1;34mChecking for available updates...\033[0m\n";
    auto updates = app.getUpdateManager()->checkAvailableUpdates();
    std::cout << "Available Updates: " << updates.totalUpdates
              << " (" << updates.securityUpdates << " security patches)\n";
    for (const auto& u : updates.updates) {
        std::cout << "- " << u.name << " " << u.currentVersion << " -> " << u.candidateVersion;
        if (u.isSecurityUpdate) std::cout << " \033[1;31m[SECURITY]\033[0m";
        std::cout << "\n";
    }
    std::cout << "\n";
}

void showServices(LinuxManager::Core::Application& app) {
    std::cout << "\n\033[1;34mInspecting systemd services...\033[0m\n";
    auto services = app.getServiceManager()->getAllServices();
    auto failed = app.getServiceManager()->getFailedServices();
    std::cout << "Total Services: " << services.size() << " | Failed: " << failed.size() << "\n\n";
    if (!failed.empty()) {
        std::cout << "\033[1;31mFailed Services:\033[0m\n";
        for (const auto& f : failed) {
            std::cout << "  ● " << f.unitName << " - " << f.description << "\n";
        }
    } else {
        std::cout << "\033[1;32m✓ All active services healthy\033[0m\n";
    }
    std::cout << "\n";
}

void showStorage(LinuxManager::Core::Application& app) {
    std::cout << "\n\033[1;34mDisk Partitions:\033[0m\n";
    auto partitions = app.getStorageScanner()->getPartitions();
    for (const auto& p : partitions) {
        std::cout << "Mount: " << std::left << std::setw(15) << p.mountPoint
                  << " | FS: " << std::setw(18) << p.filesystem
                  << " | Used: " << formatBytes(p.usedBytes) << " / " << formatBytes(p.totalBytes)
                  << " (" << std::fixed << std::setprecision(1) << p.usePercentage << "%)\n";
    }
    std::cout << "\n";
}

void showDeveloper(LinuxManager::Core::Application& app) {
    std::cout << "\n\033[1;34mDeveloper Caches:\033[0m\n";
    auto devCaches = app.getCleanupEngine()->getDeveloperCleaner()->scanAllDeveloperCaches();
    for (const auto& d : devCaches) {
        std::cout << "- " << std::left << std::setw(32) << d.title << " : " << formatBytes(d.sizeBytes) << "\n";
    }
    std::cout << "\n";
}

void showSdkDoctor(LinuxManager::Core::Application& app) {
    std::cout << "\n\033[1;34m🛠️ Developer SDK Doctor & Toolchain Diagnostics:\033[0m\n";
    auto report = app.getSdkDoctor()->runDiagnostics();
    for (const auto& sdk : report.sdks) {
        if (sdk.isInstalled) {
            std::cout << "  \033[1;32m✓\033[0m " << std::left << std::setw(16) << sdk.name
                      << " : " << std::setw(28) << sdk.version
                      << " (" << sdk.installPath << ")\n";
        } else {
            std::cout << "  \033[1;30m✗\033[0m " << std::left << std::setw(16) << sdk.name
                      << " : \033[1;30mNot Installed\033[0m\n";
        }
    }

    if (!report.issues.empty()) {
        std::cout << "\n\033[1;33mDiagnostics & Suggestions:\033[0m\n";
        for (const auto& issue : report.issues) {
            std::cout << "  - [" << issue.severity << "] " << issue.title << "\n"
                      << "    " << issue.description << "\n"
                      << "    Fix: \033[1;36m" << issue.fixCommand << "\033[0m\n";
        }
    }
    std::cout << "\n";
}

void showProcessesAndPorts(LinuxManager::Core::Application& app) {
    std::cout << "\n\033[1;34m⚡ Active Listening Ports & Process Finder:\033[0m\n";
    auto ports = app.getProcessManager()->getListeningPorts();
    if (ports.empty()) {
        std::cout << "No active listening ports detected or permission denied.\n";
    } else {
        for (const auto& p : ports) {
            std::cout << "  \033[1;32m:" << std::left << std::setw(6) << p.port << "\033[0m "
                      << std::setw(6) << p.protocol
                      << " (PID: " << std::setw(6) << p.pid << ") "
                      << std::setw(16) << p.processName
                      << " [" << p.address << "] user: " << p.user << "\n";
        }
    }
    std::cout << "\n";
}

void showStartup(LinuxManager::Core::Application& app) {
    std::cout << "\n\033[1;34m🚀 Startup & Autostart Applications:\033[0m\n";
    auto apps = app.getStartupManager()->getAutostartApplications();
    if (apps.empty()) {
        std::cout << "No autostart applications found.\n";
    } else {
        for (const auto& a : apps) {
            std::cout << "  " << (a.isEnabled ? "\033[1;32m[Enabled] \033[0m" : "\033[1;31m[Disabled]\033[0m ")
                      << std::left << std::setw(24) << a.name
                      << " : " << a.exec << " (" << (a.isSystemWide ? "System" : "User") << ")\n";
        }
    }
    std::cout << "\n";
}

void showRepositories(LinuxManager::Core::Application& app) {
    std::cout << "\n\033[1;34m📦 APT Repositories & PPAs:\033[0m\n";
    auto summary = app.getRepositoryManager()->getRepositorySummary();
    std::cout << "Total: " << summary.totalCount << " (" << summary.enabledCount << " enabled, "
              << summary.thirdPartyCount << " 3rd-party/PPAs)\n";
    for (const auto& r : summary.repositories) {
        std::cout << "  " << (r.isEnabled ? "\033[1;32m✓\033[0m " : "\033[1;30m✗\033[0m ")
                  << (r.isThirdParty ? "\033[1;33m[PPA/3rd]\033[0m " : "[Official] ")
                  << std::left << std::setw(10) << r.suite << " : " << r.uri << "\n";
    }
    std::cout << "\n";
}

void showPower(LinuxManager::Core::Application& app) {
    std::cout << "\n\033[1;34m🔋 Battery Health & Power Profile:\033[0m\n";
    auto bat = app.getPowerManager()->getBatteryHealth();
    if (bat.hasBattery) {
        std::cout << "  Device:         " << bat.deviceName << " (" << bat.modelName << ")\n"
                  << "  Status:         " << bat.status << " (" << std::fixed << std::setprecision(1) << bat.currentPercent << "%)\n"
                  << "  Health / Wear:  " << std::fixed << std::setprecision(1) << bat.healthPercent << "% ("
                  << bat.fullCapacityWh << " Wh / " << bat.designCapacityWh << " Wh design)\n"
                  << "  Charge Cycles:  " << bat.cycleCount << "\n"
                  << "  Discharge Rate: " << std::fixed << std::setprecision(1) << bat.dischargeRateWatts << " W\n";
    } else {
        std::cout << "  Battery: None detected (Desktop/AC powered system)\n";
    }

    auto prof = app.getPowerManager()->getPowerProfiles();
    if (prof.isAvailable) {
        std::cout << "  Power Profile:  \033[1;36m" << prof.activeProfile << "\033[0m (Backend: " << prof.daemonBackend << ")\n";
    }
    std::cout << "\n";
}

void showHistory(LinuxManager::Core::Application& app) {
    std::cout << "\n\033[1;34mOperation History:\033[0m\n";
    auto records = app.getHistoryManager()->getHistory();
    if (records.empty()) {
        std::cout << "No past operations recorded yet.\n";
    } else {
        for (const auto& rec : records) {
            std::cout << "[" << rec.timestamp << "] " << rec.operationId << " - "
                      << rec.category << " - " << rec.description
                      << " (Reclaimed: " << formatBytes(rec.bytesReclaimed) << ")\n";
        }
    }
    std::cout << "\n";
}

void runInteractiveConsole(LinuxManager::Core::Application& app) {
    printBanner();
    std::cout << "\033[1;33mInteractive Management Console (Session Active)\033[0m\n"
              << "Tip: To launch the Qt 6 graphical desktop window, run ./build/bin/linux-manager.\n\n";

    while (true) {
        std::cout << "\033[1;36m───────────────── Linux Manager Control Menu ─────────────────\033[0m\n"
                  << "  [1]  System Dashboard Overview\n"
                  << "  [2]  Scan All Caches & Recoverable Space\n"
                  << "  [3]  Dry-Run Cleanup Preview\n"
                  << "  [4]  Execute System Cleanup\n"
                  << "  [5]  Check Updates (APT & Flatpak)\n"
                  << "  [6]  Developer Environment Caches (Node, Python, Rust, Android)\n"
                  << "  [7]  Developer SDK Doctor & Toolchains (Go, Rust, Node, GCC)\n"
                  << "  [8]  Process Explorer & Port Killer (Resolve EADDRINUSE)\n"
                  << "  [9]  Startup & Autostart Applications\n"
                  << "  [10] APT Repositories & PPAs\n"
                  << "  [11] Battery Health & Power Profiles\n"
                  << "  [12] Systemd Services & Logs\n"
                  << "  [13] Storage Analyzer & Partitions\n"
                  << "  [14] View Audit History\n"
                  << "  [q]  Exit Linux Manager\n"
                  << "\033[1;36m─────────────────────────────────────────────────────────────\033[0m\n"
                  << "Enter selection [1-14, q]: " << std::flush;

        std::string choice;
        if (!std::getline(std::cin, choice)) {
            break; // EOF
        }

        if (choice == "q" || choice == "Q" || choice == "exit") {
            std::cout << "Exiting Linux Manager. Goodbye!\n";
            break;
        } else if (choice == "1") {
            showDashboard(app);
        } else if (choice == "2") {
            showScan(app);
        } else if (choice == "3") {
            std::cout << "\n\033[1;33mExecuting Cleanup Dry-Run...\033[0m\n";
            auto preview = app.getCleanupEngine()->scanAll();
            auto result = app.getCleanupEngine()->executeCleanup(preview.items, true);
            std::cout << "Dry-run preview completed. Estimated reclaim: " << formatBytes(result.bytesCleaned)
                      << " across " << result.itemsCleanedCount << " items.\n\n";
        } else if (choice == "4") {
            std::cout << "\n\033[1;33mScanning items for cleanup...\033[0m\n";
            auto preview = app.getCleanupEngine()->scanAll();
            if (preview.items.empty()) {
                std::cout << "No cleanable items found.\n\n";
            } else {
                std::cout << "Ready to clean " << preview.items.size() << " items ("
                          << formatBytes(preview.totalReclaimableBytes) << "). Proceed? [y/N]: " << std::flush;
                std::string confirm;
                std::getline(std::cin, confirm);
                if (confirm == "y" || confirm == "Y") {
                    auto result = app.getCleanupEngine()->executeCleanup(preview.items, false);
                    std::cout << "\033[1;32mCleanup completed! Reclaimed: "
                              << formatBytes(result.bytesCleaned) << "\033[0m\n\n";
                } else {
                    std::cout << "Cleanup cancelled.\n\n";
                }
            }
        } else if (choice == "5") {
            showUpdates(app);
        } else if (choice == "6") {
            showDeveloper(app);
        } else if (choice == "7") {
            showSdkDoctor(app);
        } else if (choice == "8") {
            showProcessesAndPorts(app);
        } else if (choice == "9") {
            showStartup(app);
        } else if (choice == "10") {
            showRepositories(app);
        } else if (choice == "11") {
            showPower(app);
        } else if (choice == "12") {
            showServices(app);
        } else if (choice == "13") {
            showStorage(app);
        } else if (choice == "14") {
            showHistory(app);
        } else {
            std::cout << "\033[1;31mInvalid option. Please enter 1-14 or q.\033[0m\n\n";
        }
    }
}

int main(int argc, char* argv[]) {
    std::vector<std::string> args(argv + 1, argv + argc);
    bool useMock = false;
    bool runCli = false;
    bool runDashboard = false;
    bool runScan = false;
    bool runClean = false;
    bool runDryRun = false;
    bool runUpdates = false;
    bool runServices = false;
    bool runStorage = false;
    bool runDeveloper = false;
    bool runSdk = false;
    bool runPorts = false;
    bool runAutostart = false;
    bool runRepos = false;
    bool runPower = false;
    bool runHistory = false;

    for (const auto& arg : args) {
        if (arg == "--mock") useMock = true;
        else if (arg == "--cli") runCli = true;
        else if (arg == "--dashboard") runDashboard = true;
        else if (arg == "--scan") runScan = true;
        else if (arg == "--clean") runClean = true;
        else if (arg == "--dry-run") runDryRun = true;
        else if (arg == "--updates") runUpdates = true;
        else if (arg == "--services") runServices = true;
        else if (arg == "--storage") runStorage = true;
        else if (arg == "--developer") runDeveloper = true;
        else if (arg == "--sdk") runSdk = true;
        else if (arg == "--ports") runPorts = true;
        else if (arg == "--autostart") runAutostart = true;
        else if (arg == "--repos") runRepos = true;
        else if (arg == "--power") runPower = true;
        else if (arg == "--history") runHistory = true;
        else if (arg == "--version" || arg == "-v") {
            std::cout << "Linux Manager v1.0.0 (C++20 Native)\n";
            return 0;
        } else if (arg == "--help" || arg == "-h") {
            printBanner();
            printHelp();
            return 0;
        }
    }

    std::shared_ptr<LinuxManager::Core::ISystemBackend> backend;
    if (useMock) {
        backend = std::make_shared<LinuxManager::Core::MockSystemBackend>();
    } else {
        backend = std::make_shared<LinuxManager::Core::RealSystemBackend>();
    }

    LinuxManager::Core::Application app(backend);

    bool hasSpecificFlag = runDashboard || runScan || runClean || runDryRun ||
                           runUpdates || runServices || runStorage || runDeveloper ||
                           runSdk || runPorts || runAutostart || runRepos || runPower || runHistory;

#ifdef HAVE_QT_GUI
    if (!hasSpecificFlag && !runCli) {
        QGuiApplication qapp(argc, argv);
        qapp.setApplicationName("Linux Manager");
        qapp.setApplicationVersion("1.0.0");
        qapp.setOrganizationName("LinuxManager");

        QQmlApplicationEngine engine;

        QUrl url;
        if (QFile::exists(":/qml/Main.qml")) {
            url = QUrl(QStringLiteral("qrc:/qml/Main.qml"));
        } else if (QFile::exists(":/LinuxManager/qml/Main.qml")) {
            url = QUrl(QStringLiteral("qrc:/LinuxManager/qml/Main.qml"));
        } else if (QFile::exists(":/qt/qml/LinuxManager/qml/Main.qml")) {
            url = QUrl(QStringLiteral("qrc:/qt/qml/LinuxManager/qml/Main.qml"));
        } else if (QFile::exists("/usr/share/linux-manager/qml/Main.qml")) {
            url = QUrl::fromLocalFile("/usr/share/linux-manager/qml/Main.qml");
        } else {
            url = QUrl(QStringLiteral("qrc:/qml/Main.qml"));
        }

        QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
            &qapp, [url](QObject *obj, const QUrl &objUrl) {
                if (!obj && url == objUrl)
                    QCoreApplication::exit(-1);
            }, Qt::QueuedConnection);
        engine.load(url);

        return qapp.exec();
    }
#else
    (void)runCli;
#endif

    if (hasSpecificFlag) {
        printBanner();
        if (runDashboard) showDashboard(app);
        if (runScan) showScan(app);
        if (runDryRun) {
            std::cout << "\033[1;33mExecuting Cleanup Dry-Run...\033[0m\n";
            auto preview = app.getCleanupEngine()->scanAll();
            auto result = app.getCleanupEngine()->executeCleanup(preview.items, true);
            std::cout << "Dry-run completed. Estimated reclaim: " << formatBytes(result.bytesCleaned)
                      << " across " << result.itemsCleanedCount << " items.\n\n";
        }
        if (runClean) {
            auto preview = app.getCleanupEngine()->scanAll();
            auto result = app.getCleanupEngine()->executeCleanup(preview.items, false);
            std::cout << "Cleanup completed. Reclaimed: " << formatBytes(result.bytesCleaned) << "\n\n";
        }
        if (runUpdates) showUpdates(app);
        if (runServices) showServices(app);
        if (runStorage) showStorage(app);
        if (runDeveloper) showDeveloper(app);
        if (runSdk) showSdkDoctor(app);
        if (runPorts) showProcessesAndPorts(app);
        if (runAutostart) showStartup(app);
        if (runRepos) showRepositories(app);
        if (runPower) showPower(app);
        if (runHistory) showHistory(app);
        return 0;
    }

    // Interactive console mode when no specific single-shot flag given
    runInteractiveConsole(app);
    return 0;
}

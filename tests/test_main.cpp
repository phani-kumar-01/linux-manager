#include "../src/core/ISystemBackend.h"
#include "../src/core/CommandRunner.h"
#include "../src/core/MockSystemBackend.h"
#include "../src/core/RealSystemBackend.h"
#include "../src/core/PermissionManager.h"
#include "../src/core/HistoryManager.h"
#include "../src/core/Application.h"
#include "../src/security/PolicyManager.h"
#include "../src/security/OperationValidator.h"
#include "../src/security/PrivilegedOperation.h"
#include "../src/security/PrivilegedDispatcher.h"
#include "../src/package/AptManager.h"
#include "../src/package/UpdateManager.h"
#include "../src/package/FlatpakManager.h"
#include "../src/cleanup/CacheScanner.h"
#include "../src/cleanup/DeveloperCleaner.h"
#include "../src/cleanup/DockerCleaner.h"
#include "../src/cleanup/CleanupEngine.h"
#include "../src/storage/StorageScanner.h"
#include "../src/storage/LargeFileScanner.h"
#include "../src/system/SystemInfo.h"
#include "../src/system/ServiceManager.h"
#include "../src/system/KernelManager.h"
#include "../src/system/LogManager.h"
#include "../src/system/PowerManager.h"
#include "../src/developer/SdkDoctor.h"
#include "../src/process/ProcessManager.h"
#include "../src/startup/StartupManager.h"
#include "../src/package/RepositoryManager.h"

#include <iostream>
#include <cassert>
#include <string>
#include <vector>

static int s_testsPassed = 0;
static int s_testsFailed = 0;

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "\033[1;31m[FAILED]\033[0m " << msg << " (line " << __LINE__ << ")\n"; \
            s_testsFailed++; \
            return; \
        } \
    } while (0)

#define RUN_TEST(fn) \
    do { \
        std::cout << "\033[1;34m[RUNNING]\033[0m " << #fn << "..." << std::flush; \
        int failedBefore = s_testsFailed; \
        fn(); \
        if (s_testsFailed == failedBefore) { \
            std::cout << "\r\033[1;32m[PASSED] \033[0m " << #fn << "\n"; \
            s_testsPassed++; \
        } \
    } while (0)

// Test 1: CommandRunner Structured Execution
void testCommandRunner() {
    auto res = LinuxManager::Core::CommandRunner::run("echo", {"hello", "linux", "manager"});
    TEST_ASSERT(res.success(), "echo command should succeed");
    TEST_ASSERT(res.stdOut.find("hello linux manager") != std::string::npos, "output should match arguments");

    // Timeout test
    auto timeoutRes = LinuxManager::Core::CommandRunner::run("sleep", {"2"}, std::chrono::milliseconds(100));
    TEST_ASSERT(timeoutRes.timedOut, "sleep 2 with 100ms timeout should time out");
}

// Test 2: Security Policy and Recursive Subtree Protection
void testSecurityPolicyAndSubtreeProtection() {
    auto policy = std::make_shared<LinuxManager::Security::PolicyManager>();
    auto validator = std::make_shared<LinuxManager::Security::OperationValidator>(policy);
    const std::string testHome = "/home/testuser";

    // Critical root and system subtrees MUST be protected
    TEST_ASSERT(policy->isPathInsideProtectedSubtree("/"), "Root directory is inside protected subtree");
    TEST_ASSERT(policy->isPathInsideProtectedSubtree("/etc"), "/etc is inside protected subtree");
    TEST_ASSERT(policy->isPathInsideProtectedSubtree("/etc/shadow"), "/etc/shadow is inside protected subtree");
    TEST_ASSERT(policy->isPathInsideProtectedSubtree("/etc/pam.d/common-auth"), "/etc/pam.d is inside protected subtree");
    TEST_ASSERT(policy->isPathInsideProtectedSubtree("/usr/bin/bash"), "/usr/bin/bash is inside protected subtree");
    TEST_ASSERT(policy->isPathInsideProtectedSubtree("/var/lib/dpkg/status"), "/var/lib/dpkg is inside protected subtree");
    TEST_ASSERT(policy->isPathInsideProtectedSubtree("/boot/vmlinuz"), "/boot is inside protected subtree");

    // Subtree deletions must be strictly rejected
    TEST_ASSERT(!policy->isDeletionPermitted("/etc/shadow", testHome), "Deleting /etc/shadow must be blocked");
    TEST_ASSERT(!policy->isDeletionPermitted("/usr/bin", testHome), "Deleting /usr/bin must be blocked");
    TEST_ASSERT(!policy->isDeletionPermitted("/var/log", testHome), "Deleting /var/log must be blocked");
    TEST_ASSERT(!policy->isDeletionPermitted("/home", testHome), "Deleting /home must be blocked");
    TEST_ASSERT(!policy->isDeletionPermitted("/home/otheruser/.cache", testHome), "Deleting other user home must be blocked");

    // Allowed user caches & project artifacts must be permitted
    TEST_ASSERT(policy->isDeletionPermitted("/home/testuser/.cache/thumbnails", testHome), "~/.cache/thumbnails must be permitted");
    TEST_ASSERT(policy->isDeletionPermitted("/home/testuser/.npm/_cacache", testHome), "~/.npm must be permitted");
    TEST_ASSERT(policy->isDeletionPermitted("/home/testuser/.cargo/registry/cache", testHome), "~/.cargo/registry must be permitted");
    TEST_ASSERT(policy->isDeletionPermitted("/home/testuser/Projects/myapp/node_modules", testHome), "project node_modules must be permitted");
    TEST_ASSERT(policy->isDeletionPermitted("/home/testuser/Projects/myapp/target", testHome), "project target/ must be permitted");

    // Protected packages
    TEST_ASSERT(policy->isPackageProtected("systemd"), "systemd must be protected");
    TEST_ASSERT(policy->isPackageProtected("libc6"), "libc6 must be protected");
    TEST_ASSERT(policy->isPackageProtected("apt"), "apt must be protected");
    TEST_ASSERT(!policy->isPackageProtected("vlc"), "vlc should not be protected");

    // Validator checks
    auto delVal = validator->validateFileDeletion("/etc/shadow", testHome);
    TEST_ASSERT(!delVal.valid, "Deleting /etc/shadow must fail validation");

    auto pkgVal = validator->validatePackageRemoval("systemd");
    TEST_ASSERT(!pkgVal.valid, "Removing systemd must fail validation");

    auto okPkgVal = validator->validatePackageRemoval("vlc");
    TEST_ASSERT(okPkgVal.valid, "Removing vlc should pass validation");
}

// Test 3: Allow-List Privileged Operations
void testPrivilegedOperationAllowList() {
    auto policy = std::make_shared<LinuxManager::Security::PolicyManager>();
    auto validator = std::make_shared<LinuxManager::Security::OperationValidator>(policy);
    auto mock = std::make_shared<LinuxManager::Core::MockSystemBackend>();

    // Test valid AptUpdate
    auto opUpdate = LinuxManager::Security::PrivilegedOperation::makeAptUpdate();
    auto valUpdate = validator->validatePrivilegedOperation(opUpdate);
    TEST_ASSERT(valUpdate.valid, "AptUpdate must be valid");

    // Test valid AptInstall
    auto opInstall = LinuxManager::Security::PrivilegedOperation::makeAptInstall({"htop", "curl"});
    auto valInstall = validator->validatePrivilegedOperation(opInstall);
    TEST_ASSERT(valInstall.valid, "AptInstall with valid pkgs must be valid");

    // Test invalid AptInstall with injection attempt
    auto opBadInstall = LinuxManager::Security::PrivilegedOperation::makeAptInstall({"htop; rm -rf /"});
    auto valBadInstall = validator->validatePrivilegedOperation(opBadInstall);
    TEST_ASSERT(!valBadInstall.valid, "AptInstall with invalid package syntax must be rejected");

    // Test AptRemove on protected package
    auto opBadRemove = LinuxManager::Security::PrivilegedOperation::makeAptRemove({"systemd"});
    auto valBadRemove = validator->validatePrivilegedOperation(opBadRemove);
    TEST_ASSERT(!valBadRemove.valid, "AptRemove on systemd must be rejected");

    // Test JournalVacuum below minimum limit (must be >= 50MB)
    auto opSmallVacuum = LinuxManager::Security::PrivilegedOperation::makeJournalVacuumSize(10);
    auto valSmallVacuum = validator->validatePrivilegedOperation(opSmallVacuum);
    TEST_ASSERT(!valSmallVacuum.valid, "Journal vacuum < 50MB must be rejected");

    auto opGoodVacuum = LinuxManager::Security::PrivilegedOperation::makeJournalVacuumSize(100);
    auto valGoodVacuum = validator->validatePrivilegedOperation(opGoodVacuum);
    TEST_ASSERT(valGoodVacuum.valid, "Journal vacuum 100MB must be valid");

    // Test Service action on protected service
    auto opStopDbus = LinuxManager::Security::PrivilegedOperation::makeServiceAction(
        LinuxManager::Security::PrivilegedOpType::ServiceStop, "dbus.service"
    );
    auto valStopDbus = validator->validatePrivilegedOperation(opStopDbus);
    TEST_ASSERT(!valStopDbus.valid, "Stopping dbus.service must be rejected");

    // Test Dispatcher execution on mock backend
    LinuxManager::Security::PrivilegedDispatcher dispatcher(mock, validator);
    auto dispRes = dispatcher.dispatch(opGoodVacuum);
    TEST_ASSERT(dispRes.success(), "Dispatching valid journal vacuum on mock should succeed");
    TEST_ASSERT(!mock->getExecutedPrivilegedOperations().empty(), "Mock backend received privileged operation");
}

// Test 4: Symlink Safety
void testSymlinkSafety() {
    auto mock = std::make_shared<LinuxManager::Core::MockSystemBackend>();
    mock->setMockDirectory("/home/testuser/.cache/thumbnails", 1024 * 1024);
    mock->setMockSymlink("/home/testuser/.cache/fake_symlink", "/etc/shadow");

    TEST_ASSERT(mock->pathExists("/home/testuser/.cache/fake_symlink"), "Mock symlink exists");
    
    // Removing the symlink itself must succeed without removing target
    bool removed = mock->removePath("/home/testuser/.cache/fake_symlink");
    TEST_ASSERT(removed, "Removing symlink node succeeds");
    TEST_ASSERT(!mock->pathExists("/home/testuser/.cache/fake_symlink"), "Symlink node is removed");
}

// Test 5: Mock Backend & History Manager
void testMockAndHistory() {
    auto mock = std::make_shared<LinuxManager::Core::MockSystemBackend>();
    mock->setMockFile("/home/testuser/.cache/thumbnails/test.png", "fake-png-data", 1024 * 1024);

    TEST_ASSERT(mock->pathExists("/home/testuser/.cache/thumbnails/test.png"), "Mock file exists");
    TEST_ASSERT(mock->getPathSize("/home/testuser/.cache/thumbnails/test.png") == 1024 * 1024, "Mock file size matches");

    auto history = std::make_shared<LinuxManager::Core::HistoryManager>(mock);
    LinuxManager::Core::HistoryRecord rec;
    rec.operationId = history->generateOperationId();
    rec.timestamp = "2026-08-22 20:00:00";
    rec.category = "System Cleanup";
    rec.description = "Test Cleanup";
    rec.bytesReclaimed = 500 * 1024 * 1024;
    rec.itemsCount = 3;
    rec.success = true;

    history->recordOperation(rec);

    auto histList = history->getHistory();
    TEST_ASSERT(!histList.empty(), "History list should not be empty");
    TEST_ASSERT(histList[0].bytesReclaimed == 500 * 1024 * 1024, "Reclaimed bytes match");
    TEST_ASSERT(history->getTotalReclaimedBytes() == 500 * 1024 * 1024, "Total reclaimed bytes match");
}

// Test 6: System Information Parsing
void testSystemInfo() {
    auto mock = std::make_shared<LinuxManager::Core::MockSystemBackend>();
    LinuxManager::System::SystemInfo info(mock);

    auto overview = info.getSystemOverview();
    TEST_ASSERT(!overview.cpu.model.empty(), "CPU model should not be empty");
    TEST_ASSERT(overview.memory.totalBytes > 0, "Memory total should be > 0");
    TEST_ASSERT(overview.battery.present, "Battery should be detected");
}

// Test 7: Cleanup Engine and Developer Cleaner
void testCleanupEngine() {
    auto mock = std::make_shared<LinuxManager::Core::MockSystemBackend>();
    auto policy = std::make_shared<LinuxManager::Security::PolicyManager>();
    auto validator = std::make_shared<LinuxManager::Security::OperationValidator>(policy);
    auto history = std::make_shared<LinuxManager::Core::HistoryManager>(mock);

    mock->setMockDirectory("/var/cache/apt/archives", 400 * 1024 * 1024);
    mock->setMockDirectory("/home/testuser/.npm", 1200 * 1024 * 1024);
    mock->setMockDirectory("/home/testuser/.cache/pip", 300 * 1024 * 1024);

    LinuxManager::Cleanup::CleanupEngine engine(mock, validator, history);
    auto preview = engine.scanAll();

    TEST_ASSERT(preview.totalReclaimableBytes > 0, "Total reclaimable bytes should be > 0");

    // Test Dry Run execution
    auto dryRes = engine.executeCleanup(preview.items, true);
    TEST_ASSERT(dryRes.success, "Dry run should succeed");
    TEST_ASSERT(dryRes.bytesCleaned > 0, "Dry run bytes cleaned > 0");
    TEST_ASSERT(mock->getDeletedPaths().empty(), "Dry run should not delete actual mock paths");

    // Test Real Execution on mock
    auto execRes = engine.executeCleanup(preview.items, false);
    TEST_ASSERT(execRes.success, "Cleanup execution on mock should succeed");
}

// Test 8: APT Package & Removal Preview
void testAptManager() {
    auto mock = std::make_shared<LinuxManager::Core::MockSystemBackend>();
    auto policy = std::make_shared<LinuxManager::Security::PolicyManager>();
    auto validator = std::make_shared<LinuxManager::Security::OperationValidator>(policy);

    std::string fakeStatus =
        "Package: test-editor\n"
        "Status: install ok installed\n"
        "Version: 1.0.0\n"
        "Section: editors\n"
        "Installed-Size: 51200\n"
        "Description: Test Code Editor\n\n";

    mock->setMockFile("/var/lib/dpkg/status", fakeStatus);
    mock->setMockDirectory("/home/testuser/.config/test-editor", 10 * 1024 * 1024);
    mock->setMockDirectory("/home/testuser/.cache/test-editor", 20 * 1024 * 1024);

    LinuxManager::Package::AptManager apt(mock, validator);
    auto installed = apt.getInstalledPackages();
    TEST_ASSERT(installed.size() == 1, "Should have 1 installed package");
    TEST_ASSERT(installed[0].name == "test-editor", "Package name should match");

    auto preview = apt.generateRemovalPreview("test-editor");
    TEST_ASSERT(preview.installedSizeBytes == 51200 * 1024, "Installed size matches");
    TEST_ASSERT(!preview.configDirectories.empty(), "Config directory detected");
    TEST_ASSERT(!preview.cacheDirectories.empty(), "Cache directory detected");
}

// Test 9: Storage & Large File Scanner
void testStorageScanner() {
    auto mock = std::make_shared<LinuxManager::Core::MockSystemBackend>();
    LinuxManager::Storage::StorageScanner scanner(mock);

    auto partitions = scanner.getPartitions();
    TEST_ASSERT(!partitions.empty(), "Partitions list should not be empty");
    TEST_ASSERT(partitions[0].mountPoint == "/", "Root partition detected");

    LinuxManager::Storage::LargeFileScanner largeScanner(mock);
    auto largeFiles = largeScanner.findLargeFiles("/home/testuser", 500 * 1024 * 1024);
    TEST_ASSERT(largeFiles.size() == 2, "Found 2 large mock files");
}

// Test 10: Service & Kernel Safeguards
void testServiceAndKernelSafeguards() {
    auto mock = std::make_shared<LinuxManager::Core::MockSystemBackend>();
    LinuxManager::System::ServiceManager srv(mock);

    TEST_ASSERT(srv.isServiceProtected("dbus.service"), "dbus.service is protected");
    auto stopDbRes = srv.stopService("dbus.service");
    TEST_ASSERT(!stopDbRes.success(), "Stopping dbus.service must be rejected");

    LinuxManager::System::KernelManager kern(mock);
    auto installedKernels = kern.getInstalledKernels();
    TEST_ASSERT(!installedKernels.empty(), "Installed kernels found");

    auto running = kern.getRunningKernel();
    if (!running.empty()) {
        auto rmRunningRes = kern.removeKernel(running);
        TEST_ASSERT(!rmRunningRes.success(), "Removing running kernel must be blocked");
    }
}

// Test 11: SDK Doctor & Toolchain Diagnostics
void testSdkDoctor() {
    auto mock = std::make_shared<LinuxManager::Core::MockSystemBackend>();
    LinuxManager::Developer::SdkDoctor doctor(mock);

    auto report = doctor.runDiagnostics();
    TEST_ASSERT(!report.sdks.empty(), "SDK list should not be empty");
    TEST_ASSERT(report.installedCount > 0, "Installed SDKs count should be > 0");

    auto cleanGo = doctor.cleanGoCache();
    TEST_ASSERT(cleanGo.success(), "Go cache clean on mock succeeds");
}

// Test 12: Process Manager & Port Finder
void testProcessManagerAndPortKiller() {
    auto mock = std::make_shared<LinuxManager::Core::MockSystemBackend>();
    LinuxManager::Process::ProcessManager pm(mock);

    auto procs = pm.getRunningProcesses(true);
    TEST_ASSERT(!procs.empty(), "Developer processes detected");
    TEST_ASSERT(procs[0].isDeveloperTool, "First process marked as developer tool");

    auto ports = pm.getListeningPorts();
    TEST_ASSERT(!ports.empty(), "Listening ports detected");
    TEST_ASSERT(ports[0].port == 3000, "Port 3000 found");

    auto found = pm.findPortsByNumber(3000);
    TEST_ASSERT(found.size() == 1, "Found single port 3000");

    bool killed = pm.killProcessOnPort(3000);
    TEST_ASSERT(killed, "Killed process on port 3000");
}

// Test 13: Startup Application Manager
void testStartupManager() {
    auto mock = std::make_shared<LinuxManager::Core::MockSystemBackend>();
    LinuxManager::Startup::StartupManager sm(mock);

    auto apps = sm.getAutostartApplications();
    TEST_ASSERT(!apps.empty(), "Autostart apps detected");
    TEST_ASSERT(apps[0].name == "Discord", "Discord autostart app parsed");

    bool toggleRes = sm.setAppEnabled("discord.desktop", false);
    TEST_ASSERT(toggleRes, "Toggling autostart app succeeds");
}

// Test 14: APT Repository & PPA Manager
void testRepositoryManager() {
    auto mock = std::make_shared<LinuxManager::Core::MockSystemBackend>();
    LinuxManager::Package::RepositoryManager rm(mock);

    auto summary = rm.getRepositorySummary();
    TEST_ASSERT(summary.totalCount == 3, "Total repositories match 3");
    TEST_ASSERT(summary.thirdPartyCount == 2, "Third-party count matches 2");
    TEST_ASSERT(!summary.repositories[1].ppaName.empty(), "PPA name parsed");
}

// Test 15: Battery Health & Power Profiles
void testPowerManager() {
    auto mock = std::make_shared<LinuxManager::Core::MockSystemBackend>();
    LinuxManager::System::PowerManager pm(mock);

    auto bat = pm.getBatteryHealth();
    TEST_ASSERT(bat.hasBattery, "Battery detected on mock");
    TEST_ASSERT(bat.healthPercent > 90.0, "Battery health calculated");
    TEST_ASSERT(bat.cycleCount == 142, "Cycle count matches");

    auto prof = pm.getPowerProfiles();
    TEST_ASSERT(prof.isAvailable, "Power profiles available");
    TEST_ASSERT(prof.activeProfile == "balanced", "Active profile is balanced");

    auto setProf = pm.setPowerProfile("performance");
    TEST_ASSERT(setProf.success(), "Setting power profile succeeds");
}

int main() {
    std::cout << "\n\033[1;36m==================================================\n"
              << "       LINUX MANAGER - TEST SUITE RUNNER\n"
              << "==================================================\033[0m\n\n";

    RUN_TEST(testCommandRunner);
    RUN_TEST(testSecurityPolicyAndSubtreeProtection);
    RUN_TEST(testPrivilegedOperationAllowList);
    RUN_TEST(testSymlinkSafety);
    RUN_TEST(testMockAndHistory);
    RUN_TEST(testSystemInfo);
    RUN_TEST(testCleanupEngine);
    RUN_TEST(testAptManager);
    RUN_TEST(testStorageScanner);
    RUN_TEST(testServiceAndKernelSafeguards);
    RUN_TEST(testSdkDoctor);
    RUN_TEST(testProcessManagerAndPortKiller);
    RUN_TEST(testStartupManager);
    RUN_TEST(testRepositoryManager);
    RUN_TEST(testPowerManager);

    std::cout << "\n\033[1;36m==================================================\033[0m\n";
    if (s_testsFailed == 0) {
        std::cout << "\033[1;32mALL " << s_testsPassed << " TESTS PASSED SUCCESSFULLY!\033[0m\n\n";
        return 0;
    } else {
        std::cout << "\033[1;31mTEST SUITE FAILED: " << s_testsFailed << " failed, " << s_testsPassed << " passed.\033[0m\n\n";
        return 1;
    }
}

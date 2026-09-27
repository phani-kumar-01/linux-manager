#pragma once

#include "../core/ISystemBackend.h"
#include <string>
#include <vector>
#include <memory>

namespace LinuxManager::Developer {

struct SdkInfo {
    std::string name;
    std::string category;
    std::string binaryName;
    std::string version;
    std::string installPath;
    bool isInstalled = false;
    std::vector<std::string> additionalDetails;
};

struct DiagnosticIssue {
    std::string title;
    std::string description;
    std::string severity; // "info", "warning", "error"
    std::string fixSuggestion;
    std::string fixCommand;
};

struct SdkDoctorReport {
    std::vector<SdkInfo> sdks;
    std::vector<DiagnosticIssue> issues;
    int installedCount = 0;
    int healthyCount = 0;
    int warningCount = 0;
};

class SdkDoctor {
public:
    explicit SdkDoctor(std::shared_ptr<Core::ISystemBackend> backend);

    [[nodiscard]] SdkDoctorReport runDiagnostics();
    [[nodiscard]] std::vector<SdkInfo> detectAllSdks();
    [[nodiscard]] std::vector<DiagnosticIssue> checkEnvironmentHealth();

    [[nodiscard]] uint64_t getGoCacheSize() const;
    [[nodiscard]] Core::CommandResult cleanGoCache();

private:
    std::shared_ptr<Core::ISystemBackend> m_backend;

    [[nodiscard]] SdkInfo detectSdk(
        const std::string& name,
        const std::string& category,
        const std::string& binary,
        const std::vector<std::string>& versionArgs = {"--version"}
    );
};

} // namespace LinuxManager::Developer

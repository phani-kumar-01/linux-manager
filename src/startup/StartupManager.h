#pragma once

#include "../core/ISystemBackend.h"
#include <string>
#include <vector>
#include <memory>

namespace LinuxManager::Startup {

struct AutostartApp {
    std::string id;
    std::string name;
    std::string comment;
    std::string exec;
    std::string icon;
    std::string filePath;
    bool isEnabled = true;
    bool isSystemWide = false;
};

class StartupManager {
public:
    explicit StartupManager(std::shared_ptr<Core::ISystemBackend> backend);

    [[nodiscard]] std::vector<AutostartApp> getAutostartApplications();
    [[nodiscard]] bool setAppEnabled(const std::string& appId, bool enabled);
    [[nodiscard]] bool addAutostartApp(const std::string& name, const std::string& exec, const std::string& comment = "");
    [[nodiscard]] bool removeAutostartApp(const std::string& appId);

private:
    std::shared_ptr<Core::ISystemBackend> m_backend;

    void parseDesktopFile(const std::string& filePath, bool isSystemWide, std::vector<AutostartApp>& list);
};

} // namespace LinuxManager::Startup

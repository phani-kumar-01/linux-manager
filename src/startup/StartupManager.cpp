#include "StartupManager.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>

namespace LinuxManager::Startup {

StartupManager::StartupManager(std::shared_ptr<Core::ISystemBackend> backend)
    : m_backend(std::move(backend)) {}

void StartupManager::parseDesktopFile(const std::string& filePath, bool isSystemWide, std::vector<AutostartApp>& list) {
    if (!m_backend) return;
    std::string content = m_backend->readFileContents(filePath);
    if (content.empty()) return;

    std::istringstream stream(content);
    std::string line;

    AutostartApp app;
    app.filePath = filePath;
    app.isSystemWide = isSystemWide;
    app.isEnabled = true;

    std::filesystem::path p(filePath);
    app.id = p.filename().string();

    bool inDesktopEntry = false;

    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty() || line[0] == '#') continue;

        if (line == "[Desktop Entry]") {
            inDesktopEntry = true;
            continue;
        } else if (line.starts_with("[") && line.ends_with("]")) {
            inDesktopEntry = false;
            continue;
        }

        if (inDesktopEntry) {
            auto eqPos = line.find('=');
            if (eqPos != std::string::npos) {
                std::string key = line.substr(0, eqPos);
                std::string val = line.substr(eqPos + 1);

                if (key == "Name" && app.name.empty()) {
                    app.name = val;
                } else if (key == "Comment" && app.comment.empty()) {
                    app.comment = val;
                } else if (key == "Exec") {
                    app.exec = val;
                } else if (key == "Icon") {
                    app.icon = val;
                } else if (key == "Hidden" && (val == "true" || val == "1")) {
                    app.isEnabled = false;
                } else if (key == "X-GNOME-Autostart-enabled" && (val == "false" || val == "0")) {
                    app.isEnabled = false;
                }
            }
        }
    }

    if (app.name.empty()) {
        app.name = app.id;
    }

    list.push_back(app);
}

std::vector<AutostartApp> StartupManager::getAutostartApplications() {
    std::vector<AutostartApp> list;

    if (m_backend && m_backend->isMock()) {
        list.push_back({"discord.desktop", "Discord", "All-in-one voice and text chat", "/usr/bin/discord", "discord", "/home/testuser/.config/autostart/discord.desktop", true, false});
        list.push_back({"docker-desktop.desktop", "Docker Desktop", "Docker Desktop application", "/usr/bin/docker-desktop", "docker", "/home/testuser/.config/autostart/docker-desktop.desktop", false, false});
        list.push_back({"geoclue-demo-autostart.desktop", "Geoclue Demo", "Geolocation Service", "/usr/libexec/geoclue", "geoclue", "/etc/xdg/autostart/geoclue-demo.desktop", true, true});
        return list;
    }

    if (!m_backend) return list;

    std::string userAutostart = m_backend->getUserHomeDirectory() + "/.config/autostart";
    std::string sysAutostart = "/etc/xdg/autostart";

    std::error_code ec;
    // 1. User autostarts
    if (std::filesystem::exists(userAutostart, ec)) {
        for (const auto& entry : std::filesystem::directory_iterator(userAutostart, ec)) {
            if (entry.path().extension() == ".desktop") {
                parseDesktopFile(entry.path().string(), false, list);
            }
        }
    }

    // 2. System autostarts
    if (std::filesystem::exists(sysAutostart, ec)) {
        for (const auto& entry : std::filesystem::directory_iterator(sysAutostart, ec)) {
            if (entry.path().extension() == ".desktop") {
                // Only add if not shadowed by user autostart
                std::string filename = entry.path().filename().string();
                bool shadowed = false;
                for (const auto& existing : list) {
                    if (existing.id == filename) {
                        shadowed = true;
                        break;
                    }
                }
                if (!shadowed) {
                    parseDesktopFile(entry.path().string(), true, list);
                }
            }
        }
    }

    return list;
}

bool StartupManager::setAppEnabled(const std::string& appId, bool enabled) {
    if (!m_backend) return false;

    if (m_backend->isMock()) {
        return true;
    }

    std::string userAutostartDir = m_backend->getUserHomeDirectory() + "/.config/autostart";
    std::error_code ec;
    std::filesystem::create_directories(userAutostartDir, ec);

    std::string targetPath = userAutostartDir + "/" + appId;

    // Check if modifying existing user file or copying from system to user directory
    std::string content;
    if (std::filesystem::exists(targetPath, ec)) {
        content = m_backend->readFileContents(targetPath);
    } else {
        std::string sysPath = "/etc/xdg/autostart/" + appId;
        if (std::filesystem::exists(sysPath, ec)) {
            content = m_backend->readFileContents(sysPath);
        }
    }

    if (content.empty()) {
        return false;
    }

    // Replace or insert Hidden / X-GNOME-Autostart-enabled
    std::istringstream stream(content);
    std::ostringstream out;
    std::string line;
    bool foundHidden = false;
    bool foundGnome = false;

    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();

        if (line.starts_with("Hidden=")) {
            out << "Hidden=" << (enabled ? "false" : "true") << "\n";
            foundHidden = true;
        } else if (line.starts_with("X-GNOME-Autostart-enabled=")) {
            out << "X-GNOME-Autostart-enabled=" << (enabled ? "true" : "false") << "\n";
            foundGnome = true;
        } else {
            out << line << "\n";
        }
    }

    if (!foundHidden) {
        out << "Hidden=" << (enabled ? "false" : "true") << "\n";
    }
    if (!foundGnome) {
        out << "X-GNOME-Autostart-enabled=" << (enabled ? "true" : "false") << "\n";
    }

    std::ofstream file(targetPath);
    if (!file.is_open()) return false;
    file << out.str();
    return true;
}

bool StartupManager::addAutostartApp(const std::string& name, const std::string& exec, const std::string& comment) {
    if (!m_backend || name.empty() || exec.empty()) return false;

    if (m_backend->isMock()) return true;

    std::string userAutostartDir = m_backend->getUserHomeDirectory() + "/.config/autostart";
    std::error_code ec;
    std::filesystem::create_directories(userAutostartDir, ec);

    std::string safeName = name;
    std::transform(safeName.begin(), safeName.end(), safeName.begin(), [](char c) {
        return (std::isalnum(c) || c == '-') ? std::tolower(c) : '-';
    });
    std::string desktopPath = userAutostartDir + "/" + safeName + ".desktop";

    std::ofstream file(desktopPath);
    if (!file.is_open()) return false;

    file << "[Desktop Entry]\n"
         << "Type=Application\n"
         << "Name=" << name << "\n"
         << "Exec=" << exec << "\n"
         << "Comment=" << comment << "\n"
         << "Hidden=false\n"
         << "X-GNOME-Autostart-enabled=true\n";

    return true;
}

bool StartupManager::removeAutostartApp(const std::string& appId) {
    if (!m_backend) return false;

    if (m_backend->isMock()) return true;

    std::string targetPath = m_backend->getUserHomeDirectory() + "/.config/autostart/" + appId;
    std::error_code ec;
    if (std::filesystem::exists(targetPath, ec)) {
        return std::filesystem::remove(targetPath, ec);
    }
    // If it's a system wide app, disable it via user override
    return setAppEnabled(appId, false);
}

} // namespace LinuxManager::Startup

#include "ProcessManager.h"
#include <fstream>
#include <sstream>
#include <regex>
#include <filesystem>
#include <algorithm>
#include <csignal>
#include <unistd.h>
#include <pwd.h>

namespace LinuxManager::Process {

ProcessManager::ProcessManager(std::shared_ptr<Core::ISystemBackend> backend)
    : m_backend(std::move(backend)) {}

bool ProcessManager::isDevProcess(const std::string& name, const std::string& cmdline) const {
    static const std::vector<std::string> devKeywords = {
        "node", "npm", "npx", "pnpm", "yarn", "bun", "deno", "vite", "next", "webpack", "tsc",
        "python", "python3", "pytest", "uvicorn", "gunicorn", "flask", "django", "celery",
        "rustc", "cargo", "rust-analyzer", "clippy",
        "go", "gopls", "dlv",
        "gcc", "g++", "clang", "clangd", "cmake", "ninja", "make",
        "java", "javac", "gradle", "mvn", "kotlin",
        "docker", "dockerd", "containerd", "podman", "kubectl", "minikube",
        "redis-server", "postgres", "mysqld", "mongod", "nginx",
        "code", "cursor", "idea", "clion", "pycharm", "webstorm"
    };

    std::string lowerName = name;
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
    std::string lowerCmd = cmdline;
    std::transform(lowerCmd.begin(), lowerCmd.end(), lowerCmd.begin(), ::tolower);

    for (const auto& kw : devKeywords) {
        if (lowerName == kw || lowerName.find(kw) != std::string::npos ||
            lowerCmd.find("/" + kw) != std::string::npos || lowerCmd.find(kw) != std::string::npos) {
            return true;
        }
    }
    return false;
}

void ProcessManager::parseProcPid(pid_t pid, std::vector<ProcessInfo>& list) {
    std::string procPath = "/proc/" + std::to_string(pid);
    std::string statusPath = procPath + "/status";
    std::string cmdlinePath = procPath + "/cmdline";

    std::ifstream statusFile(statusPath);
    if (!statusFile.is_open()) return;

    ProcessInfo info;
    info.pid = pid;

    std::string line;
    uid_t uid = 0;
    while (std::getline(statusFile, line)) {
        if (line.starts_with("Name:\t")) {
            info.name = line.substr(6);
        } else if (line.starts_with("PPid:\t")) {
            info.ppid = std::stoi(line.substr(6));
        } else if (line.starts_with("State:\t")) {
            info.state = line.substr(7, 1);
        } else if (line.starts_with("VmRSS:\t")) {
            // VmRSS in kB
            std::istringstream ss(line.substr(7));
            uint64_t kb = 0;
            ss >> kb;
            info.memoryRssBytes = kb * 1024;
        } else if (line.starts_with("Uid:\t")) {
            std::istringstream ss(line.substr(5));
            ss >> uid;
        }
    }

    struct passwd* pw = getpwuid(uid);
    info.user = (pw && pw->pw_name) ? pw->pw_name : std::to_string(uid);

    // Read cmdline
    std::ifstream cmdFile(cmdlinePath);
    if (cmdFile.is_open()) {
        std::stringstream buffer;
        buffer << cmdFile.rdbuf();
        std::string raw = buffer.str();
        std::replace(raw.begin(), raw.end(), '\0', ' ');
        if (!raw.empty() && raw.back() == ' ') raw.pop_back();
        info.cmdline = raw;
    }
    if (info.cmdline.empty()) {
        info.cmdline = "[" + info.name + "]";
    }

    info.isDeveloperTool = isDevProcess(info.name, info.cmdline);
    list.push_back(info);
}

std::vector<ProcessInfo> ProcessManager::getRunningProcesses(bool developerOnly) {
    std::vector<ProcessInfo> list;

    if (m_backend && m_backend->isMock()) {
        list.push_back({1024, 1, "node", "candy", "node /home/candy/project/server.js", 120 * 1024 * 1024, 2.5, "S", true});
        list.push_back({1025, 1, "python3", "candy", "python3 -m http.server 8000", 45 * 1024 * 1024, 0.5, "S", true});
        list.push_back({1026, 1, "rust-analyzer", "candy", "rust-analyzer", 450 * 1024 * 1024, 8.0, "S", true});
        list.push_back({500, 1, "systemd-logind", "root", "/lib/systemd/systemd-logind", 12 * 1024 * 1024, 0.0, "S", false});

        if (developerOnly) {
            std::vector<ProcessInfo> filtered;
            for (const auto& p : list) {
                if (p.isDeveloperTool) filtered.push_back(p);
            }
            return filtered;
        }
        return list;
    }

    std::error_code ec;
    for (const auto& entry : std::filesystem::directory_iterator("/proc", ec)) {
        if (!entry.is_directory(ec)) continue;
        std::string filename = entry.path().filename().string();
        if (std::all_of(filename.begin(), filename.end(), ::isdigit)) {
            pid_t pid = std::stoi(filename);
            parseProcPid(pid, list);
        }
    }

    if (developerOnly) {
        std::vector<ProcessInfo> filtered;
        for (const auto& p : list) {
            if (p.isDeveloperTool) filtered.push_back(p);
        }
        return filtered;
    }

    return list;
}

std::vector<ListeningPort> ProcessManager::getListeningPorts() {
    std::vector<ListeningPort> ports;

    if (m_backend && m_backend->isMock()) {
        ports.push_back({3000, "TCP", "0.0.0.0", 1024, "node", "candy"});
        ports.push_back({8000, "TCP", "127.0.0.1", 1025, "python3", "candy"});
        ports.push_back({5432, "TCP", "127.0.0.1", 1080, "postgres", "postgres"});
        return ports;
    }

    if (!m_backend) return ports;

    // Use ss to inspect listening sockets: ss -tulpn
    auto res = m_backend->executeCommand("ss", {"-tulpn", "-H"});
    if (!res.success() || res.stdOut.empty()) {
        return ports;
    }

    std::istringstream stream(res.stdOut);
    std::string line;

    // Example ss output line:
    // tcp LISTEN 0 511 0.0.0.0:3000 0.0.0.0:* users:(("node",pid=12345,fd=19))
    static const std::regex portRegex(R"(^([a-z0-9]+)\s+LISTEN\s+\d+\s+\d+\s+([^:]+):(\d+)\s+.*users:\(\(\"([^\"]+)\",pid=(\d+))");

    while (std::getline(stream, line)) {
        std::smatch match;
        if (std::regex_search(line, match, portRegex)) {
            ListeningPort p;
            p.protocol = (match[1].str() == "tcp") ? "TCP" : "UDP";
            p.address = match[2].str();
            p.port = static_cast<uint16_t>(std::stoi(match[3].str()));
            p.processName = match[4].str();
            p.pid = std::stoi(match[5].str());

            struct passwd* pw = getpwuid(getuid());
            p.user = (pw && pw->pw_name) ? pw->pw_name : "user";
            ports.push_back(p);
        }
    }

    return ports;
}

std::vector<ListeningPort> ProcessManager::findPortsByNumber(uint16_t port) {
    auto allPorts = getListeningPorts();
    std::vector<ListeningPort> matches;
    for (const auto& p : allPorts) {
        if (p.port == port) {
            matches.push_back(p);
        }
    }
    return matches;
}

bool ProcessManager::terminateProcess(pid_t pid, bool force) {
    // Safety guards
    if (pid <= 1 || pid == getpid() || pid == getppid()) {
        return false; // Never terminate PID 1 or self
    }

    if (m_backend && m_backend->isMock()) {
        return true;
    }

    int sig = force ? SIGKILL : SIGTERM;
    return kill(pid, sig) == 0;
}

bool ProcessManager::killProcessOnPort(uint16_t port, bool force) {
    auto ports = findPortsByNumber(port);
    if (ports.empty()) return false;

    bool allKilled = true;
    for (const auto& p : ports) {
        if (!terminateProcess(p.pid, force)) {
            allKilled = false;
        }
    }
    return allKilled;
}

} // namespace LinuxManager::Process

#pragma once

#include "../core/ISystemBackend.h"
#include <string>
#include <vector>
#include <memory>
#include <cstdint>
#include <sys/types.h>

namespace LinuxManager::Process {

struct ProcessInfo {
    pid_t pid = 0;
    pid_t ppid = 0;
    std::string name;
    std::string user;
    std::string cmdline;
    uint64_t memoryRssBytes = 0;
    double cpuPercent = 0.0;
    std::string state;
    bool isDeveloperTool = false;
};

struct ListeningPort {
    uint16_t port = 0;
    std::string protocol = "TCP";
    std::string address;
    pid_t pid = 0;
    std::string processName;
    std::string user;
};

class ProcessManager {
public:
    explicit ProcessManager(std::shared_ptr<Core::ISystemBackend> backend);

    [[nodiscard]] std::vector<ProcessInfo> getRunningProcesses(bool developerOnly = false);
    [[nodiscard]] std::vector<ListeningPort> getListeningPorts();
    [[nodiscard]] std::vector<ListeningPort> findPortsByNumber(uint16_t port);

    [[nodiscard]] bool terminateProcess(pid_t pid, bool force = false);
    [[nodiscard]] bool killProcessOnPort(uint16_t port, bool force = false);

private:
    std::shared_ptr<Core::ISystemBackend> m_backend;

    [[nodiscard]] bool isDevProcess(const std::string& name, const std::string& cmdline) const;
    void parseProcPid(pid_t pid, std::vector<ProcessInfo>& list);
};

} // namespace LinuxManager::Process

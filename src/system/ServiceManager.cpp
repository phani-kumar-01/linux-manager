#include "ServiceManager.h"
#include <sstream>
#include <set>

namespace LinuxManager::System {

static const std::set<std::string> s_protectedServices = {
    "systemd-journald.service",
    "systemd-logind.service",
    "systemd-udevd.service",
    "dbus.service",
    "polkit.service",
    "NetworkManager.service",
    "systemd-networkd.service",
    "systemd-resolved.service"
};

ServiceManager::ServiceManager(std::shared_ptr<Core::ISystemBackend> backend)
    : m_backend(std::move(backend)) {}

bool ServiceManager::isServiceProtected(const std::string& serviceName) const {
    return s_protectedServices.find(serviceName) != s_protectedServices.end();
}

std::vector<ServiceUnit> ServiceManager::getAllServices() {
    std::vector<ServiceUnit> services;
    if (!m_backend) return services;

    if (m_backend->isMock()) {
        ServiceUnit s1{"docker.service", "loaded", "active", "running", "Docker Application Container Engine", false, false};
        ServiceUnit s2{"ssh.service", "loaded", "active", "running", "OpenBSD Secure Shell server", false, false};
        ServiceUnit s3{"dbus.service", "loaded", "active", "running", "D-Bus System Message Bus", false, true};
        ServiceUnit s4{"cups.service", "loaded", "inactive", "dead", "CUPS Scheduler", false, false};
        services.push_back(s1);
        services.push_back(s2);
        services.push_back(s3);
        services.push_back(s4);
        return services;
    }

    auto res = m_backend->executeCommand("systemctl", {
        "list-units", "--type=service", "--all", "--no-pager", "--no-legend"
    });

    if (!res.success()) return services;

    std::istringstream stream(res.stdOut);
    std::string line;
    while (std::getline(stream, line)) {
        if (line.empty()) continue;
        std::istringstream ls(line);
        std::string unit, load, active, sub, desc;
        if (ls >> unit >> load >> active >> sub) {
            std::getline(ls >> std::ws, desc);
            ServiceUnit s;
            s.unitName = unit;
            s.loadState = load;
            s.activeState = active;
            s.subState = sub;
            s.description = desc;
            s.isFailed = (active == "failed");
            s.isProtected = isServiceProtected(unit);
            services.push_back(s);
        }
    }

    return services;
}

std::vector<ServiceUnit> ServiceManager::getFailedServices() {
    std::vector<ServiceUnit> failed;
    auto all = getAllServices();
    for (const auto& s : all) {
        if (s.isFailed) {
            failed.push_back(s);
        }
    }
    return failed;
}

std::string ServiceManager::getServiceLogs(const std::string& serviceName, int lines) {
    if (!m_backend) return "";

    if (m_backend->isMock()) {
        return "[2026-08-22 18:00:00] Starting " + serviceName + "...\n[2026-08-22 18:00:01] Service started successfully.";
    }

    auto res = m_backend->executeCommand("journalctl", {
        "-u", serviceName, "-n", std::to_string(lines), "--no-pager"
    });

    return res.stdOut;
}

Core::CommandResult ServiceManager::restartService(const std::string& serviceName) {
    if (!m_backend) return {};
    return m_backend->executePrivilegedOperation(
        Security::PrivilegedOperation::makeServiceAction(Security::PrivilegedOpType::ServiceRestart, serviceName)
    );
}

Core::CommandResult ServiceManager::startService(const std::string& serviceName) {
    if (!m_backend) return {};
    return m_backend->executePrivilegedOperation(
        Security::PrivilegedOperation::makeServiceAction(Security::PrivilegedOpType::ServiceStart, serviceName)
    );
}

Core::CommandResult ServiceManager::stopService(const std::string& serviceName) {
    if (!m_backend) return {};
    return m_backend->executePrivilegedOperation(
        Security::PrivilegedOperation::makeServiceAction(Security::PrivilegedOpType::ServiceStop, serviceName)
    );
}

Core::CommandResult ServiceManager::enableService(const std::string& serviceName) {
    if (!m_backend) return {};
    return m_backend->executePrivilegedOperation(
        Security::PrivilegedOperation::makeServiceAction(Security::PrivilegedOpType::ServiceEnable, serviceName)
    );
}

Core::CommandResult ServiceManager::disableService(const std::string& serviceName) {
    if (!m_backend) return {};
    return m_backend->executePrivilegedOperation(
        Security::PrivilegedOperation::makeServiceAction(Security::PrivilegedOpType::ServiceDisable, serviceName)
    );
}

} // namespace LinuxManager::System

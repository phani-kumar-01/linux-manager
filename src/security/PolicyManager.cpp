#include "PolicyManager.h"
#include <algorithm>
#include <system_error>
#include <cstdlib>
#include <unistd.h>
#include <pwd.h>

namespace LinuxManager::Security {

PolicyManager::PolicyManager() {
    // Critical system roots and subtrees that must NEVER be deleted or compromised
    m_protectedSubtrees = {
        "/",
        "/bin",
        "/boot",
        "/dev",
        "/etc",
        "/lib",
        "/lib32",
        "/lib64",
        "/libx32",
        "/media",
        "/mnt",
        "/opt",
        "/proc",
        "/root",
        "/run",
        "/sbin",
        "/srv",
        "/sys",
        "/tmp",
        "/usr",
        "/var"
    };

    // Essential system packages that must never be uninstalled automatically
    m_protectedPackages = {
        "systemd",
        "systemd-sysv",
        "init",
        "dpkg",
        "apt",
        "libc6",
        "login",
        "bash",
        "coreutils",
        "polkitd",
        "policykit-1",
        "linux-manager"
    };

    // Critical system services that must never be stopped or disabled by the app
    m_protectedServices = {
        "systemd-journald.service",
        "systemd-logind.service",
        "systemd-udevd.service",
        "dbus.service",
        "polkit.service",
        "NetworkManager.service",
        "systemd-networkd.service",
        "systemd-resolved.service"
    };

    m_allowedDevArtifactNames = {
        "node_modules",
        "target",
        "__pycache__",
        ".pytest_cache",
        ".mypy_cache",
        ".venv",
        "venv",
        ".gradle",
        "build",
        "dist"
    };
}

bool PolicyManager::isPathProtected(const std::string& path) const {
    if (path.empty()) return true;

    std::error_code ec;
    std::filesystem::path target = std::filesystem::weakly_canonical(path, ec);
    std::string norm = target.lexically_normal().string();

    for (const auto& prot : m_protectedSubtrees) {
        std::filesystem::path protPath = std::filesystem::path(prot).lexically_normal();
        if (norm == protPath.string()) {
            return true;
        }
    }

    return false;
}

bool PolicyManager::isPathInsideProtectedSubtree(const std::string& path) const {
    if (path.empty()) return true;

    std::error_code ec;
    std::filesystem::path target = std::filesystem::weakly_canonical(path, ec).lexically_normal();
    std::string targetStr = target.string();

    // Check against all protected system subtrees
    for (const auto& prot : m_protectedSubtrees) {
        if (prot == "/") {
            if (targetStr == "/") return true;
            continue;
        }
        std::filesystem::path protPath = std::filesystem::path(prot).lexically_normal();
        std::string protStr = protPath.string();

        if (targetStr == protStr || targetStr.starts_with(protStr + "/")) {
            // Exception: /var/crash and /var/cache/apt/archives are handled via specialized cleaning
            return true;
        }
    }

    return false;
}

bool PolicyManager::isAllowedDevArtifact(const std::filesystem::path& path) const {
    std::string filename = path.filename().string();
    for (const auto& allowed : m_allowedDevArtifactNames) {
        if (filename == allowed) {
            return true;
        }
    }
    return false;
}

bool PolicyManager::isAllowedUserCacheLocation(const std::filesystem::path& path, const std::string& userHome) const {
    std::string home = userHome;
    if (home.empty()) {
        const char* h = std::getenv("HOME");
        if (h) home = h;
    }
    if (home.empty()) return false;

    std::filesystem::path homePath = std::filesystem::path(home).lexically_normal();
    std::string targetStr = path.lexically_normal().string();
    std::string homeStr = homePath.string();

    if (!targetStr.starts_with(homeStr + "/")) {
        return false; // Must be strictly within user home
    }

    // Allowed sub-branches within user home
    std::vector<std::string> allowedPrefixes = {
        homeStr + "/.cache",
        homeStr + "/.npm",
        homeStr + "/.pnpm-store",
        homeStr + "/.yarn",
        homeStr + "/.cargo/registry",
        homeStr + "/.cargo/git",
        homeStr + "/.gradle/caches",
        homeStr + "/.gradle/daemon",
        homeStr + "/.m2/repository",
        homeStr + "/.android/cache",
        homeStr + "/.android/build-cache",
        homeStr + "/.local/share/Trash"
    };

    for (const auto& prefix : allowedPrefixes) {
        if (targetStr == prefix || targetStr.starts_with(prefix + "/")) {
            return true;
        }
    }

    // Check if target is a recognized developer artifact folder within user project directories
    return isAllowedDevArtifact(path);
}

bool PolicyManager::isDeletionPermitted(const std::string& path, const std::string& userHome) const {
    if (path.empty()) return false;

    std::error_code ec;
    std::filesystem::path target = std::filesystem::weakly_canonical(path, ec).lexically_normal();

    // Check if path is protected root or system subtree
    if (isPathInsideProtectedSubtree(target.string())) {
        return false;
    }

    // If symlink, ensure we check the real canonical path as well
    if (std::filesystem::is_symlink(target, ec)) {
        std::filesystem::path realTarget = std::filesystem::canonical(target, ec);
        if (!ec && isPathInsideProtectedSubtree(realTarget.string())) {
            return false;
        }
    }

    // Enforce that path is an allowed user cache location or project artifact
    return isAllowedUserCacheLocation(target, userHome);
}

bool PolicyManager::isPackageProtected(const std::string& packageName) const {
    return m_protectedPackages.find(packageName) != m_protectedPackages.end();
}

bool PolicyManager::isServiceProtected(const std::string& serviceName) const {
    return m_protectedServices.find(serviceName) != m_protectedServices.end();
}

void PolicyManager::addProtectedPath(const std::string& path) {
    if (!path.empty()) {
        m_protectedSubtrees.push_back(path);
    }
}

void PolicyManager::addProtectedPackage(const std::string& packageName) {
    if (!packageName.empty()) {
        m_protectedPackages.insert(packageName);
    }
}

void PolicyManager::addProtectedService(const std::string& serviceName) {
    if (!serviceName.empty()) {
        m_protectedServices.insert(serviceName);
    }
}

} // namespace LinuxManager::Security

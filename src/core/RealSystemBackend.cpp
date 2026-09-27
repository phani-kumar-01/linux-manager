#include "RealSystemBackend.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <unistd.h>
#include <pwd.h>

namespace LinuxManager::Core {

RealSystemBackend::RealSystemBackend() {
    m_policy = std::make_shared<Security::PolicyManager>();
    m_validator = std::make_shared<Security::OperationValidator>(m_policy);
}

CommandResult RealSystemBackend::executeCommand(
    const std::string& executable,
    const std::vector<std::string>& arguments,
    PrivilegeLevel privilege,
    std::chrono::milliseconds timeout
) {
    if (privilege == PrivilegeLevel::RootRequired) {
        // Warning: Direct root command without PrivilegedOperation is disallowed on real backend.
        // Return security error to enforce PrivilegedOperation usage.
        CommandResult err;
        err.exitCode = 126;
        err.stdErr = "Direct elevated execution deprecated. Use executePrivilegedOperation with typed operation instead.";
        return err;
    }
    return CommandRunner::run(executable, arguments, timeout);
}

CommandResult RealSystemBackend::executePrivilegedOperation(
    const Security::PrivilegedOperation& operation
) {
    Security::PrivilegedDispatcher dispatcher(
        std::shared_ptr<ISystemBackend>(this, [](ISystemBackend*){}),
        m_validator
    );
    return dispatcher.dispatch(operation);
}

bool RealSystemBackend::removePath(const std::string& path, bool recursive) {
    std::error_code ec;
    std::filesystem::path p(path);

    if (!std::filesystem::exists(p, ec) && !std::filesystem::is_symlink(p, ec)) {
        return true; // Already gone
    }

    // Critical Security Guard: If it is a symlink, remove the symlink itself!
    // Never recurse through a symlink to delete the target's contents.
    if (std::filesystem::is_symlink(p, ec)) {
        return std::filesystem::remove(p, ec);
    }

    if (recursive && std::filesystem::is_directory(p, ec)) {
        std::filesystem::remove_all(p, ec);
        return !ec;
    } else {
        return std::filesystem::remove(p, ec);
    }
}

bool RealSystemBackend::pathExists(const std::string& path) const {
    std::error_code ec;
    std::filesystem::path p(path);
    return std::filesystem::exists(p, ec) || std::filesystem::is_symlink(p, ec);
}

uint64_t RealSystemBackend::getPathSize(const std::string& path) const {
    std::error_code ec;
    std::filesystem::path p(path);

    if (!std::filesystem::exists(p, ec) && !std::filesystem::is_symlink(p, ec)) {
        return 0;
    }

    if (std::filesystem::is_symlink(p, ec)) {
        return 0; // Symlinks themselves take negligible size
    }

    if (std::filesystem::is_regular_file(p, ec)) {
        return std::filesystem::file_size(p, ec);
    }

    if (std::filesystem::is_directory(p, ec)) {
        uint64_t totalSize = 0;
        for (const auto& entry : std::filesystem::recursive_directory_iterator(
                 p, std::filesystem::directory_options::skip_permission_denied, ec)) {
            if (ec) break;
            if (entry.is_symlink(ec)) {
                // Do not follow symlinks when computing directory size
                continue;
            }
            if (entry.is_regular_file(ec)) {
                totalSize += entry.file_size(ec);
            }
        }
        return totalSize;
    }

    return 0;
}

std::string RealSystemBackend::readFileContents(const std::string& path) const {
    std::ifstream file(path);
    if (!file.is_open()) return "";
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string RealSystemBackend::getEnvironmentVariable(const std::string& name) const {
    const char* val = std::getenv(name.c_str());
    return val ? std::string(val) : "";
}

std::string RealSystemBackend::getUserHomeDirectory() const {
    const char* home = std::getenv("HOME");
    if (home && home[0] != '\0') {
        return std::string(home);
    }
    struct passwd* pw = getpwuid(getuid());
    if (pw && pw->pw_dir) {
        return std::string(pw->pw_dir);
    }
    return "/home";
}

std::string RealSystemBackend::getCurrentUserName() const {
    const char* user = std::getenv("USER");
    if (user && user[0] != '\0') {
        return std::string(user);
    }
    struct passwd* pw = getpwuid(getuid());
    if (pw && pw->pw_name) {
        return std::string(pw->pw_name);
    }
    return "user";
}

bool RealSystemBackend::isElevated() const {
    return geteuid() == 0;
}

} // namespace LinuxManager::Core

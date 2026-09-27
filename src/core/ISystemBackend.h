#pragma once

#include "../security/PrivilegedOperation.h"
#include <string>
#include <vector>
#include <cstdint>
#include <memory>
#include <chrono>

namespace LinuxManager::Core {

struct CommandResult {
    int exitCode = 0;
    std::string stdOut;
    std::string stdErr;
    bool timedOut = false;
    std::chrono::milliseconds executionTime{0};

    [[nodiscard]] bool success() const noexcept {
        return exitCode == 0 && !timedOut;
    }
};

enum class PrivilegeLevel {
    NormalUser,
    RootRequired
};

class ISystemBackend {
public:
    virtual ~ISystemBackend() = default;

    // Unprivileged structured command execution
    [[nodiscard]] virtual CommandResult executeCommand(
        const std::string& executable,
        const std::vector<std::string>& arguments,
        PrivilegeLevel privilege = PrivilegeLevel::NormalUser,
        std::chrono::milliseconds timeout = std::chrono::milliseconds(30000)
    ) = 0;

    // Type-safe Allow-Listed Privileged Operation execution
    [[nodiscard]] virtual CommandResult executePrivilegedOperation(
        const Security::PrivilegedOperation& operation
    ) = 0;

    // File / Directory operations (Symlink safe)
    [[nodiscard]] virtual bool removePath(const std::string& path, bool recursive = true) = 0;
    [[nodiscard]] virtual bool pathExists(const std::string& path) const = 0;
    [[nodiscard]] virtual uint64_t getPathSize(const std::string& path) const = 0;
    [[nodiscard]] virtual std::string readFileContents(const std::string& path) const = 0;

    // System Environment
    [[nodiscard]] virtual std::string getEnvironmentVariable(const std::string& name) const = 0;
    [[nodiscard]] virtual std::string getUserHomeDirectory() const = 0;
    [[nodiscard]] virtual std::string getCurrentUserName() const = 0;
    [[nodiscard]] virtual bool isElevated() const = 0;

    // Backend metadata
    [[nodiscard]] virtual std::string backendName() const noexcept = 0;
    [[nodiscard]] virtual bool isMock() const noexcept = 0;
};

} // namespace LinuxManager::Core

#pragma once

#include "ISystemBackend.h"
#include "CommandRunner.h"
#include "../security/PrivilegedDispatcher.h"
#include <memory>

namespace LinuxManager::Core {

class RealSystemBackend : public ISystemBackend {
public:
    RealSystemBackend();
    ~RealSystemBackend() override = default;

    [[nodiscard]] CommandResult executeCommand(
        const std::string& executable,
        const std::vector<std::string>& arguments,
        PrivilegeLevel privilege = PrivilegeLevel::NormalUser,
        std::chrono::milliseconds timeout = std::chrono::milliseconds(30000)
    ) override;

    [[nodiscard]] CommandResult executePrivilegedOperation(
        const Security::PrivilegedOperation& operation
    ) override;

    [[nodiscard]] bool removePath(const std::string& path, bool recursive = true) override;
    [[nodiscard]] bool pathExists(const std::string& path) const override;
    [[nodiscard]] uint64_t getPathSize(const std::string& path) const override;
    [[nodiscard]] std::string readFileContents(const std::string& path) const override;

    [[nodiscard]] std::string getEnvironmentVariable(const std::string& name) const override;
    [[nodiscard]] std::string getUserHomeDirectory() const override;
    [[nodiscard]] std::string getCurrentUserName() const override;
    [[nodiscard]] bool isElevated() const override;

    [[nodiscard]] std::string backendName() const noexcept override {
        return "RealSystemBackend";
    }

    [[nodiscard]] bool isMock() const noexcept override {
        return false;
    }

private:
    std::shared_ptr<Security::PolicyManager> m_policy;
    std::shared_ptr<Security::OperationValidator> m_validator;
};

} // namespace LinuxManager::Core

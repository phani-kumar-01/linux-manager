#pragma once

#include "ISystemBackend.h"
#include <map>
#include <set>
#include <string>
#include <vector>

namespace LinuxManager::Core {

class MockSystemBackend : public ISystemBackend {
public:
    MockSystemBackend();
    ~MockSystemBackend() override = default;

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
        return "MockSystemBackend";
    }

    [[nodiscard]] bool isMock() const noexcept override {
        return true;
    }

    // Mock setup helpers for unit testing
    void setMockFile(const std::string& path, const std::string& content, uint64_t size = 0);
    void setMockDirectory(const std::string& path, uint64_t size);
    void setMockSymlink(const std::string& linkPath, const std::string& targetPath);
    void setMockCommandResponse(const std::string& commandKey, const CommandResult& result);
    void setEnvironmentVariable(const std::string& name, const std::string& value);
    void setElevated(bool elevated);

    [[nodiscard]] const std::vector<std::string>& getExecutedCommands() const noexcept {
        return m_executedCommands;
    }

    [[nodiscard]] const std::vector<Security::PrivilegedOperation>& getExecutedPrivilegedOperations() const noexcept {
        return m_executedPrivilegedOps;
    }

    [[nodiscard]] const std::vector<std::string>& getDeletedPaths() const noexcept {
        return m_deletedPaths;
    }

    void clearLogs() {
        m_executedCommands.clear();
        m_executedPrivilegedOps.clear();
        m_deletedPaths.clear();
    }

private:
    struct MockFileEntry {
        std::string content;
        uint64_t size = 0;
        bool isDirectory = false;
        bool isSymlink = false;
        std::string symlinkTarget;
    };

    std::map<std::string, MockFileEntry> m_mockFiles;
    std::map<std::string, CommandResult> m_mockResponses;
    std::map<std::string, std::string> m_mockEnv;
    std::vector<std::string> m_executedCommands;
    std::vector<Security::PrivilegedOperation> m_executedPrivilegedOps;
    std::vector<std::string> m_deletedPaths;
    bool m_isElevated = false;
    std::string m_mockHome = "/home/testuser";
    std::string m_mockUser = "testuser";
};

} // namespace LinuxManager::Core

#include "MockSystemBackend.h"
#include "../security/PolicyManager.h"
#include "../security/OperationValidator.h"
#include <sstream>

namespace LinuxManager::Core {

MockSystemBackend::MockSystemBackend() {
    m_mockEnv["HOME"] = "/home/testuser";
    m_mockEnv["USER"] = "testuser";
}

CommandResult MockSystemBackend::executeCommand(
    const std::string& executable,
    const std::vector<std::string>& arguments,
    PrivilegeLevel privilege,
    [[maybe_unused]] std::chrono::milliseconds timeout
) {
    std::string fullCmd = executable;
    for (const auto& arg : arguments) {
        fullCmd += " " + arg;
    }
    if (privilege == PrivilegeLevel::RootRequired) {
        fullCmd = "/usr/bin/pkexec " + fullCmd;
    }
    m_executedCommands.push_back(fullCmd);

    // Exact match
    auto it = m_mockResponses.find(fullCmd);
    if (it != m_mockResponses.end()) {
        return it->second;
    }

    // Executable match
    it = m_mockResponses.find(executable);
    if (it != m_mockResponses.end()) {
        return it->second;
    }

    // Default mock response
    CommandResult defaultRes;
    defaultRes.exitCode = 0;
    defaultRes.stdOut = "";
    return defaultRes;
}

CommandResult MockSystemBackend::executePrivilegedOperation(
    const Security::PrivilegedOperation& operation
) {
    auto policy = std::make_shared<Security::PolicyManager>();
    Security::OperationValidator validator(policy);
    auto val = validator.validatePrivilegedOperation(operation);
    if (!val) {
        CommandResult err;
        err.exitCode = 1;
        err.stdErr = "Security Rejection: " + val.reason;
        return err;
    }

    m_executedPrivilegedOps.push_back(operation);

    CommandResult res;
    res.exitCode = 0;
    res.stdOut = "Mock operation executed successfully.";
    return res;
}

bool MockSystemBackend::removePath(const std::string& path, [[maybe_unused]] bool recursive) {
    m_deletedPaths.push_back(path);
    auto it = m_mockFiles.find(path);
    if (it != m_mockFiles.end()) {
        m_mockFiles.erase(it);
        return true;
    }
    return true;
}

bool MockSystemBackend::pathExists(const std::string& path) const {
    return m_mockFiles.find(path) != m_mockFiles.end();
}

uint64_t MockSystemBackend::getPathSize(const std::string& path) const {
    auto it = m_mockFiles.find(path);
    if (it != m_mockFiles.end()) {
        return it->second.size;
    }
    return 0;
}

std::string MockSystemBackend::readFileContents(const std::string& path) const {
    auto it = m_mockFiles.find(path);
    if (it != m_mockFiles.end()) {
        return it->second.content;
    }
    return "";
}

std::string MockSystemBackend::getEnvironmentVariable(const std::string& name) const {
    auto it = m_mockEnv.find(name);
    if (it != m_mockEnv.end()) {
        return it->second;
    }
    return "";
}

std::string MockSystemBackend::getUserHomeDirectory() const {
    return m_mockHome;
}

std::string MockSystemBackend::getCurrentUserName() const {
    return m_mockUser;
}

bool MockSystemBackend::isElevated() const {
    return m_isElevated;
}

void MockSystemBackend::setMockFile(const std::string& path, const std::string& content, uint64_t size) {
    MockFileEntry entry;
    entry.content = content;
    entry.size = (size > 0) ? size : content.size();
    entry.isDirectory = false;
    entry.isSymlink = false;
    m_mockFiles[path] = entry;
}

void MockSystemBackend::setMockDirectory(const std::string& path, uint64_t size) {
    MockFileEntry entry;
    entry.content = "";
    entry.size = size;
    entry.isDirectory = true;
    entry.isSymlink = false;
    m_mockFiles[path] = entry;
}

void MockSystemBackend::setMockSymlink(const std::string& linkPath, const std::string& targetPath) {
    MockFileEntry entry;
    entry.content = "";
    entry.size = 0;
    entry.isDirectory = false;
    entry.isSymlink = true;
    entry.symlinkTarget = targetPath;
    m_mockFiles[linkPath] = entry;
}

void MockSystemBackend::setMockCommandResponse(const std::string& commandKey, const CommandResult& result) {
    m_mockResponses[commandKey] = result;
}

void MockSystemBackend::setEnvironmentVariable(const std::string& name, const std::string& value) {
    m_mockEnv[name] = value;
}

void MockSystemBackend::setElevated(bool elevated) {
    m_isElevated = elevated;
}

} // namespace LinuxManager::Core

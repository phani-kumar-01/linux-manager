#pragma once

#include <string>
#include <vector>
#include <unordered_set>
#include <filesystem>

namespace LinuxManager::Security {

class PolicyManager {
public:
    PolicyManager();

    [[nodiscard]] bool isPathProtected(const std::string& path) const;
    [[nodiscard]] bool isPathInsideProtectedSubtree(const std::string& path) const;
    [[nodiscard]] bool isDeletionPermitted(const std::string& path, const std::string& userHome = "") const;
    [[nodiscard]] bool isPackageProtected(const std::string& packageName) const;
    [[nodiscard]] bool isServiceProtected(const std::string& serviceName) const;

    void addProtectedPath(const std::string& path);
    void addProtectedPackage(const std::string& packageName);
    void addProtectedService(const std::string& serviceName);

    [[nodiscard]] const std::vector<std::string>& getProtectedSubtrees() const noexcept {
        return m_protectedSubtrees;
    }

private:
    std::vector<std::string> m_protectedSubtrees;
    std::unordered_set<std::string> m_protectedPackages;
    std::unordered_set<std::string> m_protectedServices;
    std::vector<std::string> m_allowedDevArtifactNames;

    [[nodiscard]] bool isAllowedDevArtifact(const std::filesystem::path& path) const;
    [[nodiscard]] bool isAllowedUserCacheLocation(const std::filesystem::path& path, const std::string& userHome) const;
};

} // namespace LinuxManager::Security

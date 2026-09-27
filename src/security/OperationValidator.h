#pragma once

#include "PolicyManager.h"
#include "PrivilegedOperation.h"
#include <string>
#include <vector>
#include <memory>

namespace LinuxManager::Security {

struct ValidationResult {
    bool valid = false;
    std::string reason;
    std::vector<std::string> warnings;

    [[nodiscard]] explicit operator bool() const noexcept {
        return valid;
    }
};

class OperationValidator {
public:
    explicit OperationValidator(std::shared_ptr<PolicyManager> policyManager);

    [[nodiscard]] ValidationResult validateFileDeletion(const std::string& path, const std::string& userHome = "") const;
    [[nodiscard]] ValidationResult validatePackageName(const std::string& packageName) const;
    [[nodiscard]] ValidationResult validatePackageRemoval(const std::string& packageName) const;
    [[nodiscard]] ValidationResult validateServiceAction(PrivilegedOpType action, const std::string& serviceName) const;
    [[nodiscard]] ValidationResult validatePrivilegedOperation(const PrivilegedOperation& op) const;

    [[nodiscard]] std::shared_ptr<PolicyManager> getPolicyManager() const noexcept {
        return m_policy;
    }

private:
    std::shared_ptr<PolicyManager> m_policy;
};

} // namespace LinuxManager::Security

#include "OperationValidator.h"
#include <algorithm>
#include <regex>

namespace LinuxManager::Security {

OperationValidator::OperationValidator(std::shared_ptr<PolicyManager> policyManager)
    : m_policy(std::move(policyManager)) {
    if (!m_policy) {
        m_policy = std::make_shared<PolicyManager>();
    }
}

ValidationResult OperationValidator::validateFileDeletion(const std::string& path, const std::string& userHome) const {
    ValidationResult result;
    if (path.empty()) {
        result.valid = false;
        result.reason = "Path is empty.";
        return result;
    }

    if (m_policy->isPathInsideProtectedSubtree(path)) {
        result.valid = false;
        result.reason = "Path is inside a protected system subtree: " + path;
        return result;
    }

    if (!m_policy->isDeletionPermitted(path, userHome)) {
        result.valid = false;
        result.reason = "Deletion forbidden: path is not within an allowed cache location or project directory: " + path;
        return result;
    }

    result.valid = true;
    return result;
}

ValidationResult OperationValidator::validatePackageName(const std::string& packageName) const {
    ValidationResult result;
    if (packageName.empty()) {
        result.valid = false;
        result.reason = "Package name is empty.";
        return result;
    }

    // Validate package name format (Debian package naming rules: [a-z0-9][a-z0-9+.-]+)
    static const std::regex pkgRegex("^[a-z0-9][a-z0-9+\\.\\-]+$");
    if (!std::regex_match(packageName, pkgRegex)) {
        result.valid = false;
        result.reason = "Invalid Debian package name format: " + packageName;
        return result;
    }

    result.valid = true;
    return result;
}

ValidationResult OperationValidator::validatePackageRemoval(const std::string& packageName) const {
    ValidationResult nameVal = validatePackageName(packageName);
    if (!nameVal) {
        return nameVal;
    }

    if (m_policy->isPackageProtected(packageName)) {
        ValidationResult result;
        result.valid = false;
        result.reason = "Package is protected from removal by security policy: " + packageName;
        return result;
    }

    ValidationResult result;
    result.valid = true;
    return result;
}

ValidationResult OperationValidator::validateServiceAction(PrivilegedOpType action, const std::string& serviceName) const {
    ValidationResult result;
    if (serviceName.empty()) {
        result.valid = false;
        result.reason = "Service unit name is empty.";
        return result;
    }

    static const std::regex srvRegex("^[a-zA-Z0-9@_\\.\\-]+\\.service$");
    if (!std::regex_match(serviceName, srvRegex)) {
        result.valid = false;
        result.reason = "Invalid systemd service unit name format: " + serviceName;
        return result;
    }

    if ((action == PrivilegedOpType::ServiceStop || action == PrivilegedOpType::ServiceDisable) &&
        m_policy->isServiceProtected(serviceName)) {
        result.valid = false;
        result.reason = "Cannot stop or disable critical system service: " + serviceName;
        return result;
    }

    result.valid = true;
    return result;
}

ValidationResult OperationValidator::validatePrivilegedOperation(const PrivilegedOperation& op) const {
    ValidationResult result;

    switch (op.type) {
        case PrivilegedOpType::AptUpdate:
        case PrivilegedOpType::AptUpgrade:
        case PrivilegedOpType::AptAutoremove:
        case PrivilegedOpType::AptClean:
            result.valid = true;
            return result;

        case PrivilegedOpType::AptInstall: {
            if (op.packages.empty()) {
                result.valid = false;
                result.reason = "No packages specified for installation.";
                return result;
            }
            for (const auto& pkg : op.packages) {
                auto v = validatePackageName(pkg);
                if (!v) return v;
            }
            result.valid = true;
            return result;
        }

        case PrivilegedOpType::AptRemove:
        case PrivilegedOpType::AptPurge: {
            if (op.packages.empty()) {
                result.valid = false;
                result.reason = "No packages specified for removal.";
                return result;
            }
            for (const auto& pkg : op.packages) {
                auto v = validatePackageRemoval(pkg);
                if (!v) return v;
            }
            result.valid = true;
            return result;
        }

        case PrivilegedOpType::KernelPurge: {
            if (op.packages.empty()) {
                result.valid = false;
                result.reason = "No kernel package specified for purge.";
                return result;
            }
            for (const auto& pkg : op.packages) {
                if (!pkg.starts_with("linux-image-") && !pkg.starts_with("linux-headers-") && !pkg.starts_with("linux-modules-")) {
                    result.valid = false;
                    result.reason = "Invalid kernel package prefix: " + pkg;
                    return result;
                }
                auto v = validatePackageName(pkg);
                if (!v) return v;
            }
            result.valid = true;
            return result;
        }

        case PrivilegedOpType::JournalVacuumSize: {
            if (op.targetSizeMb < 50) {
                result.valid = false;
                result.reason = "Journal vacuum target size must be at least 50 MB for system safety.";
                return result;
            }
            result.valid = true;
            return result;
        }

        case PrivilegedOpType::JournalVacuumTime: {
            if (op.retentionDays < 1) {
                result.valid = false;
                result.reason = "Journal vacuum retention days must be at least 1 day.";
                return result;
            }
            result.valid = true;
            return result;
        }

        case PrivilegedOpType::ServiceRestart:
        case PrivilegedOpType::ServiceStart:
        case PrivilegedOpType::ServiceStop:
        case PrivilegedOpType::ServiceEnable:
        case PrivilegedOpType::ServiceDisable: {
            return validateServiceAction(op.type, op.serviceUnit);
        }
    }

    result.valid = false;
    result.reason = "Unknown or unapproved privileged operation type.";
    return result;
}

} // namespace LinuxManager::Security

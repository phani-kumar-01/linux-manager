#pragma once

#include "../core/ISystemBackend.h"
#include <memory>
#include <vector>
#include <string>

namespace LinuxManager::System {

struct KernelInfo {
    std::string version;
    std::string packageName;
    bool isRunning = false;
    bool isRemovable = false;
    uint64_t sizeBytes = 0;
};

class KernelManager {
public:
    explicit KernelManager(std::shared_ptr<Core::ISystemBackend> backend);

    [[nodiscard]] std::string getRunningKernel();
    [[nodiscard]] std::vector<KernelInfo> getInstalledKernels();
    [[nodiscard]] std::vector<KernelInfo> getOldUnusedKernels();

    [[nodiscard]] Core::CommandResult removeKernel(const std::string& packageName);

private:
    std::shared_ptr<Core::ISystemBackend> m_backend;
};

} // namespace LinuxManager::System

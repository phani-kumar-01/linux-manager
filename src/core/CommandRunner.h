#pragma once

#include "ISystemBackend.h"
#include <string>
#include <vector>
#include <chrono>

namespace LinuxManager::Core {

class CommandRunner {
public:
    static CommandResult run(
        const std::string& executable,
        const std::vector<std::string>& arguments,
        std::chrono::milliseconds timeout = std::chrono::milliseconds(30000),
        const std::string& workingDir = ""
    );

    static bool isExecutableInPath(const std::string& executableName);
};

} // namespace LinuxManager::Core

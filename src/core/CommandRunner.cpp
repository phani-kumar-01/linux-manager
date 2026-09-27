#include "CommandRunner.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <poll.h>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <array>
#include <sstream>
#include <iostream>
#include <filesystem>

namespace LinuxManager::Core {

CommandResult CommandRunner::run(
    const std::string& executable,
    const std::vector<std::string>& arguments,
    std::chrono::milliseconds timeout,
    const std::string& workingDir
) {
    CommandResult result;
    auto startTime = std::chrono::steady_clock::now();

    int stdoutPipe[2];
    int stderrPipe[2];

    if (pipe(stdoutPipe) != 0 || pipe(stderrPipe) != 0) {
        result.exitCode = -1;
        result.stdErr = "Failed to create pipes for command execution.";
        return result;
    }

    pid_t pid = fork();
    if (pid < 0) {
        close(stdoutPipe[0]); close(stdoutPipe[1]);
        close(stderrPipe[0]); close(stderrPipe[1]);
        result.exitCode = -1;
        result.stdErr = "Failed to fork child process.";
        return result;
    }

    if (pid == 0) {
        // Child Process
        close(stdoutPipe[0]);
        close(stderrPipe[0]);

        dup2(stdoutPipe[1], STDOUT_FILENO);
        dup2(stderrPipe[1], STDERR_FILENO);

        close(stdoutPipe[1]);
        close(stderrPipe[1]);

        if (!workingDir.empty()) {
            if (chdir(workingDir.c_str()) != 0) {
                // Warning if change directory fails
            }
        }

        // Prepare arguments for execvp
        std::vector<char*> argv;
        argv.push_back(const_cast<char*>(executable.c_str()));
        for (const auto& arg : arguments) {
            argv.push_back(const_cast<char*>(arg.c_str()));
        }
        argv.push_back(nullptr);

        execvp(executable.c_str(), argv.data());
        // If execvp fails:
        std::cerr << "CommandRunner: Failed to exec " << executable << ": " << strerror(errno) << std::endl;
        _exit(127);
    }

    // Parent Process
    close(stdoutPipe[1]);
    close(stderrPipe[1]);

    // Set non-blocking on read ends
    fcntl(stdoutPipe[0], F_SETFL, O_NONBLOCK);
    fcntl(stderrPipe[0], F_SETFL, O_NONBLOCK);

    std::array<pollfd, 2> fds{};
    fds[0].fd = stdoutPipe[0];
    fds[0].events = POLLIN | POLLHUP;
    fds[1].fd = stderrPipe[0];
    fds[1].events = POLLIN | POLLHUP;

    bool stdoutOpen = true;
    bool stderrOpen = true;
    char buffer[4096];

    while (stdoutOpen || stderrOpen) {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - startTime
        );

        if (timeout.count() > 0 && elapsed > timeout) {
            result.timedOut = true;
            kill(pid, SIGKILL);
            waitpid(pid, nullptr, 0);
            break;
        }

        int remainingTimeout = timeout.count() > 0 ? static_cast<int>(timeout.count() - elapsed.count()) : 1000;
        if (remainingTimeout < 0) remainingTimeout = 10;

        int ret = poll(fds.data(), 2, std::min(remainingTimeout, 200));
        if (ret > 0) {
            if (stdoutOpen && (fds[0].revents & POLLIN)) {
                ssize_t bytesRead = read(stdoutPipe[0], buffer, sizeof(buffer) - 1);
                if (bytesRead > 0) {
                    buffer[bytesRead] = '\0';
                    result.stdOut.append(buffer, bytesRead);
                } else if (bytesRead == 0) {
                    stdoutOpen = false;
                }
            }

            if (stderrOpen && (fds[1].revents & POLLIN)) {
                ssize_t bytesRead = read(stderrPipe[0], buffer, sizeof(buffer) - 1);
                if (bytesRead > 0) {
                    buffer[bytesRead] = '\0';
                    result.stdErr.append(buffer, bytesRead);
                } else if (bytesRead == 0) {
                    stderrOpen = false;
                }
            }

            if (fds[0].revents & (POLLHUP | POLLERR | POLLNVAL)) {
                // Read remaining bytes
                ssize_t bytesRead = 0;
                while ((bytesRead = read(stdoutPipe[0], buffer, sizeof(buffer) - 1)) > 0) {
                    result.stdOut.append(buffer, bytesRead);
                }
                stdoutOpen = false;
            }

            if (fds[1].revents & (POLLHUP | POLLERR | POLLNVAL)) {
                ssize_t bytesRead = 0;
                while ((bytesRead = read(stderrPipe[0], buffer, sizeof(buffer) - 1)) > 0) {
                    result.stdErr.append(buffer, bytesRead);
                }
                stderrOpen = false;
            }
        } else if (ret == 0) {
            // Check if process finished
            int status;
            pid_t w = waitpid(pid, &status, WNOHANG);
            if (w == pid) {
                // Process terminated, drain pipes
                ssize_t bytesRead = 0;
                while ((bytesRead = read(stdoutPipe[0], buffer, sizeof(buffer) - 1)) > 0) {
                    result.stdOut.append(buffer, bytesRead);
                }
                while ((bytesRead = read(stderrPipe[0], buffer, sizeof(buffer) - 1)) > 0) {
                    result.stdErr.append(buffer, bytesRead);
                }
                if (WIFEXITED(status)) {
                    result.exitCode = WEXITSTATUS(status);
                } else if (WIFSIGNALED(status)) {
                    result.exitCode = 128 + WTERMSIG(status);
                }
                break;
            }
        }
    }

    close(stdoutPipe[0]);
    close(stderrPipe[0]);

    if (!result.timedOut) {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            result.exitCode = WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
            result.exitCode = 128 + WTERMSIG(status);
        }
    }

    result.executionTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - startTime
    );

    return result;
}

bool CommandRunner::isExecutableInPath(const std::string& executableName) {
    const char* pathEnv = std::getenv("PATH");
    if (!pathEnv) return false;

    std::istringstream stream(pathEnv);
    std::string dir;
    while (std::getline(stream, dir, ':')) {
        if (dir.empty()) dir = ".";
        std::filesystem::path fullPath = std::filesystem::path(dir) / executableName;
        std::error_code ec;
        if (std::filesystem::exists(fullPath, ec) && !std::filesystem::is_directory(fullPath, ec)) {
            if (access(fullPath.c_str(), X_OK) == 0) {
                return true;
            }
        }
    }
    return false;
}

} // namespace LinuxManager::Core

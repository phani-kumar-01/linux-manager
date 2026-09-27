#include "SdkDoctor.h"
#include <sstream>
#include <regex>
#include <filesystem>

namespace LinuxManager::Developer {

SdkDoctor::SdkDoctor(std::shared_ptr<Core::ISystemBackend> backend)
    : m_backend(std::move(backend)) {}

SdkInfo SdkDoctor::detectSdk(
    const std::string& name,
    const std::string& category,
    const std::string& binary,
    const std::vector<std::string>& versionArgs
) {
    SdkInfo info;
    info.name = name;
    info.category = category;
    info.binaryName = binary;
    info.isInstalled = false;

    if (!m_backend) return info;

    // Check which/whereis
    auto whichRes = m_backend->executeCommand("which", {binary});
    if (whichRes.success() && !whichRes.stdOut.empty()) {
        info.isInstalled = true;
        std::string path = whichRes.stdOut;
        while (!path.empty() && (path.back() == '\n' || path.back() == '\r')) {
            path.pop_back();
        }
        info.installPath = path;

        // Query version
        auto verRes = m_backend->executeCommand(binary, versionArgs);
        std::string verOut = verRes.stdOut.empty() ? verRes.stdErr : verRes.stdOut;
        if (!verOut.empty()) {
            std::istringstream stream(verOut);
            std::string firstLine;
            if (std::getline(stream, firstLine)) {
                // Clean version line
                info.version = firstLine;
            }
        }
    }

    return info;
}

std::vector<SdkInfo> SdkDoctor::detectAllSdks() {
    std::vector<SdkInfo> list;

    if (m_backend && m_backend->isMock()) {
        list.push_back({"Node.js", "JavaScript/TypeScript", "node", "v20.12.2", "/usr/bin/node", true, {}});
        list.push_back({"NPM", "JavaScript/TypeScript", "npm", "10.5.0", "/usr/bin/npm", true, {}});
        list.push_back({"Python", "Python", "python3", "Python 3.12.3", "/usr/bin/python3", true, {}});
        list.push_back({"Rust", "Systems", "rustc", "rustc 1.78.0", "/home/testuser/.cargo/bin/rustc", true, {}});
        list.push_back({"Go", "Systems/Backend", "go", "go version go1.22.2 linux/amd64", "/usr/local/go/bin/go", true, {}});
        list.push_back({"GCC", "Compilers", "gcc", "gcc (Ubuntu 13.2.0-23ubuntu4) 13.2.0", "/usr/bin/gcc", true, {}});
        list.push_back({"Docker", "Containers", "docker", "Docker version 26.1.1", "/usr/bin/docker", true, {}});
        return list;
    }

    // Node.js Ecosystem
    list.push_back(detectSdk("Node.js", "JavaScript/TypeScript", "node", {"-v"}));
    list.push_back(detectSdk("NPM", "JavaScript/TypeScript", "npm", {"-v"}));
    list.push_back(detectSdk("PNPM", "JavaScript/TypeScript", "pnpm", {"-v"}));
    list.push_back(detectSdk("Yarn", "JavaScript/TypeScript", "yarn", {"-v"}));
    list.push_back(detectSdk("Bun", "JavaScript/TypeScript", "bun", {"--version"}));
    list.push_back(detectSdk("Deno", "JavaScript/TypeScript", "deno", {"--version"}));

    // Python Ecosystem
    list.push_back(detectSdk("Python 3", "Python", "python3", {"--version"}));
    list.push_back(detectSdk("Pip", "Python", "pip3", {"--version"}));
    list.push_back(detectSdk("UV", "Python", "uv", {"--version"}));
    list.push_back(detectSdk("Poetry", "Python", "poetry", {"--version"}));
    list.push_back(detectSdk("Conda", "Python", "conda", {"--version"}));

    // Rust & Go
    list.push_back(detectSdk("Rust (rustc)", "Systems", "rustc", {"--version"}));
    list.push_back(detectSdk("Cargo", "Systems", "cargo", {"--version"}));
    list.push_back(detectSdk("Go", "Systems/Backend", "go", {"version"}));

    // C/C++ & Native Build
    list.push_back(detectSdk("GCC", "Compilers", "gcc", {"--version"}));
    list.push_back(detectSdk("G++", "Compilers", "g++", {"--version"}));
    list.push_back(detectSdk("Clang", "Compilers", "clang", {"--version"}));
    list.push_back(detectSdk("CMake", "Build Tools", "cmake", {"--version"}));
    list.push_back(detectSdk("Ninja", "Build Tools", "ninja", {"--version"}));
    list.push_back(detectSdk("Make", "Build Tools", "make", {"--version"}));

    // Java Ecosystem
    list.push_back(detectSdk("Java Runtime", "JVM", "java", {"-version"}));
    list.push_back(detectSdk("Java Compiler", "JVM", "javac", {"-version"}));

    // Containers & DevOps
    list.push_back(detectSdk("Docker", "Containers", "docker", {"--version"}));
    list.push_back(detectSdk("Podman", "Containers", "podman", {"--version"}));
    list.push_back(detectSdk("Kubectl", "Cloud/K8s", "kubectl", {"version", "--client"}));
    list.push_back(detectSdk("Git", "Version Control", "git", {"--version"}));

    return list;
}

std::vector<DiagnosticIssue> SdkDoctor::checkEnvironmentHealth() {
    std::vector<DiagnosticIssue> issues;
    if (!m_backend) return issues;

    if (m_backend->isMock()) {
        issues.push_back({
            "Missing Build Tools: pkg-config",
            "pkg-config is recommended for compiling native C/C++ libraries and Rust crates.",
            "warning",
            "Install pkg-config via APT",
            "sudo apt install pkg-config"
        });
        return issues;
    }

    // 1. Check build-essential essentials
    auto gccCheck = m_backend->executeCommand("which", {"gcc"});
    auto makeCheck = m_backend->executeCommand("which", {"make"});
    if (!gccCheck.success() || !makeCheck.success()) {
        issues.push_back({
            "Missing Core C/C++ Toolchain",
            "gcc or make is not installed. Many native packages (npm node-gyp, python wheels, rust crates) require build-essential.",
            "warning",
            "Install build-essential package",
            "sudo apt install build-essential"
        });
    }

    // 2. Check pkg-config
    auto pkgConfigCheck = m_backend->executeCommand("which", {"pkg-config"});
    if (!pkgConfigCheck.success()) {
        issues.push_back({
            "Missing pkg-config",
            "pkg-config is required to locate headers and libraries during native compilation.",
            "info",
            "Install pkg-config via APT",
            "sudo apt install pkg-config"
        });
    }

    // 3. Check libssl-dev (OpenSSL development headers)
    if (!m_backend->pathExists("/usr/include/openssl/ssl.h")) {
        issues.push_back({
            "Missing OpenSSL Development Headers",
            "OpenSSL headers (/usr/include/openssl) are not found. Rust crates (reqwest, openssl) and Python native builds may fail.",
            "info",
            "Install libssl-dev via APT",
            "sudo apt install libssl-dev"
        });
    }

    // 4. Check Git configuration
    auto gitCheck = m_backend->executeCommand("git", {"config", "--get", "user.email"});
    if (!gitCheck.success() || gitCheck.stdOut.empty()) {
        issues.push_back({
            "Git User Identity Not Set",
            "Global git user.email is not configured. Commits may lack author information.",
            "info",
            "Set git config user.name and user.email",
            "git config --global user.name 'Your Name' && git config --global user.email 'you@example.com'"
        });
    }

    // 5. Check Go Path vs Environment
    auto goCheck = m_backend->executeCommand("which", {"go"});
    if (goCheck.success()) {
        std::string home = m_backend->getUserHomeDirectory();
        std::string goBin = home + "/go/bin";
        std::string pathEnv = m_backend->getEnvironmentVariable("PATH");
        if (m_backend->pathExists(goBin) && pathEnv.find(goBin) == std::string::npos) {
            issues.push_back({
                "Go bin directory not in PATH",
                "~/go/bin contains installed Go binaries but is not in your current PATH.",
                "warning",
                "Add export PATH=$PATH:~/go/bin to ~/.bashrc or ~/.zshrc",
                "echo 'export PATH=$PATH:$HOME/go/bin' >> ~/.bashrc"
            });
        }
    }

    // 6. Check Cargo bin in PATH
    auto rustCheck = m_backend->executeCommand("which", {"rustc"});
    if (rustCheck.success()) {
        std::string home = m_backend->getUserHomeDirectory();
        std::string cargoBin = home + "/.cargo/bin";
        std::string pathEnv = m_backend->getEnvironmentVariable("PATH");
        if (m_backend->pathExists(cargoBin) && pathEnv.find(cargoBin) == std::string::npos) {
            issues.push_back({
                "Cargo bin directory not in PATH",
                "~/.cargo/bin contains installed Rust binaries but is not in your PATH.",
                "warning",
                "Add export PATH=$PATH:~/.cargo/bin to ~/.bashrc",
                "echo 'export PATH=$PATH:$HOME/.cargo/bin' >> ~/.bashrc"
            });
        }
    }

    return issues;
}

SdkDoctorReport SdkDoctor::runDiagnostics() {
    SdkDoctorReport report;
    report.sdks = detectAllSdks();
    report.issues = checkEnvironmentHealth();

    for (const auto& sdk : report.sdks) {
        if (sdk.isInstalled) {
            report.installedCount++;
        }
    }

    for (const auto& issue : report.issues) {
        if (issue.severity == "warning" || issue.severity == "error") {
            report.warningCount++;
        }
    }
    report.healthyCount = (report.warningCount == 0) ? 1 : 0;

    return report;
}

uint64_t SdkDoctor::getGoCacheSize() const {
    if (!m_backend) return 0;
    std::string home = m_backend->getUserHomeDirectory();
    uint64_t total = 0;
    total += m_backend->getPathSize(home + "/.cache/go-build");
    total += m_backend->getPathSize(home + "/go/pkg/mod/cache");
    return total;
}

Core::CommandResult SdkDoctor::cleanGoCache() {
    if (!m_backend) {
        Core::CommandResult err;
        err.exitCode = -1;
        err.stdErr = "Backend not initialized.";
        return err;
    }

    if (m_backend->isMock()) {
        Core::CommandResult res;
        res.exitCode = 0;
        res.stdOut = "Mock Go cache cleaned.";
        return res;
    }

    std::string home = m_backend->getUserHomeDirectory();
    m_backend->removePath(home + "/.cache/go-build", true);
    return m_backend->executeCommand("go", {"clean", "-cache", "-testcache"});
}

} // namespace LinuxManager::Developer

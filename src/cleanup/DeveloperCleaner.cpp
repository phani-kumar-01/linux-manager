#include "DeveloperCleaner.h"
#include <filesystem>
#include <system_error>

namespace LinuxManager::Cleanup {

DeveloperCleaner::DeveloperCleaner(std::shared_ptr<Core::ISystemBackend> backend)
    : m_backend(std::move(backend)) {}

std::vector<CleanupItem> DeveloperCleaner::scanAllDeveloperCaches() {
    std::vector<CleanupItem> items;
    if (!m_backend) return items;

    items.push_back(scanNodeGlobalCache());
    items.push_back(scanPythonGlobalCache());
    items.push_back(scanJavaGlobalCache());
    items.push_back(scanRustGlobalCache());
    items.push_back(scanAndroidGlobalCache());

    return items;
}

CleanupItem DeveloperCleaner::scanNodeGlobalCache() {
    CleanupItem item;
    item.id = "dev-node-cache";
    item.title = "Node.js & npm Global Cache";
    item.category = CleanupCategory::NodeDeveloper;
    item.risk = RiskLevel::Safe;
    item.requiresElevation = false;
    item.description = "Global npm and yarn package caches (~/.npm, ~/.yarn/berry/cache).";

    if (m_backend) {
        std::string home = m_backend->getUserHomeDirectory();
        std::vector<std::string> paths = {
            home + "/.npm",
            home + "/.npm/_cacache",
            home + "/.yarn/berry/cache",
            home + "/.pnpm-store"
        };
        for (const auto& p : paths) {
            if (m_backend->pathExists(p)) {
                uint64_t sz = m_backend->getPathSize(p);
                item.sizeBytes += sz;
                item.subItems.push_back(p);
            }
        }
        item.path = home + "/.npm";
    }
    return item;
}

CleanupItem DeveloperCleaner::scanPythonGlobalCache() {
    CleanupItem item;
    item.id = "dev-python-cache";
    item.title = "Python pip & uv Cache";
    item.category = CleanupCategory::PythonDeveloper;
    item.risk = RiskLevel::Safe;
    item.requiresElevation = false;
    item.description = "Downloaded wheel/source caches from pip, uv, and poetry.";

    if (m_backend) {
        std::string home = m_backend->getUserHomeDirectory();
        std::vector<std::string> paths = {
            home + "/.cache/pip",
            home + "/.cache/uv",
            home + "/.cache/pypoetry/cache"
        };
        for (const auto& p : paths) {
            if (m_backend->pathExists(p)) {
                uint64_t sz = m_backend->getPathSize(p);
                item.sizeBytes += sz;
                item.subItems.push_back(p);
            }
        }
        item.path = home + "/.cache/pip";
    }
    return item;
}

CleanupItem DeveloperCleaner::scanJavaGlobalCache() {
    CleanupItem item;
    item.id = "dev-java-cache";
    item.title = "Java Maven & Gradle Caches";
    item.category = CleanupCategory::JavaDeveloper;
    item.risk = RiskLevel::Safe;
    item.requiresElevation = false;
    item.description = "Downloaded JARs and build caches from ~/.m2 and ~/.gradle/caches.";

    if (m_backend) {
        std::string home = m_backend->getUserHomeDirectory();
        std::vector<std::string> paths = {
            home + "/.m2/repository",
            home + "/.gradle/caches",
            home + "/.gradle/daemon"
        };
        for (const auto& p : paths) {
            if (m_backend->pathExists(p)) {
                uint64_t sz = m_backend->getPathSize(p);
                item.sizeBytes += sz;
                item.subItems.push_back(p);
            }
        }
        item.path = home + "/.gradle/caches";
    }
    return item;
}

CleanupItem DeveloperCleaner::scanRustGlobalCache() {
    CleanupItem item;
    item.id = "dev-rust-cache";
    item.title = "Rust Cargo Registry Cache";
    item.category = CleanupCategory::RustDeveloper;
    item.risk = RiskLevel::Safe;
    item.requiresElevation = false;
    item.description = "Crates.io package tarballs and git checkout caches in ~/.cargo.";

    if (m_backend) {
        std::string home = m_backend->getUserHomeDirectory();
        std::vector<std::string> paths = {
            home + "/.cargo/registry/cache",
            home + "/.cargo/git/db",
            home + "/.cargo/git/checkouts"
        };
        for (const auto& p : paths) {
            if (m_backend->pathExists(p)) {
                uint64_t sz = m_backend->getPathSize(p);
                item.sizeBytes += sz;
                item.subItems.push_back(p);
            }
        }
        item.path = home + "/.cargo/registry/cache";
    }
    return item;
}

CleanupItem DeveloperCleaner::scanAndroidGlobalCache() {
    CleanupItem item;
    item.id = "dev-android-cache";
    item.title = "Android Build & Emulator Caches";
    item.category = CleanupCategory::AndroidDeveloper;
    item.risk = RiskLevel::Moderate;
    item.requiresElevation = false;
    item.description = "Android SDK build tools caches and AVD snapshots.";

    if (m_backend) {
        std::string home = m_backend->getUserHomeDirectory();
        std::vector<std::string> paths = {
            home + "/.android/build-cache",
            home + "/.android/cache",
            home + "/Android/Sdk/.temp"
        };
        for (const auto& p : paths) {
            if (m_backend->pathExists(p)) {
                uint64_t sz = m_backend->getPathSize(p);
                item.sizeBytes += sz;
                item.subItems.push_back(p);
            }
        }
        item.path = home + "/.android/cache";
    }
    return item;
}

std::vector<DeveloperProjectItem> DeveloperCleaner::scanProjectDirectories(
    const std::vector<std::string>& searchRoots,
    int maxDepth
) {
    std::vector<DeveloperProjectItem> results;
    if (!m_backend) return results;

    for (const auto& root : searchRoots) {
        if (m_backend->pathExists(root)) {
            scanDirectoryRecursive(root, 0, maxDepth, results);
        }
    }

    return results;
}

void DeveloperCleaner::scanDirectoryRecursive(
    const std::string& currentPath,
    int currentDepth,
    int maxDepth,
    std::vector<DeveloperProjectItem>& results
) {
    if (currentDepth >= maxDepth) return;
    if (m_backend && m_backend->isMock()) return; // Mock handles via custom mock directories

    std::error_code ec;
    std::filesystem::path p(currentPath);
    if (!std::filesystem::exists(p, ec) || !std::filesystem::is_directory(p, ec)) return;

    for (const auto& entry : std::filesystem::directory_iterator(
             p, std::filesystem::directory_options::skip_permission_denied, ec)) {
        if (ec) break;

        std::string filename = entry.path().filename().string();
        if (entry.is_directory(ec)) {
            if (filename == "node_modules" || filename == "target" ||
                filename == "__pycache__" || filename == ".pytest_cache" ||
                filename == ".mypy_cache" || filename == ".venv" || filename == "venv") {
                
                DeveloperProjectItem item;
                item.projectPath = entry.path().string();
                item.artifactType = filename;
                if (m_backend) {
                    item.sizeBytes = m_backend->getPathSize(entry.path().string());
                }
                results.push_back(item);
                // Do not recurse into node_modules or target
                continue;
            } else if (filename.starts_with(".") && filename != ".git") {
                // Skip hidden metadata dirs
                continue;
            } else {
                scanDirectoryRecursive(entry.path().string(), currentDepth + 1, maxDepth, results);
            }
        }
    }
}

} // namespace LinuxManager::Cleanup

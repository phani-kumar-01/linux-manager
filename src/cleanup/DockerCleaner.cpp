#include "DockerCleaner.h"
#include <regex>
#include <sstream>
#include <cmath>

namespace LinuxManager::Cleanup {

DockerCleaner::DockerCleaner(std::shared_ptr<Core::ISystemBackend> backend)
    : m_backend(std::move(backend)) {}

bool DockerCleaner::isDockerAvailable() {
    if (!m_backend) return false;
    auto res = m_backend->executeCommand("docker", {"info"});
    return res.success();
}

uint64_t DockerCleaner::parseSizeString(const std::string& sizeStr) {
    static const std::regex sizeRegex(R"(([0-9.]+)\s*([kKmMgGtTpP]?)[bB]?)");
    std::smatch match;
    if (std::regex_search(sizeStr, match, sizeRegex)) {
        double val = std::stod(match[1].str());
        std::string unit = match[2].str();
        uint64_t multiplier = 1;
        if (unit == "k" || unit == "K") multiplier = 1024ULL;
        else if (unit == "m" || unit == "M") multiplier = 1024ULL * 1024ULL;
        else if (unit == "g" || unit == "G") multiplier = 1024ULL * 1024ULL * 1024ULL;
        else if (unit == "t" || unit == "T") multiplier = 1024ULL * 1024ULL * 1024ULL * 1024ULL;
        return static_cast<uint64_t>(val * multiplier);
    }
    return 0;
}

DockerUsage DockerCleaner::getDockerUsage() {
    DockerUsage usage;
    if (!m_backend || !isDockerAvailable()) return usage;

    auto res = m_backend->executeCommand("docker", {"system", "df"});
    if (!res.success()) return usage;

    usage.isAvailable = true;
    std::istringstream stream(res.stdOut);
    std::string line;

    // TYPE TOTAL ACTIVE SIZE RECLAIMABLE
    // Images 15 4 12.4GB 8.2GB (66%)
    // Containers 5 1 1.2GB 900MB (75%)
    // Local Volumes 8 2 8.6GB 6.1GB (70%)
    // Build Cache 32 0 4.2GB 4.2GB (100%)

    while (std::getline(stream, line)) {
        if (line.starts_with("Images")) {
            std::istringstream l(line);
            std::string type, total, active, size, sizeUnit, rec, recUnit;
            l >> type >> total >> active >> size;
            usage.imagesSize = parseSizeString(size);
            if (l >> rec) usage.reclaimableImages = parseSizeString(rec);
        } else if (line.starts_with("Containers")) {
            std::istringstream l(line);
            std::string type, total, active, size, rec;
            l >> type >> total >> active >> size;
            usage.containersSize = parseSizeString(size);
            if (l >> rec) usage.reclaimableContainers = parseSizeString(rec);
        } else if (line.starts_with("Local Volumes")) {
            std::istringstream l(line);
            std::string type1, type2, total, active, size, rec;
            l >> type1 >> type2 >> total >> active >> size;
            usage.volumesSize = parseSizeString(size);
            if (l >> rec) usage.reclaimableVolumes = parseSizeString(rec);
        } else if (line.starts_with("Build Cache")) {
            std::istringstream l(line);
            std::string type1, type2, total, active, size, rec;
            l >> type1 >> type2 >> total >> active >> size;
            usage.buildCacheSize = parseSizeString(size);
            if (l >> rec) usage.reclaimableBuildCache = parseSizeString(rec);
        }
    }

    usage.totalReclaimable = usage.reclaimableImages + usage.reclaimableContainers +
                             usage.reclaimableVolumes + usage.reclaimableBuildCache;

    return usage;
}

std::vector<CleanupItem> DockerCleaner::getDockerCleanupItems() {
    std::vector<CleanupItem> items;
    DockerUsage usage = getDockerUsage();
    if (!usage.isAvailable) return items;

    if (usage.reclaimableContainers > 0) {
        CleanupItem item;
        item.id = "docker-containers";
        item.title = "Docker Stopped Containers";
        item.description = "Reclaim space from stopped Docker containers.";
        item.category = CleanupCategory::DockerResources;
        item.risk = RiskLevel::Safe;
        item.sizeBytes = usage.reclaimableContainers;
        items.push_back(item);
    }

    if (usage.reclaimableBuildCache > 0) {
        CleanupItem item;
        item.id = "docker-build-cache";
        item.title = "Docker Build Cache";
        item.description = "Cached intermediate build layers from docker build / buildkit.";
        item.category = CleanupCategory::DockerResources;
        item.risk = RiskLevel::Safe;
        item.sizeBytes = usage.reclaimableBuildCache;
        items.push_back(item);
    }

    if (usage.reclaimableImages > 0) {
        CleanupItem item;
        item.id = "docker-images";
        item.title = "Docker Dangling & Unused Images";
        item.description = "Unreferenced Docker image layers.";
        item.category = CleanupCategory::DockerResources;
        item.risk = RiskLevel::Moderate;
        item.sizeBytes = usage.reclaimableImages;
        items.push_back(item);
    }

    if (usage.reclaimableVolumes > 0) {
        CleanupItem item;
        item.id = "docker-volumes";
        item.title = "Docker Unused Volumes";
        item.description = "Unattached Docker volumes. May contain persistent database files!";
        item.category = CleanupCategory::DockerResources;
        item.risk = RiskLevel::Caution;
        item.isSelected = false; // Never select volumes by default!
        item.sizeBytes = usage.reclaimableVolumes;
        items.push_back(item);
    }

    return items;
}

Core::CommandResult DockerCleaner::pruneContainers() {
    if (!m_backend) return {};
    return m_backend->executeCommand("docker", {"container", "prune", "-f"});
}

Core::CommandResult DockerCleaner::pruneImages() {
    if (!m_backend) return {};
    return m_backend->executeCommand("docker", {"image", "prune", "-f"});
}

Core::CommandResult DockerCleaner::pruneBuildCache() {
    if (!m_backend) return {};
    return m_backend->executeCommand("docker", {"builder", "prune", "-f"});
}

Core::CommandResult DockerCleaner::pruneSystem(bool pruneVolumes) {
    if (!m_backend) return {};
    std::vector<std::string> args = {"system", "prune", "-f"};
    if (pruneVolumes) {
        args.push_back("--volumes");
    }
    return m_backend->executeCommand("docker", args);
}

} // namespace LinuxManager::Cleanup

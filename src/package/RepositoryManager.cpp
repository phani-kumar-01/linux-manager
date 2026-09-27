#include "RepositoryManager.h"
#include <fstream>
#include <sstream>
#include <regex>
#include <filesystem>
#include <set>

namespace LinuxManager::Package {

RepositoryManager::RepositoryManager(std::shared_ptr<Core::ISystemBackend> backend)
    : m_backend(std::move(backend)) {}

void RepositoryManager::parseClassicListFile(const std::string& filePath, std::vector<AptRepository>& list) {
    if (!m_backend) return;
    std::string content = m_backend->readFileContents(filePath);
    if (content.empty()) return;

    std::istringstream stream(content);
    std::string line;
    int lineNum = 0;

    // Pattern: [opt #] deb [options] uri suite comp1 comp2...
    static const std::regex repoRegex(R"(^(#\s*)?(deb(?:-src)?)\s+(?:\[[^\]]+\]\s+)?([^\s]+)\s+([^\s]+)(?:\s+(.*))?$)");

    while (std::getline(stream, line)) {
        lineNum++;
        if (!line.empty() && line.back() == '\r') line.pop_back();

        std::string trimmed = line;
        while (!trimmed.empty() && (trimmed.front() == ' ' || trimmed.front() == '\t')) trimmed.erase(0, 1);
        if (trimmed.empty()) continue;

        std::smatch match;
        if (std::regex_match(trimmed, match, repoRegex)) {
            AptRepository repo;
            repo.isEnabled = match[1].str().empty(); // If no leading #, it's enabled
            repo.type = match[2].str();
            repo.uri = match[3].str();
            repo.suite = match[4].str();
            repo.components = match[5].str();
            repo.sourceFile = filePath;
            repo.lineNumber = lineNum;
            repo.isDeb822 = false;

            repo.id = repo.type + ":" + repo.uri + ":" + repo.suite;

            // Check if PPA or 3rd party
            if (repo.uri.find("ppa.launchpad") != std::string::npos || repo.uri.find("launchpad.net") != std::string::npos) {
                repo.isThirdParty = true;
                // Extract PPA name
                static const std::regex ppaRegex(R"(launchpad(?:content)?\.net/([^/]+)/([^/]+))");
                std::smatch ppaMatch;
                if (std::regex_search(repo.uri, ppaMatch, ppaRegex)) {
                    repo.ppaName = "ppa:" + ppaMatch[1].str() + "/" + ppaMatch[2].str();
                } else {
                    repo.ppaName = repo.uri;
                }
            } else if (repo.uri.find("ubuntu.com") == std::string::npos &&
                       repo.uri.find("debian.org") == std::string::npos &&
                       repo.uri.find("system76.com") == std::string::npos) {
                repo.isThirdParty = true;
            }

            list.push_back(repo);
        }
    }
}

void RepositoryManager::parseDeb822SourcesFile(const std::string& filePath, std::vector<AptRepository>& list) {
    if (!m_backend) return;
    std::string content = m_backend->readFileContents(filePath);
    if (content.empty()) return;

    std::istringstream stream(content);
    std::string line;

    AptRepository currentRepo;
    currentRepo.sourceFile = filePath;
    currentRepo.isDeb822 = true;
    currentRepo.isEnabled = true;

    bool inBlock = false;

    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty() || line[0] == '#') {
            if (inBlock && !currentRepo.uri.empty()) {
                currentRepo.id = currentRepo.type + ":" + currentRepo.uri + ":" + currentRepo.suite;
                if (currentRepo.uri.find("ubuntu.com") == std::string::npos &&
                    currentRepo.uri.find("debian.org") == std::string::npos) {
                    currentRepo.isThirdParty = true;
                }
                list.push_back(currentRepo);
                currentRepo = AptRepository{};
                currentRepo.sourceFile = filePath;
                currentRepo.isDeb822 = true;
                currentRepo.isEnabled = true;
                inBlock = false;
            }
            continue;
        }

        inBlock = true;
        auto colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string key = line.substr(0, colonPos);
            std::string val = line.substr(colonPos + 1);
            while (!val.empty() && (val.front() == ' ' || val.front() == '\t')) val.erase(0, 1);

            if (key == "Types") {
                currentRepo.type = val;
            } else if (key == "URIs") {
                currentRepo.uri = val;
            } else if (key == "Suites") {
                currentRepo.suite = val;
            } else if (key == "Components") {
                currentRepo.components = val;
            } else if (key == "Enabled" && (val == "no" || val == "false")) {
                currentRepo.isEnabled = false;
            }
        }
    }

    if (inBlock && !currentRepo.uri.empty()) {
        currentRepo.id = currentRepo.type + ":" + currentRepo.uri + ":" + currentRepo.suite;
        if (currentRepo.uri.find("ubuntu.com") == std::string::npos &&
            currentRepo.uri.find("debian.org") == std::string::npos) {
            currentRepo.isThirdParty = true;
        }
        list.push_back(currentRepo);
    }
}

std::vector<AptRepository> RepositoryManager::getRepositories() {
    std::vector<AptRepository> list;

    if (m_backend && m_backend->isMock()) {
        list.push_back({"deb:http://archive.ubuntu.com/ubuntu:noble", "deb", "http://archive.ubuntu.com/ubuntu", "noble", "main restricted universe", "/etc/apt/sources.list", 1, true, false, false, ""});
        list.push_back({"deb:https://ppa.launchpadcontent.net/mozillateam/ppa/ubuntu:noble", "deb", "https://ppa.launchpadcontent.net/mozillateam/ppa/ubuntu", "noble", "main", "/etc/apt/sources.list.d/mozillateam.list", 1, true, true, false, "ppa:mozillateam/ppa"});
        list.push_back({"deb:https://download.docker.com/linux/ubuntu:noble", "deb", "https://download.docker.com/linux/ubuntu", "noble", "stable", "/etc/apt/sources.list.d/docker.sources", 1, true, true, true, ""});
        return list;
    }

    if (!m_backend) return list;

    // 1. Classic /etc/apt/sources.list
    if (m_backend->pathExists("/etc/apt/sources.list")) {
        parseClassicListFile("/etc/apt/sources.list", list);
    }

    // 2. /etc/apt/sources.list.d/ directory
    std::string sourcesDir = "/etc/apt/sources.list.d";
    std::error_code ec;
    if (std::filesystem::exists(sourcesDir, ec)) {
        for (const auto& entry : std::filesystem::directory_iterator(sourcesDir, ec)) {
            if (entry.path().extension() == ".list") {
                parseClassicListFile(entry.path().string(), list);
            } else if (entry.path().extension() == ".sources") {
                parseDeb822SourcesFile(entry.path().string(), list);
            }
        }
    }

    return list;
}

RepositorySummary RepositoryManager::getRepositorySummary() {
    RepositorySummary summary;
    summary.repositories = getRepositories();
    summary.totalCount = static_cast<int>(summary.repositories.size());

    std::set<std::string> seen;
    for (const auto& repo : summary.repositories) {
        if (repo.isEnabled) {
            summary.enabledCount++;
        }
        if (repo.isThirdParty) {
            summary.thirdPartyCount++;
        }

        std::string key = repo.type + " " + repo.uri + " " + repo.suite;
        if (repo.isEnabled) {
            if (seen.find(key) != seen.end()) {
                summary.duplicateWarnings.push_back("Duplicate repository entry found: " + key + " in " + repo.sourceFile);
            } else {
                seen.insert(key);
            }
        }
    }

    return summary;
}

} // namespace LinuxManager::Package

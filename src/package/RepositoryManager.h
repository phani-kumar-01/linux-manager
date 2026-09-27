#pragma once

#include "../core/ISystemBackend.h"
#include <string>
#include <vector>
#include <memory>

namespace LinuxManager::Package {

struct AptRepository {
    std::string id;
    std::string type; // "deb" or "deb-src"
    std::string uri;
    std::string suite;
    std::string components;
    std::string sourceFile;
    int lineNumber = 0;
    bool isEnabled = true;
    bool isThirdParty = false;
    bool isDeb822 = false;
    std::string ppaName;
};

struct RepositorySummary {
    std::vector<AptRepository> repositories;
    int totalCount = 0;
    int enabledCount = 0;
    int thirdPartyCount = 0;
    std::vector<std::string> duplicateWarnings;
};

class RepositoryManager {
public:
    explicit RepositoryManager(std::shared_ptr<Core::ISystemBackend> backend);

    [[nodiscard]] RepositorySummary getRepositorySummary();
    [[nodiscard]] std::vector<AptRepository> getRepositories();

private:
    std::shared_ptr<Core::ISystemBackend> m_backend;

    void parseClassicListFile(const std::string& filePath, std::vector<AptRepository>& list);
    void parseDeb822SourcesFile(const std::string& filePath, std::vector<AptRepository>& list);
};

} // namespace LinuxManager::Package

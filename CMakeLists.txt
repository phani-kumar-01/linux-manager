cmake_minimum_required(VERSION 3.20)
project(linux-manager VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Global Compiler Flags
add_compile_options(-Wall -Wextra -Wpedantic -O2 -pthread)

# Runtime Output Directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Core Sources
set(CORE_SOURCES
    src/core/CommandRunner.cpp
    src/core/RealSystemBackend.cpp
    src/core/MockSystemBackend.cpp
    src/core/PermissionManager.cpp
    src/core/HistoryManager.cpp
    src/core/Application.cpp
    src/security/PolicyManager.cpp
    src/security/OperationValidator.cpp
    src/security/PrivilegedDispatcher.cpp
    src/package/AptManager.cpp
    src/package/UpdateManager.cpp
    src/package/FlatpakManager.cpp
    src/package/RepositoryManager.cpp
    src/developer/SdkDoctor.cpp
    src/process/ProcessManager.cpp
    src/startup/StartupManager.cpp
    src/cleanup/CacheScanner.cpp
    src/cleanup/DeveloperCleaner.cpp
    src/cleanup/DockerCleaner.cpp
    src/cleanup/CleanupEngine.cpp
    src/storage/StorageScanner.cpp
    src/storage/LargeFileScanner.cpp
    src/system/SystemInfo.cpp
    src/system/ServiceManager.cpp
    src/system/KernelManager.cpp
    src/system/LogManager.cpp
    src/system/PowerManager.cpp
)

# Build Core Static Library
add_library(linuxmanager_core STATIC ${CORE_SOURCES})
target_include_directories(linuxmanager_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(linuxmanager_core PUBLIC pthread)

# Optional Qt6 Discovery
find_package(Qt6 COMPONENTS Core Gui Quick Qml QuickControls2 QUIET)

if (Qt6_FOUND)
    message(STATUS "Qt6 found - building with native QML desktop GUI support.")
    qt_standard_project_setup()
    
    add_executable(linux-manager
        src/main.cpp
    )
    
    target_compile_definitions(linux-manager PRIVATE HAVE_QT_GUI)
    target_link_libraries(linux-manager PRIVATE
        linuxmanager_core
        Qt6::Core
        Qt6::Gui
        Qt6::Quick
        Qt6::Qml
        Qt6::QuickControls2
    )
    
    qt_add_qml_module(linux-manager
        URI LinuxManager
        VERSION 1.0
        RESOURCE_PREFIX "/"
        QML_FILES
            qml/Main.qml
            qml/Theme.qml
            qml/Dashboard.qml
            qml/Software.qml
            qml/Updates.qml
            qml/Cleanup.qml
            qml/Developer.qml
            qml/Toolchains.qml
            qml/Processes.qml
            qml/Startup.qml
            qml/Repositories.qml
            qml/Power.qml
            qml/Storage.qml
            qml/Services.qml
            qml/Health.qml
            qml/Settings.qml
            qml/components/StatCard.qml
            qml/components/ActionButton.qml
            qml/components/CleanupItemRow.qml
            qml/components/ProgressBarCustom.qml
            qml/components/ConfirmModal.qml
    )
else()
    message(STATUS "Qt6 not found - building CLI & Interactive Console engine binary.")
    add_executable(linux-manager
        src/main.cpp
    )
    target_link_libraries(linux-manager PRIVATE linuxmanager_core)
endif()

# Unit Tests Target
add_executable(linux_manager_tests
    tests/test_main.cpp
)
target_link_libraries(linux_manager_tests PRIVATE linuxmanager_core)

enable_testing()
add_test(NAME LinuxManagerCoreTests COMMAND linux_manager_tests)

# Installation definitions
install(TARGETS linux-manager RUNTIME DESTINATION /usr/bin)
install(FILES packaging/debian/linux-manager.desktop DESTINATION /usr/share/applications)
install(FILES packaging/debian/com.linuxmanager.pkexec.policy DESTINATION /usr/share/polkit-1/actions)
install(DIRECTORY qml DESTINATION /usr/share/linux-manager)

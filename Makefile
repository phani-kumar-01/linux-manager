CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Wpedantic -O2 -pthread -Isrc
AR = ar
ARFLAGS = rcs

BUILD_DIR = build
BIN_DIR = $(BUILD_DIR)/bin
OBJ_DIR = $(BUILD_DIR)/obj
LIB_DIR = $(BUILD_DIR)/lib

SOURCES = \
    src/core/CommandRunner.cpp \
    src/core/RealSystemBackend.cpp \
    src/core/MockSystemBackend.cpp \
    src/core/PermissionManager.cpp \
    src/core/HistoryManager.cpp \
    src/core/Application.cpp \
    src/security/PolicyManager.cpp \
    src/security/OperationValidator.cpp \
    src/security/PrivilegedDispatcher.cpp \
    src/package/AptManager.cpp \
    src/package/UpdateManager.cpp \
    src/package/FlatpakManager.cpp \
    src/package/RepositoryManager.cpp \
    src/developer/SdkDoctor.cpp \
    src/process/ProcessManager.cpp \
    src/startup/StartupManager.cpp \
    src/cleanup/CacheScanner.cpp \
    src/cleanup/DeveloperCleaner.cpp \
    src/cleanup/DockerCleaner.cpp \
    src/cleanup/CleanupEngine.cpp \
    src/storage/StorageScanner.cpp \
    src/storage/LargeFileScanner.cpp \
    src/system/SystemInfo.cpp \
    src/system/ServiceManager.cpp \
    src/system/KernelManager.cpp \
    src/system/LogManager.cpp \
    src/system/PowerManager.cpp

OBJECTS = $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(SOURCES))
CORE_LIB = $(LIB_DIR)/liblinuxmanager_core.a

TARGET = $(BIN_DIR)/linux-manager
TEST_TARGET = $(BIN_DIR)/linux_manager_tests

.PHONY: all clean test deb tarball run-dashboard run-scan linux-manager

all: $(TARGET) $(TEST_TARGET)

linux-manager: $(TARGET)

$(OBJ_DIR)/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(CORE_LIB): $(OBJECTS)
	@mkdir -p $(LIB_DIR)
	$(AR) $(ARFLAGS) $@ $^

$(TARGET): src/main.cpp $(CORE_LIB)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) src/main.cpp $(CORE_LIB) -o $@

$(TEST_TARGET): tests/test_main.cpp $(CORE_LIB)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) tests/test_main.cpp $(CORE_LIB) -o $@

test: $(TEST_TARGET)
	@echo "==> Running Linux Manager Test Suite..."
	@$(TEST_TARGET)

run-dashboard: $(TARGET)
	@$(TARGET) --dashboard

run-scan: $(TARGET)
	@$(TARGET) --scan

deb: $(TARGET)
	@chmod +x packaging/debian/build_deb.sh
	@./packaging/debian/build_deb.sh

tarball: $(TARGET)
	@chmod +x packaging/tarball/build_tarball.sh
	@./packaging/tarball/build_tarball.sh

clean:
	rm -rf $(BUILD_DIR)

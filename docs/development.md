# Development Guide

This guide details how to build, test, and extend Linux Manager.

---

## 1. Development Environment Setup

Ensure the required development packages are installed on Ubuntu/Debian:

```bash
sudo apt update
sudo apt install -y build-essential cmake pkg-config \
    qt6-base-dev qt6-declarative-dev qml6-module-qtquick \
    qml6-module-qtquick-controls qml6-module-qtquick-layouts \
    libsystemd-dev libdbus-1-dev dpkg-dev
```

---

## 2. Compilation and Testing

### Build with `make`

```bash
# Compile both the application and the unit test suite
make

# Run the unit test suite
make test
```

### Build with `CMake`

```bash
cmake -S . -B build
cmake --build build -j$(nproc)
ctest --test-dir build --output-on-failure
```

---

## 3. Adding a New Developer Cache Scanner

To add support for a new programming language or toolchain cache:

1. Open `src/cleanup/DeveloperCleaner.h`.
2. Add your scanner method declaration:
   ```cpp
   [[nodiscard]] CleanupItem scanMyToolchainCache();
   ```
3. Implement the scanner in `src/cleanup/DeveloperCleaner.cpp`:
   ```cpp
   CleanupItem DeveloperCleaner::scanMyToolchainCache() {
       CleanupItem item;
       item.id = "dev-mytool-cache";
       item.title = "MyToolchain Cache";
       item.category = CleanupCategory::Custom;
       item.risk = RiskLevel::Safe;
       item.description = "Downloaded cache files in ~/.mytool/cache.";
       
       if (m_backend) {
           std::string path = m_backend->getUserHomeDirectory() + "/.mytool/cache";
           if (m_backend->pathExists(path)) {
               item.sizeBytes = m_backend->getPathSize(path);
               item.path = path;
           }
       }
       return item;
   }
   ```
4. Register the new scanner in `DeveloperCleaner::scanAllDeveloperCaches()`.
5. Add a test in `tests/test_main.cpp`.

---

## 4. Writing Unit Tests

All unit tests reside in `tests/test_main.cpp`. Always use `MockSystemBackend` to simulate filesystem trees, commands, and permissions without altering local system state.

Example:

```cpp
void testMyFeature() {
    auto mock = std::make_shared<LinuxManager::Core::MockSystemBackend>();
    mock->setMockDirectory("/home/testuser/.mytool/cache", 50 * 1024 * 1024);

    LinuxManager::Cleanup::DeveloperCleaner cleaner(mock);
    auto item = cleaner.scanMyToolchainCache();

    TEST_ASSERT(item.sizeBytes == 50 * 1024 * 1024, "Size matches mock directory");
}
```

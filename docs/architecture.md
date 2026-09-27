# Linux Manager Architecture

## Layered Design

Linux Manager is organized into clear architectural layers ensuring safety, testability, and separation of concerns:

```text
┌─────────────────────────────────────────────────────────────┐
│                      Qt 6 / QML UI                          │
│   Main.qml • Dashboard • Software • Cleanup • Storage ...   │
├─────────────────────────────────────────────────────────────┤
│                    Application Layer                        │
│   LinuxManager::Core::Application • HistoryManager          │
├─────────────────────────────────────────────────────────────┤
│                      Service Layer                          │
│   AptManager • CleanupEngine • StorageScanner • SystemInfo  │
├─────────────────────────────────────────────────────────────┤
│                   System Abstraction                        │
│                  ISystemBackend Interface                   │
│         ┌───────────────────────┬────────────────────────┐  │
│         │   RealSystemBackend   │   MockSystemBackend    │  │
│         └───────────────────────┴────────────────────────┘  │
├─────────────────────────────────────────────────────────────┤
│                      Host Linux OS                          │
│   APT/dpkg │ Polkit │ systemd │ /proc │ /sys │ D-Bus        │
└─────────────────────────────────────────────────────────────┘
```

---

## 1. Core Abstractions

### `ISystemBackend`
An abstract interface (`src/core/ISystemBackend.h`) that standardizes system-level primitives:
- Command execution (`executeCommand`)
- Filesystem inspection & size calculation (`getPathSize`, `pathExists`)
- Safe file deletion (`removePath`)
- Environment queries (`getUserHomeDirectory`, `isElevated`)

### `RealSystemBackend`
Interacts with the live Linux operating system:
- Reads `/proc/meminfo`, `/proc/cpuinfo`, `/proc/stat`, `/etc/os-release`, `/sys/class/thermal/`.
- Executes commands through `CommandRunner`.

### `MockSystemBackend`
Allows simulating file trees, command responses, and permission states in-memory:
- Eliminates any risk of destructive deletions during automated tests.
- Enables CI/CD pipelines to run full integration suites on non-Linux or containerized environments.

---

## 2. Command Execution Safety

The application uses `CommandRunner` to prevent shell injection vulnerabilities. Rather than invoking `sh -c` or `system()`, `CommandRunner` executes commands directly using `pipe()`, `fork()`, and `execvp()` with structured arguments:

```cpp
// Safe execution using structured argument vectors
auto result = CommandRunner::run("apt-get", {"install", "-y", packageName});
```

Features:
- Non-blocking I/O polling via `poll()`.
- Timeout enforcement to prevent hangs on stalled background processes.
- Direct capture of `stdout` and `stderr`.

---

## 3. Package & Software Layer

- **`AptManager`**: Directly parses `/var/lib/dpkg/status` for instantaneous package indexing without process spawn overhead. Falls back to `apt-cache` and `dpkg-query` when needed.
- **`UpdateManager`**: Evaluates simulation runs (`apt-get -s dist-upgrade`) to detect pending security patches and calculate total download payloads.
- **`FlatpakManager`**: Inspects user and system Flatpak runtimes, identifies orphaned runtimes, and triggers cleanups.

---

## 4. Cleanup & Developer Modules

- **`CacheScanner`**: Scans system caches (`/var/cache/apt/archives`, `/var/crash`, `/var/log/journal`, thumbnails, browser caches).
- **`DeveloperCleaner`**: Analyzes developer environments:
  - Node.js (`~/.npm`, project `node_modules`)
  - Python (`~/.cache/pip`, `__pycache__`, `.venv`)
  - Java / Android (`~/.m2`, `~/.gradle`, Android SDK caches)
  - Rust (`~/.cargo/registry`, project `target/`)
  - Docker (`docker system df`)
- **`CleanupEngine`**: Coordinates multi-category scans, provides dry-run previews, verifies deletion safety with `OperationValidator`, and writes audit records to `HistoryManager`.

---

## 5. Storage Analyzer

- **`StorageScanner`**: Inspects disk partitions via `statvfs` and performs tree scans.
- **`LargeFileScanner`**: Recursively scans directories to detect files above a configurable threshold (e.g. 100 MB).

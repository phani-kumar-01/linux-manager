# Linux Manager

[![Language](https://img.shields.io/badge/language-C%2B%2B20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![UI](https://img.shields.io/badge/UI-Qt%206%20%2F%20QML-41cd52.svg)](https://www.qt.io/)
[![Platform](https://img.shields.io/badge/platform-Ubuntu%20%7C%20Debian-E95420.svg)](https://ubuntu.com)
[![Security](https://img.shields.io/badge/security-allow--list%20polkit-brightgreen.svg)](docs/security.md)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

> **A Native System, Software, Cleanup and Developer Environment Manager for Ubuntu and Debian.**

Linux Manager is a modern, high-performance desktop application designed specifically for Ubuntu, Debian, Pop!_OS, Linux Mint, and Debian-based distributions. It consolidates daily maintenance operations—such as system updates, APT and Flatpak package management, deep cache cleaning, developer environment cache purges, service controls, hardware telemetry, and storage breakdown—into a polished, safety-first desktop control center.

---

## Key Features

- 📊 **Unified Dashboard**: Real-time CPU, RAM, disk utilization, thermal sensor monitoring, pending updates, and recoverable cache estimates.
- 📦 **Software & Package Manager**: Search, install, inspect, and remove native APT packages and Flatpak applications. Includes left-over configuration detection and dependency cleanup calculation.
- 🔄 **System Update Manager**: Distinguish between security patches and general application updates; inspect change lists and perform batch upgrades.
- 🧹 **System Cleanup Engine**: Safely purge APT package archives, systemd journal logs (via controlled vacuuming), thumbnail caches, crash reports, and browser caches.
- ⚡ **Developer Cache Suite**: Specialized scanning and cleanup for:
  - **Node.js**: `~/.npm`, `~/.yarn`, `~/.pnpm-store`, and project `node_modules` detection.
  - **Python**: `pip` cache, `uv` cache, bytecode `__pycache__`, `.pytest_cache`, and virtual environments.
  - **Java**: Maven (`~/.m2`) and Gradle (`~/.gradle/caches`) repositories.
  - **Rust**: Cargo registry caches, git checkouts, and project `target/` directories.
  - **Android SDK**: Build tools, emulator image snapshots, and SDK temp caches.
  - **Docker**: Inspect images, containers, volumes, and build cache via `docker system df`.
- 💿 **Storage Analyzer & Large File Finder**: Hierarchical folder size visualizer and fast file scanner finding items exceeding configurable size thresholds (>100 MB).
- ⚙️ **Systemd Service Manager**: Monitor active, inactive, and failed systemd services; view journal logs; and perform safe restarts with critical unit protection.
- 🔒 **Zero-Root GUI & Strict Polkit Allow-List**: The GUI application **never** runs as root. Privileged actions delegate strictly through a typed **Allow-List Privileged Dispatcher** using trusted absolute paths (`/usr/bin/apt-get`, `/usr/bin/systemctl`, `/usr/bin/journalctl`, `/usr/bin/pkexec`).
- 🛡️ **Subtree & Symlink Safety**: Path deletion is strictly bounded to allowed user caches and recognized project directories; critical system subtrees (`/etc/**`, `/usr/**`, `/var/**`, etc.) are protected, and symlinks are never traversed for directory deletion.
- 📜 **Operation Audit History**: Maintains a persistent local audit history of all maintenance actions and reclaimed disk space in `~/.local/share/linux-manager/history.log`.

---

## Architecture Overview

```text
┌─────────────────────────────────────────────────────────┐
│                    Qt 6 / QML UI                        │
│  Main • Dashboard • Software • Cleanup • Developer ...  │
├─────────────────────────────────────────────────────────┤
│                  Application Layer                      │
│      Core Coordinator • History • Permissions           │
├─────────────────────────────────────────────────────────┤
│                    Security Layer                       │
│ PrivilegedDispatcher • PolicyManager • OperationValidator│
├─────────────────────────────────────────────────────────┤
│                    Service Layer                        │
│ AptManager • CleanupEngine • Storage • SystemInfo ...   │
├─────────────────────────────────────────────────────────┤
│            System Abstraction (ISystemBackend)          │
│         RealSystemBackend    │    MockSystemBackend     │
├─────────────────────────────────────────────────────────┤
│                   Linux Subsystems                      │
│     APT/dpkg │ systemd │ Polkit │ /proc │ /sys │ D-Bus  │
└─────────────────────────────────────────────────────────┘
```

---

## Building and Installation

### Prerequisites

- **Compiler**: GCC 11+ or Clang 13+ (C++20 support)
- **Build Tools**: `make`, `cmake`, `pkg-config`, `dpkg-deb`
- **Optional for GUI**: Qt 6.4+ development headers (`qt6-base-dev`, `qt6-declarative-dev`, `qml6-module-qtquick`)

### Quick Build (Makefile)

```bash
# Build binary and unit test suite
make

# Execute unit test suite (10 test suites)
make test

# Run interactive terminal console
./build/bin/linux-manager

# Run CLI dashboard inspection
./build/bin/linux-manager --dashboard

# Run cache and storage scan
./build/bin/linux-manager --scan
```

### Packaging

#### Debian `.deb` Package

```bash
make deb
# Outputs: build/deb_dist/linux-manager_1.0.0_amd64.deb
sudo apt install ./build/deb_dist/linux-manager_1.0.0_amd64.deb
```

#### Portable `.tar.xz` Archive

```bash
make tarball
# Outputs: build/tarball_dist/linux-manager-1.0.0-linux-x86_64.tar.xz
```

---

## CLI Usage Options

```bash
linux-manager [OPTIONS]

Options:
  --cli              Run in terminal interactive/inspect mode
  --dashboard        Show immediate system status and metrics
  --scan             Scan all system and developer caches
  --clean            Run system cleanup (interactive)
  --dry-run          Perform a simulated dry-run cleanup
  --updates          Check available APT and Flatpak updates
  --services         List systemd services and failed units
  --storage          Analyze disk partitions and large files
  --developer        Scan developer caches (Node, Python, Java, Rust)
  --history          Display past cleanup and maintenance operations
  --mock             Use MockSystemBackend (safe simulated sandbox)
  --version          Display application version
  --help             Show help message
```

---

## Security Model

1. **Unprivileged Execution**: Desktop UI runs as a regular desktop user.
2. **Allow-List Dispatcher**: Only strongly typed, allow-listed `PrivilegedOperation` types can be elevated via PolicyKit.
3. **Subtree Path Protection**: Recursive subtree checks strictly protect `/`, `/etc/**`, `/usr/**`, `/var/**`, `/boot/**`, and `/home`.
4. **Symlink Traversal Safety**: Symlinks are removed without dereferencing target contents, and targets are canonicalized and verified against protected system roots.
5. **Trusted Absolute Executable Paths**: All elevated operations invoke fixed, trusted binary paths (`/usr/bin/apt-get`, `/usr/bin/systemctl`, `/usr/bin/journalctl`, `/usr/bin/pkexec`).
6. **Zero Shell Interpolation**: Subprocesses execute via structured argument vectors (`fork()` + `execvp()`), never invoking strings through `system()`.

For full details, see [docs/security.md](docs/security.md) and [SECURITY.md](SECURITY.md).

---

## License

This project is licensed under the [MIT License](LICENSE).

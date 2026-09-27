# Changelog

All notable changes to **Linux Manager** will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [1.1.0] - 2026-08-25

### Added
- **Developer SDK Doctor & Toolchain Diagnostics**:
  - Auto-detection and version querying across Node.js, Python, Rust, Go, Java, GCC/G++, Clang, CMake, Ninja, Docker, and Kubectl.
  - Environment health diagnostics detecting missing build essentials (`build-essential`, `pkg-config`, `libssl-dev`), misconfigured `PATH`, or missing Git config.
  - Go ecosystem cache cleaner (`~/.cache/go-build` and `~/go/pkg/mod/cache`).
- **Process Explorer & Port Killer ("EADDRINUSE Resolver")**:
  - Live process table reading `/proc` with PID, memory (RSS), CPU %, user, and command.
  - Dedicated Developer Tool filter isolating developer processes (`node`, `python`, `cargo`, `rust-analyzer`, `clangd`, `docker`, `postgres`, `redis`).
  - Active listening port inspector (`ss -tulpn`) and instant port-to-process killer.
- **Startup & Autostart Application Manager**:
  - Scans user autostarts (`~/.config/autostart/*.desktop`) and system autostarts (`/etc/xdg/autostart/*.desktop`).
  - Safe enable/disable toggle (`Hidden=true/false` and `X-GNOME-Autostart-enabled`) and custom desktop entry creation.
- **APT Repository & PPA Manager**:
  - Parses `/etc/apt/sources.list` and `/etc/apt/sources.list.d/` (`.list` and modern Deb822 `.sources` format).
  - Categorizes official mirrors vs third-party PPAs, with duplicate repository warnings.
- **Battery Health & Power Profile Optimizer**:
  - Telemetry for battery degradation, wear percentage, cycle count, and discharge rate.
  - Power profile switcher integrating with `power-profiles-daemon` (`powerprofilesctl`) and `system76-power`.
- **User Interface & CLI**:
  - 5 new native Qt 6 / QML screens (`Toolchains.qml`, `Processes.qml`, `Startup.qml`, `Repositories.qml`, `Power.qml`).
  - New CLI commands: `--sdk`, `--ports`, `--autostart`, `--repos`, `--power`.
  - Expanded interactive TUI control console with 14 menu options.
- **Tests**:
  - Expanded unit test suite from 10 to 15 test suites covering all new subsystems (100% pass rate).

---

## [1.0.0] - 2026-08-23

### Added
- **Core Architecture**:
  - `ISystemBackend` abstraction with `RealSystemBackend` and `MockSystemBackend`.
  - Structured, non-blocking `CommandRunner` with timeout protection and pipe redirection.
  - Audit logging via `HistoryManager` in `~/.local/share/linux-manager/history.log`.
  - Static library `liblinuxmanager_core.a` architecture with target includes in CMake.
- **Security & Privilege Hardening**:
  - Strict allow-list `PrivilegedDispatcher` eliminating generic elevated command execution.
  - Strongly typed `PrivilegedOperation` variants for APT, systemd, and journalctl.
  - Subtree protection preventing deletion of system roots (`/`, `/etc`, `/usr`, `/var`, `/boot`, `/dev`).
  - Symlink traversal protection ensuring symlinks are never dereferenced for directory deletion.
  - Trusted absolute binary paths (`/usr/bin/apt-get`, `/usr/bin/systemctl`, `/usr/bin/journalctl`, `/usr/bin/pkexec`).
- **Software & Package Manager**:
  - Native APT/dpkg parser with `/var/lib/dpkg/status` direct indexing.
  - Simulation-based update checking with security update tagging.
  - Flatpak application browser and unused runtime detector.
- **Cleanup & Developer Suite**:
  - System cache scanners for APT archives, journal logs, thumbnails, crash reports, and browsers.
  - Developer ecosystem cache scanners for Node.js, Python, Java/Maven/Gradle, Rust/Cargo, Android SDK, and Docker.
  - Dry-run preview and interactive verification.
- **Storage & Telemetry**:
  - Partition analyzer and directory breakdown tree.
  - Large file scanner finding files >100 MB.
  - Hardware telemetry for CPU, thermal sensors, memory allocation, and battery status.
- **User Interfaces**:
  - Qt 6 / QML graphical desktop interface (Dashboard, Software, Updates, Cleanup, Developer, Storage, Services, Health, Settings).
  - Interactive Terminal Console (TUI) for headless and terminal management sessions.
- **Packaging & Tests**:
  - Debian `.deb` package builder (`build_deb.sh`).
  - Portable `.tar.xz` builder (`build_tarball.sh`).
  - Comprehensive unit test suite (`tests/test_main.cpp`).

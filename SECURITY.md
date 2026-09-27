# Security Policy

## Reporting Security Issues

The Linux Manager team takes security vulnerabilities seriously. If you discover a security vulnerability, please **do not open a public issue**. Instead, send an email to `security@linuxmanager.org` or report via private GitHub Security Advisory.

---

## Security Architecture & Guarantees

Linux Manager is built with defense-in-depth principles:

### 1. Zero-Root GUI Process
- The desktop GUI application and all user-facing interactive sessions execute strictly as the **unprivileged user**.
- The binary refuses to run the UI under `sudo` or as root.

### 2. Allow-List Privileged Operation Dispatcher
- Arbitrary elevated command execution (`pkexec <anything>`) is strictly disallowed in the codebase.
- Elevated execution is restricted to strongly typed `PrivilegedOperation` variants:
  - `AptUpdate` (`/usr/bin/apt-get update`)
  - `AptInstall` (`/usr/bin/apt-get install -y <packages>`)
  - `AptRemove` / `AptPurge` (`/usr/bin/apt-get remove/purge -y <packages>`)
  - `AptUpgrade` (`/usr/bin/apt-get dist-upgrade -y ...`)
  - `AptAutoremove` (`/usr/bin/apt-get autoremove -y`)
  - `AptClean` (`/usr/bin/apt-get clean`)
  - `JournalVacuumSize` (`/usr/bin/journalctl --vacuum-size=<N>M`, minimum 50MB)
  - `JournalVacuumTime` (`/usr/bin/journalctl --vacuum-time=<N>d`, minimum 1 day)
  - `ServiceAction` (`/usr/bin/systemctl restart/start/stop/enable/disable <unit>`)
  - `KernelPurge` (`/usr/bin/apt-get purge -y linux-image-<version>`)

### 3. Subtree Protection & Safe Deletion Boundaries
- Subtree protection prevents targeting `/`, `/bin`, `/boot`, `/dev`, `/etc`, `/lib`, `/opt`, `/proc`, `/root`, `/run`, `/sbin`, `/sys`, `/usr`, `/var` for deletion.
- User file deletions are strictly bounded to:
  - Allowed user cache directories (`~/.cache/**`, `~/.npm/**`, `~/.cargo/registry/**`, `~/.gradle/caches/**`, `~/.m2/**`, `~/.android/cache/**`).
  - Recognized developer project build artifacts (`node_modules`, `target`, `__pycache__`, `.pytest_cache`, `.mypy_cache`, `.venv`).

### 4. Symlink Traversal Protection
- Cleanup operations never recurse through directory symlinks.
- When removing a symlink, only the symlink node itself is deleted.
- Target paths are canonicalized and verified against protected subtrees.

### 5. Trusted Absolute Binary Paths
- Privileged operations execute fixed absolute paths (`/usr/bin/apt-get`, `/usr/bin/systemctl`, `/usr/bin/journalctl`, `/usr/bin/pkexec`) rather than searching the user's mutable `$PATH`.

---

## Supported Versions

| Version | Supported          |
| ------- | ------------------ |
| 1.0.x   | :white_check_mark: |

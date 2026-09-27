# Linux Manager Security Architecture

Security is a first-class engineering priority for Linux Manager. The system employs defense-in-depth mechanisms to protect user data, system stability, and administrative boundaries.

---

## 1. Unprivileged Execution (No Root GUI)

Running graphical applications as `root` (e.g. `sudo linux-manager`) is prohibited on modern Linux desktops. Linux Manager enforces:
- The UI / main process always executes as the **normal, unprivileged user**.
- Privileged tasks are strictly dispatched through an **Allow-List Privileged Dispatcher** delegating to PolicyKit (`/usr/bin/pkexec`).
- Granular Polkit policy rules are installed to `/usr/share/polkit-1/actions/com.linuxmanager.pkexec.policy`.

---

## 2. Allow-List Privileged Operation Dispatcher

Unlike generic runners, Linux Manager disallows executing arbitrary command strings elevated. Only strongly typed, allow-listed `PrivilegedOperation` types are permitted:

```cpp
enum class PrivilegedOpType {
    AptUpdate,
    AptInstall,
    AptRemove,
    AptPurge,
    AptUpgrade,
    AptAutoremove,
    AptClean,
    JournalVacuumSize,
    JournalVacuumTime,
    ServiceRestart,
    ServiceStart,
    ServiceStop,
    ServiceEnable,
    ServiceDisable,
    KernelPurge
};
```

Each operation maps strictly internally to verified, **trusted absolute binary paths**:
- `/usr/bin/apt-get`
- `/usr/bin/systemctl`
- `/usr/bin/journalctl`
- `/usr/bin/pkexec`

---

## 3. Recursive Subtree Protection & Safe Deletion Policy

### Protected System Subtrees
Any path located within or resolving to the following system trees is strictly blocked from deletion:
- `/`, `/bin`, `/sbin`, `/boot`, `/dev`, `/etc`, `/lib`, `/opt`, `/proc`, `/root`, `/run`, `/sys`, `/usr`, `/var`

### Bounded User Deletions
File and directory removals are strictly permitted only within:
1. Approved user cache locations (`~/.cache/**`, `~/.npm/**`, `~/.cargo/registry/**`, `~/.gradle/caches/**`, `~/.m2/repository/**`, `~/.android/cache/**`).
2. Recognized developer project build directories (`node_modules`, `target`, `__pycache__`, `.pytest_cache`, `.mypy_cache`, `.venv`, `venv`, `build`, `dist`).

---

## 4. Symlink & Mount Resolution Safety

To prevent directory traversal and symlink-based deletion vulnerabilities:
- When a symlink is targeted for removal, **only the symlink node itself is removed** (`std::filesystem::remove`), never the contents of the target directory.
- Before computing size or checking targets, symlinks are canonicalized and verified against protected subtrees.

---

## 5. Input Validation & Structured Execution

- **Zero Shell Invocation**: Commands are executed via structured argument vectors (`fork()` + `execvp()`), eliminating shell interpolation vulnerabilities.
- **Debian Package Validation**: Package names are strictly validated against `^[a-z0-9][a-z0-9+\.\-]+$`.
- **Systemd Unit Validation**: Service unit names are validated against `^[a-zA-Z0-9@_\.\-]+\.service$`.
- **Protected Packages**: `systemd`, `libc6`, `apt`, `dpkg`, `login`, `bash`, `coreutils`, `polkitd` are blocked from removal.
- **Protected Services**: `dbus.service`, `systemd-journald.service`, `polkit.service`, `NetworkManager.service` cannot be stopped or disabled.

---

## 6. Operation Dry-Run & Audit Logging

- **Dry-Run Mode**: Every cleanup and package removal operation can be previewed without executing filesystem modifications.
- **Immutable Audit Trail**: All operations log unique Operation IDs, timestamps, affected files count, and reclaimed bytes to `~/.local/share/linux-manager/history.log`.

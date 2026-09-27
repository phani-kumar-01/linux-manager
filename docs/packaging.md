# Packaging and Distribution Guide

Linux Manager supports two official distribution formats for Ubuntu and Debian systems:

1. **Debian Package (`.deb`)**: Standard native package for Debian, Ubuntu, Pop!_OS, Linux Mint, and derivatives.
2. **Portable Tarball (`.tar.xz`)**: Self-contained archive suitable for manual deployment and non-root user testing.

---

## 1. Building the Debian Package

The project includes standard Debian package configuration files in `packaging/debian/`:
- `control`: Defines package metadata, dependencies (`policykit-1`, `pkexec`, `apt`, `dpkg`, `systemd`), and architecture (`amd64`, `arm64`).
- `rules`: Standard debhelper automation file.
- `compat`: Debhelper compatibility version 13.
- `com.linuxmanager.pkexec.policy`: PolicyKit authorization rules.
- `linux-manager.desktop`: Desktop launcher integration.

### Build Command

```bash
make deb
```

This runs `packaging/debian/build_deb.sh`, producing:
`build/deb_dist/linux-manager_1.0.0_amd64.deb`

### Installing the `.deb`

```bash
sudo apt install ./build/deb_dist/linux-manager_1.0.0_amd64.deb
```

### Removing the Package

```bash
sudo apt remove linux-manager
```

---

## 2. Building the Portable `.tar.xz` Archive

The portable archive includes the compiled binary, QML UI assets, desktop entry, and an `install.sh` helper script.

### Build Command

```bash
make tarball
```

This produces:
`build/tarball_dist/linux-manager-1.0.0-linux-x86_64.tar.xz`

### Extracting and Running

```bash
tar -xf build/tarball_dist/linux-manager-1.0.0-linux-x86_64.tar.xz
cd linux-manager-1.0.0-linux-x86_64
./bin/linux-manager --dashboard
```

To install system-wide from the tarball:

```bash
sudo ./install.sh /usr/local
```

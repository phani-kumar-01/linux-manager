#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/../.." && pwd)"
VERSION="1.0.0"
ARCH="amd64"
PKG_NAME="linux-manager_${VERSION}_${ARCH}"
BUILD_DIR="${ROOT_DIR}/build/deb_dist"
STAGING_DIR="${BUILD_DIR}/${PKG_NAME}"

echo "==> Building Debian package for Linux Manager v${VERSION} (${ARCH})..."

# Ensure binary is built
if [ ! -f "${ROOT_DIR}/build/bin/linux-manager" ]; then
    echo "==> Compiling application binary first..."
    make -C "${ROOT_DIR}" linux-manager
fi

# Prepare staging directories
rm -rf "${STAGING_DIR}"
mkdir -p "${STAGING_DIR}/DEBIAN"
mkdir -p "${STAGING_DIR}/usr/bin"
mkdir -p "${STAGING_DIR}/usr/share/applications"
mkdir -p "${STAGING_DIR}/usr/share/polkit-1/actions"
mkdir -p "${STAGING_DIR}/usr/share/linux-manager/qml"

# Copy binary & assets
cp "${ROOT_DIR}/build/bin/linux-manager" "${STAGING_DIR}/usr/bin/"
cp "${SCRIPT_DIR}/linux-manager.desktop" "${STAGING_DIR}/usr/share/applications/"
cp "${SCRIPT_DIR}/com.linuxmanager.pkexec.policy" "${STAGING_DIR}/usr/share/polkit-1/actions/"
cp -r "${ROOT_DIR}/qml/"* "${STAGING_DIR}/usr/share/linux-manager/qml/"

# Generate DEBIAN/control
cat <<EOF > "${STAGING_DIR}/DEBIAN/control"
Package: linux-manager
Version: ${VERSION}
Section: utils
Priority: optional
Architecture: ${ARCH}
Maintainer: Linux Manager Team <maintainers@linuxmanager.org>
Depends: policykit-1 | polkitd, pkexec, apt, dpkg, systemd
Recommends: flatpak, docker.io
Description: Native System, Software, Cleanup and Developer Environment Manager
 Linux Manager is a modern desktop management and cleanup suite designed
 specifically for Ubuntu, Debian, and Debian-based distributions.
EOF

# Ensure proper directory and file permissions
chmod -R 0755 "${STAGING_DIR}"
chmod 0755 "${STAGING_DIR}/DEBIAN"
chmod 0644 "${STAGING_DIR}/DEBIAN/control"
chmod 0755 "${STAGING_DIR}/usr/bin/linux-manager"

# Build .deb with dpkg-deb
dpkg-deb --build --root-owner-group "${STAGING_DIR}" "${BUILD_DIR}/${PKG_NAME}.deb"

echo "==> Successfully created Debian package: ${BUILD_DIR}/${PKG_NAME}.deb"

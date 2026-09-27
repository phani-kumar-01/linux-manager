#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/../.." && pwd)"
VERSION="1.0.0"
ARCH="linux-x86_64"
DIST_NAME="linux-manager-${VERSION}-${ARCH}"
BUILD_DIR="${ROOT_DIR}/build/tarball_dist"
STAGING_DIR="${BUILD_DIR}/${DIST_NAME}"

echo "==> Packaging portable TAR.XZ distribution: ${DIST_NAME}.tar.xz..."

# Ensure binary is built
if [ ! -f "${ROOT_DIR}/build/bin/linux-manager" ]; then
    echo "==> Compiling application binary first..."
    make -C "${ROOT_DIR}" linux-manager
fi

rm -rf "${STAGING_DIR}"
mkdir -p "${STAGING_DIR}/bin"
mkdir -p "${STAGING_DIR}/share/applications"
mkdir -p "${STAGING_DIR}/share/linux-manager/qml"

cp "${ROOT_DIR}/build/bin/linux-manager" "${STAGING_DIR}/bin/"
cp "${ROOT_DIR}/packaging/debian/linux-manager.desktop" "${STAGING_DIR}/share/applications/"
cp -r "${ROOT_DIR}/qml/"* "${STAGING_DIR}/share/linux-manager/qml/"
cp "${ROOT_DIR}/README.md" "${STAGING_DIR}/"
cp "${ROOT_DIR}/LICENSE" "${STAGING_DIR}/"

# Write simple installer helper script
cat <<'EOF' > "${STAGING_DIR}/install.sh"
#!/usr/bin/env bash
set -euo pipefail
PREFIX="${1:-/usr/local}"
echo "Installing Linux Manager to ${PREFIX}..."
mkdir -p "${PREFIX}/bin" "${PREFIX}/share/applications" "${PREFIX}/share/linux-manager"
cp bin/linux-manager "${PREFIX}/bin/"
cp share/applications/linux-manager.desktop "${PREFIX}/share/applications/"
cp -r share/linux-manager/* "${PREFIX}/share/linux-manager/"
echo "Installation complete!"
EOF
chmod +x "${STAGING_DIR}/install.sh"

tar -cJf "${BUILD_DIR}/${DIST_NAME}.tar.xz" -C "${BUILD_DIR}" "${DIST_NAME}"

echo "==> Successfully created portable archive: ${BUILD_DIR}/${DIST_NAME}.tar.xz"

#!/usr/bin/env bash
# make_deb.sh — Debian (.deb) package builder for Cymbals plugin
#
# Produces:
#   madmcman-cymbals_X.Y.Z_amd64.deb
#
# Usage (from repo root):
#   bash scripts/installer_linux/make_deb.sh
#
# Override version:
#   bash scripts/installer_linux/make_deb.sh --version X.Y.Z
#
# Override build dir:
#   BUILDDIR=/path/to/bin bash scripts/installer_linux/make_deb.sh

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/../.." && pwd)"

# Parse CLI arguments
VERSION=""
while [[ $# -gt 0 ]]; do
    case "$1" in
        --version)
            VERSION="$2"
            shift 2
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Prefer BUILD_DIR env (set by CI) then fall back to build-linux, then build-lvgl
BUILDDIR="${BUILDDIR:-${ROOT_DIR}/${BUILD_DIR:-build-linux}/bin}"
OUTPUT_DIR="${OUTPUT_DIR:-${ROOT_DIR}}"

# Version priority: CLI arg > env var > default
PKG_VERSION="${VERSION:-${VERSION_OVERRIDE:-0.9.0}}"

PKG_NAME="madmcman-cymbals"
PKG_ARCH="amd64"
PKG_FILE="${PKG_NAME}_${PKG_VERSION}_${PKG_ARCH}.deb"

APP_NAME="MadMcManBCE"

STAGING_ROOT="${STAGING_ROOT:-${ROOT_DIR}/deb-staging}"
STAGING="${STAGING_ROOT}"
DEB_CTRL="${STAGING}/DEBIAN"

echo "==> Cymbals Debian Package Builder"
echo "    Build dir : ${BUILDDIR}"
echo "    Output dir: ${OUTPUT_DIR}"
echo "    Package   : ${PKG_FILE}"

# Clean up previous staging area
rm -rf "${STAGING}"
mkdir -p "${DEB_CTRL}"
mkdir -p "${OUTPUT_DIR}"

# -----------------------------------------------------------------------
# Write DEBIAN/control
# -----------------------------------------------------------------------
cat > "${DEB_CTRL}/control" << EOF
Package: ${PKG_NAME}
Version: ${PKG_VERSION}
Architecture: ${PKG_ARCH}
Maintainer: MadMcMan <madmcman.brewing299@passinbox.com>
Section: sound
Priority: optional
Description: Cymbals — 3-Stage Linear Synthesis Engine (VST3/CLAP percussion plugin)
 Cymbals is a metallic percussion synthesizer built on the DISTRHO Plugin Framework.
 It provides VST3 and CLAP plugin formats plus a JACK-compatible standalone binary.
 .
 Includes the AirWindows processing suite with 478 selectable algorithms.
EOF

# -----------------------------------------------------------------------
# Stage VST3
# -----------------------------------------------------------------------
if [ -d "${BUILDDIR}/${APP_NAME}.vst3" ]; then
    echo "==> Staging VST3"
    VST3_DEST="${STAGING}/usr/lib/vst3/${APP_NAME}.vst3"
    mkdir -p "${VST3_DEST}"
    cp -R "${BUILDDIR}/${APP_NAME}.vst3/." "${VST3_DEST}/"
else
    echo "WARNING: VST3 bundle not found: ${BUILDDIR}/${APP_NAME}.vst3"
fi

# -----------------------------------------------------------------------
# Stage CLAP
# -----------------------------------------------------------------------
if [ -f "${BUILDDIR}/${APP_NAME}.clap" ]; then
    echo "==> Staging CLAP"
    CLAP_DEST="${STAGING}/usr/lib/clap"
    mkdir -p "${CLAP_DEST}"
    cp -f "${BUILDDIR}/${APP_NAME}.clap" "${CLAP_DEST}/"
else
    echo "WARNING: CLAP binary not found: ${BUILDDIR}/${APP_NAME}.clap"
fi

# -----------------------------------------------------------------------
# Stage standalone
# -----------------------------------------------------------------------
BIN_DEST="${STAGING}/usr/bin"
mkdir -p "${BIN_DEST}"

if [ -f "${BUILDDIR}/${APP_NAME}" ]; then
    echo "==> Staging standalone (${APP_NAME})"
    cp -f "${BUILDDIR}/${APP_NAME}" "${BIN_DEST}/madmcman-cymbals"
    chmod 755 "${BIN_DEST}/madmcman-cymbals"
elif [ -f "${BUILDDIR}/${APP_NAME}-jack" ]; then
    echo "==> Staging JACK standalone (${APP_NAME}-jack)"
    cp -f "${BUILDDIR}/${APP_NAME}-jack" "${BIN_DEST}/madmcman-cymbals"
    chmod 755 "${BIN_DEST}/madmcman-cymbals"
else
    echo "WARNING: No standalone binary found in ${BUILDDIR}"
fi

# -----------------------------------------------------------------------
# Validate at least one component was staged
# -----------------------------------------------------------------------
STAGED_FILES="$(find "${STAGING}" -not -path "${STAGING}/DEBIAN*" -type f | wc -l)"
if [ "${STAGED_FILES}" -eq 0 ]; then
    echo "ERROR: No plugin files were staged. Check that BUILDDIR is correct: ${BUILDDIR}"
    exit 1
fi

# -----------------------------------------------------------------------
# Set permissions (dpkg-deb requires correct modes)
# -----------------------------------------------------------------------
find "${STAGING}" -type d -exec chmod 755 {} \;
find "${STAGING}" -type f -not -path "${STAGING}/DEBIAN*" -exec chmod 644 {} \;
# Restore executable bit on binaries
find "${BIN_DEST}" -type f -exec chmod 755 {} \; 2>/dev/null || true
find "${STAGING}/usr/lib" -name "*.so" -exec chmod 755 {} \; 2>/dev/null || true
find "${STAGING}/usr/lib" -name "*.clap" -exec chmod 755 {} \; 2>/dev/null || true

# -----------------------------------------------------------------------
# Build .deb
# -----------------------------------------------------------------------
echo "==> Building .deb package"
dpkg-deb --build --root-owner-group "${STAGING}" "${OUTPUT_DIR}/${PKG_FILE}"
echo "==> Created: ${OUTPUT_DIR}/${PKG_FILE}"

#!/usr/bin/env bash
# make_rpm.sh — RPM package builder for Cymbals plugin
#
# Produces:
#   ~/rpmbuild/RPMS/x86_64/madmcman-cymbals-X.Y.Z-1.x86_64.rpm
#
# Usage (from repo root):
#   bash scripts/installer_linux/make_rpm.sh
#
# Override version:
#   bash scripts/installer_linux/make_rpm.sh --version X.Y.Z
#
# Override build dir:
#   BUILDDIR=/path/to/bin bash scripts/installer_linux/make_rpm.sh
#
# Requires: rpm-build (rpmbuild command)
#   sudo dnf install rpm-build    # Fedora/RHEL
#   sudo zypper install rpm-build # openSUSE

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
PKG_RELEASE="1"
PKG_ARCH="x86_64"
APP_NAME="MadMcManBCE"

RPMBUILD_ROOT="${RPMBUILD_ROOT:-${HOME}/rpmbuild}"
SPEC_FILE="${RPMBUILD_ROOT}/SPECS/${PKG_NAME}.spec"

echo "==> Cymbals RPM Package Builder"
echo "    Build dir : ${BUILDDIR}"
echo "    Output dir: ${OUTPUT_DIR}"
echo "    Package   : ${PKG_NAME}-${PKG_VERSION}-${PKG_RELEASE}.${PKG_ARCH}.rpm"

mkdir -p "${OUTPUT_DIR}"

# -----------------------------------------------------------------------
# Set up rpmbuild directory tree
# -----------------------------------------------------------------------
for d in BUILD BUILDROOT RPMS SOURCES SPECS SRPMS; do
    mkdir -p "${RPMBUILD_ROOT}/${d}"
done

# -----------------------------------------------------------------------
# Stage plugin files into BUILD root
# -----------------------------------------------------------------------
STAGE="${RPMBUILD_ROOT}/BUILDROOT/${PKG_NAME}-${PKG_VERSION}-${PKG_RELEASE}.${PKG_ARCH}"
rm -rf "${STAGE}"

if [ -d "${BUILDDIR}/${APP_NAME}.vst3" ]; then
    echo "==> Staging VST3"
    mkdir -p "${STAGE}/usr/lib/vst3/${APP_NAME}.vst3"
    cp -R "${BUILDDIR}/${APP_NAME}.vst3/." "${STAGE}/usr/lib/vst3/${APP_NAME}.vst3/"
fi

if [ -f "${BUILDDIR}/${APP_NAME}.clap" ]; then
    echo "==> Staging CLAP"
    mkdir -p "${STAGE}/usr/lib/clap"
    cp -f "${BUILDDIR}/${APP_NAME}.clap" "${STAGE}/usr/lib/clap/"
fi

mkdir -p "${STAGE}/usr/bin"
if [ -f "${BUILDDIR}/${APP_NAME}" ]; then
    echo "==> Staging standalone"
    cp -f "${BUILDDIR}/${APP_NAME}" "${STAGE}/usr/bin/madmcman-cymbals"
    chmod 755 "${STAGE}/usr/bin/madmcman-cymbals"
elif [ -f "${BUILDDIR}/${APP_NAME}-jack" ]; then
    echo "==> Staging JACK standalone"
    cp -f "${BUILDDIR}/${APP_NAME}-jack" "${STAGE}/usr/bin/madmcman-cymbals"
    chmod 755 "${STAGE}/usr/bin/madmcman-cymbals"
fi

# Build file list for the spec %files section
STAGED_FILES="$(find "${STAGE}" -type f | sed "s|${STAGE}||" | sort)"
if [ -z "${STAGED_FILES}" ]; then
    echo "ERROR: No plugin files staged. Check BUILDDIR: ${BUILDDIR}"
    exit 1
fi

# -----------------------------------------------------------------------
# Write RPM spec file inline
# -----------------------------------------------------------------------
cat > "${SPEC_FILE}" << SPECEOF
Name:           ${PKG_NAME}
Version:        ${PKG_VERSION}
Release:        ${PKG_RELEASE}%{?dist}
Summary:        Cymbals — 3-Stage Linear Synthesis Engine (VST3/CLAP percussion plugin)
License:        GPL-2.0-or-later
URL:            https://example.com
BuildArch:      ${PKG_ARCH}

%description
Cymbals is a metallic percussion synthesizer built on the DISTRHO Plugin Framework.
It provides VST3 and CLAP plugin formats plus a JACK-compatible standalone binary.
Includes the AirWindows processing suite with 478 selectable algorithms.

%install
# Files are already staged in BUILDROOT by the build script
exit 0

%files
%defattr(-,root,root,-)
SPECEOF

# Append each staged file path
while IFS= read -r fpath; do
    echo "${fpath}" >> "${SPEC_FILE}"
done <<< "${STAGED_FILES}"

# Mark directories that need to exist
echo "%dir /usr/lib/vst3/${APP_NAME}.vst3" >> "${SPEC_FILE}" 2>/dev/null || true

# -----------------------------------------------------------------------
# Build RPM
# -----------------------------------------------------------------------
echo "==> Running rpmbuild -bb"
rpmbuild -bb \
    --define "_topdir ${RPMBUILD_ROOT}" \
    --buildroot "${STAGE}" \
    "${SPEC_FILE}"

RPM_OUTPUT="${RPMBUILD_ROOT}/RPMS/${PKG_ARCH}/${PKG_NAME}-${PKG_VERSION}-${PKG_RELEASE}.${PKG_ARCH}.rpm"
echo "==> Created: ${RPM_OUTPUT}"

# Optionally copy to project root for CI artifact upload
if [ -f "${RPM_OUTPUT}" ]; then
    cp -f "${RPM_OUTPUT}" "${OUTPUT_DIR}/"
    echo "==> Copied to: ${OUTPUT_DIR}/$(basename "${RPM_OUTPUT}")"
fi

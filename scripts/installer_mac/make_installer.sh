#!/usr/bin/env bash
# make_installer.sh — macOS installer packaging for Cymbals plugin
#
# Produces:
#   Cymbals_X.Y.Z.pkg  — combined installer package
#   Cymbals_X.Y.Z.dmg  — DMG wrapping the installer
#
# Usage:
#   BUILDDIR=../../build-lvgl/bin bash scripts/installer_mac/make_installer.sh
#
# Override version:
#   bash scripts/installer_mac/make_installer.sh --version X.Y.Z
#
# Requires: pkgbuild, productbuild, hdiutil (all part of macOS Xcode tools)
# No code signing — unsigned distribution only.

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

BUILDDIR="${BUILDDIR:-${ROOT_DIR}/build-lvgl/bin}"
# Version priority: CLI arg > env var > default
VERSION="${VERSION:-${VERSION_OVERRIDE:-0.9.0}}"
APP_NAME="MadMcManBCE"
PKG_BASE="Cymbals_${VERSION}"
WORK_DIR="${WORK_DIR:-${ROOT_DIR}/build-installer-mac}"
OUTPUT_DIR="${OUTPUT_DIR:-${ROOT_DIR}/installer-output}"

echo "==> Cymbals macOS Installer Builder"
echo "    Build dir : ${BUILDDIR}"
echo "    Version   : ${VERSION}"

# Validate build dir
if [ ! -d "${BUILDDIR}" ]; then
    echo "ERROR: Build directory not found: ${BUILDDIR}"
    exit 1
fi

mkdir -p "${WORK_DIR}" "${OUTPUT_DIR}"

# -----------------------------------------------------------------------
# Helper: stage a component payload into a temp root
# -----------------------------------------------------------------------
stage_vst3() {
    local PAYLOAD="${WORK_DIR}/vst3-payload/Library/Audio/Plug-Ins/VST3"
    mkdir -p "${PAYLOAD}"
    cp -R "${BUILDDIR}/${APP_NAME}.vst3" "${PAYLOAD}/"
    echo "${WORK_DIR}/vst3-payload"
}

stage_clap() {
    local PAYLOAD="${WORK_DIR}/clap-payload/Library/Audio/Plug-Ins/CLAP"
    mkdir -p "${PAYLOAD}"
    cp -f "${BUILDDIR}/${APP_NAME}.clap" "${PAYLOAD}/"
    echo "${WORK_DIR}/clap-payload"
}

stage_standalone() {
    local PAYLOAD="${WORK_DIR}/standalone-payload/Applications/MadMcMan"
    mkdir -p "${PAYLOAD}"
    # Accept either a plain binary or a .app bundle
    if [ -d "${BUILDDIR}/${APP_NAME}.app" ]; then
        cp -R "${BUILDDIR}/${APP_NAME}.app" "${PAYLOAD}/"
    elif [ -f "${BUILDDIR}/${APP_NAME}" ]; then
        cp -f "${BUILDDIR}/${APP_NAME}" "${PAYLOAD}/"
    else
        echo "WARNING: No standalone binary found in ${BUILDDIR}, skipping Standalone component"
        echo ""
        return
    fi
    echo "${WORK_DIR}/standalone-payload"
}

# -----------------------------------------------------------------------
# Build sub-packages with pkgbuild (unsigned: no --sign flag)
# -----------------------------------------------------------------------
PKG_PATHS=()
COMPONENTS=()

if [ -d "${BUILDDIR}/${APP_NAME}.vst3" ]; then
    echo "==> Staging VST3 component"
    VST3_ROOT="$(stage_vst3)"
    VST3_PKG="${WORK_DIR}/Cymbals_VST3.pkg"
    pkgbuild \
        --root "${VST3_ROOT}" \
        --identifier "com.madmcman.cymbals.vst3" \
        --version "${VERSION}" \
        --install-location "/" \
        "${VST3_PKG}"
    PKG_PATHS+=("${VST3_PKG}")
    COMPONENTS+=("vst3_choice|VST3 Plugin|com.madmcman.cymbals.vst3|$(basename "${VST3_PKG}")")
fi

if [ -f "${BUILDDIR}/${APP_NAME}.clap" ]; then
    echo "==> Staging CLAP component"
    CLAP_ROOT="$(stage_clap)"
    CLAP_PKG="${WORK_DIR}/Cymbals_CLAP.pkg"
    pkgbuild \
        --root "${CLAP_ROOT}" \
        --identifier "com.madmcman.cymbals.clap" \
        --version "${VERSION}" \
        --install-location "/" \
        "${CLAP_PKG}"
    PKG_PATHS+=("${CLAP_PKG}")
    COMPONENTS+=("clap_choice|CLAP Plugin|com.madmcman.cymbals.clap|$(basename "${CLAP_PKG}")")
fi

STANDALONE_ROOT="$(stage_standalone)"
if [ -n "${STANDALONE_ROOT}" ] && [ -d "${STANDALONE_ROOT}" ]; then
    echo "==> Staging Standalone component"
    STANDALONE_PKG="${WORK_DIR}/Cymbals_Standalone.pkg"
    pkgbuild \
        --root "${STANDALONE_ROOT}" \
        --identifier "com.madmcman.cymbals.standalone" \
        --version "${VERSION}" \
        --install-location "/" \
        "${STANDALONE_PKG}"
    PKG_PATHS+=("${STANDALONE_PKG}")
    COMPONENTS+=("standalone_choice|Standalone Application|com.madmcman.cymbals.standalone|$(basename "${STANDALONE_PKG}")")
fi

if [ ${#PKG_PATHS[@]} -eq 0 ]; then
    echo "ERROR: No components found to package in ${BUILDDIR}"
    exit 1
fi

# -----------------------------------------------------------------------
# Write distribution.xml for productbuild
# -----------------------------------------------------------------------
DIST_XML="${WORK_DIR}/distribution.xml"
choices_outline_lines=()
choice_entries=()
pkg_ref_entries=()

for component in "${COMPONENTS[@]}"; do
    IFS='|' read -r choice_id choice_title pkg_id pkg_file <<< "${component}"
    choices_outline_lines+=("        <line choice=\"${choice_id}\"/>")
    choice_entries+=("    <choice id=\"${choice_id}\" visible=\"true\" title=\"${choice_title}\" selected=\"true\">\n        <pkg-ref id=\"${pkg_id}\">#${pkg_file}</pkg-ref>\n    </choice>")
    pkg_ref_entries+=("    <pkg-ref id=\"${pkg_id}\" version=\"${VERSION}\" onConclusion=\"none\">${pkg_file}</pkg-ref>")
done

cat > "${DIST_XML}" << EOF
<?xml version="1.0" encoding="utf-8"?>
<installer-gui-script minSpecVersion="1">
    <title>Cymbals ${VERSION}</title>
    <welcome file="welcome.html" mime-type="text/html"/>
    <background file="background.png" alignment="bottomleft" scaling="none"/>
    <options customize="always" allow-external-scripts="no"/>
    <choices-outline>
EOF

for line in "${choices_outline_lines[@]}"; do
    printf '%s\n' "${line}" >> "${DIST_XML}"
done

cat >> "${DIST_XML}" << EOF
    </choices-outline>
EOF

for entry in "${choice_entries[@]}"; do
    printf '%b\n' "${entry}" >> "${DIST_XML}"
done

for entry in "${pkg_ref_entries[@]}"; do
    printf '%s\n' "${entry}" >> "${DIST_XML}"
done

cat >> "${DIST_XML}" << EOF
</installer-gui-script>
EOF

# -----------------------------------------------------------------------
# Combine with productbuild (unsigned: no --sign flag)
# -----------------------------------------------------------------------
echo "==> Building combined .pkg with productbuild"
COMBINED_PKG="${OUTPUT_DIR}/${PKG_BASE}.pkg"
productbuild \
    --distribution "${DIST_XML}" \
    --package-path "${WORK_DIR}" \
    "${COMBINED_PKG}"

echo "    Created: ${COMBINED_PKG}"

# -----------------------------------------------------------------------
# Wrap in a DMG with hdiutil
# -----------------------------------------------------------------------
echo "==> Creating DMG"
DMG_STAGING="${WORK_DIR}/dmg-staging"
mkdir -p "${DMG_STAGING}"
cp "${COMBINED_PKG}" "${DMG_STAGING}/"

DMG_PATH="${OUTPUT_DIR}/${PKG_BASE}.dmg"
hdiutil create \
    -volname "Cymbals ${VERSION}" \
    -srcfolder "${DMG_STAGING}" \
    -ov \
    -format UDZO \
    "${DMG_PATH}"

echo "    Created: ${DMG_PATH}"
echo "==> macOS installer packaging complete"

cmake_minimum_required(VERSION 3.15)

if(POLICY CMP0069)
    cmake_policy(SET CMP0069 NEW)
endif()
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# GPLv3 sst-* sources (Surge FX/filters) + MIT fmt. HumpCrusher (AGPLv3) is
# vendored under plugins/Cymbals/src/fx/hump.
# Header-only SIMD helpers; simde supplies SSE emulation on ARM/universal builds.
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/sst-effects/include
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/sst-basic-blocks/include
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/sst-filters/include
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/sst-waveshapers/include
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/fmt/include
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/simde
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/vendored-fx/cloudseedcore
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/vendored-fx/signalsmith-basics/include
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/vendored-fx/signalsmith-dsp/include
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/vendored-fx/signalsmith-stretch
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/vendored-fx/signalsmith-stretch/include
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/vendored-fx/signalsmith-linear/include
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/vendored-fx/soundpipe/h
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/vendored-fx/soundpipe/lib/faust)
add_compile_definitions(FMT_HEADER_ONLY=1 _USE_MATH_DEFINES)
# sst-effects requires C++20 concepts; keep the rest of the plugin on C++17.
set(_surge_fx_options "-std=c++20;-w")
if(NOT (APPLE AND CMAKE_OSX_ARCHITECTURES MATCHES "arm64")
   AND NOT CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
    list(APPEND _surge_fx_options "-msse4.2")
endif()
set_source_files_properties(plugins/Cymbals/src/surge/SurgeFx.cpp PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    COMPILE_OPTIONS "${_surge_fx_options}")

set(NAME MadMcManBCE)
set(PROJECT_VERSION_NUMBER "0.9.0")
project(${NAME} VERSION ${PROJECT_VERSION_NUMBER})

option(CYMBALS_ENABLE_IPO "Enable interprocedural optimization (LTO/IPO) when supported" ON)
option(CYMBALS_BUILD_FAST_MATH "Build with compiler fast-math flags" OFF)
option(CYMBALS_ENABLE_AUDIO_DEBUG "Enable audio debugging WAV dumps and instrumentation logging" OFF)

# Tell LVGL where to find its config
set(LV_BUILD_CONF_PATH "${PROJECT_SOURCE_DIR}/plugins/Cymbals/src/lv_conf.h" CACHE PATH "")

# Optimize LVGL build - disable unused features
set(CONFIG_LV_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CONFIG_LV_BUILD_DEMOS OFF CACHE BOOL "" FORCE)
set(CONFIG_LV_USE_THORVG_INTERNAL OFF CACHE BOOL "" FORCE)
set(LV_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(LV_BUILD_DEMOS OFF CACHE BOOL "" FORCE)

# Include DPF and LVGL
add_subdirectory(deps/DPF)
add_subdirectory(deps/lvgl)

# Cymbals UI kit (knobs/controls) — PROJECT-OWNED at plugins/Cymbals/src/cymbals-ui.
# Must come after the lvgl target.
#
# This kit used to be a foreign sibling checkout (E:/dev/deps/cymbals-ui) that an
# external "mameVst gauntlet" periodically reset to master. That cost a
# configure-time re-apply hack and made CI impossible: the workflow only checks
# out submodules, so ../deps/cymbals-ui never existed on a runner and the
# FATAL_ERROR below failed every tag/nightly configure. It is now tracked
# source in this repo — it ships with checkout, needs no gauntlet, and is
# editable/reviewable in the same commit as the plugin code it serves.
set(CYMBALS_UI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/plugins/Cymbals/src/cymbals-ui")
if(EXISTS "${CYMBALS_UI_DIR}/CMakeLists.txt")
    add_subdirectory(${CYMBALS_UI_DIR} ${CMAKE_BINARY_DIR}/cymbals-ui)
else()
    message(FATAL_ERROR "cymbals-ui kit missing at plugins/Cymbals/src/cymbals-ui - it is project-owned source")
endif()
# LVGL may include ARM-optimized sources in some configurations.
# On x86/x64 hosts (like Windows GitHub runners), exclude ARM-only files.
if (TARGET lvgl)
    # Always drop the known-problematic Helium assembly unit; on some toolchains
    # it pulls C headers into assembler and fails with typedef parse errors.
    get_target_property(_lvgl_sources lvgl SOURCES)
    if (_lvgl_sources)
        list(FILTER _lvgl_sources EXCLUDE REGEX "lv_blend_helium\\.S$")
        set_property(TARGET lvgl PROPERTY SOURCES "${_lvgl_sources}")
    endif()

    string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" _proc)
    if (NOT _proc MATCHES "arm|aarch64")
        get_target_property(_lvgl_sources lvgl SOURCES)
        if (_lvgl_sources)
            list(FILTER _lvgl_sources EXCLUDE REGEX "[\\/](helium|neon)[\\/]")
            set_property(TARGET lvgl PROPERTY SOURCES "${_lvgl_sources}")
        endif()
    endif()
endif()

# --- AirWindows overlay -----------------------------------------------------
# Upstream REMOVED ConsoleX3Buss/Channel/Pre, but our presets reference them and
# the generated winvst wrappers hard-#include their sources. They are vendored
# under deps-overlays/ and copied in here, so the submodule can point at a PURE
# upstream commit: a gitlink to a local-only submodule commit cannot be resolved
# by anyone cloning this repo ("reference is not a tree").
#
# Runs at CONFIGURE time, before anything compiles, so the headers the wrappers
# include are always present. The copy makes the submodule working tree dirty;
# that is expected - it is a generated overlay, not a change to the dependency.
file(GLOB _aw_overlay_dirs
     "${CMAKE_SOURCE_DIR}/deps-overlays/airwindows-ConsoleX3/plugins/WinVST/*")
if(_aw_overlay_dirs)
    foreach(_aw_overlay_dir ${_aw_overlay_dirs})
        file(COPY "${_aw_overlay_dir}"
             DESTINATION "${CMAKE_SOURCE_DIR}/deps/airwindows/plugins/WinVST")
    endforeach()
    message(STATUS "AirWindows overlay applied (${_aw_overlay_dirs})")
endif()

# Every generated winvst wrapper. GLOBed, not hand-listed: the list used to
# be maintained by hand and silently drifted 16 stubs behind the
# generator, leaving those algorithms with undefined factory symbols
# (silent no-ops). Re-run cmake after regenerating to pick up new ones.
file(GLOB CYMBALS_WINVST_SOURCES CONFIGURE_DEPENDS
     "${CMAKE_SOURCE_DIR}/plugins/Cymbals/src/airwindows/winvst/*_winvst.cpp")

# The local DPF working copy carries an optional CLAP parameter-modulation UI
# hook that is not in the pinned upstream commit. Detect the hook from the
# header so the plugin builds against both the pinned submodule and the local
# patched DPF; the display-only override is compiled only when it exists.
file(STRINGS "${CMAKE_SOURCE_DIR}/deps/DPF/distrho/DistrhoUI.hpp"
     CYMBALS_DPF_UI_MODULATION_HOOK
     REGEX "parameterModulationChanged")
if(CYMBALS_DPF_UI_MODULATION_HOOK)
    add_compile_definitions(CYMBALS_DPF_HAS_MODULATION=1)
endif()

# Create plugin with DPF build system
dpf_add_plugin(${NAME}
  TARGETS jack lv2 vst3 clap dssi
  FILES_DSP
      plugins/Cymbals/src/PluginCymbals.cpp
      plugins/Cymbals/src/PluginParameters.cpp
      plugins/Cymbals/src/WaspFilter.cpp
      plugins/Cymbals/src/UnifiedFilter.cpp
      plugins/Cymbals/src/CombFilterBank.cpp
      plugins/Cymbals/src/surge/SurgeFx.cpp
      plugins/Cymbals/src/fx/InternalFxSlot.cpp
      plugins/Cymbals/src/fx/VendoredFxSlot.cpp
      deps/vendored-fx/cloudseedcore/DSP/Biquad.cpp
      deps/vendored-fx/cloudseedcore/DSP/RandomBuffer.cpp
      deps/vendored-fx/soundpipe/modules/phaser.c
      plugins/Cymbals/src/ModulationEngine.cpp
      plugins/Cymbals/src/airwindows/AirSuiteUniversal.cpp
      plugins/Cymbals/src/airwindows/AirwindowsReverbs.cpp
      plugins/Cymbals/src/airwindows/AirwindowsEffects.cpp
      plugins/Cymbals/src/airwindows/AirwindowsAmpSims.cpp
      plugins/Cymbals/src/airwindows/AirwindowsDistortion.cpp
      plugins/Cymbals/src/airwindows/AirwindowsDynamics.cpp
      plugins/Cymbals/src/airwindows/AirwindowsSaturation.cpp
      plugins/Cymbals/src/airwindows/AirwindowsTape.cpp
      plugins/Cymbals/src/airwindows/AirwindowsFilters.cpp
      plugins/Cymbals/src/airwindows/AirwindowsLoFi.cpp
      plugins/Cymbals/src/airwindows/AirwindowsNoise.cpp
      plugins/Cymbals/src/airwindows/AirwindowsSubtlety.cpp
      plugins/Cymbals/src/airwindows/AirwindowsToneColour.cpp
      plugins/Cymbals/src/airwindows/AirwindowsXYZ.cpp
      plugins/Cymbals/src/airwindows/AirwindowsBiquad.cpp
      plugins/Cymbals/src/airwindows/AirwindowsClipping.cpp
      plugins/Cymbals/src/airwindows/AirwindowsDithers.cpp
      plugins/Cymbals/src/airwindows/AirwindowsUtility.cpp
      plugins/Cymbals/src/airwindows/AirwindowsStereo.cpp
      plugins/Cymbals/src/airwindows/AirwindowsBass.cpp
      plugins/Cymbals/src/airwindows/AirwindowsAmbience.cpp
      plugins/Cymbals/src/airwindows/AirwindowsBrightness.cpp
    ${CYMBALS_WINVST_SOURCES}

  FILES_UI
      plugins/Cymbals/src/PluginUI.cpp
      deps/dpf-widgets/generic/LVGL.cpp
)

# Cymbals UI kit — provides knobs/controls (links LVGL transitively)
# UI knobs live in the project-owned kit at plugins/Cymbals/src/cymbals-ui; do not re-add local UIWidgets
if(TARGET cymbals-ui)
    target_link_libraries(${NAME} PRIVATE cymbals-ui)
    if(TARGET ${NAME}-ui)
        target_link_libraries(${NAME}-ui PRIVATE cymbals-ui)
    endif()
endif()

# UI DLL needs to build before the DSP plugin exists; allow missing symbols
# (they will be provided later when the host loads the DSP shared library).
if(WIN32)
    target_link_options(${NAME}-ui PRIVATE -Wl,--allow-shlib-undefined -Wl,--unresolved-symbols=ignore-all)
    # DPF creates per-target UI libs (lv2-ui, vst3 etc) that also need the flag
    foreach(_ui IN ITEMS lv2 vst3 clap jack dssi)
        if(TARGET ${NAME}-${_ui}-ui)
            target_link_options(${NAME}-${_ui}-ui PRIVATE -Wl,--allow-shlib-undefined -Wl,--unresolved-symbols=ignore-all)
        endif()
        if(TARGET ${NAME}-${_ui})
            target_link_options(${NAME}-${_ui} PRIVATE -Wl,--allow-shlib-undefined)
        endif()
    endforeach()
    if(TARGET ${NAME}-lv2)
        target_link_options(${NAME}-lv2 PRIVATE -Wl,--allow-shlib-undefined)
    endif()
endif()

# Setup includes
target_include_directories(${NAME} PUBLIC
    plugins/Cymbals/src
    plugins/Cymbals/src/airwindows/vst2stub
    deps/dpf-widgets/generic
)

# Link LVGL
target_link_libraries(${NAME} PUBLIC lvgl::lvgl $<$<BOOL:${WIN32}>:winmm>)
# LVGL needs PIC
set_target_properties(lvgl PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Platform-specific definitions
if(WIN32)
    target_compile_definitions(${NAME} PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
endif()

if(CYMBALS_ENABLE_IPO)
    if(WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        message(WARNING "IPO/LTO requested but skipped on MinGW due known DPF/LTO linker incompatibilities")
    else()
        include(CheckIPOSupported)
        check_ipo_supported(RESULT cymbals_ipo_supported OUTPUT cymbals_ipo_output LANGUAGES CXX)
        if(cymbals_ipo_supported)
            set_property(TARGET ${NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
            if(TARGET ${NAME}-ui)
                set_property(TARGET ${NAME}-ui PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
            endif()
        else()
            message(WARNING "IPO/LTO disabled: ${cymbals_ipo_output}")
        endif()
    endif()
endif()

if(CYMBALS_BUILD_FAST_MATH)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${NAME} PRIVATE -ffast-math)
        if(TARGET ${NAME}-ui)
            target_compile_options(${NAME}-ui PRIVATE -ffast-math)
        endif()
    endif()
    target_compile_definitions(${NAME} PRIVATE CYMBALS_FAST_MATH_BUILD=1)
endif()

# Enable extended warnings for plugin-owned sources only (not deps)
# -Wno-multichar: AirWindows uses four-char OSType constants ('stuf') by design
# -Wno-cpp: suppress libstdc++ <cstdbool> deprecation notice in C++17 mode
# Note: flags go on ${NAME}-dsp (where AirWindows sources compile) and ${NAME}-ui,
# NOT on ${NAME} (DPF base library) since PRIVATE options don't propagate.
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${NAME}-dsp PRIVATE -Wall -Wextra -Wno-multichar -Wno-cpp -Werror)
    # Suppress -Werror for winvst wrapper sources (they #include upstream AirWindows .cpp files)
    set_source_files_properties(
        plugins/Cymbals/src/fx/VendoredFxSlot.cpp
        deps/vendored-fx/cloudseedcore/DSP/Biquad.cpp
        deps/vendored-fx/cloudseedcore/DSP/RandomBuffer.cpp
        PROPERTIES COMPILE_OPTIONS "-Wno-error"
    )
    set_source_files_properties(
        deps/vendored-fx/soundpipe/modules/phaser.c
        PROPERTIES COMPILE_OPTIONS "-Wno-error"
    )

    file(GLOB _winvst_sources "${CMAKE_SOURCE_DIR}/plugins/Cymbals/src/airwindows/winvst/*_winvst.cpp")
    if(_winvst_sources)
        set_source_files_properties(${_winvst_sources} PROPERTIES COMPILE_OPTIONS "-Wno-error")
    endif()
    if(TARGET ${NAME}-ui)
        target_compile_options(${NAME}-ui PRIVATE -Wall -Wextra -Wno-multichar -Wno-cpp -Werror -Wno-deprecated-declarations -Wno-error=deprecated-declarations)
        # Suppress -Werror for dpf-widgets dep source (submodule; not ours to fix)
        set_source_files_properties(
            ${CMAKE_SOURCE_DIR}/deps/dpf-widgets/generic/LVGL.cpp
            PROPERTIES COMPILE_OPTIONS "-Wno-error"
        )
        # LVGL.cpp's built-in lv_global_default() returns a thread-local pointer
        # that is still null during UI construction (Release strips the assert,
        # so the ctor memsets NULL and crashes). This project uses the
        # process-wide provider in cymbals-ui/src/lv_global_provider.c — exclude
        # the thread-local variant via the guard the file was written for.
        set_source_files_properties(
            ${CMAKE_SOURCE_DIR}/deps/dpf-widgets/generic/LVGL.cpp
            PROPERTIES COMPILE_DEFINITIONS "SNAP_HAVE_LV_GLOBAL_PROVIDER"
        )
        # Compile the provider directly into the UI library: as an object file
        # it precedes every archive on the link line, so it satisfies both the
        # LVGL.cpp references and liblvgl.a internals regardless of static-lib
        # scan order (the archive copy in libcymbals-ui.a would be dropped).
        # CYMBALS_UI_STRONG_GLOBAL strips the weak attribute — MinGW's PE/COFF
        # linker will not resolve references against weak definitions.
        set_source_files_properties(
            ${CYMBALS_UI_DIR}/src/lv_global_provider.c
            PROPERTIES COMPILE_DEFINITIONS "CYMBALS_UI_STRONG_GLOBAL"
        )
        target_sources(${NAME}-ui PRIVATE
            ${CYMBALS_UI_DIR}/src/lv_global_provider.c
        )
    endif()
endif()

if(CYMBALS_ENABLE_AUDIO_DEBUG)
    target_compile_definitions(${NAME} PRIVATE CYMBALS_AUDIO_DEBUG=1)
endif()

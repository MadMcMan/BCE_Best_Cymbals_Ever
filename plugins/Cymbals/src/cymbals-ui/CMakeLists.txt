cmake_minimum_required(VERSION 3.15)
project(cymbals-ui VERSION 0.1.0 LANGUAGES C CXX)

# This kit is PROJECT-OWNED at <repo>/plugins/Cymbals/src/cymbals-ui. The standalone
# fallbacks below used to hardcode E:/dev/cymbals, which only ever resolved on
# one dev box; derive them from this file's own location so a plain checkout
# (including CI) works.
# Walk UP until the plugin tree is found, rather than assuming a fixed depth.
# The kit used to live at <repo>/deps/cymbals-ui (two levels); it is now
# project-owned under plugins/Cymbals/src/cymbals-ui (four levels), and a
# hardcoded ../.. silently pointed at the wrong directory after the move.
set(CYMBALS_REPO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
while(NOT EXISTS "${CYMBALS_REPO_ROOT}/plugins/Cymbals/src")
    get_filename_component(CYMBALS_REPO_ROOT "${CYMBALS_REPO_ROOT}/.." ABSOLUTE)
    if(CYMBALS_REPO_ROOT STREQUAL "/" OR CYMBALS_REPO_ROOT MATCHES "^[A-Za-z]:[\\/]$")
        message(FATAL_ERROR "cymbals-ui: could not locate the repo root above ${CMAKE_CURRENT_SOURCE_DIR}")
    endif()
endwhile()
set(CYMBALS_UI_LV_CONF "${CYMBALS_REPO_ROOT}/plugins/Cymbals/src/lv_conf.h")
set(CYMBALS_UI_LVGL_DIR "${CYMBALS_REPO_ROOT}/deps/lvgl")

# Consumer must provide lvgl target first — if standalone, try fallback.
if(NOT TARGET lvgl)
    # Ensure a local lv_conf.h exists for standalone (copy from Cymbals if needed)
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lv_conf.h" AND EXISTS "${CYMBALS_UI_LV_CONF}")
        file(COPY "${CYMBALS_UI_LV_CONF}" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}")
    endif()
    # Prefer local lv_conf.h (top-level default) — clear any cached PATH that would override it
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lv_conf.h")
        unset(LV_BUILD_CONF_PATH CACHE)
        set(CONFIG_LV_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        set(CONFIG_LV_BUILD_DEMOS OFF CACHE BOOL "" FORCE)
    elseif(NOT LV_BUILD_CONF_PATH AND EXISTS "${CYMBALS_UI_LV_CONF}")
        set(LV_BUILD_CONF_PATH "${CYMBALS_UI_LV_CONF}" CACHE PATH "" FORCE)
    endif()
    # Standalone needs C (LVGL is C) and ASM for its .S helium file (filtered on x86)
    enable_language(C)
    # ASM optional — only needed if the helium .S survives filtering; enable if compiler supports it
    include(CheckLanguage)
    check_language(ASM)
    if(CMAKE_ASM_COMPILER)
        enable_language(ASM)
    endif()
    if(EXISTS "${CYMBALS_UI_LVGL_DIR}/CMakeLists.txt")
        add_subdirectory(${CYMBALS_UI_LVGL_DIR} ${CMAKE_BINARY_DIR}/lvgl_fallback)
    else()
        message(FATAL_ERROR "cymbals-ui: add_subdirectory(lvgl) before cymbals-ui; set LV_BUILD_CONF_PATH before both")
    endif()
endif()

# LVGL may include ARM-optimized sources that fail on x86 hosts (helium asm)
if(TARGET lvgl)
    get_target_property(_lvgl_sources lvgl SOURCES)
    if(_lvgl_sources)
        list(FILTER _lvgl_sources EXCLUDE REGEX "lv_blend_helium\\.S$")
        set_property(TARGET lvgl PROPERTY SOURCES "${_lvgl_sources}")
    endif()
    string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" _proc)
    if(NOT _proc MATCHES "arm|aarch64")
        get_target_property(_lvgl_sources lvgl SOURCES)
        if(_lvgl_sources)
            list(FILTER _lvgl_sources EXCLUDE REGEX "[\\/](helium|neon)[\\/]")
            set_property(TARGET lvgl PROPERTY SOURCES "${_lvgl_sources}")
        endif()
    endif()
endif()

add_library(cymbals-ui STATIC
    src/UIWidgets.cpp
    src/UICommon.cpp
    src/lv_global_provider.c
)

target_include_directories(cymbals-ui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/cymbals-ui
)

# DPF headers for UICommon (DistrhoPlugin.hpp) — consumer normally provides via add_subdirectory(DPF).
# Standalone fallback: derive from this file's location (deps/cymbals-ui -> repo
# root is ../..) instead of hardcoding E:/dev/..., which never resolved off this box.
set(CYMBALS_UI_DPF_DIR "${CYMBALS_REPO_ROOT}/deps/DPF")
if(EXISTS "${CYMBALS_UI_DPF_DIR}/distrho/DistrhoPlugin.hpp")
    target_include_directories(cymbals-ui PUBLIC ${CYMBALS_UI_DPF_DIR} ${CYMBALS_UI_DPF_DIR}/distrho)
endif()
if(EXISTS "${CYMBALS_REPO_ROOT}/plugins/Cymbals/src")
    target_include_directories(cymbals-ui PUBLIC ${CYMBALS_REPO_ROOT}/plugins/Cymbals/src)
endif()

target_link_libraries(cymbals-ui PUBLIC lvgl)
target_compile_features(cymbals-ui PUBLIC cxx_std_17)
set_target_properties(cymbals-ui PROPERTIES POSITION_INDEPENDENT_CODE ON)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(cymbals-ui PRIVATE -Wno-deprecated-declarations -Wno-error=deprecated-declarations)
endif()

# Tests (project-owned, kept local and out of the public tree)
option(CYMBALS_UI_BUILD_TESTS "Build headless knob tests" OFF)
if(CYMBALS_UI_BUILD_TESTS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
    add_subdirectory(tests)
endif()

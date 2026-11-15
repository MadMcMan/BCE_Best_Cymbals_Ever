cmake_minimum_required(VERSION 3.15)

message(FATAL_ERROR
    "Deprecated build path: plugins/Cymbals/CMakeLists.txt is intentionally blocked.\n"
    "Use the canonical repository-root CMake entrypoint instead:\n"
    "  cmake -S . -B build-lvgl -G \"MinGW Makefiles\"\n"
    "  cmake --build build-lvgl --config Release --target MadMcManBCE-vst3 MadMcManBCE-clap MadMcManBCE-jack\n"
)


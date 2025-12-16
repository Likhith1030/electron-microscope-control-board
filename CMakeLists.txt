cmake_minimum_required(VERSION 3.16)
project(ElectronMicroscopeControlBoard VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-g3 -O0)         # full debug info for GDB
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(-O2 -DNDEBUG)
    endif()
endif()

# ── library (shared between simulator and tests) ─────────────────────────────
add_library(emcb_lib STATIC
    src/control_board/MessageParser.cpp
    src/control_board/CommandHandler.cpp
    src/control_board/ControlBoard.cpp
    src/host/CommunicationChannel.cpp
    src/host/HostSystem.cpp
    src/microscope/BeamController.cpp
    src/microscope/StageController.cpp
    src/microscope/VacuumSystem.cpp
    src/microscope/MicroscopeController.cpp
)
target_include_directories(emcb_lib PUBLIC include src)
target_link_libraries(emcb_lib PUBLIC pthread)

# ── main simulator ───────────────────────────────────────────────────────────
add_executable(emcb_simulator src/main.cpp)
target_link_libraries(emcb_simulator PRIVATE emcb_lib)

# ── tests ────────────────────────────────────────────────────────────────────
enable_testing()
add_subdirectory(tests)

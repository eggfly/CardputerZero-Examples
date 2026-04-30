cmake_minimum_required(VERSION 3.16)
project(nes_emulator CXX C)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

include(FetchContent)

# --------------------------------------------------------------------------
# LaiNES — NES core (GPL-3.0; source fetched at build time, not vendored).
#
# Upstream: https://github.com/AndreaOrru/LaiNES
# No tagged releases exist, so we pin the HEAD of the default branch at the
# time this file was written. Update the hash to re-pin.
# --------------------------------------------------------------------------
set(LAINES_GIT_TAG "6b057f9b08a27bce00fa13ac9f77e64f99b65be5"
    CACHE STRING "LaiNES commit to fetch")

FetchContent_Declare(
    laines
    GIT_REPOSITORY https://github.com/AndreaOrru/LaiNES.git
    GIT_TAG        ${LAINES_GIT_TAG}
    GIT_SHALLOW    FALSE
)

FetchContent_GetProperties(laines)
if(NOT laines_POPULATED)
    FetchContent_Populate(laines)
endif()

# LaiNES ships a Makefile only. Build its sources into a static library here,
# excluding the upstream SDL frontend (src/main.cpp and src/gui.cpp) so we can
# substitute our own 320x170-aware frontend.
file(GLOB_RECURSE LAINES_SOURCES CONFIGURE_DEPENDS
    "${laines_SOURCE_DIR}/src/*.cpp"
    "${laines_SOURCE_DIR}/src/mappers/*.cpp")

list(FILTER LAINES_SOURCES EXCLUDE REGEX ".*/src/main\\.cpp$")
list(FILTER LAINES_SOURCES EXCLUDE REGEX ".*/src/gui\\.cpp$")
list(FILTER LAINES_SOURCES EXCLUDE REGEX ".*/src/menu\\.cpp$")

add_library(laines_core STATIC ${LAINES_SOURCES})

target_include_directories(laines_core PUBLIC
    "${laines_SOURCE_DIR}/include"
    "${laines_SOURCE_DIR}/lib/include"
    "${laines_SOURCE_DIR}/lib/Nes_Snd_Emu-0.1.7")

target_compile_options(laines_core PRIVATE -w -fpermissive)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDL2_IMAGE IMPORTED_TARGET SDL2_image)
pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)

target_include_directories(laines_core PUBLIC ${SDL2_INCLUDE_DIRS})
target_link_libraries(laines_core PUBLIC ${SDL2_LIBRARIES})

# --------------------------------------------------------------------------
# Our frontend
# --------------------------------------------------------------------------
add_executable(nes-emulator
    src/main.cpp
    src/viewport.cpp
    src/input.cpp)

target_include_directories(nes-emulator PRIVATE
    src
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_TTF_INCLUDE_DIRS})

target_link_libraries(nes-emulator PRIVATE
    laines_core
    ${SDL2_LIBRARIES}
    ${SDL2_TTF_LIBRARIES}
    pthread)

target_compile_options(nes-emulator PRIVATE -Wall -Wextra -O2)

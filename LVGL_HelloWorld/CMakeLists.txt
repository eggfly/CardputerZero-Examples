cmake_minimum_required(VERSION 3.16)
project(lvgl_hello C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

include(FetchContent)

# Fetch LVGL v9.2.2. Do not vendor the source tree.
FetchContent_Declare(
    lvgl
    GIT_REPOSITORY https://github.com/lvgl/lvgl.git
    GIT_TAG        v9.2.2
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(lvgl)

# Tell LVGL to use our lv_conf.h via simple include.
target_compile_definitions(lvgl PUBLIC
    LV_CONF_INCLUDE_SIMPLE=1
    LV_USE_LINUX_FBDEV=1
    LV_USE_LINUX_EVDEV=1
)

# Make our lv_conf.h visible to LVGL and to this project.
target_include_directories(lvgl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(hello_lvgl src/main.c)

target_include_directories(hello_lvgl PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${lvgl_SOURCE_DIR}
)

# LVGL v9.2 ships a separate linux backend target when built with the above flags.
# Link it if present, otherwise link the core lvgl target only.
set(_lvgl_link_libs lvgl)

if(TARGET lvgl_linux)
    list(APPEND _lvgl_link_libs lvgl_linux)
endif()

if(TARGET lvgl::lvgl_drivers)
    list(APPEND _lvgl_link_libs lvgl::lvgl_drivers)
endif()

target_link_libraries(hello_lvgl PRIVATE ${_lvgl_link_libs} m pthread)

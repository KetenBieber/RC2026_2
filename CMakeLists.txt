cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME R2026_R2)

include("cmake/cmake_func.cmake") # 工具函数

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C CXX ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE

    # Add user defined library search paths
)

file(GLOB_RECURSE USER_C_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/APP/*.c
    ${CMAKE_SOURCE_DIR}/BSP/*.c
    ${CMAKE_SOURCE_DIR}/Module/*.c
)

file(GLOB_RECURSE USER_CPP_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/APP/*.cpp
    ${CMAKE_SOURCE_DIR}/BSP/*.cpp
    ${CMAKE_SOURCE_DIR}/Module/*.cpp
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    ${USER_C_SOURCES}
    ${USER_CPP_SOURCES}

    # Add user sources here
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/Algorithm
    ${CMAKE_SOURCE_DIR}/APP/debug_task
    ${CMAKE_SOURCE_DIR}/APP/robot_com
    ${CMAKE_SOURCE_DIR}/APP/robot_init
    ${CMAKE_SOURCE_DIR}/APP/robot_task
    ${CMAKE_SOURCE_DIR}/APP/chassis_task
    ${CMAKE_SOURCE_DIR}/APP/control_task
    ${CMAKE_SOURCE_DIR}/APP/watchdog_task
    ${CMAKE_SOURCE_DIR}/BSP
    ${CMAKE_SOURCE_DIR}/Module/controller
    ${CMAKE_SOURCE_DIR}/Module/motor
    ${CMAKE_SOURCE_DIR}/Module/topics
    ${CMAKE_SOURCE_DIR}/Module/servo
    ${CMAKE_SOURCE_DIR}/Module/xbox
    ${CMAKE_SOURCE_DIR}/Module/pc
    ${CMAKE_SOURCE_DIR}/Module/watchdog

    # Add user defined include paths
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE

    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)

cmake_minimum_required(VERSION 3.16)
project(tetris LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set source and header directories
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")

# Gather all source files
file(GLOB SOURCES "${SRC_DIR}/*.cpp")

# Add include directories
include_directories(${INCLUDE_DIR})

# Find SDL3, SDL3_image, SDL3_ttf
if(WIN32)
    # On Windows, you may need to set SDL3_DIR, SDL3_IMAGE_DIR, SDL3_TTF_DIR
    find_package(SDL3 REQUIRED)
    find_package(SDL3_image REQUIRED)
    find_package(SDL3_ttf REQUIRED)
else()
    find_package(SDL3 REQUIRED)
    find_package(SDL3_image REQUIRED)
    find_package(SDL3_ttf REQUIRED)
endif()

add_executable(tetris ${SOURCES})

# Link SDL3 libraries
target_link_libraries(tetris
    SDL3::SDL3
    SDL3_image::SDL3_image
    SDL3_ttf::SDL3_ttf
)

# Set include directories for target
target_include_directories(tetris PRIVATE ${INCLUDE_DIR})

# Platform-specific settings (optional)
if(WIN32)
    # Set Windows-specific options here if needed
else()
    # Set Linux-specific options here if needed
endif()
cmake_minimum_required(VERSION 3.21)
project(tetris VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set source and header directories
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")
set(ASSETS_DIR "${CMAKE_SOURCE_DIR}/assets")

option(TETRIS_STATIC_SDL "Link SDL libraries statically on Windows" ON)

# Gather all source files
file(GLOB SOURCES "${SRC_DIR}/*.cpp")

# Add include directories
include_directories(${INCLUDE_DIR})

# Find SDL3, SDL3_image, SDL3_ttf
if(WIN32)
    if(TETRIS_STATIC_SDL)
        set(SDL3_SHARED OFF)
        set(SDL3_IMAGE_SHARED OFF)
        set(SDL3_TTF_SHARED OFF)
        set(PKG_CONFIG_USE_STATIC_LIBS TRUE)

        if(EXISTS "C:/msys64/mingw64/lib/libharfbuzz.a")
            set(harfbuzz_LIBRARY "C:/msys64/mingw64/lib/libharfbuzz.a" CACHE FILEPATH "" FORCE)
        endif()
        if(EXISTS "C:/msys64/mingw64/lib/libfreetype.a")
            set(FREETYPE_LIBRARY_RELEASE "C:/msys64/mingw64/lib/libfreetype.a" CACHE FILEPATH "" FORCE)
        endif()
    endif()

    # On Windows, you may need to set SDL3_DIR, SDL3_IMAGE_DIR, SDL3_TTF_DIR
    find_package(SDL3 REQUIRED)
    find_package(SDL3_image REQUIRED)
    find_package(SDL3_ttf REQUIRED)
else()
    find_package(SDL3 REQUIRED)
    find_package(SDL3_image REQUIRED)
    find_package(SDL3_ttf REQUIRED)
endif()

set(APP_SOURCES ${SOURCES})
if(WIN32)
    list(APPEND APP_SOURCES "${ASSETS_DIR}/app_icon.rc")
    add_executable(tetris WIN32 ${APP_SOURCES})
else()
    add_executable(tetris ${APP_SOURCES})
endif()

# Link SDL3 libraries
if(WIN32 AND TETRIS_STATIC_SDL)
    target_link_libraries(tetris
        SDL3::SDL3-static
        SDL3_image::SDL3_image-static
        SDL3_ttf::SDL3_ttf-static
    )

    # Prefer static GNU runtime libs for a standalone Windows executable.
    if(MINGW)
        target_link_options(tetris PRIVATE -static -static-libgcc -static-libstdc++)
    endif()
else()
    target_link_libraries(tetris
        SDL3::SDL3
        SDL3_image::SDL3_image
        SDL3_ttf::SDL3_ttf
    )
endif()

# Set include directories for target
target_include_directories(tetris PRIVATE ${INCLUDE_DIR})

install(TARGETS tetris
    RUNTIME DESTINATION .
)

if(WIN32)
    if(NOT TETRIS_STATIC_SDL)
        install(FILES $<TARGET_RUNTIME_DLLS:tetris>
            DESTINATION .
        )
    endif()

    set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION .)
    include(InstallRequiredSystemLibraries)

    if(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS)
        install(PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
            DESTINATION .
        )
    endif()

    set(CPACK_GENERATOR "ZIP")
    set(CPACK_PACKAGE_FILE_NAME "tetris-${PROJECT_VERSION}-windows")
    include(CPack)
endif()
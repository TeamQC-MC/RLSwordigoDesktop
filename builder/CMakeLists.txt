cmake_minimum_required(VERSION 3.12)
project(SwordigoDesktop VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find dependencies
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(OpenAL REQUIRED)

# Unicorn emulator library manual search
find_library(UNICORN_LIBRARY NAMES unicorn REQUIRED)
find_path(UNICORN_INCLUDE_DIR NAMES unicorn/unicorn.h REQUIRED)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/../src
    ${SDL2_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
    ${ZLIB_INCLUDE_DIRS}
    ${OPENAL_INCLUDE_DIR}
    ${UNICORN_INCLUDE_DIR}
)

# Source files
set(SOURCES
    ${CMAKE_SOURCE_DIR}/../src/main.cpp
    ${CMAKE_SOURCE_DIR}/../src/jni/jni_marshaller.cpp
    ${CMAKE_SOURCE_DIR}/../src/platform/display.cpp
    ${CMAKE_SOURCE_DIR}/../src/platform/gui.cpp
    ${CMAKE_SOURCE_DIR}/../src/loader/elf_loader.cpp
    ${CMAKE_SOURCE_DIR}/../src/jni/jni_bridge.cpp
    ${CMAKE_SOURCE_DIR}/../src/platform/emulator.cpp
    ${CMAKE_SOURCE_DIR}/../src/android/asset_manager.c
    ${CMAKE_SOURCE_DIR}/../src/android/log.c
)

# Add SWORDIGO_DATA_DIR definition
set(SWORDIGO_DATA_DIR "${CMAKE_INSTALL_PREFIX}/share/swordigo/")
add_compile_definitions(SWORDIGO_DATA_DIR="${SWORDIGO_DATA_DIR}")

# Executable
add_executable(swordigo_boot ${SOURCES})
target_link_libraries(swordigo_boot
    ${SDL2_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${OPENAL_LIBRARY}
    ${UNICORN_LIBRARY}
)

# Installation Rules
install(TARGETS swordigo_boot DESTINATION bin)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/../assets DESTINATION share/swordigo)
install(FILES ${CMAKE_SOURCE_DIR}/../libswordigo.so DESTINATION share/swordigo)
install(FILES ${CMAKE_SOURCE_DIR}/../Swordigo.desktop DESTINATION share/applications)

# Packaging via CPack
set(CPACK_PACKAGE_NAME "swordigo-desktop")
set(CPACK_PACKAGE_VENDOR "QuantumCreeper Labs")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Swordigo Desktop Port")
set(CPACK_PACKAGE_CONTACT "quantumcreeper@gmail.com")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/../LICENSE")

# Debian packaging details
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libsdl2-2.0-0, libopenal1, libgl1-mesa-glx | libgl1, zlib1g, libunicorn2 | libunicorn1")
set(CPACK_DEBIAN_PACKAGE_SECTION "games")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")

# RPM packaging details
set(CPACK_RPM_PACKAGE_REQUIRES "SDL2, openal-soft, mesa-libGL, zlib, unicorn")
set(CPACK_RPM_PACKAGE_LICENSE "MIT")
set(CPACK_RPM_PACKAGE_GROUP "Amusements/Games")

include(CPack)

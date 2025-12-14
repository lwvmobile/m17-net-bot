cmake_minimum_required(VERSION 3.10.2)
project(m17-net-bot)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/")

include(git_revision)
git_describe(GIT_TAG)

#default option is for ESPEAK, bot is coded hierarchially for ESPEAK, ESPEAK-NG, and PIPER1 last

#use cmake option -DUSETTS=OFF to disable TTS
option(USETTS
    "Build with espeak support" ON)
if (USETTS)
    add_definitions(-DESPEAK)
endif ()

#use cmake option -DUSEESPEAKNG=ON
option(USEESPEAKNG
    "Build with espeak-ng support" OFF)
if (USEESPEAKNG)
    add_definitions(-DESPEAKNG)
endif ()

#use cmake option -DUSEPIPER=ON
option(USEPIPER
    "Build with piper1-gpl support" OFF)
if (USEPIPER)
    add_definitions(-DPIPER1)
endif ()

# Find PkgConfig first (required for dependency detection)
find_package(PkgConfig REQUIRED)

# macOS specific configurations
if(APPLE)
    # Add Homebrew paths for macOS
    list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew" "/usr/local")
    
    # Set additional library and include paths for Homebrew packages
    set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "/opt/homebrew/lib" "/usr/local/lib")
    set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "/opt/homebrew/include" "/usr/local/include")
    
    # Ensure we can find pkg-config in Homebrew paths
    find_program(PKG_CONFIG_EXECUTABLE pkg-config PATHS "/opt/homebrew/bin" "/usr/local/bin")
    if(PKG_CONFIG_EXECUTABLE)
        set(PKG_CONFIG_FOUND TRUE)
    endif()
endif()

find_package(LibSndFile REQUIRED) #required packaged need the REQUIRED argument
find_package(CODEC2 REQUIRED)

include_directories(SYSTEM ${LIBSNDFILE_INCLUDE_DIR} ${CODEC2_INCLUDE_DIRS})
set(LIBS ${LIBSNDFILE_LIBRARY} ${CODEC2_LIBRARIES})

FILE(GLOB SRCS src/*.c)
FILE(GLOB HEADERS include/*.h)

configure_file("src/git_ver.c.in" "${CMAKE_CURRENT_BINARY_DIR}/git_ver.c" @ONLY)
list(APPEND SRCS "${CMAKE_CURRENT_BINARY_DIR}/git_ver.c")

include_directories("${PROJECT_SOURCE_DIR}/include")

ADD_EXECUTABLE(m17-net-bot ${SRCS} ${HEADERS})

# macOS specific linking
if(APPLE)
    # Add math library explicitly for macOS
    list(APPEND LIBS m)
    
    # Set RPATH for Homebrew libraries
    set_target_properties(m17-net-bot PROPERTIES
        INSTALL_RPATH "/opt/homebrew/lib;/usr/local/lib"
        BUILD_WITH_INSTALL_RPATH TRUE)
endif()

TARGET_LINK_LIBRARIES(m17-net-bot ${LIBS})

target_compile_options(m17-net-bot PRIVATE -Wunused-but-set-variable -Wall -Wextra -Wpedantic $<$<COMPILE_LANGUAGE:C>:-Wpointer-sign>)

include(GNUInstallDirs)
install(TARGETS m17-net-bot DESTINATION ${CMAKE_INSTALL_BINDIR})

# uninstall target
configure_file(
    "cmake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

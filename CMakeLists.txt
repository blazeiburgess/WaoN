cmake_minimum_required(VERSION 3.10)
project(WaoN VERSION 0.10 LANGUAGES C CXX)

# Include GNUInstallDirs for proper install directories
include(GNUInstallDirs)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")

# Find required packages
find_package(PkgConfig REQUIRED)

# FFTW3
pkg_check_modules(FFTW3 REQUIRED fftw3)

# libsndfile
pkg_check_modules(SNDFILE REQUIRED sndfile)

# libao
pkg_check_modules(AO REQUIRED ao)

# libsamplerate
pkg_check_modules(SAMPLERATE REQUIRED samplerate)

# GTK+2.0 (for gwaon)
pkg_check_modules(GTK2 REQUIRED gtk+-2.0)

# ncurses (for pv)
find_package(Curses REQUIRED)

# Math library
find_library(MATH_LIB m)

# Build options
option(BUILD_WAON "Build waon executable" ON)
option(BUILD_PV "Build pv executable" ON)
option(BUILD_GWAON "Build gwaon executable" ON)
option(BUILD_SHARED_LIB "Build shared library" OFF)
option(BUILD_PYTHON_BINDINGS "Build Python bindings (requires BUILD_SHARED_LIB)" OFF)

# Common source files
set(COMMON_SOURCES
    src/common/fft.c
    src/common/fft.h
    src/common/hc.c
    src/common/hc.h
    src/common/snd.c
    src/common/snd.h
    src/common/cleanup.c
    src/common/cleanup.h
)

# Shared library
if(BUILD_SHARED_LIB)
    add_library(waon SHARED
        src/lib/waon.c
        src/lib/waon.h
        src/waon/notes.c
        src/waon/notes.h
        src/waon/midi.c
        src/waon/midi.h
        src/waon/analyse.c
        src/waon/analyse.h
        ${COMMON_SOURCES}
    )
    
    target_include_directories(waon PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/lib>
        $<INSTALL_INTERFACE:include>
    )
    
    target_include_directories(waon PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/common
        ${CMAKE_CURRENT_SOURCE_DIR}/src/waon
        ${FFTW3_INCLUDE_DIRS}
        ${SNDFILE_INCLUDE_DIRS}
    )
    
    target_compile_options(waon PRIVATE
        ${FFTW3_CFLAGS_OTHER}
        ${SNDFILE_CFLAGS_OTHER}
    )
    
    target_link_libraries(waon
        ${FFTW3_LIBRARIES}
        ${SNDFILE_LIBRARIES}
        ${MATH_LIB}
    )
    
    target_link_directories(waon PRIVATE
        ${FFTW3_LIBRARY_DIRS}
        ${SNDFILE_LIBRARY_DIRS}
    )
    
    set_target_properties(waon PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        PUBLIC_HEADER src/lib/waon.h
    )
    
    # Install library
    install(TARGETS waon
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
endif()

# waon executable
if(BUILD_WAON)
    add_executable(waon-exe
        src/waon/main.c
        src/waon/notes.c
        src/waon/notes.h
        src/waon/midi.c
        src/waon/midi.h
        src/waon/analyse.c
        src/waon/analyse.h
        src/waon/cli.c
        src/waon/cli.h
        src/waon/config.c
        src/waon/config.h
        src/waon/progress.c
        src/waon/progress.h
        ${COMMON_SOURCES}
    )
    
    target_include_directories(waon-exe PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/common
        ${CMAKE_CURRENT_SOURCE_DIR}/src/waon
        ${FFTW3_INCLUDE_DIRS}
        ${SNDFILE_INCLUDE_DIRS}
    )
    
    target_compile_options(waon-exe PRIVATE
        ${FFTW3_CFLAGS_OTHER}
        ${SNDFILE_CFLAGS_OTHER}
    )
    
    target_link_libraries(waon-exe
        ${FFTW3_LIBRARIES}
        ${SNDFILE_LIBRARIES}
        ${MATH_LIB}
    )
    
    target_link_directories(waon-exe PRIVATE
        ${FFTW3_LIBRARY_DIRS}
        ${SNDFILE_LIBRARY_DIRS}
    )
    
    set_target_properties(waon-exe PROPERTIES
        OUTPUT_NAME waon
    )
endif()

# pv executable
if(BUILD_PV)
    add_executable(pv
        src/pv/pv.c
        src/pv/pv-complex.c
        src/pv/pv-complex.h
        src/pv/pv-conventional.c
        src/pv/pv-conventional.h
        src/pv/pv-ellis.c
        src/pv/pv-ellis.h
        src/pv/pv-freq.c
        src/pv/pv-freq.h
        src/pv/pv-loose-lock.c
        src/pv/pv-loose-lock.h
        src/pv/pv-nofft.c
        src/pv/pv-nofft.h
        src/pv/pv-complex-curses.c
        src/pv/pv-complex-curses.h
        src/pv/ao-wrapper.c
        src/pv/ao-wrapper.h
        ${COMMON_SOURCES}
    )
    
    target_include_directories(pv PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/common
        ${CMAKE_CURRENT_SOURCE_DIR}/src/pv
        ${FFTW3_INCLUDE_DIRS}
        ${SNDFILE_INCLUDE_DIRS}
        ${AO_INCLUDE_DIRS}
        ${SAMPLERATE_INCLUDE_DIRS}
        ${CURSES_INCLUDE_DIRS}
    )
    
    target_compile_options(pv PRIVATE
        ${FFTW3_CFLAGS_OTHER}
        ${SNDFILE_CFLAGS_OTHER}
        ${AO_CFLAGS_OTHER}
        ${SAMPLERATE_CFLAGS_OTHER}
    )
    
    target_link_libraries(pv
        ${FFTW3_LIBRARIES}
        ${SNDFILE_LIBRARIES}
        ${AO_LIBRARIES}
        ${SAMPLERATE_LIBRARIES}
        ${CURSES_LIBRARIES}
        ${MATH_LIB}
    )
    
    target_link_directories(pv PRIVATE
        ${FFTW3_LIBRARY_DIRS}
        ${SNDFILE_LIBRARY_DIRS}
        ${AO_LIBRARY_DIRS}
        ${SAMPLERATE_LIBRARY_DIRS}
    )
endif()

# gwaon executable
if(BUILD_GWAON)
    add_executable(gwaon
        src/gwaon/gwaon.c
        src/gwaon/gwaon-menu.c
        src/gwaon/gwaon-menu.h
        src/gwaon/gwaon-about.c
        src/gwaon/gwaon-about.h
        src/gwaon/gwaon-wav.c
        src/gwaon/gwaon-wav.h
        src/gwaon/gwaon-play.c
        src/gwaon/gwaon-play.h
        src/pv/pv-complex.c
        src/pv/pv-complex.h
        src/pv/pv-conventional.c
        src/pv/pv-conventional.h
        src/pv/ao-wrapper.c
        src/pv/ao-wrapper.h
        src/gwaon/gtk-color.c
        src/gwaon/gtk-color.h
        src/waon/midi.c
        src/waon/midi.h
        ${COMMON_SOURCES}
    )
    
    target_include_directories(gwaon PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/common
        ${CMAKE_CURRENT_SOURCE_DIR}/src/waon
        ${CMAKE_CURRENT_SOURCE_DIR}/src/pv
        ${CMAKE_CURRENT_SOURCE_DIR}/src/gwaon
        ${GTK2_INCLUDE_DIRS}
        ${FFTW3_INCLUDE_DIRS}
        ${SNDFILE_INCLUDE_DIRS}
        ${AO_INCLUDE_DIRS}
        ${SAMPLERATE_INCLUDE_DIRS}
    )
    
    target_compile_options(gwaon PRIVATE
        ${GTK2_CFLAGS_OTHER}
        ${FFTW3_CFLAGS_OTHER}
        ${SNDFILE_CFLAGS_OTHER}
        ${AO_CFLAGS_OTHER}
        ${SAMPLERATE_CFLAGS_OTHER}
    )
    
    target_link_libraries(gwaon
        ${GTK2_LIBRARIES}
        ${FFTW3_LIBRARIES}
        ${SNDFILE_LIBRARIES}
        ${AO_LIBRARIES}
        ${SAMPLERATE_LIBRARIES}
        ${MATH_LIB}
    )
    
    target_link_directories(gwaon PRIVATE
        ${GTK2_LIBRARY_DIRS}
        ${FFTW3_LIBRARY_DIRS}
        ${SNDFILE_LIBRARY_DIRS}
        ${AO_LIBRARY_DIRS}
        ${SAMPLERATE_LIBRARY_DIRS}
    )
endif()

# Python bindings
if(BUILD_PYTHON_BINDINGS)
    if(NOT BUILD_SHARED_LIB)
        message(FATAL_ERROR "Python bindings require BUILD_SHARED_LIB=ON")
    endif()
    add_subdirectory(python)
endif()

# Installation
set(INSTALL_TARGETS)
if(BUILD_WAON)
    list(APPEND INSTALL_TARGETS waon-exe)
endif()
if(BUILD_PV)
    list(APPEND INSTALL_TARGETS pv)
endif()
if(BUILD_GWAON)
    list(APPEND INSTALL_TARGETS gwaon)
endif()

if(INSTALL_TARGETS)
    install(TARGETS ${INSTALL_TARGETS}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

install(FILES docs/man/waon.1 docs/man/pv.1 docs/man/gwaon.1
    DESTINATION ${CMAKE_INSTALL_MANDIR}/man1
)

# Print build configuration
message(STATUS "Build configuration:")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Build waon: ${BUILD_WAON}")
message(STATUS "  Build pv: ${BUILD_PV}")
message(STATUS "  Build gwaon: ${BUILD_GWAON}")
message(STATUS "  Build shared library: ${BUILD_SHARED_LIB}")
message(STATUS "  Build Python bindings: ${BUILD_PYTHON_BINDINGS}")
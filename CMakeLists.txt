cmake_minimum_required(VERSION 3.10)
project(WaoN VERSION 0.10 LANGUAGES C)

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

# Common source files
set(COMMON_SOURCES
    fft.c
    fft.h
    hc.c
    hc.h
    snd.c
    snd.h
)

# waon executable
if(BUILD_WAON)
    add_executable(waon
        main.c
        notes.c
        notes.h
        midi.c
        midi.h
        analyse.c
        analyse.h
        ${COMMON_SOURCES}
    )
    
    target_include_directories(waon PRIVATE
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
endif()

# pv executable
if(BUILD_PV)
    add_executable(pv
        pv.c
        pv-complex.c
        pv-complex.h
        pv-conventional.c
        pv-conventional.h
        pv-ellis.c
        pv-ellis.h
        pv-freq.c
        pv-freq.h
        pv-loose-lock.c
        pv-loose-lock.h
        pv-nofft.c
        pv-nofft.h
        pv-complex-curses.c
        pv-complex-curses.h
        ao-wrapper.c
        ao-wrapper.h
        ${COMMON_SOURCES}
    )
    
    target_include_directories(pv PRIVATE
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
        gwaon.c
        gwaon-menu.c
        gwaon-menu.h
        gwaon-about.c
        gwaon-about.h
        gwaon-wav.c
        gwaon-wav.h
        gwaon-play.c
        gwaon-play.h
        pv-complex.c
        pv-complex.h
        pv-conventional.c
        pv-conventional.h
        ao-wrapper.c
        ao-wrapper.h
        gtk-color.c
        gtk-color.h
        midi.c
        midi.h
        ${COMMON_SOURCES}
    )
    
    target_include_directories(gwaon PRIVATE
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

# Installation
install(TARGETS waon pv gwaon
    RUNTIME DESTINATION bin
)

install(FILES waon.1 pv.1 gwaon.1
    DESTINATION share/man/man1
)

# Print build configuration
message(STATUS "Build configuration:")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Build waon: ${BUILD_WAON}")
message(STATUS "  Build pv: ${BUILD_PV}")
message(STATUS "  Build gwaon: ${BUILD_GWAON}")
# Python bindings for WaoN
if(NOT BUILD_SHARED_LIB)
    message(FATAL_ERROR "Python bindings require BUILD_SHARED_LIB=ON")
endif()

# Enable C++ for this directory
enable_language(CXX)

# Find Python and pybind11
find_package(Python COMPONENTS Interpreter Development REQUIRED)
find_package(pybind11 REQUIRED)

# Create Python module
pybind11_add_module(_waon ${CMAKE_CURRENT_SOURCE_DIR}/waon_bindings.cpp)

# Link to the waon library
target_link_libraries(_waon PRIVATE waon)

# On macOS, we need to help the linker find the libraries
if(APPLE)
    # Add link directories from the parent project
    target_link_directories(_waon PRIVATE
        ${FFTW3_LIBRARY_DIRS}
        ${SNDFILE_LIBRARY_DIRS}
    )
    
    # Set RPATH to find libraries at runtime
    set_target_properties(_waon PROPERTIES
        INSTALL_RPATH_USE_LINK_PATH TRUE
        BUILD_WITH_INSTALL_RPATH TRUE
    )
    
    # Add Homebrew library paths to RPATH
    if(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
        set_property(TARGET _waon APPEND PROPERTY
            INSTALL_RPATH "/opt/homebrew/lib"
        )
    else()
        set_property(TARGET _waon APPEND PROPERTY
            INSTALL_RPATH "/usr/local/lib"
        )
    endif()
endif()

# Set properties
set_target_properties(_waon PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/waon
)

# Copy Python files to build directory
configure_file(waon/__init__.py ${CMAKE_CURRENT_BINARY_DIR}/waon/__init__.py COPYONLY)
configure_file(setup.py ${CMAKE_CURRENT_BINARY_DIR}/setup.py @ONLY)

# Installation
# Let setup.py handle the installation for proper PyPI compatibility
# install(TARGETS _waon
#     LIBRARY DESTINATION ${Python_SITELIB}/waon
# )
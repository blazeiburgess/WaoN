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

# Set properties
set_target_properties(_waon PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/waon
)

# Copy Python files to build directory
configure_file(waon/__init__.py ${CMAKE_CURRENT_BINARY_DIR}/waon/__init__.py COPYONLY)
configure_file(setup.py ${CMAKE_CURRENT_BINARY_DIR}/setup.py @ONLY)

# Installation
install(TARGETS _waon
    LIBRARY DESTINATION ${Python_SITELIB}/waon
)
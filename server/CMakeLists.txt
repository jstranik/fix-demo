# CMakeLists.txt

# Specify the minimum version of CMake required
cmake_minimum_required(VERSION 3.10)

# Set the project name and version
project(FIXServer VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(PkgConfig REQUIRED)

pkg_check_modules(QuickFIX REQUIRED quickfix)

# If QuickFIX is not found automatically, you might need to specify its location
# set(QUICKFIX_ROOT "/path/to/quickfix")
# find_package(QuickFIX REQUIRED PATHS ${QUICKFIX_ROOT})

# Add the executable
add_executable(fix-server main.cpp)

# Link the QuickFIX library
target_link_libraries(fix-server PRIVATE QuickFIX::quickfix)

# Include directories for QuickFIX headers
target_include_directories(fix-server PRIVATE ${QUICKFIX_INCLUDE_DIRS})

# Copy the configuration file to the build directory
configure_file(${CMAKE_SOURCE_DIR}/server_settings.cfg
               ${CMAKE_BINARY_DIR}/server_settings.cfg COPYONLY)

# Install the executable
install(TARGETS fix-server DESTINATION bin)

# Install the configuration file
install(FILES ${CMAKE_SOURCE_DIR}/server_settings.cfg
        DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

# Output some status messages
message(STATUS "QuickFIX include dirs: ${QUICKFIX_INCLUDE_DIRS}")
message(STATUS "QuickFIX libraries: ${QUICKFIX_LIBRARIES}")

# CMakeLists.txt

# Specify the minimum version of CMake required
cmake_minimum_required(VERSION 3.10)

# Set the project name and version
project(FIXServer VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

find_package(PkgConfig REQUIRED)

pkg_check_modules(QuickFIX REQUIRED quickfix libxml-2.0 fmt python3)

find_package(Boost COMPONENTS python program_options filesystem REQUIRED)
find_package(Python COMPONENTS Development REQUIRED)

# Add the executable
add_executable(fix-server main.cpp)

# Link the QuickFIX library
target_link_libraries(fix-server PRIVATE ${QuickFIX_LIBRARIES}  ${Boost_LIBRARIES} ${Python_LIBRARIES})
target_link_directories(fix-server PRIVATE ${QuickFIX_LIBRARY_DIRS})

# Include directories for QuickFIX headers
target_include_directories(fix-server PRIVATE ${QuickFIX_INCLUDE_DIRS})

target_compile_definitions(fix-server PRIVATE HAVE_STD_UNIQUE_PTR ${QuickFIX_CLFAGS_OTHER})

# Install the executable
install(TARGETS fix-server DESTINATION bin)

# Output some status messages
message(STATUS "QuickFIX include dirs: ${QuickFIX_INCLUDE_DIRS}")
message(STATUS "QuickFIX libraries: ${QuickFIX_LIBRARIES}")

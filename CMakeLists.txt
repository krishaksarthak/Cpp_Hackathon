cmake_minimum_required(VERSION 3.10)
project(VehicleMonitoringSystem VERSION 1.0 LANGUAGES CXX)

# C++17 Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Find Threads
find_package(Threads REQUIRED)

# Source files
file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
)

# Remove main.cpp from SOURCES for library
list(REMOVE_ITEM SOURCES "${PROJECT_SOURCE_DIR}/src/main.cpp")

# Create library
add_library(VehicleLib STATIC ${SOURCES})
target_link_libraries(VehicleLib PRIVATE Threads::Threads)

# Main executable
add_executable(VehicleMonitoringSystem src/main.cpp)
target_link_libraries(VehicleMonitoringSystem PRIVATE VehicleLib Threads::Threads)

# Tests (optional)
option(BUILD_TESTS "Build unit tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Installation
install(TARGETS VehicleMonitoringSystem DESTINATION bin)
install(DIRECTORY data/ DESTINATION share/vehicle-system/data)
install(DIRECTORY docs/ DESTINATION share/vehicle-system/docs)

# Print build info
message(STATUS "")
message(STATUS "========================================")
message(STATUS "Vehicle Monitoring System - Build Info")
message(STATUS "========================================")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "========================================")
message(STATUS "")
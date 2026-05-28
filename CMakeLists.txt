cmake_minimum_required(VERSION 3.16)
project(VehicleMonitoringSystem VERSION 1.0 LANGUAGES CXX)

# C++17 Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Cross-platform compiler flags
if(MSVC)
    add_compile_options(/W4 /permissive-)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
endif()

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Find Threads
find_package(Threads REQUIRED)

# Source files (explicit listing for reliability)
set(SOURCES
    src/common/Utils.cpp
    src/sensors/Sensor.cpp
    src/sensors/EngineTemperatureSensor.cpp
    src/sensors/BatterySensor.cpp
    src/sensors/SpeedSensor.cpp
    src/sensors/TirePressureSensor.cpp
    src/sensors/DoorSensor.cpp
    src/sensors/SeatbeltSensor.cpp
    src/alerts/Alert.cpp
    src/alerts/AlertManager.cpp
    src/alerts/DTC.cpp
    src/dashboard/Dashboard.cpp
    src/dashboard/VehicleStatistics.cpp
    src/logging/EventLogger.cpp
    src/threading/ThreadManager.cpp
    src/threading/Watchdog.cpp
    src/config/ConfigManager.cpp
    src/config/DriverProfile.cpp
)

# Create library
add_library(VehicleLib STATIC ${SOURCES})
target_link_libraries(VehicleLib PUBLIC Threads::Threads)
target_include_directories(VehicleLib PUBLIC ${PROJECT_SOURCE_DIR}/include)

# Main executable
add_executable(VehicleMonitoringSystem src/main.cpp)
target_link_libraries(VehicleMonitoringSystem PRIVATE VehicleLib Threads::Threads)

# Copy data files to build directory
file(COPY ${PROJECT_SOURCE_DIR}/data DESTINATION ${CMAKE_BINARY_DIR})

# Create logs directory in build
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/logs)

# Tests (optional)
option(BUILD_TESTS "Build unit tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
    add_subdirectory(testcases)
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
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "========================================")
message(STATUS "")
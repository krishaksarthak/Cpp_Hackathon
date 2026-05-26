cmake_minimum_required(VERSION 3.10)

# Test executables
add_executable(test_sensors test_sensors.cpp)
target_link_libraries(test_sensors PRIVATE VehicleLib Threads::Threads)
add_test(NAME SensorTests COMMAND test_sensors)

add_executable(test_alerts test_alerts.cpp)
target_link_libraries(test_alerts PRIVATE VehicleLib Threads::Threads)
add_test(NAME AlertTests COMMAND test_alerts)

add_executable(test_threading test_threading.cpp)
target_link_libraries(test_threading PRIVATE VehicleLib Threads::Threads)
add_test(NAME ThreadingTests COMMAND test_threading)

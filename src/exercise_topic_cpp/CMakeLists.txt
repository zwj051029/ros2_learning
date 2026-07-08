cmake_minimum_required(VERSION 3.8)
project(exercise_topic_cpp)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(base_interfaces REQUIRED)

add_executable(
    battery_pub
    src/battery_pub.cpp
)
ament_target_dependencies(
    battery_pub
    "rclcpp"
    "std_msgs"
)

add_executable(
    battery_sub
    src/battery_sub.cpp
)
ament_target_dependencies(
    battery_sub
    "rclcpp"
    "std_msgs"
)

add_executable(
    sensor_status_pub
    src/sensor_status_pub.cpp
)
ament_target_dependencies(
    sensor_status_pub
    "rclcpp"
    "std_msgs"
    "base_interfaces"
)

add_executable(
    sensor_status_sub
    src/sensor_status_sub.cpp
)
ament_target_dependencies(
    sensor_status_sub
    "rclcpp"
    "std_msgs"
    "base_interfaces"
)

install(TARGETS 
    battery_pub
    battery_sub
    sensor_status_pub
    sensor_status_sub
    DESTINATION lib/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()

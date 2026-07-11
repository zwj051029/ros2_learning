cmake_minimum_required(VERSION 3.8)
project(exercise_service_cpp)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(base_interfaces REQUIRED)

add_executable(robot_mode_server src/robot_mode_server.cpp)
ament_target_dependencies(
  robot_mode_server
  "rclcpp"
  "std_msgs"
  "base_interfaces"
)

add_executable(robot_mode_client src/robot_mode_client.cpp)
ament_target_dependencies(
  robot_mode_client
  "rclcpp"
  "std_msgs"
  "base_interfaces"
)

install(TARGETS 
  robot_mode_server
  robot_mode_client
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

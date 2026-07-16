cmake_minimum_required(VERSION 3.8)
project(exercise_action_cpp)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(base_interfaces REQUIRED)

add_executable(rotate_robot_server src/rotate_robot_server.cpp)
add_executable(rotate_robot_client src/rotate_robot_client.cpp)
target_include_directories(rotate_robot_server PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_features(rotate_robot_server PUBLIC c_std_99 cxx_std_17)  # Require C99 and C++17
ament_target_dependencies(
  rotate_robot_server
  "rclcpp"
  "rclcpp_action"
  "base_interfaces"
)

ament_target_dependencies(
  rotate_robot_client
  "rclcpp"
  "rclcpp_action"
  "base_interfaces"
)

install(TARGETS
  rotate_robot_server
  rotate_robot_client
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

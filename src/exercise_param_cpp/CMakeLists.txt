cmake_minimum_required(VERSION 3.8)
project(exercise_param_cpp)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rcl_interfaces REQUIRED)

add_executable(robot_config_server src/robot_config_server.cpp)
target_compile_features(robot_config_server PUBLIC cxx_std_17)
ament_target_dependencies(
  robot_config_server
  "rclcpp"
  "rcl_interfaces"
)

add_executable(robot_config_client src/robot_config_client.cpp)
target_compile_features(robot_config_client PUBLIC cxx_std_17)
ament_target_dependencies(
  robot_config_client
  "rclcpp"
  "rcl_interfaces"
)

add_executable(navigation_config_server src/navigation_config_server.cpp)
target_compile_features(navigation_config_server PUBLIC cxx_std_17)
ament_target_dependencies(
  navigation_config_server
  "rclcpp"
  "rcl_interfaces"
)

add_executable(navigation_config_client src/navigation_config_client.cpp)
target_compile_features(navigation_config_client PUBLIC cxx_std_17)
ament_target_dependencies(
  navigation_config_client
  "rclcpp"
  "rcl_interfaces"
)

install(TARGETS
  robot_config_server
  robot_config_client
  navigation_config_server
  navigation_config_client
  DESTINATION lib/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()

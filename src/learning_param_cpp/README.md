# ROS 2 参数服务学习示例（C++）

## 功能包说明

`learning_param_cpp` 使用 `rclcpp` 学习 ROS 2 参数服务，演示节点内部的参数声明、删除、修改和查询，以及通过同步参数客户端远程查询和修改其他节点的参数。

ROS 2 参数由节点负责管理，不需要额外定义 `.msg`、`.srv` 或 `.action` 接口。节点启动后，ROS 2 会为参数查询和修改提供内置服务。

## 节点与可执行文件

| 项目 | 名称 |
| --- | --- |
| 参数服务端可执行文件 | `car_param_server` |
| 参数客户端可执行文件 | `car_param_client` |
| 参数服务端节点 | `car_param_server_cpp` |
| 参数客户端节点 | `car_param_client_cpp` |
| C++ 客户端类型 | `rclcpp::SyncParametersClient` |

## 服务端参数

服务端创建时启用：

```cpp
rclcpp::NodeOptions().allow_undeclared_parameters(true)
```

因此除了使用 `declare_parameter()` 声明参数，也可以通过 `set_parameter()` 创建未提前声明的参数。

服务端演示以下车辆参数：

| 参数 | 类型 | 初始值 | 含义 |
| --- | --- | ---: | --- |
| `car_name` | 字符串 | `tiger` | 车辆名称 |
| `car_widths` | 浮点数 | `1.55` | 车辆宽度 |
| `car_wheels` | 整数 | `5` | 车轮数量 |
| `car_heights` | 浮点数 | `2.0` | 车辆高度 |

## 服务端执行流程

服务端启动后依次执行以下操作：

1. 使用 `declare_parameter()` 声明 `car_name`、`car_widths` 和 `car_wheels`。
2. 使用 `set_parameter()` 创建未提前声明的 `car_heights`。
3. 使用 `has_parameter()` 检查 `car_heights` 是否存在。
4. 使用 `undeclare_parameter()` 删除 `car_heights`。
5. 将 `car_widths` 从 `1.55` 修改为 `1.75`。
6. 分别查询单个参数和一组参数。
7. 调用 `rclcpp::spin()`，保持节点及其参数服务持续运行。

服务端完成初始化后的参数状态为：

```text
car_name = tiger
car_widths = 1.75
car_wheels = 5
car_heights 不存在
```

## 客户端执行流程

客户端创建同步参数客户端，并连接节点 `car_param_server_cpp`：

```cpp
std::make_shared<rclcpp::SyncParametersClient>(
    this,
    "car_param_server_cpp"
)
```

连接成功后依次执行：

1. 使用 `get_parameter()` 查询指定参数。
2. 使用 `get_parameters()` 批量查询车辆名称、宽度和车轮数。
3. 使用 `has_parameter()` 检查 `car_name` 和 `car_heights` 是否存在。
4. 使用 `set_parameters()` 批量修改参数，并重新创建 `car_heights`。
5. 再次查询参数，观察修改前后的变化。

客户端提交的修改值为：

```text
car_name = pig
car_widths = 6.66
car_wheels = 6
car_heights = 2.0
```

## 构建

在工作空间根目录执行：

```bash
colcon build --packages-select learning_param_cpp
source install/setup.bash
```

## 运行示例

终端一启动参数服务端：

```bash
ros2 run learning_param_cpp car_param_server
```

终端二启动参数客户端：

```bash
ros2 run learning_param_cpp car_param_client
```

客户端第一次查询应得到：

```text
car_name = tiger
car_widths = 1.75
car_wheels = 5
car_heights 不存在
```

修改后的第二次查询应得到：

```text
car_name = pig
car_widths = 6.66
car_wheels = 6
car_heights 存在
```

## 使用 ROS 2 命令测试

参数服务端运行时，可以查看节点参数列表：

```bash
ros2 param list /car_param_server_cpp
```

查询参数：

```bash
ros2 param get /car_param_server_cpp car_name
ros2 param get /car_param_server_cpp car_widths
ros2 param get /car_param_server_cpp car_wheels
```

修改参数：

```bash
ros2 param set /car_param_server_cpp car_name robot_car
ros2 param set /car_param_server_cpp car_widths 2.5
ros2 param set /car_param_server_cpp car_wheels 4
```

查看参数描述和导出全部参数：

```bash
ros2 param describe /car_param_server_cpp car_name
ros2 param dump /car_param_server_cpp
```

还可以查看节点自动提供的参数服务：

```bash
ros2 service list | grep car_param_server_cpp
```

## CMake 配置要点

1. 使用 `find_package(rclcpp REQUIRED)` 查找 C++ 客户端库。
2. 使用 `add_executable()` 注册参数服务端和参数客户端。
3. 使用 `ament_target_dependencies()` 为两个目标添加 `rclcpp` 依赖。
4. 使用 `install(TARGETS ...)` 将可执行文件安装到 `lib/${PROJECT_NAME}`。
5. 在 `package.xml` 中声明 `rclcpp` 依赖。

## 学习要点

- `declare_parameter()` 与参数的显式声明。
- `set_parameter()`、`set_parameters()` 与参数修改。
- `get_parameter()` 和 `get_parameters()` 的单个、批量查询。
- `has_parameter()` 与 `undeclare_parameter()` 的存在性检查和删除操作。
- `allow_undeclared_parameters` 对未声明参数的影响。
- `rclcpp::SyncParametersClient` 的创建和服务连接等待。
- 节点参数服务与 `ros2 param` 命令之间的关系。

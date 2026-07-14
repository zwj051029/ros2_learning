# ROS 2 服务通信学习示例（C++）

## 功能包说明

`learning_service_cpp` 使用 `rclcpp` 实现两整数求和服务，演示请求、响应、等待服务和异步调用的基本流程。

对应的 Python 实现见 [learning_service_py](../learning_service_py/README.md)。

## 服务接口

接口定义位于 [AddInts.srv](../base_interfaces/srv/AddInts.srv)：

```text
int32 num1
int32 num2
---
int32 sum
```

客户端发送两个整数，服务端计算两数之和并返回结果。

## 节点与服务

| 项目 | 名称 |
| --- | --- |
| 服务名称 | `/sum` |
| 服务类型 | `base_interfaces/srv/AddInts` |
| 服务端可执行文件 | `sum_server` |
| 客户端可执行文件 | `sum_client` |
| 服务端节点 | `sum_server_cpp` |
| 客户端节点 | `sum_client_cpp` |

## 构建

```bash
colcon build --packages-select base_interfaces learning_service_cpp
source install/setup.bash
```

## 运行示例

终端一启动服务端：

```bash
ros2 run learning_service_cpp sum_server
```

终端二发送请求：

```bash
ros2 run learning_service_cpp sum_client 10 234
```

正确结果：

```text
sum = 244
```

客户端必须提供两个参数。客户端会等待 `/sum` 服务上线，再通过 `async_send_request()` 发送请求，并使用 `spin_until_future_complete()` 等待异步结果。

## 使用 ROS 2 命令测试

```bash
ros2 service list -t
ros2 service type /sum
ros2 interface show base_interfaces/srv/AddInts
ros2 service call /sum base_interfaces/srv/AddInts "{num1: 10, num2: 234}"
```

正确响应：

```text
sum: 244
```

还应测试负数和零：

```bash
ros2 service call /sum base_interfaces/srv/AddInts "{num1: -5, num2: 3}"
ros2 service call /sum base_interfaces/srv/AddInts "{num1: 0, num2: 0}"
```

## CMake 配置要点

1. 使用 `find_package(base_interfaces REQUIRED)` 查找自定义服务接口。
2. 使用 `add_executable()` 注册服务端和客户端。
3. 使用 `ament_target_dependencies()` 链接 `rclcpp` 与 `base_interfaces`。
4. 使用 `install(TARGETS ...)` 安装两个可执行文件。
5. 在 `package.xml` 中声明 `rclcpp` 和 `base_interfaces` 依赖。

## 跨语言测试

```bash
# C++ 服务端，Python 客户端
ros2 run learning_service_cpp sum_server
ros2 run learning_service_py sum_client 10 234

# Python 服务端，C++ 客户端
ros2 run learning_service_py sum_server
ros2 run learning_service_cpp sum_client 10 234
```

## 学习要点

- `create_service()` 与服务回调的请求、响应参数。
- `create_client()` 和服务连接等待。
- `async_send_request()` 返回的 Future。
- `spin_until_future_complete()` 对异步响应的处理。
- 客户端参数数量和服务连接失败的处理。

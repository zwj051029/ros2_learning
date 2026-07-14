# ROS 2 服务通信学习示例（Python）

## 功能包说明

`learning_service_py` 使用 `rclpy` 实现两整数求和服务，演示 Python 服务端回调、异步客户端和 Future 结果处理。

对应的 C++ 实现见 [learning_service_cpp](../learning_service_cpp/README.md)。

## 服务接口

接口定义位于 [AddInts.srv](../base_interfaces/srv/AddInts.srv)：

```text
int32 num1
int32 num2
---
int32 sum
```

## 节点与服务

| 项目 | 名称 |
| --- | --- |
| 服务名称 | `/sum` |
| 服务类型 | `base_interfaces/srv/AddInts` |
| 服务端入口 | `sum_server` |
| 客户端入口 | `sum_client` |
| 服务端节点 | `sum_server_py` |
| 客户端节点 | `sum_client_py` |

## 构建

```bash
colcon build --packages-select base_interfaces learning_service_py
source install/setup.bash
```

`setup.py` 中的入口映射为：

```text
sum_server -> learning_service_py.sum_server:main
sum_client -> learning_service_py.sum_client:main
```

## 运行示例

终端一：

```bash
ros2 run learning_service_py sum_server
```

终端二：

```bash
ros2 run learning_service_py sum_client 10 234
```

正确结果：

```text
sum = 244
```

Python 客户端的主要异步流程为：

```text
等待服务上线
创建 AddInts.Request
调用 call_async()
获得 Future
调用 spin_until_future_complete()
通过 future.result() 获取响应
```

## 使用 ROS 2 命令测试

```bash
ros2 service list -t
ros2 service type /sum
ros2 interface show base_interfaces/srv/AddInts
ros2 service call /sum base_interfaces/srv/AddInts "{num1: 10, num2: 234}"
```

还应测试：

```bash
ros2 run learning_service_py sum_client -5 3
ros2 run learning_service_py sum_client 0 0
```

对应结果分别为 `-2` 和 `0`。

## 异常处理

- 命令行参数数量不为两个时，不发送请求。
- 服务未上线时，每秒等待一次并输出连接状态。
- ROS 2 被关闭时结束等待。
- Future 中发生异常时，通过 `try/except` 输出错误信息。

## 跨语言测试

```bash
# Python 服务端，C++ 客户端
ros2 run learning_service_py sum_server
ros2 run learning_service_cpp sum_client 10 234

# C++ 服务端，Python 客户端
ros2 run learning_service_cpp sum_server
ros2 run learning_service_py sum_client 10 234
```

## 学习要点

- `Node.create_service()` 与服务回调返回值。
- `Node.create_client()` 和 `wait_for_service()`。
- `call_async()`、`Future` 与 `future.result()`。
- `spin_until_future_complete()` 的阻塞范围。
- `setup.py` 中 `console_scripts` 的作用。

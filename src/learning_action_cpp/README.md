# ROS 2 动作通信学习示例（C++）

## 功能包说明

`learning_action_cpp` 使用 `rclcpp_action` 实现从 `1` 累加到目标整数 `N` 的长时间任务，覆盖目标处理、连续反馈、最终结果和取消请求。

对应的 Python 实现见 [learning_action_py](../learning_action_py/README.md)。

## 动作接口

接口定义位于 [SumToN.action](../base_interfaces/action/SumToN.action)：

```action
int32 num
---
int64 sum
---
float32 progress
```

| 部分 | 字段 | 含义 |
| --- | --- | --- |
| Goal | `num` | 累加终点 `N` |
| Result | `sum` | `1` 到 `N` 的累加结果 |
| Feedback | `progress` | 当前进度，范围为 `0.0` 到 `1.0` |

## 节点与动作

| 项目 | 名称 |
| --- | --- |
| 动作名称 | `/sum_to_n` |
| 动作类型 | `base_interfaces/action/SumToN` |
| 服务端可执行文件 | `sum_action_server` |
| 客户端可执行文件 | `sum_action_client` |
| 服务端节点 | `sum_to_n_action_server_cpp` |
| 客户端节点 | `sum_to_n_client_cpp` |

## 服务端行为

1. `num <= 1` 时拒绝目标。
2. 合法目标返回 `ACCEPT_AND_EXECUTE`。
3. 从 `1` 开始逐项累加，每秒处理一个数字。
4. 每次累加后发布 `progress = i / num`。
5. 收到取消请求后返回当前部分累加值，并将状态设置为 `CANCELED`。
6. 正常结束后返回最终累加值，并将状态设置为 `SUCCEEDED`。
7. 接受目标后使用独立线程执行任务，避免阻塞 ROS 2 回调处理。

## 构建

```bash
colcon build --packages-select base_interfaces learning_action_cpp
source install/setup.bash
```

## 运行示例

终端一：

```bash
ros2 run learning_action_cpp sum_action_server
```

终端二：

```bash
ros2 run learning_action_cpp sum_action_client 10
```

正确反馈依次为 `10%`、`20%`，直到 `100%`，最终结果为：

```text
sum = 55
```

## 使用 ROS 2 命令测试

```bash
ros2 action list -t
ros2 action info /sum_to_n
ros2 interface show base_interfaces/action/SumToN
ros2 action send_goal /sum_to_n base_interfaces/action/SumToN "{num: 10}" --feedback
```

正确结束状态：

```text
Goal finished with status: SUCCEEDED
```

测试目标拒绝：

```bash
ros2 action send_goal /sum_to_n base_interfaces/action/SumToN "{num: 1}" --feedback
```

正确结果为 `Goal was rejected.`。

测试取消时，可以发送一个较大的目标并在运行期间按 `Ctrl + C`：

```bash
ros2 action send_goal /sum_to_n base_interfaces/action/SumToN "{num: 100}" --feedback
```

正确结束状态为 `CANCELED`，结果中的 `sum` 是取消前已经完成的部分累加值。

## CMake 配置要点

- 查找 `rclcpp`、`rclcpp_action` 和 `base_interfaces`。
- 为服务端和客户端分别注册可执行文件。
- 使用 `ament_target_dependencies()` 链接动作通信依赖。
- 将两个可执行文件安装到 `lib/${PROJECT_NAME}`。

## 跨语言测试

```bash
# C++ 服务端，Python 客户端
ros2 run learning_action_cpp sum_action_server
ros2 run learning_action_py sum_action_client 10

# Python 服务端，C++ 客户端
ros2 run learning_action_py sum_action_server
ros2 run learning_action_cpp sum_action_client 10
```

## 学习要点

- Goal、Result、Feedback 三部分接口。
- 目标响应、反馈响应和结果响应回调。
- `ServerGoalHandle` 与 `ClientGoalHandle`。
- 长时间任务的线程处理。
- `is_canceling()`、`canceled()` 和 `succeed()` 的状态转换。

# ROS 2 动作通信学习示例（Python）

## 功能包说明

`learning_action_py` 使用 `rclpy.action` 实现从 `1` 累加到目标整数 `N` 的长时间任务，覆盖目标处理、连续反馈、最终结果、取消请求和多线程回调调度。

对应的 C++ 实现见 [learning_action_cpp](../learning_action_cpp/README.md)。

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
| 服务端入口 | `sum_action_server` |
| 客户端入口 | `sum_action_client` |
| 服务端节点 | `sum_to_n_action_server_py` |
| 客户端节点 | `sum_to_n_action_client_py` |

## 服务端行为

1. 目标回调拒绝 `num <= 1` 的请求。
2. 取消回调允许客户端取消任务。
3. 执行回调每秒累加一个数字并发布一次进度。
4. 取消后调用 `goal_handle.canceled()`，返回当前部分累加值。
5. 正常完成后调用 `goal_handle.succeed()`，返回最终结果。
6. 使用 `ReentrantCallbackGroup` 允许动作回调并发。
7. 使用 `MultiThreadedExecutor` 在执行任务期间继续处理取消请求。

## 客户端异步流程

```text
等待动作服务端
创建 SumToN.Goal
调用 send_goal_async()
处理目标接受或拒绝
通过 feedback_callback() 处理进度
调用 get_result_async()
处理最终结果
```

客户端使用 `ClientGoalHandle` 类型信息辅助 Pylance 识别 `accepted` 和 `get_result_async()` 等成员。

## 构建

```bash
colcon build --packages-select base_interfaces learning_action_py
source install/setup.bash
```

`setup.py` 中的入口映射为：

```text
sum_action_server -> learning_action_py.sum_action_server:main
sum_action_client -> learning_action_py.sum_action_client:main
```

## 运行示例

终端一：

```bash
ros2 run learning_action_py sum_action_server
```

终端二：

```bash
ros2 run learning_action_py sum_action_client 10
```

正确进度从 `10%` 增加到 `100%`，最终结果为：

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

拒绝非法目标：

```bash
ros2 action send_goal /sum_to_n base_interfaces/action/SumToN "{num: 1}" --feedback
```

取消测试：

```bash
ros2 action send_goal /sum_to_n base_interfaces/action/SumToN "{num: 100}" --feedback
```

运行期间按 `Ctrl + C`，服务端应接受取消，最终状态应为 `CANCELED`。

## 跨语言测试

```bash
# Python 服务端，C++ 客户端
ros2 run learning_action_py sum_action_server
ros2 run learning_action_cpp sum_action_client 10

# C++ 服务端，Python 客户端
ros2 run learning_action_cpp sum_action_server
ros2 run learning_action_py sum_action_client 10
```

## VS Code 自定义接口提示

自定义动作构建后，Python 生成文件通常位于：

```text
install/base_interfaces/local/lib/python3.10/dist-packages
```

如果 `SumToN.Goal`、`SumToN.Result` 或 `SumToN.Feedback` 没有代码提示，应确认工作空间已构建、VS Code 使用 `/usr/bin/python3`，并将上述目录加入 `python.analysis.extraPaths`。

## 学习要点

- `ActionServer` 和 `ActionClient` 的创建。
- `GoalResponse` 与 `CancelResponse`。
- `send_goal_async()` 和多个 Future 回调。
- `goal_handle.is_cancel_requested` 与动作状态转换。
- 可重入回调组与多线程执行器的配合。
- ROS 2 自动生成动作类型的 Python 访问方式。

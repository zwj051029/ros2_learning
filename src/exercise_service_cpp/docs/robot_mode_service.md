# 题目一：机器人运动模式切换服务

## 题目背景

机器人需要根据任务切换待机、巡逻、返航和急停模式。客户端提交目标模式和最大速度，服务端校验请求；合法时更新机器人状态，不合法时保留原状态。

## 自定义服务接口

在 `base_interfaces/srv/SetRobotMode.srv` 中定义：

```text
string mode
float32 max_speed
---
bool success
string current_mode
float32 current_max_speed
string message
```

## 通信设计

| 项目 | 名称 |
| --- | --- |
| 服务名称 | `/set_robot_mode` |
| 服务端可执行文件 | `robot_mode_server` |
| 客户端可执行文件 | `robot_mode_client` |
| 服务类型 | `base_interfaces/srv/SetRobotMode` |

## 初始状态

服务端启动后，机器人的状态为：

```text
current_mode = standby
current_max_speed = 0.0
```

## 模式规则

| 模式 | 最大速度要求 |
| --- | --- |
| `standby` | 必须为 `0.0` |
| `patrol` | `0.0 < max_speed <= 1.0` |
| `return_home` | `0.0 < max_speed <= 0.5` |
| `emergency_stop` | 必须为 `0.0` |

除表中四种模式外，其他字符串全部视为未知模式。

`max_speed` 还必须是有限数值，不能是 `NaN` 或正负无穷。

## 服务端要求

1. 模式和速度都合法时，将请求值保存为当前状态。
2. 成功响应中的 `success` 为 `true`，并返回更新后的模式和速度。
3. 模式或速度不合法时，`success` 为 `false`。
4. 请求失败时不能修改机器人原来的模式和速度。
5. 失败响应仍要返回请求处理前的当前模式和速度。
6. `message` 需要清楚说明成功或失败原因。

## 客户端要求

客户端从命令行读取：

```text
模式 最大速度
```

示例：

```bash
ros2 run exercise_service_cpp robot_mode_client patrol 0.8
```

客户端需要：

1. 检查命令行参数数量。
2. 等待服务端上线，并处理 ROS 2 被关闭的情况。
3. 异步发送请求。
4. 等待响应并打印 `success`、当前模式、当前速度和提示信息。

## 测试要求

至少按顺序测试：

| 请求 | 预期结果 |
| --- | --- |
| `standby 0.0` | 成功 |
| `patrol 0.8` | 成功，当前状态变为 `patrol 0.8` |
| `patrol 1.2` | 失败，状态仍为 `patrol 0.8` |
| `return_home 0.5` | 成功 |
| `return_home 0.6` | 失败，保留原状态 |
| `emergency_stop 0.0` | 成功 |
| `emergency_stop 0.1` | 失败，保留原状态 |
| `unknown 0.0` | 失败，提示未知模式 |

也可以使用命令行直接测试服务端：

```bash
ros2 service call /set_robot_mode base_interfaces/srv/SetRobotMode "{mode: patrol, max_speed: 0.8}"
```

## 验收标准

- 所有模式和速度边界判断正确。
- 合法请求能够更新状态。
- 非法请求不会破坏原状态。
- 客户端能够处理连接、请求和响应。
- C++ 与 Python 客户端、服务端可以交叉通信。

# 题目一：机器人定角度旋转

## 题目背景

移动机器人在导航和巡逻过程中经常需要原地调整朝向。请实现一个定角度旋转动作：客户端提交目标旋转角度和角速度，服务端模拟机器人旋转，持续反馈当前角度、剩余角度和任务进度，并在任务结束后返回最终角度和执行时间。

## 自定义动作接口

在 `base_interfaces/action/RotateRobot.action` 中定义：

```text
float64 target_angle
float64 angular_speed
---
bool success
string message
float64 final_angle
float64 elapsed_time
---
float64 current_angle
float64 remaining_angle
float32 progress
```

字段分为三部分：

| 部分 | 字段 | 含义 |
| --- | --- | --- |
| 目标 | `target_angle` | 目标旋转角度，单位为度；正数和负数表示相反方向 |
| 目标 | `angular_speed` | 旋转角速度，单位为度每秒 |
| 结果 | `success` | 任务是否成功完成 |
| 结果 | `message` | 成功、取消或失败原因 |
| 结果 | `final_angle` | 机器人最终旋转到的角度 |
| 结果 | `elapsed_time` | 已执行时间，单位为秒 |
| 反馈 | `current_angle` | 当前已经旋转的角度 |
| 反馈 | `remaining_angle` | 距离目标还剩余的角度绝对值 |
| 反馈 | `progress` | 当前进度，范围为 `[0.0, 1.0]` |

## 通信设计

| 项目 | 名称 |
| --- | --- |
| 动作名称 | `/rotate_robot` |
| C++ 服务端 | `rotate_robot_server` |
| C++ 客户端 | `rotate_robot_client` |
| Python 服务端 | `rotate_robot_server` |
| Python 客户端 | `rotate_robot_client` |
| 动作类型 | `base_interfaces/action/RotateRobot` |

## 目标校验规则

服务端收到目标后，必须依次检查：

1. `target_angle` 和 `angular_speed` 必须是有限数值，不能是 `NaN` 或正负无穷。
2. `target_angle` 必须位于 `[-180.0, 180.0]`。
3. `abs(target_angle)` 必须大于或等于 `0.001`，过小的旋转目标应被拒绝。
4. `angular_speed` 必须满足 `0.0 < angular_speed <= 90.0`。
5. 任意条件不满足时拒绝目标，全部合法时接受并执行。

## 服务端执行要求

1. 每隔 `0.5` 秒更新一次旋转状态，即反馈频率为 `2 Hz`。
2. 每次理论旋转量为：

```text
angular_speed * 0.5
```

3. 正目标角度反馈正数，负目标角度反馈负数。
4. 每次反馈都需要填写当前角度、剩余角度和进度。
5. 进度按照已旋转角度绝对值除以目标角度绝对值计算。
6. 最后一条反馈必须准确表示目标角度、剩余角度 `0.0` 和进度 `1.0`。
7. 正常完成时调用成功状态，并返回：

```text
success = true
message = 机器人旋转完成
final_angle = target_angle
elapsed_time = 实际执行时间
```

## 取消要求

1. 服务端应当接受合法任务的取消请求。
2. 执行循环每次更新前都要检查任务是否已请求取消。
3. 取消后不能继续增加旋转角度。
4. 取消结果中的 `success` 为 `false`。
5. `final_angle` 和 `elapsed_time` 必须反映取消发生时的实际状态。

## 客户端要求

客户端命令格式为：

```text
目标角度 角速度 [取消进度百分比]
```

不取消任务：

```bash
ros2 run exercise_action_cpp rotate_robot_client 90.0 30.0
ros2 run exercise_action_py rotate_robot_client 90.0 30.0
```

在 `50%` 时请求取消：

```bash
ros2 run exercise_action_cpp rotate_robot_client 120.0 40.0 50
ros2 run exercise_action_py rotate_robot_client 120.0 40.0 50
```

客户端需要：

1. 检查命令行参数数量。
2. 将输入完整转换为有限数值，不能只解析字符串前半部分。
3. 取消进度必须位于 `[0, 100)`。
4. 等待动作服务端上线，并处理连接超时。
5. 异步发送目标并处理接受或拒绝结果。
6. 持续打印反馈中的当前角度、剩余角度和进度。
7. 根据 `SUCCEEDED`、`CANCELED` 和 `ABORTED` 区分最终状态。
8. 打印动作结果中的全部字段。

## 测试要求

建议每组独立测试前重新启动服务端：

| 测试参数 | 预期结果 |
| --- | --- |
| `90.0 30.0` | 成功旋转至 `90.0` 度 |
| `-90.0 30.0` | 成功向反方向旋转至 `-90.0` 度 |
| `180.0 90.0` | 目标角度和角速度上边界均合法 |
| `-180.0 90.0` | 负方向角度边界合法 |
| `0.0 30.0` | 目标角度太小，目标被拒绝 |
| `0.0005 30.0` | 目标角度太小，目标被拒绝 |
| `180.1 30.0` | 目标角度越界，目标被拒绝 |
| `90.0 0.0` | 角速度不是正数，目标被拒绝 |
| `90.0 -1.0` | 角速度不是正数，目标被拒绝 |
| `90.0 90.1` | 角速度超过上限，目标被拒绝 |
| `120.0 40.0 50` | 进度达到 `50%` 后请求取消并返回取消结果 |
| `90.0 30.0 0` | 目标被接受后立即请求取消，不应产生额外旋转 |
| 取消进度 `-1` 或 `100` | 客户端拒绝执行 |
| 任意参数为 `NaN` 或 `Inf` | 客户端或服务端安全拒绝 |

也可以使用命令行直接测试服务端：

```bash
ros2 action send_goal /rotate_robot base_interfaces/action/RotateRobot "{target_angle: 90.0, angular_speed: 30.0}" --feedback
```

## 验收标准

- 目标角度、角速度、有限数和边界判断正确。
- 正负方向的角度反馈符号正确。
- 反馈频率、剩余角度和进度计算正确。
- 最终反馈和结果不会超过目标角度。
- 客户端可以正常处理目标接受、拒绝、反馈、取消和结果。
- C++ 与 Python 客户端、服务端可以交叉通信。

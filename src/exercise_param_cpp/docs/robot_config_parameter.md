# 题目一：移动机器人底盘参数管理

## 题目背景

移动机器人在运行前需要配置名称、运动速度、车轮尺寸和安全停车距离。请实现一个底盘参数服务端，负责声明和校验机器人配置，并周期输出当前状态；C++ 版本还需要实现参数客户端，支持查询和原子修改参数。

## 参数设计

| 参数名称 | 类型 | 默认值 | 含义 |
| --- | --- | --- | --- |
| `robot_name` | string | `"delivery_robot"` | 机器人名称 |
| `max_linear_speed` | double | `1.0` | 最大线速度，单位为 `m/s` |
| `max_angular_speed` | double | `1.5` | 最大角速度，单位为 `rad/s` |
| `wheel_radius` | double | `0.10` | 车轮半径，单位为 `m` |
| `obstacle_stop_distance` | double | `0.50` | 障碍物停车距离，单位为 `m` |
| `safety_enabled` | bool | `true` | 是否启用安全保护 |

## 节点设计

| 项目 | C++ | Python |
| --- | --- | --- |
| 功能包 | `exercise_param_cpp` | `exercise_param_py` |
| 服务端可执行文件 | `robot_config_server` | `robot_config_server` |
| 服务端节点名称 | `/robot_config_server_cpp` | `/robot_config_server_py` |
| 客户端可执行文件 | `robot_config_client` | 使用 `ros2 param` |

本题不需要自定义消息、服务或动作接口。ROS 2 节点声明参数后，会自动提供参数查询和修改服务。

## 参数校验规则

服务端收到参数修改请求时，必须检查：

1. `robot_name` 必须是字符串，不能是空字符串，也不能只包含空格。
2. `max_linear_speed` 必须满足 `0.0 < value <= 2.0`。
3. `max_angular_speed` 必须满足 `0.0 < value <= 4.0`。
4. `wheel_radius` 必须满足 `0.05 <= value <= 0.50`。
5. `obstacle_stop_distance` 必须满足 `0.10 <= value <= 3.00`。
6. `safety_enabled` 必须是布尔值。
7. 所有浮点参数必须是有限数值，不能是 `NaN` 或正负无穷。
8. 参数名称或参数类型不正确时拒绝修改，并返回具体原因。

## 服务端要求

1. 节点启动时声明全部参数。
2. 启动后立即打印一次完整配置。
3. 每隔 `2` 秒重新读取并打印当前配置。
4. 根据最大线速度和车轮半径计算最大车轮转速：

```text
max_wheel_speed = max_linear_speed / wheel_radius
```

5. 参数修改成功后，下一次周期输出必须使用新值。
6. 参数不合法时保持原有参数不变。
7. 按 `Ctrl+C` 后正常销毁节点并关闭 ROS 2。

默认配置对应的最大车轮转速为：

```text
1.0 / 0.10 = 10.0 rad/s
```

## C++ 客户端要求

显示服务端参数列表：

```bash
ros2 run exercise_param_cpp robot_config_client list
```

显示全部机器人参数：

```bash
ros2 run exercise_param_cpp robot_config_client show
```

查询指定参数：

```bash
ros2 run exercise_param_cpp robot_config_client get wheel_radius
```

修改一个参数：

```bash
ros2 run exercise_param_cpp robot_config_client set max_linear_speed 1.5
```

一次修改多个参数：

```bash
ros2 run exercise_param_cpp robot_config_client set \
  robot_name patrol_robot \
  max_linear_speed 1.6 \
  max_angular_speed 2.4 \
  wheel_radius 0.2 \
  obstacle_stop_distance 0.8 \
  safety_enabled false
```

客户端需要：

1. 检查操作名称和命令行参数数量。
2. 完整解析浮点数，拒绝非法字符串、`NaN` 和 `Inf`。
3. 正确创建字符串、浮点数和布尔参数。
4. 等待远程参数服务端上线。
5. 使用原子参数接口修改一组参数。
6. 修改失败时打印服务端返回的原因并返回非零退出码。
7. 禁止在同一条命令中重复设置相同参数。

## Python 测试方式

启动 Python 服务端：

```bash
ros2 run exercise_param_py robot_config_server
```

查看参数列表和参数值：

```bash
ros2 param list /robot_config_server_py
ros2 param get /robot_config_server_py max_linear_speed
```

修改参数：

```bash
ros2 param set /robot_config_server_py max_linear_speed 1.6
ros2 param set /robot_config_server_py wheel_radius 0.2
```

修改成功后，服务端计算出的最大车轮转速应为：

```text
1.6 / 0.2 = 8.0 rad/s
```

## 完整测试顺序

1. 启动服务端，检查六个参数的默认值和 `10.0 rad/s` 计算结果。
2. 使用 `list` 查询参数列表，确认机器人参数全部存在。
3. 使用 `show` 查询完整配置。
4. 使用 `get wheel_radius` 查询单个参数，应返回 `0.10`。
5. 分别将各参数修改为合法值，服务端应接受请求。
6. 将 `robot_name` 设置为空字符串或只包含空格，应被拒绝。
7. 将最大线速度设置为 `0.0`、负数和大于 `2.0` 的数，应被拒绝。
8. 将最大角速度设置为 `0.0`、负数和大于 `4.0` 的数，应被拒绝。
9. 将车轮半径设置为小于 `0.05` 或大于 `0.50`，应被拒绝。
10. 将停车距离设置为小于 `0.10` 或大于 `3.00`，应被拒绝。
11. 向任意浮点参数提交 `NaN` 或 `Inf`，应被拒绝。
12. 使用批量命令同时提交一个合法值和一个非法值，整组参数应保持不变。
13. 输入未知参数名、错误布尔值或重复参数名，客户端应拒绝执行。
14. 服务端未启动时，客户端应持续提示正在连接，并允许通过 `Ctrl+C` 退出。

## 验收标准

- 参数名称、类型、数值范围和有限数值检查完整。
- 服务端能够根据最新参数正确计算最大车轮转速。
- 定时输出不会阻塞参数查询和修改。
- C++ 客户端的查询、单项修改和批量原子修改行为正确。
- 请求失败时不会意外改变任何旧参数。
- Python 服务端可以通过 `ros2 param` 正常访问。

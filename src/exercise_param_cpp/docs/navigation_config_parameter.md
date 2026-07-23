# 题目二：机器人导航配置模式管理

## 题目背景

机器人在待机、室内导航、室外导航和自动对接时需要使用不同的速度与安全配置。请实现一个导航参数服务端：每次修改参数时，将当前配置和请求中的新值合成为完整候选配置，再统一检查模式规则、运动安全和制动距离。C++ 版本还需要实现支持模式切换和原子批量修改的客户端。

## 参数设计

| 参数名称 | 类型 | 默认值 | 含义 |
| --- | --- | --- | --- |
| `navigation_mode` | string | `"standby"` | 当前导航模式 |
| `max_linear_speed` | double | `0.0` | 最大线速度，单位为 `m/s` |
| `max_angular_speed` | double | `0.0` | 最大角速度，单位为 `rad/s` |
| `max_deceleration` | double | `1.0` | 最大减速度，单位为 `m/s²` |
| `obstacle_stop_distance` | double | `0.5` | 障碍物停车距离，单位为 `m` |
| `safety_enabled` | bool | `true` | 是否启用运动安全保护 |

## 节点设计

| 项目 | C++ | Python |
| --- | --- | --- |
| 功能包 | `exercise_param_cpp` | `exercise_param_py` |
| 服务端可执行文件 | `navigation_config_server` | `navigation_config_server` |
| 服务端节点名称 | `/navigation_config_server_cpp` | `/navigation_config_server_py` |
| 客户端可执行文件 | `navigation_config_client` | 使用原子参数服务 |

## 导航模式规则

| 模式 | 参数要求 |
| --- | --- |
| `standby` | `max_linear_speed` 和 `max_angular_speed` 必须都是 `0.0` |
| `indoor` | `0.0 < max_linear_speed <= 0.8`，`0.0 < max_angular_speed <= 1.5` |
| `outdoor` | `0.0 < max_linear_speed <= 2.0`，`0.0 < max_angular_speed <= 2.5` |
| `docking` | `0.0 < max_linear_speed <= 0.2`，`0.0 < max_angular_speed <= 0.5` |

通用规则：

1. 模式只能是 `standby`、`indoor`、`outdoor` 或 `docking`。
2. `max_deceleration` 必须满足 `0.1 <= value <= 3.0`。
3. `obstacle_stop_distance` 必须满足 `0.1 <= value <= 5.0`。
4. 除 `standby` 外，其他模式必须启用 `safety_enabled`。
5. 所有浮点参数必须是有限数值。
6. 参数名称和类型必须正确。

## 制动距离规则

最小安全制动距离按照下面的公式计算：

```text
braking_distance =
    max_linear_speed² / (2 × max_deceleration) + 0.2
```

当模式不是 `standby` 时，必须满足：

```text
obstacle_stop_distance >= braking_distance
```

例如 `indoor` 预设使用：

```text
max_linear_speed = 0.6
max_deceleration = 1.2

braking_distance = 0.6² / (2 × 1.2) + 0.2
                 = 0.35 m
```

预设停车距离为 `0.5 m`，因此配置合法。

## 完整候选配置校验

参数回调收到的可能只是部分参数。服务端不能只检查请求中的单个值，而应该：

1. 读取当前六个参数。
2. 创建一份当前配置的副本。
3. 使用请求中的新值覆盖副本中的对应字段。
4. 对修改后的完整候选配置执行全部规则。
5. 候选配置合法时接受整次请求。
6. 任意规则不满足时拒绝整次请求，所有旧参数保持不变。

例如机器人当前处于 `standby`，直接执行：

```bash
ros2 param set /navigation_config_server_cpp navigation_mode indoor
```

应该失败，因为候选配置中的线速度和角速度仍然为 `0.0`。切换模式时需要通过原子参数接口同时提交模式和相关参数。

## 服务端要求

1. 节点启动时声明全部导航参数。
2. 启动后立即打印一次完整配置。
3. 每隔 `2` 秒读取并打印当前模式、速度、减速度、停车距离、安全状态和制动距离。
4. 合法配置应打印 `config_status = valid`。
5. 参数拒绝日志和响应中需要包含具体原因。
6. 参数修改失败时不能出现部分参数已经改变的情况。
7. 按 `Ctrl+C` 后正常释放节点资源。

## C++ 客户端要求

查看当前配置：

```bash
ros2 run exercise_param_cpp navigation_config_client show
```

切换四种预设模式：

```bash
ros2 run exercise_param_cpp navigation_config_client switch standby
ros2 run exercise_param_cpp navigation_config_client switch indoor
ros2 run exercise_param_cpp navigation_config_client switch outdoor
ros2 run exercise_param_cpp navigation_config_client switch docking
```

客户端使用的预设值：

| 模式 | 线速度 | 角速度 | 最大减速度 | 停车距离 | 安全保护 |
| --- | ---: | ---: | ---: | ---: | --- |
| `standby` | `0.0` | `0.0` | `1.0` | `0.5` | `true` |
| `indoor` | `0.6` | `1.0` | `1.2` | `0.5` | `true` |
| `outdoor` | `1.5` | `2.0` | `1.5` | `1.2` | `true` |
| `docking` | `0.15` | `0.3` | `0.5` | `0.3` | `true` |

原子修改一组参数：

```bash
ros2 run exercise_param_cpp navigation_config_client set \
  navigation_mode indoor \
  max_linear_speed 0.6 \
  max_angular_speed 1.0 \
  max_deceleration 1.2 \
  obstacle_stop_distance 0.5 \
  safety_enabled true
```

恢复默认配置：

```bash
ros2 run exercise_param_cpp navigation_config_client reset
```

客户端需要：

1. 严格检查命令行参数数量和操作类型。
2. 完整解析有限浮点数和布尔值。
3. 禁止在同一请求中重复设置同名参数。
4. 使用 `set_parameters_atomically()` 提交整组参数。
5. 修改成功后重新查询并显示服务端中的最终配置。
6. 修改失败时打印具体原因并返回非零退出码。

## Python 原子修改

Python 版本只实现服务端。切换模式时可以调用节点自动提供的原子参数服务：

```bash
ros2 service call \
  /navigation_config_server_py/set_parameters_atomically \
  rcl_interfaces/srv/SetParametersAtomically \
  "{parameters: [
    {name: navigation_mode, value: {type: 4, string_value: indoor}},
    {name: max_linear_speed, value: {type: 3, double_value: 0.6}},
    {name: max_angular_speed, value: {type: 3, double_value: 1.0}},
    {name: max_deceleration, value: {type: 3, double_value: 1.2}},
    {name: obstacle_stop_distance, value: {type: 3, double_value: 0.5}},
    {name: safety_enabled, value: {type: 1, bool_value: true}}
  ]}"
```

参数类型编号来自 `rcl_interfaces/msg/ParameterType`：

```text
1 = bool
3 = double
4 = string
```

响应中的 `successful = true` 表示整组配置修改成功。

## 完整测试顺序

1. 启动服务端，默认模式应为 `standby`，制动距离为 `0.20 m`。
2. 使用 `show` 查询默认配置。
3. 依次切换 `indoor`、`outdoor`、`docking` 和 `standby`，均应成功。
4. 使用 `switch unknown`，客户端应拒绝未知预设。
5. 使用 `set navigation_mode unknown`，服务端应拒绝未知模式。
6. 在 `indoor` 模式提交大于 `0.8` 的线速度或大于 `1.5` 的角速度，应失败。
7. 在 `outdoor` 模式提交大于 `2.0` 的线速度或大于 `2.5` 的角速度，应失败。
8. 在 `docking` 模式提交大于 `0.2` 的线速度或大于 `0.5` 的角速度，应失败。
9. 在 `standby` 模式提交非零速度，应失败。
10. 将最大减速度设置为小于 `0.1` 或大于 `3.0`，应失败。
11. 将停车距离设置为小于 `0.1` 或大于 `5.0`，应失败。
12. 运动模式下关闭 `safety_enabled`，应失败。
13. 提交合法范围内但小于计算制动距离的停车距离，应失败。
14. 向任意浮点参数提交 `NaN` 或 `Inf`，应失败。
15. 原子请求同时包含一个合法修改和一个非法修改，确认所有旧参数都保持不变。
16. 执行 `reset`，确认全部参数恢复默认值。
17. 服务端未运行时，C++ 客户端应等待连接并允许通过 `Ctrl+C` 退出。

## 验收标准

- 四种导航模式的速度边界判断正确。
- 参数类型、数值范围和有限数值检查完整。
- 服务端能够基于完整候选配置判断参数之间的关联关系。
- 制动距离计算和停车距离判断正确。
- 运动模式下不能关闭安全保护。
- 原子修改失败时不会产生部分参数更新。
- C++ 客户端可以完成查询、模式切换、自定义修改和重置。
- Python 服务端可以通过内置原子参数服务完成整组配置切换。

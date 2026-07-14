# 题目二：机器人传感器健康监控

## 题目背景

机器人通常同时使用激光雷达、相机、IMU、超声波和 GPS。请使用自定义消息循环发布多组传感器数据，由订阅方按优先级判断健康状态，并定期输出统计报告。

## 自定义消息

在 `base_interfaces/msg/SensorStatus.msg` 中定义：

```text
string sensor_name
float32 temperature
float32 voltage
bool is_online
```

字段含义：

| 字段 | 含义 |
| --- | --- |
| `sensor_name` | 传感器名称 |
| `temperature` | 当前温度，单位为摄氏度 |
| `voltage` | 当前工作电压，单位为伏特 |
| `is_online` | 传感器是否在线 |

## 通信设计

| 节点 | 可执行文件 | 发布或订阅 | 话题 | 消息类型 |
| --- | --- | --- | --- | --- |
| 状态发布方 | `sensor_status_pub` | 发布 | `/sensor_status` | `base_interfaces/msg/SensorStatus` |
| 健康监控方 | `sensor_status_sub` | 订阅 | `/sensor_status` | `base_interfaces/msg/SensorStatus` |

## 发布方要求

每隔 `1` 秒发布一条数据，并按下面的顺序循环：

| 传感器 | 温度 | 电压 | 是否在线 |
| --- | ---: | ---: | --- |
| `lidar` | `45.0` | `12.0` | `true` |
| `camera` | `62.5` | `5.0` | `true` |
| `imu` | `38.0` | `3.3` | `true` |
| `ultrasonic` | `35.0` | `2.7` | `true` |
| `gps` | `40.0` | `0.0` | `false` |

发布完 `gps` 后回到 `lidar`，继续循环。日志需要显示传感器名称、温度、电压和在线状态。

## 订阅方判断规则

每条消息只能归入一种状态，必须严格按照下面的优先级判断：

1. `is_online == false`：`OFFLINE`，使用 `ERROR` 日志。
2. 在线且 `temperature >= 60.0`：`OVERHEAT`，使用 `WARN` 日志。
3. 在线、未过热且 `voltage < 3.0`：`LOW_VOLTAGE`，使用 `WARN` 日志。
4. 其他情况：`NORMAL`，使用 `INFO` 日志。

离线判断优先级最高。例如 `gps` 的电压为 `0.0`，但它应当被统计为 `OFFLINE`，不能同时统计为 `LOW_VOLTAGE`。

## 统计要求

订阅方维护以下计数：

```text
总消息数
正常数量
离线数量
过热数量
低电压数量
```

每接收 `5` 条消息打印一次统计报告，然后将本轮计数全部清零，开始下一轮统计。

对于题目给出的五条数据，每轮正确结果为：

```text
总消息数: 5
正常: 2
离线: 1
过热: 1
低电压: 1
```

## 测试要求

```bash
ros2 run exercise_topic_cpp sensor_status_pub
ros2 run exercise_topic_cpp sensor_status_sub
ros2 topic echo /sensor_status
```

至少连续观察两轮数据，确认：

1. 发布顺序可以循环。
2. 五种数据的状态判断正确。
3. 每五条消息只打印一次统计报告。
4. 第二轮统计不会继续累加第一轮的计数。
5. `gps` 只统计为离线。

## 验收标准

- 自定义消息可以正常生成并被 C++ 节点引用。
- 发布方使用容器保存数据，并能安全地循环访问。
- 订阅方的状态判断优先级正确。
- 日志级别与传感器状态匹配。
- 每轮统计数量之和等于总消息数。

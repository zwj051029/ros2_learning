# 题目一：机器人电池状态监控

## 题目背景

机器人运行时需要持续监控剩余电量。请使用 ROS 2 原生消息实现电量发布、状态判断和状态转发，不使用自定义消息。

## 通信设计

| 节点 | 可执行文件 | 发布或订阅 | 话题 | 消息类型 |
| --- | --- | --- | --- | --- |
| 电量发布方 | `battery_pub` | 发布 | `/battery_level` | `std_msgs/msg/Float32` |
| 电量监控方 | `battery_sub` | 订阅 | `/battery_level` | `std_msgs/msg/Float32` |
| 电量监控方 | `battery_sub` | 发布 | `/battery_status` | `std_msgs/msg/String` |

建议节点名称：

```text
battery_publisher_cpp
battery_subscriber_cpp
```

## 发布方要求

1. 初始电量为 `100.0`。
2. 每隔 `0.5` 秒发布一次当前电量。
3. 每次发布后将电量减少 `1.5`。
4. 当电量小于 `0.0` 时，将电量重新设置为 `100.0`，继续循环发布。
5. 使用日志打印每次发布的电量，保留一位小数。

发布序列应类似：

```text
100.0, 98.5, 97.0, 95.5, ...
```

## 订阅方要求

订阅 `/battery_level` 后，根据电量判断机器人电池状态：

| 电量范围 | 状态 | 日志级别 | 发布到 `/battery_status` 的内容 |
| --- | --- | --- | --- |
| `battery >= 60.0` | 电量正常 | `INFO` | `NORMAL` |
| `30.0 <= battery < 60.0` | 电量偏低 | `WARN` | `LOW` |
| `battery < 30.0` | 电量严重不足 | `ERROR` | `CRITICAL` |

每收到一条电量消息，都必须完成一次状态判断，并将对应字符串发布到 `/battery_status`。

## 测试要求

启动发布方和订阅方：

```bash
ros2 run exercise_topic_cpp battery_pub
ros2 run exercise_topic_cpp battery_sub
```

检查两个话题：

```bash
ros2 topic echo /battery_level
ros2 topic echo /battery_status
```

至少验证以下情况：

1. `100.0` 对应 `NORMAL`。
2. `60.0` 对应 `NORMAL`。
3. `59.9` 对应 `LOW`。
4. `30.0` 对应 `LOW`。
5. `29.9` 对应 `CRITICAL`。
6. 电量小于零后能够回到 `100.0`。

## 验收标准

- 发布频率和电量变化正确。
- 三个电量区间的边界判断正确。
- 日志级别与状态匹配。
- `/battery_level` 与 `/battery_status` 可以通过 ROS 2 命令正常查看。
- 发布方与订阅方能够独立启动和退出。

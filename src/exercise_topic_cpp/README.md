# ROS 2 话题通信练习

本功能包保存 ROS 2 Humble 话题通信的 C++ 练习代码，题目文档统一放在 `docs` 目录中。

## 题目列表

| 序号 | 题目 | 通信接口 | 难度 |
| --- | --- | --- | --- |
| 1 | [机器人电池状态监控](docs/battery_status_topic.md) | `std_msgs/Float32`、`std_msgs/String` | 基础进阶 |
| 2 | [机器人传感器健康监控](docs/sensor_health_monitor_topic.md) | `base_interfaces/SensorStatus` | 自定义消息进阶 |

## 构建

```bash
colcon build --packages-select base_interfaces exercise_topic_cpp
source install/setup.bash
```

每道题的节点名称、话题名称、处理规则和测试要求参见对应文档。

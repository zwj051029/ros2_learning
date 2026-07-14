# ROS 2 服务通信练习

本功能包保存 ROS 2 Humble 服务通信的 C++ 练习代码，题目文档统一放在 `docs` 目录中。

## 题目列表

| 序号 | 题目 | 服务接口 | 难度 |
| --- | --- | --- | --- |
| 1 | [机器人运动模式切换](docs/robot_mode_service.md) | `base_interfaces/SetRobotMode` | 基础进阶 |
| 2 | [机器人巡逻航点管理](docs/waypoint_manager_service.md) | `base_interfaces/ManageWaypoint` | 综合进阶 |

## 构建

```bash
colcon build --packages-select base_interfaces exercise_service_cpp
source install/setup.bash
```

每道题的服务接口、业务规则、客户端参数和测试要求参见对应文档。

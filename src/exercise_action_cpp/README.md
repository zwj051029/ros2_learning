# ROS 2 动作通信练习

本功能包保存 ROS 2 Humble 动作通信的 C++ 练习代码，Python 版本位于 `exercise_action_py`，两种语言共用同一组题目要求。题目文档统一放在 `docs` 目录中。

## 题目列表

| 序号 | 题目 | 动作接口 | 难度 |
| --- | --- | --- | --- |
| 1 | [机器人定角度旋转](docs/rotate_robot_action.md) | `base_interfaces/action/RotateRobot` | 基础进阶 |
| 2 | [二维移动机器人目标点导航](docs/move_to_point_action.md) | `base_interfaces/action/MoveToPoint` | 综合进阶 |

## 构建

同时构建自定义接口、C++ 功能包和 Python 功能包：

```bash
colcon build --packages-select base_interfaces exercise_action_cpp exercise_action_py
source install/setup.bash
```

每道题的动作接口、目标校验、执行反馈、取消规则和测试要求参见对应文档。

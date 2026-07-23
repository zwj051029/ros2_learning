# ROS 2 参数服务练习

本功能包保存 ROS 2 Humble 参数服务的 C++ 练习代码，Python 版本位于 `exercise_param_py`。由于当前 Python 环境没有单独使用的参数客户端 API，Python 部分只实现服务端，并使用 ROS 2 命令访问参数。题目文档统一放在 `docs` 目录中。

## 题目列表

| 序号 | 题目 | 参数通信方式 | 难度 |
| --- | --- | --- | --- |
| 1 | [移动机器人底盘参数管理](docs/robot_config_parameter.md) | ROS 2 内置参数服务 | 基础进阶 |
| 2 | [机器人导航配置模式管理](docs/navigation_config_parameter.md) | ROS 2 原子参数服务 | 综合进阶 |

## 构建

同时构建 C++ 和 Python 参数练习功能包：

```bash
colcon build --packages-select exercise_param_cpp exercise_param_py
source install/setup.bash
```

每道题的参数定义、合法性规则、客户端命令和完整测试要求参见对应文档。

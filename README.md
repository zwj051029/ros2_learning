# ROS 2 Learning

基于 **Ubuntu 22.04** 与 **ROS 2 Humble** 的通信机制学习仓库，使用 C++ 和 Python 实现话题、服务、动作与参数四类核心通信，并通过机器人应用场景完成进阶练习。

> 项目状态：通信机制学习阶段已完成，仓库进入归档维护状态。

## 项目目标

- 使用 `rclcpp` 与 `rclpy` 对照实现相同行为，理解两种客户端库的 API 差异。
- 集中维护自定义消息、服务和动作接口，建立清晰的跨功能包契约。
- 从最小学习案例逐步过渡到电池监控、航点管理、机器人旋转和导航配置等机器人场景。
- 保留可复现的构建命令、测试方法、练习要求与知识总结，便于后续复习。

## 技术栈

| 类别 | 版本或工具 |
| --- | --- |
| 操作系统 | Ubuntu 22.04 LTS |
| ROS 2 | Humble Hawksbill |
| C++ 客户端库 | `rclcpp`、`rclcpp_action` |
| Python 客户端库 | `rclpy`、`rclpy.action` |
| 构建系统 | `colcon`、`ament_cmake`、`ament_python` |
| 接口生成 | `rosidl_default_generators` |
| Python | 3.10 |
| 代码格式 | `.clang-format`、4 空格缩进 |

## 仓库结构

```text
ros2_learning/
├── docs/                         # 四大通信机制复习笔记
├── src/
│   ├── base_interfaces/          # 自定义 msg、srv、action 接口
│   ├── learning_*_cpp/           # C++ 最小学习案例
│   ├── learning_*_py/            # Python 最小学习案例
│   ├── exercise_*_cpp/           # C++ 机器人场景练习
│   ├── exercise_*_py/            # Python 机器人场景练习
│   ├── learning_names_cpp/       # 节点名称与重映射
│   ├── learning_names_py/
│   ├── learning_time_cpp/        # Rate、Time、Duration
│   └── learning_time_py/
├── .clang-format
├── .gitignore
└── README.md
```

`build/`、`install/` 和 `log/` 为 `colcon` 生成目录，不属于源码。

## 功能包总览

### 基础学习

| 模块 | C++ 功能包 | Python 功能包 | 内容 |
| --- | --- | --- | --- |
| 话题通信 | `learning_topic_cpp` | `learning_topic_py` | 字符串消息、`Student` 自定义消息的发布与订阅 |
| 服务通信 | `learning_service_cpp` | `learning_service_py` | 两整数求和服务、异步客户端与 Future |
| 动作通信 | `learning_action_cpp` | `learning_action_py` | 目标处理、连续反馈、取消任务与最终结果 |
| 参数服务 | `learning_param_cpp` | `learning_param_py` | 参数声明、查询、修改、删除与远程访问 |
| 名称管理 | `learning_names_cpp` | `learning_names_py` | 节点名称、命名空间与重映射 |
| 时间 API | `learning_time_cpp` | `learning_time_py` | `Rate`、`Time`、`Duration` 及其运算 |

### 机器人场景练习

| 模块 | C++ 功能包 | Python 功能包 | 练习内容 |
| --- | --- | --- | --- |
| 话题通信 | `exercise_topic_cpp` | `exercise_topic_py` | 电池状态监控、传感器健康监控 |
| 服务通信 | `exercise_service_cpp` | `exercise_service_py` | 机器人模式切换、巡逻航点管理 |
| 动作通信 | `exercise_action_cpp` | `exercise_action_py` | 定角度旋转、二维目标点移动 |
| 参数服务 | `exercise_param_cpp` | `exercise_param_py` | 底盘参数管理、导航配置模式管理 |

Python 参数练习只实现参数服务端，远程访问使用 `ros2 param` 命令完成。

## 自定义接口

所有自定义接口由 `base_interfaces` 统一生成，C++ 与 Python 功能包共享同一套类型定义。

| 类型 | 接口 | 用途 |
| --- | --- | --- |
| Message | `Student.msg` | 学生基本信息话题 |
| Message | `SensorStatus.msg` | 传感器温度、电压与在线状态 |
| Service | `AddInts.srv` | 两整数求和 |
| Service | `SetRobotMode.srv` | 设置机器人运行模式和最大速度 |
| Service | `ManageWaypoint.srv` | 添加、删除、查询与列出巡逻航点 |
| Action | `SumToN.action` | 累加计算与进度反馈 |
| Action | `RotateRobot.action` | 机器人定角度旋转 |
| Action | `MoveToPoint.action` | 二维目标点移动 |

查看接口定义：

```bash
ros2 interface show base_interfaces/msg/SensorStatus
ros2 interface show base_interfaces/srv/ManageWaypoint
ros2 interface show base_interfaces/action/MoveToPoint
```

## 快速开始

### 1. 获取代码

```bash
git clone https://github.com/zwj051029/ros2_learning.git
cd ros2_learning
```

### 2. 加载 ROS 2 环境

```bash
source /opt/ros/humble/setup.bash
```

### 3. 安装依赖

确保本机已经完成 `rosdep` 初始化，然后执行：

```bash
rosdep install --from-paths src --ignore-src --rosdistro humble -r -y
```

### 4. 构建工作空间

```bash
colcon build --symlink-install
source install/setup.bash
```

每次打开新终端，都需要重新加载基础环境和当前工作空间：

```bash
source /opt/ros/humble/setup.bash
source ~/ros2_learning/install/setup.bash
```

只构建指定案例及其依赖：

```bash
colcon build --packages-up-to learning_action_cpp learning_action_py
```

## 运行示例

以下示例均需要在每个终端中执行 `source install/setup.bash`。

### 话题：C++ 发布，Python 订阅

终端一：

```bash
ros2 run learning_topic_cpp cpp_string_pub
```

终端二：

```bash
ros2 run learning_topic_py py_string_sub
```

### 服务：Python 服务端，C++ 客户端

终端一：

```bash
ros2 run learning_service_py sum_server
```

终端二：

```bash
ros2 run learning_service_cpp sum_client 10 234
```

预期结果：

```text
sum = 244
```

### 动作：C++ 服务端，Python 客户端

终端一：

```bash
ros2 run learning_action_cpp sum_action_server
```

终端二：

```bash
ros2 run learning_action_py sum_action_client 10
```

客户端将持续收到进度反馈，最终结果为 `55`。

### 参数：C++ 服务端与命令行客户端

终端一：

```bash
ros2 run learning_param_cpp car_param_server
```

终端二：

```bash
ros2 param list /car_param_server_cpp
ros2 param get /car_param_server_cpp car_name
ros2 param set /car_param_server_cpp car_widths 2.5
```

## 文档索引

### 知识总结

- [话题通信](docs/话题通信.md)
- [服务通信](docs/服务通信.md)
- [动作通信](docs/动作通信.md)
- [参数服务](docs/参数服务.md)

### 基础案例说明

- [C++ 话题通信](src/learning_topic_cpp/README.md) / [Python 话题通信](src/learning_topic_py/README.md)
- [C++ 服务通信](src/learning_service_cpp/README.md) / [Python 服务通信](src/learning_service_py/README.md)
- [C++ 动作通信](src/learning_action_cpp/README.md) / [Python 动作通信](src/learning_action_py/README.md)
- [C++ 参数服务](src/learning_param_cpp/README.md) / [Python 参数服务](src/learning_param_py/README.md)

### 练习题说明

- [话题通信练习](src/exercise_topic_cpp/README.md)
- [服务通信练习](src/exercise_service_cpp/README.md)
- [动作通信练习](src/exercise_action_cpp/README.md)
- [参数服务练习](src/exercise_param_cpp/README.md)

## 开发约定

- 功能包、可执行文件、节点、话题和接口字段统一使用 `lower_snake_case`。
- `learning_*` 保存单一知识点的最小示例，`exercise_*` 保存包含业务规则的综合练习。
- C++ 与 Python 实现保持接口名称和业务行为一致，支持跨语言组合测试。
- 自定义接口只放在 `base_interfaces`，业务功能包通过依赖引用，不重复定义。
- 修改接口后，先重新构建 `base_interfaces`，再构建依赖该接口的功能包。
- C++ 使用仓库根目录 `.clang-format`，缩进统一为 4 个空格。
- 提交源码和文档，不提交 `build/`、`install/`、`log/` 等生成内容。

## 验证与排障

查看功能包是否被发现：

```bash
colcon list
ros2 pkg list | grep learning_
```

查看构建测试结果：

```bash
colcon test
colcon test-result --verbose
```

常见问题：

| 现象 | 检查项 |
| --- | --- |
| `Package not found` | 是否构建成功并执行了 `source install/setup.bash` |
| 找不到自定义接口 | 是否先构建 `base_interfaces`，当前终端是否加载最新环境 |
| 发布方与订阅方无法通信 | 话题名称、消息类型、QoS 和 `ROS_DOMAIN_ID` 是否一致 |
| 服务或动作客户端一直等待 | 对应服务端是否启动，名称和接口类型是否一致 |
| Python 自定义接口没有代码提示 | VS Code 解释器和 `python.analysis.extraPaths` 是否指向当前工作空间 |

当前仓库以编译验证、ROS 2 CLI 测试和 C++/Python 跨语言集成为主，尚未建设完整的自动化集成测试流水线。

## 后续学习

Launch、rosbag2、TF、RViz2、URDF 与 Xacro 等 ROS 2 工具内容已迁移到独立仓库：

- [ros2_tools_learning](https://github.com/zwj051029/ros2_tools_learning)

## 许可证

当前仓库未声明开源许可证，仅用于个人学习、复习与技术交流。复制、分发或用于其他项目之前，请先获得仓库维护者许可。

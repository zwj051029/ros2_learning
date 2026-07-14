# ROS 2 话题通信学习示例（C++）

## 功能包说明

`learning_topic_cpp` 使用 `rclcpp` 学习 ROS 2 话题通信，包含原生字符串消息和自定义学生消息两组发布、订阅示例。

对应的 Python 实现见 [learning_topic_py](../learning_topic_py/README.md)。

## 示例列表

| 示例 | 发布方 | 订阅方 | 话题 | 消息类型 |
| --- | --- | --- | --- | --- |
| 字符串通信 | `cpp_string_pub` | `cpp_string_sub` | `/string` | `std_msgs/msg/String` |
| 学生信息通信 | `student_pub` | `student_sub` | `/student` | `base_interfaces/msg/Student` |

所有发布方和订阅方的 QoS 队列深度均为 `10`。

## 字符串话题

发布方每隔 `1` 秒发送一条消息，内容由 `hello world` 和递增编号组成：

```text
hello world1
hello world2
hello world3
```

订阅方接收 `/string` 消息并通过日志输出文本内容。

启动节点：

```bash
ros2 run learning_topic_cpp cpp_string_pub
ros2 run learning_topic_cpp cpp_string_sub
```

也可以直接查看话题：

```bash
ros2 topic echo /string
ros2 topic info /string --verbose
```

## 自定义学生消息

接口定义位于 [Student.msg](../base_interfaces/msg/Student.msg)：

```text
string name
int32 age
float64 height
```

发布方每隔 `0.5` 秒发布固定学生信息：

```text
name: Tom
age: 18
height: 1.83
```

启动节点：

```bash
ros2 run learning_topic_cpp student_pub
ros2 run learning_topic_cpp student_sub
```

检查接口和话题：

```bash
ros2 interface show base_interfaces/msg/Student
ros2 topic echo /student
ros2 topic info /student --verbose
```

## 构建与运行

在工作空间根目录执行：

```bash
colcon build --packages-select base_interfaces learning_topic_cpp
source install/setup.bash
```

修改 `Student.msg` 后，需要先重新构建 `base_interfaces`，再重新构建本功能包并加载环境。

## CMake 配置要点

每个 C++ 节点都需要：

1. 使用 `add_executable()` 注册可执行文件。
2. 使用 `ament_target_dependencies()` 声明 `rclcpp`、`std_msgs` 或 `base_interfaces` 依赖。
3. 使用 `install(TARGETS ...)` 将节点安装到 `lib/${PROJECT_NAME}`。
4. 在 `package.xml` 中声明相同的运行依赖。

## 跨语言测试

C++ 和 Python 只要使用相同的话题名称与消息类型即可互相通信。

```bash
# C++ 发布，Python 订阅
ros2 run learning_topic_cpp cpp_string_pub
ros2 run learning_topic_py py_string_sub

# Python 发布，C++ 订阅
ros2 run learning_topic_py student_pub
ros2 run learning_topic_cpp student_sub
```

## 学习要点

- `rclcpp::Publisher` 与 `rclcpp::Subscription` 的创建和生命周期。
- 定时器驱动的周期发布。
- 订阅回调与 `std::placeholders::_1`。
- ROS 2 原生消息与自定义消息的使用差异。
- `rclcpp::spin()` 对节点回调的持续调度。

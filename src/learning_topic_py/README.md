# ROS 2 话题通信学习示例（Python）

## 功能包说明

`learning_topic_py` 使用 `rclpy` 学习 ROS 2 话题通信，包含原生字符串消息和自定义学生消息两组发布、订阅示例。

对应的 C++ 实现见 [learning_topic_cpp](../learning_topic_cpp/README.md)。

## 示例列表

| 示例 | 发布方入口 | 订阅方入口 | 话题 | 消息类型 |
| --- | --- | --- | --- | --- |
| 字符串通信 | `py_string_pub` | `py_string_sub` | `/string` | `std_msgs/msg/String` |
| 学生信息通信 | `student_pub` | `student_sub` | `/student` | `base_interfaces/msg/Student` |

所有发布方和订阅方的 QoS 队列深度均为 `10`。

## 字符串话题

发布方每隔 `1` 秒发送一条带递增编号的文本：

```text
hello world 1
hello world 2
hello world 3
```

订阅方接收 `/string` 消息并通过日志输出文本内容。

```bash
ros2 run learning_topic_py py_string_pub
ros2 run learning_topic_py py_string_sub
```

使用命令行检查：

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

发布方每隔 `0.5` 秒发布：

```text
name: Tom
age: 18
height: 1.83
```

```bash
ros2 run learning_topic_py student_pub
ros2 run learning_topic_py student_sub
```

检查自定义接口：

```bash
ros2 interface show base_interfaces/msg/Student
ros2 topic echo /student
```

## 构建与运行

```bash
colcon build --packages-select base_interfaces learning_topic_py
source install/setup.bash
```

Python 功能包使用 `ament_python`。`setup.py` 中的 `console_scripts` 将命令名称映射到对应模块的 `main()`：

```text
py_string_pub -> learning_topic_py.py_string_pub:main
py_string_sub -> learning_topic_py.py_string_sub:main
student_pub   -> learning_topic_py.student_pub:main
student_sub   -> learning_topic_py.student_sub:main
```

新增 Python 节点后，需要同步在 `setup.py` 中注册入口，重新构建并加载环境。

## 跨语言测试

```bash
# Python 发布，C++ 订阅
ros2 run learning_topic_py py_string_pub
ros2 run learning_topic_cpp cpp_string_sub

# C++ 发布，Python 订阅
ros2 run learning_topic_cpp student_pub
ros2 run learning_topic_py student_sub
```

## 学习要点

- `Node.create_publisher()` 与 `Node.create_subscription()`。
- `Node.create_timer()` 和周期回调。
- Python 消息对象的创建、字段赋值和发布。
- 通过 `setup.py` 注册 ROS 2 Python 可执行入口。
- Python 对 ROS 2 自动生成自定义消息模块的导入方式。

# ROS 2 参数服务学习示例（Python）

## 功能包说明

`learning_param_py` 使用 `rclpy` 演示 ROS 2 参数的声明、修改、查询、删除和存在性判断。

对应的 C++ 实现见 [learning_param_cpp](../learning_param_cpp/README.md)。

当前 ROS 2 Humble 环境中未提供可直接使用的 Python 参数客户端专用 API，因此本功能包只实现参数服务端。需要从其他终端访问参数时，可以使用 `ros2 param` 命令。

## 节点与入口

| 类型 | 运行入口 | 节点名称 | 作用 |
| --- | --- | --- | --- |
| 参数服务端 | `car_param_server` | `car_param_server_py` | 创建并管理车辆参数 |

服务端节点在构造时启用了：

```python
allow_undeclared_parameters=True
```

这允许节点通过 `set_parameters()` 创建尚未声明的参数。

## 服务端参数

| 参数名称 | 初始值 | Python 类型 | 作用 |
| --- | ---: | --- | --- |
| `car_name` | `"tiger"` | `str` | 车辆名称 |
| `car_widths` | `1.75` | `float` | 车辆宽度 |
| `car_wheels` | `5` | `int` | 车轮数量 |
| `car_heights` | `2.0` | `float` | 临时创建的车辆高度 |

其中 `car_name`、`car_widths` 和 `car_wheels` 使用 `declare_parameter()` 正式声明，`car_heights` 使用 `set_parameters()` 动态创建。

## 服务端执行流程

服务端启动后依次执行以下操作：

1. 声明车辆名称、宽度和车轮数量参数。
2. 动态创建车辆高度参数。
3. 判断车辆高度参数是否存在。
4. 删除车辆高度参数，并再次判断它是否存在。
5. 读取并打印修改前的车辆参数。
6. 批量修改车辆名称、宽度和车轮数量。
7. 分别查询单个参数和多个参数。
8. 判断车辆名称和车辆高度参数是否存在。
9. 进入事件循环，继续提供 ROS 2 参数服务。

服务端完成初始化操作后，参数状态为：

| 参数名称 | 最终值 | 是否存在 |
| --- | ---: | --- |
| `car_name` | `"pig"` | 是 |
| `car_widths` | `6.66` | 是 |
| `car_wheels` | `6` | 是 |
| `car_heights` | 无 | 否 |

## Parameter 对象

`set_parameters()` 接收的是 `Parameter` 对象列表，而不是普通的名称和值列表。例如：

```python
self.set_parameters([
    rclpy.parameter.Parameter("car_name", value="pig"),
    rclpy.parameter.Parameter("car_widths", value=6.66),
    rclpy.parameter.Parameter("car_wheels", value=6),
])
```

创建参数时建议使用 `value=` 明确指定参数值。直接把数值写在第二个位置，会被当作参数类型传入，从而引发类型错误。

## 构建

在工作空间根目录执行：

```bash
colcon build --packages-select learning_param_py
source install/setup.bash
```

`setup.py` 中通过 `console_scripts` 注册运行入口：

```text
car_param_server = learning_param_py.car_param_server:main
```

因此可以使用 `ros2 run` 启动节点，而不需要直接执行 Python 文件。

## 运行示例

启动参数服务端：

```bash
ros2 run learning_param_py car_param_server
```

服务端将依次打印参数的新增、删除、修改和查询结果，随后保持运行并等待外部参数请求。

按 `Ctrl+C` 结束程序时，节点会捕获 `KeyboardInterrupt`，销毁节点并关闭 `rclpy`。

## 使用 ROS 2 命令测试

保持参数服务端运行，在新终端中执行：

```bash
source install/setup.bash
```

查看节点参数列表：

```bash
ros2 param list /car_param_server_py
```

正确情况下应包含：

```text
car_name
car_widths
car_wheels
```

`car_heights` 已经在初始化过程中删除，因此不会出现在列表中。

查询单个参数：

```bash
ros2 param get /car_param_server_py car_name
ros2 param get /car_param_server_py car_widths
ros2 param get /car_param_server_py car_wheels
```

预期结果分别为：

```text
String value is: pig
Double value is: 6.66
Integer value is: 6
```

从终端修改参数：

```bash
ros2 param set /car_param_server_py car_name rabbit
ros2 param set /car_param_server_py car_widths 2.5
ros2 param set /car_param_server_py car_wheels 4
```

成功时会返回：

```text
Set parameter successful
```

查看参数描述：

```bash
ros2 param describe /car_param_server_py car_name
```

导出节点的全部参数：

```bash
ros2 param dump /car_param_server_py
```

## 为什么没有单独编写 Python 参数客户端

ROS 2 节点会自动提供参数相关服务，`ros2 param` 命令可以通过这些服务完成参数的查询和修改。

当前安装的 ROS 2 Humble 环境没有可直接导入的 `rclpy.parameter_client.AsyncParameterClient`，因此本学习案例暂不单独实现 Python 参数客户端。后续如需更深入练习，可以直接调用参数服务接口，或在提供该 API 的 ROS 2 环境中使用异步参数客户端。

## Python 配置要点

- 功能包使用 `ament_python` 构建类型。
- `package.xml` 中需要声明对 `rclpy` 的运行依赖。
- Python 节点需要在 `setup.py` 的 `console_scripts` 中注册。
- 修改 `setup.py` 或 Python 源代码后，需要重新构建并执行 `source install/setup.bash`。

## 学习要点

- 使用 `declare_parameter()` 声明参数。
- 使用 `set_parameters()` 批量设置参数。
- 使用 `get_parameter()` 查询单个参数。
- 使用 `get_parameters()` 批量查询参数。
- 使用 `has_parameter()` 判断参数是否存在。
- 使用 `undeclare_parameter()` 删除参数。
- 理解 `allow_undeclared_parameters` 的作用。
- 正确创建和使用 `rclpy.parameter.Parameter` 对象。
- 使用 `ros2 param` 命令访问正在运行的节点参数。
- 使用 `KeyboardInterrupt` 和 `finally` 完成节点的正常退出。

# 题目二：机器人巡逻航点管理服务

## 题目背景

巡逻机器人需要在运行期间管理一组二维航点。请实现一个有状态的服务端，支持添加、删除、查询和清空航点，并在每次响应中返回当前完整航点列表。

## 自定义服务接口

在 `base_interfaces/srv/ManageWaypoint.srv` 中定义：

```text
string operation
string waypoint_name
float32 x
float32 y
float32 yaw
---
bool success
string message
int32 total_count
string[] waypoint_names
float32[] xs
float32[] ys
float32[] yaws
```

## 通信设计

| 项目 | 名称 |
| --- | --- |
| 服务名称 | `/manage_waypoint` |
| 服务端可执行文件 | `waypoint_manager_server` |
| 客户端可执行文件 | `waypoint_manager_client` |
| 服务类型 | `base_interfaces/srv/ManageWaypoint` |

## 航点数据

每个航点包含：

```text
waypoint_name：航点名称
x：二维平面 X 坐标
y：二维平面 Y 坐标
yaw：机器人到达航点后的朝向
```

服务端启动时航点列表为空，最多保存 `5` 个航点。

## 通用响应规则

无论请求成功还是失败，响应都需要返回处理完成后的完整航点列表：

```text
total_count
waypoint_names
xs
ys
yaws
```

四个数组长度必须相同，并且同一个索引位置表示同一个航点。例如：

```text
waypoint_names[0]
xs[0]
ys[0]
yaws[0]
```

共同描述第一个航点。

## `add` 添加航点

添加操作必须依次检查：

1. `waypoint_name` 不能为空。
2. 不能存在同名航点。
3. 当前航点数量不能超过 `5` 个。
4. `x`、`y`、`yaw` 必须是有限数值。
5. `x` 和 `y` 必须位于 `[-10.0, 10.0]`。
6. `yaw` 必须位于 `[-3.14, 3.14]`。

全部合法时添加航点，并返回添加成功。

## `delete` 删除航点

1. 根据 `waypoint_name` 查找航点。
2. 找不到时返回失败，不能修改列表。
3. 找到时删除名称、坐标和朝向对应的完整航点。
4. 删除成功后，其他航点的相对顺序保持不变。

删除操作不使用请求中的 `x`、`y` 和 `yaw`。

## `query` 查询航点

查询操作不修改任何数据，直接返回当前完整航点列表。请求中的名称和坐标可以使用占位值。

## `clear` 清空航点

清空全部航点并返回成功：

```text
total_count = 0
所有响应数组为空
```

## 未知操作

`operation` 不是 `add`、`delete`、`query` 或 `clear` 时返回失败，且不能修改航点列表。

## 客户端要求

客户端命令格式：

```text
操作 航点名称 x y yaw
```

示例：

```bash
ros2 run exercise_service_cpp waypoint_manager_client add reception 1.5 2.0 0.0
ros2 run exercise_service_cpp waypoint_manager_client delete reception 0.0 0.0 0.0
ros2 run exercise_service_cpp waypoint_manager_client query none 0.0 0.0 0.0
ros2 run exercise_service_cpp waypoint_manager_client clear none 0.0 0.0 0.0
```

客户端需要打印请求是否成功、提示信息、航点总数以及每个航点的完整数据。

## 完整测试顺序

1. 查询空列表，应返回 `total_count = 0`。
2. 添加 `reception (1.5, 2.0, 0.0)`，应成功。
3. 添加 `charging_station (-3.0, 1.0, 3.14)`，应成功。
4. 再次添加 `reception`，应因重名失败。
5. 添加空名称，应失败。
6. 测试超出范围的 `x`、`y` 和 `yaw`，均应失败。
7. 添加到第 `5` 个航点，应成功。
8. 添加第 `6` 个航点，应因容量已满失败。
9. 删除不存在的航点，应失败且列表不变。
10. 删除一个已存在航点，应成功且总数减一。
11. 查询列表，检查四个数组的数据是否一一对应。
12. 使用未知操作，应失败且列表不变。
13. 清空列表，应返回空数组和 `total_count = 0`。

## 验收标准

- 四种操作行为正确，未知操作可以安全拒绝。
- 航点名称、容量、有限数值和坐标范围校验完整。
- 请求失败时不会意外修改原有数据。
- 响应中的数量和四个数组始终保持一致。
- 客户端可以完整打印航点列表。
- C++ 与 Python 客户端、服务端可以交叉通信。

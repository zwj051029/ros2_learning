"""
    需求：编写参数服务服务端 实现增山改查操作
    步骤：
        1、导包
        2、初始化 ROS2 客户端
        3、自定义节点类
            3-1、实现 增 操作
            3-2、实现 删 操作
            3-3、实现 改 操作
            3-4、实现 查 操作
        4、调用 spin 函数 并传入节点对象指针
        5、释放资源
"""

import rclpy
from rclpy.node import Node
import rclpy.parameter

class CarParamServer(Node):
    def __init__(self):
        super().__init__("car_param_server_py", allow_undeclared_parameters = True)
        self.get_logger().info("参数服务端创建成功!(Python)")

    def declare_param(self):
        self.get_logger().info("----------增----------")
        self.declare_parameter("car_name", "tiger")
        self.declare_parameter("car_widths", 1.75)
        self.declare_parameter("car_wheels", 5)

        self.set_parameters([
            rclpy.parameter.Parameter("car_heights", value = 2.0)
        ])

    def delete_param(self):
        self.get_logger().info("----------删----------")

        self.get_logger().info(
            f"删除之前是否包含 car_heights ? {'yes' if self.has_parameter('car_heights') else 'no'}"
        )
        self.undeclare_parameter("car_heights")
        self.get_logger().info(
            f"删除之后是否包含 car_heights ? {'yes' if self.has_parameter('car_heights') else 'no'}"
        )

    def set_param(self):
        self.get_logger().info("----------改----------")
        self.get_logger().info("修改之前的参数")
        p1 = self.get_parameter("car_name")
        p2 = self.get_parameter("car_widths")
        p3 = self.get_parameter("car_wheels")

        self.get_logger().info(f"name = {p1.name}, val = {p1.value}")
        self.get_logger().info(f"name = {p2.name}, val = {p2.value}")
        self.get_logger().info(f"name = {p3.name}, val = {p3.value}")

        self.set_parameters([
            rclpy.parameter.Parameter("car_name", value = "pig"),
            rclpy.parameter.Parameter("car_widths", value = 6.66),
            rclpy.parameter.Parameter("car_wheels", value = 6)
        ])

        self.get_logger().info("修改之后的参数")
        p1 = self.get_parameter("car_name")
        p2 = self.get_parameter("car_widths")
        p3 = self.get_parameter("car_wheels")

        self.get_logger().info(f"name = {p1.name}, val = {p1.value}")
        self.get_logger().info(f"name = {p2.name}, val = {p2.value}")
        self.get_logger().info(f"name = {p3.name}, val = {p3.value}")

    def get_param(self):
        self.get_logger().info("----------查----------")
        # 查询指定参数
        self.get_logger().info("查询指定参数")
        p1 = self.get_parameter("car_name")
        p2 = self.get_parameter("car_widths")
        p3 = self.get_parameter("car_wheels")

        self.get_logger().info(f"name = {p1.name}, val = {p1.value}")
        self.get_logger().info(f"name = {p2.name}, val = {p2.value}")
        self.get_logger().info(f"name = {p3.name}, val = {p3.value}")

        # 查询一些参数
        self.get_logger().info("查询一些参数")
        names = ["car_name", "car_widths", "car_wheels"]
        params = self.get_parameters(names)

        for param in params:
            self.get_logger().info(f"{param.name} = {param.value}")

        # 是否包含参数
        self.get_logger().info("是否包含参数")
        self.get_logger().info(
            f"是否包含 car_name ? {'yes' if self.has_parameter('car_name') else 'no'}"
        )
        self.get_logger().info(
            f"是否包含 car_heights ? {'yes' if self.has_parameter('car_heights') else 'no'}"
        )

def main():
    rclpy.init()

    server = CarParamServer()

    server.declare_param()
    server.delete_param()
    server.set_param()
    server.get_param()

    try:
        rclpy.spin(server)
    except KeyboardInterrupt:
        pass
    finally:
        server.destroy_node()
        if rclpy.ok():
            rclpy.shutdown()

if __name__ == "__main__":
    main()
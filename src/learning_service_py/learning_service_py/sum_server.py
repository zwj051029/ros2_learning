"""
需求：编写服务端 提取客户端请求的两个整型数据 并将其相加作为响应
步骤：
    1、导包
    2、初始化 ROS2 客户端
    3、自定义节点类
        3-1、创建服务端
        3-2、编写回调函数 处理请求并做出响应
    4、调用 spin 函数 并传入节点对象
    5、释放资源
"""

import rclpy
from rclpy.node import Node
from base_interfaces.srv import AddInts

class SumServer(Node):
    def __init__(self):
        super().__init__("sum_server_py")
        self.get_logger().info("服务端创建成功!")
        self.server_ = self.create_service(AddInts, "sum", self.sum_callback)

    def sum_callback(self, request, response):
        response.sum = request.num1 + request.num2
        self.get_logger().info(
            f"客户端发送的请求为: [num1 = {request.num1}, num2 = {request.num2}]\n"
            f"服务端作出的响应为: [sum = {response.sum}]"
        )
        return response

def main():
    rclpy.init()
    rclpy.spin(SumServer())
    rclpy.shutdown()

if __name__ == "__main__":
    main()
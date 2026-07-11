"""
需求：编写客户端 组织并发送两个整型数据作为请求 并处理响应
步骤：
    1、导包
    2、初始化 ROS2 客户端
    3、自定义节点类
        3-1、创建客户端
        3-2、等待连接服务器
        3-3、编写发布请求函数
    4、创建节点对象 发送请求 并处理响应
    5、释放资源
"""

import sys
import rclpy
from rclpy.node import Node
from rclpy.logging import get_logger
from base_interfaces.srv import AddInts

class SumClient(Node):
    def __init__(self):
        super().__init__("sum_client_py")
        self.get_logger().info("客户端创建成功!(Python)")
        self.client_ = self.create_client(AddInts, "sum")

    def connect_server(self) -> bool:
        while not self.client_.wait_for_service(1.0):
            if not rclpy.ok():
                get_logger("rclpy").error("强制退出等待!")
                return False
            get_logger("rclpy").info("服务器连接中...")
        return True
    
    def send_request(self, num1: int, num2: int):
        request = AddInts.Request()
        request.num1 = num1
        request.num2 = num2
        return self.client_.call_async(request)

def main():
    if len(sys.argv) != 3:
        get_logger("rclpy").error("请提交两个整型数据!")
        return

    rclpy.init()

    sum_client = SumClient()

    connect_result = sum_client.connect_server()
    if connect_result:
        get_logger("rclpy").info("服务器连接成功!")
    else:
        get_logger("rclpy").error("服务器连接失败!")
    
    future = sum_client.send_request(int(sys.argv[1]), int(sys.argv[2]))
    rclpy.spin_until_future_complete(sum_client, future)

    try:
        response = future.result()
    except Exception as e:
        sum_client.get_logger().error(f"服务请求失败: {e}")
    else:
        sum_client.get_logger().info(f"响应结果为: sum = {response.sum}")

if __name__ == "__main__":
    main()
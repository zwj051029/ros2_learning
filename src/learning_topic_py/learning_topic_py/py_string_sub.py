"""
需求：订阅发布方发布的消息 并输出到终端
步骤：
    1、导包
    2、初始化 ROS2 客户端
    3、自定义节点类
        3-1、创建订阅方
        3-2、解析并输出数据
    4、调用 spin 函数 并传入节点对象
    5、释放资源
"""

import rclpy
from rclpy.node import Node
from std_msgs.msg import String

class StrSubscriber(Node):
    def __init__(self):
        super().__init__("str_subscriber_py")
        self.get_logger().info("订阅方创建成功!(Python)")
        self.subscriber = self.create_subscription(String, "string", self.sub_callback, 10)
    
    def sub_callback(self, msg):
        self.get_logger().info(f"订阅方订阅到的消息为: {msg.data}")

def main():
    rclpy.init()
    rclpy.spin(StrSubscriber())
    rclpy.shutdown()

if __name__ == "__main__":
    main()

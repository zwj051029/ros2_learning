"""
需求：以某个固定频率发送文本"hello world" 文本后缀编号 每发布一条消息 编号加1
步骤：
    1、导包
    2、初始化 ROS2 客户端
    3、自定义节点类
        3-1、创建发布方
        3-2、创建定时器
        3-3、组织并发布消息
    4、调用 spin 函数 传入节点对象
    5、释放资源
"""

import rclpy
from rclpy.node import Node
from std_msgs.msg import String

class StrPublisher(Node):
    def __init__(self):
        super().__init__("str_publisher_py")
        self.count_ = 0
        self.publisher_ = self.create_publisher(String, "string", 10)
        self.timer_ = self.create_timer(1.0, self.timer_callback)

    def timer_callback(self):
        self.count_ += 1
        message = String()
        message.data = "hello world " + str(self.count_)
        self.publisher_.publish(message)
        self.get_logger().info(f"发布方发布的消息: {message.data}")

def main():
    rclpy.init()
    rclpy.spin(StrPublisher())
    rclpy.shutdown()

if __name__ == "__main__":
    main()
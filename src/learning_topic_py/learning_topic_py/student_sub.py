"""
需求：订阅发布方发布的学生信息 并输出到终端
步骤：
    1、导包
    2、初始化 ROS2 客户端
    3、自定义节点类
        3-1、创建订阅方
        3-2、输出学生信息
    4、调用 spin 函数 并传入节点对象指针
    5、释放资源
"""

import rclpy
from rclpy.node import Node
from base_interfaces.msg import Student

class StuSubscriber(Node):
    def __init__(self):
        super().__init__("student_subscriber_py")
        self.get_logger().info("订阅方创建成功!(Python)")

        self.subscriber_ = self.create_subscription(Student, "student", self.stu_msg_callback, 10)

    def stu_msg_callback(self, stu_msg):
        self.get_logger().info(f"订阅到的学生信息为: name = {stu_msg.name}, age = {stu_msg.age}, height = {stu_msg.height:.2f}")

def main():
    rclpy.init()
    rclpy.spin(StuSubscriber())
    rclpy.shutdown()

if __name__ == "__main__":
    main()
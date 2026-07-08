"""
需求：以某个固定频率发送文本学生信息
步骤：
    1、导包
    2、初始化 ROS2 客户端
    3、自定义节点类
        3-1、创建发布方
        3-2、创建定时器
        3-3、组织并发布学生信息
    4、调用 spin 函数 并传入节点对象指针
    5、释放资源
"""

import rclpy
from rclpy.node import Node
from base_interfaces.msg import Student

class StuPublisher(Node):
    def __init__(self):
        super().__init__("student_publisher_py")
        self.get_logger().info("发布方创建成功!(Python)")

        self.publisher_ = self.create_publisher(Student, "student", 10)
        self.timer_ = self.create_timer(0.5, self.timer_callback)

    def timer_callback(self):
        stu_msg = Student()
        stu_msg.name = "Tom"
        stu_msg.age = 18
        stu_msg.height = 1.83

        self.publisher_.publish(stu_msg)
        self.get_logger().info(f"发布的学生信息: [name = {stu_msg.name}, age = {stu_msg.age}, height = {stu_msg.height:.2f}]")

def main():
    rclpy.init()
    rclpy.spin(StuPublisher())
    rclpy.shutdown()

if __name__ == "__main__":
    main()
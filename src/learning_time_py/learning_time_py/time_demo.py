import rclpy
from rclpy.node import Node
from rclpy.time import Time

class TimeDemo(Node):
    def __init__(self):
        super().__init__("time_demo_py")
        t1 = Time(seconds = 2, nanoseconds = 500000000)
        t2 = self.get_clock().now()

        self.get_logger().info(
            f"s = {t1.seconds_nanoseconds()[0]}, ns = {t1.seconds_nanoseconds()[1]}"
        )
        self.get_logger().info(
            f"s = {t2.seconds_nanoseconds()[0]}, ns = {t2.seconds_nanoseconds()[1]}"
        )

def main():
    rclpy.init()
    node = TimeDemo()
    rclpy.shutdown()

if __name__ == "__main__":
    main()

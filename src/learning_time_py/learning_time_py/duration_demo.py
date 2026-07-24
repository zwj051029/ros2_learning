import rclpy
from rclpy.node import Node
from rclpy.duration import Duration

class DurationDemo(Node):
    def __init__(self):
        super().__init__("duration_demo_py")
        du1 = Duration(seconds = 2, nanoseconds = 500000000)
        self.get_logger().info(
            f"ns = {du1.nanoseconds}"
        )

def main():
    rclpy.init()
    node = DurationDemo()
    rclpy.shutdown()

if __name__ == "__main__":
    main()
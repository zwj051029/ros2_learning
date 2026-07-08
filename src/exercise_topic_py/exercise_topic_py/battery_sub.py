import rclpy
from rclpy.node import Node
from std_msgs.msg import Float32
from std_msgs.msg import String

class BatterySub(Node):
    def __init__(self):
        super().__init__("battery_subscriber_py")

        self.get_logger().info("订阅方创建成功!(Python)")

        self.subscriber_ = self.create_subscription(Float32, "battery_level", self.battery_callback, 10)
        self.publisher_ = self.create_publisher(String, "battery_status", 10)

    def battery_callback(self, msg):
        battery_status = String()

        if msg.data >= 60.0:
            battery_status.data = "NORMAL"
            self.get_logger().info(f"电量正常: {msg.data:.1f}%")
        elif msg.data >= 30.0:
            battery_status.data = "LOW"
            self.get_logger().warn(f"电量偏低: {msg.data:.1f}%")
        else:
            battery_status.data = "CRITICAL"
            self.get_logger().error(f"电量严重不足: {msg.data:.1f}%")

        self.publisher_.publish(battery_status)

def main():
    rclpy.init()
    rclpy.spin(BatterySub())
    rclpy.shutdown()

if __name__ == "__main__":
    main()
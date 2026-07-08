import rclpy
from rclpy.node import Node
from std_msgs.msg import Float32

class BatteryPub(Node):
    def __init__(self):
        super().__init__("battery_publisher_py")
        
        self.get_logger().info("发布方创建成功!(Python)")

        self.battery_level_ = 100.0
        self.publisher_ = self.create_publisher(Float32, "battery_level", 10)
        self.timer_ = self.create_timer(0.5, self.timer_callback)
    
    def timer_callback(self):
        message = Float32()
        message.data = self.battery_level_

        self.publisher_.publish(message)
        self.get_logger().info(f"发布方发布的消息: {message.data}")

        self.battery_level_ -= 1.5
        if (self.battery_level_ < 0.0):
            self.battery_level_ = 100.0
        

def main():
    rclpy.init()
    rclpy.spin(BatteryPub())
    rclpy.shutdown()

if __name__ == "__main__":
    main()
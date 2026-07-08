import rclpy
from rclpy.node import Node
from base_interfaces.msg import SensorStatus

class SensorStatusPub(Node):
    def __init__(self):
        super().__init__("sensor_status_publisher_py")
        self.get_logger().info("发布方创建成功!(Python)")

        self.sensor_data_ = [
            ("lidar",       45.0, 12.0, True),
            ("camera",      62.5,  5.0, True),
            ("imu",         38.0,  3.3, True),
            ("ultrasonic",  35.0,  2.7, True),
            ("gps",         40.0,  0.0, False),
        ]
        self.current_index_ = 0

        self.publisher_ = self.create_publisher(SensorStatus, "sensor_status", 10)
        self.timer_ = self.create_timer(1.0, self.timer_callback)

    def timer_callback(self):
        sensor_name, temperature, voltage, is_online = self.sensor_data_[self.current_index_]

        status_msg = SensorStatus()
        status_msg.sensor_name = sensor_name
        status_msg.temperature = temperature
        status_msg.voltage = voltage
        status_msg.is_online = is_online

        self.publisher_.publish(status_msg)
        self.get_logger().info(
            f"发布传感器状态: [name={status_msg.sensor_name}, "
            f"temp={status_msg.temperature:.1f}, "
            f"voltage={status_msg.voltage:.1f}, "
            f"online={status_msg.is_online}]"
        )

        self.current_index_ = (self.current_index_ + 1) % len(self.sensor_data_)

def main():
    rclpy.init()
    rclpy.spin(SensorStatusPub())
    rclpy.shutdown()

if __name__ == "__main__":
    main()
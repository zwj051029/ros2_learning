import rclpy
from rclpy.node import Node
from base_interfaces.msg import SensorStatus
from enum import IntEnum

class Status(IntEnum):
    OFFLINE = 0
    OVERHEAT = 1
    LOW_VOLTAGE = 2
    NORMAL = 3

class SensorStatusSub(Node):
    def __init__(self):
        super().__init__("sensor_status_subscriber_py")
        self.get_logger().info("订阅方创建成功!(Python)")

        self.total_msg_cnt = 0
        self.offline_cnt = 0
        self.overheat_cnt = 0
        self.low_voltage_cnt = 0
        self.normal_cnt = 0

        self.status_ = Status.NORMAL
        self.subscriber_ = self.create_subscription(SensorStatus, "sensor_status", self.sensor_status_callback, 10)

    def sensor_status_callback(self, status_msg):
        self.total_msg_cnt += 1
        if status_msg.is_online == False:
            self.status_ = Status.OFFLINE
            self.offline_cnt += 1
        elif status_msg.temperature >= 60.0:
            self.status_ = Status.OVERHEAT
            self.overheat_cnt += 1
        elif status_msg.voltage < 3.0:
            self.status_ = Status.LOW_VOLTAGE
            self.low_voltage_cnt += 1
        else:
            self.status_ = Status.NORMAL
            self.normal_cnt += 1

        match self.status_:
            case Status.NORMAL:
                self.get_logger().info(
                    f"{status_msg.sensor_name} 状态正常: "
                    f"temp={status_msg.temperature:.1f}, "
                    f"voltage={status_msg.voltage:.1f}"
                )
            case Status.OVERHEAT:
                self.get_logger().warn(
                    f"{status_msg.sensor_name} 过热: "
                    f"temp={status_msg.temperature:.1f}"
                )
            case Status.LOW_VOLTAGE:
                self.get_logger().warn(
                    f"{status_msg.sensor_name} 电压过低: "
                    f"voltage={status_msg.voltage:.1f}"
                )
            case Status.OFFLINE:
                self.get_logger().error(f"{status_msg.sensor_name} 离线")
            
        if self.total_msg_cnt % 5 == 0:
            self.get_logger().info("===== 传感器健康统计 =====")
            self.get_logger().info("总消息数: 5")
            self.get_logger().info(f"正常: {self.normal_cnt}")
            self.get_logger().info(f"离线: {self.offline_cnt}")
            self.get_logger().info(f"过热: {self.overheat_cnt}")
            self.get_logger().info(f"低电压: {self.low_voltage_cnt}")
            self.get_logger().info("========================")

            self.normal_cnt = 0
            self.offline_cnt = 0
            self.overheat_cnt = 0
            self.low_voltage_cnt = 0

def main():
    rclpy.init()
    rclpy.spin(SensorStatusSub())
    rclpy.shutdown()

if __name__ == "__main__":
    main()
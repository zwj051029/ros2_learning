import sys
import rclpy
from rclpy.node import Node
from rclpy.logging import get_logger
from base_interfaces.srv import SetRobotMode

class SetRobotModeClient(Node):
    def __init__(self):
        super().__init__("set_robot_mode_client_py")
        self.get_logger().info("客户端创建成功!(Python)")
        self.client_ = self.create_client(SetRobotMode, "set_robot_mode")
        while not self.client_.wait_for_service(1.0):
            get_logger("rclpy").info("服务器连接中...")

    def send_request(self):
        request = SetRobotMode.Request()
        request.mode = sys.argv[1]
        request.max_speed = float(sys.argv[2])
        return self.client_.call_async(request)

def main():
    if len(sys.argv) != 3:
        get_logger("rclpy").error("提交参数数目不合法!")
        return

    rclpy.init()

    client = SetRobotModeClient()

    future = client.send_request()
    rclpy.spin_until_future_complete(client, future)

    try:
        response = future.result()
    except Exception as e:
        client.get_logger().error(f"请求处理异常: {e}")
    else:
        client.get_logger().info("请求正常处理")
        client.get_logger().info("====================")
        client.get_logger().info(f"success = {response.success}")
        client.get_logger().info(f"current_mode = {response.current_mode}")
        client.get_logger().info(f"current_max_speed = {response.current_max_speed}")
        client.get_logger().info(f"message = {response.message}")
        client.get_logger().info("====================")

    rclpy.shutdown()

if __name__ == "__main__":
    main()
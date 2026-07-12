import sys
import rclpy
from rclpy.node import Node
from rclpy.logging import get_logger
from base_interfaces.srv import ManageWaypoint

class WaypointManagerClient(Node):
    def __init__(self):
        super().__init__("waypoint_manager_client_py")
        self.get_logger().info("客户端创建成功!")
        self.client_ = self.create_client(ManageWaypoint, "manage_waypoint")
        while not self.client_.wait_for_service(1.0):
            get_logger("rclpy").info("服务器连接中...")

    def send_request(self, operation, waypoint_name, x, y, yaw):
        request = ManageWaypoint.Request()
        request.operation = operation
        request.waypoint_name = waypoint_name
        request.x = x
        request.y = y
        request.yaw = yaw

        return self.client_.call_async(request)

def main():
    if len(sys.argv) != 6:
        get_logger("rclpy").error("提交请求数目不正确!")
        return

    rclpy.init()

    client = WaypointManagerClient()

    future = client.send_request(sys.argv[1], sys.argv[2], float(sys.argv[3]), float(sys.argv[4]), float(sys.argv[5]))
    rclpy.spin_until_future_complete(client, future)

    try:
        response = future.result()
    except Exception as e:
        client.get_logger().error(f"请求处理异常: {e}")
    else:
        client.get_logger().info("请求正常处理!")
        client.get_logger().info("====================")
        client.get_logger().info(f"是否成功: {response.success}")
        client.get_logger().info(f"消息: {response.message}")
        client.get_logger().info(f"总共计数: {response.total_count}")

        for i in range(response.total_count):
            client.get_logger().info(
                f"航点 {i + 1}: "
                f"名称={response.waypoint_names[i]}, "
                f"x={response.xs[i]:.2f}, "
                f"y={response.ys[i]:.2f}, "
                f"yaw={response.yaws[i]:.2f}"
            )

        client.get_logger().info("====================")

    rclpy.shutdown()    

if __name__ == "__main__":
    main()
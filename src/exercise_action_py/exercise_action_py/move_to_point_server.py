import time
import math

import rclpy
from rclpy.node import Node
from rclpy.action import ActionServer, GoalResponse, CancelResponse
from rclpy.action.server import ServerGoalHandle
from rclpy.executors import MultiThreadedExecutor
from rclpy.callback_groups import ReentrantCallbackGroup

from base_interfaces.action import MoveToPoint

def get_dist(x1: float, y1: float, x2: float, y2: float) -> float:
    return math.sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2))

def is_equal(a: float, b: float, eqs: float = 1e-6) -> bool:
    return abs(a - b) < eqs

class MoveToPointServer(Node):
    def __init__(self):
        super().__init__("move_to_point_server_py")
        self.get_logger().info("动作服务端创建成功(Python)!")

        self.current_x_ = 0.0
        self.current_y_ = 0.0

        self.callback_group_ = ReentrantCallbackGroup()
        self.server_ = ActionServer(
            self, MoveToPoint, "move_to_point",
            execute_callback = self.execute_callback,
            callback_group = self.callback_group_,
            goal_callback = self.goal_callback,
            cancel_callback = self.cancel_callback,
            handle_accepted_callback = self.handle_accepted_callback
        )

    def goal_callback(self, goal_request: MoveToPoint.Goal):
        target_x = goal_request.target_x
        target_y = goal_request.target_y
        speed = goal_request.speed

        if not math.isfinite(target_x) or not math.isfinite(target_y) or not math.isfinite(speed):
            self.get_logger().error("目标参数必须为有限数!")
            return GoalResponse.REJECT
        
        if target_x < -10.0 or target_x > 10.0 or target_y < -10.0 or target_y > 10.0:
            self.get_logger().error("目标位置的坐标必须在[-10.0, 10.0]!")
            return GoalResponse.REJECT
        
        if speed <= 0.0:
            self.get_logger().error("目标速度必须为正数!")
            return GoalResponse.REJECT
        elif speed > 1.5:
            self.get_logger().error("目标速度设置过大!")
            return GoalResponse.REJECT
        
        if get_dist(self.current_x_, self.current_y_, target_x, target_y) < 0.001:
            self.get_logger().error("目标与当前位置距离太小!")
            return GoalResponse.REJECT
        
        self.get_logger().info("参数全部合法!")
        return GoalResponse.ACCEPT

    def cancel_callback(self, cancel_request):
        self.get_logger().info("已收到取消请求, 准备取消任务!")
        return CancelResponse.ACCEPT

    def handle_accepted_callback(self, goal_handle):
        goal_handle.execute()

    def execute_callback(self, goal_handle: ServerGoalHandle):
        # 目标相关
        target_x = goal_handle.request.target_x
        target_y = goal_handle.request.target_y
        speed = goal_handle.request.speed

        # 结果相关
        result = MoveToPoint.Result()
        final_x = 0.0
        final_y = 0.0
        traveled_distance = 0.0

        # 反馈相关
        feedback = MoveToPoint.Feedback()
        remaining_distance = get_dist(self.current_x_, self.current_y_, target_x, target_y)
        progress = 0.0

        # 计算相关
        dx = target_x - self.current_x_
        dy = target_y - self.current_y_
        direction_x = dx / remaining_distance
        direction_y = dy / remaining_distance
        total_dist = get_dist(self.current_x_, self.current_y_, target_x, target_y)

        while not is_equal(target_x, self.current_x_) or not is_equal(target_y, self.current_y_):
            time.sleep(0.5)

            if goal_handle.is_cancel_requested:
                result.success = False
                result.message = "任务已取消!"
                result.final_x = self.current_x_
                result.final_y = self.current_y_
                result.traveled_distance= traveled_distance

                self.get_logger().info("====================")
                self.get_logger().info("最终结果")
                self.get_logger().info(f"success = {'True' if result.success else 'False'}")
                self.get_logger().info(f"message = {result.message}")
                self.get_logger().info(f"final_x = {result.final_x:.2f}")
                self.get_logger().info(f"final_y = {result.final_y:.2f}")
                self.get_logger().info(f"traveled_distance = {result.traveled_distance:.2f}")
                self.get_logger().info("====================")

                goal_handle.canceled()
                return result                

            normal_step = speed * 0.5
            actual_step = min(normal_step, remaining_distance)

            self.current_x_ += actual_step * direction_x
            self.current_y_ += actual_step * direction_y
            traveled_distance += actual_step
            remaining_distance = max((total_dist - traveled_distance), 0.0)
            progress = min(traveled_distance / total_dist, 1.0)

            feedback.current_x = self.current_x_
            feedback.current_y = self.current_y_
            feedback.remaining_distance = remaining_distance
            feedback.progress = progress

            self.get_logger().info("====================")
            self.get_logger().info("持续反馈中...")
            self.get_logger().info(f"当前x坐标: {feedback.current_x:.2f}")
            self.get_logger().info(f"当前y坐标: {feedback.current_y:.2f}")
            self.get_logger().info(f"剩余距离: {feedback.remaining_distance:.2f}")
            self.get_logger().info(f"当前进度: {feedback.progress:.2f}")
            self.get_logger().info("====================")

            goal_handle.publish_feedback(feedback)

        result.success = True
        result.message = "任务已完成!"
        result.final_x = target_x
        result.final_y = target_y
        result.traveled_distance= traveled_distance

        self.current_x_ = target_x
        self.current_y_ = target_y

        self.get_logger().info("====================")
        self.get_logger().info("最终结果")
        self.get_logger().info(f"success = {'True' if result.success else 'False'}")
        self.get_logger().info(f"message = {result.message}")
        self.get_logger().info(f"final_x = {result.final_x:.2f}")
        self.get_logger().info(f"final_y = {result.final_y:.2f}")
        self.get_logger().info(f"traveled_distance = {result.traveled_distance:.2f}")
        self.get_logger().info("====================")

        goal_handle.succeed()
        return result

def main():
    rclpy.init()

    server = MoveToPointServer()
    executor = MultiThreadedExecutor()
    executor.add_node(server)

    try:
        executor.spin()
    except KeyboardInterrupt:
        pass
    finally:
        executor.shutdown()
        server.destroy_node()
        
        if rclpy.ok():
            rclpy.shutdown()

if __name__ == "__main__":
    main()
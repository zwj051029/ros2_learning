import time
import math

import rclpy
from rclpy.node import Node
from rclpy.action import ActionServer, GoalResponse, CancelResponse
from rclpy.action.server import ServerGoalHandle
from rclpy.executors import MultiThreadedExecutor
from rclpy.callback_groups import ReentrantCallbackGroup

from base_interfaces.action import RotateRobot


class RotateRobotServer(Node):
    def __init__(self):
        super().__init__("rotate_robot_server_py")
        self.get_logger().info("动作服务端创建成功!(Python)")

        self.callback_group_ = ReentrantCallbackGroup()
        self.server_ = ActionServer(
            self,
            RotateRobot,
            "rotate_robot",
            callback_group=self.callback_group_,
            goal_callback=self.goal_callback,
            cancel_callback=self.cancel_callback,
            handle_accepted_callback=self.handle_accepted_callback,
            execute_callback=self.execute_callback
        )

    def goal_callback(self, goal_request: RotateRobot.Goal):
        target_angle = goal_request.target_angle
        angular_speed = goal_request.angular_speed

        if not math.isfinite(target_angle) or not math.isfinite(angular_speed):
            self.get_logger().error("参数必须为有限数!")
            return GoalResponse.REJECT

        if target_angle < -180.0 or target_angle > 180.0:
            self.get_logger().error("目标角度必须在-180度至180度之间!")
            return GoalResponse.REJECT
        elif abs(target_angle) < 0.001:
            self.get_logger().error("目标角度太小!")
            return GoalResponse.REJECT
        self.get_logger().info("目标角度设置合法!")

        if angular_speed <= 0.0 or angular_speed > 90.0:
            self.get_logger().error("角速度必须在0至90区间内!")
            return GoalResponse.REJECT
        self.get_logger().info("角速度设置合法!")

        return GoalResponse.ACCEPT

    def cancel_callback(self, cancel_request):
        self.get_logger().info("收到取消请求, 准备取消任务!")
        return CancelResponse.ACCEPT

    def handle_accepted_callback(self, goal_handle):
        goal_handle.execute()

    def execute_callback(self, goal_handle: ServerGoalHandle):
        # 请求相关的参数
        target_angle = goal_handle.request.target_angle
        angular_speed = goal_handle.request.angular_speed
        abs_angle = abs(target_angle)

        # 结果相关的参数
        result = RotateRobot.Result()
        elapsed_time = 0.0

        # 反馈相关的参数
        feedback = RotateRobot.Feedback()
        current_angle = 0.0
        remaining_angle = abs_angle
        progress = 0.0

        while current_angle < abs_angle:
            time.sleep(0.5)

            if goal_handle.is_cancel_requested:
                self.get_logger().info("任务已取消!")

                result.success = False
                result.message = "机器人旋转任务已取消!"
                result.final_angle = feedback.current_angle
                result.elapsed_time = elapsed_time

                goal_handle.canceled()

                self.get_logger().info("====================")
                self.get_logger().info(f"success = {'true' if result.success else 'false'}")
                self.get_logger().info(f"message = {result.message}")
                self.get_logger().info(f"final_angle = {result.final_angle:.2f}")
                self.get_logger().info(f"elapsed_time = {result.elapsed_time}")
                self.get_logger().info("====================")

                return result

            current_angle += angular_speed * 0.5
            remaining_angle = max(abs_angle - current_angle, 0.0)
            progress = current_angle / abs_angle
            elapsed_time += 0.5

            feedback.current_angle = (current_angle if target_angle > 0.0 else -current_angle)
            feedback.remaining_angle = remaining_angle
            feedback.progress = progress

            if current_angle >= abs_angle:
                break

            self.get_logger().info("====================")
            self.get_logger().info("持续反馈中...")
            self.get_logger().info(f"current_angle = {feedback.current_angle}")
            self.get_logger().info(f"remaining_angle = {feedback.remaining_angle}")
            self.get_logger().info(f"progress = {feedback.progress:.2f}")
            self.get_logger().info("====================")

            goal_handle.publish_feedback(feedback)

        feedback.current_angle = target_angle
        feedback.remaining_angle = 0.0
        feedback.progress = 1.0

        self.get_logger().info("====================")
        self.get_logger().info("持续反馈中...")
        self.get_logger().info(f"current_angle = {feedback.current_angle}")
        self.get_logger().info(f"remaining_angle = {feedback.remaining_angle}")
        self.get_logger().info(f"progress = {feedback.progress:.2f}")
        self.get_logger().info("====================")

        goal_handle.publish_feedback(feedback)

        result.success = True
        result.message = "机器人旋转完成!"
        result.final_angle = target_angle
        result.elapsed_time = elapsed_time

        goal_handle.succeed()

        self.get_logger().info("====================")
        self.get_logger().info(f"success = {'true' if result.success else 'false'}")
        self.get_logger().info(f"message = {result.message}")
        self.get_logger().info(f"final_angle = {result.final_angle:.2f}")
        self.get_logger().info(f"elapsed_time = {result.elapsed_time}")
        self.get_logger().info("====================")

        return result


def main():
    rclpy.init()

    server = RotateRobotServer()
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

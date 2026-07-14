"""实现整数累加动作服务端，处理目标、取消、进度反馈和最终结果."""
import time

import rclpy
from rclpy.action import ActionServer, CancelResponse, GoalResponse
from rclpy.callback_groups import ReentrantCallbackGroup
from rclpy.executors import MultiThreadedExecutor
from rclpy.node import Node

from base_interfaces.action import SumToN


class SumToNActionServer(Node):
    def __init__(self):
        super().__init__("sum_to_n_action_server_py")
        self.get_logger().info("动作服务端创建成功!(Python)")

        self.callback_group_ = ReentrantCallbackGroup()
        self.server_ = ActionServer(
            self,
            SumToN,
            "sum_to_n",
            self.execute_callback,
            goal_callback=self.goal_callback,
            cancel_callback=self.cancel_callback,
            handle_accepted_callback=self.handle_accepted_callback,
            callback_group=self.callback_group_,
        )

    def goal_callback(self, goal_request):
        if goal_request.num <= 1:
            self.get_logger().error("请提交大于1的数字!")
            return GoalResponse.REJECT

        self.get_logger().info("提交的数字合法!")
        return GoalResponse.ACCEPT

    def cancel_callback(self, cancel_request):
        self.get_logger().info("收到取消请求, 允许取消!")
        return CancelResponse.ACCEPT

    def handle_accepted_callback(self, goal_handle):
        goal_handle.execute()

    def execute_callback(self, goal_handle):
        num = goal_handle.request.num
        total = 0

        feedback = SumToN.Feedback()
        result = SumToN.Result()

        for i in range(1, num + 1):
            if goal_handle.is_cancel_requested:
                result.sum = total
                goal_handle.canceled()

                self.get_logger().info("任务取消成功!")
                return result

            total += i

            feedback.progress = i / num
            goal_handle.publish_feedback(feedback)

            self.get_logger().info(f"持续反馈中: {feedback.progress:.2f}")
            time.sleep(1.0)

        result.sum = total
        goal_handle.succeed()

        self.get_logger().info(f"任务处理完成, 最终结果为: {result.sum}")
        return result


def main():
    rclpy.init()

    server = SumToNActionServer()
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

"""实现整数累加动作客户端，处理目标响应、进度反馈和最终结果."""

import sys
from typing import cast

import rclpy
from rclpy.action import ActionClient
from rclpy.action.client import ClientGoalHandle
from rclpy.logging import get_logger
from rclpy.node import Node
from rclpy.task import Future

from base_interfaces.action import SumToN


class SumToNActionClient(Node):
    def __init__(self):
        super().__init__("sum_to_n_action_client_py")
        self.get_logger().info("动作客户端创建成功!(Python)")
        self.client_ = ActionClient(self, SumToN, "sum_to_n")

    def send_goal(self, num):
        if not self.client_.wait_for_server(10.0):
            get_logger("rclpy").error("服务器连接超时, 强制退出!")
            return

        goal = SumToN.Goal()
        goal.num = num
        self.get_logger().info(f"发送的目标为: {goal.num}")

        self.future_ = self.client_.send_goal_async(goal, self.feedback_callback)
        self.future_.add_done_callback(self.goal_response_callback)

    def goal_response_callback(self, future: Future):
        goal_handle = cast(ClientGoalHandle, future.result())

        if goal_handle.accepted:
            self.get_logger().info("目标被接受!")
        else:
            self.get_logger().info("目标被拒绝!")
            return

        self.result_future_: Future = goal_handle.get_result_async()
        self.result_future_.add_done_callback(self.get_result_callback)

    def feedback_callback(self, feedback_msg: SumToN.Impl.FeedbackMessage):
        progress = feedback_msg.feedback.progress
        self.get_logger().info(f"当前进度为: {round(progress * 100)}%")

    def get_result_callback(self, future: Future):
        result = future.result().result
        self.get_logger().info(f"任务处理成功, 最终结果: {result.sum}")


def main():
    if len(sys.argv) != 2:
        get_logger("rclpy").error("提交参数数目不合法!")
        return

    rclpy.init()

    client = SumToNActionClient()
    client.send_goal(int(sys.argv[1]))

    try:
        rclpy.spin(client)
    except KeyboardInterrupt:
        pass
    finally:
        client.destroy_node()

        if rclpy.ok():
            rclpy.shutdown()


if __name__ == "__main__":
    main()

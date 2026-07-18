import math
import sys
from typing import Optional, cast

import rclpy
from action_msgs.msg import GoalStatus
from action_msgs.srv import CancelGoal
from base_interfaces.action import RotateRobot
from rclpy.action import ActionClient
from rclpy.action.client import ClientGoalHandle
from rclpy.logging import get_logger
from rclpy.node import Node
from rclpy.task import Future


class RotateRobotClient(Node):
    def __init__(self):
        super().__init__('rotate_robot_client_py')
        self.get_logger().info('动作客户端创建成功!(Python)')

        self.client_ = ActionClient(self, RotateRobot, 'rotate_robot')
        self.timer_ = self.create_timer(0.01, self.timer_callback)

        self.cancel_start_ = False
        self.cancel_enable_ = False
        self.cancel_progress_ = 0.0
        self.finished_ = False
        self.exit_code_ = 0
        self.goal_handle_: Optional[ClientGoalHandle] = None

    def send_goal(self, target_angle: float, angular_speed: float) -> bool:
        if not self.client_.wait_for_server(10.0):
            self.get_logger().error('服务器连接超时, 客户端退出!')
            return False

        goal = RotateRobot.Goal()
        goal.target_angle = target_angle
        goal.angular_speed = angular_speed

        self.get_logger().info(
            '设置的目标为: '
            f'[target_angle = {goal.target_angle}, '
            f'angular_speed = {goal.angular_speed}]'
        )

        self.goal_future_ = self.client_.send_goal_async(
            goal,
            self.feedback_callback
        )
        self.goal_future_.add_done_callback(self.goal_response_callback)
        return True

    def goal_response_callback(self, future: Future):
        try:
            self.goal_handle_ = cast(ClientGoalHandle, future.result())
        except Exception as exception:
            self.get_logger().error(f'获取目标响应失败: {exception}')
            self.finish(1)
            return

        if self.goal_handle_ is None or not self.goal_handle_.accepted:
            self.get_logger().error('目标被拒绝!')
            self.finish(1)
            return

        self.get_logger().info('目标被接受!')

        if self.cancel_enable_ and self.cancel_progress_ <= 0.0:
            self.cancel_start_ = True
            self.get_logger().info('取消进度为0%, 等待发送取消请求!')

        self.result_future_ = self.goal_handle_.get_result_async()
        self.result_future_.add_done_callback(self.get_result_callback)

    def feedback_callback(
        self,
        feedback_msg: RotateRobot.Impl.FeedbackMessage
    ):
        feedback = feedback_msg.feedback

        self.get_logger().info('====================')
        self.get_logger().info('连续接收反馈中...')
        self.get_logger().info(
            f'current_angle = {feedback.current_angle}'
        )
        self.get_logger().info(
            f'remaining_angle = {feedback.remaining_angle}'
        )
        self.get_logger().info(
            f'progress = {round(feedback.progress * 100)}%'
        )
        self.get_logger().info('====================')

        if not self.cancel_enable_ or self.finished_:
            return

        if (
            feedback.progress >= self.cancel_progress_
            and feedback.progress < 1.0
        ):
            self.cancel_start_ = True

    def get_result_callback(self, future: Future):
        try:
            wrapped_result = future.result()
        except Exception as exception:
            self.get_logger().error(f'获取任务结果失败: {exception}')
            self.finish(1)
            return

        status = wrapped_result.status
        result = wrapped_result.result

        if status == GoalStatus.STATUS_SUCCEEDED:
            self.get_logger().info('任务执行完成!')
            exit_code = 0
        elif status == GoalStatus.STATUS_CANCELED:
            self.get_logger().warning('任务已取消!')
            exit_code = 0
        elif status == GoalStatus.STATUS_ABORTED:
            self.get_logger().error('任务被中断!')
            exit_code = 1
        else:
            self.get_logger().error(f'未知的任务状态: {status}')
            exit_code = 1

        self.get_logger().info('====================')
        self.get_logger().info('任务处理结果如下')
        self.get_logger().info(f'success = {result.success}')
        self.get_logger().info(f'message = {result.message}')
        self.get_logger().info(f'final_angle = {result.final_angle}')
        self.get_logger().info(f'elapsed_time = {result.elapsed_time}')
        self.get_logger().info('====================')

        self.finish(exit_code)

    def cancel_response_callback(self, future: Future):
        try:
            response = cast(CancelGoal.Response, future.result())
        except Exception as exception:
            self.get_logger().error(f'取消请求处理失败: {exception}')
            return

        if response.goals_canceling:
            self.get_logger().info('服务端已接受取消请求!')
        else:
            self.get_logger().warning(
                '服务端未接受取消请求, '
                f'return_code = {response.return_code}'
            )

    def timer_callback(self):
        if not self.cancel_start_ or self.finished_:
            return

        self.cancel_start_ = False
        self.cancel_enable_ = False

        if self.goal_handle_ is None:
            return

        self.get_logger().info('已发送取消请求, 等待任务取消!')

        self.cancel_future_ = self.goal_handle_.cancel_goal_async()
        self.cancel_future_.add_done_callback(
            self.cancel_response_callback
        )

    def finish(self, exit_code: int):
        self.finished_ = True
        self.cancel_start_ = False
        self.cancel_enable_ = False
        self.exit_code_ = exit_code

        if rclpy.ok():
            rclpy.shutdown()


def parse_finite_number(
    text: str,
    parameter_name: str
) -> Optional[float]:
    try:
        value = float(text)
    except ValueError:
        get_logger('rotate_robot_client_py').error(
            f'{parameter_name}必须是有效数值: {text}'
        )
        return None

    if not math.isfinite(value):
        get_logger('rotate_robot_client_py').error(
            f'{parameter_name}必须是有限数值: {text}'
        )
        return None

    return value


def main():
    if len(sys.argv) not in (3, 4):
        get_logger('rotate_robot_client_py').error('提交参数数目不合法!')
        return 1

    target_angle = parse_finite_number(sys.argv[1], '目标角度')
    angular_speed = parse_finite_number(sys.argv[2], '角速度')

    if target_angle is None or angular_speed is None:
        return 1

    cancel_percentage = None
    if len(sys.argv) == 4:
        cancel_percentage = parse_finite_number(sys.argv[3], '取消进度')
        if cancel_percentage is None:
            return 1

        if cancel_percentage < 0.0 or cancel_percentage >= 100.0:
            get_logger('rotate_robot_client_py').error(
                '取消进度必须在[0, 100)范围内!'
            )
            return 1

    rclpy.init()
    client = RotateRobotClient()

    if cancel_percentage is not None:
        client.cancel_enable_ = True
        client.cancel_progress_ = cancel_percentage * 0.01

    if not client.send_goal(target_angle, angular_speed):
        client.exit_code_ = 1
    else:
        try:
            rclpy.spin(client)
        except KeyboardInterrupt:
            pass

    client.destroy_node()

    if rclpy.ok():
        rclpy.shutdown()

    return client.exit_code_


if __name__ == '__main__':
    main()

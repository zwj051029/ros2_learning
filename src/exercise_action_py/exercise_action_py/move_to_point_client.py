import sys
import math
from typing import Optional, cast

from action_msgs.msg import GoalStatus
from action_msgs.srv import CancelGoal
import rclpy
from rclpy.node import Node
from rclpy.action import ActionClient
from rclpy.logging import get_logger
from rclpy.action.client import ClientGoalHandle
from rclpy.task import Future
from base_interfaces.action import MoveToPoint

def stop_rclpy():
    if rclpy.ok():
        rclpy.shutdown()

def parse_finite_number(text: str, paramater_name: str) -> Optional[float]:
    try:
        value = float(text)
    except ValueError:
        get_logger("rclpy").error(
            f"{paramater_name}必须为有效数值: {text}"
        )
        return None

    if not math.isfinite(value):
        get_logger("rclpy").error(
            f"{paramater_name}必须为有限数值: {text}"
        )
        return None

    return value

class MoveToPointClient(Node):
    def __init__(self):
        super().__init__("move_to_point_client_py")
        self.get_logger().info("动作客户端创建成功(Python)!")

        self.cancel_start_ = False
        self.cancel_enable_ = False
        self.cancel_progress = 1.2

        self.client_ = ActionClient(self, MoveToPoint, "move_to_point")
        self.timer_ = self.create_timer(0.01, self.timer_callback)

    def send_goal(self, target_x: float, target_y: float, speed: float) -> bool:
        if not self.client_.wait_for_server(10.0):
            self.get_logger().error("服务器连接超时, 强制退出!")
            return False
        self.get_logger().info("服务器连接成功!")

        goal = MoveToPoint.Goal()
        goal.target_x = target_x
        goal.target_y = target_y
        goal.speed = speed

        self.goal_future_ = self.client_.send_goal_async(goal, self.feedback_callback)
        self.goal_future_.add_done_callback(self.goal_response_callback)
        return True

    def goal_response_callback(self, future: Future):
        try:
            self.goal_handle_ = cast(ClientGoalHandle, future.result())
        except Exception as e:
            self.get_logger().error(f"获取目标响应失败: {e}")
            stop_rclpy()
            return

        if self.goal_handle_.accepted:
            self.get_logger().info("目标被接收!")
            if self.cancel_progress <= 0.0:
                self.get_logger().info("取消进度为0%, 准备立即发送取消请求!")
                self.cancel_start_ = True
        else:
            self.get_logger().warn("目标被拒绝!")
            stop_rclpy()
            return

        self.result_future_: Future = self.goal_handle_.get_result_async()
        self.result_future_.add_done_callback(self.result_response_callback)

    def feedback_callback(self, feedback_msg: MoveToPoint.Impl.FeedbackMessage):
        current_x = feedback_msg.feedback.current_x
        current_y = feedback_msg.feedback.current_y
        remaining_distance = feedback_msg.feedback.remaining_distance
        progress = feedback_msg.feedback.progress
        percent = round(progress * 100)

        self.get_logger().info("====================")
        self.get_logger().info("持续接收反馈中...")
        self.get_logger().info(f"当前x坐标: {current_x:.2f}")
        self.get_logger().info(f"当前y坐标: {current_y:.2f}")
        self.get_logger().info(f"剩余距离: {remaining_distance:.2f}")
        self.get_logger().info(f"当前进度: {percent}%")
        self.get_logger().info("====================")

        if self.cancel_enable_ and self.cancel_progress <= progress and self.cancel_progress > 0.0 and self.cancel_progress < 1.0:
            self.cancel_start_ = True

    def result_response_callback(self, future: Future):
        try:
            wrapped_result = cast(
                MoveToPoint.Impl.GetResultService.Response, future.result()
            )
        except Exception as e:
            self.get_logger().error(f"获取任务结果失败: {e}")
            stop_rclpy()
            return

        status = wrapped_result.status
        result = wrapped_result.result

        match status:
            case GoalStatus.STATUS_SUCCEEDED:
                self.get_logger().info('任务已完成!')
            case GoalStatus.STATUS_CANCELED:
                self.get_logger().warn('任务已取消!')
            case GoalStatus.STATUS_ABORTED:
                self.get_logger().error('任务已中断!')
            case _:
                self.get_logger().error('未知错误!')

        self.get_logger().info('====================')
        self.get_logger().info('任务处理结果如下')
        self.get_logger().info(f"success = {result.success}")
        self.get_logger().info(f"message = {result.message}")
        self.get_logger().info(f"final_x = {result.final_x:.2f}")
        self.get_logger().info(f"final_y = {result.final_y:.2f}")
        self.get_logger().info(f"traveled_distance = {result.traveled_distance:.2f}")
        self.get_logger().info('====================')

        stop_rclpy()

    def timer_callback(self):
        if not self.cancel_start_:
            return

        self.cancel_start_ = False
        self.cancel_enable_ = False

        self.get_logger().info('已发送取消请求, 等待任务取消!')

        self.cancel_future_ = self.client_._cancel_goal_async(self.goal_handle_)
        self.cancel_future_.add_done_callback(self.cancel_response_callback)

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

def main():
    if len(sys.argv) != 4 and len(sys.argv) != 5:
        get_logger("rclpy").error("提交参数数目不合法!")
        stop_rclpy()
        return

    target_x = parse_finite_number(sys.argv[1], "目标位置x坐标")
    target_y = parse_finite_number(sys.argv[2], "目标位置y坐标")
    speed = parse_finite_number(sys.argv[3], "目标速度")

    if target_x is None or target_y is None or speed is None:
        stop_rclpy()
        return
    
    if len(sys.argv) == 5:
        cancel_percent = parse_finite_number((sys.argv[4]), "取消进度")

        if cancel_percent is None:
            stop_rclpy()
            return

        if cancel_percent < 0.0 or cancel_percent >= 100.0:
            get_logger("rclpy").error("取消进度必须在[0, 100)范围内!")
            stop_rclpy()
            return

    rclpy.init()

    client = MoveToPointClient()

    if len(sys.argv) == 5:
        client.cancel_enable_ = True
        client.cancel_progress = cancel_percent * 0.01

    if not client.send_goal(target_x, target_y, speed):
        stop_rclpy()
        return

    try:
        rclpy.spin(client)
    except KeyboardInterrupt:
        pass
    finally:
        client.destroy_node()
        stop_rclpy()

if __name__ == "__main__":
    main()
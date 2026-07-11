import rclpy
from rclpy.node import Node
from rclpy.logging import get_logger
from base_interfaces.srv import SetRobotMode

class SetRobotModeServer(Node):
    def __init__(self):
        super().__init__("set_robot_mode_server_py")
        self.get_logger().info("服务端创建成功!(Python)")
        self.current_mode_ = "standby"
        self.current_max_speed_ = 0.0
        self.server_ = self.create_service(SetRobotMode, "set_robot_mode", self.set_robot_mode_callback)

    """
    req.mode        req.max_speed
    res.success     res.current_mode    res.current_max_speed      res.message
    """
    def set_robot_mode_callback(self, req, res):
        mode = req.mode
        max_speed = req.max_speed

        res.current_mode = self.current_mode_
        res.current_max_speed = self.current_max_speed_

        if mode == "standby":
            if max_speed != 0.0:
                res.success = False
                res.message = "待机状态下, 最大速度必须为0.0!"
                self.get_logger().error("速度设置不合法!")
                return res
        elif mode == "patrol":
            if max_speed <= 0.0 or max_speed > 1.0:
                res.success = False
                res.message = "巡航状态下, 0.0 < max_speed <= 1.0!"
                self.get_logger().error("速度设置不合法!")
                return res
        elif mode == "return_home":
            if max_speed <= 0.0 or max_speed > 0.5:
                res.success = False
                res.message = "返航状态下, 0.0 < max_speed <= 0.5!"
                self.get_logger().error("速度设置不合法!")
                return res
        elif mode == "emergency_stop":
            if max_speed != 0.0:
                res.success = False
                res.message = "急停状态下, 最大速度必须为0.0!"
                self.get_logger().error("速度设置不合法!")
                return res
        else:
            res.success = False
            res.message = "未知模式!"
            self.get_logger().error("未知模式!")
            return res

        self.current_mode_ = mode
        self.current_max_speed_ = max_speed

        res.success = True
        res.current_mode = self.current_mode_
        res.current_max_speed = self.current_max_speed_
        res.message = "机器人模式切换成功!"
        self.get_logger().info("机器人模式切换成功!")

        return res

def main():
    rclpy.init()
    rclpy.spin(SetRobotModeServer())
    rclpy.shutdown()

if __name__ == "__main__":
    main()
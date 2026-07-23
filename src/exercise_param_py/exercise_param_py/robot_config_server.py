"""Provide the Python mobile robot parameter server exercise."""

import math

import rclpy
from rcl_interfaces.msg import SetParametersResult
from rclpy.node import Node
from rclpy.parameter import Parameter


class RobotConfigServer(Node):
    """Manage and validate mobile robot configuration parameters."""

    def __init__(self):
        """Create the parameter server and declare robot parameters."""
        super().__init__("robot_config_server_py")
        self.get_logger().info("机器人参数服务端创建成功!(Python)")

        self.declare_parameter("robot_name", "delivery_robot")
        self.declare_parameter("max_linear_speed", 1.0)
        self.declare_parameter("max_angular_speed", 1.5)
        self.declare_parameter("wheel_radius", 0.10)
        self.declare_parameter("obstacle_stop_distance", 0.50)
        self.declare_parameter("safety_enabled", True)

        self.add_on_set_parameters_callback(self.validate_parameters)

        self.print_robot_config()
        self.timer_ = self.create_timer(2.0, self.print_robot_config)

    def reject_parameter(self, reason: str) -> SetParametersResult:
        """Create a failed parameter result and record its reason."""
        self.get_logger().warning(f"参数修改被拒绝: {reason}")
        return SetParametersResult(successful=False, reason=reason)

    def validate_parameters(
        self,
        parameters: list[Parameter]
    ) -> SetParametersResult:
        """Validate every parameter before accepting the complete request."""
        for parameter in parameters:
            name = parameter.name

            if name == "robot_name":
                if parameter.type_ != Parameter.Type.STRING:
                    return self.reject_parameter("robot_name 必须是字符串")

                if not parameter.value.strip():
                    return self.reject_parameter(
                        "robot_name 不能为空或只包含空格"
                    )
            elif name == "max_linear_speed":
                if parameter.type_ != Parameter.Type.DOUBLE:
                    return self.reject_parameter(
                        "max_linear_speed 必须是浮点数"
                    )

                value = parameter.value
                if not math.isfinite(value):
                    return self.reject_parameter(
                        "max_linear_speed 必须是有限数值"
                    )
                if value <= 0.0 or value > 2.0:
                    return self.reject_parameter(
                        "max_linear_speed 必须满足 0.0 < value <= 2.0"
                    )
            elif name == "max_angular_speed":
                if parameter.type_ != Parameter.Type.DOUBLE:
                    return self.reject_parameter(
                        "max_angular_speed 必须是浮点数"
                    )

                value = parameter.value
                if not math.isfinite(value):
                    return self.reject_parameter(
                        "max_angular_speed 必须是有限数值"
                    )
                if value <= 0.0 or value > 4.0:
                    return self.reject_parameter(
                        "max_angular_speed 必须满足 0.0 < value <= 4.0"
                    )
            elif name == "wheel_radius":
                if parameter.type_ != Parameter.Type.DOUBLE:
                    return self.reject_parameter(
                        "wheel_radius 必须是浮点数"
                    )

                value = parameter.value
                if not math.isfinite(value):
                    return self.reject_parameter(
                        "wheel_radius 必须是有限数值"
                    )
                if value < 0.05 or value > 0.50:
                    return self.reject_parameter(
                        "wheel_radius 必须满足 0.05 <= value <= 0.50"
                    )
            elif name == "obstacle_stop_distance":
                if parameter.type_ != Parameter.Type.DOUBLE:
                    return self.reject_parameter(
                        "obstacle_stop_distance 必须是浮点数"
                    )

                value = parameter.value
                if not math.isfinite(value):
                    return self.reject_parameter(
                        "obstacle_stop_distance 必须是有限数值"
                    )
                if value < 0.10 or value > 3.00:
                    return self.reject_parameter(
                        "obstacle_stop_distance 必须满足 "
                        "0.10 <= value <= 3.00"
                    )
            elif name == "safety_enabled":
                if parameter.type_ != Parameter.Type.BOOL:
                    return self.reject_parameter(
                        "safety_enabled 必须是布尔值"
                    )
            else:
                return self.reject_parameter(f"不支持修改参数: {name}")

        self.get_logger().info("参数检查通过, 接受本次修改!")
        return SetParametersResult(
            successful=True,
            reason="参数检查通过"
        )

    def print_robot_config(self):
        """Read and print the current mobile robot configuration."""
        robot_name = self.get_parameter("robot_name").value
        max_linear_speed = self.get_parameter("max_linear_speed").value
        max_angular_speed = self.get_parameter("max_angular_speed").value
        wheel_radius = self.get_parameter("wheel_radius").value
        obstacle_stop_distance = self.get_parameter(
            "obstacle_stop_distance"
        ).value
        safety_enabled = self.get_parameter("safety_enabled").value
        max_wheel_speed = max_linear_speed / wheel_radius

        self.get_logger().info("====================")
        self.get_logger().info("机器人配置:")
        self.get_logger().info(f"robot_name = {robot_name}")
        self.get_logger().info(
            f"max_linear_speed = {max_linear_speed:.2f} m/s"
        )
        self.get_logger().info(
            f"max_angular_speed = {max_angular_speed:.2f} rad/s"
        )
        self.get_logger().info(f"wheel_radius = {wheel_radius:.2f} m")
        self.get_logger().info(
            f"obstacle_stop_distance = {obstacle_stop_distance:.2f} m"
        )
        self.get_logger().info(
            f"safety_enabled = {'true' if safety_enabled else 'false'}"
        )
        self.get_logger().info(
            f"max_wheel_speed = {max_wheel_speed:.2f} rad/s"
        )
        self.get_logger().info("====================")


def main():
    """Run the Python mobile robot parameter server."""
    rclpy.init()
    server = RobotConfigServer()

    try:
        rclpy.spin(server)
    except KeyboardInterrupt:
        pass
    finally:
        server.destroy_node()

        if rclpy.ok():
            rclpy.shutdown()


if __name__ == "__main__":
    main()

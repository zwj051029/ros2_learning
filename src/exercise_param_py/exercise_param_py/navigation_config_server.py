"""Provide the Python robot navigation parameter server exercise."""

from dataclasses import dataclass
import math

import rclpy
from rcl_interfaces.msg import SetParametersResult
from rclpy.node import Node
from rclpy.parameter import Parameter


@dataclass
class NavigationConfig:
    """Store a complete candidate navigation configuration."""

    navigation_mode: str
    max_linear_speed: float
    max_angular_speed: float
    max_deceleration: float
    obstacle_stop_distance: float
    safety_enabled: bool


def calculate_braking_distance(config: NavigationConfig) -> float:
    """Calculate the minimum safe braking distance."""
    return (
        config.max_linear_speed ** 2
        / (2.0 * config.max_deceleration)
        + 0.2
    )


class NavigationConfigServer(Node):
    """Manage and validate robot navigation configuration parameters."""

    def __init__(self):
        """Create the navigation parameter server."""
        super().__init__("navigation_config_server_py")
        self.get_logger().info(
            "机器人导航参数服务端创建成功!(Python)"
        )

        self.declare_parameter("navigation_mode", "standby")
        self.declare_parameter("max_linear_speed", 0.0)
        self.declare_parameter("max_angular_speed", 0.0)
        self.declare_parameter("max_deceleration", 1.0)
        self.declare_parameter("obstacle_stop_distance", 0.5)
        self.declare_parameter("safety_enabled", True)

        self.add_on_set_parameters_callback(self.validate_parameters)

        self.print_navigation_config()
        self.timer_ = self.create_timer(
            2.0,
            self.print_navigation_config
        )

    def get_current_config(self) -> NavigationConfig:
        """Read all current parameters into one configuration object."""
        return NavigationConfig(
            navigation_mode=self.get_parameter(
                "navigation_mode"
            ).value,
            max_linear_speed=self.get_parameter(
                "max_linear_speed"
            ).value,
            max_angular_speed=self.get_parameter(
                "max_angular_speed"
            ).value,
            max_deceleration=self.get_parameter(
                "max_deceleration"
            ).value,
            obstacle_stop_distance=self.get_parameter(
                "obstacle_stop_distance"
            ).value,
            safety_enabled=self.get_parameter(
                "safety_enabled"
            ).value
        )

    def reject_parameter(self, reason: str) -> SetParametersResult:
        """Create a failed result and record the rejection reason."""
        self.get_logger().warning(f"导航参数修改被拒绝: {reason}")
        return SetParametersResult(successful=False, reason=reason)

    def validate_config(
        self,
        config: NavigationConfig
    ) -> SetParametersResult:
        """Validate mode rules and relationships in a complete config."""
        known_modes = {
            "standby",
            "indoor",
            "outdoor",
            "docking"
        }

        if config.navigation_mode not in known_modes:
            return self.reject_parameter(
                "navigation_mode 必须是 "
                "standby、indoor、outdoor 或 docking"
            )

        finite_values = (
            config.max_linear_speed,
            config.max_angular_speed,
            config.max_deceleration,
            config.obstacle_stop_distance
        )
        if not all(math.isfinite(value) for value in finite_values):
            return self.reject_parameter(
                "所有浮点参数都必须是有限数值"
            )

        if not 0.1 <= config.max_deceleration <= 3.0:
            return self.reject_parameter(
                "max_deceleration 必须满足 0.1 <= value <= 3.0"
            )

        if not 0.1 <= config.obstacle_stop_distance <= 5.0:
            return self.reject_parameter(
                "obstacle_stop_distance 必须满足 "
                "0.1 <= value <= 5.0"
            )

        if config.navigation_mode == "standby":
            if (
                config.max_linear_speed != 0.0
                or config.max_angular_speed != 0.0
            ):
                return self.reject_parameter(
                    "standby 模式下线速度和角速度必须都是 0.0"
                )
        elif config.navigation_mode == "indoor":
            if not 0.0 < config.max_linear_speed <= 0.8:
                return self.reject_parameter(
                    "indoor 模式下 max_linear_speed 必须满足 "
                    "0.0 < value <= 0.8"
                )
            if not 0.0 < config.max_angular_speed <= 1.5:
                return self.reject_parameter(
                    "indoor 模式下 max_angular_speed 必须满足 "
                    "0.0 < value <= 1.5"
                )
        elif config.navigation_mode == "outdoor":
            if not 0.0 < config.max_linear_speed <= 2.0:
                return self.reject_parameter(
                    "outdoor 模式下 max_linear_speed 必须满足 "
                    "0.0 < value <= 2.0"
                )
            if not 0.0 < config.max_angular_speed <= 2.5:
                return self.reject_parameter(
                    "outdoor 模式下 max_angular_speed 必须满足 "
                    "0.0 < value <= 2.5"
                )
        else:
            if not 0.0 < config.max_linear_speed <= 0.2:
                return self.reject_parameter(
                    "docking 模式下 max_linear_speed 必须满足 "
                    "0.0 < value <= 0.2"
                )
            if not 0.0 < config.max_angular_speed <= 0.5:
                return self.reject_parameter(
                    "docking 模式下 max_angular_speed 必须满足 "
                    "0.0 < value <= 0.5"
                )

        if (
            config.navigation_mode != "standby"
            and not config.safety_enabled
        ):
            return self.reject_parameter(
                "机器人运动时必须启用 safety_enabled"
            )

        braking_distance = calculate_braking_distance(config)
        if (
            config.navigation_mode != "standby"
            and config.obstacle_stop_distance < braking_distance
        ):
            return self.reject_parameter(
                "obstacle_stop_distance "
                "不能小于计算出的最小安全停车距离"
            )

        return SetParametersResult(
            successful=True,
            reason="导航配置检查通过"
        )

    def validate_parameters(
        self,
        parameters: list[Parameter]
    ) -> SetParametersResult:
        """Overlay requested values and validate the complete candidate."""
        candidate = self.get_current_config()

        for parameter in parameters:
            name = parameter.name

            if name == "navigation_mode":
                if parameter.type_ != Parameter.Type.STRING:
                    return self.reject_parameter(
                        "navigation_mode 必须是字符串"
                    )
                candidate.navigation_mode = parameter.value
            elif name == "max_linear_speed":
                if parameter.type_ != Parameter.Type.DOUBLE:
                    return self.reject_parameter(
                        "max_linear_speed 必须是浮点数"
                    )
                candidate.max_linear_speed = parameter.value
            elif name == "max_angular_speed":
                if parameter.type_ != Parameter.Type.DOUBLE:
                    return self.reject_parameter(
                        "max_angular_speed 必须是浮点数"
                    )
                candidate.max_angular_speed = parameter.value
            elif name == "max_deceleration":
                if parameter.type_ != Parameter.Type.DOUBLE:
                    return self.reject_parameter(
                        "max_deceleration 必须是浮点数"
                    )
                candidate.max_deceleration = parameter.value
            elif name == "obstacle_stop_distance":
                if parameter.type_ != Parameter.Type.DOUBLE:
                    return self.reject_parameter(
                        "obstacle_stop_distance 必须是浮点数"
                    )
                candidate.obstacle_stop_distance = parameter.value
            elif name == "safety_enabled":
                if parameter.type_ != Parameter.Type.BOOL:
                    return self.reject_parameter(
                        "safety_enabled 必须是布尔值"
                    )
                candidate.safety_enabled = parameter.value
            else:
                return self.reject_parameter(
                    f"不支持修改参数: {name}"
                )

        result = self.validate_config(candidate)
        if result.successful:
            self.get_logger().info(
                "导航配置检查通过, 接受本次原子修改!"
            )
        return result

    def print_navigation_config(self):
        """Print the current navigation configuration and derived data."""
        config = self.get_current_config()
        braking_distance = calculate_braking_distance(config)

        self.get_logger().info("====================")
        self.get_logger().info("机器人导航配置:")
        self.get_logger().info(
            f"navigation_mode = {config.navigation_mode}"
        )
        self.get_logger().info(
            f"max_linear_speed = "
            f"{config.max_linear_speed:.2f} m/s"
        )
        self.get_logger().info(
            f"max_angular_speed = "
            f"{config.max_angular_speed:.2f} rad/s"
        )
        self.get_logger().info(
            f"max_deceleration = "
            f"{config.max_deceleration:.2f} m/s^2"
        )
        self.get_logger().info(
            f"obstacle_stop_distance = "
            f"{config.obstacle_stop_distance:.2f} m"
        )
        self.get_logger().info(
            f"safety_enabled = "
            f"{'true' if config.safety_enabled else 'false'}"
        )
        self.get_logger().info(
            f"braking_distance = {braking_distance:.2f} m"
        )
        self.get_logger().info("config_status = valid")
        self.get_logger().info("====================")


def main():
    """Run the Python robot navigation parameter server."""
    rclpy.init()
    server = NavigationConfigServer()

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

#include "rcl_interfaces/msg/set_parameters_result.hpp"
#include "rclcpp/rclcpp.hpp"

#include <cmath>
#include <functional>
#include <string>
#include <vector>

using namespace std::chrono_literals;
using std::placeholders::_1;

class RobotConfigServer : public rclcpp::Node {
public:
    RobotConfigServer() : Node("robot_config_server_cpp") {
        RCLCPP_INFO(this->get_logger(), "机器人参数服务端创建成功!");

        this->declare_parameter("robot_name", "delivery_robot");
        this->declare_parameter("max_linear_speed", 1.0);
        this->declare_parameter("max_angular_speed", 1.5);
        this->declare_parameter("wheel_radius", 0.10);
        this->declare_parameter("obstacle_stop_distance", 0.50);
        this->declare_parameter("safety_enabled", true);

        parameter_callback_handle_ = this->add_on_set_parameters_callback(
            std::bind(&RobotConfigServer::validate_parameters, this, _1)
        );

        print_robot_config();
        timer_ = this->create_wall_timer(2s, std::bind(&RobotConfigServer::print_robot_config, this));
    }

private:
    rcl_interfaces::msg::SetParametersResult reject_parameter(const std::string &reason) {
        auto result = rcl_interfaces::msg::SetParametersResult();
        result.successful = false;
        result.reason = reason;
        RCLCPP_WARN(this->get_logger(), "参数修改被拒绝: %s", reason.c_str());
        return result;
    }

    rcl_interfaces::msg::SetParametersResult validate_parameters(
        const std::vector<rclcpp::Parameter> &parameters
    ) {
        for (const auto &parameter : parameters) {
            const std::string &name = parameter.get_name();

            if (name == "robot_name") {
                if (parameter.get_type() != rclcpp::ParameterType::PARAMETER_STRING) {
                    return reject_parameter("robot_name 必须是字符串");
                }

                if (parameter.as_string().find_first_not_of(" \t\n\r") == std::string::npos) {
                    return reject_parameter("robot_name 不能为空或只包含空格");
                }
            } else if (name == "max_linear_speed") {
                if (parameter.get_type() != rclcpp::ParameterType::PARAMETER_DOUBLE) {
                    return reject_parameter("max_linear_speed 必须是浮点数");
                }

                const double value = parameter.as_double();
                if (!std::isfinite(value)) {
                    return reject_parameter("max_linear_speed 必须是有限数值");
                }
                if (value <= 0.0 || value > 2.0) {
                    return reject_parameter("max_linear_speed 必须满足 0.0 < value <= 2.0");
                }
            } else if (name == "max_angular_speed") {
                if (parameter.get_type() != rclcpp::ParameterType::PARAMETER_DOUBLE) {
                    return reject_parameter("max_angular_speed 必须是浮点数");
                }

                const double value = parameter.as_double();
                if (!std::isfinite(value)) {
                    return reject_parameter("max_angular_speed 必须是有限数值");
                }
                if (value <= 0.0 || value > 4.0) {
                    return reject_parameter("max_angular_speed 必须满足 0.0 < value <= 4.0");
                }
            } else if (name == "wheel_radius") {
                if (parameter.get_type() != rclcpp::ParameterType::PARAMETER_DOUBLE) {
                    return reject_parameter("wheel_radius 必须是浮点数");
                }

                const double value = parameter.as_double();
                if (!std::isfinite(value)) {
                    return reject_parameter("wheel_radius 必须是有限数值");
                }
                if (value < 0.05 || value > 0.50) {
                    return reject_parameter("wheel_radius 必须满足 0.05 <= value <= 0.50");
                }
            } else if (name == "obstacle_stop_distance") {
                if (parameter.get_type() != rclcpp::ParameterType::PARAMETER_DOUBLE) {
                    return reject_parameter("obstacle_stop_distance 必须是浮点数");
                }

                const double value = parameter.as_double();
                if (!std::isfinite(value)) {
                    return reject_parameter("obstacle_stop_distance 必须是有限数值");
                }
                if (value < 0.10 || value > 3.00) {
                    return reject_parameter("obstacle_stop_distance 必须满足 0.10 <= value <= 3.00");
                }
            } else if (name == "safety_enabled") {
                if (parameter.get_type() != rclcpp::ParameterType::PARAMETER_BOOL) {
                    return reject_parameter("safety_enabled 必须是布尔值");
                }
            } else {
                return reject_parameter("不支持修改参数: " + name);
            }
        }

        auto result = rcl_interfaces::msg::SetParametersResult();
        result.successful = true;
        result.reason = "参数检查通过";
        RCLCPP_INFO(this->get_logger(), "参数检查通过, 接受本次修改!");
        return result;
    }

    void print_robot_config() {
        const std::string robot_name = this->get_parameter("robot_name").as_string();
        const double max_linear_speed = this->get_parameter("max_linear_speed").as_double();
        const double max_angular_speed = this->get_parameter("max_angular_speed").as_double();
        const double wheel_radius = this->get_parameter("wheel_radius").as_double();
        const double obstacle_stop_distance = this->get_parameter("obstacle_stop_distance").as_double();
        const bool safety_enabled = this->get_parameter("safety_enabled").as_bool();
        const double max_wheel_speed = max_linear_speed / wheel_radius;

        RCLCPP_INFO(this->get_logger(), "====================");
        RCLCPP_INFO(this->get_logger(), "机器人配置:");
        RCLCPP_INFO(this->get_logger(), "robot_name = %s", robot_name.c_str());
        RCLCPP_INFO(this->get_logger(), "max_linear_speed = %.2lf m/s", max_linear_speed);
        RCLCPP_INFO(this->get_logger(), "max_angular_speed = %.2lf rad/s", max_angular_speed);
        RCLCPP_INFO(this->get_logger(), "wheel_radius = %.2lf m", wheel_radius);
        RCLCPP_INFO(this->get_logger(), "obstacle_stop_distance = %.2lf m", obstacle_stop_distance);
        RCLCPP_INFO(this->get_logger(), "safety_enabled = %s", safety_enabled ? "true" : "false");
        RCLCPP_INFO(this->get_logger(), "max_wheel_speed = %.2lf rad/s", max_wheel_speed);
        RCLCPP_INFO(this->get_logger(), "====================");
    }

    rclcpp::node_interfaces::OnSetParametersCallbackHandle::SharedPtr parameter_callback_handle_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<RobotConfigServer>());
    rclcpp::shutdown();

    return 0;
}

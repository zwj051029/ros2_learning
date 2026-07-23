#include "rcl_interfaces/msg/set_parameters_result.hpp"
#include "rclcpp/rclcpp.hpp"

#include <cmath>
#include <functional>
#include <string>
#include <vector>

using namespace std::chrono_literals;
using std::placeholders::_1;

struct NavigationConfig {
    std::string navigation_mode;
    double max_linear_speed;
    double max_angular_speed;
    double max_deceleration;
    double obstacle_stop_distance;
    bool safety_enabled;
};

double calculate_braking_distance(const NavigationConfig &config) {
    return config.max_linear_speed * config.max_linear_speed / (2.0 * config.max_deceleration) + 0.2;
}

class NavigationConfigServer : public rclcpp::Node {
public:
    NavigationConfigServer() : Node("navigation_config_server_cpp") {
        RCLCPP_INFO(this->get_logger(), "机器人导航参数服务端创建成功!");

        this->declare_parameter("navigation_mode", "standby");
        this->declare_parameter("max_linear_speed", 0.0);
        this->declare_parameter("max_angular_speed", 0.0);
        this->declare_parameter("max_deceleration", 1.0);
        this->declare_parameter("obstacle_stop_distance", 0.5);
        this->declare_parameter("safety_enabled", true);

        parameter_callback_handle_ = this->add_on_set_parameters_callback(
            std::bind(&NavigationConfigServer::validate_parameters, this, _1)
        );

        print_navigation_config();
        timer_ = this->create_wall_timer(2s, std::bind(&NavigationConfigServer::print_navigation_config, this));
    }

private:
    NavigationConfig get_current_config() const {
        NavigationConfig config;
        config.navigation_mode = this->get_parameter("navigation_mode").as_string();
        config.max_linear_speed = this->get_parameter("max_linear_speed").as_double();
        config.max_angular_speed = this->get_parameter("max_angular_speed").as_double();
        config.max_deceleration = this->get_parameter("max_deceleration").as_double();
        config.obstacle_stop_distance = this->get_parameter("obstacle_stop_distance").as_double();
        config.safety_enabled = this->get_parameter("safety_enabled").as_bool();
        return config;
    }

    rcl_interfaces::msg::SetParametersResult reject_parameter(const std::string &reason) {
        auto result = rcl_interfaces::msg::SetParametersResult();
        result.successful = false;
        result.reason = reason;
        RCLCPP_WARN(this->get_logger(), "导航参数修改被拒绝: %s", reason.c_str());
        return result;
    }

    rcl_interfaces::msg::SetParametersResult validate_config(const NavigationConfig &config) {
        const bool known_mode =
            config.navigation_mode == "standby" ||
            config.navigation_mode == "indoor" ||
            config.navigation_mode == "outdoor" ||
            config.navigation_mode == "docking";

        if (!known_mode) {
            return reject_parameter("navigation_mode 必须是 standby、indoor、outdoor 或 docking");
        }

        if (
            !std::isfinite(config.max_linear_speed) ||
            !std::isfinite(config.max_angular_speed) ||
            !std::isfinite(config.max_deceleration) ||
            !std::isfinite(config.obstacle_stop_distance)
        ) {
            return reject_parameter("所有浮点参数都必须是有限数值");
        }

        if (config.max_deceleration < 0.1 || config.max_deceleration > 3.0) {
            return reject_parameter("max_deceleration 必须满足 0.1 <= value <= 3.0");
        }

        if (config.obstacle_stop_distance < 0.1 || config.obstacle_stop_distance > 5.0) {
            return reject_parameter("obstacle_stop_distance 必须满足 0.1 <= value <= 5.0");
        }

        if (config.navigation_mode == "standby") {
            if (config.max_linear_speed != 0.0 || config.max_angular_speed != 0.0) {
                return reject_parameter("standby 模式下线速度和角速度必须都是 0.0");
            }
        } else if (config.navigation_mode == "indoor") {
            if (config.max_linear_speed <= 0.0 || config.max_linear_speed > 0.8) {
                return reject_parameter("indoor 模式下 max_linear_speed 必须满足 0.0 < value <= 0.8");
            }
            if (config.max_angular_speed <= 0.0 || config.max_angular_speed > 1.5) {
                return reject_parameter("indoor 模式下 max_angular_speed 必须满足 0.0 < value <= 1.5");
            }
        } else if (config.navigation_mode == "outdoor") {
            if (config.max_linear_speed <= 0.0 || config.max_linear_speed > 2.0) {
                return reject_parameter("outdoor 模式下 max_linear_speed 必须满足 0.0 < value <= 2.0");
            }
            if (config.max_angular_speed <= 0.0 || config.max_angular_speed > 2.5) {
                return reject_parameter("outdoor 模式下 max_angular_speed 必须满足 0.0 < value <= 2.5");
            }
        } else {
            if (config.max_linear_speed <= 0.0 || config.max_linear_speed > 0.2) {
                return reject_parameter("docking 模式下 max_linear_speed 必须满足 0.0 < value <= 0.2");
            }
            if (config.max_angular_speed <= 0.0 || config.max_angular_speed > 0.5) {
                return reject_parameter("docking 模式下 max_angular_speed 必须满足 0.0 < value <= 0.5");
            }
        }

        if (config.navigation_mode != "standby" && !config.safety_enabled) {
            return reject_parameter("机器人运动时必须启用 safety_enabled");
        }

        const double braking_distance = calculate_braking_distance(config);
        if (
            config.navigation_mode != "standby" &&
            config.obstacle_stop_distance < braking_distance
        ) {
            return reject_parameter("obstacle_stop_distance 不能小于计算出的最小安全停车距离");
        }

        auto result = rcl_interfaces::msg::SetParametersResult();
        result.successful = true;
        result.reason = "导航配置检查通过";
        return result;
    }

    rcl_interfaces::msg::SetParametersResult validate_parameters(
        const std::vector<rclcpp::Parameter> &parameters
    ) {
        NavigationConfig candidate = get_current_config();

        for (const auto &parameter : parameters) {
            const std::string &name = parameter.get_name();

            if (name == "navigation_mode") {
                if (parameter.get_type() != rclcpp::ParameterType::PARAMETER_STRING) {
                    return reject_parameter("navigation_mode 必须是字符串");
                }
                candidate.navigation_mode = parameter.as_string();
            } else if (name == "max_linear_speed") {
                if (parameter.get_type() != rclcpp::ParameterType::PARAMETER_DOUBLE) {
                    return reject_parameter("max_linear_speed 必须是浮点数");
                }
                candidate.max_linear_speed = parameter.as_double();
            } else if (name == "max_angular_speed") {
                if (parameter.get_type() != rclcpp::ParameterType::PARAMETER_DOUBLE) {
                    return reject_parameter("max_angular_speed 必须是浮点数");
                }
                candidate.max_angular_speed = parameter.as_double();
            } else if (name == "max_deceleration") {
                if (parameter.get_type() != rclcpp::ParameterType::PARAMETER_DOUBLE) {
                    return reject_parameter("max_deceleration 必须是浮点数");
                }
                candidate.max_deceleration = parameter.as_double();
            } else if (name == "obstacle_stop_distance") {
                if (parameter.get_type() != rclcpp::ParameterType::PARAMETER_DOUBLE) {
                    return reject_parameter("obstacle_stop_distance 必须是浮点数");
                }
                candidate.obstacle_stop_distance = parameter.as_double();
            } else if (name == "safety_enabled") {
                if (parameter.get_type() != rclcpp::ParameterType::PARAMETER_BOOL) {
                    return reject_parameter("safety_enabled 必须是布尔值");
                }
                candidate.safety_enabled = parameter.as_bool();
            } else {
                return reject_parameter("不支持修改参数: " + name);
            }
        }

        auto result = validate_config(candidate);
        if (result.successful) {
            RCLCPP_INFO(this->get_logger(), "导航配置检查通过, 接受本次原子修改!");
        }
        return result;
    }

    void print_navigation_config() {
        const NavigationConfig config = get_current_config();
        const double braking_distance = calculate_braking_distance(config);

        RCLCPP_INFO(this->get_logger(), "====================");
        RCLCPP_INFO(this->get_logger(), "机器人导航配置:");
        RCLCPP_INFO(this->get_logger(), "navigation_mode = %s", config.navigation_mode.c_str());
        RCLCPP_INFO(this->get_logger(), "max_linear_speed = %.2lf m/s", config.max_linear_speed);
        RCLCPP_INFO(this->get_logger(), "max_angular_speed = %.2lf rad/s", config.max_angular_speed);
        RCLCPP_INFO(this->get_logger(), "max_deceleration = %.2lf m/s^2", config.max_deceleration);
        RCLCPP_INFO(this->get_logger(), "obstacle_stop_distance = %.2lf m", config.obstacle_stop_distance);
        RCLCPP_INFO(this->get_logger(), "safety_enabled = %s", config.safety_enabled ? "true" : "false");
        RCLCPP_INFO(this->get_logger(), "braking_distance = %.2lf m", braking_distance);
        RCLCPP_INFO(this->get_logger(), "config_status = valid");
        RCLCPP_INFO(this->get_logger(), "====================");
    }

    rclcpp::node_interfaces::OnSetParametersCallbackHandle::SharedPtr parameter_callback_handle_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<NavigationConfigServer>());
    rclcpp::shutdown();

    return 0;
}

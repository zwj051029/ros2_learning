#include "rclcpp/parameter_client.hpp"
#include "rclcpp/rclcpp.hpp"

#include <cmath>
#include <exception>
#include <optional>
#include <set>
#include <string>
#include <vector>

using namespace std::chrono_literals;

const std::vector<std::string> NAVIGATION_PARAMETER_NAMES = {
    "navigation_mode",
    "max_linear_speed",
    "max_angular_speed",
    "max_deceleration",
    "obstacle_stop_distance",
    "safety_enabled"
};

class NavigationConfigClient : public rclcpp::Node {
public:
    NavigationConfigClient() : Node("navigation_config_client_cpp") {
        RCLCPP_INFO(this->get_logger(), "机器人导航参数客户端创建成功!");
        client_ = std::make_shared<rclcpp::SyncParametersClient>(this, "navigation_config_server_cpp");
    }

    bool connect_server() {
        while (!client_->wait_for_service(1s)) {
            if (!rclcpp::ok()) {
                RCLCPP_ERROR(this->get_logger(), "强制退出等待!");
                return false;
            }
            RCLCPP_INFO(this->get_logger(), "导航参数服务端连接中...");
        }

        RCLCPP_INFO(this->get_logger(), "导航参数服务端连接成功!");
        return true;
    }

    bool show_parameters() {
        const auto parameters = client_->get_parameters(NAVIGATION_PARAMETER_NAMES);
        if (parameters.size() != NAVIGATION_PARAMETER_NAMES.size()) {
            RCLCPP_ERROR(this->get_logger(), "未能获取完整的导航参数!");
            return false;
        }

        const std::string navigation_mode = parameters[0].as_string();
        const double max_linear_speed = parameters[1].as_double();
        const double max_angular_speed = parameters[2].as_double();
        const double max_deceleration = parameters[3].as_double();
        const double obstacle_stop_distance = parameters[4].as_double();
        const bool safety_enabled = parameters[5].as_bool();
        const double braking_distance =
            max_linear_speed * max_linear_speed / (2.0 * max_deceleration) + 0.2;

        RCLCPP_INFO(this->get_logger(), "====================");
        RCLCPP_INFO(this->get_logger(), "机器人导航配置:");
        RCLCPP_INFO(this->get_logger(), "navigation_mode = %s", navigation_mode.c_str());
        RCLCPP_INFO(this->get_logger(), "max_linear_speed = %.2lf m/s", max_linear_speed);
        RCLCPP_INFO(this->get_logger(), "max_angular_speed = %.2lf rad/s", max_angular_speed);
        RCLCPP_INFO(this->get_logger(), "max_deceleration = %.2lf m/s^2", max_deceleration);
        RCLCPP_INFO(this->get_logger(), "obstacle_stop_distance = %.2lf m", obstacle_stop_distance);
        RCLCPP_INFO(this->get_logger(), "safety_enabled = %s", safety_enabled ? "true" : "false");
        RCLCPP_INFO(this->get_logger(), "braking_distance = %.2lf m", braking_distance);
        RCLCPP_INFO(this->get_logger(), "====================");
        return true;
    }

    bool set_parameters(const std::vector<rclcpp::Parameter> &parameters) {
        const auto result = client_->set_parameters_atomically(parameters);
        if (!result.successful) {
            RCLCPP_ERROR(this->get_logger(), "导航参数修改失败: %s", result.reason.c_str());
            return false;
        }

        RCLCPP_INFO(this->get_logger(), "导航参数修改成功!");
        return show_parameters();
    }

    bool switch_mode(const std::string &navigation_mode) {
        std::vector<rclcpp::Parameter> parameters;

        if (navigation_mode == "standby") {
            parameters = {
                rclcpp::Parameter("navigation_mode", "standby"),
                rclcpp::Parameter("max_linear_speed", 0.0),
                rclcpp::Parameter("max_angular_speed", 0.0),
                rclcpp::Parameter("max_deceleration", 1.0),
                rclcpp::Parameter("obstacle_stop_distance", 0.5),
                rclcpp::Parameter("safety_enabled", true)
            };
        } else if (navigation_mode == "indoor") {
            parameters = {
                rclcpp::Parameter("navigation_mode", "indoor"),
                rclcpp::Parameter("max_linear_speed", 0.6),
                rclcpp::Parameter("max_angular_speed", 1.0),
                rclcpp::Parameter("max_deceleration", 1.2),
                rclcpp::Parameter("obstacle_stop_distance", 0.5),
                rclcpp::Parameter("safety_enabled", true)
            };
        } else if (navigation_mode == "outdoor") {
            parameters = {
                rclcpp::Parameter("navigation_mode", "outdoor"),
                rclcpp::Parameter("max_linear_speed", 1.5),
                rclcpp::Parameter("max_angular_speed", 2.0),
                rclcpp::Parameter("max_deceleration", 1.5),
                rclcpp::Parameter("obstacle_stop_distance", 1.2),
                rclcpp::Parameter("safety_enabled", true)
            };
        } else if (navigation_mode == "docking") {
            parameters = {
                rclcpp::Parameter("navigation_mode", "docking"),
                rclcpp::Parameter("max_linear_speed", 0.15),
                rclcpp::Parameter("max_angular_speed", 0.3),
                rclcpp::Parameter("max_deceleration", 0.5),
                rclcpp::Parameter("obstacle_stop_distance", 0.3),
                rclcpp::Parameter("safety_enabled", true)
            };
        } else {
            RCLCPP_ERROR(
                this->get_logger(),
                "不支持的导航模式: %s",
                navigation_mode.c_str()
            );
            return false;
        }

        return set_parameters(parameters);
    }

    bool reset_parameters() {
        return switch_mode("standby");
    }

private:
    rclcpp::SyncParametersClient::SharedPtr client_;
};

void print_usage() {
    RCLCPP_INFO(rclcpp::get_logger("navigation_config_client_cpp"), "使用方法:");
    RCLCPP_INFO(rclcpp::get_logger("navigation_config_client_cpp"), "  navigation_config_client show");
    RCLCPP_INFO(
        rclcpp::get_logger("navigation_config_client_cpp"),
        "  navigation_config_client switch <standby|indoor|outdoor|docking>"
    );
    RCLCPP_INFO(
        rclcpp::get_logger("navigation_config_client_cpp"),
        "  navigation_config_client set <parameter_name> <value> [<parameter_name> <value> ...]"
    );
    RCLCPP_INFO(rclcpp::get_logger("navigation_config_client_cpp"), "  navigation_config_client reset");
}

bool parse_finite_number(const char *text, const char *parameter_name, double &value) {
    const std::string input(text);
    std::size_t parsed_characters = 0;

    try {
        value = std::stod(input, &parsed_characters);
    } catch (const std::exception &) {
        RCLCPP_ERROR(
            rclcpp::get_logger("navigation_config_client_cpp"),
            "%s 必须是有效数值: %s",
            parameter_name,
            text
        );
        return false;
    }

    if (parsed_characters != input.size() || !std::isfinite(value)) {
        RCLCPP_ERROR(
            rclcpp::get_logger("navigation_config_client_cpp"),
            "%s 必须是有效的有限数值: %s",
            parameter_name,
            text
        );
        return false;
    }

    return true;
}

std::optional<rclcpp::Parameter> parse_navigation_parameter(
    const std::string &parameter_name,
    const char *text
) {
    if (parameter_name == "navigation_mode") {
        return rclcpp::Parameter(parameter_name, std::string(text));
    }

    if (parameter_name == "safety_enabled") {
        const std::string input(text);
        if (input == "true") {
            return rclcpp::Parameter(parameter_name, true);
        }
        if (input == "false") {
            return rclcpp::Parameter(parameter_name, false);
        }

        RCLCPP_ERROR(
            rclcpp::get_logger("navigation_config_client_cpp"),
            "safety_enabled 必须是 true 或 false: %s",
            text
        );
        return std::nullopt;
    }

    const bool is_double_parameter =
        parameter_name == "max_linear_speed" ||
        parameter_name == "max_angular_speed" ||
        parameter_name == "max_deceleration" ||
        parameter_name == "obstacle_stop_distance";

    if (!is_double_parameter) {
        RCLCPP_ERROR(
            rclcpp::get_logger("navigation_config_client_cpp"),
            "不支持的参数名称: %s",
            parameter_name.c_str()
        );
        return std::nullopt;
    }

    double value = 0.0;
    if (!parse_finite_number(text, parameter_name.c_str(), value)) {
        return std::nullopt;
    }

    return rclcpp::Parameter(parameter_name, value);
}

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);

    if (argc < 2) {
        RCLCPP_ERROR(rclcpp::get_logger("navigation_config_client_cpp"), "缺少操作类型!");
        print_usage();
        rclcpp::shutdown();
        return 1;
    }

    const std::string command(argv[1]);
    std::vector<rclcpp::Parameter> parameters;

    if ((command == "show" || command == "reset") && argc != 2) {
        RCLCPP_ERROR(rclcpp::get_logger("navigation_config_client_cpp"), "%s 操作不需要其他参数!", command.c_str());
        print_usage();
        rclcpp::shutdown();
        return 1;
    }

    if (command == "switch" && argc != 3) {
        RCLCPP_ERROR(rclcpp::get_logger("navigation_config_client_cpp"), "switch 操作需要一个模式名称!");
        print_usage();
        rclcpp::shutdown();
        return 1;
    }

    if (command == "set") {
        if (argc < 4 || (argc - 2) % 2 != 0) {
            RCLCPP_ERROR(rclcpp::get_logger("navigation_config_client_cpp"), "set 操作需要成对输入参数名称和值!");
            print_usage();
            rclcpp::shutdown();
            return 1;
        }

        std::set<std::string> parameter_names;
        for (int index = 2; index < argc; index += 2) {
            const std::string parameter_name(argv[index]);
            if (!parameter_names.insert(parameter_name).second) {
                RCLCPP_ERROR(
                    rclcpp::get_logger("navigation_config_client_cpp"),
                    "同一条命令中不能重复设置参数: %s",
                    parameter_name.c_str()
                );
                rclcpp::shutdown();
                return 1;
            }

            auto parameter = parse_navigation_parameter(parameter_name, argv[index + 1]);
            if (!parameter.has_value()) {
                rclcpp::shutdown();
                return 1;
            }
            parameters.push_back(parameter.value());
        }
    } else if (command != "show" && command != "switch" && command != "reset") {
        RCLCPP_ERROR(
            rclcpp::get_logger("navigation_config_client_cpp"),
            "不支持的操作: %s",
            command.c_str()
        );
        print_usage();
        rclcpp::shutdown();
        return 1;
    }

    auto client = std::make_shared<NavigationConfigClient>();
    if (!client->connect_server()) {
        rclcpp::shutdown();
        return 2;
    }

    bool success = false;
    try {
        if (command == "show") {
            success = client->show_parameters();
        } else if (command == "switch") {
            success = client->switch_mode(argv[2]);
        } else if (command == "reset") {
            success = client->reset_parameters();
        } else {
            success = client->set_parameters(parameters);
        }
    } catch (const std::exception &exception) {
        RCLCPP_ERROR(client->get_logger(), "导航参数操作出现异常: %s", exception.what());
        success = false;
    }

    rclcpp::shutdown();
    return success ? 0 : 3;
}

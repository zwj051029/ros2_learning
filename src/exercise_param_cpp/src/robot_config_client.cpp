#include "rclcpp/parameter_client.hpp"
#include "rclcpp/rclcpp.hpp"

#include <algorithm>
#include <cmath>
#include <exception>
#include <optional>
#include <set>
#include <string>
#include <vector>

using namespace std::chrono_literals;

const std::vector<std::string> ROBOT_PARAMETER_NAMES = {
    "robot_name",
    "max_linear_speed",
    "max_angular_speed",
    "wheel_radius",
    "obstacle_stop_distance",
    "safety_enabled"
};

class RobotConfigClient : public rclcpp::Node {
public:
    RobotConfigClient() : Node("robot_config_client_cpp") {
        RCLCPP_INFO(this->get_logger(), "机器人参数客户端创建成功!");
        client_ = std::make_shared<rclcpp::SyncParametersClient>(this, "robot_config_server_cpp");
    }

    bool connect_server() {
        while (!client_->wait_for_service(1s)) {
            if (!rclcpp::ok()) {
                RCLCPP_ERROR(this->get_logger(), "强制退出等待!");
                return false;
            }
            RCLCPP_INFO(this->get_logger(), "参数服务端连接中...");
        }

        RCLCPP_INFO(this->get_logger(), "参数服务端连接成功!");
        return true;
    }

    bool list_parameters() {
        auto result = client_->list_parameters({}, 0);
        std::sort(result.names.begin(), result.names.end());

        RCLCPP_INFO(this->get_logger(), "参数列表:");
        for (const auto &name : result.names) {
            RCLCPP_INFO(this->get_logger(), "- %s", name.c_str());
        }
        return true;
    }

    bool show_parameters() {
        const auto parameters = client_->get_parameters(ROBOT_PARAMETER_NAMES);

        RCLCPP_INFO(this->get_logger(), "机器人全部参数:");
        for (const auto &parameter : parameters) {
            print_parameter(parameter);
        }
        return true;
    }

    bool get_parameter(const std::string &parameter_name) {
        if (!client_->has_parameter(parameter_name)) {
            RCLCPP_ERROR(this->get_logger(), "参数不存在: %s", parameter_name.c_str());
            return false;
        }

        const auto parameters = client_->get_parameters({parameter_name});
        if (parameters.empty()) {
            RCLCPP_ERROR(this->get_logger(), "未收到参数查询结果!");
            return false;
        }

        print_parameter(parameters.front());
        return true;
    }

    bool set_parameters(const std::vector<rclcpp::Parameter> &parameters) {
        const auto result = client_->set_parameters_atomically(parameters);
        if (!result.successful) {
            RCLCPP_ERROR(this->get_logger(), "参数修改失败: %s", result.reason.c_str());
            return false;
        }

        RCLCPP_INFO(this->get_logger(), "参数修改成功!");
        for (const auto &parameter : parameters) {
            print_parameter(parameter);
        }
        return true;
    }

private:
    void print_parameter(const rclcpp::Parameter &parameter) {
        RCLCPP_INFO(
            this->get_logger(),
            "%s = %s",
            parameter.get_name().c_str(),
            parameter.value_to_string().c_str()
        );
    }

    rclcpp::SyncParametersClient::SharedPtr client_;
};

void print_usage() {
    RCLCPP_INFO(rclcpp::get_logger("robot_config_client_cpp"), "使用方法:");
    RCLCPP_INFO(rclcpp::get_logger("robot_config_client_cpp"), "  robot_config_client list");
    RCLCPP_INFO(rclcpp::get_logger("robot_config_client_cpp"), "  robot_config_client show");
    RCLCPP_INFO(rclcpp::get_logger("robot_config_client_cpp"), "  robot_config_client get <parameter_name>");
    RCLCPP_INFO(
        rclcpp::get_logger("robot_config_client_cpp"),
        "  robot_config_client set <parameter_name> <value> [<parameter_name> <value> ...]"
    );
}

bool parse_finite_number(const char *text, const char *parameter_name, double &value) {
    const std::string input(text);
    std::size_t parsed_characters = 0;

    try {
        value = std::stod(input, &parsed_characters);
    } catch (const std::exception &) {
        RCLCPP_ERROR(
            rclcpp::get_logger("robot_config_client_cpp"),
            "%s 必须是有效数值: %s",
            parameter_name,
            text
        );
        return false;
    }

    if (parsed_characters != input.size() || !std::isfinite(value)) {
        RCLCPP_ERROR(
            rclcpp::get_logger("robot_config_client_cpp"),
            "%s 必须是有效的有限数值: %s",
            parameter_name,
            text
        );
        return false;
    }

    return true;
}

std::optional<rclcpp::Parameter> parse_parameter(
    const std::string &parameter_name,
    const char *text
) {
    if (parameter_name == "robot_name") {
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
            rclcpp::get_logger("robot_config_client_cpp"),
            "safety_enabled 必须是 true 或 false: %s",
            text
        );
        return std::nullopt;
    }

    const bool is_double_parameter =
        parameter_name == "max_linear_speed" ||
        parameter_name == "max_angular_speed" ||
        parameter_name == "wheel_radius" ||
        parameter_name == "obstacle_stop_distance";

    if (!is_double_parameter) {
        RCLCPP_ERROR(
            rclcpp::get_logger("robot_config_client_cpp"),
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
        RCLCPP_ERROR(rclcpp::get_logger("robot_config_client_cpp"), "缺少操作类型!");
        print_usage();
        rclcpp::shutdown();
        return 1;
    }

    const std::string command(argv[1]);
    std::vector<rclcpp::Parameter> parameters;

    if ((command == "list" || command == "show") && argc != 2) {
        RCLCPP_ERROR(rclcpp::get_logger("robot_config_client_cpp"), "%s 操作不需要其他参数!", command.c_str());
        print_usage();
        rclcpp::shutdown();
        return 1;
    }

    if (command == "get" && argc != 3) {
        RCLCPP_ERROR(rclcpp::get_logger("robot_config_client_cpp"), "get 操作需要一个参数名称!");
        print_usage();
        rclcpp::shutdown();
        return 1;
    }

    if (command == "set") {
        if (argc < 4 || (argc - 2) % 2 != 0) {
            RCLCPP_ERROR(rclcpp::get_logger("robot_config_client_cpp"), "set 操作需要成对输入参数名称和值!");
            print_usage();
            rclcpp::shutdown();
            return 1;
        }

        std::set<std::string> parameter_names;
        for (int index = 2; index < argc; index += 2) {
            const std::string parameter_name(argv[index]);
            if (!parameter_names.insert(parameter_name).second) {
                RCLCPP_ERROR(
                    rclcpp::get_logger("robot_config_client_cpp"),
                    "同一条命令中不能重复设置参数: %s",
                    parameter_name.c_str()
                );
                rclcpp::shutdown();
                return 1;
            }

            auto parameter = parse_parameter(parameter_name, argv[index + 1]);
            if (!parameter.has_value()) {
                rclcpp::shutdown();
                return 1;
            }
            parameters.push_back(parameter.value());
        }
    } else if (command != "list" && command != "show" && command != "get") {
        RCLCPP_ERROR(rclcpp::get_logger("robot_config_client_cpp"), "不支持的操作: %s", command.c_str());
        print_usage();
        rclcpp::shutdown();
        return 1;
    }

    auto client = std::make_shared<RobotConfigClient>();
    if (!client->connect_server()) {
        rclcpp::shutdown();
        return 2;
    }

    bool success = false;
    try {
        if (command == "list") {
            success = client->list_parameters();
        } else if (command == "show") {
            success = client->show_parameters();
        } else if (command == "get") {
            success = client->get_parameter(argv[2]);
        } else {
            success = client->set_parameters(parameters);
        }
    } catch (const std::exception &exception) {
        RCLCPP_ERROR(client->get_logger(), "参数操作出现异常: %s", exception.what());
        success = false;
    }

    rclcpp::shutdown();
    return success ? 0 : 3;
}

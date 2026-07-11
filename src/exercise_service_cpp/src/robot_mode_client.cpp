#include "base_interfaces/srv/set_robot_mode.hpp"
#include "rclcpp/rclcpp.hpp"

using base_interfaces::srv::SetRobotMode;
using namespace std::chrono_literals;

class SetRobotModeClient : public rclcpp::Node {
public:
    SetRobotModeClient() : Node("set_robot_mode_client_cpp") {
        RCLCPP_INFO(this->get_logger(), "客户端创建成功!");
        client_ = this->create_client<SetRobotMode>("set_robot_mode");
    }

    bool connect_server() {
        while (!client_->wait_for_service(1s)) {
            if (!rclcpp::ok()) {
                RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "强制退出等待!");
                return false;
            }
            RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "服务器连接中...");
        }
        return true;
    }

    rclcpp::Client<SetRobotMode>::FutureAndRequestId send_request(std::string mode, float max_speed) {
        auto request = std::make_shared<SetRobotMode::Request>();
        request->mode = mode;
        request->max_speed = max_speed;

        /*
            rclcpp::Client<base_interfaces::srv::SetRobotMode>::FutureAndRequestId
            async_send_request(std::shared_ptr<base_interfaces::srv::SetRobotMode_Request> request)
        */
        return client_->async_send_request(request);
    }

private:
    rclcpp::Client<SetRobotMode>::SharedPtr client_;
};

int main(int argc, char **argv) {
    if (argc != 3) {
        RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "提交参数数量不合法!");
        return 1;
    }

    rclcpp::init(argc, argv);
    auto client = std::make_shared<SetRobotModeClient>();

    if (client->connect_server()) {
        RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "服务器连接成功!");
    } else {
        RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "服务器连接失败!");
        rclcpp::shutdown();
        return 2;
    }

    auto future = client->send_request(argv[1], std::stof(argv[2]));
    if (rclcpp::spin_until_future_complete(client, future) == rclcpp::FutureReturnCode::SUCCESS) {
        auto response = future.get();

        RCLCPP_INFO(client->get_logger(), "请求处理正常!");
        RCLCPP_INFO(client->get_logger(), "====================");
        RCLCPP_INFO(client->get_logger(), "success = %s", response->success ? "true" : "false");
        RCLCPP_INFO(client->get_logger(), "current_mode = %s", response->current_mode.c_str());
        RCLCPP_INFO(client->get_logger(), "current_max_speed = %0.1f", response->current_max_speed);
        RCLCPP_INFO(client->get_logger(), "message = %s", response->message.c_str());
        RCLCPP_INFO(client->get_logger(), "====================");
    } else {
        RCLCPP_ERROR(client->get_logger(), "请求处理异常!");
    }

    rclcpp::shutdown();

    return 0;
}
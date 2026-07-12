#include "base_interfaces/srv/manage_waypoint.hpp"
#include "rclcpp/rclcpp.hpp"

using base_interfaces::srv::ManageWaypoint;
using namespace std::chrono_literals;

class WaypointManagerClient : public rclcpp::Node {
public:
    WaypointManagerClient() : Node("waypoint_manager_client_cpp") {
        RCLCPP_INFO(this->get_logger(), "客户端创建成功!");
        client_ = this->create_client<ManageWaypoint>("manage_waypoint");
    }

    bool connect_server() {
        while (!client_->wait_for_service(1s)) {
            if (!rclcpp::ok()) {
                RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "强制退出等待!");
                return false;
            }
            RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "等待连接服务器...");
        }

        return true;
    }

    rclcpp::Client<ManageWaypoint>::FutureAndRequestId send_request(std::string operation, std::string waypoint_name, float x, float y, float yaw) {
        auto request = std::make_shared<ManageWaypoint::Request>();
        request->operation = operation;
        request->waypoint_name = waypoint_name;
        request->x = x;
        request->y = y;
        request->yaw = yaw;
        /*
            rclcpp::Client<base_interfaces::srv::ManageWaypoint>::FutureAndRequestId
            async_send_request (std::shared_ptr<base_interfaces::srv::ManageWaypoint_Request> request)
        */
        return client_->async_send_request(request);
    }

private:
    rclcpp::Client<ManageWaypoint>::SharedPtr client_;
};

int main(int argc, char **argv) {
    if (argc != 6) {
        RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "提交请求数目不正确!");
        return 1;
    }

    rclcpp::init(argc, argv);

    auto client = std::make_shared<WaypointManagerClient>();

    if (client->connect_server()) {
        RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "服务器连接成功!");
    } else {
        RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "服务器连接失败!");
        rclcpp::shutdown();
        return 2;
    }

    auto future = client->send_request(argv[1], argv[2], std::stof(argv[3]), std::stof(argv[4]), std::stof(argv[5]));

    if (rclcpp::spin_until_future_complete(client, future) == rclcpp::FutureReturnCode::SUCCESS) {
        RCLCPP_INFO(client->get_logger(), "请求处理正常!");
        auto response = future.get();
        RCLCPP_INFO(client->get_logger(), "====================");
        RCLCPP_INFO(client->get_logger(), "是否成功 = %s", response->success ? "True" : "False");
        RCLCPP_INFO(client->get_logger(), "消息 = %s", response->message.c_str());
        RCLCPP_INFO(client->get_logger(), "当前航点总数: %d", response->total_count);

        for (size_t i = 0; i < response->waypoint_names.size(); ++i) {
            RCLCPP_INFO(client->get_logger(), "[航点 %zu: 名称=%s, x=%.2f, y=%.2f, yaw=%.2f]", i + 1, response->waypoint_names[i].c_str(), response->xs[i], response->ys[i], response->yaws[i]);
        }

        RCLCPP_INFO(client->get_logger(), "====================");
    } else {
        RCLCPP_WARN(client->get_logger(), "请求处理异常!");
    }

    rclcpp::shutdown();

    return 0;
}
#include "base_interfaces/srv/manage_waypoint.hpp"
#include "rclcpp/rclcpp.hpp"
#include <cmath>
#include <string>
#include <vector>

using base_interfaces::srv::ManageWaypoint;
using std::placeholders::_1;
using std::placeholders::_2;

struct Waypoint {
    std::string waypoint_name;
    float x;
    float y;
    float yaw;
};

class WaypointManagerServer : public rclcpp::Node {
public:
    WaypointManagerServer() : Node("waypoint_manager_server_cpp") {
        RCLCPP_INFO(this->get_logger(), "服务端创建成功!");
        server_ = this->create_service<ManageWaypoint>("manage_waypoint", std::bind(&WaypointManagerServer::waypoint_manager_callback, this, _1, _2));
    }

private:
    void waypoint_manager_callback(const ManageWaypoint::Request::SharedPtr request, ManageWaypoint::Response::SharedPtr response) {
        const std::string &operation = request->operation;
        const std::string &waypoint_name = request->waypoint_name;
        const float x = request->x;
        const float y = request->y;
        const float yaw = request->yaw;

        if (operation == "add") {
            if (waypoint_name.empty()) {
                set_failure_response(response, "航点名称不能为空!");
            } else if (has_waypoint(waypoint_name)) {
                set_failure_response(response, "存在同名航点!");
            } else if (waypoints_.size() >= 5) {
                set_failure_response(response, "当前航点数量已达到 5 个!");
            } else if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(yaw) || x < -10.0F || x > 10.0F || y < -10.0F || y > 10.0F || yaw < -3.14F || yaw > 3.14F) {
                set_failure_response(response, "航点参数不合法!");
            } else {
                waypoints_.push_back({waypoint_name, x, y, yaw});
                response->success = true;
                response->message = "航点添加成功!";
                RCLCPP_INFO(this->get_logger(), "航点添加成功!");
            }
        } else if (operation == "delete") {
            const int index = find_waypoint_index(waypoint_name);
            if (index == -1) {
                set_failure_response(response, "该航点不存在!");
            } else {
                waypoints_.erase(waypoints_.begin() + index);
                response->success = true;
                response->message = "航点删除成功!";
                RCLCPP_INFO(this->get_logger(), "航点删除成功!");
            }
        } else if (operation == "query") {
            response->success = true;
            response->message = "航点查询成功!";
            RCLCPP_INFO(this->get_logger(), "航点查询成功!");
        } else if (operation == "clear") {
            waypoints_.clear();
            response->success = true;
            response->message = "航点清空成功!";
            RCLCPP_INFO(this->get_logger(), "航点清空成功!");
        } else {
            set_failure_response(response, "未知操作!");
        }

        fill_waypoints_response(response);
    }

    bool has_waypoint(const std::string &waypoint_name) const {
        for (const auto &waypoint : waypoints_) {
            if (waypoint.waypoint_name == waypoint_name) {
                return true;
            }
        }

        return false;
    }

    int find_waypoint_index(const std::string &waypoint_name) const {
        for (size_t i = 0; i < waypoints_.size(); ++i) {
            if (waypoints_[i].waypoint_name == waypoint_name) {
                return static_cast<int>(i);
            }
        }

        return -1;
    }

    void set_failure_response(const ManageWaypoint::Response::SharedPtr response, const std::string &message) {
        response->success = false;
        response->message = message;
        RCLCPP_WARN(this->get_logger(), "%s", message.c_str());
    }

    void fill_waypoints_response(const ManageWaypoint::Response::SharedPtr response) {
        response->waypoint_names.clear();
        response->xs.clear();
        response->ys.clear();
        response->yaws.clear();

        for (const auto &waypoint : waypoints_) {
            response->waypoint_names.push_back(waypoint.waypoint_name);
            response->xs.push_back(waypoint.x);
            response->ys.push_back(waypoint.y);
            response->yaws.push_back(waypoint.yaw);
        }

        response->total_count = static_cast<int32_t>(waypoints_.size());
    }

    rclcpp::Service<ManageWaypoint>::SharedPtr server_;
    std::vector<Waypoint> waypoints_;
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<WaypointManagerServer>());
    rclcpp::shutdown();

    return 0;
}

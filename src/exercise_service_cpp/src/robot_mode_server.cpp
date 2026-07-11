#include "base_interfaces/srv/set_robot_mode.hpp"
#include "rclcpp/rclcpp.hpp"
#include <cmath>

using base_interfaces::srv::SetRobotMode;
using std::placeholders::_1;
using std::placeholders::_2;

class SetRobotModeServer : public rclcpp::Node {
public:
    SetRobotModeServer() : Node("set_robot_mode_server_cpp"), current_mode_("standby"), current_max_speed_(0.0f) {
        RCLCPP_INFO(this->get_logger(), "服务端创建成功!");
        server_ = this->create_service<SetRobotMode>("set_robot_mode", std::bind(&SetRobotModeServer::set_robot_mode_callback, this, _1, _2));
    }

private:
    void set_robot_mode_callback(const SetRobotMode::Request::SharedPtr req, const SetRobotMode::Response::SharedPtr res) {
        if (!std::isfinite(req->max_speed)) {
            res->success = false;
            res->message = "最大速度必须是有效的有限数值";
            return;
        }

        const std::string mode = req->mode;
        const float max_speed = req->max_speed;

        res->current_mode = current_mode_;
        res->current_max_speed = current_max_speed_;

        // 采用先比较模式 再根据模式看速度是否合法的方式 若不合法则之间返回
        if (mode == "standby") {
            if (max_speed != 0.0f) {
                res->success = false;
                res->message = "待机模式下, 最大速度必须为0.0!";
                RCLCPP_WARN(this->get_logger(), "速度设置不合法!");
                return;
            }
        } else if (mode == "patrol") {
            if (max_speed <= 0.0f || max_speed > 1.0f) {
                res->success = false;
                res->message = "巡航模式下, 0.0 < max_speed <= 1.0!";
                RCLCPP_WARN(this->get_logger(), "速度设置不合法!");
                return;
            }
        } else if (mode == "return_home") {
            if (max_speed <= 0.0f || max_speed > 0.5f) {
                res->success = false;
                res->message = "返航模式下, 0.0 < max_speed <= 0.5!";
                RCLCPP_WARN(this->get_logger(), "速度设置不合法!");
                return;
            }
        } else if (mode == "emergency_stop") {
            if (max_speed != 0.0f) {
                res->success = false;
                res->message = "急停模式下, 最大速度必须为0.0!";
                RCLCPP_WARN(this->get_logger(), "速度设置不合法!");
                return;
            }
        } else {
            res->success = false;
            res->message = "未知模式";
            RCLCPP_WARN(this->get_logger(), "未知模式!");
            return;
        }

        // 若程序走到这 说明模式和速度都合法
        // 更新当前模式和最大速度
        current_mode_ = mode;
        current_max_speed_ = max_speed;

        // 设置响应
        res->success = true;
        res->current_mode = current_mode_;
        res->current_max_speed = current_max_speed_;
        res->message = "机器人模式切换成功!";
        RCLCPP_INFO(this->get_logger(), "机器人模式切换成功!");
    }

    std::string current_mode_;
    float current_max_speed_;

    rclcpp::Service<SetRobotMode>::SharedPtr server_;
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<SetRobotModeServer>());
    rclcpp::shutdown();

    return 0;
}
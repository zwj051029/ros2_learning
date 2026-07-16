#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "base_interfaces/action/rotate_robot.hpp"

using base_interfaces::action::RotateRobot;
using std::placeholders::_1;
using std::placeholders::_2;

class RotateRobotServer : public rclcpp::Node {
public:
    RotateRobotServer() : Node("rotate_robot_server_cpp") {
        RCLCPP_INFO(this->get_logger(), "动作服务端创建成功!");

        /*
            rclcpp_action::Server<ActionT>::SharedPtr
            <typename ActionT, typename NodeT> create_server(
                NodeT node, const std::string &name, rclcpp_action::Server<ActionT>::GoalCallback handle_goal, rclcpp_action::Server<ActionT>::CancelCallback handle_cancel, rclcpp_action::Server<ActionT>::AcceptedCallback handle_accepted
            )
        */
        server_ = rclcpp_action::create_server<RotateRobot>(
            this,
            "rotate_robot",
            std::bind(&RotateRobotServer::goal_callback, this, _1, _2),
            std::bind(&RotateRobotServer::cancel_callback, this, _1),
            std::bind(&RotateRobotServer::accepted_callback, this, _1)
        );
    }

private:
    /*
        std::function<GoalResponse(const GoalUUID &, std::shared_ptr<const typename ActionT::Goal>)>;
    */
    rclcpp_action::GoalResponse goal_callback(
        const rclcpp_action::GoalUUID &uuid, std::shared_ptr<const RotateRobot::Goal> goal
    ) {
        (void)uuid;

        double target_angle = goal->target_angle;
        double angular_speed = goal->angular_speed;

        if (target_angle < -180.0f || target_angle > 180.0f) {
            RCLCPP_ERROR(this->get_logger(), "目标角度必须在-180度至180度之间!");
            return rclcpp_action::GoalResponse::REJECT;
        } else if (abs(target_angle) < 0.001) {
            RCLCPP_ERROR(this->get_logger(), "目标角度太小!");
            return rclcpp_action::GoalResponse::REJECT;
        }

        if (angular_speed <= 0.0f) {
            RCLCPP_ERROR(this->get_logger(), "角速度必须为正数!");
            return rclcpp_action::GoalResponse::REJECT;
        } else if (angular_speed > 90.0f) {
            RCLCPP_ERROR(this->get_logger(), "角速度太大");
            return rclcpp_action::GoalResponse::REJECT;
        }

        RCLCPP_INFO(this->get_logger(), "接收该目标!");
        return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
    }

    /*
        std::function<CancelResponse(std::shared_ptr<ServerGoalHandle<ActionT>>)>;
    */
    rclcpp_action::CancelResponse cancel_callback(
        std::shared_ptr<rclcpp_action::ServerGoalHandle<RotateRobot>> goal_handle
    ) {
        (void)goal_handle;
        RCLCPP_INFO(this->get_logger(), "收到取消请求, 准备取消任务!");
        return rclcpp_action::CancelResponse::ACCEPT;
    }

    /*
        std::function<void (std::shared_ptr<ServerGoalHandle<ActionT>>)>;
    */
    void accepted_callback(
        std::shared_ptr<rclcpp_action::ServerGoalHandle<RotateRobot>> goal_handle
    ) {
        std::thread(std::bind(&RotateRobotServer::execute, this, goal_handle)).detach();
    }

    void execute(
        std::shared_ptr<rclcpp_action::ServerGoalHandle<RotateRobot>> goal_handle
    ) {
        // 目标相关
        double target_angle = goal_handle->get_goal()->target_angle;
        double abs_angle = abs(target_angle);
        double angular_speed = goal_handle->get_goal()->angular_speed;

        // RCLCPP_INFO(this->get_logger(), "%.2f", abs_angle);

        // 结果相关
        // double final_angle = 0.0f;
        double elapsed_time = 0.0f;

        // 反馈相关
        double current_angle = 0.0f;
        double remaining_angle = abs_angle;
        float progress = 0.0f;

        auto feedback = std::make_shared<RotateRobot::Feedback>();
        auto result = std::make_shared<RotateRobot::Result>();

        rclcpp::Rate rate(2.0);
        while (current_angle < abs_angle) {
            rate.sleep();

            // 处理取消请求
            if (goal_handle->is_canceling()) {
                result->success = false;
                result->message = "机器人旋转任务已取消!";
                result->final_angle = (target_angle > 0.0f) ? current_angle : -current_angle;
                result->elapsed_time = elapsed_time;

                RCLCPP_INFO(this->get_logger(), "--------------------");
                RCLCPP_INFO(this->get_logger(), "机器人旋转任务已取消!");
                RCLCPP_INFO(
                    this->get_logger(),
                    "[是否成功: %s, 提示信息: %s, 最终角度: %.2f, 执行时间: %.2f]",
                    result->success ? "true" : "false",
                    result->message.c_str(),
                    result->final_angle,
                    result->elapsed_time
                );
                RCLCPP_INFO(this->get_logger(), "--------------------");

                goal_handle->canceled(result);
                return;
            }

            elapsed_time += 0.5;
            current_angle += angular_speed * 0.5;
            remaining_angle = abs_angle - current_angle;
            progress = current_angle / abs_angle;

            if (current_angle >= abs_angle) {
                break;
            }

            // 正常执行
            feedback->current_angle = (target_angle > 0.0f) ? current_angle : -current_angle;
            feedback->remaining_angle = remaining_angle;
            feedback->progress = progress;

            RCLCPP_INFO(this->get_logger(), "--------------------");
            RCLCPP_INFO(this->get_logger(), "持续反馈中...");
            RCLCPP_INFO(
                this->get_logger(),
                "[当前角度: %.2f, 剩余角度: %.2f, 当前进度: %.2f]",
                feedback->current_angle,
                feedback->remaining_angle,
                feedback->progress
            );
            RCLCPP_INFO(this->get_logger(), "--------------------");
            goal_handle->publish_feedback(feedback);
        }

        feedback->current_angle = target_angle;
        feedback->remaining_angle = 0.0f;
        feedback->progress = 1.0f;

        RCLCPP_INFO(this->get_logger(), "--------------------");
        RCLCPP_INFO(this->get_logger(), "持续反馈中...");
        RCLCPP_INFO(
            this->get_logger(),
            "[当前角度: %.2f, 剩余角度: %.2f, 当前进度: %.2f]",
            feedback->current_angle,
            feedback->remaining_angle,
            feedback->progress
        );
        RCLCPP_INFO(this->get_logger(), "--------------------");
        goal_handle->publish_feedback(feedback);

        result->success = true;
        result->message = "机器人旋转完成!";
        result->final_angle = target_angle;
        result->elapsed_time = elapsed_time;

        RCLCPP_INFO(this->get_logger(), "--------------------");
        RCLCPP_INFO(this->get_logger(), "机器人旋转完成!");
        RCLCPP_INFO(
            this->get_logger(),
            "[是否成功: %s, 提示信息: %s, 最终角度: %.2f, 执行时间: %.2f]",
            result->success ? "true" : "false",
            result->message.c_str(),
            result->final_angle,
            result->elapsed_time
        );
        RCLCPP_INFO(this->get_logger(), "--------------------");

        goal_handle->succeed(result);
    }

    rclcpp_action::Server<RotateRobot>::SharedPtr server_;
};

int main(int argc, char **argv){
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<RotateRobotServer>());
    rclcpp::shutdown();

    return 0;
}

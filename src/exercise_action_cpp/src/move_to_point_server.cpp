#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "base_interfaces/action/move_to_point.hpp"

#include <cmath>

constexpr double EPS = 1e-8;

using base_interfaces::action::MoveToPoint;
using std::placeholders::_1;
using std::placeholders::_2;

inline double get_distance(const double &x1, const double &y1,
                           const double &x2, const double &y2
) {
    return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

inline bool is_equal(double a, double b) {
    return fabs(a - b) < EPS;
}

class MoveToPointServer : public rclcpp::Node {
    friend inline double get_distance(const double &x1, const double &y1, const double &x2, const double &y2);
    friend inline bool is_equal(double a, double b);

public:
    MoveToPointServer() : Node("move_to_point_server_cpp"), current_x_(0.0), current_y_(0.0) {
        RCLCPP_INFO(this->get_logger(), "动作服务端创建成功!");
        /*
            rclcpp_action::Server<ActionT>::SharedPtr create_server<ActionT, NodeT>(
                NodeT node, const std::string &name, rclcpp_action::Server<ActionT>::GoalCallback handle_goal, rclcpp_action::Server<ActionT>::CancelCallback handle_cancel, rclcpp_action::Server<ActionT>::AcceptedCallback handle_accepted
            )
        */
        server_ = rclcpp_action::create_server<MoveToPoint>(
            this,
            "move_to_point",
            std::bind(&MoveToPointServer::goal_callback, this, _1, _2),
            std::bind(&MoveToPointServer::cancel_callback, this, _1),
            std::bind(&MoveToPointServer::accepted_callback, this, _1)
        );
    }

private:
    /*
        std::function<GoalResponse(const GoalUUID &, std::shared_ptr<const typename ActionT::Goal>)>
    */
    rclcpp_action::GoalResponse goal_callback(
        const rclcpp_action::GoalUUID &uuid, std::shared_ptr<const MoveToPoint::Goal> goal
    ) {
        (void)uuid;

        double target_x = goal->target_x;
        double target_y = goal->target_y;
        double speed = goal->speed;

        if (!std::isfinite(target_x) ||
            !std::isfinite(target_y) ||
            !std::isfinite(speed)
        ) {
            RCLCPP_ERROR(this->get_logger(), "参数必须为有限数!");
            return rclcpp_action::GoalResponse::REJECT;
        }

        if (target_x < -10.0 || target_x > 10.0 ||
            target_y < -10.0 || target_y > 10.0
        ) {
            RCLCPP_ERROR(this->get_logger(), "目标坐标必须在[-10.0, 10.0]!");
            return rclcpp_action::GoalResponse::REJECT;
        }

        if (speed <= 0.0) {
            RCLCPP_ERROR(this->get_logger(), "速度必须为正数!");
            return rclcpp_action::GoalResponse::REJECT;
        } else if (speed > 1.5) {
            RCLCPP_ERROR(this->get_logger(), "速度太大!");
            return rclcpp_action::GoalResponse::REJECT;
        }

        if (abs(get_distance(target_x, target_y, current_x_, current_y_)) < 0.001) {
            RCLCPP_ERROR(this->get_logger(), "目标点距离当前位置太小!");
            return rclcpp_action::GoalResponse::REJECT;
        }

        RCLCPP_INFO(this->get_logger(), "参数全部合法!");
        return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
    }

    /*
        std::function<CancelResponse(std::shared_ptr<ServerGoalHandle<ActionT>>)>;
    */
    rclcpp_action::CancelResponse cancel_callback(
        std::shared_ptr<rclcpp_action::ServerGoalHandle<MoveToPoint>> goal_handle
    ) {
        (void)goal_handle;
        RCLCPP_INFO(this->get_logger(), "收到取消请求, 准备取消任务!");
        return rclcpp_action::CancelResponse::ACCEPT;
    }

    /*
        std::function<void (std::shared_ptr<ServerGoalHandle<ActionT>>)>;
    */
    void accepted_callback(
        std::shared_ptr<rclcpp_action::ServerGoalHandle<MoveToPoint>> goal_handle
    ) {
        std::thread(std::bind(&MoveToPointServer::execute, this, goal_handle)).detach();
    }

    void execute(
        std::shared_ptr<rclcpp_action::ServerGoalHandle<MoveToPoint>> goal_handle
    ) {
        // 目标相关
        double target_x = goal_handle->get_goal()->target_x;
        double target_y = goal_handle->get_goal()->target_y;
        double speed = goal_handle->get_goal()->speed;

        // 结果相关
        auto result = std::make_shared<MoveToPoint::Result>();
        double traveled_distance = 0.0;

        // 反馈相关
        auto feedback = std::make_shared<MoveToPoint::Feedback>();
        double start_x = current_x_;
        double start_y = current_y_;
        double remaining_distance = get_distance(target_x, target_y, start_x, start_y);
        float progress = 0.0;

        // 计算相关
        double dx = target_x - start_x;
        double dy = target_y - start_y;
        double total_dist = get_distance(target_x, target_y, start_x, start_y);
        double direction_x = dx / total_dist;
        double direction_y = dy / total_dist;

        rclcpp::Rate rate(2.0);

        while (!is_equal(target_x, current_x_) || !is_equal(target_y, current_y_)) {
            double normal_step = speed * 0.5;
            double actual_step = std::min(normal_step, remaining_distance);

            rate.sleep();

            // 处理取消请求
            if (goal_handle->is_canceling()) {
                result->success = false;
                result->message = "任务已被取消!";
                result->final_x = current_x_;
                result->final_y = current_y_;
                result->traveled_distance = traveled_distance;

                RCLCPP_INFO(this->get_logger(), "====================");
                RCLCPP_INFO(this->get_logger(), "最终结果: ");
                RCLCPP_INFO(this->get_logger(), "success = %s", result->success ? "true" : "false");
                RCLCPP_INFO(this->get_logger(), "message = %s", result->message.c_str());
                RCLCPP_INFO(this->get_logger(), "final_x = %.2lf", result->final_x);
                RCLCPP_INFO(this->get_logger(), "final_y = %.2lf", result->final_y);
                RCLCPP_INFO(this->get_logger(), "traveled_distance = %.2lf", result->traveled_distance);
                RCLCPP_INFO(this->get_logger(), "====================");

                goal_handle->canceled(result);
                return;
            }

            // 相关参数的计算
            current_x_ += direction_x * actual_step;
            current_y_ += direction_y * actual_step;
            traveled_distance += actual_step;
            remaining_distance = std::max(total_dist - traveled_distance, 0.0);
            progress = std::min(traveled_distance / total_dist, 1.0);

            // 反馈参数的赋值
            feedback->current_x = current_x_;
            feedback->current_y = current_y_;
            feedback->remaining_distance = remaining_distance;
            feedback->progress = progress;

            RCLCPP_INFO(this->get_logger(), "====================");
            RCLCPP_INFO(this->get_logger(), "连续反馈中...");
            RCLCPP_INFO(this->get_logger(), "当前x坐标: %.2lf", feedback->current_x);
            RCLCPP_INFO(this->get_logger(), "当前y坐标: %.2lf", feedback->current_y);
            RCLCPP_INFO(this->get_logger(), "剩余距离: %.2lf", feedback->remaining_distance);
            RCLCPP_INFO(this->get_logger(), "当前进度: %.2f", feedback->progress);
            RCLCPP_INFO(this->get_logger(), "====================");

            goal_handle->publish_feedback(feedback);
        }

        result->success = true;
        result->message = "任务已完成!";
        result->final_x = target_x;
        result->final_y = target_y;
        result->traveled_distance = total_dist;

        RCLCPP_INFO(this->get_logger(), "====================");
        RCLCPP_INFO(this->get_logger(), "最终结果: ");
        RCLCPP_INFO(this->get_logger(), "success = %s", result->success ? "true" : "false");
        RCLCPP_INFO(this->get_logger(), "message = %s", result->message.c_str());
        RCLCPP_INFO(this->get_logger(), "final_x = %.2lf", result->final_x);
        RCLCPP_INFO(this->get_logger(), "final_y = %.2lf", result->final_y);
        RCLCPP_INFO(this->get_logger(), "traveled_distance = %.2lf", result->traveled_distance);
        RCLCPP_INFO(this->get_logger(), "====================");

        goal_handle->succeed(result);
    }

    double current_x_;
    double current_y_;

    rclcpp_action::Server<MoveToPoint>::SharedPtr server_;

};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);

    auto server = std::make_shared<MoveToPointServer>();
    rclcpp::spin(server);

    rclcpp::shutdown();

    return 0;
}
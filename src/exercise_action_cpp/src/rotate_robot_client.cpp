#include "base_interfaces/action/rotate_robot.hpp"
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include <cmath>
#include <exception>
#include <string>

using namespace std::chrono_literals;
using base_interfaces::action::RotateRobot;
using std::placeholders::_1;
using std::placeholders::_2;

class RotateRobotClient : public rclcpp::Node {
    friend int main(int argc, char **argv);

public:
    RotateRobotClient()
        : Node("rotate_robot_client_cpp"), is_cancel_(false), cancel_pending_(false), goal_finished_(false),
          cancellation_enabled_(false), cancel_progress_(0.0f) {
        RCLCPP_INFO(this->get_logger(), "动作客户端创建成功!");
        client_ = rclcpp_action::create_client<RotateRobot>(this, "rotate_robot");
        cancel_timer_ = this->create_wall_timer(10ms, std::bind(&RotateRobotClient::process_cancel_request, this));
    }

    bool send_goal(double target_angle, double angular_speed) {
        if (!client_->wait_for_action_server(10s)) {
            RCLCPP_ERROR(this->get_logger(), "服务器连接失败!");
            return false;
        } else {
            RCLCPP_INFO(this->get_logger(), "服务器连接成功!");
        }

        auto goal = RotateRobot::Goal();
        auto options = rclcpp_action::Client<RotateRobot>::SendGoalOptions();

        goal.target_angle = target_angle;
        goal.angular_speed = angular_speed;

        options.goal_response_callback = std::bind(&RotateRobotClient::goal_response_callback, this, _1);
        options.feedback_callback = std::bind(&RotateRobotClient::feedback_callback, this, _1, _2);
        options.result_callback = std::bind(&RotateRobotClient::result_callback, this, _1);

        /*
            std::shared_future<rclcpp_action::ClientGoalHandle<base_interfaces::action::RotateRobot>::SharedPtr> async_send_goal(
                const base_interfaces::action::RotateRobot::Goal &goal,
                const rclcpp_action::Client<base_interfaces::action::RotateRobot>::SendGoalOptions &options
            )
        */
        client_->async_send_goal(goal, options);
        return true;
    }

private:
    /*
        using GoalHandle = ClientGoalHandle<ActionT>;
        using WrappedResult = typename GoalHandle::WrappedResult;
        using GoalResponseCallback = std::function<void (typename GoalHandle::SharedPtr)>;
    */
    void goal_response_callback(rclcpp_action::ClientGoalHandle<RotateRobot>::SharedPtr goal_handle) {
        if (goal_handle) {
            RCLCPP_INFO(this->get_logger(), "目标设置成功!");

            if (cancellation_enabled_ && cancel_progress_ <= 0.0f) {
                pending_goal_handle_ = goal_handle;
                cancel_pending_ = true;
                RCLCPP_INFO(this->get_logger(), "取消进度为0%%, 等待发送取消请求!");
            }
        } else {
            goal_finished_ = true;
            RCLCPP_ERROR(this->get_logger(), "目标设置失败!");
            stop_client();
        }
    }

    /*
        std::function<void (typename ClientGoalHandle<ActionT>::SharedPtr, const std::shared_ptr<const Feedback>)>;
    */
    void feedback_callback(
        rclcpp_action::ClientGoalHandle<RotateRobot>::SharedPtr goal_handle,
        const std::shared_ptr<const RotateRobot::Feedback> feedback
    ) {
        RCLCPP_INFO(
            this->get_logger(),
            "连续反馈中: [current_angle = %lf, remaining_angle = %lf, progress = %d%%]",
            feedback->current_angle,
            feedback->remaining_angle,
            static_cast<int>(std::round(feedback->progress * 100))
        );

        if (
            cancellation_enabled_ && !is_cancel_ && !cancel_pending_ && !goal_finished_ &&
            feedback->progress >= cancel_progress_ && feedback->progress < 1.0f
        ) {
            pending_goal_handle_ = goal_handle;
            cancel_pending_ = true;
            RCLCPP_INFO(this->get_logger(), "已达到取消进度, 等待发送取消请求!");
        }
    }

    void process_cancel_request() {
        if (goal_finished_) {
            pending_goal_handle_.reset();
            cancel_pending_ = false;
            return;
        }

        if (!cancel_pending_ || !pending_goal_handle_) {
            return;
        }

        auto goal_handle = pending_goal_handle_;
        pending_goal_handle_.reset();
        cancel_pending_ = false;
        is_cancel_ = true;

        try {
            /*
            std::shared_future<action_msgs::srv::CancelGoal_Response_<std::allocator<void>>::SharedPtr>
            async_cancel_goal(
                std::shared_ptr<rclcpp_action::ClientGoalHandle<base_interfaces::action::RotateRobot>> goal_handle,
                std::function<void (std::shared_ptr<action_msgs::srv::CancelGoal_Response>)> cancel_callback
            )
            */
            client_->async_cancel_goal(
                goal_handle,
                std::bind(&RotateRobotClient::cancel_response_callback, this, _1)
            );
            RCLCPP_INFO(this->get_logger(), "已发出任务取消请求!");
        } catch (const std::exception &exception) {
            is_cancel_ = false;
            RCLCPP_ERROR(this->get_logger(), "发送取消请求失败: %s", exception.what());
        }
    }

    void cancel_response_callback(rclcpp_action::Client<RotateRobot>::CancelResponse::SharedPtr response) {
        if (response->goals_canceling.empty()) {
            RCLCPP_WARN(this->get_logger(), "取消请求未被服务端接受!");
        } else {
            RCLCPP_INFO(this->get_logger(), "服务端已接受取消请求!");
        }
    }

    /*
        std::function<void (const WrappedResult & result)>;
    */
    void result_callback(const rclcpp_action::ClientGoalHandle<RotateRobot>::WrappedResult &result) {
        goal_finished_ = true;
        cancel_pending_ = false;
        pending_goal_handle_.reset();

        switch (result.code) {
            case rclcpp_action::ResultCode::SUCCEEDED:
                RCLCPP_INFO(this->get_logger(), "任务执行完成!");
                break;

            case rclcpp_action::ResultCode::ABORTED:
                RCLCPP_WARN(this->get_logger(), "任务被中断!");
                break;

            case rclcpp_action::ResultCode::CANCELED:
                RCLCPP_WARN(this->get_logger(), "任务被取消!");
                break;

            default:
                RCLCPP_WARN(this->get_logger(), "收到未知的任务结果!");
                stop_client();
                return;
        }

        if (result.result) {
            RCLCPP_INFO(
                this->get_logger(),
                "[success = %s, message = %s, final_angle = %lf, elapsed_time = %lf]",
                result.result->success ? "true" : "false",
                result.result->message.c_str(),
                result.result->final_angle,
                result.result->elapsed_time
            );
        }

        stop_client();
    }

    void stop_client() {
        if (rclcpp::ok()) {
            rclcpp::shutdown();
        }
    }

    bool is_cancel_;
    bool cancel_pending_;
    bool goal_finished_;
    bool cancellation_enabled_;
    float cancel_progress_;

    rclcpp_action::ClientGoalHandle<RotateRobot>::SharedPtr pending_goal_handle_;
    rclcpp_action::Client<RotateRobot>::SharedPtr client_;
    rclcpp::TimerBase::SharedPtr cancel_timer_;
};

bool parse_finite_number(const char *text, const char *parameter_name, double &value) {
    const std::string input(text);
    std::size_t parsed_characters = 0;

    try {
        value = std::stod(input, &parsed_characters);
    } catch (const std::exception &) {
        RCLCPP_ERROR(
            rclcpp::get_logger("rotate_robot_client_cpp"),
            "%s必须是有效数值: %s",
            parameter_name,
            text
        );
        return false;
    }

    if (parsed_characters != input.size() || !std::isfinite(value)) {
        RCLCPP_ERROR(
            rclcpp::get_logger("rotate_robot_client_cpp"),
            "%s必须是有效的有限数值: %s",
            parameter_name,
            text
        );
        return false;
    }

    return true;
}

int main(int argc, char **argv) {
    if (argc != 3 && argc != 4) {
        RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "输入参数不正确!");
        return 1;
    }

    rclcpp::init(argc, argv);

    double target_angle = 0.0;
    double angular_speed = 0.0;
    double cancel_percentage = 0.0;

    if (
        !parse_finite_number(argv[1], "目标角度", target_angle) ||
        !parse_finite_number(argv[2], "角速度", angular_speed)
    ) {
        rclcpp::shutdown();
        return 1;
    }

    if (argc == 4) {
        if (!parse_finite_number(argv[3], "取消进度", cancel_percentage)) {
            rclcpp::shutdown();
            return 1;
        }

        if (cancel_percentage < 0.0 || cancel_percentage >= 100.0) {
            RCLCPP_ERROR(rclcpp::get_logger("rotate_robot_client_cpp"), "取消进度必须在[0, 100)范围内!");
            rclcpp::shutdown();
            return 1;
        }
    }

    auto client = std::make_shared<RotateRobotClient>();

    if (argc == 4) {
        client->cancellation_enabled_ = true;
        client->cancel_progress_ = static_cast<float>(cancel_percentage * 0.01);
    }

    if (!client->send_goal(target_angle, angular_speed)) {
        rclcpp::shutdown();
        return 1;
    }

    rclcpp::spin(client);

    if (rclcpp::ok()) {
        rclcpp::shutdown();
    }

    return 0;
}

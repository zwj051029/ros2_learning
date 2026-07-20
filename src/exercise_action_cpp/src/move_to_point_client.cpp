#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "base_interfaces/action/move_to_point.hpp"

using namespace std::chrono_literals;
using base_interfaces::action::MoveToPoint;
using std::placeholders::_1;
using std::placeholders::_2;

class MoveToPointClient : public rclcpp::Node {
    friend int main(int argc, char **argv);

public:
    MoveToPointClient() : Node("move_to_point_client_cpp"), cancel_start_(false), cancel_enabled_(false), cancel_progress_(1.2) {
        RCLCPP_INFO(this->get_logger(), "动作客户端创建成功!");

        cancel_timer_ = this->create_wall_timer(10ms, std::bind(&MoveToPointClient::cancel_request_callback, this));
        client_ = rclcpp_action::create_client<MoveToPoint>(this, "move_to_point");
    }

    bool send_goal(double target_x, double target_y, double speed) {
        if (!client_->wait_for_action_server(10s)) {
            RCLCPP_ERROR(this->get_logger(), "服务器连接失败!");
            return false;
        } else {
            RCLCPP_INFO(this->get_logger(), "服务器连接成功!");
        }

        auto goal = MoveToPoint::Goal();
        auto options = rclcpp_action::Client<MoveToPoint>::SendGoalOptions();

        goal.target_x = target_x;
        goal.target_y = target_y;
        goal.speed = speed;

        RCLCPP_INFO(
            this->get_logger(), "设置的目标为: target_x = %.2lf, target_y = %.2lf, speed = %.2lf", goal.target_x, goal.target_y, goal.speed
        );

        options.goal_response_callback = std::bind(&MoveToPointClient::goal_response_callback, this, _1);
        options.feedback_callback = std::bind(&MoveToPointClient::feedback_callback, this, _1, _2);
        options.result_callback = std::bind(&MoveToPointClient::result_callback, this, _1);

        /*
            std::shared_future<rclcpp_action::ClientGoalHandle<base_interfaces::action::MoveToPoint>::SharedPtr>
            async_send_goal(
                const base_interfaces::action::MoveToPoint::Goal &goal, const rclcpp_action::Client<base_interfaces::action::MoveToPoint>::SendGoalOptions &options
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
    void goal_response_callback(
        rclcpp_action::ClientGoalHandle<MoveToPoint>::SharedPtr goal_handle
    ) {
        if (goal_handle) {
            RCLCPP_INFO(this->get_logger(), "目标设置成功!");

            cancel_goal_handle_ = goal_handle;
            if (cancel_enabled_ && cancel_progress_ <= 0.0) {
                cancel_start_ = true;

                RCLCPP_INFO(
                    this->get_logger(), "取消进度为0%%, 准备立即发送取消请求!"
                );
            }
        } else {
            RCLCPP_INFO(this->get_logger(), "目标设置失败!");
            stop_client();
        }
    }

    /*
        std::function<void (typename ClientGoalHandle<ActionT>::SharedPtr, const std::shared_ptr<const Feedback>)>;
    */
    void feedback_callback(
        rclcpp_action::ClientGoalHandle<MoveToPoint>::SharedPtr goal_handle,
        const std::shared_ptr<const MoveToPoint::Feedback> feedback
    ) {
        cancel_goal_handle_ = goal_handle;
        if (cancel_enabled_ && cancel_progress_ <= feedback->progress && cancel_progress_ > 0.0f && cancel_progress_ < 1.0f) {
            cancel_start_ = true;
        }

        double current_x = feedback->current_x;
        double current_y = feedback->current_y;
        double remaining_distance = feedback->remaining_distance;
        int progress = std::round(feedback->progress * 100.0);

        RCLCPP_INFO(this->get_logger(), "====================");
        RCLCPP_INFO(this->get_logger(), "连续接收反馈中...");
        RCLCPP_INFO(this->get_logger(), "当前x坐标: %.2lf", current_x);
        RCLCPP_INFO(this->get_logger(), "当前y坐标: %.2lf", current_y);
        RCLCPP_INFO(this->get_logger(), "剩余距离: %.2lf", remaining_distance);
        RCLCPP_INFO(this->get_logger(), "当前进度: %d%%", progress);
        RCLCPP_INFO(this->get_logger(), "====================");
    }

    void cancel_request_callback() {
        if (!cancel_start_) {
            return;
        }

        cancel_enabled_ = false;
        cancel_start_ = false;
        /*
            std::function<void (std::shared_ptr<action_msgs::srv::CancelGoal_Response>)> cancel_callback = nullptr)
        */
        try {
            client_->async_cancel_goal(cancel_goal_handle_, std::bind(&MoveToPointClient::cancel_callback, this, _1));
            RCLCPP_INFO(this->get_logger(), "已发送取消请求!");
        } catch(const std::exception &exception) {
            RCLCPP_WARN(this->get_logger(), "取消请求发送失败: %s", exception.what());
        }
    }

    void cancel_callback(
        std::shared_ptr<action_msgs::srv::CancelGoal_Response> cancel_response
    ) {
        if (cancel_response->goals_canceling.empty()) {
            RCLCPP_WARN(this->get_logger(), "取消请求被拒绝!");
        } else {
            RCLCPP_INFO(this->get_logger(), "取消请求被接受!");
        }
    }

    /*
        std::function<void (const WrappedResult & result)>;
    */
    void result_callback(
        const rclcpp_action::ClientGoalHandle<MoveToPoint>::WrappedResult &result
    ) {
        switch (result.code) {
            case rclcpp_action::ResultCode::SUCCEEDED:
                RCLCPP_INFO(this->get_logger(), "任务已完成!");
                break;
            case rclcpp_action::ResultCode::ABORTED:
                RCLCPP_INFO(this->get_logger(), "任务被中断!");
                break;
            case rclcpp_action::ResultCode::CANCELED:
                RCLCPP_INFO(this->get_logger(), "任务被取消!");
                break;
            default:
                RCLCPP_INFO(this->get_logger(), "未知错误!");
                stop_client();
                return;
        }

        if (result.result) {
            RCLCPP_INFO(
                this->get_logger(),
                "[success = %s, message = %s, final_x = %.2lf, final_y = %.2lf, traveled_distance = %.2lf]",
                result.result->success ? "true" : "false",
                result.result->message.c_str(),
                result.result->final_x,
                result.result->final_y,
                result.result->traveled_distance
            );
        }

        stop_client();
    }

    void stop_client() {
        if (rclcpp::ok()) {
            rclcpp::shutdown();
        }
    }

    bool cancel_start_;
    bool cancel_enabled_;
    float cancel_progress_;

    rclcpp_action::ClientGoalHandle<MoveToPoint>::SharedPtr cancel_goal_handle_;
    rclcpp::TimerBase::SharedPtr cancel_timer_;
    rclcpp_action::Client<MoveToPoint>::SharedPtr client_;
};

bool parse_finite_number(
    const char *text,
    const char *parameter_name,
    double &value
) {
    const std::string input(text);
    std::size_t parsed_characters = 0;

    try {
        value = std::stod(input, &parsed_characters);
    } catch (const std::exception &) {
        RCLCPP_ERROR(
            rclcpp::get_logger("rclcpp"),
            "%s必须为有效数值: %s",
            parameter_name, text
        );
        return false;
    }

    if (
        parsed_characters != input.size() ||
        !std::isfinite(value)
    ) {
        RCLCPP_ERROR(
            rclcpp::get_logger("rclcpp"),
            "%s必须为有效的有限数值: %s",
            parameter_name, text
        );
        return false;
    }

    return true;
}

int main(int argc, char **argv) {
    if (argc != 4 && argc != 5) {
        RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "输入参数数目不合法!");
        return 1;
    }

    double target_x = 0.0;
    double target_y = 0.0;
    double speed = 0.0;
    double cancel_progress = 120.0;

    if (
        !parse_finite_number(argv[1], "目标位置的x坐标", target_x) ||
        !parse_finite_number(argv[2], "目标位置的y坐标", target_y) ||
        !parse_finite_number(argv[3], "目标速度", speed)
    ) {
        rclcpp::shutdown();
        return 1;
    }

    if (argc == 5) {
        if (!parse_finite_number(argv[4], "取消进度", cancel_progress)) {
            rclcpp::shutdown();
            return 1;
        }

        if (cancel_progress < 0.0 || cancel_progress >= 100.0) {
            RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "取消进度必须在[0, 100)范围内!");
            rclcpp::shutdown();
            return 1;
        }
    }

    rclcpp::init(argc, argv);

    auto client = std::make_shared<MoveToPointClient>();

    if (argc == 5) {
        client->cancel_enabled_ = true;
        client->cancel_progress_ = static_cast<float>(cancel_progress * 0.01);
    }

    if (!client->send_goal(target_x, target_y, speed)) {
        rclcpp::shutdown();
        return 1;
    }

    rclcpp::spin(client);

    rclcpp::shutdown();

    return 0;
}

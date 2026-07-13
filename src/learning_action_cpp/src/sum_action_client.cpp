/**
 * 需求：编写动作通信客户端 向服务端发送一个整型数字作为目标 处理服务器发来反馈和最终结果
 * 步骤：
 *      1、包含头文件
 *      2、初始化 ROS2 客户端
 *      3、自定义节点类
 *          3-1、创建客户端
 *          3-2、编写发送目标的函数（包含等待连接服务器）
 *          3-3、处理目标发送后的反馈
 *          3-4、处理反馈
 *          3-5、处理最终结果
 *      4、发送目标 并调用 spin 函数 传入节点对象指针
 *      5、释放资源
 */

#include "base_interfaces/action/sum_to_n.hpp"
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"

using namespace std::chrono_literals;
using base_interfaces::action::SumToN;
using std::placeholders::_1;
using std::placeholders::_2;

class SumToNActionClient : public rclcpp::Node {
public:
    SumToNActionClient() : Node("sum_to_n_client_cpp") {
        RCLCPP_INFO(this->get_logger(), "客户端创建成功!");
        client_ = rclcpp_action::create_client<SumToN>(this, "sum_to_n");
    }

    void send_goal(int num) {
        if (!client_->wait_for_action_server(10s)) {
            RCLCPP_ERROR(this->get_logger(), "服务器连接超时!");
            return;
        }

        auto goal = SumToN::Goal();
        goal.num = num;
        RCLCPP_INFO(this->get_logger(), "设置的目标为: %d", goal.num);

        auto options = rclcpp_action::Client<SumToN>::SendGoalOptions();
        options.goal_response_callback = std::bind(&SumToNActionClient::goal_response_callback, this, _1);
        options.feedback_callback = std::bind(&SumToNActionClient::feedback_callback, this, _1, _2);
        options.result_callback = std::bind(&SumToNActionClient::result_callback, this, _1);

        /*
            std::shared_future<rclcpp_action::ClientGoalHandle<base_interfaces::action::SumToN>::SharedPtr>
            async_send_goal(const base_interfaces::action::SumToN::Goal &goal,
                            const rclcpp_action::Client<base_interfaces::action::SumToN>::SendGoalOptions &options)
        */
        auto future = client_->async_send_goal(goal, options);
    }

private:
    /*
        using GoalHandle = ClientGoalHandle<ActionT>;
        using WrappedResult = typename GoalHandle::WrappedResult;
        using GoalResponseCallback = std::function<void (typename GoalHandle::SharedPtr)>;
    */
    void goal_response_callback(rclcpp_action::ClientGoalHandle<SumToN>::SharedPtr goal_handle) {
        if (goal_handle) {
            RCLCPP_INFO(this->get_logger(), "目标设置成功!");
        } else {
            RCLCPP_ERROR(this->get_logger(), "目标设置失败!");
        }
    }

    /*
        std::function<void (
        typename ClientGoalHandle<ActionT>::SharedPtr,
        const std::shared_ptr<const Feedback>)>;
    */
    void feedback_callback(rclcpp_action::ClientGoalHandle<SumToN>::SharedPtr goal_handle, const std::shared_ptr<const SumToN::Feedback> feedback) {
        (void)goal_handle;
        double progress = feedback->progress;
        const int progress_percent = static_cast<int>(std::round(progress * 100.0f));

        RCLCPP_INFO(this->get_logger(), "当前进度: %d%%", progress_percent);
    }

    /*
        std::function<void (const WrappedResult & result)>;
    */
    void result_callback(const rclcpp_action::ClientGoalHandle<SumToN>::WrappedResult &result) {
        switch (result.code) {
            case rclcpp_action::ResultCode::SUCCEEDED:
                break;
            case rclcpp_action::ResultCode::ABORTED:
                RCLCPP_ERROR(this->get_logger(), "任务被中止!");
                return;
            case rclcpp_action::ResultCode::CANCELED:
                RCLCPP_ERROR(this->get_logger(), "任务被取消!");
                return;
            default:
                RCLCPP_ERROR(this->get_logger(), "未知错误!");
                return;
        }

        RCLCPP_INFO(this->get_logger(), "任务处理完成, 最终结果: %ld", result.result->sum);
    }

    rclcpp_action::Client<SumToN>::SharedPtr client_;
};

int main(int argc, char **argv) {
    if (argc != 2) {
        RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "提交参数数目不合法!");
        return 1;
    }

    rclcpp::init(argc, argv);

    auto client = std::make_shared<SumToNActionClient>();
    client->send_goal(atoi(argv[1]));
    rclcpp::spin(client);

    rclcpp::shutdown();

    return 0;
}
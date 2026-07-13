/**
 * 需求：编写动作通信服务端 提取客户端发来的目标数字 然后从1开始遍历 将数字相加作为最终响应结果 并处理连续进度反馈
 * 步骤：
 *      1、包含头文件
 *      2、初始化 ROS2 客户端
 *      3、自定义节点类
 *          3-1、创建动作服务端
 *          3-2、提取客户端发来的目标 并作出回应
 *          3-3、处理客户端的取消请求 并作出回应
 *          3-4、生成连续反馈 并响应最终结果
 *      4、调用 spin 函数 并传入节点对象指针
 *      5、释放资源
 */

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "base_interfaces/action/sum_to_n.hpp"

using base_interfaces::action::SumToN;
using std::placeholders::_1;
using std::placeholders::_2;

class SumToNActionServer : public rclcpp::Node {
public:
    SumToNActionServer() : Node("sum_to_n_action_server_cpp") {
        RCLCPP_INFO(this->get_logger(), "动作服务端创建成功!");
        /*
            rclcpp_action::Server<ActionT>::SharedPtr
            create_server<ActionT, NodeT>(
                NodeT node, const std::string &name,
                rclcpp_action::Server<ActionT>::GoalCallback handle_goal,
                rclcpp_action::Server<ActionT>::CancelCallback handle_cancel, rclcpp_action::Server<ActionT>::AcceptedCallback handle_accepted
            )
        */
        server_ = rclcpp_action::create_server<SumToN>(
            this,
            "sum_to_n",
            std::bind(&SumToNActionServer::goal_callback, this, _1, _2),
            std::bind(&SumToNActionServer::cancel_callback, this, _1),
            std::bind(&SumToNActionServer::accepted_callback, this, _1)
        );
    }

private:
    /*
        std::function<GoalResponse(const GoalUUID &, std::shared_ptr<const typename ActionT::Goal>)>;
    */
    rclcpp_action::GoalResponse goal_callback(const rclcpp_action::GoalUUID &goal_uuid,
                                              std::shared_ptr<const SumToN::Goal> goal) {
        (void)goal_uuid;
        if (goal->num <= 1) {
            RCLCPP_WARN(this->get_logger(), "提交的数字必须大于1!");
            return rclcpp_action::GoalResponse::REJECT;
        }

        RCLCPP_INFO(this->get_logger(), "提交的数字合法!");
        return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
    }

    /*
        std::function<CancelResponse(std::shared_ptr<ServerGoalHandle<ActionT>>)>;
    */
    rclcpp_action::CancelResponse cancel_callback(std::shared_ptr<rclcpp_action::ServerGoalHandle<SumToN>> server_goal_handle) {
        (void)server_goal_handle;
        RCLCPP_INFO(this->get_logger(), "收到取消请求, 允许取消!");
        return rclcpp_action::CancelResponse::ACCEPT;
    }

    /*
        std::function<void (std::shared_ptr<ServerGoalHandle<ActionT>>)>;
    */
    void accepted_callback(std::shared_ptr<rclcpp_action::ServerGoalHandle<SumToN>> server_goal_handle) {
        std::thread(std::bind(&SumToNActionServer::execute, this, server_goal_handle)).detach();
    }

    void execute(std::shared_ptr<rclcpp_action::ServerGoalHandle<SumToN>> server_goal_handle) {
        auto feedback = std::make_shared<SumToN::Feedback>();
        auto result = std::make_shared<SumToN::Result>();

        int num = server_goal_handle->get_goal()->num;
        int sum = 0;
        double progress = 0.0;

        /*
            void publish_feedback(std::shared_ptr<base_interfaces::action::SumToN_Feedback> feedback_msg)
        */
        rclcpp::Rate rate(1.0);
        for (int i = 1; i <= num; i++) {
            if (server_goal_handle->is_canceling()) {
                result->sum = sum;
                server_goal_handle->canceled(result);
                RCLCPP_INFO(this->get_logger(), "任务取消成功!");

                return;
            }

            sum += i;
            progress = i / (double)num;
            feedback->progress = progress;

            RCLCPP_INFO(this->get_logger(), "当前进度: %.2f", feedback->progress);
            server_goal_handle->publish_feedback(feedback);

            rate.sleep();
        }

        /*
            void succeed(std::shared_ptr<base_interfaces::action::SumToN_Result> result_msg)
        */
        if (rclcpp::ok()) {
            result->sum = sum;
            RCLCPP_INFO(this->get_logger(), "任务处理完毕, 最终结果为: %ld", result->sum);
            server_goal_handle->succeed(result);
        }
    }

    rclcpp_action::Server<SumToN>::SharedPtr server_;

};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<SumToNActionServer>());
    rclcpp::shutdown();

    return 0;
}
/**
 * 需求：编写客户端 组织并发布两个整型数据作为请求 并正确接收解析响应
 * 步骤：
 *      1、包含头文件
 *      2、初始化 ROS2 客户端
 *      3、自定义节点类
 *          3-1、创建客户端
 *          3-2、等待连接服务器
 *          3-3、组织并发布请求
 *      4、创建节点对象指针 并处理响应
 *      5、释放资源
 */

#include "base_interfaces/srv/add_ints.hpp"
#include "rclcpp/rclcpp.hpp"

using namespace std::chrono_literals;
using base_interfaces::srv::AddInts;

class SumClient : public rclcpp::Node {
public:
    SumClient() : Node("sum_client_cpp") {
        RCLCPP_INFO(this->get_logger(), "客户端创建成功, 等待连接服务器...");
        client_ = this->create_client<AddInts>("sum");
    }

    // 等待连接服务器
    bool connect_server() {
        while (!client_->wait_for_service(1s)) {
            // 处理 ctrl + c 关闭异常
            if (!rclcpp::ok()) {
                RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "强制退出!");
                return false;
            }
            RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "服务器连接中...");
        }
        return true;
    }

    // 组织并发送请求
    rclcpp::Client<AddInts>::FutureAndRequestId send_request(int32_t num1, int32_t num2) {
        auto request = std::make_shared<AddInts::Request>();
        request->num1 = num1;
        request->num2 = num2;

        /*
            rclcpp::Client<base_interfaces::srv::AddInts>::FutureAndRequestId 
            async_send_request(std::shared_ptr<base_interfaces::srv::AddInts_Request> request)
        */
        return client_->async_send_request(request);
    }

private:
    rclcpp::Client<AddInts>::SharedPtr client_;
};

int main(int argc, char **argv) {
    // 处理请求数量异常情况
    if (argc != 3) {
        RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "请提交两个整型数据!");
        return 1;
    }

    rclcpp::init(argc, argv);

    auto client_node = std::make_shared<SumClient>();
    if (client_node->connect_server()) {
        RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "服务器连接成功!");
    } else {
        RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "服务器连接失败!");
        return 2;
    }

    // 发送请求 并处理响应结果
    auto future = client_node->send_request(atoi(argv[1]), atoi(argv[2]));
    if (rclcpp::spin_until_future_complete(client_node, future) == rclcpp::FutureReturnCode::SUCCESS) {
        // 请求处理正常
        RCLCPP_INFO(client_node->get_logger(), "请求处理正常!");
        RCLCPP_INFO(client_node->get_logger(), "响应结果: sum = %d", future.get()->sum);
    } else {
        // 请求处理异常
        RCLCPP_WARN(client_node->get_logger(), "请求处理异常!");
    }

    rclcpp::shutdown();

    return 0;
}
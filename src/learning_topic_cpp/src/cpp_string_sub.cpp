/**
 * 需求：订阅发布方发布的消息 并输出到终端
 * 步骤：
 *      1、包含头文件；
 *      2、初始化 ROS2 客户端；
 *      3、定义节点类；
 *          3-1、创建订阅方；
 *          3-2、处理订阅到的消息
 *      4、调用 spin 函数 并传入节点对象指针；
 *      5、释放资源
 */

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

class StrSubscriber : public rclcpp::Node {
public:
    StrSubscriber() : Node("str_subscriber_cpp") {
        RCLCPP_INFO(this->get_logger(), "订阅方创建成功!");
        subscriber_ = this->create_subscription<std_msgs::msg::String>("string", 10, std::bind(&StrSubscriber::StrSubCallback, this, std::placeholders::_1));
    }

private:
    void StrSubCallback(const std_msgs::msg::String &msg) {
        RCLCPP_INFO(this->get_logger(), "订阅方订阅到的消息: %s", msg.data.c_str());
    }

    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscriber_;
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<StrSubscriber>());
    rclcpp::shutdown();

    return 0;
}
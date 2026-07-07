/**
 * 需求：以某个固定频率发送文本"hello world" 文本后缀编号 每发送一条消息 编号加一
 * 步骤：
 *      1、包含头文件
 *      2、初始化 ROS2 客户端
 *      3、定义节点类
 *          3-1、创建发布方
 *          3-2、创建定时器
 *          3-3、组织并发布消息
 *      4、调用 spin 函数 并传入节点对象指针
 *      5、释放资源
 */

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

using namespace std::chrono_literals;

class StrPublisher : public rclcpp::Node {
public:
    StrPublisher() : Node("str_publisher_cpp"), count_(1) {
        RCLCPP_INFO(this->get_logger(), "发布方创建成功!");
        publisher_ = this->create_publisher<std_msgs::msg::String>("string", 10);
        timer_ = this->create_wall_timer(1s, std::bind(&StrPublisher::StrCallback, this));
    }

private:
    void StrCallback() {
        auto message = std_msgs::msg::String();
        message.data = "hello world" + std::to_string(count_++);
        publisher_->publish(message);
        RCLCPP_INFO(this->get_logger(), "发布方发送的消息为: %s", message.data.c_str());
    }

    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;
    size_t count_;
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<StrPublisher>());
    rclcpp::shutdown();

    return 0;
}
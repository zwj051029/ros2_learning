#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/float32.hpp"
#include "std_msgs/msg/string.hpp"

class BatterySub : public rclcpp::Node {
public:
    BatterySub() : Node("battery_subscriber_cpp") {
        RCLCPP_INFO(this->get_logger(), "订阅方创建成功!");
        subscriber_ = this->create_subscription<std_msgs::msg::Float32>("battery_level", 10, std::bind(&BatterySub::battery_callback, this, std::placeholders::_1));
        publisher_ = this->create_publisher<std_msgs::msg::String>("battery_status", 10);
    }

private:
    void battery_callback(const std_msgs::msg::Float32::SharedPtr msg) {
        auto battery_status = std_msgs::msg::String();

        if (msg->data >= 60.0f) {
            battery_status.data = "NORMAL";
            RCLCPP_INFO(this->get_logger(), "电量正常: %0.1f%%", msg->data);
        } else if (msg->data >= 30.0f) {
            battery_status.data = "LOW";
            RCLCPP_WARN(this->get_logger(), "电量偏低: %0.1f%%", msg->data);
        } else {
            battery_status.data = "CRITICAL";
            RCLCPP_ERROR(this->get_logger(), "电量严重不足: %0.1f%%", msg->data);
        }

        publisher_->publish(battery_status);
    }

    rclcpp::Subscription<std_msgs::msg::Float32>::SharedPtr subscriber_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<BatterySub>());
    rclcpp::shutdown();

    return 0;
}
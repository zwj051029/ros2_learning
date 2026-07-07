#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/float32.hpp"

using namespace std::chrono_literals;

class BatteryPub : public rclcpp::Node {
public:
    BatteryPub() : Node("battery_publisher_cpp"), battery_level_(100.0f) {
        RCLCPP_INFO(this->get_logger(), "发布方创建成功!");

        publisher_ = this->create_publisher<std_msgs::msg::Float32>("battery_level", 10);
        timer_ = this->create_wall_timer(0.5s, std::bind(&BatteryPub::timer_callback, this));
    }

private:
    void timer_callback() {
        auto battery_msg = std_msgs::msg::Float32();
        battery_msg.data = battery_level_;
        publisher_->publish(battery_msg);

        RCLCPP_INFO(this->get_logger(), "发布方发布的消息: %0.1f", battery_msg.data);

        battery_level_ -= 1.5f;
        if (battery_level_ < 0.0f) {
            battery_level_ = 100.0f;
        }
    }

    rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;
    float battery_level_;
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<BatteryPub>());
    rclcpp::shutdown();

    return 0;
}
#include "rclcpp/rclcpp.hpp"

class TimeDemo : public rclcpp::Node {
public:
    TimeDemo() : Node("time_demo_cpp") {
        rclcpp::Time t1(2, 500000000L);
        rclcpp::Time t2 = this->now();

        RCLCPP_INFO(this->get_logger(), "s = %.2f, ns = %ld", t1.seconds(), t1.nanoseconds());
        RCLCPP_INFO(this->get_logger(), "s = %.2f, ns = %ld", t2.seconds(), t2.nanoseconds());
    }
private:
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<TimeDemo>();
    rclcpp::shutdown();

    return 0;
}
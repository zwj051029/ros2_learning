#include "rclcpp/rclcpp.hpp"

class DurationDemo : public rclcpp::Node {
public:
    DurationDemo() : Node("duration_demo_cpp") {
        rclcpp::Duration du1(2, 500000000);
        RCLCPP_INFO(
            this->get_logger(),
            "s = %.2f, ns = %ld",
            du1.seconds(), du1.nanoseconds()
        );
    }
private:
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<DurationDemo>();
    rclcpp::shutdown();

    return 0;
}
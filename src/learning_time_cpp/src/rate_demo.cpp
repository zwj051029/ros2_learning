#include "rclcpp/rclcpp.hpp"

using namespace std::chrono_literals;

class RateDemo : public rclcpp::Node {
public:
    RateDemo() : Node("rate_demo_cpp") {
        rate_demo();
    }
private:
    void rate_demo() {
        rclcpp::Rate rate1(1.0);
        rclcpp::Rate rate2(500ms);

        while (rclcpp::ok()) {
            RCLCPP_INFO(this->get_logger(), "==========");
            // rate1.sleep();
            rate2.sleep();
        }
    }
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<RateDemo>();
    rclcpp::shutdown();

    return 0;
}

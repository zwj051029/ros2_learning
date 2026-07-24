#include "rclcpp/rclcpp.hpp"

class MyNode : public rclcpp::Node {
public:
    MyNode() : Node("my_node_cpp", "my_namespace_cpp") {
        RCLCPP_INFO(this->get_logger(), "节点创建成功!");
    }
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<MyNode>());
    rclcpp::shutdown();

    return 0;
}
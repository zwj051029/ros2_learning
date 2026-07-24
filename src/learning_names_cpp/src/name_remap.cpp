#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

using std_msgs::msg::String;

class MyNode : public rclcpp::Node {
public:
    MyNode() : Node("my_node_cpp", "my_namespace_cpp") {
        RCLCPP_INFO(this->get_logger(), "节点创建成功!");
        // 全局话题
        // publisher_ = this->create_publisher<String>("/quanjv", 10);

        // 相对话题
        // publisher_ = this->create_publisher<String>("xiangdui", 10);

        // 私有话题
        publisher_ = this->create_publisher<String>("~/siyou", 10);
    }
private:
    rclcpp::Publisher<String>::SharedPtr publisher_;
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<MyNode>());
    rclcpp::shutdown();

    return 0;
}
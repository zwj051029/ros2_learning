/**
 * 需求：订阅发布方发来的学生信息 并输出到终端
 * 步骤：
 *      1、包含头文件
 *      2、初始化 ROS2 客户端
 *      3、自定义节点类
 *          3-1、创建订阅方
 *          3-2、解析并输出学生信息
 *      4、调用 spin 函数 并传入节点对象指针
 *      5、释放资源
 */

#include "rclcpp/rclcpp.hpp"
#include "base_interfaces/msg/student.hpp"

class StuSubscriber : public rclcpp::Node {
public:
    StuSubscriber() : Node("student_subscriber_cpp") {
        RCLCPP_INFO(this->get_logger(), "订阅方创建成功!");

        subscriber_ = this->create_subscription<base_interfaces::msg::Student>("student", 10, std::bind(&StuSubscriber::stu_msg_callback, this, std::placeholders::_1));
    }

private:
    void stu_msg_callback(const base_interfaces::msg::Student::SharedPtr stu_msg) {
        RCLCPP_INFO(this->get_logger(), "订阅到的学生信息为: [name = %s, age = %d, height = %0.2f]", stu_msg->name.c_str(), stu_msg->age, stu_msg->height);
    }

    rclcpp::Subscription<base_interfaces::msg::Student>::SharedPtr subscriber_;
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<StuSubscriber>());
    rclcpp::shutdown();

    return 0;
}
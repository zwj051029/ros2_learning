/**
 * 需求：以某个固定频率发布学生信息
 * 步骤：
 *      1、包含头文件
 *      2、初始化 ROS2 客户端
 *      3、自定义节点类
 *          3-1、创建发布方
 *          3-2、创建定时器
 *          3-3、组织并发布学生信息
 *      4、调用 spin 函数 并传入节点对象指针
 *      5、释放资源
 */

#include "rclcpp/rclcpp.hpp"
#include "base_interfaces/msg/student.hpp"

using namespace std::chrono_literals;

class StuPublisher : public rclcpp::Node {
public:
    StuPublisher() : Node("student_publisher_cpp") {
        RCLCPP_INFO(this->get_logger(), "发布方创建成功!");

        publisher_ = this->create_publisher<base_interfaces::msg::Student>("student", 10);
        timer_ = this->create_wall_timer(500ms, std::bind(&StuPublisher::timer_callback, this));
    }

private:
    void timer_callback() {
        auto stu_msg = base_interfaces::msg::Student();
        stu_msg.name = "Tom";
        stu_msg.age = 18;
        stu_msg.height = 1.83;

        publisher_->publish(stu_msg);
        RCLCPP_INFO(this->get_logger(), "发布的学生信息为: [name = %s, age = %d, height = %0.2f]", stu_msg.name.c_str(), stu_msg.age, stu_msg.height);
    }

    rclcpp::Publisher<base_interfaces::msg::Student>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;

};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<StuPublisher>());
    rclcpp::shutdown();

    return 0;
}

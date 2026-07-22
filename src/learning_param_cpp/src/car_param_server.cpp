/**
 * 需求：编写参数服务客户端 实现操作参数
 * 步骤：
 *      1、包含头文件
 *      2、初始化 ROS2 客户端
 *      3、自定义节点类
 *          3-1、实现 增 操作
 *          3-2、实现 删 操作
 *          3-3、实现 改 操作
 *          3-4、实现 查 操作
 *      4、调用 spin 函数 并传入节点对象指针
 *      5、释放资源
 */

#include "rclcpp/rclcpp.hpp"

class CarParamServer : public rclcpp::Node {
public:
    CarParamServer() : Node("car_param_server_cpp", rclcpp::NodeOptions().allow_undeclared_parameters(true)) {
        RCLCPP_INFO(this->get_logger(), "参数服务端创建成功!");
    }

    void declared_param() {
        RCLCPP_INFO(this->get_logger(), "----------增----------");
        this->declare_parameter("car_name", "tiger");
        this->declare_parameter("car_widths", 1.55);
        this->declare_parameter("car_wheels", 5);
        this->set_parameter(rclcpp::Parameter("car_heights", 2.00));
    }

    void delete_param() {
        RCLCPP_INFO(this->get_logger(), "----------删----------");
        RCLCPP_INFO(
            this->get_logger(), "删除之前是否存在 car_heights ? %s",
            this->has_parameter("car_heights") ? "yes" : "no"
        );
        this->undeclare_parameter("car_heights");
        RCLCPP_INFO(
            this->get_logger(), "删除之后是否存在 car_heights ? %s",
            this->has_parameter("car_heights") ? "yes" : "no"
        );
    }

    void set_param() {
        RCLCPP_INFO(this->get_logger(), "----------改----------");
        RCLCPP_INFO(
            this->get_logger(), "改动之前 car_widths = %.2lf",
            this->get_parameter("car_widths").as_double()
        );
        this->set_parameter(rclcpp::Parameter("car_widths", 1.75));
        RCLCPP_INFO(
            this->get_logger(), "改动之前 car_widths = %.2lf",
            this->get_parameter("car_widths").as_double()
        );
    }

    void get_param() {
        RCLCPP_INFO(this->get_logger(), "----------查----------");
        // 获取指定参数
        RCLCPP_INFO(this->get_logger(), "获取指定参数");
        
        auto p1 = this->get_parameter("car_name");
        auto p2 = this->get_parameter("car_widths");
        auto p3 = this->get_parameter("car_wheels");

        RCLCPP_INFO(
            this->get_logger(), "key = %s, val = %s",
            p1.get_name().c_str(), p1.as_string().c_str()
        );
        RCLCPP_INFO(
            this->get_logger(), "key = %s, val = %.2lf",
            p2.get_name().c_str(), p2.as_double()
        );
        RCLCPP_INFO(
            this->get_logger(), "key = %s, val = %ld",
            p3.get_name().c_str(), p3.as_int()
        );
        // 获取一些参数
        RCLCPP_INFO(this->get_logger(), "获取指定参数");

        std::vector<std::string> names = {"car_name", "car_widths", "car_wheels"};
        auto params = this->get_parameters(names);

        for (auto param: params) {
            RCLCPP_INFO(
                this->get_logger(), "%s = %s",
                param.get_name().c_str(), param.value_to_string().c_str()
            );
        }

        // 是否存在参数
        RCLCPP_INFO(this->get_logger(), "是否存在参数");
        RCLCPP_INFO(
            this->get_logger(), "是否存在 car_name ? %s",
            this->has_parameter("car_name") ? "yes" : "no"
        );
        RCLCPP_INFO(
            this->get_logger(), "是否存在 car_heights ? %s",
            this->has_parameter("car_heights") ? "yes" : "no"
        );
    }
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);

    auto server = std::make_shared<CarParamServer>();

    server->declared_param();
    server->delete_param();
    server->set_param();
    server->get_param();

    rclcpp::spin(server);

    rclcpp::shutdown();

    return 0;
}
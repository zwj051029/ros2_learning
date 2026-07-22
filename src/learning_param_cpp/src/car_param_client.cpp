/**
 * 需求：编写参数客户端 实现查询和修改参数的操作
 * 步骤：
 *      1、包含头文件；
 *      2、初始化 ROS2 客户端
 *      3、自定义节点类
 *          3-1、创建客户端
 *          3-2、实现 查询 操作
 *          3-3、实现 修改 操作
 *      4、释放资源
 */

#include "rclcpp/rclcpp.hpp"

using namespace std::chrono_literals;

class CarParamClient : public rclcpp::Node {
public:
    CarParamClient() : Node("car_param_client_cpp") {
        RCLCPP_INFO(this->get_logger(), "参数客户端创建成功!");
        client_ = std::make_shared<rclcpp::SyncParametersClient>(this, "car_param_server_cpp");
    }

    bool connect_server() {
        while (!client_->wait_for_service(1s)) {
            if (!rclcpp::ok()) {
                RCLCPP_ERROR(this->get_logger(), "连接超时, 强制退出!");
                return false;
            }
            RCLCPP_INFO(this->get_logger(), "服务器连接中...");
        }
        return true;
    }

    void get_param() {
        RCLCPP_INFO(this->get_logger(), "----------查询操作----------");
        // 查询指定参数
        std::string car_name = client_->get_parameter<std::string>("car_name");
        double car_widths = client_->get_parameter<double>("car_widths");
        int car_wheels = client_->get_parameter<int>("car_wheels");

        RCLCPP_INFO(this->get_logger(), "查询指定参数");
        RCLCPP_INFO(this->get_logger(), "car_name = %s", car_name.c_str());
        RCLCPP_INFO(this->get_logger(), "car_widths = %.2lf", car_widths);
        RCLCPP_INFO(this->get_logger(), "car_wheels = %d", car_wheels);

        // 查询一些参数
        std::vector<std::string> names = {"car_name", "car_widths", "car_wheels"};
        RCLCPP_INFO(this->get_logger(), "查询一些参数");
        auto params = client_->get_parameters(names);
        for (auto param: params) {
            RCLCPP_INFO(
                this->get_logger(), "%s = %s",
                param.get_name().c_str(), param.value_to_string().c_str()
            );
        }

        // 是否包含参数
        RCLCPP_INFO(this->get_logger(), "是否包含某些参数");
        RCLCPP_INFO(
            this->get_logger(), "是否包含 car_name ? %s",
            client_->has_parameter("car_name") ? "yes" : "no"
        );
        RCLCPP_INFO(
            this->get_logger(), "是否包含 car_heights ? %s",
            client_->has_parameter("car_heights") ? "yes" : "no"
        );
    }

    void set_param() {
        RCLCPP_INFO(this->get_logger(), "----------修改操作----------");
        client_->set_parameters(
            {
                rclcpp::Parameter("car_name", "pig"),
                rclcpp::Parameter("car_widths", 6.66),
                rclcpp::Parameter("car_wheels", 6),
                rclcpp::Parameter("car_heights", 2.0)
            }
        );
    }

private:
    rclcpp::SyncParametersClient::SharedPtr client_;
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);

    auto client = std::make_shared<CarParamClient>();

    if (client->connect_server()) {
        RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "服务器连接成功!");
    } else {
        RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "服务器连接失败!");
        rclcpp::shutdown();
        return 1;
    }

    client->get_param();
    client->set_param();
    client->get_param();

    rclcpp::shutdown();

    return 0;
}
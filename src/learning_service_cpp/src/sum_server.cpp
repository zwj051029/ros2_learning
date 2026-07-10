/**
 * 需求：编写服务端 解析客户端请求的两个数据 将其相加 响应给客户端
 * 流程：
 *      1、包含头文件
 *      2、初始化 ROS2 客户端
 *      3、自定义节点类
 *          3-1、创建服务端
 *          3-2、解析并客户端的请求 并输出响应结果
 *      4、调用 spin 函数 并传入节点对象指针
 *      5、释放资源
 */

#include "base_interfaces/srv/add_ints.hpp"
#include "rclcpp/rclcpp.hpp"

using base_interfaces::srv::AddInts;

using std::placeholders::_1;
using std::placeholders::_2;

class SumServer : public rclcpp::Node {
public:
    SumServer() : Node("sum_server_cpp") {
        RCLCPP_INFO(this->get_logger(), "服务端创建成功，等待客户端请求...");
        server_ = this->create_service<AddInts>("sum", std::bind(&SumServer::sum_callback, this, _1, _2));
    }

private:
    void sum_callback(const AddInts::Request::SharedPtr req, const AddInts::Response::SharedPtr res) {
        res->sum = req->num1 + req->num2;
        RCLCPP_INFO(
            this->get_logger(),
            "客户端请求的数据: [num1 = %d, num2 = %d], 响应结果为: sum = %d",
            req->num1, req->num2, res->sum
        );
    }

    rclcpp::Service<AddInts>::SharedPtr server_;
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<SumServer>());
    rclcpp::shutdown();

    return 0;
}

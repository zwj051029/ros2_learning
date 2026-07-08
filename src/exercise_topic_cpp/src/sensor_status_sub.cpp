#include "base_interfaces/msg/sensor_status.hpp"
#include "rclcpp/rclcpp.hpp"

typedef enum { OFFLINE, OVERHEAT, LOW_VOLTAGE, NORMAL } Sensor_Status;

class SensorStatusSub : public rclcpp::Node {
public:
    SensorStatusSub() : Node("sensor_status_subscriber_cpp") {
        RCLCPP_INFO(this->get_logger(), "订阅方创建成功!");

        total_msg_cnt = 0;
        normal_cnt = 0;
        offline_cnt = 0;
        overheat_cnt = 0;
        low_voltage_cnt = 0;

        subscriber_ = this->create_subscription<base_interfaces::msg::SensorStatus>("sensor_status", 10, std::bind(&SensorStatusSub::sensor_status_callback, this, std::placeholders::_1));
    }

private:
    void sensor_status_callback(const base_interfaces::msg::SensorStatus::SharedPtr status_msg) {
        total_msg_cnt++;

        if (status_msg->is_online == false) {
            status_ = OFFLINE;
            offline_cnt++;
        } else if (status_msg->temperature >= 60.0f) {
            status_ = OVERHEAT;
            overheat_cnt++;
        } else if (status_msg->voltage < 3.0f) {
            status_ = LOW_VOLTAGE;
            low_voltage_cnt++;
        } else {
            status_ = NORMAL;
            normal_cnt++;
        }

        switch (status_) {
            case OFFLINE:
                RCLCPP_ERROR(this->get_logger(), "%s 离线", status_msg->sensor_name.c_str());
                break;
            case OVERHEAT:
                RCLCPP_WARN(this->get_logger(), "%s 过热: temp=%0.1f", status_msg->sensor_name.c_str(), status_msg->temperature);
                break;
            case LOW_VOLTAGE:
                RCLCPP_WARN(this->get_logger(), "%s 电压过低: voltage=%0.1f", status_msg->sensor_name.c_str(), status_msg->voltage);
                break;
            case NORMAL:
                RCLCPP_INFO(this->get_logger(), "%s 状态正常: temp=%0.1f, voltage=%0.1f", status_msg->sensor_name.c_str(), status_msg->temperature, status_msg->voltage);
                break;
        }

        if (total_msg_cnt == 5) {
            RCLCPP_INFO(this->get_logger(), "===== 传感器健康统计 =====");
            RCLCPP_INFO(this->get_logger(), "总消息数: %d", total_msg_cnt);
            RCLCPP_INFO(this->get_logger(), "正常: %d", normal_cnt);
            RCLCPP_INFO(this->get_logger(), "离线: %d", offline_cnt);
            RCLCPP_INFO(this->get_logger(), "过热: %d", overheat_cnt);
            RCLCPP_INFO(this->get_logger(), "低电压: %d", low_voltage_cnt);
            RCLCPP_INFO(this->get_logger(), "========================");

            total_msg_cnt = 0;
            normal_cnt = 0;
            offline_cnt = 0;
            overheat_cnt = 0;
            low_voltage_cnt = 0;
        }
    }

    int total_msg_cnt;
    int normal_cnt;
    int offline_cnt;
    int overheat_cnt;
    int low_voltage_cnt;

    Sensor_Status status_;
    rclcpp::Subscription<base_interfaces::msg::SensorStatus>::SharedPtr subscriber_;
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<SensorStatusSub>());
    rclcpp::shutdown();

    return 0;
}
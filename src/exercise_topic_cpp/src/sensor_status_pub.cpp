#include "base_interfaces/msg/sensor_status.hpp"
#include "rclcpp/rclcpp.hpp"
#include <string>
#include <vector>

using namespace std::chrono_literals;

struct SensorData {
    std::string sensor_name;
    float temperature;
    float voltage;
    bool is_online;
};

class SensorStatusPub : public rclcpp::Node {
public:
    SensorStatusPub() : Node("sensor_status_publisher_cpp"), current_index_(0) {
        RCLCPP_INFO(this->get_logger(), "发布方创建成功!");

        sensor_data_ = {
            {"lidar", 45.0f, 12.0f, true},
            {"camera", 62.5f, 5.0f, true},
            {"imu", 38.0f, 3.3f, true},
            {"ultrasonic", 35.0f, 2.7f, true},
            {"gps", 40.0f, 0.0f, false},
        };

        publisher_ = this->create_publisher<base_interfaces::msg::SensorStatus>("sensor_status", 10);
        timer_ = this->create_wall_timer(1s, std::bind(&SensorStatusPub::timer_callback, this));
    }

private:
    void timer_callback() {
        const auto &sensor_data = sensor_data_[current_index_++];
        if (current_index_ >= sensor_data_.size()) {
            current_index_ = 0;
        }

        auto sensor_msg = base_interfaces::msg::SensorStatus();
        sensor_msg.sensor_name = sensor_data.sensor_name;
        sensor_msg.temperature = sensor_data.temperature;
        sensor_msg.voltage = sensor_data.voltage;
        sensor_msg.is_online = sensor_data.is_online;

        publisher_->publish(sensor_msg);
        RCLCPP_INFO(
            this->get_logger(),
            "发布传感器状态: [name=%s, temp=%0.1f, voltage=%0.1f, online=%s]",
            sensor_msg.sensor_name.c_str(),
            sensor_msg.temperature,
            sensor_msg.voltage,
            sensor_msg.is_online ? "true" : "false"
        );
    }

    std::vector<SensorData> sensor_data_;
    size_t current_index_;

    rclcpp::Publisher<base_interfaces::msg::SensorStatus>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<SensorStatusPub>());
    rclcpp::shutdown();

    return 0;
}
import threading

import rclpy
from rclpy.exceptions import ROSInterruptException
from rclpy.node import Node


class RateDemo(Node):
    def __init__(self):
        super().__init__("rate_demo_py")
        self.stop_event_ = threading.Event()
        self.rate_ = self.create_rate(1.0)
        self.thread_ = threading.Thread(target=self.rate_demo)
        self.thread_.start()

    def rate_demo(self):
        try:
            while rclpy.ok() and not self.stop_event_.is_set():
                self.get_logger().info("==========")
                self.rate_.sleep()
        except ROSInterruptException:
            pass
        except RuntimeError:
            if not self.stop_event_.is_set():
                raise

    def stop(self):
        self.stop_event_.set()
        self.rate_.destroy()


def main():
    rclpy.init()
    node = RateDemo()

    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.stop()
        node.thread_.join()
        node.destroy_node()

        if rclpy.ok():
            rclpy.shutdown()


if __name__ == "__main__":
    main()

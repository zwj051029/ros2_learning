import rclpy
from rclpy.node import Node

class MyNode(Node):
    def __init__(self):
        super().__init__("my_node_py", namespace = "my_namespace_py")
        self.get_logger().info("节点创建成功!")

def main():
    rclpy.init()
    node = MyNode()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()

if __name__ == "__main__":
    main()
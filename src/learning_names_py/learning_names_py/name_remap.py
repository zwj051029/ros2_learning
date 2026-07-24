import rclpy
from rclpy.node import Node
from std_msgs.msg import String

class MyNode(Node):
    def __init__(self):
        super().__init__("my_node_py", namespace = "my_namespace_py")
        self.get_logger().info("节点创建成功!")
        # 全局话题
        # self.publisher_ = self.create_publisher(String, "/quanjv", 10)
        # 相对话题
        # self.publisher_ = self.create_publisher(String, "xiangdui", 10)
        # 私有话题
        self.publisher_ = self.create_publisher(String, "~/siyou", 10)

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
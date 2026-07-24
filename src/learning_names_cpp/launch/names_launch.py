from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        # 解决节点重名
        # Node(
        #     package = "turtlesim",
        #     executable = "turtlesim_node",
        #     name = "turtle1"
        # ),
        # Node(
        #     package = "turtlesim",
        #     executable = "turtlesim_node",
        #     namespace = "t1"
        # ),
        # Node(
        #     package = "turtlesim",
        #     executable = "turtlesim_node",
        #     namespace = "t1",
        #     name = "turtle1"
        # )

        # 解决话题重名
        Node(
            package = "turtlesim",
            executable = "turtlesim_node",
            namespace = "t1"
        ),
        Node(
            package = "turtlesim",
            executable = "turtlesim_node",
            remappings = [("/turtle1/cmd_vel", "/cmd_vel")]
        )
    ])
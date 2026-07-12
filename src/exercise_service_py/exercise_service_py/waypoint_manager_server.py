import rclpy
from rclpy.node import Node
from base_interfaces.srv import ManageWaypoint

class WaypointManagerServer(Node):
    def __init__(self):
        super().__init__("waypoint_manager_server_py")
        self.get_logger().info("服务端创建成功!(Python)")
        self.server_ = self.create_service(ManageWaypoint, "manage_waypoint", self.waypoint_manager_callback)
        self.waypoint_ = []

    def waypoint_manager_callback(self, req, res):
        operation = req.operation
        waypoint_name = req.waypoint_name
        x = req.x
        y = req.y
        yaw = req.yaw

        if operation == "add":
            if not waypoint_name:
                self.set_failure_response(res, "航点名称不能为空!")
            elif self.has_waypoint(waypoint_name):
                self.set_failure_response(res, "存在同名航点!")
            elif len(self.waypoint_) >= 5:
                self.set_failure_response(res, "当前航点数量已达到 5 个!")
            elif (
                x < -10.0 - 1e-6 or x > 10.0 + 1e-6 or
                y < -10.0 - 1e-6 or y > 10.0 + 1e-6 or
                yaw < -3.14 - 1e-6 or yaw > 3.14 + 1e-6
            ):
                self.set_failure_response(res, "航点参数不合法!")
            else:
                new_waypoint = {"waypoint_name": waypoint_name, "x": x, "y": y, "yaw": yaw}
                self.waypoint_.append(new_waypoint)
                res.success = True
                res.message = "航点添加成功!"
                self.get_logger().info("航点添加成功!")
        elif operation == "delete":
            index = self.find_waypoint_index(waypoint_name)
            if index == -1:
                self.set_failure_response(res, "该航点不存在!")
            else:
                self.waypoint_.pop(index)
                res.success = True
                res.message = "航点删除成功!"
                self.get_logger().info("航点删除成功!")
        elif operation == "query":
            res.success = True
            res.message = "航点查询成功!"
            self.get_logger().info("航点查询成功!")
        elif operation == "clear":
            self.waypoint_.clear()
            res.success = True
            res.message = "航点清空成功!"
            self.get_logger().info("航点清空成功!")
        else:
            self.set_failure_response(res, "未知航点!")

        self.fill_waypoints_response(res)
        return res

    def set_failure_response(self, res, message):
        res.success = False
        res.message = message
        self.get_logger().warn(f"{message}")

    def has_waypoint(self, waypoint_name):
        for waypoint in self.waypoint_:
            if waypoint_name == waypoint["waypoint_name"]:
                return True
        return False
    
    def find_waypoint_index(self, waypoint_name):
        for i in range(len(self.waypoint_)):
            if waypoint_name == self.waypoint_[i]["waypoint_name"]:
                return i
        return -1

    def fill_waypoints_response(self, res):
        res.waypoint_names = []
        res.xs = []
        res.ys = []
        res.yaws = []

        for waypoint in self.waypoint_:
            res.waypoint_names.append(waypoint["waypoint_name"])
            res.xs.append(waypoint["x"])
            res.ys.append(waypoint["y"])
            res.yaws.append(waypoint["yaw"])
        
        res.total_count = len(self.waypoint_)

def main():
    rclpy.init()
    server = WaypointManagerServer()

    try:
        rclpy.spin(server)
    except KeyboardInterrupt:
        pass
    finally:
        server.destroy_node()

        if rclpy.ok():
            rclpy.shutdown()

if __name__ == "__main__":
    main()

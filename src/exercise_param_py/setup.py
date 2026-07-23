from setuptools import find_packages, setup

package_name = "exercise_param_py"

setup(
    name=package_name,
    version="0.0.0",
    packages=find_packages(exclude=["test"]),
    data_files=[
        (
            "share/ament_index/resource_index/packages",
            ["resource/" + package_name]
        ),
        ("share/" + package_name, ["package.xml"]),
    ],
    install_requires=["setuptools"],
    zip_safe=True,
    maintainer="zhangwenjing",
    maintainer_email="924110800235@njust.edu.cn",
    description="ROS 2 Python mobile robot parameter management exercise",
    license="TODO: License declaration",
    extras_require={
        "test": [
            "pytest",
        ],
    },
    entry_points={
        "console_scripts": [
            "robot_config_server = "
            "exercise_param_py.robot_config_server:main",
            "navigation_config_server = "
            "exercise_param_py.navigation_config_server:main",
        ],
    },
)

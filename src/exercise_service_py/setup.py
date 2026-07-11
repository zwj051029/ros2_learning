from setuptools import find_packages, setup

package_name = 'exercise_service_py'

setup(
    name=package_name,
    version='0.0.0',
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='zhangwenjing',
    maintainer_email='924110800235@njust.edu.cn',
    description='TODO: Package description',
    license='TODO: License declaration',
    extras_require={
        'test': [
            'pytest',
        ],
    },
    entry_points={
        'console_scripts': [
            'robot_mode_server = exercise_service_py.robot_mode_server:main',
            'robot_mode_client = exercise_service_py.robot_mode_client:main',
        ],
    },
)

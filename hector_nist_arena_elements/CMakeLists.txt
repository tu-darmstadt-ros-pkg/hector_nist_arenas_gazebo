# Catkin User Guide: http://www.ros.org/doc/groovy/api/catkin/html/user_guide/user_guide.html
# Catkin CMake Standard: http://www.ros.org/doc/groovy/api/catkin/html/user_guide/standards.html
cmake_minimum_required(VERSION 3.0.2)
project(hector_nist_arena_elements)
# Load catkin and all dependencies required for this package
# TODO: remove all from COMPONENTS that are not catkin packages.
find_package(catkin REQUIRED COMPONENTS gazebo_ros)

catkin_package(
    DEPENDS 
    CATKIN_DEPENDS gazebo_ros
    INCLUDE_DIRS # TODO include
    LIBRARIES # TODO
)

install(DIRECTORY elements Media DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION})

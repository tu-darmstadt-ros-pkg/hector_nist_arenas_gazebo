# Catkin User Guide: http://www.ros.org/doc/groovy/api/catkin/html/user_guide/user_guide.html
# Catkin CMake Standard: http://www.ros.org/doc/groovy/api/catkin/html/user_guide/standards.html
cmake_minimum_required(VERSION 3.0.2)
project(hector_nist_arena_worlds)
# Load catkin and all dependencies required for this package
# TODO: remove all from COMPONENTS that are not catkin packages.
find_package(catkin REQUIRED COMPONENTS hector_nist_arena_elements)

catkin_package(
    DEPENDS 
    CATKIN_DEPENDS hector_nist_arena_elements
    INCLUDE_DIRS # TODO include
    LIBRARIES # TODO
)

install(DIRECTORY arenas launch worlds DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION})

cmake_minimum_required(VERSION 3.5)
project(hector_nist_arena_worlds)
# Load catkin and all dependencies required for this package
# TODO: remove all from COMPONENTS that are not catkin packages.
find_package(ament_cmake REQUIRED)

install(DIRECTORY
  arenas
  launch
  worlds
  DESTINATION share/${PROJECT_NAME}
)

ament_package()

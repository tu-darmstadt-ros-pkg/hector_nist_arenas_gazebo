# Catkin User Guide: http://www.ros.org/doc/groovy/api/catkin/html/user_guide/user_guide.html
# Catkin CMake Standard: http://www.ros.org/doc/groovy/api/catkin/html/user_guide/standards.html
cmake_minimum_required(VERSION 3.0.2)
project(hector_nist_arena_designer)
# Load catkin and all dependencies required for this package
# TODO: remove all from COMPONENTS that are not catkin packages.
find_package(catkin REQUIRED )

find_package(Qt5 COMPONENTS Widgets Xml REQUIRED)

set(CMAKE_AUTOMOC ON)

catkin_package(
    DEPENDS Qt5Widgets Qt5Xml
    CATKIN_DEPENDS # TODO
    INCLUDE_DIRS # TODO include
    LIBRARIES
)

include_directories(src)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Here only headers neet to be listed that must be processed by Qt's moc.
# Specifically, these are headers for classes derived from QObject.
set(HEADERS
    src/model/arena.h
    src/model/arenaelement.h
    src/ui/arenascene.h
    src/ui/arenaelementtypescene.h
    src/ui/arenaview.h
    src/ui/arenasceneelement.h
    src/ui/editor.h
    src/ui/arenacontroller.h
    src/ui/metainfobox.h
)

set(SOURCES
    src/model/arena.cpp
    src/model/arenaelement.cpp
    src/model/arenaelementtype.cpp
    src/model/arenaelementtyperegistry.cpp
    src/model/xmlloadingexception.cpp
    src/ui/arenaelementtypescene.cpp
    src/ui/arenascene.cpp
    src/ui/arenaview.cpp
    src/ui/arenasceneelement.cpp
    src/ui/editor.cpp
    src/ui/arenacontroller.cpp
    src/ui/metainfobox.cpp
    src/main.cpp
)

set(UI_FILES
    src/ui/editor.ui
)

qt5_wrap_ui(HEADERS_UI_FILES ${UI_FILES})
qt5_add_resources(RESOURCES_RCC resources.qrc)

add_executable(arena_designer ${SOURCES} ${HEADERS}  ${HEADERS_UI_FILES} ${RESOURCES_RCC})
target_link_libraries(arena_designer Qt5::Widgets Qt5::Xml)

install(TARGETS arena_designer DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION})
install(DIRECTORY icons DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION})

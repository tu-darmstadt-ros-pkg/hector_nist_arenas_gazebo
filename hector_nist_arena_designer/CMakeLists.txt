# Catkin User Guide: http://www.ros.org/doc/groovy/api/catkin/html/user_guide/user_guide.html
# Catkin CMake Standard: http://www.ros.org/doc/groovy/api/catkin/html/user_guide/standards.html
cmake_minimum_required(VERSION 2.8.3)
project(hector_nist_arena_designer)
# Load catkin and all dependencies required for this package
# TODO: remove all from COMPONENTS that are not catkin packages.
find_package(catkin REQUIRED )

# include_directories(include ${Boost_INCLUDE_DIR} ${catkin_INCLUDE_DIRS})
# CATKIN_MIGRATION: removed during catkin migration
# cmake_minimum_required(VERSION 2.4.6)

catkin_package(
    DEPENDS # TODO add dependencies
    CATKIN_DEPENDS # TODO
    INCLUDE_DIRS # TODO include
    LIBRARIES # TODO
)



find_package(Qt4 4.6 COMPONENTS QtCore QtGui QtXml REQUIRED)
include(${QT_USE_FILE})
#add_definitions(-DQT_NO_KEYWORDS)
add_definitions(${QT_DEFINITIONS})

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

include(${QT_USE_FILE})


qt4_wrap_ui(HEADERS_UI_FILES ${UI_FILES})
qt4_wrap_cpp(HEADERS_MOC ${HEADERS})
qt4_add_resources(RESOURCES_RCC resources.qrc)

if($ENV{ROS_ROOT})
    add_executable(arena_designer ${SOURCES} ${HEADERS_MOC}  ${HEADERS_UI_FILES} ${RESOURCES_RCC})
else()
    add_executable(arena_designer ${SOURCES} ${HEADERS_MOC}  ${HEADERS_UI_FILES} ${RESOURCES_RCC})
endif()
target_link_libraries(arena_designer ${QT_LIBRARIES})


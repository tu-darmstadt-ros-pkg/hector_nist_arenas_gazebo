cmake_minimum_required(VERSION 2.4.6)

if($ENV{ROS_ROOT})
    include($ENV{ROS_ROOT}/core/rosbuild/rosbuild.cmake)
    set(ROS_BUILD_TYPE Debug)
    rosbuild_init()
endif()

#set the default path for built executables to the "bin" directory
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
#set the default path for built libraries to the "lib" directory
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

find_package(Qt4 4.6 COMPONENTS QtCore QtGui QtXml REQUIRED)
include(${QT_USE_FILE})

# Here only headers neet to be listed that must be processed by Qt's moc.
# Specifically, these are headers for classes derived from QObject.
set(HEADERS
    ${CMAKE_SOURCE_DIR}/src/model/arena.h
    ${CMAKE_SOURCE_DIR}/src/model/arenaelement.h
    ${CMAKE_SOURCE_DIR}/src/ui/arenascene.h
    ${CMAKE_SOURCE_DIR}/src/ui/arenaelementtypescene.h
    ${CMAKE_SOURCE_DIR}/src/ui/arenaview.h
    ${CMAKE_SOURCE_DIR}/src/ui/arenasceneelement.h
    ${CMAKE_SOURCE_DIR}/src/ui/editor.h
    ${CMAKE_SOURCE_DIR}/src/ui/arenacontroller.h
    ${CMAKE_SOURCE_DIR}/src/ui/metainfobox.h
)

set(SOURCES
    ${CMAKE_SOURCE_DIR}/src/model/arena.cpp
    ${CMAKE_SOURCE_DIR}/src/model/arenaelement.cpp
    ${CMAKE_SOURCE_DIR}/src/model/arenaelementtype.cpp
    ${CMAKE_SOURCE_DIR}/src/model/arenaelementtyperegistry.cpp
    ${CMAKE_SOURCE_DIR}/src/model/xmlloadingexception.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/arenaelementtypescene.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/arenascene.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/arenaview.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/arenasceneelement.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/editor.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/arenacontroller.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/metainfobox.cpp
    ${CMAKE_SOURCE_DIR}/src/main.cpp
)

set(UI_FILES
    ${CMAKE_SOURCE_DIR}/src/ui/editor.ui
)

include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_BINARY_DIR})

qt4_wrap_ui(HEADERS_UI_FILES ${UI_FILES})
qt4_wrap_cpp(HEADERS_MOC ${HEADERS})
qt4_add_resources(RESOURCES_RCC resources.qrc)

if($ENV{ROS_ROOT})
    rosbuild_add_executable(arena_designer ${SOURCES} ${HEADERS_MOC} ${UI_FILES} ${RESOURCES_RCC})
else()
    add_executable(arena_designer ${SOURCES} ${HEADERS_MOC} ${UI_FILES} ${RESOURCES_RCC})
endif()
target_link_libraries(arena_designer ${QT_LIBRARIES})

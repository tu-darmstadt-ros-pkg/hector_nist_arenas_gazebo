cmake_minimum_required(VERSION 3.5)
project(hector_nist_arena_designer)

find_package(ament_cmake REQUIRED)
find_package(ament_index_cpp REQUIRED)

find_package(Qt5 COMPONENTS Widgets Xml REQUIRED)

set(CMAKE_AUTOMOC ON)

include_directories(include)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Here only headers need to be listed that must be processed by Qt's moc.
# Specifically, these are headers for classes derived from QObject.
set(HEADERS
  include/hector_nist_arena_designer/model/arena.h
  include/hector_nist_arena_designer/model/arenaelement.h
  include/hector_nist_arena_designer/model/arenaelementtype.h
  include/hector_nist_arena_designer/model/arenaelementtyperegistry.h
  include/hector_nist_arena_designer/model/xmlloadingexception.h
  include/hector_nist_arena_designer/ui/arenascene.h
  include/hector_nist_arena_designer/ui/arenaelementtypescene.h
  include/hector_nist_arena_designer/ui/arenaview.h
  include/hector_nist_arena_designer/ui/arenasceneelement.h
  include/hector_nist_arena_designer/ui/editor.h
  include/hector_nist_arena_designer/ui/arenacontroller.h
  include/hector_nist_arena_designer/ui/metainfobox.h
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

add_executable(arena_designer ${SOURCES}
  ${HEADERS} ${HEADERS_UI_FILES} ${RESOURCES_RCC}
)
#add_executable(arena_designer src/main.cpp
#  ${SOURCES} ${HEADERS} ${HEADERS_UI_FILES} ${RESOURCES_RCC}
#)
target_link_libraries(arena_designer Qt5::Widgets Qt5::Xml)
ament_target_dependencies(arena_designer
  ament_index_cpp
)

install(TARGETS arena_designer
  DESTINATION lib/${PROJECT_NAME}
)
install(DIRECTORY icons include config
  DESTINATION share/${PROJECT_NAME}/
)

ament_package()

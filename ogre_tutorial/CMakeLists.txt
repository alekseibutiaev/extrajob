cmake_minimum_required(VERSION 3.5)

project(ogre_tutorial)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS_DEBUG "")
set(CMAKE_CXX_FLAGS_RELEASE "")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g3 -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -g0 -Wall")

if(WIN32)
	set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "$ENV{OGRE_HOME}/CMake")
elseif(UNIX)
	if(EXISTS "/usr/local/lib/OGRE/cmake")
	  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "/usr/local/lib/OGRE/cmake")
    set(OGRE_HOME "/usr/local/include/OGRE")
	elseif(EXISTS "/usr/lib/OGRE/cmake")
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "/usr/lib/OGRE/cmake")
    set(OGRE_HOME "/usr/include/OGRE")
  elseif(DEFINED OGRE_HOME AND EXISTS "${OGRE_HOME}/lib/OGRE/cmake")
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${OGRE_HOME}/lib/OGRE/cmake")
	else (EXISTS "/usr/local/lib/OGRE/cmake")
	  message(SEND_ERROR "Failed to find module path.")
	endif(EXISTS "/usr/local/lib/OGRE/cmake")
endif(WIN32)

message("path: " ${CMAKE_MODULE_PATH})

message("OGRE_HOME: " ${OGRE_HOME})

find_package(OIS REQUIRED)
find_package(OGRE 1.9 REQUIRED)

message("OGRE dounf is: " ${OGRE_FOUND})
message("OGRE include dir is: " ${OGRE_INCLUDE_DIRS})
message("OGRE Link dir is: " ${OGRE_LIBRARY_DIRS})
message("OGRE Libraries is: " ${OGRE_LIBRARIES})

message("system has OIS: " ${OIS_FOUND})
message("the OIS include directories: " ${OIS_INCLUDE_DIRS})
message("link these to use OIS: " ${OIS_LIBRARIES})

#file(COPY ${OGRE_CONFIG_DIR}/resources.cfg DESTINATION ${CMAKE_BINARY_DIR})

include_directories(
  ${OGRE_HOME}/include/OGRE
  ${OGRE_HOME}/include/OGRE/RenderSystems/GL
  ${OGRE_HOME}/include/OGRE/Plugins/ParticleFX

  ${OIS_INCLUDE_DIRS}
)

link_directories(
  ${OGRE_LIBRARY_DIRS}
)

add_definitions(-DOGRE_HOME="${OGRE_HOME}")

add_library(application STATIC application.cpp)


add_executable(baseapp baseapp.cpp)
add_executable(tutorial_1 tutorial_1.cpp)
add_executable(tutorial_2 tutorial_2.cpp)
add_executable(tutorial_3 tutorial_3.cpp)
add_executable(tutorial_4 tutorial_4.cpp)

target_link_libraries (baseapp
  application
  ${OGRE_LIBRARIES}
)

target_link_libraries (tutorial_1
  application
  ${OGRE_LIBRARIES}
)

target_link_libraries (tutorial_2
  application
  ${OGRE_LIBRARIES}
)

target_link_libraries (tutorial_3
  application
  ${OGRE_LIBRARIES}
)

target_link_libraries (tutorial_4
  application
  ${OGRE_LIBRARIES}
)

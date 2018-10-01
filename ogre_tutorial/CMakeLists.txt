cmake_minimum_required(VERSION 3.5)

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
	elseif(EXISTS "/usr/lib/OGRE/cmake")
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "/usr/lib/OGRE/cmake")
  elseif(DEFINED OGRE_HOME AND EXISTS "${OGRE_HOME}/lib/OGRE/cmake")
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${OGRE_HOME}/lib/OGRE/cmake")
	else (EXISTS "/usr/local/lib/OGRE/cmake")
	  message(SEND_ERROR "Failed to find module path.")
	endif(EXISTS "/usr/local/lib/OGRE/cmake")
endif(WIN32)

message("path: " ${CMAKE_MODULE_PATH})

find_package(OGRE 1.10 REQUIRED)

message("OGRE include dir is: "${OGRE_INCLUDE_DIRS})
message("OGRE Link dir is: "${OGRE_LIBRARY_DIRS})
message("OGRE Link dir is: "${OGRE_CONFIG_DIR})

file(COPY ${OGRE_CONFIG_DIR}/resources.cfg DESTINATION ${CMAKE_BINARY_DIR})

project(ogre_tutorial)

include_directories(
  ${OGRE_INCLUDE_DIRS}
  ${OGRE_INCLUDE_DIRS}/Property
  ${OGRE_INCLUDE_DIRS}/Volume
  ${OGRE_INCLUDE_DIRS}/HLMS
  ${OGRE_INCLUDE_DIRS}/MeshLodGenerator
  ${OGRE_INCLUDE_DIRS}/Bites
  ${OGRE_INCLUDE_DIRS}/Overlay
  ${OGRE_INCLUDE_DIRS}/Plugins
  ${OGRE_INCLUDE_DIRS}/Plugins/OctreeZone
  ${OGRE_INCLUDE_DIRS}/Plugins/BSPSceneManager
  ${OGRE_INCLUDE_DIRS}/Plugins/FreeImageCodec
  ${OGRE_INCLUDE_DIRS}/Plugins/ParticleFX
  ${OGRE_INCLUDE_DIRS}/Plugins/PCZSceneManager
  ${OGRE_INCLUDE_DIRS}/Plugins/OctreeSceneManager
  ${OGRE_INCLUDE_DIRS}/Plugins/STBICodec
  ${OGRE_INCLUDE_DIRS}/Terrain
  ${OGRE_INCLUDE_DIRS}/RenderSystems
  ${OGRE_INCLUDE_DIRS}/RenderSystems/GL3Plus
  ${OGRE_INCLUDE_DIRS}/RenderSystems/GL3Plus/GL
  ${OGRE_INCLUDE_DIRS}/RenderSystems/GL
  ${OGRE_INCLUDE_DIRS}/RenderSystems/GL/GL
  ${OGRE_INCLUDE_DIRS}/RTShaderSystem
  ${OGRE_INCLUDE_DIRS}/Threading
  ${OGRE_INCLUDE_DIRS}/Paging
)

link_directories(${OGRE_LIBRARY_DIRS})

add_executable(ogre_tutorial_1 ogre_tutorial_1.cpp)
add_executable(ogre_tutorial_2 ogre_tutorial_2.cpp)

target_link_libraries (ogre_tutorial_1
  OgreBites
  OgreHLMS
  OgreMain
  OgreMeshLodGenerator
  OgreOverlay
  OgrePaging
  OgreProperty
  OgreRTShaderSystem
  OgreTerrain
  OgreVolume
)

target_link_libraries (ogre_tutorial_2
  OgreBites
  OgreHLMS
  OgreMain
  OgreMeshLodGenerator
  OgreOverlay
  OgrePaging
  OgreProperty
  OgreRTShaderSystem
  OgreTerrain
  OgreVolume
)


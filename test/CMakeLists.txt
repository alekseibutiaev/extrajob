cmake_minimum_required (VERSION 3.5)


set (CMAKE_CXX_STANDARD 11)

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

message("OGRE directories is: "  ${OGRE_HOME})
message("the OGRE include directories is: "  ${OGRE_INCLUDE_DIRS})
message("the location of the libraries is: "  ${OGRE_LIBRARY_DIRS})
message("link these to use the OGRE is: "  ${OGRE_LIBRARIES})
#message("whther ogre was build as static lib: "  ${OGRE_STATIC})
#message("Bites is available is: " ${OGRE_Bites_FOUND})
#message("link these to only use Bites is: " ${OGRE_Bites_LIBRARIES}) 
#message("The directory where the OGRE plugins are located is: " ${OGRE_PLUGIN_DIR})
#message("The directory where the OGRE sample media is located is: " ${OGRE_MEDIA_DIR})
#message("The directory where the OGRE config files are located is: " ${OGRE_CONFIG_DIR})



project (cmake_test)

add_executable(cmake_test cmake_test.cpp)

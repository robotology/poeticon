# Copyright: (C) 2011 RobotCub Consortium
# Author: Vadim Tikhanoff
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

cmake_minimum_required(VERSION 2.6)
set(PROJECTNAME activeSegmentation)

project(${PROJECTNAME})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR})

option(DEBUG_INFO "Show debug messages" FALSE)

SET(USE_MULTI_THREADED FALSE CACHE BOOL "Select false if you do not have boost installed")

FILE(GLOB folder_source src/*.cpp src/*.cc src/*.c)
FILE(GLOB folder_header include/iCub/*.h include/iCub/*.inc)
source_group("Source Files" FILES ${folder_source})
source_group("Header Files" FILES ${folder_header})

if(DEBUG_INFO)
    ADD_DEFINITIONS( -DACTIVESEG_DEBUG )
endif (DEBUG_INFO)

include_directories(${PROJECT_SOURCE_DIR}/include
                    ${OpenCV_INCLUDE_DIRS})

IF (USE_MULTI_THREADED)
    add_library(${PROJECTNAME} ${folder_source} src/seg/textureGradient_multithreaded.cpp ${folder_header} ${folder_header})
ELSE()
    add_library( ${PROJECTNAME} ${folder_source} src/seg/textureGradient.cpp ${folder_header}) 
ENDIF (USE_MULTI_THREADED)	

TARGET_LINK_LIBRARIES(${PROJECTNAME} ${YARP_LIBRARIES} ${OpenCV_LIBRARIES})

# export library
icubcontrib_export_library( ${PROJECTNAME}
                            INTERNAL_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/include 
                            EXTERNAL_INCLUDE_DIRS ${OpenCV_INCLUDE_DIRS}
                            FILES ${folder_header}
                            DESTINATION include/iCub
                            )

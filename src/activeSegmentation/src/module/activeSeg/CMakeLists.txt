# Copyright: 2012 iCub Facility, Istituto Italiano di Tecnologia
# Author: Vadim Tikhanoff
# CopyPolicy: Released under the terms of the GNU GPL v2.0.
# 

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
SET(PROJECTNAME activeSeg)
PROJECT(${PROJECTNAME})

#this to be used externally
#find_package(activeSeg)

file(GLOB folder_source src/*.cpp)
file(GLOB folder_header include/iCub/*.h)

source_group("Source Files" FILES ${folder_source})
source_group("Header Files" FILES ${folder_header})

include_directories(
		    ${PROJECT_SOURCE_DIR}/include
		    ${YARP_INCLUDE_DIRS}
            ${activeSegmentation_INCLUDE_DIRS}
)

add_executable( ${PROJECTNAME} ${folder_source} ${folder_header} )

target_link_libraries(${PROJECTNAME} ${YARP_LIBRARIES} activeSegmentation ${OpenCV_LIBRARIES})

install(TARGETS ${PROJECTNAME} DESTINATION bin)

# Copyright: 2012 iCub Facility, Istituto Italiano di Tecnologia
# Author: Vadim Tikhanoff
# CopyPolicy: Released under the terms of the GNU GPL v2.0.
# 

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
SET(PROJECTNAME activeParticleTrack)
PROJECT(${PROJECTNAME})

find_package(GSL REQUIRED)
#this is needed externally
#find_package(activeSegmentation)

find_package(Threads REQUIRED)

yarp_idl_to_dir(${PROJECTNAME}.thrift ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB folder_source src/*.cpp)
file(GLOB folder_header include/*.h include/iCub/*.h)
set(idl activeParticleTrack.thrift)

source_group("Source Files" FILES ${folder_source})
source_group("Header Files" FILES ${folder_header})
source_group("IDL Files" FILES ${idl})

include_directories(
		    ${PROJECT_SOURCE_DIR}/include
            ${GSL_INCLUDE_DIRS}
            ${activeSegmentation_INCLUDE_DIRS}
)

add_executable( ${PROJECTNAME} ${folder_source} ${folder_header} ${idl} )

target_link_libraries(${PROJECTNAME} ${GSL_LIBRARIES} ${YARP_LIBRARIES} activeSegmentation ${OpenCV_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

install(TARGETS ${PROJECTNAME} DESTINATION bin)

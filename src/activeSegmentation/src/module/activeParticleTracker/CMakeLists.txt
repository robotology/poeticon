# Copyright: 2012 iCub Facility, Istituto Italiano di Tecnologia
# Author: Vadim Tikhanoff
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

cmake_minimum_required(VERSION 2.6)

project(activeParticleTrack)

find_package(GSL REQUIRED)
# removed activeSegmentation, we use IOL tracking. https://github.com/robotology/poeticon/issues/194
#find_package(activeSegmentation)
find_package(Threads REQUIRED)

yarp_idl_to_dir(${PROJECT_NAME}.thrift ${CMAKE_CURRENT_SOURCE_DIR})

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

add_executable( ${PROJECT_NAME} ${folder_source} ${folder_header} ${idl} )

target_link_libraries(${PROJECT_NAME} ${GSL_LIBRARIES} ${YARP_LIBRARIES} activeSegmentation ${OpenCV_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

install(TARGETS ${PROJECT_NAME} DESTINATION bin)

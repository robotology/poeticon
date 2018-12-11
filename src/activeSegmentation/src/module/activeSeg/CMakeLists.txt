# Copyright: 2012 iCub Facility, Istituto Italiano di Tecnologia
# Author: Vadim Tikhanoff
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

cmake_minimum_required(VERSION 2.6)

project(activeSeg)

#this to be used externally
#find_package(activeSeg)

find_package(Threads REQUIRED)

file(GLOB folder_source src/*.cpp)
file(GLOB folder_header include/iCub/*.h)

source_group("Source Files" FILES ${folder_source})
source_group("Header Files" FILES ${folder_header})

include_directories(
		    ${PROJECT_SOURCE_DIR}/include
            ${activeSegmentation_INCLUDE_DIRS}
)

add_executable( ${PROJECT_NAME} ${folder_source} ${folder_header} )

target_link_libraries(${PROJECT_NAME} ${YARP_LIBRARIES} activeSegmentation ${OpenCV_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

install(TARGETS ${PROJECT_NAME} DESTINATION bin)

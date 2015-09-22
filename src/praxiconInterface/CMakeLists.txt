# Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
# Copyright: (C) 2015 iCub Facility, Istituto Italiano di Tecnologia
# Author: Vadim Tikhanoff <Vadim.Tikhanoff@iit.it>
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

cmake_minimum_required(VERSION 2.6)

project(praxiconInterface)

find_package(YARP)
find_package(ICUB)

#optionally: use cmake find scripts provided by YARP and iCub
set(CMAKE_MODULE_PATH ${YARP_DIR}/conf ${CMAKE_MODULE_PATH})
set(CMAKE_MODULE_PATH ${ICUB_DIR}/conf ${CMAKE_MODULE_PATH})

include_directories(${YARP_INCLUDE_DIRS}
                    ${CMAKE_SOURCE_DIR})
include_directories(${ICUB_INCLUDE_DIRS})

file(GLOB folder_source src/*.cpp)
file(GLOB folder_header include/iCub/*.h include/iCub/*.nsmap)
source_group("Source Files" FILES ${folder_source})
source_group("Header Files" FILES ${folder_header})

add_executable(${PROJECT_NAME} ${folder_source} ${folder_header})

target_link_libraries(${PROJECT_NAME} ${YARP_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${ICUB_LIBRARIES})

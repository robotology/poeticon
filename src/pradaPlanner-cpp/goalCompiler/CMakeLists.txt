# Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
# Copyright: (C) 2014 VisLab, Institute for Systems and Robotics,
#                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
# Authors: Alexandre Antunes <aleksander88@gmail.com>
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

cmake_minimum_required(VERSION 2.6)

project(goalCompiler)

find_package(YARP)
find_package(ICUB)
find_package(ICUBcontrib)

list(APPEND CMAKE_MODULE_PATH ${YARP_MODULE_PATH})
list(APPEND CMAKE_MODULE_PATH ${ICUBCONTRIB_MODULE_PATH})

include(YarpInstallationHelpers)
include(ICUBcontribHelpers)
include(ICUBcontribOptions)
icubcontrib_set_default_prefix()

file(GLOB source src/*.cpp ../common/src/*.cpp)
file(GLOB header include/*.h ../common/include/*.h)

source_group("Source Files" FILES ${source})
source_group("Header Files" FILES ${header})

include_directories(${PROJECT_SOURCE_DIR}/include
                    ${PROJECT_SOURCE_DIR}/../common/include
                    ${YARP_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} ${source} ${header})

target_link_libraries(${PROJECT_NAME} ${YARP_LIBRARIES}) 

install(TARGETS ${PROJECT_NAME} DESTINATION bin)

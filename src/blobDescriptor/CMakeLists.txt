# Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
# Copyright: (C) 2014 VisLab, Institute for Systems and Robotics,
#                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
# Authors: Giovanni Saponaro <gsaponaro@isr.tecnico.ulisboa.pt>
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

cmake_minimum_required(VERSION 2.6)

project(blobDescriptor)

find_package(YARP REQUIRED
                  COMPONENTS OS sig)
find_package(ICUBcontrib)

list(APPEND CMAKE_MODULE_PATH ${ICUBCONTRIB_MODULE_PATH})

include(ICUBcontribHelpers)
include(ICUBcontribOptions)
icubcontrib_set_default_prefix()

find_package(OpenCV REQUIRED)

include_directories(
		${PROJECT_SOURCE_DIR}/include
		${OpenCV_INCLUDE_DIR})

file(GLOB folder_source src/*.cpp src/*.cc src/*.c)
file(GLOB folder_header include/iCub/*.h)
source_group("Source Files" FILES ${folder_source})
source_group("Header Files" FILES ${folder_header})

add_executable(${PROJECT_NAME} ${folder_source} ${folder_header})
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBRARIES} ${YARP_LIBRARIES})

install(TARGETS ${PROJECT_NAME} DESTINATION bin)

#icubcontrib_finalize_export (${PROJECT_NAME})
#icubcontrib_add_uninstall_target()

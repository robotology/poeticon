# Copyright: (C) 2014 VisLab, Institute for Systems and Robotics,
#                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
# Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
# Authors: Afonso Gonçalves <agoncalves@isr.ist.utl.pt>
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

cmake_minimum_required(VERSION 2.6)

set(PROJECTNAME perspectiveChanger)
project(${PROJECTNAME})

find_package(YARP)
find_package(ICUB)
find_package(ICUBcontrib)
find_package(OpenCV REQUIRED)

list(APPEND CMAKE_MODULE_PATH ${YARP_MODULE_PATH})
list(APPEND CMAKE_MODULE_PATH ${ICUB_MODULE_PATH})
list(APPEND CMAKE_MODULE_PATH ${ICUBCONTRIB_MODULE_PATH})

include(YarpInstallationHelpers)
include(ICUBcontribHelpers)
include(ICUBcontribOptions)
icubcontrib_set_default_prefix()

file(GLOB folder_source src/*.cpp)
file(GLOB folder_header include/*.h)
source_group("Source Files" FILES ${folder_source})
source_group("Header Files" FILES ${folder_header})

include_directories(${PROJECT_SOURCE_DIR}/include
                    ${OpenCV_INCLUDE_DIRS}
                    ${YARP_INCLUDE_DIRS}
                    ${ctrlLib_INCLUDE_DIRS}
                   )

add_executable(${PROJECTNAME} ${folder_source} ${folder_header})

target_link_libraries(${PROJECTNAME}
                      ${OpenCV_LIBRARIES}
                      ${YARP_LIBRARIES}
                      ctrlLib)

install(TARGETS ${PROJECTNAME} DESTINATION bin)

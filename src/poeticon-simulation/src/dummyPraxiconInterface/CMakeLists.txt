# Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
# Copyright: (C) 2018 VisLab, Institute for Systems and Robotics,
#                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
# Authors: Giovanni Saponaro <gsaponaro@isr.tecnico.ulisboa.pt>
# CopyPolicy: Released under the terms of the GNU GPL v3.0.
#
# Based on praxiconInterface by Vadim Tikhanoff

cmake_minimum_required(VERSION 2.8)

project(dummyPraxiconInterface)

find_package(YARP)
find_package(ICUBcontrib)
list(APPEND CMAKE_MODULE_PATH ${YARP_MODULE_PATH})
list(APPEND CMAKE_MODULE_PATH ${ICUBCONTRIB_MODULE_PATH})

include(ICUBcontribHelpers)
include(ICUBcontribOptions)
icubcontrib_set_default_prefix()

include_directories(${PROJECT_SOURCE_DIR}/include
                    ${YARP_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} include/DummyPraxiconInterfaceModule.h
                               src/DummyPraxiconInterfaceMain.cpp
                               src/DummyPraxiconInterfaceModule.cpp)

target_link_libraries(${PROJECT_NAME} ${YARP_LIBRARIES})

yarp_install(TARGETS ${PROJECT_NAME} DESTINATION bin)

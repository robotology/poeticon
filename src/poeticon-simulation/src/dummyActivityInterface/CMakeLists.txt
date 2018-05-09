# Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
# Copyright: (C) 2018 VisLab, Institute for Systems and Robotics,
#                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
# Authors: Giovanni Saponaro <gsaponaro@isr.tecnico.ulisboa.pt>
# CopyPolicy: Released under the terms of the GNU GPL v2.0.
#
# Based on activityInterface by Vadim Tikhanoff

cmake_minimum_required(VERSION 2.6)

project(dummyActivityInterface)

find_package(YARP)
find_package(ICUBcontrib)
list(APPEND CMAKE_MODULE_PATH ${YARP_MODULE_PATH})
list(APPEND CMAKE_MODULE_PATH ${ICUBCONTRIB_MODULE_PATH})

include(YarpIDL)
include(YarpInstallationHelpers)
include(ICUBcontribHelpers)
include(ICUBcontribOptions)

icubcontrib_set_default_prefix()

yarp_add_idl(IDL_GEN_FILES ${PROJECT_NAME}.thrift)

include_directories(${PROJECT_SOURCE_DIR}/include
                    ${YARP_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} include/Defaults.h
                               include/DummyActivityInterfaceModule.h
                               include/DummyActivityInterfaceThread.h
                               src/DummyActivityInterfaceMain.cpp
                               src/DummyActivityInterfaceModule.cpp
                               src/DummyActivityInterfaceThread.cpp
                               ${IDL_GEN_FILES})

target_link_libraries(${PROJECT_NAME} ${YARP_LIBRARIES})

yarp_install(TARGETS ${PROJECT_NAME} DESTINATION bin)

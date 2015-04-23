# Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
# Copyright: (C) 2014 VisLab, Institute for Systems and Robotics,
#                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
# Authors: Alexandre Antunes <aleksander88@gmail.com>
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

cmake_minimum_required(VERSION 2.6)

set(PROJECTNAME goalCompiler)
project(${PROJECTNAME})

find_package(YARP)
find_package(ICUBcontrib)

list(APPEND CMAKE_MODULE_PATH ${YARP_MODULE_PATH})
list(APPEND CMAKE_MODULE_PATH ${ICUBCONTRIB_MODULE_PATH})

#include(YarpIDL)
#set(IDL_GENERATED_CODE_DIR "${CMAKE_CURRENT_SOURCE_DIR}") #?
#yarp_idl_to_dir(${PROJECTNAME}.thrift ${IDL_GENERATED_CODE_DIR})

include(YarpInstallationHelpers)
include(ICUBcontribHelpers)
include(ICUBcontribOptions)
icubcontrib_set_default_prefix()

file(GLOB source *.cpp)

source_group("Source Files" FILES ${source})

add_executable(${PROJECTNAME} ${source})

target_link_libraries(${PROJECTNAME} ${YARP_LIBRARIES}) 

install(PROGRAMS ${python_scripts} DESTINATION bin)

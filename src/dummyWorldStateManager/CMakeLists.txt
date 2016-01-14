# Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
# Copyright: (C) 2016 VisLab, Institute for Systems and Robotics,
#                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
# Authors: Giovanni Saponaro <gsaponaro@isr.ist.utl.pt>
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

cmake_minimum_required(VERSION 2.6)

project(dummyWorldStateManager)

find_package(YARP)
find_package(ICUBcontrib)

list(APPEND CMAKE_MODULE_PATH ${YARP_MODULE_PATH})
list(APPEND CMAKE_MODULE_PATH ${ICUBCONTRIB_MODULE_PATH})

include(YarpIDL)
set(IDL_GENERATED_CODE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
yarp_idl_to_dir(${PROJECT_NAME}.thrift ${IDL_GENERATED_CODE_DIR})

include(YarpInstallationHelpers)
include(ICUBcontribHelpers)
include(ICUBcontribOptions)
icubcontrib_set_default_prefix()

file(GLOB source src/*.cpp)
file(GLOB header include/*.h)

source_group("Source Files" FILES ${source})
source_group("Header Files" FILES ${header})

include_directories(${PROJECT_SOURCE_DIR}/include
                    ${YARP_INCLUDE_DIRS}
                    ${IDL_GENERATED_CODE_DIR}/include
                   )

add_executable(${PROJECT_NAME} ${source} ${header})

target_link_libraries(${PROJECT_NAME} ${YARP_LIBRARIES})

install(TARGETS ${PROJECT_NAME} DESTINATION bin)

#icubcontrib_finalize_export (${PROJECT_NAME})
#icubcontrib_add_uninstall_target()

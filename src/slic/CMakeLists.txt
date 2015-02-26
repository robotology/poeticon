# Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
# Copyright: (C) 2014 VisLab, Institute for Systems and Robotics,
#                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
# Authors: Giovanni Saponaro <gsaponaro@isr.ist.utl.pt> (YARP),
#          Saburo Okita, Pascal Mettes (superpixels)
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

cmake_minimum_required(VERSION 2.6)
set(PROJECTNAME slic)
project(${PROJECTNAME})

# lambda expressions require GCC version 4.7+ and this option
add_definitions(-std=c++11)

find_package(YARP)
find_package(ICUBcontrib)

list(APPEND CMAKE_MODULE_PATH ${YARP_MODULE_PATH})
list(APPEND CMAKE_MODULE_PATH ${ICUBCONTRIB_MODULE_PATH})

find_package(OpenCV)

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
                    ${TBB_INCLUDE_DIRS})

add_executable(${PROJECTNAME} ${source} ${header})

target_link_libraries(${PROJECTNAME} ${YARP_LIBRARIES} ${OpenCV_LIBRARIES} ${TBB_LIBRARIES})

install(TARGETS ${PROJECTNAME} DESTINATION bin)

#icubcontrib_finalize_export (${PROJECT_NAME})
#icubcontrib_add_uninstall_target()

# Copyright: 2012 iCub Facility, Istituto Italiano di Tecnologia
# Author: Vadim Tikhanoff
# CopyPolicy: Released under the terms of the GNU GPL v2.0.
#

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
SET(PROJECTNAME activityInterface)
PROJECT(${PROJECTNAME})

find_package(YARP)
find_package(ICUB)
find_package(ICUBcontrib)

list(APPEND CMAKE_MODULE_PATH ${ICUB_MODULE_PATH})
list(APPEND CMAKE_MODULE_PATH ${ICUBCONTRIB_MODULE_PATH})

find_package(OpenCV)

include(ICUBcontribHelpers)
include(ICUBcontribOptions)
icubcontrib_set_default_prefix()

find_package(Threads REQUIRED)

yarp_idl_to_dir(${PROJECTNAME}.thrift ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB source src/*.cpp)
file(GLOB header include/*.h)
set(idl ${PROJECTNAME}.thrift)

source_group("Source Files" FILES ${source})
source_group("Header Files" FILES ${header})
source_group("IDL Files" FILES ${idl})

# removed ${activeSegmentation_INCLUDE_DIRS}, we use IOL tracking. https://github.com/robotology/poeticon/issues/194
include_directories(${PROJECT_SOURCE_DIR}/include
                    ${ICUB_INCLUDE_DIRS}
                    ${OpenCV_INCLUDE_DIRS})

add_executable(${PROJECTNAME} ${source} ${header} ${idl} )

# removed activeSegmentation, we use IOL tracking. https://github.com/robotology/poeticon/issues/194
target_link_libraries(${PROJECTNAME} ${YARP_LIBRARIES} ${OpenCV_LIBRARIES} iKin ${CMAKE_THREAD_LIBS_INIT})

install(TARGETS ${PROJECTNAME} DESTINATION bin)

#icubcontrib_finalize_export (${PROJECT_NAME})
#icubcontrib_add_uninstall_target()

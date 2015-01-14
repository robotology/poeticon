# Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

cmake_minimum_required(VERSION 2.6)

set(PROJECTNAME poeticon)
project(${PROJECTNAME})

find_package(YARP)
find_package(ICUBcontrib)
list(APPEND CMAKE_MODULE_PATH ${YARP_MODULE_PATH}
                              ${ICUBCONTRIB_MODULE_PATH})

include(YarpInstallationHelpers)
include(ICUBcontribHelpers)
include(ICUBcontribOptions)
icubcontrib_set_default_prefix()

# modules that do not need OpenCV
add_subdirectory(opc2prada)
add_subdirectory(pradaPlanner)
add_subdirectory(worldStateManager)

# modules that need OpenCV
if(ICUB_USE_OpenCV)
    add_subdirectory(activeSegmentation)
    add_subdirectory(blobSpotter)
    add_subdirectory(blobDescriptor)
    #add_subdirectory(objectDescriptor)
    add_subdirectory(sequentialLabeller)
    #add_subdirectory(slic)
else(ICUB_USE_OpenCV)
    message(STATUS "OpenCV not found or selected, skipping vision modules")
endif(ICUB_USE_OpenCV)

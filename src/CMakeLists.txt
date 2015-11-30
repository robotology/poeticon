# Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

cmake_minimum_required(VERSION 2.6)

set(PROJECTNAME poeticon)
project(${PROJECTNAME})

find_package(YARP)
find_package(ICUBcontrib)
find_package(OpenCV 2.4)
list(APPEND CMAKE_MODULE_PATH ${YARP_MODULE_PATH}
                              ${ICUBCONTRIB_MODULE_PATH})

include(YarpInstallationHelpers)
include(ICUBcontribHelpers)
include(ICUBcontribOptions)
icubcontrib_set_default_prefix()

# look for libraries needed by certain modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../conf") # TODO: fix this hack
find_package(TBB)

# modules that do not need OpenCV
add_subdirectory(opc2prada)
add_subdirectory(pradaPlanner-cpp)
add_subdirectory(worldStateManager)

if(WIN32)
    message(STATUS "Platform is WIN32, compiling praxiconInterface")
    add_subdirectory(praxiconInterface)
else(WIN32)
    message(STATUS "Platform is not WIN32, skipping praxiconInterface")
endif(WIN32)

# modules that need OpenCV
if(OpenCV_FOUND)
    message(STATUS "OpenCV found, compiling vision modules")
    add_subdirectory(activeSegmentation)
    add_subdirectory(activityInterface)
    add_subdirectory(affExploreManager)
    add_subdirectory(blobDescriptor)
    #add_subdirectory(blobDescriptor2)
    add_subdirectory(perspectiveChanger)
    add_subdirectory(sequentialLabeller)
    if(TBB_FOUND)
        message(STATUS "TBB found, compiling SLIC Superpixels")
        add_subdirectory(slic)
    else(TBB_FOUND)
        message(STATUS "TBB not found, skipping SLIC Superpixels")
    endif(TBB_FOUND)
    add_subdirectory(worldStateVisualizer)
else(OpenCV_FOUND)
    message(STATUS "OpenCV not found or selected, skipping vision modules")
endif(OpenCV_FOUND)

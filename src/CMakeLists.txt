# Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

cmake_minimum_required(VERSION 2.6)

option(BUILD_simulation "Build poeticon-simulation" OFF)

project(poeticon)

find_package(YARP)
set(YARP_REQUIRED_VERSION 3.1.0)
find_package(ICUBcontrib)
find_package(OpenCV)
list(APPEND CMAKE_MODULE_PATH ${ICUBCONTRIB_MODULE_PATH})

include(ICUBcontribHelpers)
include(ICUBcontribOptions)
icubcontrib_set_default_prefix()

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
    #add_subdirectory(activeSegmentation)
    add_subdirectory(activityInterface)
    add_subdirectory(affExploreManager)
    add_subdirectory(blobDescriptor)
    #add_subdirectory(perspectiveChanger)
    add_subdirectory(sequentialLabeller)
    add_subdirectory(worldStateVisualizer)
else(OpenCV_FOUND)
    message(STATUS "OpenCV not found or selected, skipping vision modules")
endif(OpenCV_FOUND)

# simulated symbolic reasoner modules
if (BUILD_simulation)
    message(STATUS "Compiling simulated symbolic reasoner")
    add_subdirectory(poeticon-simulation)
else(BUILD_simulation)
    message(STATUS "Skipping simulated symbolic reasoner")
endif(BUILD_simulation)

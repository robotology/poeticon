# Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
# Copyright: (C) 2014 VisLab, Institute for Systems and Robotics,
#                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
# Authors: Alexandre Antunes <aleksander88@gmail.com>
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

cmake_minimum_required(VERSION 2.6)

find_package(YARP)
find_package(ICUB)

add_subdirectory(affordanceCommunication)
add_subdirectory(geometricGrounding)
add_subdirectory(goalCompiler)
add_subdirectory(planningCycle)

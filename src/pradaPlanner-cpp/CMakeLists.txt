cmake_minimum_required(VERSION 2.6)

find_package(YARP)
find_package(ICUB)

add_subdirectory(geometricGrounding)
add_subdirectory(goalCompiler)
add_subdirectory(planningCycle)
add_subdirectory(affordanceCommunication)

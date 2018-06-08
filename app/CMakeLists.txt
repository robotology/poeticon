# Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

cmake_minimum_required(VERSION 2.6)

set(APPNAME poeticon)

file(GLOB conf ${CMAKE_CURRENT_SOURCE_DIR}/conf/*.ini)
file(GLOB scripts ${CMAKE_CURRENT_SOURCE_DIR}/scripts/*.template)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/conf/planner.exe")
  set(prada_FOUND ON)
  message(STATUS "libPRADA found")
else()
  set(prada_FOUND OFF)
  message(WARNING "libPRADA not found")
endif()

if(prada_FOUND)
  file(GLOB prada ${CMAKE_CURRENT_SOURCE_DIR}/conf/planner.exe)
  file(GLOB pradaFiles ${CMAKE_CURRENT_SOURCE_DIR}/conf/config ${CMAKE_CURRENT_SOURCE_DIR}/conf/*.dat)
endif()

set(lua_fsm ${CMAKE_CURRENT_SOURCE_DIR}/lua/poeticon_funcs.lua
            ${CMAKE_CURRENT_SOURCE_DIR}/lua/poeticon_interact_fsm.lua
            ${CMAKE_CURRENT_SOURCE_DIR}/lua/poeticon_root_fsm.lua)
set(lua_bin ${CMAKE_CURRENT_SOURCE_DIR}/lua/poeticon_main.lua)

find_package(YARP)
find_package(ICUB)
find_package(ICUBcontrib)
list(APPEND CMAKE_MODULE_PATH ${YARP_MODULE_PATH} ${ICUB_MODULE_PATH} ${ICUBCONTRIB_MODULE_PATH})
include(YarpInstallationHelpers)
include(ICUBcontribOptions)
include(ICUBcontribHelpers)
icubcontrib_set_default_prefix()

yarp_install(FILES ${conf} DESTINATION ${ICUBCONTRIB_CONTEXTS_INSTALL_DIR}/${APPNAME})
yarp_install(FILES ${scripts} DESTINATION ${ICUBCONTRIB_APPLICATIONS_TEMPLATES_INSTALL_DIR})

if(prada_FOUND)
  yarp_install(PROGRAMS ${prada} DESTINATION ${ICUBCONTRIB_CONTEXTS_INSTALL_DIR}/${APPNAME})
  yarp_install(FILES ${pradaFiles} DESTINATION ${ICUBCONTRIB_CONTEXTS_INSTALL_DIR}/${APPNAME})
endif()

yarp_install(PROGRAMS ${lua_fsm} DESTINATION ${ICUBCONTRIB_CONTEXTS_INSTALL_DIR}/${APPNAME}/lua)

# We cannot go for "yarp_install(PROGRAMS ${lua_bin} DESTINATION bin)", since on Windows, only
# at build time (i.e. not during installation), we have to handle the Debug|Release subfolders.
# Requirements: the cmake "-E copy" command does not change file permissions, hence the file needs
# to be stored in the versioning database with *executable* rights.
add_custom_target(copy_lua_in_build ALL)
add_custom_command(TARGET copy_lua_in_build POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy ${lua_bin} ${CMAKE_BINARY_DIR}/bin/${CMAKE_CFG_INTDIR}
                   COMMENT "Copying ${lua_bin} to ${CMAKE_BINARY_DIR}/bin/${CMAKE_CFG_INTDIR}/")
install(PROGRAMS ${lua_bin} DESTINATION bin)

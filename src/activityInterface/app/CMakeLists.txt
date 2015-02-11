file(GLOB conf ${CMAKE_CURRENT_SOURCE_DIR}/conf/*.ini)

yarp_install( FILES ${conf}
              DESTINATION ${ICUBCONTRIB_CONTEXTS_INSTALL_DIR}/${PROJECTNAME} )
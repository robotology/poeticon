# Copyright: (C) 2013 VisLab - Institute for Systems and Robotics - Istituto Superior Técnico
# Authors: Afonso Gonçalves , Lorenzo Jamone
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

set(appname ${PROJECTNAME})

file(GLOB conf ${CMAKE_CURRENT_SOURCE_DIR}/conf/*.ini)
file(GLOB scripts ${CMAKE_CURRENT_SOURCE_DIR}/scripts/*.template)

yarp_install( FILES ${conf}
              DESTINATION ${ICUBCONTRIB_CONTEXTS_INSTALL_DIR}/${appname}
            )

yarp_install( FILES ${scripts}
              DESTINATION ${ICUBCONTRIB_APPLICATIONS_TEMPLATES_INSTALL_DIR}
	    )

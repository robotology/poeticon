/*
 * Copyright (C) 2011 Department of Robotics Brain and Cognitive Sciences - Istituto Italiano di Tecnologia
 * Author: Vadim Tikhanoff
 * email:  vadim.tikhanoff@iit.it
 * Permission is granted to copy, distribute, and/or modify this program
 * under the terms of the GNU General Public License, version 2 or any
 * later version published by the Free Software Foundation.
 *
 * A copy of the license can be found at
 * http://www.robotcub.org/icub/license/gpl.txt
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details
 */

/**
 \defgroup activeParticleTrack Module that employs the activeSegmentation library and particle filtering
 @ingroup icub_activeSegmentation

 \section intro_sec Description
 A module that segments and tracks objects in a scene. It uses the
 activeSegmentation library and particle filter for tracking.
 The module can handle multi-tracks and handle object "mutation", in the sense that
 it tracks parts of the same object if the object is divided.
 
 It loads:
 \section lib_sec Libraries
 - YARP.
 - ctrlLib.
 - activeSegmentation
 
 \section portsc_sec Ports Created

 <b>Input ports</b>

 Note that the name of this port mirrors whatever is provided by the \c --name parameter value

 - \c /activeParticleTrack/image:i \n
 Receives an input from a camera eg: /icub/camcalib/left/out

 - \c /activeParticleTrack/fixation:i \n
  Receives an input a port containing x and y fixation point. 
  Eg: if yarpview is started in the following way, fixation point is what is clicked in the yarpview.
 yarpview --name /tracker --out /desiredTrack --RefreshTime 33

 <b>Output ports</b>

 - \c /activeParticleTrack/image:o \n
 Sends an image with tracking bounding boxes.

 - \c /activeParticleTrack/imageCrop:o\n
Sends an image with the generated segmentation

 - \c /activeParticleTrack/imageTpl:o \n
 Sends an image with the generated segmentation template

 - \c /activeParticleTrack/target:o \n
 Sends an Bottle lists containing info on the tracking process:
 trackIndex + cog.x + cog.y + boundingBox.topLeft.x + boundingBox.topLeft.y + boundingBox.BottomRight.x + boundingBox.BottomRight.y + group
 eg: (0.0 180.0 116.0 167.0 102.0 193.0 130.0 -1)

 <b>Port types </b>

 \section portsif_sec Ports Interface
 The interface to this module is implemented through
 \ref activeParticleTrack_IDLServer . \n

 \section tested_os_sec Tested OS
 Windows, Linux, MacOS

 \author Vadim Tikhanoff
 */

#include <iCub/activeParticle.h>

using namespace yarp::os;

int main(int argc, char * argv[])
{
    /* initialize yarp network */
    Network::init();

    /* create the module */
    TRACKERModule module;

    /* prepare and configure the resource finder */
    ResourceFinder rf;
    rf.setVerbose( true );
    rf.setDefaultContext( "activeSegmentation" );
    rf.setDefault("name","activeParticleTrack");
    rf.configure( argc, argv );

    /* run the module: runModule() calls configure first and, if successful, it then runs */
    module.runModule(rf);
    Network::fini();

    return 0;
}
//empty line to make gcc happy

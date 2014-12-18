/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2014 VisLab, Institute for Systems and Robotics,
 *                Istituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Giovanni Saponaro <gsaponaro@isr.ist.utl.pt>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 */

/**
 This module ensures that the robot has a coherent and up-to-date knowledge of
 the world state (objects, their labels and properties).

 Specifically, this module:
 1. initializes a multi-object tracker with fixation coordinates, one per
    object, previously computed from segmented blob silhouettes
 2. populates an ObjectsPropertiesCollector database
 3. maintains the database up-to-date when changes in the world are perceived
 */

#include <yarp/os/Log.h>
#include <yarp/os/Network.h>
#include "WorldStateMgrModule.h"

int main(int argc, char *argv[])
{
    Network yarp;

    ResourceFinder rf;
    rf.setVerbose(false);
    rf.setDefaultContext("poeticon");    // overridden by --context
    rf.setDefaultConfigFile("wsm.ini");  // overridden by --from
    rf.configure(argc, argv);

    if(rf.check("help"))
    {
        yInfo("Available options:");
        yInfo("--name prefix (default wsm)");
        yInfo("--playback <file> (to read data from a file)");
        return 0; // EXIT_SUCCESS
    }

    if(! yarp.checkNetwork() )
    {
        yError("YARP server not available!");
        return -1; // EXIT_FAILURE
    }

    WorldStateMgrModule mod;
    return mod.runModule(rf);
}

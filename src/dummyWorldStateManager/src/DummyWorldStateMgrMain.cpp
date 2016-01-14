/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2016 VisLab, Institute for Systems and Robotics,
 *                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Giovanni Saponaro <gsaponaro@isr.ist.utl.pt>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 */

#include <yarp/os/Log.h>
#include <yarp/os/Network.h>

#include "DummyWorldStateMgrModule.h"

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
        yInfo("--playback filename (read dummy world state data from a file, for example dbdummy.ini)");
        return 0; // EXIT_SUCCESS
    }

    if(! yarp::os::Network::checkNetwork() )
    {
        yError("YARP server not available!");
        return 1; // EXIT_FAILURE
    }

    DummyWorldStateMgrModule mod;
    return mod.runModule(rf);
}

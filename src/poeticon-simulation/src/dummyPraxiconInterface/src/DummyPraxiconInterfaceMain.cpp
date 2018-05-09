/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2018 VisLab, Institute for Systems and Robotics,
 *                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Giovanni Saponaro <gsaponaro@isr.tecnico.ulisboa.pt>
 * CopyPolicy: Released under the terms of the GNU GPL v3.0
 *
 * Based on praxiconInterface by Vadim Tikhanoff
 *
 */

#include <yarp/os/Log.h>
#include <yarp/os/Network.h>

#include "DummyPraxiconInterfaceModule.h"

using namespace yarp::os;

int main(int argc, char *argv[])
{
    Network yarp;

    ResourceFinder rf;
    rf.setVerbose(false);
    rf.setDefaultContext("poeticon");    // overridden by --context
    rf.setDefaultConfigFile("dummyPraxiconInterface.ini");  // overridden by --from
    rf.configure(argc, argv);

    if(! yarp::os::Network::checkNetwork() )
    {
        yError("YARP server not available!");
        return 1; // EXIT_FAILURE
    }

    DummyPraxiconInterfaceModule mod;
    return mod.runModule(rf);
}

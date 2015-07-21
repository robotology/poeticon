/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2015 VisLab, Institute for Systems and Robotics,
 *                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Alexandre Antunes <aleksander88@gmail.com>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 */

#include <yarp/os/Network.h>
#include "affordanceCommunicationModule.h"

int main(int argc, char *argv[])
{
    Network yarp;

    ResourceFinder rf;
    rf.setVerbose(true);
    rf.setDefaultContext("poeticon");    // overridden by --context
    rf.setDefaultConfigFile("affordanceCommunication.ini");  // overridden by --from
    rf.configure(argc, argv);


    if(! yarp.checkNetwork() )
    {
        cout << "Error: yarp server does not seem available" << endl;
        return 1; // EXIT_FAILURE
    }

    affComm module;
    return module.runModule(rf);
    
/*    if (!module.configure(rf))
    {
        cout << "something went wrong with the module configuration" << endl;
        return -1;
    }

    module.openPorts();

    if (!module.affordancesCycle())
    {
        cout << "something went wrong with the module execution" << endl;
        return -1;
    }
    return 0;*/
}

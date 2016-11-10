/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2015 VisLab, Institute for Systems and Robotics,
 *                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Alexandre Antunes <aleksander88@gmail.com>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 */

#include <yarp/os/Network.h>
#include "goalCompilerModule.h"

int main(int argc, char *argv[])
{
    Network yarp;

    ResourceFinder rf;
    rf.setVerbose(true);
    rf.setDefaultContext("poeticon");    // overridden by --context
    rf.setDefaultConfigFile("goalCompiler.ini");  // overridden by --from
    rf.configure(argc, argv);

    if(rf.check("help"))
    {
        yInfo("Options available:");
        yInfo(" --goalSimplification <on|off> (remove hand-specific goals, default: on)");
        yInfo(" --prerules <filename> (pre-rules file, default: pre_rules.dat)");
        return 0; // EXIT_SUCCESS
    }

    if(! yarp.checkNetwork() )
    {
        yError("yarp server does not seem available");
        return 1; // EXIT_FAILURE
    }

    goalCompiler module;
    return module.runModule(rf);

}

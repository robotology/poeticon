/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2018 VisLab, Institute for Systems and Robotics,
 *                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Giovanni Saponaro <gsaponaro@isr.tecnico.ulisboa.pt>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 * Based on activityInterface by Vadim Tikhanoff
 *
 */

#include <yarp/os/Log.h>
#include <yarp/os/Network.h>

#include "Defaults.h"
#include "DummyActivityInterfaceModule.h"

using namespace yarp::os;

int main(int argc, char *argv[])
{
    Network yarp;

    ResourceFinder rf;
    rf.setVerbose(true);
    rf.setDefaultContext(DefContext.c_str());    // overridden by --context
    rf.setDefaultConfigFile(DefConfigFile.c_str());  // overridden by --from
    rf.configure(argc, argv);

    if(rf.check("help"))
    {
        const double LowerBound = 0.0;
        const double UpperBound = 1.0;

        yInfo("Basic options:");
        yInfo("--context <context directory name> (default %s)", DefContext.c_str());
        yInfo("--from <ini file name> (default %s)", DefConfigFile.c_str());
        yInfo("--name <port prefix> (default %s)", DefModuleName.c_str());
        yInfo("--period <milliseconds> (default %d)", DefThreadPeriod);
        yInfo(" ");
        yInfo("Noise options (the probability of success ranges from %f to %f)", LowerBound, UpperBound);
        yInfo("--probability_grasp_tool_left <probability> (default: %f)", DefProbabilityGraspToolLeft);
        yInfo("--probability_grasp_tool_right <probability> (default: %f)", DefProbabilityGraspToolRight);
        yInfo("--probability_perceive_grasp <probability> (default: %f)", DefProbabilityPerceiveGrasp);
        yInfo("--probability_pull <probability> (default: %f)", DefProbabilityPull);
        yInfo("--probability_push <probability> (default: %f)", DefProbabilityPush);
        yInfo("--probability_put_left <probability> (default: %f)", DefProbabilityPutLeft);
        yInfo("--probability_put_right <probability> (default: %f)", DefProbabilityPutRight);
        yInfo("--probability_take_left <probability> (default: %f)", DefProbabilityTakeLeft);
        yInfo("--probability_take_right <probability> (default: %f)", DefProbabilityTakeRight);
        yInfo("--probability_vision_object <probability> (default: %f)", DefProbabilityVisionObject);
        yInfo(" ");
        yInfo("Geometric thresholds:");
        yInfo("--reachable_threshold_x <meters> (default: %f)", DefReachableThresholdX);
        yInfo("--reachable_threshold_y_left <meters> (default: %f)", DefReachableThresholdYLeft);
        yInfo("--reachable_threshold_y_right <meters> (default: %f)", DefReachableThresholdYRight);
        return 0; // EXIT_SUCCESS
    }

    if(! yarp::os::Network::checkNetwork() )
    {
        yError("YARP server not available!");
        return 1; // EXIT_FAILURE
    }

    DummyActivityInterfaceModule mod;
    return mod.runModule(rf);
}

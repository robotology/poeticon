#include <yarp/os/Network.h>
#include "PlannerModule.h"

int main(int argc, char *argv[])
{
    Network yarp;

    ResourceFinder rf;
    rf.setVerbose(false);
    rf.setDefaultContext("poeticon");    // overridden by --context
    rf.setDefaultConfigFile("planner.ini");  // overridden by --from
    rf.configure(argc, argv);



    if(! yarp.checkNetwork() )
    {
        yError("YARP server not available!");
        return -1; // EXIT_FAILURE
    }

    PlannerModule module;
    module.runModule(rf);
    return 0;
}

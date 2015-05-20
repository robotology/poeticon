#include <yarp/os/Network.h>
#include "geometricGroundingModule.h"

int main(int argc, char *argv[])
{
    Network yarp;

    ResourceFinder rf;
    rf.setVerbose(true);
    rf.setDefaultContext("poeticon");    // overridden by --context
    rf.setDefaultConfigFile("geometricGrounding.ini");  // overridden by --from
    rf.configure(argc, argv);


    if(! yarp.checkNetwork() )
    {
        cout << "Error: yarp server does not seem available" << endl;
        return -1; // EXIT_FAILURE
    }

    geoGround module;
    return module.runModule(rf);

/*    module.configure(rf);
    module.openFiles();
    module.openPorts();
    
    while (true)
    {
        if (module.plannerCommand() == "update")
        {
            if (!module.loadObjs())
            {
                cout << "failed to load objects" << endl;
                return -1;
            }
            if (!module.loadPreRules())
            {
                cout << "failed to load pre-rules" << endl;
                return -1;
            }
            module.createRulesList();
            if (!module.getAffordances())
            {
                cout << "failed to get affordances" << endl;
                return -1;
            }
            if (!module.createSymbolList())
            {
                cout << "failed to create a symbol list" << endl;
                return -1;
            }
            if (!module.writeFiles())
            {
                cout << "failed to write to files" << endl;
                return -1;
            }
            if (!module.plannerReply())
            {
                cout << "failed to communicate with planner" << endl;
                return -1;
            }
        }
        Time::delay(5);
    }
    module.close();
    return 0;*/
}

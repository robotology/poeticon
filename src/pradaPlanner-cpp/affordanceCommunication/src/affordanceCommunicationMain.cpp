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
        return -1; // EXIT_FAILURE
    }

    affComm module;

    
    if (!module.configure(rf))
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

    return 0;
}

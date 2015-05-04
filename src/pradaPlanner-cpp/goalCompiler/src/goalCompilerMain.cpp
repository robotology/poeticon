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


    if(! yarp.checkNetwork() )
    {
        cout << "Error: yarp server does not seem available" << endl;
        return -1; // EXIT_FAILURE
    }

    goalCompiler module;

    module.configure(rf);
    module.openFiles();
    module.openPorts();
    string command;    

    while (true)
    {
        command = "";
        command = module.plannerCommand();
        if (command == "praxicon")
        {
            if (!module.receiveInstructions())
            {
                cout << "failed to receive instructions" << endl;
                continue;
            }
        }
        else if (command == "update")
        {
            if (!module.loadObjs())
            {
                cout << "failed to load objects" << endl;
                return -1;
            }
            if (!module.loadRules())
            {
                cout << "failed to load rules" << endl;
                return -1;
            }
            if (!module.loadInstructions())
            {
                cout << "failed to load instructions" << endl;
                return -1;
            }
            if (!module.processFirstInst())
            {
                cout << "failed to process the first instruction" << endl;
                return -1;
            }
            if (!module.compile())
            {
                cout << "failed to compile goals" << endl;
                return -1;
            }
            if (!module.translate())
            {
                cout << "failed to translate goals" << endl;
                return -1;
            }
            if (!module.writeFiles())
            {
                cout << "failed to write files" << endl;
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
    return 0;
}







#ifndef __GOALCOMPILER_MODULE_H__
#define __GOALCOMPILER_MODULE_H__

#include <yarp/os/Network.h>
#include <yarp/os/RFModule.h>
#include <yarp/os/Bottle.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/os/Mutex.h>
#include <yarp/os/RateThread.h>
#include <yarp/os/RpcClient.h>
#include <yarp/os/Time.h>
#include <yarp/os/Vocab.h>
//#include <cstdio>
//#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>
#include <vector>
#include <algorithm>

#include "Helpers.h"



using namespace std;
using namespace yarp::os;


class goalCompiler : public RFModule
{
    private:
        // module parameters
        bool closing;
 
        string moduleName;
        string PathName;

        vector<vector<string> > instructions;
        vector<vector<string> > translat;
        vector<vector<string> > subgoals;
        vector<string> actions;
        vector<string> object_list;

        string subgoalFileName;
        string preRuleFileName;
        string goalFileName;
        string objIDsFileName;

        BufferedPort<Bottle> plannerPort;
        BufferedPort<Bottle> praxiconPort;

        Bottle *plannerBottle;
        Bottle *praxiconBottle;

        ifstream objectFile;
        ifstream preRuleFile;
        ofstream goalFile;
        ofstream subgoalFile;

    public:
        virtual bool configure(ResourceFinder &rf);
        virtual bool close();
        virtual bool updateModule();
        virtual bool interrupt();

        
        void openFiles();
        void openPorts();
        bool loadObjs();
        string plannerCommand();
        bool plannerReply();
        bool loadRules();
        bool receiveInstructions();
        bool loadInstructions();
        bool processFirstInst();
        bool compile();
        bool translate();
        bool writeFiles();
        
        
};

#endif

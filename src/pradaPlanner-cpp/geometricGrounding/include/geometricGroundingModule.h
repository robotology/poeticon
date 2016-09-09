/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2015 VisLab, Institute for Systems and Robotics,
 *                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Alexandre Antunes <aleksander88@gmail.com>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 */

#ifndef __GEOMETRICGROUNDING_MODULE_H__
#define __GEOMETRICGROUNDING_MODULE_H__

#include <yarp/os/Network.h>
#include <yarp/os/RFModule.h>
#include <yarp/os/Bottle.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/os/Mutex.h>
#include <yarp/os/RateThread.h>
#include <yarp/os/RpcClient.h>
#include <yarp/os/Time.h>
#include <yarp/os/Vocab.h>
#include <yarp/os/Log.h>
#include <fstream>
#include <sstream>
#include <string.h>
#include <vector>
#include <algorithm>
#include <string>

#include "Helpers.h"

using namespace std;
using namespace yarp::os;

class geoGround : public RFModule
{
    private:
        // module parameters
 
        string moduleName;
        string PathName;
        vector<string> objects;
        vector<string> tools;
        vector<string> rules;
        vector<string> prerules;
        vector<string> new_rule;
        vector<string> new_symbols;

        string presymbolFileName;
        string preruleFileName;
        string ruleFileName;
        string symbolFileName;
        string objectsFileName;

        BufferedPort<Bottle> plannerPort;
        BufferedPort<Bottle> affordancePort;

        RpcClient objectQueryPort;

        Bottle cmd;
        Bottle reply;

        Bottle *plannerBottle;
        Bottle *AffBottle;

        ifstream objectFile;
        ifstream preruleFile;
        ifstream presymbolFile;
        ofstream ruleFile;
        ofstream symbolFile;

    public:
        virtual bool configure(ResourceFinder &rf);
        virtual bool close();
        virtual bool updateModule();
        virtual bool interrupt();

        bool groundingCycle();
        vector<string> create_rules(string pre_rule);
        vector<string> create_symbols(string symbols);
        void openFiles();
        void openPorts();
        bool loadObjs();
        string plannerCommand();
        bool plannerReply(string replyString);
        bool loadPreRules();
        bool createRulesList();
        bool getAffordances();
        bool createSymbolList();
        bool writeFiles();
};

#endif

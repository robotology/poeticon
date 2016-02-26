/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2015 VisLab, Institute for Systems and Robotics,
 *                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Alexandre Antunes <aleksander88@gmail.com>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 */

#ifndef __AFFORDANCECOMMUNICATION_MODULE_H__
#define __AFFORDANCECOMMUNICATION_MODULE_H__

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

class affComm : public RFModule
{
    private:
        // module parameters
 
        string moduleName;
        string PathName;

        vector<vector<string> > translation;
        vector<vector<string> > objects;
        vector<vector<string> > tools;

        vector<string> act;

        vector<vector<double> > posits;
        vector<vector<double> > descriptors;
        vector<vector<vector<double> > > tooldescriptors;

        string command;
        string rule;
        string context;
        string outcome;
        string outcome2;
        string outcome3;

        BufferedPort<Bottle> plannerPort;
        BufferedPort<Bottle> geoPort;
        BufferedPort<Bottle> affnetPort;

        RpcClient descQueryPort;
        RpcClient actionQueryPort;
		RpcClient objectQueryPort;

        Bottle *plannerBottle;
        Bottle *Affor_bottle_in;
        Bottle *affnet_bottle_in;

        Bottle cmd;
        Bottle reply;

    public:

        virtual bool configure(ResourceFinder &rf);
        virtual bool close();
        virtual bool updateModule();
        virtual bool interrupt();
        virtual double getPeriod();

        void openPorts();
        bool switchDisplayOff();
        bool loadObjs();
        bool plannerCommand();
        bool affordancesCycle();
        bool queryDescriptors();
        bool queryToolDescriptors();
        bool plannerQuery();
        bool updateAffordances();
        bool sendOutcomes();
        bool getGraspAff();
        bool getDropAff();
        bool getPutAff();
        bool getPushAff();
        bool getPullAff();
};

#endif

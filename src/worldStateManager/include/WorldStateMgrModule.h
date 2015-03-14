/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2014 VisLab, Institute for Systems and Robotics,
 *                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Giovanni Saponaro <gsaponaro@isr.ist.utl.pt>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 */

#ifndef __WSM_MODULE_H__
#define __WSM_MODULE_H__

#include <yarp/os/Bottle.h>
#include <yarp/os/LogStream.h>
#include <yarp/os/RFModule.h>
#include "WorldStateMgr_IDL.h"
#include "WorldStateMgrThread.h"

using namespace std;
using namespace yarp::os;

class WorldStateMgrModule : public RFModule, public WorldStateMgr_IDL
{
    private:
        // module parameters
        string moduleName;
        string handlerPortName;
        RpcServer handlerPort;
        bool closing;

        // pointer to a new thread
        WorldStateMgrThread *thread;

        // thread stuff
        double threadPeriod;
        bool playbackMode;
        string playbackFile;
        int countFrom;

    public:
        virtual bool configure(ResourceFinder &rf);
        virtual bool interruptModule();
        virtual bool close();
        virtual bool updateModule();
        virtual double getPeriod();

        // IDL functions
        bool attach(yarp::os::RpcServer &source);
        bool dump();
        bool update();
        bool pause(const string &objName);
        bool resume(const string &objName);
        Bottle getColorHist(const int32_t u, const int32_t v);
        bool quit();
};

#endif

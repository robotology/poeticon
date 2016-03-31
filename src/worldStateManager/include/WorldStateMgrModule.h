/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2016 VisLab, Institute for Systems and Robotics,
 *                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Giovanni Saponaro <gsaponaro@isr.ist.utl.pt>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 */

#ifndef WSM_MODULE_H
#define WSM_MODULE_H

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

    public:
        virtual bool configure(ResourceFinder &rf);
        virtual bool interruptModule();
        virtual bool close();
        virtual bool updateModule();
        virtual double getPeriod();

        // IDL functions
        bool init();
        bool isInitialized();
        bool attach(yarp::os::RpcServer &source);
        bool dump();
        bool update();
        bool reset();
        bool pause(const string &objName);
        bool resume(const string &objName);
        bool pauseID(const int32_t objID);
        bool resumeID(const int32_t objID);
        Bottle getColorHist(const int32_t u, const int32_t v);
        bool quit();
};

#endif

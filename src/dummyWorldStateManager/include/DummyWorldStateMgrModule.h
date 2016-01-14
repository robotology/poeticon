/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2016 VisLab, Institute for Systems and Robotics,
 *                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Giovanni Saponaro <gsaponaro@isr.ist.utl.pt>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 */

#ifndef DUMMY_WSM_MODULE_H
#define DUMMY_WSM_MODULE_H

#include <yarp/os/Bottle.h>
#include <yarp/os/LogStream.h>
#include <yarp/os/RFModule.h>

#include "DummyWorldStateMgr_IDL.h"
#include "DummyWorldStateMgrThread.h"

using namespace std;
using namespace yarp::os;

class DummyWorldStateMgrModule : public RFModule, public DummyWorldStateMgr_IDL
{
    private:
        // module parameters
        string moduleName;
        string handlerPortName;
        RpcServer handlerPort;
        bool closing;

        // pointer to a new thread
        DummyWorldStateMgrThread *thread;

        // thread stuff
        double threadPeriod;
        bool playbackMode;
        string playbackFile;

    public:
        virtual bool configure(ResourceFinder &rf);
        virtual bool interruptModule();
        virtual bool close();
        virtual bool updateModule();
        virtual double getPeriod();

        // IDL functions
        bool attach(yarp::os::RpcServer &source);
        bool update();
        bool quit();
};

#endif

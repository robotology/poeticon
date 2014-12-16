/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2014 VisLab, Institute for Systems and Robotics,
 *                Istituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Giovanni Saponaro <gsaponaro@isr.ist.utl.pt>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 */

#ifndef __WSM_MODULE_H__
#define __WSM_MODULE_H__

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

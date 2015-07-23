/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2015 VisLab, Institute for Systems and Robotics,
 *                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Giovanni Saponaro <gsaponaro@isr.ist.utl.pt>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 */

#ifndef __WSVIS_MODULE_H__
#define __WSVIS_MODULE_H__

#include <yarp/os/Bottle.h>
#include <yarp/os/Log.h>
#include <yarp/os/RFModule.h>

#include "WorldStateVisualizerThread.h"

using namespace std;
using namespace yarp::os;

class WorldStateVisualizerModule : public RFModule
{
    private:
        // module parameters
        string moduleName;
        bool closing;

        // pointer to a new thread
        WorldStateVisualizerThread *thread;

        // thread stuff
        double threadPeriod;

    public:
        virtual bool configure(ResourceFinder &rf);
        virtual bool interruptModule();
        virtual bool close();
        virtual bool updateModule();
        virtual double getPeriod();
};

#endif

/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2014 VisLab, Institute for Systems and Robotics,
 *                Istituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Giovanni Saponaro <gsaponaro@isr.ist.utl.pt>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 */

#ifndef	__WSM_MODULE_H__
#define __WSM_MODULE_H__

#include <sstream>
#include <yarp/os/BufferedPort.h>
#include <yarp/os/Log.h>
#include <yarp/os/RFModule.h>
#include <yarp/os/RpcClient.h>
#include <yarp/os/Time.h>
#include <yarp/os/Vocab.h>

#define STATE_WAIT_BLOBS   0
#define STATE_READ_BLOBS   1
#define STATE_INIT_TRACKER 2
#define STATE_WAIT_TRACKER 3
#define STATE_READ_TRACKER 4
#define STATE_POPULATE_DB  5
#define STATE_UPDATE_DB    6

using namespace std;
using namespace yarp::os;

class WorldStateMgrModule : public RFModule
{
    private:
        string moduleName;

        string inTargetsPortName;
        string inAffPortName;
        string outFixationPortName;
        string outStatePortName;
        string opcPortName;

        BufferedPort<Bottle> inTargetsPort;
        BufferedPort<Bottle> inAffPort;
        Port outFixationPort;
        RpcClient opcPort;

        Bottle *inAff;
        Bottle *inTargets;
        int state;

        int sizeTargets, sizeAff;

        bool populated;

    public:
        virtual bool configure(ResourceFinder &rf);
        virtual bool interruptModule();
        virtual bool close();
        virtual bool updateModule();
        virtual double getPeriod();

        // TODO: bool functions (with success checks) instead of void
        void doInitTracker();
        bool doPopulateDB();
        void refreshBlobs();
        void refreshTracker();
        void refreshAll();
        bool refreshAllAndValidate();
};

#endif

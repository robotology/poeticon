/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2016 VisLab, Institute for Systems and Robotics,
 *                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Giovanni Saponaro <gsaponaro@isr.ist.utl.pt>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 */

#ifndef DUMMY_WSM_THREAD_H
#define DUMMY_WSM_THREAD_H

#include <iomanip>
#include <iostream> // __func__
#include <string>

#include <yarp/os/Bottle.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/os/Log.h>
#include <yarp/os/LogStream.h>
#include <yarp/os/Port.h>
#include <yarp/os/Property.h>
#include <yarp/os/RateThread.h>
#include <yarp/os/RpcClient.h>
#include <yarp/os/Time.h>
#include <yarp/os/Vocab.h>

// playback mode states
#define STATE_DUMMY_PARSE    100
#define STATE_DUMMY_WAIT_OPC 101
#define STATE_DUMMY_WAIT_CMD 102
#define STATE_DUMMY_STEP     103
#define STATE_DUMMY_EOF      104
#define STATE_DUMMY_ERROR    105

// make sure __func__ is set correctly, http://stackoverflow.com/a/17528983
#if __STDC_VERSION__ < 199901L
# if __GNUC__ >= 2
#  define __func__ __FUNCTION__
# else
#  define __func__ "<unknown>"
# endif
#endif

using namespace std;
using namespace yarp::os;

class DummyWorldStateMgrThread : public RateThread
{
    private:
        string moduleName;
        string opcPortName;
        RpcClient opcPort;
        bool closing;

        int fsmState;
        bool toldUserOPCConnected;
        double t;
        bool toldUserEof;
        bool playbackPaused;
        int sizePlaybackFile;
        int currPlayback;

        string playbackFile;
        Bottle stateBottle;

    public:
        DummyWorldStateMgrThread(const string &_moduleName,
                                 const double _period);
        bool openPorts();
        void close();
        void interrupt();
        bool threadInit();
        void run();

        bool initVars();
        bool tellUserConnectOPC();
        bool tellUserOPCConnected();
        void setPlaybackFile(const string &_file);
        void fsmPlayback();

        // IDL functions
        bool updateWorldState();
};

#endif

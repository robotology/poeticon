/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2014 VisLab, Institute for Systems and Robotics,
 *                Istituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Giovanni Saponaro <gsaponaro@isr.ist.utl.pt>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 */

#ifndef __WSM_THREAD_H__
#define __WSM_THREAD_H__

#include <sstream>
#include <yarp/os/Bottle.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/os/Log.h>
#include <yarp/os/Port.h>
#include <yarp/os/RateThread.h>
#include <yarp/os/RpcClient.h>
#include <yarp/os/Time.h>
#include <yarp/os/Vocab.h>

// perception states
#define STATE_WAIT_BLOBS   0
#define STATE_READ_BLOBS   1
#define STATE_INIT_TRACKER 2
#define STATE_WAIT_TRACKER 3
#define STATE_READ_TRACKER 4
#define STATE_POPULATE_DB  5
#define STATE_UPDATE_DB    6

using namespace std;
using namespace yarp::os;

class WorldStateMgrThread : public RateThread
{
    private:
        string moduleName;
        string inTargetsPortName;
        string inAffPortName;
        string outFixationPortName;
        string opcPortName;
        string arePortName;
        BufferedPort<Bottle> inTargetsPort;
        BufferedPort<Bottle> inAffPort;
        Port outFixationPort;
        RpcClient opcPort;
        RpcClient arePort;
        bool closing;

        bool playbackMode;
        bool populated;
        int perceptionState;
        Bottle *inAff;
        Bottle *inTargets;
        int sizeTargets, sizeAff;

        // playback mode
        string playbackFile;

    public:
        WorldStateMgrThread(const string &_moduleName,
                            const double _period,
                            bool _playbackMode=false);
        bool openPorts();
        void close();
        void interrupt();
        bool threadInit();
        void run();
        
        // perception and playback modes
        bool updateWorldState();
        bool doPopulateDB();
        
        // perception mode
        bool initPerceptionVars();
        bool initTracker();
        void fsmPerception();
        void refreshBlobs();
        void refreshTracker();
        void refreshPerception();
        bool refreshPerceptionAndValidate();
        bool isHandFree(const string &handName);

        // playback mode
        bool setPlaybackFile(const string &_file);
        void fsmPlayback();
};

#endif

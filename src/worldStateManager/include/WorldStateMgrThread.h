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

#include <iomanip>
#include <iostream> // __func__
#include <sstream>
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

// perception states
#define STATE_WAIT_BLOBS   0
#define STATE_READ_BLOBS   1
#define STATE_INIT_TRACKER 2
#define STATE_WAIT_TRACKER 3
#define STATE_READ_TRACKER 4
#define STATE_POPULATE_DB  5
#define STATE_UPDATE_DB    6

// playback states
#define STATE_PARSE_FILE 100
#define STATE_STEP_FILE  101
#define STATE_END_FILE   102

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

class WorldStateMgrThread : public RateThread
{
    private:
        string moduleName;
        string inTargetsPortName;
        string inAffPortName;
        string outFixationPortName;
        string opcPortName;
        string geomIFPortName;
        string iolPortName;
        BufferedPort<Bottle> inTargetsPort;
        BufferedPort<Bottle> inAffPort;
        Port outFixationPort;
        RpcClient opcPort;
        RpcClient geomIFPort;
        RpcClient iolPort;
        bool closing;

        // perception and playback modes
        bool playbackMode;
        bool populated;

        // perception mode
        int perceptionFSMState;
        Bottle *inAff;
        Bottle *inTargets;
        int sizeTargets, sizeAff;

        // playback mode
        int playbackFSMState;
        string playbackFile;
        bool playbackPaused;
        Bottle findBottle;
        int sizePlaybackFile;
        int currPlayback;

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
        
        // perception mode
        bool initPerceptionVars();
        bool initTracker();
        void fsmPerception();
        void refreshBlobs();
        void refreshTracker();
        void refreshPerception();
        bool refreshPerceptionAndValidate();
        bool doPopulateDB();
        string getName(const int &id);
        vector<double> getTooltipOffset(const int &id);
        vector<int> isOnTopOf(const int &id);
        vector<int> getIdsToReach(const int &id);
        vector<int> getIdsToPull(const int &id);
        bool isHandFree(const string &handName);
        string inWhichHand(const string &objName);

        // playback mode
        bool initPlaybackVars();
        bool setPlaybackFile(const string &_file);
        void fsmPlayback();
};

#endif

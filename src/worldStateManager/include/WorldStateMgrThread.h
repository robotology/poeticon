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

#include <algorithm>
#include <iomanip>
#include <iostream> // __func__
#include <map>
#include <sstream>
#include <string>
#include <vector>
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

// perception mode states
#define STATE_PERCEPTION_WAIT_OPC     0
#define STATE_PERCEPTION_WAIT_BLOBS   1
#define STATE_PERCEPTION_READ_BLOBS   2
#define STATE_PERCEPTION_INIT_TRACKER 3
#define STATE_PERCEPTION_WAIT_TRACKER 4
#define STATE_PERCEPTION_READ_TRACKER 5
#define STATE_PERCEPTION_POPULATE_DB  6
#define STATE_PERCEPTION_WAIT_CMD     7
#define STATE_PERCEPTION_UPDATE_DB    8

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

typedef std::map<int,double> idsMap;

class WorldStateMgrThread : public RateThread
{
    private:
        string moduleName;
        string opcPortName;
        string inTargetsPortName;
        string inAffPortName;
        string outFixationPortName;
        string activityPortName;
        string trackerPortName;

        RpcClient opcPort;
        BufferedPort<Bottle> inTargetsPort;
        BufferedPort<Bottle> inAffPort;
        Port outFixationPort;
        RpcClient activityPort;
        RpcClient trackerPort;

        bool closing;

        // perception and playback modes
        bool playbackMode;
        int fsmState;
        bool populated;
        bool toldUserConnectOPC;

        // perception mode
        bool needUpdate;
        bool trackerInit;
        Bottle *inAff;
        Bottle *inTargets;
        int sizeTargets, sizeAff;
        std::vector<int> opcIDs;
        std::vector<int> trackIDs;
        idsMap ids;

        // playback mode
        string playbackFile;
        bool playbackPaused;
        bool toldUserEof;
        Bottle stateBottle;
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
        bool initCommonVars();
        bool dumpWorldState();
        bool updateWorldState();

        // perception mode
        bool initPerceptionVars();
        bool initTracker();
        void fsmPerception();
        void refreshOPCIDs();
        void refreshBlobs();
        void refreshTracker();
        void updateTrackIDsNoDupes();
        void refreshPerception();
        bool refreshPerceptionAndValidate();
        bool doPopulateDB();
        string getLabel(const double &u, const double &v);
        bool mono2stereo(const double &u, const double &v, double x, double y, double z);
        // TODO: return Bottle instead of std::vector
        vector<double> getTooltipOffset(const string &objName);
        vector<string> isUnderOf(const string &objName);
        vector<string> isReachableWith(const string &objName);
        vector<string> isPullableWith(const string &objName);
        bool isHandFree(const string &handName);
        string inWhichHand(const string &objName);

        // playback mode
        bool initPlaybackVars();
        bool setPlaybackFile(const string &_file);
        void fsmPlayback();
};

#endif

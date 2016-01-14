/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2014 VisLab, Institute for Systems and Robotics,
 *                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Giovanni Saponaro <gsaponaro@isr.ist.utl.pt>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 */

#ifndef __WSM_THREAD_H__
#define __WSM_THREAD_H__

#include <iomanip>
#include <iostream> // __func__
#include <map>
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
#include <yarp/sig/Vector.h>
#include <iCub/ctrl/filters.h>

#include "Helpers.h"
#include "MemoryItemHand.h"
#include "MemoryItemObj.h"

// perception mode states
#define STATE_PERCEPTION_WAIT_OPC         0
#define STATE_PERCEPTION_WAIT_BLOBS       1
#define STATE_PERCEPTION_READ_BLOBS       2
#define STATE_PERCEPTION_WAIT_TRACKER     3
#define STATE_PERCEPTION_INIT_TRACKER     4
#define STATE_PERCEPTION_READ_TRACKER     5
#define STATE_PERCEPTION_WAIT_ACTIVITYIF  6
#define STATE_PERCEPTION_SET_MEMORY       7
#define STATE_PERCEPTION_POPULATE_DB      8
#define STATE_PERCEPTION_WAIT_CMD         9
#define STATE_PERCEPTION_UPDATE_DB       10

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

typedef std::map<int,string> idLabelMap;

class WorldStateMgrThread : public RateThread
{
    private:
        string moduleName;
        string opcPortName;
        string inTargetsPortName;
        string inAffPortName;
        string inToolAffPortName;
        string activityPortName;
        string trackerPortName;

        RpcClient opcPort;
        BufferedPort<Bottle> inTargetsPort;
        BufferedPort<Bottle> inAffPort;
        BufferedPort<Bottle> inToolAffPort;
        RpcClient activityPort;
        RpcClient trackerPort;

        bool closing;

        // perception and playback modes
        bool playbackMode;
        int fsmState;
        double t;
        bool toldUserOPCConnected;
        int countFrom;
        bool withFilter;
        int filterOrder;
        bool initFinished;

        // perception mode
        bool needUpdate;
        bool toldUserBlobsConnected;
        bool toldUserTrackerConnected;
        bool toldActivityGoHome;
        bool toldUserActivityIFConnected;
        Bottle *inAff;
        Bottle *inToolAff;
        Bottle *inTargets;
        int sizeTargets, sizeAff;
        std::vector<MemoryItemHand> hands;
        std::vector<MemoryItemObj> objs;
        Bottle trackIDs;

        idLabelMap candidateTrackMap;
        std::vector<iCub::ctrl::MedianFilter> posFilter;

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
                            const bool _playbackMode,
                            const int _countFrom,
                            const bool _withFilter);
        bool openPorts();
        void close();
        void interrupt();
        bool threadInit();
        void run();

        // perception and playback modes
        bool initCommonVars();
        bool updateWorldState();
        bool tellUserConnectOPC();
        bool tellUserOPCConnected();
        bool opcContainsID(const int &id);
        bool checkOPCStatus(const int &minEntries, Bottle &ids);

        // perception mode
        bool initPerceptionVars();
        bool tellUserConnectBlobs();
        bool tellUserBlobsConnected();
        bool tellUserConnectTracker();
        bool tellUserConnectActivityIF();

        bool checkTrackerStatus();
        bool configureTracker();
        bool getTrackNames();
        bool initTracker();
        bool refreshBlobs();
        bool refreshTracker();
        bool resetWorldState();
        bool resetTracker();

        bool getAffBottleIndexFromTrackROI(const int &u, const int &v, int &abi);
        bool getTrackerBottleIndexFromID(const int &id, int &tbi);

        bool computeObjProperties(const int &id, const string &label,
                                  Bottle &pos2d,
                                  Bottle &desc2d, Bottle &tooldesc2d,
                                  string &inHand,
                                  Bottle &onTopOf,
                                  Bottle &reachW, Bottle &pullW);
        bool constructMemoryFromMap();
        bool constructMemoryFromOPCID(const int &opcID);
        bool initMemoryFromOPC();
        bool memoryContainsID(const int &id);
        bool memoryContainsName(const string &n);
        bool parseHandProperties(const Bottle *fields, bool &isFree);
        bool parseObjProperties (const Bottle *fields,
                                 Bottle &pos2d,
                                 Bottle &desc2d, Bottle &tooldesc2d,
                                 string &inHand, Bottle &onTopOf,
                                 Bottle &reachW, Bottle &pullW);
        bool printMemoryState();

        bool tellActivityGoHome();
        int label2id(const string &label);
        bool getLabel(const int &u, const int &v, string &label);
        bool getLabelMajorityVote(const int &u, const int &v, string &winnerLabel, const int &rounds=5);
        bool isOnTopOf(const string &objName, Bottle &objBelow);
        bool isReachableWith(const string &objName, Bottle &objReachable);
        bool isPullableWith(const string &objName, Bottle &objPullable);
        bool isHandFree(const string &handName);
        string inWhichHand(const string &objName);

        bool setFilterOrder(const int &n);

        bool doPopulateDB();
        void fsmPerception();

        // IDL functions
        bool isInitialized();
        bool pauseTrack(const string &objName);
        bool resumeTrack(const string &objName);
        bool pauseTrackID(const int32_t &objID);
        bool resumeTrackID(const int32_t &objID);
        Bottle getColorHistogram(const int32_t &u, const int32_t &v);

        // playback mode
        bool initPlaybackVars();
        void setPlaybackFile(const string &_file);
        void fsmPlayback();
};

#endif

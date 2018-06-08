/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2016 VisLab, Institute for Systems and Robotics,
 *                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Giovanni Saponaro <gsaponaro@isr.ist.utl.pt>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 */

#ifndef WSM_THREAD_H
#define WSM_THREAD_H

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
#include <yarp/os/ResourceFinder.h>
#include <yarp/os/RpcClient.h>
#include <yarp/os/Time.h>
#include <yarp/os/Vocab.h>
#include <yarp/sig/Vector.h>
#include <iCub/ctrl/filters.h>

#include "Helpers.h"
#include "MemoryItemHand.h"
#include "MemoryItemObj.h"

// perception mode states
#define STATE_WAIT_OPC         0
#define STATE_WAIT_ACTIVITYIF  1
#define STATE_WAIT_SHAPEDESC   2
#define STATE_READ_SHAPEDESC   3
#define STATE_SET_MEMORY       4
#define STATE_POPULATE_DB      5
#define STATE_WAIT_CMD         6
#define STATE_UPDATE_DB        7

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
        ResourceFinder rf;

        string opcPortName;
        string inAffPortName;
        string inToolAffPortName;
        string activityPortName;

        RpcClient opcPort;
        BufferedPort<Bottle> inAffPort;
        BufferedPort<Bottle> inToolAffPort;
        RpcClient activityPort;

        bool closing;

        int state;
        double t;
        bool toldUserOPCConnected;
        int countFrom;
        bool withFilter;
        int filterOrder;
        bool initFinished;
        bool needUpdate;
        bool toldUserBlobsConnected;
        bool toldActivityGoHome;
        bool toldUserActivityIFConnected;
        Bottle *inAff;
        Bottle *inToolAff;
        int sizeAff;
        std::vector<MemoryItemHand> hands;
        std::vector<MemoryItemObj> objs;

        idLabelMap candidateMap;
        std::vector<iCub::ctrl::MedianFilter> posFilter;

    public:
        WorldStateMgrThread(const string &_moduleName,
                            ResourceFinder &_rf);
        bool openPorts();
        void close();
        void interrupt();
        bool threadInit();
        void run();

        bool initVars();
        bool clearAll();
        bool tellUserConnectOPC();
        bool tellUserOPCConnected();
        bool tellUserConnectBlobs();
        bool tellUserBlobsConnected();
        bool tellUserConnectActivityIF();

        bool ensureOpcNumEntries(const int &minEntries);
        bool opcContainsID(const int &id);
        bool checkOPCStatus(const int &minEntries, Bottle &ids); // TODO: remove
        bool resetOPC();
        bool resetOPCHandFields(const int &handID);
        bool updateCountFrom();

        bool refreshBlobs();

        bool getAffBottleIndexFromTrackROI(const int &u, const int &v, int &abi);

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

        bool tellActivityGoHome();
        string id2label(const int &id);
        int label2id(const string &label);
        bool getLabelsFromActivityIF(Bottle &names);
        bool getLabel(const int &u, const int &v, string &label);
        bool getLabelMajorityVote(const int &u, const int &v, string &winnerLabel, const int &rounds=5);
        bool isOnTopOf(const string &objName, Bottle &objBelow);
        bool isReachableWith(const string &objName, Bottle &objReachable);
        bool isPullableWith(const string &objName, Bottle &objPullable);
        bool isHandFree(const string &handName);
        string inWhichHand(const string &objName);
        bool getVisibilityByActivityIF(const string &objName, bool &result, int extraTries=4);
        bool belongsToStack(const string &objName, bool &result);
        bool isInHand(const string &objName, bool &result);

        bool constructMapFromNames(const Bottle &names);

        bool setFilterOrder(const int &n);

        bool doPopulateDB();
        void fsm();

        // IDL functions
        bool printMemoryState();
        bool initWorldState();
        bool updateWorldState();
        bool resetWorldState();
        bool isInitialized();
};

#endif

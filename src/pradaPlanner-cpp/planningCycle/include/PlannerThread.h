/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2015 VisLab, Institute for Systems and Robotics,
 *                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Alexandre Antunes <aleksander88@gmail.com>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 */

#ifndef __PLANNER_THREAD_H__
#define __PLANNER_THREAD_H__

#include <iomanip>
#include <iostream> // __func__
#include <fstream>
#include <sstream>
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
#include <algorithm>

#include "Helpers.h"

using namespace std;
using namespace yarp::os;

class PlannerThread : public RateThread
{
    private:
        string PathName;
        string moduleName;
        string rulesFileName;
        string goalFileName;
        string configFileName;
        string subgoalFileName;
        string stateFileName;
        string pipeFileName;
        string symbolFileName;

        string process_string;

        string next_action;
        string prev_action;
        string obj;
        string act;
        string hand;

        ifstream rulesFile;
        ifstream goalFile;
        ifstream configFile;
        ifstream subgoalFile;
        ifstream stateFile;
        ifstream objFile;
        ifstream pipeFile;
        ifstream symbolFile;
        ofstream rulesFileOut;
        ofstream goalFileOut;
        ofstream configFileOut;
        ofstream subgoalsFileOut;
        ofstream stateFileOut;
        ofstream objFileOut;
        ofstream newstateFile;

        BufferedPort<Bottle> goal_yarp;
        BufferedPort<Bottle> geo_yarp;
        BufferedPort<Bottle> prax_yarp;
        BufferedPort<Bottle> aff_yarp;
        RpcClient world_rpc;
        RpcClient actInt_rpc;
        RpcClient opc2prada_rpc;

        bool restartPlan;
        bool resumePlan;
        bool startPlan;
        bool stopping;
        
        int plan_level;

        int positx;
        int posity;
        
        Bottle object_bottle;
        Bottle cmd;
        Bottle message;
        Bottle reply;
        Bottle *state_bottle_in;
        Bottle *geo_bottle_in;
        Bottle *aff_bottle_in;
        Bottle *goal_bottle_in;

        vector<string> toolhandle;
        vector<string> state;
        vector<string> old_state;
        vector<string> rules;
        vector<string> old_rules;
        vector<string> objects_used;
        vector<string> goal;
        vector<string> failed_goal;
        //vector<string> tool_list;

        vector<vector<string> > subgoals;
        vector<vector<string> > object_IDs;

    public:

        bool closing;

        PlannerThread(const string &_moduleName,
                            const double _period,
                            const string &_PathName);
        void openFiles();
        bool openPorts();
        virtual void close();
        virtual void interrupt();
        virtual bool threadInit();
        virtual void threadRelease();
        virtual void run();

        // module
        void stopPlanning();
        bool checkPause();
        bool completePlannerState();
        bool loadSubgoals();
        bool loadObjs();
		Bottle printObjs();
        bool loadState();
        bool loadGoal();
        bool preserveState();
        bool loadRules();
        bool preserveRules();
        bool adaptRules();
        bool planCompletion();
        bool checkHoldingSymbols();
        bool loadUsedObjs();
        bool checkFailure();
        bool compareState();
        bool IDisPresent(string ID, bool &result);

        // IDL functions
        bool startPlanning();
        bool pausePlanner();
        bool resumePlanner();
        bool updateState();
        bool groundRules();
        bool compileGoal();
        bool resetConfig();
        bool resetPlanVars();
        bool goalUpdate();
        int PRADA();
        bool increaseHorizon();
        bool jumpForward();
        bool jumpBack();
        bool resetRules();
        bool execAction();
        bool codeAction();
        bool checkGoalCompletion();
        bool plan_init();
        bool planning_cycle();
        string showPlannedAction();
        string showCurrentState();
        string showCurrentGoal();
        string printSymbol(string symbol);
        //Bottle getToolList();
};

#endif

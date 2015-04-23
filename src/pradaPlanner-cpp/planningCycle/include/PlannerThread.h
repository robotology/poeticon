

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

#include "Helpers.h"


// make sure __func__ is set correctly, http://stackoverflow.com/a/17528983
//#if __STDC_VERSION__ < 199901L
//# if __GNUC__ >= 2
//#  define __func__ __FUNCTION__
//# else
//#  define __func__ "<unknown>"
//# endif
//#endif

using namespace std;
using namespace yarp::os;

typedef std::map<int,string> idLabelMap;

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
        string objFileName;
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

        bool closing;
        bool restartPlan;
        bool resumePlan;
        bool startPlan;
        
        int plan_level;

        int positx;
        int posity;
        
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

        vector<vector<string> > subgoals;
        vector<vector<string> > object_IDs;
        
        

    public:

        PlannerThread(const string &_moduleName,
                            const double _period,
                            const string &_PathName);
        void openFiles();
        bool openPorts();
        void close();
        void interrupt();
        bool threadInit();
        void run();

        // Helpers
        // int find_element(vector<string> vect, string elem);
        // int vect_compare (vector<string> vect1, vector<string> vect2);
        // std::vector<std::string> split(const std::string &s, char delim);

        // module
        bool checkPause();
        bool completePlannerState();
        bool loadSubgoals();
        bool loadObjs();
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

};

#endif

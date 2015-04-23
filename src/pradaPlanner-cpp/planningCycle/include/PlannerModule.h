





#ifndef __PLANNER_MODULE_H__
#define __PLANNER_MODULE_H__

#include <yarp/os/Bottle.h>
#include <yarp/os/LogStream.h>
#include <yarp/os/RFModule.h>
#include "Planner_IDLserver.h"
#include "PlannerThread.h"


using namespace std;
using namespace yarp::os;


class PlannerModule : public RFModule, public Planner_IDLserver
{
    private:
        // module parameters
        string moduleName;
        string PathName;
        string handlerPortName;
        RpcServer handlerPort;
        bool closing;

        // pointer to a new thread
        PlannerThread *thread;

        // thread stuff
        double threadPeriod;

    public:
        virtual bool configure(ResourceFinder &rf);
        virtual bool interruptModule();
        virtual bool close();
        virtual bool updateModule();
        virtual double getPeriod();

        // IDL functions
        bool attach(yarp::os::RpcServer &source);
        bool pausePlanner();
        bool resumePlanner();
        bool goForward();
        bool goBack();
        bool updateState();
        bool updateGoals();
        bool plan();
        bool resetRules();
        bool resetConfig();
        bool resetLevel();
        bool ground();
        bool compileGoal();
        bool increaseHorizon();
        bool executePlannedAction();
        bool checkGoalCompleted();
        bool run1Step();
        bool startPlanning();
        string showPlannedAction();
        string showCurrentState();
        string showCurrentGoal();
        bool quit();
};

#endif

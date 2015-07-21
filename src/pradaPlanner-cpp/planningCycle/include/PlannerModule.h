/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2015 VisLab, Institute for Systems and Robotics,
 *                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Alexandre Antunes <aleksander88@gmail.com>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 */

#ifndef __PLANNER_MODULE_H__
#define __PLANNER_MODULE_H__

#include <yarp/os/Bottle.h>
#include <yarp/os/LogStream.h>
#include <yarp/os/RFModule.h>
#include "planningCycle_IDLserver.h"
#include "PlannerThread.h"

using namespace std;
using namespace yarp::os;

class PlannerModule : public RFModule, public planningCycle_IDLserver
{
    private:
        // module parameters
        string moduleName;
        string PathName;
        string handlerPortName;
        RpcServer handlerPort;

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
        bool stopPlanner();
        bool attach(yarp::os::RpcServer &source);
        bool pausePlanner();
        bool resumePlanner();
        bool goForward();
        bool goBack();
        bool updateState();
		bool loadObjects();
		Bottle printObjects();
        bool loadState();
        bool updateGoals();
        bool plan();
        bool resetRules();
        bool resetConfig();
        bool resetLevel();
        bool ground();
        bool compileGoal();
        bool increaseHorizon();
        bool executePlannedAction();
        string checkGoalCompleted();
        bool run1Step();
        bool startPlanner();
        string showPlannedAction();
        string showCurrentState();
        string showCurrentGoal();
        virtual bool quit();
};

#endif

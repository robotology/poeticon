/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2015 VisLab, Institute for Systems and Robotics,
 *                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Alexandre Antunes <aleksander88@gmail.com>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 */

#include "PlannerModule.h"

bool PlannerModule::configure(ResourceFinder &rf)
{
    // module parameters
    moduleName = rf.check("name", Value("planner")).asString();
    PathName = rf.findPath("contexts/"+rf.getContext());
    setName(moduleName.c_str());

    if (PathName==""){
        yError("path to contexts/%s not found", rf.getContext().c_str());
        return false;    
    }
    else {
        yInfo("Context FOUND! %s", PathName.c_str());
    }

    handlerPortName = "/" + moduleName + "/rpc:i";
    handlerPort.open(handlerPortName.c_str());
    attach(handlerPort);
    closing = false;
    bool useAdaptability = rf.check("adaptability",Value("on")).asString()=="on"?true:false;
    bool useGoalConsistency = rf.check("goalConsistency",Value("on")).asString()=="on"?true:false;
    bool useCreativity = rf.check("creativity",Value("on")).asString()=="on"?true:false;

    // thread stuff
    threadPeriod = 0.033; // [s]

    thread = new PlannerThread(moduleName,threadPeriod, PathName);

    // start the thread to do the work
    if (!thread->start())
    {
        delete thread;
        return false;
    }
    yInfo("adaptability heuristic is %s", useAdaptability?"on":"off");
    yInfo("creativity heuristic is %s", useCreativity?"on":"off");
    yInfo("goal consistency heuristic is %s", useGoalConsistency?"on":"off");
    thread->initValues(useAdaptability, useGoalConsistency, useCreativity);
    return true;
}

bool PlannerModule::interruptModule()
{
    handlerPort.interrupt();
    thread->askToStop();

    return true;
}

bool PlannerModule::close()
{

    yInfo("starting shutdown procedure");
    thread->interrupt();
    thread->resumePlanner();
    thread->close();
    thread->stop();
    if (thread) delete thread;

    handlerPort.close();
    yInfo("Module terminated");
    return true;
}

bool PlannerModule::updateModule()
{
    return !closing;
}


double PlannerModule::getPeriod()
{
    return 0.0;
}

// IDL functions
bool PlannerModule::attach(RpcServer &source)
{
    return this->yarp().attachAsServer(source);
}

bool PlannerModule::pausePlanner()
{
    yInfo("Pausing...");
    thread->pausePlanner();
    return true;
}

bool PlannerModule::resumePlanner()
{
    yInfo("Resuming...");
    thread->resumePlanner();
    return true;
}

bool PlannerModule::goBack()
{
    return thread->jumpBack();
}

bool PlannerModule::goForward()
{
    return thread->jumpForward();
}

bool PlannerModule::updateState()
{
    return thread->updateState(); 
}

bool PlannerModule::loadObjects()
{
    return thread->loadObjs();
}

Bottle PlannerModule::printObjects()
{
    return thread->printObjs();
}

bool PlannerModule::loadState()
{
    return thread->completePlannerState() && thread->loadState();
}

bool PlannerModule::updateGoals()
{
    return thread->goalUpdate() && thread->loadGoal();
}

bool PlannerModule::plan()
{
    return thread->PRADA();
}

bool PlannerModule::resetRules()
{
    return thread->resetRules() && thread->loadRules();
}

bool PlannerModule::resetConfig()
{
    return thread->resetConfig();
}

bool PlannerModule::stopPlanner()
{
    thread->stopPlanning();
    return true;
}

bool PlannerModule::resetLevel()
{
    return thread->resetPlanVars();
}

bool PlannerModule::ground()
{
    return thread->groundRules() && thread->loadRules() && thread->preserveRules();
}

bool PlannerModule::compileGoal()
{
    return thread->compileGoal() && thread->loadSubgoals();
}

bool PlannerModule::increaseHorizon()
{
    return thread->increaseHorizon();
}

bool PlannerModule::executePlannedAction()
{
    return thread->codeAction() && thread->execAction();
}

string PlannerModule::checkGoalCompleted()
{
    if (!thread->loadGoal())
    {
        return "fail";
    }
    if (thread->checkGoalCompletion())
    {
        return "Goal achieved";
    }
    else 
    {
        return "Goal not achieved";
    }
    return "stopped";
}

bool PlannerModule::run1Step()
{
    thread->planning_cycle();
    return true;
}

bool PlannerModule::startPlanner()
{
    
    return thread->resetPlanVars() && thread->startPlanning();
}

string PlannerModule::showPlannedAction()
{
    thread->codeAction();
    return thread->showPlannedAction();
}

string PlannerModule::showCurrentState()
{
    return thread->showCurrentState();
}

string PlannerModule::showCurrentGoal()
{
    return thread->showCurrentGoal();
}

string PlannerModule::showSymbol(string symbol)
{
    return thread->printSymbol(symbol);
}

bool PlannerModule::quit()
{
    closing = true;
    return true;
}





#include "PlannerModule.h"


bool PlannerModule::configure(ResourceFinder &rf)
{
    // module parameters
    moduleName = rf.check("name", Value("planner")).asString();
    PathName = rf.findPath("contexts/"+rf.getContext());
    setName(moduleName.c_str());
    handlerPortName = "/" + moduleName + "/rpc:i";
    handlerPort.open(handlerPortName.c_str());
    attach(handlerPort);

    // thread stuff
    threadPeriod = 0.033; // [s]

    thread = new PlannerThread(moduleName,threadPeriod, PathName);

    // start the thread to do the work
    if (!thread->start())
    {
        delete thread;
        return false;
    }
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
    thread->close();
    yInfo("deleting thread");
    delete thread;
    yInfo("done deleting thread");

    yInfo("closing RPC port");
    handlerPort.close();
    return true;
}

bool PlannerModule::updateModule()
{
    return !isStopping();
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

bool PlannerModule::stopPlanning()
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
    yInfo("Running one planning loop");
    thread->planning_cycle();
    return true;
}

bool PlannerModule::startPlanning()
{
    return thread->startPlanning();
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

bool PlannerModule::quit()
{
    close();
    return true;
}

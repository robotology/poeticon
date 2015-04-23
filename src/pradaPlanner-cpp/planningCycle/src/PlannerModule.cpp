



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
    closing = false;

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

    return true;
}

bool PlannerModule::close()
{
    yInfo("closing RPC port");
    handlerPort.close();

    yInfo("starting shutdown procedure");
    thread->interrupt();
    thread->close();
    yInfo("deleting thread");
    delete thread;
    yInfo("done deleting thread");

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
    return thread->pausePlanner();
}

bool PlannerModule::resumePlanner()
{
    yInfo("Resuming...");
    return thread->resumePlanner();
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

bool PlannerModule::resetLevel()
{
    return thread->resetPlanVars();
}

bool PlannerModule::ground()
{
    return thread->groundRules() && thread->loadRules();
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
    return thread->execAction();
}

bool PlannerModule::checkGoalCompleted()
{
    return thread->loadGoal() && thread->checkGoalCompletion();
}

bool PlannerModule::run1Step()
{
    return thread->planning_cycle();
}

bool PlannerModule::startPlanning()
{
    return thread->startPlanning();
}

string PlannerModule::showPlannedAction()
{
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
    closing = true;
    return true;
}

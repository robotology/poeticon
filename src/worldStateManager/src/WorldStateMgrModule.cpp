/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2016 VisLab, Institute for Systems and Robotics,
 *                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Giovanni Saponaro <gsaponaro@isr.ist.utl.pt>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 */

#include <chrono>
#include <thread>

#include "WorldStateMgrModule.h"

bool WorldStateMgrModule::configure(ResourceFinder &rf)
{
    if (rf.check("playback"))
    {
        yError("DEPRECATED option, use dummyWorldStateManager module for this functionality");
        return false;
    }

    // module parameters
    moduleName = rf.check("name", Value("wsm")).asString();
    setName(moduleName.c_str());

    handlerPortName = "/" + moduleName + "/rpc:i";
    handlerPort.open(handlerPortName.c_str());
    attach(handlerPort);

    closing = false;

    // create new thread, pass parameters with ResourceFinder
    thread = new WorldStateMgrThread(moduleName,
                                     rf);

    // start the thread to do the work
    if (!thread->start())
    {
        delete thread;
        return false;
    }

    return true;
}

bool WorldStateMgrModule::interruptModule()
{
    handlerPort.interrupt();

    return true;
}

bool WorldStateMgrModule::close()
{
    yInfo("closing RPC port");
    handlerPort.close();

    yInfo("starting shutdown procedure");
    thread->interrupt();
    thread->close();
    thread->stop();
    yInfo("deleting thread");
    if (thread) delete thread;
    yInfo("done deleting thread");

    return true;
}

bool WorldStateMgrModule::updateModule()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    return !closing;
}

double WorldStateMgrModule::getPeriod()
{
    return 0.0;
}

// IDL functions
bool WorldStateMgrModule::init()
{
    return thread->initWorldState();
}

bool WorldStateMgrModule::isInitialized()
{
    return thread->isInitialized();
}

bool WorldStateMgrModule::attach(RpcServer &source)
{
    return this->yarp().attachAsServer(source);
}

bool WorldStateMgrModule::dump()
{
    return thread->printMemoryState();
}

bool WorldStateMgrModule::update()
{
    return thread->updateWorldState();
}

bool WorldStateMgrModule::reset()
{
    return thread->resetWorldState();
}

bool WorldStateMgrModule::pause(const string &objName)
{
    // deprecated
    return false;
}

bool WorldStateMgrModule::resume(const string &objName)
{
    // deprecated
    return false;
}

bool WorldStateMgrModule::pauseID(const int32_t objID)
{
    // deprecated
    return false;
}

bool WorldStateMgrModule::resumeID(const int32_t objID)
{
    // deprecated
    return false;
}

bool WorldStateMgrModule::quit()
{
    yInfo("quitting");
    closing = true;

    return true;
}

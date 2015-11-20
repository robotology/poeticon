/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2014 VisLab, Institute for Systems and Robotics,
 *                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Giovanni Saponaro <gsaponaro@isr.ist.utl.pt>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 */

#include "WorldStateMgrModule.h"

bool WorldStateMgrModule::configure(ResourceFinder &rf)
{
    // module parameters
    moduleName = rf.check("name", Value("wsm")).asString();
    setName(moduleName.c_str());
    handlerPortName = "/" + moduleName + "/rpc:i";
    handlerPort.open(handlerPortName.c_str());
    attach(handlerPort);
    closing = false;

    // thread stuff
    threadPeriod = 0.033; // [s]
    playbackMode = rf.check("playback");
    if (playbackMode)
    {
        yInfo("module started in playback mode");
        playbackFile = rf.findFile("playback").c_str();
        if (playbackFile.empty())
        {
            yError() << "playback file not found";
            return false;
        }
        else
            yInfo() << "playback file loaded successfully:" << playbackFile;
    }
    countFrom   = rf.check("countFrom", Value(13)).asInt();
    withFilter  = rf.check("filter") && rf.find("filter").asString()!="off";
    if (withFilter)
    {
        filterOrder = rf.check("filterOrder", Value(5)).asInt();
        yInfo("selected temporal filtering with order %d", filterOrder);
    }

    // create new thread and pass pointers to the module parameters
    thread = new WorldStateMgrThread(moduleName,
                                     threadPeriod,
                                     playbackMode,
                                     countFrom,
                                     withFilter);

    // additional settings for filtering
    if (withFilter) thread->setFilterOrder(filterOrder);

    // additional settings for playback mode
    if (playbackMode) thread->setPlaybackFile(playbackFile);

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
    return !closing;
}

double WorldStateMgrModule::getPeriod()
{
    return 0.0;
}

// IDL functions
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
    return thread->pauseTrack(objName);
}

bool WorldStateMgrModule::resume(const string &objName)
{
    return thread->resumeTrack(objName);
}

Bottle WorldStateMgrModule::getColorHist(const int32_t u, const int32_t v)
{
    return thread->getColorHistogram(u,v);
}

bool WorldStateMgrModule::quit()
{
    yInfo("quitting");
    closing = true;

    return true;
}

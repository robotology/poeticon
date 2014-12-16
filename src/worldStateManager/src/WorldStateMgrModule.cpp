/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2014 VisLab, Institute for Systems and Robotics,
 *                Istituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Giovanni Saponaro <gsaponaro@isr.ist.utl.pt>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 */

#include "WorldStateMgrModule.h"

bool WorldStateMgrModule::configure(ResourceFinder &rf)
{
    moduleName = rf.check("name", Value("wsm")).asString();
    setName(moduleName.c_str());

    threadPeriod = 0.033; // [s]

    playbackMode = rf.check("playback");
    if (playbackMode)
    {
        playbackFile = rf.find("playback").asString();
        yInfo("module started in playback mode");
    }

    handlerPortName = "/" + moduleName + "/rpc:i";
    handlerPort.open(handlerPortName.c_str());
    attach(handlerPort);
    closing = false;

    // create new thread and pass pointers to the module parameters
    thread = new WorldStateMgrThread(moduleName, threadPeriod, playbackMode);

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
    yDebug("closing rpc port");
    handlerPort.close();

    yDebug("starting shutdown procedure");
    thread->interrupt();
    thread->close();
    yDebug("deleting thread");
    delete thread;
    yDebug("done deleting thread");

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


bool WorldStateMgrModule::attach(RpcServer &source)
{
    return this->yarp().attachAsServer(source);
}

bool WorldStateMgrModule::update()
{
    if (playbackMode)
        yDebug("updating world state from playback file");
    else
        yDebug("updating world state from robot perception");

    return thread->updateWorldState();
}

bool WorldStateMgrModule::quit()
{
    yDebug("quitting");
    closing = true;

    return true;
}

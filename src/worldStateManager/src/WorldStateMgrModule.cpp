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
    moduleName = rf.check("name", Value("wsm")).asString();
    setName(moduleName.c_str());

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

    // initial thread/track index for activeParticleTrack
    countFrom = rf.check("countFrom", Value(13)).asInt();

    handlerPortName = "/" + moduleName + "/rpc:i";
    handlerPort.open(handlerPortName.c_str());
    attach(handlerPort);
    closing = false;

    // create new thread and pass pointers to the module parameters
    thread = new WorldStateMgrThread(moduleName,
                                     threadPeriod,
                                     playbackMode,
                                     countFrom);

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
    yInfo("closing rpc port");
    handlerPort.close();

    yInfo("starting shutdown procedure");
    thread->interrupt();
    thread->close();
    yInfo("deleting thread");
    delete thread;
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


bool WorldStateMgrModule::attach(RpcServer &source)
{
    return this->yarp().attachAsServer(source);
}

bool WorldStateMgrModule::dump()
{
    return thread->dumpWorldState();
}

bool WorldStateMgrModule::update()
{
    return thread->updateWorldState();
}

bool WorldStateMgrModule::quit()
{
    yInfo("quitting");
    closing = true;

    return true;
}

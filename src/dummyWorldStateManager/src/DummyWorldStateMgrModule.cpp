/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2016 VisLab, Institute for Systems and Robotics,
 *                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Giovanni Saponaro <gsaponaro@isr.ist.utl.pt>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 */

#include "DummyWorldStateMgrModule.h"

bool DummyWorldStateMgrModule::configure(ResourceFinder &rf)
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
        playbackFile = rf.findFile("playback").c_str();
        if (playbackFile.empty())
        {
            yError() << "playback file not found";
            return false;
        }
        else
            yInfo() << "playback file loaded successfully:" << playbackFile;
    }
    else
    {
        yError() << "playback file not provided, please consult the --help";
        return false;
    }

    // create new thread and pass pointers to the module parameters
    thread = new DummyWorldStateMgrThread(moduleName,
                                          threadPeriod);

    // additional settings
    thread->setPlaybackFile(playbackFile);

    // start the thread to do the work
    if (!thread->start())
    {
        delete thread;
        return false;
    }

    return true;
}

bool DummyWorldStateMgrModule::interruptModule()
{
    handlerPort.interrupt();

    return true;
}

bool DummyWorldStateMgrModule::close()
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

bool DummyWorldStateMgrModule::updateModule()
{
    return !closing;
}

double DummyWorldStateMgrModule::getPeriod()
{
    return 0.0;
}

// IDL functions
bool DummyWorldStateMgrModule::attach(RpcServer &source)
{
    return this->yarp().attachAsServer(source);
}

bool DummyWorldStateMgrModule::update()
{
    return thread->updateWorldState();
}

bool DummyWorldStateMgrModule::quit()
{
    yInfo("quitting");
    closing = true;

    return true;
}

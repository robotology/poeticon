/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2015 VisLab, Institute for Systems and Robotics,
 *                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Giovanni Saponaro <gsaponaro@isr.ist.utl.pt>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 */

#include "WorldStateVisualizerModule.h"

bool WorldStateVisualizerModule::configure(ResourceFinder &rf)
{
    // module parameters
    moduleName = rf.check("name", Value("wsvisualizer")).asString();
    setName(moduleName.c_str());
    closing = false;

    // thread stuff
    threadPeriod = 0.033; // [s]

    // create new thread and pass pointers to the module parameters
    thread = new WorldStateVisualizerThread(moduleName,
                                            threadPeriod);

    // start the thread to do the work
    if (!thread->start())
    {
        delete thread;
        return false;
    }

    return true;
}

bool WorldStateVisualizerModule::interruptModule()
{
    return true;
}

bool WorldStateVisualizerModule::close()
{
    yInfo("starting shutdown procedure");
    thread->interrupt();
    thread->close();
    thread->stop();
    yInfo("deleting thread");
    if (thread) delete thread;
    yInfo("done deleting thread");

    return true;
}

bool WorldStateVisualizerModule::updateModule()
{
    return !closing;
}

double WorldStateVisualizerModule::getPeriod()
{
    return 0.0;
}

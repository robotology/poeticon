/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2014 VisLab, Institute for Systems and Robotics,
 *                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Giovanni Saponaro <gsaponaro@isr.ist.utl.pt>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 * Note: SLICSuperpixel class is by Saburo Okita,
 *       http://subokita.com/2014/04/23/slic-superpixels-on-opencv/
 *
 */

#include "SLICModule.h"

bool SLICModule::configure(ResourceFinder &rf)
{
    moduleName = rf.check("name", Value("slic"), "module name (string)").asString();
    setName(moduleName.c_str());

    threadPeriod = rf.check("threadPeriod", Value(0.033),
        "thread period in seconds (double)").asDouble();

    numSuperpixels = rf.check("numSuperpixels", Value(400),
        "desired number of superpixels (int)").asInt();

    m = rf.check("m", Value(10),
        "trade-off between spatial distance and color distance (int)").asInt();

    maxIter = rf.check("maxIter", Value(10),
        "maximum number of iterations (int)").asInt();

    closing = false;

    // create the thread and pass pointers to the module parameters
    thread = new SLICThread(moduleName, threadPeriod, numSuperpixels, m, maxIter);

    // start the thread to do the work
    if (!thread->start()) {
        delete thread;
        return false;
    }

    return true;
}

bool SLICModule::interruptModule()
{
    // interrupt rpc ports, if any

    return true;
}

bool SLICModule::close()
{
    // close rpc ports, if any

    fprintf(stdout, "%s: starting the shutdown procedure\n", moduleName.c_str());
    thread->interrupt();
    thread->close();
    fprintf(stdout, "%s: deleting thread\n", moduleName.c_str());
    delete thread;
    fprintf(stdout, "%s: done deleting thread\n", moduleName.c_str());

    return true;
}

bool SLICModule::quit()
{
    closing = true;
    return true;
}

bool SLICModule::updateModule()
{
    return !closing;
}

double SLICModule::getPeriod()
{
    return 0.0;
}

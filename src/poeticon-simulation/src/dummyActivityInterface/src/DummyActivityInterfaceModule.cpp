/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2018 VisLab, Institute for Systems and Robotics,
 *                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Giovanni Saponaro <gsaponaro@isr.tecnico.ulisboa.pt>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 * Based on activityInterface by Vadim Tikhanoff
 *
 */

#include <yarp/os/Log.h>

#include "Defaults.h"
#include "DummyActivityInterfaceModule.h"

using namespace std;
using namespace yarp::os;

bool DummyActivityInterfaceModule::configure(ResourceFinder &rf)
{
    // quit if the ini file is not found
    const string configFile = rf.find("from").asString();
    string configFileFullPath = rf.findFile(configFile.c_str());
    if (configFileFullPath == "")
    {
        yError("config file %s not found, quitting", configFile.c_str());
        return false;
    }

    // module parameters
    moduleName = rf.check("name", Value("activityInterface")).asString();
    setName(moduleName.c_str());

    handlerPortName = "/" + moduleName + "/rpc:i";
    handlerPort.open(handlerPortName.c_str());
    attach(handlerPort);

    closing = false;

    // create new thread, pass parameters with ResourceFinder
    thread = new DummyActivityInterfaceThread(moduleName,
                                              rf);

    // start the thread to do the work
    if (!thread->start())
    {
        delete thread;
        return false;
    }

    if (rf.check("period"))
    {
        int period = rf.find("period").asInt();
        yInfo("setting thread period to %d ms", period);
        thread->setRate(period);
    }

    return true;
}

bool DummyActivityInterfaceModule::interruptModule()
{
    handlerPort.interrupt();

    return true;
}

bool DummyActivityInterfaceModule::close()
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

bool DummyActivityInterfaceModule::updateModule()
{
    return !closing;
}

double DummyActivityInterfaceModule::getPeriod()
{
    return 0.0;
}

// IDL functions
bool DummyActivityInterfaceModule::attach(RpcServer &source)
{
    return this->yarp().attachAsServer(source);
}

bool DummyActivityInterfaceModule::askForTool(const string &handName,
                                              const int32_t xpos,
                                              const int32_t ypos)
{
    return thread->askForTool(handName, xpos, ypos);
}

Bottle DummyActivityInterfaceModule::askPraxicon(const string &request)
{
    return thread->askPraxicon(request);
}

bool DummyActivityInterfaceModule::drop(const string &objName)
{
    return thread->drop(objName);
}

bool DummyActivityInterfaceModule::dump()
{
    return thread->dump();
}

Bottle DummyActivityInterfaceModule::get2D(const string &objName)
{
    return thread->get2D(objName);
}

Bottle DummyActivityInterfaceModule::get3D(const string &objName)
{
    return thread->get3D(objName);
}

string DummyActivityInterfaceModule::getLabel(const int32_t xpos, const int32_t ypos)
{
    return thread->getLabel(xpos, ypos);
}

Bottle DummyActivityInterfaceModule::getNames()
{
    return thread->getNames();
}

bool DummyActivityInterfaceModule::goHome()
{
    return thread->goHome();
}

bool DummyActivityInterfaceModule::handStat(const string &handName)
{
    return thread->handStat(handName);
}

string DummyActivityInterfaceModule::inHand(const string &objName)
{
    return thread->inHand(objName);
}

bool DummyActivityInterfaceModule::pop()
{
    return thread->pop();
}

bool DummyActivityInterfaceModule::pull(const string &objName, const string &toolName)
{
    return thread->pull(objName, toolName);
}

Bottle DummyActivityInterfaceModule::pullableWith(const string &objName)
{
    return thread->pullableWith(objName);
}

bool DummyActivityInterfaceModule::push(const string &objName, const string &toolName)
{
    return thread->push(objName, toolName);
}

bool DummyActivityInterfaceModule::put(const string &objName, const string &targetName)
{
    return thread->put(objName, targetName);
}

Bottle DummyActivityInterfaceModule::reachableWith(const string &objName)
{
    return thread->reachableWith(objName);
}

string DummyActivityInterfaceModule::simulate()
{
    return thread->simulate();
}

bool DummyActivityInterfaceModule::take(const string &objName, const string &handName)
{
    return thread->take(objName, handName);
}

Bottle DummyActivityInterfaceModule::underOf(const string &objName)
{
    return thread->underOf(objName);
}

bool DummyActivityInterfaceModule::quit()
{
    yInfo("quitting");
    closing = true;

    return true;
}

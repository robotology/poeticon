/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2018 VisLab, Institute for Systems and Robotics,
 *                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Giovanni Saponaro <gsaponaro@isr.tecnico.ulisboa.pt>
 * CopyPolicy: Released under the terms of the GNU GPL v3.0
 *
 * Based on praxiconInterface by Vadim Tikhanoff
 *
 */

#include <yarp/os/Log.h>

#include "DummyPraxiconInterfaceModule.h"

using namespace std;
using namespace yarp::os;

bool DummyPraxiconInterfaceModule::configure(ResourceFinder &rf)
{
    moduleName = rf.check("name", Value("praxInterface")).asString();
    setName(moduleName.c_str());

    const std::string speechPortName = "/"+moduleName+"/speech:i";
    if (!speechPort.open(speechPortName.c_str())) {
        yError("unable to open port %s", speechPortName.c_str());
        return false;
    }

    return true;
}

bool DummyPraxiconInterfaceModule::interruptModule()
{
    speechPort.interrupt();

    return true;
}

bool DummyPraxiconInterfaceModule::close()
{
    speechPort.close();

    return true;
}

bool DummyPraxiconInterfaceModule::updateModule()
{
    yInfo("waiting for a message...");
    speech.clear();
    response.clear();
    speechPort.read(speech, true);

    if (!speech.isNull())
    {
        yInfo("received message: %s", speech.toString().c_str());

        // hardcoded response for the simple3 and complex6 scenarios:
        // ((hand grasp Ham) (Ham reach Bun-bottom) (hand put Ham) (hand grasp Bun-top) (Bun-top reach Ham) (hand put Bun-top))
        response.addString("hand grasp Ham");
        response.addString("Ham reach Bun-bottom");
        response.addString("hand put Ham");
        response.addString("hand grasp Bun-top");
        response.addString("Bun-top reach Ham");
        response.addString("hand put Bun-top");
        response.addString(""); // workaround to ensure that the previous string is correctly parsed

        yInfo("sending response: %s", response.toString().c_str());
        speechPort.reply(response);
    }

    return true;
}

double DummyPraxiconInterfaceModule::getPeriod()
{
    return 0.0;
}

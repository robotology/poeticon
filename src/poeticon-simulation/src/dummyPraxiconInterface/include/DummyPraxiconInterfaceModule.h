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

#ifndef DUMMY_PRAXICON_INTERFACE_MODULE_H
#define DUMMY_PRAXICON_INTERFACE_MODULE_H

#include <string>

#include <yarp/os/Bottle.h>
#include <yarp/os/Port.h>
#include <yarp/os/RFModule.h>

class DummyPraxiconInterfaceModule : public yarp::os::RFModule
{
private:
    std::string moduleName;
    yarp::os::Port speechPort;
    yarp::os::Bottle speech;
    yarp::os::Bottle response;

public:
    virtual bool configure(yarp::os::ResourceFinder &rf);
    virtual bool interruptModule();
    virtual bool close();
    virtual bool updateModule();
    virtual double getPeriod();
};

#endif

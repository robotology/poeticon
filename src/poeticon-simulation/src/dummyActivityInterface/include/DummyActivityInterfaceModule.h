/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2018 VisLab, Institute for Systems and Robotics,
 *                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Giovanni Saponaro <gsaponaro@isr.tecnico.ulisboa.pt>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 */

#ifndef DUMMY_ACTIVITY_INTERFACE_MODULE_H
#define DUMMY_ACTIVITY_INTERFACE_MODULE_H

#include <string>

#include <yarp/os/Bottle.h>
#include <yarp/os/RFModule.h>

#include "DummyActivityInterface_IDL.h"
#include "DummyActivityInterfaceThread.h"

class DummyActivityInterfaceModule : public yarp::os::RFModule,
                                     public DummyActivityInterface_IDL
{
    private:
        // module parameters
        std::string moduleName;
        std::string handlerPortName;
        yarp::os::RpcServer handlerPort;
        bool closing;

        // pointer to a new thread
        DummyActivityInterfaceThread *thread;

    public:
        virtual bool configure(yarp::os::ResourceFinder &rf);
        virtual bool interruptModule();
        virtual bool close();
        virtual bool updateModule();
        virtual double getPeriod();

        // IDL functions
        bool attach(yarp::os::RpcServer &source);
        bool askForTool(const std::string &handName, const int32_t xpos, const int32_t ypos);
        yarp::os::Bottle askPraxicon(const std::string &request);
        bool drop(const std::string &objName);
        bool dump();
        yarp::os::Bottle get2D(const std::string &objName);
        yarp::os::Bottle get3D(const std::string &objName);
        std::string getLabel(const int32_t xpos, const int32_t ypos);
        yarp::os::Bottle getNames();
        bool goHome();
        bool handStat(const std::string &handName);
        std::string inHand(const std::string &objName);
        bool pop();
        bool pull(const std::string &objName, const std::string &toolName);
        yarp::os::Bottle pullableWith(const std::string &objName);
        bool push(const std::string &objName, const std::string &toolName);
        bool put(const std::string &objName, const std::string &targetName);
        yarp::os::Bottle reachableWith(const std::string &objName);
        std::string simulate();
        bool take(const std::string &objName, const std::string &handName);
        yarp::os::Bottle underOf(const std::string &objName);
        bool quit();
};

#endif

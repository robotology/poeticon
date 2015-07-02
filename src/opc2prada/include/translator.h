// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2014 VisLab, Institute for Systems and Robotics,
 *                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Pedro Vicente <pvicente@isr.ist.utl.pt>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 */

#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <yarp/os/Bottle.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/os/Mutex.h>
#include <yarp/os/RateThread.h>
#include <yarp/os/RFModule.h>
#include <yarp/os/RpcClient.h>
#include <yarp/os/Time.h>
#include <yarp/os/Vocab.h>

#include <fstream>
#include <string>

#include "OPC2PRADA_IDL.h"

using namespace yarp::os;
using namespace std;

class Thread_read : public RateThread {
public:
    Thread_read(BufferedPort<Bottle> * broad_port,RpcClient * rpcClient,int r);
    Bottle _data,_ids;
    Mutex guard;
    Mutex guard_runit;
    bool _runit;
    virtual bool threadInit();
    
    //called by start after threadInit, s is true iff the thread started
    //successfully
    virtual void afterStart(bool s);
    virtual void threadRelease();
    virtual void run();

private:
    BufferedPort<Bottle> *_port_broad;
    RpcClient *_rpc_port;
};


class TranslatorModule: public RFModule, public OPC2PRADA_IDL {
    string moduleName;
    enum switchCase {name,is_h, pos2d,desc,tooldesc2d,in_h,on_t,re_w,pu_w, free, notfound};
    string translatorPortName, handlerPortName;
    BufferedPort<Bottle> translatorPort;

    string opcName;
    BufferedPort<Bottle> port_broad;
    RpcClient rpc_port;
    RpcServer handlerPort;
    double threadPeriod;

    Thread_read *readingThread;
    
    ofstream stateFile;

private:
    string stateFileName;
public:
    
    double getPeriod();
    bool configure(yarp::os::ResourceFinder &rf);
    bool updateModule();
    bool interruptModule();
    bool close();
    switchCase hashtable(string command);

    // IDL functions
    bool attach(yarp::os::RpcServer &source);
    Bottle query2d(const int32_t ObjectID);
    Bottle querytool2d(const int32_t ObjectID);
    Bottle loadObjects();
    bool update();
    bool quit();
};
#endif // TRANSLATOR_H

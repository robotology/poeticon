// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2014 VisLab, Institute for Systems and Robotics,
 *                Istituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Pedro Vicente <pvicente@isr.ist.utl.pt>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 */

#ifndef TRANSLATOR_H
#define TRANSLATOR_H
/*
#define CMD_NAME                         VOCAB4('n','a','m','e')
#define CMD_DESC                         VOCAB4('d','e','s','c')
#define CMD_POS							 VOCAB3('p','o','s')
#define CMD_IN_H                         VOCAB4('i','n','_','h')
#define CMD_ON_T						 VOCAB4('o','n','_','t')
#define CMD_REAW						 VOCAB4('r','e','_','w')
#define CMD_PULW						 VOCAB4('p','u','_','w')
#define CMD_ISH							 VOCAB4('i','s','_','h')
#define CMD_FREE						 VOCAB4('f','r','e','e')
#define CMD_TOUC						 VOCAB4('t','o','u','c')
*/
#include <yarp/os/Bottle.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/os/Mutex.h>
#include <yarp/os/RateThread.h>
#include <yarp/os/RFModule.h>
#include <yarp/os/RpcClient.h>
#include <yarp/os/Time.h>
#include <yarp/os/Vocab.h>
//#include <cstdio>
//#include <iostream>
#include <fstream>

using namespace yarp::os;
using namespace std;

class Thread_read : public RateThread {
public: 
	Thread_read(BufferedPort<Bottle> * broad_port,RpcClient * rpcClient,int r);
	Bottle _data,_ids;
	Mutex guard;
    virtual bool threadInit();
    
    //called by start after threadInit, s is true iff the thread started
    //successfully
    virtual void afterStart(bool s);
    virtual void threadRelease()
    {
        printf("Goodbye from thread1\n");
    }
	virtual void run();

private:
	BufferedPort<Bottle> *_port_broad;
	RpcClient *_rpc_port;
	bool firstTime;
};


class TranslatorModule: public RFModule {
    string moduleName;
	enum switchCase {name,pos,desc2d,is_h,free,in_h,on_t,re_w,pu_w,touch};
    string translatorPortName;
    BufferedPort<Bottle> translatorPort;

	string opcName;
    BufferedPort<Bottle> port_broad;
	RpcClient rpc_port;

    double threadPeriod;

    Thread_read *readingThread;

public:
    double getPeriod();
    bool configure(yarp::os::ResourceFinder &rf);
    bool updateModule();
    bool interruptModule();
    bool close();
	switchCase hashtable(string command);
};
#endif // TRANSLATOR_H

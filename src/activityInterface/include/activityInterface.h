/*
 * Copyright (C) 2011 Department of Robotics Brain and Cognitive Sciences - Istituto Italiano di Tecnologia
 * Author: Vadim Tikhanoff
 * email:  vadim.tikhanoff@iit.it
 * Permission is granted to copy, distribute, and/or modify this program
 * under the terms of the GNU General Public License, version 2 or any
 * later version published by the Free Software Foundation.
 *
 * A copy of the license can be found at
 * http://www.robotcub.org/icub/license/gpl.txt
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details
 */

#ifndef __ACTIVITY_INTERFACE_H__
#define __ACTIVITY_INTERFACE_H__


#include <yarp/os/BufferedPort.h>
#include <yarp/os/RFModule.h>
#include <yarp/os/Network.h>
#include <yarp/os/Thread.h>
#include <yarp/os/RateThread.h>
#include <yarp/os/Time.h>
#include <yarp/os/Semaphore.h>
#include <yarp/os/Stamp.h>

#include <yarp/dev/Drivers.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/dev/CartesianControl.h>
#include <iCub/iKin/iKinFwd.h>

#include <yarp/os/RpcClient.h>
#include <yarp/os/PortInfo.h>
#include <yarp/math/Math.h>

#include <math.h>

#include <time.h>
#include <string>
#include <iostream>
#include <iomanip>

#include <map>

#include <activityInterface_IDLServer.h>
#include "memoryReporter.h"


/**********************************************************/
class ActivityInterface : public yarp::os::RFModule, public activityInterface_IDLServer
{
    
protected:
    
    /* module parameters */
    std::string                     moduleName;
    std::string                     handlerPortName;
    
    /* module rpc interfaces */
    yarp::os::RpcServer             rpcPort;
    yarp::os::RpcClient             rpcARE;
    yarp::os::RpcClient             rpcAREcmd;
    yarp::os::RpcClient             rpcMemory;
    
    yarp::os::Port                  robotStatus;
    
    /* left & right cartesian interfaces */
    yarp::dev::PolyDriver           client_left;
    yarp::dev::ICartesianControl    *icart_left;
    
    yarp::dev::PolyDriver           client_right;
    yarp::dev::ICartesianControl    *icart_right;
    
    yarp::dev::PolyDriver           robotTorso;
    yarp::dev::PolyDriver           robotArm;
    
    yarp::dev::IControlLimits       *limTorso, *limArm;
    
    iCub::iKin::iCubArm             arm_left;
    iCub::iKin::iCubArm             arm_right;
    
    iCub::iKin::iKinChain           *chain_left;
    iCub::iKin::iKinChain           *chain_right;
    
    yarp::sig::Vector               reachAboveOrient[2];
    yarp::sig::Vector               thetaMin, thetaMax;
    
    MemoryReporter                  memoryReporter; //OPC class
    
    friend class                    MemoryReporter;

    bool                            first;
    int                             ctxt_left;
    int                             ctxt_right;
    
    /* parameters */
    bool                            closing;
    bool                            scheduleLoadMemory;
    
    std::map<std::string, std::string> inHandStatus;
    
public:
    
    ActivityInterface();
    ~ActivityInterface();
    
    bool configure(yarp::os::ResourceFinder &rf); // configure all the module parameters and return true if successful
    bool interruptModule();                       // interrupt, e.g., the ports
    bool close();                                 // close and shut down the module
    
    double getPeriod();
    bool updateModule();
    
    /* module functions */
    yarp::os::Bottle    getMemoryBottle();
    yarp::os::Bottle    getBlobCOG(const yarp::os::Bottle &blobs, const int i);
    bool                propagateStatus();

    /* rpc interface functions */
    bool                attach(yarp::os::RpcServer &source);
    double              getManip(const std::string &objName, const std::string &handName);
    bool                handStat(const std::string &handName);
    yarp::os::Bottle    get3D(const std::string &objName);
    std::string         getLabel(const int32_t pos_x, const int32_t pos_y);
    std::string         inHand(const std::string &objName);
    bool                take(const std::string &objName, const std::string &handName);
    bool                drop(const std::string &objName, const std::string &handName);
    bool                quit();
};

#endif
//empty line to make gcc happy

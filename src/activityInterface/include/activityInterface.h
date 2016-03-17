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

#include <yarp/sig/Image.h>

#include <yarp/dev/Drivers.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/dev/CartesianControl.h>
#include <iCub/iKin/iKinFwd.h>

#include <yarp/os/RpcClient.h>
#include <yarp/os/PortInfo.h>
#include <yarp/math/Math.h>

#include <cv.h>
#include <highgui.h>
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/features2d/features2d.hpp"

#include <math.h>

#include <time.h>
#include <string>
#include <iostream>
#include <iomanip>

#include <map>

#include <activityInterface_IDLServer.h>
#include "memoryReporter.h"

#include <iterator>
#include "iCub/activeSeg.h"


typedef std::pair<int, double> Pairs;
struct compare
{
    bool operator()(const Pairs& fisrtPair, const Pairs& secondPair) const
    {
        return fisrtPair.second < secondPair.second;
    }
};


/**********************************************************/
class ActivityInterface : public yarp::os::RFModule, public activityInterface_IDLServer
{
    
protected:
    
    /* module parameters */
    std::string                         moduleName;
    std::string                         handlerPortName;
    
    /* module rpc interfaces */
    yarp::os::RpcServer                 rpcPort;
    yarp::os::RpcClient                 rpcARE;
    yarp::os::RpcClient                 rpcAREcmd;
    yarp::os::RpcClient                 rpcMemory;
    yarp::os::RpcClient                 rpcWorldState;
    yarp::os::RpcClient                 rpcIolState;
    yarp::os::RpcClient                 rpcClassifier;
    
    yarp::os::RpcClient                 rpcKarma;
    
    yarp::os::RpcClient                 rpcPraxiconInterface;
    
    yarp::os::Port                      praxiconToPradaPort;
    
    yarp::os::Port                      robotStatus;
    yarp::os::Port                      imgClassifier;

    std::string                         inputBlobPortName;
    std::string                         inputImagePortName;

    yarp::os::BufferedPort<yarp::os::Bottle>                                dispBlobRoi;
    yarp::os::BufferedPort<yarp::os::Bottle>                                blobsPort;
    yarp::os::BufferedPort<yarp::sig::ImageOf<yarp::sig::PixelRgb> >        imagePortIn;
    yarp::os::BufferedPort<yarp::sig::ImageOf<yarp::sig::PixelMono> >       imgeBlobPort;
    
    /* left & right cartesian interfaces */
    yarp::dev::PolyDriver               client_left;
    yarp::dev::ICartesianControl        *icart_left;
    
    yarp::dev::PolyDriver               client_right;
    yarp::dev::ICartesianControl        *icart_right;
    
    yarp::dev::PolyDriver               robotTorso;
    yarp::dev::PolyDriver               robotArm;
    
    yarp::dev::IControlLimits           *limTorso, *limArm;
    
    iCub::iKin::iCubArm                 arm_left;
    iCub::iKin::iCubArm                 arm_right;
    
    iCub::iKin::iKinChain               *chain_left;
    iCub::iKin::iKinChain               *chain_right;
    
    yarp::sig::Vector                   reachAboveOrient[2];
    yarp::sig::Vector                   thetaMin, thetaMax;
    
    MemoryReporter                      memoryReporter;
    PradaReporter                       pradaReporter;
    SpeechReporter                      speechReporter;
    ActiveSeg                           activeSeg;
    
    friend class                        MemoryReporter;
    friend class                        PradaReporter;
    friend class                        SpeechReporter;

    bool                                first;
    int                                 ctxt_left;
    int                                 ctxt_right;
    
    /* parameters */
    bool                                closing;
    bool                                inAction;
    bool                                scheduleLoadMemory;
    
    std::map<std::string, std::string>  inHandStatus;
    std::map<int, std::string>          onTopElements;
    
    std::map<std::string, cv::Scalar>   stakedObject;
    
    std::vector<std::string>            availableTools;
    
    int                                 elements;
    std::vector<int>                    pausedThreads;
    
    yarp::os::Semaphore                 semaphore;
    std::string                         praxiconRequest;
    
    
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
    bool                handleTrackers();
    std::string         getMemoryNameBottle(int id);
    yarp::os::Bottle    getIDs();
    bool                executeSpeech(const std::string &speech);
    yarp::os::Bottle    getToolLikeNames();
    double              getAxes(std::vector<cv::Point> &pts, cv::Mat &img);
    
    int                 getPairMinIndex(std::map<int, double> pairmap);
    double              getPairMin(std::map<int, double> pairmap);
    double              getPairMax(std::map<int, double> pairmap);
    
    bool                processPradaStatus(const yarp::os::Bottle &status);
    bool                processSpeech(const yarp::os::Bottle &speech);
    bool                pauseAllTrackers();
    bool                resumeAllTrackers();
    bool                initObjectTracker(const std::string &objName);
    yarp::os::Bottle    trackStackedObject(const std::string &objName);
    
    bool                with_robot;
    bool                shouldUpdate;
    bool                allPaused;
    
    int                 incrementSize[10];

    /* rpc interface functions */
    bool                attach(yarp::os::RpcServer &source);
    double              getManip(const std::string &objName, const std::string &handName);
    bool                handStat(const std::string &handName);
    yarp::os::Bottle    get3D(const std::string &objName);
    yarp::os::Bottle    get2D(const std::string &objName);
    std::string         getLabel(const int32_t pos_x, const int32_t pos_y);
    std::string         inHand(const std::string &objName);
    bool                take(const std::string &objName, const std::string &handName);
    bool                put(const std::string &objName, const std::string &targetName);
    yarp::os::Bottle    underOf(const std::string &objName);
    yarp::os::Bottle    queryUnderOf(const std::string &objName);
    yarp::os::Bottle    getOffset(const std::string &objName);
    bool                askForTool(const std::string &handName, const int32_t pos_x, const int32_t pos_y);
    yarp::os::Bottle    reachableWith(const std::string &objName);
    yarp::os::Bottle    pullableWith(const std::string &objName);
    yarp::os::Bottle    getNames();
    yarp::os::Bottle    getOPCNames();
    yarp::os::Bottle    askPraxicon(const std::string &request);
    bool                drop(const std::string &objName);
    bool                push(const std::string &objName, const std::string &toolName);
    bool                pull(const std::string &objName, const std::string &toolName);
    bool                goHome();
    bool                resetObjStack();
    bool                testFill();
    yarp::os::Bottle    getCog(const int32_t tlpos_x, const int32_t tlpos_y, const int32_t brpos_x, const int32_t brpos_y);
    
    bool                trainObserve(const std::string &label);
    bool                classifyObserve();
    bool                gotSpike(const std::string &handName);
    std::string         holdIn(const std::string &handName);

    std::string         processScores(const yarp::os::Bottle &scores);
    
    bool                quit();
};

#endif
//empty line to make gcc happy

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

#ifndef __ICUB_ACTIVEPARTICLE_MOD_H__
#define __ICUB_ACTIVEPARTICLE_MOD_H__

#include <yarp/os/BufferedPort.h>
#include <yarp/os/RFModule.h>
#include <yarp/os/Network.h>
#include <yarp/os/Thread.h>
#include <yarp/os/RateThread.h>
#include <yarp/os/Time.h>
#include <yarp/os/Semaphore.h>
#include <yarp/os/Stamp.h>
#include <yarp/sig/Vector.h>
#include <yarp/sig/Image.h>

#include <yarp/os/RpcClient.h>

#include <time.h>
#include <string>
#include <iostream>
#include <iomanip>

#include <cv.h>
#include <highgui.h>

#include "iCub/utils.h"

#include "iCub/activeSeg.h"

#include "iCub/targetContainer.h"
#include "activeParticleTrack_IDLServer.h"
#include <map>


class TRACKERManager : public yarp::os::BufferedPort<yarp::sig::ImageOf<yarp::sig::PixelRgb> >
{
private:

    std::string moduleName;              //string containing module name
    std::string inImgPortName;           //string containing image input port name
    std::string inFixPortName;           //string containing template input port name

    std::string outImgPortName;          //string containing image output port name
    std::string outTargetPortName;       //string containing target output port name

    std::string outputCropSegName;       //string containing target segmentation output port name
    std::string outputCropTplName;       //string containing target segmentation output port name

    yarp::os::BufferedPort<yarp::sig::ImageOf<yarp::sig::PixelRgb> >    imageOutPort;           //output port Image
    yarp::os::BufferedPort<yarp::sig::ImageOf<yarp::sig::PixelBgr> >    imageSegOutPort;        //output port Image
    yarp::os::BufferedPort<yarp::sig::ImageOf<yarp::sig::PixelBgr> >    imageTplOutPort;        //output port Image
    yarp::os::BufferedPort<yarp::os::Bottle>                            targetOutPort;          //output port containing targets

    bool                    checkClosure;

    int                     iter;

    yarp::sig::Vector       toDel;

    yarp::os::Mutex         mutexPoints;
    yarp::os::Semaphore     mutex;
    IplImage                *orig;

    FixationPoint           fixationPoint;      //class to receive points from fixation point
    
    yarp::os::ResourceFinder    rf;

    friend class            FixationPoint;

    TargetObjectRecord      &container;

    std::map< unsigned int, ParticleThread* >  workerThreads;

    double fix_x, fix_y, cropSizeWidth, cropSizeHeight;

    void cloneTracker(TargetObject *obj, cv::Point *pt);

public:
    /**
     * constructor
     * @param moduleName is passed to the thread in order to initialise all the ports correctly (default yuvProc)
     * @param imgType is passed to the thread in order to work on YUV or on HSV images (default yuv)
     */
    TRACKERManager( const std::string &moduleName, yarp::os::ResourceFinder &rf );
    ~TRACKERManager();
    bool                    disParticles;
    bool                    flag;
    bool                    deleted;

    bool    open();
    void    close();
    void    onRead( yarp::sig::ImageOf<yarp::sig::PixelRgb> &img );
    void    interrupt();
    int     processFixationPoint(yarp::os::Bottle &b);
    bool    stopTracker(int id);
    bool    stopTrackers();

    void afterStart(bool s)
    {
        if (s)
            std::cout<<"Thread started successfully"<< std::endl;
        else
            std::cout<<"Thread did not start"<< std::endl;
    }

};
class TRACKERModule:public yarp::os::RFModule, public activeParticleTrack_IDLServer
{
    /* module parameters */
    std::string         moduleName;
    std::string         handlerPortName;
    yarp::os::RpcServer rpcPort;

    /* pointer to a new thread */
    TRACKERManager      *trackerManager;
    bool                closing;

public:

    bool configure(yarp::os::ResourceFinder &rf); // configure all the module parameters and return true if successful
    bool interruptModule();                       // interrupt, e.g., the ports
    bool close();                                 // close and shut down the module
    //bool respond(const yarp::os::Bottle& command, yarp::os::Bottle& reply);

    bool attach(yarp::os::RpcServer &source);
    bool display(const std::string &value);
    int  track(const int32_t fix_x, const int32_t fix_y);
    bool untrack(const int32_t id);
    bool reset();
    bool quit();

    double getPeriod();
    bool updateModule();
};
#endif
//empty line to make gcc happy

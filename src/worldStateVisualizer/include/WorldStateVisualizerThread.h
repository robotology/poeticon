/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2015 VisLab, Institute for Systems and Robotics,
 *                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Giovanni Saponaro <gsaponaro@isr.ist.utl.pt>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 */

#ifndef __WSVIS_THREAD_H__
#define __WSVIS_THREAD_H__

#include <string>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv/cv.h> // legacy cvCopyImage

#include <yarp/os/Bottle.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/os/Log.h>
#include <yarp/os/LogStream.h>
#include <yarp/os/RateThread.h>
#include <yarp/os/Semaphore.h>
#include <yarp/os/Stamp.h>
#include <yarp/sig/Image.h>

using namespace cv;
using namespace std;
using namespace yarp::os;
using namespace yarp::sig;

class WorldStateVisualizerThread : public RateThread
{
    private:
        string moduleName;

        bool closing;
        Semaphore mutex;

        string inOpcBroadcastPortName;
        string inRawImgPortName;
        string outReachabilityImgPortName;

        BufferedPort<Bottle> inOpcBroadcastPort;
        BufferedPort<ImageOf<PixelBgr> > inRawImgPort;
        BufferedPort<ImageOf<PixelBgr> > outReachabilityImgPort;

    public:
        WorldStateVisualizerThread(const string &_moduleName,
                                   const double _period);
        bool openPorts();
        void close();
        void interrupt();
        bool threadInit();
        void run();

        void mainProcessing();

        bool parseObjProperties (const Bottle *fields,
                                 string &name,
                                 Bottle &pos2d,
                                 Bottle &desc2d, Bottle &tooldesc2d,
                                 string &inHand, Bottle &onTopOf,
                                 Bottle &reachW, Bottle &pullW);
        bool containsAtLeastOneHand(const Bottle &b);
};

#endif

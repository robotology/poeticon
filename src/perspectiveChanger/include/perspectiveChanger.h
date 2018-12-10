/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2014 VisLab, Institute for Systems and Robotics,
 *                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Afonso Gonçalves <agoncalves@isr.ist.utl.pt>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 */

#ifndef PERSPECTIVECHANGER_H
#define PERSPECTIVECHANGER_H

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>

#include <yarp/os/all.h>
#include <yarp/sig/all.h>
#include <yarp/dev/GazeControl.h>
#include <yarp/math/Math.h>
#include <iCub/ctrl/math.h>
#include <yarp/dev/PolyDriver.h>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace yarp::os;
using namespace yarp::sig;
using namespace yarp::dev;

class CtrlThread:public RateThread {
protected:

    BufferedPort<ImageOf<PixelBgr> > *perspectiveChangerPort;
    BufferedPort<Bottle> *perspectiveChangerTransformationPort;
    PolyDriver   clientGazeCtrl;
    IGazeControl *igaze;
    int     startupContextId;
    double  tableOffset;
    double  pixelDensity;

public:
    CtrlThread(BufferedPort<ImageOf<PixelBgr> > *modulePort,
               BufferedPort<Bottle> *moduleTransformationPort,
               const double table,
               const double pixelD,
               const double period);
    bool threadInit();
    void threadRelease();
    void run();

};

class PerspectiveChangerModule: public RFModule {
    string moduleName;

    string perspectiveChangerPortName;
    string perspectiveChangerTransformationPortName;
    BufferedPort<ImageOf<PixelBgr> > perspectiveChangerPort;
    BufferedPort<Bottle> perspectiveChangerTransformationPort;

    double threadPeriod;
    double tableOffset;
    double pixelDensity;

    CtrlThread *ctrlThread;

public:
    double getPeriod();
    bool configure(yarp::os::ResourceFinder &rf);
    bool updateModule();
    bool interruptModule();
    bool close();
};

#endif /* PERSPECTIVECHANGER_H */

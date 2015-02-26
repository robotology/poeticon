/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2014 VisLab, Institute for Systems and Robotics,
 *                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Giovanni Saponaro <gsaponaro@isr.ist.utl.pt>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 * Note: SLICSuperpixel class is by Saburo Okita,
 *       http://subokita.com/2014/04/23/slic-superpixels-on-opencv/
 *
 */

#ifndef	__SLIC_THREAD_H__
#define __SLIC_THREAD_H__

#include <yarp/os/BufferedPort.h>
#include <yarp/os/RateThread.h>
#include <yarp/os/Semaphore.h>
#include <yarp/os/Stamp.h>
#include <yarp/sig/Image.h>
#include <opencv/cv.h> // legacy cvCopyImage
#include <opencv2/imgproc/imgproc.hpp>
#include <tbb/tbb.h>
#include "SLICSuperpixel.h"

using namespace cv;
using namespace yarp::os;
using namespace yarp::sig;

class SLICThread : public RateThread
{
    private:
        string moduleName;
        int numSuperpixels;
        int m;
        int maxIter;

        string inRawImgPortName;
        string outContourImgPortName;
        string outRecoloredImgPortName;
        string outCielabImgPortName;
        string outBinaryImgPortName;

        BufferedPort<ImageOf<PixelBgr> > inRawImgPort;
        BufferedPort<ImageOf<PixelBgr> > outContourImgPort;
        BufferedPort<ImageOf<PixelBgr> > outRecoloredImgPort;
        BufferedPort<ImageOf<PixelBgr> > outCielabImgPort;
        BufferedPort<ImageOf<PixelMono> > outBinaryImgPort;
        
        bool closing;
        Semaphore mutex;

    public:
        SLICThread(const string &_moduleName, const double _period,
                   const int _numSuperpixels, const int _m,
                   const int _maxIter);

        bool openPorts();
        void close();
        void interrupt();

        bool threadInit();
        void run();
};

#endif

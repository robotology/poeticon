/*
 * Copyright (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright (C) 2009 RobotCub Consortium, European Commission FP6 Project IST-004370
 * Author: Giovanni Saponaro <gsaponaro@isr.ist.utl.pt>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 */

#ifndef ICUB_BLOB_DESC_MODULE_H
#define ICUB_BLOB_DESC_MODULE_H

#include <cmath>
#include <cstdio>
#include <string>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <yarp/os/BufferedPort.h>
#include <yarp/os/Log.h>
#include <yarp/os/LogStream.h>
#include <yarp/os/Network.h>
#include <yarp/os/Port.h>
#include <yarp/os/RFModule.h>
#include <yarp/os/Stamp.h>
#include <yarp/os/Time.h>
#include <yarp/sig/Image.h>

#include <iCub/BlobDescriptorSupport.h>

using namespace std;
using namespace yarp::os;
using namespace yarp::sig;

class BlobDescriptorModule : public RFModule
{
    /* private class variables and module parameters */
    string                            _moduleName;
    string                            _rawImgInputPortName;
    string                            binaryImgInputPortName;
    string                            _labeledImgInputPortName;
    string                            _viewImgOutputPortName;
    string                            _affDescriptorOutputPortName;
    string                            _trackerInitOutputPortName;
    string                            toolAffDescriptorOutputPortName;
    string                            bothPartsImgOutputPortName;
    BufferedPort<ImageOf<PixelBgr> >  _rawImgInputPort;
    BufferedPort<ImageOf<PixelBgr> >  binaryImgInputPort;
    BufferedPort<ImageOf<PixelInt> >  _labeledImgInputPort;
    BufferedPort<ImageOf<PixelBgr> >  _viewImgOutputPort;
    BufferedPort<Bottle>              _affDescriptorOutputPort;
    BufferedPort<Bottle>              toolAffDescriptorOutputPort;

    Port bothPartsImgOutputPort; // for image with top & bottom tool parts
    //BufferedPort<ImageOf<PixelRgb> >  bothPartsImgOutputPort;

    /* yarp image pointers to access image ports */
    ImageOf<PixelBgr>                *_yarpRawInputPtr;
    ImageOf<PixelBgr>                *yarpBinaryInputPtr;
    ImageOf<PixelInt>                *_yarpLabeledInputPtr;
    /* yarp internal image buffers */
    ImageOf<PixelBgr>                 _yarpRawImg;
    ImageOf<PixelBgr>                 yarpBinaryImg;
    ImageOf<PixelBgr>                 _yarpViewImg;
    ImageOf<PixelBgr>                 _yarpHSVImg;
    ImageOf<PixelMono>                _yarpHueImg;
    ImageOf<PixelMono>                _yarpSatImg;
    ImageOf<PixelMono>                _yarpValImg;
    ImageOf<PixelInt>                 _yarpLabeledImg;
    ImageOf<PixelMono>                _yarpTempImg;

    //ImageOf<PixelBgr>                 imgBothParts;

    Bottle                            _affDescriptor;
    Bottle                            toolAffDescriptor;
    Bottle                            _trackerInit;
    int                               _w, _h;
    CvSize                            _sz;

    // variables for object (tool) parts
    cv::Point2f top_center, bot_center; // w.r.t. original non-rotated image
    cv::Point2f top_center_rot, bot_center_rot; // w.r.t. rotated image
    //cv::Point2f top_circle_center, bot_circle_center; // centers of enclosing circles, good grasp points
    //float top_circle_radius, bot_circle_radius;
    cv::Point2f top_circle_center_rot, bot_circle_center_rot; // w.r.t rotated image

    // image containing both parts, stacked vertically
    cv::Mat matBothParts;

    ObjectDescriptor                 *_objDescTable;
    int                               _numObjects;

    /* other parameters that can be user-specified */
    int _minAreaThreshold; /* min. number of pixels allowed for foreground objects */
    int _maxAreaThreshold; /* min. number of pixels allowed for foreground objects */
    int _maxObjects;       /* maximum number of object to process */
    int _invalidate_boundary_objects; /*  Flag to invalidate objects touching the image boundaries */
    int _draw_holes;       /* Flag to draw the holes of the valid objects in the display image */

    // added for First-MM, Jan. 2011
    bool normalized_coords;

    // added for POETICON++, Feb. 2014
    static const int HORIZ_CROPAREA_SHIFT = 8; // to slightly enlarge tool part crop area
    static const int VERT_CROPAREA_SHIFT  = 5;
    bool synch_inputs;
    float elongatedness_thr;

public:
    virtual bool configure(ResourceFinder &rf); /* configure module parameters, return true if successful */
    virtual bool interruptModule();             /* interrupt, e.g., ports */
    virtual bool close();                       /* close and shut down module */
    virtual bool respond(const Bottle &command, Bottle &reply);
    virtual bool updateModule();
    virtual double getPeriod();
};

#endif // ICUB_BLOB_DESC_MODULE_H

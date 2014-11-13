/*
 * Copyright (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright (C) 2009 RobotCub Consortium, European Commission FP6 Project IST-004370
 * Author: Giovanni Saponaro <gsaponaro@isr.ist.utl.pt>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 */

#ifndef __ICUB_BLOB_DESC_MODULE_H__
#define __ICUB_BLOB_DESC_MODULE_H__

/* YARP */
#include <yarp/os/BufferedPort.h>
#include <yarp/os/Network.h>
#include <yarp/os/Port.h>
#include <yarp/os/RFModule.h>
#include <yarp/os/Stamp.h>
#include <yarp/os/Time.h>
using namespace yarp::os;
#include <yarp/sig/Image.h>
using namespace yarp::sig;

/* iCub */
#include <iCub/BlobDescriptorSupport.h>

/* OpenCV */
// TODO: full transition to OpenCV 2 C++ APIs
#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>
#include <opencv2/imgproc/imgproc.hpp>

/* system */
#include <string>
#include <stdio.h>
using namespace std;

class BlobDescriptorModule : public RFModule
{
    /* private class variables and module parameters */
    string                            _moduleName;
    string                            _rawImgInputPortName;
    string                            binaryImgInputPortName;
    string                            _labeledImgInputPortName;
    string                            _rawImgOutputPortName;
    string                            _viewImgOutputPortName;
    string                            _affDescriptorOutputPortName;
    string                            _trackerInitOutputPortName;
    string                            toolAffDescriptorOutputPortName;
    string                            bothPartsImgOutputPortName;
    string                            _handlerPortName;
    Port                              _handlerPort; /* rpc port to handle messages */
    BufferedPort<ImageOf<PixelRgb> >  _rawImgInputPort;
    BufferedPort<ImageOf<PixelRgb> >  binaryImgInputPort;
    BufferedPort<ImageOf<PixelInt> >  _labeledImgInputPort;
    BufferedPort<ImageOf<PixelRgb> >  _rawImgOutputPort;
    BufferedPort<ImageOf<PixelRgb> >  _viewImgOutputPort;
    BufferedPort<Bottle>              _affDescriptorOutputPort;
    BufferedPort<Bottle>              toolAffDescriptorOutputPort;

    Port bothPartsImgOutputPort; // for image with top & bottom tool parts
    //BufferedPort<ImageOf<PixelRgb> >  bothPartsImgOutputPort;

    /* yarp image pointers to access image ports */
    ImageOf<PixelRgb>                *_yarpRawInputPtr;
    ImageOf<PixelRgb>                *yarpBinaryInputPtr;
    ImageOf<PixelInt>                *_yarpLabeledInputPtr;
    /* yarp internal image buffers */
    ImageOf<PixelRgb>                 _yarpRawImg;
    ImageOf<PixelRgb>                 yarpBinaryImg;
    ImageOf<PixelRgb>                 _yarpViewImg;
    ImageOf<PixelRgb>                 _yarpHSVImg;
    ImageOf<PixelMono>                _yarpHueImg;
    ImageOf<PixelMono>                _yarpSatImg;
    ImageOf<PixelMono>                _yarpValImg;
    ImageOf<PixelInt>                 _yarpLabeledImg;
    ImageOf<PixelMono>                _yarpTempImg;

    //ImageOf<PixelRgb>                 imgBothParts;

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
    int	_invalidate_boundary_objects; /*  Flag to invalidate objects touching the image boundaries */
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

#endif // __ICUB_BLOB_DESC_MODULE_H__

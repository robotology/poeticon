/*
 * Copyright (C) 2013 POETICON++, European Commission FP7 project ICT-288382
 * Author: Giovanni Saponaro <gsaponaro@isr.ist.utl.pt>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0 
 *
 */

/* system */
#include <iostream>

#include "SequentialLabellerModule.h"

/**
 * Receive a previously initialized Resource Finder object and process module parameters,
 * both from command line and .ini file.
 */
bool SequentialLabellerModule::configure(ResourceFinder &rf)
{
    /* get the name that will form the prefix of all module port names */
    _moduleName = rf.check( "name",
                Value("sequentialLabeller"),
                "Module name (string)" ).asString();

    /* before continuing, set the module name */
    setName( ("/"+_moduleName).c_str() );

    /* now, get the remaining parameters */
    binaryImgInputPortName      = getName(
                                           rf.check( "binary_image_input_port",
                                                     Value("/binImg:i"),
                                                     "Binary image input port (string)" ).asString()
                                           );
    labeledImgOutputPortName    = getName(
                                           rf.check( "labeled_image_output_port",
                                                     Value("/labeledImg:o"),
                                                     "Labeled image output port (string)" ).asString()
                                           );
    //Network::init();
    
    /* open ports */
    if(! binaryImgInputPort.open(binaryImgInputPortName.c_str()) )
    {
        cout << getName() << ": unable to open port" << binaryImgInputPortName << endl;
        return false;
    }
    if(! labeledImgOutputPort.open(labeledImgOutputPortName.c_str()) )
    {
        cout << getName() << ": unable to open port" << labeledImgOutputPortName << endl;
        return false;
    }
    
    yarpBinaryInputPtr = binaryImgInputPort.read(true);
    
    if (yarpBinaryInputPtr!=NULL)
    {
        w   = yarpBinaryInputPtr->width();
        h   = yarpBinaryInputPtr->height();
        sz  = cvSize(w, h);

        /* allocate internal image buffers */
        yarpBinaryImg.resize(w,h);
        yarpLabeledImg.resize(w,h);
    }

    return true; /* tell RFModule that everything went well, so that it will run the module */
}

/**
 * Try to halt operations by threads managed by the module. Called asynchronously
 * after a quit command is received.
 */
bool SequentialLabellerModule::interruptModule()
{
    cout << getName() << ": interrupting module, for port cleanup." << endl;
    binaryImgInputPort.interrupt();
    labeledImgOutputPort.interrupt();
    return true;
}
    
/**
 * Close function. Called automatically when the module closes, after the last
 * updateModule call.
 */
bool SequentialLabellerModule::close()
{
    cout << getName() << ": closing module." << endl;
    binaryImgInputPort.close();
    labeledImgOutputPort.close();

    // Network::fini();
    return true;
}

/**
 * Main cycle, called iteratively every getPeriod() seconds.
 */
bool SequentialLabellerModule::updateModule()
{
    Stamp stamp; 

    yarpBinaryInputPtr = binaryImgInputPort.read(true);

    /* check if input image has timestamp */
    bool useStamp = true;
    if( !binaryImgInputPort.getEnvelope(stamp) )
    {
        cout << getName() << ": input image has no timestamp." << endl;
        useStamp = false;
    }

    if(yarpBinaryInputPtr==NULL)
    {
        cout << getName() << ": no data on input port(s). Exiting..." << endl;
        return false;    
    }
    yarpBinaryImg  = *yarpBinaryInputPtr;

    // get OpenCV pointers to images, to more easily call OpenCV functions
    // TODO: use OpenCV 2 APIs

    IplImage *opencvBinaryImg  = (IplImage *) yarpBinaryImg.getIplImage();
    
    // call sequential labelling algorithm
    IplImage *opencvLabeledImg;
    IplImage *opencvTempImg;
    opencvLabeledImg = cvCreateImage( sz, IPL_DEPTH_8U, 1);
    opencvTempImg = cvCreateImage( sz, IPL_DEPTH_32S, 1);
    cvZero(opencvTempImg);
    int seqLabReturn = 0;
    seqLabReturn = cvSeqLabel(opencvBinaryImg, opencvLabeledImg, opencvTempImg);
    
    // convert from OpenCV to yarp format
    if (opencvLabeledImg != NULL)
    {
        cvCopyImage( opencvLabeledImg, (IplImage *)yarpLabeledImg.getIplImage() );
    }

    /* output sequentially labelled image */    
    ImageOf<PixelMono> &yarpLabeledOutputImage = labeledImgOutputPort.prepare();
    yarpLabeledOutputImage = yarpLabeledImg;
    if(useStamp)
        labeledImgOutputPort.setEnvelope(stamp);
    labeledImgOutputPort.write();

    cvReleaseImage(&opencvLabeledImg);
    cvReleaseImage(&opencvTempImg);

    return true;
}

double SequentialLabellerModule::getPeriod()
{
    return 0.0;
}

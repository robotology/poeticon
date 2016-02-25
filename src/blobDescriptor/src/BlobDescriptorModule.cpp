// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/* 
 * Copyright (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright (C) 2009 RobotCub Consortium, European Commission FP6 Project IST-004370
 * Author: Giovanni Saponaro <gsaponaro@isr.ist.utl.pt>, Ivana Cingovska, Alexandre Bernardino
 * CopyPolicy: Released under the terms of the GNU GPL v2.0 
 *
 */

#include <cstdio>
#include <iostream>
 
#include <yarp/os/Network.h>

#include <iCub/BlobDescriptorModule.h>
#include <iCub/BlobDescriptorSupport.h>

using namespace std;
using namespace yarp::os;


/**
 * Receive a previously initialized Resource Finder object and process module parameters,
 * both from command line and .ini file.
 */
bool BlobDescriptorModule::configure(ResourceFinder &rf)
{
    /* module name that will form the prefix of all module port names */
    _moduleName = rf.check( "name",
                            Value("blobDescriptor"),
                            "Module name (string)" ).asString();
    setName( ("/"+_moduleName).c_str() );

    _rawImgInputPortName = getName( rf.check("raw_image_input_port",Value("/rawImg:i"),"Raw image input port (string)" ).asString() );
    binaryImgInputPortName = getName( rf.check("binary_image_input_port",Value("/binImg:i"),"Binary image input port (string)" ).asString() );
    _labeledImgInputPortName = getName( rf.check( "labeled_image_input_port",Value("/labeledImg:i"),"Labeled image input port (string)" ).asString() );
    _viewImgOutputPortName = getName( rf.check( "view_image_output_port",Value("/viewImg:o"),"View image output port (string)" ).asString() );
    _affDescriptorOutputPortName = getName( rf.check( "aff_descriptor_output_port",Value("/affDescriptor:o"),"Affordance descriptor output port (string)" ).asString() );
    toolAffDescriptorOutputPortName = getName( rf.check( "tool_aff_descriptor_output_port",Value("/toolAffDescriptor:o"),"Tool-Affordance descriptor output port (string)" ).asString() );
    bothPartsImgOutputPortName = getName( rf.check( "tool_parts_image_output_port",Value("/toolParts:o"),"Both tool parts image output port (string)" ).asString() );
    _minAreaThreshold = rf.check( "min_area_threshold",Value(200),"Minimum number of pixels allowed for foreground objects" ).asInt();
    _maxAreaThreshold = rf.check( "max_area_threshold",Value(20000),"Maximum number of pixels allowed for foreground objects" ).asInt();

    _maxObjects = rf.check( "max_objects",Value(20),"Maximum number of objects to process" ).asInt();
    if( _maxObjects <= 0)
    {
        yWarning("Invalid number of objects parameter. Will use default (20) instead.");
        _maxObjects = 20;
    }

    _invalidate_boundary_objects = rf.check("invalidate_boundary_objects",Value(0),"Flag to invalidate objects touching the image boundaries" ).asInt();

    _draw_holes = rf.check("draw_holes",Value(0),"Flag to draw the holes of the valid object in the overlay image" ).asInt();

    // RobotCub: 1; POETICON++: 0
    synch_inputs = rf.check("synch_inputs",Value(0),"Flag to ensure synchronization of input images").asInt();

    elongatedness_thr = (float) rf.check("elongatedness_thr", Value(0.8), "Minimum elongatedness required to display tool parts (only affects display image, not computation of descriptors)").asDouble();
    if( (elongatedness_thr<0.0) || (elongatedness_thr>1.0) )
    {
        yWarning("Invalid elongatedness_thr. Will use default (0.8) instead.");
        elongatedness_thr = 0.8;
    }

    // RobotCub: 1; First-MM,POETICON++: 0
    normalized_coords = (bool) rf.check("normalized_coords", Value(0), "Normalize rectangle x,y,w,h?").asInt();

    //Network::init();

    /* open ports */
    if(! _rawImgInputPort.open(_rawImgInputPortName.c_str()) )
    {
        yError("unable to open port %s", _rawImgInputPortName.c_str());
        return false;
    }
    if(! binaryImgInputPort.open(binaryImgInputPortName.c_str()) )
    {
        yError("unable to open port %s", binaryImgInputPortName.c_str());
        return false;
    }
    if(! _labeledImgInputPort.open(_labeledImgInputPortName.c_str()) )
    {
        yError("unable to open port %s", _labeledImgInputPortName.c_str());
        return false;
    }
    if(! _viewImgOutputPort.open(_viewImgOutputPortName.c_str()) )
    {
        yError("unable to open port %s", _viewImgOutputPortName.c_str());
        return false;
    }
    if(! _affDescriptorOutputPort.open(_affDescriptorOutputPortName.c_str()) )
    {
        yError("unable to open port %s", _affDescriptorOutputPortName.c_str());
        return false;
    }
    if(! toolAffDescriptorOutputPort.open(toolAffDescriptorOutputPortName.c_str()) )
    {
        yError("unable to open port %s", toolAffDescriptorOutputPortName.c_str());
        return false;
    }
    if(! bothPartsImgOutputPort.open(bothPartsImgOutputPortName.c_str()) )
    {
        yError("unable to open port %s", bothPartsImgOutputPortName.c_str());
        return false;
    }

    _yarpRawInputPtr     = _rawImgInputPort.read(true);
    yarpBinaryInputPtr   = binaryImgInputPort.read(true);
    _yarpLabeledInputPtr = _labeledImgInputPort.read(true);

    /* check to avoid memory leak when exiting and ports are not connected */
    if (_yarpRawInputPtr==NULL || _yarpLabeledInputPtr==NULL || yarpBinaryInputPtr==NULL)
    {
        yError("input images not initialized. Exiting...");
        return false;
    }

    /* check that input image dimensions are equal */
    if( (_yarpRawInputPtr->width()  != _yarpLabeledInputPtr->width())  ||
        (_yarpRawInputPtr->height() != _yarpLabeledInputPtr->height()) ||
        (_yarpRawInputPtr->width()  != yarpBinaryInputPtr->width())    ||
        (_yarpRawInputPtr->height() != yarpBinaryInputPtr->height()) )
    {
        yError("input image dimensions differ. Exiting...");
        return false;
    }

    _w   = _yarpRawInputPtr->width();
    _h   = _yarpRawInputPtr->height();
    _sz  = cvSize(_w, _h);

    /* allocate internal image buffers */
    _yarpRawImg.resize(_w,_h);
    _yarpHSVImg.resize(_w,_h);
    _yarpHueImg.resize(_w,_h);
    _yarpSatImg.resize(_w,_h);
    _yarpValImg.resize(_w,_h);
    _yarpLabeledImg.resize(_w,_h);
    _yarpViewImg.resize(_w,_h);
    _yarpTempImg.resize(_w,_h);

    /* initialize object descriptor list */
    _objDescTable = new ObjectDescriptor[_maxObjects];
    for(int i = 0; i < _maxObjects; i++)
        _objDescTable[i].Create(_w,_h);

    return true; /* tell RFModule that everything went well, so that it will run the module */
}

/**
 * Try to halt operations by threads managed by the module. Called asynchronously
 * after a quit command is received.
 */
bool BlobDescriptorModule::interruptModule()
{
    yInfo("interrupting module, for port cleanup.");

    _rawImgInputPort.interrupt();
    binaryImgInputPort.interrupt();
    _labeledImgInputPort.interrupt();
    _viewImgOutputPort.interrupt();
    _affDescriptorOutputPort.interrupt();
    toolAffDescriptorOutputPort.interrupt();
    bothPartsImgOutputPort.interrupt();
    return true;
}

/**
 * Close function. Called automatically when the module closes, after the last
 * updateModule call.
 */
bool BlobDescriptorModule::close()
{
    yInfo("closing module.");

    _rawImgInputPort.close();
    binaryImgInputPort.close();
    _labeledImgInputPort.close();
    _viewImgOutputPort.close();
    _affDescriptorOutputPort.close();
    toolAffDescriptorOutputPort.close();
    bothPartsImgOutputPort.close();

    delete [] _objDescTable;

    // Network::fini();
    return true;
}
 
/**
 * Message handler function. Echo all received messages, quit if required.
 */
bool BlobDescriptorModule::respond(const Bottle &command, Bottle &reply)
{
    yInfo("echoing received command.");
    reply = command;
    if(command.get(0).asString() == "quit")
        return false;
    else
        return true;
}
   
/**
 * Main cycle, called iteratively every getPeriod() seconds.
 */
bool BlobDescriptorModule::updateModule()
{
    Stamp rawstamp, labeledstamp, binstamp, writestamp; 

    // TODO: remove one of the inputs, change verification checks
    _yarpRawInputPtr = _rawImgInputPort.read(true);
    yarpBinaryInputPtr = binaryImgInputPort.read(true);
    _yarpLabeledInputPtr = _labeledImgInputPort.read(true);

    if( !_rawImgInputPort.getEnvelope(rawstamp) || !binaryImgInputPort.getEnvelope(binstamp) || !_labeledImgInputPort.getEnvelope(labeledstamp) )
    {
        yWarning("timestamp missing from one or more input ports!");
        if (synch_inputs) {
            // also quit
            return false;
        }
    }

    if (synch_inputs)
    {
        // synchronize input images if one of them is delayed, so that they correspond
        while( rawstamp.getCount()<labeledstamp.getCount() || rawstamp.getCount()<binstamp.getCount() )
        {
            _yarpRawInputPtr = _rawImgInputPort.read(true);
            _rawImgInputPort.getEnvelope(rawstamp);
        }
        while( binstamp.getCount()<rawstamp.getCount() || binstamp.getCount()<labeledstamp.getCount() )
        {
            yarpBinaryInputPtr = binaryImgInputPort.read(true);
            binaryImgInputPort.getEnvelope(binstamp);
        }
        while( labeledstamp.getCount()<rawstamp.getCount() || labeledstamp.getCount()<binstamp.getCount() )
        {
            _yarpLabeledInputPtr = _labeledImgInputPort.read(true);
            _labeledImgInputPort.getEnvelope(labeledstamp);
        }

    }

    // now all stamps are equal

    writestamp = rawstamp;

    if (_yarpRawInputPtr != NULL)
        _yarpRawImg     = *_yarpRawInputPtr;

    if (yarpBinaryInputPtr != NULL)
        yarpBinaryImg   = *yarpBinaryInputPtr;

    _yarpViewImg    =  _yarpRawImg;

    if (_yarpLabeledInputPtr != NULL)
        _yarpLabeledImg = *_yarpLabeledInputPtr;

    /* get OpenCV pointers to images, to more easily call OpenCV functions */
    IplImage *opencvRawImg     = (IplImage *) _yarpRawImg.getIplImage();
    IplImage *opencvHSVImg     = (IplImage *) _yarpHSVImg.getIplImage();
    IplImage *opencvHueImg     = (IplImage *) _yarpHueImg.getIplImage();
    IplImage *opencvSatImg     = (IplImage *) _yarpSatImg.getIplImage();
    IplImage *opencvValImg     = (IplImage *) _yarpValImg.getIplImage();
    IplImage *opencvBinaryImg  = (IplImage *) yarpBinaryImg.getIplImage();
    IplImage *opencvLabeledImg = (IplImage *) _yarpLabeledImg.getIplImage();
    IplImage *opencvViewImg    = (IplImage *) _yarpViewImg.getIplImage();
    IplImage *opencvTempImg    = (IplImage *) _yarpTempImg.getIplImage();
    //IplImage *opencvBothParts;//  = (IplImage *) imgBothParts.getIplImage(); // for image with top & bottom tool parts

    /* convert to HSV and get the Hue plane - to compute the histograms */
    cvCvtColor(opencvRawImg, opencvHSVImg, CV_BGR2HSV);
    cvSplit(opencvHSVImg, opencvHueImg, opencvSatImg, opencvValImg, NULL);
    IplImage *planes[] = { opencvHueImg }; //compute histogram of hue only

    /* for cvMaxMinLoc */
    double max_val, min_val;

    /* compute numLabels as the max value within opencvLabeledImg */
    cvMinMaxLoc(opencvLabeledImg, &min_val, &max_val, NULL, NULL, NULL);
    if(min_val != 0)
        yWarning("min_val of labeled image is different from zero !!!!");

    int numLabels = (int)max_val + 1;

    /* TODO: different selection criteria should be accepted here */
    _numObjects = selectObjects( opencvLabeledImg, opencvTempImg, numLabels, _minAreaThreshold);
    if(_numObjects > _maxObjects )
    {
        yDebug() << "more objects (" << _numObjects << ") than the permitted maximum. Only " << _maxObjects << " will be processed.";
        _numObjects = _maxObjects;
    }

    /* extract characteristics of objects */
    extractObj(opencvLabeledImg, _numObjects, _objDescTable);

    /* here, all objects have been segmented and are stored independently
    in _objDescTable structure. The fields mask, area, center and label are set. */

    /* contour extraction */
    for( int i=0; i < _numObjects; i++)
    {
        cvFindContours(_objDescTable[i].mask_image, 
                       _objDescTable[i].storage, 
                       &(_objDescTable[i].contours),
                       sizeof(CvContour),
                       CV_RETR_CCOMP, 
                       CV_CHAIN_APPROX_SIMPLE, 
                       cvPoint(0,0)
                       );

        if(_objDescTable[i].contours == NULL)
            yWarning("Something very wrong happened. Object without edges");

        _objDescTable[i].moments = cv::moments(cv::Mat(_objDescTable[i].mask_image), false);
    }

    /* compute histogram of each object */
    for(int i = 0; i < _numObjects; i++)
    {
        cvCalcHist(planes, _objDescTable[i].objHist, 0, _objDescTable[i].mask_image);
        float ohmax; // to normalize the object histogram
        cvGetMinMaxHistValue(_objDescTable[i].objHist, 0, &ohmax, 0, 0);
        cvConvertScale(_objDescTable[i].objHist->bins, _objDescTable[i].objHist->bins, ohmax ? 255. / ohmax : 0., 0);
    }

    /* compute saturation and value/intensity bounds for each object - not used anymore in POETICON++ */
    for(int i = 0; i < _numObjects; i++)
    {
        cvMinMaxLoc(opencvSatImg, &min_val, &max_val, 0, 0, _objDescTable[i].mask_image);
        _objDescTable[i].s_min = (int)min_val;
        _objDescTable[i].s_max = (int)max_val;
        cvMinMaxLoc(opencvValImg, &min_val, &max_val, 0, 0, _objDescTable[i].mask_image);
        _objDescTable[i].v_min = (int)min_val;
        _objDescTable[i].v_max = (int)max_val;
    }

    /* compute the roi for each object to set the target roi - not used anymore in POETICON++*/
    for(int i = 0; i < _numObjects; i++)
    {
        //MUST FIND A GOOD WAY TO DO IT
        //MAYBE USE FITELLIPSE ON THE CONTOUR
        //TRACKER COULD ALSO RECEIVE THE ANGLE
        //THE FOLLOWING INITIALIZATION IS JUST FOR EARLY TESTING
        _objDescTable[i].roi_height = 30;
        _objDescTable[i].roi_width = 30;
        _objDescTable[i].roi_x = _objDescTable[i].center.x - 15;
        _objDescTable[i].roi_y = _objDescTable[i].center.y - 15;
    }

    /*
     * SELECTING OBJECTS NOT FULFILLING CERTAIN CRITERIA
     *
     */

    // by default, all are valid
    for(int i = 0; i < _numObjects; i++ )
    {
        _objDescTable[i].valid = true;
    }

    // invalidate objects exceeding a maximum area
    for(int i = 0; i < _numObjects; i++ )
    {
        if(_objDescTable[i].area > _maxAreaThreshold)
        {
            _objDescTable[i].valid = false;  
        }
    }

    if(_invalidate_boundary_objects)
    {
        CvPoint* pt;
        for(int i = 0; i < _numObjects; i++ )
        {
            for(int j = 0; j < _objDescTable[i].contours->total; j++)
            {
                pt = (CvPoint*)cvGetSeqElem( _objDescTable[i].contours, j );

                if( (pt->x <= 1) || (pt->x >= (_w-2)) || (pt->y <= 1) || (pt->y >= (_h-2)))
                {
                    _objDescTable[i].valid = false;
                    break;
                }
            }
        }
    }
 
    /* Count the number of valid objects */
    int valid_objs = 0;
    for(int i = 0; i < _numObjects; i++)
        if( _objDescTable[i].valid)
            valid_objs++;

    // only for valid objects: approximate contours and compute shape descriptors (whole blobs).
    for( int i=0; i < _numObjects; i++)
    {
        if(_objDescTable[i].valid)
        {
            _objDescTable[i].affcontours = cvApproxPoly( 
                                           _objDescTable[i].contours,
                                           sizeof(CvContour),
                                           _objDescTable[i].storage,
                                           CV_POLY_APPROX_DP,
                                           cvContourPerimeter(_objDescTable[i].contours)*0.02,
                                           1);

            _objDescTable[i].convexhull = cvConvexHull2( _objDescTable[i].affcontours, _objDescTable[i].storage, CV_CLOCKWISE, 1 );

            // measurements of the contour
            _objDescTable[i].contour_area = fabs(cvContourArea( _objDescTable[i].affcontours, CV_WHOLE_SEQ ));
            _objDescTable[i].contour_perimeter = cvArcLength( _objDescTable[i].affcontours, CV_WHOLE_SEQ, 1 );
            _objDescTable[i].convex_perimeter = cvArcLength( _objDescTable[i].convexhull, CV_WHOLE_SEQ, 1 );
            _objDescTable[i].enclosing_rect = cvMinAreaRect2( _objDescTable[i].convexhull, _objDescTable[i].storage );
            _objDescTable[i].major_axis = (_objDescTable[i].enclosing_rect.size.width > _objDescTable[i].enclosing_rect.size.height ? _objDescTable[i].enclosing_rect.size.width : _objDescTable[i].enclosing_rect.size.height);
            _objDescTable[i].minor_axis = (_objDescTable[i].enclosing_rect.size.width > _objDescTable[i].enclosing_rect.size.height ? _objDescTable[i].enclosing_rect.size.height : _objDescTable[i].enclosing_rect.size.width);
            _objDescTable[i].rect_area = _objDescTable[i].major_axis*_objDescTable[i].minor_axis;
            _objDescTable[i].bounding_rect = cvBoundingRect(_objDescTable[i].contours,0);
        
            CvPoint2D32f center;
            float radius;
            cvMinEnclosingCircle( _objDescTable[i].affcontours, &center, &radius );

            // shape descriptors
            if(_objDescTable[i].contour_perimeter > 0)
                _objDescTable[i].convexity = _objDescTable[i].convex_perimeter/_objDescTable[i].contour_perimeter;
            else
                _objDescTable[i].convexity = 0;

            if(_objDescTable[i].major_axis > 0)
                _objDescTable[i].eccentricity = _objDescTable[i].minor_axis/_objDescTable[i].major_axis;
            else
                _objDescTable[i].eccentricity = 0;

            if(_objDescTable[i].contour_perimeter > 0)
                _objDescTable[i].compactness = _objDescTable[i].contour_area/(_objDescTable[i].contour_perimeter*_objDescTable[i].contour_perimeter);

            if( radius > 0)
                _objDescTable[i].circleness = _objDescTable[i].contour_area/(3.1415*radius*radius);

            if(_objDescTable[i].rect_area > 0)
                _objDescTable[i].squareness = _objDescTable[i].contour_area/_objDescTable[i].rect_area;

            /* POETICON++ */
            if (_objDescTable[i].eccentricity>0.0 && _objDescTable[i].eccentricity<1.0)
                _objDescTable[i].elongatedness = 1.0 - _objDescTable[i].eccentricity + _objDescTable[i].compactness*5;
            else
                _objDescTable[i].elongatedness = 0.0;

            // length descriptor, Feb. 2016
            if (_objDescTable[i].major_axis>0.0)
                _objDescTable[i].length = _objDescTable[i].major_axis;
            else
                _objDescTable[i].length = 0.0;
        }
    }

    // output shape descriptors (of whole blobs)
    Bottle &affbot = _affDescriptorOutputPort.prepare();
    affbot.clear();
    affbot.addInt(valid_objs);
    /* output affordance descriptors */
    for(int i = 0; i < _numObjects; i++)
    {
        if( _objDescTable[i].valid)
        {
            Bottle &objbot = affbot.addList();
            objbot.clear();
            //objbot.addInt(i);

            // note: poeticonManager calls these roi.x, roi.y
            double x = _objDescTable[i].enclosing_rect.center.x;
            double y = _objDescTable[i].enclosing_rect.center.y;
            double w = _objDescTable[i].enclosing_rect.size.width;
            double h = _objDescTable[i].enclosing_rect.size.height;

            if (normalized_coords)
            {
                // RobotCub
                double norm_x = (x-_w/2)/(_w/2); //between -1 and 1 
                double norm_y = (y-_h/2)/(_h/2); //between -1 and 1 
                double norm_w = (w/_w);        //between 0 and 1 
                double norm_h = (h/_h);           //between 0 and 1     
                /*0*/objbot.addDouble(norm_x);
                /*1*/objbot.addDouble(-norm_y);
                /*2*/objbot.addDouble(norm_w);
                /*3*/objbot.addDouble(norm_h);
            } else {
                // First-MM,POETICON++
                /*0*/objbot.addDouble(x);
                /*1*/objbot.addDouble(y);
                /*2*/objbot.addDouble(w);
                /*3*/objbot.addDouble(h);
            }

            /*4*/objbot.addDouble(_objDescTable[i].enclosing_rect.angle);
            double br_x, br_y, br_w, br_h;
            br_x = _objDescTable[i].bounding_rect.x;
            br_y = _objDescTable[i].bounding_rect.y;
            br_w = _objDescTable[i].bounding_rect.width;
            br_h = _objDescTable[i].bounding_rect.height;
            //estimate of the point over the table
            //coordinates of the bounding rectangle
            /*5*/objbot.addDouble(br_x+br_w/2);
            /*6*/objbot.addDouble(br_y+br_h);

            // TODO: increase number of bins (this will break backwards compatibility)

            /*7*/objbot.addDouble((double)cvQueryHistValue_1D(_objDescTable[i].objHist, 0));
            /*8*/objbot.addDouble((double)cvQueryHistValue_1D(_objDescTable[i].objHist, 1));
            /*9*/objbot.addDouble((double)cvQueryHistValue_1D(_objDescTable[i].objHist, 2));
            /*10*/objbot.addDouble((double)cvQueryHistValue_1D(_objDescTable[i].objHist, 3));
            /*11*/objbot.addDouble((double)cvQueryHistValue_1D(_objDescTable[i].objHist, 4));
            /*12*/objbot.addDouble((double)cvQueryHistValue_1D(_objDescTable[i].objHist, 5));
            /*13*/objbot.addDouble((double)cvQueryHistValue_1D(_objDescTable[i].objHist, 6));
            /*14*/objbot.addDouble((double)cvQueryHistValue_1D(_objDescTable[i].objHist, 7));
            /*15*/objbot.addDouble((double)cvQueryHistValue_1D(_objDescTable[i].objHist, 8));
            /*16*/objbot.addDouble((double)cvQueryHistValue_1D(_objDescTable[i].objHist, 9));
            /*17*/objbot.addDouble((double)cvQueryHistValue_1D(_objDescTable[i].objHist, 10));
            /*18*/objbot.addDouble((double)cvQueryHistValue_1D(_objDescTable[i].objHist, 11));
            /*19*/objbot.addDouble((double)cvQueryHistValue_1D(_objDescTable[i].objHist, 12));
            /*20*/objbot.addDouble((double)cvQueryHistValue_1D(_objDescTable[i].objHist, 13));
            /*21*/objbot.addDouble((double)cvQueryHistValue_1D(_objDescTable[i].objHist, 14));
            /*22*/objbot.addDouble((double)cvQueryHistValue_1D(_objDescTable[i].objHist, 15));
            /*23*/objbot.addDouble((double)_objDescTable[i].contour_area);
            /*24*/objbot.addDouble((double)_objDescTable[i].convexity);
            /*25*/objbot.addDouble((double)_objDescTable[i].eccentricity);
            /*26*/objbot.addDouble((double)_objDescTable[i].compactness);
            /*27*/objbot.addDouble((double)_objDescTable[i].circleness);
            /*28*/objbot.addDouble((double)_objDescTable[i].squareness);
            // for POETICON++
            /*29*/objbot.addDouble((double)_objDescTable[i].length);

            // moments
            //const double MOM_NORMALIZATION = 1000.0;
            // /*30*/objbot.addDouble((double)_objDescTable[i].moments.nu20 * MOM_NORMALIZATION);
            // /*31*/objbot.addDouble((double)_objDescTable[i].moments.nu11 * MOM_NORMALIZATION);
            // /*32*/objbot.addDouble((double)_objDescTable[i].moments.nu02 * MOM_NORMALIZATION);
            // /*33*/objbot.addDouble((double)_objDescTable[i].moments.nu30 * MOM_NORMALIZATION);
            // /*34*/objbot.addDouble((double)_objDescTable[i].moments.nu21 * MOM_NORMALIZATION);
            // /*35*/objbot.addDouble((double)_objDescTable[i].moments.nu12 * MOM_NORMALIZATION);
            // /*36*/objbot.addDouble((double)_objDescTable[i].moments.nu03 * MOM_NORMALIZATION);
        }
    }
    _affDescriptorOutputPort.setEnvelope(writestamp);
    _affDescriptorOutputPort.write();

    // output Tool-Affordance descriptors = descriptors of two halves of each elongated blob

    Bottle &toolAffBot = toolAffDescriptorOutputPort.prepare();
    toolAffBot.clear();
    toolAffBot.addInt(valid_objs);

    for(int i = 0; i < _numObjects; i++)
    {
        // it is best NOT to check against the elongatedness threshold here.
        // this way, we guarantee that both affordance ports (whole object & tool parts)
        // carry information about the same N visible blobs, in the same order.
        // poeticonManager requires this, because at the same instant it reads
        // information from both affDescriptor:o (whole object) and toolAffDescriptor:o
        // (tool parts) and the two ports need to be coherent.
        //
        // in the future, this can be changed.
        if( _objDescTable[i].valid )
        {
            // next we will compute the two halves of the i'th encl. rectangle,
            // corresponding to object top and object bottom

            //cout << "blob " << i << endl;

            // force width to be the smaller dimension, height to be the larger one
            if (_objDescTable[i].enclosing_rect.size.width > _objDescTable[i].enclosing_rect.size.height)
            {
                //cout << "WARNING: rotating rectangle by 90 deg" << endl;
                swap(_objDescTable[i].enclosing_rect.size.width, _objDescTable[i].enclosing_rect.size.height);
                _objDescTable[i].enclosing_rect.angle += 90.;
            }

            float cx = _objDescTable[i].enclosing_rect.center.x,
                  cy = _objDescTable[i].enclosing_rect.center.y;

            float wi = _objDescTable[i].enclosing_rect.size.width,
                  he = _objDescTable[i].enclosing_rect.size.height,
                  an = _objDescTable[i].enclosing_rect.angle;

            float ca = cos(an / 180.0 * CV_PI),
                  sa = sin(an / 180.0 * CV_PI);

            //cout << "\n center = [" << cx << " " << cy << "]";
            //cout << ", wi=" << wi << ", he=" << he;
            //cout << ", an=" << an << ", ca=" << ca << ", sa=" << sa << endl;

            // draw whole object encl. rectangle (bounding box) and tools if high elongatedness
            //drawBox(opencvViewImg, _objDescTable[i].enclosing_rect, CV_RGB(255,255,255)); // filled
            //drawBoxContour(opencvViewImg, _objDescTable[i].enclosing_rect, CV_RGB(255,255,255), 1); // non-filled
            if (_objDescTable[i].elongatedness > elongatedness_thr )
            {
                CvPoint min_pt1 = cvPoint( cvRound( cx+ca*wi/2. ), cvRound( cy+sa*wi/2. ) );
                CvPoint min_pt2 = cvPoint( cvRound( cx-ca*wi/2. ), cvRound( cy-sa*wi/2. ) );
                //cvLine(opencvViewImg, min_pt1, min_pt2, CV_RGB(255,255,255), 1, CV_AA, 0);
            }

            if (wi==1 || he==1)
                yWarning("enclosing_rectangle of blob %d is one-dimensional", i);

            cv::Size2f half_size(wi,he); // initially, same size as whole object encl. rect
            // force width to be the smaller dimension, height to be the larger one
            if (wi < he)
            {
                half_size.height = half_size.height/2.;
                //cout << "OK: height>width, thus I made splitting of height for cropping obj parts" << endl;
            }
            else
            {
                // this should never happen in the first place
                yWarning("width>height... need to swap them!");
                // to complete
            }

            // centers of halves calculated from cx,cy, in whole image coordinates
            top_center = cv::Point2f( cvRound(cx+sa*half_size.height/2.), cvRound(cy-ca*half_size.height/2.) );
            bot_center = cv::Point2f( cvRound(cx-sa*half_size.height/2.), cvRound(cy+ca*half_size.height/2.) );

            // top-left corners of upright halves, in whole image coordinates
            cv::Point2f top_tl2, bot_tl2;
            top_tl2 = cv::Point2f( cvRound(cx-half_size.width/2.), cvRound(cy-half_size.height) );
            bot_tl2 = cv::Point2f( cvRound(cx-half_size.width/2.), cvRound(cy) );

            // top-right corners of upright halves, in whole image coordinates
            //cv::Point2f top_tr2, bot_tr2;
            //top_tr2 = cv::Point2f( cvRound(cx+half_size.width/2.), cvRound(cy-half_size.height) );
            //bot_tr2 = cv::Point2f( cvRound(cx+half_size.width/2.), cvRound(cy) );

            // prints value then memory address?! fixed
            //cout << "top_center.x = " << top_center.x << endl;

            // get transformation matrix provided by rotation "an" and no scaling factor
            cv::Mat M = cv::getRotationMatrix2D(_objDescTable[i].enclosing_rect.center, an, 1.0);

            cv::Mat src = cv::cvarrToMat(opencvBinaryImg);
            // now src has 3 channels

            cv::cvtColor(src, src, CV_BGR2GRAY, 1);
            // now src has 1 channel - for findContours, which needs 8uC1 or 32sC1

            // perform affine transformation (rotation)
            cv::Mat rotated(src.size(), src.type()); // output
            cv::warpAffine(src, rotated, M, src.size(), cv::INTER_CUBIC);

            //cv::imshow("rotated", rotated);
            //cvWaitKey(0);

            // rotate other points inside blob parts, too:

            // top part
            cv::Point2f top_center_rot;
            top_center_rot.x =  M.at<double>(0,0)*(top_center.x-cx) + M.at<double>(0,1)*(top_center.y-cy) + cx;
            top_center_rot.y = -M.at<double>(1,0)*(top_center.y-cy) + M.at<double>(1,1)*(top_center.x-cx) + cy;

            // bottom part
            cv::Point2f bot_center_rot;
            bot_center_rot.x =  M.at<double>(0,0)*(bot_center.x-cx) + M.at<double>(0,1)*(bot_center.y-cy) + cx;
            bot_center_rot.y = -M.at<double>(1,0)*(bot_center.y-cy) + M.at<double>(1,1)*(bot_center.x-cx) + cy;

            // shift crop centers vertically by halfObjHeight/2, i.e., wholeObjHeight/4
            top_center_rot.y -= half_size.height/2.; // moving up in image
            bot_center_rot.y += half_size.height/2.; // moving down in image

            // enlarge crop area to capture pixels around blob borders that could have been missed
            cv::Size2f crop_size(half_size.width,half_size.height);
            crop_size.width  += HORIZ_CROPAREA_SHIFT; // along x
            crop_size.height += VERT_CROPAREA_SHIFT;  // small along y, or we'd capture too much of other part

            // crop resulting top image - Tool Top
            cv::Mat topRectCropped;
            cv::getRectSubPix(rotated, crop_size, top_center_rot, topRectCropped);

            //cv::imshow("topRectCropped", topRectCropped);
            //cvWaitKey(0);

            // crop resulting bottom image - Tool Bottom
            cv::Mat botRectCropped;
            cv::getRectSubPix(rotated, crop_size, bot_center_rot, botRectCropped);

            //cv::imshow("botRectCropped", botRectCropped);
            //cvWaitKey(0);

            // intermezzo:
            // because findContours will modify input images, let's display them before that,
            // by creating an image with both parts stacked vertically
            // FIXME
            /*
            if (i==1) // display only blob #1 parts
            {
                matBothParts = cv::Mat(2*botRectCropped.rows, botRectCropped.cols, CV_8UC3);

                matBothParts.adjustROI(0, -topRectCropped.rows, 0, 0); // adapt ROI to receive topRect
                //cout << "1. row size " << matBothParts.rows << endl;
                topRectCropped.copyTo(matBothParts);
                matBothParts.adjustROI(-topRectCropped.rows, topRectCropped.rows, 0, 0); // restore; adapt ROI to receive botRect
                //cout << "2. row size " << matBothParts.rows << endl;
                botRectCropped.copyTo(matBothParts);
                matBothParts.adjustROI(topRectCropped.rows, 0, 0, 0); // restore
                //cout << "3. row size " << matBothParts.rows << endl;
                if (matBothParts.rows>0 && matBothParts.rows>0)
                {
                    cv::imshow("matBothParts", matBothParts);
                    cvWaitKey(0);
                }
                //cv::cvtColor(matBothParts, matBothParts, CV_BGR2RGB);
                ImageOf<PixelBgr> imgBothParts;
                imgBothParts.resize(matBothParts.cols, matBothParts.rows);
                IplImage tempBothParts = matBothParts;
                cvCopyImage( &tempBothParts, (IplImage *)imgBothParts.getIplImage() );
                bothPartsImgOutputPort.write(imgBothParts); // Port
            }
            */

            // continue processing of top part, whose image is now safe to modify

            vector<vector<cv::Point> > top_cnt;
            vector<cv::Vec4i> top_hrch;
            topRectCropped.convertTo(topRectCropped, CV_8UC1); // redundant?
            cv::findContours(topRectCropped, top_cnt, top_hrch, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);

            double top_area,
                   top_convexity,
                   top_eccentricity,
                   top_compactness,
                   top_circleness,
                   top_squareness,
                   top_elongatedness,
                   top_length;

            vector<cv::Point> top_hull;

            if (top_cnt.size()==0)
            {
                cout << "WARNING: blob "<< i << ", tool top has zero contours" << endl;
                continue;
            }

            double top_largest_area = 0;
            int top_largest_cnt_index = 0;

            // find index of contour with largest area
            for( int c = 0; c < top_cnt.size(); c++ )
            {
                double curr_area = cv::contourArea(top_cnt[c], false);
                //cout << "\tcontour #" << c << ": area=" << curr_area << endl;

                if (curr_area > top_largest_area)
                {
                    top_largest_area = curr_area;
                    top_largest_cnt_index = c;
                }
            }
            top_area = cv::contourArea(top_cnt[top_largest_cnt_index]);

            // compute shape descriptors for largest contour
            cv::convexHull(top_cnt[top_largest_cnt_index], top_hull);
            double top_perimeter = cv::arcLength(top_cnt[top_largest_cnt_index], true);
            double top_convex_perimeter = cv::arcLength(top_hull, true);
            cv::RotatedRect top_enclosing_rect = cv::minAreaRect(top_cnt[top_largest_cnt_index]);
            double top_major_axis = (top_enclosing_rect.size.width > top_enclosing_rect.size.height ? top_enclosing_rect.size.width : top_enclosing_rect.size.height);
            double top_minor_axis = (top_enclosing_rect.size.width > top_enclosing_rect.size.height ? top_enclosing_rect.size.height : top_enclosing_rect.size.width);
            double top_rect_area = top_major_axis*top_minor_axis;
            cv::Rect top_bounding_rect = cv::boundingRect(top_cnt[top_largest_cnt_index]);
            cv::Point2f top_circle_center;
            float top_circle_radius = 0;
            cv::minEnclosingCircle(top_cnt[top_largest_cnt_index], top_circle_center, top_circle_radius);

            if (top_perimeter > 0)
                top_convexity = top_convex_perimeter / top_perimeter;
            else
                top_convexity = 0;

            if (top_major_axis > 0)
                top_eccentricity = top_minor_axis / top_major_axis;
            else
                top_eccentricity = 0;

            if (top_perimeter > 0)
                top_compactness = top_area / (top_perimeter*top_perimeter);

            if (top_circle_radius > 0)
                top_circleness = top_area / (CV_PI*top_circle_radius*top_circle_radius);

            if (top_rect_area > 0)
                top_squareness = top_area / top_rect_area;

            if (top_eccentricity > 0)
                top_elongatedness = 1 - top_eccentricity + top_compactness*5;
            else
                top_elongatedness = 0;

            if (top_major_axis > 0.0)
                top_length = top_major_axis;
            else
                top_length = 0.0;

            // top-left corners of the two sub-rectangles expressed in whole image frame
            cv::Point2f top_tl( cvRound(cx+sa*half_size.height-ca*half_size.width/2.), cvRound(cy-ca*half_size.height-sa*half_size.width/2.) );
            cv::Point2f bot_tl( cvRound(cx-ca*half_size.width/2.), cvRound(cy-sa*half_size.width/2.) );
            //cvCircle(opencvViewImg, top_tl, 3, CV_RGB(255,255,255), -1, CV_AA, 0);
            //cvCircle(opencvViewImg, bot_tl, 3, CV_RGB(255,255,255), -1, CV_AA, 0);

            // continue processing of bottom part, whose image is now safe to modify

            //cv::Mat botRectCroppedCopy = botRectCropped.clone(); // for visual debug

            vector<vector<cv::Point> > bot_cnt;
            vector<cv::Vec4i> bot_hrch;
            botRectCropped.convertTo(botRectCropped, CV_8UC1); // redundant?
            cv::findContours(botRectCropped, bot_cnt, bot_hrch, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);

            double bot_area,
                   bot_convexity,
                   bot_eccentricity,
                   bot_compactness,
                   bot_circleness,
                   bot_squareness,
                   bot_elongatedness,
                   bot_length;

            vector<cv::Point> bot_hull;

            if (bot_cnt.size()==0)
            {
                yWarning("blob %d, tool bottom has zero contours", i);
                continue;
            }

            double bot_largest_area = 0;
            int bot_largest_cnt_index = 0;

            // find index of contour with largest area
            for( int c = 0; c < bot_cnt.size(); c++ )
            {
                double curr_area = cv::contourArea(bot_cnt[c], false);
                //cout << "\tcontour #" << c << ": area=" << curr_area << endl;

                if (curr_area > bot_largest_area)
                {
                    bot_largest_area = curr_area;
                    bot_largest_cnt_index = c;
                }
            }
            bot_area = cv::contourArea(bot_cnt[bot_largest_cnt_index]);

            // compute shape descriptors for largest contour
            cv::convexHull(bot_cnt[bot_largest_cnt_index], bot_hull);
            double bot_perimeter = cv::arcLength(bot_cnt[bot_largest_cnt_index], true);
            double bot_convex_perimeter = cv::arcLength(bot_hull, true);
            cv::RotatedRect bot_enclosing_rect = cv::minAreaRect(bot_cnt[bot_largest_cnt_index]);
            double bot_major_axis = (bot_enclosing_rect.size.width > bot_enclosing_rect.size.height ? bot_enclosing_rect.size.width : bot_enclosing_rect.size.height);
            double bot_minor_axis = (bot_enclosing_rect.size.width > bot_enclosing_rect.size.height ? bot_enclosing_rect.size.height : bot_enclosing_rect.size.width);
            double bot_rect_area = bot_major_axis*bot_minor_axis;
            cv::Rect bot_bounding_rect = cv::boundingRect(bot_cnt[bot_largest_cnt_index]);
            //cout << "blob " << i << " rect corners: " << bot_bounding_rect.tl() << ", " << bot_bounding_rect.br() << endl;
            cv::Point2f bot_circle_center;
            float bot_circle_radius;
            cv::minEnclosingCircle(bot_cnt[bot_largest_cnt_index], bot_circle_center, bot_circle_radius);

            if (bot_perimeter > 0)
                bot_convexity = bot_convex_perimeter / bot_perimeter;
            else
                bot_convexity = 0;

            if (bot_major_axis > 0)
                bot_eccentricity = bot_minor_axis / bot_major_axis;
            else
                bot_eccentricity = 0;

            if (bot_perimeter > 0)
                bot_compactness = bot_area / (bot_perimeter*bot_perimeter);

            if (bot_circle_radius > 0)
                bot_circleness = bot_area / (CV_PI*bot_circle_radius*bot_circle_radius);

            if (bot_rect_area > 0)
                bot_squareness = bot_area / bot_rect_area;

            if (bot_eccentricity > 0)
                bot_elongatedness = 1 - bot_eccentricity + bot_compactness*5;
            else
                bot_elongatedness = 0;

            if (bot_major_axis > 0.0)
                bot_length = bot_major_axis;
            else
                bot_length = 0.0;

            //cv::rectangle(botRectCroppedCopy, bot_bounding_rect, CV_BGR(0,0,255), 2, CV_AA, 0);
            //cv::imshow("bot", botRectCroppedCopy);
            //cv::waitKey(0);

            // consider the centers of newly-estimated (upright) rectangles as the new top, bottom centers
            cv::Point2f top_new, bot_new;

            // compute top_new, bot_new as respective convex hull centres of gravity - Vadim 2015-03-14
            cv::Moments top_mo = cv::moments(top_hull);
            top_new = cv::Point( top_mo.m10/top_mo.m00, top_mo.m01/top_mo.m00 );
            cv::Moments bot_mo = cv::moments(bot_hull);
            bot_new = cv::Point( bot_mo.m10/bot_mo.m00, bot_mo.m01/bot_mo.m00 );
            // transform back from (upright) crop frame to whole image frame
            top_new.x += top_center.x;
            top_new.x -= top_bounding_rect.width/2.;
            top_new.x -= HORIZ_CROPAREA_SHIFT;

            top_new.y += top_center.y;
            top_new.y -= top_bounding_rect.height/2.;
            top_new.y -= VERT_CROPAREA_SHIFT;

            bot_new.x += bot_center.x;
            bot_new.x -= bot_bounding_rect.width/2.;
            bot_new.x -= HORIZ_CROPAREA_SHIFT;

            bot_new.y += bot_center.y;
            bot_new.y -= bot_bounding_rect.height/2.;
            bot_new.y -= VERT_CROPAREA_SHIFT;

            if (_objDescTable[i].elongatedness > elongatedness_thr)
            {
                // draw centers of halves of original rectangle (whole object)
                //cvCircle(opencvViewImg, top_center, 2, CV_BGR(0,255,0), -1, CV_AA, 0);
                //cvCircle(opencvViewImg, bot_center, 2, CV_BGR(255,0,0), -1, CV_AA, 0);

                // draw centers of newly-estimated part rectangles, more likely to belong to contour
                cvCircle(opencvViewImg, top_new, 5, CV_RGB(0,255,0), -1, CV_AA, 0);
                cvCircle(opencvViewImg, bot_new, 5, CV_RGB(0,255,0), -1, CV_AA, 0);
            }

            // draw centre of mass (converted to integer)
            cv::Point com(_objDescTable[i].moments.m10/_objDescTable[i].moments.m00,
                          _objDescTable[i].moments.m01/_objDescTable[i].moments.m00);
            // point-in-contour test
            double belongs;
            belongs = cv::pointPolygonTest(cv::cvarrToMat(_objDescTable[i].contours), com, false);
            if (belongs < 0.0)
            {
                yWarning("blob %d, centroid outside contour", i);
            }
            //cvCircle(opencvViewImg, com, 5, CV_BGR(0,0,255), -1, CV_AA, 0);

            // new list for current object
            Bottle &bothPartsBot = toolAffBot.addList();
            // sublist for current object's top half
            Bottle &topBot = bothPartsBot.addList();
            topBot.clear();
            //topBot.addDouble(top_center.x);
            //topBot.addDouble(top_center.y);
            topBot.addDouble(top_new.x);
            topBot.addDouble(top_new.y);
            topBot.addDouble(top_area);
            topBot.addDouble(top_convexity);
            topBot.addDouble(top_eccentricity);
            topBot.addDouble(top_compactness);
            topBot.addDouble(top_circleness);
            topBot.addDouble(top_squareness);
            topBot.addDouble(top_length);

            // sublist for current object's bottom half
            Bottle &botBot = bothPartsBot.addList();
            botBot.clear();
            //botBot.addDouble(bot_center.x);
            //botBot.addDouble(bot_center.y);
            botBot.addDouble(bot_new.x);
            botBot.addDouble(bot_new.y);
            botBot.addDouble(bot_area);
            botBot.addDouble(bot_convexity);
            botBot.addDouble(bot_eccentricity);
            botBot.addDouble(bot_compactness);
            botBot.addDouble(bot_circleness);
            botBot.addDouble(bot_squareness);
            botBot.addDouble(bot_length);
        } // end if(..valid)
    } // end for(..numObjects)
    toolAffDescriptorOutputPort.setEnvelope(writestamp);
    toolAffDescriptorOutputPort.write(); 

    // drawing of overlay edges
    for( int i=0; i < _numObjects; i++)
    {
        if( _objDescTable[i].valid ) 
        //if( _objDescTable[i].valid && _objDescTable[i].elongatedness < elongatedness_thr )
        {
            cvDrawContours(
                opencvViewImg,
                _objDescTable[i].contours,
                //CV_RGB(0,0,255),
                CV_RGB(100,255,255),
                CV_RGB(100,255,255),
                0, // 0: don't draw holes. was: 2
                2,
                CV_AA,
                cvPoint(0, 0)       // ROI offset
            );

            if (_objDescTable[i].elongatedness > elongatedness_thr)
            {
                cvDrawContours(
                    opencvViewImg,
                    _objDescTable[i].contours,
                    CV_RGB(0,0,255),
                    CV_RGB(0,0,255),
                    0, // 0: don't draw holes. was: 2
                    2,
                    CV_AA,
                    cvPoint(0, 0)       // ROI offset
                );
            }

            //Draw also center point in table for debug
            //double x = _objDescTable[i].enclosing_rect.center.x;
            //double y = _objDescTable[i].enclosing_rect.center.y + _objDescTable[i].enclosing_rect.size.height/2;
            //CvRect r = cvBoundingRect(_objDescTable[i].contours,0);
            //cvCircle(opencvViewImg, cvPoint(r.x+r.width/2, r.y+r.height), 5, cvScalar(255,255,255,255), 3 );
            //cvCircle(opencvViewImg, cvPoint(r.x+r.width/2, r.y+r.height),10,CV_RGB(255,0,0),2,CV_AA,0);
            //double br_x, br_y, br_w, br_h;
            //br_x = _objDescTable[i].bounding_rect.x;
            //br_y = _objDescTable[i].bounding_rect.y;
            //br_w = _objDescTable[i].bounding_rect.width;
            //br_h = _objDescTable[i].bounding_rect.height;
            //cvCircle(opencvViewImg, cvPoint(br_x+br_w/2, br_y+br_h), 5, cvScalar(255,255,255,255), 3 );
            //cvCircle(opencvViewImg, cvPoint(_objDescTable[i].roi_x, _objDescTable[i].roi_y), 5, cvScalar(255,255,255,255), 3 );

        }
        else // invalid objects
        {
            if (_objDescTable[i].area > _maxAreaThreshold)
                continue; // skip drawing this; next iteration

            cvDrawContours(
                opencvViewImg,
                _objDescTable[i].contours,
                //CV_RGB(128,128,128),
                CV_RGB(230,216,173),
                CV_RGB(230,216,173),
                0, // 0: don't draw holes. was: 2
                2,
                CV_AA,
                cvPoint(0, 0) // ROI offset
            );
        }
    }

    // output image to view results
    if (opencvViewImg != NULL)
    {
        // convert from OpenCV to yarp format
        cvCopyImage( opencvViewImg, (IplImage *)_yarpViewImg.getIplImage() );
    }
    ImageOf<PixelBgr> &yarpViewOutputImg = _viewImgOutputPort.prepare();
    yarpViewOutputImg = _yarpViewImg;
    _viewImgOutputPort.setEnvelope(writestamp);
    _viewImgOutputPort.write();

    /*
    if (opencvRawImg != NULL)
        cvReleaseImage(&opencvRawImg); // TODO: segfault, fix
    if (opencvHSVImg != NULL)
        cvReleaseImage(&opencvHSVImg);
    if (opencvHueImg != NULL)
        cvReleaseImage(&opencvHueImg);
    if (opencvSatImg != NULL)
       cvReleaseImage(&opencvSatImg);
    if (opencvValImg != NULL)
        cvReleaseImage(&opencvValImg);
    if (opencvBinaryImg != NULL)
        cvReleaseImage(&opencvBinaryImg);
    if (opencvLabeledImg != NULL)
        cvReleaseImage(&opencvLabeledImg);
    if (opencvViewImg != NULL)
        cvReleaseImage(&opencvViewImg);
    if (opencvTempImg != NULL)
        cvReleaseImage(&opencvTempImg);
    if (opencvBothParts != NULL)
        cvReleaseImage(&opencvBothParts);
    //cvReleaseImage(&(planes[0]));
    */

    if (valid_objs > 0)
        yInfo("computed descriptors of %d valid blobs out of %d", valid_objs, _numObjects);

      return true;
} // end updateModule

double BlobDescriptorModule::getPeriod() {
    return 0.0;
}

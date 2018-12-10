/**
 *
 * @ingroup icub_module
 *
 * \defgroup icub_sequentialLabeller sequentialLabeller
 *
 * Compute a labelled image from a binary segmented image (see below for
 * definitions).
 *
 * \section intro_sec Description
 *
 * Use this module in combination with your favourite segmentation algorithm
 * (usually blobSpotter or lumaChroma+blobExtractor or EDISON), obtain a
 * labelled image suitable for computing shape features (with blobDescriptor)
 * and carry on from there (object affordances).
 *
 * Definitions:
 *
 * Binary image: an image containing 0=background, 1=blobs
 * Labelled image: an image containing 0=background, 1=object, 2=another object,
 * 3=another object, etc.
 *
 * \section lib_sec Libraries
 *
 * YARP, OpenCV.
 *
 * <b>Command-line Parameters</b>
 *
 * - --from name_of_configuration_file.ini (default sequentialLabeller.ini)
 * - --context name_of_context_dir (default sequentialLabeller/conf)
 *
 * <b>Configuration File Parameters</b>
 *
 * - name: prefix of ports created by this module (default /sequentialLabeler)
 * - binary_image_input_port (default <name>binImg:i)
 * - labeled_image_output_port (default <name>/labeledImg:o)
 *
 * \section portsa_sec Ports Accessed
 *
 * - <tt>binary:o</tt> \n
 *   Binary image port, previously created by a segmentation application
 *
 * \section portsc_sec Ports Created
 *
 * <b>Input ports</b>
 *
 * - <tt>/sequentialLabeller/binImg:i</tt> \n
 *   Binary image input port
 *
 * <b>Output ports</b>
 *
 * - <tt>/sequentialLabeller/labeledImg:o</tt> \n
 *   Labelled image output port
 *
 * <b>Input/Output ports</b>
 *
 * None
 *
 * <b>Port Types</b>
 *
 * - <tt>BufferedPort<ImageOf<PixelMono> > binaryImgInputPort</tt>
 * - <tt>BufferedPort<ImageOf<PixelMono> > labeledImgOutputPort</tt>
 *
 * \section in_data_sec Input Data Files
 *
 * None
 *
 * \section out_data_sec Output Data Files
 *
 * None
 *
 * \section conf_file_sec Configuration Files
 *
 * sequentialLabeller.ini (optional)
 *
 * \section tested_os_sec Tested OS
 *
 * Linux
 *
 * \section example_sec Example Instantiation of the Module
 * - <tt>sequentialLabeller</tt>\n
 *
 * \author Giovanni Saponaro <gsaponaro@isr.ist.utl.pt>
 *
 * Copyright (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 */

#ifndef SEQUENTIAL_LABELLER_MODULE_H
#define SEQUENTIAL_LABELLER_MODULE_H

#include <string>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <yarp/os/BufferedPort.h>
#include <yarp/os/Log.h>
#include <yarp/os/Network.h>
#include <yarp/os/Port.h>
#include <yarp/os/RFModule.h>
#include <yarp/os/Stamp.h>
#include <yarp/sig/Image.h>

#include "cvSeqLabel.h"

using namespace std;
using namespace yarp::os;
using namespace yarp::sig;

class SequentialLabellerModule : public RFModule
{
    /* private class variables and module parameters */
    string                            _moduleName;

    string                            binaryImgInputPortName;
    string                            labeledImgOutputPortName;
    /* yarp image pointers to access image ports */
    ImageOf<PixelMono>               *yarpBinaryInputPtr;
    /* yarp internal image buffers */
    ImageOf<PixelMono>                yarpBinaryImg;
    ImageOf<PixelMono>                yarpLabeledImg;

    int                               w, h;
    CvSize                            sz;

    BufferedPort<ImageOf<PixelMono> > binaryImgInputPort;
    BufferedPort<ImageOf<PixelMono> > labeledImgOutputPort;

public:
    virtual bool configure(ResourceFinder &rf); /* configure module parameters, return true if successful */
    virtual bool interruptModule();             /* interrupt, e.g., ports */
    virtual bool close();                       /* close and shut down module */
    virtual bool updateModule();
    virtual double getPeriod();
};

#endif // SEQUENTIAL_LABELLER_MODULE_H

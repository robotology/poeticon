/*
 * Copyright (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright (C) 2009 RobotCub Consortium, European Commission FP6 Project IST-004370
 * Author: Giovanni Saponaro, Ivana Cingovska, Alexandre Bernardino
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 */

/**
 * @ingroup icub_module
 *
 * \defgroup icub_blobDescriptor blobDescriptor
 *   @ingroup icub_contrib_modules
 *
 * Compute a list of shape descriptors from a segmentation module.
 *
 * The resulting descriptors can then be used in a number of applications:
 * - reasoning about object affordances (action possibilities available in the
 *   environment);
 * - object recognition;
 * - computation of optimal grasping points for robots.
 *
 * NOTE: this module was heavily changed for the POETICON++ 
 *       project in 2014. Main changes: \n
 *       1. new required input port: binary image. \n
 *       2. new output port: descriptors of object top part and object bottom
 *       part, to be used for tool affordance learning. \n
 *       3. removed userSelection and trackerInit functionalities. \n
 *       If this does not work for you, please contact the author.
 *
 * \section lib_sec Libraries
 *
 * OpenCV, YARP, iCub.
 *
 * \section parameters_sec Parameters
 *
 * <b>Command-line Parameters</b>
 *
 * The following key-value pairs can be specified as command-line parameters by
 * prefixing -- to the key (e.g., <tt>--from file.ini</tt>). The value part can
 * be changed; default values are shown below.
 *
 * - <tt>from blobDescriptor.ini</tt> \n
 *   Configuration file
 *
 * - <tt>context blobDescriptor/conf</tt> \n
 *   Path of context containing the configuration file
 *
 * - <tt>name /blobDescriptor</tt> \n
 *   Root name of the module, used to form the stem of module port names
 *
 * <b>Configuration File Parameters</b>
 *
 * The following key-value pairs can be specified as parameters in the
 * configuration file (they can also be specified as command-line parameters if
 * you so wish).
 *
 * - <tt>conf_port /blobDescriptor/conf</tt> \n
 *   Complete configuration and message handling port name (currently only
 *   recognizes "quit")
 *
 * - <tt>raw_image_input_port /blobDescriptor/rawImg:i</tt> \n
 *   Complete raw image input port name
 *
 * - <tt>binary_image_input_port /blobDescriptor/binImg:i</tt> \n
 *   Complete binary image input port name
 *
 * - <tt>labeled_image_input_port /blobDescriptor/labeledImg:i</tt> \n
 *   Complete labeled image input port name
 *
 * - <tt>raw_image_output_port /blobDescriptor/rawImg:o</tt> \n
 *   Complete raw image output port name (propagated image)
 *
 * - <tt>view_image_output_port /blobDescriptor/viewImg:o</tt> \n
 *   Complete port name of output image (displaying overlay edges, bounding
 *   boxes, object top and bottom, etc.)
 *
 * - <tt>aff_descriptor_output_port /blobDescriptor/affDescriptor:o</tt> \n
 *   Complete affordance object descriptor output port name
 *
 * - <tt>tool_aff_descriptor_output_port /blobDescriptor/toolAffDescriptor:o</tt> \n
 *   Complete tool affordance object descriptor output port name
 *
 * - <tt>min_area_threshold 100</tt> \n
 *   Minimum number of pixels allowed for foreground objects
 *
 * - <tt>max_area_threshold 20000</tt> \n
 *   Maximum number of pixels allowed for foreground objects
 *
 * - <tt>max_objects 20</tt> \n
 *   Maximum number of objects to process
 *
 * - <tt>invalidate_boundary_objects 0</tt> \n
 *   Flag to invalidate objects that touch image boundaries 
 *
 * - <tt>draw_holes 0</tt> \n
 *   Flag to draw holes of the valid objects in the display image 
 *
 * - <tt>normalized_coords 1</tt> \n
 *   Normalize coordinates of enclosing rectangle to unit?
 *   RobotCub: 1; First-MM, POETICON++: 0
 *
 * - <tt>synch_inputs 0</tt> \n
 *   Ensure synchronization of input images?
 *   RobotCub: 1; POETICON++: 0
 *
 * \section portsa_sec Ports Accessed
 *
 * - <tt>/<SEGMENTATION_ALGO>/propagated:o</tt> \n
 *   Raw image port from the robot camera or from a segmentation module such as
 *   blobSpotter or edison
 *
 * - <tt>/<SEGMENTATION_ALGO>/binary:o</tt> \n
 *   Binary image port from a segmentation module such as blobSpotter or edison
 *
 * - <tt>/sequentialLabeller/labelImg:o</tt> \n
 *   Labeled image port. In case of edison, this port is exposed from edison itself.
 *   In case of other segmentation algorithms like blobSpotter, this port can
 *   be obtained by running sequentialLabeller. 
 *
 * \section portsc_sec Ports Created
 *
 *  <b>Input ports</b>
 *
 * - <tt>/blobDescriptor/rawImg:i</tt> \n
 *   Raw image input port
 *
 * - <tt>/blobDescriptor/binImg:i</tt> \n
 *   Binary image input port
 *
 * - <tt>/blobDescriptor/labeledImg:i</tt> \n
 *   Labeled image input port
 *
 * <b>Output ports</b>
 *
 * - <tt>/blobDescriptor/rawImg:o</tt> \n
 *   Raw image output port
 *
 * - <tt>/blobDescriptor/viewImg:o</tt> \n
 *   Port to display output image, including overlay edges.
 *   Valid vs invalid objects (see ini parameters) are outlined with contours
 *   of different colours.
 *   Additionally, for elongated objects (tools) this image also shows bounding boxes of
 *   the top and bottom parts.
 *
 * - <tt>/blobDescriptor/affDescriptor:o</tt> \n
 *   Affordance shape descriptor output port (whole objects) \n
 *   The message is a Bottle containing several values.
 *   The first value is an integer indicating the number of objects (N).
 *   The consecutive N values (one per object) are lists (Bottles) containing 
 *   the objects descriptors, with the following order:
 *		- 0 (double) - normalized x coordinate of the center of the enclosing rectangle (between -1 and 1 w.r.t. image size).
 *		- 1 (double) - normalized y coordinate of the center of the enclosing rectangle (between -1 and 1 w.r.t. image size). 
 *		- 2 (double) - normalized width of the enclosing rectangle (between 0 and 1 w.r.t. image size)
 *		- 3 (double) - normalized height of the enclosing rectangle (between 0 and 1 w.r.t. image size)
 *		- 4 (double) - angle (orientation) of the object's enclosing rectangle, according to OpenCV CvBox2D.
 *		- 5 (double) - x coordinate of the center of the bounding rectangle.
 *		- 6 (double) - y coordinate of the center of the bounding rectangle.
 *		- 7 (double) - value of the 1st bin of the Hue normalized color histogram of the pixels inside the object's region.
 *		- ...
 *		- 22 (double) - value of the 16th (last) bin of the Hue normalized color histogram of the pixels inside the object's region.
 *		- 23 (double) - area
 *		- 24 (double) - convexity - ratio between the perimeter of the object's convex hull and the perimeter of the object's contour.
 *		- 25 (double) - eccentricity - ratio between the minor and major axis of the minimum area enclosing rectangle
 *		- 26 (double) - compactness - ratio between the object area and its squared perimeter.
 *		- 27 (double) - circleness - ratio between the object area and the area of its enclosing circle.
 *		- 28 (double) - squareness - ratio between the object area and the area of its minimum-area enclosing rectangle
 *		- 29 (double) - elongatedness - linear combination between eccentricity and compactness
 *
 * - <tt>/blobDescriptor/toolAffDescriptor:o</tt> \n
 *   Tool affordance shape descriptor output port for the two object parts (tool top, tool bottom) \n
 *   The message is a Bottle containing several values and lists-of-lists.
 *   The first value is an integer indicating the number of objects (N).
 *   Then, there are N lists (one per object), each one further containing two sub-lists (one for the object top
 *   part and one for object bottom part, where "top" means with respect to 2D image coordinates). Each sub-list     contains
 *   the coordinates of the center of the object part, followed by shape descriptors relative to that object part.
 *
 *   Example, where the numbers 1..N indicate the object index:
 *
 *      N ((1top_center.x 1top_center.y 1top_area 1top_convexity 1top_eccentricity 1top_compactness 1top_circleness 1top_squareness 1top_elongatedness) (1bottom_center.x 1bottom_center.y 1bottom_area 1bottom_convexity 1bottom_eccentricity 1bottom_compactness 1bottom_circleness 1bottom_squareness 1bottom_elongatedness)) ((2top_center.x 2top_center.y 2top_area ...) (2bottom_center.x 2bottom_center.y 2bottom_area ...)) ... ((Ntop_center.x Ntop_center.y Ntop_area ...) (Nbottom_center.x Nbottom_center.y Nbottom_area ...))
 *
 * <b>Input/Output ports</b>
 *
 * - <tt>/blobDescriptor/conf</tt> \n
 *   Complete configuration and message handling port name (currently only recognizes "quit")
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
 * - \c blobDescriptor.ini in \c icub-contrib/src/poeticon/poeticonpp/app/conf
 * - \c blobDescriptor.ini in \c backup/oldBuild/app/demoAffv2/conf (old version)
 *
 * \section tested_os_sec Tested OS
 *
 * Linux
 *
 * \section example_sec Example Instantiation of the Module
 *
 * - <tt>blobDescriptor --name blobDescriptor --context blobDescriptor --from blobDescriptor.ini </tt>\n
 * - <tt>blobDescriptor --context demoAffv2/conf</tt> (RobotCub Object Affordances demo)
 *
 * \author Giovanni Saponaro <gsaponaro@isr.ist.utl.pt>, Ivana Cingovska, Alexandre Bernardino
 *
 * Copyright (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright (C) 2009 RobotCub Consortium, European Commission FP6 Project IST-004370
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 * This file can be edited at src/blobDescriptor/src/BlobDescriptorMain.cpp
 *
 */


/* YARP */
#include <yarp/os/Network.h>
#include <yarp/os/RFModule.h>

/* iCub */
#include <iCub/BlobDescriptorModule.h>

int main(int argc, char *argv[])
{
        /* initialize YARP network */
        Network yarp;
        if(! yarp.checkNetwork() )
        {
            fprintf(stdout,"Error: yarp server does not seem available\n");
            return -1; // EXIT_FAILURE
        }

	/* create module */
	BlobDescriptorModule bdm;

        /* prepare and configure Resource Finder */
        ResourceFinder rf;
        rf.setVerbose(true);
        rf.setDefaultContext("blobDescriptor");    // overridden by --context parameter
        rf.setDefaultConfigFile("blobDescriptor.ini");  // overridden by --from parameter
        rf.configure(argc, argv);

	/* runModule calls configure first and, if successful, it then runs the module */
	bdm.runModule(rf);
	return 0; // EXIT_SUCCESS
}

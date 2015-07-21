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

#include <yarp/os/Network.h>
#include "SLICModule.h"

int main(int argc, char *argv[])
{
    ResourceFinder rf;
    rf.setVerbose(true);
    rf.setDefaultContext("poeticon"); // overridden by --context
    rf.setDefaultConfigFile("slic.ini"); // overridden by --from
    rf.configure(argc, argv);
    
    fprintf(stdout,"\n");
    #ifdef CV_MAJOR_VERSION
    fprintf(stdout,"This module has been compiled with OpenCV %d.%d.%d\n",CV_MAJOR_VERSION,CV_MINOR_VERSION,CV_SUBMINOR_VERSION);
    #else
    fprintf(stdout,"This module has been compiled with an unknown version of OpenCV (probably < 1.0)\n");
    #endif
    #ifdef TBB_VERSION_MAJOR
    fprintf(stdout,"This module has been compiled with TBB %d.%d\n",TBB_VERSION_MAJOR,TBB_VERSION_MINOR);
    #else
    fprintf(stdout,"This module has been compiled with an unknown version of TBB\n");
    #endif
    fprintf(stdout,"\n");

    if(rf.check("help"))
    {
        fprintf(stdout,"Available options:\n");
        fprintf(stdout,"--name <module name> (default: slic)\n");
        fprintf(stdout,"--threadPeriod <thread period in seconds> (default: 0.033)\n");
        fprintf(stdout,"--numSuperpixels <desired number of superpixels> (default: 400)\n");
        fprintf(stdout,"--m <trade-off between spatial distance and color distance> (default: 10)\n");        
        fprintf(stdout,"--maxIter <maximum number of iterations> (default: 10)\n");                

        return 0; // EXIT_SUCCESS
    }

    Network yarp;
    if(! yarp.checkNetwork() )
    {
        fprintf(stdout,"YARP server not available!\n");
        return 1; // EXIT_FAILURE
    }

    SLICModule mod;

    return mod.runModule(rf);
}

/* 
 * Copyright (C) 2009 RobotCub Consortium, European Commission FP6 Project IST-004370
 * Authors: Vadim Tikhanoff
 * email:   vadim.tikhanoff@iit.it
 * website: www.robotcub.org 
 * Permission is granted to copy, distribute, and/or modify this program
 * under the terms of the GNU General Public License, version 2 or any
 * later version published by the Free Software Foundation.
 *
 * A copy of the license can be found at
 * http://www.robotcub.org/icub/license/gpl.txt
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details
 */

#include "iCub/objSegMod.h"
#include <math.h>

//OPENCV
#include <cv.h>
#include <highgui.h>


using namespace std;
using namespace yarp::os; 
using namespace yarp::sig;

bool objSegMod::configure(yarp::os::ResourceFinder &rf)
{    
    /* Process all parameters from both command-line and .ini file */

    /* get the module name which will form the stem of all module port names */
    moduleName            = rf.check("name", 
                           Value("objSegMod"), 
                           "module name (string)").asString();

   /*
    * before continuing, set the module name before getting any other parameters, 
    * specifically the port names which are dependent on the module name
    */
   
    setName(moduleName.c_str());

   /*
    * attach a port of the same name as the module (prefixed with a /) to the module
    * so that messages received from the port are redirected to the respond method
    */

    handlerName =  "/";
    handlerName += getName();         // use getName() rather than a literal 
 
    if (!handlerPort.open(handlerName.c_str())) {           
        cout << getName() << ": Unable to open port " << handlerName << endl;  
        return false;
    }

    //attach(handlerPort);                  // attach to port
    //attachTerminal();                     // attach to terminal (maybe not such a good thing...)
    /* create the thread and pass pointers to the module parameters */
    objSegThread = new OBJSEGThread( moduleName, rf );

    /* now start the thread to do the work */
    objSegThread->start(); // this calls threadInit() and it if returns true, it then calls run()

    return true ;// let the RFModule know everything went well
}


bool objSegMod::interruptModule()
{
    handlerPort.interrupt();
    return true;
}

bool objSegMod::close()
{
	handlerPort.interrupt();
    cout << "attempting to stop thread " << endl;
    objSegThread->stop();
    cout << "Thread stopped now deleting " << endl;
    delete objSegThread;

    return true;
}

bool objSegMod::respond(const Bottle& command, Bottle& reply) 
{
    //useless for now..
    string helpMessage =  string(getName().c_str()) + 
                        " commands are: \n" +  
                        "help";

    reply.clear();
    if (command.get(0).asString()=="help") {
        cout << helpMessage;
        reply.addString("ok");
    }
    else{
			cout << "command not known - type help for more info" << endl;
	}
    return true;
}

/* Called periodically every getPeriod() seconds */

bool objSegMod::updateModule()
{
    return true;
}

double objSegMod::getPeriod()
{
    /* module periodicity (seconds), called implicitly by myModule */
    
    return 0.1;
}

OBJSEGThread::~OBJSEGThread()
{
}

OBJSEGThread::OBJSEGThread( string &moduleName, yarp::os::ResourceFinder &rf )
{
    cout << "initialising Variables" << endl;
    this->moduleName = moduleName;

    inputNameImage = "/" + moduleName + "/image:i";
    imageIn.open( inputNameImage.c_str() );

    outputNameSeg = "/" + moduleName + "/image:o";
    imageOut.open( outputNameSeg.c_str() );

    outputNameSegCrop = "/" + moduleName + "/imageCrop:o";
    imageOutCrop.open( outputNameSegCrop.c_str() );

    fixationNamePort = "/" + moduleName + "/fixPoint:i";
    fixationPort.open( fixationNamePort.c_str() );
    
    activeSeg.configure(rf);

}

bool OBJSEGThread::threadInit() 
{
    /* initialize variables and create data-structures if needed */
    allocated = false;
    first = true;
    fix_x_prev = 0.0;
    fix_y_prev = 0.0;
    cout << "ObjSeg Started..." << endl;
    return true;
}

void OBJSEGThread::run(){

    while (isStopping() != true) { // the thread continues to run until isStopping() returns true
       
    	//get the images and run
        cout << "ok, waiting for new fixation point..." << endl;
        Bottle fixIn;
        fixIn.clear();
        
        fixationPort.read(fixIn);

        if (fixIn.get(0).asString()=="again"){
            //ImageOf<PixelBgr> &segCrop = imageOutCrop.prepare();
            //segCrop.wrapIplImage(segOnlyRgb);
            //imageOutCrop.write();
            cout << "sent the previous segmentation as a template" << endl;
        }
        else if (fixIn.get(0).asDouble() < 1 || fixIn.get(1).asDouble() < 1)
        {
            cout << "error in the fixation point, ignoring it" << endl;
        }
        else
        {
            double fix_x = fixIn.get(0).asDouble();
            double fix_y = fixIn.get(1).asDouble();
            double cropSizeWidth = fixIn.get(2).asInt();
            double cropSizeHeight = fixIn.get(3).asInt();

            if (!cropSizeWidth)
                cropSizeWidth = 50;

            if (!cropSizeHeight)
                cropSizeHeight = 50;

            cout << "Fixation point is " << fix_x << " " << fix_y << endl;
            cout << fix_x_prev << " " << fix_y_prev << " " << fix_x << " " << fix_y << endl;

            IplImage *img_in = (IplImage *) imageIn.read(true)->getIplImage();
            if( img_in != NULL )
            {
                double start = Time::now();

                IplImage *seg;
                IplImage *tpl;
                SegInfo info (fix_x, fix_y, cropSizeWidth,  cropSizeHeight);
                activeSeg.getSegWithFixation(img_in, seg, info);

                activeSeg.getTemplateFromSeg(img_in, seg, tpl, info);

                //sendSegOnly(img_crop, seg );

                printf( "\n\nTime elapsed: %f seconds\n", ( Time::now() - start ) );
                //Send and clean
                ImageOf<PixelBgr> *segImg = new ImageOf<PixelBgr>;
                segImg->resize( seg->width, seg->height );
                cvCopyImage(seg, (IplImage*)segImg->getIplImage());
                imageOut.prepare() = *segImg;
                imageOut.write();

                ImageOf<PixelBgr> *segCrop = new ImageOf<PixelBgr>;
                segCrop->resize(tpl->width, tpl->height);
                cvCopyImage(tpl, (IplImage*)segCrop->getIplImage());
                imageOutCrop.prepare() = *segCrop;             
                imageOutCrop.write();

                delete segImg;
                delete segCrop;
                cvReleaseImage(&seg);
                cvReleaseImage(&tpl);
            }
            fix_x_prev = fix_x;
            fix_y_prev = fix_y;
        }
    }
}

void OBJSEGThread::onStop(){

    cout << "cleaning up..." << endl;
    cout << "attempting to close ports" << endl;
    fixationPort.interrupt();
    fixationPort.close();
    imageIn.interrupt();
    imageOut.interrupt();
    imageOutCrop.interrupt();
    imageIn.close();
    imageOut.close();
    imageOutCrop.close();

    cout << "finished closing ports" << endl;
}

void OBJSEGThread::threadRelease() 
{
    /* for example, delete dynamically created data-structures */
}


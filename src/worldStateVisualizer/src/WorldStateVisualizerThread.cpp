/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2015 VisLab, Institute for Systems and Robotics,
 *                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Giovanni Saponaro <gsaponaro@isr.ist.utl.pt>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 */

#include "WorldStateVisualizerThread.h"

/**********************************************************/
WorldStateVisualizerThread::WorldStateVisualizerThread(
    const string &_moduleName,
    const double _period)
    : moduleName(_moduleName),
      RateThread(int(_period*1000.0))
{
}

/**********************************************************/
bool WorldStateVisualizerThread::openPorts()
{
    bool ret = true;

    inOpcBroadcastPortName = "/" + moduleName + "/broadcast:i";
    ret = ret && inOpcBroadcastPort.open(inOpcBroadcastPortName.c_str());

    inRawImgPortName = "/" + moduleName + "/rawImg:i";
    ret = ret && inRawImgPort.open(inRawImgPortName.c_str());

    outReachabilityImgPortName = "/" + moduleName + "/reachabilityImg:o";
    ret = ret && outReachabilityImgPort.open(outReachabilityImgPortName.c_str());

    return ret;
}

/**********************************************************/
void WorldStateVisualizerThread::close()
{
    yInfo("closing ports");

    mutex.wait();

    inOpcBroadcastPort.close();
    inRawImgPort.close();
    outReachabilityImgPort.close();

    mutex.post();
}

/**********************************************************/
void WorldStateVisualizerThread::interrupt()
{
    closing = true;

    yInfo("interrupting ports");

    inOpcBroadcastPort.interrupt();
    inRawImgPort.interrupt();
    outReachabilityImgPort.interrupt();
}

/**********************************************************/
bool WorldStateVisualizerThread::threadInit()
{
    if ( !openPorts() )
    {
        yError("problem opening ports");
        return false;
    }

    closing = false;

    return true;
}

/**********************************************************/
void WorldStateVisualizerThread::run()
{
    while (!closing)
        mainProcessing();
}

/**********************************************************/
void WorldStateVisualizerThread::mainProcessing()
{
    //yDebug("main processing");

    // acquire new input image (and timestamp, if present)
    Stamp ts;
    bool useTimestamp = false;

    ImageOf<PixelBgr> *inRawImg = inRawImgPort.read(true);
    if (inRawImgPort.getEnvelope(ts))
        useTimestamp = true;

    if (inRawImg != NULL)
    {
        mutex.wait();

        // create OpenCV output images, for now identical to raw input image
        Mat outReachabilityMat;
        outReachabilityMat = static_cast<IplImage*>( inRawImg->getIplImage() );

        // read database
        Bottle *db = inOpcBroadcastPort.read(false);

        if (db!=NULL && inRawImg!=NULL)
        {
            // process database Bottle, which we expect like this:
            // async ((is_hand true) ... (name left)) ((is_hand true) ... (name right)) (((name obj1) (is_hand false) ...)) ...
            bool valid = db->size()>=2 &&
                         db->get(0).asString()=="async" &&
                         db->get(1).asString()!="empty" &&
                         db->get(1).isList() &&
                         db->get(1).asList()->size()>=1;

            int numEntries = 0;
            if (valid)
                numEntries = db->size()-1;

            for (int e=0; e<numEntries; e++)
            {
                Bottle *fields = db->get(1+e).asList();
                //yDebug() << e << ":" << fields->toString().c_str();

                bool hasMandatoryFields = fields->check("name") &&
                                          fields->check("is_hand");
                if (!hasMandatoryFields)
                {
                    yWarning("lacks mandatory fields");
                    continue;
                }

                if (fields->find("is_hand").asString()=="true") // TODO: use asBool() ?
                    continue; // hand -> skip

                // object -> parse fields
                string name;
                Bottle pos2d;
                Bottle desc2d;
                Bottle tooldesc2d;
                string inHand;
                Bottle onTopOf;
                Bottle reachW;
                Bottle pullW;

                parseObjProperties(fields,
                                   name,
                                   pos2d,desc2d,tooldesc2d,inHand,onTopOf,reachW,pullW);

                double area;
                area = pos2d.get(0).asDouble();

                // check if object is reachable with at least 1 hand
                bool isReachable = false;
                isReachable = reachW.size()>0 &&
                              containsAtLeastOneHand(reachW);

                // draw on output image
                const Scalar Reachable = Scalar(0,200,0);   // dark green
                const Scalar Unreachable = Scalar(0,0,255); // red
                if (isReachable)
                {
                    circle(outReachabilityMat,
                           Point2f(pos2d.get(0).asDouble(),pos2d.get(1).asDouble()),
                           sqrt(area), // radius
                           Reachable, // color
                           3, // thickness
                           CV_AA); // lineType
                }
                else
                {
                    circle(outReachabilityMat,
                           Point2f(pos2d.get(0).asDouble(),pos2d.get(1).asDouble()),
                           sqrt(area), // radius
                           Unreachable, // color
                           3, // thickness
                           CV_AA); // lineType
                }
            }

            // write overlay image
            ImageOf<PixelBgr> &outReachabilityYarp = outReachabilityImgPort.prepare();
            IplImage outReachabilityIpl = outReachabilityMat;
            outReachabilityYarp.resize(outReachabilityIpl.width,
                                       outReachabilityIpl.height);
            cvCopyImage( &outReachabilityIpl,
                         static_cast<IplImage*>(outReachabilityYarp.getIplImage()) );
            if (useTimestamp) outReachabilityImgPort.setEnvelope(ts);
            outReachabilityImgPort.write();
        }


        mutex.post();
    }
}

/**********************************************************/
bool WorldStateVisualizerThread::parseObjProperties(const Bottle *fields,
                                     string &name, // addition w.r.t. WSM
                                     Bottle &pos2d,
                                     Bottle &desc2d, Bottle &tooldesc2d,
                                     string &inHand, Bottle &onTopOf,
                                     Bottle &reachW, Bottle &pullW)
{
    if (fields==NULL)
        return false;

    //yDebug() << __func__ << "*** fields =" << fields->toString().c_str();

    // addition w.r.t. WSM
    if ( fields->check("name") &&
         fields->find("name").isString() &&
         fields->find("name").asString()!="" )
        name = fields->find("name").asString();
    else
        yWarning("problem parsing name");


    if ( fields->check("pos2d") &&
         fields->find("pos2d").isList() &&
         fields->find("pos2d").asList()->size()>0 ) // TODO: actual size
        pos2d = * fields->find("pos2d").asList();
    else
        yWarning("problem parsing pos2d");

    if ( fields->check("desc2d") &&
         fields->find("desc2d").isList() &&
         fields->find("desc2d").asList()->size()>0 ) // TODO: actual size
        desc2d = * fields->find("desc2d").asList();
    else
        yWarning("problem parsing desc2d");

    if ( fields->check("tooldesc2d") &&
         fields->find("tooldesc2d").isList() &&
         fields->find("tooldesc2d").asList()->size()==2 &&
         fields->find("tooldesc2d").asList()->get(0).isList() &&
         fields->find("tooldesc2d").asList()->get(0).asList()->size()>0 && // TODO: actual size
         fields->find("tooldesc2d").asList()->get(1).isList() &&
         fields->find("tooldesc2d").asList()->get(1).asList()->size()>0 ) // TODO: actual size
        tooldesc2d = * fields->find("tooldesc2d").asList();
    else
        yWarning("problem parsing tooldesc2d");

    if ( fields->check("in_hand") &&
         fields->find("in_hand").isString() &&
         fields->find("in_hand").asString()!="" )
        inHand = fields->find("in_hand").asString();
    else
        yWarning("problem parsing in_hand");

    if ( fields->check("on_top_of") &&
         fields->find("on_top_of").isList() )
        onTopOf = * fields->find("on_top_of").asList();
    else
        yWarning("problem parsing on_top_of");

    if ( fields->check("reachable_with") &&
         fields->find("reachable_with").isList() )
        reachW = * fields->find("reachable_with").asList();
    else
        yWarning("problem parsing reachable_with");

    if ( fields->check("pullable_with") &&
         fields->find("pullable_with").isList() )
        pullW = * fields->find("pullable_with").asList();
    else
        yWarning("problem parsing pullable_with");

    return true;
}

/**********************************************************/
bool WorldStateVisualizerThread::containsAtLeastOneHand(const Bottle &b)
{
    const int Left = 11;
    const int Right = 12;

    for (int idx=0; idx<b.size(); idx++)
    {
        if (b.get(idx).asInt()==Left || b.get(idx).asInt()==Right)
            return true;
    }

    return false;
}

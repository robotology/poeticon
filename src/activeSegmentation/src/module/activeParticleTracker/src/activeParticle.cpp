/*
 * Copyright (C) 2011 Department of Robotics Brain and Cognitive Sciences - Istituto Italiano di Tecnologia
 * Author: Vadim Tikhanoff
 * email:  vadim.tikhanoff@iit.it
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

#include <iCub/activeParticle.h>

using namespace std;
using namespace yarp::os;
using namespace yarp::sig;

#if (CV_MAJOR_VERSION<=2)
    #define KMEANS_WITH_POINTER
    #if (CV_MAJOR_VERSION==2) && (CV_MINOR_VERSION>2)
        #undef KMEANS_WITH_POINTER
    #endif
#endif

/**********************************************************/
bool TRACKERModule::configure(yarp::os::ResourceFinder &rf)
{
    moduleName = rf.check("name", Value("activeParticleSeg"), "module name (string)").asString();
    
    setName(moduleName.c_str());

    handlerPortName =  "/";
    handlerPortName += getName();
    handlerPortName +=  "/rpc:i";

    if (!rpcPort.open(handlerPortName.c_str()))
    {
        fprintf(stdout, "%s : Unable to open port %s\n", getName().c_str(), handlerPortName.c_str());
        return false;
    }

    attach(rpcPort);
    closing = false;

    /* create the thread and pass pointers to the module parameters */
    trackerManager = new TRACKERManager( moduleName, rf );

    /* now start the thread to do the work */
    trackerManager->open();
    trackerManager->disParticles = false;

    return true ;
}

/**********************************************************/
bool TRACKERModule::interruptModule()
{
    rpcPort.interrupt();
    return true;
}

/**********************************************************/
bool TRACKERModule::close()
{
    rpcPort.close();
    fprintf(stdout, "starting the shutdown procedure\n");
    trackerManager->interrupt();
    trackerManager->close();
    fprintf(stdout, "deleting thread\n");
    delete trackerManager;
    fprintf(stdout, "done deleting thread\n");
    return true;
}
/**********************************************************/
bool TRACKERModule::quit()
{
    closing = true;
    return true;
}

/**********************************************************/
bool TRACKERModule::display(const string &value)
{
    if (value=="on")
    {
        trackerManager->disParticles = true;
        return true;
    }
    else if (value=="off")
    {
        trackerManager->disParticles = false;
        return true;
    }
    else
        return false;
}

/**********************************************************/
int TRACKERModule::track(const double fix_x, const double fix_y)
{
    Bottle b;
    b.addDouble(fix_x);
    b.addDouble(fix_y);
    int rep = trackerManager->processFixationPoint(b);
    return rep;
}

/**********************************************************/
bool TRACKERModule::countFrom(const int32_t index)
{
    trackerManager->countFrom(index);
    return true;
}

/**********************************************************/
bool TRACKERModule::untrack(const int32_t id)
{
    trackerManager->stopTracker(id);
    return true;
}

/**********************************************************/
bool TRACKERModule::reset()
{
    trackerManager->stopTrackers();
    trackerManager->flag = false;
    return true;
}

/************************************************************************/
bool TRACKERModule::attach(RpcServer &source)
{
    return this->yarp().attachAsServer(source);
}

/**********************************************************/
bool TRACKERModule::updateModule()
{
    return !closing;
}

/**********************************************************/
double TRACKERModule::getPeriod()
{
    return 0.1;
}

/**********************************************************/
TRACKERManager::~TRACKERManager()
{
    fprintf(stdout, "Going to stop %d Threads\n",(int)workerThreads.size());

    std::map< unsigned int, ParticleThread* >::iterator itr;
    for (itr = workerThreads.begin(); itr!=workerThreads.end(); itr++)
    {
        itr->second->stop();
        delete itr->second;
    }
}

/**********************************************************/
TRACKERManager::TRACKERManager( const string &moduleName, yarp::os::ResourceFinder &rf ) : container(TargetObjectRecord::getInstance())
{
    fprintf(stdout,"initialising Variables\n");
    this->moduleName = moduleName;
    fixationPoint.setManager(this);
    orig = NULL;
    iter = 0;
    flag = true;
    deleted = true;
    this->rf = rf;
}

/**********************************************************/
bool TRACKERManager::open()
{
    this->useCallback();

    //create all ports
    inImgPortName = "/" + moduleName + "/image:i";
    BufferedPort<ImageOf<PixelRgb> >::open( inImgPortName.c_str() );

    inFixPortName = "/" + moduleName + "/fixation:i";
    fixationPoint.open( inFixPortName.c_str() );

    outImgPortName = "/" + moduleName + "/image:o";
    imageOutPort.open( outImgPortName.c_str() );

    outputCropSegName = "/" + moduleName + "/imageCrop:o";
    imageSegOutPort.open( outputCropSegName.c_str() );

    outputCropTplName = "/" + moduleName + "/imageTpl:o";
    imageTplOutPort.open( outputCropTplName.c_str() );

    outTargetPortName = "/" + moduleName + "/target:o";
    targetOutPort.open( outTargetPortName.c_str() );

    checkClosure=false;

    return true;
}

/**********************************************************/
void TRACKERManager::close()
{
    fprintf(stdout,"now closing ports...\n");
    mutex.wait();
    imageOutPort.close();
	fixationPoint.close();
    targetOutPort.writeStrict();
    targetOutPort.close();
    imageSegOutPort.close();
    imageTplOutPort.close();
    BufferedPort<ImageOf<PixelRgb> >::close();
    mutex.post();
    fprintf(stdout,"finished closing the read port...\n");
}

/**********************************************************/
void TRACKERManager::interrupt()
{
    checkClosure=true;
    fprintf(stdout,"cleaning up...\n");
    fprintf(stdout,"attempting to interrupt ports\n");

	fixationPoint.interrupt();

    BufferedPort<ImageOf<PixelRgb> >::interrupt();
    fprintf(stdout,"finished interrupt ports\n");
}

/**********************************************************/
bool TRACKERManager::countFrom(int index)
{
    iter=index;
    return true;
}

/**********************************************************/
bool TRACKERManager::stopTracker(int id)
{
    mutex.wait();
    fprintf(stdout,"attempting to stop tracker id %d\n", id);
    workerThreads[id]->stop();
    delete workerThreads[id];
    workerThreads.erase(id);
    mutex.post();
    return true;
}

/**********************************************************/
bool TRACKERManager::stopTrackers()
{
    yarp::sig::Vector listID;
    std::map< unsigned int, ParticleThread* >::iterator itr;
    for (itr = workerThreads.begin(); itr!=workerThreads.end(); itr++)
        listID.push_back(itr->first);

    for (int i = 0; i<listID.size(); i++)
        stopTracker(listID[i]);

    return true;
}
/**********************************************************/
int TRACKERManager::processFixationPoint(Bottle &b)
{
    mutexPoints.lock();
    int id = -1;

    if (b.get(0).asString()=="again")
        fprintf(stdout,"------------------TO DO-------------------send the previous segmentation(s) as template (s) \n");
    else if (b.get(0).asDouble() < 1 || b.get(1).asDouble() < 1)
        fprintf(stdout,"error in the fixation point, ignoring it\n");
    else
    {
        fix_x = b.get(0).asDouble();
        fix_y = b.get(1).asDouble();
        cropSizeWidth = b.get(2).asInt();
        cropSizeHeight = b.get(3).asInt();

        if (!cropSizeWidth)
            cropSizeWidth = 50;

        if (!cropSizeHeight)
            cropSizeHeight = 50;

        fprintf(stdout,"Fixation point is %lf %lf \n",fix_x, fix_y);

        bool shouldStart = true;
        if (orig!=NULL)
        {
            std::map< unsigned int, ParticleThread* >::iterator itr;
            for (itr = workerThreads.begin(); itr!=workerThreads.end(); itr++)
            {
                double limits_x = fabs(workerThreads[itr->first]->particles[0].x - fix_x );
                double limits_y = fabs(workerThreads[itr->first]->particles[0].y - fix_y );

                fprintf(stdout,"The X points are: p: %lf fix: %lf with diff: %lf\n", workerThreads[itr->first]->particles[0].x, fix_x, limits_x );
                fprintf(stdout,"The Y points are: p: %lf fix: %lf with diff: %lf\n", workerThreads[itr->first]->particles[0].y, fix_y, limits_y );

                if ( limits_x < cropSizeWidth-30 && limits_y < cropSizeHeight-30)
                {
                    fprintf(stdout, "HAVE THE SAME POINT AS PATICLES, should NOT start thread\n");
                    shouldStart = false;
                    break;
                }
                else
                    fprintf(stdout, "OK SHOULD START THREAD\n");
            }
            if (shouldStart)
            {
                SegInfo info (fix_x, fix_y, cropSizeWidth,  cropSizeHeight);
                id = iter;
                fprintf(stdout, "OK SHOULD START THREAD %d with ITER %d\n", id, iter);
                
                workerThreads[id] = new ParticleThread(id, rf, info);
                workerThreads[id]->start();
                workerThreads[id]->update(orig);
                
                workerThreads[id]->isInitialized();
                
                iter++;
            }
        }
    }
    mutexPoints.unlock();
    return id;
    
}

/**********************************************************/
void TRACKERManager::onRead(ImageOf<yarp::sig::PixelRgb> &img)
{
    ImageOf<PixelRgb> &outImg = imageOutPort.prepare();
    outImg = img;
    orig = (IplImage*)img.getIplImage();

    mutex.wait();

    std::map< unsigned int, ParticleThread* >::iterator itr;
    for (itr = workerThreads.begin(); itr!=workerThreads.end(); itr++)
    {
        //workerThreads[itr->first]->
        workerThreads[itr->first]->update(orig);
    }
    mutex.post();

    if (flag)
    {
        if (!deleted)
        {
            fprintf(stdout,"will now delete TRACKER\n");
            stopTracker(toDel[0]);
            fprintf(stdout,"done ");
            toDel.clear();
            fprintf(stdout,"cleared ");
            deleted = true;
        }
    }

    Bottle &b = targetOutPort.prepare();
    b.clear();
    container.lock();

    for (TargetObjectRecord::iterator itr = container.targets.begin(); itr!=container.targets.end(); itr++)
    {
        TargetObject *obj = *itr;
        //send template and segmentation only once
        if (!obj->hasSent)
        {
            ImageOf<PixelBgr> &segImg = imageSegOutPort.prepare();
            segImg.resize( obj->seg->width, obj->seg->height );
            cvCopyImage(obj->seg, (IplImage*)segImg.getIplImage());
            imageSegOutPort.write();

            ImageOf<PixelBgr> &segCrop = imageTplOutPort.prepare();
            segCrop.resize(obj->tpl->width, obj->tpl->height);
            cvCopyImage(obj->tpl, (IplImage*)segCrop.getIplImage());
            imageTplOutPort.write();

            obj->hasSent = true;
        }
        if (obj->boundingBox.size())
        {
            Bottle &t = b.addList();
            t.addDouble(obj->owner);
            for (int i=0; i<obj->boundingBox.size(); i++)
                t.addDouble(obj->boundingBox[i]);

            t.addInt(obj->group);

            int x0 = obj->boundingBox[2];
            int y0 = obj->boundingBox[3];
            int x1 = obj->boundingBox[4];
            int y1 = obj->boundingBox[5];
            int cx = obj->boundingBox[0];
            int cy = obj->boundingBox[1];

            if (disParticles)
            {
                for (int j = 0; j < obj->particlePoints.size(); j++ )
                    cvCircle ((IplImage*)outImg.getIplImage(), obj->particlePoints[j], 3,  obj->colour, 1);
            }

            cvRectangle( (IplImage*)outImg.getIplImage(), cvPoint( x0, y0 ), cvPoint( x1, y1 ), obj->colour, 1, 8, 0 );

            cvCircle ( (IplImage*)outImg.getIplImage(), cvPoint( cx, cy ), 3, CV_RGB(255, 0 , 0), 1 );
            cvCircle ( (IplImage*)outImg.getIplImage(), cvPoint( x0, y0 ), 3, CV_RGB(0, 255 , 0), 1 );
            cvCircle ( (IplImage*)outImg.getIplImage(), cvPoint( x1, y1 ), 3, CV_RGB(0, 255 , 0), 1 );

            int sampleCount = obj->particlePoints.size();
            int dimensions = 2;
            int clusterCount = 2;
            cv::Mat points(sampleCount, dimensions, CV_32F);
            cv::Mat labels;
            cv::Mat centers(clusterCount, dimensions, points.type());
            for(int i = 0; i<sampleCount;i++)
            {
                points.at<float>(i,0) = obj->particlePoints[i].x;
                points.at<float>(i,1) = obj->particlePoints[i].y;
            }

#ifdef KMEANS_WITH_POINTER

            cv::kmeans(points, clusterCount, labels, cv::TermCriteria( cv::TermCriteria::EPS+cv::TermCriteria::COUNT, 1, 10.0), 3, cv::KMEANS_PP_CENTERS, &centers);
#else
            cv::kmeans(points, clusterCount, labels, cv::TermCriteria( cv::TermCriteria::EPS+cv::TermCriteria::COUNT, 1, 10.0), 3, cv::KMEANS_PP_CENTERS, centers);
#endif
            cv::Point pts[3];

            for (int i = 0; i < clusterCount; i++)
            {
                cv::Point ipt;
                ipt.x = (int) centers.at<float>(i,0);
                ipt.y = (int) centers.at<float>(i,1);

                pts[i] = ipt;

                //cvCircle ((IplImage*)outImg.getIplImage(), ipt, 5,  CV_RGB(0, 0 , 255), CV_FILLED, CV_AA);//obj->colour,  CV_FILLED, CV_AA);
            }
            int limits_x = abs(pts[0].x - pts[1].x );
            int limits_y = abs(pts[0].y - pts[1].y );

            int mutationThres = 30;

            if ( limits_x > mutationThres || limits_y > mutationThres)
            {
                if (!flag)
                {
                    cvCircle ((IplImage*)outImg.getIplImage(), pts[0], 5,  CV_RGB(255, 0 , 255), CV_FILLED, CV_AA);
                    cvCircle ((IplImage*)outImg.getIplImage(), pts[1], 5,  CV_RGB(255, 0 , 255), CV_FILLED, CV_AA);
                    //cvSaveImage("output.png", (IplImage*)outImg.getIplImage());
                    cloneTracker(obj, pts);
                }
            }
        }
    }
    
    container.unlock();
    if (b.size())
    targetOutPort.write();

    imageOutPort.write();
}

/**********************************************************/
void TRACKERManager::cloneTracker(TargetObject *obj, cv::Point *pt)
{
    if (!flag)
    {
        toDel.push_back(obj->owner);

        int group       = ( obj->group == -1 ) ? obj->owner : obj->group;

        obj->group = group;
        SegInfo info (pt[0].x, pt[0].y, cropSizeWidth, cropSizeHeight);

        //CvScalar temp = CV_RGB(255, 0 , 0);

        int id = iter;
        workerThreads[id] = new ParticleThread(id, rf, info, group, &obj->colour);
        workerThreads[id]->start();
        workerThreads[id]->update(orig);
        //increase thread iter
        iter++;

        SegInfo info1 (pt[1].x, pt[1].y, cropSizeWidth, cropSizeHeight);
        id = workerThreads.size();

        workerThreads[id] = new ParticleThread(id, rf, info1, group, &obj->colour);//&obj->colour);
        workerThreads[id]->start();
        workerThreads[id]->update(orig);
        //increase thread iter
        iter++;

        flag = true;
        deleted = false;
    }
}

//empty line to make gcc happy

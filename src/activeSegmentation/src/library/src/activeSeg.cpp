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

#include "iCub/activeSeg.h"

using namespace yarp::os;

/**********************************************************/

bool ActiveSeg::configure(ResourceFinder &rf)
{
    string texture13=rf.findFile("texture_filterbank_13.txt").c_str();
    string texture19=rf.findFile("texture_filterbank_19.txt").c_str();
    string textonsFile=rf.findFile("textons.txt").c_str();

    fileLoc.put("texture_13", texture13);
    fileLoc.put("texture_19", texture19);
    fileLoc.put("textons", textonsFile);
    return true;
}
/**********************************************************/
ActiveSeg::Error ActiveSeg::getSegWithFixation( const IplImage *img, IplImage* &seg, SegInfo &info )
{
    yAssert(img);

    if (info.fix_x >= img->width || info.fix_x <= 0 || info.fix_y >=img->height || info.fix_y <= 0)
    {
        //fprintf(stdout, "\n\nFixation point is outside image dimensions,\n fixPoint: %d %d and imageDim %d %d \n\n", fix_x, fix_y, img->width, img->height );

        return ActiveSeg::ERR_OUT_OF_BOUND;
    }

    //check if x and crop are within width range
    if ( (info.fix_x - info.cropWidth/2) < 0 )
    {
        info.cropWidth = (info.fix_x-1) * 2; //make sure it lies within bounds
#ifdef ACTIVESEG_DEBUG
        fprintf(stdout, "\n\nInvalid left size for x position,reducing crop to: %d ", info.cropWidth);
#endif
    }
    if ( (info.fix_x + info.cropWidth/2) > img->width  )
    {
        info.cropWidth = (img->width - info.fix_x - 1) * 2;//make sure it lies within bounds
#ifdef ACTIVESEG_DEBUG
        fprintf(stdout, "Invalid right size for x position,reducing crop to: %d \n", info.cropWidth);
#endif
    }
    //check if y and crop are within width range
    if ( (info.fix_y - info.cropHeight/2) < 0 )
    {
        info.cropHeight = (info.fix_y-1) * 2; //make sure it lies within bounds
#ifdef ACTIVESEG_DEBUG
        fprintf(stdout, "Invalid upper size for y position,reducing cropHeight to: %d \n", info.cropHeight);
#endif
    }
    if ( (info.fix_y + info.cropHeight/2) > img->height  )
    {
        info.cropHeight = (img->height - info.fix_y - 1) * 2;//make sure it lies within bounds
#ifdef ACTIVESEG_DEBUG
        fprintf(stdout, "Invalid lower size for y position,reducing cropHeight to: %d \n\n", info.cropHeight);
#endif
    }
#ifdef ACTIVESEG_DEBUG
    double start = Time::now();
#endif
    IplImage* tmp = cvCloneImage(img);
    cvCvtColor(tmp, tmp, CV_BGR2RGB);
    cvSetImageROI(tmp, cvRect(info.fix_x-info.cropWidth/2, info.fix_y-info.cropHeight/2, info.cropWidth, info.cropHeight));
    seg = cvCreateImage(cvSize(info.cropWidth,  info.cropHeight),  tmp->depth, tmp->nChannels );
    cvCopy(tmp, seg, NULL);
    cvResetImageROI(tmp);

    segmentWithFixation(seg, seg->width/2, seg->height/2);
    cvCircle(seg, cvPoint( seg->width/2, seg->height/2 ), 2, cvScalar( 0,255,0 ), 3 );
    frame.deallocateMemForContours();

#ifdef ACTIVESEG_DEBUG
    fprintf(stdout, "\n\nTime elapsed: %f seconds\n", ( Time::now() - start ) );
#endif

    cvReleaseImage(&tmp);
    //mutex.post();
    return ActiveSeg::OK;
}

/**********************************************************/

inline void ActiveSeg::segmentWithFixation(IplImage *img_in, double x, double y)
{

#ifdef ACTIVESEG_DEBUG
    fprintf(stdout, "\nSetting the image\n");
#endif
    //set the image
    frame.setImage(img_in);
#ifdef ACTIVESEG_DEBUG
    fprintf(stdout, "finished setting and processing edge started \n");
#endif
    // Edge detection!
    frame.edgeCGTG(fileLoc);
#ifdef ACTIVESEG_DEBUG
    fprintf(stdout, "finished processing and generating boundaries \n");
#endif
    frame.generatePbBoundary();
#ifdef ACTIVESEG_DEBUG
    fprintf(stdout, "finished generating boundaries and setting fixation point\n");
#endif
    //use fixation point
	frame.assignFixPt(x,y);
    //segment
    frame.allocateMemForContours();
    frame.segmentCurrFix();
    //get the seg
    img_in = frame.getSeg( img_in );
}

/**********************************************************/
ActiveSeg::Error  ActiveSeg::getTemplateFromSeg(const IplImage *img, IplImage* seg, IplImage* &tpl, SegInfo &info)
{
    yAssert(img);
    yAssert(seg);

    int top = -1;
    int left = -1;
    int right = -1;
    int bottom = -1;

    cv::Mat imgMat = cv::cvarrToMat(seg);

    for (int j=0;j<imgMat.rows;j++){
        for (int i=0;i<imgMat.cols;i++){
            if ( imgMat.at<cv::Vec3b>(j,i)[0] == 0 &&
                imgMat.at<cv::Vec3b>(j,i)[1] == 255 &&
                imgMat.at<cv::Vec3b>(j,i)[2] == 0 )
            {
                top = j;
                goto topFound;
            }
        }
    }

topFound:
    for (int j=imgMat.rows-1; j>0; j--){
        for (int i=imgMat.cols-1; i>0 ;i--){
            if ( imgMat.at<cv::Vec3b>(j,i)[0] == 0 &&
                imgMat.at<cv::Vec3b>(j,i)[1] == 255 &&
                imgMat.at<cv::Vec3b>(j,i)[2] == 0 )
            {
                bottom = j;
                goto bottomFound;
            }
        }
    }

bottomFound:
    for (int i=0;i<imgMat.cols;i++){
        for (int j=0;j<imgMat.rows;j++){
            if ( imgMat.at<cv::Vec3b>(j,i)[0] == 0 &&
                imgMat.at<cv::Vec3b>(j,i)[1] == 255 &&
                imgMat.at<cv::Vec3b>(j,i)[2] == 0 )
            {
                left = i;
                goto leftFound;
            }
        }
    }

leftFound:
    for (int i=imgMat.cols-1;i>0;i--){
        for (int j=0;j<imgMat.rows;j++){
            if ( imgMat.at<cv::Vec3b>(j,i)[0] == 0 &&
                imgMat.at<cv::Vec3b>(j,i)[1] == 255 &&
                imgMat.at<cv::Vec3b>(j,i)[2] == 0 )
            {
                right = i;
                goto rightFound;
            }
        }
    }

rightFound:

    IplImage* tmp = cvCloneImage(img);
    cvCvtColor(tmp, tmp, CV_BGR2RGB);
    cvSetImageROI (tmp, cvRect((info.fix_x-info.cropWidth/2)+left, (info.fix_y-info.cropHeight/2)+top, right-left, bottom-top) );
    tpl = cvCreateImage(cvSize(right-left, bottom-top), tmp->depth, tmp->nChannels );
    cvCopy(tmp, tpl);

    cvReleaseImage(&tmp);

    return ActiveSeg::OK;
}

/**********************************************************/

ActiveSeg::Error ActiveSeg::getTemplate( const IplImage *img, IplImage* &tpl, SegInfo &info )
{
    yAssert(img);
    IplImage* seg = cvCloneImage(img);
    ActiveSeg::Error ret;

    if ( (ret = getSegWithFixation( img, seg, info ) ) != ActiveSeg::OK )
    {
        cvReleaseImage(&seg);
        return ret;
    }
    if ( (ret = getTemplateFromSeg( img, seg, tpl, info ) ) != ActiveSeg::OK )
    {
        cvReleaseImage(&seg);
        return ret;
    }
    cvReleaseImage(&seg);

    return ActiveSeg::OK;
}

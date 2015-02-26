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

#include "SLICThread.h"

SLICThread::SLICThread(const string &_moduleName, const double _period,
    const int _numSuperpixels, const int _m, const int _maxIter)
    : moduleName(_moduleName), RateThread(int(_period*1000.0)),
    numSuperpixels(_numSuperpixels), m(_m), maxIter(_maxIter)
{
    fprintf(stdout, "%s: constructed thread with variables ", moduleName.c_str());
    fprintf(stdout, "numSuperpixels=%d ", numSuperpixels);
    fprintf(stdout, "m=%d ", m);
    fprintf(stdout, "maxIter=%d\n", maxIter);
}

bool SLICThread::openPorts()
{
    inRawImgPortName = "/" + moduleName + "/img:i";
    inRawImgPort.open(inRawImgPortName.c_str());

    outContourImgPortName = "/" + moduleName + "/contourImg:o";
    outContourImgPort.open(outContourImgPortName.c_str());

    outRecoloredImgPortName = "/" + moduleName + "/recoloredImg:o";
    outRecoloredImgPort.open(outRecoloredImgPortName.c_str());

    outCielabImgPortName = "/" + moduleName + "/cielabImg:o";
    outCielabImgPort.open(outCielabImgPortName.c_str());

    outBinaryImgPortName = "/" + moduleName + "/binaryImg:o";
    outBinaryImgPort.open(outBinaryImgPortName.c_str());

    return true;
}

void SLICThread::close()
{
    fprintf(stdout, "%s: closing ports\n", moduleName.c_str());
    
    // critical section
    mutex.wait();

    inRawImgPort.close();
    outContourImgPort.close();
    outRecoloredImgPort.close();
    outCielabImgPort.close();
    outBinaryImgPort.close();

    mutex.post();
}

void SLICThread::interrupt()
{
    fprintf(stdout, "%s: interrupting ports\n", moduleName.c_str());

    closing = true;

    inRawImgPort.interrupt();
    outContourImgPort.interrupt();
    outRecoloredImgPort.interrupt();
    outCielabImgPort.interrupt();
    outBinaryImgPort.interrupt();
}

bool SLICThread::threadInit()
{
    fprintf(stdout, "%s: thread initialization\n", moduleName.c_str());

    if( !openPorts() )
    {
        fprintf(stdout, "%s: problem opening ports\n", moduleName.c_str());
    };

    closing = false;

    return true;
}

void SLICThread::run()
{
    while (!closing)
    {
        // acquire new input image
        ImageOf<PixelBgr> *inRawImg = inRawImgPort.read(true);

        if (inRawImg != NULL)
        {
            // critical section
            mutex.wait();
        
            // acquire timestamp
            Stamp ts;
            inRawImgPort.getEnvelope(ts);

            // generate superpixels
            Mat inMat;
            inMat = static_cast<IplImage*>( inRawImg->getIplImage() );
            tbb::tick_count begin = tbb::tick_count::now();
            SLICSuperpixel slic(inMat, numSuperpixels, m, maxIter);
            slic.generateSuperPixels();
            cout << moduleName << ": ";
            cout << (tbb::tick_count::now() - begin).seconds() << " seconds elapsed" << endl;
            
            // image with overlay SLIC contours bordering the clusters
            Mat outMat(inMat);
            std::vector<Point2i> contours = slic.getContours();
            for( Point2i contour : contours )
                outMat.at<Vec3b>(contour.y, contour.x) = Vec3b(255, 0, 255);

            ImageOf<PixelBgr> &outYarp = outContourImgPort.prepare();
            IplImage outIpl = outMat;
            outYarp.resize(outIpl.width, outIpl.height);
            cvCopyImage( &outIpl, static_cast<IplImage*>(outYarp.getIplImage()) );
            outContourImgPort.setEnvelope(ts);
            outContourImgPort.write();
                                                
            // recolored image based on average cluster color
            Mat recoloredMat = slic.recolor();
            cvtColor(recoloredMat, recoloredMat, CV_Lab2BGR);
            ImageOf<PixelBgr> &recoloredYarp = outRecoloredImgPort.prepare();
            IplImage recoloredIpl = recoloredMat;
            recoloredYarp.resize(recoloredIpl.width, recoloredIpl.height);
            cvCopyImage( &recoloredIpl, static_cast<IplImage*>(recoloredYarp.getIplImage()) );
            outRecoloredImgPort.setEnvelope(ts);
            outRecoloredImgPort.write();
            
            // image in CIELab color space
            Mat cielabMat = slic.getImage();
            ImageOf<PixelBgr> &cielabYarp = outCielabImgPort.prepare();
            IplImage cielabIpl = cielabMat;
            cielabYarp.resize(cielabIpl.width, cielabIpl.height);
            cvCopyImage( &cielabIpl, static_cast<IplImage*>(cielabYarp.getIplImage()) );
            outCielabImgPort.setEnvelope(ts);
            outCielabImgPort.write();
            
            // binary image
            Mat binaryMat;
            recoloredMat.convertTo(binaryMat, CV_8UC1);
            cvtColor(binaryMat, binaryMat, CV_BGR2GRAY);
            // white objects on black background
            threshold(binaryMat, binaryMat, 100, 255.0, THRESH_BINARY_INV|THRESH_OTSU);
            ImageOf<PixelMono> &binaryYarp = outBinaryImgPort.prepare();
            IplImage binaryIpl = binaryMat;
            binaryYarp.resize(binaryIpl.width, binaryIpl.height);
            cvCopyImage( &binaryIpl, static_cast<IplImage*>(binaryYarp.getIplImage()) );
            outBinaryImgPort.setEnvelope(ts);
            outBinaryImgPort.write();
            
            mutex.post();
        }
    }
}

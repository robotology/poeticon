/*
 * Copyright (C) 2015 VisLab, Instituto Superior Tecnico
 * Authors: Lorenzo Jamone, Afonso Gonçalves
 * email:  ljamone@isr.ist.utl.pt
 * Based on software from Vadim Tikhanoff and Ugo Pattacini (Istituto Italiano di Tecnologia)
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


#ifndef __MODULE_H__
#define __MODULE_H__

#include <string>
#include <fstream>
#include <sstream>

#include <yarp/os/Time.h>
#include <yarp/os/Semaphore.h>
#include <yarp/os/RFModule.h>
#include <yarp/os/RpcServer.h>
#include <yarp/os/RpcClient.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/os/Port.h>
#include <yarp/sig/Image.h>
#include <yarp/sig/Vector.h>

#include <cv.h>

#include "iCub/utils.h"

#define LEFTARM             0
#define RIGHTARM            1

const int                       N_ACTIONS                    = 4;
//const int                       N_TOOLS                      = 2;
const double                    DEFAULT_ACTION_TIME          = 2.0;
const int                       DEFAULT_MOTION_STEPS         = 4;
const int                       DEFAULT_MOTION_STEPS_MIN     = 2;
const int                       DEFAULT_MAX_OBJ              = 1;
const double                    DEFAULT_CLOSE_THR            = 30.0;
const double                    VDRAW_THR                    = 0.1;
const double                    TOOL_TRANSFORM_DEFAULT[]     = {0.17, -0.17, 0.06};
const double                    TABLE_HEIGHT_OFFSET_DEFAULT  = 0.03;
const double                    OBJECT_SIZE_OFFSET_DEFAULT   = 0.04;
const int                       HAND_NATURAL_POSE_DEFAULT    = 0; // 0 = straight pose, 1 = pronated pose.

/**********************************************************/
class BlobInfo
{
public:
  double roi_x;
  double roi_y;
  double roi_width;
  double roi_height;
  double angle;
  double special_point_x; //point in the bottom part of the blob (if the object is on the table, this point should be on the table as well...)
  double special_point_y;

  double hist[16];

  double area;
  double convexity;
  double eccentricity;
  double compactness;
  double circleness;
  double squareness;
  double elongatedness;

  std::string toString()
  {

      std::string blobsString;
      std::ostringstream s;

      s << roi_x << " " << roi_y << " " << roi_width << " " << roi_height << " "  << angle << " " << special_point_x << " " << special_point_y << " ";

      for(int i=0; i<16; i++)
      {
          s << hist[i] << " ";
      }

      s << area << " " << convexity << " " << eccentricity << " " << compactness << " " << circleness << " " << squareness << " " << elongatedness;

      blobsString = s.str();
      return blobsString;

  }

  std::string geomToString()
  {

      std::string blobsString;
      std::ostringstream s;

      s << convexity << " " << eccentricity << " " << compactness << " " << circleness << " " << squareness;

      blobsString = s.str();
      return blobsString;

  }

};

/**********************************************************/
class BlobPartInfo
{
public:
  double roi_x;
  double roi_y;

  double area;
  double convexity;
  double eccentricity;
  double compactness;
  double circleness;
  double squareness;
  double elongatedness;

  std::string toString()
  {

      std::string blobsString;
      std::ostringstream s;

      s << roi_x << " " << roi_y << " " ;

      s << area << " " << convexity << " " << eccentricity << " " << compactness << " " << circleness << " " << squareness << " " << elongatedness;

      blobsString = s.str();
      return blobsString;

  }

  std::string geomToString()
  {

      std::string blobsString;
      std::ostringstream s;

      s << convexity << " " << eccentricity << " " << compactness << " " << circleness << " " << squareness;

      blobsString = s.str();
      return blobsString;

  }
};

/**********************************************************/
class Manager : public yarp::os::RFModule
{
protected:

    std::string                 name;               //name of the module
    std::string                 hand;               //hand
    std::string                 camera;             //camera

    std::string                 descdataFileName;   //name of visual descriptors data file
    std::string                 descdataMinFileName;   //name of (minimal) visual descriptors data file
    std::string                 descdataPCFileName;   //name of visual descriptors data file
    std::string                 descdataMinPCFileName;   //name of (minimal) visual descriptors data file
    std::string                 effdataFileName;    //name of effects data file
    std::string                 simMode;            //flags if the module is running exclusively with the simulator
    yarp::os::Port              rpcHuman;           //human rpc port (receive commands via rpc)
    yarp::os::RpcClient         rpcMotorAre;        //rpc motor port ARE
    yarp::os::RpcClient         rpcMotorKarma;      //rpc motor port KARMA
    yarp::os::RpcClient         rpcBlobSpot;        //rpc port blobSpotter
    yarp::os::RpcClient         rpcToolFinder;      //rpc port KARMA tool finder

    SegmentationPoint           segmentPoint;       //class to request segmentation from activeSegmentation module


    yarp::os::BufferedPort<yarp::os::Bottle>        blobExtractor;
    yarp::os::BufferedPort<yarp::sig::Vector>       targetPF;

    int                         n_tools;
    yarp::os::Semaphore         mutexResources;     //mutex for ressources
    CvPoint                     pointLocation;      //x and y of the pointed location
    yarp::os::Bottle            lastBlobs;
    yarp::os::Bottle            lastTool;
    yarp::sig::Vector           objectPos;

    std::vector<yarp::sig::Vector> toolTransform;
    yarp::sig::Vector              toolTransformDefault;

    bool                        get3DPosition(const CvPoint &point, yarp::sig::Vector &x);
    int                         processHumanCmd(const yarp::os::Bottle &cmd, yarp::os::Bottle &b);

    int                         executeToolAttach(const yarp::sig::Vector &tool);

    void                        segmentAndTrack( int x, int y );

    void                        goHomeArmsHead();
    void                        goHomeAll();
    void                        goHomeHands();

    int                         actionId;
    double                      actionStartTime;
    double                      actionDurationTime;
    double                      actionTime;
    int                         toolId;
    int                         objectId;
    std::string                 toolName;
    int                         toolSimNum;
    std::string                 targetName;
    int                         targetSimNum;

    double                      x_start;
    double                      y_start;
    double                      z_start;
    double                      u_start;
    double                      v_start;

    double                      tableHeightOffset;
    double                      objectSizeOffset;

    int                         motSteps;
    int                         motStepsMin;

    int                         handNaturalPose;

    std::ofstream               descData;
    std::ofstream               descDataMin;
    std::ofstream               descDataPC;
    std::ofstream               descDataMinPC;
    std::ofstream               effData;

    BlobInfo                    objDesc;
    BlobPartInfo                objTopDesc;
    BlobPartInfo                objBottomDesc;
    BlobInfo                    objDescPC;
    BlobPartInfo                objTopDescPC;
    BlobPartInfo                objBottomDescPC;

    CvPoint                     objImgPos;
    CvPoint                     objImgPos_onTable;
    int                         maxObjects;
    double                      closeThr;

    yarp::os::BufferedPort<yarp::os::Bottle>        fullBlobDescriptorInputPort;
    yarp::os::BufferedPort<yarp::os::Bottle>        partsBlobDescriptorInputPort;
    yarp::os::BufferedPort<yarp::os::Bottle>        fullBlobDescriptorPCInputPort;
    yarp::os::BufferedPort<yarp::os::Bottle>        partsBlobDescriptorPCInputPort;

    void                        performAction();
    int                         lookAtTool();   //might be used if tools are on a rack...
    void                        lookAtObject();
    void                        computeObjectDesc();
    int                         askForTool();
    void                        graspTool();
    void                        updateObjVisPos();
    yarp::os::Bottle            executeToolLearning();

    bool                        updateObjectDesc(const yarp::os::Bottle *msgO, const yarp::os::Bottle *msgOP, const yarp::os::Bottle *msgO_PC, const yarp::os::Bottle *msgOP_PC);

    yarp::os::Port              simObjLoaderModuleOutputPort;
    yarp::os::Port              automatorReadyPort;

    void                        lookAtRack();

    // Ports and functions used in the 2014/02 tests at IIT, requires matlab module with affordances BN:
//    yarp::os::Port              bayesianNetOutputPort;
//    yarp::os::Port              bayesianNetInputPort;
//    void                        chooseTool(const int & effectLon, const int & effectLat, int & x, int & y);
//    void                        evaluateAllToolsDesc(std::vector<BlobPartInfo> & toolsTops, std::vector<BlobPartInfo> & toolsBottoms);
//    void                        updateToolPartDesc(BlobPartInfo & toolPart, const yarp::os::Bottle * toolPartBottle);
//    void                        discretizeToolsTops(std::vector<BlobPartInfo> & toolsTops);
//    void                        findBestToolTop(int & choosenTool, const std::vector<BlobPartInfo> & toolsTops, const yarp::os::Bottle & queryResponse);

public:
    bool    configure(yarp::os::ResourceFinder &rf);
    bool    interruptModule();
    bool    close();
    bool    updateModule();
    double  getPeriod();
};
#endif


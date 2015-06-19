/*
 * Copyright (C) 2015 VisLab, Instituto Superior Tecnico
 * Authors: Lorenzo Jamone, Afonso Gonçalves
 * email:  ljamone@isr.ist.utl.pt
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

#include <sstream>
#include <stdio.h>
#include <yarp/math/Rand.h>
#include <yarp/math/Math.h>
#include <yarp/os/Os.h>
#include "iCub/module.h"
#include <gsl/gsl_math.h>
#include <time.h>

using namespace std;
using namespace yarp::os;
using namespace yarp::sig;
using namespace yarp::math;

#define RET_INVALID     -1

#define CMD_TRAIN               VOCAB4('t','r','a','i')
#define CMD_EXECUTE             VOCAB4('e','x','e','c')
#define CMD_TOOLEXTEND          VOCAB4('e','x','t','d')

/**********************************************************/
bool Manager::configure(ResourceFinder &rf)
{
    name=rf.find("name").asString().c_str();
    camera=rf.find("camera").asString().c_str();
    if ((camera!="left") && (camera!="right"))
        camera="left";

    hand=rf.find("hand").asString().c_str();
    if ((hand!="left") && (hand!="right"))
        hand="left";

    maxObjects=rf.check("maxObjects", Value(DEFAULT_MAX_OBJ)).asInt();

    actionTime=rf.check("actionTime", Value(DEFAULT_ACTION_TIME)).asDouble();

    motSteps=rf.check("motSteps", Value(DEFAULT_MOTION_STEPS)).asInt();
    motStepsMin=rf.check("motStepsMin", Value(DEFAULT_MOTION_STEPS_MIN)).asInt();
    if (motStepsMin>=motSteps)
    {
        motStepsMin=motSteps-1;
    }

    tableHeightOffset=rf.check("tableHeightOffset", Value(TABLE_HEIGHT_OFFSET_DEFAULT)).asDouble();
    objectSizeOffset=rf.check("objectSizeOffset", Value(OBJECT_SIZE_OFFSET_DEFAULT)).asDouble();

    handNaturalPose=rf.check("handNaturalPose", Value(HAND_NATURAL_POSE_DEFAULT)).asInt();
    
    const ConstString icubContribEnvPath = yarp::os::getenv("ICUBcontrib_DIR");
    const ConstString localPath = "/share/ICUBcontrib/contexts/affExploreManager/";
    
    descdataFileName = rf.check("descdataFileName",Value("descData.txt")).asString();
    descdataFileName = icubContribEnvPath + localPath + descdataFileName;
    descData.open(descdataFileName.c_str(), ofstream::out | ofstream::app);
    descdataMinFileName = rf.check("descdataMinFileName",Value("descDataMin.txt")).asString();
    descdataMinFileName = icubContribEnvPath + localPath + descdataMinFileName;
    descDataMin.open(descdataMinFileName.c_str(), ofstream::out | ofstream::app);

    effdataFileName = rf.check("effdataFileName",Value("effData.txt")).asString();
    effdataFileName = icubContribEnvPath + localPath + effdataFileName;
    effData.open(effdataFileName.c_str(), ofstream::out | ofstream::app);

    closeThr=rf.check("closeThr", Value(DEFAULT_CLOSE_THR)).asDouble();
    simMode=rf.check("simMode", Value("off"),"simMode (string)").asString();

    //incoming
    fullBlobDescriptorInputPort.open(("/"+name+"/fullObjDesc:i").c_str());
    partsBlobDescriptorInputPort.open(("/"+name+"/partsObjDesc:i").c_str());
    
    targetPF.open(("/"+name+"/particle:i").c_str());
    blobExtractor.open(("/"+name+"/blobs:i").c_str());
    //bayesianNetInputPort.open(("/"+name+"/bayesnet:i").c_str());
    if (!affPredictionsInPort.open(("/"+name+"/affPred:i").c_str()))
    fprintf(stderr,"\n\nMERDA!\n\n");

    //outgoing
    simObjLoaderModuleOutputPort.open(("/"+name+"/simObjLoader:o").c_str());
    segmentPoint.open(("/"+name+"/segmentTarget:o").c_str());       //port to send off target Points to segmentator
    automatorReadyPort.open(("/"+name+"/automator:o").c_str());
    //bayesianNetOutputPort.open(("/"+name+"/bayesnet:o").c_str());
    affPredictionsOutPort.open(("/"+name+"/affPred:o").c_str());


    //rpc
    rpcHuman.open(("/"+name+"/human:rpc").c_str());                 //rpc server to interact with the italkManager
    rpcMotorAre.open(("/"+name+"/are:rpc").c_str());                //rpc server to query ARE
    rpcMotorKarma.open(("/"+name+"/karma:rpc").c_str());            //rpc server to query Karma
    rpcBlobSpot.open(("/"+name+"/blobSpot:rpc").c_str());           //rpc server to query blobSpotter
    rpcToolFinder.open(("/"+name+"/toolFinder:rpc").c_str());       //rpc server to query karma toolFinder

    srand(time(NULL));


    testBlob = new BlobInfo[3];

    n_tools = rf.check("tools",Value("1")).asInt();

    toolTransform.resize(n_tools);
    Bottle tool;
    char toolName[] = {'t', 'o', 'o', 'l', '0', '\0'};

    for (int toolIndex = 0; toolIndex<n_tools; toolIndex++) 
    {
        toolName[4]++;
        tool.clear();
        tool = rf.findGroup(toolName);
        toolTransform[toolIndex].resize(3);
        toolTransform[toolIndex][0] = tool.get(1).asDouble();
        toolTransform[toolIndex][1] = tool.get(2).asDouble();
        toolTransform[toolIndex][2] = tool.get(3).asDouble();
    }

    toolId=1;   //default tool

    toolTransformDefault.resize(3);
    toolTransformDefault[0] = TOOL_LEFT_TRANSFORM_DEFAULT[0];
    toolTransformDefault[1] = TOOL_LEFT_TRANSFORM_DEFAULT[1];
    toolTransformDefault[2] = TOOL_LEFT_TRANSFORM_DEFAULT[2];

    if (hand=="right")
    {
        toolTransformDefault[0] = TOOL_RIGHT_TRANSFORM_DEFAULT[0];
        toolTransformDefault[1] = TOOL_RIGHT_TRANSFORM_DEFAULT[1];
        toolTransformDefault[2] = TOOL_RIGHT_TRANSFORM_DEFAULT[2];
    }

    return true;
}

/**********************************************************/
bool Manager::interruptModule()
{
    segmentPoint.interrupt();
    blobExtractor.interrupt();
    rpcHuman.interrupt();
    rpcMotorAre.interrupt();
    rpcMotorKarma.interrupt();
    rpcToolFinder.interrupt();
    rpcBlobSpot.interrupt();
    targetPF.interrupt();
    fullBlobDescriptorInputPort.interrupt();
    partsBlobDescriptorInputPort.interrupt();
    simObjLoaderModuleOutputPort.interrupt();
    automatorReadyPort.interrupt();
//    bayesianNetInputPort.interrupt();
//    bayesianNetOutputPort.interrupt();

    return true;
}

/**********************************************************/
bool Manager::close()
{
    segmentPoint.close();
    blobExtractor.close();
    rpcHuman.close();
    rpcMotorAre.close();
    rpcMotorKarma.close();
    rpcToolFinder.close();
    rpcBlobSpot.close();
    targetPF.close();   
    descData.close();
    descDataMin.close();
    effData.close();
    fullBlobDescriptorInputPort.close();
    partsBlobDescriptorInputPort.close();
    simObjLoaderModuleOutputPort.close();
    automatorReadyPort.close();
//    bayesianNetInputPort.close();
//    bayesianNetOutputPort.close();

    return true;
}

/**********************************************************/
double Manager::getPeriod()
{
    return 0.1;
}

/**********************************************************/
bool Manager::updateModule()
{
    if (isStopping())
    {
        return false;
    }

    Bottle cmd, val, reply;
    Bottle cmdSim, replySim;

    Bottle autoGo;
    autoGo.addString("go");
    automatorReadyPort.write(autoGo);
    Time::delay(0.1);

    rpcHuman.read(cmd, true);

    int rxCmd=processHumanCmd(cmd,val);

    if (rxCmd==Vocab::encode("extend")) 
    {
        if (cmd.size() > 2) 
        {
            fprintf(stdout, "Will now use user selected arm and camera \n");
            hand   = cmd.get(1).asString().c_str();
            camera = cmd.get(2).asString().c_str();
        }
        else
        {
            fprintf(stdout, "Will now use default arm and cam \n");
        }

        lastTool.clear();
        lastTool = executeToolLearning();
        reply.addString("ack");
        rpcHuman.reply(reply);

        fprintf(stdout, "GOT TOOL at  %s, \n",lastTool.toString().c_str());
        goHomeArmsHead();

        yarp::sig::Vector tTransform;
        tTransform.resize(3);
        tTransform[0] = lastTool.get(0).asDouble();
        tTransform[1] = lastTool.get(1).asDouble();
        tTransform[2] = lastTool.get(2).asDouble();
        executeToolAttach(tTransform);
    }

    if (rxCmd==Vocab::encode("test")) 
    {
        testAction = cmd.get(1).asString();
    
        if (testAction.compare("draw")==0)
        {
            fprintf(stderr,"\n\n Will test a draw action\n\n");

            int draw_id = 1;

            goHomeArmsHead();    //look at the table (simplified version)
            computeAllDesc_TEST();

            makePrediction_TEST(testToolA, testTargetObject, draw_id, testPredA);
            makePrediction_TEST(testToolB, testTargetObject, draw_id, testPredB);

            fprintf(stderr,"\n\n Should I do the action? (y/n)\n\n");

            Bottle cmdAck,valAck;
            int ackCmd;

            rpcHuman.read(cmdAck, true);

            ackCmd=processHumanCmd(cmdAck,valAck);
            fprintf(stderr,"read...\n");

            while ( !(ackCmd==Vocab::encode("y") || ackCmd==Vocab::encode("n")) && !isStopping()) 
            {
                fprintf(stderr,"\n\n Should I do the action? (y/n)\n\n");
                rpcHuman.read(cmdAck, true);
                ackCmd=processHumanCmd(cmdAck,valAck);
                Time::delay(0.1);
            }

            if (ackCmd==Vocab::encode("y"))
            {
                fprintf(stderr,"\n\n Ok, will do the action\n\n");

                if (compareEffects_TEST(testPredA,testPredB)==1)
                {
                    makeAction_TEST(testToolA, testTargetObject, draw_id);
                }
                else if (compareEffects_TEST(testPredA,testPredB)==2)
                {
                    makeAction_TEST(testToolB, testTargetObject, draw_id);
                }
                else
                {
                    fprintf(stderr,"\n\n ERROR - Problem when comparing predictions \n\n");
                    fprintf(stderr,"\n\n ERROR - No actions will be executed \n\n");
                }   

            }
            else
            {
                fprintf(stderr,"\n\n Ok, will not do the action\n\n");
            }
            
        }

        goHomeArmsHead();
    }


    if (rxCmd==Vocab::encode("tip")) 
    {
        Bottle toSend;
        toSend.clear();
        toSend.addDouble(lastTool.get(0).asDouble());
        toSend.addDouble(lastTool.get(1).asDouble());
        toSend.addDouble(lastTool.get(2).asDouble());
        rpcHuman.reply(toSend);

        fprintf(stdout, "Replied with  %s, \n",toSend.toString().c_str());
    }

    /*
    // tool choice debug command, old 2014/02 tests, never used
    if (rxCmd==Vocab::encode("tdeb")) {
        int effectLon = cmd.get(1).asInt();
        int effectLat = cmd.get(2).asInt();
        int ximg, yimg;

        chooseTool(effectLon, effectLat, ximg, yimg);

        //fprintf(stdout, "\n\n******* x=%d  y=%d ************ \n\n",ximg,yimg);
        reply.addInt(ximg);
        reply.addInt(yimg);
        rpcHuman.reply(reply);

    }
    */


    if (rxCmd==Vocab::encode("desc")) 
    {
        string objectName;

        objectName = cmd.get(1).asString();

        reply.addString("desc");
        reply.addString(objectName.data());
        rpcHuman.reply(reply);

        goHomeArmsHead();

        if (simMode.compare("on")==0) 
        {
            objectId = cmd.get(1).asInt();
            //clears the space  in front of the robot
            cmdSim.clear();
            replySim.clear();
            cmdSim.addString("clea");
            simObjLoaderModuleOutputPort.write(cmdSim,replySim);
            //moves the table and object to the front of the robot
            cmdSim.clear();
            replySim.clear();
            cmdSim.addString("desc");
            cmdSim.addInt(objectId);
            simObjLoaderModuleOutputPort.write(cmdSim,replySim);
        }

        lookAtObject();
        computeObjectDesc();

        descData << "desc" << " ";
        if (simMode.compare("on")==0)
            descData << objectId << " ";
        else
            descData << objectName.data() << " ";
        descData << objDesc.toString() << " ";
        descData << objTopDesc.toString() << " ";
        descData << objBottomDesc.toString() << " ";
        descData << '\n';
        descData.close();
        descData.open(descdataFileName.c_str(), ofstream::out | ofstream::app);

        descDataMin << "descMin" << " ";
        if (simMode.compare("on")==0)
            descDataMin << objectId << " ";
        else
            descDataMin << objectName.data() << " ";
        descDataMin << objDesc.geomToString() << " ";
        descDataMin << objTopDesc.geomToString() << " ";
        descDataMin << '\n';
        descDataMin.close();
        descDataMin.open(descdataMinFileName.c_str(), ofstream::out | ofstream::app);

        if (simMode.compare("on")==0) 
        {
            //clears the space  in front of the robot
            cmdSim.clear();
            replySim.clear();
            cmdSim.addString("clea");
            simObjLoaderModuleOutputPort.write(cmdSim,replySim);
        }
    }

    if (rxCmd==Vocab::encode("eff")) 
    {
        actionId = cmd.get(1).asInt();

        if (simMode.compare("on")==0) 
        {
            //moves the table and object to the front of the robot
            cmdSim.clear();
            replySim.clear();
            cmdSim.addString("eff");
            cmdSim.addInt(targetSimNum);
            simObjLoaderModuleOutputPort.write(cmdSim,replySim);
        }

        reply.addString("eff");
        reply.addInt(actionId);
        reply.addString(toolName.data());
        reply.addString(targetName.data());
        rpcHuman.reply(reply);

        lookAtObject();
        updateObjVisPos();
        objectPos.clear();
        get3DPosition(objImgPos_onTable, objectPos);
        segmentAndTrack(objImgPos.x, objImgPos.y);  

        //fprintf(stderr,"eff %d %s %s\n", actionId, toolName.data(), targetName.data());
        
        effData << "eff" << " ";
        if (simMode.compare("on")==0)
        {
            effData << toolSimNum   << " ";
            effData << targetSimNum << " ";
        }
        else 
        {
            effData << toolName.data()   << " ";
            effData << targetName.data() << " ";
        }
        effData << actionId   << " ";

        yarp::sig::Vector objectPosTracker;
        yarp::sig::Vector *trackVec = targetPF.read(true);

        objImgPos.x = (*trackVec)[0];
        objImgPos.y = (*trackVec)[5];

        get3DPosition(objImgPos,objectPosTracker);

        fprintf(stderr,"\n\n***********Get FIRST object sample\n\n");

        effData << objectPosTracker[0] << " "
        << objectPosTracker[1] << " "
        << objectPosTracker[2] << " "
        << objImgPos.x << " "
        << objImgPos.y;

        performAction(); //on objectPos (not on objectPosTracker, which might be inaccurate)

        for (int i=0; i<motSteps; i++) 
        {
            fprintf(stderr,"Error: While reading and storing the tracker data");
            Time::delay(actionTime/(double)motSteps);
            trackVec = targetPF.read(true);
            objImgPos.x = (*trackVec)[0];
            objImgPos.y = (*trackVec)[5];
            objectPosTracker.clear();
            get3DPosition(objImgPos, objectPosTracker);

            effData << " " <<  objectPosTracker[0] << " "
            << objectPosTracker[1] << " "
            << objectPosTracker[2] << " "
            << objImgPos.x << " "
            << objImgPos.y;
        }
        
        goHomeArmsHead();

        Time::delay(0.5);

        fprintf(stderr,"\n\n**********Get LAST object sample\n\n");

        effData << " " <<  objectPosTracker[0] << " "
        << objectPosTracker[1] << " "
        << objectPosTracker[2] << " "
        << objImgPos.x << " "
        << objImgPos.y;

        effData << '\n';
        effData.close();
        effData.open(effdataFileName.c_str(), ofstream::out | ofstream::app);

        if (simMode.compare("on")==0) 
        {
            //removes the object and table from the front of the robot
            cmdSim.clear();
            replySim.clear();
            cmdSim.addString("remo");
            cmdSim.addInt(targetSimNum);
            simObjLoaderModuleOutputPort.write(cmdSim,replySim);
        }
    }

    if (rxCmd==Vocab::encode("grab")) 
    {
        if (cmd.size()>1)
        {
            toolName = cmd.get(1).asString();
        }
        else
        {
            toolName = "default";
        }

        reply.addString("grab");
        reply.addString(toolName.c_str());
        rpcHuman.reply(reply);

        if (simMode.compare("on")!=0) 
        {
            if (askForTool()) 
            {
                graspTool();

                /*
                // IF ALL TOOLS HAVE DIFFERENT TRANSFORMS
                if (cmd.size()>1)
		{
                    yarp::sig::Vector tTransform;
                    tTransform.resize(3);
                    tTransform[0] = cmd.get(2).asDouble();
                    tTransform[1] = cmd.get(3).asDouble();
                    tTransform[2] = cmd.get(4).asDouble();

		    if (executeToolAttach(tTransform))
		       fprintf(stderr,"\nTool attached \n");
		    else
		       fprintf(stderr,"\nERROR -- Problem in attaching the tool...\n"); 
		}
		else
		{
                    if (executeToolAttach(toolTransformDefault))
		       fprintf(stderr,"\nTool attached \n");
		    else
		       fprintf(stderr,"\nERROR -- Problem in attaching the tool...\n");
		} 
                */         

                // IF ALL TOOLS HAVE THE SAME, DEFAULT, TRANSFORM
                if (executeToolAttach(toolTransformDefault))
		    fprintf(stderr,"\nTool attached (default transform)\n");
		else
		    fprintf(stderr,"\nERROR -- Problem in attaching the tool...\n");  
                
            }

        }
        else 
        {
            if (cmd.size()>1)
            {
                toolSimNum = cmd.get(1).asInt();
            }
	    else
	    {
		toolSimNum = 1;
	    }
            //toolSimNum = cmd.get(1).asInt();

            goHomeArmsHead(); 
            cmdSim.clear(); //clears the space in front of the robot
            replySim.clear();
            cmdSim.addString("clea");
            simObjLoaderModuleOutputPort.write(cmdSim,replySim);
            //moves the tool to the hand of the robot, magnet ON
            cmdSim.clear();
            replySim.clear();
            cmdSim.addString("grab");
            cmdSim.addInt(toolSimNum);
            simObjLoaderModuleOutputPort.write(cmdSim,replySim);
            
            executeToolAttach(toolTransform[toolSimNum-1]);
        }

        goHomeArmsHead();
        reply.addString("grab:");
        reply.addString(toolName.c_str());
        rpcHuman.reply(reply);
    }

    if (rxCmd==Vocab::encode("drop")) 
    {

        if (simMode.compare("on")==0) 
        {
            //removes the tool from the hand of the robot, magnet OFF
            cmdSim.clear();
            replySim.clear();
            cmdSim.addString("drop");
            cmdSim.addInt(toolSimNum);
            simObjLoaderModuleOutputPort.write(cmdSim,replySim);
        }

        reply.addString("drop:");
        reply.addString(toolName.data());
        rpcHuman.reply(reply);

        toolName.clear();
        goHomeAll();
    }

    if (rxCmd==Vocab::encode("targ")) 
    {
        targetName = cmd.get(1).asString();
        if (simMode.compare("on")==0) 
        {
            targetSimNum = cmd.get(1).asInt();
        }

        reply.addString("target:");
        reply.addString(targetName.data());
        rpcHuman.reply(reply);
    }

    return true;
}

/**********************************************************/
int Manager::processHumanCmd(const Bottle &cmd, Bottle &b)
{
    int ret=Vocab::encode(cmd.get(0).asString().c_str());
    b.clear();
    if (cmd.size()>1) 
    {
        if (cmd.get(1).isList())
            b=*cmd.get(1).asList();
        else
            b=cmd.tail();
    }
    return ret;
}

/**********************************************************/
bool Manager::updateObjectDesc(const Bottle *msgO, const Bottle *msgOP)
{
    fprintf(stderr,"check descriptors...\n");
    int N_blobs, N_blobs_parts;
    N_blobs = msgO->get(0).asInt();
    printf("%d blobs have been detected\n", N_blobs);
    N_blobs_parts = msgOP->get(0).asInt();
    printf("%d blobs have been divided in parts (two parts each, top and bottom)\n", N_blobs_parts);
    
    printf("\n\nOnly the descriptors of one blob (the first from top left in the image) will be considered. \n");

    Bottle *objbot;

    if (msgO->size()>1)
    {
        yarp::os::Value& element = msgO->get(1);
        objbot = element.asList();

        //printf("list of %d elements\n", objbot->size());

        // width and height
        objDesc.roi_x           = objbot->get(0).asDouble();
        objDesc.roi_y           = objbot->get(1).asDouble();
        objDesc.roi_width       = objbot->get(2).asDouble();
        objDesc.roi_height      = objbot->get(3).asDouble();
        objDesc.angle           = objbot->get(4).asDouble();
        objDesc.special_point_x = objbot->get(5).asDouble();
        objDesc.special_point_y = objbot->get(6).asDouble();

        for (int j=0; j<16; j++) 
        {
             objDesc.hist[j]     = objbot->get(7+j).asDouble();
        }

        objDesc.area            = objbot->get(23).asDouble();
        objDesc.convexity       = objbot->get(24).asDouble();
        objDesc.eccentricity    = objbot->get(25).asDouble();
        objDesc.compactness     = objbot->get(26).asDouble();
        objDesc.circleness      = objbot->get(27).asDouble();
        objDesc.squareness      = objbot->get(28).asDouble();
        objDesc.elongatedness   = objbot->get(29).asDouble();
        
    }
    else
    {
        objDesc.clear();
    }
    
    if (msgOP->size()>1)
    {

        // -- OBJECT TOP PART (in the image) --
        yarp::os::Value& elementOP = msgOP->get(1);
        objbot = elementOP.asList()->get(0).asList();

        objTopDesc.roi_x           = objbot->get(0).asDouble();
        objTopDesc.roi_y           = objbot->get(1).asDouble();
        objTopDesc.area            = objbot->get(2).asDouble();
        objTopDesc.convexity       = objbot->get(3).asDouble();
        objTopDesc.eccentricity    = objbot->get(4).asDouble();
        objTopDesc.compactness     = objbot->get(5).asDouble();
        objTopDesc.circleness      = objbot->get(6).asDouble();
        objTopDesc.squareness      = objbot->get(7).asDouble();
        objTopDesc.elongatedness   = objbot->get(8).asDouble();

        // -- OBJECT BOTTOM PART (in the image) --
        objbot = elementOP.asList()->get(1).asList();

        objBottomDesc.roi_x           = objbot->get(0).asDouble();
        objBottomDesc.roi_y           = objbot->get(1).asDouble();
        objBottomDesc.area            = objbot->get(2).asDouble();
        objBottomDesc.convexity       = objbot->get(3).asDouble();
        objBottomDesc.eccentricity    = objbot->get(4).asDouble();
        objBottomDesc.compactness     = objbot->get(5).asDouble();
        objBottomDesc.circleness      = objbot->get(6).asDouble();
        objBottomDesc.squareness      = objbot->get(7).asDouble();
        objBottomDesc.elongatedness   = objbot->get(8).asDouble();

    }
    else
    {
        objTopDesc.clear();
        objBottomDesc.clear();
    }

    return true;
}

/**********************************************************/
bool Manager::updateAllDesc_TEST(const Bottle *msgO, const Bottle *msgOP)
{
    fprintf(stderr,"check descriptors...\n");
    int N_blobs, N_blobs_parts;
    N_blobs = msgO->get(0).asInt();
    printf("%d blobs have been detected\n", N_blobs);
    N_blobs_parts = msgOP->get(0).asInt();
    printf("%d blobs have been divided in parts (two parts each, top and bottom)\n", N_blobs_parts);

    if (N_blobs != N_blobs_parts)
    {
        fprintf(stderr, "\n\nERROR! Conflict in blobs detection\n");
        return false;
    }

    if (N_blobs < 3)
    {
        fprintf(stderr, "\n\nERROR! Less than 3 blobs detected\n");
        return false;
    }
    
    printf("\n\nOnly the descriptors of three blobs will be considered: \n");
    printf("- The leftmost one (TOOL A) \n");
    printf("- The rightmost one (TOOL B) \n");
    printf("- One in the middle (the target object) \n\n");

    int toolA_index = 0;
    int toolB_index = 0;
    int target_index = 0;

    double minX = 500;
    double maxX = 0;

    Bottle *objbot;

    for (int i=0; i<3; i++)
    {
        yarp::os::Value& element = msgO->get(i+1);
        objbot = element.asList();

        //printf("list of %d elements\n", objbot->size());
        fprintf(stderr, "\nstart reading data \n");

        // width and height
        testBlob[i].roi_x           = objbot->get(0).asDouble();
        testBlob[i].roi_y           = objbot->get(1).asDouble();
        testBlob[i].roi_width       = objbot->get(2).asDouble();
        testBlob[i].roi_height      = objbot->get(3).asDouble();
        testBlob[i].angle           = objbot->get(4).asDouble();
        testBlob[i].special_point_x = objbot->get(5).asDouble();
        testBlob[i].special_point_y = objbot->get(6).asDouble();

        for (int j=0; j<16; j++) 
        {
             testBlob[i].hist[j]     = objbot->get(7+j).asDouble();
        }

        testBlob[i].area            = objbot->get(23).asDouble();
        testBlob[i].convexity       = objbot->get(24).asDouble();
        testBlob[i].eccentricity    = objbot->get(25).asDouble();
        testBlob[i].compactness     = objbot->get(26).asDouble();
        testBlob[i].circleness      = objbot->get(27).asDouble();
        testBlob[i].squareness      = objbot->get(28).asDouble();
        testBlob[i].elongatedness   = objbot->get(29).asDouble();

        fprintf(stderr, "\ncheck min x \n");

        if (testBlob[i].roi_x < minX)
        {
            toolA_index = i;
            minX = testBlob[i].roi_x;
        }

        fprintf(stderr, "\ncheck max x \n");

        if (testBlob[i].roi_x > maxX)
        {
            toolB_index = i;
            maxX = testBlob[i].roi_x;
        }

        fprintf(stderr, "\nend of loop 1 \n");
        
    }

    printf("\n\nTools indexes: \n");
    printf("- TOOL A: %d\n", toolA_index);
    printf("- TOOL B: %d \n", toolB_index);

    for (int i=0; i<3; i++)
    {
    
        fprintf(stderr, "\nstart of loop 2 \n");

        if (i==toolA_index)
        {
            fprintf(stderr, "\ntool A - debug 1 \n");
            yarp::os::Value& elementOPA = msgOP->get(i+1);
            fprintf(stderr, "\ntool A - debug 2 \n");
            objbot = elementOPA.asList()->get(0).asList();

            fprintf(stderr, "\nread TOOL A data \n");

            testToolA.roi_x           = objbot->get(0).asDouble();
            testToolA.roi_y           = objbot->get(1).asDouble();
            testToolA.area            = objbot->get(2).asDouble();
            testToolA.convexity       = objbot->get(3).asDouble();
            testToolA.eccentricity    = objbot->get(4).asDouble();
            testToolA.compactness     = objbot->get(5).asDouble();
            testToolA.circleness      = objbot->get(6).asDouble();
            testToolA.squareness      = objbot->get(7).asDouble();
            testToolA.elongatedness   = objbot->get(8).asDouble();
        }
        else if (i==toolB_index)
        {
            fprintf(stderr, "\ntool B - debug 1 \n");

            yarp::os::Value& elementOPB = msgOP->get(i+1);
            objbot = elementOPB.asList()->get(0).asList();

            fprintf(stderr, "\nread TOOL B data \n");

            testToolB.roi_x           = objbot->get(0).asDouble();
            testToolB.roi_y           = objbot->get(1).asDouble();
            testToolB.area            = objbot->get(2).asDouble();
            testToolB.convexity       = objbot->get(3).asDouble();
            testToolB.eccentricity    = objbot->get(4).asDouble();
            testToolB.compactness     = objbot->get(5).asDouble();
            testToolB.circleness      = objbot->get(6).asDouble();
            testToolB.squareness      = objbot->get(7).asDouble();
            testToolB.elongatedness   = objbot->get(8).asDouble();
        }
        else
        {
            fprintf(stderr, "\nread OBJECT data \n");

            testTargetObject.roi_x           = testBlob[i].roi_x;
            testTargetObject.roi_y           = testBlob[i].roi_y;
            testTargetObject.special_point_x           = testBlob[i].special_point_x;
            testTargetObject.special_point_y           = testBlob[i].special_point_y;
            testTargetObject.area            = testBlob[i].area;
            testTargetObject.convexity       = testBlob[i].convexity;
            testTargetObject.eccentricity    = testBlob[i].eccentricity;
            testTargetObject.compactness     = testBlob[i].compactness;
            testTargetObject.circleness      = testBlob[i].circleness;
            testTargetObject.squareness      = testBlob[i].squareness;
            testTargetObject.elongatedness   = testBlob[i].elongatedness;
        }

        fprintf(stderr, "\nend of loop 2 \n");

    }

    printf("\n\nTest objects: \n");
    printf("- TOOL A: %d %d \n", (int)testBlob[toolA_index].roi_x, (int)testBlob[toolA_index].roi_y);
    printf("- %s ", testBlob[toolA_index].geomToString().c_str());
    printf("\n");
    printf("- %s ", testToolA.geomToString().c_str());
    
    printf("\n\n- TOOL B: %d %d \n", (int)testBlob[toolB_index].roi_x, (int)testBlob[toolB_index].roi_y);
    printf("- %s ", testBlob[toolB_index].geomToString().c_str());
    printf("\n");
    printf("- %s ", testToolB.geomToString().c_str());

    printf("\n\n- TARGET OBJECT: %d %d \n\n", (int)testTargetObject.roi_x, (int)testTargetObject.roi_y);  
    
    return true;
}

/***********************************************************/
bool Manager::makePrediction_TEST(BlobPartInfo tool, BlobInfo object, int action, double* effects)
{
   
    Bottle query;
    Bottle queryResponse;

    query.clear();

    query.addDouble(tool.convexity);
    query.addDouble(tool.eccentricity);
    query.addDouble(tool.compactness);
    query.addDouble(tool.circleness);
    query.addDouble(tool.elongatedness);

    query.addDouble(object.convexity);
    query.addDouble(object.eccentricity);
    query.addDouble(object.compactness);
    query.addDouble(object.circleness);
    query.addDouble(object.elongatedness);

    query.addDouble(action);

    printf("\n\nSending query to affordance network: \n");
    printf("%s\n", query.toString().c_str());

    affPredictionsOutPort.write(query);

    bool got=false;
    while (!got)
    {
        got=affPredictionsInPort.read(queryResponse);
    }
        
    if (queryResponse.size() < 1)
    {
        fprintf(stderr,"\n\nERROR - No response to query \n");
        return false;
    }

    for (int i=0; i<5; i++)
    {
        for (int j=0; j<5; j++)
        {
            effects[i*5+j]=queryResponse.get(0).asList()->get(i).asList()->get(j).asDouble();
        }
    }

    printf("\n\nPrediction received: \n");
    for (int i=0; i<5; i++)
    {
        for (int j=0; j<5; j++)
        {
            printf("%.2lf ", effects[i*5+j]);
        }
        printf("\n");
    }
    printf("\n\n");

}

/*************************************************************/
int Manager::compareEffects_TEST(double* A, double* B)
{
    double scoreA=0;
    double scoreB=0;

    int ret=0;

    //sum the probabilities of the 4' and 5' line
    for (int i=3; i<5; i++)
    {
        for (int j=0; j<5; j++)
        {
            scoreA=scoreA+A[i*5+j];
        }
    }

    //sum the probabilities of the 4' and 5' line
    for (int i=3; i<5; i++)
    {
        for (int j=0; j<5; j++)
        {
            scoreB=scoreB+B[i*5+j];
        }
    }

    fprintf(stderr, "\n\nSCORE A= %.2lf \n", scoreA);
    fprintf(stderr, "\nSCORE B= %.2lf \n\n", scoreB);

    if (scoreA >= scoreB)
    {
        ret=1;
    }
    else if (scoreA < scoreB)
    {
        ret=2;
    }

    return ret;

}

/***************************************************************/
bool Manager::makeAction_TEST(BlobPartInfo tool, BlobInfo object, int action)
{
    yarp::sig::Vector actingPos;
    actingPos.resize(3);

    yarp::sig::Vector pointingPos;
    pointingPos.resize(3);

    testToolPos_onTable.x = tool.roi_x;
    testToolPos_onTable.y = tool.roi_y;
    targetPos_onTable.x = object.special_point_x;
    targetPos_onTable.y = object.special_point_y;

    get3DPosition(testToolPos_onTable, pointingPos);
    get3DPosition(targetPos_onTable, actingPos);

    fprintf(stderr, "\n\nTool position = %.2lf %.2lf %.2lf \n", pointingPos[0], pointingPos[1], pointingPos[2]);
    fprintf(stderr, "\n Object position = %.2lf %.2lf %.2lf \n", actingPos[0], actingPos[1], actingPos[2]);

    //add offsets if needed
    pointingPos[0]=pointingPos[0];
    pointingPos[1]=pointingPos[1];
    pointingPos[2]=pointingPos[2] + tableHeightOffset*2.0;

    //add offsets if needed
    actingPos[0]=actingPos[0];
    actingPos[1]=actingPos[1];
    actingPos[2]=actingPos[2] + tableHeightOffset;

    fprintf(stdout,"\n\nI want to use this tool!\n");
    //point to tool
    Bottle cmdAre, replyAre;
    cmdAre.clear();
    replyAre.clear();
    cmdAre.addString("point");
    Bottle &tmpList=cmdAre.addList();
    tmpList.addDouble(pointingPos[0]);
    tmpList.addDouble(pointingPos[1]);
    tmpList.addDouble(pointingPos[2]);
    cmdAre.addString("left");
    rpcMotorAre.write(cmdAre,replyAre);
    fprintf(stdout,"cmd sent: %s\n",cmdAre.toString().c_str());
    fprintf(stdout,"reply: %s\n",replyAre.toString().c_str());

    if (askForTool()) 
    {
        graspTool();       

        // IF ALL TOOLS HAVE THE SAME, DEFAULT, TRANSFORM
        if (executeToolAttach(toolTransformDefault))
        {
            fprintf(stderr,"\nTool attached (default transform)\n");
        }
	else
        {
            fprintf(stderr,"\nERROR -- Problem in attaching the tool...\n");  
        }
                
    }

    cmdAre.clear();
    replyAre.clear();
    cmdAre.addString("look");
    cmdAre.addString("hand");
    cmdAre.addString("left");
    rpcMotorAre.write(cmdAre,replyAre);
    fprintf(stdout,"cmd sent: %s\n",cmdAre.toString().c_str());
    fprintf(stdout,"reply: %s\n",replyAre.toString().c_str());

    fprintf(stderr,"\n Draw simulation: \n");

    Bottle karmaMotor, karmaReply;
    karmaMotor.clear();
    karmaReply.clear();        
    karmaMotor.addString("vdra");
    karmaMotor.addDouble(actingPos[0]);
    karmaMotor.addDouble(actingPos[1]);
    karmaMotor.addDouble(actingPos[2]);
    karmaMotor.addDouble(90.0); // direction (before it was 180.0)
    karmaMotor.addDouble(objectSizeOffset); // initial tool-object distance
    karmaMotor.addDouble(MOVEMENT_LENGTH); // movement lenght
    fprintf(stdout,"Will now send to karmaMotor:\n");
    rpcMotorKarma.write(karmaMotor, karmaReply);
    fprintf(stdout,"cmd sent: %s\n",karmaMotor.toString().c_str());
    fprintf(stdout,"reply: %s\n",karmaReply.toString().c_str());

    if (karmaReply.get(1).asDouble()<VDRAW_THR)
    {
        fprintf(stderr,"\n...success!\n\n");
        fprintf(stderr,"\n Draw action: \n");

        karmaMotor.clear();
        karmaMotor.addString("draw");
        karmaMotor.addDouble(actingPos[0]);
        karmaMotor.addDouble(actingPos[1]);
        karmaMotor.addDouble(actingPos[2]);
        karmaMotor.addDouble(90.0); // direction (before it was 180.0)
        karmaMotor.addDouble(objectSizeOffset); // initial tool-object distance
        karmaMotor.addDouble(MOVEMENT_LENGTH); // movement lenght
        fprintf(stdout,"Will now send to karmaMotor:\n");
        karmaReply.clear();
        rpcMotorKarma.write(karmaMotor, karmaReply);
        fprintf(stdout,"cmd sent: %s\n",karmaMotor.toString().c_str());
        fprintf(stdout,"reply: %s\n",karmaReply.toString().c_str());
    }
    else 
    {
        fprintf(stderr,"iCub: Sorry man, cannot do that :( \n");
    }   

    return true;
}

/**********************************************************/
void Manager::updateObjVisPos()
{
    computeObjectDesc();

    objImgPos.x=objDesc.roi_x;
    objImgPos.y=objDesc.roi_y;

    objImgPos_onTable.x=objDesc.special_point_x;
    objImgPos_onTable.y=objDesc.special_point_y;
}

/**********************************************************/

void Manager::performAction()
{
    //Attach the tool
    //executeToolAttach(toolTransform[toolId-1]);

    Bottle karmaMotor, KarmaReply;
    int pose=handNaturalPose;

    yarp::sig::Vector actPos;
    actPos.resize(3);

    if (simMode.compare("on")==0) 
    {
        actPos[0]=-0.476;
        actPos[1]=-0.100;
        actPos[2]=-0.095;

        fprintf(stdout,"Sim Object in position: x=-0.476 y=-0.100  z=-0.100\n");
    }
    else 
    {
        //actPos[0]=objectPos[0] - objectSizeOffset;      // takes in consideration the object dimension, for an object of about objectSizeOffset radius. This might be taken from the object descriptors as well... (FUTURE WORKS)
        actPos[0]=objectPos[0];
        actPos[1]=objectPos[1];
        actPos[2]=objectPos[2] + tableHeightOffset;     // define table offset
    }

    fprintf(stdout,"Acting on position: x=%+.3f y=%+.3f z=%+.3f\n",actPos[0],actPos[1],actPos[2]);

    yarp::sig::Vector *trackVec = targetPF.read(true);
    yarp::sig::Vector vec = *trackVec;
    objImgPos.x=vec[0];
    objImgPos.y=vec[5];
    yarp::sig::Vector objectPosTracker;
    get3DPosition(objImgPos,objectPosTracker);

    x_start = objectPosTracker[0];
    y_start = objectPosTracker[1];
    z_start = objectPosTracker[2];
    u_start = objImgPos.x;
    v_start = objImgPos.y;

    //Bottle karmaMotor, KarmaReply;

    switch(actionId) 
    {
        case 1:
            fprintf(stderr,"\n Tap from right \n");

            fprintf(stdout,"Will now send to karmaMotor:\n");
            //karmaMotor.addString("pusp");  //when using this method, you should add the 'pose' parameter
            karmaMotor.addString("push");
            //karmaMotor.addInt(pose);
            karmaMotor.addDouble(actPos[0]);
            karmaMotor.addDouble(actPos[1]); 
            karmaMotor.addDouble(actPos[2]);
            karmaMotor.addDouble(90.0); // direction (from right)
            karmaMotor.addDouble(objectSizeOffset); // Amount of the movement. It takes in consideration the object dimension, for an object of about objectSizeOffset radius. This might be taken from the object descriptors as well... (FUTURE WORKS)

            fprintf(stdout,"%s\n",karmaMotor.toString().c_str());
            actionStartTime=Time::now();
            rpcMotorKarma.write(karmaMotor, KarmaReply);
            actionDurationTime=Time::now()-actionStartTime;
            fprintf(stdout,"action is %s:\n",KarmaReply.toString().c_str());
            fprintf(stdout,"Action duration time was: %.3lf\n",actionDurationTime);
            break;

        case 2:
            fprintf(stderr,"\n Tap from left: \n");

            fprintf(stdout,"Will now send to karmaMotor:\n");
            //karmaMotor.addString("pusp");  //when using this method, you should add the 'pose' parameter
            karmaMotor.addString("push");
            //karmaMotor.addInt(pose);
            karmaMotor.addDouble(actPos[0]);
            karmaMotor.addDouble(actPos[1]); 
            karmaMotor.addDouble(actPos[2]);
            karmaMotor.addDouble(270.0); // direction (from left)
            karmaMotor.addDouble(objectSizeOffset); // Amount of the movement. It takes in consideration the object dimension, for an object of about objectSizeOffset radius. This might be taken from the object descriptors as well... (FUTURE WORKS)

            fprintf(stdout,"%s\n",karmaMotor.toString().c_str());
            actionStartTime=Time::now();
            rpcMotorKarma.write(karmaMotor, KarmaReply);
            actionDurationTime=Time::now()-actionStartTime;
            fprintf(stdout,"outcome is %s:\n",KarmaReply.toString().c_str());
            fprintf(stdout,"Action duration time was: %.3lf\n",actionDurationTime);
            break;

        case 3:
            fprintf(stderr,"\n Drawing simulation: \n");

            //karmaMotor.addString("vdrp");  //when using this method, you should add the 'pose' parameter
            karmaMotor.addString("vdra");
            //karmaMotor.addInt(pose);
            karmaMotor.addDouble(actPos[0]);
            karmaMotor.addDouble(actPos[1]);
            karmaMotor.addDouble(actPos[2]);
            karmaMotor.addDouble(90.0); // direction (before it was 180.0)
            karmaMotor.addDouble(objectSizeOffset); // initial tool-object distance
            karmaMotor.addDouble(MOVEMENT_LENGTH); // movement lenght
            fprintf(stdout,"Will now send to karmaMotor:\n");
            fprintf(stdout,"%s\n",karmaMotor.toString().c_str());
            rpcMotorKarma.write(karmaMotor, KarmaReply);
            fprintf(stdout,"outcome is %s:\n",KarmaReply.toString().c_str());

            if (KarmaReply.get(1).asDouble()<VDRAW_THR)
            {
                fprintf(stderr,"\n Draw: \n");

                karmaMotor.clear();
                //karmaMotor.addString("drap");  //when using this method, you should add the 'pose' parameter
                karmaMotor.addString("draw");
                //karmaMotor.addInt(pose);
                karmaMotor.addDouble(actPos[0]);
                karmaMotor.addDouble(actPos[1]);
                karmaMotor.addDouble(actPos[2]);
                karmaMotor.addDouble(90.0); // direction (before it was 180.0)
                karmaMotor.addDouble(objectSizeOffset); // initial tool-object distance
                karmaMotor.addDouble(MOVEMENT_LENGTH); // movement lenght
                fprintf(stdout,"Will now send to karmaMotor:\n");
                fprintf(stdout,"%s\n",karmaMotor.toString().c_str());
                KarmaReply.clear();
                actionStartTime=Time::now();
                rpcMotorKarma.write(karmaMotor, KarmaReply);
                actionDurationTime=Time::now()-actionStartTime;
                fprintf(stdout,"outcome is %s:\n",KarmaReply.toString().c_str());
                fprintf(stdout,"Action duration time was: %.3lf\n",actionDurationTime);
            }
            else 
            {
                fprintf(stderr,"iCub: Sorry man, cannot do that :( \n");
            }
            break;

        case 4:
            fprintf(stderr,"\n Pushing simulation: \n");

            //karmaMotor.addString("vdrp");  //when using this method, you should add the 'pose' parameter
            karmaMotor.addString("vdra");
            //karmaMotor.addInt(pose);
            karmaMotor.addDouble(actPos[0]);
            karmaMotor.addDouble(actPos[1]);
            karmaMotor.addDouble(actPos[2] + 0.03);
            karmaMotor.addDouble(90.0); // direction (before it was 180.0)
            karmaMotor.addDouble(-objectSizeOffset*2.0 - 0.05); // initial tool-object distance
            karmaMotor.addDouble(-MOVEMENT_LENGTH); // movement lenght
            fprintf(stdout,"Will now send to karmaMotor:\n");
            fprintf(stdout,"%s\n",karmaMotor.toString().c_str());
            rpcMotorKarma.write(karmaMotor, KarmaReply);
            fprintf(stdout,"outcome is %s:\n",KarmaReply.toString().c_str());

            if (KarmaReply.get(1).asDouble()<VDRAW_THR) 
            {
                fprintf(stderr,"\n Push (opposite draw) \n");

                karmaMotor.clear();
                //karmaMotor.addString("drap"); //when using this method, you should add the 'pose' parameter
                karmaMotor.addString("draw");
                //karmaMotor.addInt(pose);
                karmaMotor.addDouble(actPos[0]);
                karmaMotor.addDouble(actPos[1]);
                karmaMotor.addDouble(actPos[2] + 0.03);  //table offset is bigger for the Push action
                karmaMotor.addDouble(90.0); // direction (before it was 180.0)
                karmaMotor.addDouble(-objectSizeOffset*2.0 - 0.05); // initial tool-object distance
                karmaMotor.addDouble(-MOVEMENT_LENGTH); // movement lenght
                fprintf(stdout,"Will now send to karmaMotor:\n");
                fprintf(stdout,"%s\n",karmaMotor.toString().c_str());
                KarmaReply.clear();
                actionStartTime=Time::now();
                rpcMotorKarma.write(karmaMotor, KarmaReply);
                actionDurationTime=Time::now()-actionStartTime;
                fprintf(stdout,"outcome is %s:\n",KarmaReply.toString().c_str());
                fprintf(stdout,"Action duration time was: %.3lf\n",actionDurationTime);
            }
            else 
            {
                fprintf(stderr,"iCub: Sorry man, cannot do that :( \n");
            }
            break;

        default:
            fprintf(stderr,"\n Error! Doing nothing... \n");
    }
}

/**********************************************************/
void Manager::lookAtRack()
{
    // TODO: implement lookAtRack()
}


/**********************************************************/
int Manager::lookAtTool()
{
    //Asks for tool:

    //if the tool is on the rack, the robot should look at the rack
    //lookAtRack();

    //if the tool is on the rack, the expected background (for blob segmentation) should be modified from "table" to "rack"
/*
    fprintf(stdout,"Looking at the rack...\n");
    Bottle cmdBlobSpot,replyBlobSpot;
    cmdBlobSpot.clear();
    replyBlobSpot.clear();
    cmdBlobSpot.addString("setHist");
    cmdBlobSpot.addInt(2); //rack-histogram id (if the tool is on the rack... otherwise should be 1, the table id, or anyway the proper one...)
    fprintf(stdout,"CMD: %s\n",cmdBlobSpot.toString().c_str());
    rpcBlobSpot.write(cmdBlobSpot, replyBlobSpot);
    fprintf(stdout,"REPLY: %s:\n",replyBlobSpot.toString().c_str());
*/
    //else [look at the table]...

    //Positions the head in the home position:
    fprintf(stdout,"Start 'home' 'head' proceedure:\n");
    Bottle cmdAre,replyAre,reply;
    cmdAre.clear();
    replyAre.clear();
    cmdAre.addString("home");
    cmdAre.addString("head");
    fprintf(stdout,"%s\n",cmdAre.toString().c_str());
    rpcMotorAre.write(cmdAre, replyAre);
    fprintf(stdout,"'home' 'head' action is %s:\n",replyAre.toString().c_str());

    Bottle autoGo;
    autoGo.addString("go");
    automatorReadyPort.write(autoGo);

    Bottle cmd,val;
    int rxCmd;

    rpcHuman.read(cmd, true);
    rxCmd=processHumanCmd(cmd,val);
    fprintf(stderr,"read...\n");

    //waits for a "done" message

    while (rxCmd!=Vocab::encode("done") && !isStopping()) 
    {
        fprintf(stderr,"looping...\n");
        rpcHuman.read(cmd, true);
        rxCmd=processHumanCmd(cmd,val);
        Time::delay(0.1);
    }

    reply.addString("done");
    rpcHuman.reply(reply);

    return 1;
}

/**********************************************************/
void Manager::lookAtObject()
{
    fprintf(stderr,"\niCub: Show me an object please...\n");

    //Positions the head in the home position:
    fprintf(stdout,"Start 'home' 'head' proceedure:\n");
    Bottle cmdAre,replyAre,reply;
    cmdAre.clear();
    replyAre.clear();
    cmdAre.addString("home");
    cmdAre.addString("head");
    fprintf(stdout,"%s\n",cmdAre.toString().c_str());
    rpcMotorAre.write(cmdAre, replyAre);
    fprintf(stdout,"'home' 'head' action is %s:\n",replyAre.toString().c_str());

    //if the object is on the table, the expected background (for blob segmentation) should be "table"
    fprintf(stdout,"Looking at the table...\n");
    Bottle cmdBlobSpot,replyBlobSpot;
    cmdBlobSpot.clear();
    replyBlobSpot.clear();
    cmdBlobSpot.addString("setHist");
    cmdBlobSpot.addInt(1); //table-histogram id (if the object is on the table...)
    fprintf(stdout,"CMD: %s\n",cmdBlobSpot.toString().c_str());
    rpcBlobSpot.write(cmdBlobSpot, replyBlobSpot);
    fprintf(stdout,"REPLY: %s:\n",replyBlobSpot.toString().c_str());

    Bottle autoGo;
    autoGo.addString("go");
    automatorReadyPort.write(autoGo);

    Bottle cmd,val;
    int rxCmd;

    rpcHuman.read(cmd, true);

    rxCmd=processHumanCmd(cmd,val);
    fprintf(stderr,"read...\n");

    while (rxCmd!=Vocab::encode("done") && !isStopping()) 
    {
        fprintf(stderr,"looping...\n");
        rpcHuman.read(cmd, true);
        rxCmd=processHumanCmd(cmd,val);
        Time::delay(0.1);
    }

    reply.addString("done");
    rpcHuman.reply(reply);
}


/**********************************************************/
void Manager::computeObjectDesc()
{
    fprintf(stderr,"update object description...\n");

    fprintf(stderr,"get entire-blob descriptors...\n");
    //Reads data from the port connected to the blobDescriptor:
    Bottle *objBottle = fullBlobDescriptorInputPort.read(true);

    fprintf(stderr,"get blob-parts descriptors...\n");
    //Reads data from the tools-port connected to the blobDescriptor:
    Bottle *objPartsBottle = partsBlobDescriptorInputPort.read(true);

    fprintf(stderr,"check blobs...\n");

    if (objBottle->size()>1) 
    {
        updateObjectDesc(objBottle, objPartsBottle);
        fprintf(stderr,"msg received...\n");
        fprintf(stderr,"Position of the object center in image field is: \n U = %d - V = %d \n", (int) objDesc.roi_x, (int) objDesc.roi_y);
    }
    else
    {
        fprintf(stderr,"\n\n[WARNING] - No blobs detected! \n\n");
    }
    fprintf(stderr,"...done!\n");

    //Writes data to the port connected to the knowledge database:
    //knowledgeOutputPort.write(toolBottle, response);
}

/*************************************************************/
void Manager::computeAllDesc_TEST()
{
    fprintf(stderr,"update objects and tools descriptions...\n");

    fprintf(stderr,"get entire-blob descriptors...\n");
    //Reads data from the port connected to the blobDescriptor:
    Bottle *objBottle = fullBlobDescriptorInputPort.read(true);

    fprintf(stderr,"get blob-parts descriptors...\n");
    //Reads data from the tools-port connected to the blobDescriptor:
    Bottle *objPartsBottle = partsBlobDescriptorInputPort.read(true);

    fprintf(stderr,"check blobs...\n");

    if (objBottle->size()>1) 
    {
        updateAllDesc_TEST(objBottle, objPartsBottle);
        fprintf(stderr,"msg received...\n");
        //fprintf(stderr,"Position of the object center in image field is: \n U = %d - V = %d \n", (int) objDesc.roi_x, (int) objDesc.roi_y);
    }
    else
    {
        fprintf(stderr,"\n\n[WARNING] - No blobs detected! \n\n");
    }
    fprintf(stderr,"...done!\n");

    //Writes data to the port connected to the knowledge database:
    //knowledgeOutputPort.write(toolBottle, response);
}

/**********************************************************/
int Manager::askForTool()
{
    //Asks for tool:

    //gets ready to take tool with the left/right hand:
    fprintf(stdout,"Will now start 'tato' '%s' (take tool %s) proceedure:\n",hand.data(),hand.data());
    Bottle cmdAre,replyAre;
    cmdAre.clear();
    replyAre.clear();
    cmdAre.addString("tato");
    cmdAre.addString(hand.c_str());
    fprintf(stdout,"cmd sent: %s\n", cmdAre.toString().c_str());
    rpcMotorAre.write(cmdAre, replyAre);
    fprintf(stdout,"reply: %s\n", replyAre.toString().c_str());

    return 1;
}

/**********************************************************/
void Manager::graspTool()
{
    Time::delay(1.0);
    fprintf(stdout,"Start 'clto' '%s' (close tool %s) proceedure:\n",hand.c_str(),hand.c_str());
    Bottle cmdAre,replyAre;
    cmdAre.clear();
    replyAre.clear();
    cmdAre.addString("clto");
    cmdAre.addString(hand.c_str());
    fprintf(stdout,"cmd sent: %s\n",cmdAre.toString().c_str());
    rpcMotorAre.write(cmdAre, replyAre);
    fprintf(stdout,"reply: %s\n",replyAre.toString().c_str());
}

/**********************************************************/
Bottle Manager::executeToolLearning()
{
    //to fill with data for karmaMotor
    fprintf(stdout,"Will now start the tool learn proceedure:\n");
    Bottle karmaMotor,KarmaReply;
    karmaMotor.clear();
    KarmaReply.clear();
    karmaMotor.addString("find");
    karmaMotor.addString(hand.c_str());
    karmaMotor.addString(camera.c_str());
    fprintf(stdout,"%s\n",karmaMotor.toString().c_str());
    rpcMotorKarma.write(karmaMotor, KarmaReply);
    fprintf(stdout,"action is %s:\n",KarmaReply.toString().c_str());

    Bottle toReturn;
    toReturn.clear();
    toReturn.addDouble( KarmaReply.get(1).asDouble() );
    toReturn.addDouble( KarmaReply.get(2).asDouble() );
    toReturn.addDouble( KarmaReply.get(3).asDouble() );
    return toReturn;
}

/**********************************************************/
void Manager::segmentAndTrack( int x, int y )
{
    Bottle toSegment;
    toSegment.clear();

    toSegment.addInt(x);
    toSegment.addInt(y);
    toSegment.addInt(80);
    toSegment.addInt(80);
    fprintf(stdout, "segmenting cmd is %s\n",toSegment.toString().c_str());
    segmentPoint.write(toSegment);

    Time::delay(1);

    Bottle cmdAre, replyAre;
    cmdAre.addString("track");
    cmdAre.addString("track");
    cmdAre.addString("no_sacc");
    rpcMotorAre.write(cmdAre,replyAre);
    fprintf(stdout,"tracking started: %s\n",replyAre.toString().c_str());
}

/**********************************************************/
void Manager::goHomeArmsHead()
{
    Bottle cmdAre, replyAre;
    cmdAre.clear();
    replyAre.clear();
    cmdAre.addString("home");
    cmdAre.addString("arms");
    cmdAre.addString("head");
    rpcMotorAre.write(cmdAre,replyAre);
    fprintf(stdout,"gone home %s:\n",replyAre.toString().c_str());
}

/**********************************************************/
void Manager::goHomeAll()
{
    Bottle cmdAre, replyAre;
    cmdAre.clear();
    replyAre.clear();
    cmdAre.addString("home");
    cmdAre.addString("all");
    rpcMotorAre.write(cmdAre,replyAre);
    fprintf(stdout,"gone home %s:\n",replyAre.toString().c_str());
}

/**********************************************************/
void Manager::goHomeHands()
{
    Bottle cmdAre, replyAre;
    cmdAre.clear();
    replyAre.clear();
    cmdAre.addString("home");
    cmdAre.addString("hands");
    rpcMotorAre.write(cmdAre,replyAre);
    fprintf(stdout,"gone home %s:\n",replyAre.toString().c_str());
}

/**********************************************************/
int Manager::executeToolAttach(const Vector &tool)
{
    fprintf(stderr,"Reset tool transform.\n");
    Bottle karmaMotor, KarmaReply;
    Bottle toolFinderCmd, toolFinderRpl;

    karmaMotor.addString("tool");
    karmaMotor.addString("remove");
    rpcMotorKarma.write(karmaMotor, KarmaReply);
    fprintf(stderr,"cmd sent: %s\n",karmaMotor.toString().c_str());
    fprintf(stderr,"reply: %s\n",KarmaReply.toString().c_str());

    karmaMotor.clear();
    KarmaReply.clear();
    karmaMotor.addString("tool");
    karmaMotor.addString("attach");
    karmaMotor.addString(hand.c_str());
    karmaMotor.addDouble(tool[0]);
    karmaMotor.addDouble(tool[1]);
    karmaMotor.addDouble(tool[2]);
    fprintf(stderr,"Will now send to karmaMotor:\n");
    rpcMotorKarma.write(karmaMotor, KarmaReply);
    fprintf(stderr,"cmd sent: %s\n",karmaMotor.toString().c_str());
    fprintf(stderr,"reply: %s\n",KarmaReply.toString().c_str());

    karmaMotor.clear();
    KarmaReply.clear();
    karmaMotor.addString("tool");
    karmaMotor.addString("get");
    rpcMotorKarma.write(karmaMotor, KarmaReply);
    fprintf(stderr,"cmd sent: %s\n",karmaMotor.toString().c_str());
    fprintf(stderr,"reply: %s\n",KarmaReply.toString().c_str());

    toolFinderCmd.clear();
    toolFinderRpl.clear();
    toolFinderCmd.addString("select");
    toolFinderCmd.addString(hand.c_str());
    toolFinderCmd.addString(hand.c_str());
    rpcToolFinder.write(toolFinderCmd, toolFinderRpl);

    toolFinderCmd.clear();
    toolFinderRpl.clear();
    toolFinderCmd.addString("show");
    toolFinderCmd.addDouble(tool[0]);
    toolFinderCmd.addDouble(tool[1]);
    toolFinderCmd.addDouble(tool[2]);
    rpcToolFinder.write(toolFinderCmd, toolFinderRpl);

    return 1;
}

/**********************************************************/
bool Manager::get3DPosition(imgPoint point, Vector &x)
{
    Bottle cmdMotor,replyMotor;
    cmdMotor.addVocab(Vocab::encode("get"));
    cmdMotor.addVocab(Vocab::encode("s2c"));
    Bottle &options=cmdMotor.addList();
    options.addString(camera.c_str());
    options.addInt(point.x);
    options.addInt(point.y);
    printf("Sending motor query: %s\n",cmdMotor.toString().c_str());
    rpcMotorAre.write(cmdMotor,replyMotor);
    printf("Received blob cartesian coordinates (for 2D coordinates %d %d): %s\n", point.x, point.y, replyMotor.toString().c_str());

    if (replyMotor.size()>=3) 
    {
        x.resize(3);
        x[0]=replyMotor.get(0).asDouble();
        x[1]=replyMotor.get(1).asDouble();
        x[2]=replyMotor.get(2).asDouble();
        return true;
    }
    else
        return false;
}

// Methods for tool selection, based on the learned Bayesian Net of affordances (with Matlab)
// used in the 2014/02 tests at IIT
//
//
///**********************************************************/
//void Manager::chooseTool(const int & effectLon, const int & effectLat, int & x, int & y)
//{
//    Bottle prior;
//    Bottle posterior;
//    Bottle query;
//    Bottle queryResponse;
//    int toolIndex;
//
//    if (effectLon >= 1 && effectLat <= 5) {
//        prior.addInt(14);
//        prior.addInt(effectLon);
//    }
//
//    if (effectLat >=1 && effectLat <= 5) {
//        prior.addInt(15);
//        prior.addInt(effectLat);
//    }
//
//    query.addList() = prior;
//
//    for (int n=1; n<=6; n++) {
//        posterior.addInt(n);
//    }
//
//    query.addList() = posterior;
//
//    bayesianNetOutputPort.write(query);
//
//    bayesianNetInputPort.read(queryResponse);
//
//    lookAtRack();
//
//    std::vector<BlobPartInfo> toolsTops;
//    std::vector<BlobPartInfo> toolsBottoms;
//    evaluateAllToolsDesc(toolsTops, toolsBottoms);
//
//    discretizeToolsTops(toolsTops);
//
//    findBestToolTop(toolIndex, toolsTops, queryResponse);
//
//    //returnbest
//    x=toolsBottoms[toolIndex].roi_x;
//    y=toolsBottoms[toolIndex].roi_y;
//}
//
///**********************************************************/
//void Manager::findBestToolTop(int & choosenTool, const std::vector<BlobPartInfo> & toolsTops, const Bottle & queryResponse)
//{
//    int nNodes = queryResponse.get(1).asList()->size();
//    int node;
//    int nodeValue;
//    int nAns;
//    double toolDistCurr = 5;
//    double toolDistBest = 5; // Cartesian distance between (1,1,1,1,1,1) and (3,3,3,3,3,3) is 2sqrt(6)≃4.89898. If other kind distance is used this value need changed.
//    double x = 0;
//
//    choosenTool = 0;
//
//    // TODO (Afonso#4#): Improve. Find a way to choose the best tool by taking into account the probability of success of each answer and the distance of the seen tools to the answers.
//
//    //nAns = queryResponse.get(0).asInt();  //for evaluating all the best answers received
//    nAns = 1;                             //limited to evaluating only the best answer
//    for (int ans = 2; ans <= nAns+1; ans++) {
//        for (int tool = 0; tool < toolsTops.size(); tool++) {
//            fprintf(stderr,"tool top (%d), located at x=%.0f y=%.0f:\n", tool, toolsTops[tool].roi_x , toolsTops[tool].roi_y);
//            toolDistCurr = 0;
//            x = 0;
//            for (int n = 0; n<nNodes; n++) {
//                x = 0;
//                node = queryResponse.get(1).asList()->get(n).asInt();
//                switch (node) {
//                    case 1:
//                        //area
//                        nodeValue = queryResponse.get(ans).asList()->get(n+1).asInt();
//                        x = (toolsTops[tool].area - nodeValue);
//                        fprintf(stderr,"(tool area         = %.0f) - (node %d = %d) = %.0f\n", toolsTops[tool].area, node, nodeValue, x);
//                        x = x * x;
//                        break;
//                    case 2:
//                        //convexity
//                        nodeValue = queryResponse.get(ans).asList()->get(n+1).asInt();
//                        x = (toolsTops[tool].convexity - nodeValue);
//                        fprintf(stderr,"(tool convexity    = %.0f) - (node %d = %d) = %.0f\n", toolsTops[tool].convexity, node, nodeValue, x);
//                        x = x * x;
//                        break;
//                    case 3:
//                        //eccentricity
//                        nodeValue = queryResponse.get(ans).asList()->get(n+1).asInt();
//                        x = (toolsTops[tool].eccentricity - nodeValue);
//                        fprintf(stderr,"(tool eccentricity = %.0f) - (node %d = %d) = %.0f\n", toolsTops[tool].eccentricity, node, nodeValue, x);
//                        x = x * x;
//                        break;
//                    case 4:
//                        //compactness
//                        nodeValue = queryResponse.get(ans).asList()->get(n+1).asInt();
//                        x = (toolsTops[tool].compactness - nodeValue);
//                        fprintf(stderr,"(tool compactness  = %.0f) - (node %d = %d) = %.0f\n", toolsTops[tool].compactness, node, nodeValue, x);
//                        x = x * x;
//                        break;
//                    case 5:
//                        //circleness
//                        nodeValue = queryResponse.get(ans).asList()->get(n+1).asInt();
//                        x = (toolsTops[tool].circleness - nodeValue);
//                        fprintf(stderr,"(tool circleness   = %.0f) - (node %d = %d) = %.0f\n", toolsTops[tool].circleness, node, nodeValue, x);
//                        x = x * x;
//                        break;
//                    case 6:
//                        //squareness
//                        nodeValue = queryResponse.get(ans).asList()->get(n+1).asInt();
//                        x = (toolsTops[tool].squareness - nodeValue);
//                        fprintf(stderr,"(tool squareness   = %.0f) - (node %d = %d) = %.0f\n", toolsTops[tool].squareness, node, nodeValue, x);
//                        x = x * x;
//                        break;
//                }
//                toolDistCurr = toolDistCurr + x;
//            }
//            toolDistCurr = sqrt(toolDistCurr);
//            fprintf(stderr,"total distance to ideal tool top = %f\n", toolDistCurr);
//            if (toolDistCurr<toolDistBest) {
//                choosenTool = tool;
//                toolDistBest = toolDistCurr;
//            }
//        }
//    }
//}
//
///**********************************************************/
//void Manager::evaluateAllToolsDesc(std::vector<BlobPartInfo> & toolsTops, std::vector<BlobPartInfo> & toolsBottoms)
//{
//    fprintf(stderr,"update every tool in sight top and bottom description...\n");
//
//    // Gets the descriptors of the entire tool blob:
//    Bottle *toolBottle = blobDescriptorInputPort.read(true);
//
//    // Gets the descriptors of the tool top blob:
//    Bottle *toolPartsBottle = toolsBlobDescriptorInputPort.read(true);
//
//    if (toolBottle->size()>1 && toolPartsBottle->size()>1) {
//        int nTools = toolPartsBottle->get(0).asInt();
//        BlobPartInfo toolTop, toolBottom;
//        for (int toolN = 1; toolN<=nTools; toolN++) {
//            updateToolPartDesc(toolTop, toolPartsBottle->get(toolN).asList()->get(0).asList());
//            toolsTops.push_back(toolTop);
//            updateToolPartDesc(toolBottom, toolPartsBottle->get(toolN).asList()->get(1).asList());
//            toolsBottoms.push_back(toolBottom);
//        }
//        fprintf(stderr,"tools' tops' and bottoms' descriptors evaluated\n");
//    }
//    else {
//        fprintf(stderr,"WARNING! - empty bottles...\n");
//    }
//
//    fprintf(stderr,"...done!\n");
//}
//
///**********************************************************/
//void Manager::updateToolPartDesc(BlobPartInfo & toolPart, const Bottle * toolPartBottle)
//{
//    toolPart.roi_x           = toolPartBottle->get(0).asDouble();
//    toolPart.roi_y           = toolPartBottle->get(1).asDouble();
//    toolPart.area            = toolPartBottle->get(2).asDouble();
//    toolPart.convexity       = toolPartBottle->get(3).asDouble();
//    toolPart.eccentricity    = toolPartBottle->get(4).asDouble();
//    toolPart.compactness     = toolPartBottle->get(5).asDouble();
//    toolPart.circleness      = toolPartBottle->get(6).asDouble();
//    toolPart.squareness      = toolPartBottle->get(7).asDouble();
//    toolPart.elongatedness   = toolPartBottle->get(8).asDouble();
//}
//
///**********************************************************/
//void Manager::discretizeToolsTops(std::vector<BlobPartInfo> & toolsTops)
//{
//    int discValue = 0;
//    for (int n=0; n<toolsTops.size(); n++) {
//        if (toolsTops[n].area <= 747)
//            discValue = 1;
//        else if (747 < toolsTops[n].area && toolsTops[n].area <= 953)
//            discValue = 2;
//        else if (953 < toolsTops[n].area)
//            discValue = 3;
//
//        toolsTops[n].area = discValue;
//
//        if (toolsTops[n].convexity <= 0.65)
//            discValue = 1;
//        else if (0.65 < toolsTops[n].convexity && toolsTops[n].convexity <= 0.79)
//            discValue = 2;
//        else if (0.79 < toolsTops[n].convexity)
//            discValue = 3;
//
//        toolsTops[n].convexity = discValue;
//
//        if (toolsTops[n].eccentricity <= 0.26)
//            discValue = 1;
//        else if (0.26 < toolsTops[n].eccentricity && toolsTops[n].eccentricity <= 0.46)
//            discValue = 2;
//        else if (0.46 < toolsTops[n].eccentricity)
//            discValue = 3;
//
//        toolsTops[n].eccentricity = discValue;
//
//        if (toolsTops[n].compactness <= 0.015)
//            discValue = 1;
//        else if (0.015 < toolsTops[n].compactness && toolsTops[n].compactness <= 0.025)
//            discValue = 2;
//        else if (0.025 < toolsTops[n].compactness)
//            discValue = 3;
//
//        toolsTops[n].compactness = discValue;
//
//        if (toolsTops[n].circleness <= 0.13)
//            discValue = 1;
//        else if (0.13 < toolsTops[n].circleness && toolsTops[n].circleness <= 0.25)
//            discValue = 2;
//        else if (0.25 < toolsTops[n].circleness)
//            discValue = 3;
//
//        toolsTops[n].circleness = discValue;
//
//        if (toolsTops[n].squareness <= 0.40)
//            discValue = 1;
//        else if (0.40 < toolsTops[n].squareness && toolsTops[n].squareness <= 0.75)
//            discValue = 2;
//        else if (0.75 < toolsTops[n].squareness)
//            discValue = 3;
//
//        toolsTops[n].squareness = discValue;
//    }
//}

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

#include "activityInterface.h"

#include <string>
#include <algorithm>

#include <iostream>
#include <fstream>

#define LEFT  0
#define RIGHT 1

using namespace std;
using namespace yarp::os;
using namespace yarp::sig;
using namespace iCub::iKin;
using namespace yarp::math;
using namespace yarp::dev;

ActivityInterface::ActivityInterface():arm_left("left"), arm_right("right")
{

}

/**********************************************************/
ActivityInterface::~ActivityInterface()
{
    
}

/**********************************************************/
bool ActivityInterface::configure(yarp::os::ResourceFinder &rf)
{
    with_robot = true;
    
    moduleName = rf.check("name", Value("activityInterface"), "module name (string)").asString();
    Bottle bArm[2];
    
    bArm[LEFT]=rf.findGroup("left_arm");
    bArm[RIGHT]=rf.findGroup("right_arm");
    
    if (rf.check("no_robot"))
        with_robot = false;
    
    if (Bottle *pB=bArm[LEFT].find("reach_above_orientation").asList())
    {
        reachAboveOrient[LEFT].resize(pB->size());
        
        for (int i=0; i<pB->size(); i++)
            reachAboveOrient[LEFT][i]=pB->get(i).asDouble();
    }
    
    if (Bottle *pB=bArm[RIGHT].find("reach_above_orientation").asList())
    {
        reachAboveOrient[RIGHT].resize(pB->size());
        
        for (int i=0; i<pB->size(); i++)
            reachAboveOrient[RIGHT][i]=pB->get(i).asDouble();
    }
    
    fprintf(stdout, "reachAboveOrient[LEFT] %s\n", reachAboveOrient[LEFT].toString().c_str());
    fprintf(stdout, "reachAboveOrient[RIGHT] %s\n", reachAboveOrient[RIGHT].toString().c_str());
    
    setName(moduleName.c_str());
    
    handlerPortName =  "/";
    handlerPortName += getName();
    handlerPortName +=  "/rpc:i";
    closing=false;
    
    if (!rpcPort.open(handlerPortName.c_str()))
    {
        fprintf(stdout, "%s : Unable to open port %s\n", getName().c_str(), handlerPortName.c_str());
        return false;
    }
    
    memoryReporter.setManager(this);
    rpcMemory.setReporter(memoryReporter);
    rpcMemory.open(("/"+moduleName+"/memory:rpc").c_str());
    
    rpcARE.setReporter(memoryReporter);
    rpcARE.open(("/"+moduleName+"/are:rpc").c_str());
    
    rpcAREcmd.setReporter(memoryReporter);
    rpcAREcmd.open(("/"+moduleName+"/arecmd:rpc").c_str());
    
    robotStatus.open(("/"+moduleName+"/status:o").c_str());
    
    yarp::os::Network::connect("/activityInterface/arecmd:rpc", "/actionsRenderingEngine/cmd:io");
    yarp::os::Network::connect("/activityInterface/are:rpc", "/actionsRenderingEngine/get:io");
    yarp::os::Network::connect("/activityInterface/memory:rpc", "/memory/rpc");
    
    if (with_robot)
    {
        Property optionLeft("(device cartesiancontrollerclient)");
        optionLeft.put("remote","/icub/cartesianController/left_arm");
        optionLeft.put("local","/cartesian_client/left_arm");
        
        Property optionRight("(device cartesiancontrollerclient)");
        optionRight.put("remote","/icub/cartesianController/right_arm");
        optionRight.put("local","/cartesian_client/right_arm");
        
        if (!client_left.open(optionLeft))
            return false;
        
        if (!client_right.open(optionRight))
        {
            client_left.close();
            return false;
        }
        
        // open the view
        client_left.view(icart_left);
        client_right.view(icart_right);
        
        Property optArm;
        
        optArm.put("device", "remote_controlboard");
        optArm.put("remote","/icub/left_arm");
        optArm.put("local",("/localArm/left"));
        
        Property optTorso;
        optTorso.put("device", "remote_controlboard");
        optTorso.put("remote","/icub/torso");
        optTorso.put("local",("/grasplocalTorso/left"));
        
        robotArm.open(optArm);
        robotTorso.open(optTorso);
        
        if (!robotTorso.isValid() || !robotArm.isValid())
        {
            fprintf(stdout, "Device not available\n");
            return false;
        }
        
        robotArm.view(limArm);
        robotTorso.view(limTorso);
        
        chain_left=arm_left.asChain();
        chain_right=arm_right.asChain();
        
        chain_left->releaseLink(0);
        chain_left->releaseLink(1);
        chain_left->releaseLink(2);
        
        chain_right->releaseLink(0);
        chain_right->releaseLink(1);
        chain_right->releaseLink(2);
        
        deque<IControlLimits*> lim_left;
        lim_left.push_back(limTorso);
        lim_left.push_back(limArm);
        arm_left.alignJointsBounds(lim_left);
        
        arm_left.setAllConstraints(false);
        arm_right.setAllConstraints(false);
        
        thetaMin.resize(10,0.0);
        thetaMax.resize(10,0.0);
        
        for (unsigned int i=0; i<chain_left->getDOF(); i++)
        {
            thetaMin[i]=(*chain_left)(i).getMin();
            thetaMax[i]=(*chain_left)(i).getMax();
        }
    }
    attach(rpcPort);
    
    first = true;
    elements = 0;
    
    return true ;
}

/**********************************************************/
bool ActivityInterface::interruptModule()
{
    rpcMemory.interrupt();
    rpcARE.interrupt();
    rpcAREcmd.interrupt();
    rpcPort.interrupt();
    robotStatus.interrupt();
    return true;
}

/**********************************************************/
bool ActivityInterface::close()
{
    fprintf(stdout, "starting the shutdown procedure\n");
    rpcPort.close();
    client_left.close();
    client_right.close();
    rpcMemory.close();
    rpcARE.close();
    rpcAREcmd.close();
    robotStatus.close();
    return true;
}

/**********************************************************/
bool ActivityInterface::propagateStatus()
{
    Bottle are, replyAre;
    are.clear(),replyAre.clear();
    are.addString("get");
    are.addString("idle");
    rpcAREcmd.write(are,replyAre);
    
    Bottle status;
    status.clear();
    
    if (strcmp (replyAre.toString().c_str(),"[nack]") == 0)
        status.addString("busy");
    else if (strcmp (replyAre.toString().c_str(),"[ack]") == 0)
        status.addString("idle");
    else
        status.addString("error");

    robotStatus.write(replyAre);
    
    return true;
}

/**********************************************************/
bool ActivityInterface::updateModule()
{
    propagateStatus();
    return !closing;
}

/**********************************************************/
double ActivityInterface::getPeriod()
{
    return 0.5;
}

/**********************************************************/
Bottle ActivityInterface::getMemoryBottle()
{
    Bottle memoryReply;
    Bottle cmdMemory,replyMemory,replyMemoryProp;
    cmdMemory.addVocab(Vocab::encode("ask"));
    Bottle &cont=cmdMemory.addList().addList();
    cont.addString("entity");
    cont.addString("==");
    cont.addString("object");
    rpcMemory.write(cmdMemory,replyMemory);
    
    if (replyMemory.get(0).asVocab()==Vocab::encode("ack"))
    {
        if (Bottle *idField=replyMemory.get(1).asList())
        {
            if (Bottle *idValues=idField->get(1).asList())
            {
                //cycle over items
                for (int i=0; i<idValues->size(); i++)
                {
                    int id=idValues->get(i).asInt();
                    
                    Bottle cmdTime;Bottle cmdReply;
                    cmdTime.addVocab(Vocab::encode("time"));
                    Bottle &contmp=cmdTime.addList();
                    Bottle &list_tmp=contmp.addList();
                    list_tmp.addString("id");
                    list_tmp.addInt(id);
                    rpcMemory.write(cmdTime,cmdReply);
                    
                    Bottle *timePassed = cmdReply.get(1).asList();
                    double time = timePassed->get(0).asDouble();
                    
                    if (time < 0.5)
                    {
                        cmdMemory.clear();
                        cmdMemory.addVocab(Vocab::encode("get"));
                        Bottle &content=cmdMemory.addList();
                        Bottle &list_bid=content.addList();
                        list_bid.addString("id");
                        list_bid.addInt(id);
                        rpcMemory.write(cmdMemory,replyMemoryProp);
                        
                        memoryReply.addList() = *replyMemoryProp.get(1).asList();
                    }
                }
            }
        }
    }
    
    return memoryReply;
}

/**********************************************************/
Bottle ActivityInterface::get3D(const string &objName)
{
    Bottle Memory = getMemoryBottle();
    Bottle position3D;
    
    for (int i=0; i<Memory.size(); i++)
    {
        if (Bottle *propField = Memory.get(i).asList())
        {
            if (propField->check("name"))
            {
                if (strcmp (propField->find("name").asString().c_str(), objName.c_str() ) == 0)
                {
                    if (propField->check("position_3d"))
                    {
                        Bottle *propFieldPos = propField->find("position_3d").asList();
                        
                        for (int i=0; i < propFieldPos->size(); i++)
                        {
                            position3D.addDouble(propFieldPos->get(i).asDouble());
                        }
                    }
                }
            }
        }
    }
    return position3D;
}

/**********************************************************/
bool ActivityInterface::attach(RpcServer &source)
{
    return this->yarp().attachAsServer(source);
}

/**********************************************************/
bool ActivityInterface::quit()
{
    closing = true;
    return true;
}

/**********************************************************/
bool ActivityInterface::handStat(const string &handName)
{
    //FILL IN HAND AVAILABILITY
    Bottle are, replyAre;
    are.clear(),replyAre.clear();
    are.addString("get");
    are.addString("holding");
    are.addString(handName.c_str());
    rpcARE.write(are,replyAre);
    
    if (strcmp (replyAre.toString().c_str(),"[nack]") == 0)
        return false;
    else if (strcmp (replyAre.toString().c_str(),"[ack]") == 0)
        return true;
    else
        return false;
}

/**********************************************************/
Bottle ActivityInterface::getBlobCOG(const Bottle &blobs, const int i)
{
    Bottle cog;
    
    if ((i>=0) && (i<blobs.size()))
    {
        Bottle tl,br;
        Bottle *item=blobs.get(i).asList();
        if (item==NULL)
            return cog;
        
        double X = ((int)item->get(0).asDouble()+(int)item->get(2).asDouble())>>1;
        double Y = ((int)item->get(1).asDouble()+(int)item->get(3).asDouble())>>1;
        
        cog.addDouble(X);
        cog.addDouble(Y);
    }
    return cog;
}


/**********************************************************/
string ActivityInterface::getLabel(const int32_t pos_x, const int32_t pos_y)
{
    Bottle Memory = getMemoryBottle();
    Bottle positionBBox;
    string label;
    
    for (int i=0; i<Memory.size(); i++)
    {
        if (Bottle *propField=Memory.get(i).asList())
        {
            if (propField->check("position_2d_left"))
            {
                Bottle *propFieldPos = propField->find("position_2d_left").asList();
                
                positionBBox.clear();
                Bottle &list=positionBBox.addList();
                        
                for (int i=0; i < propFieldPos->size(); i++)
                {
                    list.addDouble(propFieldPos->get(i).asDouble());
                }
                
                Bottle cog = getBlobCOG(positionBBox, 0);
                
                int diffx = abs(cog.get(0).asInt() - pos_x);
                int diffy = abs(cog.get(1).asInt() - pos_y);
                
                if ( diffx < 10 && diffy < 10)
                {
                    if (propField->check("name"))
                    {
                        label = propField->find("name").asString().c_str();
                    }
                }
            }
        }
    }
    return label;
}

/**********************************************************/
double ActivityInterface::getManip(const string &objName, const std::string &handName)
{
    Bottle getObjectPos = get3D(objName);
    Vector o_left, o_right, x;
    
    o_left  = reachAboveOrient[LEFT];
    o_right = reachAboveOrient[RIGHT];
    double manip = 0.0;
    
    if (with_robot)
    {
        if ( getObjectPos.size() >2 )
        {
            x.resize(getObjectPos.size());
            x[0] = getObjectPos.get(0).asDouble();
            x[1] = getObjectPos.get(1).asDouble();
            x[2] = getObjectPos.get(2).asDouble();
            
            fprintf(stdout," \n\n\nPosition is: %s \n", x.toString().c_str());
            fprintf(stdout,"Left orientation is: %s \n", o_left.toString().c_str());
            fprintf(stdout,"Right orientation is: %s \n \n \n", o_right.toString().c_str());
            
            Vector xhat_left, xhat_right, ohat_left, ohat_right;
            Vector q_left, q_right;
            
            int ctxt_left_tmp;
            int ctxt_right_tmp;
            
            icart_left->storeContext(&ctxt_left_tmp);
            icart_right->storeContext(&ctxt_right_tmp);
            
            Vector dof(10, 1.0);
            dof[1]=0.0;
                
            icart_left->setDOF(dof, dof);
            icart_right->setDOF(dof, dof);
            
            icart_left->askForPose(x, o_left, xhat_left, ohat_left, q_left);
            icart_right->askForPose(x, o_right, xhat_right, ohat_right, q_right);
            
            icart_left->restoreContext(ctxt_left_tmp);
            icart_right->restoreContext(ctxt_right_tmp);
            
            icart_left->deleteContext(ctxt_left_tmp);
            icart_right->deleteContext(ctxt_right_tmp);
            
            fprintf(stdout,"q_left is: %s \n", q_left.toString().c_str());
            fprintf(stdout,"q_right is: %s \n", q_right.toString().c_str());
            
            q_left  *= M_PI / 180.0;
            q_right *= M_PI / 180.0;
            
            Matrix J_left  = arm_left.GeoJacobian(q_left);
            Matrix J_right = arm_right.GeoJacobian(q_right);

            double manip_left = sqrt(det(J_left * J_left.transposed()));
            double manip_right = sqrt(det(J_right * J_right.transposed()));
            
            double limits_left=0.0;
            for (unsigned int k=0; k<thetaMin.size(); k++)
                limits_left+=(q_left[k]-thetaMin[k])*(thetaMax[k]-q_left[k])/((thetaMax[k]-thetaMin[k])*(thetaMax[k]-thetaMin[k]));
            
            manip_left*=(1-exp(-limits_left));
            
            double limits_right=0.0;
            for (unsigned int k=0; k<thetaMin.size(); k++)
                limits_right+=(q_right[k]-thetaMin[k])*(thetaMax[k]-q_right[k])/((thetaMax[k]-thetaMin[k])*(thetaMax[k]-thetaMin[k]));
            
            manip_right*=(1-exp(-limits_right));
            
            
            if (strcmp (handName.c_str(),"left") == 0)
                manip = manip_left;
            else if (strcmp (handName.c_str(),"right") == 0)
                manip = manip_right;
            else
                manip = 0.0;
            
        }
        
        //manip = max(manip_left, manip_right);
    }
    else
    {
        if (strcmp (handName.c_str(),"left") == 0)
            manip = 0.6;
        else if (strcmp (handName.c_str(),"right") == 0)
            manip = 0.4;
        else
            manip = 0.0;
    }
    
    return manip;
}

/**********************************************************/
string ActivityInterface::inHand(const string &objName)
{
    string handName;
    
    //handName = inHandStatus.find(objName.c_str())->second;
    
    for (std::map<string, string>::iterator it=inHandStatus.begin(); it!=inHandStatus.end(); ++it)
    {
        if (strcmp (it->first.c_str(), objName.c_str() ) == 0)
            handName = it->second.c_str();
        
    }
    if (handName.empty())
        handName = "none";
        
    return handName;
}

/**********************************************************/
bool ActivityInterface::take(const string &objName, const string &handName)
{
    //check for hand status beforehand to make sure that it is empty
    
    //handStatus(handName);
    
    //talk to iolStateMachineHandler
    //figure out if object is visible
    
    //if object is visible do the following
    
    for (std::map<int, string>::iterator it=onTopElements.begin(); it!=onTopElements.end(); ++it)
    {
        if (strcmp (it->second.c_str(), objName.c_str() ) == 0)
        {
            int id = it->first;
            onTopElements.erase(id);
            elements--;
        }
    }
    
    //do the take actions
    
    //update inHandStatus map
    inHandStatus.insert(pair<string, string>(objName.c_str(), handName.c_str()));
    return true;
}

/**********************************************************/
bool ActivityInterface::drop(const string &objName, const string &targetName)
{
    //talk to iolStateMachineHandler
    //should drop objName on top of targetName
    
    if (!targetName.empty())
    {
        if (elements == 0)
        {
            onTopElements.insert(pair<int, string>(elements, targetName.c_str()));
            elements++;
        }

        onTopElements.insert(pair<int, string>(elements, objName.c_str()));
        elements++;
    }
    
    for (std::map<string, string>::iterator it=inHandStatus.begin(); it!=inHandStatus.end(); ++it)
    {
        if (strcmp (it->first.c_str(), objName.c_str() ) == 0)
        {
            inHandStatus.erase(objName.c_str());
            break;
        }
    }
    
    return true;
}


/**********************************************************/
Bottle ActivityInterface::underOf(const std::string &objName)
{
    Bottle replyList;
    
    replyList.clear();
    Bottle &list=replyList.addList();
    
    int id = -1;
    
    for (std::map<int, string>::reverse_iterator rit=onTopElements.rbegin(); rit!=onTopElements.rend(); ++rit)
        if (strcmp (objName.c_str(), rit->second.c_str() ) == 0)
            id = rit->first;
    
    
    for (std::map<int, string>::reverse_iterator rit=onTopElements.rbegin(); rit!=onTopElements.rend(); ++rit)
        if (strcmp (objName.c_str(), rit->second.c_str() ) != 0 && id >= 0 && rit->first <= id)
            list.addString(rit->second.c_str());
    
    return replyList;
}




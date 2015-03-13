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
    
    rpcWorldState.setReporter(memoryReporter);
    rpcWorldState.open(("/"+moduleName+"/worldState:rpc").c_str());
    
    robotStatus.open(("/"+moduleName+"/status:o").c_str());
    
    rpcIolState.setReporter(memoryReporter);
    rpcIolState.open(("/"+moduleName+"/iolState:rpc").c_str());
    
    inputBlobPortName = "/" + moduleName + "/blobs:i";
    blobPortIn.open( inputBlobPortName.c_str() );
    
    rpcPraxiconInterface.setReporter(memoryReporter);
    rpcPraxiconInterface.open(("/"+moduleName+"/praxicon:rpc").c_str());
    
    pradaStatus.setManager(this);
    pradaStatus.open(("/"+moduleName+"/prada:i").c_str());
    
    praxiconToPradaPort.open(("/"+moduleName+"/praxicon:o").c_str());
    
    
    yarp::os::Network::connect(("/"+moduleName+"/arecmd:rpc").c_str(), "/actionsRenderingEngine/cmd:io");
    yarp::os::Network::connect(("/"+moduleName+"/are:rpc").c_str(), "/actionsRenderingEngine/get:io");
    yarp::os::Network::connect(("/"+moduleName+"/memory:rpc").c_str(), "/memory/rpc");
    yarp::os::Network::connect(("/"+moduleName+"/iolState:rpc").c_str(), "/iolStateMachineHandler/human:rpc");

    yarp::os::Network::connect("/blobSpotter/image:o", inputBlobPortName.c_str());
    yarp::os::Network::connect(("/"+moduleName+"/praxicon:rpc").c_str(), "/praxInterface/speech:i");
    
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
    
    for (int i=0; i<10; i++)
        incrementSize[i] = 0;
    
    Bottle cmd, reply;
    cmd.clear(), reply.clear();
    cmd.addString("attention");
    cmd.addString("stop");
    rpcIolState.write(cmd, reply);
    
    cmd.clear(), reply.clear();
    cmd.addString("home");
    cmd.addString("head");
    rpcAREcmd.write(cmd, reply);
    
    fprintf(stdout, "done initialization\n");
    
    return true ;
}

/**********************************************************/
bool ActivityInterface::interruptModule()
{
    semaphore.wait();
    rpcMemory.interrupt();
    rpcARE.interrupt();
    rpcAREcmd.interrupt();
    rpcPort.interrupt();
    rpcWorldState.interrupt();
    robotStatus.interrupt();
    rpcIolState.interrupt();
    blobPortIn.interrupt();
    rpcPraxiconInterface.interrupt();
    pradaStatus.interrupt();
    praxiconToPradaPort.interrupt();
    semaphore.post();
    return true;
}

/**********************************************************/
bool ActivityInterface::close()
{
    semaphore.wait();
    fprintf(stdout, "starting the shutdown procedure\n");
    rpcPort.close();
    client_left.close();
    client_right.close();
    rpcMemory.close();
    rpcARE.close();
    rpcAREcmd.close();
    robotStatus.close();
    rpcWorldState.close();
    rpcIolState.close();
    blobPortIn.close();
    rpcPraxiconInterface.close();
    pradaStatus.close();
    praxiconToPradaPort.close();
    fprintf(stdout, "finished shutdown procedure\n");
    semaphore.post();
    return true;
}

/**********************************************************/
bool ActivityInterface::goHome()
{
    Bottle are, replyAre;
    are.clear(),replyAre.clear();
    are.addString("home");
    are.addString("head");
    rpcAREcmd.write(are,replyAre);
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
bool ActivityInterface::executeSpeech (const string &speech)
{
    Bottle cmdIol;
    Bottle replyIol;
    cmdIol.clear(), replyIol.clear();
    cmdIol.addString("say");
    cmdIol.addString(speech.c_str());
    rpcIolState.write(cmdIol,replyIol);
    fprintf(stdout,"%s\n", replyIol.toString().c_str());
    return true;
}

/**********************************************************/
Bottle ActivityInterface::askPraxicon(const string &request)
{
    praxiconRequest = request;
    Bottle listOfGoals, cmdPrax, replyPrax;
    
    Bottle toolLikeMemory = getToolLikeNames();
    Bottle objectsMemory = getNames();
    
    
    fprintf(stdout, "tool names: %s \n", toolLikeMemory.toString().c_str());
    fprintf(stdout, "object names: %s \n", objectsMemory.toString().c_str());
    
    Bottle &listOfObjects = cmdPrax.addList();
    
    //create available list
    listOfObjects.addString("available");
    
    int passed[toolLikeMemory.size()];
    
    for (int i=0; i<objectsMemory.size(); i++)
    {
        for (int x=0; x<toolLikeMemory.size(); x++)
            passed[x] = 0;
        
        for (int ii=0; ii<toolLikeMemory.size(); ii++)
            if (strcmp (objectsMemory.get(i).asString().c_str(), toolLikeMemory.get(ii).asString().c_str() ) != 0)
                passed[ii] = 1;
        
        int total=0;
        for (int x=0; x<toolLikeMemory.size(); x++)
            total+=passed[x];
        
        //fprintf(stdout,"toolLikeMemory size: %d and total %d\n", toolLikeMemory.size(), total);
        
        if (total==toolLikeMemory.size())
            listOfObjects.addString(objectsMemory.get(i).asString().c_str());
    }
    
    Bottle &query=cmdPrax.addList();
    //create query list
    query.addString("query");
    query.addString(request.c_str());
    
    Bottle &missing=cmdPrax.addList();
    //create query list
    missing.addString("missing");
    //missing.addString("stirrer");
    //missing.addString("plate");
    
    fprintf(stdout, "sending: \n %s \n", cmdPrax.toString().c_str());
    //send it all to praxicon
    rpcPraxiconInterface.write(cmdPrax,replyPrax);
    
    
    Bottle &tmpList = listOfGoals.addList();
    //listOfGoals.clear();
    vector<string> tokens;
    
    if (replyPrax.size() > 0)
    {
    
        for (int i=0; i<replyPrax.size()-1; i++) //-1 to remove mouth speak
        {
            string replytmp = replyPrax.get(i).asString().c_str();
            istringstream iss(replytmp);
            
            copy(istream_iterator<string>(iss),
                 istream_iterator<string>(),
                  back_inserter(tokens));
        }
        
        //adding this to prevent missing the last goal or going out of range
        tokens.push_back("endofstring");
        
        int inc = 0;
        Bottle tmp;
        for (int i=0; i<tokens.size(); i++)
        {
            if ( ++inc == 4 )
            {
                tmpList.addList() = tmp;
                inc=1;
                tmp.clear();
            }
            tmp.addString(tokens[i].c_str());
        }
        //fprintf(stdout, "Final praxicon is: %s \n",replyPrax.toString().c_str());
    }
    else
    {
        executeSpeech("something went terribly wrong. I cannot " + request);
    }
    
    praxiconToPradaPort.write(listOfGoals);
    
    return listOfGoals;
}

/**********************************************************/
bool ActivityInterface::processPradaStatus(const Bottle &status)
{
    Bottle objectsUsed;
    if ( status.size() > 0 )
    {
        fprintf(stdout, "the status is %s \n", status.toString().c_str());
        if (strcmp (status.get(0).asString().c_str(), "ok" ) == 0)
        {
            for (int i=1; i< status.size(); i++)
            {
                if (strcmp (status.get(i).asString().c_str(), "bun_bottom" ) != 0 && strcmp (status.get(i).asString().c_str(), "bun_top" ) != 0)
                    objectsUsed.addString(status.get(i).asString().c_str());

            }
            executeSpeech("I made a " + objectsUsed.toString() + " sandwich");
        }
        else if (strcmp (status.get(0).asString().c_str(), "fail" ) == 0)
        {
            Bottle objectsMissing;
            for (int i=1; i< status.size(); i++)
            {
                objectsMissing.addString(status.get(i).asString());
            }
            
            string toSay = "I seem to be missing the " ;
            for (int i=0; i<objectsMissing.size(); i++)
            {
                toSay += objectsMissing.get(i).asString();
                if (i < objectsMissing.size()-1 )
                    toSay+=" and";
            }
            executeSpeech(toSay);
            executeSpeech("I need to ask the praxicon for help!" );
            
            askPraxicon(praxiconRequest);
        }
        else
        {
            fprintf(stdout, "Something is wrong with the status\n");
        }
    }
    return true;
}

/**********************************************************/
string ActivityInterface::getMemoryNameBottle(int id)
{
    string name;
    name.clear();
    
    Bottle replyMemoryName, cmdMemory;
    replyMemoryName.clear();
    cmdMemory.clear();
    cmdMemory.clear();
    cmdMemory.addVocab(Vocab::encode("get"));
    Bottle &content=cmdMemory.addList();
    Bottle &list_bid=content.addList();
    list_bid.addString("id");
    list_bid.addInt(id);
    
    Bottle &list_propSet=content.addList();
    list_propSet.addString("propSet");
    Bottle &list_name=list_propSet.addList();
    list_name.addString("name");
    
    rpcMemory.write(cmdMemory,replyMemoryName);
    
    if (replyMemoryName.get(0).asVocab()==Vocab::encode("ack"))
    {
        if (Bottle *nameField=replyMemoryName.get(1).asList())
        {
            if (Bottle *nameValues=nameField->get(0).asList())
                name = nameValues->get(1).asString().c_str();
        }
    }
    return name;
}

/**********************************************************/
Bottle ActivityInterface::getIDs()
{
    Bottle ids;
    ids.clear();
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
                    ids.addInt(id);
                }
            }
        }
    }
    return ids;
}

/**********************************************************/
bool ActivityInterface::handleTrackers()
{
    Bottle ids = getIDs();

    for (int i=0; i<ids.size(); i++ )
    {
        int id = ids.get(i).asInt();
        bool paused = false;
        
        for (int y = 0; y < pausedThreads.size(); y++  )
            if (id == pausedThreads[y])
                paused = true;
        
        string name = getMemoryNameBottle(id);
        
        Bottle position = get3D(name);
    
        if (position.size() < 1 )
        {
            if (incrementSize[i] < 5)
                incrementSize[i]++;
            
            if (incrementSize[i] > 4 && !paused)
            {
                Bottle cmdPauseThread, replyPauseThread;
                cmdPauseThread.clear();
                cmdPauseThread.addString("pause");
                cmdPauseThread.addString(name.c_str());
                
                fprintf(stdout, "will send: %s \n", cmdPauseThread.toString().c_str() );
                
                rpcWorldState.write(cmdPauseThread,replyPauseThread);
                pausedThreads.push_back(id);
            }
        }
        if (position.size() > 1 )
        {
            if (incrementSize[i] > 0 && incrementSize[i] !=-1)
                incrementSize[i]--;
            
            if (incrementSize[i] == 0 && paused)
            {
                Bottle cmdPauseThread, replyPauseThread;
                cmdPauseThread.clear();
                cmdPauseThread.addString("resume");
                cmdPauseThread.addString(name.c_str());
                
                fprintf(stdout, "will send: %s \n", cmdPauseThread.toString().c_str() );
                
                rpcWorldState.write(cmdPauseThread,replyPauseThread);
                pausedThreads.erase(std::remove(pausedThreads.begin(), pausedThreads.end(), id), pausedThreads.end());
            }
        }
    }
    return true;
}

/**********************************************************/
bool ActivityInterface::updateModule()
{
    semaphore.wait();
    
    propagateStatus();
    handleTrackers();
    
    semaphore.post();
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
Bottle ActivityInterface::getOffset(const string &objName)
{
    Bottle toolOffset;
    toolOffset.clear();
    
    if (strcmp ("rake", objName.c_str() ) == 0)
    {
        toolOffset.addDouble(0.18);
        toolOffset.addDouble(-0.18);
        toolOffset.addDouble(0.04); //left hand should be negative
    }
    else if (strcmp ("stick", objName.c_str() ) == 0)
    {
        toolOffset.addDouble(0.18);
        toolOffset.addDouble(-0.18);
        toolOffset.addDouble(0.04);
    }
    else
    {
        toolOffset.addDouble(0.0);
        toolOffset.addDouble(0.0);
        toolOffset.addDouble(0.0);
    }
    
    return toolOffset;
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
    string handStatus = inHand(objName);
    if (strcmp (handStatus.c_str(), "none" ) == 0 )
    {
        //talk to iolStateMachineHandler
        Bottle position = get3D(objName);
    
        if (position.size()>0)
        {
            fprintf(stdout, "object is visible at %s will do the take action \n", position.toString().c_str());
            
            executeSpeech("ok, I will take the " + objName);
            //do the take actions
            Bottle cmd, reply;
            cmd.clear(), reply.clear();
            cmd.addString("take");
            cmd.addString(objName.c_str());
            cmd.addString(handName.c_str());
            rpcAREcmd.write(cmd, reply);
            
            if (reply.get(0).asVocab()==Vocab::encode("ack"))
            {
                for (std::map<int, string>::iterator it=onTopElements.begin(); it!=onTopElements.end(); ++it)
                {
                    if (strcmp (it->second.c_str(), objName.c_str() ) == 0)
                    {
                        int id = it->first;
                        onTopElements.erase(id);
                        elements--;
                    }
                }
                
                //update inHandStatus map
                inHandStatus.insert(pair<string, string>(objName.c_str(), handName.c_str()));
            }
            else
            {
                executeSpeech("I have failed to take the" + objName);
            }
        }
        else
        {
            executeSpeech("I am sorry I cannot see the" + objName);
        }
    }
    else
    {
        executeSpeech("I already have the " + objName + " in my hand");
        fprintf(stdout, "Cannot grasp already have something in hand\n");
    }

    return true;
}

/**********************************************************/
bool ActivityInterface::drop(const string &objName)
{
    string handName = inHand(objName);
    if (strcmp (handName.c_str(), "none" ) != 0 )
    {
        executeSpeech("ok, I will drop the " + objName );
        //do the take actions
        Bottle cmd, reply;
        cmd.clear(), reply.clear();
        cmd.addString("drop");
        cmd.addString(handName.c_str());
        rpcAREcmd.write(cmd, reply);
    }
    else
    {
        fprintf(stdout, "I am not holding anything\n");
    }
    return true;
}

/**********************************************************/
bool ActivityInterface::put(const string &objName, const string &targetName)
{
    string handName = inHand(objName);
    if (strcmp (handName.c_str(), "none" ) != 0 )
    {
        //talk to iolStateMachineHandler
        Bottle position = get3D(targetName);
        
        if (position.size()>0)
        {
            executeSpeech("ok, I will place the " + objName + " on the " + targetName);
            
            //do the take actions
            Bottle cmd, reply;
            cmd.clear(), reply.clear();
            cmd.addString("drop");
            cmd.addString("over");
            cmd.addString(targetName.c_str());
            cmd.addString("gently");
            cmd.addString(handName.c_str());
            rpcAREcmd.write(cmd, reply);
            
            if (reply.get(0).asVocab()==Vocab::encode("ack"))
            {
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
            }
        }
    }
    
    return true;
}

/**********************************************************/
bool ActivityInterface::askForTool(const std::string &handName, const int32_t pos_x, const int32_t pos_y)
{

    // Get the label of the object requested
    int whichArm = 0;
    string label = getLabel(pos_x, pos_y);
    
    Bottle cmdHome, cmdReply;
    cmdHome.clear();
    cmdReply.clear();
    cmdHome.addString("home");
    cmdHome.addString("head");
    rpcAREcmd.write(cmdHome,cmdReply);
    
    executeSpeech ("can you give me the " + label + "please?");
    
    fprintf(stdout, "tool label is: %s \n",label.c_str());
    
    Bottle cmdAre, replyAre;
    cmdAre.clear();
    replyAre.clear();
    cmdAre.addString("point");
    Bottle &tmp=cmdAre.addList();
    tmp.addInt (pos_x);
    tmp.addInt (pos_y);
    
    if (pos_x > 0 && pos_y <= 160 )
    {
        whichArm = LEFT;
        cmdAre.addString("left");
        rpcAREcmd.write(cmdAre,replyAre);
    }
    else if (pos_x > 160 && pos_y < 320 )
    {
        whichArm = RIGHT;
        cmdAre.addString("right");
        rpcAREcmd.write(cmdAre, replyAre);
    }
    else
        executeSpeech ("oh my...I seemed to have gotten confused...sorry");
    
    cmdAre.clear();
    replyAre.clear();
    cmdAre.addString("tato");
    cmdAre.addString(handName.c_str());
    rpcAREcmd.write(cmdAre, replyAre);
    
    printf("done requesting\n");
    
    cmdAre.clear();
    replyAre.clear();
    cmdAre.addString("close");
    cmdAre.addString(handName.c_str());
    rpcAREcmd.write(cmdAre, replyAre);
    Time::delay(5.0);
    printf("done closing\n");
    
    cmdAre.clear();
    replyAre.clear();
    cmdAre.addString("home");
    cmdAre.addString("arms");
    cmdAre.addString("head");
    rpcAREcmd.write(cmdAre, replyAre);
    
    return true;
}

/**********************************************************/
bool ActivityInterface::pull(const string &objName, const string &toolName)
{
    //ask for tool
    //do the pulling action
    return true;
}

/**********************************************************/
bool ActivityInterface::push(const string &objName, const string &toolName)
{
    //ask for tool
    //do the pushing action
    return true;
}

/**********************************************************/
Bottle ActivityInterface::underOf(const std::string &objName)
{
    Bottle replyList;
    
    replyList.clear();
    //Bottle &list=replyList.addList();
    
    int id = -1;
    
    for (std::map<int, string>::reverse_iterator rit=onTopElements.rbegin(); rit!=onTopElements.rend(); ++rit)
        if (strcmp (objName.c_str(), rit->second.c_str() ) == 0)
            id = rit->first;
    
    
    for (std::map<int, string>::reverse_iterator rit=onTopElements.rbegin(); rit!=onTopElements.rend(); ++rit)
        if (strcmp (objName.c_str(), rit->second.c_str() ) != 0 && id >= 0 && rit->first <= id)
            replyList.addString(rit->second.c_str());
    
    return replyList;
}

/**********************************************************/
Bottle ActivityInterface::getNames()
{
    Bottle Memory = getMemoryBottle();
    Bottle names;
    
    for (int i=0; i<Memory.size(); i++)
    {
        if (Bottle *propField = Memory.get(i).asList())
        {
            if (propField->check("name"))
            {
                names.addString(propField->find("name").asString());
            }
        }
    }
    return names;
}

/**********************************************************/
Bottle ActivityInterface::reachableWith(const string &objName)
{
    Bottle replyList, position, names;
    position.clear();
    names.clear();
    replyList.clear();
    
    position = get3D(objName);

    if (position.get(0).asDouble() < -0.47)
    {
        Bottle list = pullableWith(objName);
        for (int i = 0; i<list.size(); i++)
            replyList.addString(list.get(i).asString());
    }
    else
    {
        Bottle list = getNames();
        for (int i = 0; i<list.size(); i++)
        {
            if (strcmp (objName.c_str(), list.get(i).asString().c_str() ) != 0)
                replyList.addString(list.get(i).asString());
        }
        
        //double leftManip = getManip(objName, "left");
        //double rightManip = getManip(objName, "right");
        //fprintf(stdout, "\nleftManip: %lf and rightManip: %lf\n", leftManip, rightManip);
        
        //using 3D instead of manip for testing
        if(position.get(1).asDouble() < - 0.1 )
            replyList.addString("left");
        else if (position.get(1).asDouble() >  0.1)
            replyList.addString("right");
        else
        {
            replyList.addString("left");
            replyList.addString("right");
        }
    }
    
    return replyList;
}
/**********************************************************/
Bottle ActivityInterface::pullableWith(const string &objName)
{
    Bottle list = getToolLikeNames();
    Bottle replyList;
    replyList.clear();
    for (int i = 0; i<list.size(); i++)
    {
        if (strcmp (objName.c_str(), list.get(i).asString().c_str() ) != 0)
        {
            replyList.addString(list.get(i).asString());
        }
    }
    return replyList;
}

/**********************************************************/
Bottle ActivityInterface::getToolLikeNames()
{
    Bottle names;
    cv::Point res;
    
    IplImage *blob_in = (IplImage *) blobPortIn.read(true)->getIplImage();
    
    cv::Mat img(blob_in);
    
    cv::Mat temp(img.rows,img.cols,CV_8UC1);
    cv::Mat dst(img.rows,img.cols,CV_8UC1,cv::Scalar::all(0));
    img.copyTo(temp);
    
    std::vector<std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;
    
    std::vector<cv::Point> convex_hull;
    
    findContours( temp, contours, hierarchy,CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE );
    
    //std::vector<double> allLenghts;
    std::map<int, double> allLengths;
    
    for( int i = 0; i< contours.size(); i++ )
    {
        double area = cv::contourArea( contours[i],false);
        
        if (area > 1000 && area < 5000) //first screaning - only accept something big enough
        {
            double length = getAxes(contours[i], dst);
            allLengths.insert(pair<int, double>(i, length));
        }
    }
    
    //make sure that the max diff is a bit smaller than only min (therefore min+min/2)
    double max_diff = getPairMax(allLengths) - (getPairMin(allLengths) + getPairMin(allLengths)/2);
    
    //fprintf(stdout, "the diff is %lf \n", max_diff);
    
    std::vector<cv::Point > tempPoints;
    
    for (std::map<int, double>::iterator it=allLengths.begin(); it!=allLengths.end(); ++it)
    {
        int id = it->first;
        
        if (it->second > max_diff)
        {
            // convex hull
            cv::convexHull(contours[id], convex_hull, false);
            if (convex_hull.size() < 3 )
                fprintf(stdout, "ERROR in getToolLikeNames with convexHull \n");
            
            // center of gravity
            cv::Moments mo = cv::moments(convex_hull);
            res = cv::Point(mo.m10/mo.m00 , mo.m01/mo.m00);
            
            bool shouldAdd = true;
            for (int i = 0; i < tempPoints.size(); i++)
            {
                if ( abs(res.x - tempPoints[i].x)<10 && abs(res.y - tempPoints[i].y)<10)
                    shouldAdd = false;
            }
            if (shouldAdd)
            {
                string label = getLabel(res.x, res.y);
                names.addString(label.c_str());
            }
            tempPoints.push_back(res);
        }
    }
    
    return names;
}

/**********************************************************/
double ActivityInterface::getAxes(vector<cv::Point> &pts, cv::Mat &img)
{

    cv::Mat data_pts = cv::Mat(pts.size(), 2, CV_64FC1);
    for (int i = 0; i < data_pts.rows; ++i)
    {
        data_pts.at<double>(i, 0) = pts[i].x;
        data_pts.at<double>(i, 1) = pts[i].y;
    }
    
    //Perform PCA analysis
    cv::PCA pca_analysis(data_pts, cv::Mat(), CV_PCA_DATA_AS_ROW);
    
    //Store the position of the object
    cv::Point pos = cv::Point(pca_analysis.mean.at<double>(0, 0), pca_analysis.mean.at<double>(0, 1));
    
    //Store the eigenvalues and eigenvectors
    vector<cv::Point2d> eigen_vecs(2);
    vector<double> eigen_val(2);
    for (int i = 0; i < 2; ++i)
    {
        eigen_vecs[i] = cv::Point2d(pca_analysis.eigenvectors.at<double>(i, 0), pca_analysis.eigenvectors.at<double>(i, 1));
        eigen_val[i] = pca_analysis.eigenvalues.at<double>(0, i);
    }
    
    cv::Point ptaxe1 = pos;
    cv::Point ptaxe2 = pos + 0.02 * cv::Point(eigen_vecs[0].x * eigen_val[0], eigen_vecs[0].y * eigen_val[0]);
    
    double axe1 = cv::norm(ptaxe1-ptaxe2);
    
    //orientation just in case
    //atan2(eigen_vecs[0].y, eigen_vecs[0].x);
    
    return axe1;
}

/**********************************************************/
double ActivityInterface::getPairMin(std::map<int, double> pairmap)
{
    std::pair<int, double> min
    = *min_element(pairmap.begin(), pairmap.end(), compare());
    return min.second;
}

/**********************************************************/
double ActivityInterface::getPairMax(std::map<int, double> pairmap)
{
    std::pair<int, double> min
    = *max_element(pairmap.begin(), pairmap.end(), compare());
    return min.second;
}

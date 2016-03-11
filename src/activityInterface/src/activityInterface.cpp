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
#include <yarp/os/Log.h>
#include <yarp/os/LogStream.h>

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
    with_robot = false; //make sure it does not go here
    shouldUpdate = true;
    
    moduleName = rf.check("name", Value("activityInterface"), "module name (string)").asString();
    Bottle bArm[2];
    
    bArm[LEFT]=rf.findGroup("left_arm");
    bArm[RIGHT]=rf.findGroup("right_arm");
    
    if (rf.check("no_update"))
        shouldUpdate = false;
    
    if (rf.check("no_kinematics"))
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
    
    yInfo("reachAboveOrient[LEFT] %s\n", reachAboveOrient[LEFT].toString().c_str());
    yInfo("reachAboveOrient[RIGHT] %s\n", reachAboveOrient[RIGHT].toString().c_str());
    
    setName(moduleName.c_str());
    
    handlerPortName =  "/";
    handlerPortName += getName();
    handlerPortName +=  "/rpc:i";
    closing=false;
    
    if (!rpcPort.open(handlerPortName.c_str()))
    {
        yError("%s : Unable to open port %s\n", getName().c_str(), handlerPortName.c_str());
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
    
    inputBlobPortName = "/" + moduleName + "/blobImg:i";
    imgeBlobPort.open( inputBlobPortName.c_str() );
    
    inputImagePortName= "/" + moduleName + "/image:i";
    imagePortIn.open(inputImagePortName.c_str());
    
    rpcPraxiconInterface.setReporter(memoryReporter);
    rpcPraxiconInterface.open(("/"+moduleName+"/praxicon:rpc").c_str());
    
    pradaReporter.setManager(this);
    pradaReporter.open(("/"+moduleName+"/prada:i").c_str());
    
    speechReporter.setManager(this);
    speechReporter.open(("/"+moduleName+"/speech:i").c_str());
    
    praxiconToPradaPort.open(("/"+moduleName+"/praxicon:o").c_str());
    
    blobsPort.open(("/"+moduleName+"/blobs:i").c_str());
    
    rpcKarma.open(("/"+moduleName+"/karma:o").c_str());
    
    imgClassifier.open(("/"+moduleName+"/imgClassifier:o").c_str());
    dispBlobRoi.open(("/"+moduleName+"/dispBlobRoi:i").c_str());
    
    rpcClassifier.open(("/"+moduleName+"/classify:rpc").c_str());
    
    yarp::os::Network::connect(("/"+moduleName+"/arecmd:rpc").c_str(), "/actionsRenderingEngine/cmd:io");
    yarp::os::Network::connect(("/"+moduleName+"/are:rpc").c_str(), "/actionsRenderingEngine/get:io");
    yarp::os::Network::connect(("/"+moduleName+"/memory:rpc").c_str(), "/memory/rpc");
    yarp::os::Network::connect(("/"+moduleName+"/iolState:rpc").c_str(), "/iolStateMachineHandler/human:rpc");

    
    yarp::os::Network::connect("/icub/camcalib/left/out", inputImagePortName.c_str());
    yarp::os::Network::connect(("/"+moduleName+"/praxicon:rpc").c_str(), "/praxInterface/speech:i");
    
    
    yarp::os::Network::connect("/lbpExtract/extractedlbp:o", ("/" + moduleName + "/blobImg:i").c_str());
    yarp::os::Network::connect(("/lbpExtract/blobs:o"), ("/"+moduleName+"/blobs:i").c_str());
    
    yarp::os::Network::connect(("/"+moduleName+"/karma:o"), ("/karmaMotor/rpc"));
    
    yarp::os::Network::connect("/dispBlobber/roi/left:o", ("/"+moduleName+"/dispBlobRoi:i").c_str());
    
    yarp::os::Network::connect("/"+moduleName+"/imgClassifier:o", ("/himrepClassifierRoi/img:i"));
    yarp::os::Network::connect("/"+moduleName+"/classify:rpc", ("/himrepClassifierRoi/rpc"));
    
    yarp::os::Network::connect("/fingerSpikes/activity:rpc ", ("/"+moduleName+"/rpc:i"));
    
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
            yError("Device not available\n");
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
    
    goHome();
    
    allPaused = false;
    
    activeSeg.configure(rf);
    
    inAction = false;
    
    yInfo("[configure] done initialization\n");
    
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
    imgeBlobPort.interrupt();
    rpcPraxiconInterface.interrupt();
    pradaReporter.interrupt();
    speechReporter.interrupt();
    praxiconToPradaPort.interrupt();
    imagePortIn.interrupt();
    blobsPort.interrupt();
    rpcKarma.interrupt();
    imgClassifier.interrupt();
    dispBlobRoi.interrupt();
    rpcClassifier.interrupt();
    semaphore.post();
    return true;
}

/**********************************************************/
bool ActivityInterface::close()
{
    semaphore.wait();
    yInfo("[closing] starting the shutdown procedure\n");
    rpcPort.close();
    client_left.close();
    client_right.close();
    rpcMemory.close();
    rpcARE.close();
    rpcAREcmd.close();
    robotStatus.close();
    rpcWorldState.close();
    rpcIolState.close();
    imgeBlobPort.close();
    rpcPraxiconInterface.close();
    pradaReporter.close();
    speechReporter.close();
    praxiconToPradaPort.close();
    imagePortIn.close();
    blobsPort.close();
    rpcKarma.close();
    imgClassifier.close();
    dispBlobRoi.close();
    rpcClassifier.close();
    yInfo("[closing] finished shutdown procedure\n");
    semaphore.post();
    return true;
}

/**********************************************************/
bool ActivityInterface::goHome()
{
    bool reply;

    yInfo("[goHome] requested\n");
    Bottle are, replyAre;
    are.clear(),replyAre.clear();
    are.addString("home");
    are.addString("head");
    rpcAREcmd.write(are,replyAre);
    
    if (strcmp (replyAre.toString().c_str(),"[ack]") == 0)
        reply = true;
    else
        reply = false;
    
    yInfo("[goHome] excecuted\n");
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
    return true;
}

/**********************************************************/
Bottle ActivityInterface::askPraxicon(const string &request)
{
    praxiconRequest = request;
    Bottle listOfGoals, cmdPrax, replyPrax;
    
    yInfo("[askPraxicon] request is : %s \n", praxiconRequest.c_str());
    
    Bottle toolLikeMemory = getToolLikeNames();
    
    Bottle objectsMemory = getNames();
    
    yInfo("[askPraxicon] tool names: %s \n", toolLikeMemory.toString().c_str());
    yInfo("[askPraxicon] object names: %s \n", objectsMemory.toString().c_str());
    
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
    
    yInfo("[askPraxicon] sending \n %s \n", cmdPrax.toString().c_str());
    //send it all to praxicon
    rpcPraxiconInterface.write(cmdPrax,replyPrax);
    
    Bottle &tmpList = listOfGoals.addList();
    //listOfGoals.clear();
    vector<string> tokens;
    
    if (replyPrax.size() > 0)
    {
        for (int i=0; i<replyPrax.size()-1; i++) //-1 to remove (mouth speak goal from praxicon)
        {
            string replytmp = replyPrax.get(i).asString().c_str();
            istringstream iss(replytmp);
            
            copy(istream_iterator<string>(iss),
                 istream_iterator<string>(),
                  back_inserter(tokens));
        }
        
        tokens.push_back("endofstring"); //adding this to prevent missing the last goal or going out of range
        
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
    }
    else
        executeSpeech("something went terribly wrong. I cannot " + request);
    
    
    
    praxiconToPradaPort.write(listOfGoals);
    
    return listOfGoals;
}

/**********************************************************/
bool ActivityInterface::processSpeech(const Bottle &speech)
{
    if ( speech.size() > 0 )
    {
        string s = speech.toString();
        //remove extra characters
        //    "\"make a sandwich\""
        s.erase(0,1);
        s.erase(s.size()-1,1);
        askPraxicon(s.c_str());
    }
    return true;
}

/**********************************************************/
bool ActivityInterface::processPradaStatus(const Bottle &status)
{
    Bottle objectsUsed;
    Bottle buns;
    buns.addString("bun-bottom");
    buns.addString("bun-top");
    int passed[buns.size()];
    
    if ( status.size() > 0 )
    {
        yInfo( "[processPradaStatus] the status is %s \n", status.toString().c_str());
        if (strcmp (status.get(0).asString().c_str(), "OK" ) == 0)
        {
            for (int i=1; i< status.size(); i++)
            {
                for (int x=0; x<buns.size(); x++)
                    passed[x] = 0;
                
                for (int ii=0; ii<buns.size(); ii++)
                    if (strcmp (status.get(i).asString().c_str(), buns.get(ii).asString().c_str() ) != 0 )
                        passed[ii] = 1;
                
                int total=0;
                for (int x=0; x<buns.size(); x++)
                    total+=passed[x];
                        
                if (total==buns.size())
                    objectsUsed.addString(status.get(i).asString().c_str());
            }
            executeSpeech("I made a " + objectsUsed.toString() + " sandwich");
            yInfo("[processPradaStatus] made a %s \n", objectsUsed.toString().c_str());
        }
        else if (strcmp (status.get(0).asString().c_str(), "FAIL" ) == 0)
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
            yInfo( "[processPradaStatus] Something is wrong with the status\n");
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
bool ActivityInterface::pauseAllTrackers()
{
    Bottle objects = getOPCNames();
    
    for (int i=0; i<objects.size();i++)
    {
        Bottle cmdPauseThread, replyPauseThread;
        cmdPauseThread.clear();
        cmdPauseThread.addString("pause");
        cmdPauseThread.addString(objects.get(i).asString().c_str());
        
        yInfo("[pauseAll]: %s \n", cmdPauseThread.toString().c_str() );
        
        rpcWorldState.write(cmdPauseThread,replyPauseThread);
    }
    allPaused = true;
    return true;
}

/**********************************************************/
bool ActivityInterface::resumeAllTrackers()
{
    Bottle objects = getOPCNames();
    
    for (int i=0; i<objects.size();i++)
    {
        Bottle cmdPauseThread, replyPauseThread;
        cmdPauseThread.clear();
        cmdPauseThread.addString("resume");
        cmdPauseThread.addString(objects.get(i).asString().c_str());
        
        yInfo("[resumeAll]: %s \n", cmdPauseThread.toString().c_str() );
        
        rpcWorldState.write(cmdPauseThread,replyPauseThread);
    }
    allPaused = false;
    return true;
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
        
        //yError("[handleTrackers] asking 3D");
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
                
                yInfo("[handleTrackers] will send: %s \n", cmdPauseThread.toString().c_str() );
                
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
                
                yInfo("[handleTrackers] will send: %s \n", cmdPauseThread.toString().c_str() );
                
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
    if(shouldUpdate)
    {
        propagateStatus();
        
        if (!allPaused)
            handleTrackers();
    }
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
                    
                    if (time < 1.0)
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
    
    //yError("[get3D] getMemoryBottle %s",Memory.toString().c_str() );
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
Bottle ActivityInterface::get2D(const string &objName)
{
    Bottle Memory = getMemoryBottle();
    Bottle position2D;
    
    for (int i=0; i<Memory.size(); i++)
    {
        if (Bottle *propField = Memory.get(i).asList())
        {
            if (propField->check("name"))
            {
                if (strcmp (propField->find("name").asString().c_str(), objName.c_str() ) == 0)
                {
                    if (propField->check("position_2d_left"))
                    {
                        Bottle *propFieldPos = propField->find("position_2d_left").asList();
                        
                        for (int i=0; i < propFieldPos->size(); i++)
                        {
                            position2D.addDouble(propFieldPos->get(i).asDouble());
                        }
                    }
                }
            }
        }
    }
    return position2D;
}

/**********************************************************/
Bottle ActivityInterface::getOffset(const string &toolName)
{
    Bottle toolOffset;
    toolOffset.clear();
    
    if (strcmp ("rake", toolName.c_str() ) == 0)
    {
        toolOffset.addDouble(0.18);
        toolOffset.addDouble(-0.18); //right hand should be negative
        toolOffset.addDouble(0.04);
    }
    else if (strcmp ("stick", toolName.c_str() ) == 0)
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
    /*Bottle toolLikeMemory = getToolLikeNames();
    bool isTool = false;
    
    string handStatus;
    for (int i =0; i< toolLikeMemory.size(); i++)
    {
        handStatus = inHand(toolLikeMemory.get(i).toString().c_str());
        
        if (strcmp (handStatus.c_str() , handName.c_str()) == 0)
        {
            isTool = true;
        }
    }*/
    bool isTool = false;
    string handRake = inHand("rake");
    
    if (strcmp (handRake.c_str(), handName.c_str() ) == 0 )
        isTool = true;
    
    string handStick = inHand("stick");
    if (strcmp (handStick.c_str(), handName.c_str() ) == 0 )
        isTool = true;
    
    if (!isTool)
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
    else if (isTool)
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
Bottle ActivityInterface::getCog(const int32_t tlpos_x, const int32_t tlpos_y, const int32_t brpos_x, const int32_t brpos_y)
{
    Bottle cmd;
    Bottle &list=cmd.addList();
    list.addInt(tlpos_x);
    list.addInt(tlpos_y);
    list.addInt(brpos_x);
    list.addInt(brpos_y);

    Bottle cog = getBlobCOG(cmd, 0);
    
    yInfo("the orig points are %d %d %d %d\n", tlpos_x, tlpos_y, brpos_x, brpos_y);
    yInfo("the blob points are %d %d\n", cog.get(0).asInt(), cog.get(1).asInt());
    
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
                
                yInfo("[getLabel] the bounding box list is %s", positionBBox.toString().c_str());
                
                
                Bottle *item=positionBBox.get(0).asList();
                
                
                double a = item->get(0).asDouble();
                double b = item->get(1).asDouble();
                double c = item->get(2).asDouble();
                double d = item->get(3).asDouble();
               
                yInfo("a= %lf, b= %lf, c= %lf, d= %lf", a, b, c, d);
                
                if (a < pos_x && pos_x < c)
                {
                    if (b < pos_y && pos_y < d)
                    {
                        yInfo("INSIDE BB");
                        if (propField->check("name"))
                        {
                            label = propField->find("name").asString().c_str();
                            yInfo("[getLabel] adding label %s",label.c_str());
                            break;
                        }
                    }
                    
                }else
                    yInfo("OUTSIDE bounding box");
                                
            }
        }
    }
    return label;
}

/**********************************************************/
double ActivityInterface::getManip(const string &objName, const std::string &handName)
{
    yInfo("[getManip] asking 3D");
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
            
            yInfo(" \n\n\nPosition is: %s \n", x.toString().c_str());
            yInfo("Left orientation is: %s \n", o_left.toString().c_str());
            yInfo("Right orientation is: %s \n \n \n", o_right.toString().c_str());
            
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
            
            yInfo("q_left is: %s \n", q_left.toString().c_str());
            yInfo("q_right is: %s \n", q_right.toString().c_str());
            
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
        //adding this in case of no robot
        yInfo("[getManip] asking 3D");
        Bottle position = get3D(objName);
        
        if (position.get(0).asDouble() > -0.55 && position.get(1).asDouble() < - 0.07)
        {
            if (strcmp (handName.c_str(),"left") == 0)
                manip = 0.8;
            if (strcmp (handName.c_str(),"right") == 0)
                manip = 0.3;
        }
        else if (position.get(0).asDouble() > -0.55 && position.get(1).asDouble() > 0.07)
        {
            if (strcmp (handName.c_str(),"left") == 0)
                manip = 0.3;
            if (strcmp (handName.c_str(),"right") == 0)
                manip = 0.8;
        }
        else if (position.get(0).asDouble() > -0.55 && position.get(1).asDouble() < 0.07 && position.get(1).asDouble() > - 0.07)
        {
            if (strcmp (handName.c_str(),"left") == 0)
                manip = 0.8;
            if (strcmp (handName.c_str(),"right") == 0)
                manip = 0.8;
        }
        else
        {
            if (strcmp (handName.c_str(),"left") == 0)
                manip = 0.1;
            if (strcmp (handName.c_str(),"right") == 0)
                manip = 0.1;
        }
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
string ActivityInterface::holdIn(const string &handName)
{
    string object;
    
    for (std::map<string, string>::iterator it=inHandStatus.begin(); it!=inHandStatus.end(); ++it)
    {
        if (strcmp (it->second.c_str(), handName.c_str() ) == 0)
            object = it->first.c_str();
        
    }
    if (object.empty())
        object = "none";
    
    return object;
}

/**********************************************************/
bool ActivityInterface::take(const string &objName, const string &handName)
{
    inAction = true;
    pauseAllTrackers();
    
    //check for hand status beforehand to make sure that it is empty
    string handStatus = inHand(objName);
    
    if (strcmp (handStatus.c_str(), "none" ) == 0 )
    {
        //talk to iolStateMachineHandler
        yInfo("[take] asking 3D");
        Bottle position = get3D(objName);
    
        if (position.size()>0)
        {
            yInfo("[take] object is visible at %s will do the take action \n", position.toString().c_str());
            
            yInfo("[take] will initialise obj \n");
            initObjectTracker(objName);
            yInfo("[take] done initialising obj \n");
            
            executeSpeech("ok, I will take the " + objName);
            
            //check that objName ! = tools
            //initObjectTracker(objName);
            
            //do the take actions
            Bottle cmd, reply;
            cmd.clear(), reply.clear();
            cmd.addString("take");
            cmd.addString(objName.c_str());
            cmd.addString(handName.c_str());
            cmd.addString("still");
            rpcAREcmd.write(cmd, reply);
            
            if (reply.get(0).asVocab()==Vocab::encode("ack"))
            {
                //do the take actions
                Bottle cmd, reply;
                cmd.clear(), reply.clear();
                cmd.addString("observe");
                cmd.addString(handName.c_str());
                rpcAREcmd.write(cmd, reply);
                
                if (classifyObserve())
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

                    //do the take actions
                    Bottle cmd, reply;
                    cmd.clear(), reply.clear();
                    cmd.addString("home");
                    cmd.addString("arms");
                    cmd.addString("head");
                    rpcAREcmd.write(cmd, reply);

                    //update inHandStatus map
                    inHandStatus.insert(pair<string, string>(objName.c_str(), handName.c_str()));
                    
                }else
                {
                    Bottle cmd, reply;
                    cmd.clear(), reply.clear();
                    cmd.addString("home");
                    cmd.addString("all");
                    rpcAREcmd.write(cmd, reply);
                }
            }
            else
            {
                executeSpeech("I have failed to take the" + objName);
                yError("[take] I have failed to take the %s\n" , objName.c_str());
            }
        }
        else
        {
            executeSpeech("I am sorry I cannot see the" + objName);
            yError("[take] I cannot see the %s\n" , objName.c_str());
        }
    }
    else
    {
        executeSpeech("I already have the " + objName + " in my hand");
        yError("[take] Cannot grasp already have something in hand\n");
    }

    resumeAllTrackers();
    inAction = false;
    return true;
}

/**********************************************************/
bool ActivityInterface::drop(const string &objName)
{
    pauseAllTrackers();
    string handName = inHand(objName);
    if (strcmp (handName.c_str(), "none" ) != 0 )
    {
        executeSpeech("ok, I will drop the " + objName );
        //do the take actions
        Bottle cmd, reply;
        cmd.clear(), reply.clear();
        cmd.addString("drop");
        cmd.addString("away");
        cmd.addString(handName.c_str());
        rpcAREcmd.write(cmd, reply);
        
        if (reply.get(0).asVocab()==Vocab::encode("ack"))
        {
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
    else
    {
        executeSpeech("I am not holding anything");
        yError("[drop]I am not holding anything\n");
    }
    resumeAllTrackers();
    return true;
}

/**********************************************************/
bool ActivityInterface::put(const string &objName, const string &targetName)
{
    pauseAllTrackers();
    string handName = inHand(objName);
    
    if (strcmp (handName.c_str(), "none" ) != 0 )
    {
        bool useStackedObjs = false;
        
        for (std::map<string, cv::Scalar>::iterator it=stakedObject.begin(); it!=stakedObject.end(); ++it)
        {
            if (strcmp (it->first.c_str(), targetName.c_str() ) == 0)
            {
                useStackedObjs = true;
            }
        }
        Bottle cmd, reply;
        if(useStackedObjs)
        {
            Bottle position = trackStackedObject(targetName);
            executeSpeech("ok, I will place the " + objName + " on the " + targetName );
            
            yInfo("I will place %s on the %s with position %d %d ", objName.c_str(), targetName.c_str(), position.get(0).asInt(), position.get(1).asInt() );
            
            //do the take actions
            
            cmd.clear(), reply.clear();
            cmd.addString("drop");
            cmd.addString("over");
            Bottle &tmp=cmd.addList();
            tmp.addInt (position.get(0).asInt());
            tmp.addInt (position.get(1).asInt());
            cmd.addString("gently");
            cmd.addString(handName.c_str());
            rpcAREcmd.write(cmd, reply);
        }
        else
        {
            //talk to iolStateMachineHandler
            yInfo("[put] asking 3D");
            Bottle position = get3D(targetName);
            
            if (position.size()>0)
            {
                executeSpeech("ok, I will place the " + objName + " on the " + targetName);
                
                //do the take actions
                cmd.clear(), reply.clear();
                cmd.addString("drop");
                cmd.addString("over");
                cmd.addString(targetName.c_str());
                cmd.addString("gently");
                cmd.addString(handName.c_str());
                rpcAREcmd.write(cmd, reply);
            }
            else
            {
                yError("[put] there seems to be a problem with the object\n");
                executeSpeech("There seems to be an issue with the command");
            }
        }
        
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
    else
    {
        yInfo("[put] I have nothing in my hand ");
    }
    resumeAllTrackers();
    return true;
}

/**********************************************************/
bool ActivityInterface::askForTool(const std::string &handName, const int32_t pos_x, const int32_t pos_y)
{
    // Get the label of the object requested
    string label = getLabel(pos_x, pos_y);
    
    Bottle cmdHome, cmdReply;
    cmdHome.clear();
    cmdReply.clear();
    cmdHome.addString("home");
    cmdHome.addString("head");
    rpcAREcmd.write(cmdHome,cmdReply);
    
    if (!label.empty())
    {
        executeSpeech ("can you give me the " + label + "please?");
        
        yInfo( "[askForTool] tool label is: %s \n",label.c_str());
        
        pauseAllTrackers();
        Bottle cmdAre, replyAre;
        cmdAre.clear();
        replyAre.clear();
        cmdAre.addString("point");
        Bottle &tmp=cmdAre.addList();
        tmp.addInt (pos_x);
        tmp.addInt (pos_y);
        
        cmdAre.addString(handName.c_str());
        rpcAREcmd.write(cmdAre,replyAre);
        
        cmdAre.clear();
        replyAre.clear();
        cmdAre.addString("tato");
        cmdAre.addString(handName.c_str());
        rpcAREcmd.write(cmdAre, replyAre);
        
        yInfo("[askForTool] done tato\n");
        
        cmdAre.clear();
        replyAre.clear();
        cmdAre.addString("hand");
        cmdAre.addString("close_hand_tool");
        cmdAre.addString(handName.c_str());
        rpcAREcmd.write(cmdAre, replyAre);
        Time::delay(5.0);
        yInfo( "[askForTool] done close\n");
        
        cmdAre.clear();
        replyAre.clear();
        cmdAre.addString("home");
        cmdAre.addString("arms");
        cmdAre.addString("head");
        rpcAREcmd.write(cmdAre, replyAre);
        
        //update inHandStatus map
        
        inHandStatus.insert(pair<string, string>(label.c_str(), handName.c_str()));
        
        if (availableTools.size()<1)
        {
            availableTools.push_back(label.c_str());
            yInfo("[askForTool] availableTools is empty\n");
            yInfo("[askForTool] adding %s to list\n", label.c_str());
        }
        else
        {
            if (std::find(availableTools.begin(), availableTools.end(), label.c_str()) == availableTools.end())
            {
                yInfo("[askForTool] name %s NOT available\n", label.c_str());
                yInfo("[askForTool] adding it to list\n");
                availableTools.push_back(label.c_str());
            }
        }
        resumeAllTrackers();
    }
    else
    {
        executeSpeech ("I cannot see anything at this position");
        yError("[askForTool] Cannot see anything at this position\n");
    }
    
    return true;
}

/**********************************************************/
bool ActivityInterface::push(const string &objName, const string &toolName)
{
    pauseAllTrackers();
    yInfo("[push] asked to pull %s with %s\n", objName.c_str(), toolName.c_str());
    //ask for tool
    //do the pulling action
    yInfo( "[push] done");
    resumeAllTrackers();
    return true;
}

/**********************************************************/
bool ActivityInterface::pull(const string &objName, const string &toolName)
{
    pauseAllTrackers();
    
    yInfo("[pull] asked to pull %s with %s\n", objName.c_str(), toolName.c_str());
    yInfo("[pull] asking 3D");
    Bottle position = get3D(objName);
    Bottle toolOffset = getOffset(toolName);
    
    //tool attach
    string handName = inHand(toolName);
    
    yInfo( "[pull] will use the %s hand on position %s\n", handName.c_str(), position.toString().c_str());
    
    if (strcmp (handName.c_str(), "none" ) != 0 && position.size()>0)
    {
        yInfo("[pull] will remove any tool\n");
        Bottle cmdkarma, replykarma;
        cmdkarma.clear();
        cmdkarma.clear();
        cmdkarma.addString("tool");
        cmdkarma.addString("remove");
        
        rpcKarma.write(cmdkarma, replykarma);
        
        yInfo("[pull] will attach the tool using\n");
        
        cmdkarma.clear();replykarma.clear();
        cmdkarma.addString("tool");
        cmdkarma.addString("attach");
        cmdkarma.addString(handName.c_str());
        cmdkarma.addDouble(0.18);
        cmdkarma.addDouble(-0.18);
        
        if (strcmp (handName.c_str(), "left" ) == 0)
            cmdkarma.addDouble(0.04);
        else
            cmdkarma.addDouble(-0.04);

        yInfo("[pull] %s\n",cmdkarma.toString().c_str());
        rpcKarma.write(cmdkarma, replykarma);
        
        double result = 0.0;
        double xoffset = 0.05;
        
        yInfo("[pull] Will now send to karmaMotor:\n");
        Bottle karmaMotor,KarmaReply;
        karmaMotor.addString("vdraw");
        karmaMotor.addDouble(position.get(0).asDouble() + 0.05);
        karmaMotor.addDouble(position.get(1).asDouble());
        karmaMotor.addDouble(position.get(2).asDouble() + xoffset);
        karmaMotor.addDouble(90.0);
        karmaMotor.addDouble(0.04); //10 cm
        karmaMotor.addDouble(0.1);
        yInfo("[pull] will send \n %s to KarmaMotor \n", karmaMotor.toString().c_str());
        rpcKarma.write(karmaMotor, KarmaReply);
        
        yInfo("[pull] vdraw is %s:\n",KarmaReply.toString().c_str());
        result = KarmaReply.get(1).asDouble();
        
        yInfo("[pull] vdraw is %lf:\n",result);
        
        if (result > 0 && result < 0.2)
        {
            Bottle karmaMotor,KarmaReply;
            karmaMotor.addString("draw");
            karmaMotor.addDouble(position.get(0).asDouble() + 0.05);
            karmaMotor.addDouble(position.get(1).asDouble());
            karmaMotor.addDouble(position.get(2).asDouble() + xoffset);
            karmaMotor.addDouble(90.0);
            karmaMotor.addDouble(0.04);
            karmaMotor.addDouble(0.15);
            yInfo("[pull] will send \n %s to KarmaMotor \n", karmaMotor.toString().c_str());
            rpcKarma.write(karmaMotor, KarmaReply);
            
            Bottle are,reply;
            are.clear(),reply.clear();
            are.addString("home");
            are.addString("head");
            are.addString("arms");
            rpcAREcmd.write(are,reply);
            
        }
        else
        {
            executeSpeech("I cannot safely do this");
            yError("[pull] Cannot safely do this\n");
        }
    }
    else
    {
        executeSpeech("I have nothing in my hand");
        yError("[pull] Nothing in my hand\n");
    }
    
    //ask for tool
    //do the pushing action
    yInfo( "[push] done");
    resumeAllTrackers();
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
bool ActivityInterface::testFill()
{
    int elements = 0;
    
    onTopElements.insert(pair<int, string>(elements, "bun-bottom"));
    elements ++;
    onTopElements.insert(pair<int, string>(elements, "meat"));
    elements ++;
    onTopElements.insert(pair<int, string>(elements, "cheese"));
    elements ++;
    onTopElements.insert(pair<int, string>(elements, "pickles"));
    elements ++;
    onTopElements.insert(pair<int, string>(elements, "bun-top"));
    
    return true;
}

/**********************************************************/
bool ActivityInterface::resetObjStack()
{
    onTopElements.clear();
    elements = 0;
    return true;
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
            if (propField->check("position_2d_left"))
            {
                if (propField->check("name"))
                {
                    names.addString(propField->find("name").asString());
                }
            }
        }
    }
    return names;
}

/**********************************************************/
Bottle ActivityInterface::getOPCNames()
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
    
    yError("[reachableWith] asking 3D");
    position = get3D(objName);
    
    yInfo("[reachableWith] position is %lf %lf %lf \n", position.get(0).asDouble(), position.get(1).asDouble(), position.get(2).asDouble());

    if (position.get(0).asDouble() < -0.48){
        
        Bottle list = pullableWith(objName);
        
        yInfo("[reachableWith] pullableWith list is %s", list.toString().c_str());
        
        for (int i = 0; i<list.size(); i++)
            replyList.addString(list.get(i).asString());
        
        // check if tool is in hand
        if (handStat("left")){
         
            for (std::map<string, string>::iterator it=inHandStatus.begin(); it!=inHandStatus.end(); ++it){
                
                if (strcmp (it->second.c_str(), "left" ) == 0)
                    replyList.addString(it->first.c_str());
            }
            
        }
        if (handStat("right")){
            for (std::map<string, string>::iterator it=inHandStatus.begin(); it!=inHandStatus.end(); ++it){
                
                if (strcmp (it->second.c_str(), "right" ) == 0)
                    replyList.addString(it->first.c_str());
            }
        }
    }
    else{
        
        //Bottle toolList = getToolLikeNames();
        //yInfo("toolList list is %s", toolList.toString().c_str());
        
        Bottle list = getNames();
        
        yInfo("[reachableWith] getNames list is %s with size %d", list.toString().c_str(), list.size());
        
        for (int i = 0; i<list.size(); i++){
            
            if (strcmp (objName.c_str(), list.get(i).asString().c_str() ) != 0)
            {
                //yInfo("check for objects, adding: %s", list.get(i).toString().c_str());
                replyList.addString(list.get(i).asString());
                
            }
            //yInfo("replyList so far: %s", replyList.toString().c_str())
        }
        
        // check if tool is in hand
        if (handStat("left")){
            
            for (std::map<string, string>::iterator it=inHandStatus.begin(); it!=inHandStatus.end(); ++it){
                
                if (strcmp (it->second.c_str(), "left" ) == 0)
                    replyList.addString(it->first.c_str());
            }
            
        }
        if (handStat("right")){
            for (std::map<string, string>::iterator it=inHandStatus.begin(); it!=inHandStatus.end(); ++it){
                
                if (strcmp (it->second.c_str(), "right" ) == 0)
                    replyList.addString(it->first.c_str());
            }
        }
        //double leftManip = getManip(objName, "left");
        //double rightManip = getManip(objName, "right");
        //fprintf(stdout, "\nleftManip: %lf and rightManip: %lf\n", leftManip, rightManip);
        
        yInfo("after all: %s", replyList.toString().c_str());
        
        //using 3D instead of manip for testing
        if(position.get(1).asDouble() < - 0.2 )
            replyList.addString("left");
        else if (position.get(1).asDouble() >  0.2 )
            replyList.addString("right");
        else{
            replyList.addString("left");
            replyList.addString("right");
        }
        yInfo("will now send: %s", replyList.toString().c_str());
    }
    
    return replyList;
}
/**********************************************************/
Bottle ActivityInterface::pullableWith(const string &objName)
{
    Bottle list = getToolLikeNames();
    
    //-----------------------for tests @ IIT
    
    /*Bottle list;
    list.clear();
    list.addString("stick");
    list.addString("hoe");
     */
    
    //-----------------------for tests @ IIT
    
    yInfo("[pullableWith] available tools size = %lu \n", availableTools.size());
    yInfo("[pullableWith] list  size = %d \n", list.size());
    if (availableTools.size()<1)
    {
        for (int i = 0; i<list.size(); i++)
        {
            availableTools.push_back(list.get(i).asString().c_str());
            yInfo("[pullableWith] adding %s\n", list.get(i).asString().c_str());
        }
    }
    else // check that all names are in, in case new object come in view
    {
        for (int i = 0; i<list.size(); i++)
        {
            if (std::find(availableTools.begin(), availableTools.end(), list.get(i).asString().c_str()) == availableTools.end())
            {
                yInfo("[pullableWith] name %s NOT available\n", list.get(i).asString().c_str());
                yInfo("[pullableWith] adding %s\n", list.get(i).asString().c_str());
                availableTools.push_back(list.get(i).asString().c_str());
            }
        }
    }
    
    Bottle replyList;
    replyList.clear();
    for (int i = 0; i<availableTools.size(); i++)
    {
        if (strcmp (objName.c_str(), availableTools[i].c_str() ) != 0)
        {
            replyList.addString(availableTools[i].c_str());
        }
    }
    return replyList;
}

/**********************************************************/
Bottle ActivityInterface::getToolLikeNames()
{
    Bottle names;
    cv::Point res;
    
    IplImage *blob_in = (IplImage *) imgeBlobPort.read(true)->getIplImage();
    
    cv::Mat img(blob_in);
    
    cv::Mat temp(img.rows,img.cols,CV_8UC1);
    cv::Mat dst(img.rows,img.cols,CV_8UC1,cv::Scalar::all(0));
    img.copyTo(temp);
    
    std::vector<std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;
    
    std::vector<cv::Point> convex_hull;
    
    findContours( temp, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_TC89_L1 );
    
    // Get the moments
    vector<cv::Moments> mu(contours.size() );
    for( int i = 0; i < contours.size(); i++ )
        mu[i] = moments( contours[i], false );
    
    // Get the mass centers:
    vector<cv::Point2f> mc( contours.size() );
    
    for( int i = 0; i < contours.size(); i++ )
        mc[i] = cv::Point2f( mu[i].m10/mu[i].m00 , mu[i].m01/mu[i].m00 );
    
    std::map<int, double> allLengths;
    
    yDebug("[getToolLikeNames] Number of contours is %zu \n", contours.size());
    
    //vector<cv::Vec4i> hrch;
    
    for( int i = 0; i< contours.size(); i++ )
    {
        yDebug("[getToolLikeNames] Contour[%d] -X[%lf]  -Y[%lf] Area: %.2f Length: %.2f \n", i, mc[i].x, mc[i].y, contourArea(contours[i]), arcLength( contours[i], true ) );
        
        double lenght = arcLength( contours[i], true );//cv::contourArea( contours[i], false);
        
        //yDebug("[getToolLikeNames] area %lf \n", area);
        
        if (lenght > 200 ) //first screaning - only accept something big enough
        {
            double axes = getAxes(contours[i], dst);
            allLengths.insert(pair<int, double>(i, axes));
        }
        //cv::drawContours( temp, contours, i, cvScalar(255,255,255), 2, 8, hrch, 0, cv::Point() );
    }
    
    //make sure that the max diff is a bit smaller than only min (therefore min+min/2)
    double max_diff = getPairMax(allLengths) - (getPairMin(allLengths) + getPairMin(allLengths)/2);
    
    std::vector<cv::Point > tempPoints;
    
    yError("[getToolLikeNames] number of tool like objects found %zu \n", allLengths.size());
    
    for (std::map<int, double>::iterator it=allLengths.begin(); it!=allLengths.end(); ++it)
    {
        int id = it->first;
        
        if (it->second > max_diff)
        {
            // convex hull
            cv::convexHull(contours[id], convex_hull, false);
            if (convex_hull.size() < 3 )
                yError("[getToolLikeNames] ERROR in getToolLikeNames with convexHull \n");
            
            // center of gravity
            cv::Moments mo = cv::moments(convex_hull);
            res = cv::Point(mo.m10/mo.m00 , mo.m01/mo.m00);
            
            bool shouldAdd = true;
            for (int i = 0; i < tempPoints.size(); i++)
            {
                yDebug("[getToolLikeNames] res %d %d \n", res.x, res.y);
                yDebug("[getToolLikeNames] tempPoints %d %d \n", tempPoints[i].x, tempPoints[i].y);
                
                if ( abs(res.x - tempPoints[i].x)<10 && abs(res.y - tempPoints[i].y)<10)
                {
                    shouldAdd = false;
                    yDebug("Not adding ");
                }
            }
            if (shouldAdd)
            {
                string label = getLabel(res.x, res.y);
                
                if (label.length()>0)
                {
                    names.addString(label.c_str());
                    yDebug("[getToolLikeNames] Adding %s \n", label.c_str());
                }
                
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
int ActivityInterface::getPairMinIndex(std::map<int, double> pairmap)
{
    std::pair<int, double> min
    = *min_element(pairmap.begin(), pairmap.end(), compare());
    return min.first;
}

/**********************************************************/
double ActivityInterface::getPairMax(std::map<int, double> pairmap)
{
    std::pair<int, double> min
    = *max_element(pairmap.begin(), pairmap.end(), compare());
    return min.second;
}

/**********************************************************/
bool ActivityInterface::initObjectTracker(const string &objName)
{
    IplImage *image_in = (IplImage *) imagePortIn.read(true)->getIplImage();
    
    bool toReturn = false;
    if( image_in != NULL )
    {
        cvCvtColor(image_in,image_in,CV_BGR2RGB);
        Bottle BB = get2D(objName);
        
        if (BB.size()>0)
        {
            yInfo("[initialiseObjectTracker] the BB is %s\n",BB.toString().c_str());
            
            int cropSizeWidth = abs((int)BB.get(2).asDouble()-(int)BB.get(0).asDouble());
            int cropSizeHeight = abs((int)BB.get(3).asDouble()-(int)BB.get(1).asDouble());
            
            yInfo("[initialiseObjectTracker] cropSizeWidth is %d\n",cropSizeWidth);
            yInfo("[initialiseObjectTracker] cropSizeHeight is %d\n",cropSizeHeight);
            
            cv::Point cog;
            cog.x = ((int)BB.get(0).asDouble()+(int)BB.get(2).asDouble())>>1;
            cog.y = ((int)BB.get(1).asDouble()+(int)BB.get(3).asDouble())>>1;
            
            yInfo("[initialiseObjectTracker] the cog is %d %d\n", cog.x, cog.y);
            
            IplImage *tpl;
            IplImage *seg;
            
            SegInfo info (cog.x, cog.y, cropSizeWidth,  cropSizeHeight);
            
            activeSeg.getSegWithFixation(image_in, seg, info);
            
            activeSeg.getTemplateFromSeg(image_in, seg, tpl, info);
            
            cvCvtColor(tpl,tpl,CV_BGR2RGB);
            
            cv::Mat segmentation(cv::Mat(tpl, true));
            
            //computes mean over seg
            cv::Scalar avgPixel = cv::mean( segmentation );
            
            yInfo("[initialiseObjectTracker] The %s average is: %lf %lf %lf \n",objName.c_str(), avgPixel.val[0], avgPixel.val[1], avgPixel.val[2]);
            
            string name;
            bool shouldDelete = false;
            for (std::map<string, cv::Scalar>::iterator it=stakedObject.begin(); it!=stakedObject.end(); ++it)
            {
                if (strcmp (it->first.c_str(), objName.c_str() ) == 0)
                {
                    yInfo("[initialiseObjectTracker] Already have the %s will delete the old one\n", it->first.c_str());
                    name = it->first.c_str();
                    shouldDelete = true;
                }
            }
            
            if (shouldDelete)
                stakedObject.erase(name.c_str());

            //update stakedObject map
            stakedObject.insert(pair<string, cv::Scalar>(objName.c_str(), avgPixel));
            
            toReturn = true;
            cvReleaseImage(&tpl);
            cvReleaseImage(&seg);
        }
    }
    return toReturn;
}

/**********************************************************/
Bottle ActivityInterface::trackStackedObject(const string &objName)
{
    Bottle position;
    
    Bottle names;
    cv::Point res;
    
    std::map<int, double> allDistances;
    
    Bottle *blobsList = blobsPort.read(false);
    
    cv::Mat img((IplImage *) imagePortIn.read(true)->getIplImage(),true);
    
    if (blobsList->size()>0)
    {
        cv::Point point;
        for (int i=0; i<blobsList->size(); i++)
        {
            Bottle *item=blobsList->get(i).asList();
           
            int blobWidth = abs((int)item->get(2).asDouble()-(int)item->get(0).asDouble());
            int blobHeight = abs((int)item->get(3).asDouble()-(int)item->get(1).asDouble());
            
            
            if (blobHeight < 60)
            {
                cv::Rect roi( (int)item->get(0).asDouble(), (int)item->get(1).asDouble(), blobWidth, blobHeight );
                
                //copies input image in roi
                cv::Mat image_roi = img( roi );
                
                cvtColor( image_roi, image_roi, CV_BGR2RGB );
                
                /*std::string text = "roi_ ";
                text += std::to_string( i );
                text += ".jpg";
                
                imwrite(text.c_str(), image_roi);*/
                
                //computes mean over roi
                cv::Scalar avgPixel = cv::mean( image_roi );
                
                double dist[3];
                double totalDistance;
                
                for (std::map<string, cv::Scalar>::iterator it=stakedObject.begin(); it!=stakedObject.end(); ++it)
                {
                    if (strcmp (it->first.c_str(), objName.c_str() ) == 0)
                    {
                        yInfo("[trackStackedObject] The %s average is: %lf %lf %lf \n",it->first.c_str(), avgPixel.val[0], avgPixel.val[1], avgPixel.val[2]);
                        dist[0] = fabs( it->second.val[0] - avgPixel.val[0]);
                        dist[1] = fabs( it->second.val[1] - avgPixel.val[1]);
                        dist[2] = fabs( it->second.val[2] - avgPixel.val[2]);
                        
                        totalDistance = dist[0] + dist[1] + dist[2];
                        allDistances.insert(pair<int, double>(i, totalDistance));

                    }
                }
            }
        }
        int winner = getPairMinIndex(allDistances);
        double distance = getPairMin(allDistances);
        
        Bottle *item=blobsList->get(winner).asList();
        point.x = ((int)item->get(0).asDouble() + (int)item->get(2).asDouble())>>1;
        point.y = ((int)item->get(1).asDouble() + (int)item->get(3).asDouble())>>1;
        
        yInfo("[trackStackedObject] the winner is blob %d with distance %lf and cog %d %d \n", winner, distance, point.x, point.y);
        
        position.addInt(point.x);
        position.addInt(point.y);
    }
    return position;
}

/**********************************************************/
bool ActivityInterface::trainObserve(const string &label)
{
    ImageOf<PixelRgb> img= *imagePortIn.read(true);
    imgClassifier.write(img);
    
    Bottle bot = *dispBlobRoi.read(true);
    yarp::os::Bottle *items=bot.get(0).asList();
    
    double tlx = items->get(0).asDouble();
    double tly = items->get(1).asDouble();
    double brx = items->get(2).asDouble();
    double bry = items->get(3).asDouble();
    yInfo("got bounding Box is %lf %lf %lf %lf", tlx, tly, brx, bry);
    
    Bottle cmd,reply;
    cmd.addVocab(Vocab::encode("train"));
    Bottle &options=cmd.addList().addList();
    options.addString(label.c_str());
    
    options.add(bot.get(0));
    
    printf("Sending training request: %s\n",cmd.toString().c_str());
    rpcClassifier.write(cmd,reply);
    printf("Received reply: %s\n",reply.toString().c_str());
    
    return true;
}

/**********************************************************/
bool ActivityInterface::classifyObserve()
{
    ImageOf<PixelRgb> img= *imagePortIn.read(true);
    imgClassifier.write(img);
    
    bool answer;
    
    Bottle cmd,reply;
    cmd.addVocab(Vocab::encode("classify"));
    Bottle &options=cmd.addList();
    
    Bottle bot = *dispBlobRoi.read(true);
    
    for (int i=0; i<bot.size(); i++)
    {
        ostringstream tag;
        tag<<"blob_"<<i;
        Bottle &item=options.addList();
        item.addString(tag.str().c_str());
        item.addList()=*bot.get(i).asList();
    }
    
    printf("Sending classification request: %s\n",cmd.toString().c_str());
    rpcClassifier.write(cmd,reply);
    printf("Received reply: %s\n",reply.toString().c_str());
    
    string handStatus = processScores(reply);
    
    yInfo("the hand is %s", handStatus.c_str());
    
    if (strcmp (handStatus.c_str(),"full") == 0)
        answer = true;
    else
        answer = false;
    
    return answer;
}

/**********************************************************/
string ActivityInterface::processScores(const Bottle &scores)
{
    
    double maxScoreObj=0.0;
    string label  ="";
    
    for (int i=0; i<scores.size(); i++)
    {
        ostringstream tag;
        tag<<"blob_"<<i;

        Bottle *blobScores=scores.find(tag.str().c_str()).asList();
        if (blobScores==NULL)
            continue;
        
        for (int j=0; j<blobScores->size(); j++)
        {
            Bottle *item=blobScores->get(j).asList();
            if (item==NULL)
                continue;
            
            string name=item->get(0).asString().c_str();
            double score=item->get(1).asDouble();
            
            yInfo("name is %s with score %f", name.c_str(), score);
            
            if (score>maxScoreObj)
            {
                maxScoreObj = score;
                label.clear();
                label = name;
            }
            
        }
    }
    return label;
}

/**********************************************************/
bool ActivityInterface::gotSpike(const string &handName)
{
    bool report = handStat(handName);
    if (report && !inAction)
    {
        executeSpeech("what was that??");
        
        yInfo("something has changed in hand %s", handName.c_str());
        
        //do the take actions
        Bottle cmd, reply;
        cmd.clear(), reply.clear();
        cmd.addString("observe");
        cmd.addString(handName.c_str());
        rpcAREcmd.write(cmd, reply);
        
        string objName = holdIn(handName);
        
        if (classifyObserve())
        {
            yInfo("[gotSpike] holding a %s in hand %s", objName.c_str(), handName.c_str());
            
            string say = "Still have the " + objName + " in my hand";
            executeSpeech(say);
            
            //do the take actions
            Bottle cmd, reply;
            cmd.clear(), reply.clear();
            cmd.addString("home");
            cmd.addString("arms");
            cmd.addString("head");
            rpcAREcmd.write(cmd, reply);
            
        }else
        {
            string say = "I seem to have lost the " + objName;
            executeSpeech(say);
            
            for (std::map<string, string>::iterator it=inHandStatus.begin(); it!=inHandStatus.end(); ++it)
            {
                if (strcmp (it->first.c_str(), objName.c_str() ) == 0)
                {
                    inHandStatus.erase(objName.c_str());
                    break;
                }
            }
            
            Bottle cmd, reply;
            cmd.clear(), reply.clear();
            cmd.addString("home");
            cmd.addString("all");
            rpcAREcmd.write(cmd, reply);
        }
    }
    return true;
}

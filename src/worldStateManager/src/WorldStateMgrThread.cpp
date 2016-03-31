/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2016 VisLab, Institute for Systems and Robotics,
 *                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Giovanni Saponaro <gsaponaro@isr.ist.utl.pt>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 */

#include "WorldStateMgrThread.h"

/**********************************************************/
WorldStateMgrThread::WorldStateMgrThread(
    const string &_moduleName,
    ResourceFinder &_rf)
    : RateThread(33), // [ms]
      moduleName(_moduleName),
      rf(_rf)
{
}

/**********************************************************/
bool WorldStateMgrThread::openPorts()
{
    bool ret = true;

    opcPortName = "/" + moduleName + "/opc:io";
    ret = ret && opcPort.open(opcPortName.c_str());

    inTargetsPortName = "/" + moduleName + "/target:i";
    ret = ret && inTargetsPort.open(inTargetsPortName.c_str());

    inAffPortName = "/" + moduleName + "/affDescriptor:i";
    ret = ret && inAffPort.open(inAffPortName.c_str());

    inToolAffPortName = "/" + moduleName + "/toolAffDescriptor:i";
    ret = ret && inToolAffPort.open(inToolAffPortName.c_str());

    activityPortName = "/" + moduleName + "/activity:rpc";
    ret = ret && activityPort.open(activityPortName.c_str());

    trackerPortName = "/" + moduleName + "/tracker:rpc";
    ret = ret && trackerPort.open(trackerPortName.c_str());

    return ret;
}

/**********************************************************/
void WorldStateMgrThread::close()
{
    yInfo("closing ports");

    opcPort.close();
    inTargetsPort.close();
    inAffPort.close();
    inToolAffPort.close();
    activityPort.close();
    trackerPort.close();
}

/**********************************************************/
void WorldStateMgrThread::interrupt()
{
    closing = true;

    yInfo("interrupting ports");
    opcPort.interrupt();
    inTargetsPort.interrupt();
    inAffPort.interrupt();
    inToolAffPort.interrupt();
    activityPort.interrupt();
    trackerPort.interrupt();
}

/**********************************************************/
bool WorldStateMgrThread::threadInit()
{
    closing = false;

    // ports
    if ( !openPorts() )
    {
        yError("problem opening ports");
        return false;
    }

    // user-specified parameters
    countFrom   = rf.check("countFrom", Value(13)).asInt();
    withFilter  = rf.check("filter") && rf.find("filter").asString()!="off";
    if (withFilter)
    {
        filterOrder = rf.check("filterOrder", Value(5)).asInt();
        setFilterOrder(filterOrder);
        yInfo("selected temporal filtering with order %d", filterOrder);
    }

    // variables
    if (! initVars() )
    {
        yError("problem initializing variables");
        return false;
    }

    return true;
}

/**********************************************************/
void WorldStateMgrThread::run()
{
    while (!closing)
    {
        /*
        // update internal memory all the time
        // TODO: call it also without filter
        if (withFilter)
        {
            updateMemoryFilters();
            yarp::os::Time::delay(1.0);
        }
        */

        // enter perception state machine
        fsmPerception();

        yarp::os::Time::delay(0.01);
    }
}

/**********************************************************/
bool WorldStateMgrThread::initVars()
{
    fsmState = STATE_PERCEPTION_WAIT_OPC;
    toldUserOPCConnected = false;
    initFinished = false;
    t = yarp::os::Time::now();
    inAff = NULL;
    inToolAff = NULL;
    inTargets = NULL;
    needTrackerInit = false;
    needUpdate = false;
    toldUserBlobsConnected = false;
    toldUserTrackerConnected = false;
    toldActivityGoHome = false;
    toldUserActivityIFConnected = false;

    return true;
}

/**********************************************************/
bool WorldStateMgrThread::clearAll()
{
    // reset variables
    toldUserOPCConnected = false;
    needTrackerInit = false;
    initFinished = false;

    // activityInterface automatically detects unstacking events for the
    // "sabotage" scenario - no need to manually reset the whole stack
    // in memory. it's better not to do it at all and leave it to activityIF,
    // so that one can transition from an experiment to the next one (with
    // possibly a partial stack inherited from the previous experiment).
    //
    // reset activityInterface's objects stack (full stack memory)
    //if (activityPort.getOutputCount()>=1)
    //{
    //    Bottle activityCmd, activityReply;
    //    activityCmd.addString("resetObjStack");
    //    yInfo() << "sending activityInterface instruction:" << activityCmd.toString().c_str();
    //    activityPort.write(activityCmd, activityReply); // reply is always "ok"
    //}

    // reset activeParticleTracker
    resetTracker();

    // reset internal short-term memory
    hands.clear(); // empty hands container
    initMemoryFromOPC(); // put left and right entries into hands container
    objs.clear();
    trackIDs.clear();
    candidateTrackMap.clear();

    return true;
}

/**********************************************************/
bool WorldStateMgrThread::tellUserConnectOPC()
{
    double t0 = yarp::os::Time::now();
    if (t0-t>10.0 && opcPort.getOutputCount()<1)
    {
        yInfo("waiting for connection: %s /wsopc/rpc", opcPortName.c_str());
        t = t0;
    }

    return true;
}

/**********************************************************/
bool WorldStateMgrThread::tellUserOPCConnected()
{
    if (opcPort.getOutputCount()<1)
        return false;

    if (!toldUserOPCConnected)
    {
        yInfo("connected to WSOPC, you can now send RPC commands to /%s/rpc:i", moduleName.c_str());
        toldUserOPCConnected = true;
    }

    return true;
}

/**********************************************************/
bool WorldStateMgrThread::tellUserConnectBlobs()
{
    double t0 = yarp::os::Time::now();
    if (t0-t>10.0)
    {
        if (inAffPort.getInputCount()<1 || inToolAffPort.getInputCount()<1)
        {
            yInfo("waiting for connections:");
            yInfo("/blobDescriptor/affDescriptor:o %s", inAffPortName.c_str());
            yInfo("/blobDescriptor/toolAffDescriptor:o %s", inToolAffPortName.c_str());
            t = t0;
        }
    }

    return true;
}

/**********************************************************/
bool WorldStateMgrThread::tellUserBlobsConnected()
{
    if (inAffPort.getInputCount()<1 || inToolAffPort.getInputCount()<1)
        return false;

    if (!toldUserBlobsConnected)
    {
        yInfo("connected to BlobDescriptor, waiting for shape data - requires segmentation");
        toldUserBlobsConnected = true;
    }

    return true;
}

/**********************************************************/
bool WorldStateMgrThread::tellUserConnectTracker()
{
    double t0 = yarp::os::Time::now();
    if (t0-t>10.0 && trackerPort.getOutputCount()<1)
    {
        yInfo("waiting for connection: /activeParticleTrack/target:o %s",
              inTargetsPortName.c_str());
        t = t0;
    }

    return true;
}

/**********************************************************/
bool WorldStateMgrThread::tellUserConnectActivityIF()
{
    double t0 = yarp::os::Time::now();
    if (t0-t>10.0 && activityPort.getOutputCount()<1)
    {
        yInfo("waiting for connection: %s /activityInterface/rpc:i",
              activityPortName.c_str());
        t = t0;
    }

    return true;
}

/**********************************************************/
bool WorldStateMgrThread::opcContainsID(const int &id)
{
    if (opcPort.getOutputCount()<1)
        return false;

    // [get] ("id" <num>)
    Bottle opcCmd, opcCmdContent, opcReply;
    opcCmd.addVocab(Vocab::encode("get"));
    opcCmdContent.addString("id");
    opcCmdContent.addInt(id);
    opcCmd.addList() = opcCmdContent;
    //yDebug() << __func__ << "sending query:" << opcCmd.toString().c_str();
    opcPort.write(opcCmd, opcReply);
    //yDebug() << __func__ << "obtained response:" << opcReply.toString().c_str();

    return opcReply.get(0).asVocab()==Vocab::encode("ack");
}

/**********************************************************/
bool WorldStateMgrThread::checkOPCStatus(const int &minEntries, Bottle &ids)
{
    // this function returns true/false if WSOPC already
    // contains at least minEntries entries (hands + objects on the table).
    // it also returns (as a parameter) the current OPC IDs.

    if (opcPort.getOutputCount()<1)
        return false;

    // query: [ask] (all)
    Bottle opcCmd, opcCmdContent, opcReply;
    opcCmd.addVocab(Vocab::encode("ask"));
    opcCmdContent.addString("all");
    opcCmd.addList() = opcCmdContent;
    opcPort.write(opcCmd, opcReply);

    // reply: [ack] (id (11 12 13 ...))
    bool init = false;
    init = opcReply.size()>1 &&
           opcReply.get(0).asVocab()==Vocab::encode("ack") &&
           opcReply.get(1).isList() &&
           opcReply.get(1).asList()->size()>0 &&
           opcReply.get(1).asList()->get(0).asString()=="id" &
           opcReply.get(1).asList()->get(1).isList() &&
           opcReply.get(1).asList()->get(1).asList()->size()>=minEntries;

    // return the current OPC IDs in the ids variable
    if (init)
    {
        ids = * opcReply.get(1).asList()->get(1).asList();
        //yDebug("%s: ids has size %d and content %s", __func__, ids.size(), ids.toString().c_str());
    }

    return init;
}

/**********************************************************/
bool WorldStateMgrThread::resetOPC()
{
    if (opcPort.getOutputCount()<1)
        return false;

    // 1. get current IDs in the database
    Bottle opcIDs;
    const int numHandEntries = 2;
    if (!checkOPCStatus(numHandEntries,opcIDs))
    {
        yWarning("%s problem verifying that WSOPC has at least %d entries",
                 __func__, numHandEntries);
        return false;
    }

    // increase countFrom (activeParticleTracker start index) to account for
    // the fact that some IDs will be deleted from WSOPC (never to be used again)
    // and the tracker will need to start counting from a fresh ID in order to
    // make the world state and planner happy.
    increaseCountFrom();

    // cycle over opcIDs entries (hands and objects)
    const int LeftHandID = 11;
    const int RightHandID = 12;
    for (int entryIdx=0; entryIdx<opcIDs.size(); ++entryIdx)
    {
        // hands or objects distinction
        if ((opcIDs.get(entryIdx).asInt() == LeftHandID) ||
            (opcIDs.get(entryIdx).asInt() == RightHandID))
        {
            // 2. hand entries: keep them (need to preserve IDs), just reset the fields
            std::string handName = id2label(opcIDs.get(entryIdx).asInt());
            yInfo("resetting fields of hand entry %d/%s",
                  opcIDs.get(entryIdx).asInt(), handName.c_str());
            if (!resetOPCHandFields(opcIDs.get(entryIdx).asInt()))
            {
                yError("%s problem resetting fields of hand entry %d/%s",
                         __func__, opcIDs.get(entryIdx).asInt(), handName.c_str());
                return false;
            }
        }
        else
        {
            // 3. object entries: first check if object must be protected from deletion
            //    (i.e., when it is in a stack or in a robot hand), then act accordingly
            std::string label = id2label(opcIDs.get(entryIdx).asInt());

            bool isStacked;
            if (!belongsToStack(label, isStacked))
            {
                yWarning("%s %d/%s: problem with belongsToStack",
                         __func__, opcIDs.get(entryIdx).asInt(), label.c_str());
            }

            bool isGrasped;
            isInHand(label, isGrasped);

            // if true, protect this entry from deletion
            bool keep = isStacked || isGrasped;

            // object protected vs non-protected distinction
            if (keep)
            {
                // protected cases
                if (isStacked)
                {
                    yInfo("not deleting object entry %d/%s because it is under a stack",
                          opcIDs.get(entryIdx).asInt(), label.c_str());
                }

                if (isGrasped)
                {
                    yInfo("not deleting object entry %d/%s because it is in a robot hand",
                          opcIDs.get(entryIdx).asInt(), label.c_str());
                }
            }
            else
            {
                // not protected cases -> delete
                // (it is ok to change the ID from one
                // experiment to the other -- the only restriction is that an <object,ID>
                // pair is fixed within the same experiment)

                yInfo("deleting object entry %d/%s",
                      opcIDs.get(entryIdx).asInt(), label.c_str());
                // query: [del] (("id" <num>))
                Bottle opcCmd, opcCmdContent, opcReply;
                opcCmd.clear();
                opcCmdContent.clear();
                opcReply.clear();
                opcCmd.addVocab(Vocab::encode("del"));
                opcCmdContent.addString("id");
                opcCmdContent.addInt(opcIDs.get(entryIdx).asInt());
                opcCmd.addList() = opcCmdContent;

                //yDebug() << __func__ << "sending query:" << opcCmd.toString().c_str();
                opcPort.write(opcCmd, opcReply);
                //yDebug() << __func__ << "obtained response:" << opcReply.toString().c_str();

                bool validResponse = opcReply.size()>0 &&
                                     opcReply.get(0).asVocab()==Vocab::encode("ack");

                if (!validResponse)
                {
                    yError("%s problem deleting object entry %d/%s",
                             __func__, opcIDs.get(entryIdx).asInt(), label.c_str());
                    return false;
                }
            } // end object protected vs non-protected distinction
        } // end hands or objects distinction
    } // end cycle over opcIDs entries

    yInfo() << "successfully reset WSOPC -"
            << "hand entries had their fields reset (with IDs kept),"
            << "visible objects entries were deleted (objects under a stack or currently grasped were not deleted)";

    return true;
}

/**********************************************************/
bool WorldStateMgrThread::resetOPCHandFields(const int &handID)
{
    // reset the field is_free in the WSOPC database entry corresponding to
    // handID. the other field is_hand is already correct and static.

    if (opcPort.getOutputCount()<1)
        return false;

    const int LeftHandID = 11;
    const int RightHandID = 12;
    if ((handID != LeftHandID) && (handID != RightHandID))
    {
        yError("%s called with a non-hand ID argument", __func__);
        return false;
    }
    const string handName = (handID==LeftHandID ? "left" : "right");

    Bottle opcCmd;
    Bottle opcCmdContent;
    Bottle opcReply;

    // query: [set] (("id" <num>) ("is_free" "true"))
    opcCmd.clear();
    opcCmdContent.clear();
    opcReply.clear();
    opcCmd.addVocab(Vocab::encode("set"));

    Bottle bID;
    bID.clear();
    bID.addString("id");
    bID.addInt(handID);
    opcCmdContent.addList() = bID;

    Bottle bIsFree;
    bIsFree.addString("is_free");
    bool newIsFreeValue = isHandFree(handName);
    bIsFree.addString( BoolToString(newIsFreeValue) );
    opcCmdContent.addList() = bIsFree;
    opcCmd.addList() = opcCmdContent;

    //yDebug() << __func__ << "sending command to WSOPC:" << opcCmd.toString().c_str();
    opcPort.write(opcCmd, opcReply);
    //yDebug() << __func__ << "received response:" << opcReply.toString().c_str();

    // process WSOPC response
    bool validResponse = opcReply.size()>0 &&
                         opcReply.get(0).asVocab()==Vocab::encode("ack");
    if (!validResponse)
    {
        yWarning("%s when resetting field %s of %d did not receive valid response from WSOPC",
                 __func__, "is_free", handID);
        return false;
    }

    return true;
}

/**********************************************************/
bool WorldStateMgrThread::increaseCountFrom()
{
    if (opcPort.getOutputCount()<1)
        return false;

    Bottle opcIDs;
    if (!checkOPCStatus(2,opcIDs))
    {
        yWarning("problem verifying that WSOPC has at least 2 entries");
        return false;
    }

    // assumption: querying OPC with: ask (all)
    // we obtain a list of IDs in ascending order, like this:
    // >>ask (all)
    // Response: [ack] (id (11 12 23 24 25 26 27))
    // therefore to obtain the maximum we can just get the last value
    const int maxOldID = opcIDs.get(opcIDs.size()-1).asInt();

    countFrom = maxOldID+1;
    yInfo("reset routine: increased countFrom to %d", countFrom);

    return true;
}

/**********************************************************/
bool WorldStateMgrThread::refreshBlobs()
{
    if (inAffPort.getInputCount()<1 || inToolAffPort.getInputCount()<1)
        return false;

    // update whole object descriptors
    inAff = inAffPort.read();

    // update object parts descriptors
    inToolAff = inToolAffPort.read();

    if (inAff!=NULL && inToolAff!=NULL)
    {
        // number of blobs
        sizeAff = static_cast<int>( inAff->get(0).asDouble() );

        // BlobDescriptor should prevent this to happen, but just in case:
        if ( static_cast<int>(inToolAff->get(0).asDouble()) != sizeAff )
        {
            yWarning("number of whole object descriptors differ from number of object parts descriptors!");
        }
    }

    return true;
}

/**********************************************************/
bool WorldStateMgrThread::checkTrackerStatus()
{
    // this function returns true/false if activeParticleTrack is already
    // tracking or not. additionally, it updates the trackIDs variable.

    if (trackerPort.getOutputCount()<1)
        return false;

    Bottle trackerCmd, trackerReply;
    trackerCmd.addString("getIDs");
    //yDebug() << __func__ <<  "sending query to tracker:" << trackerCmd.toString().c_str();
    trackerPort.write(trackerCmd, trackerReply);
    //yDebug() << __func__ <<  "obtained response:" << trackerReply.toString().c_str();

    bool notNullList = trackerReply.size()>0 &&
                       trackerReply.get(0).isList() &&
                       trackerReply.get(0).asList()->size()>0;

    // update trackIDs - includes paused IDs
    if (notNullList)
        trackIDs = * trackerReply.get(0).asList();

    return notNullList;
}

/**********************************************************/
bool WorldStateMgrThread::configureTracker()
{
    if (trackerPort.getOutputCount()<1)
        return false;

    if (!toldUserTrackerConnected)
    {
        yInfo("connected to tracker");
        toldUserTrackerConnected = true;
    }

    // if tracker already initialized -> reset it
    if (checkTrackerStatus())
    {
        yInfo("tracker was already initialized from a previous experiment -> will reset it and start again with index countFrom %d",
              countFrom);

        // reset
        yInfo("now resetting tracker...");
        if (!resetTracker())
            yWarning("problem resetting tracker");
    }

    // now send countFrom rpc instruction
    yInfo("configuring tracker with instruction: countFrom %d", countFrom);
    Bottle trackerCmd, trackerReply;
    trackerCmd.addString("countFrom");
    trackerCmd.addInt(countFrom);
    //yDebug() << __func__ <<  "sending query to tracker:" << trackerCmd.toString().c_str();
    trackerPort.write(trackerCmd, trackerReply);
    //yDebug() << __func__ <<  "obtained response:" << trackerReply.toString().c_str();
    bool validResponse = false;
    validResponse = trackerReply.size()>0 &&
                    trackerReply.get(0).asVocab()==Vocab::encode("ok");
    if (!validResponse)
    {
        yWarning() << __func__ << "obtained invalid response from tracker:" << trackerReply.toString().c_str();
        return false;
    }

    return true;
}

/**********************************************************/
bool WorldStateMgrThread::getTrackNames()
{
    // note: when this function fails (returns false),
    //       candidateTrackMap must be cleared

    if (activityPort.getOutputCount()<1)
        return false;

    // update trackIDs
    if (!checkTrackerStatus())
    {
        yWarning() << __func__ << "problem with tracker";
        return false;
    }

    // cycle over items, acquire candidate names
    bool allCandidateNamesUnique = true;
    for (int i=0; i<trackIDs.size(); i++)
    {
        int id = trackIDs.get(i).asInt();

        if (i==0) yInfo("asking for object labels of visible objects");

        // get fresh blob coordinates
        if (!refreshTracker())
        {
            yWarning() << __func__ << "problem with refreshTracker";
            return false;
        }

        bool safetyCheck = false;
        safetyCheck = inTargets != NULL &&
            inTargets->get(i).isList() &&
            inTargets->get(i).asList()->size()>0; // expected size 8
        if (!safetyCheck)
        {
            yWarning() << __func__ <<
                "problem with tracker, probably one or more objects disappeared." <<
                "try restarting activeParticleTrack.";
            return false;
        }

        // TODO: make sure desired id corresponds to get(i)
        int u = static_cast<int>( inTargets->get(i).asList()->get(1).asDouble() );
        int v = static_cast<int>( inTargets->get(i).asList()->get(2).asDouble() );
        string label;
        if (!getLabelMajorityVote(u, v, label))
        {   
            // failed acquiring not-null label -> try again
            yWarning() << __func__ << "problem with getLabelMajorityVote";
            return false;
        }

        // fail if this already exists in memory or in candidateTrackMap
        if ( memoryContainsID(id) ||
             memoryContainsName(label) ||
             mapContainsKey(candidateTrackMap,id) ||
             mapContainsValue(candidateTrackMap,label) )
        {
            yWarning("winning label --> %s <--, however not going to add it to memory because it seems to be a duplicate",
                     label.c_str());
            allCandidateNamesUnique = false;
            continue;
        }

        // assume names cannot change -> use insert(), not operator[]
        // http://stackoverflow.com/questions/326062/in-stl-maps-is-it-better-to-use-mapinsert-than
        candidateTrackMap.insert(make_pair(id,label));

        if (i==trackIDs.size()-1) yInfo("done asking for visible object labels");
    }

    if (allCandidateNamesUnique)
    {
        yInfo("success, all candidate object IDs and names are unique:");
        dumpMap(candidateTrackMap);
    }
    else
    {
        // failed initializing unique ID-name map -> clear and try again
        return false;
    }

    return true;
}

/**********************************************************/
bool WorldStateMgrThread::initTracker()
{
    if (trackerPort.getOutputCount()<1)
        return false;

    if (checkTrackerStatus())
    {
        yDebug("tracker already initialized, will not do it again!");
        return false;
    }

    // get fresh coordinates from segmentation/blobDesc
    refreshBlobs();

    yInfo("initializing tracking of %d objects:", sizeAff);
    double u=0.0, v=0.0;

    for(int a=0; a<sizeAff; a++)
    {
        u = inAff->get(a+1).asList()->get(0).asDouble();
        v = inAff->get(a+1).asList()->get(1).asDouble();

        Bottle trackerCmd;
        Bottle trackerReply;

        trackerCmd.addString("track");
        trackerCmd.addDouble(u);
        trackerCmd.addDouble(v);
        //yDebug() << __func__ <<  "sending fixation request to tracker:" << trackerCmd.toString().c_str();
        trackerPort.write(trackerCmd,trackerReply);
        //yDebug() << __func__ <<  "obtained response:" << trackerReply.toString().c_str();

        if (trackerReply.size()>0 && trackerReply.get(0).isInt() && trackerReply.get(0).asInt()!=-1)
            yInfo("%f %f -> id %d", u, v, trackerReply.get(0).asInt());
        else
            yWarning("problem initializing tracker with fixation %f %f, got invalid response %s, was expecting an integer such as %d",
                u, v, trackerReply.toString().c_str(), countFrom+a);
    }

    return true;
}

/**********************************************************/
bool WorldStateMgrThread::refreshTracker()
{
    if (trackerPort.getOutputCount()<1)
    {
        yWarning() << __func__ << "not connected to tracker";
        return false;
    }

    // compute pausedIDs Bottle with "getPausedIDs" rpc
    Bottle pausedIDs;
    Bottle trackerCmd, trackerReply;
    trackerCmd.addString("getPausedIDs");
    //yDebug() << __func__ <<  "sending query to tracker:" << trackerCmd.toString().c_str();
    trackerPort.write(trackerCmd, trackerReply);
    //yDebug() << __func__ <<  "obtained response:" << trackerReply.toString().c_str();
    bool validReply = trackerReply.size()>0 &&
                      trackerReply.get(0).isList();
    if (validReply)
        pausedIDs = * trackerReply.get(0).asList();

    // update number of currently visible tracks: num. all tracks minus num. paused
    sizeTargets = trackIDs.size() - pausedIDs.size();
    //yDebug("sizeTargets=%d", sizeTargets);

    if (sizeTargets > 0)
    {
        // default case: there are visible tracks -> blocking read
        inTargets = inTargetsPort.read(true);
    }
    else
    {
        // no visible tracks (all tracks paused) -> a few non-blocking reads
        const int numTries = 5;
        for (int t=0; t<numTries; ++t)
        {
            inTargets = inTargetsPort.read(false);
            yarp::os::Time::delay(0.1);
            if (inTargets != NULL)
                break;
        }
        if (inTargets == NULL)
        {
            yWarning("%s did not receive data from tracker after %d tries",
                     __func__, numTries);
            return false;
        }
    }

    return true;
}

/**********************************************************/
bool WorldStateMgrThread::resetTracker()
{
    if (trackerPort.getOutputCount()<1)
    {
        yWarning() << __func__ << "not connected to tracker";
        return false;
    }

    Bottle trackerCmd, trackerReply;

    // due to https://github.com/robotology/poeticon/issues/186
    // resume all paused tracks first (all paused tracks that are valid i.e.
    // that are currently in "getIDs") before doing activeParticleTrack "reset".
    // now the issue has been solved, but this workaround is harmless, leaving it for now.
    Bottle pausedIDs;
    trackerCmd.addString("getPausedIDs");
    //yDebug() << __func__ <<  "sending instruction to activeParticleTracker:" << trackerCmd.toString().c_str();
    trackerPort.write(trackerCmd, trackerReply);
    //yDebug() << __func__ <<  "obtained response:" << trackerReply.toString().c_str();
    bool gotPausedTracks = trackerReply.size()>0 &&
                           trackerReply.get(0).isList() &&
                           trackerReply.get(0).asList()->size()>0;
    if (gotPausedTracks)
    {
        Bottle pausedIDs = *trackerReply.get(0).asList();
        yDebug("pausedIDs: %s", pausedIDs.toString().c_str());
        for (int p=0; p<pausedIDs.size(); ++p)
        {
            const int id = pausedIDs.get(p).asInt();
            // validity test for this paused ID (check if it is present in "getIDs")
            if (ensureTrackerHasID(id))
            {
                yDebug("%s resuming valid track %d before tracker reset", __func__, id);
                resumeTrackID(id);
            }
            else
                yDebug("%s not resuming invalid track %d before tracker reset", __func__, id);
        }
    }

    trackerCmd.clear();
    trackerReply.clear();
    trackerCmd.addString("reset");
    trackerPort.write(trackerCmd, trackerReply);
    yInfo() << __func__ <<  "sending instruction to activeParticleTracker:" << trackerCmd.toString().c_str();
    bool validResponse = false;
    validResponse = trackerReply.size()>0 &&
                    trackerReply.get(0).asVocab()==Vocab::encode("ok");
    if (!validResponse)
    {
        yWarning() << __func__ <<  "obtained invalid response:" << trackerReply.toString().c_str();
        return false;
    }

    return true;
}

/**********************************************************/
bool WorldStateMgrThread::ensureTrackerHasID(const int &id)
{
    if (trackerPort.getOutputCount()<1)
    {
        yWarning() << __func__ << "not connected to tracker";
        return false;
    }

    // refresh trackIDs Bottle
    if (!checkTrackerStatus())
    {
        yWarning() << __func__ << "problem refreshing tracker and trackIDs";
        return false;
    }

    if (trackIDs.size()>0)
    {
        for (int idx=0; idx<trackIDs.size(); ++idx)
        {
            if (trackIDs.get(idx).asInt()==id)
            {
                //yDebug() << __func__ << "found ID" << id << "in trackIDs";
                return true;
            }
        }
    }

    yWarning() << __func__ << "did not find ID" << id << "in trackIDs";
    return false;
}

/**********************************************************/
bool WorldStateMgrThread::getAffBottleIndexFromTrackROI(const int &u, const int &v, int &abi)
{
    // Finds the AffBottleIndex of inAff->get(abi) corresponding to
    // the TrackerBottleIndex inTargets->get(tbi).
    //
    // Current implementation: from tracker ROI centre, detect the most likely
    // blob index within blobDescriptor Bottle, using Euclidean distance. 
    // It iterates over pairs
    //bPosValue.addDouble(inAff->get(*+1).asList()->get(0).asDouble());
    //bPosValue.addDouble(inAff->get(*+1).asList()->get(1).asDouble());
    // and return the closest one (in Euclidean distance sense) to u,v.
    //
    // Possible, more robust alternative: cv::pointPolygonTest.

    yarp::sig::Vector trackerROI(2, 0.0);
    trackerROI[0] = static_cast<double>( u );
    trackerROI[1] = static_cast<double>( v );

    float minDist    = 1000.0; // minimum distance found so far
    int minBlobIdx   =   -1;   // index of minimum distance (argmin)
    float maxDistThr =   50.0; // threshold of maximum distance for ABI update

    for(int a=1; a<=sizeAff; a++)
    {
        yarp::sig::Vector currentBlob(2, 0.0);
        currentBlob[0] = inAff->get(a).asList()->get(0).asDouble();
        currentBlob[1] = inAff->get(a).asList()->get(1).asDouble();
        float dist = 0.0;
        euclideanDistance(trackerROI, currentBlob, dist);

        //yDebug() << "currentBlob" << a << currentBlob.toString().c_str();
        //yDebug() << "dist" << dist;

        if (dist<maxDistThr && dist<minDist)
        {
            minBlobIdx = a;
            minDist = dist;
        }
    }
    //yDebug("winner %d (dist=%f)", minBlobIdx, minDist);

    // it nothing was found
    if (minBlobIdx == -1)
    {
        yWarning() << __func__ << "did not find a shape descriptor blob"
                   << "corresponding to the tracker blob with coordinates"
                   << u << v;
        return false;
    }

    abi = minBlobIdx;
    return true;
}

/**********************************************************/
bool WorldStateMgrThread::getTrackerBottleIndexFromID(const int &id, int &tbi)
{
    // no current tracks (or all tracks paused)
    if (inTargets == NULL)
        return false;

    // we assume that activeParticleTrack is streaming a Bottle with ordered IDs:
    // ((13 ...) (14 ...) (15 ...))
    // TODO: verify that this is the case, else reorder

    // compare first element of each sublist with searched id
    for (int currBottleIdx = 0;
         currBottleIdx < inTargets->size();
         ++currBottleIdx)
    {
        if (inTargets->get(currBottleIdx).asList()->get(0).asInt() == id)
        {
            // found
            tbi = currBottleIdx;
            //yDebug() << __func__ << "found track" << id << "at Bottle index" << tbi;
            return true;
        }
    }

    //yDebug() << __func__ << "did not find track id" << id <<
    //         "in Bottle: likely this object stopped being tracked";
    return false;
}

/**********************************************************/
bool WorldStateMgrThread::computeObjProperties(const int &id, const string &label,
                                               Bottle &pos2d,
                                               Bottle &desc2d, Bottle &tooldesc2d,
                                               string &inHand,
                                               Bottle &onTopOf,
                                               Bottle &reachW, Bottle &pullW)
{
    //yDebug("%s begin %d/%s", __func__, id, label.c_str());

    if (activityPort.getOutputCount()<1)
        return false;

    /*
    bool visibleByActivityIF;
    if (!getVisibilityByActivityIF(label, visibleByActivityIF))
    {
        yWarning("%s %d/%s: problem with getVisibilityByActivityIF",
                 __func__, id, label.c_str());
    }

    bool isStacked;
    if (!belongsToStack(label, isStacked))
    {
        yWarning("%s %d/%s: problem with belongsToStack",
                 __func__, id, label.c_str());
    }

    // distinguish between geometric changes (invisible but stacked/occluded)
    // and semantic changes (invisible and disappeared from scene)
    if (!visibleByActivityIF)
    {
        if (isStacked)
        {
            yInfo("detected a GEOMETRIC change in the world: %d/%s is not visible because it is below something else",
                  id, label.c_str());
        }
        else
        {
            yInfo("detected a SEMANTIC change in the world: %d/%s disappeared from the scene",
                  id, label.c_str());
        }
    }
    */

    // by default the object is tracked and we can compute all symbols
    bool visibleByTracker = true;

    // if possible, refresh inTargets Bottle
    if (trackerPort.getOutputCount()>0)
        refreshTracker();

    // find the "tracker Bottle index" within inTargets Bottle that matches id
    int tbi = -1;
    if (!getTrackerBottleIndexFromID(id,tbi))
    {
        //yDebug() << __func__ << "did not find track id" << id
        //         << "in tracker Bottle: object not visible ->"
        //         << "going to update activityInterface symbols only,"
        //         << "leaving shape descriptors untouched";
        visibleByTracker = false;
    }

    // find the "affordance Bottle index" within inAff Bottle that matches
    // with the selected tracker blob
    double u=0.0, v=0.0;
    if (tbi != -1)
    {
        //yDebug("%s start reading coordinates of %d/%s", __func__, id, label.c_str());
        u = inTargets->get(tbi).asList()->get(1).asDouble();
        v = inTargets->get(tbi).asList()->get(2).asDouble();
        //yDebug("%s finish reading coordinates of %d/%s", __func__, id, label.c_str());
    }
    int abi = -1; // affordance blobs Bottle index
    if (visibleByTracker)
    {
        if (!getAffBottleIndexFromTrackROI(u,v,abi))
        {
            yDebug() << __func__
                << "did not find affordance blob index from coordinates"
                << u << v << "-> going to update activityInterface symbols only,"
                << "leaving shape descriptors untouched";
            visibleByTracker = false;
        }
    }

    //yDebug("%s %d/%s: visibleByActivityIF=%s; trackerBottleIndex=%d affordanceBottleIndex=%d visibleByTracker=%s",
    //       __func__, id, label.c_str(), BoolToString(visibleByActivityIF), tbi, abi, BoolToString(visibleByTracker));

    yDebug("%s %d/%s: trackerBottleIndex=%d affordanceBottleIndex=%d visibleByTracker=%s",
           __func__, id, label.c_str(), tbi, abi, BoolToString(visibleByTracker));

    // now we know that object was found in both tracker and shape descriptors
    if (visibleByTracker)
    {
        // begin symbols that depend on tracker and shape descriptors

        // prepare position property (pos2d)
        pos2d.clear();
        pos2d.addDouble(u); // unfiltered values
        pos2d.addDouble(v);

        // prepare 2D shape descriptors property (desc2d)
        bool validDesc = false;
        validDesc = inAff != NULL &&
                    abi >= 1 &&
                    abi <= sizeAff &&
                    inAff->get(abi).isList() &&
                    inAff->get(abi).asList()->size() > 0;
        const int areaIdx = 23;
        if (validDesc)
        {
            desc2d.clear();
            desc2d.addDouble(inAff->get(abi).asList()->get(areaIdx).asDouble()); // area
            desc2d.addDouble(inAff->get(abi).asList()->get(areaIdx+1).asDouble()); // conv
            desc2d.addDouble(inAff->get(abi).asList()->get(areaIdx+2).asDouble()); // ecc
            desc2d.addDouble(inAff->get(abi).asList()->get(areaIdx+3).asDouble()); // comp
            desc2d.addDouble(inAff->get(abi).asList()->get(areaIdx+4).asDouble()); // circ
            desc2d.addDouble(inAff->get(abi).asList()->get(areaIdx+5).asDouble()); // sq
        }
        else
            yWarning("problem reading descriptors of whole object");

        // prepare 2D tool-object parts shape desc. property (tooldesc2d)
        bool validToolDesc = false;
        validToolDesc = inToolAff != NULL &&
            abi >= 1 &&
            abi <= sizeAff &&
            inToolAff->get(abi).asList()->size() == 2 &&
            inToolAff->get(abi).asList()->get(0).isList() &&
            inToolAff->get(abi).asList()->get(1).isList() &&
            inToolAff->get(abi).asList()->get(0).asList()->size() > 0 && // TODO: actual size
            inToolAff->get(abi).asList()->get(1).asList()->size() > 0; // TODO: actual size
        if (validToolDesc)
        {
            tooldesc2d.clear();
            // add top half info
            Bottle &topBot = tooldesc2d.addList();
            topBot.clear();
            topBot.addDouble(inToolAff->get(abi).asList()->get(0).asList()->get(0).asDouble()); // x
            topBot.addDouble(inToolAff->get(abi).asList()->get(0).asList()->get(1).asDouble()); // y
            topBot.addDouble(inToolAff->get(abi).asList()->get(0).asList()->get(2).asDouble()); // ar
            topBot.addDouble(inToolAff->get(abi).asList()->get(0).asList()->get(3).asDouble()); // con
            topBot.addDouble(inToolAff->get(abi).asList()->get(0).asList()->get(4).asDouble()); // ecc
            topBot.addDouble(inToolAff->get(abi).asList()->get(0).asList()->get(5).asDouble()); // com
            topBot.addDouble(inToolAff->get(abi).asList()->get(0).asList()->get(6).asDouble()); // cir
            topBot.addDouble(inToolAff->get(abi).asList()->get(0).asList()->get(7).asDouble()); // sq

            // add bottom half info
            Bottle &botBot = tooldesc2d.addList();
            botBot.clear();
            botBot.addDouble(inToolAff->get(abi).asList()->get(1).asList()->get(0).asDouble()); // x
            botBot.addDouble(inToolAff->get(abi).asList()->get(1).asList()->get(1).asDouble()); // y
            botBot.addDouble(inToolAff->get(abi).asList()->get(1).asList()->get(2).asDouble()); // ar
            botBot.addDouble(inToolAff->get(abi).asList()->get(1).asList()->get(3).asDouble()); // con
            botBot.addDouble(inToolAff->get(abi).asList()->get(1).asList()->get(4).asDouble()); // ecc
            botBot.addDouble(inToolAff->get(abi).asList()->get(1).asList()->get(5).asDouble()); // com
            botBot.addDouble(inToolAff->get(abi).asList()->get(1).asList()->get(6).asDouble()); // cir
            botBot.addDouble(inToolAff->get(abi).asList()->get(1).asList()->get(7).asDouble()); // sq
        }
        else
            yWarning("problem reading descriptors of object parts");

        // end symbols that depend on tracker and shape descriptors
    }

    // begin symbols that depend on activityInterface

    // prepare in_hand property (none/left/right)
    inHand = inWhichHand(label);

    // prepare on_top_of property
    onTopOf.clear();
    Bottle bLabelsBelow; // strings
    isOnTopOf(label, bLabelsBelow);
    for (int o=0; o<bLabelsBelow.size(); o++)
    {
        int foundID;
        foundID = label2id( bLabelsBelow.get(o).asString().c_str() );
        if (foundID != -1)
            onTopOf.addInt( foundID );
        else
            yWarning("problem translating element %s of on_top_of=%s to IDs",
                     bLabelsBelow.get(o).asString().c_str(),
                     bLabelsBelow.toString().c_str());
    }

    // prepare reachable_with property
    reachW.clear();
    Bottle bLabelsReaching; // strings
    isReachableWith(label, bLabelsReaching);
    for (int o=0; o<bLabelsReaching.size(); o++)
    {
        int foundID;
        foundID = label2id( bLabelsReaching.get(o).asString().c_str() );
        if (foundID != -1)
            reachW.addInt( foundID );
        else
            yWarning("problem translating element %s of reachable_with=%s to IDs",
                     bLabelsReaching.get(o).asString().c_str(),
                     bLabelsReaching.toString().c_str());
    }

    // prepare pullable_with property
    pullW.clear();
    Bottle bLabelsPulling; // strings
    isPullableWith(label, bLabelsPulling);
    for (int o=0; o<bLabelsPulling.size(); o++)
    {
        int foundID;
        foundID = label2id( bLabelsPulling.get(o).asString().c_str() );
        if (foundID != -1)
            pullW.addInt( foundID );
        else
            yWarning("problem translating element %s of pullable_with=%s to IDs",
                     bLabelsPulling.get(o).asString().c_str(),
                     bLabelsPulling.toString().c_str());
    }

    // end symbols that depend on activityInterface

    //yDebug("%s end %d/%s", __func__, id, label.c_str());

    return true;
}

/**********************************************************/
bool WorldStateMgrThread::constructMemoryFromMap()
{
    if (candidateTrackMap.empty())
    {
        yWarning() << __func__ << "candidateTrackMap empty";
        return false;
    }

    // for each <id,label> pair
    for (idLabelMap::const_iterator iter = candidateTrackMap.begin();
        iter != candidateTrackMap.end();
        ++iter)
    {
        // make sure item is not already present in memory containers
        if (memoryContainsID(iter->first) || memoryContainsName(iter->second))
        {
            yWarning("item already present in memory containers!");
            return false;
        }

        // construct item and add it to objs memory container
        Bottle pos2d;
        Bottle desc2d;
        Bottle tooldesc2d;
        string inHand;
        Bottle onTopOf;
        Bottle reachW;
        Bottle pullW;
        computeObjProperties(iter->first,iter->second,
                             pos2d,
                             desc2d,tooldesc2d,
                             inHand,
                             onTopOf,
                             reachW,pullW);
        objs.push_back(
            MemoryItemObj(iter->first,iter->second,false,
                          pos2d,
                          desc2d, tooldesc2d,
                          inHand,
                          onTopOf,
                          reachW,pullW) );
    }

    return true;
}

/**********************************************************/
bool WorldStateMgrThread::constructMemoryFromOPCID(const int &opcID)
{
    if (opcPort.getOutputCount()<1)
        return false;

    // query: [get] (id 11)
    Bottle opcCmd, opcCmdContent, opcReply;
    opcCmd.addVocab(Vocab::encode("get"));
    opcCmdContent.addString("id");
    opcCmdContent.addInt(opcID);
    opcCmd.addList() = opcCmdContent;
    opcPort.write(opcCmd, opcReply);

    // reply: [ack] ((is_hand true) (is_free true) (name left))
    bool validResponse = opcReply.size()>1 &&
                         opcReply.get(0).asVocab()==Vocab::encode("ack") &&
                         opcReply.get(1).isList();
    if (!validResponse)
    {
        yWarning() << __func__ << "cannot construct memory item" << opcID
                   << "because of invalid reply from WSOPC";
        return false;
    }

    Bottle *fields = opcReply.get(1).asList();

    // checks before adding this item to internal model: uniqueness, etc.

    bool hasMandatoryFields = fields->check("name") &&
                              fields->check("is_hand");
    if (!hasMandatoryFields)
    {
        yWarning() << __func__ << "cannot construct memory item" << opcID
                   << "because it lacks mandatory fields";
        return false;
    }

    if (memoryContainsID(opcID))
    {
        // note: this happens e.g. when WSOPC is started & initialized before WSM,
        // or when WSOPC has entries from a previous experiment that must be kept
        yInfo() << __func__ << "will not construct memory item"
                << opcID << "because this ID is already present in internal short-term memory";
        return false;
    }

    string name = fields->find("name").asString();
    if (name=="")
    {
        yWarning() << __func__ << "cannot construct memory item"
                   << opcID << "because its name field is empty";
        return false;
    }
    if (memoryContainsName(name))
    {
        // note: this happens e.g. when WSOPC is started & initialized before WSM
        //yDebug() << __func__ << "cannot construct memory item"
        //           << opcID << "with associated name" << name
        //           << "because this name is already present in internal short-term memory";
        return false;
    }

    if (fields->find("is_hand").asString()=="true") // TODO: use asBool() ?
    {
        // hand - parse remaining fields and add
        bool isFree;
        parseHandProperties(fields,isFree);
        hands.push_back(MemoryItemHand(opcID,name,true,isFree));
    }
    else
    {
        // object - parse remaining fields and add
        Bottle pos2d;
        Bottle desc2d;
        Bottle tooldesc2d;
        string inHand;
        Bottle onTopOf;
        Bottle reachW;
        Bottle pullW;
        parseObjProperties(fields,
                           pos2d,desc2d,tooldesc2d,inHand,onTopOf,reachW,pullW);
        objs.push_back(MemoryItemObj(opcID,name,false,
                       pos2d,desc2d,tooldesc2d,inHand,onTopOf,reachW,pullW));
    }

    return true;
}

/**********************************************************/
bool WorldStateMgrThread::initMemoryFromOPC()
{
    if (opcPort.getOutputCount()<1)
        return false;

    // make sure WSOPC contains at least 2 entries, and save them to opcIDs
    Bottle opcIDs;
    if (!checkOPCStatus(2,opcIDs))
    {
        yWarning("problem verifying that WSOPC has at least 2 entries");
        return false;
    }

    /*
    // if WSOPC contained more than 2 entries (hands), it means it has data from
    // a previous experiment -> increase countFrom index for tracker
    if (checkOPCStatus(3,opcIDs))
    {
        yDebug() << __func__ << "increasing countFrom...";
        increaseCountFrom();
    }

    // if WSOPC contained more than 2 entries (hands), it means it has data from
    // a previous experiment -> reset it
    if (opcIDs.size() >= 3)
    {
        yInfo("WSOPC has some object entries from a previous experiment -> resetting it...");
        if (resetOPC())
            yInfo("... done resetting WSOPC");
        else
            yError("... problem resetting WSOPC");
    }

    // refresh opcIDs again, it should have exactly 2 entries (hands)
    checkOPCStatus(2,opcIDs);
    if (opcIDs.size() >= 3)
    {
        yError("WSOPC has 3+ entries after resetting it, cannot safely complete %s",
               __func__);
        return false;
    }
    */

    // try constructing memory item from each ID (will not construct those whose IDs already exist)
    for (int o=0; o<opcIDs.size(); o++)
    {
        if (!constructMemoryFromOPCID(opcIDs.get(o).asInt()))
            yWarning() << "will not constructMemoryFromOPCID" << opcIDs.get(o).asInt();
    }

    return true;
}

/**********************************************************/
bool WorldStateMgrThread::memoryContainsID(const int &id)
{
    bool found = false;

    for(std::vector<MemoryItemHand>::const_iterator iter = hands.begin();
        iter != hands.end();
        ++iter)
    {
        if (iter->id == id)
            found = true;
    }

    for(std::vector<MemoryItemObj>::const_iterator iter = objs.begin();
        iter != objs.end();
        ++iter)
    {
        if (iter->id == id)
            found = true;
    }

    return found;
}

/**********************************************************/
bool WorldStateMgrThread::memoryContainsName(const string &n)
{
    bool found = false;

    for(std::vector<MemoryItemHand>::const_iterator iter = hands.begin();
        iter != hands.end();
        ++iter)
    {
        if (iter->name == n)
            found = true;
    }

    for(std::vector<MemoryItemObj>::const_iterator iter = objs.begin();
        iter != objs.end();
        ++iter)
    {
        if (iter->name == n)
            found = true;
    }

    return found;
}

/**********************************************************/
bool WorldStateMgrThread::parseHandProperties(const Bottle *fields,
                              bool &isFree)
{
    if (fields==NULL)
        return false;

    if (fields->check("is_free"))
        isFree = fields->find("is_free").asBool();
    else
    {
        yWarning("is_free field empty, assuming true");
        isFree = true;
    }

    return true;
}

/**********************************************************/
bool WorldStateMgrThread::parseObjProperties(const Bottle *fields,
                              Bottle &pos2d,
                              Bottle &desc2d, Bottle &tooldesc2d,
                              string &inHand, Bottle &onTopOf,
                              Bottle &reachW, Bottle &pullW)
{
    if (fields==NULL)
        return false;

    //yDebug() << __func__ << "*** fields =" << fields->toString().c_str();

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
bool WorldStateMgrThread::tellActivityGoHome()
{
    if (activityPort.getOutputCount()<1)
        return false;

    if (!toldActivityGoHome)
    {
        Bottle activityCmd, activityReply;
        activityCmd.addString("goHome");
        yInfo() << "connected to activityInterface, sending it instruction:" << activityCmd.toString().c_str();
        toldUserActivityIFConnected = true;
        activityPort.write(activityCmd, activityReply);
        //yDebug() << __func__ <<  "obtained response:" << activityReply.toString().c_str();
        bool validResponse = false;
        validResponse = activityReply.size()>0 &&
                        activityReply.get(0).asVocab()==Vocab::encode("ok");

        if (!validResponse)
            yWarning() << __func__ <<  "obtained invalid response:" << activityReply.toString().c_str();

        toldActivityGoHome = true;
    }

    return true;
}

/**********************************************************/
string WorldStateMgrThread::id2label(const int &id)
{
    //yDebug() << __func__ << "looking for id" << id;

    // search in WSOPC
    if(opcPort.getOutputCount()>0)
    {
        // query: [get] (("id" <num>) ("propSet" ("name")))
        Bottle opcCmd;
        Bottle opcCmdContent;
        Bottle opcReply;
        opcCmd.addVocab(Vocab::encode("get"));

        Bottle bID;
        bID.addString("id");
        bID.addInt(id);
        opcCmdContent.addList() = bID;

        Bottle bProp;
        Bottle bPropContent;
        bProp.addString("propSet");
        bPropContent.addString("name");
        bProp.addList() = bPropContent;
        opcCmdContent.addList() = bProp;

        opcCmd.addList() = opcCmdContent;

        //yDebug() << __func__ << "sending command to WSOPC:" << opcCmd.toString().c_str();
        opcPort.write(opcCmd, opcReply);

        //reply: [ack] ((name blah))
        bool found = opcReply.size()==2 &&
                     opcReply.get(0).asVocab()==Vocab::encode("ack");

        if (found)
        {
            Bottle *fields = opcReply.get(1).asList();

            string name = fields->find("name").asString();
            if (name!="")
                return name;
        }
    }

    // search in hands memory
    for(std::vector<MemoryItemHand>::const_iterator iter = hands.begin();
        iter != hands.end();
        ++iter)
    {
        if (iter->id == id)
            return iter->name;
    }

    // search in objects memory
    for(std::vector<MemoryItemObj>::const_iterator iter = objs.begin();
        iter != objs.end();
        ++iter)
    {
        if (iter->id == id)
        {
            return iter->name;
        }
    }

    // search in candidateTrackMap (not yet saved to objects memory)
    for(idLabelMap::const_iterator iter = candidateTrackMap.begin();
        iter != candidateTrackMap.end();
        ++iter)
    {
        if (iter->first == id)
            return iter->second;
    }

    // id not found anywhere
    yWarning("did not find label corresponding to id %d", id);
    return "NOTFOUND";
}

/**********************************************************/
int WorldStateMgrThread::label2id(const string &label, bool useTrackerCheck)
{
    if (label.empty())
    {
        yWarning() << __func__ << "was called with empty label argument!";
        return false;
    }

    //yDebug() << __func__ << "looking for label" << label.c_str();

    // search in hands memory
    for(std::vector<MemoryItemHand>::const_iterator iter = hands.begin();
        iter != hands.end();
        ++iter)
    {
        if (iter->name == label)
            return iter->id;
    }

    // search in objects memory
    for(std::vector<MemoryItemObj>::const_iterator iter = objs.begin();
        iter != objs.end();
        ++iter)
    {
        if (iter->name == label)
        {
            if (useTrackerCheck) // extra check used for pause/resume
            {
                if (ensureTrackerHasID(iter->id))
                {
                    //yDebug("confirmed that tracker has ID %d", iter->id);
                    return iter->id;
                }
                else
                    yWarning("tracker does not have ID %d but short-term memory has it!", iter->id);
            }
            else // other cases
            {
                return iter->id;
            }
        }
    }

    // search in candidateTrackMap (not yet saved to objects memory)
    for(idLabelMap::const_iterator iter = candidateTrackMap.begin();
        iter != candidateTrackMap.end();
        ++iter)
    {
        if (iter->second == label)
            return iter->first;
    }

    // id not found anywhere
    //yWarning("did not find id corresponding to label %s", label.c_str());
    return -1;
}

/**********************************************************/
bool WorldStateMgrThread::getLabel(const int &u, const int &v, string &label)
{
    if (activityPort.getOutputCount()<1)
    {
        yWarning() << __func__ << "not connected to activityInterface";
        return false;
    }

    Bottle activityCmd, activityReply;
    activityCmd.addString("getLabel");
    activityCmd.addInt(u);
    activityCmd.addInt(v);
    //yDebug() << __func__ <<  "sending query:" << activityCmd.toString().c_str();
    activityPort.write(activityCmd, activityReply);

    // response format: "winning-label"
    bool validDeterministicResponse = activityReply.get(0).isString();
    
    if (validDeterministicResponse)
    {
        if (activityReply.get(0).asString().size()==0)
        {
            yWarning() << __func__ << "obtained valid but empty deterministic response:" << activityReply.toString().c_str();
            return false;
        }
        else
        {
            //yDebug() << __func__ << "obtained valid deterministic response:" << activityReply.toString().c_str();
            label = activityReply.get(0).asString();
        }
    }
    else
    {
        yWarning() << __func__ << "obtained invalid response:" << activityReply.toString().c_str();
        return false;
    }

    return true;
}

/**********************************************************/
bool WorldStateMgrThread::getLabelMajorityVote(const int &u, const int &v,
                                               string &winnerLabel,
                                               const int &rounds)
{
    int majority = ceil( static_cast<double>(rounds/2.0) );

    // accumulate vote strings
    vector<string> votes;
    for (int r=0; r<rounds; ++r)
    {
        string thisVote;
        if (getLabel(u, v, thisVote))
        {
            if (!thisVote.empty())
                votes.push_back(thisVote);
        }
        yarp::os::Time::delay(0.1);
    }

    if (votes.empty())
    {
        yWarning() << "got" << rounds << "consecutive empty labels from object recognition!";
        return false;
    }

    // sort strings
    sort(votes.begin(), votes.end());

    // histogram with corresponding counts - http://stackoverflow.com/a/9616995
    typedef map<string, int> counts_t;
    counts_t histogram;
    for(vector<string>::const_iterator iter = votes.begin();
        iter != votes.end();
        ++iter)
    {
        ++histogram[*iter];
    }

    // pick winner - http://stackoverflow.com/a/3798349
    std::pair<string, int> max_el = *std::max_element(histogram.begin(), histogram.end(), compareSecond());
    if (max_el.second < majority)
        yInfo("winning label --> %s <-- with %d/%zu non-empty entries after %d queries, despite being less than majority %d",
                 max_el.first.c_str(), max_el.second, votes.size(), rounds, majority);
    else
        yInfo("winning label --> %s <--", max_el.first.c_str());

    winnerLabel = max_el.first;

    return true;
}

/**********************************************************/
bool WorldStateMgrThread::isOnTopOf(const string &objName, Bottle &objBelow)
{
    // ask "underOf objName" to activityInterface, put result in objBelow

    if (activityPort.getOutputCount()<1)
    {
        yWarning() << __func__ << "not connected to activityInterface";
        return false;
    }

    Bottle activityCmd, activityReply;
    activityCmd.addString("underOf");
    activityCmd.addString(objName.c_str());
    //yDebug() << __func__ << "sending query:" << activityCmd.toString().c_str();
    activityPort.write(activityCmd, activityReply);

    bool validResponse = false;
    validResponse = activityReply.size()>0 &&
                    activityReply.get(0).isList();

    if (validResponse)
    {
        //yDebug() << __func__ << "obtained valid response:" << activityReply.toString().c_str();
        objBelow = *activityReply.get(0).asList();
        return true;
    }
    else
    {
        yWarning() << __func__ << "obtained invalid response:" << activityReply.toString().c_str();
        return false;
    }
}

/**********************************************************/
bool WorldStateMgrThread::isReachableWith(const string &objName,
                                          Bottle &objReachable)
{
    if (activityPort.getOutputCount()<1)
    {
        yWarning() << __func__ << "not connected to activityInterface";
        return false;
    }

    Bottle activityCmd, activityReply;
    activityCmd.addString("reachableWith");
    activityCmd.addString(objName.c_str());
    //yDebug() << __func__ << "sending query:" << activityCmd.toString().c_str();
    activityPort.write(activityCmd, activityReply);

    bool validResponse = false;
    validResponse = activityReply.size()>0 &&
                    activityReply.get(0).isList();

    if (validResponse)
    {
        //yDebug() << __func__ << "obtained valid response:" << activityReply.toString().c_str();
        objReachable = *activityReply.get(0).asList();
        return true;
    }
    else
    {
        yWarning() << __func__ << "obtained invalid response:" << activityReply.toString().c_str();
        return false;
    }
}

/**********************************************************/
bool WorldStateMgrThread::isPullableWith(const string &objName,
                                         Bottle &objPullable)
{
    if (activityPort.getOutputCount()<1)
    {
        yWarning() << __func__ << "not connected to activityInterface";
        return false;
    }

    Bottle activityCmd, activityReply;
    activityCmd.addString("pullableWith");
    activityCmd.addString(objName.c_str());
    //yDebug() << __func__ <<  "sending query:" << activityCmd.toString().c_str();
    activityPort.write(activityCmd, activityReply);

    bool validResponse = false;
    validResponse = activityReply.size()>0 &&
                    activityReply.get(0).isList();

    if (validResponse)
    {
        //yDebug() << __func__ << "obtained valid response:" << activityReply.toString().c_str();
        objPullable = *activityReply.get(0).asList();
        return true;
    }
    else
    {
        yWarning() << __func__ << "obtained invalid response:" << activityReply.toString().c_str();
        return false;
    }
}

/**********************************************************/
bool WorldStateMgrThread::isHandFree(const string &handName)
{
    if (activityPort.getOutputCount()<1)
    {
        yWarning() << __func__ << "not connected to activityInterface";
        return true; // hand free
    }

    if ((handName != "left") && (handName != "right"))
    {
        yWarning() << __func__ << "argument handName must be left or right";
        return true; // hand free
    }

    Bottle activityCmd, activityReply;
    activityCmd.addString("handStat");
    activityCmd.addString(handName.c_str());
    //yDebug() << __func__ << "sending query:" << activityCmd.toString().c_str();
    activityPort.write(activityCmd, activityReply);

    bool validResponse = false;
    validResponse = activityReply.size()>0 &&
                    activityReply.get(0).isVocab(); // isBool()?

    if (validResponse)
    {
        //yDebug() << __func__ << "obtained valid response:" << activityReply.toString().c_str();
        if (activityReply.get(0).asVocab() == Vocab::encode("ok"))
            return false; // hand occupied
        else if (activityReply.get(0).asVocab() == Vocab::encode("nack"))
            return true; // hand free
    }
    else
    {
        yWarning() << __func__ << "obtained invalid response:" << activityReply.toString().c_str() << "-> assuming [nack] i.e. hand free";
        return true; // hand free
    }
}

/**********************************************************/
string WorldStateMgrThread::inWhichHand(const string &objName)
{
    string ret = "none"; // default result

    if (activityPort.getOutputCount()<1)
    {
        yWarning() << __func__ << "not connected to activityInterface";
        return ret;
    }

    if ((objName == "left") || (objName == "right"))
    {
        yWarning() << __func__ << "argument objName must be an object name, not a hand name";
        return ret;
    }

    Bottle activityCmd, activityReply;
    activityCmd.addString("inHand");
    activityCmd.addString(objName.c_str());
    //yDebug() << __func__ << "sending query:" << activityCmd.toString().c_str();
    activityPort.write(activityCmd, activityReply);

    bool validResponse = false;
    validResponse = activityReply.size()>0 &&
                    activityReply.get(0).isString() &&
                    activityReply.get(0).asString().size()>0;

    if (validResponse)
    {
        if ( activityReply.get(0).asString()=="left" ||
             activityReply.get(0).asString()=="right" ||
             activityReply.get(0).asString()=="none" )
        {
            //yDebug() << __func__ <<  "obtained valid response:" << activityReply.toString().c_str();
            ret = activityReply.get(0).asString();
        }
        else
            yWarning() << __func__ << "obtained valid response (but unexpected string):" << activityReply.toString().c_str();
    }
    else
        yWarning() << __func__ << "obtained invalid response:" << activityReply.toString().c_str();

    return ret;
}

/**********************************************************/
bool WorldStateMgrThread::getVisibilityByActivityIF(const string &objName,
                                                    bool &result,
                                                    int extraTries)
{
    // result=true if the 2D position of objName is not null. function always
    // tries a first time, and if that fails it tries another extraTries times.

    if (activityPort.getOutputCount()<1)
    {
        yWarning() << __func__ << "not connected to activityInterface";
        return false;
    }

    Bottle activityCmd, activityReply;
    activityCmd.addString("get2D");
    activityCmd.addString(objName.c_str());
    //yDebug() << __func__ << "sending query:" << activityCmd.toString().c_str();
    activityPort.write(activityCmd, activityReply);

    result = activityReply.size()>0 &&
             activityReply.get(0).isList() &&
             activityReply.get(0).asList()->size()>0; // valid ROI coordinates

    // if first try returned empty ROI (not visible object), try more times
    if (!result && extraTries>0)
    {
        yDebug("%s: %d remaining tries of query: get2D %s",
               __func__, extraTries, objName.c_str());
        extraTries--;
        getVisibilityByActivityIF(objName, result, extraTries);
    }

    return result;
}

/**********************************************************/
bool WorldStateMgrThread::belongsToStack(const string &objName, bool &result)
{
    // result=true iff objName is present in the stack of any other object

    result = false; // by default, objName is not in any stack

    if (activityPort.getOutputCount()<1)
    {
        yWarning() << __func__ << "not connected to activityInterface";
        return false;
    }

    // list of objects other than objName
    Bottle otherObjs;
    Bottle activityCmd;
    Bottle activityReply;
    activityCmd.addString("getNames");
    //yDebug() << __func__ << "sending query:" << activityCmd.toString().c_str();
    activityPort.write(activityCmd, activityReply);

    bool validResponse = activityReply.size()>0 &&
                         activityReply.get(0).isList();

    if (!validResponse)
    {
        yError("%s: obtained invalid response from activityInterface",
              __func__);
        return false;
    }

    bool somethingVisible = validResponse &&
                            activityReply.get(0).asList()->size()>0;

    if (!somethingVisible)
    {
        yDebug("%s: getNames is empty -> %s does not belong to any stack",
               __func__, objName.c_str());
        return false;
    }

    // cycle over objects returned by getNames
    for (int o=0; o<activityReply.get(0).asList()->size(); ++o)
    {
        // skip checking if objName is below itself
        if (activityReply.get(0).asList()->get(o).asString()==objName)
            continue;

        // list of objects below o
        Bottle bLabelsBelow; // strings
        isOnTopOf(activityReply.get(0).asList()->get(o).asString(), bLabelsBelow);

        // cycle over the objects below o
        for (int b=0; b<bLabelsBelow.size(); ++b)
        {
            // if one of them is objName, set result to true and exit inner cycle
            if (bLabelsBelow.get(b).asString()==objName)
            {
                //yDebug("%s: %s is below %s -> isStacked=true",
                //       __func__, objName.c_str(),
                //       activityReply.get(0).asList()->get(o).asString().c_str());
                result = true;
                break;
            }
        }

        // if we determined a true result already, exit outer loop
        if (result)
            break;
    }

    yDebug("%s: determining if %s is below any of these: (%s)... %s",
           __func__, objName.c_str(), activityReply.get(0).asList()->toString().c_str(), BoolToString(result));

    return true;
}

/**********************************************************/
bool WorldStateMgrThread::isInHand(const string &objName, bool &result)
{
    // result=true iff objName is in one of the robot hands

    if (activityPort.getOutputCount()<1)
    {
        yWarning() << __func__ << "not connected to activityInterface";
        return false;
    }

    string inHand = inWhichHand(objName); // none, left, right
    result = (inHand=="left" || inHand=="right");

    return true;
}

/**********************************************************/
bool WorldStateMgrThread::setFilterOrder(const int &n)
{
    if (is_integer(n))
    {
        filterOrder = n;
        return true;
    }
    else
    {
        yWarning() << __func__ << "argument must be integer";
        return false;
    }
}

/**********************************************************/
bool WorldStateMgrThread::doPopulateDB()
{
    // cycle over hands memory
    for(std::vector<MemoryItemHand>::iterator iter = hands.begin();
        iter != hands.end();
        ++iter)
    {
        // compute updated properties
        bool newIsFreeValue = isHandFree(iter->name);

        // write to internal memory model
        iter->isFree = newIsFreeValue;

        // prepare content for WSOPC
        Bottle bIsFree;
        bIsFree.addString("is_free");
        bIsFree.addString( BoolToString(newIsFreeValue) );

        // write to WSOPC
        if (opcContainsID(iter->id))
        {
            // WSOPC already contains this entry, as expected
            // -> update it with command
            // [set] (("id" <num>) ("prop0" <val0>) ...)

            //yDebug("modifying existing entry %d in database", iter->id);
            Bottle opcCmd;
            Bottle opcCmdContent;
            Bottle opcReply;
            opcCmd.addVocab(Vocab::encode("set"));

            Bottle bID;
            bID.clear();
            bID.addString("id");
            bID.addInt(iter->id);
            opcCmdContent.addList() = bID;

            opcCmdContent.addList() = bIsFree;
            opcCmd.addList() = opcCmdContent;

            //yDebug() << __func__ << "sending command to WSOPC:" << opcCmd.toString().c_str();
            opcPort.write(opcCmd, opcReply);
            //yDebug() << __func__ << "received response:" << opcReply.toString().c_str();

            // process WSOPC response
            bool validResponse = opcReply.size()>0 &&
                     opcReply.get(0).asVocab()==Vocab::encode("ack");
            if (!validResponse)
                yWarning() << __func__ << "did not receive valid response from WSOPC";
        }
        else
        {
            yWarning("WSOPC does not contain hand entry %d/%s -> something's fishy!",
                    iter->id, iter->name.c_str());
        }
    }

    // cycle over objects memory
    for(std::vector<MemoryItemObj>::iterator iter = objs.begin();
        iter != objs.end();
        ++iter)
    {
        // get current properties
        Bottle pos2d = iter->pos2d; // value to keep if obj not visible
        Bottle desc2d = iter->desc2d; // value to keep if obj not visible
        Bottle tooldesc2d = iter->tooldesc2d; // value to keep if obj not visible
        string inHand;
        Bottle onTopOf;
        Bottle reachW;
        Bottle pullW;
        // compute updated properties
        computeObjProperties(iter->id,iter->name,
                             pos2d,             // if not visible, keep current
                             desc2d,tooldesc2d, // if not visible, keep current
                             inHand,
                             onTopOf,
                             reachW,pullW);

        // write to internal memory model
        iter->pos2d = pos2d;
        iter->desc2d = desc2d;
        iter->tooldesc2d = tooldesc2d;
        iter->inHand = inHand;
        iter->onTopOf = onTopOf;
        iter->reachW = reachW;
        iter->pullW = pullW;

        // prepare content for WSOPC
        Bottle bName;
        bName.addString("name");
        bName.addString(iter->name);
        Bottle bIsHand;
        bIsHand.addString("is_hand");
        bool isHandValueBool = iter->isHand;
        // make sure that is_hand==false i.e. this is an object
        if (isHandValueBool)
        {
            yWarning("object %d/%s: was expecting is_hand=false, got is_hand=true! skipping...",
                iter->id, iter->name.c_str());
            continue;
        }
        string isHandValueString = BoolToString(isHandValueBool);
        bIsHand.addString(isHandValueString);
        Bottle bPos2d;
        bPos2d.addString("pos2d");
        bPos2d.addList() = pos2d;
        Bottle bDesc2d;
        bDesc2d.addString("desc2d");
        bDesc2d.addList() = desc2d;
        Bottle bToolDesc2d;
        bToolDesc2d.addString("tooldesc2d");
        bToolDesc2d.addList() = tooldesc2d;
        Bottle bInHand;
        bInHand.addString("in_hand");
        bInHand.addString(inHand);
        Bottle bOnTopOf;
        bOnTopOf.addString("on_top_of");
        bOnTopOf.addList() = onTopOf;
        Bottle bReachW;
        bReachW.addString("reachable_with");
        bReachW.addList() = reachW;
        Bottle bPullW;
        bPullW.addString("pullable_with");
        bPullW.addList() = pullW;

        // write to WSOPC
        if (opcContainsID(iter->id))
        {
            // WSOPC already contains this entry
            // -> update it with command
            // [set] (("id" <num>) ("prop0" <val0>) ...)

            yDebug("modifying existing entry in database: %d/%s",
                   iter->id, iter->name.c_str());
            Bottle opcCmd;
            Bottle opcCmdContent;
            Bottle opcReply;
            opcCmd.addVocab(Vocab::encode("set"));

            Bottle bID;
            bID.clear();
            bID.addString("id");
            bID.addInt(iter->id);
            opcCmdContent.addList() = bID;

            // name and is_hand fields are already set

            opcCmdContent.addList() = bPos2d;
            opcCmdContent.addList() = bDesc2d;
            opcCmdContent.addList() = bToolDesc2d;
            opcCmdContent.addList() = bInHand;
            opcCmdContent.addList() = bOnTopOf;
            opcCmdContent.addList() = bReachW;
            opcCmdContent.addList() = bPullW;
            opcCmd.addList() = opcCmdContent;

            //yDebug() << __func__ << "sending command to WSOPC:" << opcCmd.toString().c_str();
            opcPort.write(opcCmd, opcReply);
            //yDebug() << __func__ << "received response:" << opcReply.toString().c_str();

            // process WSOPC response
            bool validResponse = opcReply.size()>0 &&
                     opcReply.get(0).asVocab()==Vocab::encode("ack");
            if (!validResponse)
                yWarning() << __func__ << "did not receive valid response from WSOPC";
        }
        else
        {
            // WSOPC does not contain this entry ->
            // [add] (("prop0" <val0>) ("prop1" <val1>) ...)
            // including name, is_hand

            yInfo("adding new entry to database: %d/%s",
                   iter->id, iter->name.c_str());
            Bottle opcCmd;
            Bottle opcCmdContent;
            Bottle opcReply;
            opcCmd.addVocab(Vocab::encode("add"));

            opcCmdContent.addList() = bName;
            opcCmdContent.addList() = bIsHand;

            opcCmdContent.addList() = bPos2d;
            opcCmdContent.addList() = bDesc2d;
            opcCmdContent.addList() = bToolDesc2d;
            opcCmdContent.addList() = bInHand;
            opcCmdContent.addList() = bOnTopOf;
            opcCmdContent.addList() = bReachW;
            opcCmdContent.addList() = bPullW;
            opcCmd.addList() = opcCmdContent;

            //yDebug() << __func__ << "sending command to WSOPC:" << opcCmd.toString().c_str();
            opcPort.write(opcCmd, opcReply);
            //yDebug() << __func__ << "received response:" << opcReply.toString().c_str();

            // process WSOPC response
            bool validResponse = opcReply.size()>=2 &&
                     opcReply.get(0).asVocab()==Vocab::encode("ack") &&
                     opcReply.get(1).isList() &&
                     opcReply.get(1).asList()->get(0).asVocab()==Vocab::encode("id");

            if (validResponse)
            {
                // make sure that OPC's returned id equals memory id
                const int returnedID =
                    opcReply.get(1).asList()->get(1).asInt();
                if (returnedID != iter->id)
                    yError() << "ID mismatch while adding" << iter->name
                             << "to database! got" << returnedID
                             << "was expecting" << iter->id << "!";
            }
        }
    }

    // send "dump" instruction to WSOPC
    Bottle opcCmd, opcReply;
    opcCmd.addVocab(Vocab::encode("dump"));
    //yDebug() << __func__ << "sending command to WSOPC:" << opcCmd.toString().c_str();
    opcPort.write(opcCmd, opcReply);
    //yDebug() << __func__ << "received response:" << opcReply.toString().c_str();

    return true;
}

/**********************************************************/
void WorldStateMgrThread::fsmPerception()
{
    //yDebug("perception state=%d", fsmState);
    switch(fsmState)
    {
        case STATE_PERCEPTION_WAIT_OPC:
        {
            tellUserConnectOPC();

            if (opcPort.getOutputCount()>0)
            {
                tellUserOPCConnected();

                // acquire initial entries (robot hands) from WSOPC database
                if (!initMemoryFromOPC())
                    yWarning("problem with initMemoryFromOPC");

                printMemoryState();

                // proceed
                fsmState = STATE_PERCEPTION_WAIT_BLOBS;
            }

            break;
        }

        case STATE_PERCEPTION_WAIT_BLOBS:
        {
            tellUserConnectBlobs();

            if(inAffPort.getInputCount()>0 && inToolAffPort.getInputCount()>0)
                tellUserBlobsConnected();

            // acquire shape descriptors
            refreshBlobs();

            // when something arrives, proceed
            if (inAff != NULL)
                fsmState = STATE_PERCEPTION_READ_BLOBS;

            break;
        }

        case STATE_PERCEPTION_READ_BLOBS:
        {
            // if size>0 proceed, else go back one state
            if (sizeAff > 0)
                fsmState = STATE_PERCEPTION_WAIT_TRACKER;
            else
                fsmState = STATE_PERCEPTION_WAIT_BLOBS;

            break;
        }

        case STATE_PERCEPTION_WAIT_TRACKER:
        {
            tellUserConnectTracker();

            if (inTargetsPort.getInputCount()>0)
            {
                // pre-initialize tracker, setting the "countFrom" starting
                // index (13 upon a clean start of all modules, a higher number
                // if something was initialized previously and had to be reset).
                configureTracker();

                // proceed
                fsmState = STATE_PERCEPTION_INIT_TRACKER;
            }

            break;
        }

        case STATE_PERCEPTION_INIT_TRACKER:
        {
            if (!checkTrackerStatus())
            {
                if (needTrackerInit)
                {
                    // start tracking blobs provided by segmentation/blobDesc
                    initTracker();

                    needTrackerInit = false;
                }
            }
            else
            {
                yInfo("%d objects are being tracked", trackIDs.size());

                // proceed
                fsmState = STATE_PERCEPTION_READ_TRACKER;
            }

            break;
        }

        case STATE_PERCEPTION_READ_TRACKER:
        {
            // acquire tracks
            if(!refreshTracker())
                yWarning("problem with refreshTracker");

            // if size>0 proceed, else stay in same state
            if (sizeTargets > 0)
                fsmState = STATE_PERCEPTION_WAIT_ACTIVITYIF;

            break;
        }

        case STATE_PERCEPTION_WAIT_ACTIVITYIF:
        {
            tellUserConnectActivityIF();
            tellActivityGoHome();

            if (toldActivityGoHome)
            {
                // proceed
                fsmState = STATE_PERCEPTION_SET_MEMORY;
            }

            break;
        }

        case STATE_PERCEPTION_SET_MEMORY:
        {
            Bottle opcIDs;
            if (checkOPCStatus(3,opcIDs))
            {
                yInfo("WSOPC was already initialized, it contains 3+ entries: %s. updating short-term memory.",
                     opcIDs.toString().c_str());
                printMemoryState();
                // construct memory item from each ID
                for (int o=0; o<opcIDs.size(); o++)
                    constructMemoryFromOPCID(opcIDs.get(o).asInt());

                // proceed
                fsmState = STATE_PERCEPTION_POPULATE_DB;
            }

            // here we know that WSOPC does not have object entries yet ->
            // we must acquire labels from object recognition then construct
            // our internal short-term memory data structures
            // (so that we can later add object entries to WSOPC)

            // try acquiring unique labels
            if (getTrackNames())
            {
                // add unique <ID,label> pairs to objs container
                if (constructMemoryFromMap())
                    yInfo("success, added candidate object IDs and names to short-term memory");
                else
                    yWarning("problem with constructMemoryFromMap"); // TODO: go back?

                // proceed
                fsmState = STATE_PERCEPTION_POPULATE_DB;
            }
            else
            {
                // failure -> stay in same state, clear candidates & try again

                // acquire getNames list to print it below for debug
                Bottle activityCmd;
                Bottle activityReply;
                activityCmd.clear();
                activityReply.clear();
                activityCmd.addString("getNames");
                activityPort.write(activityCmd, activityReply);
                bool validResponse = activityReply.size()>0 &&
                                     activityReply.get(0).isList();
                if (!validResponse)
                    yError("STATE_PERCEPTION_SET_MEMORY: obtained invalid response from activityInterface");
                bool somethingVisible = validResponse &&
                                        activityReply.get(0).asList()->size()>0;
                if (!somethingVisible)
                    yWarning("STATE_PERCEPTION_SET_MEMORY: getNames is empty");

                // acquire opcIDs to print it below for debug
                Bottle opcIDs;
                const int numHandEntries = 2;
                if (!checkOPCStatus(numHandEntries,opcIDs))
                    yWarning("STATE_PERCEPTION_SET_MEMORY: problem verifying that WSOPC has at least %d entries", numHandEntries);

                yWarning() << "failure in initializing IDs and names:" <<
                              "at least one of the candidate names was" <<
                              "a duplicate or was skipped. trying again." <<
                              "if this goes on forever, check the status of" <<
                              "object recognition and WSOPC database. getNames() =" <<
                              activityReply.get(0).asList()->toString().c_str() <<
                              "; opcIDs =" << opcIDs.toString().c_str();

                candidateTrackMap.clear();
            }

            break;
        }

        case STATE_PERCEPTION_POPULATE_DB:
        {
            // read new data
            if(!refreshBlobs())
                yWarning("problem with refreshBlobs");
            if(!refreshTracker())
                yWarning("problem with refreshTracker");

            // populate database: if success proceed, else stay in same state
            if ( doPopulateDB() )
            {
                yInfo("initialization complete (tracker, short-term memory, WSOPC database)");
                initFinished = true;
                fsmState = STATE_PERCEPTION_WAIT_CMD;
            }
            else
                yWarning("problem populating database");

            printMemoryState();

            break;
        }

        case STATE_PERCEPTION_WAIT_CMD:
        {
            if (needUpdate)
                fsmState = STATE_PERCEPTION_UPDATE_DB;

            break;
        }

        case STATE_PERCEPTION_UPDATE_DB:
        {
            if (needUpdate)
            {
                // read new data
                if(!refreshBlobs())
                    yWarning("problem with refreshBlobs");
                if(!refreshTracker())
                    yWarning("problem with refreshTracker");

                // populate database
                if ( !doPopulateDB() )
                    yWarning() << __func__ << "problem populating database";

                needUpdate = false;
                printMemoryState();

                // go back one state
                fsmState = STATE_PERCEPTION_WAIT_CMD;
            }

            break;
        }

        default:
        {
            break;
        }
    }
}

// IDL functions

/**********************************************************/
bool WorldStateMgrThread::printMemoryState()
{
    yInfo("short-term memory:");
        
    for(std::vector<MemoryItemHand>::const_iterator iter = hands.begin();
        iter != hands.end();
        ++iter)
    {
        // print result of MemoryItemHand::toString()
        ostringstream s;
        s << *iter;
        yInfo("%s\n", s.str().c_str());
    }

    for(std::vector<MemoryItemObj>::const_iterator iter = objs.begin();
        iter != objs.end();
        ++iter)
    {
        // print result of MemoryItemObj::toString()
        ostringstream s;
        s << *iter;
        yInfo("%s\n", s.str().c_str());
    }

    return true;
}

/**********************************************************/
bool WorldStateMgrThread::initWorldState()
{
    if (initFinished)
    {
        yWarning("world state is already initialized");
        // return "ok" even if already initialized, to make PlannerThread::startPlanning() proceed
        return true;
    }

    if (activityPort.getOutputCount()<1)
    {
        yWarning("not connected to activityInterface, cannot initialize world state!");
        return false;
    }

    if (inAffPort.getInputCount()<1 || inToolAffPort.getInputCount()<1)
    {
        yWarning("not connected to BlobDescriptor, cannot initialize world state!");
        return false;
    }

    if (opcPort.getOutputCount()<1)
    {
        yWarning("not connected to WSOPC, cannot initialize world state!");
        return false;
    }

    // reset variables, objects stack, activeParticleTracker, internal short-term memory
    clearAll();

    yInfo("initializing world state from robot perception...");
    needTrackerInit = true;

    // enter FSM
    fsmState = STATE_PERCEPTION_WAIT_TRACKER;

    while (!initFinished)
        yarp::os::Time::delay(0.1);

    return true;
}

/**********************************************************/
bool WorldStateMgrThread::updateWorldState()
{
    if (activityPort.getOutputCount()<1)
    {
        yWarning("not connected to activityInterface, cannot update world state!");
        return false;
    }
    if (!checkTrackerStatus())
    {
        yWarning("tracker not initialized, cannot update world state!");
        return false;
    }

    yInfo("updating world state from robot perception...");
    needUpdate = true;

    while (needUpdate)
        yarp::os::Time::delay(0.1);

    yInfo("...world state updated");

    // TODO: opcPort.write() should be here instead of inner functions

    return true;
}

/**********************************************************/
bool WorldStateMgrThread::resetWorldState()
{
    // clean WSOPC so that it has only hand entries (and objects currently hidden under a stack)
    if (!resetOPC())
    {
        yWarning("%s problem resetting WSOPC", __func__);
        return false;
    }

    // this needs to be reset before the call to initWorldState()
    initFinished = false;

    initWorldState();

    return true;
}

/**********************************************************/
bool WorldStateMgrThread::isInitialized()
{
    return initFinished;
}

/**********************************************************/
bool WorldStateMgrThread::pauseTrack(const string &objName)
{
    if (objName.empty())
    {
        yWarning() << __func__ << "was called with empty objName argument!";
        return false;
    }

    if (trackerPort.getOutputCount()<1)
    {
        yWarning("not connected to /activeParticleTrack/rpc:i");
        return false;
    }

    if (!checkTrackerStatus())
    {
        yWarning("tracker not initialized, cannot pause %s", objName.c_str());
        return false;
    }

    int id;
    id = label2id(objName, true); // calls ensureTrackerHasID
    if (id==-1)
    {
        yWarning() << __func__ << "did not find tracker ID corresponding to" << objName.c_str() << " - not able to pause it";
        return false;
    }

    Bottle trackerCmd, trackerReply;
    trackerCmd.addString("pause");
    trackerCmd.addInt(id);
    trackerPort.write(trackerCmd, trackerReply);
    //yDebug() << __func__ <<  "sending query to activeParticleTracker:" << trackerCmd.toString().c_str();

    if (trackerReply.size()>0)
    {
        if (trackerReply.get(0).asVocab()==Vocab::encode("fail"))
        {
            yWarning() << __func__ << "received negative reply when asking to pause" << objName.c_str()
                     << "with ID" << id;
        }
        else if (trackerReply.get(0).asVocab()==Vocab::encode("ok"))
        {
            yDebug() << __func__ << "successfully paused" << objName.c_str() << "with ID" << id;
        }
        else
        {
            yWarning() << __func__ << "received invalid response:" << trackerReply.toString().c_str();
        }
    }

    return true;
}

/**********************************************************/
bool WorldStateMgrThread::resumeTrack(const string &objName)
{
    if (objName.empty())
    {
        yWarning() << __func__ << "was called with empty objName argument!";
        return false;
    }

    if (trackerPort.getOutputCount()<1)
    {
        yWarning("not connected to /activeParticleTrack/rpc:i");
        return false;
    }

    if (!checkTrackerStatus())
    {
        yWarning("tracker not initialized, cannot resume %s", objName.c_str());
        return false;
    }

    int id;
    id = label2id(objName, true); // calls ensureTrackerHasID
    if (id==-1)
    {
        yWarning() << __func__ << "did not find tracker ID corresponding to" << objName.c_str() << "- not able to resume it";
        return false;
    }

    Bottle trackerCmd, trackerReply;
    trackerCmd.addString("resume");
    trackerCmd.addInt(id);
    trackerPort.write(trackerCmd, trackerReply);
    //yDebug() << __func__ <<  "sending query to activeParticleTracker:" << trackerCmd.toString().c_str();

    if (trackerReply.size()>0)
    {
        if (trackerReply.get(0).asVocab()==Vocab::encode("fail"))
        {
            yWarning() << __func__ << "received negative reply when asking to resume" << objName.c_str()
                     << "with ID" << id;
        }
        else if (trackerReply.get(0).asVocab()==Vocab::encode("ok"))
        {
            yDebug() << __func__ << "successfully resumed" << objName.c_str() << "with ID" << id;
        }
        else
        {
            yWarning() << __func__ << "received invalid response:" << trackerReply.toString().c_str();
        }
    }

    return true;
}

/**********************************************************/
bool WorldStateMgrThread::pauseTrackID(const int32_t &objID)
{
    if (objID<0)
    {
        yWarning() << __func__ << "was called with invalid objID argument!";
        return false;
    }

    if (trackerPort.getOutputCount()<1)
    {
        yWarning("not connected to /activeParticleTrack/rpc:i");
        return false;
    }

    if (!checkTrackerStatus())
    {
        yWarning("tracker not initialized, cannot pause %d", objID);
        return false;
    }

    Bottle trackerCmd, trackerReply;
    trackerCmd.addString("pause");
    trackerCmd.addInt(objID);
    trackerPort.write(trackerCmd, trackerReply);
    //yDebug() << __func__ <<  "sending query to activeParticleTracker:" << trackerCmd.toString().c_str();

    if (trackerReply.size()>0)
    {
        if (trackerReply.get(0).asVocab()==Vocab::encode("fail"))
        {
            yWarning() << __func__ << "received negative reply when asking to pause ID" << objID;
        }
        else if (trackerReply.get(0).asVocab()==Vocab::encode("ok"))
        {
            yDebug() << __func__ << "successfully paused ID" << objID;
        }
        else
        {
            yWarning() << __func__ << "received invalid response:" << trackerReply.toString().c_str();
        }
    }

    return true;
}

/**********************************************************/
bool WorldStateMgrThread::resumeTrackID(const int32_t &objID)
{
    if (objID<0)
    {
        yWarning() << __func__ << "was called with invalid objID argument!";
        return false;
    }

    if (trackerPort.getOutputCount()<1)
    {
        yWarning("not connected to /activeParticleTrack/rpc:i");
        return false;
    }

    if (!checkTrackerStatus())
    {
        yWarning("tracker not initialized, cannot resume %d", objID);
        return false;
    }

    Bottle trackerCmd, trackerReply;
    trackerCmd.addString("resume");
    trackerCmd.addInt(objID);
    trackerPort.write(trackerCmd, trackerReply);
    //yDebug() << __func__ <<  "sending query to activeParticleTracker:" << trackerCmd.toString().c_str();

    if (trackerReply.size()>0)
    {
        if (trackerReply.get(0).asVocab()==Vocab::encode("fail"))
        {
            yWarning() << __func__ << "received negative reply when asking to resume ID" << objID;
        }
        else if (trackerReply.get(0).asVocab()==Vocab::encode("ok"))
        {
            yDebug() << __func__ << "successfully resumed ID" << objID;
        }
        else
        {
            yWarning() << __func__ << "received invalid response:" << trackerReply.toString().c_str();
        }
    }

    return true;
}

/**********************************************************/
Bottle WorldStateMgrThread::getColorHistogram(const int32_t &u,
                                              const int32_t &v)
{
    Bottle colors;

    if (inAffPort.getInputCount()<1)
    {
        yWarning("not connected to BlobDescriptor!");
        return colors;
    }

    // look for Affordance Bottle Index: between 1 and NumBlobs
    int abi = 0;
    if ( !getAffBottleIndexFromTrackROI(u, v, abi) )
    {
        yWarning("problem with getAffBottleIndexFromTrackROI");
        return colors;
    }

    if (abi > 0)
        yDebug() << __func__ << "found affordance blob index" << abi << "from coordinates" << u << v;
    else
    {
        yDebug() << "getAffBottleIndexFromTrackROI did not find affordance blob index from coordinates" << u << v;
        return colors;
    }

    int firstColorIdx = 7;
    int lastColorIdx = 22;
    if (inAff != NULL)
    {
        for (int col = firstColorIdx; col <= lastColorIdx; ++col)
            colors.addDouble(inAff->get(abi).asList()->get(firstColorIdx+col).asDouble());
    }
    yDebug() << "found color histogram:" << colors.toString().c_str();

    return colors;
}

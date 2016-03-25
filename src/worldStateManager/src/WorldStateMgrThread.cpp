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

    inAffPortName = "/" + moduleName + "/affDescriptor:i";
    ret = ret && inAffPort.open(inAffPortName.c_str());

    inToolAffPortName = "/" + moduleName + "/toolAffDescriptor:i";
    ret = ret && inToolAffPort.open(inToolAffPortName.c_str());

    activityPortName = "/" + moduleName + "/activity:rpc";
    ret = ret && activityPort.open(activityPortName.c_str());

    return ret;
}

/**********************************************************/
void WorldStateMgrThread::close()
{
    yInfo("closing ports");

    opcPort.close();
    inAffPort.close();
    inToolAffPort.close();
    activityPort.close();
}

/**********************************************************/
void WorldStateMgrThread::interrupt()
{
    closing = true;

    yInfo("interrupting ports");
    opcPort.interrupt();
    inAffPort.interrupt();
    inToolAffPort.interrupt();
    activityPort.interrupt();
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
        fsm();

        yarp::os::Time::delay(0.01);
    }
}

/**********************************************************/
bool WorldStateMgrThread::initVars()
{
    state = STATE_WAIT_OPC;
    toldUserOPCConnected = false;
    initFinished = false;
    t = yarp::os::Time::now();
    inAff = NULL;
    inToolAff = NULL;
    needUpdate = false;
    toldUserBlobsConnected = false;
    toldActivityGoHome = false;
    toldUserActivityIFConnected = false;

    return true;
}

/**********************************************************/
bool WorldStateMgrThread::clearAll()
{
    // reset variables
    toldUserOPCConnected = false;
    initFinished = false;

    // reset internal short-term memory
    hands.clear(); // empty hands container
    initMemoryFromOPC(); // put left and right entries into hands container
    objs.clear();
    candidateMap.clear();

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

bool WorldStateMgrThread::ensureOpcNumEntries(const int &minEntries)
{
    if (opcPort.getOutputCount()<1)
        return false;

    // query: [ask] (all)
    Bottle opcCmd, opcCmdContent, opcReply;
    opcCmd.addVocab(Vocab::encode("ask"));
    opcCmdContent.addString("all");
    opcCmd.addList() = opcCmdContent;
    opcPort.write(opcCmd, opcReply);

    // reply: [ack] (id (11 12 13 ...))
    bool ret = false;
    ret = opcReply.size()>1 &&
          opcReply.get(0).asVocab()==Vocab::encode("ack") &&
          opcReply.get(1).isList() &&
          opcReply.get(1).asList()->size()>0 &&
          opcReply.get(1).asList()->get(0).asString()=="id" &
          opcReply.get(1).asList()->get(1).isList() &&
          opcReply.get(1).asList()->get(1).asList()->size()>=minEntries;

    return ret;
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
// TODO: remove
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

    // increase countFrom (future WSOPC additions start index) to account for
    // the fact that some IDs will now deleted from WSOPC (never to be used again)
    updateCountFrom();

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

                // first, delete from objs container
                // inefficient way:
                int idxToRemove = 0;
                for(std::vector<MemoryItemObj>::const_iterator iter = objs.begin();
                    iter != objs.end();
                    ++iter)
                {
                    if ((iter->id == opcIDs.get(entryIdx).asInt()) &&
                        (iter->name == label.c_str()))
                    {
                        idxToRemove = iter - objs.begin();
                    }
                }
                yInfo("deleting object entry %d/%s from internal short-term memory (element %d)",
                      opcIDs.get(entryIdx).asInt(), label.c_str(), idxToRemove);
                objs.erase(objs.begin() + idxToRemove);

                // TODO efficient way:
                //MemoryItemObj objToRemove(...);
                //const int idxToRemove = std::find(objs.begin(), objs.end(), objToRemove) - objs.begin();
                //objs.erase(std::remove(objs.begin(), objs.end(), objToRemove), objs.end());

                // secondly, delete from WSOPC
                yInfo("deleting object entry %d/%s from WSOPC",
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
bool WorldStateMgrThread::updateCountFrom()
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
    yInfo("reset routine: updated countFrom to %d", countFrom);

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

/*
bool WorldStateMgrThread::getTrackNames()
{
    // note: when this function fails (returns false),
    //       candidateMap must be cleared

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

        // fail if this already exists in memory or in candidateMap
        if ( memoryContainsID(id) ||
             memoryContainsName(label) ||
             mapContainsKey(candidateMap,id) ||
             mapContainsValue(candidateMap,label) )
        {
            yWarning("winning label --> %s <--, however not going to add it to memory because it seems to be a duplicate",
                     label.c_str());
            allCandidateNamesUnique = false;
            continue;
        }

        // assume names cannot change -> use insert(), not operator[]
        // http://stackoverflow.com/questions/326062/in-stl-maps-is-it-better-to-use-mapinsert-than
        candidateMap.insert(make_pair(id,label));

        if (i==trackIDs.size()-1) yInfo("done asking for visible object labels");
    }

    if (allCandidateNamesUnique)
    {
        yInfo("success, all candidate object IDs and names are unique:");
        dumpMap(candidateMap);
    }
    else
    {
        // failed initializing unique ID-name map -> clear and try again
        return false;
    }

    return true;
}
*/

/**********************************************************/
bool WorldStateMgrThread::getAffBottleIndexFromTrackROI(const int &u, const int &v, int &abi)
{
    // Finds the AffBottleIndex of inAff->get(abi) corresponding to
    // the tracker ROI coordinates u v.
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
bool WorldStateMgrThread::computeObjProperties(const int &id, const string &label,
                                               Bottle &pos2d,
                                               Bottle &desc2d, Bottle &tooldesc2d,
                                               string &inHand,
                                               Bottle &onTopOf,
                                               Bottle &reachW, Bottle &pullW)
{
    yDebug("%s begin %d/%s", __func__, id, label.c_str());

    if (activityPort.getOutputCount()<1)
        return false;

    Bottle activityCmd, activityReply;
    activityCmd.addString("get2D");
    activityCmd.addString(label.c_str());
    //yDebug() << __func__ << "sending query:" << activityCmd.toString().c_str();
    activityPort.write(activityCmd, activityReply);

    bool isVisible = activityReply.size()>0 &&
                     activityReply.get(0).isList() &&
                     activityReply.get(0).asList()->size()>0; // has valid ROI coordinates

    // begin symbols that depend on IOL tracker and shape descriptors

    double u, v;

    if (isVisible)
    {
        double tl_x = activityReply.get(0).asList()->get(0).asDouble();
        double tl_y = activityReply.get(0).asList()->get(1).asDouble();
        double br_x = activityReply.get(0).asList()->get(2).asDouble();
        double br_y = activityReply.get(0).asList()->get(3).asDouble();

        u = (tl_x + br_x) / 2.0;
        v = (tl_y + br_y) / 2.0;

        // prepare position property (pos2d)
        pos2d.clear();
        pos2d.addDouble(u); // unfiltered values
        pos2d.addDouble(v);

        // prepare 2D shape descriptors property (desc2d)
        int abi = -1; // affordance blobs Bottle index
        if (!getAffBottleIndexFromTrackROI(u,v,abi))
        {
            yWarning() << __func__
                       << "did not find affordance blob index from coordinates"
                       << u << v;
        }
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
    }
    else
        yWarning("%s is not visible", label.c_str());

    // end symbols that depend on IOL tracker and shape descriptors

    // begin symbols that depend on activityInterface only

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
    if (candidateMap.empty())
    {
        yWarning() << __func__ << "candidateMap empty";
        return false;
    }

    // for each <id,label> pair
    for (idLabelMap::const_iterator iter = candidateMap.begin();
        iter != candidateMap.end();
        ++iter)
    {
        // make sure item is not already present in memory containers
        if (memoryContainsID(iter->first) || memoryContainsName(iter->second))
        {
            yWarning("item %d/%s already present in memory containers!", iter->first, iter->second.c_str());
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

    //yDebug() << __func__ << opcID << "start";

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
        yDebug() << __func__ << "cannot construct memory item"
                   << opcID << "with associated name" << name
                   << "because this name is already present in internal short-term memory";
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

    // put list of current OPC IDs here
    Bottle opcIDs;

    // query: [ask] (all)
    Bottle opcCmd, opcCmdContent, opcReply;
    opcCmd.addVocab(Vocab::encode("ask"));
    opcCmdContent.addString("all");
    opcCmd.addList() = opcCmdContent;
    opcPort.write(opcCmd, opcReply);

    // reply: [ack] (id (11 12 13 ...))
    bool valid = false;
    valid = opcReply.size()>1 &&
            opcReply.get(0).asVocab()==Vocab::encode("ack") &&
            opcReply.get(1).isList() &&
            opcReply.get(1).asList()->size()>0 &&
            opcReply.get(1).asList()->get(0).asString()=="id" &
            opcReply.get(1).asList()->get(1).isList();

    if (!valid)
        return false;

    opcIDs = * opcReply.get(1).asList()->get(1).asList();

    // try constructing memory item from each ID (will not construct those whose IDs already exist)
    for (int o=0; o<opcIDs.size(); o++)
    {
        constructMemoryFromOPCID(opcIDs.get(o).asInt());
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

    // search in candidateMap (not yet saved to objects memory)
    for(idLabelMap::const_iterator iter = candidateMap.begin();
        iter != candidateMap.end();
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
int WorldStateMgrThread::label2id(const string &label)
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
            return iter->id;
    }

    // search in candidateMap (not yet saved to objects memory)
    for(idLabelMap::const_iterator iter = candidateMap.begin();
        iter != candidateMap.end();
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
bool WorldStateMgrThread::getLabelsFromActivityIF(Bottle &names)
{
    if (activityPort.getOutputCount()<1)
    {
        yWarning() << __func__ << "not connected to activityInterface";
        return false;
    }

    // TODO: explore using getOPCNames, which includes trained but not visible names
    // acquire getNames list
    Bottle activityCmd;
    Bottle activityReply;
    activityCmd.addString("getNames");
    activityPort.write(activityCmd, activityReply);

    bool validResponse = activityReply.size()>0 &&
                         activityReply.get(0).isList();

    if (!validResponse)
    {
        yError("%s obtained invalid response from activityInterface", __func__);
        return false;
    }

    bool somethingVisible = validResponse &&
                            activityReply.get(0).asList()->size()>0;

    if (!somethingVisible)
    {
        yWarning("%s getNames is empty", __func__);
        return false;
    }

    names = * activityReply.get(0).asList();

    return true;
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
bool WorldStateMgrThread::constructMapFromNames(const Bottle &names)
{
    if (names.size() <= 0)
        return false;

    for (int n=0; n<names.size(); ++n)
    {
        int id = countFrom+n;
        // assume names cannot change -> use insert(), not operator[]
        // http://stackoverflow.com/questions/326062/in-stl-maps-is-it-better-to-use-mapinsert-than
        yDebug("%s inserting %d/%s in candidateMap", __func__, id, names.get(n).asString().c_str());
        candidateMap.insert(make_pair(id,names.get(n).asString()));
    }

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
                const int returnedID =
                    opcReply.get(1).asList()->get(1).asInt();

                // make sure that OPC's returned id equals memory id
                if (returnedID != iter->id)
                    yError() << "ID mismatch while adding" << iter->name
                             << "to database! got" << returnedID
                             << "but was expecting" << iter->id;
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
void WorldStateMgrThread::fsm()
{
    //yDebug("state=%d", state);
    switch(state)
    {
        case STATE_WAIT_OPC:
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
                state = STATE_WAIT_ACTIVITYIF;
            }

            break;
        }

        case STATE_WAIT_ACTIVITYIF:
        {
            tellUserConnectActivityIF();
            tellActivityGoHome();

            if (toldActivityGoHome)
            {
                // proceed
                state = STATE_WAIT_SHAPEDESC;
            }

            break;
        }

        case STATE_WAIT_SHAPEDESC:
        {
            tellUserConnectBlobs();

            if(inAffPort.getInputCount()>0 && inToolAffPort.getInputCount()>0)
                tellUserBlobsConnected();

            // acquire shape descriptors
            refreshBlobs();

            // when something arrives, proceed
            if (inAff != NULL)
                state = STATE_READ_SHAPEDESC;

            break;
        }

        case STATE_READ_SHAPEDESC:
        {
            // if size>0 proceed, else go back one state
            if (sizeAff > 0)
                state = STATE_SET_MEMORY;
            else
                state = STATE_WAIT_SHAPEDESC;

            break;
        }

        case STATE_SET_MEMORY:
        {
            //yDebug("candidateMap.size()=%lu, hands.size()=%lu, objs.size()=%lu", candidateMap.size(), hands.size(), objs.size());
            if (ensureOpcNumEntries(3))
            {
                // WSOPC already initialized
                //yDebug("WSOPC has 3+ entries, it was already initialized");

                // construct memory item for each ID
                initMemoryFromOPC();

                //yDebug("candidateMap.size()=%lu, hands.size()=%lu, objs.size()=%lu", candidateMap.size(), hands.size(), objs.size());

                // proceed
                state = STATE_POPULATE_DB;
            }
            else
            {
                // WSOPC not yet initialized (it has only 2 hand entries)
                //yDebug("WSOPC only has the 2 hand entries");

                // acquire labels, if successful construct memory items from map
                Bottle names;
                if (getLabelsFromActivityIF(names))
                {
                    // fill up <ID,label> map
                    if (constructMapFromNames(names))
                    {
                        // add unique <ID,label> pairs to objs container
                        if (constructMemoryFromMap())
                        {
                            yInfo("success, added candidate object IDs and names to short-term memory");
                        }
                        else
                            yWarning("problem with constructMemoryFromMap"); // TODO: go back?

                        // proceed
                        state = STATE_POPULATE_DB;
                    }
                    else
                    {
                        // failure constructing map
                        yDebug("problem with constructMapFromNames, clearing map and trying again...");
                        candidateMap.clear();
                    }
                }
                else
                    yWarning("received empty names list from activityInterface");
            }

            break;
        }

        case STATE_POPULATE_DB:
        {
            // read new data
            if(!refreshBlobs())
                yWarning("problem with refreshBlobs");

            // populate database: if success proceed, else stay in same state
            if ( doPopulateDB() )
            {
                yInfo("initialization complete (short-term memory, WSOPC database)");
                initFinished = true;
                state = STATE_WAIT_CMD;
            }
            else
                yWarning("problem populating database");

            printMemoryState();

            break;
        }

        case STATE_WAIT_CMD:
        {
            if (needUpdate)
                state = STATE_UPDATE_DB;

            break;
        }

        case STATE_UPDATE_DB:
        {
            if (needUpdate)
            {
                // read new data
                if(!refreshBlobs())
                    yWarning("problem with refreshBlobs");

                // populate database
                if ( !doPopulateDB() )
                    yWarning() << __func__ << "problem populating database";

                needUpdate = false;
                printMemoryState();

                // go back one state
                state = STATE_WAIT_CMD;
            }

            break;
        }

        default:
        {
            break;
        }
    } // end switch
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

    // enter FSM
    state = STATE_WAIT_OPC; // TODO: check

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

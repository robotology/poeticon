/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2014 VisLab, Institute for Systems and Robotics,
 *                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Giovanni Saponaro <gsaponaro@isr.ist.utl.pt>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 */

#include "WorldStateMgrThread.h"

/**********************************************************/
WorldStateMgrThread::WorldStateMgrThread(
    const string &_moduleName,
    const double _period,
    const bool _playbackMode,
    const int _countFrom,
    const bool _withFilter)
    : moduleName(_moduleName),
      RateThread(int(_period*1000.0)),
      playbackMode(_playbackMode),
      countFrom(_countFrom),
      withFilter(_withFilter)
{
}

/**********************************************************/
bool WorldStateMgrThread::openPorts()
{
    // perception and playback modes
    bool ret = true;
    opcPortName = "/" + moduleName + "/opc:io";
    ret = ret && opcPort.open(opcPortName.c_str());
    
    if (playbackMode) return ret;

    // perception mode only
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
    // perception and playback modes
    yInfo("closing ports");
    opcPort.close();

    if (playbackMode) return;

    // perception mode only
    inTargetsPort.close();
    inAffPort.close();
    inToolAffPort.close();
    activityPort.close();
    trackerPort.close();
}

/**********************************************************/
void WorldStateMgrThread::interrupt()
{
    // perception and playback modes
    closing = true;
    yInfo("interrupting ports");
    opcPort.interrupt();

    if (playbackMode) return;

    // perception mode only
    inTargetsPort.interrupt();
    inAffPort.interrupt();
    inToolAffPort.interrupt();
    activityPort.interrupt();
    trackerPort.interrupt();
}

/**********************************************************/
bool WorldStateMgrThread::threadInit()
{
    // perception and playback modes
    closing = false;

    if ( !openPorts() )
    {
        yError("problem opening ports");
        return false;
    }

    // initialize variables common to both perception and playback modes
    initCommonVars();

    // initialize specific variables
    return ( playbackMode ? initPlaybackVars() : initPerceptionVars() );
}

/**********************************************************/
void WorldStateMgrThread::run()
{
    while (!closing)
    {
        if (!playbackMode)
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
        }
        else
        {
            // enter playback state machine
            fsmPlayback();
        }

        yarp::os::Time::delay(0.01);
    }
}

/* ************************************************************************** */
/* perception and playback modes                                              */
/* ************************************************************************** */

/**********************************************************/
bool WorldStateMgrThread::initCommonVars()
{
    fsmState = (playbackMode ? STATE_DUMMY_PARSE : STATE_PERCEPTION_WAIT_OPC);
    toldUserConnectOPC = false;
    toldUserOPCConnected = false;

    return true;
}

/**********************************************************/
bool WorldStateMgrThread::updateWorldState()
{
    if (!playbackMode)
    {
        // perception mode
        if (activityPort.getOutputCount()<1)
        {
            yWarning("cannot update world state, not connected to ActivityIF!");
            return false;
        }
        if (!checkTrackerStatus())
        {
            yWarning("tracker not initialized, cannot update world state!");
            return false;
        }
        needUpdate = true;
        yInfo("updating world state from robot perception");
    }
    else
    {
        // playback mode
        playbackPaused = false;
        yInfo("updating world state from playback file");
    }

    // TODO: opcPort.write() should be here instead of inner functions

    return true;
}

/**********************************************************/
bool WorldStateMgrThread::tellUserConnectOPC()
{
    if (!toldUserConnectOPC)
    {
        yInfo("waiting for connection: %s /wsopc/rpc", opcPortName.c_str());
        toldUserConnectOPC = true;
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

/* ************************************************************************** */
/* perception mode                                                            */
/* ************************************************************************** */

/**********************************************************/
bool WorldStateMgrThread::initPerceptionVars()
{
    inAff = NULL;
    inToolAff = NULL;
    inTargets = NULL;
    needUpdate = false;
    //memoryInit = false;
    toldUserWaitBlobs = false;
    toldUserBlobsConnected = false;
    toldUserWaitTracker = false;
    toldUserTrackerConnected = false;
    toldUserWaitActivityIF = false;
    toldActivityGoHome = false;
    toldUserActivityIFConnected = false;

    return true;
}

/**********************************************************/
bool WorldStateMgrThread::tellUserConnectBlobs()
{
    if (!toldUserWaitBlobs || inAffPort.getInputCount()<1 || inToolAffPort.getInputCount()<1)
    {
        yInfo("waiting for connections:");
        yInfo("/blobDescriptor/affDescriptor:o %s", inAffPortName.c_str());
        yInfo("/blobDescriptor/toolAffDescriptor:o %s", inToolAffPortName.c_str());
        toldUserWaitBlobs = true;
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
    if (!toldUserWaitTracker)
    {
        yInfo("waiting for connection: /activeParticleTrack/target:o %s",
              inTargetsPortName.c_str());
        toldUserWaitTracker = true;
    }

    return true;
}

/**********************************************************/
bool WorldStateMgrThread::tellUserConnectActivityIF()
{
    if (!toldUserWaitActivityIF)
    {
        yInfo("waiting for connection: %s /activityInterface/rpc:i",
              activityPortName.c_str());
        toldUserWaitActivityIF = true;
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

    if (!checkTrackerStatus()) // tracker not yet initialized
    {
        yInfo("sending instruction to tracker: countFrom %d", countFrom);
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
    }
    else // tracker already initialized
    {
        yWarning() << "tracker was already initialized and tracking -"
                   << "confirm consistency among: 1) tracker viewer"
                   << "2) trackIDs =" << trackIDs.toString()
                   << "3) memory state:";
        printMemoryState();
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

        if (i==0) yInfo("asking for object labels");

        // get fresh blob coordinates
        if (!refreshTracker())
        {
            yWarning() << __func__ << "problem with refreshTracker";
            return false;
        }

        bool safetyCheck = false;
        safetyCheck = inTargets->get(i).isList() &&
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
        if ( memoryContainsID(id) || // countFrom already prevents this
             memoryContainsName(label) ||
             mapContainsKey(candidateTrackMap,id) ||
             mapContainsValue(candidateTrackMap,label) )
        {
            yWarning("winning label --> %s <--, but not going to add it to memory because it seems to be a duplicate",
                     label.c_str());
            allCandidateNamesUnique = false;
            continue;
        }

        // assume names cannot change -> use insert(), not operator[]
        // http://stackoverflow.com/questions/326062/in-stl-maps-is-it-better-to-use-mapinsert-than
        candidateTrackMap.insert(make_pair(id,label));

        if (i==trackIDs.size()-1) yInfo("done asking for object labels");
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
bool WorldStateMgrThread::refreshBlobs()
{
    if (inAffPort.getInputCount()<1 || inToolAffPort.getInputCount()<1)
        return false;

    // update whole object descriptors
    inAff = inAffPort.read();

    if (inAff != NULL)
    {
        // number of blobs
        sizeAff = static_cast<int>( inAff->get(0).asDouble() );
    }

    // update object parts descriptors
    inToolAff = inToolAffPort.read();
    if (inToolAff != NULL)
    {
        // BlobDescriptor design prevents this to happen, but just in case:
        if ( static_cast<int>(inToolAff->get(0).asDouble()) != sizeAff )
        {
            yWarning("number of whole object descriptors differ from number of object parts descriptors!");
        }
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

    inTargets = inTargetsPort.read();

    if (inTargets==NULL)
    {
        yWarning() << __func__ << "did not receive data from tracker";
        return false;
    }

    // number of tracked objects
    sizeTargets = inTargets->size();

    return true;
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
    // assumption: activeParticleTrack is streaming a Bottle with ordered IDs:
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

    yWarning() << __func__ << "did not find track id" << id << "in Bottle";
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
    if (activityPort.getOutputCount()<1)
        return false;

    // by default the object is tracked/blobbed and we can compute all symbols
    bool currentlySeen = true;

    // if possible, refresh inTargets Bottle
    if (trackerPort.getOutputCount()>0)
        refreshTracker();

    // find the "tracker Bottle index" within inTargets Bottle that matches id
    int tbi = 0;
    if (!getTrackerBottleIndexFromID(id,tbi))
    {
        yDebug() << __func__ << "did not find track id" << id
                 << "in tracker Bottle: object not visible ->"
                 << "going to update activityInterface symbols only,"
                 << "leaving shape descriptors untouched";
        currentlySeen = false;
    }

    if (currentlySeen)
    {
        // begin symbols that depend on tracker and shape descriptors
        double u=0.0, v=0.0;
        u = inTargets->get(tbi).asList()->get(1).asDouble();
        v = inTargets->get(tbi).asList()->get(2).asDouble();

        // find the "affordance Bottle index" within inAff Bottle that matches
        // with the selected tracker blob
        int abi = tbi+1; // affordance blobs Bottle index
        if (!getAffBottleIndexFromTrackROI(u,v,abi))
            yDebug() << __func__ << "did not find affordance Bottle index";

        // prepare position property (pos2d)
        pos2d.clear();
        pos2d.addDouble(u); // unfiltered values
        pos2d.addDouble(v);

        // prepare 2D shape descriptors property (desc2d)
        const int areaIdx = 23;
        if ((inAff != NULL) && (abi >= 1) && (abi <= sizeAff))
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
            inToolAff->get(abi).asList()->get(0).asList()->size() > 0 && // TODO: actual size
            inToolAff->get(abi).asList()->get(1).asList()->size() > 0; // TODO: actual size
        if (validToolDesc)
        {
            tooldesc2d.clear();
            // add top half info: x y ar con ecc com cir sq el
            tooldesc2d.add( inToolAff->get(abi).asList()->get(0) );
            /*
            Bottle &tooldesc2d_top = tooldesc2d.addList();
            for (int z=0;
                 z<inToolAff->get(abi).asList()->get(0).asList()->size();
                 z++)
            {
                tooldesc2d_top.addDouble(
                    inToolAff->get(abi).asList()->get(0).asList()->get(z).asDouble() );
            }
            */
            // add bottom half info: x y ar con ecc com cir sq el
            tooldesc2d.add( inToolAff->get(abi).asList()->get(1) );
            /*
            Bottle &tooldesc2d_bot = tooldesc2d.addList();
            for (int z=0;
                 z<inToolAff->get(abi).asList()->get(1).asList()->size();
                 z++)
            {
                tooldesc2d_bot.addDouble(
                    inToolAff->get(abi).asList()->get(1).asList()->get(z).asDouble() );
            }
            */
            // OLD - add list with 2 lists containing top half and bottom half
            // descriptors, each having: x y ar con ecc com cir sq el
            //tooldesc2d.add( inToolAff->get(abi) );
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
            yWarning("problem translating on_top_of=%s to IDs",
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
            yWarning("problem translating reachable_with=%s to IDs",
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
            yWarning("problem translating pullable_with=%s to IDs",
                     bLabelsPulling.toString().c_str());
    }

    // end symbols that depend on activityInterface

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
        // note: this happens e.g. when WSOPC is started & initialized before WSM
        //yDebug() << __func__ << "cannot construct memory item"
        //           << opcID << "because this ID is already present in internal short-term memory";
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

    // construct memory item from each ID
    for (int o=0; o<opcIDs.size(); o++)
    {
        if (!constructMemoryFromOPCID(opcIDs.get(o).asInt()))
            yWarning() << "problem with constructMemoryFromOPCID" << opcIDs.get(o).asInt();
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
        yInfo(s.str());
    }

    for(std::vector<MemoryItemObj>::const_iterator iter = objs.begin();
        iter != objs.end();
        ++iter)
    {
        // print result of MemoryItemObj::toString()
        ostringstream s;
        s << *iter;
        yInfo(s.str());
    }

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
int WorldStateMgrThread::label2id(const string &label)
{
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

    // search in candidateTrackMap (not yet saved to objects memory)
    for(idLabelMap::const_iterator iter = candidateTrackMap.begin();
        iter != candidateTrackMap.end();
        ++iter)
    {
        if (iter->second == label)
            return iter->first;
    }

    // id not found anywhere
    yWarning("did not find id corresponding to label %s", label.c_str());
    return -1;
}

/**********************************************************/
bool WorldStateMgrThread::getLabel(const int &u, const int &v, string &label)
{
    if (activityPort.getOutputCount()<1)
    {
        yWarning() << __func__ << "not connected to ActivityIF";
        return false;
    }

    Bottle activityCmd, activityReply;
    activityCmd.addString("getLabel");
    activityCmd.addInt(u);
    activityCmd.addInt(v);
    //yDebug() << __func__ <<  "sending query:" << activityCmd.toString().c_str();
    activityPort.write(activityCmd, activityReply);

    // old response format: "winning-label"
    bool validDeterministicResponse = activityReply.get(0).isString();
    
    // new response format: ((label1 prob1) (label2 prob2) ...)
    bool validProbabilisticResponse = activityReply.get(0).isList();

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
    else if (validProbabilisticResponse)
    {
        if (activityReply.get(0).asList()->size()==0)
        {
            yWarning() << __func__ << "obtained valid but empty probabilistic response:" << activityReply.toString().c_str();
            return false;
        }
        else
        {
            yDebug() << __func__ << "obtained valid probabilistic response:" << activityReply.toString().c_str()
                                 << "-> for now selecting first label in the list";
            label = activityReply.get(0).asList()->get(0).asList()->get(0).asString();
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
        yInfo("winning label --> %s <-- with %d/%d non-empty entries after %d queries, despite being less than majority %d",
                 max_el.first.c_str(), max_el.second, votes.size(), rounds, majority);
    else
        yInfo("winning label --> %s <--", max_el.first.c_str());

    winnerLabel = max_el.first;

    return true;
}

/**********************************************************/
bool WorldStateMgrThread::isOnTopOf(const string &objName, Bottle &objBelow)
{
    if (activityPort.getOutputCount()<1)
    {
        yWarning() << __func__ << "not connected to ActivityIF";
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
        yWarning() << __func__ << "not connected to ActivityIF";
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
        yWarning() << __func__ << "not connected to ActivityIF";
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
        yWarning() << __func__ << "not connected to ActivityIF";
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
        yWarning() << __func__ << "not connected to ActivityIF";
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
            yWarning("WSOPC does not contain hand entry %d %s -> something's fishy!",
                    iter->id, iter->name.c_str());
        }
    }

    // cycle over objects memory
    for(std::vector<MemoryItemObj>::iterator iter = objs.begin();
        iter != objs.end();
        ++iter)
    {
        // compute updated properties
        Bottle pos2d;
        Bottle desc2d;
        Bottle tooldesc2d;
        string inHand;
        Bottle onTopOf;
        Bottle reachW;
        Bottle pullW;
        computeObjProperties(iter->id,iter->name,
                             pos2d,
                             desc2d,tooldesc2d,
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
            yWarning("object %d %s: was expecting is_hand=false, got is_hand=true! skipping...",
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

            yDebug("modifying existing entry in database: %d %s",
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

            yInfo("adding new entry to database: %d %s",
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
                    yWarning() << "ID mismatch while adding" << iter->name
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
                // pre-initialize tracker (countFrom 13)
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
                // start tracking blobs provided by segmentation/blobDesc
                initTracker();
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

            if (toldUserWaitActivityIF && toldActivityGoHome)
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
                yWarning() << "failure in initializing IDs and names:" <<
                              "at least one of the candidate names was" <<
                              "a duplicate or was skipped. trying again." <<
                              "if this goes on forever, check the status of" <<
                              "1) object recognition, 2) activityInterface and "
                              "3) WSOPC database.";
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
bool WorldStateMgrThread::pauseTrack(const string &objName)
{
    if (playbackMode)
    {
        yWarning("not available in playback mode, requires perception mode!");
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
    id = label2id(objName);
    if (id==-1)
    {
        yWarning() << __func__ << "did not find tracker ID corresponding to label, not going to pause" << objName.c_str();
        return false;
    }

    Bottle trackerCmd, trackerReply;
    trackerCmd.addString("pause");
    trackerCmd.addInt(id);
    trackerPort.write(trackerCmd, trackerReply);
    //yDebug() << __func__ <<  "sending query to activeParticleTracker:" << trackerCmd.toString().c_str();
    bool validResponse = false;
    validResponse = trackerReply.size()>0 &&
                    trackerReply.get(0).asVocab()==Vocab::encode("ok");
    if (!validResponse)
        yWarning() << __func__ <<  "obtained invalid response:" << trackerReply.toString().c_str();

    return true;
}

/**********************************************************/
bool WorldStateMgrThread::resumeTrack(const string &objName)
{
    if (playbackMode)
    {
        yWarning("not available in playback mode, requires perception mode!");
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
    id = label2id(objName);
    if (id==-1)
    {
        yWarning() << __func__ << "did not find tracker ID corresponding to label, not going to resume" << objName.c_str();
        return false;
    }

    Bottle trackerCmd, trackerReply;
    trackerCmd.addString("resume");
    trackerCmd.addInt(id);
    trackerPort.write(trackerCmd, trackerReply);
    //yDebug() << __func__ <<  "sending query to activeParticleTracker:" << trackerCmd.toString().c_str();
    bool validResponse = false;
    validResponse = trackerReply.size()>0 &&
                    trackerReply.get(0).asVocab()==Vocab::encode("ok");
    if (!validResponse)
        yWarning() << __func__ <<  "obtained invalid response:" << trackerReply.toString().c_str();

    return true;
}

/**********************************************************/
Bottle WorldStateMgrThread::getColorHistogram(const int32_t &u,
                                              const int32_t &v)
{
    Bottle colors;

    if (playbackMode)
    {
        yWarning("not available in playback mode, requires perception mode!");
        return colors;
    }

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

/* ************************************************************************** */
/* playback mode                                                              */
/* ************************************************************************** */

/**********************************************************/
bool WorldStateMgrThread::initPlaybackVars()
{
    toldUserEof = false;
    playbackPaused = true;
    sizePlaybackFile = -1;
    currPlayback = -1;

    return true;
}

/**********************************************************/
void WorldStateMgrThread::setPlaybackFile(const string &file)
{
    playbackFile = file;
}

/**********************************************************/
void WorldStateMgrThread::fsmPlayback()
{
    //yDebug("playback state=%d", fsmState);
    switch (fsmState)
    {
        case STATE_DUMMY_PARSE:
        {
            // acquire Bottle with whole file content
            Property wholeFile;
            wholeFile.fromConfigFile(playbackFile.c_str());
            stateBottle.read(wholeFile);
            sizePlaybackFile = stateBottle.size();
            if (sizePlaybackFile < 1)
            {
                yError("file empty or not parsable");
                fsmState = STATE_DUMMY_ERROR;
                break;
            }
            
            yDebug("file parsed successfully: %d state entries", sizePlaybackFile);
            fsmState = STATE_DUMMY_WAIT_OPC;
            break;
        }

        case STATE_DUMMY_WAIT_OPC:
        {
            tellUserConnectOPC();

            if (opcPort.getOutputCount()>0)
            {
                tellUserOPCConnected();

                // proceed
                fsmState = STATE_DUMMY_WAIT_CMD;
            }

            break;
        }

        case STATE_DUMMY_WAIT_CMD:
        {
            playbackPaused = true;

            // initially we are in "[state00]"
            currPlayback = 0;
            fsmState = STATE_DUMMY_STEP;

            break;
        }

        case STATE_DUMMY_STEP:
        {
            if (!playbackPaused)
            {
                // parse group "[state##]" of file
                // TODO: make it work for "[state##]" with ##>9, simplify
                ostringstream tag;
                tag << "state" << std::setw(2) << std::setfill('0') << currPlayback << "";
                Bottle &bCurr = stateBottle.findGroup(tag.str().c_str());
                if (bCurr.size() < 1)
                {
                    yWarning() << tag.str().c_str() << "not found";
                    playbackPaused = true;
                    fsmState = STATE_DUMMY_EOF;
                    break;
                }
                yDebug("loaded group %s, has size %d incl. group tag", tag.str().c_str(), bCurr.size());

                Bottle opcCmd, content, opcReply;
                Bottle *obj_j;

                // parse each entry/line of current group "[state##]"
                for (int j=1; j<bCurr.size(); j++)
                {
                    // going to ask WSOPC whether entry already exists
                    opcCmd.clear();
                    content.clear();
                    opcCmd.addVocab(Vocab::encode("get"));
                    obj_j = bCurr.get(j).asList();
                    opcCmd.addList() = *obj_j->get(0).asList();
                    yDebug() << __func__ <<  "sending query to WSOPC:" << opcCmd.toString().c_str();
                    opcPort.write(opcCmd, opcReply);
                    yDebug() << __func__ <<  "received response:" << opcReply.toString().c_str();

                    // stop here if no WSOPC connection
                    if (opcPort.getOutputCount()<1)
                    {
                        yWarning() << __func__ << "not connected to WSOPC";
                        break;
                    }

                    if (opcReply.get(0).asVocab() == Vocab::encode("ack"))
                    {
                        // yes -> just update entry's properties
                        yDebug("modifying existing entry in database");
                        opcCmd.clear();
                        opcCmd.addVocab(Vocab::encode("set"));
                        obj_j = bCurr.get(j).asList();
                        content.addList() = *obj_j->get(0).asList(); // ID
                        content.append(*obj_j->get(1).asList());     // propSet
                        opcCmd.addList() = content;
                        opcPort.write(opcCmd, opcReply);
                    }
                    else
                    {
                        // no -> add entry
                        yDebug("adding new entry to database");
                        opcCmd.clear();
                        opcCmd.addVocab(Vocab::encode("add"));
                        obj_j = bCurr.get(j).asList();
                        content.append(*obj_j->get(1).asList()); // propSet
                        opcCmd.addList() = content;
                        opcPort.write(opcCmd, opcReply);

                        // handle problems and inconsistencies
                        if (opcReply.get(1).asList()->get(1).asInt() !=
                            obj_j->get(0).asList()->get(1).asInt())
                        {
                            yWarning() << "ID problem:" <<
                                opcReply.get(1).asList()->get(1).asInt() <<
                                "in WSOPC database, but" <<
                                obj_j->get(0).asList()->get(1).asInt() <<
                                "in playback file";

                            //break;
                        }
                    }

                } // end for parse each entry/line

                // send "dump" instruction to WSOPC
                opcCmd.clear();
                opcReply.clear();
                opcCmd.addVocab(Vocab::encode("dump"));
                //yDebug() << __func__ << "sending command to WSOPC:" << opcCmd.toString().c_str();
                opcPort.write(opcCmd, opcReply);
                //yDebug() << __func__ << "received response:" << opcReply.toString().c_str();
                opcCmd.clear();
                opcReply.clear();

                ++currPlayback;
                playbackPaused = true;
            } // end if (!playbackPaused)

            // stay in same state, wait for next update instruction over rpc

            break;
        }

        case STATE_DUMMY_EOF:
        {
            if (!toldUserEof)
                yInfo("finished reading from playback file");

            toldUserEof = true;
            break;        
        }

        case STATE_DUMMY_ERROR:
        {
            closing = true;
            yError("please quit the module");
            break;        
        }

        default:
        {
            yWarning("unknown FSM state");
            break;
        }
    } // end switch
}

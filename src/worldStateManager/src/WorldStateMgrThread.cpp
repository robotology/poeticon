/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2014 VisLab, Institute for Systems and Robotics,
 *                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Giovanni Saponaro <gsaponaro@isr.ist.utl.pt>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 */

#include "WorldStateMgrThread.h"

WorldStateMgrThread::WorldStateMgrThread(
    const string &_moduleName,
    const double _period,
    bool _playbackMode,
    const int _countFrom)
    : moduleName(_moduleName),
      RateThread(int(_period*1000.0)),
      playbackMode(_playbackMode),
      countFrom(_countFrom)
{
}

bool WorldStateMgrThread::openPorts()
{
    // perception and playback modes
    opcPortName = "/" + moduleName + "/opc:io";
    opcPort.open(opcPortName.c_str());
    
    if (playbackMode) return true;

    // perception mode only
    inTargetsPortName = "/" + moduleName + "/target:i";
    inTargetsPort.open(inTargetsPortName.c_str());

    inAffPortName = "/" + moduleName + "/affDescriptor:i";
    inAffPort.open(inAffPortName.c_str());

    inToolAffPortName = "/" + moduleName + "/toolAffDescriptor:i";
    inToolAffPort.open(inToolAffPortName.c_str());

    activityPortName = "/" + moduleName + "/activity:rpc";
    activityPort.open(activityPortName.c_str());

    trackerPortName = "/" + moduleName + "/tracker:rpc";
    trackerPort.open(trackerPortName.c_str());

    return true;
}

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

void WorldStateMgrThread::run()
{
    while (!closing)
    {
        if (!playbackMode)
        {
            // enter perception state machine
            fsmPerception();
        }
        else
        {
            // enter playback state machine
            fsmPlayback();
        }
    }
}

/* ************************************************************************** */
/* perception and playback modes                                              */
/* ************************************************************************** */

bool WorldStateMgrThread::initCommonVars()
{
    fsmState = (playbackMode ? STATE_DUMMY_PARSE : STATE_PERCEPTION_WAIT_OPC);
    toldUserConnectOPC = false;
    toldUserOPCConnected = false;

    return true;
}

bool WorldStateMgrThread::dumpWorldState()
{
    // TODO: remove refresh*() calls

    if (opcPort.getOutputCount() < 1)
    {
        yWarning() << __func__ << "not connected to WSOPC";
        return false;
    }

    refreshOPC();

    if (!playbackMode)
    {
        // perception mode
        mergeMaps(opcMap, trackMap, wsMap);
        if (trackerInit)
            refreshTracker();
    }
    else
    {
        // playback mode
        mergeMaps(opcMap, wsMap, wsMap);
    }

    yInfo("world state map:");
    dumpMap(wsMap);

    return true;
}

bool WorldStateMgrThread::updateWorldState()
{
    if (!playbackMode)
    {
        // perception mode
        if (activityPort.getOutputCount() < 1)
        {
            yWarning("cannot update world state, not connected to ActivityIF!");
            return false;
        }
        if (!trackerInit)
        {
            yWarning("cannot update world state, tracker not initialized!");
            return false;
        }
        needUpdate = true;
        refreshPerceptionAndValidate(); // TODO: redundant?
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

/* ************************************************************************** */
/* perception mode                                                            */
/* ************************************************************************** */

bool WorldStateMgrThread::initPerceptionVars()
{
    inAff = NULL;
    inToolAff = NULL;
    inTargets = NULL;
    needUpdate = false;
    trackerInit = false;
    toldUserWaitBlobs = false;
    toldUserBlobsConnected = false;
    toldUserWaitTracker = false;
    toldUserTrackerConnected = false;
    toldUserWaitActivityIF = false;
    toldActivityGoHome = false;
    toldUserActivityIFConnected = false;

    return true;
}

bool WorldStateMgrThread::initTracker()
{
    if (trackerPort.getOutputCount() < 1)
    {
        yWarning() << __func__ << "exiting," << trackerPortName.c_str()
                   << "not connected to /activeParticleTrack/rpc:i";
        return false;
    }

    yInfo("initializing tracking of %d objects:", sizeAff);
    Bottle trackerCmd;
    Bottle trackerReply;
    double u=0.0, v=0.0;

    for(int a=0; a<sizeAff; a++)
    {
        u = inAff->get(a+1).asList()->get(0).asDouble();
        v = inAff->get(a+1).asList()->get(1).asDouble();

        trackerCmd.clear();
        trackerReply.clear();

        trackerCmd.clear();
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
    yInfo("done initializing tracker");

    return true;
}

void WorldStateMgrThread::fsmPerception()
{
    //yDebug("perception state=%d", fsmState);
    switch(fsmState)
    {
        case STATE_PERCEPTION_WAIT_OPC:
        {
            if (!toldUserConnectOPC && opcPort.getInputCount()<1)
            {
                yInfo("waiting for %s to be connected to /wsopc/rpc", opcPortName.c_str());
                toldUserConnectOPC = true;
            }

            if (!toldUserOPCConnected && opcPort.getInputCount()>=1)
            {
                yInfo("connected to WSOPC, you can now send RPC commands to /%s/rpc:i", moduleName.c_str());
                toldUserOPCConnected = true;
            }

            if (opcPort.getOutputCount()>=1)
            {
                dumpWorldState();
                // proceed
                fsmState = STATE_PERCEPTION_WAIT_BLOBS;
            }

            break;
        }

        case STATE_PERCEPTION_WAIT_BLOBS:
        {
            // acquire initial entries (robot hands) from WSOPC database
            refreshOPC();

            if (!toldUserWaitBlobs || inAffPort.getInputCount()<1 || inToolAffPort.getInputCount()<1)
            {
                yInfo("waiting for connections to BlobDescriptor:");
                yInfo("/blobDescriptor/affDescriptor:o %s", inAffPortName.c_str());
                yInfo("/blobDescriptor/toolAffDescriptor:o %s", inToolAffPortName.c_str());
                toldUserWaitBlobs = true;
            }

            if (!toldUserBlobsConnected && inAffPort.getInputCount()>=1 && inToolAffPort.getInputCount()>=1)
            {
                yInfo("connected to BlobDescriptor, waiting for shape data - requires segmentation");
                toldUserBlobsConnected = true;
            }

            // wait for blobs data to arrive
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
            if (!toldUserWaitTracker && inTargetsPort.getOutputCount()<1)
            {
                yInfo("waiting for %s to be connected to /activeParticleTrack/target:o",
                      inTargetsPortName.c_str());
                toldUserWaitTracker = true;
            }

            // TODO: check for !initTracker here
            if (!toldUserTrackerConnected && inTargetsPort.getInputCount()>=1)
            {
                yInfo("connected to tracker, sending it instruction: countFrom %d", countFrom);
                toldUserTrackerConnected = true;
                Bottle trackerCmd, trackerReply;
                trackerCmd.clear();
                trackerReply.clear();
                trackerCmd.addString("countFrom");
                trackerCmd.addInt(countFrom);
                //yDebug() << __func__ <<  "sending query to tracker:" << trackerCmd.toString().c_str();
                trackerPort.write(trackerCmd, trackerReply);
                //yDebug() << __func__ <<  "obtained response:" << trackerReply.toString().c_str();
                bool validResponse = false;
                validResponse = ( (trackerReply.size()>0) &&
                                  (trackerReply.get(0).asVocab()==Vocab::encode("ok")) );
                if (!validResponse)
                    yWarning() << __func__ << "obtained invalid response from tracker:" << trackerReply.toString().c_str();
            }

            // TODO: this should be sent once as soon as activityPort is connected
            if (!toldActivityGoHome && activityPort.getOutputCount()>=1)
            {
                Bottle activityCmd, activityReply;
                activityCmd.clear();
                activityReply.clear();
                activityCmd.addString("goHome");
                //yDebug() << __func__ <<  "sending query to activityInterface:" << activityCmd.toString().c_str();
                activityPort.write(activityCmd, activityReply);
                //yDebug() << __func__ <<  "obtained response:" << activityReply.toString().c_str();
                bool validResponse = false;
                validResponse = ( activityReply.size()>0 &&
                                  activityReply.get(0).asVocab()==Vocab::encode("ok") );

                if (!validResponse)
                    yWarning() << __func__ <<  "obtained invalid response:" << activityReply.toString().c_str();

                toldActivityGoHome = true;
            }

            // proceed
            fsmState = STATE_PERCEPTION_INIT_TRACKER;

            break;
        }

        case STATE_PERCEPTION_INIT_TRACKER:
        {
            // if tracker was already initialized, do nothing in this state
            if (inTargetsPort.getInputCount()>=1)
            {
                Bottle trackerCmd, trackerReply;
                trackerCmd.clear();
                trackerReply.clear();
                trackerCmd.addString("getIDs");
                //yDebug() << __func__ <<  "sending query to tracker:" << trackerCmd.toString().c_str();
                trackerPort.write(trackerCmd, trackerReply);
                //yDebug() << __func__ <<  "obtained response:" << trackerReply.toString().c_str();
                bool validResponse = false;
                validResponse = ( trackerReply.size()>0 &&
                                  trackerReply.get(0).isList() &&
                                  trackerReply.get(0).asList()->size()>0 );

                if (validResponse)
                {
                    yInfo() << "tracker is already initialized, it contains IDs:" << trackerReply.get(0).asList()->toString();
                    trackerInit = true;
                }
            }

            if (!trackerInit)
            {
                initTracker();
                trackerInit = true;
            }

            if (trackerInit)
            {
                // proceed
                fsmState = STATE_PERCEPTION_READ_TRACKER;
            }

            break;
        }

        case STATE_PERCEPTION_READ_TRACKER:
        {
            // wait for tracker data to arrive
            refreshTracker(); // internally checks for !=NULL

            // if size>0 proceed, else stay in same state
            if (sizeTargets > 0)
                fsmState = STATE_PERCEPTION_WAIT_ACTIVITYIF;

            break;
        }

        case STATE_PERCEPTION_WAIT_ACTIVITYIF:
        {
            if (!toldUserWaitActivityIF && activityPort.getOutputCount()<1)
            {
                yInfo("waiting for %s to be connected to /activityInterface/rpc:i",
                      activityPortName.c_str());
                toldUserWaitActivityIF = true;
            }

            if (!toldUserActivityIFConnected && activityPort.getOutputCount()>=1)
            {
                yInfo("connected to ActivityInterface");
                toldUserActivityIFConnected = true;
            }

            if (opcPort.getOutputCount()<1 || inAffPort.getOutputCount()<1 ||
                inTargetsPort.getOutputCount()<1 || activityPort.getOutputCount()<1)
            {
                // one of the inputs is missing, go back to beginning
                fsmState = STATE_PERCEPTION_WAIT_OPC;
            }

            // TODO: make sure "connected to ActivityInterface" is printed

            // proceed
            fsmState = STATE_PERCEPTION_POPULATE_DB;

            break;
        }

        case STATE_PERCEPTION_POPULATE_DB:
        {
            // read new data and ensure validity
            refreshPerceptionAndValidate();
            mergeMaps(opcMap, trackMap, wsMap);

            // populate database: if success proceed, else stay in same state
            if ( doPopulateDB() )
            {
                //yDebug() << __func__ << "successfully populated database";
                fsmState = STATE_PERCEPTION_WAIT_CMD;
            }
            else
                yWarning() << __func__ << "problem populating database";

            dumpWorldState();

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
                // read new data and ensure validity
                refreshPerceptionAndValidate();
                mergeMaps(opcMap, trackMap, wsMap);

                // populate database
                if ( !doPopulateDB() )
                    yWarning() << __func__ << "problem populating database";

                needUpdate = false;
                dumpWorldState();

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

void WorldStateMgrThread::refreshOPC()
{
    if (opcPort.getOutputCount()>0)
    {
        refreshOPCIDs();
        refreshOPCNames();
    }
}

void WorldStateMgrThread::refreshOPCIDs()
{
    // assume opcPort.getOutputCount()>0

    // query: [ask] (all)
    Bottle opcCmd, opcCmdContent, opcReply;
    opcCmd.addVocab(Vocab::encode("ask"));
    opcCmdContent.addString("all");
    opcCmd.addList() = opcCmdContent;
    opcPort.write(opcCmd, opcReply);

    // reply: [ack] (id (11 12 ...))
    if (opcReply.size() > 1)
    {
        if (opcReply.get(0).asVocab()==Vocab::encode("ack") &&
            opcReply.get(1).asList()->get(0).asString()=="id")
        {
            opcIDs.clear();
            Bottle *currIDs = opcReply.get(1).asList()->get(1).asList();
            for (int o=0; o<currIDs->size(); o++)
            {
                opcIDs.push_back(currIDs->get(o).asInt());
            }
        }
        else
            yDebug() << __func__ << "did not receive ack from WSOPC";
    }
}

void WorldStateMgrThread::refreshOPCNames()
{
    // inserts in opcMap
    // assume opcPort.getOutputCount()>0

    // adapted from iolHelper by Ugo Pattacini
    Bottle opcCmd, opcReplyProp;

    // cycle over items
    for (int i=0; i<opcIDs.size(); i++)
    {
        int id=opcIDs[i];

        // get the relevant properties
        // [get] (("id" <num>) ("propSet" ("name")))
        opcCmd.clear();
        opcCmd.addVocab(Vocab::encode("get"));
        Bottle &content=opcCmd.addList();
        Bottle &list_bid=content.addList();
        list_bid.addString("id");
        list_bid.addInt(id);
        Bottle &list_propSet=content.addList();
        list_propSet.addString("propSet");
        list_propSet.addList().addString("name");
        opcPort.write(opcCmd,opcReplyProp);

        // get name as propField->find("name") (if if exists)
        if (opcReplyProp.get(0).asVocab()==Vocab::encode("ack"))
           if (Bottle *propField=opcReplyProp.get(1).asList())
               if (propField->check("name"))
               {
                   // fill map
                   // assume names cannot change -> use insert(), not operator[]
                   // http://stackoverflow.com/questions/326062/in-stl-maps-is-it-better-to-use-mapinsert-than
                   opcMap.insert(make_pair(id, propField->find("name").asString().c_str()));
               }
    }
}

void WorldStateMgrThread::refreshTrackNames()
{
    // inserts in trackMap

    // cycle over items
    for (int i=0; i<trackIDs.size(); i++)
    {
        int id=trackIDs[i];

        if ( trackMap.find(id)==trackMap.end() )
        {
            // name not found -> ask ActivityIF for label
            if (activityPort.getOutputCount() < 1)
            {
                yWarning() << __func__ << "cannot ask for label, not connected to ActivityIF!";
                return;
            }

            // assume names cannot change -> use insert(), not operator[]
            // http://stackoverflow.com/questions/326062/in-stl-maps-is-it-better-to-use-mapinsert-than
            // TODO: make sure id corresponds to get(i)
            int u = static_cast<int>( inTargets->get(i).asList()->get(1).asDouble() );
            int v = static_cast<int>( inTargets->get(i).asList()->get(2).asDouble() );
            string label;
            if (!getLabelMajorityVote(u, v, label))
            {
                yWarning() << __func__ << "error calling getLabelMajorityVote";
                // try again, another 10 times
                //if (!getLabelMajorityVote(u, v, label, 10))
                //    yWarning() << __func__ << "tried getLabelMajorityVote 5+10 times, no valid result";
            }

            trackMap.insert(make_pair(id,label));
        }
        // else name found -> don't ask for it again (assume they cannot change)
    }
}

void WorldStateMgrThread::refreshBlobs()
{
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
        if ( static_cast<int>(inToolAff->get(0).asDouble()) != sizeAff )
        {
            // BlobDescriptor design prevents this to happen, but just in case:
            yWarning("number of whole object descriptors differ from number of object parts descriptors!");
        }
    }
}

void WorldStateMgrThread::refreshTracker()
{
    inTargets = inTargetsPort.read();

    if (inTargets != NULL)
    {
        // number of tracked objects
        sizeTargets = inTargets->size();

        // update trackIDs and trackMap labels (if IDs changed)
        std::vector<int> old_trackIDs(trackIDs);
        updateTrackIDsNoDupes();
        if ( vectorsDiffer(old_trackIDs,trackIDs) )
        {
            if (activityPort.getOutputCount()>=1)
            {
                // get labels, update trackMap
                refreshTrackNames();
            }
        }
    }
    else
    {
        yWarning() << __func__ << "did not receive data from tracker";
    }
}

void WorldStateMgrThread::updateTrackIDsNoDupes()
{
    // TODO: simplify by using activityIF getIDs (including paused IDs)

    trackIDs.clear();
    for (int t=0; t<sizeTargets; t++)
    {
        trackIDs.push_back( inTargets->get(t).asList()->get(0).asDouble() );
    }

    /*
    // remove duplicates
    std::sort( trackIDs.begin(),trackIDs.end() );
    trackIDs.erase( std::unique(trackIDs.begin(),trackIDs.end()), trackIDs.end() );
    */
}

void WorldStateMgrThread::refreshPerception()
{
    refreshBlobs();
    refreshTracker();
}

bool WorldStateMgrThread::refreshPerceptionAndValidate()
{
    refreshPerception();

    if (inAff==NULL || inTargets==NULL)
    {
        yWarning("no data");
        return false;
    }

    if (sizeAff != sizeTargets)
    {
        // this happens, but is compensated by getAffBottleIndexFromTrackROI
        //yWarning("sizeAff=%d differs from sizeTargets=%d", sizeAff, sizeTargets);
        ;
    }

    return true;
}

// http://stackoverflow.com/a/29798
inline const char * const BoolToString(bool &b)
{
    return b ? "true" : "false";
}

bool WorldStateMgrThread::doPopulateDB()
{
    // cycle over wsMap key IDs
    for (idLabelMap::const_iterator iter = wsMap.begin();
        iter != wsMap.end();
        ++iter)
    {
        // common properties
        Bottle bName;
        Bottle bIsHand;

        // object properties
        Bottle bPos;
        Bottle bOffset;
        Bottle bDesc;
        Bottle bToolDesc;
        Bottle bInHand;
        Bottle bOnTopOf;
        Bottle bReachW;
        Bottle bPullW;

        // hand properties
        Bottle bIsFree;

        int wsID = iter->first;
        yDebug("going to update world state id %d", wsID);

        bool bIsHandValue = false; // by default it is an object
        bool currentlySeen = true; // by default it is tracked/blobbed thus we will compute all symbols
        int tbi = 0; // tracker Bottle index
        int abi = tbi+1; // affordance blobs Bottle index
        if ( getTrackerBottleIndexFromID(wsID, tbi) )
        {
            // present in WSOPC and in tracker -> currently tracker object
            //yDebug("corresponds to tracker Bottle index %d", tbi);
            ;
        }
        else
        {
            if (opcMap.count(wsID) && !trackMap.count(wsID))
            {
                // present in WSOPC but not in tracker -> robot hand
                bIsHandValue = true;
            }
            else
            {
                yDebug() << __func__ << "did not find track id" << wsID
                         << "in tracker Bottle -> going to update the ActivityIF symbols only, then skip to next id";
                currentlySeen = false;
            }
        }

        if (currentlySeen)
        {
            if (!bIsHandValue)
            {
                double u=0.0, v=0.0;
                u = inTargets->get(tbi).asList()->get(1).asDouble();
                v = inTargets->get(tbi).asList()->get(2).asDouble();

                if ( getAffBottleIndexFromTrackROI(u, v, abi) )
                    //yDebug("corresponds to affordance blobs Bottle index %d", abi);
                    ;
                else
                    yWarning("problem with getAffBottleIndexFromTrackROI");
            }
        }

        // prepare name property
        bName.addString("name");
        string bNameValue;
        if (!bIsHandValue)
        {
            // object
            bNameValue = iter->second;
            //yDebug("name found in short-term memory (from tracker): %s", bNameValue.c_str());

            // in theory this should never happen!
            // if name not found in wsMap -> ask ActivityIF
            //if (!getLabel(u, v, bNameValue))
            //    yWarning() << __func__ << "got invalid label";
        }
        else
        {
            // robot hand -> get name from WSOPC
            bNameValue = opcMap[wsID];
            //yDebug("name found in short-term memory (from WSOPC): %s", bNameValue.c_str());
        }
        bName.addString(bNameValue.c_str());
        // override empty labels - prevents error
        // yarp: BottleImpl reader failed, unrecognized object code 25
        if (bNameValue=="")
        {
            yWarning("overriding empty label with default value");
            bNameValue = "default";
        }

        // prepare is_hand property
        bIsHand.addString("is_hand");
        bIsHand.addString( BoolToString(bIsHandValue) );

        if (!bIsHandValue)
        {
            // object properties

            double u=0.0, v=0.0;
            if (currentlySeen)
            {
                u = inTargets->get(tbi).asList()->get(1).asDouble();
                v = inTargets->get(tbi).asList()->get(2).asDouble();
            }

            if (currentlySeen)
            {
                // prepare position property
                bPos.addString("pos");
                Bottle &bPosValue = bPos.addList();
                double x=0.0, y=0.0, z=0.0;
                mono2stereo(bNameValue.c_str(), x, y, z);
                bPosValue.addDouble(x);
                bPosValue.addDouble(y);
                bPosValue.addDouble(z);

                // median-filtered position
                yarp::sig::Vector pos(3);
                pos[0]=x; pos[1]=y; pos[2]=z;
                if (posFilter.size()<=wsID-countFrom) // TODO: more robust check
                {
                //    yDebug("adding median filter %d to vector", wsID-countFrom);
                    size_t order = 1;
                    iCub::ctrl::MedianFilter newFilter(order, pos);
                    posFilter.push_back(newFilter);
                }
                //else
                //    yDebug("I think that filter %d is already present in container", wsID-countFrom);
                yarp::sig::Vector posFiltered = posFilter[wsID-countFrom].filt(pos);
                yDebug() << "pos:" << pos.toString().c_str() << "\tposFiltered:" << posFiltered.toString().c_str();
            }

            // prepare offset property (end-effector transform when grasping tools)
            bOffset.addString("offset");
            Bottle &bOffsetValue = bOffset.addList();
            getTooltipOffset(bNameValue.c_str(), bOffsetValue);

            if (currentlySeen)
            {
                // prepare 2D shape descriptors property
                bDesc.addString("desc2d");
                Bottle &bDescValue = bDesc.addList();
                int areaIdx = 23;

                if ((inAff != NULL) && (abi >= 1) && (abi <= sizeAff))
                {
                    bDescValue.addDouble(inAff->get(abi).asList()->get(areaIdx).asDouble()); // area
                    bDescValue.addDouble(inAff->get(abi).asList()->get(areaIdx+1).asDouble()); // conv
                    bDescValue.addDouble(inAff->get(abi).asList()->get(areaIdx+2).asDouble()); // ecc
                    bDescValue.addDouble(inAff->get(abi).asList()->get(areaIdx+3).asDouble()); // comp
                    bDescValue.addDouble(inAff->get(abi).asList()->get(areaIdx+4).asDouble()); // circ
                    bDescValue.addDouble(inAff->get(abi).asList()->get(areaIdx+5).asDouble()); // sq
                }
                else
                    yWarning("problem reading descriptors of whole object");

                // prepare 2D tool-object parts (top and bottom) property
                bToolDesc.addString("tooldesc2d");
                if ((inToolAff != NULL) && (abi >= 1) && (abi <= sizeAff))
                {
                    // add list with 2 lists containing top half and bottom half
                    // descriptors, each having: x y ar con ecc com cir sq el
                    bToolDesc.add( inToolAff->get(abi) );
                }
                else
                    yWarning("problem reading descriptors of object parts");
            }

            // prepare in_hand property (none/left/right)
            bInHand.addString("in_hand");
            string bInHandValue = inWhichHand(bNameValue.c_str());
            bInHand.addString(bInHandValue);

            // prepare on_top_of property
            bOnTopOf.addString("on_top_of");
            Bottle &bOnTopOfValue = bOnTopOf.addList(); // IDs
            Bottle bLabelsBelow; // strings
            isOnTopOf(bNameValue.c_str(), bLabelsBelow);
            for (int o=0; o<bLabelsBelow.size(); o++)
            {
                int id;
                id = label2id( bLabelsBelow.get(o).asString().c_str() );
                if (id != -1)
                    bOnTopOfValue.addInt( id );
            }

            // prepare reachable_with property
            bReachW.addString("reachable_with");
            Bottle &bReachWValue = bReachW.addList(); // IDs
            Bottle bLabelsReaching; // strings
            isReachableWith(bNameValue.c_str(), bLabelsReaching);
            for (int o=0; o<bLabelsReaching.size(); o++)
            {
                int id;
                id = label2id( bLabelsReaching.get(o).asString().c_str() );
                if (id != -1)
                    bReachWValue.addInt( id );
            }

            // prepare pullable_with property
            bPullW.addString("pullable_with");
            Bottle &bPullWValue = bPullW.addList(); // IDs
            Bottle bLabelsPulling; // strings
            isPullableWith(bNameValue.c_str(), bLabelsPulling);
            for (int o=0; o<bLabelsPulling.size(); o++)
            {
                int id;
                id = label2id( bLabelsPulling.get(o).asString().c_str() );
                if (id != -1)
                    bPullWValue.addInt( id );
            }
        }
        else
        {
            // hand properties

            // prepare is_free property
            bIsFree.addString("is_free");
            bool bIsFreeValue = isHandFree(bNameValue.c_str());
            bIsFree.addString( BoolToString(bIsFreeValue) );
        }

        // going to ask WSOPC whether entry already exists
        // [get] ("id" <num>)
        Bottle opcCmd, opcCmdContent, opcReply;
        opcCmd.clear();
        opcCmdContent.clear();
        opcReply.clear();
        opcCmd.addVocab(Vocab::encode("get"));
        opcCmdContent.addString("id");
        opcCmdContent.addInt(wsID);
        opcCmd.addList() = opcCmdContent;
        //yDebug() << __func__ << "sending query:" << opcCmd.toString().c_str();
        opcPort.write(opcCmd, opcReply);
        //yDebug() << __func__ << "obtained response:" << opcReply.toString().c_str();

        // TODO: factorize common parts inside the next two cases
        if (opcReply.get(0).asVocab() == Vocab::encode("ack"))
        {
            // yes -> just update entry's properties
            // [set] (("id" <num>) ("prop0" <val0>) ...) 
            //yDebug("modifying existing entry %d in database", wsID);
            opcCmd.clear();
            opcCmdContent.clear();
            opcReply.clear();
            opcCmd.addVocab(Vocab::encode("set"));

            Bottle bID;
            bID.clear();
            bID.addString("id");
            bID.addInt(wsID);
            opcCmdContent.addList() = bID;

            opcCmdContent.addList() = bName;
            opcCmdContent.addList() = bIsHand;
            if (!bIsHandValue)
            {
                if (currentlySeen)
                {
                    opcCmdContent.addList() = bPos;
                }
                opcCmdContent.addList() = bOffset;
                if (currentlySeen)
                {
                    opcCmdContent.addList() = bDesc;
                    opcCmdContent.addList() = bToolDesc;
                }
                opcCmdContent.addList() = bInHand;
                opcCmdContent.addList() = bOnTopOf;
                opcCmdContent.addList() = bReachW;
                opcCmdContent.addList() = bPullW;
            }
            else
            {
                opcCmdContent.addList() = bIsFree;
            }

            opcCmd.addList() = opcCmdContent;
        }
        else
        {
            // no -> add entry
            // [add] (("prop0" <val0>) ("prop1" <val1>) ...)
            //yDebug("adding new entry %d to database", wsID);
            opcCmd.clear();
            opcCmdContent.clear();
            opcReply.clear();
            opcCmd.addVocab(Vocab::encode("add"));

            opcCmdContent.addList() = bName;
            opcCmdContent.addList() = bIsHand;
            if (!bIsHandValue)
            {
                if (currentlySeen)
                {
                    opcCmdContent.addList() = bPos;
                }
                opcCmdContent.addList() = bOffset;
                if (currentlySeen)
                {
                    opcCmdContent.addList() = bDesc;
                    opcCmdContent.addList() = bToolDesc;
                }
                opcCmdContent.addList() = bInHand;
                opcCmdContent.addList() = bOnTopOf;
                opcCmdContent.addList() = bReachW;
                opcCmdContent.addList() = bPullW;
            }
            else
            {
                opcCmdContent.addList() = bIsFree;
            }

            opcCmd.addList() = opcCmdContent;
        }

        //yDebug() << __func__ << "sending command to WSOPC:" << opcCmd.toString().c_str();
        opcPort.write(opcCmd, opcReply);
        //yDebug() << __func__ << "received response:" << opcReply.toString().c_str();

        // process WSOPC response
        if (opcReply.size() > 1)
        {
            if (! opcReply.get(0).asVocab()==Vocab::encode("ack"))
                yWarning() << __func__ << "did not receive ack from WSOPC";
        }
    }

    mergeMaps(opcMap, trackMap, wsMap);

    // send "dump" instruction to WSOPC
    Bottle opcCmd, opcReply;
    opcCmd.addVocab(Vocab::encode("dump"));
    //yDebug() << __func__ << "sending command to WSOPC:" << opcCmd.toString().c_str();
    opcPort.write(opcCmd, opcReply);
    //yDebug() << __func__ << "received response:" << opcReply.toString().c_str();
    opcCmd.clear();
    opcReply.clear();

    return true;
}

bool WorldStateMgrThread::getTrackerBottleIndexFromID(const int &id, int &tbi)
{
    // assumption: activeParticleTrack is streaming a Bottle with ordered IDs:
    // ((13 ...) (14 ...) (15 ...))
    // TODO: verify that this is the case, else reorder

    // first, check that id key exists in wsMap
    if ( wsMap.find(id)==wsMap.end() )
    {
        // key not found
        yWarning() << __func__ << "did not find key id" << id << "in wsMap";
    }
    else
    {
        // key found
        // temporarily store all the IDs exposed by tracker
        // TODO: need to check (inTargets != NULL) ?
        std::vector<int> curr_tracks;
        for (int t=0; t<sizeTargets; ++t)
        {
            curr_tracks.push_back(
                static_cast<int>( inTargets->get(t).asList()->get(0).asDouble() )
            );
        }
        std::vector<int>::iterator iter;
        iter = find(curr_tracks.begin(), curr_tracks.end(), id);
        if ( iter != curr_tracks.end() )
        {
            tbi = iter - curr_tracks.begin();
            //yDebug() << __func__ << "found track" << id << "at Bottle index" << tbi;
            return true;
        }
        //else
        //    yWarning() << __func__ << "did not find track id" << id << "in Bottle";
    }

    return false;
}

bool WorldStateMgrThread::getAffBottleIndexFromTrackROI(const int &u, const int &v, int &abi)
{
    // Finds the AffBottleIndex of inAff->get(abi) corresponding to
    // the TrackerBottleIndex inTargets->get(tbi).
    //
    // Current implementation: from tracker ROI centre, detect the most likely
    // blob index within blobDescriptor Bottle, using Euclidean distance.
    //
    // Future, more robust implementation: cv::pointPolygonTest.

    // iterates over pairs
    //bPosValue.addDouble(inAff->get(*+1).asList()->get(0).asDouble());
    //bPosValue.addDouble(inAff->get(*+1).asList()->get(1).asDouble());
    // and return closest one (in Euclidean distance sense) to u,v

    yarp::sig::Vector trackerROI(2, 0.0);
    trackerROI[0] = static_cast<double>( u );
    trackerROI[1] = static_cast<double>( v );
    //yDebug() << "trackerROI" << trackerROI.toString().c_str();

    int minBlobIdx = -1;
    float minDist = 1000.0; // minimum distance found so far
    float distThr = 50.0; // minimum required distance to update ABI

    for(int a=1; a<=sizeAff; a++)
    {
        yarp::sig::Vector currentBlob(2, 0.0);
        currentBlob[0] = inAff->get(a).asList()->get(0).asDouble();
        currentBlob[1] = inAff->get(a).asList()->get(1).asDouble();
        float dist = 0.0;
        euclideanDistance(trackerROI, currentBlob, dist);

        //yDebug() << "currentBlob" << a << currentBlob.toString().c_str();
        //yDebug() << "dist" << dist;

        if (dist < distThr && dist < minDist)
        {
            minBlobIdx = a;
            minDist = dist;
        }
    }
    //yDebug("winner %d (dist=%f)", minBlobIdx, minDist);

    abi = minBlobIdx;
    return true;
}

int WorldStateMgrThread::label2id(const string &label)
{
    //yDebug() << __func__ << "looking for label" << label.c_str();
    int key = -1;

    idLabelMap::const_iterator iter;
    for (iter = wsMap.begin(); iter != wsMap.end(); ++iter)
    {
        if (iter->second == label)
        {
            key = iter->first;
            //yDebug() << __func__ << "label" << label.c_str() << "corresponds to" << key;
            break;
        }
    }

    //if (key == -1)
    //    yWarning() << __func__ << "did not find id corresponding to label" << label.c_str();

    return key;
}

bool WorldStateMgrThread::getLabel(const int &u, const int &v, string &label)
{
    if (activityPort.getOutputCount() < 1)
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

    bool validResponse = false;
    validResponse = activityReply.get(0).isString();

    if (validResponse)
    {
        if (activityReply.get(0).asString().size()==0)
            yWarning() << __func__ << "obtained valid but empty response:" << activityReply.toString().c_str();
        //else
            //yDebug() << __func__ << "obtained valid response:" << activityReply.toString().c_str();
        label = activityReply.get(0).asString(); 
        return true;
    }
    else
    {
        yWarning() << __func__ << "obtained invalid response:" << activityReply.toString().c_str();
        return false;
    }
}

struct compareSecond
{
    template <typename Pair>
    bool operator()(const Pair &a, const Pair &b)
    {
        return a.second < b.second;
    }
};

bool WorldStateMgrThread::getLabelMajorityVote(const int &u, const int &v, string &winnerLabel, const int &rounds)
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
        yWarning("selected winning label --> %s <-- with %d/%d non-empty entries after %d queries, despite being less than majority %d",
                 max_el.first.c_str(), max_el.second, votes.size(), rounds, majority);
    else
        yDebug("selected winning label --> %s <--", max_el.first.c_str());

    winnerLabel = max_el.first;

    return true;
}

bool WorldStateMgrThread::mono2stereo(const string &objName, double &x, double &y, double &z)
{
    if (activityPort.getOutputCount() < 1)
    {
        yWarning() << __func__ << "not connected to ActivityIF";
        return false;
    }

    Bottle activityCmd, activityReply;
    activityCmd.addString("get3D");
    activityCmd.addString(objName);
    //yDebug() << __func__ <<  "sending query:" << activityCmd.toString().c_str();
    activityPort.write(activityCmd, activityReply);

    bool validResponse = false;
    validResponse = ( activityReply.size()>0 &&
                      activityReply.get(0).isList() );

    if (validResponse && activityReply.get(0).asList()->size()==3)
    {
        if (activityReply.get(0).asList()->size()==3)
            //yDebug() << __func__ << "obtained valid response:" << activityReply.toString().c_str();
            ;
        else
        {
            yWarning() << __func__ << "obtained valid response (but unexpected length):" << activityReply.toString().c_str();
            return false;
        }

        x = activityReply.get(0).asList()->get(0).asDouble();
        y = activityReply.get(0).asList()->get(1).asDouble();
        z = activityReply.get(0).asList()->get(2).asDouble();
    }
    else
    {
        yWarning() << __func__ << "obtained invalid response:" << activityReply.toString().c_str();
        return false;
    }

    return true;
}

bool WorldStateMgrThread::getTooltipOffset(const string &objName, Bottle &offset)
{
    if (activityPort.getOutputCount() < 1)
    {
        yWarning() << __func__ << "not connected to ActivityIF";
        return false;
    }

    Bottle activityCmd, activityReply;
    activityCmd.addString("getOffset");
    activityCmd.addString(objName.c_str());
    //yDebug() << __func__ << "sending query:" << activityCmd.toString().c_str();
    activityPort.write(activityCmd, activityReply);

    bool validResponse = false;
    validResponse = ( activityReply.size()>0 &&
                      activityReply.get(0).isList() &&
                      activityReply.get(0).asList()->size()==3 );

    if (validResponse)
    {
        //yDebug() << __func__ << "obtained valid response:" << activityReply.toString().c_str();
        offset = *activityReply.get(0).asList();
        return true;
    }
    else
    {
        yWarning() << __func__ << "obtained invalid response:" << activityReply.toString().c_str();
        return false;
    }
}

bool WorldStateMgrThread::isOnTopOf(const string &objName, Bottle &objBelow)
{
    if (activityPort.getOutputCount() < 1)
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
    validResponse = ( activityReply.size()>0 &&
                      activityReply.get(0).isList() );

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

bool WorldStateMgrThread::isReachableWith(const string &objName, Bottle &objReachable)
{
    if (activityPort.getOutputCount() < 1)
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
    validResponse = ( activityReply.size()>0 &&
                      activityReply.get(0).isList() );

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

bool WorldStateMgrThread::isPullableWith(const string &objName, Bottle &objPullable)
{
    if (activityPort.getOutputCount() < 1)
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
    validResponse = ( activityReply.size()>0 &&
                      activityReply.get(0).isList() );

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

bool WorldStateMgrThread::isHandFree(const string &handName)
{
    if (activityPort.getOutputCount() < 1)
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
    validResponse = ( activityReply.size()>0 &&
                      activityReply.get(0).isVocab() ); // isBool()?

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

string WorldStateMgrThread::inWhichHand(const string &objName)
{
    string ret = "error";

    if (activityPort.getOutputCount() < 1)
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
    validResponse = ( activityReply.size()>0 &&
                      activityReply.get(0).isString() &&
                      activityReply.get(0).asString().size()>0 );

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

// IDL functions
bool WorldStateMgrThread::pauseTrack(const string &objName)
{
    if (playbackMode)
    {
        yWarning("not available in playback mode, requires perception mode!");
        return false;
    }

    if (trackerPort.getOutputCount() < 1)
    {
        yWarning("not connected to /activeParticleTrack/rpc:i");
        return false;
    }

    if (!trackerInit)
    {
        yWarning("tracker was not initialized by WSM yet: not going to pause %s", objName.c_str());
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
    validResponse = ( (trackerReply.size()>0) &&
                      (trackerReply.get(0).asVocab()==Vocab::encode("ok")) );
    if (!validResponse)
        yWarning() << __func__ <<  "obtained invalid response:" << trackerReply.toString().c_str();

    return true;
}

bool WorldStateMgrThread::resumeTrack(const string &objName)
{
    if (playbackMode)
    {
        yWarning("not available in playback mode, requires perception mode!");
        return false;
    }

    if (trackerPort.getOutputCount() < 1)
    {
        yWarning("not connected to /activeParticleTrack/rpc:i");
        return false;
    }

    if (!trackerInit)
    {
        yWarning("tracker was not initialized by WSM yet: not going to resume %s", objName.c_str());
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
    validResponse = ( (trackerReply.size()>0) &&
                      (trackerReply.get(0).asVocab()==Vocab::encode("ok")) );
    if (!validResponse)
        yWarning() << __func__ <<  "obtained invalid response:" << trackerReply.toString().c_str();

    return true;
}

Bottle WorldStateMgrThread::getColorHistogram(const int32_t &u, const int32_t &v)
{
    Bottle colors;

    if (playbackMode)
    {
        yWarning("not available in playback mode, requires perception mode!");
        return colors;
    }

    if (inAffPort.getInputCount() < 1)
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

bool WorldStateMgrThread::initPlaybackVars()
{
    toldUserEof = false;
    playbackPaused = true;
    sizePlaybackFile = -1;
    currPlayback = -1;

    return true;
}

void WorldStateMgrThread::setPlaybackFile(const string &file)
{
    playbackFile = file;
}

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
            if (!toldUserConnectOPC && opcPort.getOutputCount()<1)
            {
                yInfo("waiting for /%s/opc:io to be connected to /wsopc/rpc", moduleName.c_str());
                toldUserConnectOPC = true;
            }

            if (!toldUserOPCConnected && opcPort.getOutputCount()>=1)
            {
                dumpWorldState();
                yInfo("connected to WSOPC, you can now send commands over RPC");
                toldUserOPCConnected = true;
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
                    if (opcPort.getOutputCount() < 1)
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

                refreshOPC();
                dumpWorldState();

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
            // TODO: exit cleanly and automatically
            closing = true;
            yError("please quit the module");
            break;        
        }

        default:
        {
            yWarning("unknown FSM state");
            break;
        }
    }
}

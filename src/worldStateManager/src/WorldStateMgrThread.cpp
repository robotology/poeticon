/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2014 VisLab, Institute for Systems and Robotics,
 *                Istituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Giovanni Saponaro <gsaponaro@isr.ist.utl.pt>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 */

#include "WorldStateMgrThread.h"

WorldStateMgrThread::WorldStateMgrThread(
    const string &_moduleName,
    const double _period,
    bool _playbackMode)
    : moduleName(_moduleName),
      RateThread(int(_period*1000.0)),
      playbackMode(_playbackMode)
{
    //yDebug("constructed thread");
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

    outFixationPortName = "/" + moduleName + "/fixation:o";
    outFixationPort.open(outFixationPortName.c_str());

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
    outFixationPort.close();
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
    outFixationPort.interrupt();
    activityPort.interrupt();
    trackerPort.interrupt();
}

bool WorldStateMgrThread::threadInit()
{
    // perception and playback modes
    //yDebug("thread initialization");
    closing = false;
    populated = false; // TODO: really check if opc was populated before this module started
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
    if (opcPort.getOutputCount() < 1)
    {
        yWarning() << __func__ << "not connected to WSOPC";
        return false;
    }

    refreshOPC();
    //yDebug() << "opcIDs =" << opcIDs;

    if (!playbackMode)
    {
        if (trackerInit)
            refreshTracker();

        //yDebug() << "trackIDs =" << trackIDs;
    }

    yDebug("opcMap, trackMap:");
    dumpMap(opcMap);
    dumpMap(trackMap);

    return true;
}

bool WorldStateMgrThread::updateWorldState()
{
    if (!playbackMode)
    {
        // perception mode
        if (activityPort.getOutputCount() < 1)
        {
            yWarning() << __func__ << "not connected to ActivityIF";
        }
        needUpdate = true;
        refreshPerceptionAndValidate(); // TODO: redundant?
        yInfo("updated world state from robot perception");
    }
    else
    {
        // playback mode
        playbackPaused = false;
        yInfo("updated world state from playback file");
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
    inTargets = NULL;
    needUpdate = false;
    trackerInit = false;
    toldUserWaitBlobs = false;
    toldUserBlobsConnected = false;
    toldUserWaitTracker = false;
    toldUserTrackerConnected = false;
    toldUserWaitActivityIF = false;
    toldUserActivityIFConnected = false;

    return true;
}

bool WorldStateMgrThread::initTracker()
{
    if (outFixationPort.getOutputCount() < 1)
    {
        yWarning() << __func__ << ("exiting, %s not connected to /activeParticleTrack/fixation:i", outFixationPortName.c_str());
        return false;
    }

    if (trackerPort.getOutputCount() < 1)
    {
        yWarning() << __func__ << ("exiting, %s not connected to /activeParticleTrack/rpc:i", trackerPortName.c_str());
        return false;
    }

    // TODO: do this earlier, as soon as trackerPort is connected?
    int startID = 13; // TODO: set with ResourceFinder
    Bottle trackerCmd, trackerReply;
    trackerCmd.addString("countFrom");
    trackerCmd.addInt(startID);
    //yDebug() << __func__ <<  "sending query to tracker:" << trackerCmd.toString().c_str();
    trackerPort.write(trackerCmd, trackerReply);
    //yDebug() << __func__ <<  "obtained response:" << trackerReply.toString().c_str();
    bool validResponse = false;
    validResponse = ( (trackerReply.size()>0) &&
                      (trackerReply.get(0).asVocab()==Vocab::encode("ok")) );
    if (!validResponse)
    {
        yWarning() << __func__ << "invalid response from tracker";
        return false;
    }
    //else yDebug() << __func__ << "successfully communicated countFrom index to tracker";
    yDebug("told tracker to assign IDs starting from %d", startID);

    yInfo("initializing multi-object tracking of %d objects:", sizeAff);
    Bottle fixation;
    double u=0.0, v=0.0;

    for(int a=0; a<sizeAff; a++)
    {
        u = inAff->get(a+1).asList()->get(0).asDouble();
        v = inAff->get(a+1).asList()->get(1).asDouble();

        fixation.clear();
        fixation.addDouble(u);
        fixation.addDouble(v);
        outFixationPort.write(fixation);

        yInfo("id %d: %f %f", startID+a, u, v);
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
            if (!toldUserConnectOPC && opcPort.getOutputCount()<1)
            {
                yDebug("waiting for %s to be connected to /wsopc/rpc", opcPortName.c_str());
                toldUserConnectOPC = true;
            }

            if (!toldUserOPCConnected && opcPort.getOutputCount()>=1)
            {
                dumpWorldState();
                yInfo("connected to WSOPC, you can now send RPC commands to /%s/rpc:i", moduleName.c_str());
                toldUserOPCConnected = true;
            }

            if (opcPort.getOutputCount()>=1)
            {
                // proceed
                fsmState = STATE_PERCEPTION_WAIT_BLOBS;
            }

            break;
        }

        case STATE_PERCEPTION_WAIT_BLOBS:
        {
            // acquire initial entries (robot hands) from WSOPC database
            refreshOPC();

            if (!toldUserWaitBlobs && inAffPort.getOutputCount()<1)
            {
                yDebug("waiting for %s to be connected to /blobDescriptor/affDescriptor:o",
                      inAffPortName.c_str());
                toldUserWaitBlobs = true;
            }

            if (!toldUserBlobsConnected && inAffPort.getInputCount()>=1)
            {
                yDebug("connected to BlobDescriptor, waiting for data");
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
                yDebug("waiting for %s to be connected to /activeParticleTrack/target:o",
                      inTargetsPortName.c_str());
                toldUserWaitTracker = true;
            }

            if (!toldUserTrackerConnected && inTargetsPort.getInputCount()>=1)
            {
                yDebug("connected to tracker");
                toldUserTrackerConnected = true;
            }

            // proceed
            fsmState = STATE_PERCEPTION_INIT_TRACKER;

            break;
        }

        case STATE_PERCEPTION_INIT_TRACKER:
        {
            // initialize multi-object active particle tracker
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
                yDebug("waiting for %s to be connected to /activityInterface/rpc:i",
                      activityPortName.c_str());
                toldUserWaitActivityIF = true;
            }

            if (!toldUserActivityIFConnected && activityPort.getOutputCount()>=1)
            {
                yDebug("connected to ActivityInterface");
                toldUserActivityIFConnected = true;

                // proceed
                fsmState = STATE_PERCEPTION_POPULATE_DB;
            }

            // if one of the inputs is missing, go back to beginning
            if (opcPort.getOutputCount()<1 || inAffPort.getOutputCount()<1 ||
                inTargetsPort.getOutputCount()<1 || activityPort.getOutputCount()<1)
            {
                fsmState = STATE_PERCEPTION_WAIT_OPC;
            }

            break;
        }

        case STATE_PERCEPTION_POPULATE_DB:
        {
            // read new data and ensure validity
            refreshPerceptionAndValidate();

            // try populating database
            populated = doPopulateDB();

            // if database was successfully populated proceed, else stay in same state
            if (populated)
                fsmState = STATE_PERCEPTION_WAIT_CMD;

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
                // TODO: doPopulateDB() part
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
    if (activityPort.getOutputCount() < 1)
    {
        yWarning() << __func__ << "not connected to ActivityIF";
        return;
    }

    // cycle over items
    for (int i=0; i<trackIDs.size(); i++)
    {
        int id=trackIDs[i];

        if ( trackMap.find(id)==trackMap.end() )
        {
            // name not found -> ask ActivityIF for label
            // assume names cannot change -> use insert(), not operator[]
            // http://stackoverflow.com/questions/326062/in-stl-maps-is-it-better-to-use-mapinsert-than
            // TODO: make sure id corresponds to get(i)
            double u = inTargets->get(i).asList()->get(1).asDouble();
            double v = inTargets->get(i).asList()->get(2).asDouble();
            trackMap.insert(make_pair(id, getLabel(u,v)));
        }
        // else name found -> don't ask for it again (assume they cannot change)
    }
}

void WorldStateMgrThread::dumpMap(const idLabelMap &m)
{
    ostringstream fullMapContent; // output stream we'll feed to yDebug macro
    size_t items_remaining = m.size(); // http://stackoverflow.com/a/151112
    bool last_iteration = false; 
    for(idLabelMap::const_iterator iter = m.begin();
        iter != m.end();
        ++iter)
    {
        fullMapContent << iter->first << " " << iter->second;
        last_iteration = (items_remaining-- == 1);
        if (!last_iteration)
            fullMapContent << "; ";
    }
    yInfo() << "map = [" << fullMapContent.str().c_str() << "]";
}

void WorldStateMgrThread::refreshBlobs()
{
    inAff = inAffPort.read();

    if (inAff != NULL)
    {
        // number of blobs
        sizeAff = static_cast<int>( inAff->get(0).asDouble() );
    }

    //yDebug("successfully refreshed blob descriptor input");
}

void WorldStateMgrThread::refreshTracker()
{
    inTargets = inTargetsPort.read();

    if (inTargets != NULL)
    {
        // number of tracked objects
        sizeTargets = inTargets->size();

        // get current track IDs, update container, no duplicates
        updateTrackIDsNoDupes();
        
        // get labels, update map container
        refreshTrackNames();
    }
    else
    {
        yWarning() << __func__ << "did not receive data from tracker";
    }
}

void WorldStateMgrThread::updateTrackIDsNoDupes()
{
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
        yWarning("sizeAff=%d differs from sizeTargets=%d", sizeAff, sizeTargets);
        return false;
    }

    //yDebug("successfully refreshed and validated perception");
    return true;
}

bool WorldStateMgrThread::doPopulateDB()
{

    // TODO: cycle over OPC IDs & tracker IDs not yet in OPC
    for(int a=0; a<sizeAff; a++)
    {
        yDebug("doPopulateDB, a=%d", a);

        // common properties
        Bottle bName;
        Bottle bPos;
        Bottle bIsHand;

        // object properties
        Bottle bOffset;
        Bottle bDesc;
        Bottle bInHand;
        Bottle bOnTopOf;
        Bottle bReachW;
        Bottle bPullW;

        // hand properties
        Bottle bIsFree;

        // prepare name property
        double u = inTargets->get(a).asList()->get(1).asDouble(); // from tracker
        double v = inTargets->get(a).asList()->get(2).asDouble();
        //double u = inAff->get(a+1).asList()->get(0).asDouble(); // from blobs
        //double v = inAff->get(a+1).asList()->get(1).asDouble();
        bName.addString("name");
        string bNameValue = getLabel(u, v);
        bName.addString(bNameValue.c_str());

        // override empty labels - prevents error
        // yarp: BottleImpl reader failed, unrecognized object code 25
        if (bNameValue=="")
        {
            yDebug("overriding empty label with default value");
            bNameValue = "default";
        }

        // prepare position property
        bPos.addString("pos");
        Bottle &bPosValue = bPos.addList();
        double x=0.0, y=0.0, z=0.0;
        mono2stereo(u, v, x, y, z);
        bPosValue.addDouble(x);
        bPosValue.addDouble(y);
        bPosValue.addDouble(z);

        // prepare is_hand property
        bIsHand.addString("is_hand");
        bool bIsHandValue = false; // except for special entries in dbhands.ini
        bIsHand.addInt(bIsHandValue); // 1=true, 0=false

        if (!bIsHandValue)
        {
            // object properties

            // prepare offset property (end-effector transform when grasping tools)
            bOffset.addString("offset");
            Bottle &bOffsetValue = bOffset.addList();
            vector<double> offset = getTooltipOffset(bNameValue.c_str());

            if (offset.size()>1)
            {
                cout << offset.size() << endl;
                for (int o=0; o<offset.size(); o++)
                {
                    bOffsetValue.addDouble(offset[o]);
                }
            }

            // prepare 2D shape descriptors property
            bDesc.addString("desc2d");
            Bottle &bDescValue = bDesc.addList();
            int areaIdx = 23;
            bDescValue.addDouble(inAff->get(a+1).asList()->get(areaIdx).asDouble()); // area
            bDescValue.addDouble(inAff->get(a+1).asList()->get(areaIdx+1).asDouble()); // conv
            bDescValue.addDouble(inAff->get(a+1).asList()->get(areaIdx+2).asDouble()); // ecc
            bDescValue.addDouble(inAff->get(a+1).asList()->get(areaIdx+3).asDouble()); // comp
            bDescValue.addDouble(inAff->get(a+1).asList()->get(areaIdx+4).asDouble()); // circ
            bDescValue.addDouble(inAff->get(a+1).asList()->get(areaIdx+5).asDouble()); // sq

            // prepare in_hand property (none/left/right)
            bInHand.addString("in_hand");
            string bInHandValue = inWhichHand(bNameValue.c_str());
            bInHand.addString(bInHandValue);

            // prepare on_top_of property
            bOnTopOf.addString("on_top_of");
            Bottle &bOnTopOfValue = bOnTopOf.addList();
            vector<string> labelsBelow = isUnderOf(bNameValue.c_str());
            for (int o=0; o<labelsBelow.size(); o++)
            {
                bOnTopOfValue.addString(labelsBelow[o]);
            }

            // prepare reachable_with property
            bReachW.addString("reachable_with");
            Bottle &bReachWValue = bReachW.addList();
            vector<string> labelsRW = isReachableWith(bNameValue.c_str());
            for (int o=0; o<labelsRW.size(); o++)
            {
                bReachWValue.addString(labelsRW[o]);
            }

            // prepare pullable_with property
            bPullW.addString("pullable_with");
            Bottle &bPullWValue = bPullW.addList();
            vector<string> labelsPW = isPullableWith(bNameValue.c_str());
            for (int o=0; o<labelsPW.size(); o++)
            {
                bPullWValue.addString(labelsPW[o]);
            }
        }
        else
        {
            // hand properties

            // prepare is_free property
            bIsFree.addString("is_free");
            bool bIsFreeValue = isHandFree(bNameValue.c_str());
            bIsFree.addInt(bIsFreeValue); // 1=true, 0=false
        }

        // populate
        Bottle opcCmd, opcCmdContent, opcReply;
        opcCmd.addVocab(Vocab::encode("add"));

        opcCmdContent.addList() = bName;
        opcCmdContent.addList() = bPos;
        opcCmdContent.addList() = bIsHand;
        if (!bIsHandValue)
        {
            opcCmdContent.addList() = bOffset;
            opcCmdContent.addList() = bDesc;        
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

        yDebug("%d, populating OPC with: %s", a, opcCmd.toString().c_str());
        opcPort.write(opcCmd, opcReply);

        // process OPC response
        if (opcReply.size() > 1)
        {
            if (opcReply.get(0).asVocab()==Vocab::encode("ack"))
            {
                yDebug() << __func__ << "received ack from WSOPC";
                // TODO: store opc ID into std::map
            }
            else
                yDebug() << __func__ << "did not receive ack from WSOPC";
        }
    }

    return true;
}

string WorldStateMgrThread::getLabel(const double &u, const double &v)
{
    if (activityPort.getOutputCount() < 1)
    {
        yWarning() << __func__ << "not connected to ActivityIF";
        return string();
    }

    Bottle activityCmd, activityReply;
    activityCmd.addString("getLabel");
    activityCmd.addDouble(u);
    activityCmd.addDouble(v);
    yDebug() << __func__ <<  "sending query:" << activityCmd.toString().c_str();
    activityPort.write(activityCmd, activityReply);
    yDebug() << __func__ <<  "obtained response:" << activityReply.toString().c_str();

    bool validResponse = false;
    validResponse = ( activityReply.size()>0 &&
                      activityReply.get(0).isString() &&
                      activityReply.get(0).asString().size()>0 );

    if (validResponse)
    {
        yDebug() << __func__ << "valid response";
        return activityReply.get(0).asString();
    }
    else
    {
        yWarning() << __func__ << "invalid response";
        return string();
    }
}

bool WorldStateMgrThread::mono2stereo(const double &u, const double &v,
                                      double x, double y, double z)
{
    if (activityPort.getOutputCount() < 1)
    {
        yWarning() << __func__ << "not connected to ActivityIF";
        return false;
    }

    Bottle activityCmd, activityReply;
    activityCmd.addString("get3D");
    activityCmd.addDouble(u);
    activityCmd.addDouble(v);
    yDebug() << __func__ <<  "sending query:" << activityCmd.toString().c_str();
    activityPort.write(activityCmd, activityReply);
    yDebug() << __func__ <<  "obtained response:" << activityReply.toString().c_str();

    bool validResponse = false;
    validResponse = ( activityReply.size()>0 &&
                      activityReply.get(0).isList() &&
                      activityReply.get(0).asList()->size()==3 );

    if (validResponse)
    {
        x = activityReply.get(0).asList()->get(0).asDouble();
        y = activityReply.get(0).asList()->get(1).asDouble();
        z = activityReply.get(0).asList()->get(2).asDouble();
        yDebug() << __func__ << "valid response";
    }
    else
    {
        yWarning() << __func__ << "invalid response";
        return false;
    }

    return true;
}

vector<double> WorldStateMgrThread::getTooltipOffset(const string &objName)
{
    if (activityPort.getOutputCount() < 1)
    {
        yWarning() << __func__ << "not connected to ActivityIF";
        return vector<double>();
    }

    Bottle activityCmd, activityReply;
    activityCmd.addString("getOffset");
    activityCmd.addString(objName.c_str());
    yDebug() << __func__ << "sending query:" << activityCmd.toString().c_str();
    activityPort.write(activityCmd, activityReply);
    yDebug() << __func__ <<  "obtained response:" << activityReply.toString().c_str();

    bool validResponse = false;
    validResponse = ( activityReply.size()>0 &&
                      activityReply.get(0).isList() &&
                      activityReply.get(0).asList()->size()==3 );

    if (validResponse)
    {
        Bottle *bOffset = activityReply.get(0).asList();
        vector<double> offset; // TODO: pre-allocate size
        for (int t=0; t<bOffset->size(); t++)
        {
            offset.push_back( bOffset->get(t).asDouble() );
        }
        yDebug() << __func__ << "valid response";
        return offset;
    }
    else
    {
        yWarning() << __func__ << "invalid response";
        return vector<double>();
    }
}

vector<string> WorldStateMgrThread::isUnderOf(const string &objName)
{
    if (activityPort.getOutputCount() < 1)
    {
        yWarning() << __func__ << "not connected to ActivityIF";
        return vector<string>();
    }

    Bottle activityCmd, activityReply;
    activityCmd.addString("isUnderOf");
    activityCmd.addString(objName.c_str());
    yDebug() << __func__ << "sending query:" << activityCmd.toString().c_str();
    activityPort.write(activityCmd, activityReply);
    yDebug() << __func__ <<  "obtained response:" << activityReply.toString().c_str();

    bool validResponse = false;
    validResponse = ( activityReply.size()>0 &&
                      activityReply.get(0).isList() );

    if (validResponse)
    {
        Bottle *bLabels = activityReply.get(0).asList();
        vector<string> lab; // TODO: pre-allocate size
        for (int o=0; o<bLabels->size(); o++)
        {
            lab.push_back( bLabels->get(o).asString() );
        }
        yDebug() << __func__ << "valid response";
        return lab;
    }
    else
    {
        yWarning() << __func__ << "invalid response";
        return vector<string>();
    }
}

vector<string> WorldStateMgrThread::isReachableWith(const string &objName)
{
    if (activityPort.getOutputCount() < 1)
    {
        yWarning() << __func__ << "not connected to ActivityIF";
        return vector<string>();
    }

    Bottle activityCmd, activityReply;
    activityCmd.addString("reachableWith");
    activityCmd.addString(objName.c_str());
    yDebug() << __func__ << "sending query:" << activityCmd.toString().c_str();
    activityPort.write(activityCmd, activityReply);
    yDebug() << __func__ <<  "obtained response:" << activityReply.toString().c_str();

    bool validResponse = false;
    validResponse = ( activityReply.size()>0 &&
                      activityReply.get(0).isList() );

    if (validResponse)
    {
        Bottle *bLabels = activityReply.get(0).asList();
        vector<string> lab; // TODO: pre-allocate size
        for (int o=0; o<bLabels->size(); o++)
        {
            lab.push_back( bLabels->get(o).asString() );
        }
        yDebug() << __func__ << "valid response";
        return lab;
    }
    else
    {
        yWarning() << __func__ << "invalid response";
        return vector<string>();
    }
}

vector<string> WorldStateMgrThread::isPullableWith(const string &objName)
{
    if (activityPort.getOutputCount() < 1)
    {
        yWarning() << __func__ << "not connected to ActivityIF";
        return vector<string>();
    }

    Bottle activityCmd, activityReply;
    activityCmd.addString("pullableWith");
    activityCmd.addString(objName.c_str());
    yDebug() << __func__ <<  "sending query:" << activityCmd.toString().c_str();
    activityPort.write(activityCmd, activityReply);
    yDebug() << __func__ <<  "obtained response:" << activityReply.toString().c_str();

    bool validResponse = false;
    validResponse = ( activityReply.size()>0 &&
                      activityReply.get(0).isList() );

    if (validResponse)
    {
        Bottle *bLabels = activityReply.get(0).asList();
        vector<string> lab; // TODO: pre-allocate size
        for (int o=0; o<bLabels->size(); o++)
        {
            lab.push_back( bLabels->get(o).asString() );
        }
        yDebug() << __func__ << "valid response";
        return lab;
    }
    else
    {
        yWarning() << __func__ << "invalid response";
        return vector<string>();
    }
}

bool WorldStateMgrThread::isHandFree(const string &handName)
{
    if (activityPort.getOutputCount() < 1)
    {
        yWarning() << __func__ << "not connected to ActivityIF";
        return false;
    }

    // TODO: use consistent names everywhere (incl. RPC): left or left_hand or lefthand
    if ((handName != "left_hand") && (handName != "right_hand"))
    {
        yWarning() << __func__ << "argument handName must be left or right";
    }

    Bottle activityCmd, activityReply;
    activityCmd.addString("handStat");
    activityCmd.addString(handName.c_str());
    yDebug() << __func__ << "sending query:" << activityCmd.toString().c_str();
    activityPort.write(activityCmd, activityReply);
    yDebug() << __func__ <<  "obtained response:" << activityReply.toString().c_str();

    bool validResponse = false;
    validResponse = ( activityReply.size()>0 &&
                      activityReply.get(0).isBool() );

    if (validResponse)
    {
        yDebug() << __func__ << "valid response";
        return activityReply.get(0).asBool();
    }
    else
        yWarning() << __func__ << "invalid response";
        return false;
}

string WorldStateMgrThread::inWhichHand(const string &objName)
{
    if (activityPort.getOutputCount() < 1)
    {
        yWarning() << __func__ << "not connected to ActivityIF";
    }

    // TODO: use consistent names everywhere (incl. RPC): left or left_hand or lefthand
    if ((objName == "left_hand") || (objName == "right_hand"))
    {
        yWarning() << __func__ << "argument objName must be an object name, not a hand name";
    }

    Bottle activityCmd, activityReply;
    activityCmd.addString("inHand");
    activityCmd.addString(objName.c_str());
    yDebug() << __func__ << "sending query:" << activityCmd.toString().c_str();
    activityPort.write(activityCmd, activityReply);
    yDebug() << __func__ <<  "obtained response:" << activityReply.toString().c_str();

    bool validResponse = false;
    validResponse = ( activityReply.size()>0 &&
                      activityReply.get(0).isString() &&
                      activityReply.get(0).asString().size()>0 );

    // TODO: use consistent names everywhere (incl. RPC): left or left_hand or lefthand
    if (validResponse)
    {
        if ( activityReply.get(0).asString()=="left" ||
             activityReply.get(0).asString()=="right" ||
             activityReply.get(0).asString()=="none" )
        {
            yDebug() << __func__ <<  "valid response";
            return activityReply.get(0).asString();
        }
        else
            yWarning() << __func__ << "invalid response";
    }
    else
        yWarning() << __func__ << "invalid response";

    // default
    return "none";
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
                    // going to ask OPC whether entry already exists
                    opcCmd.clear();
                    content.clear();
                    opcCmd.addVocab(Vocab::encode("get"));
                    obj_j = bCurr.get(j).asList();
                    opcCmd.addList() = *obj_j->get(0).asList();
                    opcPort.write(opcCmd, opcReply);

                    // stop here if no OPC connection
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
                            "in OPC database, but" <<
                            obj_j->get(0).asList()->get(1).asInt() <<
                            "in playback file";

                            //break;
                        }
                    }

                } // end for parse each entry/line

                refreshOPC();
                dumpWorldState();

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

bool WorldStateMgrThread::setPlaybackFile(const string &file)
{
    playbackFile = file;
    //yDebug("going to read from playback file %s", playbackFile.c_str());
}

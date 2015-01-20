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
    yDebug("constructed thread");
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

    geomIFPortName = "/" + moduleName + "/geomIF:rpc";
    geomIFPort.open(geomIFPortName.c_str());

    return true;
}

void WorldStateMgrThread::close()
{
    // perception and playback modes
    yDebug("closing ports");
    opcPort.close();

    if (playbackMode) return;

    // perception mode only
    inTargetsPort.close();
    inAffPort.close();
    outFixationPort.close();
    geomIFPort.close();
}

void WorldStateMgrThread::interrupt()
{
    // perception and playback modes
    closing = true;
    yDebug("interrupting ports");
    opcPort.interrupt();

    if (playbackMode) return;

    // perception mode only
    inTargetsPort.interrupt();
    inAffPort.interrupt();
    outFixationPort.interrupt();
    geomIFPort.interrupt();
}

bool WorldStateMgrThread::threadInit()
{
    // perception and playback modes
    yDebug("thread initialization");
    closing = false;
    populated = false; // TODO: really check if opc was populated before this module started
    if ( !openPorts() )
    {
        yError("problem opening ports");
        return false;
    }

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

bool WorldStateMgrThread::updateWorldState()
{
     if (opcPort.getOutputCount() < 1)
     {
         yWarning("not connected to OPC");
         return false;
     }

     if (!playbackMode)
     {
         // perception mode
         refreshPerceptionAndValidate();
     }
     else
     {
         // playback mode
         playbackPaused = false;
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
    perceptionFSMState = STATE_WAIT_BLOBS;

    return true;
}

bool WorldStateMgrThread::initTracker()
{
    if (playbackMode)
    {
        yWarning("initTracker called when it was not supposed to");
        return false;
    }

    if (outFixationPort.getOutputCount() < 1)
    {
        yWarning("fixation:o not connected to tracker input, exiting initTracker");
        return false;
    }

    yInfo("initializing multi-object tracking of %d objects:", sizeAff);

    Bottle fixation;
    double x=0.0, y=0.0;

    for(int a=0; a<sizeAff; a++)
    {
        x = inAff->get(a+1).asList()->get(0).asDouble();
        y = inAff->get(a+1).asList()->get(1).asDouble();

        fixation.clear();
        fixation.addDouble(x);
        fixation.addDouble(y);
        Time::delay(1.0); // prevents activeParticleTrack crash
        outFixationPort.write(fixation);

        yInfo("id %d: %f %f", a, x, y);
    }

    yInfo("done initializing tracker");
    return true;
}

void WorldStateMgrThread::fsmPerception()
{
    yDebug("perception state=%d", perceptionFSMState);
    switch(perceptionFSMState)
    {
        case STATE_WAIT_BLOBS:
        {
            // wait for blobs data to arrive
            refreshBlobs();
            // when something arrives, proceed
            if (inAff != NULL)
                perceptionFSMState = STATE_READ_BLOBS;

            break;
        }

        case STATE_READ_BLOBS:
        {
            // if size>0 proceed, else go back one state
            if (sizeAff > 0)
                perceptionFSMState = STATE_INIT_TRACKER;
            else
                perceptionFSMState = STATE_WAIT_BLOBS;

            break;
        }

        case STATE_INIT_TRACKER:
        {
            // initialize multi-object active particle tracker
            initTracker();

            // proceed
            perceptionFSMState = STATE_WAIT_TRACKER;

            break;
        }

        case STATE_WAIT_TRACKER:
        {
            // wait for tracker data to arrive
            refreshTracker();

            // when something arrives, proceed
            if (inTargets != NULL)
                perceptionFSMState = STATE_READ_TRACKER;

            break;
        }

        case STATE_READ_TRACKER:
        {
            // if size>0 proceed, else go back one state
            if (sizeTargets > 0)
                perceptionFSMState = STATE_POPULATE_DB;
            else
                perceptionFSMState = STATE_WAIT_TRACKER;

            break;
        }

        case STATE_POPULATE_DB:
        {
            // read new data and ensure validity
            refreshPerceptionAndValidate();

            // try populating database
            populated = doPopulateDB();

            // if database was successfully populated proceed, else stay in same state
            if (populated)
                perceptionFSMState = STATE_UPDATE_DB;

            break;
        }

        case STATE_UPDATE_DB:
        {
            // read new data and ensure validity
            refreshPerceptionAndValidate();

            // update opc
            updateWorldState();

            break;
        }
        default:
        {
            break;
        }
    }
}

void WorldStateMgrThread::refreshBlobs()
{
    if (playbackMode)
    {
        yWarning("refreshBlobs called when it was not supposed to");
        return;
    }

    inAff = inAffPort.read();

    if (inAff != NULL)
    {
        // number of blobs
        sizeAff = static_cast<int>( inAff->get(0).asDouble() );
    }
}

void WorldStateMgrThread::refreshTracker()
{
    if (playbackMode)
    {
        yWarning("refreshTracker called when it was not supposed to");
        return;
    }

    inTargets = inTargetsPort.read();

    if (inTargets != NULL)
    {
        // number of tracked objects
        sizeTargets = inTargets->size();
    }
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

    return true;
}

bool WorldStateMgrThread::doPopulateDB()
{
    for(int a=0; a<sizeAff; a++)
    {
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
        bName.addString("name");
        std::stringstream bNameValue; // TODO: real name from IOL
        bNameValue << "myLabel" << a;
        bName.addString( bNameValue.str() );

        // prepare position property
        // TODO: transform to 3D ref frame with iKinGazeCtrl
        bPos.addString("pos");
        Bottle &bPosValue = bPos.addList();
        // from blobs
        //bPosValue.addDouble(inAff->get(a+1).asList()->get(0).asDouble());
        //bPosValue.addDouble(inAff->get(a+1).asList()->get(1).asDouble());
        // from tracker
        bPosValue.addDouble(inTargets->get(a).asList()->get(1).asDouble());
        bPosValue.addDouble(inTargets->get(a).asList()->get(2).asDouble());

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
            //bOffsetValue.addDouble(); // TODO: real values from tool exploration

            // prepare 2D shape descriptors property
            bDesc.addString("desc2d");
            Bottle &bDescValue = bDesc.addList();
            bDescValue.addDouble(inAff->get(a+1).asList()->get(23).asDouble()); // area
            bDescValue.addDouble(inAff->get(a+1).asList()->get(24).asDouble());
            bDescValue.addDouble(inAff->get(a+1).asList()->get(25).asDouble());
            bDescValue.addDouble(inAff->get(a+1).asList()->get(26).asDouble());
            bDescValue.addDouble(inAff->get(a+1).asList()->get(27).asDouble());
            bDescValue.addDouble(inAff->get(a+1).asList()->get(28).asDouble());

            // prepare in_hand property (none/left/right)
            bInHand.addString("in_hand");
            string bInHandValue = inWhichHand(bNameValue.str());
            bInHand.addString(bInHandValue);
            
            // prepare on_top_of property
            bOnTopOf.addString("on_top_of");
            Bottle &bOnTopOfValue = bOnTopOf.addList();
            bOnTopOfValue.addInt(0); // TODO: real list

            // prepare reachable_with property
            bReachW.addString("reachable_with");
            Bottle &bReachWValue = bReachW.addList();
            bReachWValue.addInt(0); // TODO: real list

            // prepare pullable_with property
            bPullW.addString("pullable_with");
            Bottle &bPullWValue = bPullW.addList();
            bPullWValue.addInt(0); // TODO: real list
        }
        else
        {
            // hand properties

            // prepare is_free property
            bIsFree.addString("is_free");
            //bool bIsFreeValue;
            bool bIsFreeValue = isHandFree(bNameValue.str());
            bIsFree.addInt(bIsFreeValue); // 1=true, 0=false
        }

        // populate
        Bottle opcCmd, opcReply;
        opcCmd.addVocab(Vocab::encode("add"));
        opcCmd.addList() = bName;
        opcCmd.addList() = bPos;
        opcCmd.addList() = bIsHand;
        if (!bIsHandValue)
        {
            opcCmd.addList() = bOffset;
            opcCmd.addList() = bDesc;        
            opcCmd.addList() = bInHand;
            opcCmd.addList() = bOnTopOf;
            opcCmd.addList() = bReachW;
            opcCmd.addList() = bPullW;
        }
        else
        {
            opcCmd.addList() = bIsFree;        
        }

        yDebug("%d, populating OPC with: %s", a, opcCmd.toString().c_str());
        opcPort.write(opcCmd, opcReply);
        
        // process OPC response
        if (opcReply.size() > 1)
        {
            if (opcReply.get(0).asVocab()==Vocab::encode("ack"))
                yDebug("received ack from OPC");
            else
                yDebug("did not receive ack from OPC");
        }
    }
    // now we have populated the database with all objects

    return true;
}

bool WorldStateMgrThread::isHandFree(const string &handName)
{
    if (geomIFPort.getOutputCount() < 1)
    {
        yWarning("not connected to GeometricIF");
        return false;
    }

    // TODO: use consistent names everywhere (incl. RPC): left or left_hand or lefthand
    if ((handName != "left_hand") && (handName != "right_hand"))
    {
        yWarning("isHandFree: argument handName must be left_hand or right_hand");
    }
    
    Bottle geomIFCmd, geomIFReply;
    geomIFCmd.addVocab(Vocab::encode("isfree"));
    geomIFCmd.addString(handName.c_str());
    yDebug("sending query to GeometricIF:", geomIFCmd.toString().c_str());
    geomIFPort.write(geomIFCmd, geomIFReply);
    
    bool validResponse = false;
    validResponse = ( (geomIFReply.size()>1) &&
                      (geomIFReply.get(0).asVocab()==Vocab::encode("ack")) );

    return (validResponse && (geomIFReply.get(1).asBool()==true));
}

string WorldStateMgrThread::inWhichHand(const string &objName)
{
    if (geomIFPort.getOutputCount() < 1)
    {
        yWarning("not connected to GeometricIF");
    }

    // TODO: use consistent names everywhere (incl. RPC): left or left_hand or lefthand
    if ((objName == "left_hand") || (objName == "right_hand"))
    {
        yWarning("inWhichHand: argument objName must be an object name, not a hand name");
    }

    Bottle geomIFCmd, geomIFReply;
    geomIFCmd.addVocab(Vocab::encode("inhand"));
    geomIFCmd.addString(objName.c_str());
    yDebug("sending query to GeometricIF:", geomIFCmd.toString().c_str());
    geomIFPort.write(geomIFCmd, geomIFReply);
    
    bool validResponse = false;
    validResponse = ( (geomIFReply.size()>1) &&
                      (geomIFReply.get(0).asVocab()==Vocab::encode("ack")) );

    // TODO: use consistent names everywhere (incl. RPC): left or left_hand or lefthand
    if (validResponse)
    {
        if ((geomIFReply.get(1).asString()=="left") || (geomIFReply.get(1).asString()=="right"))
        {
            // successful case, return "left" or "right"
            return geomIFReply.get(1).asString();
        }
        else
            yWarning("inWhichHand: obtained valid but unknown response from GeometricIF");
    }
    else
        yWarning("inWhichHand: obtained invalid response from GeometricIF");

    // default
    return "none";
}

/* ************************************************************************** */
/* playback mode                                                              */
/* ************************************************************************** */

bool WorldStateMgrThread::initPlaybackVars()
{
    playbackPaused = true;
    playbackFSMState = STATE_PARSE_FILE;
    sizePlaybackFile = -1;
    currPlayback = -1;

    return true;
}

void WorldStateMgrThread::fsmPlayback()
{
    //yDebug("playback state=%d", playbackFSMState);
    switch (playbackFSMState)
    {
        case STATE_PARSE_FILE:
        {
            // acquire Bottle with whole file content
            Property findProperty;
            findProperty.fromConfigFile(playbackFile.c_str());
            findBottle.read(findProperty);
            sizePlaybackFile = findBottle.size();
            if (sizePlaybackFile < 1)
            {
                yError("file empty or not parsable");
                playbackFSMState = STATE_END_FILE;
            }
            
            // proceed to "[state00]"
            currPlayback = 0;
            playbackFSMState = STATE_STEP_FILE;
            break;
        }
        
        case STATE_STEP_FILE:
        {
            if (!playbackPaused)
            {
                // parse group "[state##]" of file
                // TODO: make it work for "[state##]" with ##>9, simplify
                ostringstream tag;
                tag << "state" << std::setw(2) << std::setfill('0') << currPlayback << "";
                Bottle &bCurr = findBottle.findGroup(tag.str().c_str());
                if (bCurr.size() < 1)
                {
                    yWarning() << tag.str().c_str() << "not found";
                    playbackPaused = true;
                    playbackFSMState = STATE_END_FILE;
                    break;
                }
                yDebug("loaded group %s, has size %d incl. group tag", tag.str().c_str(), bCurr.size());

                Bottle opcCmd, content, opcReply;
                Bottle *obj_j;

                // parse each entry/line of current group "[state##]"
                for (int j=1; j<bCurr.size(); j++)
                {
                    // check if entry already exists in OPC
                    opcCmd.clear();
                    content.clear();
                    opcCmd.addVocab(Vocab::encode("get"));
                    obj_j = bCurr.get(j).asList();
                    opcCmd.addList() = *obj_j->get(0).asList();
                    opcPort.write(opcCmd, opcReply);
                    
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
                }
                
                ++currPlayback;
                playbackPaused = true;
            }
            
            // stay in same state, wait for next update instruction over rpc
            
            break;
        }
        
        case STATE_END_FILE:
        {
            yInfo("finished reading from playback file");
            closing = true;
            break;        
        }
        
        default:
            break;
    }
}

bool WorldStateMgrThread::setPlaybackFile(const string &file)
{
    playbackFile = file;
    yDebug("going to read from playback file %s", playbackFile.c_str());
}

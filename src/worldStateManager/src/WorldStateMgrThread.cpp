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
}

bool WorldStateMgrThread::threadInit()
{
    // perception and playback modes
    yDebug("thread initialization");
    closing = false;
    populated = false; // TODO: really check if opc was populated before
    if ( !openPorts() )
    {
        yError("problem opening ports");
        return false;
    }

    // playback mode only
    if (playbackMode) return true;

    // perception mode only
    return initVariables();
}

bool WorldStateMgrThread::initVariables()
{
    if (playbackMode)
    {
        yWarning("initVariables called when it was not supposed to");
        return false;
    }

    inAff = NULL;
    inTargets = NULL;
    state = STATE_WAIT_BLOBS;

    return true;
}

bool WorldStateMgrThread::initTracker()
{
    if (playbackMode)
    {
        yWarning("initTracker called when it was not supposed to");
        return false;
    }

    if (outFixationPort.getOutputCount()<1)
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

void WorldStateMgrThread::fsmPerception()
{
    yDebug("perception state=%d", state);
    switch(state)
    {
        case STATE_WAIT_BLOBS:
        {
            // wait for blobs data to arrive
            refreshBlobs();
            // when something arrives, proceed
            if (inAff != NULL)
                state = STATE_READ_BLOBS;

            break;
        }

        case STATE_READ_BLOBS:
        {
            // if size>0 proceed, else go back one state
            if (sizeAff > 0)
                state = STATE_INIT_TRACKER;
            else
                state = STATE_WAIT_BLOBS;

            break;
        }

        case STATE_INIT_TRACKER:
        {
            initTracker();

            // proceed
            state = STATE_WAIT_TRACKER;

            break;
        }

        case STATE_WAIT_TRACKER:
        {
            // wait for tracker data to arrive
            refreshTracker();

            // when something arrives, proceed
            if (inTargets != NULL)
                state = STATE_READ_TRACKER;

            break;
        }

        case STATE_READ_TRACKER:
        {
            // if size>0 proceed, else go back one state
            if (sizeTargets > 0)
                state = STATE_POPULATE_DB;
            else
                state = STATE_WAIT_TRACKER;

            break;
        }

        case STATE_POPULATE_DB:
        {
            // read new data and ensure validity
            refreshAllAndValidate();

            // try populating database
            populated = doPopulateDB();

            // if database was successfully populated proceed, else stay in same state
            if (populated)
                state = STATE_UPDATE_DB;

            break;
        }

        case STATE_UPDATE_DB:
        {
            // read new data and ensure validity
            refreshAll();

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

bool WorldStateMgrThread::updateWorldState()
{
     if (!playbackMode) refreshAllAndValidate();
     // TODO: playbackMode
     // TODO: write to opc
     yDebug("updated world state");
     return true;
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

void WorldStateMgrThread::refreshAll()
{
    refreshBlobs();
    refreshTracker();
}

bool WorldStateMgrThread::refreshAllAndValidate()
{
    refreshAll();

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
    // TODO: make this function compatible with playback mode, too
    for(int a=0; a<sizeAff; a++)
    {
        // prepare position property
        Bottle bPos;
        bPos.addString("pos");
        Bottle &bPosValue = bPos.addList();
        // from blobs
        //bPosValue.addDouble(inAff->get(a+1).asList()->get(0).asDouble());
        //bPosValue.addDouble(inAff->get(a+1).asList()->get(1).asDouble());
        // from tracker
        bPosValue.addDouble(inTargets->get(a).asList()->get(1).asDouble());
        bPosValue.addDouble(inTargets->get(a).asList()->get(2).asDouble());

        // prepare name property
        Bottle bName;
        bName.addString("name");
        std::stringstream fakename;
        fakename << "myLabel" << a;
        bName.addString( fakename.str() ); // TODO: real name from object recognition

        // prepare 2D shape descriptors property
        Bottle bDesc;
        bDesc.addString("desc2d");
        Bottle &bDescValue = bDesc.addList();
        bDescValue.addDouble(inAff->get(a+1).asList()->get(23).asDouble()); // area
        bDescValue.addDouble(inAff->get(a+1).asList()->get(24).asDouble());
        bDescValue.addDouble(inAff->get(a+1).asList()->get(25).asDouble());
        bDescValue.addDouble(inAff->get(a+1).asList()->get(26).asDouble());
        bDescValue.addDouble(inAff->get(a+1).asList()->get(27).asDouble());
        bDescValue.addDouble(inAff->get(a+1).asList()->get(28).asDouble());

        // prepare is_hand property
        Bottle bIsHand;
        bIsHand.addString("is_hand");
        bool bIsHandValue = false; // TODO: real value from perception
        bIsHand.addInt(bIsHandValue); // 1=true, 0=false
        
        // prepare is_free property
        Bottle bIsFree;
        bIsFree.addString("is_free");
        bool bIsFreeValue = true; // TODO: real value from perception
        bIsFree.addInt(bIsFreeValue); // 1=true, 0=false

        // prepare in_hand property (none/left/right)
        Bottle bInHand;
        bInHand.addString("in_hand");
        string bInHandValue = "none"; // TODO: real value
        bInHand.addString(bInHandValue);

        // prepare on_top_of property
        Bottle bOnTopOf;
        bOnTopOf.addString("on_top_of");
        Bottle &bOnTopOfValue = bOnTopOf.addList();
        bOnTopOfValue.addInt(0); // TODO: real list

        // prepare reachable_with property
        Bottle bReachW;
        bReachW.addString("reachable_with");
        Bottle &bReachWValue = bReachW.addList();
        bReachWValue.addInt(0); // TODO: real list

        // prepare pullable_with property
        Bottle bPullW;
        bPullW.addString("pullable_with");
        Bottle &bPullWValue = bPullW.addList();
        bPullWValue.addInt(0); // TODO: real list

        // prepare is_touching property
        Bottle bIsTouching;
        bIsTouching.addString("is_touching");
        Bottle &bIsTouchingValue = bIsTouching.addList();
        bIsTouchingValue.addInt(0); // TODO: real list

        // populate
        Bottle opcCmd, opcReply;
        opcCmd.addVocab(Vocab::encode("add"));
        opcCmd.addList() = bPos;
        opcCmd.addList() = bName;
        opcCmd.addList() = bDesc;
        opcCmd.addList() = bIsHand;
        opcCmd.addList() = bInHand;
        opcCmd.addList() = bOnTopOf;
        opcCmd.addList() = bReachW;
        opcCmd.addList() = bPullW;
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

// playback mode

void WorldStateMgrThread::fsmPlayback()
{
    // TODO: validate file, parse new instant and stepOnce
    yDebug("playback state=");
}

bool WorldStateMgrThread::setPlaybackFile(const string &file)
{
    playbackFile = file;
    yDebug("going to read from playback file %s", playbackFile.c_str());
}

bool WorldStateMgrThread::stepOnce()
{
     yDebug("stepping to next state");
     // TODO
     return true;
}

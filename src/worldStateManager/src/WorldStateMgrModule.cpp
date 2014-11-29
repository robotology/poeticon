/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2014 VisLab, Institute for Systems and Robotics,
 *                Istituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Giovanni Saponaro <gsaponaro@isr.ist.utl.pt>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 */

#include "WorldStateMgrModule.h"

bool WorldStateMgrModule::configure(ResourceFinder &rf)
{
    moduleName = rf.check("name", Value("wsm")).asString();
    setName(moduleName.c_str());

    inTargetsPortName = "/" + moduleName + "/target:i";
    inTargetsPort.open(inTargetsPortName.c_str());

    inAffPortName = "/" + moduleName + "/affDescriptor:i";
    inAffPort.open(inAffPortName.c_str());

    outFixationPortName = "/" + moduleName + "/fixation:o";
    outFixationPort.open(outFixationPortName.c_str());

    opcPortName = "/" + moduleName + "/opc:io";
    opcPort.open(opcPortName.c_str());

    inAff = NULL;
    inTargets = NULL;
    state = STATE_WAIT_BLOBS;
    populated = false; // TODO: robustly check whether OPC has been populated

    return true;
}

bool WorldStateMgrModule::interruptModule()
{
    inTargetsPort.interrupt();
    inAffPort.interrupt();
    outFixationPort.interrupt();
    opcPort.interrupt();

    return true;
}

bool WorldStateMgrModule::close()
{
    inTargetsPort.close();
    inAffPort.close();
    outFixationPort.close();
    opcPort.close();

    return true;
}

bool WorldStateMgrModule::updateModule()
{
    //yDebug("state=%d", state);

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
            doInitTracker();

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

            // TODO: keep database updated with current perception data

            break;
        }
    }

    return true;
}

double WorldStateMgrModule::getPeriod()
{
    return 0.0;
}

void WorldStateMgrModule::doInitTracker()
{
    if (outFixationPort.getOutputCount()<1)
    {
        yWarning("fixation:o not connected to tracker input, exiting doInitTracker()");
        return;
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
        Time::delay(1.0); // fixes activeParticleTrack crash
        outFixationPort.write(fixation);

        yInfo("id %d: %f %f", a, x, y);
    }

    yInfo("done initializing tracker");
}

bool WorldStateMgrModule::doPopulateDB()
{
    for(int a=0; a<sizeAff; a++)
    {
        // prepare position property
        Bottle bPos;
        bPos.addVocab(Vocab::encode("pos"));
        Bottle &bPosValue = bPos.addList();
        // from blobs
        //bPosValue.addDouble(inAff->get(a+1).asList()->get(0).asDouble());
        //bPosValue.addDouble(inAff->get(a+1).asList()->get(1).asDouble());
        // from tracker
        bPosValue.addDouble(inTargets->get(a).asList()->get(1).asDouble());
        bPosValue.addDouble(inTargets->get(a).asList()->get(2).asDouble());

        // prepare name property
        Bottle bName;
        bName.addVocab(Vocab::encode("name"));
        std::stringstream fakename;
        fakename << "myLabel" << a;
        bName.addString( fakename.str() ); // TODO: real name from object recognition

        // prepare shape descriptors property
        Bottle bDesc;
        bDesc.addVocab(Vocab::encode("desc"));
        Bottle &bDescValue = bDesc.addList();
        bDescValue.addDouble(inAff->get(a+1).asList()->get(23).asDouble()); // area
        bDescValue.addDouble(inAff->get(a+1).asList()->get(24).asDouble());
        bDescValue.addDouble(inAff->get(a+1).asList()->get(25).asDouble());
        bDescValue.addDouble(inAff->get(a+1).asList()->get(26).asDouble());
        bDescValue.addDouble(inAff->get(a+1).asList()->get(27).asDouble());
        bDescValue.addDouble(inAff->get(a+1).asList()->get(28).asDouble());

        // prepare is_hand property (true/false)
        Bottle bIsHand;
        bIsHand.addVocab(Vocab::encode("is_h"));
        bool bIsHandValue = false; // TODO: real value from perception
        bIsHand.addInt(bIsHandValue); // 1=true, 0=false

        // prepare in_hand property (none/left/right)
        Bottle bInHand;
        bInHand.addVocab(Vocab::encode("in_h"));
        string bInHandValue = "none"; // TODO: real value
        bInHand.addString(bInHandValue);

        // prepare on_top_of property
        Bottle bOnTopOf;
        bOnTopOf.addVocab(Vocab::encode("on_t"));
        Bottle &bOnTopOfValue = bOnTopOf.addList();
        bOnTopOfValue.addInt(0); // TODO: real list

        // prepare reachable_with property
        Bottle bReachW;
        bReachW.addVocab(Vocab::encode("re_w"));
        Bottle &bReachWValue = bReachW.addList();
        bReachWValue.addInt(0); // TODO: real list

        // prepare pullable_with property
        Bottle bPullW;
        bPullW.addVocab(Vocab::encode("pu_w"));
        Bottle &bPullWValue = bPullW.addList();
        bPullWValue.addInt(0); // TODO: real list

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
    }
    // now we have populated the database with all objects

    return true;
}

void WorldStateMgrModule::refreshBlobs()
{
    inAff = inAffPort.read();

    if (inAff != NULL)
    {
        // number of blobs
        sizeAff = static_cast<int>( inAff->get(0).asDouble() );
    }
}

void WorldStateMgrModule::refreshTracker()
{
    inTargets = inTargetsPort.read();

    if (inTargets != NULL)
    {
        // number of tracked objects
        sizeTargets = inTargets->size();
    }
}

void WorldStateMgrModule::refreshAll()
{
    refreshBlobs();
    refreshTracker();
}

bool WorldStateMgrModule::refreshAllAndValidate()
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

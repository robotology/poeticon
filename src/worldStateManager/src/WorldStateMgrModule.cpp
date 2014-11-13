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

    //outStatePortName = "/" + moduleName + "/state:o";
    //outStatePort.open(outStatePortName.c_str());

    inAff = NULL;
    inTargets = NULL;
    state = STATE_WAIT_BLOBS;

    return true;
}

bool WorldStateMgrModule::interruptModule()
{
    inTargetsPort.interrupt();
    inAffPort.interrupt();
    outFixationPort.interrupt();
    //outStatePort.interrupt();

    return true;
}

bool WorldStateMgrModule::close()
{
    inTargetsPort.close();
    inAffPort.close();
    outFixationPort.close();
    //outStatePort.close();

    return true;
}

bool WorldStateMgrModule::updateModule()
{
    switch(state)
    {
        case STATE_WAIT_BLOBS:
        {
            // wait for blobs data to arrive
            updateBlobs();

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
            updateTracker();

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
            // TODO: populate OPC and keep it updated with current perception data

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

/*
void ShortTermMemModule::doWriteState()
{
    // read new data, ensure validity
    updateBlobs();
    updateTracker();
    if (inAff==NULL || inTargets==NULL)
    {
        yWarning("no data");
        return;
    }
    if (sizeAff != sizeTargets)
    {
        //yWarning("sizeAff=%d differs from sizeTargets=%d", sizeAff, sizeTargets);
        return;
    }

    Bottle &state = outStatePort.prepare();
    state.clear();
    for(int a=0; a<sizeAff; a++)
    {
        Bottle &obj = state.addList();
        obj.clear();

        // add ID of object from tracker
        obj.addInt(inTargets->get(a).asList()->get(0).asInt());

        // add other fields from tracker
        //obj.addDouble(inTargets->get(a).asList()->get(1).asDouble());
        //obj.addDouble(inTargets->get(a).asList()->get(2).asDouble());

        // add other fields from blobs
        obj.addDouble(inAff->get(a+1).asList()->get(0).asDouble());
        obj.addDouble(inAff->get(a+1).asList()->get(1).asDouble());
    }
    outStatePort.write();

    //yInfo("sent state of %d objects", sizeAff);
}
*/

void WorldStateMgrModule::updateBlobs()
{
    inAff = inAffPort.read();

    if (inAff != NULL)
    {
        // number of blobs
        sizeAff = static_cast<int>( inAff->get(0).asDouble() );
    }
}

void WorldStateMgrModule::updateTracker()
{
    inTargets = inTargetsPort.read();

    if (inTargets != NULL)
    {
        // number of tracked objects
        sizeTargets = inTargets->size();
    }
}

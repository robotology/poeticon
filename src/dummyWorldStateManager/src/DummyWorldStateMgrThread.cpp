/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2016 VisLab, Institute for Systems and Robotics,
 *                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Giovanni Saponaro <gsaponaro@isr.ist.utl.pt>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 */

#include "DummyWorldStateMgrThread.h"

/**********************************************************/
DummyWorldStateMgrThread::DummyWorldStateMgrThread(
    const string &_moduleName,
    const double _period)
    : moduleName(_moduleName),
      RateThread(int(_period*1000.0))
{
}

/**********************************************************/
bool DummyWorldStateMgrThread::openPorts()
{
    bool ret = true;
    opcPortName = "/" + moduleName + "/opc:io";
    ret = ret && opcPort.open(opcPortName.c_str());

    return ret;
}

/**********************************************************/
void DummyWorldStateMgrThread::close()
{
    yInfo("closing ports");
    opcPort.close();

    return;
}

/**********************************************************/
void DummyWorldStateMgrThread::interrupt()
{
    closing = true;

    yInfo("interrupting ports");
    opcPort.interrupt();

    return;
}

/**********************************************************/
bool DummyWorldStateMgrThread::threadInit()
{
    closing = false;

    if ( !openPorts() )
    {
        yError("problem opening ports");
        return false;
    }

    if (! initVars() )
    {
        yError("problem initializing variables");
        return false;
    }

    return true;
}

/**********************************************************/
void DummyWorldStateMgrThread::run()
{
    while (!closing)
    {
        // enter playback state machine
        fsmPlayback();

        yarp::os::Time::delay(0.01);
    }
}

/**********************************************************/
bool DummyWorldStateMgrThread::initVars()
{
    fsmState = STATE_DUMMY_PARSE;
    toldUserOPCConnected = false;
    t = yarp::os::Time::now();
    toldUserEof = false;
    playbackPaused = true;
    sizePlaybackFile = -1;
    currPlayback = -1;

    return true;
}

/**********************************************************/
bool DummyWorldStateMgrThread::tellUserConnectOPC()
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
bool DummyWorldStateMgrThread::tellUserOPCConnected()
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
bool DummyWorldStateMgrThread::updateWorldState()
{
    // playback mode
    playbackPaused = false;
    yInfo("updating world state from playback file");

    // TODO: opcPort.write() should be here instead of inner functions

    return true;
}

/**********************************************************/
void DummyWorldStateMgrThread::setPlaybackFile(const string &file)
{
    playbackFile = file;
}

/**********************************************************/
void DummyWorldStateMgrThread::fsmPlayback()
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

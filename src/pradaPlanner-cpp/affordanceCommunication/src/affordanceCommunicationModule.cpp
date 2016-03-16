/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2015 VisLab, Institute for Systems and Robotics,
 *                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Alexandre Antunes <aleksander88@gmail.com>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 */

#include "affordanceCommunicationModule.h"

bool affComm::configure(ResourceFinder &rf)
{
    // module parameters
    vector<string> temp_vect;
    moduleName = rf.check("name", Value("affordanceCommunication")).asString();
    PathName = rf.findPath("contexts/"+rf.getContext());
    setName(moduleName.c_str());
    display = rf.check("display",Value("off")).asString()=="on"?true:false;
    yInfo("MATLAB display is %s", (display ? "on" : "off"));

    temp_vect.clear();
    temp_vect.push_back("1");
    temp_vect.push_back("grasp");
    translation.push_back(temp_vect);
    temp_vect.clear();
    temp_vect.push_back("2");
    temp_vect.push_back("drop");
    translation.push_back(temp_vect);
    temp_vect.clear();
    temp_vect.push_back("3");
    temp_vect.push_back("put");
    translation.push_back(temp_vect);
    temp_vect.clear();
    temp_vect.push_back("4");
    temp_vect.push_back("pull");
    translation.push_back(temp_vect);
    temp_vect.clear();
    temp_vect.push_back("5");
    temp_vect.push_back("push");
    translation.push_back(temp_vect);
    temp_vect.clear();
    temp_vect.push_back("6");
    temp_vect.push_back("reach");
    translation.push_back(temp_vect);
    // translation = {{"1","grasp"},{"2","drop"},{"3","put"},{"4","pull"},{"5","push"},{"6","reach"}};

    if (PathName==""){
        yError("Path to contexts/%s not found", rf.getContext().c_str());
        return false;    
    }
    else {
        yInfo("Context Found!");
    }

    openPorts();

    if (!affordancesCycle())
    {
        yError("Something went wrong with the module execution");
        return false;
    }
    return true;
}

double affComm::getPeriod()
{
    return 0.1;
}

bool affComm::updateModule()
{
    return !isStopping();
}

void affComm::openPorts()
{
    string portName;

    portName = "/" + moduleName + "/ground_cmd:io";
    geoPort.open(portName);

    portName = "/" + moduleName + "/planner_cmd:io";
    plannerPort.open(portName);

    portName = "/" + moduleName + "/aff_query:io";
    affnetPort.open(portName);

    // RPC ports
    portName = "/" + moduleName + "/opc2prada_query:io";
    descQueryPort.open(portName);

    portName = "/" + moduleName + "/actInt_rpc:o";
    actionQueryPort.open(portName);

    portName = "/" + moduleName + "/planner_rpc:o";
    objectQueryPort.open(portName);
}

bool affComm::close()
{
    yInfo("Closing...");
    plannerPort.close();
    geoPort.close();
    affnetPort.close();

    descQueryPort.close();
    actionQueryPort.close();
    return true;
}

bool affComm::interrupt()
{
    yInfo("Interrupting ports");
    plannerPort.interrupt();
    geoPort.interrupt();
    affnetPort.interrupt();

    descQueryPort.interrupt();
    actionQueryPort.interrupt();
    return true;
}

bool affComm::plannerCommand()
{
    if (plannerPort.getInputCount() == 0)
    {
        //cout << "planner not connected" << endl;
        yError("Planner not connected");
        return false;
    }
    while (!isStopping())
    {
        plannerBottle = plannerPort.read(false);
        if (plannerBottle != NULL)
        {
            command = plannerBottle->toString().c_str();
            return true;
        }
        yarp::os::Time::delay(0.1);
    }
    return false;
}

bool affComm::loadObjs()
{
    vector<string> temp_vect;
    if (objectQueryPort.getOutputCount() == 0)
    {
        yError("Planner not connected");
        return false;
    }
    objects.clear();
    tools.clear();
    cmd.clear();
    cmd.addString("printObjects");
    objectQueryPort.write(cmd,reply);
    if (reply.size() > 0 && reply.get(0).isList() && reply.get(0).asList()->size() > 2)
    {
        yInfo("Objects updated!");
        for (int i = 0; i < reply.get(0).asList()->size(); ++i)
        {
            temp_vect.clear();
            temp_vect.push_back( NumbertoString(reply.get(0).asList()->get(i).asList()->get(0).asInt() ) );
            temp_vect.push_back(reply.get(0).asList()->get(i).asList()->get(1).asString());
            yDebug("%s - %s", temp_vect[0].c_str(), temp_vect[1].c_str());
            objects.push_back(temp_vect);
            string check_str = temp_vect[1];
            transform(check_str.begin(), check_str.end(), check_str.begin(), ::tolower);
            if (check_str == "stick" || check_str == "rake")
            {
                tools.push_back(temp_vect);
            }
        }
        return true;
    }
    else {
        yError("Objects update failed");
        return false;
    }
    return false;
}

bool affComm::affordancesCycle()
{
    while (!isStopping())
    {
        if (plannerPort.getInputCount() == 0)
        {
            yWarning("Planner not connected, waiting...");
            yarp::os::Time::delay(1);
        }
        else
        {
            if (plannerCommand())
            {
                if (command == "update")
                {
                    yInfo("Command received: update");
                    Bottle& planner_bottle_out = plannerPort.prepare();
                    planner_bottle_out.clear();
                    planner_bottle_out.addString("ready");
                    plannerPort.write();
                }
            }
        }
        if (command == "query")
        {
            yInfo("Command received: query");
            if (!plannerQuery())
            {
                yError("Failed to perform query");
                continue;
            }
        }
        if (command == "update")
        {
            if (!switchDisplay())
            {
                yError("Failed to set graphics display %s", (display ? "on" : "off"));
                continue;
            }
            if (!loadObjs())
            {
                yError("Failed to initialize objects");
                continue;
            }
            yInfo("Objects loaded");
            if (!queryDescriptors())
            {
                yError("Failed to obtain object descriptors");
                continue;
            }
            if (!queryToolDescriptors())
            {
                yError("Failed to obtain tool descriptors");
                continue;
            }
            posits.clear();
            if (!updateAffordances())
            {
                yError("Failed to update affordances");
                continue;
            }
        }
        yarp::os::Time::delay(0.1);
    }
    return true;
}

bool affComm::switchDisplay()
{
    if (affnetPort.getOutputCount() == 0)
    {
        yWarning("Affordance database not connected, using default values");
        return true;
    }
    else
    {
        Bottle& displayBottle = affnetPort.prepare();
        displayBottle.clear();
        string displayMessage = (display ? "displayON" : "displayOFF");
        displayBottle.addString(displayMessage.c_str());
        affnetPort.write();
    }
    return true;
}

bool affComm::queryDescriptors()
{
    vector<double> data;
    if (descQueryPort.getOutputCount() == 0)
    {
        yError("opc2prada not connected");
        return false;
    }
    descriptors.clear();
    for (int i = 0; i < objects.size(); ++i)
    {
        if (objects[i][0] != "11" && objects[i][0] != "12")
        {
            cmd.clear();
            cmd.addString("query2d");
            cmd.addInt(atoi(objects[i][0].c_str()));
            reply.clear();
            descQueryPort.write(cmd, reply);
            if (reply.size() == 1){
                if (reply.get(0).asList()->size() > 0 && reply.get(0).asList()->get(0).asList()->size() > 0)
                {
                    data.clear();
                    data.push_back(atof(objects[i][0].c_str()));
                    for (int j = 0; j < reply.get(0).asList()->get(0).asList()->size(); ++j)
                    {
                        data.push_back(reply.get(0).asList()->get(0).asList()->get(j).asDouble());
                    }
                    descriptors.push_back(data);
                }
                else
                {
                    data.clear();
                    data.push_back(atof(objects[i][0].c_str()));
                    data.push_back(0.0);
                    data.push_back(0.0);
                    data.push_back(0.0);
                    data.push_back(0.0);
                    data.push_back(0.0);
                    data.push_back(0.0);
                    data.push_back(0.0);
                    descriptors.push_back(data);
                }
                yDebug("Object descriptors:");
                for (int d = 0; d < descriptors.size(); ++d)
                {
                    yDebug() << descriptors[d];
                }
            }
        }
    }
    return true;
}

bool affComm::queryToolDescriptors()
{
    vector<double> data, temp_vect;
    vector<vector<double> > tool_data;
    tooldescriptors.clear();
    for (int i = 0; i < objects.size(); ++i) //tools
    {
        if (objects[i][0] != "11" && objects[i][0] != "12")
        {
            cmd.clear();
            cmd.addString("querytool2d");
            cmd.addInt(atoi(objects[i][0].c_str())); // tools
            reply.clear();
            descQueryPort.write(cmd,reply);
            if (reply.size() == 1){
                if (reply.get(0).asList()->size() > 0 && reply.get(0).asList()->get(0).asList()->size() > 1)
                {
                    //yDebug("reply: %s", reply.toString().c_str());
                    data.clear();
                    tool_data.clear();
                    data.push_back(atof(objects[i][0].c_str())); // tools
                    tool_data.push_back(data);
                    data.clear();
                    //yDebug("storing tool descriptors");
                    //yDebug("size %d", reply.get(0).asList()->size());
                    //yDebug("size %d", reply.get(0).asList()->get(0).asList()->size());
                    //yDebug("size %d", reply.get(0).asList()->get(0).asList()->get(0).asList()->size());
                    if (reply.get(0).asList()->get(0).asList()->get(0).isList())
                    {
                        for (int j = 0; j < reply.get(0).asList()->get(0).asList()->get(0).asList()->size(); ++j)
                        {
                            data.push_back(reply.get(0).asList()->get(0).asList()->get(0).asList()->get(j).asDouble());
                        }
                        tool_data.push_back(data);
                        //yDebug("got first batch");
                        data.clear();
                        for (int j = 0; j < reply.get(0).asList()->get(0).asList()->get(1).asList()->size(); ++j)
                        {
                            data.push_back(reply.get(0).asList()->get(0).asList()->get(1).asList()->get(j).asDouble());
                        }
                        //yDebug("got second batch");
                        tool_data.push_back(data);
                        tooldescriptors.push_back(tool_data);
                    }
                    else
                    {
                        yWarning("failed obtaining descriptors, using default descriptors");
                        tool_data.clear();
                        temp_vect.clear();
                        temp_vect.push_back(atof(objects[i][0].c_str())); // tools
                        tool_data.push_back(temp_vect);
                        temp_vect.clear();
                        temp_vect.push_back(0.0);
                        temp_vect.push_back(0.0);
                        temp_vect.push_back(0.0);
                        temp_vect.push_back(0.0);
                        temp_vect.push_back(0.0);
                        temp_vect.push_back(0.0);
                        tool_data.push_back(temp_vect);
                        temp_vect.clear();
                        temp_vect.push_back(0.0);
                        temp_vect.push_back(0.0);
                        temp_vect.push_back(0.0);
                        temp_vect.push_back(0.0);
                        temp_vect.push_back(0.0);
                        temp_vect.push_back(0.0);
                        tool_data.push_back(temp_vect);
                        tooldescriptors.push_back(tool_data);
                    }
                }
                else
                {
                    yWarning("Default descriptors");
                    tool_data.clear();
                    temp_vect.clear();
                    temp_vect.push_back(atof(objects[i][0].c_str())); // tools
                    tool_data.push_back(temp_vect);
                    temp_vect.clear();
                    temp_vect.push_back(0.0);
                    temp_vect.push_back(0.0);
                    temp_vect.push_back(0.0);
                    temp_vect.push_back(0.0);
                    temp_vect.push_back(0.0);
                    temp_vect.push_back(0.0);
                    tool_data.push_back(temp_vect);
                    temp_vect.clear();
                    temp_vect.push_back(0.0);
                    temp_vect.push_back(0.0);
                    temp_vect.push_back(0.0);
                    temp_vect.push_back(0.0);
                    temp_vect.push_back(0.0);
                    temp_vect.push_back(0.0);
                    tool_data.push_back(temp_vect);
                    tooldescriptors.push_back(tool_data);
                }
                yDebug("Tool descriptors:");
                for (int d = 0; d < tooldescriptors.size(); ++d)
                {
                    yDebug() << tooldescriptors[d];
                }
            }
        }
    }
    return true;
}

bool affComm::plannerQuery()
{
    if (plannerPort.getInputCount() == 0)
    {
        yError("Planner not connected");
        return false;
    }
    vector<double> new_posits;
    vector<string> toolsIDs;
    string message = "", temp_str;
    for (int i = 0; i < posits.size(); ++i)
    {
        std::ostringstream sin;
        sin << posits[i][0];
        temp_str = sin.str();
        if (find_element(toolsIDs, temp_str) == 0)
        {
            toolsIDs.push_back(temp_str);
            new_posits.push_back(posits[i][0]);
            new_posits.push_back(posits[i][1]);
            new_posits.push_back(posits[i][2]);
            
            std::ostringstream sin1, sin2, sin3;
            sin1 << int(posits[i][0]);
            sin2 << int(posits[i][1]);
            sin3 << int(posits[i][2]);
            message = message + sin1.str() + " " + sin2.str() + " " + sin3.str() + " ";
            yDebug("Coordinates: %s", message.c_str());
        }
    }
    Bottle& planner_bottle_out = plannerPort.prepare();
    planner_bottle_out.clear();
    planner_bottle_out.addString(message);
    plannerPort.write();
    return true;
}

bool affComm::updateAffordances()
{
    string comm;
    vector<string> data, temp_vect;
    while (!isStopping())
    {
        comm = "";
        while (!isStopping())
        {
            if (geoPort.getInputCount() == 0)
            {
                yError("Geometric grounding module not connected");
                return false;
            }
            Affor_bottle_in = geoPort.read(false);
            if (Affor_bottle_in)
            {
                comm = Affor_bottle_in->toString();
                break;
            }
            yarp::os::Time::delay(0.1);
        }
        if (comm == "done")
        {
            command = "";
            break;
        }
        if (comm == "update")
        {
            while (!isStopping())
            {
                Affor_bottle_in = geoPort.read(false);
                if (Affor_bottle_in)
                {
                    data.clear();
                    for (int i = 0; i < Affor_bottle_in->size(); ++i)
                    {
                        data.push_back(Affor_bottle_in->get(i).asString());
                    }
                    break;
                }
                yarp::os::Time::delay(0.1);
            }
            rule = data[0];
            context = data[1];
            outcome = data[2];
            outcome2 = data[3];
            outcome3 = data[4];
            for (int i = 0; i < translation.size(); ++i)
            {
                act.clear();
                act = split(rule, '_');
                while (!isStopping())
                {
                    if (act[0].find(" ") != std::string::npos)
                    {
                        act[0].replace(act[0].find(" "),1,""); // remove space
                    }
                    else if (act[3].find("(") != std::string::npos)
                    {
                        act[3].replace(act[3].find("("),2,""); // remove "()"
                    }
                    else
                    {
                        break;
                    }
                }
                if (translation[i][1] == act[0])
                {
                    if (translation[i][1] == "grasp")
                    {
                        if (!getGraspAff())
                        {
                            yError("Failed to obtain grasping affordances");
                            return false;
                        }
                    }
                    if (translation[i][1] == "push")
                    {
                        if (!getPushAff())
                        {
                            yError("Failed to obtain pushing affordances");
                            return false;
                        }
                    }
                    if (translation[i][1] == "pull")
                    {
                        if (!getPullAff())
                        {
                            yError("Failed to obtain pulling affordances");
                            return false;
                        }
                    }
                    if (translation[i][1] == "drop")
                    {
                        if (!getDropAff())
                        {
                            yError("Failed to obtain dropping affordances");
                            return false;
                        }
                    }
                    if (translation[i][1] == "put")
                    {
                        if (!getPutAff())
                        {
                            yError("Failed to obtain putting affordances");
                            return false;
                        }
                    }
                }
            }
        }
    }
    return true;
}

bool affComm::sendOutcomes()
{
    if (geoPort.getInputCount() == 0)
    {
        yError("Geometric grounding module not connected");
        return false;
    }
    Bottle& Affor_bottle_out = geoPort.prepare();
    Affor_bottle_out.clear();
    Affor_bottle_out.addString(outcome);
    Affor_bottle_out.addString(outcome2);
    Affor_bottle_out.addString(outcome3);
    geoPort.write();
    return true;
}

bool affComm::getGraspAff()
{
    if (!sendOutcomes())
    {
        yError("Failed to send probabilities to the grounding module");
        return false;
    }
    return true;
}

bool affComm::getDropAff()
{
    if (!sendOutcomes())
    {
        yError("Failed to send probabilities to the grounding modules");
        return false;
    }
    return true;
}

bool affComm::getPutAff()
{
    if (!sendOutcomes())
    {
        yError("Failed to send probabilities to the grounding modules");
        return false;
    }
    return true;
}

bool affComm::getPushAff()
{
    vector<double> data, obj_desc, tool_desc1, tool_desc2;
    vector<string> new_outcome, new_outcome2;
    string obj, tool, new_outcome_string, new_outcome2_string;
    double prob_succ1, prob_succ2, prob_fail, prob_succ;
    int toolnum=0;
    yDebug("Push: %s %s %s %s", act[0].c_str(), act[1].c_str(), act[2].c_str(), act[3].c_str());
    if (affnetPort.getInputCount() == 0)
    {
        yWarning("Affordances network not connected, going for default values");
        if (!sendOutcomes())
        {
            yError("Failed to send probabilities to the grounding modules");
            return false;
        }
        if (act[1] != "11" && act[1] != "12" && act[3] != "11" && act[3] != "12")
        {
            for (int j = 0; j < tooldescriptors.size(); ++j)
            {
                if (strtof(act[3].c_str(), NULL) == tooldescriptors[j][0][0])
                {
                    data.clear();
                    data.push_back(strtof(act[3].c_str(), NULL));
                    if (tooldescriptors[j][1][1] > tooldescriptors[j][2][1])
                    {
                        data.push_back(tooldescriptors[j][1][0]);
                        data.push_back(tooldescriptors[j][1][1]);
                    }
                    else
                    {
                        data.push_back(tooldescriptors[j][2][0]);
                        data.push_back(tooldescriptors[j][2][1]);
                    }
                    posits.push_back(data);
                }
            }
        }
        return true;
    }
    else
    {
        if (act[1] != "11" && act[1] != "12" && act[3] != "11" && act[3] != "12")
        {
            Bottle& affnet_bottle_out = affnetPort.prepare();
            affnet_bottle_out.clear();
            obj = act[1];
            tool = act[3];
            for (int o = 0; o < descriptors.size(); ++o)
            {
                if (descriptors[o][0] == strtof(obj.c_str(), NULL))
                {
                    obj_desc = descriptors[o];
                }
            }
            for (int o = 0; o < tooldescriptors.size(); ++o)
            {
                if (tooldescriptors[o][0][0] == strtof(tool.c_str(),NULL))
                {
                    tool_desc1 = tooldescriptors[o][1];
                    tool_desc2 = tooldescriptors[o][2];
                    toolnum = o;
                }
            }
            if (tooldescriptors[toolnum][1][1] < tooldescriptors[toolnum][2][1])
            {
                for (int j = 3; j < tool_desc1.size(); ++j)
                {
                    affnet_bottle_out.addDouble(tool_desc1[j]);
                }
                for (int j = 2; j < obj_desc.size(); ++j)
                {
                    affnet_bottle_out.addDouble(obj_desc[j]);
                }
                affnet_bottle_out.addDouble(2.0);
                affnetPort.write();
                while (!isStopping())
                {
                    affnet_bottle_in = affnetPort.read(false);
                    if (affnet_bottle_in)
                    {
                        yDebug() << affnet_bottle_in->toString();
                        if (affnet_bottle_in->toString() == "nack")
                        {
                            yWarning("Query failed, using default values");
                            if (!sendOutcomes())
                            {
                                yError("Failed to send probabilities to the grounding modules");
                                return false;
                            } 
                            return true;
                        }
                        break;
                    }
                    yarp::os::Time::delay(0.1);
                }
                prob_succ1 = 0.0;
                //prob_not_moving1 = 0.0;
                for (int g = 0; g < affnet_bottle_in->get(0).asList()->size(); ++g)
                {
                    if (g < 2)
                    {
                        for (int j = 0; j < affnet_bottle_in->get(0).asList()->get(g).asList()->size(); ++j)
                        {
                            prob_succ1 = prob_succ1 + affnet_bottle_in->get(0).asList()->get(g).asList()->get(j).asDouble();
                        }
                    }
                    /*if (g == 2)
                    {
                        for (int j = 0; j < affnet_bottle_in->get(0).asList()->get(g).asList()->size(); ++j)
                        {
                            prob_not_moving1 = prob_not_moving1 + affnet_bottle_in->get(0).asList()->get(g).asList()->get(j).asDouble();
                        }
                    }*/
                }
                yDebug("Success probability was: %f", prob_succ1);
                if (prob_succ1 >= 0.95)
                {
                    prob_succ1 = 0.95;
                }
                if (prob_succ1 < 0.5)
                {
                    prob_succ1 = 0.5;
                }
                /*if (prob_succ1 + prob_not_moving1 >= 0.95)
                {
                    prob_succ1 = 0.95*prob_succ1;
                    prob_not_moving1 = 0.95*prob_not_moving1;
                }
                prob_not_moving = prob_not_moving1;*/
                prob_succ = prob_succ1;
                data.clear();
                data.push_back(strtof(tool.c_str(), NULL));
                data.push_back(tooldescriptors[toolnum][1][0]);
                data.push_back(tooldescriptors[toolnum][1][1]);
                posits.push_back(data);
            }
            else
            {
                for (int j = 3; j < tool_desc2.size(); ++j)
                {
                    affnet_bottle_out.addDouble(tool_desc2[j]);
                }
                for (int j = 2; j < obj_desc.size(); ++j)
                {
                    affnet_bottle_out.addDouble(obj_desc[j]);
                }
                affnet_bottle_out.addDouble(2.0);
                affnetPort.write();
                while (!isStopping())
                {
                    affnet_bottle_in = affnetPort.read(false);
                    if (affnet_bottle_in)
                    {
                        yDebug() << affnet_bottle_in->toString();
                        if (affnet_bottle_in->toString() == "nack")
                        {
                            yWarning("Query failed, using default values");
                            if (!sendOutcomes())
                            {
                                yError("Failed to send probabilities to the grounding modules");
                                return false;
                            } 
                            return true;
                        }
                        break;
                    }
                    yarp::os::Time::delay(0.1);
                }
                prob_succ2 = 0.0;
                //prob_not_moving2 = 0.0;
                for (int g = 0; g < affnet_bottle_in->get(0).asList()->size(); ++g)
                {
                    if (g < 2)
                    {
                        for (int j = 0; j < affnet_bottle_in->get(0).asList()->get(g).asList()->size(); ++j)
                        {
                            prob_succ2 = prob_succ2 + affnet_bottle_in->get(0).asList()->get(g).asList()->get(j).asDouble();
                        }
                    }
                    /*if (g == 2)
                    {
                        for (int j = 0; j < affnet_bottle_in->get(0).asList()->get(g).asList()->size(); ++j)
                        {
                            prob_not_moving2 = prob_not_moving2 + affnet_bottle_in->get(0).asList()->get(g).asList()->get(j).asDouble();
                        }
                    }*/
                }
                yDebug("Success probability was: %f", prob_succ2);
                if (prob_succ2 >= 0.95)
                {
                    prob_succ2 = 0.95;
                }
                if (prob_succ2 < 0.5)
                {
                    prob_succ2 = 0.5;
                }
                /*if (prob_succ2 + prob_not_moving2 >= 0.95)
                {
                    prob_succ2 = 0.95*prob_succ2;
                    prob_not_moving2 = 0.95*prob_not_moving2;
                }
                prob_not_moving = prob_not_moving1;*/
                prob_succ = prob_succ2;
                data.clear();
                data.push_back(strtof(tool.c_str(), NULL));
                data.push_back(tooldescriptors[toolnum][2][0]);
                data.push_back(tooldescriptors[toolnum][2][1]);
                posits.push_back(data);
            }
            std::ostringstream sin, sfail;
            //std::ostringstream snotmove;
            prob_fail = 1.0 - prob_succ;
            //prob_fail = 1.0 - prob_succ - prob_not_moving;
            new_outcome = split(outcome, ' ');
            sin << prob_succ;
            new_outcome[2] = sin.str();
            /*new_outcome2 = split(outcome2, ' ');
            snotmove << prob_not_move;
            new_outcome2[2] = snotmove.str();
            new_outcome3 = split(outcome3, ' ');
            sfail << prob_fail;
            new_outcome3[2] = sfail.str();*/
            new_outcome2 = split(outcome2, ' ');
            sfail << prob_fail;
            new_outcome2[2] = sfail.str();
            new_outcome_string = "";
            for (int i = 0; i < new_outcome.size(); ++i)
            {
                new_outcome_string = new_outcome_string + new_outcome[i] + " ";
            }
            outcome = new_outcome_string;
            new_outcome2_string = "";
            for (int i = 0; i < new_outcome2.size(); ++i)
            {
                new_outcome2_string = new_outcome2_string + new_outcome2[i] + " ";
            }
            outcome2 = new_outcome2_string;
            /*new_outcome3_string = "";
            for (int i = 0; i < new_outcome3.size(); ++i)
            {
                new_outcome3_string = new_outcome3_string + new_outcome3[i] + " ";
            }
            outcome3 = new_outcome3_string;*/
            if (!sendOutcomes())
            {
                yError("Failed to send probabilities to the grounding module");
                return false;
            }
        }
        else
        {
            if (!sendOutcomes())
            {
                yError("Failed to send probabilities to the grounding module");
                return false;
            }
        }
        return true;

    }
}
bool affComm::getPullAff()
{
    vector<double> data, obj_desc, tool_desc1, tool_desc2;
    vector<string> new_outcome, new_outcome2;
    string obj, tool, new_outcome_string, new_outcome2_string;
    double prob_succ1, prob_succ2, prob_fail, prob_succ;
    int toolnum=0;

    yDebug("Pull: %s %s %s %s", act[0].c_str(), act[1].c_str(), act[2].c_str(), act[3].c_str());
    if (affnetPort.getInputCount() == 0)
    {
        yWarning("Affordances network not connected, going for default");
        if (!sendOutcomes())
        {
            yError("Failed to send probabilities to the grounding module");
            return false;
        }
        if (act[1] != "11" && act[1] != "12" && act[3] != "11" && act[3] != "12")
        {
            for (int j = 0; j < tooldescriptors.size(); ++j)
            {
                if (strtof(act[3].c_str(), NULL) == tooldescriptors[j][0][0])
                {
                    data.clear();
                    data.push_back(strtof(act[3].c_str(), NULL));
                    if (tooldescriptors[j][1][1] > tooldescriptors[j][2][1])
                    {
                        data.push_back(tooldescriptors[j][1][0]);
                        data.push_back(tooldescriptors[j][1][1]);
                    }
                    else
                    {
                        data.push_back(tooldescriptors[j][2][0]);
                        data.push_back(tooldescriptors[j][2][1]);
                    }
                    posits.push_back(data);
                }
            }
        }
        return true;
    }
    else
    {
        if (act[1] != "11" && act[1] != "12" && act[3] != "11" && act[3] != "12")
        {
            Bottle& affnet_bottle_out = affnetPort.prepare();
            affnet_bottle_out.clear();
            obj = act[1];
            tool = act[3];
            for (int o = 0; o < descriptors.size(); ++o)
            {
                if (descriptors[o][0] == strtof(obj.c_str(), NULL))
                {
                    obj_desc = descriptors[o];
                }
            }
            for (int o = 0; o < tooldescriptors.size(); ++o)
            {
                if (tooldescriptors[o][0][0] == strtof(tool.c_str(),NULL))
                {
                    tool_desc1 = tooldescriptors[o][1];
                    tool_desc2 = tooldescriptors[o][2];
                    toolnum = o;
                }
            }
            // select the half with lower y coord (further from the robot i.e. top of the screen)
            if (tooldescriptors[toolnum][1][1] < tooldescriptors[toolnum][2][1])
            {
                for (int j = 3; j < tool_desc1.size(); ++j)
                {
                    affnet_bottle_out.addDouble(tool_desc1[j]);
                }
                for (int j = 2; j < obj_desc.size(); ++j)
                {
                    affnet_bottle_out.addDouble(obj_desc[j]);
                }
                affnet_bottle_out.addDouble(1.0);
                affnetPort.write();
                while (!isStopping())
                {
                    affnet_bottle_in = affnetPort.read(false);
                    if (affnet_bottle_in)
                    {
                        yDebug() << affnet_bottle_in->toString();
                        if (affnet_bottle_in->toString() == "nack")
                        {
                            yWarning("Query failed, using default values");
                            if (!sendOutcomes())
                            {
                                yError("Failed to send probabilities to the grounding module");
                                return false;
                            } 
                            return true;
                        }
                        break;
                    }
                    yarp::os::Time::delay(0.1);
                }
                prob_succ1 = 0.0;
                //prob_not_moving1 = 0.0;
                for (int g = 0; g < affnet_bottle_in->get(0).asList()->size(); ++g)
                {
                    if (g > 2)
                    {
                        for (int j = 0; j < affnet_bottle_in->get(0).asList()->get(g).asList()->size(); ++j)
                        {
                            prob_succ1 = prob_succ1 + affnet_bottle_in->get(0).asList()->get(g).asList()->get(j).asDouble();
                        }
                    }
                    /*if (g == 2)
                    {
                        for (int j = 0; j < affnet_bottle_in->get(0).asList()->get(g).asList()->size(); ++j)
                        {
                            prob_not_moving1 = prob_not_moving1 + affnet_bottle_in->get(0).asList()->get(g).asList()->get(j).asDouble();
                        }
                    }*/
                }
                yDebug("Success probability was: %f", prob_succ1);
                if (prob_succ1 >= 0.95)
                {
                    prob_succ1 = 0.95;
                }
                if (prob_succ1 < 0.5)
                {
                    prob_succ1 = 0.5;
                }
                /*if (prob_succ1 + prob_not_moving1 >= 0.95)
                {
                    prob_succ1 = 0.95*prob_succ1;
                    prob_not_moving1 = 0.95*prob_not_moving1;
                }
                prob_not_moving = prob_not_moving1;*/
                prob_succ = prob_succ1;
                data.clear();
                data.push_back(strtof(tool.c_str(), NULL));
                data.push_back(tooldescriptors[toolnum][1][0]);
                data.push_back(tooldescriptors[toolnum][1][1]);
                posits.push_back(data);
            }
            else
            {
                for (int j = 3; j < tool_desc2.size(); ++j)
                {
                    affnet_bottle_out.addDouble(tool_desc2[j]);
                }
                for (int j = 2; j < obj_desc.size(); ++j)
                {
                    affnet_bottle_out.addDouble(obj_desc[j]);
                }
                affnet_bottle_out.addDouble(1.0);
                affnetPort.write();
                while (!isStopping())
                {
                    affnet_bottle_in = affnetPort.read(false);
                    if (affnet_bottle_in)
                    {
                        yDebug() << affnet_bottle_in->toString();
                        if (affnet_bottle_in->toString() == "nack")
                        {
                            yWarning("Query failed, using default values");
                            if (!sendOutcomes())
                            {
                                yError("Failed to send probabilities to the grounding module");
                                return false;
                            } 
                            return true;
                        }
                        break;
                    }
                    yarp::os::Time::delay(0.1);
                }
                prob_succ2 = 0.0;
                //prob_not_moving2 = 0.0;
                for (int g = 0; g < affnet_bottle_in->get(0).asList()->size(); ++g)
                {
                    if (g > 2)
                    {
                        for (int j = 0; j < affnet_bottle_in->get(0).asList()->get(g).asList()->size(); ++j)
                        {
                            prob_succ2 = prob_succ2 + affnet_bottle_in->get(0).asList()->get(g).asList()->get(j).asDouble();
                        }
                    }
                    /*if (g == 2)
                    {
                        for (int j = 0; j < affnet_bottle_in->get(0).asList()->get(g).asList()->size(); ++j)
                        {
                            prob_not_moving2 = prob_not_moving2 + affnet_bottle_in->get(0).asList()->get(g).asList()->get(j).asDouble();
                        }
                    }*/
                }
                yDebug("Success probability was: %f", prob_succ2);
                if (prob_succ2 >= 0.95)
                {
                    prob_succ2 = 0.95;
                }
                if (prob_succ2 < 0.5)
                {
                    prob_succ2 = 0.5;
                }
                /*if (prob_succ2 + prob_not_moving2 >= 0.95)
                {
                    prob_succ2 = 0.95*prob_succ2;
                    prob_not_moving2 = 0.95*prob_not_moving2;
                }
                prob_not_moving = prob_not_moving2;*/
                prob_succ = prob_succ2;
                data.clear();
                data.push_back(strtof(tool.c_str(), NULL));
                data.push_back(tooldescriptors[toolnum][2][0]);
                data.push_back(tooldescriptors[toolnum][2][1]);
                posits.push_back(data);
            }
            std::ostringstream sin, sfail;
            //std::ostringstream snotmove;
            //prob_fail = 1.0 - prob_succ - prob_not_moving;
            prob_fail = 1.0 - prob_succ;
            new_outcome = split(outcome, ' ');
            sin << prob_succ;
            new_outcome[2] = sin.str();
            /*new_outcome2 = split(outcome2, ' ');
            snotmove << prob_not_move;
            new_outcome2[2] = snotmove.str();
            new_outcome3 = split(outcome3, ' ');
            sfail << prob_fail;
            new_outcome3[2] = sfail.str();*/
            new_outcome2 = split(outcome2, ' ');
            sfail << prob_fail;
            new_outcome2[2] = sfail.str();
            new_outcome_string = "";
            for (int i = 0; i < new_outcome.size(); ++i)
            {
                new_outcome_string = new_outcome_string + new_outcome[i] + " ";
            }
            outcome = new_outcome_string;
            new_outcome2_string = "";
            for (int i = 0; i < new_outcome2.size(); ++i)
            {
                new_outcome2_string = new_outcome2_string + new_outcome2[i] + " ";
            }
            outcome2 = new_outcome2_string;
            /*new_outcome3_string = "";
            for (int i = 0; i < new_outcome3.size(); ++i)
            {
                new_outcome3_string = new_outcome3_string + new_outcome3[i] + " ";
            }
            outcome3 = new_outcome3_string;*/
            if (!sendOutcomes())
            {
                yError("Failed to send probabilities to the grounding module");
                return false;
            }
        }
        else
        {
            if (!sendOutcomes())
            {
                yError("Failed to send probabilities to the grounding module");
                return false;
            }
        }
        return true;

    }
}

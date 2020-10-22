/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2015 VisLab, Institute for Systems and Robotics,
 *                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Alexandre Antunes <aleksander88@gmail.com>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 */

#include "goalCompilerModule.h"

bool goalCompiler::configure(ResourceFinder &rf)
{
    // module parameters
    moduleName = rf.check("name", Value("goalCompiler")).asString();
    setName(moduleName.c_str());

    PathName = rf.findPath("contexts/"+rf.getContext());
    if (PathName=="")
    {
        yError("Path to context %s not found", rf.getContext().c_str());
        return false;
    }
    else {
        yInfo("Context FOUND! %s", PathName.c_str());
    }

    bool useGoalSimplification = rf.check("goalSimplification",Value("on")).asString()=="on"?true:false;
    yInfo("goal simplification (removal of hand-specific goals) is %s", useGoalSimplification?"on":"off");

    // set filenames
    goalFileName =PathName + "/goal.dat";
    subgoalFileName = PathName + "/subgoals.dat";
    preRuleFileName = PathName + "/" + rf.check("prerules",Value("pre_rules.dat")).asString();
    yInfo("pre-rules filename: %s", preRuleFileName.c_str());

    openPorts();

    string command;

    while (!isStopping())
    {
        yarp::os::Time::delay(0.1);
        if (plannerPort.getInputCount() == 0)
        {
            yWarning("planner not connected");
            yarp::os::Time::delay(5);
        }
        command = "";
        command = plannerCommand();
        yDebug("Command received: %s", command.c_str());
/*        if (!loadObjs())
        {
            yWarning("failed to load objects");
            if (!plannerReply("failed objects"))
            {
                yError("failed to communicate with planner");
                //return false;
            }
            continue;
        }*/
        if (command == "praxicon")
        {
            if (!loadObjs())
            {
                yWarning("failed to load objects");
                if (!plannerReply("failed_objects"))
                {
                    yError("failed to communicate with planner");
                    //return false;
                }
                continue;
            }
            if (!receiveInstructions())
            {
                yWarning("failed to receive instructions");
                continue;
            }
        }
        else if (command == "update")
        {
            if (!loadObjs())
            {
                yWarning("failed to load objects");
                if (!plannerReply("failed_objects"))
                {
                    yError("failed to communicate with planner");
                    //return false;
                }
                continue;
            }
            if (!loadRules())
            {
                yWarning("failed to load rules");
                if (!plannerReply("failed_rules"))
                {
                    yError("failed to communicate with planner");
                    //return false;
                }
                continue;
            }
            if (!loadInstructions())
            {
                yWarning("failed to load instructions");
                if (!plannerReply("failed_instructions"))
                {
                    yError("failed to communicate with planner");
                    //return false;
                }
                continue;
            }
            if (!compile())
            {
                yWarning("failed to compile goals");
                if (!plannerReply("failed_compiling"))
                {
                    yError("failed to communicate with planner");
                    //return false;
                }
                continue;
            }
            if (!translate())
            {
                yWarning("failed to translate goals");
                if (!plannerReply("failed_translation"))
                {
                    yError("failed to communicate with planner");
                    //return false;
                }
                continue;
            }
            if (!checkConsistency())
            {
                yWarning("failed consistency test");
                if (!plannerReply("failed_consistency"))
                {
                    yError("failed to communicate with planner");
                    //return false;
                }
                continue;
            }
            if (useGoalSimplification)
            {
                if (!clearUnimportantGoals())
                {
                    yWarning("failed clearing unimportant subgoals");
                    if (!plannerReply("failed_pruning"))
                    {
                        yError("failed to communicate with planner");
                        //return false;
                    }
                    continue;
                }
            }
            if (!writeFiles())
            {
                yWarning("failed to write files");
                if (!plannerReply("failed_writing"))
                {
                    yError("failed to communicate with planner");
                    //return false;
                }
                continue;
            }
            if (!plannerReply("done"))
            {
                yError("failed to communicate with planner");
                //return false;
            }
        }
    }
    close();
    return true;
}

bool goalCompiler::updateModule()
{
    return !isStopping();
}

void goalCompiler::openPorts()
{
    string portName;

    portName = "/" + moduleName + "/planner_cmd:io";
    plannerPort.open(portName);

    portName = "/" + moduleName + "/prax_inst:i";
    praxiconPort.open(portName);

    portName = "/" + moduleName + "/planner_rpc:o";
    objectQueryPort.open(portName);
}

bool goalCompiler::close()
{
    yInfo("closing...");
    plannerPort.close();
    praxiconPort.close();
    return true;
}

bool goalCompiler::interrupt()
{
    yInfo("interrupting ports");
    plannerPort.interrupt();
    praxiconPort.interrupt();
    return true;
}

string goalCompiler::plannerCommand()
{
    string command;
    while (!isStopping()){
        yarp::os::Time::delay(0.1);
        plannerBottle = plannerPort.read(false);
        if (plannerBottle != NULL){
            command = plannerBottle->toString().c_str();
            //plannerBottle->clear();
            return command;
        }
    }
    return "stopped";
}

bool goalCompiler::receiveInstructions()
{
    string temp_str;
    int timer_count = 0;
    while (!isStopping()){
        yarp::os::Time::delay(0.1);
        if (timer_count == 3000)
        {
            yError("timeout: no instructions received before 5 minutes time");
            Bottle &plannerBottleOut = plannerPort.prepare();
            plannerBottleOut.clear();
            plannerBottleOut.addString("failed_Praxicon");
            plannerPort.write();
            return false;
        }
        if (plannerPort.getPendingReads() > 0)
        {
            return true; //there is a new planner command, exit the function and read it
        }
        praxiconBottle = praxiconPort.read(false);
        if (praxiconBottle != NULL){
            if (!loadObjs())
            {
                yWarning("failed to load objects");
                if (!plannerReply("failed_objects"))
                {
                    yError("failed to communicate with planner");
                }
                return false;
            }
            yInfo("bottle received: %s", praxiconBottle->toString().c_str());
            instructions.clear();
            if (praxiconBottle->toString().find("a") != -1){
                for (int g=0; g < praxiconBottle->size(); ++g){
                    vector<string> temp_instructions;
                    for (int y=0; y < praxiconBottle->get(g).asList()->size(); ++y){
                        string temp1_instructions;
                        for (int t=0; t < praxiconBottle->get(g).asList()->get(y).asList()->size(); ++t){
                            temp_str = praxiconBottle->get(g).asList()->get(y).asList()->get(t).toString();
                            if ((t == 0 || t == 2) && (temp_str != "hand") && (find_element(object_list, temp_str) == 0))
                            {
                                Bottle &plannerBottleOut = plannerPort.prepare();
                                plannerBottleOut.clear();
                                plannerBottleOut.addString("unknown");
                                plannerPort.write();
                                yError("failed to compile: unknown object -> %s", temp_str.c_str());
                                return false;
                            }
                            temp1_instructions = temp1_instructions + temp_str + " " ;
                        }
                        temp_instructions.push_back(temp1_instructions);
                    }
                    instructions.push_back(temp_instructions);
                }
                Time::delay(0.1);
                Bottle &plannerBottleOut = plannerPort.prepare();
                plannerBottleOut.clear();
                plannerBottleOut.addString("done");
                plannerPort.write();
                //praxiconBottle->clear();
                return true;
            }
            else
            {
                Bottle &plannerBottleOut = plannerPort.prepare();
                plannerBottleOut.clear();
                plannerBottleOut.addString("empty_bottle");
                plannerPort.write();
                return false;
            }
        }
        if (praxiconPort.getInputCount() == 0)
        {
            yError("praxicon crashed or disconnected");
            Bottle &plannerBottleOut = plannerPort.prepare();
            plannerBottleOut.clear();
            plannerBottleOut.addString("failed_Praxicon");
            plannerPort.write();
            return false;
        }
        timer_count = timer_count + 1;
        Time::delay(0.1);
    }
}

bool goalCompiler::loadObjs()
{
    vector<string> temp_vect;
    if (objectQueryPort.getOutputCount() == 0){
        yError("planner not connected!" );
        return false;
    }
    object_list.clear();
    translat.clear();
    action_sequence.clear();
    cmd.clear();
    subgoals.clear();
    cmd.addString("printObjects");
    objectQueryPort.write(cmd,reply);
    if (reply.size() > 0 && reply.get(0).isList() && reply.get(0).asList()->size() > 2){
        yInfo("Objects updated!");
        for (int i = 0; i < reply.get(0).asList()->size(); ++i)
        {
            temp_vect.clear();
            temp_vect.push_back( NumbertoString(reply.get(0).asList()->get(i).asList()->get(0).asInt() ) );
            temp_vect.push_back(reply.get(0).asList()->get(i).asList()->get(1).asString());
            if (find_element(object_list, temp_vect[1]) == 1)
            {
                yError("There are objects that share labels, unable to compile");
                return false;
            }
            translat.push_back(temp_vect);
            object_list.push_back(temp_vect[1]);
        }
        return true;
    }
    else {
        yError("Objects update failed!");
        return false;
    }
    return false;
}


bool goalCompiler::loadRules()
{
    string line;
    actions.clear();
    preRuleFile.open(preRuleFileName.c_str());
    if (!preRuleFile.is_open())
    {
        yError("unable to open rule file");
        return false;
    }
    while ( getline(preRuleFile, line)){
        actions.push_back(line);
    }
    preRuleFile.close();
    return true;
}

bool goalCompiler::loadInstructions()
{
    for (int g = 0; g < instructions.size(); ++g){
        for (int l = 0; l < instructions[g].size(); ++l){
            if (instructions[g][l].find("hand") != std::string::npos){
                instructions[g][l].replace(instructions[g][l].find("hand"),4,"left");
            }
        }
    }
    return true;
}

bool goalCompiler::compile()
{
    if (instructions[0].size() == 0)
    {
        yError("no instructions to compile.");
        return false;
    }
    vector<string> prax_action, new_action, temp_rule, new_temp_rule, aux_subgoal, temp_vect, temp_action;
    string obj, tool, temp_str, found_action;
    for (int g = 0; g<instructions[0].size(); ++g){
        prax_action = split(instructions[0][g], ' ');
        prax_action[1].push_back('_');
        if (prax_action[1] != "reach_"){
            for (int j = 0; j < actions.size(); ++j){
                if (actions[j].find(prax_action[1]) != std::string::npos){
                    obj = prax_action[2];
                    found_action = actions[j];
                    if (actions[j+4].find("_ALL") != std::string::npos){
                        tool = prax_action[0];
                        new_action = actions;
                        aux_subgoal = split(actions[j+4],' ');
                        for ( int u = 0; u < aux_subgoal.size(); ++u){
                            if (aux_subgoal[u].find("_ALL") != std::string::npos){
                                temp_str = new_action[j+4];
                                while (!isStopping()) {
                                    if (temp_str.find("_obj") != std::string::npos){
                                        temp_str.replace(temp_str.find("_obj"),4,obj);
                                    }
                                    else {
                                        break;
                                    }
                                }
                                while (!isStopping()) {
                                    if (temp_str.find("_tool") != std::string::npos){
                                        temp_str.replace(temp_str.find("_tool"),5,tool);
                                    }
                                    else {
                                        break;
                                    }
                                }
                                while (!isStopping()) {
                                    if (temp_str.find("_hand") != std::string::npos){
                                        temp_str.replace(temp_str.find("_hand"),5,"left");
                                    }
                                    else {
                                        break;
                                    }
                                }
                                temp_rule = split(temp_str,' ');
                                temp_rule.erase(temp_rule.begin(), temp_rule.begin()+1);
                                temp_rule.erase(temp_rule.begin(), temp_rule.begin()+1);
                                temp_rule.erase(temp_rule.begin(), temp_rule.begin()+1);
                                for (int k = 0; k< translat.size(); ++k){
                                    temp_str = aux_subgoal[u];
                                    while (!isStopping()) {
                                        if (temp_str.find("_obj") != std::string::npos){
                                            temp_str.replace(temp_str.find("_obj"),4,obj);
                                        }
                                        else {
                                            break;
                                        }
                                    }
                                    while (!isStopping()) {
                                        if (temp_str.find("_tool") != std::string::npos){
                                            temp_str.replace(temp_str.find("_tool"),5,tool);
                                        }
                                        else {
                                            break;
                                        }
                                    }

                                    if (temp_str.find(translat[k][1]) == std::string::npos){
                                        while (!isStopping()) {
                                            if (temp_str.find("_ALL") != std::string::npos){
                                                temp_str.replace(temp_str.find("_ALL"),4,translat[k][1]);
                                            }
                                            else {
                                                break;
                                            }
                                        }
                                        temp_rule.push_back(temp_str);
                                    }
                                }
                                string var_find;
                                new_temp_rule.clear();
                                for (int w = 0; w < temp_rule.size(); ++w){
                                    int not_add_flag = 0;
                                    if (temp_rule[w].find("-") != 0){
                                        var_find = temp_rule[w];
                                    }
                                    if (temp_rule[w].find("-") == 0){
                                        var_find = temp_rule[w];
                                        var_find.replace(var_find.find("-"),1,"");
                                    }
                                    for (int v = 0; v < new_temp_rule.size(); ++v){
                                        if (new_temp_rule[v].find(var_find) != std::string::npos){
                                            not_add_flag = 1;
                                            break;
                                    }
                                    }
                                    if (not_add_flag != 1){
                                        new_temp_rule.push_back(temp_rule[w]);
                                    }
                                }
                                temp_str = "";
                                for (int h = 0; h<new_temp_rule.size(); ++h){
                                    temp_str += new_temp_rule[h];
                            }
                                new_action[j] = temp_str;
                            }
                        }
                        for (int h = new_temp_rule.size()-1; h>0; --h){
                            if (new_temp_rule[h].find("_ALL") != std::string::npos){
                                new_temp_rule.erase(new_temp_rule.begin()+h);
                            }
                        }
                        for (int m = 0; m < new_temp_rule.size(); ++m){
                            yDebug("new_temp_rule[%d]: %s", m, new_temp_rule[m].c_str());
                        }
                        aux_subgoal = new_temp_rule;
                        vector<string> temp_subgoal;
                        if (subgoals.size() > 0)
                        {
                            temp_subgoal = subgoals[subgoals.size()-1];
                        }
                        for (int i = 0; i < aux_subgoal.size(); ++i){
                            temp_subgoal.push_back(aux_subgoal[i]);
                        }
                        temp_action.clear();
                        temp_action.push_back(found_action);
                        temp_action.push_back(obj);
                        temp_action.push_back(tool);
                        action_sequence.push_back(temp_action);
                        subgoals.push_back(temp_subgoal);
                    }
                    else if (actions[j].find("put_") != std::string::npos){
                        found_action = actions[j];
                        tool = prax_action[2];
                        temp_vect = split(instructions[0][g-1], ' ');
                        obj = temp_vect[2];
                        temp_str = actions[j+4];
                        while (!isStopping()) {
                            if (temp_str.find("_obj") != std::string::npos){
                                temp_str.replace(temp_str.find("_obj"),4,obj);
                            }
                            else {
                                break;
                            }
                        }
                        while (!isStopping()) {
                            if (temp_str.find("_tool") != std::string::npos){
                                temp_str.replace(temp_str.find("_tool"),5,tool);
                            }
                            else {
                                break;
                            }
                        }
                        while (!isStopping()) {
                            if (temp_str.find("_hand") != std::string::npos){
                                temp_str.replace(temp_str.find("_hand"),5,"left");
                            }
                            else {
                                break;
                            }
                        }
                        aux_subgoal = split(temp_str,' ');
                        aux_subgoal.erase(aux_subgoal.begin(),aux_subgoal.begin()+3); // Remove two trailing spaces + probability
                        vector<string> temp_subgoal;
                        if (subgoals.size() > 0)
                        {
                            temp_subgoal = subgoals[subgoals.size()-1];
                        }
                        for (int i = 0; i < aux_subgoal.size(); ++i){
                            temp_subgoal.push_back(aux_subgoal[i]);
                        }
                        yDebug("temp_subgoal:");
                        for (int m = 0; m < temp_subgoal.size(); ++m){
                            yDebug("%d: %s", m, temp_subgoal[m].c_str());
                        }
                        temp_action.clear();
                        temp_action.push_back(found_action);
                        temp_action.push_back(obj);
                        temp_action.push_back(tool);
                        action_sequence.push_back(temp_action);
                        subgoals.push_back(temp_subgoal);
                    }
                    else if ((actions[j].find("_obj") != std::string::npos && actions[j].find("_tool") != std::string::npos) || (actions[j].find("_obj") != std::string::npos && actions[j].find("_hand") != std::string::npos)){
                        found_action = actions[j];
                        tool = prax_action[0];
                        obj = prax_action[2];
                        temp_str = actions[j+4];
                        while (!isStopping()) {
                            if (temp_str.find("_obj") != std::string::npos){
                                temp_str.replace(temp_str.find("_obj"),4,obj);
                            }
                            else {
                                break;
                            }
                        }
                        while (!isStopping()) {
                            if (temp_str.find("_tool") != std::string::npos){
                                temp_str.replace(temp_str.find("_tool"),5,tool);
                            }
                            else {
                                break;
                            }
                        }
                        while (!isStopping()) {
                            if (temp_str.find("_hand") != std::string::npos){
                                temp_str.replace(temp_str.find("_hand"),5,"left");
                            }
                            else {
                                break;
                            }
                        }
                        aux_subgoal = split(temp_str, ' ');
                        for (int m = 0; m < aux_subgoal.size(); ++m){
                            yInfo("%s", aux_subgoal[m].c_str());
                        }
                        aux_subgoal.erase(aux_subgoal.begin(),aux_subgoal.begin()+3); // Remove two trailing spaces + probability
                        for (int m = 0; m < aux_subgoal.size(); ++m){
                            yDebug("aux_subgoal[%d]: %s", m, aux_subgoal[m].c_str());
                        }
                        for (int m = 0; m < subgoals.size(); ++m){
                            for (int n = 0; n < subgoals[m].size(); ++n){
                            }
                        }
                        vector<string> temp_subgoal;
                        if (subgoals.size() > 0) {
                            temp_subgoal = subgoals[subgoals.size()-1];
                        }
                        for (int m=0; m<temp_subgoal.size(); ++m){
                        }
                        yInfo("converting");
                        for (int i=0; i<aux_subgoal.size(); ++i){
                            temp_subgoal.push_back(aux_subgoal[i]);
                        }
                        yInfo("temp_subgoal done");
                        temp_action.clear();
                        temp_action.push_back(found_action);
                        temp_action.push_back(obj);
                        temp_action.push_back(tool);
                        action_sequence.push_back(temp_action);
                        subgoals.push_back(temp_subgoal);
                    }
                    yInfo("action translated: %s", found_action.c_str());
                    vector<int> index_var;
                    int flag_detect;
                    // Detect changed symbols, delete old ones
                    for (int g = 0; g < aux_subgoal.size(); ++g){
                        flag_detect = 0;
                        if (aux_subgoal[g].find("-") == 0){
                            temp_str = aux_subgoal[g];
                            temp_str.replace(temp_str.find("-"),1,"");
                            for (int h = 0; h<subgoals[subgoals.size()-1].size(); ++h){
                                if (subgoals[subgoals.size()-1][h] == temp_str){
                                    index_var.push_back(h);
                                }
                                if (subgoals[subgoals.size()-1][h] == aux_subgoal[g] && flag_detect == 1){
                                    index_var.push_back(h);
                                }
                                if (subgoals[subgoals.size()-1][h] == aux_subgoal[g] && flag_detect == 0){
                                    flag_detect = 1;
                                }
                            }
                        }
                        else {
                            for (int h = 0; h<subgoals[subgoals.size()-1].size(); ++h){
                                temp_str = "-";
                                temp_str += aux_subgoal[g];
                                if (subgoals[subgoals.size()-1][h] == temp_str){
                                    index_var.push_back(h);
                                }
                                if (subgoals[subgoals.size()-1][h] == aux_subgoal[g] && flag_detect == 1){
                                    index_var.push_back(h);
                                }
                                if (subgoals[subgoals.size()-1][h] == aux_subgoal[g] && flag_detect == 0){
                                    flag_detect = 1;
                                }
                            }
                        }
                    }
                    for (int m = 0; m < index_var.size(); ++m){
                        yDebug("index_var[%d]: %d", m, index_var[m]);
                    }

                    vector<string> temp_goal;
                    // Iterate over all symbols of last sub-goal
                    for (int y = 0; y<subgoals[subgoals.size()-1].size(); ++y){
                        if (find(index_var.begin(), index_var.end(), y) == index_var.end()) {
                            temp_goal.push_back(subgoals[subgoals.size()-1][y]);
                            yDebug("symbol to add: %s", subgoals[subgoals.size()-1][y].c_str());
                        }
                    }
                    for (int m = 0; m < subgoals[subgoals.size()-1].size(); ++m){
                        yDebug("before, subgoal %d: %s", m, subgoals[subgoals.size()-1][m].c_str());
                    }
                    subgoals[subgoals.size()-1] = temp_goal;
                    for (int m = 0; m < subgoals[subgoals.size()-1].size(); ++m){
                        yDebug("after, subgoal %d: %s", m, subgoals[subgoals.size()-1][m].c_str());
                    }
                }
            }
        }
        else {
            obj = prax_action[2];
        }
    }
    yInfo("action sequence: ");
    for (int i = 0; i < action_sequence.size(); ++i)
    {
        yInfo("%s %s %s", action_sequence[i][0].c_str(), action_sequence[i][1].c_str(), action_sequence[i][2].c_str() );
    }
    return true;
}

bool goalCompiler::translate()
{
    if (subgoals.size() == 0)
    {
        yError("subgoals not compiled yet.");
        return false;
    }
    for (int j = 0; j < translat.size(); ++j){
        for (int h = 0; h< subgoals.size(); ++h){
            for (int l = 0; l <subgoals[h].size(); ++l){
                while (!isStopping()) {
                    if (subgoals[h][l].find(translat[j][1]) != std::string::npos){
                        subgoals[h][l].replace(subgoals[h][l].find(translat[j][1]),translat[j][1].size(),translat[j][0]);
                    }
                    else {
                        break;
                    }
                }
            }
        }
    }
    string temp_str="";
    for (int i = 0; i < subgoals.size(); ++i){
        for (int j=0; j < subgoals[i].size(); ++j){
            //debugging message
            temp_str = temp_str + " " + subgoals[i][j];
        }
        yDebug("subgoal list: %s", temp_str.c_str());
        temp_str = "";
    }
    return true;
}

bool goalCompiler::writeFiles()
{
    goalFile.open(goalFileName.c_str());
    subgoalFile.open(subgoalFileName.c_str());
    if (!goalFile.is_open())
    {
        yError("unable to open goal file.");
        return false;
    }
    if (!subgoalFile.is_open())
    {
        yError("unable to open subgoal file.");
        return false;
    }
    string temp_str="";
    for (int i = 0; i < subgoals.size(); ++i){
        for (int j=0; j < subgoals[i].size(); ++j){
            subgoalFile << subgoals[i][j] ;
            //debugging message
            //temp_str = temp_str + " " + subgoals[i][j];
            if ( j != subgoals[i].size()-1){
                subgoalFile << " ";
            }
        }
        //yDebug("subgoal list: %s", temp_str.c_str());
        //temp_str = "";
        subgoalFile << endl;
    }
    subgoalFile.close();
    if (subgoals.size() > 0)
    {
        for (int i = 0; i<subgoals[subgoals.size()-1].size(); ++i){
            goalFile << subgoals[subgoals.size()-1][i] << " ";
        }
    }
    else
    {
        yError("No goals were compiled: the goal list is empty");
        goalFile.close();
        subgoals.clear();
        return false;
    }
    goalFile.close();
    subgoals.clear();
    return true;
}

bool goalCompiler::plannerReply(string reply)
{
    if (plannerPort.getInputCount() == 0)
    {
        yError("planner not connected");
        return false;
    }
    Bottle &plannerBottleOut = plannerPort.prepare();
    plannerBottleOut.clear();
    plannerBottleOut.addString(reply);
    plannerPort.write();
    return true;
}

bool goalCompiler::checkConsistency()
{
    string requirements, negated_symbol, failed_action, temp_str;
    vector<string> required_state_vector, requirement_vector;
    yInfo("checking consistency");

    for (int i = 1; i < action_sequence.size(); ++i)
    {
        for (int j = 0; j < actions.size(); ++j)
        {
            if (actions[j] == action_sequence[i][0])
            {
                requirements = actions[j+2];
                while (!isStopping())
                {
                    if (requirements.find("_obj") != std::string::npos)
                    {
                        requirements.replace(requirements.find("_obj"),4,action_sequence[i][1]);
                    }
                    else if (requirements.find("_tool") != std::string::npos)
                    {
                        requirements.replace(requirements.find("_tool"),5,action_sequence[i][2]);
                    }
                    else if (requirements.find("_hand") != std::string::npos)
                    {
                        requirements.replace(requirements.find("_hand"),5,"left");
                    }
                    else {
                        break;
                    }
                }
                yDebug("requirements: %s", requirements.c_str());
                requirement_vector = split(requirements, ' ');
                requirements = "";
                for (int k = 0; k < requirement_vector.size(); ++k)
                {
                    if (requirement_vector[k].find("_ALL") != std::string::npos)
                    {
                        for (int y = 0; y < translat.size(); ++y)
                        {
                            if (requirement_vector[k].find(translat[y][1]) == std::string::npos)
                            {
                                temp_str = requirement_vector[k];
                                temp_str.replace(requirement_vector[k].find("_ALL"),4,translat[y][1]);
                                requirements = requirements + temp_str + " ";
                            }
                        }
                    }
                    else
                    {
                        requirements = requirements + requirement_vector[k] + " ";
                    }
                }
                yDebug("requirements: %s", requirements.c_str());
                for (int k = 0; k < translat.size(); ++k)
                {
                    while (!isStopping()) {
                        if (requirements.find(translat[k][1]) != std::string::npos){
                            requirements.replace(requirements.find(translat[k][1]),translat[k][1].size(),translat[k][0]);
                        }
                        else {
                            break;
                        }
                    }
                }
                yDebug("requirements: %s", requirements.c_str());
                required_state_vector = split(requirements, ' ');
                for (int k = 0; k < required_state_vector.size(); ++k)
                {
                    if (required_state_vector[k].find('-') != std::string::npos)
                    {
                        negated_symbol = required_state_vector[k];
                        negated_symbol.replace(negated_symbol.find('-'),1,"");
                        if (find_element(subgoals[i-1], negated_symbol) == 1)
                        {
                            yWarning("plan not executable");
                            failed_action = action_sequence[i][0];
                            if (failed_action.find("_obj") != std::string::npos)
                            {
                                failed_action.replace(failed_action.find("_obj"),4,action_sequence[i][1]);
                            }
                            if (failed_action.find("_tool") != std::string::npos)
                            {
                                failed_action.replace(failed_action.find("_tool"),5,action_sequence[i][2]);
                            }
                            if (failed_action.find("_hand") != std::string::npos)
                            {
                                failed_action.replace(failed_action.find("_hand"),5,"left");
                            }
                            yWarning("failed step: %s",    failed_action.c_str());
                            yDebug("failing symbol: %s", negated_symbol.c_str());
                            subgoals.clear();
                            return false;
                        }
                    }
                    else
                    {
                        negated_symbol = "-" + required_state_vector[k];
                        if (find_element(subgoals[i-1], negated_symbol) == 1)
                        {
                            yWarning( "plan not executable");
                            failed_action = action_sequence[i][0];
                            if (failed_action.find("_obj") != std::string::npos)
                            {
                                failed_action.replace(failed_action.find("_obj"),4,action_sequence[i][1]);
                            }
                            if (failed_action.find("_tool") != std::string::npos)
                            {
                                failed_action.replace(failed_action.find("_tool"),5,action_sequence[i][2]);
                            }
                            if (failed_action.find("_hand") != std::string::npos)
                            {
                                failed_action.replace(failed_action.find("_hand"),5,"left");
                            }
                            yWarning("failed step: %s", failed_action.c_str());
                            yDebug("failing symbol: %s", negated_symbol.c_str());
                            subgoals.clear();
                            return false;
                        }
                    }
                }
                break;
            }
        }
    }
    return true;
}

bool goalCompiler::clearUnimportantGoals()
{
    if (subgoals.size() == 0)
    {
        yError("subgoals not compiled yet.");
        return false;
    }
    vector< vector<string> > temp_subgoals;
    vector<string> temp_list;
    for (int h = 0; h< subgoals.size(); ++h){
        temp_list.clear();
        for (int l = 0; l <subgoals[h].size(); ++l){
            if (subgoals[h][l].find("11") == std::string::npos && subgoals[h][l].find("12") == std::string::npos){
                temp_list.push_back(subgoals[h][l]);
            }
        }
        if (temp_subgoals.size() != 0)
        {
            if (temp_list != temp_subgoals[temp_subgoals.size()-1])
            {
                temp_subgoals.push_back(temp_list);
            }
        }
        else if (temp_list.size() > 0)
        {
            temp_subgoals.push_back(temp_list);
        }
    }
    subgoals = temp_subgoals;
    return true;
}

/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2015 VisLab, Institute for Systems and Robotics,
 *                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Alexandre Antunes <aleksander88@gmail.com>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 */

#include "PlannerThread.h"
#include "Helpers.h"

PlannerThread::PlannerThread(const string &_moduleName, const double _period, const string &_PathName):moduleName(_moduleName),RateThread(int(_period*1000.0)),PathName(_PathName)
{
}

void PlannerThread::openFiles()
{
    rulesFileName = PathName + "/rules.dat";
    goalFileName = PathName+ "/goal.dat";
    configFileName = PathName + "/config";
    subgoalFileName = PathName + "/subgoals.dat";
    stateFileName = PathName + "/state.dat";
    pipeFileName = PathName + "/pipe.txt";
    symbolFileName = PathName + "/symbols.dat";
}

bool PlannerThread::openPorts()
{
    string portName;
    portName = "/" + moduleName + "/goal_cmd:io";
    goal_yarp.open(portName);

    portName = "/" + moduleName + "/ground_cmd:io";
    geo_yarp.open(portName);

    portName = "/" + moduleName + "/prax_inst:o";
    prax_yarp.open(portName);

    portName = "/" + moduleName + "/affordances_cmd:io";
    aff_yarp.open(portName);

    portName = "/" + moduleName + "/wsm_rpc:o";
    world_rpc.open(portName);

    portName = "/" + moduleName + "/actInt_rpc:o";
    actInt_rpc.open(portName);

    portName = "/" + moduleName + "/opc2prada_rpc:o";
    opc2prada_rpc.open(portName);

    return true;
}

void PlannerThread::threadRelease()
{
    cout << "thread terminated" << endl;
}

void PlannerThread::close()
{
    yInfo("Closing module");
    closing = true;
    goal_yarp.close();
    geo_yarp.close();
    prax_yarp.close();
    aff_yarp.close();
    world_rpc.close();
    actInt_rpc.close();
    opc2prada_rpc.close();
    yarp::os::Time::delay(0.1);
}

void PlannerThread::interrupt()
{
    yInfo("Interrupting ports");
    closing = true;
    goal_yarp.interrupt();
    geo_yarp.interrupt();
    prax_yarp.interrupt();
    aff_yarp.interrupt();
    world_rpc.interrupt();
    actInt_rpc.interrupt();
    opc2prada_rpc.interrupt();
}

bool PlannerThread::threadInit()
{
    closing = false;
    startPlan = false;
    resumePlan = true;
    plan_level = 0;
    // initialize file names
    openFiles();
    process_string = "cd && cd .. && cd .. && cd " + PathName + " && ./planner.exe > " + pipeFileName; // TODO: cd PathName
    if ( !openPorts() )
    {
        yError("problem opening ports");
        return false;
    }

    return true;
}

void PlannerThread::run()
{
    while (!closing)
    {
        while (!startPlan)
        {
            yarp::os::Time::delay(0.05);
            if (closing)
            {
                return;
            }
        }
        if (!plan_init())
        {
            startPlan = false;
        }
        restartPlan = false;
        while (!restartPlan)
        {
            yarp::os::Time::delay(0.1);
            if (closing)
            {
                return;
            }
            if (!planning_cycle())
            {
                startPlan = false;
                break;
            }
        }
        startPlan = false;
        yarp::os::Time::delay(0.05);
    }
    return;
}

bool PlannerThread::startPlanning()
{
    if (world_rpc.getOutputCount() == 0){
        yError("WorldStateManager not connected!");
        return false;
    }
    cmd.clear();
    cmd.addString("init");
    world_rpc.write(cmd, reply);
    yInfo("Initializing World State....");
    if ((reply.size() == 1) && (reply.get(0).asVocab() == yarp::os::Vocab::encode("ok")))
    {
        yInfo("World State initialized");
        startPlan = true;
    }
    else 
    {
        yError("Failed to initialize World State.");
        startPlan = false;
    }
    return startPlan;
}

bool PlannerThread::checkPause()
{
    if (!startPlan)
    {
        return false;
    }
    int timer = 0;
    while (!resumePlan)
    {
        yarp::os::Time::delay(0.1);
        timer = timer + 1;
        if (timer == 200)
        {
            yInfo("Planning cycle paused");
            timer = 0;
        }
    }
    if (!startPlan)
    {
        return false;
    }
    return true;
}

bool PlannerThread::pausePlanner()
{
    resumePlan = false;
    return resumePlan;
}

bool PlannerThread::resumePlanner()
{
    resumePlan = true;
    return resumePlan;
}

bool PlannerThread::updateState()
{
    if (opc2prada_rpc.getOutputCount() == 0){
        yError("opc2prada not connected!");
        return false;
    }
    if (world_rpc.getOutputCount() == 0){
        yError("WorldStateManager not connected!");
        return false;
    }
    cmd.clear();
    cmd.addString("update");
    world_rpc.write(cmd, reply);
    if ((reply.size() == 1) && (reply.get(0).asVocab() == yarp::os::Vocab::encode("ok"))){
        yarp::os::Time::delay(0.5);
        cmd.clear();
        cmd.addString("update");
        opc2prada_rpc.write(cmd,reply);
        if ((reply.size() == 1) && (reply.get(0).asVocab() == yarp::os::Vocab::encode("ok"))){
            yInfo("Planner state updated!");
            return true;
        }
        else {
            yError("Planner state update failed: something wrong with the opc2prada module");
            return false;
        }
    }
    else {
        yError("Planner state update failed: something wrong with the World State Manager");
    }
    return false;
}

bool PlannerThread::completePlannerState()
{
    string line, state_str, temp_str;
    vector<string> data, temp_vect, temp_vect2, avail_symb, temp_state;
    vector<vector<string> >  symbols;
    symbolFile.open(symbolFileName.c_str());
    stateFile.open(stateFileName.c_str());
    if (!stateFile.is_open())
    {
        yError("unable to open state file!");
        return false;
    }
    if (!symbolFile.is_open())
    {
        yError("unable to open symbols file!");
        return false;
    }
    while (getline(symbolFile, line)){
        data.push_back(line);
    }
    symbolFile.close();
    for (int i = 0; i < data.size(); ++i){
        temp_vect = split(data[i],' ');
        temp_vect2.clear();
        temp_vect2.push_back(temp_vect[0]);
        temp_vect2.push_back(temp_vect[2]);
        symbols.push_back(temp_vect2);
    }
    if (stateFile.is_open()){
        getline(stateFile, line);
        while (!closing){
            if (line.find('-') != std::string::npos){
                line.replace(line.find('-'),1,"");
            }
            else if (line.find('(') != std::string::npos){
                line.replace(line.find('('),1,"");
            }
            else if (line.find(')') != std::string::npos){
                line.replace(line.find(')'),1,"");
            }
            else {
                break;
            }
        }
        temp_state = split(line,' ');
    }
    stateFile.close();
    for (int i = 0; i < symbols.size(); ++i){
        temp_str = "-" + symbols[i][0];
        avail_symb.push_back(symbols[i][0]);
        avail_symb.push_back(temp_str);
    }
    data.clear();
    for (int i = 0; i < temp_state.size(); ++i){
        if (find_element(avail_symb, temp_state[i]) == 1){
            data.push_back(temp_state[i]);
        }
    }
    for (int i = 0; i < data.size(); ++i){
        temp_str = data[i]+"()";
        if (i == 0){
            state_str = temp_str;
        }
        else {
            state_str = state_str + " " + temp_str;
        }
    }
    state_str = state_str + " ";
    for (int i = 0; i < symbols.size(); ++i){
        if ((find_element(data, symbols[i][0]) == 0) && (symbols[i][1] == "primitive")){
            state_str = state_str + "-" + symbols[i][0] + "() ";
        }
    }
    newstateFile.open(stateFileName.c_str());
    if (!newstateFile.is_open())
    {
        yError("unable to open state file!");
        return false;
    }
    state_str = state_str + '\n';
    newstateFile << state_str;
    newstateFile.close();
    return true;
}

       
bool PlannerThread::groundRules()
{
    string command, data;
    if (geo_yarp.getOutputCount() == 0) {
        yError("geometric grounding module not connected, unable to ground rules!");
        return false;
    }
    if (aff_yarp.getOutputCount() == 0) {
        yError("Affordances communication module not connected, unable to ground rules!");
        return false;
    }
    Bottle& aff_bottle_out = aff_yarp.prepare();
    aff_bottle_out.clear();
    aff_bottle_out.addString("start");
    aff_yarp.write();
    Time::delay(0.3);
    aff_bottle_out = aff_yarp.prepare();
    aff_bottle_out.clear();
    aff_bottle_out.addString("update");
    aff_yarp.write();
    while (!closing) {
        aff_bottle_in = aff_yarp.read(false);
        if (aff_bottle_in){
            break;
        }
        if (aff_yarp.getOutputCount() == 0)
        {
            yError("Affordances communication module crashed");
            return false;
        }
        Time::delay(0.1);
    }
    Bottle& geo_bottle_out = geo_yarp.prepare();
    geo_bottle_out.clear();
    geo_bottle_out.addString("update");
    geo_yarp.write();
    yInfo("Grounding...");
    while (!closing) {
        geo_bottle_in = geo_yarp.read(false);
        if (geo_bottle_in != NULL){
            command = geo_bottle_in->toString();
            cout << command << endl;
        }
        if (command == "ready"){
            yInfo("Grounding Complete!");
            break;
        }
        if (geo_yarp.getOutputCount() == 0)
        {
            yError("Geometric Grounding module crashed");
            return false;
        }
        if (command == "fail")
        {
            yError("Grounding failed, there might be something wrong with the object list.");
            return false;
        }
        Time::delay(0.1);
    }
    aff_bottle_out = aff_yarp.prepare();
    aff_bottle_out.clear();
    aff_bottle_out.addString("query");
    aff_yarp.write();
    while (!closing) {
        yarp::os::Time::delay(0.1);
        aff_bottle_in = aff_yarp.read(false);
        if (aff_bottle_in){
            data = aff_bottle_in->toString();
            if (data == "()" || data == "")
            {
                yError("empty bottle received, something might be wrong with the affordances module.");
                return false;
            }
            //yDebug(data);
            while (!closing){
                if (data.find('"') != std::string::npos){
                    data.replace(data.find('"'),1,"");
                }
                else {
                    break;
                }
            }
            toolhandle = split(data,' ');
            break;
        }
        if (aff_yarp.getOutputCount() == 0)
        {
            yError("Affordance communication module crashed");
            return false;
        }
        Time::delay(0.1);
    }
    return true;
}

bool PlannerThread::compileGoal()
{
    if (goal_yarp.getOutputCount() == 0) {
        yError("Goal Compiler module not connected, unable to compile goals");
        return false;
    }
    Bottle& goal_bottle_out = goal_yarp.prepare();
    goal_bottle_out.clear();
    goal_bottle_out.addString("praxicon");
    goal_yarp.write();
    yInfo("Waiting for praxicon...");
    while (!closing) {
        yarp::os::Time::delay(0.1);
        goal_bottle_in = goal_yarp.read(false);
        if (goal_bottle_in){
            if (goal_bottle_in->toString() == "done")
            {
                yInfo("Praxicon instruction received, compiling...");
                break;
            }
            else if (goal_bottle_in->toString() == "failed")
            {
                yError("Praxicon disconnected or crashed, compiling failed.");
                return false;
            }
            else if (goal_bottle_in->toString() == "unknown")
            {
                yWarning("Unknown object in Praxicon message, unable to compile.");
                return false;
            }
            else
            {
                yError("non-standard message received, something failed with the Goal Compiler module.");
                return false;
            }
        }
        if (goal_yarp.getOutputCount() == 0)
        {
            yError("Goal compiler module crashed");
            return false;
        }
        Time::delay(0.5);
    }
    goal_bottle_out = goal_yarp.prepare();
    goal_bottle_out.clear();
    goal_bottle_out.addString("update");
    goal_yarp.write();
    while (!closing) {
        yarp::os::Time::delay(0.1);
        goal_bottle_in = goal_yarp.read(false);
        if (goal_bottle_in)
        {
            if (goal_bottle_in->toString() == "done")
            {
                yInfo("Goal Compiling is complete!");
                break;
            }
            else if (goal_bottle_in->toString() == "fail")
            {
                yWarning("Something failed while loading the objects on the Goal Compiler module");
                return false;
            }
            else
            {
                yError("non-standard message received, something might be wrong with the Goal Compiler module.");
                return false;
            }
        }
        if (goal_yarp.getOutputCount() == 0)
        {
            yError("Goal compiler module crashed");
            return false;
        }
        Time::delay(0.5);
    }
    return true;
}

bool PlannerThread::loadSubgoals()
{
    string line;
    vector<string> aux_subgoals;
    subgoalFile.open(subgoalFileName.c_str());
    if (!subgoalFile.is_open())
    {
        yError("unable to open subgoal file!");
        return false;
    }
    subgoals.clear();
    while (getline(subgoalFile, line)){
        aux_subgoals = split(line, ' ');
        subgoals.push_back(aux_subgoals);
    }
    subgoalFile.close();
    return true;
}

bool PlannerThread::loadGoal()
{
    string line;
    goalFile.open(goalFileName.c_str());
    if (!goalFile.is_open())
    {
        yError("unable to open goal file!");
        return false;
    }
    goal.clear();
    while (getline(goalFile, line,' ')){
        goal.push_back(line);
    }
    goalFile.close();
    return true;
}

bool PlannerThread::resetConfig()
{
    string line;
    int horizon;
    vector<string> config_data;
    configFile.open(configFileName.c_str());
    if (!configFile.is_open())
    {
        yError("unable to open config file!");
        return false;
    }
    while (getline(configFile, line)){
        config_data.push_back(line);
    }
    for (int i=0; i<config_data.size(); ++i){
        if (config_data[i].find("[PRADA]") != std::string::npos){
            horizon = 5;
            config_data[i+2] = "PRADA_horizon " + static_cast<ostringstream*>( &(ostringstream() << horizon) )->str();
            break;
        }
    }
    configFile.close();
    configFileOut.open(configFileName.c_str());
    if (!configFileOut.is_open())
    {
        yError("unable to open PRADA config file!");
        return false;
    }
    for (int i = 0; i<config_data.size();++i){
        configFileOut << config_data[i];
        configFileOut << '\n';
    }
    configFileOut.close();
    yInfo("PRADA config file reset");
    return true;
}

Bottle PlannerThread::printObjs()
{
    return object_bottle;
}

bool PlannerThread::loadObjs()
{
    vector<string> temp_vect;
    vector<string> labels;

    // first confirm that worldStateManager has completed initialization phase
    // (tracker, short-term memory, WSOPC database)
    if (world_rpc.getOutputCount() == 0)
    {
        yError("worldStateManager not connected!");
        return false;
    }
    Bottle wsmCmd, wsmReply;
    wsmCmd.addString("isInitialized");
    world_rpc.write(wsmCmd, wsmReply);
    if (wsmReply.size() > 0 &&
        wsmReply.get(0).asVocab() != Vocab::encode("ok"))
    {
        yWarning("worldStateManager not initialized yet, waiting...");
        yarp::os::Time::delay(0.1);
        return false;
    }

    // then query opc2prada for list of items in database
    if (opc2prada_rpc.getOutputCount() == 0){
        yError("opc2prada not connected!");
        return false;
    }
    object_IDs.clear();
    cmd.clear();
    cmd.addString("loadObjects");
    opc2prada_rpc.write(cmd,reply);
    if (reply.size() > 0 && reply.get(0).isList() && reply.get(0).asList()->size() > 2){
        object_bottle.clear();
        object_bottle = *reply.get(0).asList();
        for (int i = 0; i < reply.get(0).asList()->size(); ++i)
        {
            temp_vect.clear();
            temp_vect.push_back( NumbertoString(reply.get(0).asList()->get(i).asList()->get(0).asInt() ) );
            temp_vect.push_back(reply.get(0).asList()->get(i).asList()->get(1).asString());
            object_IDs.push_back(temp_vect);
               if (find_element(labels,temp_vect[1]) == 1)
               {
                   yWarning("There are objects that share labels: %s", temp_vect[1].c_str());
               }
               labels.push_back(temp_vect[1]);
        }
        return true;
    }
    else {
        yError("Objects update failed!");
        return false;
    }
    return false;
}

void PlannerThread::stopPlanning()
{
    startPlan = false;
    restartPlan = true;
    plan_level = 0;
    resetConfig();
    yInfo("Planning stopped");
    return;
}

bool PlannerThread::resetPlanVars()
{
    plan_level = 0;
    objects_used.clear();
    next_action == "";
    return resetConfig();
}

bool PlannerThread::loadState()
{
    string line;
    stateFile.open(stateFileName.c_str());
    if (stateFile.is_open()){
        getline(stateFile, line);
        state = split(line,' ');
        state.pop_back();
    }
    else {
        yError("unable to open state file!");
        return false;
    }
    stateFile.close();
    return true;
}

bool PlannerThread::preserveState()
{
    old_state = state;
    return true;
}

bool PlannerThread::compareState()
{
    vector<string> temp_vect;
    if (next_action != "")
    {
        for (int i = 0; i < rules.size(); ++i)
        {
            if (rules[i].find(next_action) != std::string::npos)
            {
                temp_vect = split(rules[i+4], ' ');
                temp_vect.erase(temp_vect.begin());
                temp_vect.erase(temp_vect.begin());
                temp_vect.erase(temp_vect.begin());
                //yDebug("State comparison: %s", temp_vect[0].c_str());
                break;
            }
        }
    }
    for (int i = 0; i < temp_vect.size(); ++i)
    {
        if (find_element(state, temp_vect[i]) == 0)
        {
            return true;
        }
    }
    if (vect_compare(state, old_state) == 1)
    {
        return true; // state did not change
    }
    else
    {
        return false; // state did change
    }
    return false;
}

bool PlannerThread::preserveRules()
{
    old_rules = rules;
    return true;
}

bool PlannerThread::loadRules()
{
    string line;
    rulesFile.open(rulesFileName.c_str());
    if (!rulesFile.is_open())
    {
        yError("unable to open rules file!");
        return false;
    }
    rules.clear();
    while (getline(rulesFile, line,'\n')){
        rules.push_back(line);
    }
    rulesFile.close();
    return true;
}

bool PlannerThread::adaptRules()
{
    string temp_str;
    vector<string> adapt_rules, adapt_noise;
    rulesFileOut.open(rulesFileName.c_str());
    if (!rulesFileOut.is_open())
    {
        yError("unable to open rules file!");
        return false;
    }
    for (int t = 0; t < rules.size(); ++t){
        temp_str = rules[t];
        while (!closing){
            if (temp_str.find(' ') != std::string::npos){
                temp_str.replace(temp_str.find(' '),1,"");
            }
            else if (temp_str.find('\n') != std::string::npos){
                temp_str.replace(temp_str.find('\n'),1,"");
            }
            else if (temp_str.find('\r') != std::string::npos){
                temp_str.replace(temp_str.find('\r'),1,"");
            }
            else {
                break;
            }
        }
        if (temp_str == next_action && next_action != ""){
            int p = 0;
            while (!closing){
                if (rules[t+p] == ""){
                    adapt_rules = split(rules[t+4], ' ');
                    adapt_rules[2] = static_cast<ostringstream*>( &(ostringstream() << (atof(adapt_rules[2].c_str())/5) ))->str();
                    temp_str = "";
                    for (int h = 0; h < adapt_rules.size(); ++h){
                        if (adapt_rules[h] != "0")
                        {
                            temp_str = temp_str + adapt_rules[h] + " ";
                        }
                    }
                    rules[t+4] = temp_str;
                    adapt_noise = split(rules[t+p-1], ' ');
                    adapt_noise[2] = static_cast<ostringstream*>( &(ostringstream() << (atof(adapt_noise[2].c_str()) + 4*atof(adapt_rules[2].c_str())) ))->str();
                    temp_str = "";
                    for (int h = 0; h < adapt_noise.size(); ++h){
                        if (adapt_noise[h] != "0")
                        {
                            temp_str = temp_str + adapt_noise[h] + " ";
                        }
                    }
                    rules[t+p-1] = temp_str;
                    break;
                }
                p = p +1;
            }
            for (int y = 0; y < rules.size(); ++y){
                rulesFileOut << rules[y] << endl;
            }
            yInfo("Rules adapted, probability of %s reduced", next_action.c_str());
            break;
        }
    }
    next_action = "";
    rulesFileOut.close();
    return true;
}

bool PlannerThread::goalUpdate()
{
    goalFileOut.open(goalFileName.c_str());
    if (!goalFileOut.is_open())
    {
        yError("unable to open goal file!");
        return false;
    }
    yInfo("Goal updated");
    if (subgoals.size() == 0)
    {
        yError("Goals not compiled correctly, recompile goals");
        return false;
    }
    if (plan_level >= subgoals.size())
    {
        plan_level = subgoals.size()-1;
    }
    for (int y = 0; y < subgoals[plan_level].size(); ++y){
        goalFileOut << subgoals[plan_level][y] << " ";
    }
    goalFileOut.close();
    return true;
}

bool PlannerThread::planCompletion()
{
    if (plan_level >= subgoals.size()){
        yInfo("Plan completed!!");
        Bottle& prax_bottle_out = prax_yarp.prepare();
        prax_bottle_out.clear();
        prax_bottle_out.addString("OK");
        for (int u = 0; u < objects_used.size(); ++u){
            for (int inde = 0; inde < object_IDs.size(); ++inde){
                if (object_IDs[inde][0] == objects_used[u]){
                    string check_str;
                    transform(object_IDs[inde][1].begin(), object_IDs[inde][1].end(), check_str.begin(), ::tolower);
                    if (check_str != "rake" && check_str != "stick" && check_str != "left" && check_str != "right"){
                        prax_bottle_out.addString(object_IDs[inde][1]);
                    }
                }
            }
        }
        yDebug("string sent to activityInterface prada:i port: %s", prax_bottle_out.toString().c_str());
        prax_yarp.write();
        restartPlan = true;
        return false;
    }
    return true;
}

int PlannerThread::PRADA()
{
    string line;
    vector<string> pipe_vect;
    string next_sequence;
    int retrn_flag = 2;
    FILE * pFile;
    pFile = fopen(pipeFileName.c_str(),"w");
    fclose(pFile);
    int sys_flag = system(process_string.c_str());
    if (sys_flag == 34304)
    {
        yError("Error with PRADA files, load failed");
        return 0;
    }
    pipeFile.open(pipeFileName.c_str());
    if (!pipeFile.is_open())
    {
        yError("unable to communicate with PRADA, pipe file not available.");
        return 0;
    }
    while (getline(pipeFile, line)){
        pipe_vect.push_back(line);
    }
    pipeFile.close();
    for (int t = 0; t < pipe_vect.size(); ++t){
        if (pipe_vect[t] == "The planner would like to kindly recommend the following action to you:" && t+1 < pipe_vect.size()){
            next_action = pipe_vect[t+1];
            cout << endl;
            yInfo("Action found: %s", next_action.c_str());
            retrn_flag = 1;
        }
        if (pipe_vect[t] == "*** Planning for a complete plan." && t+3 < pipe_vect.size()){
            next_sequence = pipe_vect[t+3];
            cout << endl;
            yDebug("Sequence found: %s", next_sequence.c_str());
            return retrn_flag;
        }
    }
    return retrn_flag;
}

bool PlannerThread::increaseHorizon()
{
    int horizon;
    string line;
    vector<string> configData, temp_vect, objects_failed;
    configFile.open(configFileName.c_str());
    if (!configFile.is_open())
    {
        yError("unable to open PRADA config file!");
        return false;
    }
    while (getline(configFile, line)){
        configData.push_back(line);
    }
    configFile.close();
    for (int w = 0; w < configData.size(); ++w){ 
        if (configData[w].find("[PRADA]") != std::string::npos){
            temp_vect = split(configData[w+2], ' ');
            horizon = atoi(temp_vect[1].c_str());
            horizon = horizon + 1;
            if (horizon > 10)
            {
                yWarning("horizon too large, jumping to next goal");
                jumpForward();
                horizon = 5;
                if (failed_goal.size() == 0)
                {
                    for (int t = 0; t < goal.size(); ++t)
                    {
                        if (find_element(state,goal[t]) == 0)
                        {
                            failed_goal.push_back(goal[t]);
                        }
                    }
                }
                if ((plan_level >= subgoals.size() && !checkGoalCompletion()) || !checkHoldingSymbols())
                {
                    for (int t = 0; t < failed_goal.size(); ++t)
                    {
                        temp_vect = split(failed_goal[t], '_');
                        objects_failed.push_back(temp_vect[0]);
                    }
                    yInfo("Plan failed");
                    Bottle& prax_bottle_out = prax_yarp.prepare();
                    prax_bottle_out.clear();
                    prax_bottle_out.addString("FAIL");
                    for (int u = 0; u < objects_failed.size(); ++u){
                        for (int inde = 0; inde < object_IDs.size(); ++inde){
                            if (object_IDs[inde][0] == objects_failed[u]){
                                string check_str;
                                transform(object_IDs[inde][1].begin(), object_IDs[inde][1].end(), check_str.begin(), ::tolower);
                                if (check_str != "rake" && check_str != "stick" && check_str != "left" && check_str != "right"){
                                    prax_bottle_out.addString(object_IDs[inde][1]);
                                }
                            }
                        }
                    }
                    yDebug("Sending to Praxicon: %s", prax_bottle_out.toString().c_str());
                    prax_yarp.write(); 
                    restartPlan = true;
                    return false;
                }
            }
            configData[w+2] = "PRADA_horizon " + static_cast<ostringstream*>( &(ostringstream() << horizon) )->str();
            break;
        }
    }
    configFileOut.open(configFileName.c_str());
    if (!configFileOut.is_open())
    {
        yError("unable to open PRADA config file!");
        return false;
    }
    for (int w = 0; w < configData.size(); ++w){
        configFileOut << configData[w] << endl;
    }
    configFileOut.close();
    return true;
}

bool PlannerThread::checkHoldingSymbols()
{
    vector<string> holding_symbols;
    if (plan_level >= 1){
        for (int t = 0; t < subgoals[plan_level-1].size(); ++t){
            if (find_element(subgoals[plan_level],subgoals[plan_level-1][t]) == 1){
                holding_symbols.push_back(subgoals[plan_level-1][t]);
            }
        }
        if (holding_symbols.size() != 0){
            holding_symbols.erase(holding_symbols.end());
        }
        for (int t = 0; t < holding_symbols.size(); ++t){
            if (find_element(state, holding_symbols[t])== 0){
                yInfo("Situation changed, receding in plan");
                return false;
            }
        }
    }
    return true;
}

bool PlannerThread::jumpForward()
{
    plan_level = plan_level + 1;
    return true;
}

bool PlannerThread::jumpBack()
{
    plan_level = plan_level - 1;
    return true;
}

bool PlannerThread::resetRules()
{
    rulesFileOut.open(rulesFileName.c_str());
    if (!rulesFileOut.is_open())
    {
        yError("unable to open rules file!");
        return false;
    }
    for (int y = 0; y < old_rules.size(); ++y){
        rulesFileOut << old_rules[y] << endl;
    }
    rulesFileOut.close();
    return true;
}

bool PlannerThread::loadUsedObjs()
{
    vector<string> aux_used;
    for (int y = 0; y < object_IDs.size(); ++y){
        string check_str;
        transform(object_IDs[y][1].begin(), object_IDs[y][1].end(), check_str.begin(), ::tolower);
        if (next_action.find(object_IDs[y][0]) != std::string::npos && check_str != "stick" && check_str != "rake" && check_str != "left" && check_str != "right"){
            aux_used.push_back(object_IDs[y][0]);
        }
    }
    yDebug("Objects used in last action:");
    for (int u = 0; u < aux_used.size(); ++u){
        yDebug("%s", aux_used[u].c_str());
        if (find_element(objects_used, aux_used[u]) == 0){
            objects_used.push_back(aux_used[u]);
        }
    }
    yDebug("Objects used until now:");
    for (int u = 0; u < objects_used.size(); ++u)
    {
        yDebug("%s", objects_used[u].c_str());
    }
    return true;
}

bool PlannerThread::codeAction()
{
    vector<string> temp_vect;
    float temp_float;
    string tool1, tool2;
    temp_vect = split(next_action, '_');
    if (find_element(temp_vect, "on") == 1){
        obj = temp_vect[1];
        hand = temp_vect[3];
        while (!closing){
            if (hand.find("(") != std::string::npos){
                hand.replace(hand.find("("),1,"");
            }
            else if (hand.find(")") != std::string::npos){
                hand.replace(hand.find(")"),1,"");
            }
            else {
                break;
            }
        }
        act = temp_vect[0];
    }
    else {
        act = temp_vect[0];
        obj = temp_vect[1];
        hand = temp_vect[3];
        while (!closing){
            if (hand.find("(") != std::string::npos){
                hand.replace(hand.find("("),1,"");
            }
            else if (hand.find(")") != std::string::npos){
                hand.replace(hand.find(")"),1,"");
            }
            else {
                break;
            }
        }
        for (int ID = 0; ID < object_IDs.size();++ID){
            string check_str;
            transform(object_IDs[ID][1].begin(), object_IDs[ID][1].end(), check_str.begin(), ::tolower);
            if (check_str == "rake"){
                tool1 = object_IDs[ID][0];
            }
            if (check_str == "stick"){
                tool2 = object_IDs[ID][0];
            }
        }
        if (act == "grasp" && (obj == tool1 || obj == tool2)){
            for (int i = 0; i < toolhandle.size(); ++i){
                if (toolhandle[i] == obj){
                    temp_float = strtof(toolhandle[i+1].c_str(), NULL);
                    positx = (int) (temp_float);
                    temp_float = strtof(toolhandle[i+2].c_str(), NULL);
                    posity = (int) (temp_float);
                }
            }
        }
    }
    for (int k = 0; k < object_IDs.size(); ++k){
        if (act == object_IDs[k][0]){
            act = object_IDs[k][1];
        }
        if (obj == object_IDs[k][0]){
            obj = object_IDs[k][1];
        }
        if (hand == object_IDs[k][0]){
            hand = object_IDs[k][1];
            while (!closing){
                if (hand.find("hand") != std::string::npos){
                    hand.replace(hand.find("hand"),4,"");
                }
                else {
                    break;
                }
            }
        }
    }
    yDebug("%s %s %s", act.c_str(), obj.c_str(), hand.c_str());
    return true;
}

bool PlannerThread:: execAction()
{
    string temp_str;
    message.clear();
    string check_strobj;
    transform(obj.begin(), obj.end(), check_strobj.begin(), ::tolower);
    if (act == "grasp" && (check_strobj == "rake" || check_strobj == "stick")){
        act = "askForTool";
        message.addString(act);
        message.addString(hand);
        message.addInt(positx);
        message.addInt(posity);
    }
    else if (act == "grasp" && (check_strobj != "rake" && check_strobj != "stick")){
        act = "take";
        message.addString(act);
        message.addString(obj);
        message.addString(hand);
    }
    else {
        message.addString(act);
        message.addString(obj);
        message.addString(hand);
    }
    yInfo("Request execution of action: %s" , message.toString().c_str());
    while (!closing){
        yarp::os::Time::delay(0.1);
        actInt_rpc.write(message, reply);
        yInfo("Received reply: %s", reply.toString().c_str());
        if (reply.size() == 1 && reply.get(0).asVocab() == 27503){
            prev_action = message.get(1).toString();
            return true;
        }
        if (reply.size() == 1 && reply.get(0).asVocab() != 27503){
            yWarning("Nack received, there might be something wrong with the message.");
            yWarning(" %s",message.toString().c_str());
            return false;
        }
        if (reply.size() != 1){
            yError("activityInterface is not connected, verify if the module is running, and all connections are established.");
            return false;
        }
    }
    return false;
}

bool PlannerThread::checkGoalCompletion()
{
    for (int t = 0; t < goal.size(); ++t){
        if (find_element(state, goal[t]) == 0){
            return false;
        }
    }
    return true;
}

bool PlannerThread::checkFailure()
{
    string line;
    int horizon;
    vector<string> config_data;
    configFile.open(configFileName.c_str());
    if (!configFile.is_open())
    {
        yError("unable to open config file!");
        return false;
    }
    while (getline(configFile, line)){
        config_data.push_back(line);
    }
    for (int i=0; i<config_data.size(); ++i){
        if (config_data[i].find("[PRADA]") != std::string::npos){
            horizon = 5;
            break;
        }
    }
    configFile.close();
    vector<string> not_comp_goals, fail_obj, aux_fail_obj;
    string temp_str;
    if (horizon > 10){
        not_comp_goals.clear();
        for (int t = 0; t < goal.size(); ++t){
            if (find_element(state, goal[t]) == 0){
                not_comp_goals.push_back(goal[t]);
            }
        }
        temp_str = "";
        for (int i = 0; i < not_comp_goals.size(); ++i){
            temp_str = temp_str + not_comp_goals[i] + " ";
        }
        for (int i = 0; i < object_IDs.size(); ++i){
            if (temp_str.find(object_IDs[i][0]) != std::string::npos){
                fail_obj.push_back(object_IDs[i][0]);
            }
        }
        for (int u = 0; u < fail_obj.size(); ++u){
            if (fail_obj[u] != "11" && fail_obj[u] != "12" && find_element(toolhandle,fail_obj[u]) == 0){
                for (int t = 0; t < object_IDs.size(); ++t){
                    if (find_element(object_IDs[t], fail_obj[u]) == 1){
                        aux_fail_obj.push_back(object_IDs[t][1]);
                        break;
                    }
                }
            }
        }
        Bottle& prax_bottle_out = prax_yarp.prepare();
        prax_bottle_out.clear();
        prax_bottle_out.addString("FAIL");
        for (int u = 0; u < aux_fail_obj.size(); ++u){
            for (int inde = 0; inde < object_IDs.size(); ++inde){
                if (object_IDs[inde][0] == aux_fail_obj[u]){
                    string check_str;
                    transform(object_IDs[inde][1].begin(), object_IDs[inde][1].end(), check_str.begin(), ::tolower);
                    if (check_str != "rake" && check_str != "stick" && check_str != "left" && check_str != "right"){
                        prax_bottle_out.addString(object_IDs[inde][1]);
                    }
                }
            }
        }
        yInfo("Sending to Praxicon: %s", prax_bottle_out.toString().c_str());
        prax_yarp.write();
        restartPlan = true;
        return true;
    }
    return true;
}

bool PlannerThread::plan_init()
{
    if (!checkPause())
    {
        return false;
    }
    if (!resetPlanVars())
    {
        return false;
    }
    if (!checkPause())
    {
        return false;
    }
    if (!updateState())
    {
        return false;
    }
    if (!checkPause())
    {
        return false;
    }
    if (!loadObjs())
    {
        return false;
    }
    if (!loadState())
    {
        return false;
    }
    if (!checkPause())
    {
        return false;
    }
    if (!groundRules())
    {
        return false;
    }
    if (!checkPause())
    {
        return false;
    }
    if (!completePlannerState())
    {
        return false;
    }
    if (!checkPause())
    {
        return false;
    }
    if (!preserveState())
    {
        return false;
    }
    if (!checkPause())
    {
        return false;
    }
    if (!compileGoal())
    {
        return false;
    }
    if (!checkPause())
    {
        return false;
    }
    if (!loadSubgoals())
    {
        return false;
    }
    if (!checkPause())
    {
        return false;
    } 
    if (!loadRules())
    {
        return false;
    }
    if (!checkPause())
    {
        return false;
    }
    if (!preserveRules())
    {
        return false;
    }
    if (!checkPause())
    {
        return false;
    }
    if (!goalUpdate())
    {
        return false;
    }
    if (!checkPause())
    {
        return false;
    }
    if (!loadGoal())
    {
        return false;
    }
    if (!checkPause())
    {
        return false;
    }
    return true;
}

bool PlannerThread::planning_cycle()
{
    if (!checkPause())
    {
        return false;
    }
    if (!updateState())
    {
        return false;
    }
    if (!checkPause())
    {
        return false;
    }
    if (!completePlannerState())
    {
        return false;
    }
    if (!checkPause())
    {
        return false;
    }
    if (!loadState())
    {
        return false;
    }
    if (!checkPause())
    {
        return false;
    }
    if (compareState())
    {
        if (!checkPause())
        {
            return false;
        }
        if (!adaptRules())
        {
            return false;
        }
        if (!checkPause())
        {
            return false;
        }
    }
    if (!checkPause())
    {
        return false;
    }
    if (!goalUpdate())
    {
        return false;
    }
    if (!checkPause())
    {
        return false;
    }
    if (!loadGoal())
    {
        return false;
    }
    if (!checkPause())
    {
        return false;
    }
    if (!checkHoldingSymbols())
    {
        if (!checkPause())
        {
            return false;
        }
        jumpBack();
        if (!checkPause())
        {
            return false;
        }
        if (!resetConfig())
        {
            return false;
        }
        if (!checkPause())
        {
            return false;
        }
    }
    else 
    {
        if (!checkPause())
        {
            return false;
        }
        if (checkGoalCompletion())
        {
            if (!checkPause())
            {
                return false;
            }
            if (!resetRules())
            {
                return false;
            }
            if (!checkPause())
            {
                return false;
            }
            if (!resetConfig())
            {
                return false;
            }
            if (!checkPause())
            {
                return false;
            }
            if (!loadRules())
            {
                return false;
            }
            if (!checkPause())
            {
                return false;
            }
            if (!jumpForward())
            {
                return false;
            }
            if (!checkPause())
            {
                return false;
            }
            if (!planCompletion())
            {
                return true;
            }
            if (!checkPause())
            {
                return false;
            }
            return true;
        }
        else {
            if (!checkPause())
            {
                return false;
            }
            string tmp_str = showCurrentGoal(); 
            yInfo("Current subgoal: %s", tmp_str.c_str());
            int flag_prada = PRADA();
            if (!checkPause())
            {
                return false;
            }
            if (flag_prada == 0)
            {
                return false;
            }
            else if (flag_prada == 2)
            {
                if (!increaseHorizon())
                {
                    return false;
                }
                if (!checkPause())
                {
                    return false;
                }
                return true;
            }
            if (!loadUsedObjs())
            {
                return false;
            }
            if (!checkPause())
            {
                return false;
            }
            if (!codeAction())
            {
                return false;
            }
            if (!checkPause())
            {
                return false;
            }
            if (!execAction())
            {
                return false;
            }
            if (!checkPause())
            {
                return false;
            }
            if (!preserveState())
            {
                return false;
            }
            if (!checkPause())
            {
                return false;
            }
            if (!checkFailure())
            {
                return false;
            }
            if (!checkPause())
            {
                return false;
            }
        }
    }
    if (!checkPause())
    {
        return false;
    }
    return true;
}

string PlannerThread::showPlannedAction()
{
    return next_action;
}

string PlannerThread::showCurrentState()
{
    string temp_str = "";
    for (int i = 0; i < state.size(); ++i)
    {
        temp_str = temp_str + state[i] + " ";
    }
    return temp_str; 
}

string PlannerThread::showCurrentGoal()
{
    if (!loadGoal())
    {
        return "failed";
    }
    string temp_str = "";
    for (int i = 0; i < goal.size(); ++i)
    {
        temp_str = temp_str + goal[i] + " ";
    }
    return temp_str; 
}

string PlannerThread::printSymbol(string symbol)
{
    loadObjs();
    string reply_string = "";
    vector<string> temp_vect, temp_vect2;
    vector<vector<string> > answer;

    if (symbol == "reachable")
    {
        for (int i = 0; i < object_IDs.size(); ++i)
        {
            temp_vect.clear();
            temp_vect.push_back(object_IDs[i][1]);
            for (int j = 0; j < state.size(); ++j)
            {
                if (state[j].find("reachable") != std::string::npos && state[j].find('-') == std::string::npos && state[j].find("with_"+ object_IDs[i][0]) != std::string::npos)
                {
                    temp_vect2 = split(state[j], '_');
                    temp_vect.push_back(temp_vect2[0]);
                }
            }
            answer.push_back(temp_vect);
        }
        reply_string = "reachable with: \n";
        for (int i = 0; i < answer.size(); ++i)
        {
            reply_string = reply_string + answer[i][0] + ":";
            for (int j = 1; j < answer[i].size(); ++j)
            {
                reply_string = reply_string + " " + answer[i][j];
            }
            reply_string = reply_string + "\n";
        }
        return reply_string;
    }

    else if (symbol == "pullable")
    {
        for (int i = 0; i < object_IDs.size(); ++i)
        {
            temp_vect.clear();
            temp_vect.push_back(object_IDs[i][1]);
            for (int j = 0; j < state.size(); ++j)
            {
                if (state[j].find("pullable") != std::string::npos && state[j].find('-') == std::string::npos && state[j].find("with_"+ object_IDs[i][0]) != std::string::npos)
                {
                    temp_vect2 = split(state[j], '_');
                    temp_vect.push_back(temp_vect2[0]);
                }
            }
            answer.push_back(temp_vect);
        }
        reply_string = "pullable with: \n";
        for (int i = 0; i < answer.size(); ++i)
        {
            reply_string = reply_string + answer[i][0] + ":";
            for (int j = 1; j < answer[i].size(); ++j)
            {
                reply_string = reply_string + " " + answer[i][j];
            }
            reply_string = reply_string + "\n";
        }
        return reply_string;
    }

    else if (symbol == "inhand")
    {
        for (int i = 0; i < object_IDs.size(); ++i)
        {
            temp_vect.clear();
            temp_vect.push_back(object_IDs[i][1]);
            for (int j = 0; j < state.size(); ++j)
            {
                if (state[j].find('-') == std::string::npos && state[j].find("inhand_"+ object_IDs[i][0]) != std::string::npos)
                {
                    temp_vect2 = split(state[j], '_');
                    temp_vect.push_back(temp_vect2[0]);
                }
            }
            answer.push_back(temp_vect);
        }
        reply_string = "inhand of: \n";
        for (int i = 0; i < answer.size(); ++i)
        {
            reply_string = reply_string + answer[i][0] + ":";
            for (int j = 1; j < answer[i].size(); ++j)
            {
                reply_string = reply_string + " " + answer[i][j];
            }
            reply_string = reply_string + "\n";
        }
        return reply_string;
    }

    else if (symbol == "ontopof")
    {
        for (int i = 0; i < object_IDs.size(); ++i)
        {
            temp_vect.clear();
            temp_vect.push_back(object_IDs[i][1]);
            for (int j = 0; j < state.size(); ++j)
            {
                if (state[j].find('-') == std::string::npos && state[j].find("on_"+ object_IDs[i][0]) != std::string::npos)
                {
                    temp_vect2 = split(state[j], '_');
                    temp_vect.push_back(temp_vect2[0]);
                }
            }
            answer.push_back(temp_vect);
        }
        reply_string = "on top of: \n";
        for (int i = 0; i < answer.size(); ++i)
        {
            reply_string = reply_string + answer[i][0] + ":";
            for (int j = 1; j < answer[i].size(); ++j)
            {
                reply_string = reply_string + " " + answer[i][j];
            }
            reply_string = reply_string + "\n";
        }
        return reply_string;
    }
    
    else 
    {
        return "symbol not recognised, please insert one of the symbols on the list";
    }
    return "fail";
}

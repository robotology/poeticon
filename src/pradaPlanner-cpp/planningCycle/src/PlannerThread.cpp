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
    objFileName = PathName + "/Object_names-IDs.dat";
    pipeFileName = PathName + "/pipe.txt";
    symbolFileName = PathName + "/symbols.dat";
}

bool PlannerThread::openPorts()
{
    //BufferedPort<Bottle> goal_yarp;
    goal_yarp.open("/planner/goal_cmd:io");

    //BufferedPort<Bottle> geo_yarp;
    geo_yarp.open("/planner/grounding_cmd:io");

    //BufferedPort<Bottle> state_yarp;
    //state_yarp.open("/planner/opc_cmd:io");

    //BufferedPort<Bottle> prax_yarp;
    prax_yarp.open("/planner/prax_inst:o");

    //BufferedPort<Bottle> aff_yarp;
    aff_yarp.open("/planner/Aff_cmd:io");

    //RpcClient world_rpc;
    world_rpc.open("/planner/wsm_rpc:o");

    //RpcClient actInt_rpc;
    actInt_rpc.open("/planner/actInt_rpc:o");

    //RpcClient opc2prada_rpc;
    opc2prada_rpc.open("/planner/opc2prada_rpc:o");

    return true;
}

void PlannerThread::close()
{
    yInfo("closing ports");
    goal_yarp.close();
    geo_yarp.close();
    prax_yarp.close();
    aff_yarp.close();
    world_rpc.close();
    actInt_rpc.close();
    opc2prada_rpc.close();
}

void PlannerThread::interrupt()
{
    yInfo("interrupting ports");
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
    // initialize file names
    openFiles();
    process_string = "cd && cd .. && cd .. && cd " + PathName + " && ./planner.exe > " + pipeFileName;
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
        }
        if (!plan_init())
        {
            return;
        }
        restartPlan = false;
        while (!restartPlan)
        {
            if (!planning_cycle())
            {
                return;
            }
        }
        startPlan = false;
    }
    return;
}

bool PlannerThread::startPlanning()
{
    startPlan = true;
    return startPlan;
}

bool PlannerThread::checkPause()
{
    int timer = 0;
    while (!resumePlan)
    {
        yarp::os::Time::delay(0.1);
        timer = timer + 1;
        if (timer == 100)
        {
            yInfo("Planning cycle paused");
            timer = 0;
        }
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

//bool PlannerThread::initCommonVars();
//{
//    
//    return true;
//}

bool PlannerThread::updateState()
{
    if (opc2prada_rpc.getOutputCount() == 0){
        yWarning("opc2prada not connected!");
        return false;
    }
    if (world_rpc.getOutputCount() == 0){
        yWarning("WorldStateManager not connected!");
        return false;
    }
    cmd.clear();
    cmd.addString("update");
    world_rpc.write(cmd, reply);
    if ((reply.size() == 1) && (reply.get(0).asVocab() == yarp::os::Vocab::encode("ok"))){
        yarp::os::Time::delay(0.1);
        cmd.clear();
        cmd.addString("update");
        opc2prada_rpc.write(cmd,reply);
        if ((reply.size() == 1) && (reply.get(0).asVocab() == yarp::os::Vocab::encode("ok"))){
            Time::delay(0.3);
            yInfo("planner state updated!");
            return true;
        }
        else {
            yWarning("planner state update failed!");
            return false;
        }
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
        yWarning("unable to open state file!");
        return false;
    }
    if (!symbolFile.is_open())
    {
        yWarning("unable to open symbols file!");
        return false;
    }
    /*objFile.open(objFileName.c_str());
    if (!objFile.is_open())
    {
        yWarning("unable to open objects file!");
        return false;
    }*/
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
        while (true){
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
        yWarning("unable to open state file!");
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
        yWarning("geometric grounding module not connected, unable to ground rules!");
        return false;
    }
    if (aff_yarp.getOutputCount() == 0) {
        yWarning("Affordances communication module not connected, unable to ground rules!");
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
    while (true) {
        aff_bottle_in = aff_yarp.read(false);
        if (aff_bottle_in){
            break;
        }
        if (aff_yarp.getOutputCount() == 0)
        {
            yWarning("Affordances communication module crashed");
            return false;
        }
        Time::delay(0.1);
    }
    Bottle& geo_bottle_out = geo_yarp.prepare();
    geo_bottle_out.clear();
    geo_bottle_out.addString("update");
    geo_yarp.write();
    yInfo("grounding...");
    while (true) {
        geo_bottle_in = geo_yarp.read(false);
        if (geo_bottle_in != NULL){
            command = geo_bottle_in->toString();
        }
        if (command == "ready"){
            yInfo("Grounding Complete!");
            break;
        }
        if (geo_yarp.getOutputCount() == 0)
        {
            yWarning("Geometric Grounding module crashed");
            return false;
        }
        Time::delay(0.5);
    }
    aff_bottle_out = aff_yarp.prepare();
    aff_bottle_out.clear();
    aff_bottle_out.addString("query");
    aff_yarp.write();
    while (true) {
        aff_bottle_in = aff_yarp.read(false);
        if (aff_bottle_in){
            data = aff_bottle_in->toString();
            while (true){
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
            yWarning("Affordance communication module crashed");
            return false;
        }
        Time::delay(0.1);
    }
    return true;
}

bool PlannerThread::compileGoal()
{
    if (goal_yarp.getOutputCount() == 0) {
        yWarning("Goal Compiler module not connected, unable to compile goals");
        return false;
    }
    Bottle& goal_bottle_out = goal_yarp.prepare();
    goal_bottle_out.clear();
    goal_bottle_out.addString("praxicon");
    goal_yarp.write();
    yInfo("waiting for praxicon...");
    while (true) {
        goal_bottle_in = goal_yarp.read(false);
        if (goal_bottle_in){
            if (goal_bottle_in->toString() == "done")
            {
                yInfo("Praxicon instruction received, compiling...");
                break;
            }
            else if (goal_bottle_in->toString() == "failed")
            {
                yWarning("Praxicon disconnected or crashed, compiling failed.");
                return false;
            }
        }
        if (goal_yarp.getOutputCount() == 0)
        {
            yWarning("Goal compiler module crashed");
            return false;
        }
        Time::delay(0.5);
    }
    goal_bottle_out = goal_yarp.prepare();
    goal_bottle_out.clear();
    goal_bottle_out.addString("update");
    goal_yarp.write();
    while (true) {
        goal_bottle_in = goal_yarp.read(false);
        if (goal_bottle_in){
            yInfo("Goal Compiling is complete!");
            break;
        }
        if (goal_yarp.getOutputCount() == 0)
        {
            yWarning("Goal compiler module crashed");
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
        yWarning("unable to open subgoal file!");
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
        yWarning("unable to open goal file!");
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
        yWarning("unable to open config file!");
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
        yWarning("unable to open config file!");
        return false;
    }
    for (int i = 0; i<config_data.size();++i){
        configFileOut << config_data[i];
        configFileOut << '\n';
    }
    configFileOut.close();
    yInfo("config file reset");
    return true;
}

bool PlannerThread::loadObjs()
{
    string line;
    vector<string> aux_objs, temp_vect;
    objFile.open(objFileName.c_str());
    object_IDs.clear();
    if (objFile.is_open()){
        getline(objFile, line);
        aux_objs = split(line, ';');
        for (int j = 0; j < aux_objs.size(); ++j){
            temp_vect = split(aux_objs[j], ',');
            temp_vect[0].replace(temp_vect[0].find("("),1,"");
            temp_vect[1].replace(temp_vect[1].find(")"),1,"");
            object_IDs.push_back(temp_vect);
        }
        objFile.close();
    }
    else {
        yWarning("unable to open objects file!");
        return false;
    }
    return true;
}

bool PlannerThread::resetPlanVars()
{
    plan_level = 0;
    restartPlan = false;
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
        yWarning("unable to open state file!");
        return false;
    }
    stateFile.close();
}

bool PlannerThread::preserveState()
{
    old_state = state;
    return true;
}

bool PlannerThread::compareState()
{
    if (vect_compare(state, old_state) == 1)
    {
        return true; // state did not change
    }
    else
    {
        return false; // state did change
    }
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
        yWarning("unable to open rules file!");
        return false;
    }
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
        yWarning("unable to open rules file!");
        return false;
    }
    for (int t = 0; t < rules.size(); ++t){
        temp_str = rules[t];
        while (true){
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
            while (true){
                if (rules[t+p] == ""){
                    adapt_rules = split(rules[t+4], ' ');
                    adapt_rules[2] = static_cast<ostringstream*>( &(ostringstream() << (atof(adapt_rules[2].c_str())/2) ))->str();
                    temp_str = "";
                    for (int h = 0; h < adapt_rules.size(); ++h){
                         temp_str = temp_str + " " + adapt_rules[h];
                    }
                    rules[t+4] = temp_str;
                    adapt_noise = split(rules[t+p-1], ' ');
                    adapt_noise[2] = static_cast<ostringstream*>( &(ostringstream() << (atof(adapt_noise[2].c_str()) +atof(adapt_rules[2].c_str())) ))->str();
                    temp_str = "";
                    for (int h = 0; h < adapt_noise.size(); ++h){
                        temp_str = temp_str + " " + adapt_noise[h];
                    }
                    rules[t+p-1] = temp_str;
                    break;
                }
                p = p +1;
            }
            for (int y = 0; y < rules.size(); ++y){
                rulesFileOut << rules[y] << endl;
            }
            yInfo("rules adapted");
            break;
        }
    }
    rulesFileOut.close();
    return true;
}

bool PlannerThread::goalUpdate()
{
    goalFileOut.open(goalFileName.c_str());
    if (!goalFileOut.is_open())
    {
        yWarning("unable to open goal file!");
        return false;
    }
    for (int y = 0; y < subgoals[plan_level].size(); ++y){
        goalFileOut << subgoals[plan_level][y] << " ";
    }
    goalFileOut.close();
    return true;
}

bool PlannerThread::planCompletion()
{
    loadObjs();
    if (plan_level >= subgoals.size()-1){
        yInfo("plan completed!!");
        Bottle& prax_bottle_out = prax_yarp.prepare();
        prax_bottle_out.clear();
        prax_bottle_out.addString("OK");
        for (int u = 0; u < objects_used.size(); ++u){
            for (int inde = 0; inde < object_IDs.size(); ++inde){
                if (object_IDs[inde][0] == objects_used[u]){
                    if (object_IDs[inde][1] != "rake" && object_IDs[inde][1] != "stick" && object_IDs[inde][1] != "left" && object_IDs[inde][1] != "right"){
                        prax_bottle_out.addString(object_IDs[inde][1]);
                    }
                }
            }
        }
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
    /*remove(pipeFileName.c_str());*/
    FILE * pFile;
    pFile = fopen(pipeFileName.c_str(),"w");
    fclose(pFile);
    int sys_flag = system(process_string.c_str());
    if (sys_flag == 34304)
    {
        yWarning("Error with PRADA files, load failed");
        return 0;
    }
    pipeFile.open(pipeFileName.c_str());
    if (!pipeFile.is_open())
    {
        yWarning("unable to communicate with PRADA, pipe file not available.");
        return 0;
    }
    while (getline(pipeFile, line)){
        pipe_vect.push_back(line);
    }
    pipeFile.close();
    for (int t = 0; t < pipe_vect.size(); ++t){
        if (pipe_vect[t] == "The planner would like to kindly recommend the following action to you:" && t+1 < pipe_vect.size()){
            next_action = pipe_vect[t+1];
            yInfo("action found: %s", next_action.c_str());
            return 1;
        }
    }
    return 2;
}

bool PlannerThread::increaseHorizon()
{
    int horizon;
    string line;
    vector<string> configData, temp_vect;
    configFile.open(configFileName.c_str());
    if (!configFile.is_open())
    {
        yWarning("unable to open config file!");
        return false;
    }
    while (getline(configFile, line)){
        configData.push_back(line);
    }
    for (int w = 0; w < configData.size(); ++w){ 
        if (configData[w].find("[PRADA]") != std::string::npos){
            temp_vect = split(configData[w+2], ' ');
            horizon = atoi(temp_vect[1].c_str());
            horizon = horizon + 1;
            configData[w+2] = "PRADA_horizon " + static_cast<ostringstream*>( &(ostringstream() << horizon) )->str();
            break;
        }
    }
    configFile.close();
    configFileOut.open(configFileName.c_str());
    if (!configFileOut.is_open())
    {
        yWarning("unable to open config file!");
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
                yInfo("situation changed, receding in plan");
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
        yWarning("unable to open rules file!");
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
    loadObjs();
    vector<string> aux_used;
    objects_used.clear();
    for (int y = 0; y < object_IDs.size(); ++y){
        if (next_action.find(object_IDs[y][0]) != std::string::npos){
            aux_used.push_back(object_IDs[y][0]);
        }
    }
    for (int u = 0; u < aux_used.size(); ++u){
        if (find_element(toolhandle,aux_used[u]) == 0){
            objects_used.push_back(aux_used[u]);
        }
    }
    return true;
}

bool PlannerThread::codeAction()
{
    loadObjs();
    vector<string> temp_vect;
    float temp_float;
    string tool1, tool2;
    temp_vect = split(next_action, '_');
    if (find_element(temp_vect, "on") == 1){
        obj = temp_vect[1];
        hand = temp_vect[3];
        while (true){
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
        while (true){
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
            if (object_IDs[ID][1] == "rake"){
                tool1 = object_IDs[ID][0];
            }
            if (object_IDs[ID][1] == "stick"){
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
        yInfo("%s", object_IDs[k][0].c_str());
        if (act == object_IDs[k][0]){
            act = object_IDs[k][1];
        }
        if (obj == object_IDs[k][0]){
            obj = object_IDs[k][1];
        }
        if (hand == object_IDs[k][0]){
            hand = object_IDs[k][1];
            while (true){
                if (hand.find("hand") != std::string::npos){
                    hand.replace(hand.find("hand"),4,"");
                }
                else {
                    break;
                }
            }
        }
    }
    yInfo("%s %s %s", act.c_str(), obj.c_str(), hand.c_str());
    return true;
}

bool PlannerThread:: execAction()
{
    string temp_str;
    if (act == "grasp" && (obj == "rake" || obj == "stick")){
        act = "askForTool";
        //temp_str = act + " " + hand;
        //message.addString(temp_str);
        message.addString(act);
        message.addString(hand);
        message.addInt(positx);
        message.addInt(posity);
    }
    else if (act == "grasp" && (obj != "rake" && obj != "stick")){
        act = "take";
        //temp_str = act + " " + obj + " " + hand;
        //message.addString(temp_str);
        message.addString(act);
        message.addString(obj);
        message.addString(hand);
    }
    else {
        //temp_str = act + " " + obj + " " + hand;
        //message.addString(temp_str);
        message.addString(act);
        message.addString(obj);
        message.addString(hand);
    }
    yInfo("message: %s" , message.toString().c_str());
    while (true){
        actInt_rpc.write(message, reply);
        yInfo("%s", reply.toString().c_str());
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
            yWarning("activity interface is not connected, verify if the module is running, and all connections are established.");
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
    loadObjs();
    string line;
    int horizon;
    vector<string> config_data;
    configFile.open(configFileName.c_str());
    if (!configFile.is_open())
    {
        yWarning("unable to open config file!");
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
    if (horizon > 15){
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
                    if (object_IDs[inde][1] != "rake" && object_IDs[inde][1] != "stick" && object_IDs[inde][1] != "left" && object_IDs[inde][1] != "right"){
                        prax_bottle_out.addString(object_IDs[inde][1]);
                    }
                }
            }
        }
        yInfo(" %s", prax_bottle_out.toString().c_str());
        prax_yarp.write();
        restartPlan = true;
        return true;
    }
    return true;
}

bool PlannerThread::plan_init()
{
    checkPause();
    if (!resetPlanVars())
    {
        return false;
    }
    checkPause();
    if (!updateState())
    {
        return false;
    }
    checkPause();
    if (!loadState())
    {
        return false;
    }
    checkPause();
    if (!groundRules())
    {
        return false;
    }
    checkPause();
    if (!completePlannerState())
    {
        return false;
    }
    checkPause();
    if (!preserveState())
    {
        return false;
    }
    checkPause();
    if (!compileGoal())
    {
        return false;
    }
    checkPause(); 
    if (!loadRules())
    {
        return false;
    }
    checkPause();
    if (!preserveRules())
    {
        return false;
    }
    checkPause();
    if (!goalUpdate())
    {
        return false;
    }
    checkPause();
    if (!loadGoal())
    {
        return false;
    }
    checkPause();
    return true;
}

bool PlannerThread::planning_cycle()
{
    checkPause();
    if (!updateState())
    {
        return false;
    }
    checkPause();
    if (!completePlannerState())
    {
        return false;
    }
    checkPause();
    if (!loadState())
    {
        return false;
    }
    checkPause();
    if (compareState())
    {
        checkPause();
        if (!adaptRules())
        {
            return false;
        }
        checkPause();
    }
    checkPause();
    if (!goalUpdate())
    {
        return false;
    }
    checkPause();
    if (!loadGoal())
    {
        return false;
    }
    checkPause();
    if (!planCompletion())
    {
        return true;
    }
    checkPause();
    if (!checkHoldingSymbols())
    {
        checkPause();
        jumpBack();
        checkPause();
        if (!resetConfig())
        {
            return false;
        }
        checkPause();
    }
    else 
    {
        checkPause();
        if (checkGoalCompletion())
        {
            checkPause();
            if (!resetRules())
            {
                return false;
            }
            checkPause();
            if (!resetConfig())
            {
                return false;
            }
            checkPause();
            if (!loadRules())
            {
                return false;
            }
            checkPause();
            if (!jumpForward())
            {
                return false;
            }
            checkPause();
            return true;
        }
        checkPause();
        int flag_prada = PRADA();
        checkPause();
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
            checkPause();
            return true;
        }
        if (!loadUsedObjs())
        {
            return false;
        }
        checkPause();
        if (!codeAction())
        {
            return false;
        }
        checkPause();
        if (!execAction())
        {
            return false;
        }
        checkPause();
        if (!preserveState())
        {
            return false;
        }
        checkPause();
        if (!checkFailure())
        {
            return false;
        }
        checkPause();
    }
    checkPause();
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

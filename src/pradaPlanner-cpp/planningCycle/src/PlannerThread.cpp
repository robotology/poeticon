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
    //Port to issue commands to goalCompiler module
    portName = "/" + moduleName + "/goal_cmd:io";
    goal_yarp.open(portName);

    //Port to issue commands to geometricGrounding module
    portName = "/" + moduleName + "/ground_cmd:io";
    geo_yarp.open(portName);

    //Port to reply the plan status to the activityInterface/Praxicon: either FAIL or OK
    portName = "/" + moduleName + "/prax_inst:o";
    prax_yarp.open(portName);

    //Port to issue commands to affordanceCommunication module
    //Also receives the position for the tool-handles (after a query)
    portName = "/" + moduleName + "/affordances_cmd:io";
    aff_yarp.open(portName);

    //Port to issue commands to worldStateManager module (update)
    portName = "/" + moduleName + "/wsm_rpc:o";
    world_rpc.open(portName);

    //Port to issue commands to activityInterface module (issue actions, etc)
    portName = "/" + moduleName + "/actInt_rpc:o";
    actInt_rpc.open(portName);

    //Port to issue commands to OPC2PRADA module (writing state files, getting object IDs - Labels)
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
    closing = false; //If the module is closing, either by crt+C or "quit"
    startPlan = false; //When the module is issued the startPlanner command by rpc, this becomes true
    stopping = false; //When the module is issued the stopPlanner command by rpc, this becomes true
    resumePlan = true; //Variable responsible for pausing/resuming the plan, changed by pausePlanner/resumePlanner on rpc.
    plan_level = 0; //Variable that stores the current steps made by the planner


    // initialize file names
    openFiles();
    process_string = "cd && cd .. && cd .. && cd " + PathName + " && ./planner.exe > " + pipeFileName; // TODO: cd PathName, location of the PRADA executable
    if ( !openPorts() )
    {
        yError("problem opening ports");
        return false;
    }

    return true;
}

void PlannerThread::initValues(bool adaptability, bool goalConsistency, bool creativity)
{
    useAdaptability = adaptability;
    useGoalConsistency = goalConsistency;
    useCreativity = creativity;
}

void PlannerThread::run()
{
    while (!closing) //while running
    {
        while (!startPlan) //while plan doesn't start
        {
            yarp::os::Time::delay(0.05);
            if (closing)
            {
                return;
            }
        }
        if (!plan_init()) //If planner is not initialized, don't start planningCycle
        {
            startPlan = false;
        }
        restartPlan = false; //plan should not be restarted until success/failure
        while (!restartPlan) //While the planner doesn't need to restart:
        {
            yarp::os::Time::delay(0.1);
            if (closing)
            {
                return;
            }
            if (!planning_cycle()) //run planning cycle
            {
                startPlan = false; //If plan fails, wait for new startPlanner command
                break;
            }
        }
        startPlan = false; //wait for new startPlanner command
        yarp::os::Time::delay(0.05);
    }
    return;
}

bool PlannerThread::startPlanning()
{
    stopPlanning();
    if (world_rpc.getOutputCount() == 0){
        yError("WorldStateManager not connected!");
        return false;
    }

    // Asks the worldStateManager if it is already initialized
    cmd.clear();
    cmd.addString("isInitialized");
    yInfo("Checking World State....");
    world_rpc.write(cmd, reply);
    if ((reply.size() == 1) && (reply.get(0).asVocab() == yarp::os::Vocab::encode("ok")))
    {
        yInfo("World State already initialized");
        cmd.clear();
        cmd.addString("reset"); //if it was initialized, reset it
        yInfo("Resetting World State....");
        world_rpc.write(cmd, reply);
        if ((reply.size() == 1) && (reply.get(0).asVocab() == yarp::os::Vocab::encode("ok")))
        {
            yInfo("World State reset");
            startPlan = true; //When and if worldStateManager is initialized, we can start the plan
            stopping = false; //The plan is not stopped unless ordered
        }
        else 
        {
            yError("Failed to reset World State.");
            startPlan = false; //If the worldStateManager fails to reset, the plan should not continue
        }
    }
    else
    {
    // Initialize worldStateManager, wait until it is done
        cmd.clear();
        cmd.addString("init");
        yInfo("Initializing World State....");
        world_rpc.write(cmd, reply);
        if ((reply.size() == 1) && (reply.get(0).asVocab() == yarp::os::Vocab::encode("ok")))
        {
            yInfo("World State initialized");
            startPlan = true; //When and if worldStateManager is initialized, we can start the plan
            stopping = false; //The plan is not stopped unless ordered
        }
        else 
        {
            yError("Failed to initialize World State.");
            startPlan = false; //If the worldStateManager fails to initialize, the plan should not continue
        }
    }
    return startPlan;
}

//Function responsible for pausing/resuming planning
bool PlannerThread::checkPause()
{
    if (!startPlan) //If there is no plan running
    {
        return false; //Then it shouldn't pause
    }
    int timer = 0;
    while (!resumePlan && !stopping && !closing) //While the plan is not resumed, wait for command resumePlanner
    {
        yarp::os::Time::delay(0.1);
        timer = timer + 1;
        if (timer == 200)
        {
            yInfo("Planning cycle paused");
            timer = 0;
        }
    }
    if (!startPlan) //If plan was stopped while paused, pause should stop current planning
    {
        return false;
    }
    return true;
}

// command to pause Planner
bool PlannerThread::pausePlanner()
{
    resumePlan = false;
    return resumePlan;
}

// command to resume Planner
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
    // Update worldStateManager
    cmd.clear();
    cmd.addString("update");
    world_rpc.write(cmd, reply);
    if ((reply.size() == 1) && (reply.get(0).asVocab() == yarp::os::Vocab::encode("ok"))){
        yarp::os::Time::delay(0.5);
        // If the worldStateManager updates successfully, then update OPC2PRADA
        cmd.clear();
        cmd.addString("update");
        opc2prada_rpc.write(cmd,reply);
        if ((reply.size() == 1) && (reply.get(0).asVocab() == yarp::os::Vocab::encode("ok"))){
            yInfo("Planner state updated!");
            return true;
        }
        else { //If opc2prada update fails:
            yError("Planner state update failed: something wrong with the opc2prada module");
            return false;
        }
    }
    else { //If WSM update fails
        yError("Planner state update failed: something wrong with the World State Manager");
    }
    return false;
}

// function responsible for completing the state.dat file. It negates all symbols not present on opc2prada update, due to the closed world assumption (if it is not true, it must be false)
bool PlannerThread::completePlannerState()
{
    string line, state_str, temp_str;
    vector<string> data, temp_vect, temp_vect2, avail_symb, temp_state;
    vector<vector<string> >  symbols;
    symbolFile.open(symbolFileName.c_str()); //symbols.dat
    stateFile.open(stateFileName.c_str()); //state.dat
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
    while (getline(symbolFile, line)){ // get symbols
        data.push_back(line);
    }
    symbolFile.close();
    for (int i = 0; i < data.size(); ++i){ // process symbols from the symbol file
        temp_vect = split(data[i],' ');
        temp_vect2.clear();
        temp_vect2.push_back(temp_vect[0]); // get symbol
        temp_vect2.push_back(temp_vect[2]); // get if it is primitive or action
        symbols.push_back(temp_vect2);
    }
    if (stateFile.is_open()){ // get state symbols
        getline(stateFile, line);
        while (!closing && !stopping){
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
    for (int i = 0; i < symbols.size(); ++i){ // add "negated" symbols to the possible symbol list
        temp_str = "-" + symbols[i][0];
        avail_symb.push_back(symbols[i][0]);
        avail_symb.push_back(temp_str);
    }
    data.clear();
    for (int i = 0; i < temp_state.size(); ++i){ // sanity check on world state symbols: symbols that are present on the world state, but not on the symbol file, will be deleted.
        if (find_element(avail_symb, temp_state[i]) == 1){
            data.push_back(temp_state[i]);
        }
    }
    for (int i = 0; i < data.size(); ++i){ // creates new "checked" world state
        temp_str = data[i]+"()";
        if (i == 0){
            state_str = temp_str;
        }
        else {
            state_str = state_str + " " + temp_str;
        }
    }
    state_str = state_str + " ";
    for (int i = 0; i < symbols.size(); ++i){ // adds symbols that were not present on the world state with value "false"
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
        yError("affordanceCommunication module not connected, unable to ground rules!");
        return false;
    }
    // instructs affordanceCommunication module to init
    Bottle& aff_bottle_out = aff_yarp.prepare();
    if (!stopping)
    {
        aff_bottle_out.clear();
        aff_bottle_out.addString("start");
        aff_yarp.write();
    }
    Time::delay(0.3);
    // instructs affordanceCommunication module to await commands from geometricGrounding
    if (!stopping)
    {
        aff_bottle_out = aff_yarp.prepare();
        aff_bottle_out.clear();
        aff_bottle_out.addString("update");
        aff_yarp.write();
    }
    while (!closing && !stopping) { // await confirmation from affordanceCommunication
        aff_bottle_in = aff_yarp.read(false);
        if (aff_bottle_in){
            yInfo("affordanceCommunication module is ready");
            break;
        }
        if (aff_yarp.getOutputCount() == 0)
        {
            yError("affordanceCommunication module crashed");
            return false;
        }
        Time::delay(0.1);
    }
    // instructs geometricGrounding module to start grounding
    double t0, t1; // for timing
    if (!stopping)
    {
        Bottle& geo_bottle_out = geo_yarp.prepare();
        geo_bottle_out.clear();
        geo_bottle_out.addString("update");
        t0 = yarp::os::Time::now();
        geo_yarp.write();
        yInfo("Grounding...");
    }
    while (!closing && !stopping) { // awaits confirmation that grounding is complete
        geo_bottle_in = geo_yarp.read(false);
        if (geo_bottle_in != NULL){
            command = geo_bottle_in->toString();
            yInfo("message received: %s", command.c_str());
        }
        if (command == "ready"){ // success!
            t1 = yarp::os::Time::now();
            yInfo("Grounding Complete! Elapsed time: %f", t1-t0);
            break;
        }
        if (geo_yarp.getOutputCount() == 0) // module crashed
        {
            yError("Geometric Grounding module crashed");
            return false;
        }
        if (command == "fail") // something failed on grounding (not crash)
        {
            yError("Grounding failed, there might be something wrong with the object list.");
            return false;
        }
        Time::delay(0.1);
    }
    // Query affordanceCommunication to get the position of the tool-handles (to point when asking for them)
    if (!stopping)
    {
        aff_bottle_out = aff_yarp.prepare();
        aff_bottle_out.clear();
        aff_bottle_out.addString("query");
        aff_yarp.write();
    }
    while (!closing && !stopping) {
        yarp::os::Time::delay(0.1);
        aff_bottle_in = aff_yarp.read(false);
        if (aff_bottle_in){
            data = aff_bottle_in->toString();
            if (data == "()" || data == "") // If message is corrupted/non-standard
            {
                yError("empty bottle received, something might be wrong with the affordances module.");
                return false;
            }
            //yDebug(data);
            while (!closing && !stopping){
                if (data.find('"') != std::string::npos){
                    data.replace(data.find('"'),1,"");
                }
                else {
                    break;
                }
            }
            toolhandle = split(data,' '); // store the position of the tool handles: (ID pos.x pos.y ID pos.x pos.y etc...)
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
    // Instructs goalCompiler module to await an instruction from praxiconInterface. Message might be sent before this, but it will only compile after this command
    Bottle& goal_bottle_out = goal_yarp.prepare();
    if (!stopping)
    {
        goal_bottle_out.clear();
        goal_bottle_out.addString("praxicon");
        goal_yarp.write();
    }
    yInfo("Waiting for praxicon..."); // waits for message, timeout at 5 minutes
    string mess_receiv;
    while (!closing && !stopping) {
        yarp::os::Time::delay(0.1);
        mess_receiv = "";
        goal_bottle_in = goal_yarp.read(false);
        if (goal_bottle_in)
        {
            mess_receiv = goal_bottle_in->toString();
            yDebug("message received: %s", goal_bottle_in->toString().c_str());
            if (mess_receiv == "done") // message arrived
            {
                yInfo("Praxicon instruction received, compiling...");
                break;
            }
            else if (mess_receiv == "failed_objects") // failed to obtain objects from planningCycle
            {
                yError("goalCompiler failed to load object list.");
                return false;
            }
            else if (mess_receiv == "failed_Praxicon") // timeout
            {
                yError("Praxicon crashed or took too long to reply (5 minutes timeout).");
                return false;
            }
            else if (mess_receiv == "unknown") // unknown object in the instructions provided by Praxicon
            {
                yWarning("Unknown object in Praxicon message, unable to compile.");
                return false;
            }
            else if (mess_receiv == "empty_bottle") // received an empty bottle from praxicon
            {
                yError("Empty bottle received from Praxicon; change the world and askPraxicon again");
                return false;
            }
            else // any other message:
            {
                yError("non-standard message received, something failed with the goalCompiler module.");
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
    double t0, t1; // for timing
    if (!stopping)
    {
        // Instructs the goalCompiler module to compile the set of subgoals
        goal_bottle_out = goal_yarp.prepare();
        goal_bottle_out.clear();
        goal_bottle_out.addString("update");
        t0 = yarp::os::Time::now();
        goal_yarp.write();
    }
    while (!closing && !stopping) {
        yarp::os::Time::delay(0.1);
        goal_bottle_in = goal_yarp.read(false);
        if (goal_bottle_in)
        {
            mess_receiv = goal_bottle_in->toString().c_str();
            yDebug("message received: %s", goal_bottle_in->toString().c_str());
            if (mess_receiv == "done") // success!
            {
                t1 = yarp::os::Time::now();
                yInfo("Goal Compiling is complete! Elapsed time: %f", t1-t0);
                break;
            }
            else if (mess_receiv == "failed_objects") // failed to obtain the objects from the planningCycle
            {
                yWarning("goalCompiler failed to load object list.");
                return false;
            }
            else if (mess_receiv == "failed_rules") // failed to obtain the rules from the rules file (rules.dat)
            {
                yWarning("goalCompiler failed to load rules list.");
                return false;
            }
            else if (mess_receiv == "failed_instructions") // failed to obtain instructions from the Praxicon message (probably won't ever happen, given previous messages)
            {
                yWarning("goalCompiler failed to load instructions list.");
                return false;
            }
            else if (mess_receiv == "failed_compiling") // failed during the compilation of the subgoals
            {
                yWarning("goalCompiler failed to compile the subgoals.");
                return false;
            }
            else if (mess_receiv == "failed_translation") // failed during the translation of the subgoals (labels->IDs)
            {
                yWarning("goalCompiler failed to translate the subgoals.");
                return false;
            }
            else if (mess_receiv == "failed_consistency") // failed the consistency check performed by the module (undoable plan)
            {
                yWarning("Consistency Check failed; the instructions provided by the Praxicon cannot be achieved.");
                // send message to praxicon
                return false;
            }
            else if (mess_receiv == "failed_pruning") // failed in eliminating undesirable subgoals
            {
                yWarning("goalCompiler failed to prune the subgoal list.");
                return false;
            }
            else if (mess_receiv == "failed_writing") // failed in writing to the subgoal.dat file
            {
                yWarning("goalCompiler failed to write subgoals to the subgoal file.");
                return false;
            }
            else // any other error:
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

// function that loads the subgoals from the subgoal.dat file to a vector (internal memory of the planningCycle)
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

// function that loads the goal from the goal.dat file to a vector (internal memory of the planningCycle)
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

// resets the Config file (used by PRADA), to the default values (horizon = 5)
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

// rpc function that prints the list of object IDs and labels
Bottle PlannerThread::printObjs()
{
    return object_bottle;
}

/* - For future use (tool names propagation)
Bottle PlannerThread::printTools()
{
    return tool_bottle;
}

bool PlannerThread::loadTools()
{
    vector<string> temp_vect;
    vector<string> labels;

    if (actInt_rpc.getOutputCount() == 0)
    {
        yError("activityInterface not connected!");
        return false;
    }

    Bottle actIntCmd, actIntReply;
    wsmCmd.addString("getToolNames"); // command might be different
    actInt_rpc.write(actIntCmd, actIntReply);
    if (actIntReply.size() > 0)
    {
        tool_bottle.clear();
        tool_bottle = *actIntReply.get(0).asList();
        return true;
    }
    else
    {
        yError("Failed to obtain tool-like names");
        return false;
    }
    return false;
}
*/

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
        object_bottle.clear(); // creates a bottle for the "printObjects" command
        object_bottle = *reply.get(0).asList();
        for (int i = 0; i < reply.get(0).asList()->size(); ++i)
        {
            temp_vect.clear();
            temp_vect.push_back( NumbertoString(reply.get(0).asList()->get(i).asList()->get(0).asInt() ) );
            temp_vect.push_back(reply.get(0).asList()->get(i).asList()->get(1).asString());
            object_IDs.push_back(temp_vect);
               if (find_element(labels,temp_vect[1]) == 1) // If one of the labels is already present on the list, that is, is a duplicate
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

// stops all planning and functions, resets planning variables
void PlannerThread::stopPlanning()
{
    startPlan = false;
    stopping = true;
    restartPlan = true;
    plan_level = 0;
    resetConfig();
    resumePlan = true;
    yInfo("Planning stopped");
    return;
}

// resets planning variables at init
bool PlannerThread::resetPlanVars()
{
    plan_level = 0;
    objects_used.clear();
    next_action == "";
    return resetConfig();
}

// loads state symbols from the state.dat file to a vector (internal memory)
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

// stores the state into a vector that is only changed once an action is successful (to check for failure/success)
bool PlannerThread::preserveState()
{
    old_state = state;
    return true;
}

// compares the current world state with the previous one, to detect action failure/success
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

// preserves rules after grounding, in order to revert after adapting them 
bool PlannerThread::preserveRules()
{
    old_rules = rules;
    return true;
}

// loads rules from the file rules.dat into a vector (internal memory)
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

// function that lowers the probability of success of actions that fail
// adaptability heuristic
bool PlannerThread::adaptRules()
{
    string temp_str;
    vector<string> adapt_rules, adapt_noise;
    if (next_action == "")
    {
        yDebug("no action to adapt");
        return true;
    }
    rulesFileOut.open(rulesFileName.c_str());
    if (!rulesFileOut.is_open())
    {
        yError("unable to open rules file!");
        return false;
    }
    //yDebug("before for: %s", next_action.c_str());
    for (int t = 0; t < rules.size(); ++t){ // clears the action vector to be able to compare it with "next_action"
        temp_str = rules[t];
        while (!closing && !stopping){
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
        //yDebug("action to be adapted: %s", next_action.c_str());
        if (temp_str == next_action && next_action != ""){ // if there was a planned next action:
            int p = 0;
            while (!closing && !stopping){
                if (rules[t+p] == ""){
                    adapt_rules = split(rules[t+4], ' ');
                    adapt_rules[2] = static_cast<ostringstream*>( &(ostringstream() << (atof(adapt_rules[2].c_str())/5) ))->str(); // probability = probability/5
                    temp_str = "";
                    for (int h = 0; h < adapt_rules.size(); ++h){
                        if (adapt_rules[h] != "0")
                        {
                            temp_str = temp_str + adapt_rules[h] + " ";
                        }
                    }
                    rules[t+4] = temp_str;
                    adapt_noise = split(rules[t+p-1], ' ');
                    adapt_noise[2] = static_cast<ostringstream*>( &(ostringstream() << (atof(adapt_noise[2].c_str()) + 4*atof(adapt_rules[2].c_str())) ))->str(); // adds the remaining probability to noise so that all adds up to 1
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
            for (int y = t; y < t+p; ++y)
            {
                yDebug("%s", rules[y].c_str());
            }
            yInfo("Rules adapted, probability of %s reduced", next_action.c_str());
            break;
        }
    }
    next_action = "";
    rulesFileOut.close();
    return true;
}

// updates the goal according to the plan-level (indicating where in the plan we are)
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
    if (plan_level >= subgoals.size()) // if plan-level is bigger than the maximum size of the subgoals:
    {
        plan_level = subgoals.size()-1; // point to the last sub-goal
    }
    for (int y = 0; y < subgoals[plan_level].size(); ++y){ // writes the new goal to the file
        goalFileOut << subgoals[plan_level][y] << " ";
    }
    goalFileOut.close();
    return true;
}

// function that checks if the plan was completed, and warns activity Interface
bool PlannerThread::planCompletion()
{
    if (plan_level >= subgoals.size()){
        yInfo("Plan completed!!");
        Bottle& prax_bottle_out = prax_yarp.prepare();
        prax_bottle_out.clear();
        prax_bottle_out.addString("OK"); // adds "OK" to the bottle to activityInterface
        for (int u = 0; u < objects_used.size(); ++u){
            for (int inde = 0; inde < object_IDs.size(); ++inde){
                if (object_IDs[inde][0] == objects_used[u]){
                    string check_str=object_IDs[inde][1];
                    transform(check_str.begin(), check_str.end(), check_str.begin(), ::tolower);
                    if (check_str != "rake" && check_str != "stick" && check_str != "left" && check_str != "right" /*&& find_element(tool_list,check_str) == 0 */){
                        prax_bottle_out.addString(object_IDs[inde][1]); // adds objects used in completing the goal to the activityInterface bottle
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

// function that directly calls planner.exe
int PlannerThread::PRADA()
{
    string line;
    vector<string> pipe_vect;
    string next_sequence;
    int retrn_flag = 2;
    // prepare a pipe file
    FILE * pFile;
    pFile = fopen(pipeFileName.c_str(),"w");
    fclose(pFile);
    double t0 = yarp::os::Time::now();
    int sys_flag = system(process_string.c_str());
    double t1 = yarp::os::Time::now();
    yInfo("PRADA elapsed time: %f", t1-t0);
    if (sys_flag == 34304) // code for executable failure
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
    for (int t = 0; t < pipe_vect.size(); ++t){ // searches for the "recomended action" line
        if (pipe_vect[t] == "The planner would like to kindly recommend the following action to you:" && t+1 < pipe_vect.size()){
            next_action = pipe_vect[t+1];
            cout << endl;
            yInfo("Action found: %s", next_action.c_str());
            retrn_flag = 1;
        }
        if (pipe_vect[t] == "*** Planning for a complete plan." && t+3 < pipe_vect.size()){ // gets full action sequence planned
            next_sequence = pipe_vect[t+3];
            cout << endl;
            yDebug("Sequence found: %s", next_sequence.c_str());
            return retrn_flag;
        }
    }
    return retrn_flag;
}

// increases planning horizon by 1 (config file must be changed for PRADA to noticed this)
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

            // creativity heuristic
            if (horizon > 10 && useCreativity) // if horizon is too large already
            {
                yWarning("horizon too large, jumping to next goal");
                jumpForward(); // jumps to next plan-level
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
                if ((plan_level >= subgoals.size() && !checkGoalCompletion()) || (!checkHoldingSymbols() && useGoalConsistency)) // if plan-level already exceeds maximum subgoal size, and the goal was not completed
                {
                    for (int t = 0; t < failed_goal.size(); ++t)
                    {
                        temp_vect = split(failed_goal[t], '_');
                        objects_failed.push_back(temp_vect[0]);
                        objects_failed.push_back(temp_vect[2]);
                    }
                    yInfo("Plan failed");
                    // plan fails, add FAIL to activityInterface bottle
                    Bottle& prax_bottle_out = prax_yarp.prepare();
                    prax_bottle_out.clear();
                    prax_bottle_out.addString("FAIL");
                    for (int u = 0; u < objects_failed.size(); ++u){
                        for (int inde = 0; inde < object_IDs.size(); ++inde){
                            if (object_IDs[inde][0] == objects_failed[u]){
                                string check_str=object_IDs[inde][1];
                                transform(check_str.begin(), check_str.end(), check_str.begin(), ::tolower);
                                if (check_str != "rake" && check_str != "stick" && check_str != "left" && check_str != "right" /*&& find_element(tool_list,check_str) == 0 */){
                                    bool isPresent;
                                    IDisPresent(objects_failed[u], isPresent);
                                    if (isPresent)
                                    {
                                        yDebug("Object is present: %s", objects_failed[u].c_str());
                                    }
                                    else
                                    {
                                        yDebug("object is not present: %s", objects_failed[u].c_str());
                                    }
                                    if (!isPresent)
                                    {
                                        prax_bottle_out.addString(object_IDs[inde][1]); // add objects that were involved in the failure
                                    }
                                }
                            }
                        }
                    }
                    yDebug("Sending to Praxicon: %s", prax_bottle_out.toString().c_str());
                    prax_yarp.write(); 
                    restartPlan = true;
                    return false; // leaves function
                }
            }
//----------In case we are NOT using the creativity heuristic-----------------------------------
            else if (horizon > 10 && !useCreativity)
            {
                yInfo("Plan failed, horizon is too large");
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
                if ((!checkGoalCompletion()) || (!checkHoldingSymbols() && useGoalConsistency)) // if the goal was not completed
                {
                    for (int t = 0; t < failed_goal.size(); ++t)
                    {
                        temp_vect = split(failed_goal[t], '_');
                        objects_failed.push_back(temp_vect[0]);
                        objects_failed.push_back(temp_vect[2]);
                    }
                    yInfo("Plan failed");
                    // plan fails, add FAIL to activityInterface bottle
                    Bottle& prax_bottle_out = prax_yarp.prepare();
                    prax_bottle_out.clear();
                    prax_bottle_out.addString("FAIL");
                    for (int u = 0; u < objects_failed.size(); ++u){
                        for (int inde = 0; inde < object_IDs.size(); ++inde){
                            if (object_IDs[inde][0] == objects_failed[u]){
                                string check_str=object_IDs[inde][1];
                                transform(check_str.begin(), check_str.end(), check_str.begin(), ::tolower);
                                if (check_str != "rake" && check_str != "stick" && check_str != "left" && check_str != "right" /*&& find_element(tool_list,check_str) == 0 */){
                                    bool isPresent;
                                    IDisPresent(objects_failed[u], isPresent);
                                    if (isPresent)
                                    {
                                        yDebug("Object is present: %s", objects_failed[u].c_str());
                                    }
                                    else
                                    {
                                        yDebug("object is not present: %s", objects_failed[u].c_str());
                                    }
                                    if (!isPresent)
                                    {
                                        prax_bottle_out.addString(object_IDs[inde][1]); // add objects that were involved in the failure
                                    }
                                }
                            }
                        }
                    }
                    yDebug("Sending to Praxicon: %s", prax_bottle_out.toString().c_str());
                    prax_yarp.write();
                    restartPlan = true;
                    return false; // leaves function
                }
            }

            configData[w+2] = "PRADA_horizon " + static_cast<ostringstream*>( &(ostringstream() << horizon) )->str(); // if it didn't fail, update horizon + 1
            break;
        }
    }
    configFileOut.open(configFileName.c_str()); // re-writes the config file with the new horizon

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

bool PlannerThread::IDisPresent(string ID, bool &result)
{
    vector<string> split_symbol;
    if (state.size() > 0)
    {
        for (int i = 0; i < state.size(); ++i)
        {
            if (state[i].find('-') == std::string::npos && state[i].find(ID) != std::string::npos) // If there is a "true" symbol on the world state with the object ID
            {
                //yDebug("the symbol being checked is: %s", state[i].c_str());
                if (state[i].find("isreachable_") != std::string::npos) // If that symbols is "isreachable"
                {
                    split_symbol = split(state[i], '_');
                    if (split_symbol[0] == ID) // If the object is the one that is reachable
                    {
                        result = true;
                        return true;
                    } 
                }
                if (state[i].find("inhand_") != std::string::npos) // if that symbol is "inhand"
                {
                    result = true;
                    return true;
                }
                if (state[i].find("on_") != std::string::npos) // if that symbol is "on"; object is present on a stack
                {
                    result = true;
                    return true;
                }
            }
        }
    }
    else
    {
        return false;
    }
    yDebug("Object not present: %s", ID.c_str());
    result = false;
    return true;
}

// function that checks which symbols are present across subgoals
// goal consistency heuristic
bool PlannerThread::checkHoldingSymbols()
{
    yDebug("checking symbols that still hold...");
    vector<string> holding_symbols;
    if (plan_level >= 1){
        for (int t = 0; t < subgoals[plan_level-1].size(); ++t){
            if (find_element(subgoals[plan_level],subgoals[plan_level-1][t]) == 1){
                holding_symbols.push_back(subgoals[plan_level-1][t]);
                yDebug("%s", subgoals[plan_level-1][t].c_str());
            }
        }
/*        if (holding_symbols.size() != 0){
            holding_symbols.erase(holding_symbols.end());
        }*/
        for (int t = 0; t < holding_symbols.size(); ++t){
            yDebug("checking: %s", holding_symbols[t].c_str());
            if (find_element(state, holding_symbols[t])== 0){ // If one of the symbols present in an already-achieved subgoal is not present on the current state:
                yInfo("Situation changed, receding in plan");
                return false; 
            }
        }
    }
    return true;
}

// jumps one step forward on the subgoal list (needs goalUpdate to load the new subgoal)
bool PlannerThread::jumpForward()
{
    plan_level = plan_level + 1;
    return true;
}

// jumps one step back on the subgoal list (needs goalUpdate to load new subgoal)
bool PlannerThread::jumpBack()
{
    plan_level = plan_level - 1;
    return true;
}

// Resets the rules to after-grounding levels (resets any adaptation during the plan)
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

// function that stores the list of objects used to complete the goal
bool PlannerThread::loadUsedObjs()
{
    vector<string> aux_used;
    for (int y = 0; y < object_IDs.size(); ++y){
        string check_str=object_IDs[y][1];
        transform(check_str.begin(), check_str.end(), check_str.begin(), ::tolower);
        if (next_action.find(object_IDs[y][0]) != std::string::npos && check_str != "stick" && check_str != "rake" && check_str != "left" && check_str != "right" /*&& find_element(tool_list,check_str) == 0 */){ // If the object is not a tool or hand (commented segment for when tool names are propagated)
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

// Function that encodes the action to send to the motor executor (IDs for labels, correct action name, , object positions, etc)
bool PlannerThread::codeAction()
{
    vector<string> temp_vect;
    float temp_float;
    string tool1, tool2;
    temp_vect = split(next_action, '_');
    if (find_element(temp_vect, "on") == 1){ // planned action is "put_tool_on_obj_with_hand()" so must parse the message to remove these elements
        obj = temp_vect[1]; // "tool"
        hand = temp_vect[3]; // "obj"
        while (!closing && !stopping){ // remove the final ()
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
        act = temp_vect[0]; // "put"
    }
    else { // for actions like "drop/grasp_obj_with_hand, or pull/push_obj_with_tool_in_hand"
        act = temp_vect[0]; // "pull, push, grasp or drop"
        obj = temp_vect[1]; // obj
        hand = temp_vect[3]; // tool (or hand, in case of grasp)
        while (!closing && !stopping){ // remove the final ()
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
        for (int ID = 0; ID < object_IDs.size();++ID){ // checks the object list for tools, and isolates them (this can be improved by a "tool list", propagated through the whole pipeline)
            string check_str = object_IDs[ID][1];
            transform(check_str.begin(), check_str.end(), check_str.begin(), ::tolower);
            if (check_str == "rake"){
                tool1 = object_IDs[ID][0];
            }
            if (check_str == "stick"){
                tool2 = object_IDs[ID][0];
            }
            /*if (find_element(tool_list,check_str) == 1)
            {
                tool_obj = object_IDs[ID][0];
            }*/
        }
        if (act == "grasp" && (obj == tool1 || obj == tool2) /*&& obj == tool_obj*/){ // If one of the objects used in the action is a tool, and the action is grasp, then it should request "askForTool", with position X and Y
            for (int i = 0; i < toolhandle.size(); ++i){
                if (toolhandle[i] == obj){
                    temp_float = strtof(toolhandle[i+1].c_str(), NULL);
                    positx = (int) (temp_float); // position on X axis
                    temp_float = strtof(toolhandle[i+2].c_str(), NULL);
                    posity = (int) (temp_float); // position on Y axis
                }
            }
        }
    }
    for (int k = 0; k < object_IDs.size(); ++k){ // translation from IDs to labels
        if (act == object_IDs[k][0]){
            act = object_IDs[k][1];
        }
        if (obj == object_IDs[k][0]){
            obj = object_IDs[k][1];
        }
        if (hand == object_IDs[k][0]){
            hand = object_IDs[k][1];
            while (!closing && !stopping){ // eliminates the "hand" string, since only "left/right" should be sent
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

// Function that creates the message and sends it to the motor executor
bool PlannerThread:: execAction()
{
    string temp_str;
    message.clear();
    string check_strobj = obj;
    transform(check_strobj.begin(), check_strobj.end(), check_strobj.begin(), ::tolower);
    if (act == "grasp" && (check_strobj == "rake" || check_strobj == "stick") /*&& find_element(tool_list,check_strobj) == 1 */){ // in case the action is an "askForTool hand X Y"
        act = "askForTool";
        message.addString(act); // askForTool
        message.addString(hand); // hand (left/right)
        message.addInt(positx); // position on X axis of the tool
        message.addInt(posity); // position on Y axis of the tool
    }
    else if (act == "grasp" && (check_strobj != "rake" && check_strobj != "stick" /*&& find_element(tool_list,check_strobj) == 0 */)){ // If it is not an "askForTool"
        act = "take";
        message.addString(act); // take
        message.addString(obj); // object label
        message.addString(hand); // hand for grasping (left/right)
    }
    else { // for drop, pull, push, and put
        message.addString(act); 
        message.addString(obj);
        message.addString(hand);
    }
    yInfo("Request execution of action: %s" , message.toString().c_str());
    while (!closing && !stopping){ // send message to activityInterface, looped in case empty message is returned, action should be repeated
        yarp::os::Time::delay(0.1);
        actInt_rpc.write(message, reply);
        yInfo("Received reply: %s", reply.toString().c_str());
        if (reply.size() == 1 && reply.get(0).asVocab() == 27503){ // In case action was well received
            prev_action = message.get(1).toString();
            return true;
        }
        if (reply.size() == 1 && reply.get(0).asVocab() != 27503){ // In case the message failed to be received
            yWarning("Nack received, there might be something wrong with the message.");
            yWarning(" %s",message.toString().c_str());
            return false;
        }
        if (reply.size() != 1){ // In case something happens with activityInterface, reply is empty
            yError("activityInterface is not connected, verify if the module is running, and all connections are established.");
            return false;
        }
    }
    return false;
}

// Function that checks the current world state to see if all the current goals have been completed
bool PlannerThread::checkGoalCompletion()
{
    for (int t = 0; t < goal.size(); ++t){
        if (find_element(state, goal[t]) == 0){
            return false;
        }
    }
    return true;
}

// Function that detects and reports the failure of a plan
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
    if (horizon > 10){ // if the Horizon is already too large
        not_comp_goals.clear();
        for (int t = 0; t < goal.size(); ++t){
            if (find_element(state, goal[t]) == 0){ // adds all subgoals not present on the world state to the "failed goals" list
                not_comp_goals.push_back(goal[t]);
            }
        }
        temp_str = "";
        for (int i = 0; i < not_comp_goals.size(); ++i){
            temp_str = temp_str + not_comp_goals[i] + " ";
        }
        for (int i = 0; i < object_IDs.size(); ++i){ // adds all objects belonging to failed goals to the list of objects that resulted in the failure of the plan
            if (temp_str.find(object_IDs[i][0]) != std::string::npos){
                fail_obj.push_back(object_IDs[i][0]);
            }
        }
        /*
        for (int u = 0; u < fail_obj.size(); ++u){ // If an object from the failed list is not a tool or a hand, add it to the list to be reported
            if (fail_obj[u] != "11" && fail_obj[u] != "12" && find_element(toolhandle,fail_obj[u]) == 0){
                for (int t = 0; t < object_IDs.size(); ++t){
                    if (find_element(object_IDs[t], fail_obj[u]) == 1){
                        aux_fail_obj.push_back(object_IDs[t][1]);
                        break;
                    }
                }
            }
        }
        */
        Bottle& prax_bottle_out = prax_yarp.prepare();
        prax_bottle_out.clear();
        prax_bottle_out.addString("FAIL"); // Reports "FAIL" back to activityInterface
        // and adds the bottle of objects responsible for the failure
        for (int u = 0; u < fail_obj.size(); ++u){
            for (int inde = 0; inde < object_IDs.size(); ++inde){
                if (object_IDs[inde][0] == fail_obj[u]){
                    string check_str = object_IDs[inde][1];
                    transform(check_str.begin(), check_str.end(), check_str.begin(), ::tolower);
                    if (check_str != "rake" && check_str != "stick" && check_str != "left" && check_str != "right" /*&& find_element(tool_list,check_str) == 0 */) // if not a tool or a hand
                    {
                        bool isPresent;
                        IDisPresent(fail_obj[u], isPresent);
                        if (!isPresent)
                        {
                            prax_bottle_out.addString(object_IDs[inde][1]); // add objects that were involved in the failure
                        }
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

// Planning initialization state machine
bool PlannerThread::plan_init()
{
    if (!checkPause())
    {
        return false;
    }
    if (!resetPlanVars()) // resets planning variables and initializes world state
    {
        return false;
    }
    if (!checkPause())
    {
        return false;
    }
    if (!updateState()) // first world state update
    {
        return false;
    }
    if (!checkPause())
    {
        return false;
    }
    if (!loadObjs()) // request object list
    {
        return false;
    }
    if (!loadState()) // loads world state
    {
        return false;
    }
    if (!checkPause())
    {
        return false;
    }
    if (!groundRules()) // instructs geometricGrounding to ground rules
    {
        return false;
    }
    if (!checkPause())
    {
        return false;
    }
    if (!completePlannerState()) // completes the world state with the negated symbols that are not present on the world state
    {
        return false;
    }
    if (!checkPause())
    {
        return false;
    }
    if (!preserveState()) // stores current state
    {
        return false;
    }
    if (!checkPause())
    {
        return false;
    }
    if (!compileGoal()) // instructs goalCompiler module to compile the set of instructions from Praxicon
    {
        return false;
    }
    if (!checkPause())
    {
        return false;
    }
    if (!loadSubgoals()) // loads subgoals compiled by the goalCompiler
    {
        return false;
    }
    if (!checkPause())
    {
        return false;
    } 
    if (!loadRules()) // loads rules grounded by the geometricGrounding
    {
        return false;
    }
    if (!checkPause())
    {
        return false;
    }
    if (!preserveRules()) // stores grounded rules for future resets
    {
        return false;
    }
    if (!checkPause())
    {
        return false;
    }
    if (!goalUpdate()) // updates the current goal from the subgoals list
    {
        return false;
    }
    if (!checkPause())
    {
        return false;
    }
    if (!loadGoal()) // loads the current goal
    {
        return false;
    }
    if (!checkPause())
    {
        return false;
    }
    return true;
}

// main planning cycle state machine
bool PlannerThread::planning_cycle()
{
    if (!checkPause())
    {
        return false;
    }
    if (!updateState()) // updates the world state at the beginning of each cycle
    {
        return false;
    }
    if (!checkPause())
    {
        return false;
    }
    if (!completePlannerState()) // completes the world state with the negated symbols that are not present on the world state
    {
        return false;
    }
    if (!checkPause())
    {
        return false;
    }
    if (!loadState()) // loads current state into memory
    {
        return false;
    }
    if (!checkPause())
    {
        return false;
    }
    if (useAdaptability)
    {
        if (compareState()) // compares the current world state to the previous world state (from preserve state)
        {
            if (!checkPause())
            {
                return false;
            }
            if (!adaptRules()) // If the state hasn't changed, adapt the last action (it failed)
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
    if (!goalUpdate()) // update current goal with the corresponding subgoal (depending on plan-level)
    {
        return false;
    }
    if (!checkPause())
    {
        return false;
    }
    if (!loadGoal()) // loads current goal into memory
    {
        return false;
    }
    if (!checkPause())
    {
        return false;
    }
    if (useGoalConsistency)
    {
        if (!checkHoldingSymbols()) // check if symbols belonging to previous goals are still true on the current goal
        {
            if (!checkPause())
            {
                return false;
            }
            jumpBack(); // If there are symbols that disappeared, jump back one plan-level
            if (!checkPause())
            {
                return false;
            }
            if (!resetConfig()) // reset Horizon
            {
                return false;
            }
            if (!checkPause())
            {
                return false;
            }
        }
        else // If the previous goal is still verified
        {
// NOTE: THIS CAN PROBABLY BE OPTIMISED (the code in this "else" is the same as the one when "useGoalConsistency == false"). NEEDS FURTHER TESTING BEFORE THAT.
            if (!checkPause())
            {
                return false;
            }
            if (checkGoalCompletion()) // check if the current goal has been completed
            {
                if (!checkPause())
                {
                    return false;
                }
                if (!resetRules()) // If the goal was completed, reset the rules to after-grounding probability values
                {
                    return false;
                }
                if (!checkPause())
                {
                    return false;
                }
                if (!resetConfig()) // reset planning horizon back to 5
                {
                    return false;
                }
                if (!checkPause())
                {
                    return false;
                }
                if (!loadRules()) // load reset rules
                {
                    return false;
                }
                if (!checkPause())
                {
                    return false;
                }
                if (!jumpForward()) // jump one plan-level forward
                {
                    return false;
                }
                if (!checkPause())
                {
                    return false;
                }
                if (!planCompletion()) // check if all the subgoals have been met (plan completed)
                {
                    return true; // if there are still goals to be completed, planning should be resumed
                }
                if (!checkPause())
                {
                    return false;
                }
                return true;
            }
            else { // if the current goal has yet to be completed
                if (!checkPause())
                {
                    return false;
                }
                string tmp_str = showCurrentGoal(); 
                yInfo("Current subgoal: %s", tmp_str.c_str());
                int flag_prada = PRADA(); // run PRADA planner (planner.exe)
                if (!checkPause())
                {
                    return false;
                }
                if (flag_prada == 0) // if PRADA fails for any reason, terminate plan
                {
                    return false;
                }
                else if (flag_prada == 2) // If PRADA finds no valid action:
                {
                    if (!increaseHorizon()) // increase planning horizon by 1
                    {
                        return false;
                    }
                    if (!checkPause())
                    {
                        return false;
                    }
                    return true; // resume plan
                }
                if (!loadUsedObjs()) // loads the objects used by the current action into memory
                {
                    return false;
                }
                if (!checkPause())
                {
                    return false;
                }
                if (!codeAction()) // encodes the action to be sent to activityInterface
                {
                    return false;
                }
                if (!checkPause())
                {
                    return false;
                }
                if (!execAction()) // instructs execution of action
                {
                    return false;
                }
                if (!checkPause())
                {
                    return false;
                }
                if (!preserveState()) // stores the current world state to compare later
                {
                    return false;
                }
                if (!checkPause())
                {
                    return false;
                }
                if (!checkFailure()) // checks if plan has failed before continuing
                {
                    return false;
                }
                if (!checkPause())
                {
                    return false;
                }
            }
        }
    }
    else // If goal consistency heuristic is not being used
    {
        if (!checkPause())
        {
            return false;
        }
        if (checkGoalCompletion()) // check if the current goal has been completed
        {
            if (!checkPause())
            {
                return false;
            }
            if (!resetRules()) // If the goal was completed, reset the rules to after-grounding probability values
            {
                return false;
            }
            if (!checkPause())
            {
                return false;
            }
            if (!resetConfig()) // reset planning horizon back to 5
            {
                return false;
            }
            if (!checkPause())
            {
                return false;
            }
            if (!loadRules()) // load reset rules
            {
                return false;
            }
            if (!checkPause())
            {
                return false;
            }
            if (!jumpForward()) // jump one plan-level forward
            {
                return false;
            }
            if (!checkPause())
            {
                return false;
            }
            if (!planCompletion()) // check if all the subgoals have been met (plan completed)
            {
                return true; // if there are still goals to be completed, planning should be resumed
            }
            if (!checkPause())
            {
                return false;
            }
            return true;
        }
        else { // if the current goal has yet to be completed
            if (!checkPause())
            {
                return false;
            }
            string tmp_str = showCurrentGoal(); 
            yInfo("Current subgoal: %s", tmp_str.c_str());
            int flag_prada = PRADA(); // run PRADA planner (planner.exe)
            if (!checkPause())
            {
                return false;
            }
            if (flag_prada == 0) // if PRADA fails for any reason, terminate plan
            {
                return false;
            }
            else if (flag_prada == 2) // If PRADA finds no valid action:
            {
                if (!increaseHorizon()) // increase planning horizon by 1
                {
                    return false;
                }
                if (!checkPause())
                {
                    return false;
                }
                return true; // resume plan
            }
            if (!loadUsedObjs()) // loads the objects used by the current action into memory
            {
                return false;
            }
            if (!checkPause())
            {
                return false;
            }
            if (!codeAction()) // encodes the action to be sent to activityInterface
            {
                return false;
            }
            if (!checkPause())
            {
                return false;
            }
            if (!execAction()) // instructs execution of action
            {
                return false;
            }
            if (!checkPause())
            {
                return false;
            }
            if (!preserveState()) // stores the current world state to compare later
            {
                return false;
            }
            if (!checkPause())
            {
                return false;
            }
            if (!checkFailure()) // checks if plan has failed before continuing
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

// RPC function to show the human the current planned action
string PlannerThread::showPlannedAction()
{
    return next_action;
}

// RPC function to show the human the current world state
string PlannerThread::showCurrentState()
{
    string temp_str = "";
    for (int i = 0; i < state.size(); ++i)
    {
        temp_str = temp_str + state[i] + " ";
    }
    return temp_str; 
}

// RPC function to show the current goal
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

// RPC Function that prints all instances of specified "symbol"
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
/*
Bottle PlannerThread::getToolList()
{
    tool_list.clear();
    message.clear();
    reply.clear();
    message.addString("getToolNames");
    actInt_rpc.write(message, reply);
    if (reply.size() != 0);
    {
        for (int i = 0; i < reply.size(); ++i)
        {
            tool_list.push_back(reply.get(i).asString());
        }
    }
    else
    {
        yWarning("no tools found: either there are no tools present, or there was an error with the message");
    }
    return reply;
}
*/

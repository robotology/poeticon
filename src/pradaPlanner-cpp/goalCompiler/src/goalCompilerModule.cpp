#include "goalCompilerModule.h"

bool goalCompiler::configure(ResourceFinder &rf)
{
    // module parameters
    moduleName = rf.check("name", Value("goalCompiler")).asString();
    PathName = rf.findPath("contexts/"+rf.getContext());
    setName(moduleName.c_str());

    closing = false;

    if (PathName==""){
        cout << "path to contexts/"+rf.getContext() << " not found" << endl;
        return false;    
    }
    else {
        cout << "Context FOUND!" << endl;
    }
    return true;
}

bool goalCompiler::updateModule()
{
    return !closing;
}

void goalCompiler::openFiles()
{
    objIDsFileName = PathName + "/Object_names-IDs.dat";
    goalFileName =PathName + "/goal.dat";
    subgoalFileName = PathName + "/subgoals.dat";
    preRuleFileName = PathName + "/pre_rules.dat";
}

void goalCompiler::openPorts()
{
    plannerPort.open("/goal_imag/planner_cmd:io");
    praxiconPort.open("/goal_imag/prax_inst:i");
}

bool goalCompiler::close()
{
    cout << "closing..." << endl;
    closing = true;
    plannerPort.close();
    praxiconPort.close();
    return true;
}

bool goalCompiler::interrupt()
{
    cout << "interrupting ports" << endl;
    plannerPort.interrupt();
    praxiconPort.interrupt();
    return true;
}

string goalCompiler::plannerCommand()
{
    string command;
    if (plannerPort.getInputCount() == 0)
    {
        cout << "planner not connected" << endl;
        return "failed";
    }
    while (true){
        plannerBottle = plannerPort.read(false);
        if (plannerBottle != NULL){
            command = plannerBottle->toString().c_str();
            return command;
        }
    }
}

bool goalCompiler::receiveInstructions()
{
    string temp_str;
    int timer_count = 0;
    while (true){
        if (timer_count == 3000)
        {
            cout << "timeout: no instructions received before 5 minutes time" << endl;
            return false;
        }
        praxiconBottle = praxiconPort.read(false);
        if (praxiconBottle != NULL){
            cout << "bottle received:" << praxiconBottle->toString().c_str() << endl;
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
                                cout << "failed to compile: unknown object -> " << temp_str << endl;
                                return false;
                            }
                            temp1_instructions = temp1_instructions + temp_str + " " ;
                            cout << temp1_instructions << endl;
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
                return true;
            }
        }
        if (praxiconPort.getInputCount() == 0)
        {
            cout << "praxicon crashed or disconnected" << endl;
            Bottle &plannerBottleOut = plannerPort.prepare();
            plannerBottleOut.clear();
            plannerBottleOut.addString("failed");
            plannerPort.write();
            return false;
        }
        timer_count = timer_count + 1;
        Time::delay(0.1);
    }
}

bool goalCompiler::loadObjs()
{
    string line;
    objectFile.open(objIDsFileName.c_str());
    vector<string> objects;
    if (objectFile.is_open()){
        getline(objectFile,line);
        objects = split(line, ';');
    }
    else {
        cout << "unable to open objects file" << endl;
        return false;
    }
    vector<string> temp_trans;
    string temp_str;
    for (int j = 0; j < objects.size(); ++j){
        temp_str = objects[j];
        temp_str.replace(temp_str.find("("), 1,"");
        temp_str.replace(temp_str.find(")"), 1,"");
        temp_trans = split(temp_str,',');
        translat.push_back(temp_trans);
    }
    return true;
}

bool goalCompiler::loadRules()
{
    string line;
    preRuleFile.open(preRuleFileName.c_str());
    if (!preRuleFile.is_open())
    {
        cout << "unable to open rule file" << endl;
        return false;
    }
    while ( getline(preRuleFile, line)){
        actions.push_back(line);
    }
    return true;
}

bool goalCompiler::loadInstructions()
{
    for (int g = 0; g < instructions.size(); ++g){
        for (int l = 0; l < instructions[g].size(); ++l){
            cout << instructions[g][l] << endl;
            if (instructions[g][l].find("hand") != std::string::npos){
                instructions[g][l].replace(instructions[g][l].find("hand"),4,"left");
            }
        }
    }
    return true;
}

bool goalCompiler::processFirstInst()
{
    string temp_str;
    vector<string> temp_vect, aux_subgoal;
    for (int g = 0; g<instructions[0].size(); ++g){
        temp_vect = split(instructions[0][g], ' ');
        temp_str = temp_vect[1];
        if (temp_str != "reach"){
            temp_str.push_back('_');
            for (int h = 0; h < actions.size(); ++h){
                if (actions[h].find(temp_str) != std::string::npos){
                    temp_str = actions[h+2];
                    while (true){
                        if (temp_str.find("_obj") != std::string::npos){
                            temp_str.replace(temp_str.find("_obj"),4,temp_vect[2]);
                        }
                        else {
                            break;
                        }
                    }
                    while (true){
                        if (temp_str.find("_tool") != std::string::npos){
                            temp_str.replace(temp_str.find("_tool"),5,temp_vect[0]);
                        }
                        else {
                            break;
                        }
                    }
                    while (true){
                        if (temp_str.find("_hand") != std::string::npos){
                            temp_str.replace(temp_str.find("_hand"),5,"left");
                        }
                        else {
                            break;
                        }
                    }
                    aux_subgoal = split(temp_str,' ');
                    aux_subgoal.erase(aux_subgoal.begin(),aux_subgoal.begin()+2);
                    break;
                }
            }
        break;
        }
    }
    subgoals.push_back(aux_subgoal);
    return true;
}

bool goalCompiler::compile()
{
    if (instructions[0].size() == 0)
    {
        cout << "no instructions to compile." << endl;
        return false;
    }
    vector<string> prax_action, new_action, temp_rule, new_temp_rule, aux_subgoal;
    string obj, tool, temp_str;
    for (int g = 0; g<instructions[0].size(); ++g){
        prax_action = split(instructions[0][g], ' ');
        prax_action[1].push_back('_');
        if (prax_action[1] != "reach"){
            cout << "action detected" << endl;
            for (int j = 0; j < actions.size(); ++j){
                if (actions[j].find(prax_action[1]) != std::string::npos){
                    obj = prax_action[2];
                    cout << "action found" << endl;
                    if (actions[j+4].find("_ALL") != std::string::npos){
                        cout << "ALL detected" << endl;
                        tool = prax_action[0];
                        new_action = actions;
                        aux_subgoal = split(actions[j+4],' ');
                        for ( int u = 0; u < aux_subgoal.size(); ++u){
                            if (aux_subgoal[u].find("_ALL") != std::string::npos){
                                temp_str = new_action[j+4];
                                while (true) {
                                    if (temp_str.find("_obj") != std::string::npos){
                                        temp_str.replace(temp_str.find("_obj"),4,obj);
                                    }
                                    else {
                                        break;
                                    }
                                }
                                while (true) {
                                    if (temp_str.find("_tool") != std::string::npos){
                                        temp_str.replace(temp_str.find("_tool"),5,tool);
                                    }
                                    else {
                                        break;
                                    }
                                }
                                while (true) {
                                    if (temp_str.find("_hand") != std::string::npos){
                                        temp_str.replace(temp_str.find("_hand"),5,"left");
                                    }
                                    else {
                                        break;
                                    }
                                }
                                cout << "temp rule:" << temp_str << endl;
                                temp_rule = split(temp_str,' ');
                                temp_rule.erase(temp_rule.begin(), temp_rule.begin()+1);
                                for (int k = 0; k< translat.size(); ++k){
                                    temp_str = aux_subgoal[u];
                                    while (true) {
                                        if (temp_str.find("_obj") != std::string::npos){
                                            temp_str.replace(temp_str.find("_obj"),4,obj);
                                        }
                                        else {
                                            break;
                                        }
                                    }
                                    while (true) {
                                        if (temp_str.find("_tool") != std::string::npos){
                                            temp_str.replace(temp_str.find("_tool"),5,tool);
                                        }
                                        else {
                                            break;
                                        }
                                    }
                                    
                                    if (temp_str.find(translat[k][1]) == std::string::npos){
                                        while (true) {
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
                        for (int h = new_temp_rule.size(); h>0; --h){
                            if (new_temp_rule[h].find("_ALL") != std::string::npos){
                                new_temp_rule.erase(new_temp_rule.begin()+h);
                            }
                        }
                        new_temp_rule.erase(new_temp_rule.begin(),new_temp_rule.begin()+1);
                        aux_subgoal = new_temp_rule;
                        vector<string> temp_subgoal = subgoals[subgoals.size()-1];
                        for (int i = 0; i < aux_subgoal.size(); ++i){
                            temp_subgoal.push_back(aux_subgoal[i]);
                        }
                        subgoals.push_back(temp_subgoal);
                    }
                    else if (actions[j].find("put_") != std::string::npos){
                        cout << "put detected" << endl;
                        tool = prax_action[0];
                        obj = prax_action[2];
                        temp_str = actions[j+4];
	                while (true) {
                            if (temp_str.find("_obj") != std::string::npos){
                                temp_str.replace(temp_str.find("_obj"),4,obj);
                            }
                            else {
                                break;
                            }
                        }
                        while (true) {
                            if (temp_str.find("_tool") != std::string::npos){
                                temp_str.replace(temp_str.find("_tool"),5,tool);
                            }
                            else {
                                break;
                            }
                        }
                        while (true) {
                            if (temp_str.find("_hand") != std::string::npos){
                                temp_str.replace(temp_str.find("_hand"),5,"left");
                            }
                            else {
                                break;
                            }
                        }
                        aux_subgoal = split(temp_str,' ');
                        aux_subgoal.erase(aux_subgoal.begin(),aux_subgoal.begin()+3);

                        vector<string> temp_subgoal = subgoals[subgoals.size()-1];
                        for (int i = 0; i < aux_subgoal.size(); ++i){
                            temp_subgoal.push_back(aux_subgoal[i]);
                        }
                        subgoals.push_back(temp_subgoal);
                    }
            	    else if ((actions[j].find("_obj") != std::string::npos && actions[j].find("_tool") != std::string::npos) || (actions[j].find("_obj") != std::string::npos && actions[j].find("_hand") != std::string::npos)){
                        cout << "action detected" << endl;
                        tool = prax_action[0];
                        obj = prax_action[2];
                        temp_str = actions[j+4];
                        while (true) {
                            if (temp_str.find("_obj") != std::string::npos){
                                temp_str.replace(temp_str.find("_obj"),4,obj);
                            }
                            else {
                                break;
                            }
                        }
                        while (true) {
                            if (temp_str.find("_tool") != std::string::npos){
                                temp_str.replace(temp_str.find("_tool"),5,tool);
                            }
                            else {
                                break;
                            }
                        }
                        while (true) {
                            if (temp_str.find("_hand") != std::string::npos){
                                temp_str.replace(temp_str.find("_hand"),5,"left");
                            }
                            else {
                                break;
                            }
                        }
                        cout << temp_str << endl;
                        aux_subgoal = split(temp_str, ' ');
                        for (int m = 0; m < aux_subgoal.size(); ++m){
                            cout << aux_subgoal[m] << endl;
                        }
                        aux_subgoal.erase(aux_subgoal.begin(),aux_subgoal.begin()+3);
                        cout << "subgoals total size:" << subgoals.size() << endl;
                        for (int m = 0; m < subgoals.size(); ++m){
                            cout << "subgoals size:" << subgoals[m].size() << endl;
                            for (int n = 0; n < subgoals[m].size(); ++n){
                                cout << subgoals[m][n] << endl;
                            }
                        }
                        vector<string> temp_subgoal;
                        if (subgoals.size() > 0) {
                            temp_subgoal = subgoals[subgoals.size()-1];
                        }
                        for (int m=0; m<temp_subgoal.size(); ++m){
                            cout << temp_subgoal[m] << endl;
                        }
                        cout << "converting" << endl;
                        for (int i=0; i<aux_subgoal.size(); ++i){
                            cout << aux_subgoal[i] << endl;
                            temp_subgoal.push_back(aux_subgoal[i]);
                        }
                        cout << "temp_subgoal done" << endl;
                        subgoals.push_back(temp_subgoal);
                    }
                    cout << "action translated" << endl;
                    vector<int> index_var;
                    int flag_detect;
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
                    vector<string> temp_goal;
                    for (int y = 0; y<subgoals[subgoals.size()-1].size(); ++y){
                        if (find(index_var.begin(), index_var.end(), y) == index_var.end()){
                            temp_goal.push_back(subgoals[subgoals.size()-1][y]);
                        }
                    }
                    subgoals[subgoals.size()-1] = temp_goal;
                }
            }
        }
        else {
            obj = prax_action[2];
        }
    }
    return true;
}

bool goalCompiler::translate()
{
    if (subgoals.size() == 0)
    {
        cout << "subgoals not compiled yet." << endl;
        return false;
    }
    for (int j = 0; j < translat.size(); ++j){
        for (int h = 0; h< subgoals.size(); ++h){
            for (int l = 0; l <subgoals[h].size(); ++l){
                while (true) {
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
    return true;
}

bool goalCompiler::writeFiles()
{
    goalFile.open(goalFileName.c_str());
    subgoalFile.open(subgoalFileName.c_str());
    if (!goalFile.is_open())
    {
        cout << "unable to open goal file." << endl;
        return false;
    }
    if (!subgoalFile.is_open())
    {
        cout << "unable to open subgoal file." << endl;
        return false;
    }
    for (int i = 0; i < subgoals.size(); ++i){
        for (int j=0; j < subgoals[i].size(); ++j){
            subgoalFile << subgoals[i][j] ;
            if ( j != subgoals[i].size()-1){
                subgoalFile << " ";
            }
        }
        subgoalFile << endl;
    }
    subgoalFile.close();
    for (int i = 0; i<subgoals[subgoals.size()-1].size(); ++i){
        goalFile << subgoals[subgoals.size()-1][i] << " ";
    }
    goalFile.close();
    return true;
}

bool goalCompiler::plannerReply()
{
    if (plannerPort.getInputCount() == 0)
    {
        cout << "planner not connected" << endl;
        return false;
    }    
    Bottle &plannerBottleOut = plannerPort.prepare();
    plannerBottleOut.clear();
    plannerBottleOut.addString("done");
    plannerPort.write();
    return true;
}

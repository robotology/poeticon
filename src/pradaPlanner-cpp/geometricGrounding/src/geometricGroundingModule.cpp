/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2015 VisLab, Institute for Systems and Robotics,
 *                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Alexandre Antunes <aleksander88@gmail.com>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 */

#include "geometricGroundingModule.h"


bool geoGround::configure(ResourceFinder &rf)
{
    // module parameters
    moduleName = rf.check("name", Value("geometricGrounding")).asString();
    PathName = rf.findPath("contexts/"+rf.getContext());
    setName(moduleName.c_str());


    if (PathName==""){
        yError("path to contexts/%s not found", rf.getContext().c_str());
        return false;    
    }
    else {
        yInfo("Context FOUND!");
    }

    openFiles();
    openPorts();
    if (!groundingCycle())
    {
        yError("something went wrong with the module execution");
        return false;
    }
    return true;
}

bool geoGround::groundingCycle()
{
    while (!isStopping())
    {
		yarp::os::Time::delay(0.1);
        if (plannerPort.getInputCount() == 0)
        {
            yWarning("planner not connected");
            yarp::os::Time::delay(5);
        }
        if (plannerCommand() == "update")
        {
            if (!loadObjs())
            {
                yError("failed to load objects");
                if (!plannerReply("fail"))
                {
                    yError("failed to communicate with planner");
                }
                continue;
            }
            if (!loadPreRules())
            {
                yError("failed to load pre-rules");
                if (!plannerReply("fail"))
                {
                    yError("failed to communicate with planner");
                }
                continue;
            }
            createRulesList();
            if (!getAffordances())
            {
                yError("failed to get affordances");
                if (!plannerReply("fail"))
                {
                    yError("failed to communicate with planner");
                }
                continue;
            }
            if (!createSymbolList())
            {
                yError("failed to create a symbol list");
                if (!plannerReply("fail"))
                {
                    yError("failed to communicate with planner");
                }
                continue;
            }
            if (!writeFiles())
            {
                yError("failed to write to files");
                if (!plannerReply("fail"))
                {
                    yError("failed to communicate with planner");
                }
                continue;
            }
            if (!plannerReply("ready"))
            {
                yError("failed to communicate with planner");
            }
        }
    }
    return true;
}

bool geoGround::updateModule()
{
    return !isStopping();
}

vector<string> geoGround::create_rules(string pre_rule)
{
    vector<string> temp_vect;
    vector<string> new_rule;
    vector<string> hands;
    int k = 0;
    temp_vect = split(pre_rule, '\n');
    if (find_element(objects,"11") != 0){
        hands.push_back("11");
    }
    if (find_element(objects,"12") != 0){
        hands.push_back("12");
    }
    if (temp_vect.size() >= 2){
        k = 0;
        if ((temp_vect[2].find("_tool") != std::string::npos) || (temp_vect[2].find("_obj") != std::string::npos)){
            if (temp_vect[2].find("_hand") != std::string::npos){
                if (temp_vect[2].find("_tool") != std::string::npos){
                    for (int t = 0; t < hands.size(); ++t){
                        for (int j= 0; j < objects.size(); ++j){
                            for (int c = 0; c < objects.size(); ++c){
                                if ((c != j) && (find_element(hands, objects[c]) == 0) && (find_element(hands, objects[j]) == 0)){
                                    for (int o = 0; o < temp_vect.size(); ++o){
                                        new_rule.push_back(temp_vect[o]);
                                    }
                                    new_rule.push_back("");
                                    new_rule.push_back("");
                                    for (int o = 0; o < temp_vect.size(); ++o){
                                        while (!isStopping()){
                                            if (new_rule[k].find("_obj") != std::string::npos){
                                                new_rule[k].replace(new_rule[k].find("_obj"),4,objects[c]);
                                            }
                                            else {
                                                break;
                                            }
                                        }
                                        while (!isStopping()){
                                            if (new_rule[k].find("_tool") != std::string::npos){
                                                new_rule[k].replace(new_rule[k].find("_tool"),5,objects[j]);
                                            }
                                            else {
                                                break;
                                            }
                                        }
                                        while (!isStopping()){
                                            if (new_rule[k].find("_hand") != std::string::npos){
                                                new_rule[k].replace(new_rule[k].find("_hand"),5,hands[t]);
                                            }
                                            else {
                                                break;
                                            }
                                        }
                                        k++;
                                    }
                                    k=k+2;
                                }
                            }
                        }
                    }
                }
                else {
                    for (int t = 0; t < hands.size(); ++t){
                        for (int c = 0; c < objects.size(); ++c){
                            if (find_element(hands,objects[c]) == 0){
                                for (int o = 0; o < temp_vect.size(); ++o){
                                    new_rule.push_back(temp_vect[o]);
                                }
                                new_rule.push_back("");
                                new_rule.push_back("");
                                for (int o = 0; o < temp_vect.size(); ++o){
                                    while (!isStopping()){
                                        if (new_rule[k].find("_obj") != std::string::npos){
                                            new_rule[k].replace(new_rule[k].find("_obj"),4,objects[c]);
                                        }
                                        else {
                                            break;
                                        }
                                    }
                                    while (!isStopping()){
                                        if (new_rule[k].find("_hand") != std::string::npos){
                                            new_rule[k].replace(new_rule[k].find("_hand"),5,hands[t]);
                                        }
                                        else {
                                            break;
                                        }
                                    }
                                    k++;
                                }
                                k=k+2;
                            }
                        }
                    }
                }
            }
            else {
                for (int j = 0; j < objects.size(); ++j){
                    for (int c = 0; c < objects.size(); ++c){
                        if ((c != j) && ( (find_element(hands,objects[c]) == 0) || (find_element(hands,objects[j]) == 0))){
                            for (int o = 0; o < temp_vect.size(); ++o){
                                new_rule.push_back(temp_vect[o]);
                            }
                            new_rule.push_back("");
                            new_rule.push_back("");
                            for (int o = 0; o < temp_vect.size(); ++o){
                                while (!isStopping()){
                                    if (new_rule[k].find("_obj") != std::string::npos){
                                        new_rule[k].replace(new_rule[k].find("_obj"),4,objects[c]);
                                    }
                                    else {
                                        break;
                                    }
                                }
                                while (!isStopping()){
                                    if (new_rule[k].find("_tool") != std::string::npos){
                                        new_rule[k].replace(new_rule[k].find("_tool"),5,objects[j]);
                                    }
                                    else {
                                        break;
                                    }
                                }
                                k++;
                            }
                            k=k+2;
                        }
                    }
                }
            }
        }
        else {
            for (int o = 0; o < temp_vect.size(); ++o){
                new_rule.push_back(temp_vect[o]);
            }
            new_rule.push_back("");
            new_rule.push_back("");
        }
    }
    vector<string> aux_rule, temp_rule, aux_temp_rule;
    string temp_str, var_find;
    int flag_not_add;
    for (int j = 0; j < new_rule.size(); ++j){
        if (new_rule[j].find("_ALL") != std::string::npos){
            aux_rule = split(new_rule[j],' ');
            for (int u = 0; u < aux_rule.size(); ++u){
                if (aux_rule[u].find("_ALL") != std::string::npos){
                    temp_rule.push_back(" ");
                    for (int k = 0; k < aux_rule.size(); ++k){
                        temp_rule.push_back(aux_rule[k]);
                    }
                    for (int k = 0; k < objects.size(); ++k){
                        if (aux_rule[u].find(objects[k]) == std::string::npos){
                            temp_str = " " + aux_rule[u];
                            while (!isStopping()){
                                if (temp_str.find("_ALL") != std::string::npos){
                                    temp_str.replace(temp_str.find("_ALL"),4,objects[k]);
                                }
                                else {
                                    break;
                                }
                            }
                            temp_rule.push_back(temp_str);
                        }
                    }
                    for (int w = 0; w < temp_rule.size(); ++w){
                        flag_not_add = 0;
                        if (temp_rule[w].find('-') == std::string::npos){
                            var_find = temp_rule[w];
                        }
                        if (temp_rule[w].find('-') != std::string::npos){
                            var_find = temp_rule[w];
                            var_find.replace(var_find.find("-"),1,"");
                        }
                        if (aux_temp_rule.size() > 0){
                            for (int v = 0; v < aux_temp_rule.size(); ++v){
                                if (aux_temp_rule[v].find(var_find) != std::string::npos){
                                    flag_not_add = 1;
                                    break;
                                }
                            }
                        }
                        if (flag_not_add != 1){
                            aux_temp_rule.push_back(temp_rule[w]);
                            
                        }
                    }
                    temp_rule.clear();
                    temp_str = "";
                    for (int w = 0; w < aux_temp_rule.size(); ++w){
                        temp_str = temp_str + aux_temp_rule[w];
                    }
                     
                    new_rule[j] = temp_str;
                }
            }
            for (int h = aux_temp_rule.size()-1; h >= 0; --h){
                if (aux_temp_rule[h].find("_ALL") != std::string::npos){
                    aux_temp_rule.erase(aux_temp_rule.begin()+h);
                }
            }
            temp_str = " ";
            for (int h = 0; h < aux_temp_rule.size(); ++h){
                temp_str = temp_str + aux_temp_rule[h];
            }
            aux_temp_rule.clear();
            new_rule[j] = temp_str;
        }
    }
    for (int k = 0; k < new_rule.size(); ++k){
    }
    return new_rule;
}

vector<string> geoGround::create_symbols(string symbols){
    vector<string> temp_vect;
    vector<string> new_symbol;
    vector<string> hands;
    int k = 0;
    temp_vect = split(symbols, '\n');
    if (find_element(objects,"11") != 0){
        hands.push_back("11");
    }
    if (find_element(objects,"12") != 0){
        hands.push_back("12");
    }
    if (temp_vect[0].find("_obj") != std::string::npos){
        if (temp_vect[0].find("_hand") != std::string::npos){
            k = 0;
            if (temp_vect[0].find("_tool") != std::string::npos){
                for (int g = 0; g < hands.size(); ++g){
                    for (int j = 0; j < objects.size(); ++j){
                        for (int i = 0; i < objects.size(); ++i){
                            if ((i!=j) && (hands[g] != objects[i]) && (hands[g] != objects[j])){
                                for (int o = 0; o < temp_vect.size(); ++o){
                                    new_symbol.push_back(temp_vect[o]);
                                }
                                while (!isStopping()){
                                    if (new_symbol[k].find("_obj") != std::string::npos){
                                        new_symbol[k].replace(new_symbol[k].find("_obj"),4,objects[i]);
                                    }
                                    else {
                                        break;
                                    }
                                }
                                while (!isStopping()){
                                    if (new_symbol[k].find("_tool") != std::string::npos){
                                        new_symbol[k].replace(new_symbol[k].find("_tool"),5,objects[j]);
                                    }
                                    else {
                                        break;
                                    }
                                }
                                while (!isStopping()){
                                    if (new_symbol[k].find("_hand") != std::string::npos){
                                        new_symbol[k].replace(new_symbol[k].find("_hand"),5,hands[g]);
                                    }
                                    else {
                                        break;
                                    }
                                }
                                k++;
                            }
                        }
                    }
                }
            }
            else {
                for (int g = 0; g < hands.size(); ++g){
                    for (int i = 0; i < objects.size(); ++i){
                        if (hands[g] != objects[i]){
                            for (int o = 0; o < temp_vect.size(); ++o){
                                new_symbol.push_back(temp_vect[o]);
                            }
                            while (!isStopping()){
                                if (new_symbol[k].find("_obj") != std::string::npos){
                                    new_symbol[k].replace(new_symbol[k].find("_obj"),4,objects[i]);
                                }
                                else {
                                    break;
                                }
                            }
                            while (!isStopping()){
                                if (new_symbol[k].find("_hand") != std::string::npos){
                                    new_symbol[k].replace(new_symbol[k].find("_hand"),5,hands[g]);
                                }
                                else {
                                    break;
                                }
                            }
                            k++;
                        }
                    }
                }
            }
        }
        else {
            k = 0;
            for (int j = 0; j < objects.size(); ++j){
                for (int i = 0; i < objects.size(); ++i){
                    if (i != j){
                        for (int o = 0; o < temp_vect.size(); ++o){
                            new_symbol.push_back(temp_vect[o]);
                        }
                        while (!isStopping()){
                            if (new_symbol[k].find("_tool") != std::string::npos){
                                new_symbol[k].replace(new_symbol[k].find("_tool"),5,objects[j]);
                            }
                            else {
                                break;
                            }
                        }
                        while (!isStopping()){
                            if (new_symbol[k].find("_obj") != std::string::npos){
                                new_symbol[k].replace(new_symbol[k].find("_obj"),4,objects[i]);
                            }
                            else {
                                break;
                            }
                        }
                        k++;
                    }
                }
            }
        }
    }
    else {
        for (int j=0; j < objects.size(); ++j){
            for (int o = 0; o < temp_vect.size(); ++o){
                new_symbol.push_back(temp_vect[o]);
            }
            while (!isStopping()){
                if (new_symbol[j].find("_hand") != std::string::npos){
                    new_symbol[j].replace(new_symbol[j].find("_hand"),5,objects[j]);
                }
                else {
                    break;
                }
            }
        }
    }
    return new_symbol;
}

void geoGround::openFiles()
{
    presymbolFileName = PathName + "/pre_symbols.dat";
    preruleFileName = PathName + "/pre_rules.dat";
    ruleFileName = PathName + "/rules.dat";
    symbolFileName = PathName + "/symbols.dat";
    objectsFileName = PathName + "/Object_names-IDs.dat";
}

void geoGround::openPorts()
{
	string portName;

	portName = "/" + moduleName + "/planner_cmd:io";
    plannerPort.open(portName);

	portName = "/" + moduleName + "/affordances_cmd:io";
    affordancePort.open(portName);

	portName = "/" + moduleName + "/planner_rpc:o";
	objectQueryPort.open(portName);
}

bool geoGround::close()
{
    yInfo("closing...");
    plannerPort.close();
    affordancePort.close();
    objects.clear();
    tools.clear();
    rules.clear();
    prerules.clear();
    new_rule.clear();
    new_symbols.clear();
    yInfo("vectors cleared");
    return true;
}

bool geoGround::interrupt()
{
    yInfo("interrupting ports");
    plannerPort.interrupt();
    affordancePort.interrupt();
    return true;
}

bool geoGround::loadObjs()
{

	vector<string> temp_vect;
	if (objectQueryPort.getOutputCount() == 0){
        yError("planner not connected!");
        return false;
    }
	objects.clear();
	tools.clear();
    cmd.clear();
    cmd.addString("printObjects");
    objectQueryPort.write(cmd,reply);
    if (reply.size() > 0 && reply.get(0).isList() && reply.get(0).asList()->size() > 2){
        yInfo("Objects updated!");
		for (int i = 0; i < reply.get(0).asList()->size(); ++i)
		{
			temp_vect.clear();
			temp_vect.push_back( NumbertoString(reply.get(0).asList()->get(i).asList()->get(0).asInt() ) );
			temp_vect.push_back(reply.get(0).asList()->get(i).asList()->get(1).asString());
			objects.push_back(temp_vect[0]);
            string check_str=temp_vect[1];
            transform(check_str.begin(), check_str.end(), check_str.begin(), ::tolower);
        	if (check_str == "stick" || check_str == "rake")
        	{
            	tools.push_back(temp_vect[0]);
        	}
		}
		return true;
    }
    else {
        yError("Objects update failed!");
		return false;
    }
	return false;
}

string geoGround::plannerCommand()
{
    string command;
    while (!isStopping()){
		yarp::os::Time::delay(0.1);
        plannerBottle = plannerPort.read(false);
        if (plannerBottle != NULL){
            command = plannerBottle->toString().c_str();
            return command;
        }
    }
    return "stopped";
}

bool geoGround::plannerReply(string replyString)
{
    yInfo("replying to planner");
    if (plannerPort.getInputCount() == 0)
    {
        yError("planner not connected");
        return false;
    }
    Bottle& plannerBottleOut = plannerPort.prepare();
    plannerBottleOut.clear();
    plannerBottleOut.addString(replyString);
    plannerPort.write();
    return true;
}

bool geoGround::loadPreRules()
{
    yInfo("loading prerules");
    string line, temp_str;
    preruleFile.open(preruleFileName.c_str());
    if (!preruleFile.is_open())
    {
        yError("failed to open pre-rules file");
        return false;
    }
    prerules.clear();
    while ( getline(preruleFile, line, '\n')){
        if (line == ""){
            prerules.push_back(temp_str);
            temp_str = "";
        }
        else {
            temp_str += line + '\n';
        }
    }
    preruleFile.close();
    return true;
}

bool geoGround::createRulesList()
{
    vector<string> temp_vect;
    rules.clear();
    yInfo("creating rules");
    for (int i = 0; i < prerules.size(); ++i){
        temp_vect = geoGround::create_rules(prerules[i]);
        for (int g = 0; g < temp_vect.size(); ++g){
            rules.push_back(temp_vect[g]);
        }
    }
    yInfo("rules created");
    return true;
}

bool geoGround::getAffordances()
{
    if (affordancePort.getOutputCount() == 0)
    {
        yError("Affordances module not connected");
        return false;
    }
    string temp_str, bottle_decode_aux;
    vector<string> decode_vect_aux;
    int l = 0;
    int timer_count;
    int num_act = count(rules.begin(), rules.end(), "ACTION:");
    new_rule.clear();
    for (int i = 0; i < rules.size(); ++i){
        if (rules[i].find("Rule") != std::string::npos){
            Bottle& AffBottleOut = affordancePort.prepare();
            AffBottleOut.clear();
            AffBottleOut.addString("update");
            affordancePort.write();
            Time::delay(0.1);
            AffBottleOut.clear();
            temp_str = "Rule #" + static_cast<ostringstream*>( &(ostringstream() << l) )->str() + "  (" + static_cast<ostringstream*>( &(ostringstream() << l+1) )->str() + " out of " + static_cast<ostringstream*>( &(ostringstream() << num_act) )->str() + ")";
            rules[i].replace(rules[i].find("Rule"),21,temp_str);
            new_rule.push_back(rules[i]);
            new_rule.push_back(rules[i+1]);
            new_rule.push_back(rules[i+2]);
            new_rule.push_back(rules[i+3]);
            new_rule.push_back(rules[i+4]);
            new_rule.push_back(rules[i+5]);
            AffBottleOut = affordancePort.prepare();
            AffBottleOut.clear();
            AffBottleOut.addString(rules[i+2]);
            AffBottleOut.addString(rules[i+4]);
            AffBottleOut.addString(rules[i+6]);
            AffBottleOut.addString(rules[i+7]);
            AffBottleOut.addString(rules[i+8]);
            affordancePort.write();
            Time::delay(0.1);
            AffBottleOut.clear();
            timer_count = 0;
            while (!isStopping()) {
                timer_count = timer_count + 1;
                if (timer_count == 600) // 1 minute timeout
                {
                    yError("connection with affordance module closed: timeout");
                    return false;
                }
                AffBottle = affordancePort.read(false);
                if (AffBottle  != NULL){
                    bottle_decode_aux = AffBottle->toString();
                    break;
                }
                if (affordancePort.getOutputCount() == 0)
                {
                    yError("Affordance communication module crashed.");
                    return false;
                }
                Time::delay(0.1);
            }
            decode_vect_aux = split(bottle_decode_aux, '"');
            for (int y = 0; y < decode_vect_aux.size(); ++y){
                while (!isStopping()){
                    if (decode_vect_aux[y].find('"') != std::string::npos){
                        decode_vect_aux[y].erase(decode_vect_aux[y].find('"'));
                    }
                    else {
                        break;
                    }
                }
            }
            for (int b = 0; b < decode_vect_aux.size(); ++b){
                if ((decode_vect_aux[b] != " ") && (decode_vect_aux[b] != "")){
                    new_rule.push_back(decode_vect_aux[b]);
                }
            }
            new_rule.push_back("\n");
            l++;
        }
    }
    Bottle& AffBottleOut = affordancePort.prepare();
    AffBottleOut.clear();
    AffBottleOut.addString("done");
    affordancePort.write();
    return true;
}

bool geoGround::createSymbolList()
{
    string line;
    vector<string> symbols, temp_symbols;
    presymbolFile.open(presymbolFileName.c_str());
    if (!presymbolFile.is_open())
    {
        yError("failed to open pre symbol file");
        return false;
    }
    while (getline(presymbolFile, line, '\n')){
        symbols.push_back(line);
    }
    for (int i = 0; i < symbols.size(); ++i){
        temp_symbols = geoGround::create_symbols(symbols[i]);
        for (int t = 0; t < temp_symbols.size(); ++t){
            if (find_element(new_symbols, temp_symbols[t])==0){
                new_symbols.push_back(temp_symbols[t]);
            }
        }
    }
    presymbolFile.close();
    return true;
}

bool geoGround::writeFiles()
{
    ruleFile.open(ruleFileName.c_str());
    if (!ruleFile.is_open())
    {
        yError("failed to open rule file");
        return false; 
    }
    symbolFile.open(symbolFileName.c_str());
    if (!symbolFile.is_open())
    {
        yError("failed to open symbol file");
        ruleFile.close();
        return false; 
    }
    for (int i = 0; i < new_rule.size(); ++i){
        ruleFile << new_rule[i] << endl;
    }
    for (int i = 0; i < new_symbols.size(); ++i){
        symbolFile << new_symbols[i] + " " << endl;
    }
    ruleFile.close();
    symbolFile.close();
    yInfo("files written");
    return true;
}

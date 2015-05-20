#include "geometricGroundingModule.h"

/*geoGround::geoGround(const string &_moduleName, const string &_PathName):moduleName(_moduleName),PathName(_PathName)
{
}*/

bool geoGround::configure(ResourceFinder &rf)
{
    // module parameters
    moduleName = rf.check("name", Value("geometricGrounding")).asString();
    PathName = rf.findPath("contexts/"+rf.getContext());
    setName(moduleName.c_str());


    if (PathName==""){
        cout << "path to contexts/"+rf.getContext() << " not found" << endl;
        return false;    
    }
    else {
        cout << "Context FOUND!" << endl;
    }

    openFiles();
    openPorts();
    
    while (!isStopping())
    {
        if (plannerCommand() == "update")
        {
            if (!loadObjs())
            {
                cout << "failed to load objects" << endl;
                return false;
            }
            if (!loadPreRules())
            {
                cout << "failed to load pre-rules" << endl;
                return false;
            }
            createRulesList();
            if (!getAffordances())
            {
                cout << "failed to get affordances" << endl;
                return false;
            }
            if (!createSymbolList())
            {
                cout << "failed to create a symbol list" << endl;
                return false;
            }
            if (!writeFiles())
            {
                cout << "failed to write to files" << endl;
                return false;
            }
            if (!plannerReply())
            {
                cout << "failed to communicate with planner" << endl;
                return false;
            }
        }
        Time::delay(5);
    }
    close();
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
            else if ((temp_vect[2].find("push") != std::string::npos) || (temp_vect[2].find("pull") != std::string::npos)){
                for (int j = 0; j < tools.size(); ++j){
                    for (int c = 0; c < objects.size(); ++c){
                        if ((objects[c] != tools[j]) && (find_element(hands, objects[c]) == 0)){
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
                                        new_rule[k].replace(new_rule[k].find("_tool"),5,tools[j]);
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
         cout << new_rule[k] << endl;
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
                            if ((i!=j) && (hands[g] != objects[i]) && (hands[g] != objects[j])/*&& (find_element(hands,objects[i]) == 0) && (find_element(hands,objects[j]) == 0)*/){
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
                        if (/*find_element(hands,objects[i]) == 0*/hands[g] != objects[i]){
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
        else if ((temp_vect[0].find("push_") != std::string::npos) || (temp_vect[0].find("pull_") != std::string::npos)){
            k = 0;
            for (int j = 0; j < tools.size(); ++j){
                for (int i = 0; i < objects.size(); ++i){
                    if ((objects[i] != tools[j]) && (find_element(hands, objects[i]) == 0)){
                        for (int o = 0; o < temp_vect.size(); ++o){
                            new_symbol.push_back(temp_vect[o]);
                        }
                        while (!isStopping()){
                            if (new_symbol[k].find("_tool") != std::string::npos){
                                new_symbol[k].replace(new_symbol[k].find("_tool"),5,tools[j]);
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
    plannerPort.open("/grounding/planner_cmd:io");
    affordancePort.open("/grounding/Aff_cmd:io");
}

bool geoGround::close()
{
    cout << "closing..." << endl;
    plannerPort.close();
    affordancePort.close();
    return true;
}

bool geoGround::interrupt()
{
    cout << "interrupting ports" << endl;
    plannerPort.interrupt();
    affordancePort.interrupt();
    return true;
}

bool geoGround::loadObjs()
{
    string line;
    vector<string> temp_objects, temp_vect;
    cout << objectsFileName << endl;
    objectFile.open(objectsFileName.c_str());
    if (objectFile.is_open()){
        getline(objectFile,line);
        cout << line << endl;
        temp_objects = split(line, ';');
    }
    else {
        cout << "failed to open objects-IDs file" << endl;
        return false;
    }
    objectFile.close();
    objects.clear();
    for (int i = 0; i < temp_objects.size(); ++i){
        temp_vect = split(temp_objects[i],',');
        temp_vect[0].replace(temp_vect[0].find('('),1,"");
        temp_vect[1].replace(temp_vect[1].find(')'),1,"");
        objects.push_back(temp_vect[0]);
        if ((temp_vect[1] == "stick") || (temp_vect[1] == "rake")){
            tools.push_back(temp_vect[0]);
        }
    }
    return true;
}

string geoGround::plannerCommand()
{
    string command;
    if (plannerPort.getInputCount() == 0)
    {
        cout << "planner not connected" << endl;
        return "failed";
    }
    while (!isStopping()){
        plannerBottle = plannerPort.read(false);
        if (plannerBottle != NULL){
            command = plannerBottle->toString().c_str();
            return command;
        }
    }
}

bool geoGround::plannerReply()
{
    if (plannerPort.getInputCount() == 0)
    {
        cout << "planner not connected" << endl;
        return false;
    }
    Bottle& plannerBottleOut = plannerPort.prepare();
    plannerBottleOut.clear();
    plannerBottleOut.addString("ready");
    plannerPort.write();
    return true;
}

bool geoGround::loadPreRules()
{
    cout << "loading prerules" << endl;
    string line, temp_str;
    preruleFile.open(preruleFileName.c_str());
    if (!preruleFile.is_open())
    {
        cout << "failed to open pre-rules file" << endl;
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
    cout << "creating rules" << endl;
    for (int i = 0; i < prerules.size(); ++i){
        cout << prerules[i] << endl;
        temp_vect = geoGround::create_rules(prerules[i]);
        for (int g = 0; g < temp_vect.size(); ++g){
            rules.push_back(temp_vect[g]);
        }
    }
    return true;
}

bool geoGround::getAffordances()
{
    if (affordancePort.getOutputCount() == 0)
    {
        cout << "Affordances module not connected" << endl;
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
                    cout << "connection with affordance module closed: timeout" << endl;
                    return false;
                }
                AffBottle = affordancePort.read(false);
                if (AffBottle  != NULL){
                    bottle_decode_aux = AffBottle->toString();
                    break;
                }
                if (affordancePort.getOutputCount() == 0)
                {
                    cout << "Affordance communication module crashed." << endl;
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
        cout << "failed to open pre symbol file" << endl;
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
        cout << "failed to open rule file" << endl;
        return false; 
    }
    symbolFile.open(symbolFileName.c_str());
    if (!symbolFile.is_open())
    {
        cout << "failed to open symbol file" << endl;
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
    return true;
}



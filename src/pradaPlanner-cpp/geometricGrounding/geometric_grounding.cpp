
#include <yarp/os/Network.h>
#include <yarp/os/RFModule.h>
#include <yarp/os/Bottle.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/os/Mutex.h>
#include <yarp/os/RateThread.h>
#include <yarp/os/RpcClient.h>
#include <yarp/os/Time.h>
#include <yarp/os/Vocab.h>
//#include <cstdio>
//#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>
#include <vector>
#include <algorithm>

using namespace yarp::os;
using namespace std;

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

int find_element(vector<string> vect, string elem) {
    int flag_true = 0;
    for (int i = 0; i < vect.size(); ++i){
        if (vect[i] == elem){
            flag_true = 1;
            break;
        }
    }
    return flag_true;
}

vector<string> create_rules(vector<string> objects, string pre_rule, vector<string> tools){
    cout << "creating rule" << endl;
    //cout << pre_rule << endl;
    vector<string> temp_vect;
    vector<string> new_rule;
    vector<string> hands;
    int k = 0;
    temp_vect = split(pre_rule, '\n');
    /*for (int t = 0; t < objects.size(); ++t){
        cout << objects[t] << endl;
    }*/
    if (find_element(objects,"11") != 0){
        hands.push_back("11");
    }
    if (find_element(objects,"12") != 0){
        hands.push_back("12");
    }
    /*for (int t = 0; t < hands.size(); ++t ){
        cout << hands[t] << endl;
    }*/
    if (temp_vect.size() >= 2){
        k = 0;
        //cout << "vector has size" << endl;
        //cout << temp_vect[2] << endl;
        if ((temp_vect[2].find("_tool") != std::string::npos) || (temp_vect[2].find("_obj") != std::string::npos)){
            //cout << "tool or object found" << endl;
            if (temp_vect[2].find("_hand") != std::string::npos){
                //cout << "hand found" << endl;
                if (temp_vect[2].find("_tool") != std::string::npos){
                    //cout << "tool found" << endl;
                    for (int t = 0; t < hands.size(); ++t){
                        for (int j= 0; j < objects.size(); ++j){
                            for (int c = 0; c < objects.size(); ++c){
                                /*cout << objects[c] << endl;
                                cout << objects[j] << endl;*/
                                if ((c != j) && (find_element(hands, objects[c]) == 0) && (find_element(hands, objects[j]) == 0)){
                                    //cout << "objects not hands" << endl;
                                    for (int o = 0; o < temp_vect.size(); ++o){
                                        new_rule.push_back(temp_vect[o]);
                                    }
                                    new_rule.push_back("");
                                    new_rule.push_back("");
                                    for (int o = 0; o < temp_vect.size(); ++o){
                                        while (true){
                                            if (new_rule[k].find("_obj") != std::string::npos){
                                                new_rule[k].replace(new_rule[k].find("_obj"),4,objects[c]);
                                            }
                                            else {
                                                break;
                                            }
                                        }
                                        while (true){
                                            if (new_rule[k].find("_tool") != std::string::npos){
                                                new_rule[k].replace(new_rule[k].find("_tool"),5,objects[j]);
                                            }
                                            else {
                                                break;
                                            }
                                        }
                                        while (true){
                                            if (new_rule[k].find("_hand") != std::string::npos){
                                                new_rule[k].replace(new_rule[k].find("_hand"),5,hands[t]);
                                            }
                                            else {
                                                break;
                                            }
                                        }
                                        //cout << new_rule[k] << endl;
                                        k++;
                                    }
                                    k=k+2;
                                }
                            }
                        }
                    }
                }
                else {
                    //cout << "tool not found" << endl;
                    for (int t = 0; t < hands.size(); ++t){
                        for (int c = 0; c < objects.size(); ++c){
                            if (find_element(hands,objects[c]) == 0){
                                for (int o = 0; o < temp_vect.size(); ++o){
                                    new_rule.push_back(temp_vect[o]);
                                }
                                new_rule.push_back("");
                                new_rule.push_back("");
                                for (int o = 0; o < temp_vect.size(); ++o){
                                    while (true){
                                        if (new_rule[k].find("_obj") != std::string::npos){
                                            new_rule[k].replace(new_rule[k].find("_obj"),4,objects[c]);
                                        }
                                        else {
                                            break;
                                        }
                                    }
                                    while (true){
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
                //cout << "checking for push/pull" << endl;
                for (int j = 0; j < tools.size(); ++j){
                    for (int c = 0; c < objects.size(); ++c){
                        if ((objects[c] != tools[j]) && (find_element(hands, objects[c]) == 0)){
                            for (int o = 0; o < temp_vect.size(); ++o){
                                new_rule.push_back(temp_vect[o]);
                            }
                            new_rule.push_back("");
                            new_rule.push_back("");
                            for (int o = 0; o < temp_vect.size(); ++o){
                                while (true){
                                    if (new_rule[k].find("_obj") != std::string::npos){
                                        new_rule[k].replace(new_rule[k].find("_obj"),4,objects[c]);
                                    }
                                    else {
                                        break;
                                    }
                                }
                                while (true){
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
                                while (true){
                                    if (new_rule[k].find("_obj") != std::string::npos){
                                        new_rule[k].replace(new_rule[k].find("_obj"),4,objects[c]);
                                    }
                                    else {
                                        break;
                                    }
                                }
                                while (true){
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
            //cout << new_rule[j] << endl;
            aux_rule = split(new_rule[j],' ');
            for (int u = 0; u < aux_rule.size(); ++u){
                //cout << "aux-rule" << aux_rule[u] << endl;
                if (aux_rule[u].find("_ALL") != std::string::npos){
                    temp_rule.push_back(" ");
                    for (int k = 0; k < aux_rule.size(); ++k){
                        temp_rule.push_back(aux_rule[k]);
                    }
                    for (int k = 0; k < objects.size(); ++k){
                        if (aux_rule[u].find(objects[k]) == std::string::npos){
                            temp_str = " " + aux_rule[u];
                            while (true){
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
                    /*for (int w = 0 ; w < temp_rule.size(); ++w){
                         cout << temp_rule[w] << endl;
                    }*/
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
                        //cout << aux_temp_rule[w] << endl;
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

vector<string> create_symbols(vector<string> objects, string symbols, vector<string> tools){
    vector<string> temp_vect;
    vector<string> new_symbol;
    vector<string> hands;
    int k = 0;
    temp_vect = split(symbols, '\n');
    /*cout << symbols << endl;*/
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
                            if ((i!=j) && (find_element(hands,objects[i]) == 0) && (find_element(hands,objects[j]) == 0)){
                                for (int o = 0; o < temp_vect.size(); ++o){
                                    new_symbol.push_back(temp_vect[o]);
                                }
                                while (true){
                                    if (new_symbol[k].find("_obj") != std::string::npos){
                                        new_symbol[k].replace(new_symbol[k].find("_obj"),4,objects[i]);
                                    }
                                    else {
                                        break;
                                    }
                                }
                                while (true){
                                    if (new_symbol[k].find("_tool") != std::string::npos){
                                        new_symbol[k].replace(new_symbol[k].find("_tool"),5,objects[j]);
                                    }
                                    else {
                                        break;
                                    }
                                }
                                while (true){
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
                        if (find_element(hands,objects[i]) == 0){
                            for (int o = 0; o < temp_vect.size(); ++o){
                                new_symbol.push_back(temp_vect[o]);
                            }
                            while (true){
                                if (new_symbol[k].find("_obj") != std::string::npos){
                                    new_symbol[k].replace(new_symbol[k].find("_obj"),4,objects[i]);
                                }
                                else {
                                    break;
                                }
                            }
                            while (true){
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
                        while (true){
                            if (new_symbol[k].find("_tool") != std::string::npos){
                                new_symbol[k].replace(new_symbol[k].find("_tool"),5,tools[j]);
                            }
                            else {
                                break;
                            }
                        }
                        while (true){
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
                        while (true){
                            if (new_symbol[k].find("_tool") != std::string::npos){
                                new_symbol[k].replace(new_symbol[k].find("_tool"),5,objects[j]);
                            }
                            else {
                                break;
                            }
                        }
                        while (true){
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
            while (true){
                if (new_symbol[j].find("_hand") != std::string::npos){
                    new_symbol[j].replace(new_symbol[j].find("_hand"),5,objects[j]);
                }
                else {
                    break;
                }
            }
        }
    }
    /*for (int i  = 0; i < new_symbol.size(); ++i){
        cout << new_symbol[i] << endl;
    }*/
    return new_symbol;
}

int main (int argc, char *argv[]){
    
    Network yarp;

    if(! yarp.checkNetwork() ) {
        fprintf(stdout,"Error: yarp server does not seem available\n");
        return -1;
    }

    ResourceFinder rf;
    rf.setVerbose(true);
    rf.setDefaultContext("poeticon");
    rf.configure(argc, argv);

    Bottle *plannerBottle, *AffBottle;
    BufferedPort<Bottle> plannerPort;
    BufferedPort<Bottle> affordancePort;
    plannerPort.open("/grounding/planner_cmd:io");
    affordancePort.open("/grounding/Aff_cmd:io");

    ifstream preruleFile, presymbolFile;
    
    string presymbolFileName, preruleFileName;
    string preruleName = "/pre_rules.dat";
    string presymbolName = "/pre_symbols.dat";
    string path = rf.findPath("contexts/"+rf.getContext());
    string line, command;


    if (path==""){
        cout << "path to contexts/"+rf.getContext() << " not found" << endl;
        return false;    
    }
    else {
        cout << "Context FOUND!" << endl;
        presymbolFileName = path+presymbolName;
        preruleFileName = path+preruleName;
        cout << presymbolFileName << ", " << preruleFileName << endl;
    }
    ifstream objectFile;
    ofstream ruleFile, symbolFile;
    string ruleFileName = (path + "/rules.dat").c_str();
    string symbolFileName = (path + "/symbols.dat").c_str();
    string objectsFileName = (path + "/Object_names-IDs.dat").c_str();

    while (true) {
        command = "";
        while (true){
            plannerBottle = plannerPort.read(false);
            if (plannerBottle != NULL){
                command = plannerBottle->toString().c_str();
                break;
            }
        }
        if (command == "update"){
            cout << "starting" << endl;



            ruleFile.open(ruleFileName.c_str());
            objectFile.open(objectsFileName.c_str());
            symbolFile.open(symbolFileName.c_str());
            preruleFile.open(preruleFileName.c_str());
            presymbolFile.open(presymbolFileName.c_str());
            vector<string> objects;
            string line;
            if (objectFile.is_open()){
                getline(objectFile,line);
                objects = split(line, ';');
            }
            objectFile.close();
            vector<string> new_obj, aux_obj, tools;
            for (int i = 0; i < objects.size(); ++i){
                aux_obj = split(objects[i],',');
                aux_obj[0].replace(aux_obj[0].find('('),1,"");
                aux_obj[1].replace(aux_obj[1].find(')'),1,"");
                //cout << aux_obj[0] << endl;
                new_obj.push_back(aux_obj[0]);
                if ((aux_obj[1] == "stick") || (aux_obj[1] == "rake")){
                    tools.push_back(aux_obj[0]);
                }
            }
            cout << "files initialised" << endl;
            
            vector<string> prerules, rules, aux_rule, new_rule;
            string temp_str = "";
            while ( getline(preruleFile, line, '\n')){
                /*cout << line << endl;*/
                if (line == ""){
                    prerules.push_back(temp_str);
                    temp_str = "";
                }
                else {
                    temp_str += line + '\n';
                }
            }
            /*for (int i = 0; i < prerules.size(); ++i){
                cout << prerules[i] << endl;
            }*/
            for (int i = 0; i < prerules.size(); ++i){
                cout << prerules[i] << endl;
                aux_rule = create_rules(new_obj, prerules[i], tools);
                cout << "rule created" << endl;
                for (int g = 0; g < aux_rule.size(); ++g){
                    rules.push_back(aux_rule[g]);
                    cout << aux_rule[g] << endl;
                }
            }
            /*for (int i = 0; i < rules.size(); ++i){
                cout << rules[i] << endl;
            }*/
            cout << "creating rules" << endl;
            temp_str = "";
            int l = 0;
            int num_act = count(rules.begin(), rules.end(), "ACTION:");
            for (int i = 0; i < rules.size(); ++i){
                if (rules[i].find("Rule") != std::string::npos){
                    cout << "sending rule:" << endl;
                    cout << rules[i+2] << endl;
                    Bottle& AffBottleOut = affordancePort.prepare();
                    AffBottleOut.clear();
                    AffBottleOut.addString("update");
                    affordancePort.write();
                    Time::delay(0.1);
                    AffBottleOut.clear();

                    temp_str = "Rule #" + static_cast<ostringstream*>( &(ostringstream() << l) )->str() + "  (" + static_cast<ostringstream*>( &(ostringstream() << l+1) )->str() + " out of " + static_cast<ostringstream*>( &(ostringstream() << num_act) )->str() + ")";
                    /*cout << temp_str << endl;*/
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
                    //cout << AffBottleOut.toString() << endl;
                    Time::delay(0.1);
                    AffBottleOut.clear();
                    //cout << "bottle sent" << endl;
                    string bottle_decode_aux;
                    vector<string> decode_vect_aux;
                    while (true) {
                        AffBottle = affordancePort.read(false);
                        if (AffBottle  != NULL){
                            bottle_decode_aux = AffBottle->toString();
                            break;
                        }
                    }
                    //cout << bottle_decode_aux << endl;
                    decode_vect_aux = split(bottle_decode_aux, '"');
                    for (int y = 0; y < decode_vect_aux.size(); ++y){
                        while (true){
                            if (decode_vect_aux[y].find('"') != std::string::npos){
                                decode_vect_aux[y].erase(decode_vect_aux[y].find('"'));
                            }
                            else {
                                break;
                            }
                        }
                        //cout << decode_vect_aux[y] << endl;
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
            cout << "creating symbols..." << endl;
            vector<string> symbols, temp_symbols, new_symbols;
            while ( getline(presymbolFile, line, '\n')){
                symbols.push_back(line);
            }
            for (int i = 0; i < symbols.size(); ++i){
                temp_symbols = create_symbols(new_obj, symbols[i], tools);
                for (int t = 0; t < temp_symbols.size(); ++t){
                    if (find_element(new_symbols, temp_symbols[t])==0){
                        new_symbols.push_back(temp_symbols[t]);
                    }
                }
            }
            cout << "writing files..." << endl;
            for (int i = 0; i < new_rule.size(); ++i){
                ruleFile << new_rule[i] << endl;
            }
            for (int i = 0; i < new_symbols.size(); ++i){
                symbolFile << new_symbols[i] + " " << endl;
            }
            ruleFile.close();
            symbolFile.close();
            preruleFile.close();
            presymbolFile.close();
            Bottle& plannerBottleOut = plannerPort.prepare();
            plannerBottleOut.clear();
            plannerBottleOut.addString("ready");
            plannerPort.write();
        }
        if (command == "kill"){
            Bottle& AffBottleOut = affordancePort.prepare();
            AffBottleOut.clear();
            AffBottleOut.addString("kill");
            affordancePort.write();
            break;
        }
    }
    return 0;
}

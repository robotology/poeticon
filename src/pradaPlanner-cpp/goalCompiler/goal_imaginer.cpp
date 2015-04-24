
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

int main (int argc, char *argv[])
{
    Network yarp;

    if(! yarp.checkNetwork() ) {
        fprintf(stdout,"Error: yarp server does not seem available\n");
        return -1;
    }

    ResourceFinder rf;
    rf.setVerbose(true);
    rf.setDefaultContext("poeticon");
    rf.configure(argc, argv);

    Bottle *plannerBottle, *praxiconBottle;
    BufferedPort<Bottle> plannerPort;
    BufferedPort<Bottle> praxiconPort;
    plannerPort.open("/goal_imag/planner_cmd:io");
    praxiconPort.open("/goal_imag/prax_inst:i");

    ofstream goalFile, subgoalFile;
    ifstream preRuleFile, objectFile;

    vector<vector<string> > instructions;
    vector<string> new_temp_rule;
    vector<vector<string> > subgoals;
    string temp_str;


    string goalFileName, subgoalFileName, preRuleFileName, objIDsFileName;

    string objIdsName ="/Object_names-IDs.dat";
    string goalName ="/goal.dat";
    string subgoalsName = "/subgoals.dat";
    string preRuleName = "/pre_rules.dat";
    string path = rf.findPath("contexts/"+rf.getContext());
    string line, command;
    if (path==""){
        cout << "path to contexts/"+rf.getContext() << " not found" << endl;
        return false;    
    }
    else {
        cout << "Context FOUND!" << endl;
        goalFileName = path+goalName;
        subgoalFileName = path+subgoalsName;
        preRuleFileName = path+preRuleName;
        objIDsFileName = path+objIdsName;
        cout << objIDsFileName << ", " << goalFileName<< ", " << subgoalFileName << ", " << preRuleFileName << endl;
    }

    while (true){
        command = "";
        while (true){
            plannerBottle = plannerPort.read(false);
            if (plannerBottle != NULL){
                command = plannerBottle->toString().c_str();
                break;
            }
        }
        if (command == "praxicon"){
            while (true){
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
                        break;
                    }
                }
            }
        }
        if (command == "update"){
            cout << "starting" << endl;
            goalFile.open(goalFileName.c_str());
            objectFile.open(objIDsFileName.c_str());
            subgoalFile.open(subgoalFileName.c_str());
            preRuleFile.open(preRuleFileName.c_str());
            vector<string> objects;
            if (objectFile.is_open()){
                getline(objectFile,line);
                objects = split(line, ';');
            }
            vector<vector<string> > translat;
            vector<string> temp_trans;
            string temp_str;
            cout << "files initialised" << endl;
            for (int j = 0; j < objects.size(); ++j){
                temp_str = objects[j];
                cout << temp_str << endl;
                temp_str.replace(temp_str.find("("), 1,"");
                cout << temp_str << endl;
                temp_str.replace(temp_str.find(")"), 1,"");
                cout << temp_str << endl;
                temp_trans = split(temp_str,',');
                translat.push_back(temp_trans);
            }
            cout << "translation ready" << endl;
            vector<string> actions;
            while ( getline(preRuleFile, line)){
                actions.push_back(line);
            }
            cout << "actions ready" << endl;
            for (int g = 0; g < instructions.size(); ++g){
                for (int l = 0; l < instructions[g].size(); ++l){
                    cout << instructions[g][l] << endl;
                    if (instructions[g][l].find("hand") != std::string::npos){
                        instructions[g][l].replace(instructions[g][l].find("hand"),4,"left");
                    }
                }
            }
            cout << "instructions obtained" << endl;
            vector<string> aux_subgoal;
            vector<string> temp_vect;
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
            cout << "first instruction processed" << endl;
            vector<string> prax_action;
            vector<string> temp_rule;
            int not_add_flag;
            for (int g = 0; g<instructions[0].size(); ++g){
                prax_action = split(instructions[0][g], ' ');
                prax_action[1].push_back('_');
                if (prax_action[1] != "reach"){
                    cout << "action detected" << endl;
                    for (int j = 0; j < actions.size(); ++j){
                        /*cout << actions[j] << endl;
                        cout << prax_action[1] << endl;*/
                        if (actions[j].find(prax_action[1]) != std::string::npos){
                            string obj = prax_action[2];
                            cout << "action found" << endl;
                            if (actions[j+4].find("_ALL") != std::string::npos){
                                cout << "ALL detected" << endl;
                                string tool = prax_action[0];
                                /*cout << "tool:" << tool << endl;*/
                                vector<string> new_action = actions;
                                aux_subgoal = split(actions[j+4],' ');
                                for ( int u = 0; u < aux_subgoal.size(); ++u){
                                    if (aux_subgoal[u].find("_ALL") != std::string::npos){
                                        temp_str = new_action[j+4];
                                        /*cout << temp_str << endl;*/
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
                                            not_add_flag = 0;
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
                                string tool = prax_action[0];
                                string obj = prax_action[2];
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
                                string tool = prax_action[0];
                                string obj = prax_action[2];
                                /*cout << "object:" << obj << endl;
                                cout << "tool:" << tool << endl;*/
                                temp_str = actions[j+4];
                                /*cout << "effects:" << temp_str << endl;*/
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
                    string obj = prax_action[2];
                }
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
            objectFile.close();
            Bottle &plannerBottleOut = plannerPort.prepare();
            plannerBottleOut.clear();
            plannerBottleOut.addString("done");
            plannerPort.write();
        }
    }            
    return 0;
}

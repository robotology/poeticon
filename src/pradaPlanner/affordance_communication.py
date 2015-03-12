#!/usr/bin/python

# Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
# CopyPolicy: Released under the terms of the GNU GPL v2.0.
# Copyright: (C) 2015 VisLab, Institute for Systems and Robotics,
#                Instituto Superior Tecnico, Universidade de Lisboa, Lisbon, Portugal
# Author: Alexandre Antunes
# CopyPolicy: Released under the terms of the GNU GPL v2.0

##                              Affordances_communication.py                        ##
######################################################################################
##                      Bottles to be received!!                                    ##
##                                                                                  ##
##      1) World+Robot State module                                                 ##
##          - Sends Query with ID                                                   ##
##          - Returns:                                                              ##
##              - Object ID                                                         ##
##              - Object Descriptors                                                ##
##                                                                                  ##
##      2) Affordance Database module                                               ##
##          - Sends Query with Action ID, Object descriptors, tool descriptors      ##
##          - Returns:                                                              ##
##              - A list of all the possible effects, and their probabilities       ##
##                                                                                  ##
##      - Will only work for pushing/pulling                                        ##
##      - all others will have a chance of success of 0.85 + 0.15 noise (for now)   ##
##                                                                                  ##
##      IDs:                                                                        ##
##          11 - left hand                                                          ##
##          12 - right hand                                                         ##
######################################################################################

import yarp
## from multiprocessing import Process, Pipe

class ActionQueryCommunication:
    ## for rpc communication with action executor

    def __init__(self):
        self._rpc_client = yarp.RpcClient()
        self._port_name = "/AffordanceComm/actInt_rpc:o"
        self._rpc_client.open(self._port_name)
##        self._rpc_client.addOutput("/activityInterface/rpc:i") ## need to verify the port!!!!

    def _execute(self, PathName, cmd):
        Objects_file = open(''.join(PathName +"/Object_names-IDs.dat"))
        Object_list = Objects_file.read().split(';')
        Objects_file.close()
        for k in range(len(Object_list)):
            Object_list[k] = Object_list[k].replace('(','').replace(')','').split(',')
        cmd = cmd.split('_')
        act = cmd[0]
        obj = cmd[1]
        hand = cmd[3].replace('()','')
        for k in range(len(Object_list)):
            if str(act) == Object_list[k][0]:
                act = Object_list[k][1]
            if str(obj) == Object_list[k][0]:
                obj = Object_list[k][1]
            if str(hand) == Object_list[k][0]:
                hand = Object_list[k][1].replace('hand','')
        print act, obj, hand
        message = yarp.Bottle()
        message.clear()
        map(message.addString, ["getManip", obj, hand])
        ## this needs to be discussed, what is the syntax?
        ans = yarp.Bottle()
        self._rpc_client.write(message, ans)
        return ans
    def _is_success(self, ans):
            return ans.size() == 1 ## and ans.get(0).asVocab() == 27503

def Affordance_comm():

    mode = 0
    ## set mode  = 0 for no motor engine, 1 for motor engine active 
    yarp.Network.init()
    geo_yarp = yarp.BufferedPortBottle()
    geo_yarp.open("/AffordanceComm/ground_cmd:io")##

    desc_yarp = yarp.BufferedPortBottle()
    desc_yarp.open("/AffordanceComm/desc_query:io")

    planner_yarp = yarp.BufferedPortBottle()
    planner_yarp.open("/AffordanceComm/planner_cmd:io")

    affnet_yarp = yarp. BufferedPortBottle()
    affnet_yarp.open("/AffordanceComm/aff_query:io")

    ActionQuery = ActionQueryCommunication()

    rf = yarp.ResourceFinder()
    rf.setVerbose(True)
    rf.setDefaultContext("poeticon")
    PathName = rf.findPath("contexts/poeticon")

    toolhandle = [[],[]]
    
## waits for instruction from the planner to update
    while 1:
        planner_bottle_in = planner_yarp.read(False)
        yarp.Time.delay(0.1)
        if planner_bottle_in:
            command = planner_bottle_in.toString()
            if command == 'update':
                break

    descriptors = []
    object_file = open(''.join(PathName + "/Object_names-IDs.dat"))
    object_IDs = object_file.read().replace(')','').replace('(','').split(';')
    object_file.close()
    object_IDs.pop(-1)
    Obj_ID = []
    for i in range(len(object_IDs)):
        if object_IDs[i].split(',')[0] != '11' and object_IDs[i].split(',')[0] != '12':
            Obj_ID = Obj_ID + [object_IDs[i].split(',')[0]]
        
    for i in range(len(Obj_ID)):
            
        desc_bottle_out = desc_yarp.prepare()
        desc_bottle_out.clear()
        desc_bottle_out.addString("querydesc2d")
        desc_bottle_out.addInt(int(Obj_ID[i]))
        desc_yarp.write()

        while 1:
            desc_bottle_in = desc_yarp.read(False)
            yarp.Time.delay(0.1)
            print "waiting for reply..."
            if desc_bottle_in:
                data = desc_bottle_in.toString()
                if data != 'ACK':
                    data = desc_bottle_in.get(0).asList()
                    data = data.toString().split(' ')
                    for t in range(len(data)):
                        data[t] = float(data[t])
                    break
        descriptors = descriptors + [[Obj_ID[i], data]]
    tooldesc = []
    tools = []
    for j in range(len(object_IDs)):
        if object_IDs[j].split(',')[1] == 'stick' or object_IDs[j].split(',')[1] == 'rake':
            tools = tools + [object_IDs[j].split(',')[0]]
    for i in range(len(tools)):
        desc_bottle_out = desc_yarp.prepare()
        desc_bottle_out.clear()
        desc_bottle_out.addString("querytooldesc2d")
        desc_bottle_out.addInt(int(tools[i]))
        desc_yarp.write()

        while 1:
            desc_bottle_in = desc_yarp.read(False)
            yarp.Time.delay(0.1)
            print "waiting for reply..."
            if desc_bottle_in:
                data = desc_bottle_in.toString()
                if data != 'ACK':
                    data = desc_bottle_in.toString()
                    data = data.replace('((','').replace('))','').split(') (')
                    data[0] = data[0].split(' ')
                    data[1] = data[1].split(' ')
                    for t in range(len(data)):
                        data[0][t] = float(data[0][t])
                        data[1][t] = float(data[1][t])
                    data[0] = data[0][2:-1]
                    data[1] = data[1][2:-1]
                    break
        tooldesc = tooldesc + [[tools[i], data]]
    translation_file = open(''.join(PathName +"/Action_Affordance_Translation.dat"))
    aux_translation = translation_file.read().split('\n')
    translation = []
    for i in range(len(aux_translation)):
        translation = translation + [aux_translation[i].split(' ')]
    while 1:
        command = ''
        while 1:
            Affor_bottle_in = geo_yarp.read(False)
            yarp.Time.delay(0.1)
            if Affor_bottle_in:
                command = Affor_bottle_in.toString()
                print "updating rule..."
                break
##            planner_bottle_in = planner_yarp.read(False)
##            if planner_bottle_in:
##                data = planner_bottle_in.toString()
##                if data == 'query':
##                    planner_bottle_out = planner_yarp.prepare()
##                    planner_bottle_out.clear()
##                    print '*****************', toolhandle
##                    for k in range(len(toolhandle)):
##                        for y in range(len(toolhandle[k])):
##                            planner_bottle_out.addDouble(toolhandle[k][y])
##                        planner_bottle_out.addList()
##                        print planner_bottle_out
##                    planner_yarp.write()
##                    break
        new_rule = []
                    
                
                    
        if command == 'kill':
            break
        elif command == 'update':
            while 1:
                Affor_bottle_in = geo_yarp.read(False)
                yarp.Time.delay(0.1)
                if Affor_bottle_in:
                    data = Affor_bottle_in.toString()
                    break
            data = data.split(' "')
            for y in range(len(data)):
                data[y] = data[y].replace('"','')
            rule = data[0]
            print rule
            context = data[1]
            outcome = data[2]
            outcome2 = data[3]
            outcome3 = data[4]
            for i in range(len(translation)):
                new_rule = []
                if translation[i][2].replace('\r','') == rule.split('_')[0].replace(' ',''):


## Se for grasp ask ARE/Karma
                    print "checking probabilities"
                    if translation[i][2].replace('\r','').replace(' ','') == 'grasp':
                        print "grasp detected"
                        if mode != 0:
                            print "checking for motor executer"
                            ans = ActionQuery._execute(PathName, rule)
                            if ActionQuery._is_success(ans):
                                probability = ans.get(0).asDouble()
                                new_outcome = outcome.split(' ')
                                new_outcome[2] = str(probability)
                                new_outcome = ' '.join(new_outcome)
                                new_outcome2 = outcome2.split(' ')
                                new_outcome2[2] = str(1-probability)
                                new_outcome2 = ' '.join(new_outcome2)
                                Affor_bottle_out = geo_yarp.prepare()
                                Affor_bottle_out.clear()
                                Affor_bottle_out.addString(new_outcome)
                                Affor_bottle_out.addString(new_outcome2)
                                Affor_bottle_out.addString(outcome3)
                                geo_yarp.write()
                            else:
                                print "failed query, going for default"
                                Affor_bottle_out = geo_yarp.prepare()
                                Affor_bottle_out.clear()
                                Affor_bottle_out.addString(outcome)
                                Affor_bottle_out.addString(outcome2)
                                Affor_bottle_out.addString(outcome3)
                                geo_yarp.write()

                        if mode == 0:
                            print "going for default probability"
                            Affor_bottle_out = geo_yarp.prepare()
                            Affor_bottle_out.clear()
                            Affor_bottle_out.addString(outcome)
                            Affor_bottle_out.addString(outcome2)
                            Affor_bottle_out.addString(outcome3)
                            geo_yarp.write()

## Se for pull/push, -> ask Affordances
                    if translation[i][2].replace('\r','').replace(' ','') == 'push':
                        if affnet_yarp.getOutputCount() == 0:
                            print "no affordance network detected. using default."
                            Affor_bottle_out = geo_yarp.prepare()
                            Affor_bottle_out.clear()
                            Affor_bottle_out.addString(outcome)
                            Affor_bottle_out.addString(outcome2)
                            Affor_bottle_out.addString(outcome3)
                            geo_yarp.write()
                        else:
                            if rule.split('_')[1] != '11' and rule.split('_')[1] != '12' and rule.split('_')[3] != '11' and rule.split('_')[3] != '12':
                                affnet_bottle_out = affnet_yarp.prepare()
                                affnet_bottle_out.clear()
                                obj = rule.split('_')[1]
                                tool = rule.split('_')[3].replace('()','')
                                for o in range(len(descriptors)):
                                    if descriptors[o][0] == obj:
                                        obj_desc = descriptors[o][1]
                                for o in range(len(tooldesc)):
                                    if tooldesc[o][0] == tool:
                                        tool_desc1 = tooldesc[o][1][0]
                                        tool_desc2 = tooldesc[o][1][1]
                                        toolhandle[o] = [tool] + [tooldesc[o]]
                                        toolnum = o
                                message = tool_desc1[1:] + obj_desc[1:] + [2]
                                print '******', message
                                mess_list = []
                                for t in range(len(message)):
                                    mess_num = float(message[t])
                                    mess_list = mess_list + [mess_num]
                                    affnet_bottle_out.addDouble(mess_num)
                                print '*****',affnet_bottle_out.toString()
                                affnet_yarp.write()
                                while 1:
                                    affnet_bottle_in = affnet_yarp.read(False)
                                    yarp.Time.delay(0.1)
                                    if affnet_bottle_in:
                                        data = affnet_bottle_in.toString().replace('((','').replace('))','').split(') (')
                                        for j in range(len(data)):
                                            data[j] = data[j].split(' ')
                                        for g in range(len(data)):
                                            for j in range(len(data[g])):
                                                data[g][j] = float(data[g][j])
                                        break
                                prob_succ1 = 0
                                for g in range(len(data)):
                                    if g > 3:
                                        for j in range(len(data[g])):
                                            prob_succ1 = prob_succ1 + data[g][j]
                                if prob_succ1 == 1.0:
                                    prob_succ1 = 0.95

                                message = tool_desc2[1:] + obj_desc[1:] + [2]
                                affnet_bottle_out = affnet_yarp.prepare()
                                affnet_bottle_out.clear()
                                for t in range(len(message)):
                                    mess_num = float(message[t])
                                    mess_list = mess_list + [mess_num]
                                    affnet_bottle_out.addDouble(mess_num)
                                affnet_yarp.write()
                                while 1:
                                    affnet_bottle_in = affnet_yarp.read(False)
                                    yarp.Time.delay(0.1)
                                    if affnet_bottle_in:
                                        data = affnet_bottle_in.toString().replace('((','').replace('))','').split(') (')
                                        for j in range(len(data)):
                                            data[j] = data[j].split(' ')
                                        for g in range(len(data)):
                                            for j in range(len(data[g])):
                                                data[g][j] = float(data[g][j])
                                        break
                                prob_succ2 = 0
                                for g in range(len(data)):
                                    if g > 3:
                                        for j in range(len(data[g])):
                                            prob_succ2 = prob_succ2 + data[g][j]
                                if prob_succ2 == 1.0:
                                    prob_succ2 = 0.95

                                if prob_succ1 >= prob_succ2:
                                    prob_succ = prob_succ1
                                    toolhandle[toolnum] = toolhandle[toolnum]+['1']
                                else:
                                    prob_succ = prob_succ2
                                    toolhandle[toolnum] = toolhandle[toolnum]+['2']
                                prob_fail = 1 - prob_succ
                                
                                new_outcome = outcome.split(' ')
                                new_outcome[2] = str(prob_succ)
                                new_outcome = ' '.join(new_outcome)
                                new_outcome2 = outcome2.split(' ')
                                new_outcome2[2] = str(prob_fail)
                                new_outcome2 = ' '.join(new_outcome2)
                                Affor_bottle_out = geo_yarp.prepare()
                                Affor_bottle_out.clear()
                                Affor_bottle_out.addString(new_outcome)
                                Affor_bottle_out.addString(new_outcome2)
                                Affor_bottle_out.addString(outcome3)
                                geo_yarp.write()
                            else:
                                Affor_bottle_out = geo_yarp.prepare()
                                Affor_bottle_out.clear()
                                Affor_bottle_out.addString(outcome)
                                Affor_bottle_out.addString(outcome2)
                                Affor_bottle_out.addString(outcome3)
                                geo_yarp.write()
                    if translation[i][2].replace('\r','').replace(' ','') == 'pull':
                        if affnet_yarp.getOutputCount() == 0:
                            print "no affordance network detected. using default."
                            Affor_bottle_out = geo_yarp.prepare()
                            Affor_bottle_out.clear()
                            Affor_bottle_out.addString(outcome)
                            Affor_bottle_out.addString(outcome2)
                            Affor_bottle_out.addString(outcome3)
                            geo_yarp.write()
                        else:
                            if rule.split('_')[1] != '11' and rule.split('_')[1] != '12' and rule.split('_')[3] != '11' and rule.split('_')[3] != '12':
                                affnet_bottle_out = affnet_yarp.prepare()
                                affnet_bottle_out.clear()
                                obj = rule.split('_')[1]
                                tool = rule.split('_')[3].replace('()','')
                                for o in range(len(descriptors)):
                                    if descriptors[o][0] == obj:
                                        obj_desc = descriptors[o][1]
                                for o in range(len(tooldesc)):
                                    if tooldesc[o][0] == tool:
                                        tool_desc1 = tooldesc[o][1][0]
                                        tool_desc2 = tooldesc[o][1][1]
                                        toolhandle[o] = [tool] + [tooldesc[o]]
                                        toolnum = o
                                message = tool_desc1[1:] + obj_desc[1:] + [1]
                                for t in range(len(message)):
                                    mess_num = float(message[t])
                                    mess_list = mess_list + [mess_num]
                                    affnet_bottle_out.addDouble(mess_num)
                                affnet_yarp.write()
                                while 1:
                                    affnet_bottle_in = affnet_yarp.read(False)
                                    yarp.Time.delay(0.1)
                                    if affnet_bottle_in:
                                        data = affnet_bottle_in.toString().replace('((','').replace('))','').split(') (')
                                        for j in range(len(data)):
                                            data[j] = data[j].split(' ')
                                        for g in range(len(data)):
                                            for j in range(len(data[g])):
                                                data[g][j] = float(data[g][j])
                                        break
                                prob_succ1 = 0
                                for g in range(len(data)):
                                    if g < 3:
                                        for j in range(len(data[g])):
                                            prob_succ1 = prob_succ1 + data[g][j]
                                if prob_succ1 == 1.0:
                                    prob_succ1 = 0.95

                                
                                message = tool_desc2[1:] + obj_desc[1:] + [1]
                                affnet_bottle_out = affnet_yarp.prepare()
                                affnet_bottle_out.clear()
                                for t in range(len(message)):
                                    mess_num = float(message[t])
                                    mess_list = mess_list + [mess_num]
                                    affnet_bottle_out.addDouble(mess_num)
                                affnet_yarp.write()
                                while 1:
                                    affnet_bottle_in = affnet_yarp.read(False)
                                    yarp.Time.delay(0.1)
                                    if affnet_bottle_in:
                                        data = affnet_bottle_in.toString().replace('((','').replace('))','').split(') (')
                                        for j in range(len(data)):
                                            data[j] = data[j].split(' ')
                                        for g in range(len(data)):
                                            for j in range(len(data[g])):
                                                data[g][j] = float(data[g][j])
                                        break
                                prob_succ2 = 0
                                for g in range(len(data)):
                                    if g < 3:
                                        for j in range(len(data[g])):
                                            prob_succ2 = prob_succ2 + data[g][j]
                                if prob_succ2 == 1.0:
                                    prob_succ2 = 0.95

                                if prob_succ1 >= prob_succ2:
                                    prob_succ = prob_succ1
                                    toolhandle[toolnum] = toolhandle[toolnum]+['1']
                                else:
                                    prob_succ = prob_succ2
                                    toolhandle[toolnum] = toolhandle[toolnum]+['2']
                                prob_fail = 1 - prob_succ
                                
                                new_outcome = outcome.split(' ')
                                new_outcome[2] = str(prob_succ)
                                new_outcome = ' '.join(new_outcome)
                                new_outcome2 = outcome2.split(' ')
                                new_outcome2[2] = str(prob_fail)
                                new_outcome2 = ' '.join(new_outcome2)
                                Affor_bottle_out = geo_yarp.prepare()
                                Affor_bottle_out.clear()
                                Affor_bottle_out.addString(new_outcome)
                                Affor_bottle_out.addString(new_outcome2)
                                Affor_bottle_out.addString(outcome3)
                                geo_yarp.write()
                            else:
                                Affor_bottle_out = geo_yarp.prepare()
                                Affor_bottle_out.clear()
                                Affor_bottle_out.addString(outcome)
                                Affor_bottle_out.addString(outcome2)
                                Affor_bottle_out.addString(outcome3)
                                geo_yarp.write()
## drop and put keep the "default" probabilities, as we don't have affordances for them yet.

                    if translation[i][2].replace('\r','').replace(' ','') == 'drop':
                        Affor_bottle_out = geo_yarp.prepare()
                        Affor_bottle_out.clear()
                        Affor_bottle_out.addString(outcome)
                        Affor_bottle_out.addString(outcome2)
                        Affor_bottle_out.addString(outcome3)
                        geo_yarp.write()
                        
                    if translation[i][2].replace('\r','').replace(' ','') == 'put':
                        Affor_bottle_out = geo_yarp.prepare()
                        Affor_bottle_out.clear()
                        Affor_bottle_out.addString(outcome)
                        Affor_bottle_out.addString(outcome2)
                        Affor_bottle_out.addString(outcome3)
                        geo_yarp.write()
                    
                    new_rule = new_rule + [outcome]
                    new_rule = new_rule + [outcome2]
                    new_rule = new_rule + [outcome3]
    translation_file.close()

Affordance_comm()

##if __name__ == '__main__':
##    parent_conn, child_conn = Pipe()
##    p = Process(target=Affordance_comm, args = (child_conn,))
##    p.start()
##    parent_conn.send(['update'])
##    print parent_conn.recv()
##    parent_conn.send(['  push_14_with_11()'])
##    print parent_conn.recv()
##    parent_conn.send(['outcome'])
##    print parent_conn.recv()
##    parent_conn.send(['outcome2'])
##    print parent_conn.recv()
##    parent_conn.send(['outcome3'])
##    print parent_conn.recv()
##    final_data = []
##    print parent_conn.recv()
##    parent_conn.send(['kill'])

#!/usr/bin/python

# Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
# CopyPolicy: Released under the terms of the GNU GPL v2.0.
# Copyright: (C) 2015 VisLab, Institute for Systems and Robotics,
#                Instituto Superior Tecnico, Universidade de Lisboa, Lisbon, Portugal
# Author: Alexandre Antunes
# CopyPolicy: Released under the terms of the GNU GPL v2.0

## system imports/includes
from multiprocessing import Pipe,Process
import subprocess
import os
import yarp
import re
import copy

yarp.Network.init()


class worldStateCommunication:
    ## for rpc communication with worldStateManager

    def __init__(self):
        self._rpc_client = yarp.RpcClient()
        self._port_name = "/planner/wsm_rpc:o"
        self._rpc_client.open(self._port_name)
        ## self._rpc_client.addOutput("/wsm/rpc:i") 

    def _execute(self,cmd):
        message = yarp.Bottle()
        message.clear()
        map(message.addString, [cmd])
        ans = yarp.Bottle()
        self._rpc_client.write(message, ans)
        return ans
    def _is_success(self, ans):
        return ans.size() == 1 and ans.get(0).asVocab() == yarp.Vocab.encode("ok")


class ActionExecutorCommunication:
    ## for rpc communication with action executor

    def __init__(self):
        self._rpc_client = yarp.RpcClient()
        self._port_name = "/planner/actInt_rpc:o"
        self._rpc_client.open(self._port_name)
##        self._rpc_client.addOutput("/activityInterface/rpc:i") ## need to verify the port!!!!

    def _execute(self, PathName, cmd, toolhandle):
        Objects_file = open(''.join(PathName +"/Object_names-IDs.dat"))
        Object_list = Objects_file.read().split(';')
        Object_list.pop()
        Objects_file.close()
        for k in range(len(Object_list)):
            Object_list[k] = Object_list[k].replace('(','').replace(')','').split(',')
        cmd = cmd.split('_')
        if 'on' in cmd:
            obj = cmd[1].replace('()','')
            hand = cmd[3]
            act = cmd[0]
            
        else:
            act = cmd[0]
            obj = cmd[1]
            hand = cmd[3].replace('()','')
            tool1 = ''
            tool2 = ''
            for objID in range(len(Object_list)):
                if Object_list[objID][1] == 'rake':
                    tool1 = Object_list[objID][0]
                if Object_list[objID][1] == 'stick':
                    tool2 = Object_list[objID][0]
            if act == 'grasp' and ( obj == tool1 or obj == tool2):
                print 'in'
                for i in range(len(toolhandle)):
                    if str(toolhandle[i]) == (obj):
                        positx = toolhandle[i+1]
                        posity = toolhandle[i+2]
                        ind = i
        for k in range(len(Object_list)):
            if str(act) == Object_list[k][0]:
                act = Object_list[k][1]
            if str(obj) == Object_list[k][0]:
                obj = Object_list[k][1]
            if str(hand) == Object_list[k][0]:
                hand = Object_list[k][1].replace('hand','')
        if act == 'grasp' and (obj == 'rake' or obj == 'stick'):
            act = 'askForTool'
            obj = hand
            hand = ' '.join([positx]+[posity])
        elif act == 'grasp' and (obj != 'rake' and obj != 'stick'):
            act = 'take'
            
            
        print act, obj, hand
        message = yarp.Bottle()
        message.clear()
        map(message.addString, [act, obj, hand])
        ans = yarp.Bottle()
        self._rpc_client.write(message, ans)
        return ans
    def _is_success(self, ans):
        return ans.size() == 1 and ans.get(0).asVocab() == 27503
    
def update_state(PathName):
    print "updating state"
    symbol_file = open(''.join(PathName +"/symbols.dat"))
    symbols = symbol_file.read()
    symbol_file.close()
    data = symbols.split('\n')
    symbols = []
    data.pop()
    for i in range(len(data)):
        aux_data = data[i].split(' ')
        symbols = symbols + [[aux_data[0], aux_data[2]]]
    state_file = open(''.join(PathName +"/state.dat"))
    state = state_file.read()
    state_file.close()
    data = state.replace('-','').replace('()','').replace('\n','')
    data = data.split(' ')
    state = state.replace('\n','').replace(r'(.+?)_touch_(.+?)','')
    for j in range(len(symbols)):
        if symbols[j][0] not in data and symbols[j][1] == 'primitive':
            state = '-'.join((state,''.join((symbols[j][0],'() '))))
    state = ''.join((state,'\n'))
    state_file = open(''.join(PathName +"/state.dat"),'w')
    state_file.write(state)
    state_file.close()
    print "done"
        

def planning_cycle():
    ## mode definition: 1-with praxicon; 2-debug with opc; 3-debug without opc; 4- normal demo 

    mode = 4

    rf = yarp.ResourceFinder()
    rf.setVerbose(True)
    rf.setDefaultContext("poeticon")
    PathName = rf.findPath("contexts/poeticon")
    print(''.join("cd " + PathName +" && " + "./planner.exe"))
    world_rpc = worldStateCommunication()
    motor_rpc = ActionExecutorCommunication()
    world_rpc.__init__()
    motor_rpc.__init__()
    
    geo_yarp = yarp.BufferedPortBottle()##
    geo_yarp.open("/planner/grounding_cmd:io")
    
    goal_yarp = yarp.BufferedPortBottle()##
    goal_yarp.open("/planner/goal_cmd:io")
    
    State_yarp = yarp.BufferedPortBottle()##
    State_yarp.open("/planner/opc_cmd:io")

    prax_yarp_out = yarp.BufferedPortBottle()##
    prax_yarp_out.open("/planner/prax_inst:o")

    Aff_yarp = yarp.BufferedPortBottle()##
    Aff_yarp.open("/planner/Aff_cmd:io")

    instructions = []
    while 1:
        old_state = []
        objects_used = []
        toolhandle = []
        while 1:
            if goal_yarp.getOutputCount() != 0:
                break
        if mode == 1 or mode == 4:
            print 'waiting for praxicon...'
            goal_bottle_out = goal_yarp.prepare()
            goal_bottle_out.clear()
            goal_bottle_out.addString('praxicon')
            goal_yarp.write()
            while 1:
                goal_bottle_in = goal_yarp.read(False)
                if goal_bottle_in:
                    break
                yarp.Time.delay(0.1)
                    
        while 1:
            if State_yarp.getOutputCount() != 0:
                break

    ## cycle that will check if we have any object on the table. if not, it won't continue
        if mode != 3:
            print "opc mode engaged"
            yarp.Time.delay(0.1)
            while 1:
                print "attempting communication"
                if world_rpc._is_success(world_rpc._execute("update")):
                    yarp.Time.delay(0.1)
                    state_flag = 0
                    State_bottle_out = State_yarp.prepare()
                    State_bottle_out.clear()
                    State_bottle_out.addString('update')
                    State_yarp.write()
                
                    while 1:
                        print 'waiting...'
                        State_bottle_in = State_yarp.read(False)
                        if State_bottle_in:
                            Object_file = open(''.join(PathName +"/Object_names-IDs.dat"))
                            Objects = Object_file.read().split(';')
                            if len(Objects) > 3:
                                state_flag = 1
                                break
                            else:
                                print 'number of objects too low, updating'
                                break
                        yarp.Time.delay(1)
                    if state_flag == 1:
                        break
                    yarp.Time.delay(1)
                yarp.Time.delay(1)
            yarp.Time.delay(0.1)
    ###################
        
        print 'state updated'
        Aff_bottle_out = Aff_yarp.prepare()
        Aff_bottle_out.clear()
        Aff_bottle_out.addString('update')
        Aff_yarp.write()
        command = ''
        while 1:
            if goal_yarp.getOutputCount() != 0:
                break
        print 'goal connection done'
##        if mode != 1 and mode != 4:
##            instructions = [['hand','grasp','cheese'],['cheese','reach','bun-bottom'],['hand','put','cheese'],['hand','grasp','bun-top'],['bun-top','reach','cheese'],['hand','put','bun-top']]
##            goal_bottle_out = goal_yarp.prepare()
##            goal_bottle_out.clear()
##            goal_bottle_out.addList()
##            for t in range(len(instructions)):
##                goal_bottle_out.get(t).addString(' '.join(instructions[t]))                    
##            goal_yarp.write()
##        else:
        goal_bottle_out = goal_yarp.prepare()
        goal_bottle_out.clear()
        goal_bottle_out.addString('update')
        goal_yarp.write()
        while 1:
            goal_bottle_in = goal_yarp.read(False)
            print 'waiting...'
            if goal_bottle_in:
                command = goal_bottle_in.toString()
                break
            yarp.Time.delay(1)
        print 'goal is done'
            
##        goal_file = open(''.join(PathName +"/final_goal.dat"))
        subgoalsource_file = open(''.join(PathName +"/subgoals.dat"))
##        goal = goal_file.read().split(' ')
##        goal_file.close()
        plan_level = 0

        subgoals = subgoalsource_file.read().split('\n')
        subgoalsource_file.close()
        aux_subgoals = []
        for t in range(len(subgoals)):
            aux_subgoals = aux_subgoals + [subgoals[t].split(' ')]
        print 'started'
        while 1:
            comm = raw_input('update rules? y/n \n')
            if comm == 'y' or comm == 'n':
                break
            else:
                print 'please write y for yes and n for no'
        if comm == 'y':
            while 1:
                if geo_yarp.getOutputCount() != 0:
                    break
            geo_bottle_out = geo_yarp.prepare()
            geo_bottle_out.clear()
            geo_bottle_out.addString('update')
            geo_yarp.write()
            while 1:
                print 'waiting....'
                geo_bottle_in = geo_yarp.read(False)
                if geo_bottle_in:
                    command = geo_bottle_in.toString()
                if command == 'ready':
                    print 'ready'
                    break
                yarp.Time.delay(1)
        Aff_bottle_out = Aff_yarp.prepare()
        Aff_bottle_out.clear()
        Aff_bottle_out.addString('query')
        Aff_yarp.write() 
        while 1:
            Aff_bottle_in = Aff_yarp.read(False)
            if Aff_bottle_in:
                data = Aff_bottle_in.toString()
                data = data.replace('"','').split(' ')
                print "tool position received:\n", data
                toolhandle = data
                break
            yarp.Time.delay(0.1)
        rules_file = open(''.join(PathName +"/rules.dat"),'r')
        old_rules = rules_file.read().split('\n')
        rules_file.close()
        update_state(PathName)

        config_file = open(''.join(PathName +"/config"),'r')
        config_data = config_file.read().split('\n')
        for w in range(len(config_data)):
            if config_data[w].find('[PRADA]') != -1:
                horizon = 5
                config_data[w+2] = 'PRADA_horizon %d' %horizon
                break
        config_file.close()
        config_file = open(''.join(PathName +"/config"), 'w')
        for w in range(len(config_data)):
            config_file.write(config_data[w])
            config_file.write('\n')
        config_file.close()
        
        Aff_bottle_out = Aff_yarp.prepare()
        Aff_bottle_out.clear()
        Aff_bottle_out.addString('query')
        Aff_yarp.write()

##        while 1:
##            Aff_bottle_in = Aff_yarp.read(False)
##            if Aff_bottle_in:
##                yarp.Time.delay(0.2)
##                toolhandle = Aff_bottle_in.asList().toString()
##                break
##        toolhandle = toolhandle.replace('((','').replace('))','').split(') (')
##        for j in range(len(toolhandle)):
##            toolhandle[j] = toolhandle[j].split(' ')
##        print toolhandle
            
        while(True):
            ## Plan!!!
            
            flag_kill = 0
            cont = 0
            if mode != 3:
                while 1:
                    print "communicating..."
                    state_flag = 0
                    State_bottle_out = State_yarp.prepare()
                    State_bottle_out.clear()
                    State_bottle_out.addString('update')
                    State_yarp.write()
                
                    while 1:
                        print 'waiting...'
                        State_bottle_in = State_yarp.read(False)
                        if State_bottle_in:
                            Object_file = open(''.join(PathName +"/Object_names-IDs.dat"))
                            Objects = Object_file.read().split(';')
                            if len(Objects) > 3:
                                state_flag = 1
                                break
                            else:
                                print 'number of objects too low, updating'
                                break
                        yarp.Time.delay(1)
                    if state_flag == 1:
                        break
                    yarp.Time.delay(1)
            yarp.Time.delay(0.1)
                   
            update_state(PathName)
            
################# function under construction, updating when objects change ################
## requires the geometric grounding to change, to ground object by object
## will take two types of commands: a "complete", to make the full grounding
##                                  an object ID, to update only for that object
##
##
## if len(Objects) > len(old_Objects):
##    print 'new object found, updating...'
##    new_obj = []
##    while h in range(len(Objects)):
##        if Objects[h] not in old_Objects:
##            new_obj = new_obj + [Objects[h].split(',')[0].replace('(','')]
##    geo_bottle_out = geo_yarp.prepare()
##    geo_bottle_out.clear()
##    geo_bottle_out.addString('update ' + new_obj)
##    geo_yarp.write()
##    while 1:
##        print 'waiting....'
##        yarp.Time.delay(1)
##        geo_bottle_in = geo_yarp.read(False)
##        if geo_bottle_in:
##             command = geo_bottle_in.toString()
##             if command == 'ready':
##                 print 'ready'
##                 break
##    old_Objects = Objects
                    
            state_file = open(''.join(PathName +"/state.dat"),'r')
            state = state_file.read().split(' ')
            state[-1] = state[-1].replace('\r','').replace('\n','')
            state_file.close()
            not_to_add = []

            #####################################
            if state == old_state:
                print "state hasn't changed, action failed"
                for t in range(len(rules)):
                    if rules[t].replace(' ','').replace('\n','').replace('\r','') == next_action and next_action != '':
                        p = 0
                        while 1:
                            if rules[t+p] == '':
                                adapt_rules = rules[t+4].split(' ')
                                adapt_rules[2] = str(float(adapt_rules[2])/2)
                                rules[t+4] = ' '.join(adapt_rules)
                                adapt_noise = rules[t+p-1].split(' ')
                                adapt_noise[2] = str(float(adapt_noise[2])+float(adapt_rules[2]))
                                rules[t+p-1] = ' '.join(adapt_noise)
                                break
                            p = p+1
                        rules_file = open(''.join(PathName +"/rules.dat"),'w')
                        for y in range(len(rules)):
                            rules_file.write(rules[y])
                            rules_file.write('\n')
                        rules_file.close()
                        print "rules adapted"
                        break

            subgoal_file = open(''.join(PathName +"/goal.dat"),'w')
            subgoal_file.write(subgoals[plan_level])
            subgoal_file.close()
            if plan_level >= len(subgoals)-1:
                print 'plan finished'
                prax_bottle_out = prax_yarp_out.prepare()
                prax_bottle_out.clear()
                prax_bottle_out.addString('OK')
                for u in range(len(objects_used)):
                    prax_bottle_out.addString(objects_used[u])
                prax_yarp_out.write()
##                geo_bottle_out = geo_yarp.prepare()
##                geo_bottle_out.clear()
##                geo_bottle_out.addString('kill')
##                geo_yarp.write()
##                goal_bottle_out = goal_yarp.prepare()
##                goal_bottle_out.clear()
##                goal_bottle_out.addString('kill')
##                goal_yarp.write()
                break
            print("process-planner.exe")
            print(''.join(PathName + "/planner.exe"))
            planner = subprocess.Popen([''.join(PathName + "/planner.exe")],stdout = subprocess.PIPE, stderr = subprocess.PIPE,cwd = PathName)
            data = planner.communicate()
            
            data = data[0].split('\n')
            rules_file = open(''.join(PathName +"/rules.dat"),'r')
            rules = rules_file.read().split('\n')
            rules_file.close()
            next_action = []
            for t in range(len(data)):
                if data[t] == 'The planner would like to kindly recommend the following action to you:':
                    next_action = data[t+1]
            if next_action == []:
                next_action = data[-2].split(' ')[0]
                if '  %s' %next_action not in rules:
                    config_file = open(''.join(PathName +"/config"),'r')
                    config_data = config_file.read().split('\n')
                    for w in range(len(config_data)):
                        if config_data[w].find('[PRADA]') != -1:
                            temp_var = config_data[w+2].split(' ')
                            horizon = int(temp_var[1])
                            horizon = horizon + 1
                            config_data[w+2] = 'PRADA_horizon %d' %horizon
                            break
                    config_file.close()
                    config_file = open(''.join(PathName +"/config"), 'w')
                    for w in range(len(config_data)):
                        config_file.write(config_data[w])
                        config_file.write('\n')
                    config_file.close()
                    
            print '\nprocessing...\n'
            
            ## processes output of planner
            ## executes next action
            
            subgoal_file = open(''.join(PathName +"/goal.dat"),'r')
            object_file = open(''.join(PathName + "/Object_names-IDs.dat"))
            object_IDs = object_file.read().replace(')','').replace('(','').split(';')
            object_file.close()
            object_IDs.pop()
            for h in range(len(object_IDs)):
                object_IDs[h] = object_IDs[h].split(',')
            goal = subgoal_file.read().split(' ')
            subgoal_file.close()
            prtmess = copy.deepcopy(goal)
            for i in range(len(goal)):
                for t in range(len(object_IDs)):
                    prtmess[i] = prtmess[i].replace(object_IDs[t][0], object_IDs[t][1])
            print 'goals: \n',prtmess
            print '\n goals not met:'
            not_comp_goals = []
            for t in range(len(goal)):
                if goal[t] not in state:
                    prtmess = copy.deepcopy(goal[t])
                    for o in range(len(object_IDs)):
                        prtmess = prtmess.replace(object_IDs[o][0], object_IDs[o][1])
                    not_comp_goals = not_comp_goals + [goal[t]]
                    print prtmess
                    cont = 1
            print '\n'
            raw_input("press enter to continue")
            
            holding_symbols = []
            for t in range(len(aux_subgoals[plan_level-1])):
                if aux_subgoals[plan_level-1][t] in aux_subgoals[plan_level]:
                    holding_symbols = holding_symbols + [aux_subgoals[plan_level-1][t]]
            if holding_symbols != []:
                holding_symbols.pop(-1)
            if plan_level >= 1:
                for t in range(len(holding_symbols)):
                    if holding_symbols[t] not in state:
                        failed_steps = plan_level
                        cont = -1
                        print 'situation changed, receding in plan'
                        break
            if cont == -1:
                plan_level = plan_level-1
                config_file = open(''.join(PathName +"/config"),'r')
                config_data = config_file.read().split('\n')
                for w in range(len(config_data)):
                    if config_data[w].find('[PRADA]') != -1:
                        horizon = 5
                        config_data[w+2] = 'PRADA_horizon %d' %horizon
                        break
                config_file.close()
                config_file = open(''.join(PathName +"/config"), 'w')
                for w in range(len(config_data)):
                    config_file.write(config_data[w])
                    config_file.write('\n')
                config_file.close()
            if cont == 0:
                rules_file = open(''.join(PathName +"/rules.dat"),'w')
                for y in range(len(old_rules)):
                    rules_file.write(old_rules[y])
                    rules_file.write('\n')
                rules_file.close()
                plan_level = plan_level+1
                config_file = open(''.join(PathName +"/config"),'r')
                config_data = config_file.read().split('\n')
                for w in range(len(config_data)):
                    if config_data[w].find('[PRADA]') != -1:
                        horizon = 5
                        config_data[w+2] = 'PRADA_horizon %d' %horizon
                        break
                config_file.close()
                config_file = open(''.join(PathName +"/config"), 'w')
                for w in range(len(config_data)):
                    config_file.write(config_data[w])
                    config_file.write('\n')
                config_file.close()    
                rules_file = open(''.join(PathName +"/rules.dat"),'r')
                rules = rules_file.read().split('\n')
                rules_file.close()
            act_check = '  %s' %next_action
            if act_check in rules:
                objects_used_now = re.findall(r'\d+',next_action)
                for u in range(len(objects_used_now)):
                    if objects_used_now[u] not in toolhandle and objects_used_now[u] not in objects_used:
                        objects_used = objects_used + [objects_used_now[u]]
                prtmess = next_action
                for t in range(len(object_IDs)):
                    prtmess = prtmess.replace(object_IDs[t][0], object_IDs[t][1])
                print 'action to be executed: ', prtmess, '\n'
                motor_rpc._is_success(motor_rpc._execute(PathName, next_action, toolhandle))
                raw_input("press any key")



                old_state = state
                ## send action to motor executor
##                await for reply
##                if reply == 'fail':

                            

    #########################################################################
    ##			Communication with action executor
    ##            ARE_yarp_bottle_out = ARE_yarp.prepare()
    ##            ARE_yarp_bottle_out.clear()
    ##            ARE_yarp_bottle_out.addString(next_action)
    ##            ARE_yarp.write()
    ##            ARE_command = ''
    ##            while 1:
    ##                ARE_yarp_bottle_in = ARE_yarp.read(False)
    ##                if ARE_yarp_bottle_in:
    ##                    ARE_command = ARE_yarp_bottle_in.toString()
    ##                if ARE_command == '[ack]':
    ##                    print 'action confirmed'
    ##                    break
    ##                if ARE_command == '[nack]':
    ##                    print 'action not acknowledged, stopping'
    ##                    flag_kill = 1
    ##                    break
    ##########################################################################
                
                if flag_kill == 1:
                    break

    ##########################################################################
    ##			DEBUG MODE ONLY					##
                if mode == 3:
                    next_state = []
                    for t in range(len(rules)):
                        if rules[t] == '  %s' %next_action:
                            next_state = rules[t+4].split(' ')
                            next_state.pop(2)
                            next_state.pop(1)
                            next_state.pop(0)
                            break
                    for t in range(len(next_state)):
                        if next_state[t].find('-') == 0:
                            selec_state = next_state[t]
                            for b in range(len(state)):
                                if selec_state == state[b]:
                                    not_to_add = not_to_add + [selec_state]
                            selec_state = selec_state.replace('-','',1)
                            for b in range(len(state)):
                                if selec_state == state[b]:
                                    not_to_add = not_to_add + [selec_state]
                        if next_state[t].find('-') == -1:
                            selec_state = next_state[t]
                            for b in range(len(state)):
                                if selec_state == state[b]:
                                    not_to_add = not_to_add + [selec_state]
                            selec_state = '-%s' %next_state[t]
                            for b in range(len(state)):
                                if selec_state == state[b]:
                                    not_to_add = not_to_add + [selec_state]
                    temp_state = []
                    
                    for t in range(len(state)):
                        if state[t] not in not_to_add:
                            temp_state = temp_state + [state[t]]
                    state = temp_state + next_state
                    state = ' '.join(state)
                    state_file = open("state.dat",'w')
                    state_file.write(state)
                    state_file.close()
    ##########################################################################
            
            world_rpc._is_success(world_rpc._execute("update"))
            print 'planning step: ' ,plan_level
            print 'planning horizon: ',horizon
            if horizon > 15:
                print 'cant find a solution, abandoning plan'
                object_file = open(''.join(PathName +"/Object_names-IDs.dat"))
                Objects = object_file.read()
                Objects = Objects.split(';')
                Objects.pop()
                fail_obj_now = []
                for u in range(len(Objects)):
                    Objects[u] = Objects[u].replace('(','').replace(')','').split(',')
                fail_obj = re.findall(r'\d+',' '.join(not_comp_goals + [' ']))
                for u in range(len(fail_obj)):
                    if fail_obj != '11' and fail_obj != '12' and fail_obj not in toolhandle:
                        for t in range(len(Objects)):
                            if fail_obj[u] in Objects[t]:
                                fail_obj_now = fail_obj_now + [Objects[t][1]]
                                break
                prax_bottle_out = prax_yarp_out.prepare()
                prax_bottle_out.clear()
                prax_bottle_out.addString('FAIL')
                for u in range(len(fail_obj_now)):
                    prax_bottle_out.addString(fail_obj_now[u])
                prax_yarp_out.write()
##                geo_bottle_out = geo_yarp.prepare()
##                geo_bottle_out.clear()
##                geo_bottle_out.addString('kill')
##                geo_yarp.write()
##                goal_bottle_out = goal_yarp.prepare()
##                goal_bottle_out.clear()
##                goal_bottle_out.addString('kill')
##                goal_yarp.write()
                break
    if flag_kill == 1:
        geo_bottle_out = geo_yarp.prepare()
        geo_bottle_out.clear()
        geo_bottle_out.addString('kill')
        geo_yarp.write()
        goal_bottle_out = goal_yarp.prepare()
        goal_bottle_out.clear()
        goal_bottle_out.addString('kill')
        goal_yarp.write()

if __name__ == '__main__':
    planning_cycle()    

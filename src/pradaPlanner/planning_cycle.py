#!/usr/bin/python

## system imports/includes
from multiprocessing import Pipe,Process
import subprocess
import os
import yarp

yarp.Network.init()
## function imports/includes
##from geometric_grounding import geometric_grounding, create_rules, create_symbols
##from Affordance_communication import Affordance_comm
##from Goal_Imaginer import goal_imaginer

##import geometric_grounding
##import Affordance_communication
##import Goal_Imaginer

class worldStateCommunication:
    ## for rpc communication with worldStateManager

    def __init__(self):
        self._rpc_client = yarp.RpcClient()
        self._port_name = "/planner/rpc:o"
        self._rpc_client.open(self._port_name)
        self._rpc_client.addOutput("/wsm/rpc:i")

    def execute(self,cmd):
        ans = yarp.Bottle()
        self._rpc_client.write(cmd, ans)
        return ans
    def is_success(self, ans):
        return ans.size() == 1 and ans.get(0).asVocad == 27503
    
def update_state(PathName):
    ##rf = yarp.ResourceFinder()
    ##rf.setVerbose(True)
    ##rf.setDefaultContext("poeticon")
    ##PathName = rf.findPath("contexts/poeticon")
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
        

def planning_cycle():

    rf = yarp.ResourceFinder()
    rf.setVerbose(True)
    rf.setDefaultContext("poeticon")
    PathName = rf.findPath("contexts/poeticon")
    print(''.join("cd " + PathName +" && " + "./planner.exe"))
    world_rpc = worldStateCommunication()
    geo_yarp = yarp.BufferedPortBottle()
    geo_yarp.open("/planner/grounding_cmd:io")
    goal_yarp = yarp.BufferedPortBottle()
    goal_yarp.open("/planner/goal_cmd:io")
    ARE_yarp = yarp.BufferedPortBottle()
    ARE_yarp.open("/planner/are_cmd:io")
    State_yarp = yarp.BufferedPortBottle()
    State_yarp.open("/planner/cmd:io")

    while(1):
        if State_yarp.getOutputCount() != 0:
            break

    State_bottle_out = State_yarp.prepare()
    State_bottle_out.clear()
    State_bottle_out.addString('update')
    State_yarp.write()
    
    while 1:
        print 'waiting...'
        yarp.Time.delay(1)
        State_bottle_in = State_yarp.read(False)
        if State_bottle_in:
            break
    
    print 'state updated'    
    command = ''
    while 1:
        if goal_yarp.getOutputCount() != 0:
            break
    print 'goal connection done'
    goal_bottle_out = goal_yarp.prepare()
    goal_bottle_out.clear()
    goal_bottle_out.addString('start')
    goal_yarp.write()
    while 1:
        goal_bottle_in = goal_yarp.read(False)
        print 'waiting...'
        yarp.Time.delay(1)
        if goal_bottle_in:
            command = goal_bottle_in.toString()
            break
    print 'goal is done'
    
    goal_file = open(''.join(PathName +"/final_goal.dat"))
    subgoalsource_file = open(''.join(PathName +"/subgoals.dat"))
    goal = goal_file.read().split(' ')
    goal_file.close()
    plan_level = 0

    subgoals = subgoalsource_file.read().split('\n')
    subgoalsource_file.close()
    aux_subgoals = []
    for t in range(len(subgoals)):
        aux_subgoals = aux_subgoals + [subgoals[t].split(' ')]
    print 'started'
    comm = raw_input('update rules? y/n')
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
            yarp.Time.delay(1)
            geo_bottle_in = geo_yarp.read(False)
            if geo_bottle_in:
                command = geo_bottle_in.toString()
            if command == 'ready':
                print 'ready'
                break
    update_state(PathName)
    raw_input('press any key')

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
    
    while(True):
        ## Plan!!!
        flag_kill = 0
        cont = 0
        State_bottle_out = State_yarp.prepare()
        State_bottle_out.clear()
        State_bottle_out.addString('update')
        State_yarp.write()
        while 1:
            print 'waiting...'
            yarp.Time.delay(1)
            State_bottle_in = State_yarp.read(False)
            if State_bottle_in:
                print 'state updated'
                break
        update_state(PathName)
        state_file = open(''.join(PathName +"/state.dat"),'r')
        state = state_file.read().split(' ')
        state[-1] = state[-1].replace('\r','').replace('\n','')
        state_file.close()
        not_to_add = []
        subgoal_file = open(''.join(PathName +"/goal.dat"),'w')
        subgoal_file.write(subgoals[plan_level])
        subgoal_file.close()
        if plan_level >= len(subgoals)-1:
            print 'plan finished'
            geo_bottle_out = geo_yarp.prepare()
            geo_bottle_out.clear()
            geo_bottle_out.addString('kill')
            geo_yarp.write()
            goal_bottle_out = goal_yarp.prepare()
            goal_bottle_out.clear()
            goal_bottle_out.addString('kill')
            goal_yarp.write()
            break
        #planner = subprocess.Popen(["./planner.exe"],stdout = subprocess.PIPE, stderr = subprocess.PIPE)
        print("process-planner.exe")
        #planner = subprocess.Popen([''.join("." +PathName +"/planner.exe")],stdout = subprocess.PIPE, stderr = subprocess.PIPE)
        print(''.join(PathName + "/planner.exe"))
        planner = subprocess.Popen([''.join(PathName + "/planner.exe")],stdout = subprocess.PIPE, stderr = subprocess.PIPE,cwd = PathName)
        data = planner.communicate()
        
        data = data[0].split('\n')
        next_action = []
        rules_file = open(''.join(PathName +"/rules.dat"),'r')
        rules = rules_file.read().split('\n')
        rules_file.close()
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

        ## update ws - Vai ser aldrabado por enquanto:
        ## next state = outcome do PRADA
        ## copia este outcome para o ficheiro de world state
        
        subgoal_file = open(''.join(PathName +"/goal.dat"),'r')
        goal = subgoal_file.read().split(' ')
        subgoal_file.close()
        for t in range(len(goal)):
            if goal[t] not in state:
                print goal[t]
                cont = 1
        
        holding_symbols = []
        for t in range(len(aux_subgoals[plan_level-1])):
            if aux_subgoals[plan_level-1][t] in aux_subgoals[plan_level]:
                holding_symbols = holding_symbols + [aux_subgoals[plan_level-1][t]]
        if holding_symbols != []:
            holding_symbols.pop(-1)
        print holding_symbols
        if plan_level >= 1:
            for t in range(len(holding_symbols)):
                if holding_symbols[t] not in state:
                    print holding_symbols[t]
                    cont = -1
                    print 'situation changed, receding in plan'
                    break
        print 'continue:',cont
        raw_input('press enter to continue')
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
        act_check = '  %s' %next_action
        if act_check in rules:
            print 'action to be executed: ', next_action, '\n'
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
            if flag_kill == 1:
                break
##            next_state = []
##            for t in range(len(rules)):
##                if rules[t] == '  %s' %next_action:
##                    next_state = rules[t+4].split(' ')
##                    next_state.pop(2)
##                    next_state.pop(1)
##                    next_state.pop(0)
##                    break
##            for t in range(len(next_state)):
##                if next_state[t].find('-') == 0:
##                    selec_state = next_state[t]
##                    for b in range(len(state)):
##                        if selec_state == state[b]:
##                            not_to_add = not_to_add + [selec_state]
##                    selec_state = selec_state.replace('-','',1)
##                    for b in range(len(state)):
##                        if selec_state == state[b]:
##                            not_to_add = not_to_add + [selec_state]
##                if next_state[t].find('-') == -1:
##                    selec_state = next_state[t]
##                    for b in range(len(state)):
##                        if selec_state == state[b]:
##                            not_to_add = not_to_add + [selec_state]
##                    selec_state = '-%s' %next_state[t]
##                    for b in range(len(state)):
##                        if selec_state == state[b]:
##                            not_to_add = not_to_add + [selec_state]
##            temp_state = []
##            
##            for t in range(len(state)):
##                if state[t] not in not_to_add:
##                    temp_state = temp_state + [state[t]]
##            state = temp_state + next_state
##            state = ' '.join(state)
##            state_file = open("state.dat",'w')
##            state_file.write(state)
##            state_file.close()

            raw_input('press any key')
            
        print 'planning step: ' ,plan_level
        print 'planning horizon: ',horizon
        if horizon > 15:
            print 'cant find a solution, abandoning plan'
            geo_bottle_out = geo_yarp.prepare()
            geo_bottle_out.clear()
            geo_bottle_out.addString('kill')
            geo_yarp.write()
            goal_bottle_out = goal_yarp.prepare()
            goal_bottle_out.clear()
            goal_bottle_out.addString('kill')
            goal_yarp.write()
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

# -*- coding: cp1252 -*-

##                                   Goal Imaginer.py                               ##
######################################################################################
##                      Bottles to be received!!                                    ##
##                                                                                  ##
##      1) World+Robot State module                                                 ##
##          - Sends Query for objects                                               ##
##          - Returns:                                                              ##
##              - Object ID                                                         ##
##              - Object Names                                                      ##
##          - Sends Query for hand clearance                                        ##
##          - Returns:                                                              ##
##              - Hands available, and clear.                                       ##
##                                                                                  ##
##                                                                                  ##
##      - Will create a discrete state                                              ##
##      - Will build the tree using the actions given by the praxicon               ##
##                                                                                  ##
##      IDs:                                                                        ##
##          11 - left hand                                                          ##
##          12 - right hand                                                         ##
######################################################################################
import re
from copy import deepcopy
import yarp

def goal_imaginer():
    yarp.Network.init()
    goal_yarp = yarp.BufferedPortBottle()
    goal_yarp.open("/goal_imag:io")
    while 1:
        goal_bottle_in = goal_yarp.read(True)
        command = goal_bottle_in.toString()
        if command == 'start':
            print 'starting'
            goal_file = open("goal.dat",'w')
            sub_goal_file = open("subgoals.dat",'w')
            actions_file = open("pre_rules.dat")

            ## this file will be replaced by a call to World+Robot State module
            Object_file = open("Object names-IDs.dat")
            Objects = Object_file.read()
            translat = []
            object_list = re.findall(r'\d+',Objects) ## list of objects IDs
            Objects = Objects.split(';')
            Objects.pop()
            for j in range(len(Objects)):
                translat = translat + [Objects[j].replace('(','').replace(')','').split(',')] ## list of objects IDs+numbers

            actions = actions_file.read().split('\n')

            subgoals = [[]]

            ## instructions = bottle from yarp, já defino isso
            ## Assumo que as instruções vêm na forma: ((objecto, acção, objecto),(objecto, acção, objecto))
            ## ou seja, uma lista de listas, cada uma destas a ser uma acção
            ## 
            ## instructions = '((hand,grasp,cheese),(cheese,reach,bun-bottom),(hand,put,cheese),(hand,grasp,salami),(salami,reach,cheese),(hand,put,salami),(hand,grasp,bun-top),(bun-top,reach,salami),(hand,put,bun-top))'
            instructions = '((hand,grasp,cheese),(cheese,reach,bun-bottom),(hand,put,cheese),(hand,grasp,bun-top),(bun-top,reach,cheese),(hand,put,bun-top))'
            instructions = instructions.replace('hand','lefthand')
            ## state = verificar mãos (query hand clearance), verificar objectos (query world state)
            ## objects = object name list
            ## we can create a state from here (replacing _obj with the name)
            instructions = instructions.split('),(')
            first_inst = instructions[0].replace('((','').replace('))','').split(',')
            for i in range(0,len(actions)):
                if actions[i].find("%s_" %first_inst[1]) != -1:
                    aux_subgoal = actions[i+2].replace('_obj','%s' %first_inst[2]).replace('_tool','%s' %first_inst[0]).split(' ')
                    aux_subgoal.pop(1)
                    aux_subgoal.pop(0)
                    subgoals = [aux_subgoal]
                    break
            aux_subgoal = []
            for i in range(len(instructions)):
                prax_action = instructions[i].replace('((','').replace('))','').split(',')
                if prax_action[1] != 'reach':
                    for j in range(0,len(actions)):
                        if actions[j].find("%s_" %prax_action[1]) != -1:
                            if actions[j+4].find('_ALL') != -1:
                                
                                tool = prax_action[0] ## no final, isto não vai ser o nome mas o ID
                                obj = prax_action[2] ## no final, isto não vai ser o nome mas o ID
                                    
                                new_action = deepcopy(actions)
                                aux_subgoal = actions[j+4].split(' ')
                                new_temp_rule = ['']
                                for u in range(len(aux_subgoal)):
                                    if aux_subgoal[u].find('_ALL') != -1:
                                        temp_rule = new_action[j+4].replace('_obj','%s' %obj).replace('_tool','%s' %tool)
                                        if actions[j].find('_hand'):
                                            temp_rule = temp_rule.replace('_hand','lefthand')
                                        temp_rule = temp_rule.split(' ')
                                        for k in range(len(translat)):
                                            if aux_subgoal[u].replace('_obj','%s' %obj).replace('_tool','%s' %tool).find(translat[k][1]) == -1:
                                                temp_rule = temp_rule+[(aux_subgoal[u].replace('_obj','%s' %obj).replace('_tool','%s' %tool).replace('_ALL', translat[k][1]))]
                                        for w in range(len(temp_rule)):
                                            flag_not_add = 0
                                            if temp_rule[w].find('-') != 0:
                                                var_find = temp_rule[w]
                                            if temp_rule[w].find('-') == 0:
                                                var_find = temp_rule[w].replace('-','',1)
                                            for v in range(len(new_temp_rule)):
                                                if new_temp_rule[v].find(var_find) != -1:
                                                    flag_not_add = 1
                                                    break
                                            if flag_not_add != 1:
                                                new_temp_rule = new_temp_rule + [temp_rule[w]]
                                        new_action[j] = ' '.join(new_temp_rule)
                                                
                                for h in range(len(new_temp_rule)-1,-1,-1):
                                    if new_temp_rule[h].find('_ALL') != -1:
                                        new_temp_rule.pop(h)
                                new_temp_rule.pop(1)
                                new_temp_rule.pop(0)
                                aux_subgoal = new_temp_rule
                                subgoals = subgoals + [subgoals[-1] + aux_subgoal]
            ##                    print subgoals[-1], '\nall\n'
                            elif actions[j].find('put_') != -1:
                                tool = prax_action[2] ## substituir com ID
                                tool2 = prax_action[0]
            ##                    print subgoals[-1], '\n\n'
##                                obj = re.findall(r'(.+?)_touch_(.+?) ', ' '.join(subgoals[-1]))
##                                for q in range(len(obj)):
##                                    if obj[q][1] == ''.join(tool+'()'):
##                                        if obj[q][0].find('-')!=0:
##                                            obj = obj[q][1].replace('()','')
##                                            break
##            ##                    print obj
##                                for q in range(len(subgoals[-1])):
##                                    if subgoals[-1][q].find('%s_touch' %tool) != -1 and subgoals[-1][q].find('-') != 0:
##                                        obj = subgoals[-1][q].split('_')[2].replace('()','')
                                aux_subgoal = actions[j+4].replace('_obj','%s' %obj).replace('_tool','%s' %tool)
            ##                    print aux_subgoal
                                if actions[j].find('_hand'):
                                    aux_subgoal = aux_subgoal.replace('_hand','lefthand')
                                aux_subgoal = aux_subgoal.split(' ')
                                aux_subgoal.pop(2)
                                aux_subgoal.pop(1)
                                aux_subgoal.pop(0) ## para retirar a probabilidade
            ##                    print aux_subgoal
                                subgoals = subgoals + [subgoals[-1]+aux_subgoal]
            ##                    print subgoals[-1], '\nput\n'
                            elif actions[j].find('_obj') and actions[j].find('_tool'):
                                tool = prax_action[0] ## no final, isto não vai ser o nome mas o ID
                                obj = prax_action[2] ## no final, isto não vai ser o nome mas o ID
                                aux_subgoal = actions[j+4].replace('_obj','%s' %obj).replace('_tool','%s' %tool)
                                if actions[j].find('_hand'):
                                    aux_subgoal = aux_subgoal.replace('_hand','lefthand')
                                aux_subgoal = aux_subgoal.split(' ')
                                aux_subgoal.pop(2)
                                aux_subgoal.pop(1)
                                aux_subgoal.pop(0) ## para retirar a probabilidade
            ##                    print aux_subgoal
                                subgoals = subgoals + [subgoals[-1]+aux_subgoal]
            ##                    print subgoals[-1], '\nother\n'
            ##                elif actions[j].find('_tool') == -1:
            ##                    obj = prax_action[2]
            ##                    aux_subgoal = actions[j+4].replace('_obj','%s' %obj).replace('_tool','%s' %tool)
            ##                    if actions[j].find('_hand'):
            ##                        aux_subgoal = aux_subgoal.replace('_hand','lefthand')
            ##                    aux_subgoal = aux_subgoal.split(' ')
            ##                    aux_subgoal.pop(2)
            ##                    aux_subgoal.pop(1)
            ##                    aux_subgoal.pop(0) ## para retirar a probabilidade
            ####                    print aux_subgoal
            ##                    subgoals = subgoals + [subgoals[-1]+aux_subgoal]
            ##                    print subgoals[-1], '\ndrop\n'

                                    
                            index_var = []
                            for g in range(len(aux_subgoal)):
                                flag_detect = 0
                                if aux_subgoal[g].find('-') == 0:
                                    temp_var = aux_subgoal[g].replace('-','',1)
                                    for h in range(len(subgoals[-1])):
                                        if subgoals[-1][h] == temp_var:
                                            index_var = index_var + [h]
                                        if subgoals[-1][h] == aux_subgoal[g] and flag_detect == 1:
                                            index_var = index_var + [h]
                                        if subgoals[-1][h] == aux_subgoal[g] and flag_detect == 0:
                                            flag_detect = 1
                                else: 
                                    for h in range(len(subgoals[-1])):
                                        if subgoals[-1][h] == ''.join(['-']+[aux_subgoal[g]]):
                                            index_var = index_var + [h]
                                        if subgoals[-1][h] == aux_subgoal[g] and flag_detect == 1:
                                            index_var = index_var + [h]
                                        if subgoals[-1][h] == aux_subgoal[g] and flag_detect == 0:
                                            flag_detect = 1
                            temp_goal  = []
                            for y in range(len(subgoals[-1])):
                                if y not in index_var:
                                    temp_goal = temp_goal + [subgoals[-1][y]]
                            subgoals[-1] = temp_goal
                else:
                    obj = prax_action[2]
            for j in range(len(translat)):
                for h in range(len(subgoals)):
                    for l in range(len(subgoals[h])):
                        subgoals[h][l] = subgoals[h][l].replace('%s' %translat[j][1],'%s' %translat[j][0])
            for i in range(len(subgoals)):
                for j in range(len(subgoals[i])):
                    sub_goal_file.write(subgoals[i][j])
                    if j != len(subgoals[i])-1:
                        sub_goal_file.write(' ')
                sub_goal_file.write('\n')
            sub_goal_file.close()

            for i in range(len(subgoals[-1])):
                goal_file.write(subgoals[-1][i])
                goal_file.write(' ')
            yarp.Time.delay(0.5)
            goal_file.close()
            goal_bottle_out = goal_yarp.prepare()
            goal_bottle_out.clear()
            goal_bottle_out.addString('done')
            goal_yarp.write(True)
        if command == 'kill':
            break
##
##    for y in range(len(subgoals)):
##        print subgoals[y], '\n\n'

goal_imaginer()
    

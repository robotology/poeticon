#!/usr/bin/python

# Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
# CopyPolicy: Released under the terms of the GNU GPL v2.0.
# Copyright: (C) 2015 VisLab, Institute for Systems and Robotics,
#                Instituto Superior Tecnico, Universidade de Lisboa, Lisbon, Portugal
# Author: Alexandre Antunes
# CopyPolicy: Released under the terms of the GNU GPL v2.0

##                              Geometric Grouding.py                               ##
######################################################################################
##                      Bottles to be received - only on Affordance_comm!!          ##
##                                                                                  ##
##      - Creates a rule for all the objects present on the world                   ##
##      - Creates symbols for all the objects in the world                          ##
##                                                                                  ##
##      - Requires pre_rules and pre_symbols (files)                                ##
##      - Creates a Rules file, and a Symbols file                                  ##
##                                                                                  ##
##                                                                                  ##
##      IDs:                                                                        ##
##          11 - left hand                                                          ##
##          12 - right hand                                                         ##
######################################################################################


import re
import string
import yarp
import time, sys, os
from multiprocessing import Pipe, Process


def create_rules(objects, pre_rules, tools):
    pre_rules = pre_rules.split('\n')
    new_rule = []
    hands = []
    if '11' in objects:
        hands = hands + ['11']
    if '12' in objects:
        hands = hands + ['12']
    if len(pre_rules) >=2:
        k=0
        if pre_rules[3].find('_tool') != -1 or pre_rules[3].find('_obj') != -1:
            if pre_rules[3].find('_hand') != -1:
                if pre_rules[3].find('_tool') != -1:
                    for t in range(len(hands)):
                        for j in range(len(objects)):                        
                            for c in range(len(objects)):
                                if c != j and objects[c] not in hands and objects[j] not in hands:
                                    new_rule = new_rule + pre_rules
                                    for i in range(len(pre_rules)):
                                        new_rule[k] = new_rule[k].replace('_obj', str(objects[c]))
                                        new_rule[k] = new_rule[k].replace('_tool', str(objects[j]))
                                        new_rule[k] = new_rule[k].replace('_hand', str(hands[t]))
                                        k = k+1
                else:
                    for t in range(len(hands)):                       
                        for c in range(len(objects)):
                            if objects[c] not in hands:
                                new_rule = new_rule + pre_rules
                                for i in range(len(pre_rules)):
                                    new_rule[k] = new_rule[k].replace('_obj', str(objects[c]))
                                    new_rule[k] = new_rule[k].replace('_hand', str(hands[t]))
                                    k = k+1
            ## hard coded method to make it run faster
            elif pre_rules[3].find('push') != -1 or pre_rules[3].find('pull') != -1:
                print "checking for push/pull"
                for j in range(len(tools)):
                    for c in range(len(objects)):
                        if objects[c] != tools[j] and objects[c] not in hands:
                            print "working"
                            new_rule = new_rule + pre_rules
                            for i in range(len(pre_rules)):
                                new_rule[k] = new_rule[k].replace('_obj', str(objects[c]))
                                new_rule[k] = new_rule[k].replace('_tool', str(tools[j]))
                                k = k+1
            ##########################################
            else:
                for j in range(len(objects)):
                    for c in range(len(objects)):
                        if (c != j and (objects[c] not in hands or objects[j] not in hands)):
                            new_rule = new_rule + pre_rules
                            for i in range(len(pre_rules)):
                                new_rule[k] = new_rule[k].replace('_obj', str(objects[c]))
                                new_rule[k] = new_rule[k].replace('_tool', str(objects[j]))
                                k = k+1          
        else:
            new_rule = new_rule + pre_rules            
    aux_rule = []
    
    for j in range(len(new_rule)):
        if new_rule[j].find('_ALL') != -1:
            aux_rule = new_rule[j].split(' ')
            for u in range(len(aux_rule)):
                if aux_rule[u].find('_ALL') != -1:
                    temp_rule = new_rule[j].split(' ')
                    for k in range(len(objects)):
                        if aux_rule[u].find(str(objects[k])) == -1:
                            temp_rule = temp_rule+[(aux_rule[u].replace('_ALL', str(objects[k])))]
                    new_temp_rule = ['']
                    for w in range(len(temp_rule)):
                        flag_not_add = 0
                        if temp_rule[w].find('-') == -1:
                            var_find = temp_rule[w]
                        if temp_rule[w].find('-') != -1:
                            var_find = temp_rule[w].replace('-','')
                        for v in range(len(new_temp_rule)):
                            if new_temp_rule[v].find(var_find) != -1:
                                flag_not_add = 1
                                break
                        if flag_not_add != 1:
                            new_temp_rule = new_temp_rule + [temp_rule[w]]
                    new_rule[j] = ' '.join(new_temp_rule)
            for h in range(len(new_temp_rule)-1,-1,-1):
                if new_temp_rule[h].find('_ALL') != -1:
                    new_temp_rule.pop(h)
            new_rule[j] = ''.join([' '] +[' '.join(new_temp_rule)])
                    
            aux_rule = []
    return new_rule

##################################################################################

def create_symbols(objects, symbols, tools):
    symbols = symbols.split('\n')
    new_symbol = []
    hands = []
    if '11' in objects:
        hands = hands + ['11']
    if '12' in objects:
        hands = hands + ['12']
    if symbols[0].find('_obj') != -1:
        if symbols[0].find('_hand') != -1:
            i=0
            if symbols[0].find('_tool') != -1:
                for g in range(len(hands)):
                    for j in range(len(objects)):
                        for k in range(len(objects)):
                            if k!=j and objects[k] not in hands and objects[j] not in hands:
                                new_symbol = new_symbol + symbols
                                new_symbol[i] = new_symbol[i].replace('_tool', str(objects[j]))
                                new_symbol[i] = new_symbol[i].replace('_obj', str(objects[k]))
                                new_symbol[i] = new_symbol[i].replace('_hand', str(hands[g]))
                                i = i+1
            else:
                for g in range(len(hands)):
                    for k in range(len(objects)):
                        if objects[k] not in hands:
                            new_symbol = new_symbol + symbols
                            new_symbol[i] = new_symbol[i].replace('_obj', str(objects[k]))
                            new_symbol[i] = new_symbol[i].replace('_hand', str(hands[g]))
                            i = i+1
        ## method implemented to make the process go faster
        elif symbols[0].find('push') != -1 or symbols[0].find('pull') != -1:
            i=0
            for j in range(len(tools)):
                for k in range(len(objects)):
                    if objects[k] != tools[j] and objects[k] not in hands:
                        new_symbol = new_symbol + symbols
                        new_symbol[i] = new_symbol[i].replace('_tool', str(tools[j]))
                        new_symbol[i] = new_symbol[i].replace('_obj', str(objects[k]))
                        i = i+1
        ###################################################
        else:
            i=0
            for j in range(len(objects)):
                for k in range(len(objects)):
                    if (k!=j):
                        new_symbol = new_symbol + symbols
                        new_symbol[i] = new_symbol[i].replace('_tool', str(objects[j]))
                        new_symbol[i] = new_symbol[i].replace('_obj', str(objects[k]))
                        i = i+1
    else:
        for j in range(len(objects)):
            new_symbol = new_symbol + symbols
            new_symbol[j] = new_symbol[j].replace('_hand', str(objects[j]))
    return new_symbol

#############################################################################

def geometric_grounding():
        yarp.Network.init()
        p = yarp.BufferedPortBottle()
        p.open("/grounding/planner_cmd:io")##
        Affor_yarp = yarp.BufferedPortBottle()
        Affor_yarp.open("/grounding/Aff_cmd:io")##
        rf = yarp.ResourceFinder()
        rf.setVerbose(True)
        rf.setDefaultContext("poeticon")
        PathName = rf.findPath("contexts/poeticon")
        print(PathName)
        right_hand = 11
        left_hand= 12
        prerule_file = open(''.join(PathName + "/pre_rules.dat"))
        presymbol_file = open(''.join(PathName + "/pre_symbols.dat"))



## wait until he receives an instruction:
        while 1:
            command = ''
            while 1:
                bottle_in = p.read(False)
                yarp.Time.delay(0.2)
                if bottle_in:
                    command = bottle_in.toString()
                    break
            print command
            if command == 'update':

## opens files that might have been updated
                rule_file = open(''.join(PathName + "/rules.dat"),'w')
                symbol_file = open(''.join(PathName + "/symbols.dat"),'w')
                newrule_file = open(''.join(PathName + "/new_rules.dat"),'w')
                objects_file = open(''.join(PathName + "/Object_names-IDs.dat"),'r')
                
## reads objects in world
                lines = objects_file.read()
                objects_file.close()
                objects = re.findall(r'\d+',lines)
                temp_obj = []
                print '->',objects
                for i in range(len(objects)):
                    if objects[i] not in temp_obj:
                        temp_obj = temp_obj + [objects[i]]
                objects = temp_obj

## reads tools in the world
                print lines
                temp_data = lines.replace('(','').replace(')','').split(';')  ## problem! it's showing up blank.
                temp_data.pop(-1)
                print temp_data
                tools = []
                for i in range(len(temp_data)):
                    temp_data[i] = temp_data[i].split(',')
                print temp_data
                for i in range(len(temp_data)):
                    if temp_data[i][1] == 'stick' or temp_data[i][1] == 'rake':
                        tools = tools + [temp_data[i][0]]
                print tools
                        
## reads pre_rules
                rules = []
                prerules = prerule_file.read().split('\n\n')
                for i in range(len(prerules)):
                    rules = rules + create_rules(objects, prerules[i], tools)
                

## grounding geometrico
                l = 0
                num_act = rules.count('ACTION:')
                Context_flag = 0
                cumu_prob = 0
                new_rules = []
                print 'creating rules...'
                for i in range(len(rules)):
                    proba = 0
                    ran = rules[i].find('Rule')
                    if ran != -1:
                        print rules[i]
                        print rules[i+1]
                        print rules[i+2]
                        print rules[i+3]
                        print rules[i+4]
                        print rules[i+5]
                        print rules[i+6]
                        print rules[i+7]
                        print rules[i+8]
                        
## Sends instruction to start updating this rule
                        Affor_bottle_out = Affor_yarp.prepare()
                        Affor_bottle_out.clear()
                        Affor_bottle_out.addString('update')
                        Affor_yarp.write()
                        yarp.Time.delay(0.1)
                        Affor_bottle_out.clear()
                        
## Sends info about the rule being updated
                        Affor_bottle_out = Affor_yarp.prepare()
                        rules[i] = rules[i].replace(rules[i][ran:ran+21], 'Rule #%d  (%d out of %d)' % (l,(l+1),num_act))
                        new_rules = new_rules + [rules[i]]+[rules[i+1]]+[rules[i+2]]+[rules[i+3]]+[rules[i+4]]+[rules[i+5]]
                        Affor_bottle_out.addString(rules[i+2])
                        Affor_bottle_out.addString(rules[i+4])
                        Affor_bottle_out.addString(rules[i+6])
                        Affor_bottle_out.addString(rules[i+7])
                        Affor_bottle_out.addString(rules[i+8])
                        Affor_yarp.write()
                        yarp.Time.delay(0.1)
                        Affor_bottle_out.clear()
                        
                        while 1:
                            Affor_bottle_in = Affor_yarp.read(False)
                            yarp.Time.delay(0.1)
                            if Affor_bottle_in:
                                bottle_decode_aux = Affor_bottle_in.toString()
                                break
                            
                        bottle_decode_aux = bottle_decode_aux.split(' "')
                        
                        for y in range(len(bottle_decode_aux)):
                            bottle_decode_aux[y] = bottle_decode_aux[y].replace('"','')
                        aux_rules = bottle_decode_aux
                        
                        for b in range(len(aux_rules)):
                            if aux_rules[b] != '':
                                new_rules = new_rules + [aux_rules[b]]
                        new_rules = new_rules + ['']
                        l = l + 1
## symbol creation
                print 'creating symbols...'
                symbols = presymbol_file.read().split('\n')
                tempsymbols = []
                newsymbols = []
                for i in range(len(symbols)-1):
                    tempsymbols = tempsymbols+create_symbols(objects,symbols[i], tools)
                for i in range(len(tempsymbols)):
                    if tempsymbols[i] not in newsymbols:
                        newsymbols = newsymbols + [tempsymbols[i]]
## writes output files
                print 'writing files...'
                for i in range(len(rules)):
                    newrule_file.write(rules[i])
                    newrule_file.write('\n')
                for i in range(len(new_rules)):
                    rule_file.write(new_rules[i])
                    rule_file.write('\n')
                for i in range(len(newsymbols)):
                    print 'symbol:', newsymbols[i]
                    symbol_file.write(newsymbols[i])
                    symbol_file.write(' ')
                    symbol_file.write('\n')
                rule_file.close()
                newrule_file.close()
                symbol_file.close()
                bout = p.prepare()
                bout.clear()
                bout.addString('ready')
                print 'sending'
                p.write()
            if command == 'kill':
                prerule_file.close()
                presymbol_file.close()
                Affor_bottle_out = Affor_yarp.prepare()
                Affor_bottle_out.addString("kill")
                Affor_yarp.write()
                break

        return objects;

geometric_grounding()
Affor_yarp.close()
## test function here, geo_grounding will be called from main program later
                                                   
##if __name__ == '__main__':
##    parent_conn, child_conn = Pipe()
##    p = Process(target=geometric_grounding, args = (child_conn,))
##    p.start()
##    parent_conn.send(['update'])
##    while 1:
##        if parent_conn.poll()==True:
##            data = parent_conn.recv()
##            if data == ['finished']:
##                parent_conn.send(['kill'])
##                break
##            else:
##                print data
##    print parent_conn.recv()

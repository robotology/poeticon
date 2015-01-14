
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
from multiprocessing import Process, Pipe



def Affordance_comm():
    yarp.Network.init()
    geo_yarp = yarp.BufferedPortBottle()
    geo_yarp.open("/Aff_ground:io")
##    Aff_yarp = yarp.BufferedPortBottle()
##    Aff_yarp.open("/Aff_query:io")
    translation_file = open("Action_Affordance Translation.dat")
    aux_translation = translation_file.read().split('\n')
    translation = []
    for i in range(len(aux_translation)):
        translation = translation + [aux_translation[i].split(' ')]
    while 1:
        command = ''
        while 1:
            Affor_bottle_in = geo_yarp.read()
            if Affor_bottle_in:
                command = Affor_bottle_in.toString()
                break

        new_rule = []
        if command == 'kill':
            break
        elif command == 'update':
            while 1:
                Affor_bottle_in = geo_yarp.read()
                if Affor_bottle_in:
                    data = Affor_bottle_in.toString()
                    break
            data = data.split(' "')
            for y in range(len(data)):
                data[y] = data[y].replace('"','')
            rule = data[0]
            context = data[1]
            outcome = data[2]
            outcome2 = data[3]
            outcome3 = data[4]
            for i in range(len(translation)):
                new_rule = []
                if translation[i][2].replace('\r','') == rule.split('_')[0].replace(' ',''):
## Se for pull/push, -> ask Affordances

## Se for grasp, reach, drop ou put, ask ARE/Karma
## What ports to connect to, what kind of messages to send?

##                    if translation[i][2] == 'push' or translation[i][2] == 'pull':
##                        Query_bottle_out = Aff_query.prepare()
##                        Query_bottle_out.clear()
##                        action = translation[i][0]
##                        tool = command.split('_')[3]
##                        obj = command.split('_')[1]
##                        Query_bottle_out.addString('P(E|%d,%d,%d)' %action %tool %obj)
## probabilidade dos diferentes efeitos, dados a accao, ferramenta e objecto
##                        Aff_query.write()
##                        yarp.time.delay(1)
##                        data = []
##                        while 1:
##                            Query_bottle_in = Aff_query.read(False)
##                            if Query_bottle_in:
##                                temp_data = Query_bottle_in.toString().split(')')
##                                for l in range(len(temp_data)):
##                                    data = data + [temp_data[l].replace("(),","").split(' ')
##                                prob_noth_happens = 0
##                                prob_moving = 0
##                                prob_noise = 0
##                                for k in range(0, len(data),3):
##                                break
##                  If pull, Y=1 ou 2 e "bom", 3, 4 ou 5 e mau (away).
##                  If push, Y=4 ou 5 e "bom", 3, 2 ou 1 e mau (vem para nos).
##                  in either case, o X nao ira interessar
##                  a menos que seja muito grande (noise?)

                    

                    new_rule = new_rule + [outcome]
                    new_rule = new_rule + [outcome2]
                    new_rule = new_rule + [outcome3]
                    Affor_bottle_out = geo_yarp.prepare()
                    Affor_bottle_out.clear()
                    Affor_bottle_out.addString(outcome)
                    Affor_bottle_out.addString(outcome2)
                    Affor_bottle_out.addString(outcome3)
                    geo_yarp.write()
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

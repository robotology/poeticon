#!/usr/bin/env python3

# Giovanni Saponaro <gsaponaro@isr.tecnico.ulisboa.pt>
# 2018

import argparse
import signal
import subprocess
import os
import sys
import time
import xml.etree.ElementTree as ET

# installation:
# sudo apt install python3-colorama
# or
# pip3 install colorama
from colorama import Fore

from lxml import etree

# installation: pip3 install stopit
import stopit

# installation:
# https://github.com/robotology/yarp/issues/1149#issuecomment-355042530
import yarp

def check_process(ps_name, *pars):
    """
    Check if a process is running or not (using subprocess and pgrep search).
    Note: in theory this function could use YARP APIs internally
          ('yarprun --on SERVERPORT --isrunning TAG'), but that would require
          handling yarprun tags properly in the case of two processes with
          the same name but a different tag (disambiguation via tag).

    Arguments:
    ps_name -- process executable name
    pars -- (optional) parameters with which the process was launched. This
            field is necessary to disambiguate when two or more processes
            are running with the same name but different parameters

    Return values:
    found -- boolean indicating whether the process is running or not
    """

    found = False

    if not pars:
        ps_full_string = ps_name
    else:
        ps_full_string = ps_name + ' ' + str(pars[0])

    # example of using subprocess and pgrep:
    # pgrep -a -f objectsPropertiesCollector --name dummyMemory
    #             --context poeticon --db dummy_memory_simple3.ini --no-save-db
    # where -f matches long exec names, https://unix.stackexchange.com/a/267021
    p = subprocess.Popen(['pgrep', '-a', '-f', ps_full_string], stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    out, _ = p.communicate()
    # print('out: {}'.format(out))
    # print('err: {}'.format(err))
    for line in out.decode('utf-8').split('\n'): # note: stdout
        if ps_full_string in line:
            found = True
            break

    return found

def cleaning_routine():
    """
    Clean YARP ports and stop all running modules from the YARP XML application
    file. To be called between two experiments.
    """

    subprocess.run(['yarp', 'clean'],
                   stdout=subprocess.DEVNULL,
                   stderr=subprocess.DEVNULL)
    my_delay(5)
    stop_all()

def connect(from_port, to_port, *protocol):
    """
    Connect two YARP ports (using YARP APIs). If the connection is already
    established, do nothing. Return whether the ports are connected.

    Arguments:
    from_port -- name of the source port
    to_port -- name of the destination port
    protocol -- (optional) YARP network protocol: tcp (default), udp, mcast, etc.

    Return values:
    success -- boolean indicating whether the ports are connected or not
    """

    success = False

    if yarp.Network.isConnected(from_port, to_port):
        # already connected, nothing to do
        success = True
        return success

    if protocol:
        success = yarp.Network.connect(from_port, to_port, str(protocol[0]))
    else:
        success = yarp.Network.connect(from_port, to_port)

    return success

def connect_missing(conns):
    """
    Iterate over all the <connection> tags in the YARP application XML file,
    establish the missing connections (using the connect helper function).
    Does not return a success variable, because we want to permit the absence
    of some connections (e.g., the ones with affordanceCommunicator).
    """

    for c in conns:
        from_port = c.find('from').text
        to_port = c.find('to').text
        has_protocol = c.find('protocol')

        if has_protocol is not None:
            protocol = c.find('protocol').text
            success = connect(from_port, to_port, protocol)
        else:
            success = connect(from_port, to_port)

        if args.v:
            if success:
                print(from_port, to_port, Fore.GREEN, 'success', Fore.RESET)
            else:
                print(from_port, to_port, Fore.RED, 'failure', Fore.RESET)

def initial_setup_and_checks():
    """
    Perform initial checks: YARP network, yarprun server, existence of
    camera port containing the initial visual scenario. If unsuccessful,
    print an error message and quit.
    """

    # open yarp network
    yarp.Network.init()

    # check if required modules/processes are running
    ps_name = 'yarprun'
    if not check_process(ps_name):
        print('Missing ' + ps_name)
        sys.exit()

    # check if required YARP ports exist, exit on failure
    required_ports = [initial_img_port, yarprun_port]
    for p in required_ports:
        if not yarp.NetworkBase.exists(p):
            print('Missing ' + p)
            sys.exit()

def modules_to_start(all_modules):
    """
    Compute a list of modules/processes to run, excluding those that are
    already running, but always including the brittle ones (i.e., modules that
    need to be force-restarted at the beginning of all experiments).

    Arguments:
    all_modules -- XML tree (xml.etree.ElementTree)

    Return values:
    list_of_modules_to_start -- list of modules that need (re-)running
    """

    list_of_modules_to_start = []
    for m in all_modules:
        name = m.find('name').text
        has_params = m.find('parameters')
        if has_params is not None:
            params = m.find('parameters').text
            curr_mod = [name, params]
            is_running = check_process(name, params)
        else:
            curr_mod = [name]
            is_running = check_process(name)

        if not is_running:
            if args.v:
                print(*curr_mod, Fore.RED, 'not running', Fore.RESET)

            list_of_modules_to_start.append(curr_mod)
        else:
            if args.v:
                print(*curr_mod, Fore.GREEN, 'running', Fore.RESET)

            # modules which must be restarted before every experiment
            brittle = ['dummyActivityInterface',
                       'objectsPropertiesCollector',
                       'worldStateManager']

            for r in brittle:
                if name == r:
                    if args.v:
                        print(Fore.YELLOW, 'will restart it anyway', Fore.RESET)

                    list_of_modules_to_start.append(curr_mod)

    return list_of_modules_to_start

def my_delay(secs):
    """
    Sleep for some time.

    Arguments:
    secs -- number of seconds
    """

    # yarp
    #yarp.Time.delay(secs)

    # native Python
    time.sleep(secs)

def parse_cmdline_options():
    """
    Parse the command-line options specified by the user (using argparse).

    Return values:
    parser.parse_args() -- namespace of parameters, see argparse documentation
    """

    parser = argparse.ArgumentParser(description='Simulate the POETICON++ experiment')
    parser.add_argument('-c', '--clean', help='clean up or stop existing modules then quit (default false)', action='store_true')
    parser.add_argument('-d', help='path where the YARP application XML file (yarpmanager format) is located', default='/home/gsaponaro/NOBACKUP/unstable-modules/poeticon-simulation/app/scripts')  # /usr/local/src/robot/poeticon/src/poeticon-simulation/app/scripts
    parser.add_argument('-i', help='YARP application XML filename (yarpmanager format)', type=argparse.FileType('r'), default='sim_complex6_creativity+adaptability.xml')  # sim_simple3_noheur.xml
    parser.add_argument('-n', help='number of experiments to run (default 1)', type=int, default=1)
    parser.add_argument('-s', help='safe mode (asking user confirmation, default false)', action='store_true')
    parser.add_argument('-t', help='timeout in seconds for an experiment (default 600)', type=int, default=600)
    parser.add_argument('-o', help='output file', default='output.txt', type=argparse.FileType('a'))
    parser.add_argument('-v', help='print verbose output (default false)', action='store_true')

    return parser.parse_args()

def parse_yarp_application_xml():
    """
    Parse a YARP application XML file (yarpmanager format).

    Return values:
    tree.getroot -- XML tree (xml.etree.ElementTree)
    """

    xml_fullpath = os.path.join(args.d, args.i.name)
    print('Application XML:', xml_fullpath)

    my_parser = etree.XMLParser() # etree parser has XPath support, unlike ET
    tree = ET.parse(xml_fullpath, parser=my_parser)

    return tree.getroot()

def run(ps_name, *pars):
    """
    Launch a process (using yarprun).

    Arguments:
    ps_name -- process executable name
    pars -- (optional) parameters of the process

    Return values:
    success -- boolean indicating whether the process was started or not
    """

    if not pars:
        ps_full_string = ps_name
    else:
        ps_full_string = ps_name + ' ' + str(pars[0])

    args = ['yarprun', '--on', yarprun_port, '--as', ps_name, '--cmd', ps_full_string]

    p = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    _, err = p.communicate()
    # print('out: {}'.format(out))
    # print('err: {}'.format(err))

    # output on success:
    # RESPONSE:
    # =========
    # 9112
    # STARTED: server=/enigma alias=lbpExtract cmd=lbpExtract pid=9112

    # output when executable is invalid:
    # RESPONSE:
    # =========
    # -1
    # ABORTED: server=/enigma alias=lbpExtract5 cmd=lbpExtract5
    # Can't execute command because No such file or directory

    success = False
    for line in err.decode('utf-8').split('\n'): # note: stderr
        if 'STARTED:' in line:
            success = True
            break
    return success

def run_many(modules):
    """
    Launch many processes (using the run helper function).

    Arguments:
    modules -- list of modules, each of them optionally has parameters
    """

    for m in modules:

        has_params = False
        if len(m) > 1:
            has_params = True
            params = m[1]

        name = m[0]

        # stop it if necessary
        # modules which must be restarted before every experiment
        must_restart = ['dummyActivityInterface', 'objectsPropertiesCollector', 'worldStateManager']
        for r in must_restart:
            if name == r:

                if args.v:
                    print('stopping', name)

                if has_params is not None:
                    if check_process(name, params):
                        stopped = stop(name, params)
                else:
                    if check_process(name):
                        stopped = stop(name)

        # run
        if has_params:
            if args.s:
                print('Will run: {} {}'.format(name, params))
                input('Press enter to continue.')
            success = run(name, params)
        else:
            if args.s:
                print('Will run: {}'.format(name))
                input('Press enter to continue.')
            success = run(name)

        if args.v:
            if success:
                print('Ran {}'.format(name), Fore.GREEN, 'successfully', Fore.RESET)
            else:
                print(Fore.RED, 'Error', Fore.RESET, 'running {}'.format(name))

def stop(ps_name, *pars):
    """
    Stop a process (using os.kill).

    Arguments:
    ps_name -- process executable name
    pars -- (optional) parameters of the process

    Return values:
    success -- boolean indicating whether the process was stopped or not
    """

    success = False

    if not pars:
        ps_full_string = ps_name
    else:
        ps_full_string = ps_name + ' ' + str(pars[0])

    if args.v:
        print('Trying to kill process:', ps_full_string)

    # based on https://stackoverflow.com/a/24589815
    try:
        pgrep = subprocess.check_output(['pgrep', '-a', '-f', ps_full_string]).decode('utf-8')
    except subprocess.CalledProcessError as e:
        print('pgrep failed because ({}):'.format(e.returncode), e.output.decode())
    else:
        pid = pgrep.split(' ')[0] # split by space and get first element
        try:
            os.kill(int(pid), signal.SIGTERM) # not always kills zombies
            os.kill(int(pid), signal.SIGKILL) # not always kills zombies
            os.kill(int(pid), 0) # https://stackoverflow.com/a/13402639
            # Alex: try sending "quit" command over RPC

            if args.v:
                print('Killed process:', ps_full_string)

            success = True
        except ProcessLookupError as e:
            print('Tried to kill an old entry of', ps_name)
        except ValueError as e:
            print('No', ps_name, 'found')

        return success

def stop_all():
    """
    Stop all modules in the application XML (using the stop helper function).
    """

    # for m in list_of_modules_to_start:
    for m in all_modules:
        name = m.find('name').text
        has_params = m.find('parameters')
        if has_params is not None:
            params = m.find('parameters').text
            if check_process(name, params):
                stop(name, params)
        else:
            if check_process(name):
                stop(name)

def mainroutine():
    """
    Execute the main routine of one experiment.

    Return values:
    score -- result of experiment, in #good/#total[/success] format, where
      * #good is the number of motor actions that succeeded,
      * #total is the total number of attempted motor actions (incl. #good),
      * (optional) success is a boolean value (1 or 0) indicating whether the
        final goal of the plan was achieved (1, default and not printed)
        or not (0)
    """

    # (stop and) run the selected modules
    run_many(list_of_modules_to_start)

    my_delay(5)

    # extract list of connection nodes from XML
    conns = root.findall('.//connection')

    print()

    # make yarp connections
    connect_missing(conns)

    my_delay(5)

    # connect to dummyActivityInterface RPC server
    rpc = yarp.RpcClient()
    local_name = '/simpoeticon/activity:rpc'
    remote_name = '/activityInterface/rpc:i'
    rpc.open(local_name)
    print()
    _ = connect(local_name, remote_name)
    my_delay(1)

    # while(not connect(local_name, remote_name)):
    #     print("waiting for connection to activityInterface RPC server")
    #     my_delay(1)

    # start simulation
    cmd = yarp.Bottle()
    cmd.clear()
    cmd.addString('simulate')
    rep = yarp.Bottle()
    rep.clear()

    if args.v:
        print('started simulation {}/{}...'.format(r+1, args.n))

    # TODO mechanism that monitors whether planningCycle crashes during RPC

    rpc.write(cmd, rep)
    if args.v:
        print('reply from dummyActivityInterface:', rep.toString())

    # remove double quotes
    rep = rep.toString().strip('\"')

    # compact score
    score = rep.split()[0]

    # log of robot actions
    ra = rep.split()[1:]
    ra = ' '.join(ra)
    ra = ra.replace(';', '\n')

    print('Result of experiment {}/{}:'.format(r+1, args.n), score)
    print('Full log:')
    print('{}'.format(ra))

    return score

if __name__ == '__main__':
    """
    Perform necessary parsing, configuration, and checks. Run the experiment
    (defined in the mainroutine helper function) N times. Print the
    concatenation of all the N results to a file.
    """

    args = parse_cmdline_options()

    if args.v:
        # print arguments
        for var, arg in vars(args).items():
            print('{}: {}'.format(var, arg))

    # global variables
    initial_img_port = '/icub/camcalib/left/out'
    yarprun_port = '/enigma'

    initial_setup_and_checks()

    root = parse_yarp_application_xml()

    # extract list of module nodes from XML
    all_modules = root.findall('.//module')

    # cleaning routine upon -c, --clean
    if args.clean:
        print('Cleaning ports and stopping modules')
        cleaning_routine()
        sys.exit()

    # build a list of modules that need starting, with respective parameters
    list_of_modules_to_start = modules_to_start(all_modules)

    # repeat N times, as requested by the user
    all_results = ''
    for r in range(args.n):

        # call main routine, make sure it finishes in less than args.t seconds
        start_time = time.time()
        # note: stopit.SignalTimeout is able to stop YARP RPC calls that get
        #       stuck or last a long time :) however it works in Linux only,
        #       because it's based on supports signal.SIGALRM.
        #       stopit.ThreadingTimeout is not able to stop those stuck calls.
        with stopit.SignalTimeout(args.t) as to_ctx_mgr:
            print('Starting experiment {}/{}'.format(r+1, args.n))
            rep = mainroutine()
            print('DEBUG: elapsed time {} s'.format(time.time() - start_time))

        # the code under time control completed successfully
        if to_ctx_mgr:
            print('Result: {}'.format(rep))
            # concatenate result to previous ones
            all_results = all_results + ' ' + rep
        else:
            # main routine did NOT complete successfully
            print('timeout: {} seconds (user asked: {})'.format(time.time()-start_time, args.t))

        # temporary fix for cleaning zombies
        cleaning_routine()
        my_delay(5)

    print('Result of all experiments:')
    print(all_results)

    with open(args.o.name, mode='a+') as f:
        f.write(all_results)
        f.write('\n')
        f.close()
    print('Written to {}'.format(args.o.name))

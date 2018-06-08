#!/usr/bin/lua

-- adapted from IOL Lua scripts, written by Ali Paikan

require("yarp")
require("rfsm")

yarp.Network()

-------
shouldExit = false

-- load state machine model and initalize it
rf = yarp.ResourceFinder()
rf:setDefaultContext("poeticon/lua")
rf:configure(arg)
fsm_file = rf:findFile("poeticon_root_fsm.lua")
fsm_model = rfsm.load(fsm_file)
fsm = rfsm.init(fsm_model)
rfsm.run(fsm)

repeat
    rfsm.run(fsm)
    yarp.Time_delay(0.1)
until shouldExit ~= false

print("finishing")
-- Deinitialize yarp network
yarp.Network_fini()

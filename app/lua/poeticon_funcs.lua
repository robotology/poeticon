-- adapted from IOL Lua scripts, written by Ali Paikan

function IOL_Initialize()
  -- initilization
  ispeak_port = yarp.BufferedPortBottle()
  speechRecog_port = yarp.Port()
  iol_port = yarp.Port()
  object_port = yarp.Port()

  -- defining objects and actions vocabularies
    objects = {"Rake", "Stick", "Ham", "Tomato", "Bun-bottom", "Bun-top"}
  actions = {"{point at}", "{what is this}"}

  -- defining speech grammar for Menu
  grammar = "Return to home position | Calibrate on table | Where is the #Object | Take the #Object | Grasp the #Object | See you soon  | I will teach you a new object | "
         .."Touch the #Object | Push the #Object | Let me show you how to reach the #Object with your right arm | Let me show you how to reach the #Object with your left arm | "
          .."Forget #Object | Forget all objects | Execute a plan | What is this | This is a #Object | Explore the #Object "

  -- defining speech grammar for Reward
  grammar_reward = "Yes you are | No here it is | Skip it"

  -- defining speech grammar for teaching a new object
  grammar_track = "There you go | Skip it"

  -- defining speech grammar for what function (ack)
  grammar_whatAck = "Yes you are | No you are not | Skip it | Wrong this is a #Object"

  -- defining speech grammar for what function (nack)
  grammar_whatNack = "This is a #Object | Skip it"

  -- defining speech grammar teach reach
  grammar_teach = "Yes I do | No I do not | Finished"

  return (ispeak_port ~= nil) and (speechRecog_port ~= nil) and (iol_port ~= nil) and (object_port ~= nil)
end

function speak(port, str)
   local wb = port:prepare()
    wb:clear()
    wb:addString(str)
    port:write()
   yarp.Time_delay(1.0)
end

----------------------------------
-- functions MOTOR - IOL        --
----------------------------------

function IOL_goHome(port)
   local wb = yarp.Bottle()
   local reply = yarp.Bottle()
   wb:clear()
    wb:addString("home")
    port:write(wb,reply)
end

function IOL_calibrate(port)
   local wb = yarp.Bottle()
   local reply = yarp.Bottle()
   wb:clear()
    wb:addString("cata")
    port:write(wb,reply)
end

function IOL_where_is(port, objName)
   local wb = yarp.Bottle()
   local reply = yarp.Bottle()
   wb:clear()
    wb:addString("where")
   wb:addString(objName)
    port:write(wb,reply)
   return reply
end

function IOL_reward(port, reward)
   local wb = yarp.Bottle()
   local reply = yarp.Bottle()
   wb:clear()
    wb:addString(reward)
    port:write(wb,reply)
end

function IOL_track_start(port)
   local wb = yarp.Bottle()
   local reply = yarp.Bottle()
   wb:clear()
    wb:addString("track")
   wb:addString("start")
    port:write(wb,reply)
end

function IOL_track_stop(port)
   local wb = yarp.Bottle()
   local reply = yarp.Bottle()
   wb:clear()
    wb:addString("track")
   wb:addString("stop")
    port:write(wb,reply)
end

function IOL_populate_name(port, objName)
   local wb = yarp.Bottle()
   local reply = yarp.Bottle()
   wb:clear()
    wb:addString("name")
   wb:addString(objName)
    port:write(wb,reply)
   return reply:get(0):asString()
end

function IOL_take(port, objName)
   local wb = yarp.Bottle()
   local reply = yarp.Bottle()
   wb:clear()
    wb:addString("take")
   wb:addString(objName)
    port:write(wb,reply)
   return reply:get(0):asString()
end

function IOL_grasp(port, objName)
   local wb = yarp.Bottle()
   local reply = yarp.Bottle()
   wb:clear()
    wb:addString("grasp")
   wb:addString(objName)
    port:write(wb,reply)
   return reply:get(0):asString()
end

function IOL_touch(port, objName)
   local wb = yarp.Bottle()
   local reply = yarp.Bottle()
   wb:clear()
    wb:addString("touch")
   wb:addString(objName)
    port:write(wb,reply)
   return reply:get(0):asString()
end

function IOL_push(port, objName)
   local wb = yarp.Bottle()
   local reply = yarp.Bottle()
   wb:clear()
    wb:addString("push")
   wb:addString(objName)
    port:write(wb,reply)
   return reply:get(0):asString()
end

function IOL_forget(port, objName)
   local wb = yarp.Bottle()
   local reply = yarp.Bottle()
   wb:clear()
    wb:addString("forget")
   wb:addString(objName)
    port:write(wb,reply)
   return reply:get(0):asString()
end

function IOL_explore(port, objName)
   local wb = yarp.Bottle()
   local reply = yarp.Bottle()
   wb:clear()
    wb:addString("explore")
   wb:addString(objName)
    port:write(wb,reply)
   return reply:get(0):asString()
end

function IOL_what(port)
   local wb = yarp.Bottle()
   local reply = yarp.Bottle()
   wb:clear()
    wb:addString("what")
    port:write(wb,reply)
   return reply:get(0):asString()
end

function IOL_this_is(port,objName)
   local wb = yarp.Bottle()
   local reply = yarp.Bottle()
   wb:clear()
    wb:addString("this")
    wb:addString(objName)
    port:write(wb,reply)
   return reply:get(0):asString()
end

function IOL_calib_kin_start(port, side, objName)
   local wb = yarp.Bottle()
   local reply = yarp.Bottle()
   wb:clear()
    wb:addString("caki")
   wb:addString("start")
   wb:addString(side)
   wb:addString(objName)
    port:write(wb,reply)
   return reply:get(0):asString()
end

function IOL_calib_kin_stop(port)
   local wb = yarp.Bottle()
   local reply = yarp.Bottle()
   wb:clear()
   wb:addString("caki")
   wb:addString("stop")
   port:write(wb,reply)
   return reply:get(0):asString()
end

----------------------------------
-- functions SPEECH             --
----------------------------------

function IH_Expand_vocab(port, objects)
    local wb = yarp.Bottle()
    local reply = yarp.Bottle()

    wb:addString("name")
    for key, word in pairs(objects) do
        wb:addString(word)
    end
    port:write(wb,reply)

    local rep  =  reply:get(0):asString()

    if rep == "ack" then
        for k in pairs (objects) do
            objects[k] = nil
        end
        for i=1, reply:size()-1 do
            objects[i] = reply:get(i):asString()
            print("objects are: ", objects[i])
        end
    else
        print("Was not able to set the new vocabulary: ", reply:get(0):asString() )
    end
   return rep
end

function SM_RGM_Expand(port, vocab, word)
    local wb = yarp.Bottle()
    local reply = yarp.Bottle()
    wb:clear()
    wb:addString("RGM")
    wb:addString("vocabulory")
    wb:addString("add")
    wb:addString(vocab)
    wb:addString(word)
    port:write(wb)
    --port:write(wb,reply)
    return "OK" --reply:get(1):asString()
end

function SM_Expand_asyncrecog(port, gram)
    local wb = yarp.Bottle()
    local reply = yarp.Bottle()
    wb:clear()
    wb:addString("asyncrecog")
    wb:addString("addGrammar")
    wb:addString(gram)
    port:write(wb,reply)
end

function SM_Reco_Grammar(port, gram)
    local wb = yarp.Bottle()
    local reply = yarp.Bottle()
    wb:clear()
    wb:addString("recog")
    wb:addString("grammarSimple")
    wb:addString(gram)
    port:write(wb,reply)
    return reply
end

function SM_RGM_Expand_Auto(port, vocab)
    local wb = yarp.Bottle()
    local reply = yarp.Bottle()
    wb:clear()
    wb:addString("RGM")
    wb:addString("vocabulory")
    wb:addString("addAuto")
    wb:addString(vocab)
    port:write(wb,reply)
    return reply:get(1):asString()
end

--[[
proc SM_Reco_Grammar { gram } {

   bottle clear
   bottle addString "recog"
   bottle addString "grammarSimple"
   bottle addString $gram
   SpeechManagerPort write bottle reply
   puts "Received from SpeechManager : [reply toString] "
   set wordsList ""
   for { set i 1 } { $i< [reply size] } {incr i 2} {
           set wordsList [lappend wordsList [ [reply get $i] toString] ]
   }
   return $wordsList
}
]]--

-- adapted from IOL Lua scripts, written by Ali Paikan

-- initialize yarp
if yarp == nil then
    require("yarp")
    yarp.Network()
end

-- find all required files
if rf ~= nil then
    iol_interact_fsm = rf:findFile("poeticon_interact_fsm.lua")
    iol_funcs = rf:findFile("poeticon_funcs.lua")
else
    iol_interact_fsm = "poeticon_interact_fsm.lua"
    iol_funcs = "poeticon_funcs.lua"
end


return rfsm.state {

  ----------------------------------
  -- entry of root state          --
  ----------------------------------
    entry=function()
        dofile(iol_funcs)
    end,

  ----------------------------------
  -- state INIT_IOL               --
  ----------------------------------
  ST_INITIOL = rfsm.state{
          doo=function()
                  ret = IOL_Initialize()
                  if ret == false then
                          rfsm.send_events(fsm, 'e_error')
                  else
                          rfsm.send_events(fsm, 'e_iol_ok')
                  end
          end
  },

   ----------------------------------
   -- state INITPORTS             --
   ----------------------------------
   ST_INITPORTS = rfsm.state{
           doo=function()
                   ret = ispeak_port:open("/IOL/speak")
                   ret = ret and speechRecog_port:open("/IOL/speechRecog")
                   ret = ret and iol_port:open("/IOL/iolmanager")
                   ret = ret and object_port:open("/IOL/objectHelper")
                   if ret == false then
                           rfsm.send_events(fsm, 'e_error')
                   else
                           rfsm.send_events(fsm, 'e_connect')
                   end
           end
   },

   ----------------------------------
   -- state CONNECTPORTS           --
   ----------------------------------
   ST_CONNECTPORTS = rfsm.state{
           doo=function()
                   ret = yarp.NetworkBase_connect(ispeak_port:getName(), "/iSpeak")
                   ret =  ret and yarp.NetworkBase_connect(speechRecog_port:getName(), "/speechRecognizer/rpc")
                   ret =  ret and yarp.NetworkBase_connect(iol_port:getName(), "/iolStateMachineHandler/human:rpc")
                   ret =  ret and yarp.NetworkBase_connect(object_port:getName(), "/iolHelper/rpc")
                   if ret == false then
                           print("\n\nERROR WITH CONNECTIONS, PLEASE CHECK\n\n")
                           rfsm.send_events(fsm, 'e_error')
                   end
           end
   },

   ----------------------------------
   -- state RETREIVEMEMORY         --
   ----------------------------------
   ST_RETREIVEMEMORY = rfsm.state{
           doo=function()
                   ret = true
                   ret = ret and (IH_Expand_vocab(object_port, objects) == "OK")

                   if ret == false then
                           rfsm.send_events(fsm, 'e_error')
                   end
           end
           },

   ----------------------------------
   -- state INITVOCABS             --
   ----------------------------------
   ST_INITVOCABS = rfsm.state{
           doo=function()
                   ret = true
                   for key, word in pairs(objects) do
                           ret = ret and (SM_RGM_Expand(speechRecog_port, "#Object", word) == "OK")
                   end

                   SM_Expand_asyncrecog(speechRecog_port, "icub-stop-now")

                   if ret == false then
                           rfsm.send_events(fsm, 'e_error')
                   end
           end
   },

   ----------------------------------
   -- state HOME                   --
   ----------------------------------
   ST_HOME = rfsm.state{
           doo=function()
                   print("everything is fine, going home!")
                   speak(ispeak_port, "Ready")
                   IOL_goHome(iol_port)
           end
   },


   ----------------------------------
   -- state FATAL                  --
   ----------------------------------
   ST_FATAL = rfsm.state{
           doo=function()
                   print("Fatal!")
                   shouldExit = true;
           end
   },

   ----------------------------------
   -- state FINI                   --
   ----------------------------------
   ST_FINI = rfsm.state{
           doo=function()
                   print("Closing...")
                   yarp.NetworkBase_disconnect(ispeak_port:getName(), "/iSpeak")
                   yarp.NetworkBase_disconnect(speechRecog_port:getName(), "/speechRecognizer/rpc")
                   yarp.NetworkBase_disconnect(iol_port:getName(), "/iolStateMachineHandler/human:rpc")
                   yarp.NetworkBase_disconnect(object_port:getName(), "/iolHelper/rpc")
                   ispeak_port:close()
                   speechRecog_port:close()
                   iol_port:close()
                   object_port:close()

                   shouldExit = true;
           end
   },


   --------------------------------------------
   -- state MENU  is defined in menu_fsm.lua --
   --------------------------------------------
   ST_INTERACT = dofile(iol_interact_fsm),


   ----------------------------------
   -- setting the transitions      --
   ----------------------------------

   rfsm.transition { src='initial', tgt='ST_INITIOL' },
   rfsm.transition { src='ST_INITIOL', tgt='ST_INITPORTS', events={'e_iol_ok'} },
   rfsm.transition { src='ST_INITIOL', tgt='ST_FATAL', events={ 'e_error' } },

   rfsm.transition { src='ST_INITPORTS', tgt='ST_CONNECTPORTS', events={ 'e_connect' } },
   rfsm.transition { src='ST_INITPORTS', tgt='ST_FATAL', events={ 'e_error' } },

   rfsm.transition { src='ST_CONNECTPORTS', tgt='ST_FINI', events={ 'e_error' } },
   rfsm.transition { src='ST_CONNECTPORTS', tgt='ST_RETREIVEMEMORY', events={ 'e_done' } },
   rfsm.transition { src='ST_RETREIVEMEMORY', tgt='ST_INITVOCABS', events={ 'e_done' } },

   rfsm.transition { src='ST_RETREIVEMEMORY', tgt='ST_FINI', events={ 'e_error' } },
   rfsm.transition { src='ST_INITVOCABS', tgt='ST_FINI', events={ 'e_error' } },
   rfsm.transition { src='ST_INITVOCABS', tgt='ST_HOME', events={ 'e_done' } },

   rfsm.transition { src='ST_HOME', tgt='ST_INTERACT', events={ 'e_done' } },
   rfsm.transition { src='ST_INTERACT', tgt='ST_FINI', events={ 'e_menu_done' } },

}

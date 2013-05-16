local function InvCmd(ply, argc, argv)
        local cmd = argv[1]
        local NPCEnt = ents.GetByIndex(ply.CurrentlyLooting)
        local NPC = NPCEnt:ToNPC()
        --local lootedNPC = ents.GetByIndex(ply.CurrentlyLooting):ToNPC()
        -- if lootedNPC.IsNPC == false then
        --      return
        if cmd == "clo" then
                ply:SendCommand("loot cl")
                ply.CurrentlyLooting = 1023
                NPC.CurrentLooter = 1023
                return
        end
        if cmd == "cls" then
                ply.CurrentlyLooting = 1023
                NPC.CurrentLooter = 1023
                return
        end
end
 
cmds.Add("~inv", InvCmd)
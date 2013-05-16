--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Code
	Sunset6 init
	
	Written by BobaFett
--------------------------------------------------]]

local function InitQuest(ply, argc, argv)
	local lst = ents.GetByClass("quest_homestead")
	if #lst > 0 then
		ply:SendPrint("Tusken Troubles quest is already activated")
		return
	end
	local entfact = ents.CreateEntityFactory("quest_homestead")
	local ent = entfact:Create()
	if ent and ent:IsValid() then
		timer.Simple(500, function(ent) ent:Activate() end, ent)
		ply:SendPrint("Tusken Troubles ^2Activated^7!")
	else
		ply:SendPrint("Failed to activate quest")
	end
end

cmds.Add("initquest", InitQuest)

local function TerminateQuest(ply, argc, argv)
	local lst = ents.GetByClass("quest_homestead")
	if #lst == 0 then
		ply:SendPrint("Tusken Troubles quest is not activated")
		return
	end
	local ent = lst[1]
	ent:Terminate()
	ply:SendPrint("Tusken Troubles ^1Deactivated^7!")
end

cmds.Add("terminatequest", TerminateQuest)

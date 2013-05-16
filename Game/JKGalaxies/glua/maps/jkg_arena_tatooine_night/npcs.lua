--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Code
	Tatooine arena backend: npcs
	
	Written by BobaFett
--------------------------------------------------]]

-- Here we'll spawn all the npcs that belong to the arena

local npctable = {
	-- The tourney/match signup clerk
	{
		NPCType = "prisoner",
		NPCScript = "arena_tourneyclerk",
		Origin = Vector(-1250, 1665, 1590),
		Angles = Vector(0, -140, 0),
	},
	-- 3 Ticket clerks
	{
		NPCType = "prisoner",
		NPCScript = "arena_ticketclerk",
		Origin = Vector(-2010, 1420, 1592),
		Angles = Vector(0, 0, 0),
	},
	{
		NPCType = "prisoner",
		NPCScript = "arena_ticketclerk",
		Origin = Vector(-1945, 1675, 1592),
		Angles = Vector(0, -45, 0),
	},
	{
		NPCType = "prisoner",
		NPCScript = "arena_ticketclerk",
		Origin = Vector(-1660, 1725, 1592),
		Angles = Vector(0, -90, 0),
	},
	-- Merchant
	{
		NPCType = "prisoner",
		NPCScript = "arena_merchant",
		Origin = Vector(0, -1550, 1496),
		Angles = Vector(0, 90, 0),
	},
	-- Info clerk (@ 1v1 arena-side)
	{
		NPCType = "prisoner",
		NPCScript = "arena_info1clerk",
		Origin = Vector(1575, -2085, 1608),
		Angles = Vector(0, 90, 0),
	},
	{
		NPCType = "prisoner",
		NPCScript = "arena_info1clerk",
		Origin = Vector(1880, -2085, 1608),
		Angles = Vector(0, 90, 0),
	},
	-- Info clerk (3v3-only side)
	{
		NPCType = "prisoner",
		NPCScript = "arena_info3clerk",
		Origin = Vector(1585, 2085, 1644),
		Angles = Vector(0, -90, 0),
	},
	{
		NPCType = "prisoner",
		NPCScript = "arena_info3clerk",
		Origin = Vector(1890, 2085, 1644),
		Angles = Vector(0, -90, 0),
	},
	-- 3v3 betting
	{
		NPCType = "prisoner",
		NPCScript = "arena_betting3clerk",
		Origin = Vector(-1415, 3085, 664),
		Angles = Vector(0, -90, 0),
	},
	{
		NPCType = "prisoner",
		NPCScript = "arena_betting3clerk",
		Origin = Vector(-1640, 3085, 664),
		Angles = Vector(0, -90, 0),
	},
	{
		NPCType = "prisoner",
		NPCScript = "arena_betting3clerk",
		Origin = Vector(-1640, -3085, 664),
		Angles = Vector(0, 90, 0),
	},
	{
		NPCType = "prisoner",
		NPCScript = "arena_betting3clerk",
		Origin = Vector(-1415, -3085, 664),
		Angles = Vector(0, 90, 0),
	},
	-- 1v1 betting
	{
		NPCType = "prisoner",
		NPCScript = "arena_betting1clerk",
		Origin = Vector(-3085, -5160, 600),
		Angles = Vector(0, 0, 0),
	},
	{
		NPCType = "prisoner",
		NPCScript = "arena_betting1clerk",
		Origin = Vector(-3085, -4905, 600),
		Angles = Vector(0, 0, 0),
	},
	{
		NPCType = "prisoner",
		NPCScript = "arena_betting1clerk",
		Origin = Vector(1100, -4905, 600),
		Angles = Vector(0, 180, 0),
	},
	{
		NPCType = "prisoner",
		NPCScript = "arena_betting1clerk",
		Origin = Vector(1100, -5160, 600),
		Angles = Vector(0, 180, 0),
	},
	-- Bartender
	{
		NPCType = "prisoner",
		NPCScript = "arena_bartender",
		Origin = Vector(-385, -3235, 664),
		Angles = Vector(0, -56, 0),
	},
}


local function InitNPCs()
	-- First, delete all spawners we already made
	local k,v
	local entlist = ents.GetByName("arena_npcs")
	for k,v in pairs(entlist) do
		v:Free()
	end

	-- Spawn our npcs
	local entfact = ents.CreateEntityFactory("NPC_spawner")
	-- Fill in the shared data
	entfact:SetSpawnVar("targetname", "arena_npcs")
	entfact:SetSpawnVar("count", "-1")
	for k,v in pairs(npctable) do
		entfact:SetSpawnVar("NPC_type", v.NPCType)
		entfact:SetSpawnVar("origin", string.format("%i %i %i", v.Origin.x, v.Origin.y, v.Origin.z))
		entfact:SetSpawnVar("angles", string.format("%i %i %i", v.Angles.x, v.Angles.y, v.Angles.z))
		entfact:SetSpawnVar("npcscript", v.NPCScript)
		ent = entfact:Create()
		if not ent or ent:IsValid() == false then
			print("ERROR: Failed to spawn arena NPC")
		end
		-- Spawn the npc
		ent:Use(nil, nil)
	end
end

hook.Add("MapLoaded", "ArenaNPCInit", InitNPCs)
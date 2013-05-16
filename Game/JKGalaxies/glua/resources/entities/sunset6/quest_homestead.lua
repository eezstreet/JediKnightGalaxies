--[[ ------------------------------------------------
	Jedi Knight Galaxies Entity
	Quest controller
	
	Quest: Homestead alpha quest: Tusken Troubles
	
	Quest script by DarthLex
	Written by BobaFett
--------------------------------------------------]]

ENT.ClassName = "quest_homestead"
ENT.Type = "logical"
ENT.Base = "base_logical"

-- Quest tag: homestead
-- 

function ENT:OnSpawn()
	-- Only 1 of us is allowed at any given time
	local entlist = ents.GetByClass("quest_homestead")
	local k,v
	if #entlist ~= 1 then
		print("A quest_homestead entity has already been spawned")
		self:Free()
		return
	end

	-- Wait a moment before creating our spawners
	self.OnThink = self.CreateSpawns
	self.TargetList = nil
	self.NPCList = nil
	
	self.RoamNPCs = nil
	self.BenettNPC = nil
	
	self:SetNextThink(100)
	
	self.FinishedInit = false
end

function ENT:CreateSpawns()
	self.TargetList = {}
	-- Time to create our spawnpoints for the npc's
	local ent
	local k,v
	local entfact = ents.CreateEntityFactory("NPC_spawner")
	-- Fill in the shared data
	entfact:SetSpawnVar("targetname", "quest_homestead_spawn")
	entfact:SetSpawnVar("npcscript", "hsroam_normal")
	entfact:SetSpawnVar("count", "-1")
	for k,v in pairs(self.RoamNPCInfo) do
		entfact:SetSpawnVar("NPC_type", v.NPCType)
		entfact:SetSpawnVar("origin", string.format("%i %i %i", v.SpawnPoint.x, v.SpawnPoint.y, v.SpawnPoint.z))
		entfact:SetSpawnVar("npcid", v.NPCID)
		ent = entfact:Create()
		if not ent or ent:IsValid() == false then
			print("ERROR: quest_homestead could not create spawnpoint")
		else
			ent.Controller = self
			table.insert(self.TargetList, ent)
		end
	end
	-- Spawn benett
	entfact:SetSpawnVar("npcscript", "hsroam_benett")
	v = self.BenettInfo
	entfact:SetSpawnVar("NPC_type", v.NPCType)
	entfact:SetSpawnVar("origin", string.format("%i %i %i", v.SpawnPoint.x, v.SpawnPoint.y, v.SpawnPoint.z))
	entfact:SetSpawnVar("npcid", v.NPCID)
	ent = entfact:Create()
	if not ent or ent:IsValid() == false then
		print("ERROR: quest_homestead could not create spawnpoint")
	else
		ent.Controller = self
		table.insert(self.TargetList, ent)
	end
	
	-- And we're done
	self.OnThink = nil -- We're done
	self.FinishedInit = true
end

function ENT:Activate()
	if not self.FinishedInit then return end
	-- Lets get ready to roll
	if self.NPCList then
		-- If we got npcs, destroy them
		local k,v
		for k,v in pairs(self.NPCList) do
			if v and v:IsValid() then
				v:Remove()
			end
		end
	end
	self.NPCList = {}	-- Clear the list
	for k,v in pairs(self.TargetList) do
		v:Use(self, self)	-- Spawn them, the npcs themselves will link up to me
	end
end

-- Will destoy itself, all spawners, and all npcs
function ENT:Terminate()
	if not self.FinishedInit then return end
	-- Lets get ready to roll
	if self.NPCList then
		-- If we got npcs, destroy them
		local k,v
		for k,v in pairs(self.NPCList) do
			if v and v:IsValid() then
				v:Remove()
			end
		end
	end
	for k,v in pairs(self.TargetList) do
		v:Free()
	end
	self:Free()
end

function ENT:RegisterNPC(npc, id)
	if id == 0 then
		self.BenettNPC = npc
		-- ID 0 is benett, the rest are the roamers
	else
		if not self.RoamNPCs then
			self.RoamNPCs = {}
		end
		self.RoamNPCs[id] = npc
	end
	table.insert(self.NPCList, npc)
end

ENT.RoamNPCInfo = {
	[1] = {
			NPCID = 1,
			SpawnPoint = Vector(1425,-400, 0),
			NPCType = "hsroam_1"
		  },
	[2] = {
			NPCID = 2,
			SpawnPoint = Vector(900, -200, 0),
			NPCType = "hsroam_2"
		  },
	[3] = {
			NPCID = 3,
			SpawnPoint = Vector(675, -1285, 0),
			NPCType = "hsroam_3"
		  }
}

ENT.BenettInfo = {
	NPCID = 0, 	-- Special ID for benett
	SpawnPoint = Vector(640, -260, -295),
	NPCType = "hsroam_benett"
}
	

ENT.NavRoutes = {
	-- Routes for roamers, the holdtime determines how long the npc will stay at the waypoint before going to the next (random between min and max)
	-- TODO: All of this :P
	[1] = { 
			{
				Origin = Vector(1425,-400, 0),
				HoldtimeMin = 1000,
				HoldtimeMax = 4000
			},
			{
				Origin = Vector(1500, -1420, 0),
				HoldtimeMin = 1000,
				HoldtimeMax = 4000
			},
			{
				Origin = Vector(800,-1500, 0),
				HoldtimeMin = 1500,
				HoldtimeMax = 6000
			},
			{
				Origin = Vector(1715, -1050, 0),
				HoldtimeMin = 750,
				HoldtimeMax = 2000
			},
			{
				Origin = Vector(1660, -650, 0),
				HoldtimeMin = 750,
				HoldtimeMax = 2000
			},
			{
				Origin = Vector(1000, -500, 0),
				HoldtimeMin = 1500,
				HoldtimeMax = 6000
			}
		  },
	[2] = { 
			{
				Origin = Vector(900, -200, 0),
				HoldtimeMin = 1000,
				HoldtimeMax = 4000
			},
			{
				Origin = Vector(840, 485, 0),
				HoldtimeMin = 500,
				HoldtimeMax = 1000
			},
			{
				Origin = Vector(275, 660, 0),
				HoldtimeMin = 500,
				HoldtimeMax = 1000
			},
			{
				Origin = Vector(-175, 420, 0),
				HoldtimeMin = 500,
				HoldtimeMax = 1000
			},
			{
				Origin = Vector(-300, -65, 0),
				HoldtimeMin = 500,
				HoldtimeMax = 1000
			},
			{
				Origin = Vector(130, -575, 0),
				HoldtimeMin = 1500,
				HoldtimeMax = 6000
			},
			{
				Origin = Vector(-60, -925, 0),
				HoldtimeMin = 500,
				HoldtimeMax = 1000
			},
			{
				Origin = Vector(15, -1365, 0),
				HoldtimeMin = 1500,
				HoldtimeMax = 4000
			},
			{
				Origin = Vector(-70, -1950, 0),
				HoldtimeMin = 500,
				HoldtimeMax = 1000
			},
			{
				Origin = Vector(325, -2160, 0),
				HoldtimeMin = 500,
				HoldtimeMax = 1000
			},
			{
				Origin = Vector(800, -1775, 0),
				HoldtimeMin = 1500,
				HoldtimeMax = 3000
			},
			{
				Origin = Vector(615, -750, 0),
				HoldtimeMin = 750,
				HoldtimeMax = 2000
			},			
		  },
	[3] = { 
			{
				Origin = Vector(675, -1285, 0),
				HoldtimeMin = 1000,
				HoldtimeMax = 4000
			},
			{
				Origin = Vector(100, -1380, 0),
				HoldtimeMin = 500,
				HoldtimeMax = 2000
			},
			{
				Origin = Vector(-400, -1015, 0),
				HoldtimeMin = 500,
				HoldtimeMax = 2000
			},
			{
				Origin = Vector(-570, -725, 0),
				HoldtimeMin = 500,
				HoldtimeMax = 2000
			},
			{
				Origin = Vector(-225, -525, 0),
				HoldtimeMin = 500,
				HoldtimeMax = 1500
			},
			{
				Origin = Vector(500, -540, 0),
				HoldtimeMin = 1000,
				HoldtimeMax = 4000
			},
			{
				Origin = Vector(575, -860, 0),
				HoldtimeMin = 1000,
				HoldtimeMax = 4000
			}
		  }
}
	

function ENT:GetNavRoute(id)
	-- The roaming npcs will call this to obtain their navigational route
	return self.NavRoutes[id]
end

function ENT:RespawnNPC(spawner)
	if spawner and spawner:IsValid() then
		spawner:Use(self, self)
	end
end

function ENT:NPCKilled(npc, id)
	if id > 0 then
		self.RoamNPCs[id] = nil
	else
		-- We lost benett
		self.BenettNPC = nil
	end
	timer.Simple(math.random(2000,20000), self.RespawnNPC, self, npc.Spawner)
end

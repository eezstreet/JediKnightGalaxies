-- WARNING: BROKEN - DO NOT USE


-- Classname to be used for this entity (Mandatory, without this defined, the entity is discarded by the loader)
ENT.ClassName = "misc_weapshop"
-- Type of this entity (generic, logical, mover and trigger are currently supported, also mandatory for the loader to accept this entity)
ENT.Type = "generic"
-- Scripted entity to derrive this one from.
-- Note that this must be another scripted entity. You cannot provide a fx_runner as base entity for example.
-- The entity manager/loader automatically creates standard base entities for every type, called base_<type>, so for this one, we'll use "base_generic"
ENT.Base = "base_generic"

-- In every event, the entity in question is passed as a special argument, called 'self'
-- ( table:function(a,b,c) is syntactic sugar for table.function(self, a, b, c), so in this case ENT:OnSpawn() is identical to ENT.OnSpawn(self) )

ENT.Convos = {}
ENT.Convos[1] = {}		-- Startup conversation
ENT.Convos[1].DoConvo = function(ent, ply)
	local convo = sys.CreateConversation()
	convo:AddText("Welcome to the armory", 3000)
	convo:AddText("How can I help you?", 1500)
	convo:AddChoices(ent.Convos[1].ProcessChoice, {"Buy weapons", "Remove weapons", "I have to go"})
	convo:SetUserData(ent)
	convo:RunConvo(ply)
end
ENT.Convos[1].ReturnConvo = function(convo, ply)
	convo:Reset()
	convo:AddText("How can I help you?", 1500)
	convo:AddChoices(convo:GetUserData().Convos[1].ProcessChoice, {"Buy weapons", "Remove weapons", "I have to go"})
	convo:RunConvo(ply)
end
ENT.Convos[1].ProcessChoice = function(convo, ply, response)
	if (response == 1) then
		convo:GetUserData().Convos[2].DoConvo(convo, ply)
	elseif (response == 2) then
		convo:GetUserData().Convos[3].DoConvo(convo, ply)
	else
		-- Failsafe
		convo:Reset()
		convo:AddText("Goodbye", 2000)
		convo:RunConvo(ply)
	end
end

ENT.Convos[2] = {}		-- Buy weapons
ENT.Convos[2].DoConvo = function(convo, ply)
	convo:Reset()
	convo:AddText("Which weapon would you like to buy?", 3000)
	convo:AddChoices(convo:GetUserData().Convos[2].ProcessChoice, {"E-11 Blaster", "Repeater", "Disruptor rifle", "Rocket launcher", "Never mind"})
	convo:RunConvo(ply)
end

ENT.Convos[2].ReturnConvo = function(convo, ply)
	-- WARNING: Reset the convo before going here
	convo:AddText("Anything else you'd like to buy?", 3000)
	convo:AddChoices(convo:GetUserData().Convos[2].ProcessChoice, {"E-11 Blaster", "Repeater", "Disruptor rifle", "Rocket launcher", "Never mind"})
	convo:RunConvo(ply)
end

ENT.Convos[2].ProcessChoice = function(convo, ply, response)
	local goback = true
	convo:Reset()
	if (response == 1) then
		ply:GiveWeapon(WP_BLASTER)
		convo:AddText("*You obtain the E-11 Blaster*", 2000)
	elseif (response == 2) then
		ply:GiveWeapon(WP_REPEATER)
		convo:AddText("*You obtain the Repeater*", 2000)
	elseif (response == 3) then
		ply:GiveWeapon(WP_DISRUPTOR)
		convo:AddText("*You obtain the Disruptor rifle*", 2000)
	elseif (response == 4) then
		ply:GiveWeapon(WP_ROCKET_LAUNCHER)
		convo:AddText("*You obtain the Rocket launcher*", 2000)
	else
		convo:GetUserData().Convos[1].ReturnConvo(convo, ply)
		goback = false
	end
	if goback then
		convo:GetUserData().Convos[2].ReturnConvo(convo, ply)
	end
	
end

ENT.Convos[3] = {}		-- Remove weapons
ENT.Convos[3].DoConvo = function(convo, ply)
	convo:Reset()
	convo:AddText("Are you sure you want your weapons removed?", 3000)
	convo:AddChoices(convo:GetUserData().Convos[3].ProcessChoice, {"Yes i am sure", "No, never mind"})
	convo:RunConvo(ply)
end
ENT.Convos[3].ProcessChoice = function(convo, ply, response)
	if (response == 1) then
		ply:StripClipAmmo()
		ply:StripAmmo()
		ply:StripWeapons()
		ply:GiveWeapon(WP_MELEE)
		ply:SetWeapon(WP_MELEE)
		convo:Reset()
		convo:AddText("All your weapons have been removed", 3000)
		convo:AddText("Have a nice day", 2000)
		convo:RunConvo(ply)
	elseif (response == 2) then
		convo:GetUserData().Convos[1].ReturnConvo(convo, ply)
	else
		-- Failsafe
		convo:Reset()
		convo:AddText("Goodbye", 2000)
		convo:RunConvo(ply)
	end
end

-- OnSpawn event, called when the entity just spawned
-- In here we have to do the initialization of the entity, processing of the spawnvars, and doing all the setup required to get the ent workin the way we want to
function ENT:OnSpawn()
	-- Set-up position, rotation (based on spawnvars) and model
	-- The 'or' is to specify a default value. A call to GetSpawnVar with an undefined var will return nil, in which case the alternative value is used
	-- So for example: If the spawnvar 'angles' is not defined, it'll use '0 0 0' instead
	self:SetPos( Vector(self:GetSpawnVar("origin") or "0 0 0") )
	self:SetAngles( Vector(self:GetSpawnVar("angles") or "0 0 0") )
	self:SetModel("models/map_objects/h_evil/control_station.md3")
	-- Automatically create a bounding box to fit the model + rotation
	self:AutoBox()
	-- Creating custom variables in entities is very simple, just use entity.<variable name>.
	-- So creating a custom variable called lol containing "my value", can be done with entity.lol = "my value"
	-- Any valid variable name is allowed as long as the name doesn't match one of the built-in functions of entities (so entity.SetPos = "my value" won't work as expected)
	
	-- Set timeout for the use to 0, so we can use it right now
	self.UseTimeout = 0
	-- Make the entity solid
	self:SetContents(CONTENTS_SOLID)
	self:SetClipmask(MASK_SOLID)
	-- Make the entity usable
	self:SetPlayerUsable(true)
	-- Put this entity in the world
	self:LinkEntity()
end

-- OnUse event, called after a player presses use on the entity (assuming the entity is usable by players) or if the entity is triggered by another entity
-- The 'other' argument is the entity that triggered it (in case it was triggered by another entity),
-- the 'activator' is the (usually player) responsible for the start of the chain
-- So, for example, if you have a button, targetting a target_relay, which targets this entity,
-- the 'other' will be the target_relay entity, and the 'activator' the player that pressed the button
function ENT:OnUse(other, activator)
	-- If we're in a timeout, ignore the use for now
	if self.UseTimeout > sys.Time() then
		return
	end
	self.UseTimeout = sys.Time() + 1000
	local ply = activator:ToPlayer()
	self.Convos[1].DoConvo(self,ply)
end

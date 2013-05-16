
-- Classname to be used for this entity (Mandatory, without this defined, the entity is discarded by the loader)
ENT.ClassName = "target_logicaltest"
-- Type of this entity (generic, logical, mover and trigger are currently supported, also mandatory for the loader to accept this entity)
ENT.Type = "logical"
-- Scripted entity to derrive this one from.
-- Note that this must be another scripted entity. You cannot provide a fx_runner as base entity for example.
-- The entity manager/loader automatically creates standard base entities for every type, called base_<type>, so for this one, we'll use "base_generic"
ENT.Base = "base_logical"

-- In every event, the entity in question is passed as a special argument, called 'self'
-- ( table:function(a,b,c) is syntactic sugar for table.function(self, a, b, c), so in this case ENT:OnSpawn() is identical to ENT.OnSpawn(self) )

-- OnSpawn event, called when the entity just spawned
-- In here we have to do the initialization of the entity, processing of the spawnvars, and doing all the setup required to get the ent workin the way we want to
function ENT:OnSpawn()
	-- This is a logical entity, so no need to set position, angles or linking it
	
	-- Lets do a test of the think function by adding in a timer
	self.Count = 0
	self:SetNextThink(500)
end

-- OnUse event, called after a player presses use on the entity (assuming the entity is usable by players) or if the entity is triggered by another entity
-- The 'other' argument is the entity that triggered it (in case it was triggered by another entity),
-- the 'activator' is the (usually player) responsible for the start of the chain
-- So, for example, if you have a button, targetting a target_relay, which targets this entity,
-- the 'other' will be the target_relay entity, and the 'activator' the player that pressed the button
function ENT:OnUse(other, activator)
	-- If we're in a timeout, ignore the use for now
	if not activator:IsPlayer() then
		return
	end
	local ply = activator:ToPlayer()
	ply:SendCenterPrint(string.format("^7Testing... %i", self.Count))
end

function ENT:OnThink()
	self.Count = self.Count + 1
	self:SetNextThink(500)
end

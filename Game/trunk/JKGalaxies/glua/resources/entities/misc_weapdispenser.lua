
-- Classname to be used for this entity (Mandatory, without this defined, the entity is discarded by the loader)
ENT.ClassName = "misc_weapdispenser"
-- Type of this entity (generic, logical, mover and trigger are currently supported, also mandatory for the loader to accept this entity)
ENT.Type = "generic"
-- Scripted entity to derrive this one from.
-- Note that this must be another scripted entity. You cannot provide a fx_runner as base entity for example.
-- The entity manager/loader automatically creates standard base entities for every type, called base_<type>, so for this one, we'll use "base_generic"
ENT.Base = "base_generic"

-- In every event, the entity in question is passed as a special argument, called 'self'
-- ( table:function(a,b,c) is syntactic sugar for table.function(self, a, b, c), so in this case ENT:OnSpawn() is identical to ENT.OnSpawn(self) )

ENT.Packs = {}
ENT.Packs[0] = {}
ENT.Packs[0].Name = "Basic merc arms"
ENT.Packs[0].Weapons = { WP_BLASTER, WP_MELEE }
ENT.Packs[1] = {}
ENT.Packs[1].Name = "Bounty hunter arms"
ENT.Packs[1].Weapons = { WP_BLASTER, WP_MELEE, WP_REPEATER, WP_THERMAL, WP_DISRUPTOR, WP_ROCKET_LAUNCHER }
ENT.Packs[2] = {}
ENT.Packs[2].Name = "Heavy Demolitions"
ENT.Packs[2].Weapons = { WP_BLASTER, WP_MELEE, WP_THERMAL, WP_DET_PACK, WP_TRIP_MINE, WP_CONCUSSION, WP_ROCKET_LAUNCHER }
ENT.Packs[3] = {}
ENT.Packs[3].Name = "Sareth's weapons"
ENT.Packs[3].Weapons = { WP_BLASTER, WP_MELEE, WP_REPEATER, WP_THERMAL }
ENT.Packs[4] = {}
ENT.Packs[4].Name = "All weapons"
ENT.Packs[4].Weapons = { WP_BRYAR_OLD, WP_BOWCASTER, WP_BRYAR_PISTOL, WP_STUN_BATON, WP_BLASTER, WP_MELEE, WP_REPEATER, WP_THERMAL, WP_SABER, WP_DISRUPTOR, WP_CONCUSSION, WP_ROCKET_LAUNCHER, WP_TRIP_MINE, WP_THERMAL, WP_DET_PACK, WP_DEMP2, WP_FLECHETTE }
ENT.Packs[5] = {}
ENT.Packs[5].Name = "Quest weapons"
ENT.Packs[5].Weapons = { WP_BLASTER, WP_MELEE, WP_REPEATER, WP_THERMAL, WP_DISRUPTOR }


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
	
	-- Get the 'fuse' spawnvar (or 10 if not defined) to use as fuse time
	self.WPPackage = tonumber(self:GetSpawnVar("pack") or "10")
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
	local weap = self.Packs[self.WPPackage]
	for k,v in pairs(weap.Weapons) do
		ply:GiveWeapon(v)
	end
	ply:SendCenterPrint("^7You have obtained the\n^1" .. weap.Name .. "\n^7Weapons pack")
end

--[[ ------------------------------------------------
	Jedi Knight Galaxies Entity

	Misc_bomb_remote
	
	Modified version of the misc_bomb, that explodes by remote instead of taking damage or being activated manualy
	
	Written by BobaFett
--------------------------------------------------]]

-- Classname to be used for this entity (Mandatory, without this defined, the entity is discarded by the loader)
ENT.ClassName = "misc_bomb_detonator"
-- Type of this entity (generic, logical, mover and trigger are currently supported, also mandatory for the loader to accept this entity)
ENT.Type = "generic"
-- Scripted entity to derrive this one from.
-- Note that this must be another scripted entity. You cannot provide a fx_runner as base entity for example.
-- The entity manager/loader automatically creates standard base entities for every type, called base_<type>, so for this one, we'll use "base_generic"
ENT.Base = "base_generic"

-- In every event, the entity in question is passed as a special argument, called 'self'
-- ( table:function(a,b,c) is syntactic sugar for table.function(self, a, b, c), so in this case ENT:OnSpawn() is identical to ENT.OnSpawn(self) )

-- OnSpawn event, called when the entity just spawned
-- In here we have to do the initialization of the entity, processing of the spawnvars, and doing all the setup required to get the ent workin the way we want to
function ENT:OnSpawn()
	-- Set-up position, rotation (based on spawnvars) and model
	-- The 'or' is to specify a default value. A call to GetSpawnVar with an undefined var will return nil, in which case the alternative value is used
	-- So for example: If the spawnvar 'angles' is not defined, it'll use '0 0 0' instead
	self:SetPos( Vector(self:GetSpawnVar("origin") or "0 0 0") )
	self:SetAngles( Vector(self:GetSpawnVar("angles") or "0 0 0") )
	self:SetModel("models/map_objects/factory/f_con1.md3")
	-- Automatically create a bounding box to fit the model + rotation
	self:AutoBox()
	-- Creating custom variables in entities is very simple, just use entity.<variable name>.
	-- So creating a custom variable called lol containing "my value", can be done with entity.lol = "my value"
	-- Any valid variable name is allowed as long as the name doesn't match one of the built-in functions of entities (so entity.SetPos = "my value" won't work as expected)
	
	-- Get the 'fuse' spawnvar (or 10 if not defined) to use as fuse time
	self.Code = self:GetSpawnVar("code")
	if (self.Code == nil) then
		print("ERROR: misc_bomb_detonator without code spawned")
		self:Free()
		return
	end
	-- Set timeout for the use to 0, so we can use it right now
	self.UseTimeout = 0
	-- Make the entity solid
	self:SetContents(CONTENTS_SOLID)
	self:SetClipmask(MASK_SOLID)
	-- Put this entity in the world
	self:SetPlayerUsable(true)
	self:LinkEntity()
end

function ENT:OnUse(other, activator)
	if self.UseTimeout > sys.Time() then
		return
	end
	self.UseTimeout = sys.Time() + 1000
	local enttbl = ents.GetByClass("misc_bomb_remote")
	local showmsg = false
	for k,v in pairs(enttbl) do
		if v.Code == self.Code then
			showmsg = true
			v:Explode()
		end
	end
	if showmsg then
		activator:ToPlayer():SendCenterPrint("Bombs detonated")
	end
end

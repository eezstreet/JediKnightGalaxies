
-- Classname to be used for this entity (Mandatory, without this defined, the entity is discarded by the loader)
ENT.ClassName = "misc_questprepare"
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
ENT.Packs[0].Name = "Quest weapons"
ENT.Packs[0].Type = 1
ENT.Packs[0].Weapons = { {WP_BRYAR_PISTOL, 0}, {WP_BLASTER, 65}, {WP_MELEE, 0}, {WP_REPEATER, 60}, {WP_THERMAL, 0}, {WP_DISRUPTOR, 90} }
ENT.Packs[0].DefWeapon = WP_BLASTER
ENT.Packs[0].Ammo = { {AMMO_BLASTER, 200} , {AMMO_POWERCELL, 300}, {AMMO_METAL_BOLTS, 300} , {AMMO_THERMAL, 3} }
ENT.Packs[1] = {}
ENT.Packs[1].Name = "Quest powers"
ENT.Packs[1].Type = 2
ENT.Packs[1].Forces = { {FP_SABER_OFFENSE, 3}, {FP_SABER_DEFENSE, 3}, {FP_SABERTHROW, 3}, {FP_LEVITATION, 2}, {FP_HEAL, 2}, {FP_PUSH, 2}, {FP_PULL, 2} , {FP_TELEPATHY, 2} }
ENT.Packs[2] = {}
ENT.Packs[2].Name = "2vs2 Weapns"
ENT.Packs[2].Type = 1
ENT.Packs[2].Weapons = { {WP_BRYAR_OLD, 50}, {WP_BLASTER, 65}, {WP_MELEE, 0}, {WP_THERMAL, 0} }
ENT.Packs[2].DefWeapon = WP_BLASTER
ENT.Packs[2].Ammo = { {AMMO_BLASTER, 3000} , {AMMO_POWERCELL, 3000}, {AMMO_METAL_BOLTS, 3000} , {AMMO_THERMAL, 1} }


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
	
	self.WPPackage = tonumber(self:GetSpawnVar("pack") or "0")
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
	local pack = self.Packs[self.WPPackage]
	ply:StripClipAmmo()
	ply:StripAmmo()
	ply:StripWeapons()
	local i
	for i = 0, 17 do
		ply:TakeForce(i)
	end
	if pack.Type == 1 then
		for k,v in pairs(pack.Weapons) do
			ply:GiveWeapon(v[1])
			ply:SetClipAmmo(v[1], v[2])
		end
		for k,v in pairs(pack.Ammo) do
			ply:SetAmmo(v[1], v[2])
		end
		ply:SetWeapon(WP_BLASTER)
	elseif pack.Type == 2 then
		for k,v in pairs(pack.Forces) do
			ply:GiveForce(v[1])
			ply:SetForceLevel(v[1], v[2])
		end
		ply:GiveWeapon(WP_SABER)
		ply:GiveWeapon(WP_MELEE)
		ply:SetWeapon(WP_MELEE)
	end
	ply:SetHealth(100)
	ply:SetArmor(100)
	ply:SendCenterPrint("^7You have obtained the\n^1" .. pack.Name .. "\n^7loadout\n\nYour health and armor are restored to maximum")
end

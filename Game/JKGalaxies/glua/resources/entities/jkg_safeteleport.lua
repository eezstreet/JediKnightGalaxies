-- Safe teleportation entity. Will delay teleportation until the target area is safe

ENT.ClassName = "jkg_safeteleport"
ENT.Type = "logical"
ENT.Base = "base_logical"

function ENT:OnSpawn()
	self.Owner = tonumber(self:GetSpawnVar("owner"))
	self.OwnerEnt = ents.GetByIndex(self.Owner)
	if not self.OwnerEnt:IsPlayer() and not self.OwnerEnt:IsNPC() then
		self:Free()	-- We can only teleport players or npcs, nothing else
		return
	end
	local temp = self:GetSpawnVar("origin")
	if not temp then
		self:Free()	-- No target = no go
		return
	end
	self.Target = Vector(temp)
	self.SelfDestruct = false
	self:SetNextThink(50)
end

function ENT:OnThink()
	if self.SelfDestruct then
		self:Free()
		return
	end
	if not sys.SpotWouldTelefrag(self.OwnerEnt, self.Target) then
		-- Time to go
		if self.OwnerEnt:IsPlayer() then
			self.OwnerEnt:ToPlayer():Teleport(self.Target)
		elseif self.OwnerEnt:IsNPC() then
			self.OwnerEnt:ToNPC():Teleport(self.Target)
		end
		self.SelfDestruct = true
	end
	self:SetNextThink(50)
end

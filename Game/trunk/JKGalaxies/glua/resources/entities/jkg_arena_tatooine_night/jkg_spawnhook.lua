-- Safe teleportation entity. Will delay teleportation until the target area is safe

ENT.ClassName = "jkg_spawnhook"
ENT.Type = "logical"
ENT.Base = "base_logical"

function ENT:OnSpawn()
	
end

function ENT:OnUse(other, activator)
	if not self.HookFunc then
		return
	end
	self.HookFunc(activator)
end

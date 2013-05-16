ENT.ClassName = "misc_dlgrunner"
ENT.Type = "generic"
ENT.Base = "base_generic"

function ENT:OnSpawn()
	self:SetPos( Vector(self:GetSpawnVar("origin") or "0 0 0") )
	self:SetAngles( Vector(self:GetSpawnVar("angles") or "0 0 0") )
	self:SetModel("models/map_objects/h_evil/control_station.md3")
	self:AutoBox()
	
	-- Set timeout for the use to 0, so we can use it right now
	self.UseTimeout = 0
	self.DlgToUse = self:GetSpawnVar("dlg")
	-- Make the entity solid
	self:SetContents(CONTENTS_SOLID)
	self:SetClipmask(MASK_SOLID)
	-- Make the entity usable
	self:SetPlayerUsable(true)
	-- Put this entity in the world
	self:LinkEntity()
end

function ENT:OnUse(other, activator)
	-- If we're in a timeout, ignore the use for now
	if self.UseTimeout > sys.Time() then
		return
	end
	self.UseTimeout = sys.Time() + 1000
	local ply = activator:ToPlayer()
	
	local dlg = dialogue.CreateDialogueObject(self.DlgToUse)
	if not dlg then
		print("ERROR: Cannot find dialogue '", self.DlgToUse, "'")
		return
	end
	printf("Running dialogue %s...", self.DlgToUse)
	dlg:RunDialogue(self, ply)
end

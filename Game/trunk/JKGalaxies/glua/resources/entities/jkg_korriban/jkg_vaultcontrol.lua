ENT.ClassName = "jkg_vaultcontrol"
ENT.Type = "logical"
ENT.Base = "base_logical"

function ENT:OnSpawn()
	self.UseTimeout = 0
	self.IsOutside = self:GetSpawnVar("outside") == "1"
end

function ENT:OnUse(other, activator)
	-- If we're in a timeout, ignore the use for now
	if self.UseTimeout > sys.Time() then
		return
	end
	self.UseTimeout = sys.Time() + 1000
	local ply = activator:ToPlayer()
	
	local dlg = dialogue.CreateDialogueObject("gmh_vaultcontrol")
	if not dlg then
		print("ERROR: Cannot find dialogue 'gmh_vaultcontrol'")
		return
	end
	dlg:RunDialogue(self, ply)
end

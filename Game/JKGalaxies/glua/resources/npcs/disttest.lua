NPC.NPCName = "disttest"

function NPC:OnSpawn()
	print("Test NPC OnSpawn")
	self:SetUseRange(128)
end

function NPC:OnUse(other, activator)
	print(string.format("Test NPC OnUse (Used by %s)", tostring(activator)))
end

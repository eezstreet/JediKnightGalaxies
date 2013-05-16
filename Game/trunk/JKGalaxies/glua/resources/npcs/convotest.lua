NPC.NPCName = "convotest"

function NPC.ConvoResponse(convo, ply, convoid, resp, resptag)
	local self = convo:GetUserData()
	if convoid == 0 then
		-- Initial convo
		if resp == 1 then
			-- Who are you?
			convo:AddText("Well, i'm a rebel soldier, and who might you be?", 4000)
			convo:SetConvoID(1)
			convo:AddChoices(self.ConvoResponse, {"I'm " .. ply:GetName(), "I have to go"}, {1, 2})
			convo:RunConvo(ply)
		else
			convo:AddText("Goodbye", 2000)
			convo:RunConvo(ply)
		end
	elseif convoid == 1 then
		if resptag == 1 then
			convo:AddText("Nice to meet you " .. ply:GetName(), 2000)
			convo:RunConvo(ply)
		else
			convo:AddText("Goodbye", 2000)
			convo:RunConvo(ply)
		end
	else
		print("Error")
	end
end

function NPC:OnUse(other, activator)
	if not activator:IsPlayer() then
		return
	end
	if self:Health() < 1 then
		return	-- Cant talk to me when i'm dead
	end
	local convo = sys.CreateConversation()
	convo:AddText("Hello there!", 3000)
	convo:AddChoices(self.ConvoResponse, {"Who are you?", "I have to go"}, {1,2})
	convo:SetUserData(self)
	convo:SetConvoID(0)
	convo:RunConvo(activator:ToPlayer())
end
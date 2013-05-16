-- WARNING: BROKEN - DO NOT USE

NPC.NPCName = "tuskenconvo"

function NPC.ConvoResponse(convo, ply, convoid, resp, resptag)
	local self = convo:GetUserData()
	if not self:IsValid() then
		convo:RunConvo(ply)
	end
	local responses
	if convoid == 0 then
		-- Initial convo
		if resp == 1 then
			-- Who are you?
			convo:AddText("You slaughtered my entire colony, what can you possibly offer me", 3000)
			responses = { "If you let the jawas go, I'll let you live and give you these 500 Credits",
						  "If you let the jawas go, I'll promise not to kill you",
						  "If you let the jawas go, I'll promise not to kill you [Lie]",
						  "I changed my mind, time to die!"
						}
			convo:SetConvoID(1)
			convo:AddChoices(self.ConvoResponse, responses)
			convo:RunConvo(ply)
		elseif resp == 2 then
			convo:AddText("Are you trying to anger me?", 2000)
			responses = { "Yes, and I bet you dare not press that button.",
						  "No, no! I am sorry, I'm sure we can solve this peacefully."
						}
			convo:SetConvoID(2)
			convo:AddChoices(self.ConvoResponse, responses)
			convo:RunConvo(ply)
		elseif resp == 3 then
			convo:AddText("Are you ^x2CFthreatening ^x0AFme?", 2000)
			responses = { "Let the jawas go now, or I'll cut you into tiny little pieces!",
						  "Forget about the jawas, you're mine!",
						  "No, not at all, sorry! I'm sure we can handle this peacefully!"
						}
			convo:SetConvoID(3)
			convo:AddChoices(self.ConvoResponse, responses)
			convo:RunConvo(ply)
		end
	elseif convoid == 1 then
		if resp == 1 then
			convo:AddText("That is a tempting offer, " .. sys.StripColorcodes(ply:GetName()) .. " you have a deal there", 4000)
			local function func(convo, ply)
				local npc = convo:GetUserData()
				ply:SendChat("^4You gained 200 lightside points")
				ply:SendChat("^x3F3*The tusken runs off and the jawa's are freed*")
				local angs = npc:GetAngles()
				angs:ToForward()
				local navgoal = npc:GetPos() + (angs * -500) -- 500 units backwards
				npc:SetNavGoal(navgoal)
				function npc:OnReached()
					-- DANGER: NEVER USE :GetEntity():Free() ON NPCS!!!
					-- THIS WILL CRASH THE SERVER!
					-- ALWAYS USE :Remove()
					self:Remove()
				end
				function npc:OnStuck()	-- Just to ensure the npc will always go poof
					self:Remove()
				end
				npc.CanTalk = false
			end
			convo:SetFinishCallback(func)
			convo:RunConvo(ply)
		elseif resp == 2 then
			convo:AddText("A life like this is not worth living!", 2000)
			local function func(convo, ply)
				local npc = convo:GetUserData()
				ply:SendChat("^x3F3*The tusken walks away*")
				local angs = npc:GetAngles()
				angs:ToForward()
				local navgoal = npc:GetPos() + (angs * -500) -- 500 units backwards
				npc:SetNavGoal(navgoal)
				function npc:OnReached()
					self:Remove()
				end
				function npc:OnStuck()
					self:Remove()
				end
				npc.CanTalk = false
			end
			convo:SetFinishCallback(func)
			convo:RunConvo(ply)
		elseif resp == 3 then
			convo:AddText("Maybe living a half life is better than no life, I will take your offer", 4000)
			convo:AddText("^xF33*As the tusken tries to leave, you kill him*", 3000)
			local function func(convo, ply)
				local npc = convo:GetUserData()
				ply:SendChat("^xF33*The tusken was killed*")
				npc:Kill()
			end
			convo:SetFinishCallback(func)
			convo:RunConvo(ply)
		elseif resp == 4 then
			convo:AddText("Then these jawas die with me!", 2000)
			local function func(convo, ply)
				local npc = convo:GetUserData()
				ply:SendChat("^xF33*The tusken blows up the jawas*")
				npc:SetBehaviorState("BS_DEFAULT")
				npc:SetEnemy(ply:GetEntity())
				npc.CanTalk = false
			end
			convo:SetFinishCallback(func)
			convo:RunConvo(ply)
		end
	elseif convoid == 2 then
		if resp == 1 then
			convo:AddText("^xF33*Tusken blows up jawas and attack you*", 2000)
			local function func(convo, ply)
				local npc = convo:GetUserData()
				ply:SendChat("^xF33*The tusken blows up the jawas*")
				npc:SetEnemy(ply:GetEntity())
				npc:SetGodMode(false)
				npc:SetBehaviorState("BS_DEFAULT")
				npc.CanTalk = false
			end
			convo:SetFinishCallback(func)
			convo:RunConvo(ply)
		elseif resp == 2 then
			-- Forward to another convo option
			return self.ConvoResponse(convo, ply, 0, 1, nil)
		end
	elseif convoid == 3 then
		if resp == 1 then
			convo:AddText("If you spare me, I'll let the jawa's go! Please!", 3000)
			responses = { "Ok, then get out of here",
						  "No way! You're not going anywhere… Time to die!",
						}
			convo:SetConvoID(4)
			convo:AddChoices(self.ConvoResponse, responses)
			convo:RunConvo(ply)
		elseif resp == 2 then
			convo:AddText("^xF33*The tusken is killed, along with the jawas*", 1000)
			local function func(convo, ply)
				local npc = convo:GetUserData()
				ply:SendChat("^xF33*The tusken is killed, along with the jawas*")
				npc:Kill()
			end
			convo:SetFinishCallback(func)
			convo:RunConvo(ply)
		elseif resp == 3 then
			-- Forward to another convo option
			return self.ConvoResponse(convo, ply, 0, 1, nil)
		end
	elseif convoid == 4 then
		if resp == 1 then
			convo:AddText("^x3F3*Tusken runs off*", 1000)
			local function func(convo, ply)
				local npc = convo:GetUserData()
				ply:SendChat("^x3F3*The tusken runs away*")
				local angs = npc:GetAngles()
				angs:ToForward()
				local navgoal = npc:GetPos() + (angs * -500) -- 500 units backwards
				npc:SetNavGoal(navgoal)
				function npc:OnReached()
					self:Remove()
				end
				function npc:OnStuck()
					self:Remove()
				end
				npc.CanTalk = false
			end
			convo:SetFinishCallback(func)
			convo:RunConvo(ply)
		elseif resp == 2 then
			convo:AddText("^xF33*Tusken blows up jawas and attack you*", 2000)
			local function func(convo, ply)
				local npc = convo:GetUserData()
				ply:SendChat("^xF33*The tusken blows up the jawas*")
				npc:SetEnemy(ply:GetEntity())
				npc:SetBehaviorState("BS_DEFAULT")
				npc:SetGodMode(false)
				npc.CanTalk = false
			end
			convo:SetFinishCallback(func)
			convo:RunConvo(ply)
		end
	else
		print("INTERNAL ERROR: Invalid convoid!")
	end
end

function NPC:OnSpawn()
	self:SetBehaviorState("BS_CINEMATIC")
	self:SetWalkSpeed(125)
	self:SetRunSpeed(250)
	self:SetWalking(false)
	self:SetRunning(true)
	self.CanTalk = true
	self.InUse = false
	self.LastUse = 0
end

function NPC:OnUse(other, activator)
	-- If the npc cant talk, ignore
	if not self.CanTalk then return end
		
	-- If the activator is not a player, ignore
	if not activator:IsPlayer() then
		return
	end
	
	if self:Health() < 1 then
		return	-- Cant talk to me when i'm dead
	end
	
	-- If we used him within the last 500 msecs, ignore
	if sys.Time() - self.LastUse < 500 then return end
	self.LastUse = sys.Time()
	
	-- If already in a convo, ignore others (for now)
	if self.InUse then
		activator:ToPlayer():SendChat("This NPC is currently busy talking")
		return
	end
	self.InUse = true
	
	self:SetWatchTarget(activator)
	self:SetGodMode(true)

	-- Start the convo
	local convo = sys.CreateConversation()
	convo:AddText("Move one step closer and I'll blow up these jawas!", 3000)
	local responses = { "Don't do anything irrational, I'm sure we can work something out! [Intelligence]",
						"I dare you to press that button, coward!",
						"If you press that button, I'll make sure you die a very slow, painful death. [Intimidation]"
						}
	convo:AddChoices(self.ConvoResponse, responses)
	convo:SetUserData(self)
	convo:SetConvoID(0)
	convo:RunConvo(activator:ToPlayer())
end

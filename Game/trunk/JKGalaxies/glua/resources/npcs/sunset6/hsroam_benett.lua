--[[ ------------------------------------------------
	Jedi Knight Galaxies NPC
	Quest NPC
	
	Quest: Homestead alpha quest: Tusken Troubles
	
	This script is for benett himself
	
	Quest script by DarthLex
	Written by BobaFett
--------------------------------------------------]]


NPC.NPCName = "hsroam_benett"


function NPC:OnSpawn()
	-- Initial setup
	-- Prevent the AI from messing things up
	self:SetBehaviorState("BS_CINEMATIC") 
	-- Give it a pistol as weapon
	self:GiveWeapon(WP_MELEE)
	self:SetWeapon(WP_MELEE)
	-- Godmode it, so they dont get killed
	self:SetGodMode(true)
	-- Explicitly tell it to walk and not run
	self:SetWalking(true)
	self:SetRunning(false)	
	-- Prevent him from targetting anyone
	self:SetLookForEnemies(false)
	self:SetChaseEnemies(false)
	-- Register this NPC in its controller
	self.Controller:RegisterNPC(self, 0)
	-- Set up local variables
	self.CanTalk = true
	self.InUse = false
	self.LastUse = 0
	-- Make him face the center of the area
	self:SetViewTarget(Vector(360, -15, -295))
	self:SetAnimBoth("BOTH_STAND1IDLE1")
	self:SetAnimHoldTime(3, -1)	-- SetAnimHoldTime(SETANIM_BOTH, INFINITE)
end

function NPC:OnAnger()
	-- Clear his enemy, we dont want his combat AI to kick in
	self:SetEnemy(nil)
end

function NPC:OnInit(spawner)
	if not spawner.Controller then
		-- If spawner.Controller is not set, then the spawner is not linked to a controller
		-- That would mean we get no nav route either, so dont bother spawning in that case
		self:Remove()
		return
	end
	self.Spawner = spawner
	self.Controller = spawner.Controller
	self.NPCID = tonumber(spawner:GetSpawnVar("npcid"))
end

function NPC:OnUse(other, activator)
	if sys.Time() - self.LastUse < 500 then
		return
	end
	self.LastUse = sys.Time()
	
	if not self.CanTalk then return end
	
	if not activator:IsPlayer() then
		return		-- Only talk to players, nothin else
	end
	
	local ply = activator:ToPlayer()
	
	if self.InUse then
		ply:SendChat("^7This NPC is already speaking to someone else")
		return
	end
	-- Obtain the quest info, if set
	self:SetViewTarget(activator)
	self.InUse = true
	
	local state = 0
	
	if not ply.Quests then
		ply.Quests = {}
	end
	if ply.Quests["tuskentroubles"] then
		state = ply.Quests["tuskentroubles"].state
	end
	if state == 0 then
		-- Initial state, meaning we havent talked to the outside npcs yet
		-- Create convo object
		local convo = sys.CreateConversation()
		-- Set userdata to ourself, so we can reference ourselve in convo responses
		convo:SetUserData(self)
		-- Set the callback function (only has to be set once, as it's not cleared when processing responses)
		convo:SetChoiceCallback(self.ConvoResponse)
		
		-- Set the convo ID
		convo:SetConvoID(0)
		-- Add text for the convo to show (text, hold time)
		convo:AddText("H-Hi there, I-I m Bennett, what c-can I h-help you with, s-sir?", 5000)
		
		convo:AddChoice("Can you tell me anything interesting about this place?", 3)
		convo:AddChoice("I need to leave, bye.", 4)
		-- Execute the conversation on the player that used me
		convo:RunConvo(ply)
		return
	elseif state == 1 or state == 2 then
		local convo = sys.CreateConversation()
		-- Set userdata to ourself, so we can reference ourselve in convo responses
		convo:SetUserData(self)
		-- Set the callback function (only has to be set once, as it's not cleared when processing responses)
		convo:SetChoiceCallback(self.ConvoResponse)
		
		-- Set the convo ID
		convo:SetConvoID(0)
		if not ply.Quests["tuskentroubles"].talkedToBenett then
			ply.Quests["tuskentroubles"].talkedToBenett = true
			convo:AddText("H-Hi there, I-I m Bennett, what c-can I h-help you with, s-sir?", 5000)
		else
			convo:AddText("W-what d-do you w-want n-n-now?", 3000)
		end

		convo:AddChoice("You're in charge of the security system?", 1)
		convo:AddChoice("What's wrong with you?", 2)
		convo:AddChoice("Can you tell me anything interesting about this place?", 3)
		convo:AddChoice("I need to leave, bye.", 4)
		
		convo:RunConvo(ply)
		return
	elseif state == 3 then
		local convo = sys.CreateConversation()
		convo:SetUserData(self)
		convo:SetChoiceCallback(self.ConvoResponse)
		convo:SetConvoID(3)

		convo:AddText("Y-you again! W-w-what do you w-want this t-time?", 3000)
		
		convo:AddChoice("Quit the yapping Bennett! I know you've been lying!", 1)
		convo:AddChoice("I need you to tell me the truth if I am to help.", 2)
		convo:AddChoice("I have no desire to talk to you", 3)
		convo:AddChoice("It's really a pity with Siran.", 4)
		convo:AddChoice("I need you to tell me the truth Bennett, If I am to find your son.", 5)
		
		convo:RunConvo(ply)
		return
	end
	-- TODO: The rest..
end

function NPC.ConvoResponse(convo, ply, convoid, resp, resptag)
	-- We got a response to a conversation, first check out which convo it was (based on convo ID)
	-- Self is the NPC this is running on, since we stored it as userdata before, we can retreive it like this
	local self = convo:GetUserData()

	-- Arguments:
	-- convo: Conversation object used earlier, already primed up to contain new data. It's callbacks havent been reset, so you don have to set them again
	-- ply: Player that this conversation is running on
	-- convoid: Convo ID provided earlier
	-- resp: Response chosen, from 1 to the amount of responses
	-- resptag: Tag associated with the selected response, if any (mainly useful for conversations that have dynamic options)
	
	if convoid == 0 then
		-- Initial convo ("H-Hi there, I-I m Bennett, what c-can I h-help you with, s-sir?")
		if resptag == 1 then
			-- First response: 'You're in charge of the security system?'
			if ply.Quests["tuskentroubles"].state == 1 then
				ply.Quests["tuskentroubles"].state = 2	-- We talked to benett ^^
			end
			convo:SetConvoID(1)
			convo:AddText("Y-yup that's me.", 2000)
			convo:AddText("B-but you can't blame me for the errors lately!", 3000)
			convo:AddText("A-all my tools are gone, and the d-defensive turrets are simply irreparable!", 6000)
			
			convo:AddChoice("Got any idea where your tools could be?", 1)
			convo:AddChoice("What do I need to fix the turrets?", 2)
			convo:AddChoice("Is there anything else you can tell me?", 3)
			convo:AddChoice("I'm clearly wasting my time with you.", 4)
			convo:AddChoice("Could we go back to my other questions?", 5)
			
			convo:Continue()
			return
		elseif resptag == 2 then
			-- Second response: 'What's wrong with you?'
			convo:SetConvoID(2)
			convo:AddText("N-nothing's wrong! W-why would there b-be anything w-wrong?", 4000)
			
			convo:AddChoice("Nothing, I'll be back later")
			convo:AddChoice("You look really ugly!")
			convo:AddChoice("You're stuttering really badly, huh?")
			
			convo:Continue()
			return
		elseif resptag == 3 then
			-- Third response: 'Can you tell me anything interesting about this place?'
			convo:AddText("W-what.. d-does it l-look l-like I'm a t-tour guide t-to you? W-what do ya w-want?!", 7000)
			self.RestartConvo(convo, ply)
			return
		elseif resptag == 4 then
			-- Fourth response: 'No, I don't need your help.'
			convo:AddText("Then g-get away f-from m-me, a-already.", 3000)
			convo:SetFinishCallback(self.OnConvoEnd)
			convo:Continue()
			return
		end
	elseif convoid == 1 then
		-- Convo #1, this is the "Yea i'm benett, but i lost my tools' convo
		if resptag == 1 then
			-- First response: 'Got any idea where your tools could be?'
			convo:AddText("N-no! W-why would I know where m-my tools were, when s-someone else t-took them?", 6000)
			convo:AddText("L-leave me alone with the q-questioning will ya!?", 4000)
			
			convo:AddChoice("What do I need to fix the turrets?", 2)
			convo:AddChoice("Is there anything else you can tell me?", 3)
			convo:AddChoice("I'm clearly wasting my time with you.", 4)
			convo:AddChoice("Could we go back to my other questions?", 5)
			
			convo:Continue()
			return
		elseif resptag == 2 then
			-- Second response: 'What do I need to fix the turrets?'
			convo:AddText("You'd n-need a pair of new t-targeting p-processors, b-but the ones I had,\nw-were s-stolen, so there's n-nothing to d-d-do.", 10000)
			
			convo:AddChoice("Got any idea where your tools could be?", 1)
			convo:AddChoice("Is there anything else you can tell me?", 3)
			convo:AddChoice("I'm clearly wasting my time with you.", 4)
			convo:AddChoice("Could we go back to my other questions?", 5)
			
			convo:Continue()
			return
		elseif resptag == 3 then
			-- Third response: 'Is there anything else you can tell me?'
			convo:AddText("N-no, n-nothing! C-Can't you just l-leave me a-alone?!", 4000)
						
			convo:AddChoice("Got any idea where your tools could be?", 1)
			convo:AddChoice("What do I need to fix the turrets?", 2)
			convo:AddChoice("I'm clearly wasting my time with you.", 4)
			convo:AddChoice("Could we go back to my other questions?", 5)
			
			convo:Continue()
			return
		elseif resptag == 4 then 
			-- Fourth response: 'I'm clearly wasting my time with you.'
			convo:AddText("Then g-get away f-from m-me, a-already.", 3000)
			convo:SetFinishCallback(self.OnConvoEnd)
			convo:Continue()
			return
		elseif resptag == 5 then
			-- Fifth response: 'Could we go back to my other questions?'
			convo:AddText("W-what d-do you w-want n-n-now?", 3000)
			self.RestartConvo(convo, ply)
			return
		end
	elseif convoid == 2 then
		-- Convo #2, this is the 'Is there anything wrong?' convo
		if resp == 1 then
			-- First Response:  "Nothing, I'll be back later'
			convo:AddText("Then g-get away f-from m-me, a-already.", 3000)
			convo:SetFinishCallback(self.OnConvoEnd)
			convo:Continue()
			return
		elseif resp == 2 then
			-- Second Response: 'You look really ugly!'
			convo:AddText("A-are you t-trying to be f-funny? Q-quit wasting my t-time.", 5000)
			convo:SetFinishCallback(self.OnConvoEnd)
			convo:Continue()
			return
		elseif resp == 3 then
			-- Third Response: 'You're stuttering really badly, huh?'
			convo:AddText("Yeah eh... I- I've been d-doing that all my l-life you know...", 5000)
			convo:AddText("Y-yeah, I have! Now l-leave me a-alone will ya!?", 4000)
			convo:SetFinishCallback(self.OnConvoEnd)
			convo:Continue()
			return
		end
	elseif convoid == 3 then
		-- Convo after his lies have been unconvered
		if resptag == 1 then
			-- First response: 'Quit the yapping Bennett! I know you've been lying!'
			convo:AddText("I-I have n-no idea w-what you're t-t-talking about...", 3500)
			convo:AddText("D- Don't you dare c-calling me l-liar!", 3000)
			convo:SetFinishCallback(self.OnConvoEnd)
			convo:Continue()
			return
		elseif resptag == 2 then
			-- Second response: 'I need you to tell me the truth if I am to help."
			convo:AddText("W-why d-do you keep b-bothering me with this?!", 3500)
			convo:AddText("I t-told you a-all I k-know!", 3000)
			
			-- I'm fusing multiple conversations into 1 using tags, so we start at 6, rather than 1
			convo:AddChoice("Well, then let's talk about something else.", 6)
			convo:AddChoice("If you don't tell me, I can't help you with Siran...", 4)
			convo:AddChoice("Who cares about your problems anyhow, I'm out of here.", 7)
			convo:Continue()
			return
		elseif resptag == 3 then
			-- Third response: 'I have no desire to talk to you'
			convo:AddText("W-would you l-leave me alone t-then a-already then!?", 3500)
			convo:SetFinishCallback(self.OnConvoEnd)
			convo:Continue()
			return
		elseif resptag == 4 then
			-- Fourth response: 'It's really a pity with Siran.'
			-- Subconvo 1 - Second response response: 'If you don't tell me, I can't help you with Siran...'
			convo:AddText("S-Siran... M-y little b-boy. [Bennett sniffles and tries not to cry]", 5000) 
			convo:AddText("L-Look, it's n-not because I WANTED this. [Sniffles]", 4000)
			
			convo:AddChoice("Look, maybe I can help you, why don't you tell me what happened", 8)
			convo:AddChoice("Maybe we should take this later.", 7)
			
			convo:Continue()
			return
		elseif resptag == 5 then
			-- Fifth response: 'I need you to tell me the truth Bennett, If I am to find your son.'
			convo:AddText("I-I-I It's... It's... N-no. NO, I c-can't t-tell you anything. L-Leave me alone!", 6000)
			convo:SetFinishCallback(self.OnConvoEnd)
			convo:Continue()
			return
		elseif resptag == 6 then
			-- Subconvo 1 - First response: 'Well, then let's talk about something else.'
			convo:AddText("Well, then let's talk about something else.", 0)
			
			convo:AddChoice("I need you to tell me the truth if I am to help.", 2)
			convo:AddChoice("I have no desire to talk to you", 3)
			convo:AddChoice("It's really a pity with Siran.", 4)
			convo:AddChoice("I need you to tell me the truth Bennett, If I am to find your son.", 5)
			
			convo:Continue()
			return
		elseif resptag == 7 then
			-- Subconvo 1 - Third response: 'Who cares about your problems anyhow, I'm out of here.'
			-- Subconvo 2 - Second response: 'Maybe we should take this later.'
			convo:SetFinishCallback(self.OnConvoEnd)
			convo:Continue()
			return
		elseif resptag == 8 then
			-- Subconvo 2 - First response: 'Look, maybe I can help you, why don't you tell me what happened'
			convo:AddText("[^7PLACEHOLDER^x0AF] Confession [^7/PLACEHOLDER^x0AF]", 5000)
			convo:SetFinishCallback(self.OnConvoEnd)
			convo:Continue()
			return
		end
	end
	-- TODO: The rest
end

function NPC.RestartConvo(convo, ply)
	-- Brings ya back to the original convo, to avoid having to type it several times up in there
	local state = 0
	if ply.Quests["tuskentroubles"] then
		state = ply.Quests["tuskentroubles"].state
	end
	
	convo:SetConvoID(0)
	-- Return to the intial convo
	if state == 1 or state == 2 then
		convo:AddChoice("You're in charge of the security system?", 1)
		convo:AddChoice("What's wrong with you?", 2)
	end
	convo:AddChoice("Can you tell me anything interesting about this place?", 3)
	convo:AddChoice("I need to leave, bye.", 4)
	convo:Continue()
end

function NPC.OnConvoEnd(convo, ply)
	-- This is a finish callback for the convo system
	-- If this function returns true, it'll override the default behavior and keeps the convo active
	-- If anything else (or nothing) is returned, the conversation will be ended once this function finishes
	self = convo:GetUserData()
	self.InUse = false
	self:SetViewTarget(Vector(360, -15, -295))
end

function NPC:OnDie(inflictor, attacker, damage, meansofdeath)
	-- We got killed -.-.. probably some rogue npc kill all
	-- Inform the controller so we can be respawned in a few moments
	self.Controller:NPCKilled(self, self.NPCID)
end

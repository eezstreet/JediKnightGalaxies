--[[ ------------------------------------------------
	Jedi Knight Galaxies NPC
	Quest NPC
	
	Quest: Homestead alpha quest: Tusken Troubles
	
	This script is used for the roamers outside, it'll make them walk their waypoints and talk to the player
	
	Quest script by DarthLex
	Written by BobaFett
--------------------------------------------------]]


NPC.NPCName = "hsroam_normal"


function NPC:OnSpawn()
	-- Initial setup
	-- Give it default AI for now, we use the OnAnger event to stop combat AI
	self:SetBehaviorState("BS_DEFAULT") 
	-- Give it a pistol as weapon
	self:GiveWeapon(WP_BRYAR_PISTOL)
	self:SetWeapon(WP_BRYAR_PISTOL)
	-- Godmode it, so they dont get killed
	self:SetGodMode(true)
	-- Explicitly tell it to walk and not run
	self:SetWalking(true)
	self:SetRunning(false)	
	-- Prevent him from targetting anyone
	self:SetLookForEnemies(false)
	self:SetChaseEnemies(false)
	-- Register this NPC in its controller
	self.Controller:RegisterNPC(self, self.NPCID)
	-- Obtain the navigation route
	self.Waypoints = self.Controller:GetNavRoute(self.NPCID)
	-- Set up local variables
	self.CurrWaypoint = 1
	self.CanTalk = true
	self.InUse = false
	self.LastUse = 0
	-- Lets get started on the movement
	self:Roam()
end

function NPC:OnAnger()
	-- Clear his enemy, we dont want his combat AI to kick in
	self:SetEnemy(nil)
end

function NPC:Roam()
	self.SuppressWalk = false
	-- Go towards the next waypoint
	self:SetNavGoal(self.Waypoints[self.CurrWaypoint].Origin)
end

function NPC:NextWaypoint()
	if not self:IsValid() then
		return	-- Since this function is called by a timer, ensure we bail if we got deleted before the timer fired
	end
	self.CurrWaypoint = self.CurrWaypoint + 1
	if self.CurrWaypoint > # self.Waypoints then
		self.CurrWaypoint = 1
	end

	if self.SuppressWalk then
		return	-- If we're talking to a player, we should update the waypoint, but not talk right away. The npc will run NPC:Roam() when the convo finishes
	end
	self:Roam()
end

function NPC:StopRoam()
	-- Stop moving, do this when a player wants to talk to us
	self.SuppressWalk = true
	self:SetNavGoal(nil)
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
	self:StopRoam()
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
		-- Initial state, first convo, or we havent proceeded yet
		-- Create convo object
		local convo = sys.CreateConversation()
		-- Set userdata to ourself, so we can reference ourselve in convo responses
		convo:SetUserData(self)
		-- Set the callback function (only has to be set once, as it's not cleared when processing responses)
		convo:SetChoiceCallback(self.ConvoResponse)
		
		-- Set the convo ID
		convo:SetConvoID(0)
		-- Add text for the convo to show (text, hold time)
		convo:AddText("What are we going to do... we will all die! Oh, what are we going to do?", 5000)
		-- Add our options (and optionally a tag, but that is omitted here)
		-- First option will be #1, second #2, etc
		convo:AddChoice("Get your act together and spit it out before I cut you in pieces.")
		convo:AddChoice("Calm down, Maybe I can help. What's the problem?")
		convo:AddChoice("You bore me, bye.")
		-- Execute the conversation on the player that used me
		convo:RunConvo(ply)
		return
	elseif state == 1 then
		local convo = sys.CreateConversation()
		convo:SetUserData(self)
		convo:SetFinishCallback(self.OnConvoEnd)
		
		if self.NPCID == ply.Quests["tuskentroubles"].talkid then
			-- The player initially talked to me
			convo:AddText("You should talk to benett.", 2000)
			convo:RunConvo(ply)
		else
			-- The player initially talked to someone else, use a generic line
			if math.random(0,1) > 0.5 then
				convo:AddText("I don't want to die!", 2000)
			else
				convo:AddText("What are we going to do?!", 2000)
			end
			convo:RunConvo(ply)
		end
		return
	elseif state == 2 or state == 3 then
		-- We already talked to benett
		local convo = sys.CreateConversation()
		convo:SetUserData(self)
		convo:SetChoiceCallback(self.ConvoResponse)
		convo:SetConvoID(2)
		convo:AddText("Why are you bothering me now? I have to find somewhere to hide!", 5000)
		convo:AddChoice("Have you seen some tools around here somewhere?", 1)
		convo:AddChoice("Did you notice anything suspicious going on lately?", 2)
		convo:AddChoice("I'm sorry, I'll leave now.", 3)
		
		convo:RunConvo(ply)
		return
	end
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
		-- Our initial conversation ("What are we going to do… we will all die! Oh, what are we going to do?")
		-- Check which response the player chose
		if resp == 1 or resp == 2 then
			-- First response: 'Get your act together and spit it out before I cut you in pieces.'
			-- or Second response: 'Calm down, Maybe I can help. What's the problem?'
			
			-- Just put in new text and options, set a new convo ID, and run it
			convo:SetConvoID(1)
			convo:AddText("I-I, I'm sorry... It's just the tuskens!\nThose damn beasts have been raiding this place daily for almost a week now!", 6000)
			-- Since these options can change later on, we fit them with a tag
			convo:AddChoice("Why would they attack you?", 1)
			convo:AddChoice("Who can I talk to, about helping with this problem?", 2)
			-- While these two choices are different, they have the same result, so to make things easier, we give them the same tag
			convo:AddChoice("If you are too weak to fight, you deserve it. Bye.", 0)
			convo:AddChoice("Maybe I'll be back later", 0)
			-- Continue the conversation
			convo:Continue()
			return
		elseif resp == 3 then
			-- Third response: 'You bore me, bye.'
			-- This time, we just want to show a message and stop the convo, so we just put in some new text and thats it
			-- If no options are provided, instead of showing them, it will execute its finish callback and then stop the conversation
			-- (Unless the callback overrides that)
			convo:AddText("No one can help us anyways! We're doomed, doomed I tell you!", 4000)
			-- Tell the convo system to run NPC.OnConvoEnd when  we're finished
			convo:SetFinishCallback(self.OnConvoEnd)
			convo:Continue()
			return
		end
	elseif convoid == 1 then
		-- Second converation ("I-I, I'm sorry… It's just the tuskens! Those damn beasts have been raiding this place daily for almost a week now!”)
		-- Responses are tagged, so check resptag, not resp itself
		if resptag == 0 then
			-- Latter 2 responses, which basically are a 'goodbye' worded differently
			convo:AddText("No one can help us anyways! We're doomed, doomed I tell you!", 4000)
			-- Tell the convo system to run NPC.OnConvoEnd when  we're finished
			convo:SetFinishCallback(self.OnConvoEnd)
			convo:Continue()
			return
		elseif resptag == 1 then
			-- First response: 'Why would they attack you?'
			convo:AddText("Cause they are beasts, Savage animal-like beasts I tell you!", 4000)
			-- Remove the first option and then display it again
			convo:AddChoice("Who can I talk to, about helping with this problem?", 2)
			convo:AddChoice("If you are too weak to fight, you deserve it. Bye.", 0)
			convo:AddChoice("Maybe I'll be back later", 0)
			convo:Continue()
			return
		elseif resptag == 2 then
			-- Second response: 'Who can I talk to, about helping with this problem?'
			convo:AddText("You should talk to Bennett.", 2000)
			convo:AddText("He's in charge of maintaining the security around here.", 3000)
			convo:AddText("But lately it seems he isn't even doing his job!", 3000)
			convo:SetFinishCallback(self.GiveQuest)
			convo:Continue()
			return			
		end
	elseif convoid == 2 then
		if resptag == 1 then
			-- First response: 'Have you seen some tools around here somewhere?'
			convo:AddText("Why are you bothering me with a question like that?", 3000)
			convo:AddText("The only one who would know where the tools are, is Bennett so go ask him!", 5000)
			
			convo:AddChoice("Did you notice anything suspicious going on lately?", 2)
			convo:AddChoice("I'm sorry, I'll leave now.", 3)
			
			convo:Continue()
			return
		elseif resptag == 2 then
			-- Second response: 'Did you notice anything suspicious going on lately?'
			convo:AddText("NO I haven't and would...", 2000)
			convo:AddText("Wait, yes, now that you say it, for the past week I haven't seen\nBennett's kid Siran running around.", 6000)
			convo:AddText("He usually plays around in the halls.", 3000)
			convo:AddText("Now that I come to think of it, he disappeared about the same time\nBennett suddenly started stuttering!", 6000)
			
			convo:AddChoice("He didn't always stutter like that?", 4)
			convo:Continue()
			return
		elseif resptag == 3 then
			-- Third response: 'I'm sorry, I'll leave now.'
			convo:SetFinishCallback(self.OnConvoEnd)
			convo:Continue()
			return
		elseif resptag == 4 then
			-- Special response: 'He didn't always stutter like that?'
			-- When we get here, we finished the quest as far as it's done now
			ply.Quests["tuskentroubles"].state = 3
			convo:AddText("Stutter? Bennett? He used to be as confident and stubborn as a bantha!", 4000)
			convo:AddText("But these days he has been quiet and stuttering whenever anyone has talked to him...", 5000)
			convo:AddText("But would you please leave me alone now? I got nothing more to say.", 4000)
			convo:SetFinishCallback(self.OnConvoEnd)
			convo:Continue()
			return
		end
	end
end

function NPC.GiveQuest(convo, ply)
	-- Finish callback, will set up the quest state, as well as play the benett zoom-in cinematic
	local self = convo:GetUserData()
	convo:Halt()
	
	if not ply.Quests["tuskentroubles"] then
		ply.Quests["tuskentroubles"] = {}
	end
	ply.Quests["tuskentroubles"].state = 1
	ply.Quests["tuskentroubles"].talkid = self.NPCID
	
	local cin = cinematics.Get("tt_benettzoom")
	if not cin then
		-- Fallback, should the cin not exist, just skip it
		ply:SendChat("Quest given: ^2Tusken troubles")
		convo:SetFinishCallback(self.OnConvoEnd)
		convo:Continue()
		return
	end
	cin:SetFinishCallback(self.OnCinEnd)
	cin.Convo = convo
	cin:PlayCinematic(ply)
	return true -- Prevent the convo from being terminated
end

function NPC.OnCinEnd(cin)
	-- Finish callback for the cinematic, will run a blank cinematic (this will instantly run the finish callback and stop the convo)
	local convo = cin.Convo
	local self = convo:GetUserData()
	local ply = cin:GetPlayer()
	ply:SendChat("Quest given: ^2Tusken troubles")
	convo:SetFinishCallback(self.OnConvoEnd)
	convo:Continue()
end

function NPC.OnConvoEnd(convo, ply)
	-- This is a finish callback for the convo system
	-- If this function returns true, it'll override the default behavior and keeps the convo active
	-- If anything else (or nothing) is returned, the conversation will be ended once this function finishes
	self = convo:GetUserData()
	self.InUse = false
	self:Roam()
end

function NPC:OnDie(inflictor, attacker, damage, meansofdeath)
	-- We got killed -.-.. probably some rogue npc kill all
	-- Inform the controller so we can be respawned in a few moments
	self.Controller:NPCKilled(self, self.NPCID)
end

function NPC:OnReached()
	-- We reached our navgoal
	-- We create a timer here, with a delay between the holdtime min and max
	if self.SuppressWalk then return end
	local wp = self.Waypoints[self.CurrWaypoint]
	timer.Simple(math.random(wp.HoldtimeMin, wp.HoldtimeMax), self.NextWaypoint, self)
end

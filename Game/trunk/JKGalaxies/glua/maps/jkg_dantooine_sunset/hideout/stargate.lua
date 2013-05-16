--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Code
	Stargate controls - Korriban GM Hideout
	
	Written by BobaFett
--------------------------------------------------]]

-- Initialize the GM Hideout on korriban
JKG.GMH.StarGate = {}
local stargate = JKG.GMH.StarGate

local SGThreads = {}
local SGSounds = {}
local SGEnts = {}

local SGDisFXID = -1

local SGGlyphs = "#0123456789abcdefghijklmnopqrstuvwxyz@$"

local SGSafeTime = 0
local SGOpenTime = 0
local SGState = 0 

local SGIncomingThread = false -- Whether or not the player incoming thread is active
local SGPlayerQueue = {}

local SGAddress = nil

--[[ ------
--|| Stargate states:
--|| 0 - Idle
--|| 1 - In motion
--|| 2 - Open (Dialed)
--|| 3 - Open (Incoming)
--|| 4 - Shutting down
--]] ------

stargate.AddressTable = {}
local AddressTable = stargate.AddressTable

-- The address list is stored in here
include("stargate_addresses.lua")

local SGPollMins = Vector(-2, -42, -42)  -- Mins 'n maxs for the poller, relative to the ring center
local SGPollMaxs = Vector(2, 42, 42)
--local SGPollAxis = 1 -- X axis
--local SGPollDir = 1	 -- Positive

local function InitEnts()
	SGEnts.Chevron = {}
	SGEnts.Chevron[1] = ents.GetByName("StargateChevron1")[1]
	SGEnts.Chevron[2] = ents.GetByName("StargateChevron2")[1]
	SGEnts.Chevron[3] = ents.GetByName("StargateChevron3")[1]
	SGEnts.Chevron[4] = ents.GetByName("StargateChevron4")[1]
	SGEnts.Chevron[5] = ents.GetByName("StargateChevron5")[1]
	SGEnts.Chevron[6] = ents.GetByName("StargateChevron6")[1]
	SGEnts.Chevron[7] = ents.GetByName("StargateChevron7")[1]
	SGEnts.ChevronLock = ents.GetByName("StargateChevron7Lock")[1]
	
	SGEnts.Ring = ents.GetByName("StargateRing")[1]
	SGEnts.Woosh = ents.GetByName("StargateWoosh")[1]
	SGEnts.Death = ents.GetByName("StargateDeath")[1]
	SGEnts.Horizon = ents.GetByName("StargateHorizon")[1]
	SGEnts.Disappear = ents.GetByName("StargateDisappear")[1]
end

hook.Add("MapLoaded", "GMH.Stargate", InitEnts)

local function PlaySound(sID)
	sys.PlaySoundIdx(SGEnts.Ring:GetPos(), 0, sID)
end

local function InitSounds()
	SGSounds.Chevron = sys.SoundIndex("sound/jkg/gmh/chevron")
	SGSounds.Chevron2 = sys.SoundIndex("sound/jkg/gmh/chevron2")
	SGSounds.ChevronIncoming = sys.SoundIndex("sound/jkg/gmh/chevron_incoming")
	SGSounds.ChevronLock = sys.SoundIndex("sound/jkg/gmh/chevron_lock")
	SGSounds.ChevronLockDHD = sys.SoundIndex("sound/jkg/gmh/chevron_lock_dhd")
	SGSounds.DHD = sys.SoundIndex("sound/jkg/gmh/dhd_sg1")
	SGSounds.DialFail = sys.SoundIndex("sound/jkg/gmh/dial_fail")
	SGSounds.GateOpen = sys.SoundIndex("sound/jkg/gmh/gate_open")
	SGSounds.GateClose = sys.SoundIndex("sound/jkg/gmh/gate_close")
	SGSounds.GateRoll = sys.SoundIndex("sound/jkg/gmh/gate_roll")
	-- Roll loop sound: sound/jkg/gmh/ring_loop
end

InitSounds()

local function RedirSpawnLocOverride(ply) 
	if ply:GetUserInfo("cflag") != "S" then
		return
	end
	-- Mark the player, the PlayerSpawned hook will need this
	ply.StarGateSpawn = true
	-- Spawn the player deep in the void
	return { Vector(0 + 50 * ply.ID, 0, -50000), Vector(0,0,0) }
	--local pos = SGEnts.Ring:GetPos() + Vector(-150, 0, 0)
	--return { pos, Vector(0,180,0) }
end

hook.Add("SelectInitialSpawn", "SGRedirSpawnLocOverride", RedirSpawnLocOverride, 10)

local function RedirSpawnOverride(ply)
	if not ply.StarGateSpawn then
		return
	end
	ply.StarGateSpawn = nil
	-- This is a stargate spawn, the player's been spawned deep in the void, so lock him in place
	ply:SetFreeze(true, true)
	local cin = cinematics.Get("sg_dantooine")
	if cin then
		cin:SetFinishCallback(cin:PlayCinematic(ply)) -- Automatically replay the cinematic once it finishes
		ply:StartCinematic()
		cin:PlayCinematic(ply)
	end
	
	local k,v
	local add = true
	for k,v in pairs(SGPlayerQueue) do	-- Should not be the case, but let's check if the player's already in the list
		if v == ply then
			add = false
			break
		end
	end
	if add then
		table.insert(SGPlayerQueue, ply)
	end
	if not SGIncomingThread then
		SGIncomingThread = true
		thread.Create("GMHSG_IncomingPlayer", SGThreads.IncomingPlayers)
	end
end

hook.Add("PlayerSpawned", "SGRedirSpawnOverride", RedirSpawnOverride, 10)

function SGThreads.LockChevron(chevron)
	-- If encode = true, then we turn the light back off, otherwise it stays on~
	if chevron < 7 then
		stargate.SetChevronState(7, 1)
		SGEnts.ChevronLock:Move(Vector(0,0,-5), 400, 8)
		thread.Wait(400)
		stargate.SetChevronState(chevron, 1)
		SGEnts.ChevronLock:Move(Vector(0,0,0), 400, 8)
		thread.Wait(400)
		stargate.SetChevronState(7, 0)
	else
		SGEnts.ChevronLock:Move(Vector(0,0,-5), 400, 8)
		thread.Wait(1000)
		SGEnts.ChevronLock:Move(Vector(0,0,0), 400, 8)
		thread.Wait(400)
		stargate.SetChevronState(7, 1)
	end
end

function SGThreads.LockChevronFail()
	SGEnts.ChevronLock:Move(Vector(0,0,-5), 400, 8)
	thread.Wait(1000)
	SGEnts.ChevronLock:Move(Vector(0,0,0), 400, 8)
	thread.Wait(400)
	stargate.SetChevronState(7, 0)
end

local function NormalizeRingAngles()
	local ang = SGEnts.Ring:GetAngles()
	ang.x = ang.x % 360
	ang.y = ang.y % 360
	ang.z = ang.z % 360
	SGEnts.Ring:SetAngles(ang)
end

local function GetNextRingPos(glyph)
	local angs = SGEnts.Ring:GetAngles()
	local gn = string.find(SGGlyphs, glyph)
	-- If the glyph is invalid (and this should never happen)
	--  do a full revolution
	if not gn then
		angs.z = angs.z + 360
		return angs, 4000
	end
	gn = gn - 1 -- Change to base 0
	local desiredangle = 360 - ( ( 360 / 39 ) * gn )
	local currangle = angs.z % 360 -- Normalize it
	if currangle < 0 then currangle = currangle + 360 end -- Ensure currangle is in the 0 - 360 range
	-- Find the longest (180 to 360 angle rotation) to get to the desired angle
	local deltaangle = desiredangle - currangle
	if deltaangle < 0 then deltaangle = 360 + deltaangle end
	if deltaangle < 180 then deltaangle = -360 + deltaangle end
	local delay = ( 4000 / 360 ) * math.abs(deltaangle)
	angs.z = angs.z + deltaangle
	return angs, delay	
end

-- Thread for dialing
function SGThreads.DialingThread(address, fail)	
	local valid, reason = stargate.IsValidAddress(address)
	if not valid then
		print("Cannot dial gate: " .. reason)
		return false
	end
	
	if SGState == 1 or SGState == 4 then
		-- Gate is being activated or shut down, bail
		return
	elseif SGState == 2 or SGState == 3 then
		-- Gate is active, shut it down
		SGThreads.ShutdownThread()
		SGState = 1
		thread.Wait(500)
	end
	
	SGState = 1
	SGAddress = ""
	NormalizeRingAngles()
	local i
	local rot, dur
	for i=1,7 do
		PlaySound(SGSounds.GateRoll)
		rot, dur = GetNextRingPos(string.sub(address, i, i))
		SGEnts.Ring:Rotate(rot, dur, 8)
		SGEnts.Ring:SetLoopSound("sound/jkg/gmh/ring_loop")
		thread.Wait(dur)
		SGEnts.Ring:SetLoopSound(nil)
		if i < 7 then
			-- Normal
			PlaySound(math.random(1,10) < 5 and SGSounds.Chevron or SGSounds.Chevron2)
			stargate.SetChevronState(7, 1)
			thread.Wait(1000)
			SGThreads.LockChevron(i)
			thread.Wait(250)
		else
			-- Locking
			PlaySound(SGSounds.ChevronLockDHD)
			thread.Wait(1000)
			PlaySound(SGSounds.ChevronLock)
			if fail then
				SGThreads.LockChevronFail()
			else
				SGThreads.LockChevron(7)
			end
		end
	end
	if fail then
		PlaySound(SGSounds.DialFail)
		thread.Wait(1000)
		for i=1,6 do
			stargate.SetChevronState(i, 0)
		end
		SGState = 0
	else
		PlaySound(SGSounds.GateOpen)
		SGEnts.Horizon:Use()
		--SGEnts.Ring:SetAngles(Vector(0,0,0))
		SGEnts.Woosh:Use()
		SGEnts.Death:Use()
		SGAddress = address
		thread.Wait(1000)
		SGEnts.Death:Use()
		SGState = 2
		thread.Create("GMHSG_Poller", SGThreads.PollerThread)
	end
end

-- Thread for incoming wormholes
function SGThreads.IncomingThread()
	
	if SGState == 1 or SGState == 4 then
		-- Gate is being activated or shut down, bail
		thread.Signal("GMHSG_IncomingOpen", false)
		return
	elseif SGState == 2 or SGState == 3 then
		-- Gate is active, shut it down
		SGThreads.ShutdownThread()
		SGState = 1
		thread.Wait(500)
	end
	
	SGState = 1
	SGAddress = ""
	
	NormalizeRingAngles()
	SGEnts.Ring:Rotate(SGEnts.Ring:GetAngles() + Vector(0, 0, 720), 9000, 8)
	PlaySound(SGSounds.GateRoll)
	SGEnts.Ring:SetLoopSound("sound/jkg/gmh/ring_loop")
	thread.Wait(1000)
	local i
	for i=1,6 do
		PlaySound(SGSounds.ChevronIncoming)
		stargate.SetChevronState(i, 1)
		thread.Wait(1250)
	end
	thread.Wait(500)
	PlaySound(SGSounds.ChevronLockDHD)
	SGEnts.Ring:SetLoopSound(nil)
	thread.Wait(1000)
	PlaySound(SGSounds.ChevronLock)
	SGThreads.LockChevron(7)
	PlaySound(SGSounds.GateOpen)
	SGEnts.Horizon:Use()
	SGEnts.Ring:SetAngles(Vector(0,0,0))
	SGEnts.Woosh:Use()
	SGEnts.Death:Use()
	thread.Wait(1000)
	SGEnts.Death:Use()
	SGState = 3
	thread.Create("GMHSG_Poller", SGThreads.PollerThread)
	thread.Wait(3000)
	thread.Signal("GMHSG_IncomingOpen", true)
end

-- Thread for shutting down the gate
function SGThreads.ShutdownThread()
	-- TODO: Clear event horizon
	if SGState == 1 or SGState == 4 then
		return
	end
	thread.TerminateThread("GMHSG_Poller")
	SGState = 4
	SGAddress = ""
	PlaySound(SGSounds.GateClose)
	SGEnts.Disappear:Use()
	thread.Wait(500)
	SGEnts.Horizon:Use()
	thread.Wait(1500)
	local i
	for i=1,7 do
		stargate.SetChevronState(i, 0)
	end
	SGState = 0
end


-- Thread for handling incoming players
function SGThreads.IncomingPlayers()
	-- First, try to get the gate to switch to incoming
	SGIncomingThread = true
	while 1 do
		if SGState == 3 then
			-- Already at incoming?
			break
		elseif SGState == 2 then
			-- Stargate's open, see if the safe period expired, if so, switch to incoming
			if sys.Time() > SGSafeTime then
				stargate.Incoming()
				local ok = thread.WaitForSignal("GMHSG_IncomingOpen")
				if ok then
					break
				end
			end
		elseif SGState == 0 then
			-- Gate's inactive, switch to incoming
			stargate.Incoming()
			local ok = thread.WaitForSignal("GMHSG_IncomingOpen")
			if ok then
				break
			end
		end
		thread.Yield()
	end
	-- The gate's on incoming now, time to handle the spawning
	local lastPlayerTime = sys.Time() -- Track the time the last player was processed
	local SpawnLoc = SGEnts.Ring:GetPos() + Vector(-25, 0, 0)
	local SpawnMins = SpawnLoc + Vector(-15, -15, -24)
	local SpawnMaxs = SpawnLoc + Vector(15, 15, 40)
	
	local k,v
	
	while 1 do
		while SGPlayerQueue[1] do	-- If there are people waiting to spawn, process them now
			local ply = SGPlayerQueue[1]
			local SpawnTries = 0 -- Try 5 times, 1 second between each
			
			while SpawnTries < 5 do
				local SafeToSpawn = true
				local bents = ents.FindInBox(SpawnMins, SpawnMaxs) -- Get Blocking ents
				for k,v in pairs(bents) do
					if not v:IsPlayer() then continue end 	-- Ignore non-players
					-- If we get here, there's a player blocking the path
					SafeToSpawn = false
					v:SendCenterPrint("^1You're blocking the stargate\n^1Please move aside")
				end
				if SafeToSpawn == true then break end
				SpawnTries = SpawnTries + 1
			end
			
			cinematics.AbortCinematic(ply)
			ply:SetFreeze(false, false)
			ply:Teleport(SpawnLoc, Vector(0,180,0))
			ply.Invulnerable = 5000 -- 5 seconds of invulnerability, to avoid being killed right after exiting the gate
			table.remove(SGPlayerQueue, 1)
			thread.Wait(2000)
			lastPlayerTime = sys.Time()
		end
		-- If we get here, no more people in the queue, however, we'll wait anyway in case someone joins
		-- If no one gets in queue within 5 seconds, we'll shut down the gate
		thread.Yield()
		if sys.Time() - lastPlayerTime > 5000 then
			stargate.Shutdown()		-- Time has expired, shut down the gate and close this thread
			SGIncomingThread = false
			return
		end
	end
end

function SGThreads.PollerThread()
	-- Poller for the stargate, detects if people are in the event horizon and deals with it accordingly
	local absmins = SGPollMins + SGEnts.Ring:GetPos()
	local absmaxs = SGPollMaxs + SGEnts.Ring:GetPos()
	local entlist
	local k,v,p,vel
	
	SGSafeTime = sys.Time() + 10000 -- Stay open for 10 seconds at least, before being able to get overridden
	SGOpenTime = sys.Time() + 20000 -- Stay open for 20 seconds before automatically closing
	
	while 1 do
		entlist = ents.FindInBox(absmins, absmaxs)
		for k,v in pairs(entlist) do
			if not v:IsPlayer() then continue end
			
			p = v:ToPlayer()
			
			if p.Health < 1 or p.InDeathcam then
				continue
			end
			vel = p:GetVelocity()
			
			if vel.x >= 0 then
				-- Moving into the gate from the front
				if SGState == 2 then
					-- All good, do the tele
					-- TODO: inter and intra server transportation
					local addr = AddressTable[SGAddress]
					if not addr then continue end
					if addr.Type == 1 then
						p:Teleport(addr.Target[1], addr.Target[2])
					elseif addr.Type == 2 then
						--p:SendCommand(string.format("svr \"%s\" S", addr.Target ))
						p:ServerTransfer(addr.Target, "S", "Travelling to " .. addr.Description)
						SGOpenTime = sys.Time() + 10000 -- Stay open for 10 seconds before automatically closing
						--p:Kick("has travelled to another location")
					end
				elseif SGState == 3 then
					-- Incoming wormhole, we're walking backwards...
					p:Disintegrate()
				end
			elseif vel.x < 0 then
				-- Moving into the gate from the back
				p:Disintegrate()
			end
		end
		if SGState == 2 and sys.Time() > SGOpenTime then
			timer.Simple(0, stargate.Shutdown)
			return
		end
		thread.Yield()
	end
end

--[[ -----
--|| Stargate Function
--|| SetChevronState ( int chevron, int lit )
--|| 
--|| Turns the specified chevron (1-7) on or off
--|| lit must be either 1 (to light) or 0 (to turn off)
--]] -----
function stargate.SetChevronState(chevron, lit)
	if chevron < 1 or chevron > 7 then
		return
	elseif chevron >= 1 and chevron <= 6 then
		SGEnts.Chevron[chevron]:SetShaderFrame(lit)
	elseif chevron == 7 then
		SGEnts.Chevron[7]:SetShaderFrame(lit)
		SGEnts.ChevronLock:SetShaderFrame(lit)
	end
end

--[[ -----
--|| Stargate Function
--|| bool IsValidAddress ( string Address )
--|| 
--|| Returns whether or not the address is valid, along with the reason if it's not
--]] -----

--local SGGlyphs = "#0123456789abcdefghijklmnopqrstuvwxyz@$"
function stargate.IsValidAddress(address)
	local usedglyphs = {}
	local curr
	local i
	
	if type(address) != "string" then
		return false, "No address specified"
	end
	
	if string.len(address) != 7 then
		return false, "Incorrect length"
	end
	
	for i=1,7 do
		curr = string.sub(address, i, i)
		if usedglyphs[curr] then
			return false, "Symbol '" .. curr .. "' used twice"
		end
		if not string.find(SGGlyphs, curr) then
			return false, "Invalid symbol '" .. curr .. "'"
		end
		usedglyphs[curr] = true
	end
	return true
end


--[[ -----
--|| Stargate Variable
--|| bool GMOnly
--|| 
--|| Whether or not non-gm's can use the DHD
--]] -----
stargate.GMOnly = true

--[[ -----
--|| Stargate Function
--|| Incoming ( )
--|| 
--|| Activates the incoming wormhole sequence
--]] -----
function stargate.Incoming()
	thread.Create("GMHSG_Incoming", SGThreads.IncomingThread)
end

--[[ -----
--|| Stargate Function
--|| Dial ( )
--|| 
--|| Activates the dialing sequence
--]] -----
function stargate.Dial(address)
	local k,v, b = false
	for k,v in pairs(AddressTable) do
		if k == address then
			b = true
		end
	end
	if not b then
		stargate.DialFail(address)
	end
	thread.Create("GMHSG_Dial", SGThreads.DialingThread, address)
end

--[[ -----
--|| Stargate Function
--|| DialFail ( )
--|| 
--|| Activates the dialing sequence and fails
--]] -----
function stargate.DialFail(address)
	thread.Create("GMHSG_DialFail", SGThreads.DialingThread, address, true)
end


--[[ -----
--|| Stargate Function
--|| Shutdown ( )
--|| 
--|| Shuts down the wormhole
--]] -----
function stargate.Shutdown()
	thread.Create("GMHSG_Shutdown", SGThreads.ShutdownThread)
end

--[[ -----
--|| Stargate Function
--|| GetState ( )
--|| 
--|| Returns the current stargate state
--]] -----
function stargate.GetState()
	return SGState
end

--[[ -----
--|| Stargate Function
--|| GetAddress ( )
--|| 
--|| Returns the current dialed address (only valid if state is 2)
--]] -----
function stargate.GetAddress()
	return SGAddress
end

if RELOADING then
	InitEnts()
end
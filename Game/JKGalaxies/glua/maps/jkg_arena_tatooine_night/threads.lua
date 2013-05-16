--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Code
	Tatooine arena backend: threads
	
	Written by BobaFett
--------------------------------------------------]]

local backend = JKG.ArenaBackend
local controls = backend.Controls
local th = backend.Threads -- For higher speed and quicker access

local MS_NONE = 0			-- No matches in progress
local MS_AWAITINGPLAYERS = 1 -- Players are being summoned to the waiting rooms
local MS_STARTING = 2		-- Players are in the waiting rooms, doing 10 sec countdown
local MS_INPROGRESS = 3		-- The match is in progress
local MS_FINISHED = 4		-- The match finished, people can now leave the arena

-- Match Game Modes
local MGM_NONE = 0
local MGM_REINFORCEMENTS = 1
local MGM_KILLS = 2
local MGM_MAX = 3

local function MAChatToAll(message)
	local data = backend.Data.MainArena
	local k,v
	for k,v in pairs(data.RedTeam) do
		v:SendChat(message)
	end
	for k,v in pairs(data.BlueTeam) do
		v:SendChat(message)
	end
end

local function MACPToAll(message)
	local data = backend.Data.MainArena
	local k,v
	for k,v in pairs(data.RedTeam) do
		v:SendCenterPrint(message)
	end
	for k,v in pairs(data.BlueTeam) do
		v:SendCenterPrint(message)
	end
end

-- Returns the entities for both doors
local function FindDoors()
	local k,v
	local red = nil
	local blue = nil
	
	local reddoors = ents.FindInBox(Vector(3389, -289, 209), Vector(3391, -291, 221))
	for k,v in pairs(reddoors) do
		if v:GetClassName() == "func_door" then
			red = v
		end
	end
	
	local bluedoors = ents.FindInBox(Vector(-3391, 289, 209), Vector(-3389, 291, 221))
	for k,v in pairs(bluedoors) do
		if v:GetClassName() == "func_door" then
			blue = v
		end
	end
	
	return red, blue
end

function th.MAMain()	-- Main arena, main events thread
	local data = backend.Data.MainArena
	local i
	local k,v
	local rd, bd = FindDoors()
	-- Lock the waiting room doors
	rd:SetDoorLocked(true)
	bd:SetDoorLocked(true)
	
	controls.SetRedScore(-1) -- Dashes
	controls.SetBlueScore(-1) -- Dashes
	MAChatToAll("^5Notice: ^7The arena match will commence in 10 seconds, get ready!")
	for i=1, 10 do
		if i % 2 == 0 then
			controls.SetTime(-2)	-- Blank
		else
			controls.SetTime(-3)	-- 88:88
		end
		thread.Wait(1000)
	end
	controls.SetTime(-1)
	-- Teleport everyone into the waiting rooms and give them the default loadout
	-- Also make sure they cannot be dismembered and disintegrated
	local spawns = ents.GetByName("3v3ArenaRed") -- There are 3 of these
	i=1
	for k,v in pairs(data.RedTeam) do
		v:Teleport(spawns[i]:GetPos(), spawns[i]:GetAngles())
		v.NoDismember = true
		v.NoDisintegrate = true
		i=i+1
		backend.DoLoadout(1, v)
	end
	local spawns = ents.GetByName("3v3ArenaBlue") -- There are 3 of these
	i=1
	for k,v in pairs(data.BlueTeam) do
		v:Teleport(spawns[i]:GetPos(), spawns[i]:GetAngles())
		v.NoDismember = true
		v.NoDisintegrate = true
		i=i+1
		backend.DoLoadout(1, v)
	end
	for i=10, 1, -1 do
		if i <= 3 then
			MACPToAll(string.format("The match begins in %i", i))
		end
		controls.SetTimeCountDown(i)
		thread.Wait(1000)
	end
	MACPToAll("GO!")
	data.Stage = MS_INPROGRESS
	-- Unlock the doors and open them
	rd:SetDoorLocked(false)
	rd:Use(rd, rd)
	bd:SetDoorLocked(false)
	bd:Use(bd, bd)
	
	-- Redo loadouts, so the players get their ammo
	for k,v in pairs(data.RedTeam) do
		backend.DoLoadout(1, v)
	end
	for k,v in pairs(data.BlueTeam) do
		backend.DoLoadout(1, v)
	end
	
	-- Initialize displays
	if data.GameMode == MGM_REINFORCEMENTS then
		controls.SetRedReinforcements(data.ReinfCount)
		controls.SetBlueReinforcements(data.ReinfCount)
	else
		controls.SetRedScore(0)
		controls.SetBlueScore(0)
	end
	data.Stage = 3
	data.TimeLeft = data.TimeLimit
	while data.TimeLeft >= 0 do
		controls.SetTime(data.TimeLeft)
		data.TimeLeft = data.TimeLeft - 1
		thread.Wait(1000)
	end
	backend.GameFinished(1, 2)
end


function th.MAFinalize(condition, timeout, aborted)	-- Main arena, finalize thread
	-- Conditions:
	-- 1 = Red team won
	-- 2 = Blue team won
	-- 3 = Tie
	local data = backend.Data.MainArena
	local i
	local k,v
	thread.Wait(500)
	if aborted then
		MACPToAll("Game aborted!")
	elseif timeout then
		MACPToAll("Time up!")
	else
		MACPToAll("Game set!")
	end
	-- Strip all ammo
	for k,v in pairs(data.RedTeam) do
		v:StripClipAmmo()
		v:StripAmmo()
	end
	for k,v in pairs(data.BlueTeam) do
		v:StripClipAmmo()
		v:StripAmmo()
	end
	for i=1, 4 do
		if i % 2 == 0 then
			if condition == 1 then
				controls.SetRedScore(-2)
			elseif condition == 2 then
				controls.SetBlueScore(-2)
			end
			if timeout then
				controls.SetTime(-2)	-- Blank
			end
		else
			if condition == 1 then
				controls.SetRedScore(data.RedScore)
			elseif condition == 2 then
				controls.SetBlueScore(data.BlueScore)
			end
			if timeout then
				controls.SetTime(data.TimeLeft)	-- Blank
			end
		end
		thread.Wait(500)
	end
	if condition == 1 then
		MACPToAll("The red team wins!")
	elseif condition == 2 then
		MACPToAll("The blue team wins!")
	else
		MACPToAll("The teams are tied!")
	end
	
	-- Everyone who is still waiting to be spawned get spawned now
	for k,v in pairs(data.RedTeam) do
		if v.InDeathcam then
			v:Spawn()
		end
	end
	for k,v in pairs(data.BlueTeam) do
		if v.InDeathcam then
			v:Spawn()
		end
	end
	
	for i=1, 16 do
		if i % 2 == 0 then
			if condition == 1 then
				controls.SetRedScore(-2)
			elseif condition == 2 then
				controls.SetBlueScore(-2)
			end
			if timeout then
				controls.SetTime(-2)	-- Blank
			end
		else
			if condition == 1 then
				controls.SetRedScore(data.RedScore)
			elseif condition == 2 then
				controls.SetBlueScore(data.BlueScore)
			end
			if timeout then
				controls.SetTime(data.TimeLeft)	-- Blank
			end
		end
		thread.Wait(500)
	end
	-- Reset
	controls.SetRedScore(-2)
	controls.SetBlueScore(-2)
	controls.SetTime(-2)
	thread.Wait(500)
	data.Active = false
	data.Stage = 0
	-- Force everyone out of the arena
	for k,v in pairs(data.RedTeam) do
		v:Spawn()
	end
	for k,v in pairs(data.BlueTeam) do
		v:Spawn()
	end
end

--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Code
	Tatooine arena backend: events
	
	Written by BobaFett
--------------------------------------------------]]

local backend = JKG.ArenaBackend
local controls = backend.Controls
local evt = backend.Events -- For higher speed and quicker access

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

function evt.OnPlayerDeath(ply, inflictor, attacker, damage, means)
	-- Player died, check if he's on one of the teams
	local arena,team = backend.FindPlayer(ply)
	if arena == 0 then
		return
	elseif arena == 1 then
		local data = backend.Data
		if data.MainArena.Stage ~= MS_INPROGRESS then return end
		if data.MainArena.GameMode == MGM_REINFORCEMENTS then
			if team == 1 then
				data.MainArena.RedScore = data.MainArena.RedScore - 1
				controls.SetRedReinforcements(data.MainArena.RedScore >= 0 and data.MainArena.RedScore or 0)
				if data.MainArena.RedScore <= -#data.MainArena.RedTeam then
					-- Game finished
					backend.GameFinished(1, 1)
				end
			elseif team == 2 then
				data.MainArena.BlueScore = data.MainArena.BlueScore - 1
				controls.SetBlueReinforcements(data.MainArena.BlueScore >= 0 and data.MainArena.BlueScore or 0)
				if data.MainArena.BlueScore <= -#data.MainArena.BlueTeam then
					-- Game finished
					backend.GameFinished(1, 1)
				end
			end
		elseif data.MainArena.GameMode == MGM_KILLS then
			if team == 1 then
				data.MainArena.BlueScore = data.MainArena.BlueScore + 1
				controls.SetBlueScore(data.MainArena.BlueScore)
				if data.MainArena.BlueScore >= data.MainArena.KillLimit then
					-- Game finished
					backend.GameFinished(1, 1)
				end
			elseif team == 2 then
				data.MainArena.RedScore = data.MainArena.RedScore + 1
				controls.SetRedScore(data.MainArena.RedScore)
				if data.MainArena.RedScore >= data.MainArena.KillLimit then
					-- Game finished
					backend.GameFinished(1, 1)
				end
			end		
		end
	elseif arena == 2 then
	
	end
end

hook.Add("PlayerDeath", "ArenaPlayerDeath", evt.OnPlayerDeath)

function evt.OnPlayerDeathcam(ply)
	-- Player died, check if he's on one of the teams
	local arena,team = backend.FindPlayer(ply)
	if arena == 0 then
		return
	elseif arena == 1 then
		local data = backend.Data
		if data.MainArena.Stage < MS_INPROGRESS then return end
		if data.MainArena.GameMode == MGM_REINFORCEMENTS then
			if data.MainArena.Stage == MS_FINISHED then return -1 end
			if team == 1 then
				if data.MainArena.RedScore < 0 then
					return (data.MainArena.TimeLeft + 60) * 1000
				else
					return -1
				end
			elseif team == 2 then
				if data.MainArena.BlueScore < 0 then
					return (data.MainArena.TimeLeft + 60) * 1000
				else
					return -1
				end
			end
		elseif data.MainArena.GameMode == MGM_KILLS then
			return -1	
		end
	elseif arena == 2 then
	
	end
end

hook.Add("PlayerDeathcam", "ArenaPlayerDeathcam", evt.OnPlayerDeathcam, 10)

function evt.OnSelectSpawn(ply, team, avoidpoint)
	-- Player died, check if he's on one of the teams
	local arena,pteam = backend.FindPlayer(ply)
	if arena == 0 then
		return
	elseif arena == 1 then
		local data = backend.Data
		local spawnlist
		if pteam == 1 then
			spawnlist = ents.GetByName("3v3InArenaRed")
		elseif pteam == 2 then
			spawnlist = ents.GetByName("3v3InArenaBlue")
		else
			return
		end
		while 1 do
			local n = math.random(1, #spawnlist)
			if not sys.SpotWouldTelefrag(ply.Entity, spawnlist[n]:GetPos()) then
				return spawnlist[n]
			end
			if #spawnlist == 1 then
				return spawnlist[1]
			end
			table.remove(spawnlist, n)
		end
	elseif arena == 2 then
	
	end
end

hook.Add("SelectSpawn", "ArenaSelectSpawn", evt.OnSelectSpawn, 10)

function evt.OnPlayerSpawned(ply)
	-- Player died, check if he's on one of the teams
	local arena,pteam = backend.FindPlayer(ply)
	if arena == 0 then
		return
	elseif arena == 1 then
		ply.NoDismember = true
		ply.NoDisintegrate = true
		backend.DoLoadout(1, ply)
	elseif arena == 2 then
	
	end
end

hook.Add("PlayerSpawned", "ArenaPlayerSpawned", evt.OnPlayerSpawned, 10)



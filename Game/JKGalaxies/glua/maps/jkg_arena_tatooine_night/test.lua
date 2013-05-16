--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Code
	Tatooine arena test controls
	
	Written by BobaFett
--------------------------------------------------]]

-- These functions allow testing of the arena
local testdata = {
	Teams = {
		Red = {},
		Blue = {},
	},
	LivingTeamMembers = {
		Red = 0,
		Blue = 0,
	},
	Scores = {
		Red = 0,
		Blue = 0,
	},
	TimeRemaining = 0,
	Ingame = false,
	RoundTime = 300, -- 5 minutes
	CountdownTime = 3, -- 3 seconds of countdown before the game starts
	Ready = false,
}

-- Runs on a thread
local function MainGameFunc()
	local gametime
	local k,v
	-- Start with the countdown
	gametime = testdata.CountdownTime
	while gametime > 0 do
		JKG.ArenaBackend.Controls.SetTimeCountDown(gametime)
		for k,v in pairs(testdata.Teams.Red) do
			v:SendCenterPrint(string.format("The round starts in %i...", gametime))
		end
		for k,v in pairs(testdata.Teams.Blue) do
			v:SendCenterPrint(string.format("The round starts in %i...", gametime))
		end
		thread.Wait(1000)
		gametime = gametime - 1
	end
	-- Time to get going!
	for k,v in pairs(testdata.Teams.Red) do
		v:SendCenterPrint("GO!")
	end
	for k,v in pairs(testdata.Teams.Blue) do
		v:SendCenterPrint("GO!")
	end
	gametime = testdata.RoundTime
	while gametime > 0 do
		JKG.ArenaBackend.Controls.SetTime(gametime)
		thread.Wait(1000)
		gametime = gametime - 1
	end
	JKG.ArenaBackend.Controls.SetTime(0)
	testdata.Ingame = false
	for k,v in pairs(testdata.Teams.Red) do
		v:SendCenterPrint("Time is up!")
	end
	for k,v in pairs(testdata.Teams.Blue) do
		v:SendCenterPrint("Time is up!")
	end
end

local function TestCommand(ply, argc, argv)
	if argc < 1 then
		ply:SendPrint("Insufficient arguments")
		return
	end
	
	local cmd = string.lower(argv[1])
	local k,v
	
	if cmd == "resetteam" then
		local team = string.lower(argv[2] or "")
		if team == "" then
			testdata.Teams.Red = {}
			testdata.Teams.Blue = {}
			ply:SendPrint("All teams have been cleared")
		elseif team == "red" or team == "r" then
			testdata.Teams.Red = {}
			ply:SendPrint("Red team has been cleared")
		elseif team == "blue" or team == "b" then
			testdata.Teams.Blue = {}
			ply:SendPrint("Blue team has been cleared")
		else
			ply:SendPrint("Invalid team specified")
		end
		return
	elseif cmd == "addtoteam" then
		local team = string.lower(argv[2] or "")
		local pl = players.GetByArg(argv[3] or "")
		if not pl then
			ply:SendPrint("Player not found")
			return
		end
		if team == "red" or team == "r" then
			table.insert(testdata.Teams.Red, pl)
			ply:SendPrint(string.format("Added player %s to the red team", tostring(pl)))
		elseif team == "blue" or team == "b" then
			table.insert(testdata.Teams.Blue, pl)
			ply:SendPrint(string.format("Added player %s to the blue team", tostring(pl)))
		else
			ply:SendPrint("Invalid team specified")
		end
		return
	elseif cmd == "listteams" then
		local SB = sys.CreateStringBuilder()
		SB:Append("Team list:\nRed team: ")
		for k,v in pairs(testdata.Teams.Red) do
			SB:Append(v:GetName())
			SB:Append("^7 ")
		end
		SB:Append("\nBlue team: ")
		for k,v in pairs(testdata.Teams.Blue) do
			SB:Append(v:GetName())
			SB:Append("^7 ")
		end
		ply:SendPrint(SB:ToString())
		return
	elseif cmd == "startround" then
		if #testdata.Teams.Red < 1 or #testdata.Teams.Blue < 1 then
			ply:SendPrint("Error: Insufficient players")
			return
		end
		for k,v in pairs(testdata.Teams.Red) do
			v.arenaKilled = nil
		end
		for k,v in pairs(testdata.Teams.Blue) do
			v.arenaKilled = nil
		end
		testdata.LivingTeamMembers.Red = #testdata.Teams.Red
		testdata.LivingTeamMembers.Blue = #testdata.Teams.Blue
		testdata.Ingame = true
		thread.Create("arenathread01", MainGameFunc)
		ply:SendPrint("Round started")
		return
	elseif cmd == "stopround" then
		if testdata.Ingame == false then
			ply:SendPrint("No round is in progress")
			return
		end
		testdata.Ingame = false
		thread.TerminateThread("arenathread01")
		for k,v in pairs(testdata.Teams.Red) do
			v:SendCenterPrint("The round has ended!")
		end
		for k,v in pairs(testdata.Teams.Blue) do
			v:SendCenterPrint("The round has ended!")
		end
		return
	elseif cmd == "resetscores" then
		testdata.Scores.Red = 0
		testdata.Scores.Blue = 0
		JKG.ArenaBackend.Controls.SetRedScore(0)
		JKG.ArenaBackend.Controls.SetBlueScore(0)
		return
	elseif cmd == "terminate" then
		thread.TerminateThread("arenathread01")
		testdata.Ingame = false
		JKG.ArenaBackend.Controls.SetTime(-2)
		JKG.ArenaBackend.Controls.SetRedScore(-2)
		JKG.ArenaBackend.Controls.SetBlueScore(-2)
		ply:SendPrint("Arena test terminated")
		return
	elseif cmd == "init" then
		thread.TerminateThread("arenathread01")
		testdata = {
			Teams = {
				Red = {},
				Blue = {},
			},
			LivingTeamMembers = {
				Red = 0,
				Blue = 0,
			},
			Scores = {
				Red = 0,
				Blue = 0,
			},
			TimeRemaining = 0,
			Ingame = false,
			RoundTime = 300, -- 5 minutes
			CountdownTime = 3, -- 3 seconds of countdown before the game starts
			Ready = false,
		}
		JKG.ArenaBackend.Controls.SetTime(-1)
		JKG.ArenaBackend.Controls.SetRedScore(0)
		JKG.ArenaBackend.Controls.SetBlueScore(0)
		ply:SendPrint("Arena test reinitialized")
		return
	end
end

cmds.Add("arenatest", TestCommand)

local function abortgame()
	thread.TerminateThread("arenathread01")
	testdata.Ingame = false
	for k,v in pairs(testdata.Teams.Red) do
		v:SendCenterPrint("Round ended!")
	end
	for k,v in pairs(testdata.Teams.Blue) do
		v:SendCenterPrint("Round ended!")
	end
	JKG.ArenaBackend.Controls.SetTime(-1)
end

local function hook_PlayerDeath(ply, inflictor, attacker, damage, meansofdeath)
	local k,v
	if not testdata.Ingame then
		return
	end
	
	for k,v in pairs(testdata.Teams.Red) do
		if v == ply and not ply.arenaKilled then
			-- Player on the red team died
			testdata.Scores.Blue = testdata.Scores.Blue + 1
			JKG.ArenaBackend.Controls.SetBlueScore(testdata.Scores.Blue)
			testdata.LivingTeamMembers.Red = testdata.LivingTeamMembers.Red - 1
			ply.arenaKilled = true
			if testdata.LivingTeamMembers.Red < 1 then
				abortgame()
			end
			return
		end
	end
	for k,v in pairs(testdata.Teams.Blue) do
		if v == ply and not ply.arenaKilled then
			-- Player on the red team died
			testdata.Scores.Red = testdata.Scores.Red + 1
			JKG.ArenaBackend.Controls.SetRedScore(testdata.Scores.Red)
			testdata.LivingTeamMembers.Blue = testdata.LivingTeamMembers.Blue - 1
			ply.arenaKilled = true
			if testdata.LivingTeamMembers.Blue < 1 then
				abortgame()
			end
			return
		end
	end
end

hook.Add("PlayerDeath", "arenatestPlayerDeath", hook_PlayerDeath)
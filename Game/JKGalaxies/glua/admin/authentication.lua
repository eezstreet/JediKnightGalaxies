--[[ ------------------------------------------------
	JKA Galaxies Lua Code
	Authentication module
	
	This module is responsible for handling access to the server, and handing out hammer bans (bans after failing to connnect a certain amount of times)
	
	Written by BobaFett
--------------------------------------------------]]

-- Handle hammer bans

JKG.Hammerbans = JKG.Hammerbans or {} -- Only clear of not already initialized
--------------------------------------------
-- Config
local HammerTimeout = CreateCvar("jkg_hammertimeout", 10, 0) -- minutes before failed attempts are ignored
local HammerFailAttempts = CreateCvar("jkg_hammerfailattempts", 5, 0) -- number of failed attempts required to trigger the Hammer Ban
local HammerBanTime = CreateCvar("jkg_hammerbantime", 120, 0) -- minutes to ban a player if the hammer ban is triggered
local MaxPendingConns = CreateCvar("jkg_maxpendingconns", 2, 0)
--------------------------------------------

local function IPCheck(iptofind)
	local matches = 0
	local ip
	for k,v in pairs(players.GetAll()) do
		ip = v.IP
		if ip == iptofind and v.Connected == false then
			matches = matches + 1
		end
	end
	return matches
end


local function PlayerConnect(ply, firsttime, isbot)
	if isbot then return end
	if not firsttime then return end
	local ip = ply.IP
	-- New player, check if he's on the hammer ban list
	if IPCheck() > MaxPendingConns:GetInteger() then
		return "Too many pending connection"
	end
 	for k,v in pairs(JKG.Hammerbans) do
		if v.banned == false and v.timeout < sys.Time() then
			-- Timed out, wipe entry
			JKG.Hammerbans[k] = nil
		elseif v.banned == true and v.bantimeout < sys.Time() then
			-- Ban expired
			JKG.Hammerbans[k] = nil			
		else
			if v.ip == ip then
				if v.banned then
					local minsremaining = (v.bantimeout - sys.Time()) / 60000
					return string.format("Too many failed connection attempts, banned for %i minute(s)\nBe sure you have JKA Galaxies installed before attempting to connect", minsremaining)
				elseif v.warning then
					v.warning = false
					return string.format("Your last connection attempt failed\nPlease ensure you have JKA Galaxies installed (http:\\\\galaxiesmod.com)\n\nWarning: If you fail to connect %i more times, you will be temporarily banned", HammerFailAttempts:GetInteger() - v.count + 1)
				end
			end
		end
	end
end
	
hook.Add("PlayerConnect", "Auth.PlayerConnect", PlayerConnect)

local function ValidationFailed(ply)
	local ip = ply.IP
	-- New player, check if he's on the hammer ban list
 	for k,v in pairs(JKG.Hammerbans) do
		if v.banned == false and v.timeout < sys.Time() then
			-- Timed out, wipe entry
			JKG.Hammerbans[k] = nil
		elseif v.banned == true and v.bantimeout < sys.Time() then
			-- Ban expired
			JKG.Hammerbans[k] = nil
		else
			if v.ip == ip then
				if v.banned == false then
					v.count = v.count + 1
					v.warning = true
					v.timeout = sys.Time() + (HammerTimeout:GetInteger() * 60000)
					if v.count > HammerFailAttempts:GetInteger() then
						v.banned = true
						v.bantimeout = sys.Time() + (HammerBanTime:GetInteger() * 60000)
					end
				end
				return
			end
		end
	end
	-- If we get here, the IP isnt listed, so add it
	print("Creating new entry")
	local newdata = {}
	newdata.ip = ip
	newdata.count = 1
	newdata.banned = false
	newdata.timeout = sys.Time() + (HammerTimeout:GetInteger() * 60000)
	newdata.bantimeout = 0 -- N/A, not banned yet
	newdata.warning = true -- Warn the player next time he connects
	table.insert(JKG.Hammerbans, newdata)
end

hook.Add("PlayerValidationFailed","Auth.ValidationFailed", ValidationFailed)

local function ClearHammerBans(argc, argv)
	JKG.Hammerbans = {}
	print("Hammer bans cleared")
end

cmds.AddRcon("clearhammerbans", ClearHammerBans)

local function AuthDebug(ply, argc, argv)
	local func = argv[1]
	if not func then 
		ply:SendPrint("Usage: AuthDebug <settings/bans>")
		return
	end
	func = string.lower(func)
	if func == "settings" then
		ply:SendPrint(string.format("^3Auth settings:\nHammer Timeout: %i\nHammer Fail Attempts: %i\nHammer Ban Time: %i\nMax Pending connections: %i", HammerTimeout:GetInteger(), HammerFailAttempts:GetInteger(), HammerBanTime:GetInteger(), MaxPendingConns:GetInteger()))
		return
	end
	if func == "bans" then
		local resp = "Current hammer bans:\n"
		for k,v in pairs(JKG.Hammerbans) do
			if v.banned and v.bantimeout > sys.Time() then
				resp = resp .. v.ip .. "\n"
			end
		end
		ply:SendPrint(resp)
		return
	end
	ply:SendPrint("Invalid argument")
end

cmds.Add("AuthDebug", AuthDebug)

local function AntiCheatTriggered(ply, argc, argv)
	-- If this is called, the player in question cheated, and the AC caught him
	-- Record this offense and kick the player
	print("DEBUG: AntiCheatTriggered called! Code: " .. (tonumber(argv[1]) or 0))
	ply:Kick("was kicked, cheating detected!")
end

cmds.Add("~~actr", AntiCheatTriggered)
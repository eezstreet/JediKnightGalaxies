--[[ ------------------------------------------------
	JKA Galaxies Lua Code
	Ban Manager
	
	This module handles all bans
	
	Written by BobaFett
--------------------------------------------------]]


if not BanSys then
	BanSys = {}
	BanSys.Bans = {}
end

-- Removes the port from an IP
local function StripPort(ip)
	if not string.find(ip,":") then return ip end
	return string.sub(ip, 1, string.find(ip,":") - 1)
end

-- Splits up the IP in 4 individual numbers
local function SplitIP(ip)
	local d1, d2, d3, d4
	_,_,d1,d2,d3,d4 = string.find(StripPort(ip),"(%d+)%.(%d+)%.(%d+)%.(%d+)")
	return tonumber(d1),tonumber(d2),tonumber(d3),tonumber(d4)
end

function BanSys.AddIP(ip, time, reason)
	local entry = BanSys.GetBanEntry(ip)
	if not entry then
		entry = {}
		entry.IP = {}
		entry.IP.Txt = StripPort(ip)
		entry.IP["d1"],entry.IP["d2"],entry.IP["d3"],entry.IP["d4"] = SplitIP(ip)
		if time and time ~= 0 then
			entry.Tempban = true
			entry.Unban = os.time() + time
		end
		entry.Reason = reason
		table.insert(BanSys.Bans, entry)
	else
		if entry.Tempban then
			if time == 0 then
				entry.Tempban = nil
				entry.Unban = nil
			else
				if entry.Unban < (os.time() + time) then
					entry.Unban = os.time() + time
				end
			end
		end
		entry.Reason = reason
	end
end

function BanSys.RemoveIP(ip)
	local k,v
	local lip = StripPort(ip)
	for k,v in pairs(BanSys.Bans) do
		if v.IP.Txt == lip then
			BanSys.Bans[k] = nil
			return true
		end
	end	
	return false
end

function BanSys.IsBanned(ip)
	local d1, d2, d3, d4 = SplitIP(ip)
	local passes = 0
	local k,v
	for k,v in pairs(BanSys.Bans) do
		if v.Tempban and v.Unban < os.time() then
			-- Expired
			BanSys.Bans[k] = nil	
		else
			passes = 0
			if v.IP["d1"] == d1 or v.IP["d1"] == 0 then
				passes = passes + 1
			end
			if v.IP["d2"] == d2 or v.IP["d2"] == 0 then
				passes = passes + 1
			end
			if v.IP["d3"] == d3 or v.IP["d3"] == 0 then
				passes = passes + 1
			end
			if v.IP["d4"] == d4 or v.IP["d4"] == 0 then
				passes = passes + 1
			end
			if passes == 4 then
				return true, v.Reason, v.Unban
			else
				return false
			end
		end
	end
	return false
end

local function BanTimeRemaining(lifttime)
	local days, hours, mins, secs, timediff
	-- Converts remaining ban time into text
	-- Display approach:
	-- if days > 1 then only display days
	-- if days is 0 then hours and minutes are used
	-- If hours is 0 the only minutes is used
	-- If minutes is 0 then seconds are used
	timediff = lifttime - os.time()
	if timediff <= 0 then
		return "0 seconds"
	end
	days = math.floor(timediff / 86400)
	timediff = timediff - (days * 86400)
	hours = math.floor(timediff / 3600)
	timediff = timediff - (hours * 3600)
	mins = math.floor(timediff / 60)
	timediff = timediff - (mins * 60)
	secs = timediff
	
	if days > 0 then
		-- More than 24 hours, do only display days and hours
		local dstr, hstr
		if days == 1 then dstr = "day" else dstr = "days" end
		if hours == 1 then hstr = "hour" else hstr = "hours" end
		return string.format("%i %s and %i %s", days, dstr, hours, hstr)
	elseif hours > 0 then
		-- Less than a day but more than an hour, display hours and minutes
		local hstr, mstr
		if hours == 1 then hstr = "hour" else hstr = "hours" end
		if mins == 1 then mstr = "minute" else mstr = "minutes" end
		return string.format("%i %s and %i %s", hours, hstr, mins, mstr)
	elseif mins > 0 then
		-- Less than an hour but more than a minute, display only minutes
		if mins == 1 then
			return "1 minute"
		else
			return string.format("%i minutes", mins)
		end
	else
		-- Display seconds
		if secs == 1 then
			return "1 second"
		else
			return string.format("%i seconds", secs)
		end
	end
end
	

function BanSys.CheckBan(ply, firsttime, isbot)
	if isbot then 
		return		-- Bots cant be banned
	end
	local banned, reason, unban = BanSys.IsBanned(ply:GetIP())
	if banned then
		if unban then	-- Tempban if this isn't nil
			if reason ~= "" then
				return string.format("You have been banned from this server\nYour ban expires in %s\nReason: %s\n", BanTimeRemaining(unban), reason)
			else
				return string.format("You have been banned from this server\nYour ban expires in %s\n", BanTimeRemaining(unban))
			end
		else
			if reason ~= "" then
				return string.format("You have been banned from this server\nReason: %s\n", reason)
			else
				return string.format("You have been banned from this server\n")
			end
		end
	end
end

function BanSys.GetBanEntry(ip)
	local k,v
	local lip = StripPort(ip)
	for k,v in pairs(BanSys.Bans) do
		if v.IP.Txt == lip then
			return v
		end
	end	
end

function BanSys.FilterBans()
	-- Remove expired bans
	local k,v
	for k,v in pairs(BanSys.Bans) do
		if v.Tempban then
			if v.Unban < os.time() then
				BanSys.Bans[k] = nil
			end
		end
	end
end

function BanSys.LoadData()
	local data = file.Read("admin/bandata.dat")
	if not data or data == "" then
		BanSys.Bans = {}
	else
		BanSys.Bans = Json.Decode(data)
		BanSys.FilterBans()
	end
end

function BanSys.SaveData()
	local data = Json.Encode(BanSys.Bans)
	file.Write("admin/bandata.dat", data)
end

function BanSys.AddBan(argc, argv)
	-- Rcon command, so use print to respond
	if argc < 2 then
		print("Usage: addban <IP> [duration (s/m/h/d), 0 = perma] [reason]")
		print("Example: 'addban 127.0.0.1 5m Testing' will ban 127.0.0.1 for 5 minutes, with the reason 'Testing'")
		return
	end
	local duration
	if argv[2] == nil then
		duration = 0
	else
		local _, matches, tm, mod = string.find(argv[2], "(%d+)(%w?)")
		if matches == 0 then
			print("Error: Could not interpret the duration")
			return
		end
		if mod == nil then
			duration = tm
		elseif mod == "s" then
			duration = tm
		elseif mod == "m" then
			duration = tm * 60
		elseif mod == "h" then
			duration = tm * 3600
		elseif mod == "d" then
			duration = tm  * 86400
		else
			print("Error: Invalid duration specified, identifier " , mod, " unknown")
		end
	end
	local reason
	if argc > 3 then
		reason = string.trim(table.concat(argv," ",3, argc-1))
	end
	BanSys.AddIP(argv[1], duration, reason)
	print("IP added to banlist")
end

function BanSys.RemoveBan(argc, argv)
	-- Rcon command, so use print to respond
	if argc < 2 then
		print("Usage: removeban <IP>")
		return
	end
	local ok = BanSys.RemoveIP(argv[1])
	if not ok then
		print("IP not in banlist")
	else
		print("IP removed from banlist")
	end
end

function BanSys.ListBans(argc, argv)
	local k,v
	print(string.format("%-18s %-25s %s\n", "IP", "Expire time", "Reason"))
	for k,v in pairs(BanSys.Bans) do
		if v.Tempban then
			if v.Unban < os.time() then
				BanSys.Bans[k] = nil
			else
				print(string.format("%-18s %-25s %s\n", v.IP.Txt, BanTimeRemaining(v.Unban), v.Reason))
			end
		else
			print(string.format("%-18s %-25s %s\n", v.IP.Txt, "Permanent", v.Reason))
		end
	end	
end

cmds.AddRcon("addban", BanSys.AddBan)
cmds.AddRcon("removeban", BanSys.RemoveBan)
cmds.AddRcon("listbans", BanSys.ListBans)
hook.Add("PlayerConnect", "BanSys.CheckBan", BanSys.CheckBan)
hook.Add("Init", "BanSys.LoadData", BanSys.LoadData)
hook.Add("Shutdown", "BanSys.SaveData", BanSys.SaveData)
--[[ ------------------------------------------------
	JKA Galaxies Lua Code
	Admin system
		
	Written by BobaFett
--------------------------------------------------]]

local admins = { {user = "boba", pass="lemmein"},
                 {user = "alex", pass="alex2217"} }

				 
local autologins = { "127.0.0.1" }
				 
local function AdmReply(ply, message)
	ply:SendChat("^7Admin System: " .. message)
end

local function Evt_PlayerBegin(ply)
	local k,v
	local ip = ply:GetIP()
	for k,v in pairs(autologins) do
		if v == ip then
			ply.IsAdmin = true
		end
	end
end
hook.Add("PlayerBegin", "AdminHook", Evt_PlayerBegin)

local function Admin_Login(ply, argc, argv)
	local k,v
	if argc ~= 3 then
		AdmReply(ply, "^1Invalid arguments")
		return
	end
	if ply.IsAdmin then
		AdmReply(ply, "^5You are already logged in")
		return
	end
	for k,v in pairs(admins) do
		if v.user == argv[1] and v.pass == argv[2] then
			ply.IsAdmin = true
			AdmReply(ply, "^2Admin login successful")
			return
		end
	end
	AdmReply(ply, "^1Bad username/password")
end

chatcmds.Add("admlogin", Admin_Login)

local function Admin_Logout(ply, argc, argv)
	local k,v
	if ply.IsAdmin then
		ply.IsAdmin = false
		AdmReply(ply, "^5You are now logged out")
		return
	end
	chatcmds.Ignore()
end

chatcmds.Add("admlogout", Admin_Logout)

local function Admin_Kick(ply, argc, argv)
	local k,v
	if ply.IsAdmin then
		if argc < 2 then
			AdmReply(ply, "^1Please specify a player to kick")
			return
		end
		local target =  players.GetByArg(argv[1])
		if not target then
			AdmReply(ply, "^1Invalid player specified")
			return
		end
		local reason
		if argc > 2 then
			reason = table.concat(argv," ",2, argc-1)
		else 
			reason = nil
		end
		AdmReply(ply, "^5Player " .. target:GetName() .. " ^5has been kicked")
		target:Kick(reason)
		return
	end
	chatcmds.Ignore()
end

chatcmds.Add("admkick", Admin_Kick)
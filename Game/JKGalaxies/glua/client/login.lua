--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Code
	Login management
	
	Written by BobaFett
--------------------------------------------------]]

-- DEPRECATED

JKG.users = {}

function JKG.clLogin(ply, argc, argv)
	local user = argv[1] 
	local pass = argv[2]
	print("clLogin: " .. user .. " " .. pass)
	if argc ~= 3 then
		-- Invalid arg count
		ply:SendCommand("~clLoginResp f \"Malformed login request\"")
		return
	end
	-- Just an example :P
	print("ply.loggedin = " .. tostring(ply.loggedin))
	if ply.loggedin then
		
		--- Log out
		ply.loggedin = false
		ply.logindata = nil
	end	
	print("JKG.users[user] = " .. tostring(JKG.users[user]))
	if JKG.users[user] ~= nil and JKG.users[user].pass == pass then
		ply.loggedin = true
		ply.logindata = {}
		ply.logindata.username = user
		ply.logindata.password = pass
		InitUserData(ply)
		ply:SendCommand("~clLoginResp s")
	else
		ply:SendCommand("~clLoginResp f \"Invalid username or password\"")
	end
end

cmds.Add("~clLogin", JKG.clLogin)

local function InitUserData(ply)
	-- Initialize the shit :P
end


function JKG.clRegister(ply, argc, argv)
	local user = argv[1] 
	local pass = argv[2]
	local email = argv[3]
	print("clRegister: " .. user .. " " .. pass .. " " .. email)
	if argc ~= 4 then
		-- Invalid arg count
		ply:SendCommand("~clRegisterResp f \"Invalid data\"")
		return
	end
	-- Just an example :P
	if JKG.users[user] ~= nil then
		ply:SendCommand("~clRegisterResp f \"Username already in use\"")
	else
		JKG.users[user] = {}
		JKG.users[user].pass = pass
		JKG.users[user].email = email
		ply:SendCommand("~clRegisterResp s")
	end
end

cmds.Add("~clRegister", JKG.clRegister)
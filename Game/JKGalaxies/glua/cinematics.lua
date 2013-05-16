--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Code
	Cinematics commands
	
	Written by BobaFett
--------------------------------------------------]]

-- Initialize managers
cinematics.Init()
cinematics.RegisterDir("glua/resources/cinematics")
cinematics.RegisterDir("glua/resources/cinematics/" .. string.lower(sys.MapName()))

print(string.format("Loaded %i cinematic(s)", cinematics.GetCinCount()))

local function PlayCin(ply, argc, argv)
	if argc < 2 then 
		ply:SendPrint("Usage: playcin <cinematic name>")
		return
	end
	local cin = cinematics.Get(argv[1])
	if cin == nil then
		ply:SendPrint(string.format("Error: Could not find cinematic '%s'", argv[1]))
		return
	end
	cin:SetFinishCallback(function(cin) cin:GetPlayer():StopCinematic() end)
	ply:StartCinematic()
	cin:PlayCinematic(ply)
end

local function CinList(ply, argc, argv)
	local sb = sys.CreateStringBuilder()
	local cins = cinematics.GetCinList() -- Get a copy of the cinematic list
	sb:Append("Cinematics list:\nName                Description\n")
	for k,v in pairs(cins) do
		if sb:Length() > 900 then
			ply:SendPrint(sb:ToString())
			sb:Clear()
		end
		sb:Append(string.format("%-20s%s\n", k, tostring(v.data.Description)))
	end
	ply:SendPrint(sb:ToString())
end

local function CinReload(ply, argc, argv)
	if argc < 2 then 
		ply:SendPrint("Usage: cinreload <cinematic name/@all>")
		return
	end
	local cinname = argv[1]
	if cinname == "@all" then
		cinematics.Init()
		cinematics.RegisterDir("glua/resources/cinematics")
		cinematics.RegisterDir("glua/resources/cinematics/" .. string.lower(sys.MapName()))
		ply:SendPrint("Reloaded all cinematics")
	else
		if cinematics.ReloadCinematic(cinname) then
			ply:SendPrint("Successfully reloaded cinematic '" .. cinname .. "'")
		else
			ply:SendPrint("Failed to reload cinematic '" .. cinname .. "'")
		end
	end
end

local function CinPanic(ply, argc, argv)
	cinematics.AbortCinematic(ply)
	ply:StopCinematic()
end


local function PlayCinToAll(ply, argc, argv)
	if argc < 2 then 
		ply:SendPrint("Usage: playcintoall <cinematic name>")
		return
	end
	local p, cin
	for _, p in pairs(players.GetAll()) do
		cin = cinematics.Get(argv[1])
		if cin == nil then
			ply:SendPrint(string.format("Error: Could not find cinematic '%s'", argv[1]))
			return
		end
		cin:SetFinishCallback(function(cin) cin:GetPlayer():StopCinematic() end)
		p:StartCinematic()
		cin:PlayCinematic(p)
	end
end

cmds.Add("playcintoall", PlayCinToAll)
cmds.Add("playcin", PlayCin)
cmds.Add("cinlist", CinList)
cmds.Add("cinreload", CinReload)
cmds.Add("cinpanic", CinPanic)
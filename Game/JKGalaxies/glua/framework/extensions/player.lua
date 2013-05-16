--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Framework
	Player class extensions
	
	DO NOT MODIFY THIS FILE
	
	Written by BobaFett
--------------------------------------------------]]

-- Defines use of the __sys subtable of players, used to store internal information and functions

local function _PlayerConnect(ply, firsttime, isbot)
	ply.__sys = {}
end

hook.Add("PlayerConnect", "_int_ext_PC", _PlayerConnect)

local function _HandleEsc(ply, argc, argv)
	if ply.__sys.EscFunc then
		ply.__sys.EscFunc(ply)
	end
end

cmds.Add("~esc", _HandleEsc)

local pl = findmetatable("Player")
if pl ~= nil then
	function pl:SetEscapeFunc(func)
		self.__sys.EscFunc = func
	end
end

-- In case of a restart
local function fixsys() 
	local k,v
	for k,v in pairs(players.GetAll()) do
		if v.__sys == nil then
			print(tostring(v) .. " is missing __sys, adding")
			v.__sys = {}
			v.__sys.valid = true
		else
			print(tostring(v) .. " already has __sys")
		end
	end
end

cmds.AddRcon("fixsys", fixsys)

local function playertable() 
	local k,v
	for k,v in pairs(players.GetAll()) do
		print("Displaying table for: " .. tostring(v))
		PrintTable(v:GetTable())
	end
end

cmds.AddRcon("playertable", playertable)

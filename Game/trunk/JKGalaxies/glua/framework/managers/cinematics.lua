--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Framework
	Cinematics Manager
	
	DO NOT MODIFY THIS FILE
	
	Written by BobaFett
--------------------------------------------------]]
local _G = _G
local include = include
local print = print
local pairs = pairs
local tostring = tostring
local file = file
local sys = sys
local string = string
local Vector = Vector

module("cinematics")

local Cinematics = {}
local CinCount = 0

function Init()
	Cinematics = {}
	CinCount = 0
	-- Create cinematic base class
	local cinbase = {}
	
	cinbase.Base = nil
	cinbase.Name = "cin_base"
	cinbase.Description = ""
	
	cinbase.Skippable = false
	
	cinbase.FinishCallback = nil
	cinbase.AbortCallback = nil
	cinbase.PlayerLeftCallback = nil

	cinbase.CinObj = nil

	function cinbase:SetFinishCallback(func)
		self.FinishCallback = func
	end

	function cinbase:SetAbortCallback(func)
		self.AbortCallback = func
	end

	function cinbase:SetPlayerLeftCallback(func)
		self.PlayerLeftCallback = func
	end
	
	function cinbase:AbortCinematic()
		self.CinObj:Abort()
	end

	function cinbase:GetPlayer()
		if self.CinObj == nil then
			return nil
		elseif self.CinObj.Playing == false then
			return nil
		else
			return self.CinObj.Player
		end
	end

	function cinbase:SetUpCinematic(cin)
		-- Override this function to put the cinematic commands in
	end
	
	function cinbase:PlayCinematic(ply)
		if self.CinObj then
			-- Already playing..abort it
			self.CinObj.Playing = false
		end
		local cin = sys.CreateCinematic()
		cin:SetCallbacks(function(cinobj, self) 
							if self.FinishCallback then
								self.FinishCallback(self)
							end
							self.CinObj = nil
						 end,
						 function(cinobj, self)
							if self.AbortCallback then
								self.AbortCallback(self)
							end
							self.CinObj = nil
						 end,
						 function(cinobj, self)
							if self.PlayerLeftCallback then
								self.PlayerLeftCallback(self)
							end
							self.CinObj = nil
						 end,
						 self)
		cin:SetAllowAbort(self.Skippable)
		
		self:SetUpCinematic(cin)
		
		self.CinObj = cin

		cin:PlayCinematic(ply)
	end
	Cinematics["cin_base"] = {}
	Cinematics["cin_base"].Path = ""
	Cinematics["cin_base"].Data = cinbase
	Cinematics["cin_base"].Base = nil
end
	
local function RegisterCinematic(path) -- path must be rooted! (/glua/xxxx)
	_G.CIN = {} -- Declare global namespace
	include(path) -- This will fill in the CIN namespace
	local name = _G.CIN.Name
	if name == nil then
		print(string.format("Error: RegisterCinematic: Failed to load %s, no name specified", path))
		_G.CIN = nil
		return false
	end
	if _G.CIN.Base == nil then
		_G.CIN.Base = "cin_base"
	end
	
	if Cinematics[name] == nil then
		CinCount = CinCount + 1
	end
	
	Cinematics[name] = {}
	Cinematics[name].Path = path
	Cinematics[name].Data = _G.CIN
	Cinematics[name].Base = _G.CIN.Base
	_G.CIN = nil
	return true
end

function RegisterDir(dir)
	local files = file.ListFiles(dir, ".lua")
	if #files == 0 then return end
	for k, v in pairs(files) do
		RegisterCinematic(string.format("/%s/%s", dir, v))
	end
end

local function TableInherit(tbl, base)
	local k,v
	if not base then
		return
	end
	for k,v in pairs(base) do
		if tbl[k] == nil then
			tbl[k] = base[k]
		end
	end
end

function Get(name)
	if not name then
		return nil
	end
	local retval = {}
	if Cinematics[name] == nil then
		return nil
	end
	
	for k,v in pairs(Cinematics[name].Data) do
		retval[k] = v
	end
	
	if name ~= Cinematics[name].Base then
		TableInherit(retval, Get(Cinematics[name].Base))
	end
	
	return retval
end

function ReloadCinematic(name)
	if Cinematics[name] == nil then
		return false
	end
	return RegisterCinematic(Cinematics[name].Path)
end

function GetCinCount()
	return CinCount
end

function GetCinList()
	local lst = {}
	for k,v in pairs(Cinematics) do
		lst[k] = v
	end
	return lst
end

function AbortCinematic(ply)
	if ply.__sys.CurrCin then
		ply.__sys.CurrCin.Playing = false
		ply:StopCinematic()
		ply:SetCinematicMode(false)
		ply.__sys.CurrCin = nil
	end
end

local CIN_ACT_STATICCAM = 3
local CIN_ACT_AIMEDCAM = 4
local CIN_ACT_LINEARCAM = 6
local CIN_ACT_SPLINECAM = 7
local CIN_ACT_HITCHCOCKEFFECT = 14

function GetCameraPos(ply)
	if not ply.__sys.CurrCin then return nil end -- Not in cinematic
	local cin = ply.__sys.CurrCin
	local act = cin.CurrCamAct
	if cin.PVSLock then				-- If PVS Lock is activated, use that position
		return cin.PVSLock
	end
	if not act then return nil end -- No special cam act, so use default
	if act.ActType == CIN_ACT_STATICCAM or act.ActType == CIN_ACT_AIMEDCAM or act.ActType == CIN_ACT_HITCHCOCKEFFECT then
		return act.Origin
	elseif act.ActType == CIN_ACT_LINEARCAM or act.ActType == CIN_ACT_SPLINECAM then
		-- Interpolate to get the estimated position
		local targettime = act.Offset + (sys.Time() - cin.CurrCamStart)
		local tps = nil
		local tpe = nil
		local k2, v2
		for k2,v2 in pairs(act.Points) do
			if targettime > v2.Offset then
				tps = v2
			end
			if targettime < v2.Offset and not tpe then
				tpe = v2
			end
		end
		if not tps and not tpe then
			-- Shouldn't happen
			return nil
		elseif tps and not tpe then	-- No endpoint: Time is beyond the last point
			return tps.Origin
		elseif tpe and not tps then	-- No startpoint: Time is before the first point
			return tpe.Origin
		else							-- Got both, do linear interp
			local phase = (targettime - tps.Offset) / (tpe.Offset - tps.Offset)
			local vec = {}
			vec.x = tps.Origin.x * (1-phase) + tpe.Origin.x * phase
			vec.y = tps.Origin.y * (1-phase) + tpe.Origin.y * phase
			vec.z = tps.Origin.z * (1-phase) + tpe.Origin.z * phase
			vec = Vector(vec.x, vec.y, vec.z)
			return vec
		end
		
	end
	return nil
end
			
	
		
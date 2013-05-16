--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Framework
	Custom Entities Manager
	
	DO NOT MODIFY THIS FILE
	
	Written by BobaFett
--------------------------------------------------]]

local _G = _G
local include = include
local print = print
local pairs = pairs
local tostring = tostring
local Vector = Vector
local file = file
local string = string

module("entmanager")

-- Set types with base classes
local EntTypes = {}
EntTypes['generic'] = {BaseClass = "base_generic", ID = 1}
EntTypes['logical'] = {BaseClass = "base_logical", ID = 2}
EntTypes['mover'] = {BaseClass = "base_mover", ID = 3}
EntTypes['trigger'] = {BaseClass = "base_trigger", ID = 4}

local Ents = {}
local EntCount = 0

local function InitBaseEnts() 
	local nent = {}
	-- Define base_generic
	nent.Base = nil
	nent.Type = "generic"
	nent.ClassName = "base_generic"
	function nent:OnSpawn()
		self.Entity:SetPos(Vector(self.Entity:GetSpawnVar("origin") or "0 0 0"))
		self.Entity:SetAngles(Vector(self.Entity:GetSpawnVar("angles") or "0 0 0"))
		-- Empty placeholder
	end
	function nent:OnUse(other, activator) end
	function nent:OnTakeDamage(attacker, damage) end
	function nent:OnThink() end
	function nent:OnRemove() end
	function nent:OnDie(inflictor, attacker, damage, meansofdeath) end
	
	Ents['base_generic'] = {}
	Ents['base_generic'].Type = "generic"
	Ents['base_generic'].Path = ""
	Ents['base_generic'].TypeID = 1
	Ents['base_generic'].Data = nent
	Ents['base_generic'].Base = ""
	-- Define base_logical
	nent = {}
	nent.Base = nil
	nent.Type = "logical"
	nent.ClassName = "base_logical"
	function nent:OnSpawn()	end
	function nent:OnUse(other, activator) end
	function nent:OnThink() end
	function nent:OnRemove() end
	Ents['base_logical'] = {}
	Ents['base_logical'].Type = "logical"
	Ents['base_logical'].Path = ""
	Ents['base_logical'].TypeID = 2
	Ents['base_logical'].Data = nent
	Ents['base_logical'].Base = ""
	-- Define base_mover
	nent = {}
	nent.Base = nil
	nent.Type = "mover"
	nent.ClassName = "base_mover"
	function nent:OnSpawn()
		self.Entity:SetPos(Vector(self.Entity:GetSpawnVar("origin") or "0 0 0"))
		self.Entity:SetAngles(Vector(self.Entity:GetSpawnVar("angles") or "0 0 0"))
		self.Entity:SetModel(self.Entity:GetSpawnVar("model"))
		-- Empty placeholder
	end
	function nent:OnUse(other, activator) end
	function nent:OnThink() end
	function nent:OnRemove() end
	function nent:OnTakeDamage(attacker, damage) end
	function nent:OnReached() end
	function nent:OnBlocked(other) end
	function nent:OnDie(inflictor, attacker, damage, meansofdeath) end
	Ents['base_mover'] = {}
	Ents['base_mover'].Type = "mover"
	Ents['base_mover'].Path = ""
	Ents['base_mover'].TypeID = 3
	Ents['base_mover'].Data = nent
	Ents['base_mover'].Base = ""
	-- Define base_trigger
	nent = {}
	nent.Base = nil
	nent.Type = "trigger"
	nent.ClassName = "base_trigger"
	function nent:OnSpawn()
		self.Entity:SetPos(Vector(self.Entity:GetSpawnVar("origin") or "0 0 0"))
		self.Entity:SetAngles(Vector(self.Entity:GetSpawnVar("angles") or "0 0 0"))
		self.Entity:SetModel(self.Entity:GetSpawnVar("model"))
		-- Empty placeholder
	end
	function nent:OnUse(other, activator) end
	function nent:OnThink() end
	function nent:OnRemove() end
	Ents['base_trigger'] = {}
	Ents['base_trigger'].Type = "trigger"
	Ents['base_trigger'].Path = ""
	Ents['base_trigger'].TypeID = 4
	Ents['base_trigger'].Data = nent
	Ents['base_trigger'].Base = ""
end

function Init()
	Ents = {}
	EntCount = 0
	-- Init base classes
	InitBaseEnts()
end

local function RegisterEntity(path)
	_G.ENT = {} -- Declare global namespace
	include(path) -- This will fill in the ENT namespace
	tmp = nil
	local name = _G.ENT.ClassName or _G.ENT.Classname
	if name == nil then
		print(string.format("Error: RegisterEntity: Failed to load %s, no class name specified", path))
		_G.ENT = nil
		return false
	end
	
	if _G.ENT.Type == nil then
		print(string.format("Error: RegisterEntity: Failed to load %s, no type specified", path))
		_G.ENT = nil
		return false
	end
	if EntTypes[_G.ENT.Type] == nil then
		print(string.format("Error: RegisterEntity: Failed to load %s, invalid type specified", path))
		_G.ENT = nil
		return false
	end
	
	if _G.ENT.Base == nil then
		_G.ENT.Base = EntTypes[_G.ENT.Type].BaseClass
	else
		if GetEntType(_G.ENT.Base) ~= _G.ENT.Type then
			print(string.format("Error: RegisterEntity: Failed to load %s, invalid base class specified (different type)", path))
			_G.ENT = nil
			return false
		end
	end
	
	if Ents[name] == nil then
		EntCount = EntCount + 1
	end

	Ents[name] = {}
	Ents[name].Type = _G.ENT.Type
	Ents[name].Path = path
	Ents[name].TypeID = EntTypes[_G.ENT.Type].ID
	Ents[name].Data = _G.ENT
	Ents[name].Base = _G.ENT.Base
	
	_G.ENT = nil
	return true
end

function RegisterDir(dir)
	local files = file.ListFiles(dir, ".lua")
	if #files == 0 then return end
	for k, v in pairs(files) do
		RegisterEntity(string.format("/%s/%s", dir, v))
	end
end

function GetEntType(name)
	if Ents[name] then
		return Ents[name].Type
	else
		return nil
	end
end

function GetEntTypeID(name)
	if Ents[name] then
		return Ents[name].TypeID
	else
		return 0
	end
end

local function TableInherit(tbl, base)
	local k,v
	for k,v in pairs(base) do
		if tbl[k] == nil then
			tbl[k] = base[k]
		end
	end
end

local function Get(name)
	-- Returns a fused table with the ent and all its base classes combined
	if not name then return nil end
	if Ents[name] == nil then return {} end
	local tbl = {}
	local k, v
	for k,v in pairs(Ents[name].Data) do
		tbl[k] = v
	end
	
	if name ~= Ents[name].Base then
		TableInherit(tbl, Get(Ents[name].Base))
	end
	return tbl
end

function GetCount()
	return EntCount
end

function SpawnEntity(ent, name)	-- Sets up a new entity, args: ent (newly spawned - blank - entity), name: defined entityname to load
	if Ents[name] == nil then
		error("Tried to spawn an invalid entity")
	else
		local tbl = ent:GetTable()
		local enttbl = Get(name)
		local k, v
		for k,v in pairs(enttbl) do
			tbl[k] = v
		end
	end
end

function CallEntityFunc(ent, func, ...)
	if not ent:IsValid() then return nil end
	if not ent[func] then return nil end
	return ent[func](ent, ...)
end
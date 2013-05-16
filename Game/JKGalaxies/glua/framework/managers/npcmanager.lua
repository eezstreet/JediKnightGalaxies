--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Framework
	NPC Manager
	
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

module("npcmanager")

local NPCs = {}
local NPCCount = 0

local function InitBaseNPC() 
	local nnpc = {}
	-- Define base_npc
	nnpc.Base = nil
	nnpc.NPCName = "base_npc"
	function nnpc:OnInit(spawner) end
	function nnpc:OnSpawn() end
	function nnpc:OnUse(other, activator) end
	function nnpc:OnPain(attacker, damage) end
	function nnpc:OnThink() end
	function nnpc:OnTouch(other) end
	function nnpc:OnRemove() end
	function nnpc:OnReached() end
	function nnpc:OnStuck() end
	function nnpc:OnDie(inflictor, attacker, damage, meansofdeath) end
	function nnpc:OnBlocked(blocker) end
	function nnpc:OnAwake() end
	function nnpc:OnAnger(enemy) end
	function nnpc:OnAttack() end
	function nnpc:OnVictory() end
	function nnpc:OnLostEnemy() end
	function nnpc:OnMindTrick(user) end
	
	NPCs['base_npc'] = {}
	NPCs['base_npc'].Path = ""
	NPCs['base_npc'].Data = nnpc
	NPCs['base_npc'].Base = ""
end

function Init()
	NPCs = {}
	NPCCount = 0
	-- Init base npc class
	InitBaseNPC()
end

local function RegisterNPC(path)
	_G.NPC = {} -- Declare global namespace
	include(path) -- This will fill in the ENT namespace
	tmp = nil
	local name = _G.NPC.NPCName
	if name == nil then
		print(string.format("Error: RegisterNPC: Failed to load %s, no NPC name specified", path))
		_G.NPC = nil
		return false
	end
	
	if _G.NPC.Base == nil then
		_G.NPC.Base = "base_npc"
	end
	
	if NPCs[name] == nil then
		NPCCount = NPCCount + 1
	end

	NPCs[name] = {}
	NPCs[name].Path = path
	NPCs[name].Data = _G.NPC
	NPCs[name].Base = _G.NPC.Base
	
	_G.NPC = nil
	return true
end

function RegisterDir(dir)
	local files = file.ListFiles(dir, ".lua")
	if #files == 0 then return end
	for k, v in pairs(files) do
		RegisterNPC(string.format("/%s/%s", dir, v))
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
	-- Returns a fused table with the npc and all its base npcs combined
	if (NPCs[name] == nil) then return {} end
	local tbl = {}
	local k, v
	for k,v in pairs(NPCs[name].Data) do
		tbl[k] = v
	end
	
	if name ~= NPCs[name].Base then
		TableInherit(tbl, Get(NPCs[name].Base))
	end
	return tbl
end

function GetCount()
	return NPCCount
end

function NPCExists(name)
	if NPCs[name] == nil then
		return false 
	end
	return true
end

function SpawnNPC(npc, name)	-- Sets up a new npc entity, args: ent (newly spawned - blank - npc entity), name: defined npc name to load
	if NPCs[name] == nil then
		error("Tried to spawn an invalid npc")
	else
		local tbl = npc:GetTable()
		local npctbl = Get(name)
		local k, v
		for k,v in pairs(npctbl) do
			tbl[k] = v
		end
	end
end

function CallNPCFunc(npc, func, ...)
	if not npc:IsValid() then return nil end
	if not npc[func] then return nil end
	return npc[func](npc, ...)
end

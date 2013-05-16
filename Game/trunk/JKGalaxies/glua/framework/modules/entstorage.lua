--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Framework
	Entity data storage
	
	DO NOT MODIFY THIS FILE
	
	This module manages the storage of arbitrary values in entity and derivative objects
	
	NOTE: This module is only to be used by the engine, do not call this module directly though lua!
	
	Written by BobaFett
--------------------------------------------------]]

local print = print
local tostring = tostring

module("entstorage")

local EntData = {} -- Data table

local function InitData(ent)
	local idx = ent:GetIndex()
	if not idx then return end
	EntData[idx] = {}
	EntData[idx].Entity = ent
end

--[[----------------------------------------------------------------------------------------
	entstorage.GetData
	
	INTERNAL FUNCTION
	
	Returns the requested variable
-------------------------------------------------------------------------------------------]]

function GetData(ent, key)
	local idx = ent:GetIndex()
	if not idx then return nil end
	if EntData[idx] == nil then
		InitData(ent)
	end
	return EntData[idx][key]
end


--[[----------------------------------------------------------------------------------------
	entstorage.SetData
	
	INTERNAL FUNCTION
	
	Sets the specified variable
-------------------------------------------------------------------------------------------]]
function SetData(ent, key, value)
	local idx = ent:GetIndex()
	if not idx then return end
	if EntData[idx] == nil then
		InitData(ent)
	end
	EntData[idx][key] = value
end

--[[----------------------------------------------------------------------------------------
	entstorage.ClearData
	
	INTERNAL FUNCTION
	
	Clears the ent's data storage
-------------------------------------------------------------------------------------------]]

function ClearData(ent)
	local idx = ent:GetIndex()
	if not idx then return end
	EntData[idx] = nil
end

--[[----------------------------------------------------------------------------------------
	entstorage.GetTable
	
	INTERNAL FUNCTION
	
	Returns the ent's data storage (table)
-------------------------------------------------------------------------------------------]]

function GetTable(ent)
	local idx = ent:GetIndex()
	if not idx then return nil end
	if EntData[idx] == nil then
		InitData(ent)
	end
	return EntData[idx]
end

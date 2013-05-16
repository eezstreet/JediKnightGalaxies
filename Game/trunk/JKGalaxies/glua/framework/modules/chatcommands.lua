--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Framework
	Chat Command management Module 
	
	DO NOT MODIFY THIS FILE
	
	This module manages registration of commands
	
	Written by BobaFett
--------------------------------------------------]]


-- Locals (for faster access, and because we're about to lose access to the globals)
local pairs = pairs
local pcall = pcall
local print = print
local tostring = tostring
local tolower = string.lower
local skipcmd

-- Start the cmds module, all subsequent functions are variables, are stored within the cmds. table, and access to the global environment is gone
module("chatcmds")

local Commands = {} --Chat Commands table

--[[----------------------------------------------------------------------------------------
	chatcmds.Add
	
	Registers a chat command
-------------------------------------------------------------------------------------------]]
function Add(cmd_name, cmd_func)
	local tmp = tolower(cmd_name)
	Commands[tmp] = cmd_func
end

--[[----------------------------------------------------------------------------------------
	chatcmds.Remove
	
	Removes a previously registered chat command
-------------------------------------------------------------------------------------------]]
function Remove(event_name, event_id)
	local tmp = tolower(cmd_name)
	if (Commands[tmp] == nil) then return end
	Commands[tmp] = nil
end

--[[----------------------------------------------------------------------------------------
	chatcmds.Exec (Internal function)
	
	Called by the engine when a command is used
-------------------------------------------------------------------------------------------]]

function Exec(cmd_name, ply, argc, argv)
	local ok, retval
	local tmp = tolower(cmd_name)
	if Commands[tmp] == nil then return false end
	skipcmd = false
	Commands[tmp](ply, argc, argv)
	if skipcmd then
		return false
	else
		return true
	end
end

--[[----------------------------------------------------------------------------------------
	chatcmds.Ignore
	
	Call this function if you wish to ignore a command (and thereby pretend it doesn't exist)
-------------------------------------------------------------------------------------------]]

function Ignore()
	skipcmd = true
end

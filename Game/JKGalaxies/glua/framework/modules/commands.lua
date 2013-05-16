--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Framework
	Command management Module 
	
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

-- Start the cmds module, all subsequent functions are variables, are stored within the cmds. table, and access to the global environment is gone
module("cmds")

local Commands = {} -- Commands table
local CommandsRcon = {} -- Commands table (rcon)

--[[----------------------------------------------------------------------------------------
	cmds.Add
	
	Registers a command
-------------------------------------------------------------------------------------------]]
function Add(cmd_name, cmd_func)
	local tmp = tolower(cmd_name)
	Commands[tmp] = cmd_func
end

function AddRcon(cmd_name, cmd_func)
	local tmp = tolower(cmd_name)
	CommandsRcon[tmp] = cmd_func
end

--[[----------------------------------------------------------------------------------------
	cmds.Remove
	
	Removes a previously registered command
-------------------------------------------------------------------------------------------]]
function Remove(event_name, event_id)
	local tmp = tolower(cmd_name)
	if (Commands[tmp] == nil) then return end
	Commands[tmp] = nil
end

function RemoveRcon(event_name, event_id)
	local tmp = tolower(cmd_name)
	if (CommandsRcon[tmp] == nil) then return end
	CommandsRcon[tmp] = nil
end

--[[----------------------------------------------------------------------------------------
	cmds.Exec (Internal function)
	
	Called by the engine when a command is used
-------------------------------------------------------------------------------------------]]

function Exec(cmd_name, ply, argc, argv)
	local ok, retval
	local tmp = tolower(cmd_name)
	if Commands[tmp] == nil then return false end
	Commands[tmp](ply, argc, argv)
	return true
end

function ExecRcon(cmd_name, argc, argv)
	local tmp = tolower(cmd_name)
	if CommandsRcon[tmp] == nil then return false end
	CommandsRcon[tmp](argc, argv)
	return true
end
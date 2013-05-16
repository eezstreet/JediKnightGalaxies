--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Framework
	Hooks Module
	
	DO NOT MODIFY THIS FILE
	
	This module manages event hook registrations.
	Use hook.Add and hook.Remove to manage hooks.
	NOTE: If a hooked function errors out, whatever the reason, the hook will be removed!
	
	Hooks also have a priority, going from 0 to 10 where 0 is default and 10 is max priority.
	Hooks with lower priority are called first, that way the return values of high priority hooks will override that of low priority ones
	
	Written by BobaFett
--------------------------------------------------]]

-- Locals (for faster access, and because we're about to lose access to the globals)
local pairs = pairs
local pcall = pcall
local print = print
local tostring = tostring

-- Start the hook module, all subsequent functions are variables, are stored within the hook. table, and access to the global environment is gone
module("hook")

local Hooks = {} -- Hooks table

--[[----------------------------------------------------------------------------------------
	hook.GetHooks
	
	Returns the hook table, use it to dump registered hooks for example
-------------------------------------------------------------------------------------------]]

function GetHooks()
	return Hooks
end


--[[----------------------------------------------------------------------------------------
	hook.Add
	
	Hooks a function to an event.
	The event id is used to avoid duplicate hooks.
-------------------------------------------------------------------------------------------]]
function Add(event_name, event_id, event_func, priority)
	local i
	if (Hooks[event_name] == nil) then
		Hooks[event_name] = {} -- Create this 'table' first, otherwise the next line will mess up
		for i=0,10 do
			Hooks[event_name][i] = {}
		end
	end
	
	priority = priority or 0
	if priority < 0 then
		priority = 0
	elseif priority > 10 then
		priority = 10
	end
	Hooks[event_name][priority][event_id] = event_func -- Registered
end

--[[----------------------------------------------------------------------------------------
	hook.Remove
	
	Remove a previously registered event hook.
-------------------------------------------------------------------------------------------]]
function Remove(event_name, event_id)
	local i
	if (Hooks[event_name] == nil) then return end
	for i=0,10 do
		Hooks[event_name][i][event_id] = nil
	end
end

--[[----------------------------------------------------------------------------------------
	hook.Call (Internal function)
	
	This function is called by the engine when a hook needs to be called.
	Do not call this from a script unless you have a very good reason to do so.
-------------------------------------------------------------------------------------------]]
function Call(event_name, allowret, ...)
	local HookList  = Hooks[event_name]
	
	local ok, retval
	
	local sretval
	local i
	if HookList ~= nil then
		for i=0,10 do
			for k,v in pairs(HookList[i]) do
				if (v == nil) then
					HookList[i][k] = nil -- silently remove it
				else
					ok, retval = pcall(v, ...)
					if (ok == false) then
						-- Display error here
						print("Lua Hook Error: Hook " .. tostring(k) .. "failed: " .. tostring(retval) ..  ". Removing hook... ")
						HookList[i][k] = nil
					else
						if allowret then
							if retval ~= nil then
								sretval = retval
							end
						end
					end	-- if (ok == false) then
				end	-- if (v == nil) then
			end	--for k,v in pairs(HookList[i]) do
		end	-- for i=0,10 do
	end -- HookList ~= nil
	return sretval
end
				
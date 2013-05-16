--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Framework
	Timer Module
	
	DO NOT MODIFY THIS FILE
	
	This module manages timers
	
	Written by BobaFett
--------------------------------------------------]]

-- Locals (for faster access, and because we're about to lose access to the globals)
local pairs = pairs
local pcall = pcall
local Timer = sys.Milliseconds
local print = print
local table = table
local unpack = unpack
local tostring = tostring

-- Start the timer module, all subsequent functions are variables, are stored within the timer.* table, and access to the global environment is gone
module("timer")

local CurrentTimer = nil

local Timers = {} -- Timers table
local SimpleTimers = {} -- Simple Timers table

--[[----------------------------------------------------------------------------------------
	timer.Create
	
	Creates a new timer
-------------------------------------------------------------------------------------------]]
function Create(timer_name, delay, reps, func, ...)
	if not timer_name then return end
	if (Timers[timer_name] == nil) then
		Timers[timer_name] = {}
	end
	if (reps < 0) then reps = 1 end
	Timers[timer_name].delay = delay
	Timers[timer_name].reps = reps
	Timers[timer_name].func = func
	Timers[timer_name].trigger = Timer()
	Timers[timer_name].args = {...}
	Timers[timer_name].repcount = 0
	Timers[timer_name].deleteme = nil
	Timers[timer_name].triggered = nil
end

--[[----------------------------------------------------------------------------------------
	timer.Simple
	
	Creates a new simple timer (no name, no repeats.. just set and forget)
-------------------------------------------------------------------------------------------]]
function Simple(delay, func, ...)
	local tmr = {}
	tmr.func = func
	tmr.finish = Timer()+delay
	tmr.args = {...}
	table.insert(SimpleTimers, tmr)
end

--[[----------------------------------------------------------------------------------------
	timer.Remove
	
	Remove a previously created timer
-------------------------------------------------------------------------------------------]]
function Remove(timer_name)
	if not timer_name then return end
	if (Timers[timer_name] == nil) then return end
	if CurrentTimer == timer_name then
		-- This timer is currently being processed, so dont delete it from the table.
		-- Instead, we mark it as 'delete-me'
		Timers[timer_name].deleteme = true
	else
		Timers[timer_name] = nil
	end
end

--[[----------------------------------------------------------------------------------------
	timer.Change
	
	Change a previously created timer, all args (except for the name, are optional)
-------------------------------------------------------------------------------------------]]
function Change(timer_name, delay, reps, func, ...)
	if not timer_name then return end
	if (Timers[timer_name] == nil) then return end
	Timers[timer_name].delay = delay or Timers[timer_name].delay
	Timers[timer_name].reps = reps or Timers[timer_name].reps
	Timers[timer_name].func = func or Timers[timer_name].func
	if #{...} ~= 0 then
		Timers[timer_name].args = {...}
	end
	Timers[timer_name].triggered = nil
	Timers[timer_name].deleteme = nil
end

--[[----------------------------------------------------------------------------------------
	timer.Reset
	
	Restart the timer and clear the repetition count
-------------------------------------------------------------------------------------------]]
function Reset(timer_name)
	if (Timers[timer_name] == nil) then return end
	Timers[timer_name].trigger = Timer()
	Timers[timer_name].repcount = 0
	Timers[timer_name].deleteme = nil
	Timers[timer_name].triggered = nil
end

--[[----------------------------------------------------------------------------------------
	timer.Call (Internal function)
	
	This function is called by the engine every frame to check if a timer needs to be triggered
	Do not call this from a script unless you have a very good reason to do so.
-------------------------------------------------------------------------------------------]]
function Check()	
	local ok, retval
	
	-- First check  normal timers
	for k,v in pairs(Timers) do
		if v.trigger + v.delay <= Timer() then
			v.triggered = true
			CurrentTimer = k
			ok, retval = pcall(v.func, unpack(v.args))
			CurrentTimer = nil
			if v.triggered then -- If this is false, the timer got modified, so ignore it
				v.trigger = Timer()
				v.repcount = v.repcount +1
			end
			if (not ok) then
				print("Timer error: " .. tostring(retval))
			end
			
			if (v.deleteme or (v.repcount >= v.reps and v.reps ~= 0 and v.triggered)) then
				Remove(k)
			else
				v.triggered = false
			end
		end
	end
	
	-- Check simple timers next
	
	for k,v in pairs (SimpleTimers) do
		if v.finish <= Timer() then
			ok,retval = pcall(v.func, unpack(v.args))
			if (not ok) then
				print("Timer error: " .. tostring(retval))
			end
			SimpleTimers[k]=nil
		end
	end
			
end

--[[----------------------------------------------------------------------------------------
	timer.Reset (Internal function)
	
	This function is called by the engine when lua_reload is used.
	It will delete all pending Simple Timers, as they can create complications.
	Do not call this from a script unless you have a very good reason to do so.
-------------------------------------------------------------------------------------------]]

function Reset()
	SimpleTimers = {}
end

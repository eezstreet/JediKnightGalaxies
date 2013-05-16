--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Framework
	Threads Module
	
	DO NOT MODIFY THIS FILE
	
	This module manages threads and thread objects
	
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
local setmetatable = setmetatable
local coroutine = coroutine

-- Start the thread module, all subsequent functions are variables, are stored within the thread.* table, and access to the global environment is gone
module("thread")

-- Currently running thread (nil = main thread)
local CurrentThread = nil

local AutoThreads = {} -- Automatic thread table
local ManualThreads = {} -- Manual thread table (weak table)
setmetatable(ManualThreads, {__mode = "v"})

local TS_NEW = 0
local TS_RUNNING = 1
local TS_SUSPENDED = 2
local TS_TERMINATED = 3
local TS_WAITING = 4
local TS_WAITINGFORSIGNAL = 5
local TS_SIGNALRECEIVED = 6

local ThreadObject = {}		-- The thread object itself
ThreadObject.__index = ThreadObject

local function CreateTheadObject(func, args)
	local o = {}
	o.Func = func
	o.Args = args
	o.State = TS_NEW	-- TS_* constants
	o.WaitData = nil	-- Additional args for waiting states (resume time or signal, depending on state)
	o.Thread = nil 	-- Thread handle
	setmetatable(o, ThreadObject)
	return o
end

function ThreadObject:CanResume()
	if self.State == TS_TERMINATED then
		return false
	elseif self.State == TS_WAITING then
		if Timer() < self.WaitData then
			return false
		end
	elseif self.State == TS_WAITINGFORSIGNAL then
		return false		-- Once the signal is received, the state is changed to TS_SIGNALRECEIVED
	end
	return true
end

function ThreadObject:Run()
	-- Execute/resume the thread
	if CurrentThread then	-- Cannot nest threads
		return
	end
	local sendargs = false
	if not self:CanResume() then
		return
	end
	if self.State == TS_SIGNALRECEIVED then
		sendargs = true
	end
	
	self.State = TS_RUNNING
	
	if not self.Thread then
		-- Initial execution
		CurrentThread = self
		self.Thread = coroutine.create(self.Func)
		local state, msg = coroutine.resume(self.Thread, unpack(self.Args))
		if not state then
			print("Error running thread: " .. msg)
		end
	else
		local state, msg
		CurrentThread = self
		if sendargs then
			state, msg = coroutine.resume(self.Thread, unpack(self.WaitData))
		else
			state, msg = coroutine.resume(self.Thread)
		end
		if not state then
			print("Error running thread: " .. msg)
		end
	end
	if coroutine.status(self.Thread) == "dead" then
		self.State = TS_TERMINATED
		self.WaitData = nil
		self.Thread = nil 	-- So we can garbage collect it
	end
	if self.State == TS_RUNNING then
		print("INTERNAL ERROR: Thread yielded incorrectly!")
		self.State = TS_SUSPENDED
	end
	CurrentThread = nil
end

function ThreadObject:Restart()
	-- Restart the thread
	self.State = TS_NEW
	self.Thread = nil
	self:Run()
end

function ThreadObject:Terminate()
	self.State = TS_TERMINATED
	self.WaitData = nil
	self.Thread = nil
end

function ThreadObject:IsTerminated()
	if self.State == TS_TERMINATED then
		return true
	else
		return false
	end
end

function ThreadObject:Signal(signal, ...)
	if self.State == TS_WAITINGFORSIGNAL and self.WaitData == signal then
		self.State = TS_SIGNALRECEIVED
		self.WaitData = {...}
	end
end


--[[----------------------------------------------------------------------------------------
	thread.Create
	
	Creates a new automatic thread and runs it
-------------------------------------------------------------------------------------------]]
function Create(name, func, ...)
	local th = CreateTheadObject(func, {...})
	th.Name = name
	th:Run()
	if not th:IsTerminated() then
		table.insert(AutoThreads, th)
	end
end

--[[----------------------------------------------------------------------------------------
	thread.CreateObject
	
	Creates a new manual thread and returns the thread object
-------------------------------------------------------------------------------------------]]
function CreateObject(func, ...)
	local th = CreateTheadObject(func, {...})
	table.insert(ManualThreads, th)
	return th
end

--[[----------------------------------------------------------------------------------------
	thread.Signal
	
	Adds the specified signal to all threads
-------------------------------------------------------------------------------------------]]
function Signal(signal, ...)
	local k,v
	for k,v in pairs(AutoThreads) do
		v:Signal(signal, ...)
	end
	for k,v in pairs(ManualThreads) do
		v:Signal(signal, ...)
	end
end

--[[----------------------------------------------------------------------------------------
	thread.SignalThread
	
	Adds the specified signal to the specified thread
-------------------------------------------------------------------------------------------]]
function SignalThread(thread, signal, ...)
	local k,v
	if not thread then
		return Signal(signal, ...)
	end
	for k,v in pairs(AutoThreads) do
		if v.Name == thread then
			v:Signal(signal, ...)
		end
	end
end

--[[----------------------------------------------------------------------------------------
	thread.RunThreads
	
	This function is called by the engine every frame
	Runs all automatic threads
-------------------------------------------------------------------------------------------]]
function RunThreads()
	local k,v
	for k,v in pairs(AutoThreads) do
		v:Run()
		if v:IsTerminated() then
			AutoThreads[k] = nil
		end
	end
end

--[[----------------------------------------------------------------------------------------
	thread.Terminate
	
	Terminate all automatic threads with the given name
-------------------------------------------------------------------------------------------]]
function TerminateThread(name)
	local k,v
	for k,v in pairs(AutoThreads) do
		if v.Name == name then
			v:Terminate()
			AutoThreads[k] = nil
		end
	end
end	

--[[----------------------------------------------------------------------------------------
	thread.Reset
	
	This function is called by the engine when lua_reload is used
	Removes all automatic threads
-------------------------------------------------------------------------------------------]]
function Reset()
	local k,v
	for k,v in pairs(AutoThreads) do
		v:Terminate()
	end
	AutoThreads = {}
end	


--[[         Thread Only Functions          ]]--

--[[----------------------------------------------------------------------------------------
	thread.Wait
	
	Suspends the current thread for the delay specified (ms)
-------------------------------------------------------------------------------------------]]
function Wait(delay)
	local th = CurrentThread
	if not th then
		return
	end
	if coroutine.running() ~= th.Thread then
		return
	end
	th.State = TS_WAITING
	th.WaitData = Timer() + delay
	coroutine.yield()
end

--[[----------------------------------------------------------------------------------------
	thread.Yield
	
	Yields the current thread 
	(suspends it for the remainder of the current frame)
-------------------------------------------------------------------------------------------]]
function Yield(delay)
	local th = CurrentThread
	if not th then
		return
	end
	if coroutine.running() ~= th.Thread then
		return
	end
	th.State = TS_SUSPENDED
	th.WaitData = nil
	coroutine.yield()
end

--[[----------------------------------------------------------------------------------------
	thread.WaitForSignal
	
	Suspends the thread until the specified signal is received
-------------------------------------------------------------------------------------------]]
function WaitForSignal(signal)
	local th = CurrentThread
	if not th then
		return
	end
	if coroutine.running() ~= th.Thread then
		return
	end
	th.State = TS_WAITINGFORSIGNAL
	th.WaitData = signal
	return coroutine.yield()		-- Signals can come with additional data, so return it
end

--[[----------------------------------------------------------------------------------------
	thread.Terminate
	
	Terminates the current thread
-------------------------------------------------------------------------------------------]]
function Terminate()
	local th = CurrentThread
	if not th then
		return
	end
	if coroutine.running() ~= th.Thread then
		return
	end
	th.State = TS_TERMINATED
	th.WaitData = nil
	coroutine.yield()
end

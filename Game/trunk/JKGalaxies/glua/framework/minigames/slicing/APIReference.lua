SlicingGame API

--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Code
	Slicing
	
	Written by BobaFett
--------------------------------------------------]]

local SlicingGame = {}

local NODETYPE_ALARM = 0
local NODETYPE_RELAY = 1
local NODETYPE_RESET = 2
local NODETYPE_ACCESS1 = 3
local NODETYPE_ACCESS2 = 4
local NODETYPE_ACCESS3 = 5
local NODETYPE_ACCESS4 = 6
local NODETYPE_ACCESS5 = 7

SlicingGame.__index = SlicingGame

--[[
		Creation
]]
function sys.CreateSlicingGame()
	local o = {}
	setmetatable(o, SlicingGame)
	o.Width = 0			-- Field width
	o.Height = 0		-- Field height
	o.NodeCounts = {	-- Node quantities (used for grid generation)
		Alarm = 0,
		Reset = 0,
		Access = {0,0,0,0,0}
	}
	o.Grid = nil			-- Security grid (generated when the game is started)
	o.ActiveGrid = nil		-- Grid of activated nodes (generated and filled when game is in progress)
	o.AccessLevels = nil	-- Access Level progression
	o.UnlockLevel = 0
	
	o.Intrusion = false		-- Intrusion detection present?
	o.IntrusionTimer = 0	-- Intrusion start time
	o.IntrusionTriggered = false -- Intrusion triggered?
	o.IntrusionTimerName = nil	-- Name of the intrusion timer
	
	o.Warnings = 0			-- Current warning level
	o.WarningThreshold = 0	-- Warning threshold (max level)
	o.SecurityLevels = 0	-- Amount of security levels (1-5)
	o.Active = false		-- Game in progress?
	o.Player = nil			-- Player who's playing
	
	o.DisableGrid = false	-- Disable the grid (prevent activation of any nodes,  temporarily enabled when a node is activated)
	
	o.FinishCallback = nil	-- Callback to call when the game has finished
	o.CallbackUserData = nil -- Userdata to pass along to the callback
	
	o.ProgramList = {}		-- List of available programs (taken from JKG.Slicing.Programs)
	o.ProgramCount = 0
	o.ProgramData = {}		-- Storage for programs
	
	o.MsgStream = nil		-- Message Bitstream to use for networking
	
	o.Seed = nil			-- Generator seed
	
	-- Callback function prototype: func(state, response, data)
	o.DlgCallback = nil		-- Dialog Callback function
	o.DlgCallbackData = nil
	return o
end

--	Configuration functions

SlicingGame:SetFieldSize(width, height)
SlicingGame:SetNodeCounts(alarm, reset, access1, access2, access3, access4, access5)
SlicingGame:SetWarningThreshold(threshold)
SlicingGame:SetSecurityLevelCount(count)
SlicingGame:SetIntrusionDetection(enabled, timer)
SlicingGame:SetCallback(callback, userdata)
SlicingGame:SetSeed(seed)
SlicingGame:AddProgram(name)

--	Gameplay

SlicingGame:StartGame(ply)

--	Internal functions


SlicingGame:__IntrusionStart()
SlicingGame:__IntrusionAbort()
SlicingGame:__IntrusionTrigger()
SlicingGame:__VerifyParameters()
SlicingGame:__GenerateGrid()
SlicingGame:__LockField(locked)
SlicingGame:__ActivateNode(row, col)
SlicingGame:__ActivateAlarm()
SlicingGame:__Victory()
SlicingGame:__RaiseWarningLevel(amount) -- Returns true if the alarm was triggered
SlicingGame:__ShowDialog(mode, message, callback, data)
SlicingGame:__FinishGame(result)
SlicingGame:__RunProgram(progname)

-- Dialog callbacks

SlicingGame:__DlgFailedCallback(response, data)
SlicingGame:__DlgVictoryCallback(response, data)

-- Networking

SlicingGame:__NetTransmit()
SlicingGame:__NetStartGame()
SlicingGame:__NetEndGame()
SlicingGame:__NetRevealNode(row, col, id)
SlicingGame:__NetWarningLevel(level)
SlicingGame:__NetAlarm()
SlicingGame:__NetLock(lock)
SlicingGame:__NetUpdateSecLevels(seclevels)
SlicingGame:__NetBlink(row, col, safe)
SlicingGame:__NetDialog(mode, message)
SlicingGame:__NetEndDialog()
SlicingGame:__NetSummary(summary)
SlicingGame:__NetIntrusion(state)

-- Client input

SliceCommand(ply, argc, argv)

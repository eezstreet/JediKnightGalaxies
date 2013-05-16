--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Code
	Tatooine arena display controls
	
	Written by BobaFett
--------------------------------------------------]]

-- Internal test thread: 'ArenaDisplayTest1' and 'ArenaDisplayTest2'

local backend = JKG.ArenaBackend -- Faster and easier access
local controls = backend.Controls
local display = backend.Display

local function KillTestThread(arena)
	thread.TerminateThread(string.format("ArenaDisplayTest%i", arena))
end

--[[--------------------------------------------------------------------------------------------------------------
	display.ClearDisplay(arena)
	
	Disables the display
	
	Params: arena - Arena to affect from (1 = main arena, 2 = 1v1 arena)
--]]--------------------------------------------------------------------------------------------------------------

function display.ClearDisplay(arena)
	KillTestThread(arena)
	if arena == 1 then
		controls.SetTime(-2)
		controls.SetRedScore(-2)
		controls.SetBlueScore(-2)
	elseif arena == 2 then
		controls.SetSingleTime(-2)
	end
end

--[[--------------------------------------------------------------------------------------------------------------
	display.InitDisplay(arena)
	
	Init's the display, showing only dashes
	
	Params: arena - Arena to affect from (1 = main arena, 2 = 1v1 arena)
--]]--------------------------------------------------------------------------------------------------------------

function display.InitDisplay(arena)
	KillTestThread(arena)
	if arena == 1 then
		controls.SetTime(-1)
		controls.SetRedScore(-1)
		controls.SetBlueScore(-1)
	elseif arena == 2 then
		controls.SetSingleTime(-1)
	end
end

--[[--------------------------------------------------------------------------------------------------------------
	display.TestCountdown(arena, starttime)
	
	Init's the display, showing only dashes
	
	Params:
	arena - Arena to affect from (1 = main arena, 2 = 1v1 arena)
	starttime - Time to start counting down from
--]]--------------------------------------------------------------------------------------------------------------

function display.TestCountdown(arena, starttime)
	KillTestThread(arena)
	if arena == 1 then
		local function dispthread(ltime)
			while ltime > -1 do
				controls.SetTime(ltime)
				ltime = ltime - 1
				thread.Wait(1000)
			end
			controls.SetTime(-2)
		end
		thread.Create("ArenaDisplayTest1", dispthread, starttime)
	elseif arena == 2 then
		local function dispthread(ltime)
			while ltime > -1 do
				controls.SetSmallTime(ltime)
				ltime = ltime - 1
				thread.Wait(1000)
			end
			controls.SetSmallTime(-2)
		end
		thread.Create("ArenaDisplayTest2", dispthread, starttime)
	end
end

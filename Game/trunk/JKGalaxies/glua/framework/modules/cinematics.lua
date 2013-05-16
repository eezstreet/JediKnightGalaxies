--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Framework
	Cinematics system/object
	
	DO NOT MODIFY THIS FILE
	
	Written by BobaFett
--------------------------------------------------]]

local unpack = unpack

local CIN_ACT_FADEIN = 1
local CIN_ACT_FADEOUT = 2
local CIN_ACT_STATICCAM = 3
local CIN_ACT_AIMEDCAM = 4
local CIN_ACT_VIDEO = 5
local CIN_ACT_LINEARCAM = 6
local CIN_ACT_SPLINECAM = 7
local CIN_ACT_MOTIONBLUR = 8
local CIN_ACT_FOV = 9

local CIN_ACT_RESTORE = 11

local CIN_ACT_HITCHCOCKEFFECT = 14
local CIN_ACT_CAMSHAKE = 15

local CIN_ACT_BLUR = 17
local CIN_ACT_COLORMOD_SCALE = 18
local CIN_ACT_COLORMOD_BIAS = 19
local CIN_ACT_COLORMOD_BC = 20
local CIN_ACT_COLORMOD_FX = 21
local CIN_ACT_COLORMOD_INV = 22

local CIN_ACT_CAPTION = 24
local CIN_ACT_FLASHIN = 25
local CIN_ACT_FLASHOUT = 26


local CIN_ACT_CONTDATA = 31
local CIN_ACT_END = 0	-- End of message

local CIN_ACT_CODE = 32	-- Not transmitted
local CIN_ACT_PVSLOCK = 33
local CIN_ACT_FINISH = 34
local CIN_ACT_SKIP = 35


local PlayerMeta = findmetatable("Player")
if PlayerMeta then
	function PlayerMeta:StartCinematic() 
		self:SendCommand("cin start")
		self:SetCinematicMode(true)
	end
	
	function PlayerMeta:StopCinematic() 
		self:SendCommand("cin stop")
		self:SetCinematicMode(false)
	end
end
	
local function CheckVector(vec, sError)
	if vec ~= nil then 
		if type(vec) == "userdata" then
			if vec.ObjName == "Vector" then 
				return true
			end
		end
	end
	if sError then
		error(sError)
	else
		return false
	end
end

local function CheckEntity(ent, sError)
	if ent ~= nil then 
		if type(ent) == "userdata" then
			if ent.ObjName == "Entity" then 
				return true
			end
		end
	end
	if sError then
		error(sError)
	else
		return false
	end
end

local function CheckPlayer(ply, sError)
	if ply ~= nil then 
		if type(ply) == "userdata" then
			if ply.ObjName == "Player" then 
				return true
			end
		end
	end
	if sError then
		error(sError)
	else
		return false
	end
end

local Cinematic = {}

Cinematic.__index = Cinematic

Cinematic.SPL_CUBIC = 0
Cinematic.SPL_BSPLINE = 1
Cinematic.SPL_CATMULLROM = 2
Cinematic.SPL_LINEAR = 3

function sys.CreateCinematic()
	local o = {}
	setmetatable(o, Cinematic)
	o.Queue = {}
	o.AllowAbort = false
	o.Time = 0
	o.Player = nil
	o.Playing = false
	o.Locked = false
	return o
end

function Cinematic:Reset()
	self.Queue = {}
	self.AllowAbort = false
	self.Time = 0
	self.Player = nil
	self.Playing = false
	self.UserData = nil
end

function Cinematic:SetAllowAbort(allow)
	self.AllowAbort = allow
end

function Cinematic:SetCallbacks(FinishCB, AbortCB, PlayerLeftCB, ...)
	self.FinishCallback = FinishCB
	self.AbortCallback = AbortCB
	self.PlayerLeftCallback = PlayerLeftCB
	self.CallbackArgs = {...}
end

function Cinematic:AddFadeIn(Offset, FadeTime)
	if self.Locked then error("Attempted to add actions to a locked cinematic") return end
	local tbl = {}
	tbl.ActType = CIN_ACT_FADEIN
	tbl.FadeTime = FadeTime
	tbl.Offset = Offset
	table.insert(self.Queue, tbl)
end

function Cinematic:AddFadeOut(Offset, FadeTime)
	if self.Locked then error("Attempted to add actions to a locked cinematic") return end
	local tbl = {}
	tbl.ActType = CIN_ACT_FADEOUT
	tbl.FadeTime = FadeTime
	tbl.Offset = Offset
	table.insert(self.Queue, tbl)
end

function Cinematic:AddFlashIn(Offset, FadeTime)
	if self.Locked then error("Attempted to add actions to a locked cinematic") return end
	local tbl = {}
	tbl.ActType = CIN_ACT_FLASHIN
	tbl.FadeTime = FadeTime
	tbl.Offset = Offset
	table.insert(self.Queue, tbl)
end

function Cinematic:AddFlashOut(Offset, FadeTime)
	if self.Locked then error("Attempted to add actions to a locked cinematic") return end
	local tbl = {}
	tbl.ActType = CIN_ACT_FLASHOUT
	tbl.FadeTime = FadeTime
	tbl.Offset = Offset
	table.insert(self.Queue, tbl)
end
function Cinematic:AddStaticCam(Offset, Origin, Angles)
	if self.Locked then error("Attempted to add actions to a locked cinematic") return end
	CheckVector(Origin, "Cinematic:AddStaticCam: Invalid origin specified")
	CheckVector(Angles, "Cinematic:AddStaticCam: Invalid angles specified")
	local tbl = {}
	tbl.ActType = CIN_ACT_STATICCAM
	tbl.Origin = Origin
	tbl.Angles = Angles
	tbl.Offset = Offset
	table.insert(self.Queue, tbl)
end

function Cinematic:AddAimingCam(Offset, Origin, Ent)
	if self.Locked then error("Attempted to add actions to a locked cinematic") return end
	CheckVector(Origin, "Cinematic:AddAimingCam: Invalid origin specified")
	CheckEntity(Ent, "Cinematic:AddAimingCam: Invalid entity specified")
	local tbl = {}
	tbl.ActType = CIN_ACT_AIMEDCAM
	tbl.Origin = Origin
	tbl.Ent = Ent
	tbl.Offset = Offset
	table.insert(self.Queue, tbl)
end

function Cinematic:AddVideo(Offset, Filename, Loop)
	if self.Locked then error("Attempted to add actions to a locked cinematic") return end
	local tbl = {}
	tbl.ActType = CIN_ACT_VIDEO
	tbl.Filename = Filename
	tbl.Loop = Loop or false
	tbl.Offset = Offset
	table.insert(self.Queue, tbl)
end

function Cinematic:StartLinearCam(Offset, Target)
	if self.Locked then error("Attempted to add actions to a locked cinematic") return end
	local tbl = {}
	tbl.ActType = CIN_ACT_LINEARCAM
	if CheckEntity(Target) then
		tbl.Targetted = true
		tbl.TargetType = 1
		tbl.Ent = Target
	elseif CheckVector(Target) then
		tbl.Targetted = true
		tbl.TargetType = 2
		tbl.Target = Target
	else
		tbl.Targetted = false
	end
	tbl.PointCount = 0
	tbl.Points = {}
	tbl.Offset = Offset
	self.LinearCamObj = tbl
end

function Cinematic:AddLinearCamPoint(Offset, Origin, Angles)
	if self.Locked then error("Attempted to add actions to a locked cinematic") return end
	if self.LinearCamObj == nil then return end
	CheckVector(Origin, "Cinematic:AddLinearCamPoint: Invalid origin specified")
	local tbl = {}
	tbl.Origin = Origin
	if self.LinearCamObj.Targetted == false then
		CheckVector(Angles, "Cinematic:AddLinearCamPoint: Invalid angles specified")
		tbl.Angles = Angles
	end
	tbl.Offset = Offset
	self.LinearCamObj.PointCount = self.LinearCamObj.PointCount + 1
	table.insert(self.LinearCamObj.Points, tbl)
end

function Cinematic:FinishLinearCam()
	if self.Locked then error("Attempted to add actions to a locked cinematic") return end
	if self.LinearCamObj == nil then return end
	table.insert(self.Queue, self.LinearCamObj)
	self.LinearCamObj = nil
end

function Cinematic:StartSplineCam(Offset, Target)
	if self.Locked then error("Attempted to add actions to a locked cinematic") return end
	local tbl = {}
	tbl.ActType = CIN_ACT_SPLINECAM
	if CheckEntity(Target) then
		tbl.Targetted = true
		tbl.TargetType = 1
		tbl.Ent = Target
	elseif CheckVector(Target) then
		tbl.Targetted = true
		tbl.TargetType = 2
		tbl.Target = Target
	else
		tbl.Targetted = false
	end
	tbl.PointCount = 0
	tbl.Points = {}
	tbl.Offset = Offset
	tbl.Algorithms = {0,0,0,0,0,0}
	self.SplineCamObj = tbl
end

function Cinematic:SetSplineCamAlgos(XAlg, YAlg, ZAlg, PitchAlg, YawAlg, RollAlg)
	if self.Locked then error("Attempted to add actions to a locked cinematic") return end
	if self.SplineCamObj == nil then return end
	if XAlg then
		self.SplineCamObj.Algorithms[1] = XAlg
	end
	if YAlg then
		self.SplineCamObj.Algorithms[2] = YAlg
	end
	if ZAlg then
		self.SplineCamObj.Algorithms[3] = ZAlg
	end
	if PitchAlg then
		self.SplineCamObj.Algorithms[4] = PitchAlg
	end
	if YawAlg then
		self.SplineCamObj.Algorithms[5] = YawAlg
	end
	if RollAlg then
		self.SplineCamObj.Algorithms[6] = RollAlg
	end
end

function Cinematic:AddSplineCamPoint(Offset, Origin, Angles)
	if self.Locked then error("Attempted to add actions to a locked cinematic") return end
	if self.SplineCamObj == nil then return end
	CheckVector(Origin, "Cinematic:AddSplineCamPoint: Invalid origin specified")
	local tbl = {}
	tbl.Origin = Origin
	if self.SplineCamObj.Targetted == false then
		CheckVector(Angles, "Cinematic:AddSplineCamPoint: Invalid angles specified")
		tbl.Angles = Angles
	end
	self.SplineCamObj.PointCount = self.SplineCamObj.PointCount + 1
	tbl.Offset = Offset
	table.insert(self.SplineCamObj.Points, tbl)
end

function Cinematic:FinishSplineCam()
	if self.Locked then error("Attempted to add actions to a locked cinematic") return end
	if self.SplineCamObj == nil then return end
	table.insert(self.Queue, self.SplineCamObj)
	self.SplineCamObj = nil
end

function Cinematic:AddMotionBlur(Offset, From, To, Time)
	if self.Locked then error("Attempted to add actions to a locked cinematic") return end
	local tbl = {}
	tbl.ActType = CIN_ACT_MOTIONBLUR
	if To == nil and Time == nil then
		tbl.Fade = false
		tbl.Value = From
	else
		tbl.Fade = true
		tbl.From = From
		tbl.To = To
		tbl.Time = Time
	end
	tbl.Offset = Offset
	table.insert(self.Queue, tbl)
end

function Cinematic:AddFov(Offset, From, To, Time) -- From/To 0 = Default FOV (cg_fov)
	if self.Locked then error("Attempted to add actions to a locked cinematic") return end
	local tbl = {}
	tbl.ActType = CIN_ACT_FOV
	if To == nil and Time == nil then
		tbl.Fade = false
		tbl.Value = From
	else
		tbl.Fade = true
		tbl.From = From
		tbl.To = To
		tbl.Time = Time
	end
	tbl.Offset = Offset
	table.insert(self.Queue, tbl)
end

function Cinematic:AddHitchcockEffect(Offset, Origin, Angles, Width, StartFov, EndFov, Duration)
	-- Hitchcock effect, dolly zoom, vertigo shot.. w/e ya wanna call it, its the funky zoom effect :P
	if self.Locked then error("Attempted to add actions to a locked cinematic") return end
	CheckVector(Origin, "Cinematic:AddHitchcockEffect: Invalid origin specified")
	CheckVector(Angles, "Cinematic:AddHitchcockEffect: Invalid angles specified")
	tbl = {}
	tbl.ActType = CIN_ACT_HITCHCOCKEFFECT
	tbl.Origin = Origin
	tbl.Angles = Angles
	tbl.Width = Width or 20
	tbl.StartFov = StartFov or -1
	tbl.EndFov = EndFov or -1
	tbl.Duration = Duration
	tbl.Offset = Offset
	table.insert(self.Queue, tbl)
end

--[[

function Cinematic:AddColorMod(Offset, RFrom, GFrom, BFrom, RTo, GTo, BTo, Time) 
	if self.Locked then error("Attempted to add actions to a locked cinematic") return end
	local tbl = {}
	tbl.ActType = CIN_ACT_COLORMOD
	if RFrom == nil then
		-- Disable colormod
		tbl.Disable = true
		tbl.Fade = false
	else 
		tbl.Disable = false
	end
	if Time == nil then
		tbl.Fade = false
		tbl.Value = {RFrom, GFrom, BFrom}
	else
		tbl.Fade = true
		tbl.From = {RFrom, GFrom, BFrom}
		tbl.To = {RTo, GTo, BTo}
		tbl.Time = Time
	end
	tbl.Offset = Offset
	table.insert(self.Queue, tbl)
end
]]--
function Cinematic:AddPVSLock(Offset, Pos)
	if self.Locked then error("Attempted to add actions to a locked cinematic") return end
	local tbl = {}
	tbl.ActType = CIN_ACT_PVSLOCK
	if Pos ~= nil then
		CheckVector(Pos, "Cinematic:AddPVSLock: Invalid position specified")
	end
	tbl.Pos = Pos
	tbl.Offset = Offset
	table.insert(self.Queue, tbl)
end

function Cinematic:AddCameraShake(Offset, SIntensity, EIntensity, time)
	if self.Locked then error("Attempted to add actions to a locked cinematic") return end
	local tbl = {}
	tbl.ActType = CIN_ACT_CAMSHAKE
	if EIntensity == nil then
		tbl.Fade = false
	else
		tbl.Fade = true
	end
	tbl.Start = SIntensity
	tbl.End = EIntensity
	tbl.Time = time
	tbl.Offset = Offset
	table.insert(self.Queue, tbl)
end

function Cinematic:AddBlur(Offset, Passes, SIntensity, EIntensity, time)
	if self.Locked then error("Attempted to add actions to a locked cinematic") return end
	local tbl = {}
	tbl.ActType = CIN_ACT_BLUR
	tbl.Passes = Passes
	if EIntensity == nil then
		tbl.Fade = false
	else
		tbl.Fade = true
	end
	tbl.Start = SIntensity
	tbl.End = EIntensity
	tbl.Time = time
	tbl.Offset = Offset
	table.insert(self.Queue, tbl)
end

-- AddColorModScale (Offset, Start RGB, [End RGB], [time for fade])
function Cinematic:AddColorModScale(Offset, RFrom, GFrom, BFrom, RTo, GTo, BTo, Time) 
	if self.Locked then error("Attempted to add actions to a locked cinematic") return end
	local tbl = {}
	tbl.ActType = CIN_ACT_COLORMOD_SCALE
	if RFrom == nil then
		-- Disable colormod
		tbl.Disable = true
		tbl.Fade = false
	else 
		tbl.Disable = false
	end
	if Time == nil then
		tbl.Fade = false
		tbl.Value = {RFrom, GFrom, BFrom}
	else
		tbl.Fade = true
		tbl.From = {RFrom, GFrom, BFrom}
		tbl.To = {RTo, GTo, BTo}
		tbl.Time = Time
	end
	tbl.Offset = Offset
	table.insert(self.Queue, tbl)
end

-- AddColorModBias (Offset, Start RGB, [End RGB], [time for fade])
function Cinematic:AddColorModBias(Offset, RFrom, GFrom, BFrom, RTo, GTo, BTo, Time) 
	if self.Locked then error("Attempted to add actions to a locked cinematic") return end
	local tbl = {}
	tbl.ActType = CIN_ACT_COLORMOD_BIAS
	if RFrom == nil then
		-- Disable colormod
		tbl.Disable = true
		tbl.Fade = false
	else 
		tbl.Disable = false
	end
	if Time == nil then
		tbl.Fade = false
		tbl.Value = {RFrom, GFrom, BFrom}
	else
		tbl.Fade = true
		tbl.From = {RFrom, GFrom, BFrom}
		tbl.To = {RTo, GTo, BTo}
		tbl.Time = Time
	end
	tbl.Offset = Offset
	table.insert(self.Queue, tbl)
end

-- AddColorModBC (Offset, Start brightness, Start Contrast, [End brightness], [End contrast], [time for fade])
function Cinematic:AddColorModBC(Offset, BFrom, CFrom, BTo, CTo, Time) 
	if self.Locked then error("Attempted to add actions to a locked cinematic") return end
	local tbl = {}
	tbl.ActType = CIN_ACT_COLORMOD_BC
	if BFrom == nil then
		-- Disable colormod
		tbl.Disable = true
		tbl.Fade = false
	else 
		tbl.Disable = false
	end
	if Time == nil then
		tbl.Fade = false
		tbl.Value = {BFrom, CFrom}
	else
		tbl.Fade = true
		tbl.From = {BFrom, CFrom}
		tbl.To = {BTo, CTo}
		tbl.Time = Time
	end
	tbl.Offset = Offset
	table.insert(self.Queue, tbl)
end

-- AddColorModBC (Offset, FxID, Start intensity, Start brightness, [End intensity], [End brightness], [time for fade])
function Cinematic:AddColorModFX(Offset, FxID, IFrom, BFrom, ITo, BTo, Time) 
	if self.Locked then error("Attempted to add actions to a locked cinematic") return end
	local tbl = {}
	tbl.ActType = CIN_ACT_COLORMOD_FX
	if FxID == nil then
		-- Disable colormod
		tbl.Disable = true
		tbl.Fade = false
	else 
		tbl.Disable = false
	end
	if Time == nil then
		tbl.Fade = false
		tbl.Value = {IFrom, BFrom}
	else
		tbl.Fade = true
		tbl.From = {IFrom, BFrom}
		tbl.To = {ITo, BTo}
		tbl.Time = Time
	end
	tbl.FxID = FxID or 0
	tbl.Offset = Offset
	table.insert(self.Queue, tbl)
end

-- AddColorModBC (Offset, Start inversion,  [End inversion], [time for fade])
function Cinematic:AddColorModInv(Offset, IFrom, ITo, Time) 
	if self.Locked then error("Attempted to add actions to a locked cinematic") return end
	local tbl = {}
	tbl.ActType = CIN_ACT_COLORMOD_INV
	if IFrom == nil then
		-- Disable colormod
		tbl.Disable = true
		tbl.Fade = false
	else 
		tbl.Disable = false
	end
	if Time == nil then
		tbl.Fade = false
		tbl.Value = IFrom
	else
		tbl.Fade = true
		tbl.From = IFrom
		tbl.To = ITo
		tbl.Time = Time
	end
	tbl.Offset = Offset
	table.insert(self.Queue, tbl)
end

function Cinematic:AddRestoreCam(Offset)
	if self.Locked then error("Attempted to add actions to a locked cinematic") return end
	local tbl = {}
	tbl.ActType = CIN_ACT_RESTORE
	tbl.Offset = Offset
	table.insert(self.Queue, tbl)
end

function Cinematic:AddFinish(Offset)
	if self.Locked then error("Attempted to add actions to a locked cinematic") return end
	local tbl = {}
	tbl.ActType = CIN_ACT_FINISH
	tbl.Offset = Offset
	table.insert(self.Queue, tbl)
end

function Cinematic:AddCode(Offset, Func, ...)
	if self.Locked then error("Attempted to add actions to a locked cinematic") return end
	local tbl = {}
	tbl.ActType = CIN_ACT_CODE
	tbl.Func = Func
	tbl.Args = arg
	tbl.Offset = Offset
	table.insert(self.Queue, tbl)
end

function Cinematic:AddCaption(Offset, Location, Text)
	if self.Locked then error("Attempted to add actions to a locked cinematic") return end
	local tbl = {}
	tbl.ActType = CIN_ACT_CAPTION
	tbl.Location = Location or -1
	tbl.Text = Text
	tbl.Offset = Offset
	table.insert(self.Queue, tbl)
end
	

function Cinematic:StartSkip(Offset, ResumeTime, WipeThreshold)
	if self.Locked then error("Attempted to add actions to a locked cinematic") return end
	if self.AddingSkipAct then error("Attempted to add a skip act inside another one") return end
	self.AddingSkipAct = true
	self.QueueBak = self.Queue
	self.Queue = {} -- So we can store child acts
	local tbl = {}
	tbl.ActType = CIN_ACT_SKIP
	tbl.Offset = Offset
	tbl.ResumeTime = ResumeTime
	tbl.WipeThreshold = WipeThreshold
	self.SkipObj = tbl
end

function Cinematic:FinishSkip()
	if self.Locked then error("Attempted to add actions to a locked cinematic") return end
	if not self.AddingSkipAct then return end
	self.AddingSkipAct = nil
	self.SkipObj.Queue = self.Queue
	self.Queue = self.QueueBak
	table.insert(self.Queue, self.SkipObj)
	self.SkipObj = nil
end

function Cinematic:IsPlaying()
	return self.Playing
end

function Cinematic:PlayCinematic(ply)
	if self.AddingSkipAct then
		self.Queue = self.QueueBak
		self.AddingSkipAct = nil
	end
	CheckPlayer(ply, "Cinematic:PlayCinematic: Invalid player specified")
	self.Player = ply
	ply.__sys.CurrCin = self
	if self.AllowAbort then
		ply:SendCommand("cin ae")
		ply:SetEscapeFunc(function(ply)
			if ply.__sys.CurrCin == nil then return end
			local cin = ply.__sys.CurrCin
			local skipact
			local k,v
			-- Locate the next skip act
			for k,v in pairs(cin.Queue) do
				if v.ActType == CIN_ACT_SKIP then
					if skipact and skipact.Offset > v.Offset then
						skipact = v
					elseif not skipact then
						skipact = v
					end
				end
			end
			-- None found? bail
			if skipact == nil then return end
			local wipeoffset = skipact.WipeThreshold or skipact.ResumeTime
			-- Determine the wipe offset and wipe all acts before this offset
			for k,v in pairs(cin.Queue) do
				if v.Offset < wipeoffset then
					cin.Queue[k] = nil
				end
			end
			-- Add the child acts if any
			for k,v in pairs(skipact.Queue) do
				table.insert(cin.Queue, v)
			end
			
			self.Time = skipact.ResumeTime
			self.NextTime = skipact.ResumeTime
			self:ProcessCinematic()
		end	)
	else
		ply:SetEscapeFunc(nil)
	end
	self.Playing = true
	self.Locked = true
	self.Time = 0
	self.NextTime = 0
	self.StartTime = sys.Time()
	self.ActTime = sys.Time()
	self.CurrCamAct = nil
	self.TimerName = "Cin" .. self.Player:GetID()
	self:ProcessCinematic()
end

function Cinematic:Abort()
	if not self.Playing then return end
	self.Queue = {}
	self:ProcessCinematic()
end

function Cinematic:ProcessCinematic()

	-- Redirect (EXPERIMENTAL)
	self:ProcessCinematicNew()
	if (1) then return end
	-- This is where it all happens
	local FinishCin = false
	if self.Playing == false then return end
	-- First, ensure the player we're targetting is actually valid
	if self.Player:IsValid() == false then 
		if self.PlayerLeftCallback then -- Player is gone, abort it
			self.PlayerLeftCallback(self, unpack(self.CallbackArgs))
			return
		end
	end
	self.Time = self.NextTime
	self.ActTime = sys.Time()
	local NextActTime = -1
	local k,v
	local SB = sys.CreateStringBuilder()
	SB:Append("cin ")
	
	for k,v in pairs(self.Queue) do
		if v.Offset == self.Time then
			-- Alright queue this action
			if v.ActType == CIN_ACT_FADEIN then
				SB:Append(string.format("fi %i ", v.FadeTime))
			elseif v.ActType == CIN_ACT_FADEOUT then
				SB:Append(string.format("fo %i ", v.FadeTime))
			elseif v.ActType == CIN_ACT_FLASHIN then
				SB:Append(string.format("fli %i ", v.FadeTime))
			elseif v.ActType == CIN_ACT_FLASHOUT then
				SB:Append(string.format("flo %i ", v.FadeTime))
			elseif v.ActType == CIN_ACT_STATICCAM then
				if self.BroadcastEnt then
					self.Player:BroadcastEntity(self.BroadcastEnt, true)
					self.BroadcastEnt = nil
				end
				self.CurrCamAct = v
				self.CurrCamStart = sys.Time()
				SB:Append(string.format("sc %i %i %i %i %i %i ", v.Origin.x, v.Origin.y, v.Origin.z, v.Angles.x, v.Angles.y, v.Angles.z))
			elseif v.ActType == CIN_ACT_AIMEDCAM then
				self.CurrCamAct = v
				self.CurrCamStart = sys.Time()
				if self.BroadcastEnt then
					self.Player:BroadcastEntity(self.BroadcastEnt, true)
					self.BroadcastEnt = nil
				end
				if v.Ent:IsValid() == false then
					-- Ok, serious problem here
					print("WARNING: ProcessCinematic: Aimed cam act without valid entity! Creating static cam instead!")
					SB:Append(string.format("sc %i %i %i 0 0 0 ", v.Origin.x, v.Origin.y, v.Origin.z))
				else
					SB:Append(string.format("ac %i %i %i %i ", v.Ent:GetIndex(), v.Origin.x, v.Origin.y, v.Origin.z))
					self.Player:BroadcastEntity(v.Ent) -- Broadcast the ent to the client, to ensure its always aware of the ent's position
					self.BroadcastEnt = v.Ent
				end
			elseif v.ActType == CIN_ACT_LINEARCAM then
				-- Ok this one is complex
				self.CurrCamAct = v
				self.CurrCamStart = sys.Time()
				if v.Targetted == true then
					if v.TargetType == 1 then
						if v.Ent:IsValid() == false then
							-- FFS...
							print("WARNING: ProcessCinematic: Linear cam act without valid entity! Using 0 0 0 instead!")
							SB:Append(string.format("lc 2 0 0 0 %i ", v.PointCount))
						else
							SB:Append(string.format("lc 1 %i %i ", v.Ent:GetIndex(), v.PointCount))
						end
					elseif v.TargetType == 2 then
						SB:Append(string.format("lc 2 %i %i %i %i ", v.Target.x, v.Target.y, v.Target.z, v.PointCount))
					else
						error("ProcessCinematic: Internal error: Invalid TargetType for linear cam")
					end
				else
					SB:Append(string.format("lc 0 %i ", v.PointCount))
				end
				local Flush = false -- Set to true if we had to cut the command in 2+ pieces
				if v.Targetted == true then
					for k2, v2 in pairs(v.Points) do
						SB:Append(string.format("%i %i %i %i ", v2.Offset - v.Offset, v2.Origin.x,v2.Origin.y,v2.Origin.z))
						if SB:Length() > 900 then
							self.Player:SendCommand(SB:ToString())
							SB:Clear()
							SB:Append("cin cont ")
							Flush = true
						end
					end
				else
					for k2, v2 in pairs(v.Points) do
						SB:Append(string.format("%i %i %i %i %i %i %i ", v2.Offset - v.Offset, v2.Origin.x,v2.Origin.y,v2.Origin.z, v2.Angles.x, v2.Angles.y, v2.Angles.z))
						if SB:Length() > 900 then
							self.Player:SendCommand(SB:ToString())
							SB:Clear()
							SB:Append("cin cont ")
							Flush = true
						end
					end
				end
				if Flush == true then
					self.Player:SendCommand(SB:ToString())
					SB:Clear()
					SB:Append("cin ")
					Flush = false
				end
			elseif v.ActType == CIN_ACT_SPLINECAM then
				-- Ok this one is complex
				self.CurrCamAct = v
				self.CurrCamStart = sys.Time()
				if v.Targetted == true then
					if v.TargetType == 1 then
						if v.Ent:IsValid() == false then
							-- FFS...
							print("WARNING: ProcessCinematic: Spline cam act without valid entity! Using 0 0 0 instead!")
							SB:Append(string.format("spc 2 0 0 0 %i ", v.PointCount))
						else
							SB:Append(string.format("spc 1 %i %i ", v.Ent:GetIndex(), v.PointCount))
						end
					elseif v.TargetType == 2 then
						SB:Append(string.format("spc 2 %i %i %i %i ", v.Target.x, v.Target.y, v.Target.z, v.PointCount))
					else
						error("ProcessCinematic: Internal error: Invalid TargetType for linear cam")
					end
				else
					SB:Append(string.format("spc 0 %i ", v.PointCount))
				end
				-- Create bitvalue for algorithms, and add that
				local Algos = 0
				Algos = v.Algorithms[1] * 1024
				Algos = Algos + v.Algorithms[2] * 256
				Algos = Algos + v.Algorithms[3] * 64
				Algos = Algos + v.Algorithms[4] * 16
				Algos = Algos + v.Algorithms[5] * 4
				Algos = Algos + v.Algorithms[6]
				SB:Append(string.format("%i ", Algos))
				local Flush = false -- Set to true if we had to cut the command in 2+ pieces
				if v.Targetted == true then
					for k2, v2 in pairs(v.Points) do
						SB:Append(string.format("%i %i %i %i ", v2.Offset - v.Offset, v2.Origin.x,v2.Origin.y,v2.Origin.z))
						if SB:Length() > 900 then
							self.Player:SendCommand(SB:ToString())
							SB:Clear()
							SB:Append("cin cont ")
							Flush = true
						end
					end
				else
					for k2, v2 in pairs(v.Points) do
						SB:Append(string.format("%i %i %i %i %i %i %i ", v2.Offset - v.Offset, v2.Origin.x,v2.Origin.y,v2.Origin.z, v2.Angles.x, v2.Angles.y, v2.Angles.z))
						if SB:Length() > 900 then
							self.Player:SendPrint("Sending fragmented cin message, size: ", SB:Length())
							self.Player:SendCommand(SB:ToString())
							SB:Clear()
							SB:Append("cin cont ")
							Flush = true
						end
					end
				end
				if Flush == true then
					self.Player:SendCommand(SB:ToString())
					SB:Clear()
					SB:Append("cin ")
					Flush = false
				end
			elseif v.ActType == CIN_ACT_MOTIONBLUR then
				if v.Fade == true then
					SB:Append(string.format("mbf %i %i %i ", v.From, v.To, v.Time))
				else
					SB:Append(string.format("mb %i ", v.Value))
				end
			elseif v.ActType == CIN_ACT_FOV then
				if v.Fade == true then
					SB:Append(string.format("fovf %i %i %i ", v.From, v.To, v.Time))
				else
					SB:Append(string.format("fov %i ", v.Value))
				end
			elseif v.ActType == CIN_ACT_COLORMOD_SCALE then
				if v.Disable == true then
					SB:Append(string.format("cmso "))
				else
					if v.Fade == true then
						SB:Append(string.format("cmsf %.3f %.3f %.3f %.3f %.3f %.3f %i ", v.From[1], v.From[2], v.From[3], v.To[1], v.To[2], v.To[3], v.Time))
					else
						SB:Append(string.format("cms %.3f %.3f %.3f ", v.Value[1], v.Value[2], v.Value[3]))
					end
				end
			elseif v.ActType == CIN_ACT_COLORMOD_BIAS then
				if v.Disable == true then
					SB:Append(string.format("cmbo "))
				else
					if v.Fade == true then
						SB:Append(string.format("cmbf %.3f %.3f %.3f %.3f %.3f %.3f %i ", v.From[1], v.From[2], v.From[3], v.To[1], v.To[2], v.To[3], v.Time))
					else
						SB:Append(string.format("cmb %.3f %.3f %.3f ", v.Value[1], v.Value[2], v.Value[3]))
					end
				end
			elseif v.ActType == CIN_ACT_COLORMOD_BC then
				if v.Disable == true then
					SB:Append(string.format("cmco "))
				else
					if v.Fade == true then
						SB:Append(string.format("cmcf %.3f %.3f %.3f %.3f %i ", v.From[1], v.From[2], v.To[1], v.To[2], v.Time))
					else
						SB:Append(string.format("cmc %.3f %.3f ", v.Value[1], v.Value[2]))
					end
				end
			elseif v.ActType == CIN_ACT_COLORMOD_FX then
				if v.Disable == true then
					SB:Append(string.format("cmfo "))
				else
					if v.Fade == true then
						SB:Append(string.format("cmff %i %.3f %.3f %.3f %.3f %i ", v.FxID, v.From[1], v.From[2], v.To[1], v.To[2], v.Time))
					else
						SB:Append(string.format("cmf %i %.3f %.3f ", v.FxID, v.Value[1], v.Value[2]))
					end
				end
			elseif v.ActType == CIN_ACT_COLORMOD_INV then
				if v.Disable == true then
					SB:Append(string.format("cmio "))
				else
					if v.Fade == true then
						SB:Append(string.format("cmif %.3f %.3f %i ", v.From, v.To, v.Time))
					else
						SB:Append(string.format("cmi %.3f ", v.FxID, v.Value))
					end
				end
			elseif v.ActType == CIN_ACT_HITCHCOCKEFFECT then
				self.CurrCamAct = v
				self.CurrCamStart = sys.Time()
				SB:Append(string.format("he %.3f %.3f %.3f %.3f %.3f %.3f %i %i %i %i ", v.Origin.x, v.Origin.y, v.Origin.z, v.Angles.x, v.Angles.y, v.Angles.z, v.Width, v.StartFov, v.EndFov, v.Duration))				
			elseif v.ActType == CIN_ACT_RESTORE then
				self.CurrCamAct = nil
				SB:Append("rdc ")
			elseif v.ActType == CIN_ACT_CAMSHAKE then
				if v.Fade then
					SB:Append(string.format("cs %.3f %.3f %i ", v.Start, v.End, v.Time))
				else
					SB:Append(string.format("cs %.3f ", v.Start))
				end
			elseif v.ActType == CIN_ACT_PVSLOCK then
				self.PVSLock = v.Pos
			elseif v.ActType == CIN_ACT_BLUR then
				if v.Fade then
					SB:Append(string.format("bl %i %.3f %.3f %i ", v.Passes, v.Start, v.End, v.Time))
				else
					SB:Append(string.format("bl %i %.3f ", v.Passes, v.Start))
				end
			elseif v.ActType == CIN_ACT_FINISH then
				FinishCin = true
			elseif v.ActType == CIN_ACT_CODE then
				v.Func(unpack(v.Args))
			end
			if SB:Length() > 900 then
				-- Alright its gettin too big, purge
				self.Player:SendCommand(SB:ToString())
				SB:Clear()
				SB:Append("cin ")
			end
			self.Queue[k] = nil -- Remove this entry
		elseif v.Offset > self.Time then
			if NextActTime == -1 then
				NextActTime = v.Offset
			elseif v.Offset < NextActTime then
				NextActTime = v.Offset
			end
		elseif v.Offset < self.Time then
			-- Shouldnt happen but just in case:
			self.Queue[k] = nil
		end
	end
	if SB:Length() > 4 then
		self.Player:SendCommand(SB:ToString())
	end
	if NextActTime == -1 or FinishCin == true then
		if self.AllowAbort then
			-- Disallow escape
			self.Player:SendCommand("cin de")
		end
		-- No more pending actions, end the animation
		timer.Remove(self.TimerName)
		self.Player.__sys.CurrCin = nil
		if self.FinishCallback then
			self.FinishCallback(self, unpack(self.CallbackArgs))
		end
	else
		-- Set up a timer for the next actions
		self.NextTime = NextActTime
		if self.TimerName == nil then
			self.TimerName = "Cin" .. self.Player:GetID()
			print("Cinematic for " .. tostring(self.Player) .. " had no name, assigning " .. self.TimerName .. " to it")
		end
		timer.Create(self.TimerName, NextActTime - self.Time, 1, self.ProcessCinematic, self)
	end
end

-- Auxiliary functions for ProcessCinematic
local function round(num)
	return math.floor(num + 0.5)
end

local function ConvTime(time)	-- Encode as 10 bits	(.25 max deviation)
	return round(time / 50)
end

local function ConvTimeHR(time)	-- Encode as 11 bits	(.125 max deviation)
	return round(time / 25)
end

local function ConvCoord(coord)	-- Encode as 19 bits (.125 max deviation)
	return round(coord * 4)
end

local function WriteVector(msg, vector)
	msg:WriteBits(-19, ConvCoord(vector.x))
	msg:WriteBits(-19, ConvCoord(vector.y))
	msg:WriteBits(-19, ConvCoord(vector.z))
end

function Cinematic:ProcessCinematicNew()
	-- This is where it all happens
	local FinishCin = false
	if self.Playing == false then return end
	-- First, ensure the player we're targetting is actually valid
	if self.Player:IsValid() == false then 
		if self.PlayerLeftCallback then -- Player is gone, abort it
			self.PlayerLeftCallback(self, unpack(self.CallbackArgs))
			return
		end
	end
	self.Time = self.NextTime
	self.ActTime = sys.Time()
	local NextActTime = -1
	local k,v
	local Msg = bitstream.Create(840) -- Create a 840 byte bitstream (this gets expanded to 960 when encoded, which is the safety limit used here)
	
	local function FinishMessage(finalize)
		if finalize then
			Msg:WriteBits(5, CIN_ACT_END)
		end
		self.Player:SendCommand(string.format("cinb %s", encoding.Base128Encode(Msg:GetData())))
		Msg:Reset()
	end
	
	local function EnsureAvailable(bits, addEnd)
		if addEnd then bits = bits + 5 end
		if Msg:BitsRemaining() < bits then
			FinishMessage(addEnd)
		end
	end	
	
	for k,v in pairs(self.Queue) do
		if v.Offset == self.Time then
			-- Alright queue this action
			if v.ActType == CIN_ACT_FADEIN then
				EnsureAvailable(15, true)
				Msg:WriteBits(5, CIN_ACT_FADEIN)
				Msg:WriteBits(10, ConvTime(v.FadeTime))
				--SB:Append(string.format("fi %i ", v.FadeTime))
			elseif v.ActType == CIN_ACT_FADEOUT then
				EnsureAvailable(15, true)
				Msg:WriteBits(5, CIN_ACT_FADEOUT)
				Msg:WriteBits(10, ConvTime(v.FadeTime))
				--SB:Append(string.format("fo %i ", v.FadeTime))
			elseif v.ActType == CIN_ACT_FLASHIN then
				EnsureAvailable(15, true)
				Msg:WriteBits(5, CIN_ACT_FLASHIN)
				Msg:WriteBits(10, ConvTime(v.FadeTime))
				--SB:Append(string.format("fli %i ", v.FadeTime))
			elseif v.ActType == CIN_ACT_FLASHOUT then
				EnsureAvailable(15, true)
				Msg:WriteBits(5, CIN_ACT_FLASHOUT)
				Msg:WriteBits(10, ConvTime(v.FadeTime))
				--SB:Append(string.format("flo %i ", v.FadeTime))
			elseif v.ActType == CIN_ACT_STATICCAM then
				EnsureAvailable(119, true) -- 6x19 bits + 5 bits
				Msg:WriteBits(5, CIN_ACT_STATICCAM)
				
				if self.BroadcastEnt then
					self.Player:BroadcastEntity(self.BroadcastEnt, true)
					self.BroadcastEnt = nil
				end
				self.CurrCamAct = v
				self.CurrCamStart = sys.Time()
				
				WriteVector(Msg, v.Origin)
				WriteVector(Msg, v.Angles)
				--SB:Append(string.format("sc %i %i %i %i %i %i ", v.Origin.x, v.Origin.y, v.Origin.z, v.Angles.x, v.Angles.y, v.Angles.z))
			elseif v.ActType == CIN_ACT_AIMEDCAM then
				self.CurrCamAct = v
				self.CurrCamStart = sys.Time()
				if self.BroadcastEnt then
					self.Player:BroadcastEntity(self.BroadcastEnt, true)
					self.BroadcastEnt = nil
				end
				if v.Ent:IsValid() == false then
					-- Ok, serious problem here
					print("WARNING: ProcessCinematic: Aimed cam act without valid entity! Creating static cam instead!")
					
					EnsureAvailable(119, true) -- 6x19 bits + 5 bits
					Msg:WriteBits(5, CIN_ACT_STATICCAM)
					WriteVector(Msg, v.Origin)
					WriteVector(Msg, Vector(0,0,0))
					--SB:Append(string.format("sc %i %i %i 0 0 0 ", v.Origin.x, v.Origin.y, v.Origin.z))
				else
					EnsureAvailable(72, true) -- 3x19 bits + 5 bits + 10 bits
					Msg:WriteBits(5, CIN_ACT_AIMEDCAM)
					Msg:WriteBits(10, v.Ent:GetIndex())
					WriteVector(Msg, v.Origin)
					--SB:Append(string.format("ac %i %i %i %i ", v.Ent:GetIndex(), v.Origin.x, v.Origin.y, v.Origin.z))
					self.Player:BroadcastEntity(v.Ent) -- Broadcast the ent to the client, to ensure its always aware of the ent's position
					self.BroadcastEnt = v.Ent
				end
			elseif v.ActType == CIN_ACT_LINEARCAM then
				-- Ok this one is complex
				-- Format:
				-- ID - 5 bits
				-- Targetted - 1 bit
				-- [Target Type] - 1 bit (If Targetted == 1)
				-- [Target Ent ID] - 10 bit (If Targetted == 1 and TargetType = 1)
				-- [Target Coords] - 3x19 bit (If Targetted == 1 and TargetType = 2)
				-- Point count	- 8 bit
				-- Fragmented - 1 bit
				-- [Points in fragment] - 8 bit (if Fragmented == 1)
				-- <Camera points> -- ?? bit (see below)
				
				----- Camera point -------
				-- Offset - 11 bit (TimeHR)
				-- Origin - 3x19 bit
				-- [Angles] - 3x19 bit (If Targetted == 0)
				
				----- Continuation -------
				-- ID - 5 bit
				-- Fragmented - 1 bit
				-- [Points in fragment] - 8 bit (if Fragmented == 1)
				-- <Camera points> -- ?? bit
				
				self.CurrCamAct = v
				self.CurrCamStart = sys.Time()
				if v.Targetted == true then
					if v.TargetType == 1 then
						if v.Ent:IsValid() == false then
							-- FFS...
							print("WARNING: ProcessCinematic: Linear cam act without valid entity! Using 0 0 0 instead!")
							EnsureAvailable(149, true)
							Msg:WriteBits(5, CIN_ACT_LINEARCAM)
							Msg:WriteBool(true) -- Targetted?
							Msg:WriteBits(1, 1) -- Target type
							Msg:WriteBits(-19, 0)
							Msg:WriteBits(-19, 0)
							Msg:WriteBits(-19, 0)
							Msg:WriteByte(v.PointCount)
							--SB:Append(string.format("lc 2 0 0 0 %i ", v.PointCount))
						else
							EnsureAvailable(102, true)
							Msg:WriteBits(5, CIN_ACT_LINEARCAM)
							Msg:WriteBool(true) -- Targetted?
							Msg:WriteBits(1, 0) -- Target type
							Msg:WriteBits(10, v.Ent:GetIndex())
							Msg:WriteByte(v.PointCount)
							--SB:Append(string.format("lc 1 %i %i ", v.Ent:GetIndex(), v.PointCount))
						end
					elseif v.TargetType == 2 then
						EnsureAvailable(149, true)
						Msg:WriteBits(5, CIN_ACT_LINEARCAM)
						Msg:WriteBool(true) -- Targetted?
						Msg:WriteBits(1, 1) -- Target type
						WriteVector(Msg, v.Target)
						Msg:WriteByte(v.PointCount)
						--SB:Append(string.format("lc 2 %i %i %i %i ", v.Target.x, v.Target.y, v.Target.z, v.PointCount))
					else
						error("ProcessCinematic: Internal error: Invalid TargetType for linear cam")
					end
				else
					EnsureAvailable(149, true)
					Msg:WriteBits(5, CIN_ACT_LINEARCAM)
					Msg:WriteBool(false) -- Targetted?
					Msg:WriteByte(v.PointCount)
					--SB:Append(string.format("lc 0 %i ", v.PointCount))
				end
				if v.Targetted == true then
					-- 68 bits per cam point
					-- Determine if this message will be fragmented
					local points = #v.Points
					local pointsprocessed = 0
					if Msg:BitsRemaining() < (#v.Points * 68) then
						-- We have to fragment this message
						Msg:WriteBool(true)
						points = math.floor(Msg:BitsRemaining() / 68)
						Msg:WriteByte(points)
					else
						Msg:WriteBool(false)
					end
					local lastTime = v.Offset
					for k2, v2 in pairs(v.Points) do
						Msg:WriteBits(11, ConvTimeHR(v2.Offset - lastTime))
						lastTime = v2.Offset
						WriteVector(Msg, v2.Origin)
						
						points = points - 1
						pointsprocessed = pointsprocessed + 1
						
						if points == 0 and ((#v.Points - pointsprocessed) > 0) then
							-- We filled the current packet, flush it out and start a new one
							FinishMessage(false)
							Msg:WriteBits(5, CIN_ACT_CONTDATA)
							if Msg:BitsRemaining() < ((#v.Points - pointsprocessed) * 68) then
								-- We have to fragment this message
								Msg:WriteBool(true)
								points = math.floor(Msg:BitsRemaining() / 68)
								Msg:WriteByte(points)
							else
								Msg:WriteBool(false)
							end
						end
					end
				else
					-- 125 bits per cam point
					local points = #v.Points
					local pointsprocessed = 0
					if Msg:BitsRemaining() < (#v.Points * 125) then
						-- We have to fragment this message
						Msg:WriteBool(true)
						points = math.floor(Msg:BitsRemaining() / 125)
						Msg:WriteByte(points)
					else
						Msg:WriteBool(false)
					end
					local lastTime = v.Offset
					for k2, v2 in pairs(v.Points) do
						Msg:WriteBits(11, ConvTimeHR(v2.Offset - lastTime))
						lastTime = v2.Offset
						WriteVector(Msg, v2.Origin)
						WriteVector(Msg, v2.Angles)
						
						points = points - 1
						pointsprocessed = pointsprocessed + 1
						
						if points == 0 and ((#v.Points - pointsprocessed) > 0) then
							-- We filled the current packet, flush it out and start a new one
							FinishMessage(false)
							Msg:WriteBits(5, CIN_ACT_CONTDATA)
							if Msg:BitsRemaining() < ((#v.Points - pointsprocessed) * 125) then
								-- We have to fragment this message
								Msg:WriteBool(true)
								points = math.floor(Msg:BitsRemaining() / 125)
								Msg:WriteByte(points)
							else
								Msg:WriteBool(false)
							end
						end
					end
				end
			elseif v.ActType == CIN_ACT_SPLINECAM then
				-- Ok this one is complex
				-- Format:
				-- ID - 5 bits
				-- Targetted - 1 bit
				-- [Target Type] - 1 bit (If Targetted == 1)
				-- [Target Ent ID] - 10 bit (If Targetted == 1 and TargetType = 1)
				-- [Target Coords] - 3x19 bit (If Targetted == 1 and TargetType = 2)
				-- Point count	- 8 bit
				-- Algorithms - 6x2 bit
				-- Fragmented - 1 bit
				-- [Points in fragment] - 8 bit (if Fragmented == 1)
				-- <Camera points> -- ?? bit (see below)
				
				----- Camera point -------
				-- Offset - 11 bit (TimeHR)
				-- Origin - 3x19 bit
				-- [Angles] - 3x19 bit (If Targetted == 0)
				
				----- Continuation -------
				-- ID - 5 bit
				-- Fragmented - 1 bit
				-- [Points in fragment] - 8 bit (if Fragmented == 1)
				-- <Camera points> -- ?? bit
				self.CurrCamAct = v
				self.CurrCamStart = sys.Time()
				if v.Targetted == true then
					if v.TargetType == 1 then
						if v.Ent:IsValid() == false then
							-- FFS...
							print("WARNING: ProcessCinematic: Spline cam act without valid entity! Using 0 0 0 instead!")
							EnsureAvailable(149, true)
							Msg:WriteBits(5, CIN_ACT_SPLINECAM)
							Msg:WriteBool(true) -- Targetted?
							Msg:WriteBits(1, 1) -- Target type
							Msg:WriteBits(-19, 0)
							Msg:WriteBits(-19, 0)
							Msg:WriteBits(-19, 0)
							Msg:WriteByte(v.PointCount)
							--SB:Append(string.format("spc 2 0 0 0 %i ", v.PointCount))
						else
							EnsureAvailable(102, true)
							Msg:WriteBits(5, CIN_ACT_SPLINECAM)
							Msg:WriteBool(true) -- Targetted?
							Msg:WriteBits(1, 0) -- Target type
							Msg:WriteBits(10, v.Ent:GetIndex())
							Msg:WriteByte(v.PointCount)
							--SB:Append(string.format("spc 1 %i %i ", v.Ent:GetIndex(), v.PointCount))
						end
					elseif v.TargetType == 2 then
						EnsureAvailable(149, true)
						Msg:WriteBits(5, CIN_ACT_SPLINECAM)
						Msg:WriteBool(true) -- Targetted?
						Msg:WriteBits(1, 1) -- Target type
						WriteVector(Msg, v.Target)
						Msg:WriteByte(v.PointCount)
						--SB:Append(string.format("spc 2 %i %i %i %i ", v.Target.x, v.Target.y, v.Target.z, v.PointCount))
					else
						error("ProcessCinematic: Internal error: Invalid TargetType for linear cam")
					end
				else
					EnsureAvailable(149, true)
					Msg:WriteBits(5, CIN_ACT_SPLINECAM)
					Msg:WriteBool(false) -- Targetted?
					Msg:WriteByte(v.PointCount)
					--SB:Append(string.format("lc 0 %i ", v.PointCount))
				end
				-- Send algorithms
				Msg:WriteBits(2, v.Algorithms[1])
				Msg:WriteBits(2, v.Algorithms[2])
				Msg:WriteBits(2, v.Algorithms[3])
				Msg:WriteBits(2, v.Algorithms[4])
				Msg:WriteBits(2, v.Algorithms[5])
				Msg:WriteBits(2, v.Algorithms[6])
				if v.Targetted == true then
					-- 68 bits per cam point
					-- Determine if this message will be fragmented
					local points = #v.Points
					local pointsprocessed = 0
					if Msg:BitsRemaining() < (#v.Points * 68) then
						-- We have to fragment this message
						Msg:WriteBool(true)
						points = math.floor(Msg:BitsRemaining() / 68)
						Msg:WriteByte(points)
					else
						Msg:WriteBool(false)
					end
					local lastTime = v.Offset
					for k2, v2 in pairs(v.Points) do
						Msg:WriteBits(11, ConvTimeHR(v2.Offset - lastTime))
						lastTime = v2.Offset
						WriteVector(Msg, v2.Origin)
						
						points = points - 1
						pointsprocessed = pointsprocessed + 1
						
						if points == 0 and ((#v.Points - pointsprocessed) > 0) then
							-- We filled the current packet, flush it out and start a new one
							FinishMessage(false)
							Msg:WriteBits(5, CIN_ACT_CONTDATA)
							if Msg:BitsRemaining() < ((#v.Points - pointsprocessed) * 68) then
								-- We have to fragment this message
								Msg:WriteBool(true)
								points = math.floor(Msg:BitsRemaining() / 68)
								Msg:WriteByte(points)
							else
								Msg:WriteBool(false)
							end
						end
					end
				else
					-- 125 bits per cam point
					local points = #v.Points
					local pointsprocessed = 0
					if Msg:BitsRemaining() < (#v.Points * 125) then
						-- We have to fragment this message
						Msg:WriteBool(true)
						points = math.floor(Msg:BitsRemaining() / 125)
						Msg:WriteByte(points)
					else
						Msg:WriteBool(false)
					end
					local lastTime = v.Offset
					for k2, v2 in pairs(v.Points) do
						Msg:WriteBits(11, ConvTimeHR(v2.Offset - lastTime))
						lastTime = v2.Offset
						WriteVector(Msg, v2.Origin)
						WriteVector(Msg, v2.Angles)
						
						points = points - 1
						pointsprocessed = pointsprocessed + 1
						
						if points == 0 and ((#v.Points - pointsprocessed) > 0) then
							-- We filled the current packet, flush it out and start a new one
							FinishMessage(false)
							Msg:WriteBits(5, CIN_ACT_CONTDATA)
							if Msg:BitsRemaining() < ((#v.Points - pointsprocessed) * 125) then
								-- We have to fragment this message
								Msg:WriteBool(true)
								points = math.floor(Msg:BitsRemaining() / 125)
								Msg:WriteByte(points)
							else
								Msg:WriteBool(false)
							end
						end
					end
				end
			elseif v.ActType == CIN_ACT_MOTIONBLUR then
				if v.Fade == true then
					EnsureAvailable(38, true)
					Msg:WriteBits(5, CIN_ACT_MOTIONBLUR)
					Msg:WriteBool(true) -- Fading?
					Msg:WriteBits(11, ConvTimeHR(v.From))
					Msg:WriteBits(11, ConvTimeHR(v.To))
					Msg:WriteBits(10, ConvTime(v.Time))
					--SB:Append(string.format("mbf %i %i %i ", v.From, v.To, v.Time))
				else
					EnsureAvailable(17, true)
					Msg:WriteBits(5, CIN_ACT_MOTIONBLUR)
					Msg:WriteBool(false) -- Fading?
					Msg:WriteBits(11, ConvTimeHR(v.Value))
					--SB:Append(string.format("mb %i ", v.Value))
				end
			elseif v.ActType == CIN_ACT_FOV then
				if v.Fade == true then
					EnsureAvailable(34, true)
					Msg:WriteBits(5, CIN_ACT_FOV)
					Msg:WriteBool(true) -- Fading?
					Msg:WriteBits(-9, v.From)
					Msg:WriteBits(-9, v.To)
					Msg:WriteBits(10, ConvTime(v.Time))
					--SB:Append(string.format("mbf %i %i %i ", v.From, v.To, v.Time))
				else
					EnsureAvailable(15, true)
					Msg:WriteBits(5, CIN_ACT_FOV)
					Msg:WriteBool(false) -- Fading?
					Msg:WriteBits(-9, v.Value)
					--SB:Append(string.format("mb %i ", v.Value))
				end
			elseif v.ActType == CIN_ACT_COLORMOD_SCALE then
				if v.Disable == true then
					EnsureAvailable(6, true)
					Msg:WriteBits(5, CIN_ACT_COLORMOD_SCALE)
					Msg:WriteBool(true) -- Disable
					--SB:Append(string.format("cmso "))
				else
					if v.Fade == true then
						EnsureAvailable(89, true)
						Msg:WriteBits(5, CIN_ACT_COLORMOD_SCALE)
						Msg:WriteBool(false) -- Disable
						Msg:WriteBool(true) -- Fade
						Msg:WriteBits(12, round(v.From[1] * 100))
						Msg:WriteBits(12, round(v.From[2] * 100))
						Msg:WriteBits(12, round(v.From[3] * 100))
						Msg:WriteBits(12, round(v.To[1] * 100))
						Msg:WriteBits(12, round(v.To[2] * 100))
						Msg:WriteBits(12, round(v.To[3] * 100))
						Msg:WriteBits(10, ConvTime(v.Time))
						--SB:Append(string.format("cmsf %.3f %.3f %.3f %.3f %.3f %.3f %i ", v.From[1], v.From[2], v.From[3], v.To[1], v.To[2], v.To[3], v.Time))
					else
						EnsureAvailable(43, true)
						Msg:WriteBits(5, CIN_ACT_COLORMOD_SCALE)
						Msg:WriteBool(false) -- Disable
						Msg:WriteBool(false) -- Fade
						Msg:WriteBits(12, round(v.Value[1] * 100))
						Msg:WriteBits(12, round(v.Value[2] * 100))
						Msg:WriteBits(12, round(v.Value[3] * 100))
						--SB:Append(string.format("cms %.3f %.3f %.3f ", v.Value[1], v.Value[2], v.Value[3]))
					end
				end
			elseif v.ActType == CIN_ACT_COLORMOD_BIAS then
				if v.Disable == true then
					EnsureAvailable(6, true)
					Msg:WriteBits(5, CIN_ACT_COLORMOD_BIAS)
					Msg:WriteBool(true) -- Disable
					--SB:Append(string.format("cmbo "))
				else
					if v.Fade == true then
						EnsureAvailable(89, true)
						Msg:WriteBits(5, CIN_ACT_COLORMOD_BIAS)
						Msg:WriteBool(false) -- Disable
						Msg:WriteBool(true) -- Fade
						Msg:WriteBits(12, round(v.From[1] * 100))
						Msg:WriteBits(12, round(v.From[2] * 100))
						Msg:WriteBits(12, round(v.From[3] * 100))
						Msg:WriteBits(12, round(v.To[1] * 100))
						Msg:WriteBits(12, round(v.To[2] * 100))
						Msg:WriteBits(12, round(v.To[3] * 100))
						Msg:WriteBits(10, ConvTime(v.Time))
						--SB:Append(string.format("cmbf %.3f %.3f %.3f %.3f %.3f %.3f %i ", v.From[1], v.From[2], v.From[3], v.To[1], v.To[2], v.To[3], v.Time))
					else
						EnsureAvailable(43, true)
						Msg:WriteBits(5, CIN_ACT_COLORMOD_BIAS)
						Msg:WriteBool(false) -- Disable
						Msg:WriteBool(false) -- Fade
						Msg:WriteBits(12, round(v.Value[1] * 100))
						Msg:WriteBits(12, round(v.Value[2] * 100))
						Msg:WriteBits(12, round(v.Value[3] * 100))
						--SB:Append(string.format("cmb %.3f %.3f %.3f ", v.Value[1], v.Value[2], v.Value[3]))
					end
				end
			elseif v.ActType == CIN_ACT_COLORMOD_BC then
				if v.Disable == true then
					EnsureAvailable(6, true)
					Msg:WriteBits(5, CIN_ACT_COLORMOD_BC)
					Msg:WriteBool(true) -- Disable
					--SB:Append(string.format("cmco "))
				else
					if v.Fade == true then
						EnsureAvailable(69, true)
						Msg:WriteBits(5, CIN_ACT_COLORMOD_BC)
						Msg:WriteBool(false) -- Disable
						Msg:WriteBool(true) -- Fade
						Msg:WriteBits(-13, round(v.From[1] * 100))
						Msg:WriteBits(-13, round(v.From[2] * 100))
						Msg:WriteBits(-13, round(v.To[1] * 100))
						Msg:WriteBits(-13, round(v.To[2] * 100))
						Msg:WriteBits(10, ConvTime(v.Time))
						--SB:Append(string.format("cmcf %.3f %.3f %.3f %.3f %i ", v.From[1], v.From[2], v.To[1], v.To[2], v.Time))
					else
						EnsureAvailable(33, true)
						Msg:WriteBits(5, CIN_ACT_COLORMOD_BC)
						Msg:WriteBool(false) -- Disable
						Msg:WriteBool(false) -- Fade
						Msg:WriteBits(-13, round(v.Value[1] * 100))
						Msg:WriteBits(-13, round(v.Value[2] * 100))
						--SB:Append(string.format("cmc %.3f %.3f %.3f ", v.Value[1], v.Value[2]))
					end
				end
			elseif v.ActType == CIN_ACT_COLORMOD_FX then
				if v.Disable == true then
					EnsureAvailable(6, true)
					Msg:WriteBits(5, CIN_ACT_COLORMOD_FX)
					Msg:WriteBool(true) -- Disable
					--SB:Append(string.format("cmfo "))
				else
					if v.Fade == true then
						EnsureAvailable(67, true)
						Msg:WriteBits(5, CIN_ACT_COLORMOD_FX)
						Msg:WriteBool(false) -- Disable
						Msg:WriteBool(true) -- Fade
						Msg:WriteBits(2, v.FxID)
						Msg:WriteBits(12, round(v.From[1] * 100))
						Msg:WriteBits(12, round(v.From[2] * 100))
						Msg:WriteBits(12, round(v.To[1] * 100))
						Msg:WriteBits(12, round(v.To[2] * 100))
						Msg:WriteBits(10, ConvTime(v.Time))
						--SB:Append(string.format("cmff %i %.3f %.3f %.3f %.3f %i ", v.FxID, v.From[1], v.From[2], v.To[1], v.To[2], v.Time))
					else
						EnsureAvailable(33, true)
						Msg:WriteBits(5, CIN_ACT_COLORMOD_FX)
						Msg:WriteBool(false) -- Disable
						Msg:WriteBool(false) -- Fade
						Msg:WriteBits(2, v.FxID)
						Msg:WriteBits(12, round(v.Value[1] * 100))
						Msg:WriteBits(12, round(v.Value[2] * 100))
						--SB:Append(string.format("cmf %i %.3f %.3f ", v.FxID, v.Value[1], v.Value[2]))
					end
				end
			elseif v.ActType == CIN_ACT_COLORMOD_INV then
				if v.Disable == true then
					EnsureAvailable(6, true)
					Msg:WriteBits(5, CIN_ACT_COLORMOD_INV)
					Msg:WriteBool(true) -- Disable
					--SB:Append(string.format("cmio "))
				else
					if v.Fade == true then
						EnsureAvailable(31, true)
						Msg:WriteBits(5, CIN_ACT_COLORMOD_INV)
						Msg:WriteBool(false) -- Disable
						Msg:WriteBool(true) -- Fade
						Msg:WriteBits(7, round(v.From * 100))
						Msg:WriteBits(7, round(v.To * 100))
						Msg:WriteBits(10, ConvTime(v.Time))
						--SB:Append(string.format("cmif %.3f %.3f %i ", v.From, v.To, v.Time))
					else
						EnsureAvailable(16, true)
						Msg:WriteBits(5, CIN_ACT_COLORMOD_INV)
						Msg:WriteBool(false) -- Disable
						Msg:WriteBool(false) -- Fade
						Msg:WriteBits(2, v.FxID)
						Msg:WriteBits(7, round(v.Value * 100))
						--SB:Append(string.format("cmi %.3f ", v.FxID, v.Value))
					end
				end
			elseif v.ActType == CIN_ACT_HITCHCOCKEFFECT then
				self.CurrCamAct = v
				self.CurrCamStart = sys.Time()
				EnsureAvailable(163, true)
				Msg:WriteBits(5, CIN_ACT_HITCHCOCKEFFECT)
				WriteVector(Msg, v.Origin)
				WriteVector(Msg, v.Angles)
				Msg:WriteUShort(v.Width)
				Msg:WriteBits(-9, v.StartFov)
				Msg:WriteBits(-9, v.EndFov)
				Msg:WriteBits(10, ConvTime(v.Time))
				--SB:Append(string.format("he %.3f %.3f %.3f %.3f %.3f %.3f %i %i %i %i ", v.Origin.x, v.Origin.y, v.Origin.z, v.Angles.x, v.Angles.y, v.Angles.z, v.Width, v.StartFov, v.EndFov, v.Duration))
			elseif v.ActType == CIN_ACT_VIDEO then
				self.CurrCamAct = v
				self.CurrCamStart = sys.Time()
				EnsureAvailable(14 + (string.len(v.Filename) * 8), true)
				Msg:WriteBits(5, CIN_ACT_VIDEO)
				Msg:WriteBool(v.Loop)
				Msg:WriteString(v.Filename)
			elseif v.ActType == CIN_ACT_RESTORE then
				self.CurrCamAct = nil
				EnsureAvailable(5, true)
				Msg:WriteBits(5, CIN_ACT_RESTORE)
				--SB:Append("rdc ")
			elseif v.ActType == CIN_ACT_CAMSHAKE then
				if v.Fade then
					--SB:Append(string.format("cs %.3f %.3f %i ", v.Start, v.End, v.Time))
				else
					--SB:Append(string.format("cs %.3f ", v.Start))
				end
			elseif v.ActType == CIN_ACT_PVSLOCK then
				self.PVSLock = v.Pos
			elseif v.ActType == CIN_ACT_BLUR then
				if v.Fade then
					--SB:Append(string.format("bl %i %.3f %.3f %i ", v.Passes, v.Start, v.End, v.Time))
				else
					--SB:Append(string.format("bl %i %.3f ", v.Passes, v.Start))
				end
			elseif v.ActType == CIN_ACT_FINISH then
				FinishCin = true
			elseif v.ActType == CIN_ACT_CODE then
				v.Func(unpack(v.Args))
			end
			--[[if SB:Length() > 900 then
				-- Alright its gettin too big, purge
				self.Player:SendCommand(SB:ToString())
				SB:Clear()
				SB:Append("cin ")
			end]]
			self.Queue[k] = nil -- Remove this entry
		elseif v.Offset > self.Time then
			if NextActTime == -1 then
				NextActTime = v.Offset
			elseif v.Offset < NextActTime then
				NextActTime = v.Offset
			end
		elseif v.Offset < self.Time then
			-- Shouldnt happen but just in case:
			self.Queue[k] = nil
		end
	end
	--if SB:Length() > 4 then
	--	self.Player:SendCommand(SB:ToString())
	--end
	FinishMessage(true)
	
	if NextActTime == -1 or FinishCin == true then
		if self.AllowAbort then
			-- Disallow escape
			self.Player:SendCommand("cin de")
		end
		-- No more pending actions, end the animation
		timer.Remove(self.TimerName)
		self.Player.__sys.CurrCin = nil
		if self.FinishCallback then
			self.FinishCallback(self, unpack(self.CallbackArgs))
		end
	else
		-- Set up a timer for the next actions
		self.NextTime = NextActTime
		if self.TimerName == nil then
			self.TimerName = "Cin" .. self.Player:GetID()
			print("Cinematic for " .. tostring(self.Player) .. " had no name, assigning " .. self.TimerName .. " to it")
		end
		timer.Create(self.TimerName, NextActTime - self.Time, 1, self.ProcessCinematic, self)
	end
end
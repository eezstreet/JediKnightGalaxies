--[[ ------------------------------------------------
	Jedi Knight Galaxies Cinematic

	Tatooine Cinematic Teaser - Homestead
	
	Written by BobaFett
--------------------------------------------------]]

CIN.Name = "jkgcin2"
CIN.Description = "Official cinematic #1 - part 2"
CIN.AllowSkip = true


CIN.FinishCallback = nil
CIN.AbortCallback = nil
CIN.PlayerLeftCallback = nil

CIN.CinObj = nil

function CIN:SetFinishCallback(func)
	self.FinishCallback = func
end

function CIN:SetAbortCallback(func)
	self.AbortCallback = func
end

function CIN:SetPlayerLeftCallback(func)
	self.PlayerLeftCallback = func
end

function CIN:GetPlayer()
	if self.CinObj == nil then
		return nil
	elseif self.CinObj.Playing == false then
		return nil
	else
		return self.CinObj.Player
	end
end

function CIN:PlayCinematic(ply)
	if self.CinObj then
		-- Already playing..abort it
		self.CinObj.Playing = false
	end
	local cin = sys.CreateCinematic()
	--[[
		CinTime is a little helper function that makes working with camera trajectories easier
		With it, you can just provide the duration and it'll automatically work out the offset
	]]
	local function CinTime(t, time)
		t.time = t.time + time
		return t.time
	end
	cin:SetAllowAbort(true)
	cin:SetCallbacks(function(cinobj, self) 
						if self.FinishCallback then
							self.FinishCallback(self)
						end
						self.CinObj = nil
					 end,
					 function(cinobj, self)
						if self.AbortCallback then
							self.AbortCallback(self)
						end
						self.CinObj = nil
					 end,
					 function(cinobj, self)
						if self.PlayerLeftCallback then
							self.PlayerLeftCallback(self)
						end
						self.CinObj = nil
					 end,
					 self)
	
	-- Setup
	cin:AddFadeOut(0, 500)
	--[[
		SHOT 1:		Show of house
		Start: 0.5 seconds
		Duration: 7 seconds	
		End Time: 7.5 seconds
	]]
	cin:AddFadeIn(500, 1000)
	t = {time = 500}
	cin:StartLinearCam(500)
	cin:AddLinearCamPoint(CinTime(t, 0), Vector(1703, -967, 64), Vector(0, 153, 0))
	cin:AddLinearCamPoint(CinTime(t, 7000), Vector(1656, -977, 66), Vector(1, 153, 0))
	cin:FinishLinearCam()
	
	cin:AddFadeOut(3500, 4000)
	
	--[[
		SHOT 2:		Show of water vaporator
		Start: 7.5 seconds
		Duration: 5.25 seconds	
		End Time:  13 seconds
	]]
	cin:StartSkip(7500, 7500)
	cin:FinishSkip()
	
	cin:AddFadeIn(7500, 0)
	t = {time = 7500}
	cin:StartLinearCam(7500)
	cin:AddLinearCamPoint(CinTime(t, 0), Vector(704, -2076, 14), Vector(-8, -38, 0))
	cin:AddLinearCamPoint(CinTime(t, 5250), Vector(704, -2076, 14), Vector(-15, -38, 0))
	cin:FinishLinearCam()
	
	cin:AddColorModBC(12500, 0, 1, 0.85, 1, 250)
	
	cin:StartSkip(12750, 12750)
	cin:FinishSkip()
	cin:AddColorModBC(12750, 0.85, 1, 0, 1, 250)
	--[[
		SHOT 3:		Show of ship
		Start: 12.75 seconds
		Duration: 5 seconds	
		End Time: 17.75 seconds
	]]
	t = {time = 12750}
	cin:StartLinearCam(12750)
	cin:AddLinearCamPoint(CinTime(t, 0), Vector(3399, -2506, 69), Vector(4, 41, 0))
	cin:AddLinearCamPoint(CinTime(t, 5000), Vector(3436, -2432, 68), Vector(3, 43, 0))
	cin:FinishLinearCam()
	
	cin:AddFadeOut(17250, 500)
	
	--[[
		SHOT 4:		Double shot (inside)
		Start: 17.75 seconds
		Duration: 5.75 seconds	
		End Time:  23.25 seconds
	]]
	cin:StartSkip(17750, 17750)
	cin:FinishSkip()
	cin:AddFadeIn(17750, 0)
	
	t = {time = 17750}
	cin:StartLinearCam(17750)
	cin:AddLinearCamPoint(CinTime(t, 0), Vector(669, -720, -199), Vector(1, 179, 0))
	cin:AddLinearCamPoint(CinTime(t, 2750), Vector(570, -712, -203), Vector(0, 181, 0))
	cin:AddLinearCamPoint(CinTime(t, 0), Vector(542, -158, -254), Vector(3, -35, 0))
	cin:AddLinearCamPoint(CinTime(t, 2750), Vector(514, -137, -252), Vector(6, -35, 0))
	cin:FinishLinearCam()
	
	--[[
		SHOT 5:		Double shot (outside) static
		Start: 23.25 seconds
		Duration: 5 seconds	
		End Time:  28.25 seconds
	]]
	cin:StartSkip(23250, 23250)
	cin:FinishSkip()
	cin:AddFadeIn(23250, 250)
	cin:AddStaticCam(23250, Vector(346, 767, -308), Vector(0, 143, 0))
	
	cin:AddFadeIn(25750, 250)
	cin:AddStaticCam(25750, Vector(311, 1192, 8), Vector(6, 60, 0))
	
	--[[
		SHOT 6:		Inside shot
		Start: 28.25 seconds
		Duration: 2.75 seconds	
		End Time:  31 seconds
	]]
	
	cin:StartSkip(28250, 28250)
	cin:FinishSkip()
	cin:AddFadeIn(28250, 250)
	t = {time = 28250}
	cin:StartLinearCam(28250)
	cin:AddLinearCamPoint(CinTime(t, 0), Vector(413, -813, -228), Vector(2, -115, 0))
	cin:AddLinearCamPoint(CinTime(t, 2750), Vector(326, -1057, -231), Vector(0, -107, 0))
	cin:FinishLinearCam()
	
	--[[
		SHOT 7:		Inside shot (spline)
		Start: 31 seconds
		Duration: 2.75 seconds	
		End Time:  33.75 seconds
	]]
	cin:StartSkip(31000, 31000)
	cin:FinishSkip()
	cin:AddFadeIn(31000, 250)
	t = {time = 31000}
	cin:StartSplineCam(31000)
	cin:AddSplineCamPoint(CinTime(t, 0), Vector(3051, 1369, 284), Vector(7, -131, 0))
	cin:AddSplineCamPoint(CinTime(t, 1750), Vector(2697, 912, 200), Vector(4, -120, 0))
	cin:AddSplineCamPoint(CinTime(t, 1000), Vector(2504, 478, 184), Vector(7, -114, 0))
	cin:FinishSplineCam()
	
	--[[
		SHOT 7:		Static shot
		Start: 33.75 seconds
		Duration: 2.5 seconds	
		End Time:  36.25 seconds
	]]
	cin:StartSkip(33750, 33750)
	cin:FinishSkip()
	cin:AddStaticCam(33750, Vector(434, -370, -115), Vector(10, -37, 0))
	
	--[[
		SHOT 8:		Inside shot (spline) table rotate
		Start: 36.25 seconds
		Duration:  2.5 seconds	
		End Time:  38.75 seconds
	]]
	cin:StartSkip(36250, 36250)
	cin:FinishSkip()
	t = {time = 36250}
	cin:StartSplineCam(36250)
	cin:AddSplineCamPoint(CinTime(t, 0), Vector(358, 183, -241), Vector(5, 75, 0))
	cin:AddSplineCamPoint(CinTime(t, 1000), Vector(340, 391, -255), Vector(8, 50, 0))
	cin:AddSplineCamPoint(CinTime(t, 1500), Vector(267, 465, -256), Vector(5, 12, 0))
	cin:FinishSplineCam()
	
	cin:AddFadeOut(38250, 500)
	
	--[[
		SHOT 9:		Inside shot (spline) droids rotate
		Start: 38.75 seconds
		Duration: 3 seconds	
		End Time:  41.75 seconds
	]]
	cin:StartSkip(38750, 38750)
	cin:FinishSkip()
	cin:AddFadeIn(38750, 0)
	t = {time = 38750}
	cin:StartSplineCam(38750, Vector(309, -1418, -255))
	cin:SetSplineCamAlgos(1,1,1,0,0,0)
	cin:AddSplineCamPoint(CinTime(t, 0), Vector(258, -1199, -211))
	cin:AddSplineCamPoint(CinTime(t, 500), Vector(410, -1366, -211))
	cin:AddSplineCamPoint(CinTime(t, 500), Vector(393, -1562, -216))
	cin:AddSplineCamPoint(CinTime(t, 500), Vector(215, -1563, -219))
	cin:AddSplineCamPoint(CinTime(t, 750), Vector(209, -1415, -228))
	cin:AddSplineCamPoint(CinTime(t, 750), Vector(290, -1310, -205))
	cin:FinishSplineCam()
	
	--[[
		SHOT 10:		Inside shot (spline) water vaporator rotate
		Start: 41.75 seconds
		Duration: 5.5 seconds	
		End Time:  47  seconds
	]]
	cin:StartSkip(41750, 41750)
	cin:FinishSkip()
	t = {time = 41750}
	cin:StartSplineCam(41750, Vector(323, 2, -215))
	cin:SetSplineCamAlgos(1,1,1,0,0,0)
	cin:AddSplineCamPoint(CinTime(t, 0), Vector(636, -287, 206))
	cin:AddSplineCamPoint(CinTime(t, 1500), Vector(527, 177, -147))
	cin:AddSplineCamPoint(CinTime(t, 1000), Vector(118, 216, -149))
	cin:AddSplineCamPoint(CinTime(t, 1500), Vector(112, -180, -168))
	cin:AddSplineCamPoint(CinTime(t, 1500), Vector(473, -249, -177))
	cin:FinishSplineCam()
	
	--[[
		SHOT 11:		linear shot
		Start: 47 seconds
		Duration: 2.62  seconds	
		End Time:  49.62 seconds
	]]
	
	t = {time = 47000}
	cin:StartLinearCam(47000)
	cin:AddLinearCamPoint(CinTime(t, 0), Vector(-237, 797, 265), Vector(10, -61, 0))
	cin:AddLinearCamPoint(CinTime(t, 2620), Vector(65, 7, 139), Vector(7, -72, 0))
	cin:FinishLinearCam()
	
	--[[
		SHOT 12:		4 static shots
		Start: 49.62 seconds
		Duration: 2.5 seconds	
		End Time:  52.12  seconds
	]]
	cin:StartSkip(49620, 49620)
	cin:FinishSkip()
	t = {time = 49620}
	cin:AddStaticCam(CinTime(t, 0), Vector(425, -849, -245), Vector(0, -119, 0))
	cin:AddStaticCam(CinTime(t, 500), Vector(329, -1510, -218), Vector(26, 110, 0))
	cin:AddStaticCam(CinTime(t, 750), Vector(-304, -1400, 46), Vector(5, 14, 0))
	cin:AddStaticCam(CinTime(t, 500), Vector(1615, -2148, 91), Vector(2, 138, 0))
	--750
	cin:StartSkip(52370, 52370)
	cin:FinishSkip()
	cin:AddFinish(52370)
	
	self.CinObj = cin

	cin:PlayCinematic(ply)
end

--[[ ------------------------------------------------
	Jedi Knight Galaxies Cinematic

	Tatooine Cinematic Teaser - Mos Kreetle
	
	Written by BobaFett
--------------------------------------------------]]

CIN.Name = "jkgcin1"
CIN.Description = "Official cinematic #1"
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
		SHOT 1:		Slow move to crashed ship
		Start: 0.5 seconds
		Duration: 16 seconds	
		End Time: 16.5 seconds
	]]
	cin:AddFadeIn(500, 3000)
	t = {time = 500}
	cin:StartSplineCam(500)
	cin:SetSplineCamAlgos(1,1,1,0,0,0)
	cin:AddSplineCamPoint(CinTime(t, 0), Vector(1595, -617, 211), Vector(-3, -126, 0))
	cin:AddSplineCamPoint(CinTime(t, 8000), Vector(1341, -976, 209), Vector(-8, -124, 0))
	cin:AddSplineCamPoint(CinTime(t, 6000), Vector(1150, -1398, 293), Vector(-17, -134, 0))
	cin:AddSplineCamPoint(CinTime(t, 5000), Vector(910, -1581, 394), Vector(-19, -137, 0))
	cin:FinishSplineCam()
	
	cin:StartSkip(14500, 14500)
	cin:FinishSkip()
	cin:AddColorModBC(13500, 0, 1, 1, 1, 500)
	cin:AddFadeOut(14500, 2000)
	-- 16500
	--[[
		SHOT 2:		Entrace to Mos Kreetle
		Start: 16.75 seconds
		Duration: 10.5 seconds
		End Time: 27.25 seconds
	]]
	cin:AddColorModBC(16750, 0, 1)
	cin:AddFadeIn(16750, 250)
	
	t = {time = 16750}
	cin:StartSplineCam(16750)
	cin:SetSplineCamAlgos(1,1,1,0,0,0)
	cin:AddSplineCamPoint(CinTime(t, 0), Vector(5672, -1945, 1026), Vector(9, 153, 0))
	cin:AddSplineCamPoint(CinTime(t, 5000), Vector(4660, -1221, 722), Vector(10, 159, 0))
	cin:AddSplineCamPoint(CinTime(t, 4000), Vector(3382, -596, 320), Vector(8, 177, 0))
	cin:AddSplineCamPoint(CinTime(t, 2500), Vector(2729, -549, 231), Vector(2, 179, 0))
	cin:FinishSplineCam()
	
	cin:StartSkip(27000, 27000)
	cin:FinishSkip()
	cin:AddFadeOut(27000, 250)
	-- 27250
	--[[
		SHOT 3:		Show of Mos Kreetle
		Start: 27.25 seconds
		Duration: 10 seconds
		End Time:  37.25 seconds
	]]
	cin:AddFadeIn(27250, 250)
	t = {time = 27250}
	cin:StartSplineCam(27250)
	cin:SetSplineCamAlgos(1,1,1,0,0,0)
	cin:AddSplineCamPoint(CinTime(t, 0), Vector(1432, -1357, 51), Vector(-2, 152, 0))
	cin:AddSplineCamPoint(CinTime(t, 5500), Vector(1342, -765, 46), Vector(0, 160, 0))
	cin:AddSplineCamPoint(CinTime(t, 5000), Vector(1286, -406, 46), Vector(0, 186, 0))
	cin:FinishSplineCam()
	
	cin:StartSkip(33250, 33250)
	cin:FinishSkip()
	cin:AddFadeOut(33250, 4000)
	--[[
		SHOT 4:		Quick shots (3)
		Start: 37.50 seconds
		Duration: 10.25 seconds
		End Time:  47.75 seconds
	]]
	cin:AddFadeIn(37500, 250)
	t = {time = 37500}
	cin:StartLinearCam(37500)
	cin:AddLinearCamPoint(CinTime(t, 0), Vector(950, -742, 42), Vector(8, -28, 0))
	cin:AddLinearCamPoint(CinTime(t, 2000), Vector(1015, -720, 30), Vector(8, -28, 0))
	-- Immediately continue here, so do a junction point and proceed
	cin:AddLinearCamPoint(CinTime(t, 0), Vector(-234, -996, 34), Vector(4, -130, 0))
	cin:AddLinearCamPoint(CinTime(t, 3250), Vector(-331, -976, 31), Vector(4, -140, 0))
	-- And again
	cin:AddLinearCamPoint(CinTime(t, 0), Vector(-929, -925, 66), Vector(3, 144, 0))
	cin:AddLinearCamPoint(CinTime(t, 5500), Vector(-820, -697, 58), Vector(2, 161, 0))
	cin:FinishLinearCam()
	--[[
		SHOT 5:		Fly through
		Start: 47.75 seconds
		Duration: 11.75 seconds
		End Time: 59,5  seconds
	]]
	cin:StartSkip(47750, 47750)
		cin:AddFadeIn(47750, 0)
	cin:FinishSkip()
	cin:AddFov(47750, 130, -1, 2000)
	cin:AddMotionBlur(47750, 600, 0, 2000)
	cin:AddColorModFX(47750, 1, 1, 0.3, 0, 4, 2000)
	t = {time = 47750}
	cin:StartSplineCam(47750)
	cin:SetSplineCamAlgos(1,1,1,1,1,1)
	cin:AddSplineCamPoint(CinTime(t, 0), Vector(1939, 1101, 460), Vector(10, 248, 0))
	cin:AddSplineCamPoint(CinTime(t, 1500), Vector(1649, 455, 337), Vector(10, 248, 0))
	cin:AddSplineCamPoint(CinTime(t, 1000), Vector(1458, -143, 245), Vector(10, 248, 0))
	cin:AddSplineCamPoint(CinTime(t, 1000), Vector(1175, -640, 26), Vector(7, 248, 0))
	cin:AddSplineCamPoint(CinTime(t, 500), Vector(1058, -955, 21), Vector(-10, 273, 0))
	cin:AddSplineCamPoint(CinTime(t, 500), Vector(1099, -1193, 62), Vector(0, 254, 0))
	cin:AddSplineCamPoint(CinTime(t, 700), Vector(964, -1484, 40), Vector(-10, 208, 0))
	cin:AddSplineCamPoint(CinTime(t, 500), Vector(698, -1797, 68), Vector(0, 220, 0))
	cin:AddSplineCamPoint(CinTime(t, 300), Vector(593, -1874, 44), Vector(-16, 175, 50))
	cin:AddSplineCamPoint(CinTime(t, 1000), Vector(-9, -1556, 281), Vector(-14, 148, 0))
	cin:AddColorModScale(t.time, 1,1,1, 3, 1.8, 0.8, 3000)
	cin:AddColorModBC(t.time, 0,1,-0.1, 1, 2000)
	cin:AddSplineCamPoint(CinTime(t, 500), Vector(-514, -1289, 520), Vector(40, 227,-50))
	cin:AddSplineCamPoint(CinTime(t, 750), Vector(-881, -1639, 163), Vector(30, 258,0))
	cin:AddSplineCamPoint(CinTime(t, 750), Vector(-862, -2056, -32), Vector(0, 292,0))
	cin:AddSplineCamPoint(CinTime(t, 500), Vector(-693, -2331, -33), Vector(0, 349,-30))
	cin:AddSplineCamPoint(CinTime(t, 750), Vector(-418, -2219, -14), Vector(-45, 345,0))
	cin:AddSplineCamPoint(CinTime(t, 250), Vector(-418, -2219, -14), Vector(-45, 345,0))
	cin:AddColorModInv(t.time-250, 1, 0, 250)
	cin:AddColorModBC(t.time-250, -0.1, 1, 0, 1, 250)
	cin:AddColorModScale(t.time-250, 3, 1.8, 0.8,1,1,1, 250)
	
	cin:AddSplineCamPoint(CinTime(t, 500), Vector(-690, -2311, -30), Vector(-21, 364,0))
	cin:AddSplineCamPoint(CinTime(t, 250), Vector(-565, -2377, -36), Vector(0, 275,0))
	cin:AddSplineCamPoint(CinTime(t, 250), Vector(-546, -2564, -42), Vector(-10, 275,0))
	cin:AddSplineCamPoint(CinTime(t, 250), Vector(-544, -2755, -34), Vector(-10, 275,0))
	cin:FinishSplineCam()
	
	--[[
		SHOT 6:		Jabba's cantina
		Start: 59.5 seconds
		Duration: 4 seconds
		End Time:  63.5 seconds
	]]
	
	cin:StartSkip(59500, 59500)
	cin:FinishSkip()
	
	t = {time = 59500}
	cin:StartLinearCam(59500)
	cin:AddLinearCamPoint(CinTime(t, 0), Vector(2997, -956, -780), Vector(12, 50, 0))
	cin:AddLinearCamPoint(CinTime(t, 4000), Vector(3085, -1094, -790), Vector(15, 19, 0))
	cin:FinishLinearCam()
	
	--[[
		SHOT 7:		Ebon Hawk!
		Start: 63.5 seconds
		Duration: 27 seconds
		End Time:  90.5 seconds
	]]
	cin:StartSkip(63500, 63500)
	cin:FinishSkip()
	
	t = {time = 63500}
	cin:StartSplineCam(63500)
	cin:SetSplineCamAlgos(1,1,1,1,1,1)
	cin:AddSplineCamPoint(CinTime(t, 0), Vector(2778, 2999, -351), Vector(0, 89, 0))
	cin:AddSplineCamPoint(CinTime(t, 1000), Vector(2784, 3465, -324), Vector(-18, 90, 0))
	cin:AddSplineCamPoint(CinTime(t, 750), Vector(2793, 3798, -161), Vector(0, 90, 0))
	cin:AddSplineCamPoint(CinTime(t, 1000), Vector(2791, 3950, -161), Vector(0, 90, 0))
	cin:AddSplineCamPoint(CinTime(t, 1000), Vector(2359, 3941, -161), Vector(0, 0, 0))
	cin:AddFov(t.time - 250, -1, 130, 500);
	cin:AddMotionBlur(t.time - 250, 0, 600, 500);
	cin:AddColorModScale(t.time -250, 1, 1, 1, 1.2, 1.3, 2, 500)
	cin:AddColorModBC(t.time-250, 0, 1, -0.2, 2, 500)
	cin:AddSplineCamPoint(CinTime(t, 1250), Vector(3885, 3962, -173), Vector(0, -1, 0))
	cin:AddFov(t.time - 250, 130, -1, 500);
	cin:AddMotionBlur(t.time - 250, 600, 0, 500);
	cin:AddColorModScale(t.time -250, 1.2, 1.3, 2, 1, 1, 1, 500)
	cin:AddColorModBC(t.time-250, -0.2, 2, 0, 1, 500)
	cin:AddSplineCamPoint(CinTime(t, 750), Vector(4080, 3907, -179), Vector(2, -54, 0))
	cin:AddSplineCamPoint(CinTime(t, 750), Vector(4147, 3822, -171), Vector(1, -88, 0))
	cin:AddSplineCamPoint(CinTime(t, 750), Vector(4094, 3343, -158), Vector(5, 24, 0))
	cin:AddSplineCamPoint(CinTime(t, 1000), Vector(4259, 3470, -176), Vector(1, 99, 0))
	cin:AddSplineCamPoint(CinTime(t, 750), Vector(4117, 3873, -169), Vector(3, 144, 0))
	cin:AddSplineCamPoint(CinTime(t, 750), Vector(3980, 3951, -177), Vector(1, 179, 0))
	cin:AddSplineCamPoint(CinTime(t, 750), Vector(3680, 3951, -177), Vector(0, 111, 0))
	cin:AddSplineCamPoint(CinTime(t, 750), Vector(3597, 4136, -173), Vector(1, 21, 0))
	cin:AddSplineCamPoint(CinTime(t, 750), Vector(3597, 4258, -173), Vector(0, -20, 0))	
	cin:AddSplineCamPoint(CinTime(t, 750), Vector(3530, 4423, -172), Vector(0, 157, 0))	
	cin:AddSplineCamPoint(CinTime(t, 750), Vector(3240, 4490, -169), Vector(0, 99, 0))
	cin:AddSplineCamPoint(CinTime(t, 750), Vector(3206, 4748, -174), Vector(0, 90, 0))
	cin:AddSplineCamPoint(CinTime(t, 1000), Vector(3207, 4931, -151), Vector(1, 271, 0))
	cin:AddSplineCamPoint(CinTime(t, 1250), Vector(3211, 4059, -169), Vector(0, 271, 0))	
	
	cin:AddSplineCamPoint(CinTime(t, 1000), Vector(3292, 3965, -157), Vector(9, 213, 0))
	cin:AddSplineCamPoint(CinTime(t, 1000), Vector(3312, 3718, -155), Vector(5, 131, 0))
	cin:AddSplineCamPoint(CinTime(t, 750), Vector(3091, 3766, -154), Vector(8, 23, 0))
	cin:AddSplineCamPoint(CinTime(t, 500), Vector(3280, 3616, -154), Vector(8, 32, 0))
	cin:AddSplineCamPoint(CinTime(t, 750), Vector(3282, 3504, -169), Vector(3, 218, 0))
	cin:AddSplineCamPoint(CinTime(t, 1000), Vector(3274, 3370, -173), Vector(2, 142, 0))
	cin:AddSplineCamPoint(CinTime(t, 1000), Vector(3233, 3124, -169), Vector(1, 239, 0))
	cin:AddSplineCamPoint(CinTime(t, 2000), Vector(3147, 3019, -159), Vector(1, 344, 0))
	
	cin:AddSplineCamPoint(CinTime(t, 1000), Vector(3136, 2991, -161), Vector(4, 312, 0))
	cin:AddSplineCamPoint(CinTime(t, 1500), Vector(3103, 2931, -165), Vector(8, 299, 0))
	cin:FinishSplineCam()
	
	cin:AddFadeOut(86500, 4000)	
	
	--[[
		SHOT 8:		Ebon hawk, outside
		Start: 90.5 seconds
		Duration: 6 seconds
		End Time: 96.5  seconds
	]]
	
	cin:StartSkip(90500, 90500)
	cin:FinishSkip()
	
	t = {time = 90500}
	cin:AddFadeIn(90500, 500)
	cin:StartLinearCam(90500)
	cin:AddLinearCamPoint(CinTime(t, 0), Vector(1731, 1851, 66), Vector(9, 42, 0))
	cin:AddLinearCamPoint(CinTime(t, 6000), Vector(2546, 2153, -52), Vector(7, 49, 0))
	cin:FinishLinearCam()
	
	--[[
		SHOT 9:		Slave quarters
		Start: 96.5 seconds
		Duration: 5.25 seconds
		End Time: 101.75  seconds
	]]
	
	cin:StartSkip(96500, 96500)
	cin:FinishSkip()
	
	t = {time = 96500}
	cin:StartLinearCam(96500)
	cin:AddLinearCamPoint(CinTime(t, 0), Vector(-99, 3490, -187), Vector(-3, 60, 0))
	cin:AddLinearCamPoint(CinTime(t, 5250), Vector(-224, 3886, -54), Vector(2, 49, 0))
	cin:FinishLinearCam()
	
	--[[
		SHOT 10:		Millenium falcon
		Start: 101.75 seconds
		Duration: 5.5 seconds
		End Time: 107.25  seconds
	]]
	cin:StartSkip(101750, 101750)
	cin:FinishSkip()
	
	t = {time = 101750}

	cin:StartLinearCam(101750)
	cin:AddLinearCamPoint(CinTime(t, 0), Vector(967, 1076, 435), Vector(11, 135, 0))
	cin:AddLinearCamPoint(CinTime(t, 5500), Vector(534, 980, 324), Vector(11, 133, 0))
	cin:FinishLinearCam()
	
	--[[
		SHOT 11:		Dolly shot
		Start: 107.25 seconds
		Duration: 5.5 seconds
		End Time: 112.75  seconds
	]]
	cin:StartSkip(107250, 107250)
	cin:FinishSkip()
	cin:AddHitchcockEffect(107250, Vector(370, 246, 10), Vector(0, 180, 0), 90, 90, 15, 5500)
	
	--[[
		SHOT 12:		
		Start: 112.75 seconds
		Duration: 5.25 seconds
		End Time: 118  seconds
	]]
	
	cin:StartSkip(112750, 112750)
	cin:FinishSkip()
	cin:AddFov(112750, -1)
	
	t = {time = 112750}
	cin:StartLinearCam(112750)
	cin:AddLinearCamPoint(CinTime(t, 0), Vector(2114, -1383, -758), Vector(0, 50, 0))
	cin:AddLinearCamPoint(CinTime(t, 5250), Vector(2321, -1417, -747), Vector(0, 105, 0))
	cin:FinishLinearCam()
	
	--[[
		SHOT 13:		Town runthrough
		Start: 118 seconds
		Duration: 11.75  seconds
		End Time: 129.75 seconds
	]]

	cin:StartSkip(118000, 118000)
	cin:FinishSkip()
	t = {time = 118000}
	cin:StartSplineCam(118000)
	cin:SetSplineCamAlgos(1,1,1,1,1,1)
	cin:AddSplineCamPoint(CinTime(t, 0), Vector(-1355, -595, 16), Vector(0, 0, 0))
	cin:AddSplineCamPoint(CinTime(t, 2000), Vector(-970, -595, 16), Vector(0, -40, 0))
	cin:AddSplineCamPoint(CinTime(t, 1500), Vector(-250, -595, 16), Vector(0, 40, 0))
	cin:AddSplineCamPoint(CinTime(t, 1250), Vector(250, -595, 16), Vector(0, -40, 0))
	cin:AddSplineCamPoint(CinTime(t, 1000), Vector(940, -595, 16), Vector(0, 90, 0))
	cin:AddSplineCamPoint(CinTime(t, 1000), Vector(1615, -595, 16), Vector(0, -90, 0))
	cin:AddSplineCamPoint(CinTime(t, 1000), Vector(2175, -595, 16), Vector(0, -180, 0))
	cin:AddSplineCamPoint(CinTime(t, 1000), Vector(2700, -595, 16), Vector(5, -180, 0))
	cin:AddSplineCamPoint(CinTime(t, 750), Vector(3900, -595, 500), Vector(30, -180, 0))
	cin:AddSplineCamPoint(CinTime(t, 750), Vector(5045, -647, 778), Vector(20, -180, 0))
	cin:AddSplineCamPoint(CinTime(t, 750), Vector(5333, -805, 475), Vector(-15, -180, 0))
	-- This lasts 10.75 secs, though the whole shot lasts 11.25 (fade adds .25)
	cin:FinishSplineCam()
	cin:AddColorModBC(t.time-1000, 0, 1, 1, 1, 250)
	cin:AddFadeOut(t.time-750, 1000)
	
	--[[
		SHOT 14:		Med bay
		Start: 130 seconds
		Duration:  6.5 seconds
		End Time: 137 seconds
	]]

	cin:StartSkip(130000, 130000)
	cin:FinishSkip()
	
	cin:AddFadeIn(130000, 250)
	cin:AddColorModBC(130000, 0, 1, -0.2, 1, 7000)
	cin:AddColorModInv(130000, 1, 0, 750)
	cin:AddColorModScale(130000, 1, 1, 1, 1, 1, 4, 7000)
	t = {time = 130000}
	cin:StartSplineCam(130000)
	cin:SetSplineCamAlgos(2,2,2,1,1,1)
	cin:AddSplineCamPoint(CinTime(t, 0), Vector(1544, -1505, 57), Vector(15, 313, 0))
	cin:AddSplineCamPoint(CinTime(t, 2000), Vector(1537, -1652, 56), Vector(15, 293, 0))
	cin:AddSplineCamPoint(CinTime(t, 2000), Vector(1426, -1652, 63), Vector(15, 253, 0))
	cin:AddSplineCamPoint(CinTime(t, 2000), Vector(1339, -1656, 58), Vector(15, 223, 0))
	cin:AddSplineCamPoint(CinTime(t, 1000), Vector(1331, -1659, 82), Vector(20, 150, 0))
	cin:FinishSplineCam()
	
	--[[
		SHOT 15:		Port and falcon
		Start: 137 seconds
		Duration:  18.75 seconds
		End Time:  155.75 seconds
	]]
	--[[ 
		This shot features doors opening, so to make that go right, we'll set up a door opening function, and locate the doors we gotta use
	]]
	
	local function OpenDoor(ent)
		if ent and ent:IsValid() then
			ent:Use()
		end
	end
	local doormodels = { "*56", "*44", "*16", "*7" }
	local tmp = {}
	local doors = {}
	tmp = ents.GetByClass("func_door")
	local k,v,k2,v2
	for k,v in pairs(tmp) do
		for k2, v2 in pairs(doormodels) do
			if v:GetModel() == v2 then
				doors[k2] = v
			end
		end
	end
	
	cin:StartSkip(137000, 137000)
	cin:FinishSkip()
	
	cin:AddFadeIn(137000, 250)
	cin:AddColorModBC(137000)
	cin:AddColorModInv(137000)
	cin:AddColorModScale(137000)
	
	
	t = {time = 137000}
	cin:StartSplineCam(137000)
	cin:SetSplineCamAlgos(1,1,1,1,1,1)
	cin:AddSplineCamPoint(CinTime(t, 0), Vector(4, 3713, -220), Vector(0, 270, 0))
	cin:AddSplineCamPoint(CinTime(t, 750), Vector(8, 3338, -250), Vector(15, 308, 0))
	cin:AddSplineCamPoint(CinTime(t, 500), Vector(248, 3203, -300), Vector(10, 350, 0))
	cin:AddSplineCamPoint(CinTime(t, 500), Vector(527, 3133, -343), Vector(10, 300, 50))
	cin:AddColorModBC(t.time, 0, 1, 1, 1, 500)
	cin:AddColorModScale(t.time+500, 3, 2, 1)
	cin:AddColorModBC(t.time+500, 1, 1, -0.3, 1, 250)
	cin:AddCode(t.time, OpenDoor, doors[1])
	cin:AddSplineCamPoint(CinTime(t, 500), Vector(618, 2914, -350), Vector(0, 270, 30))
	cin:AddSplineCamPoint(CinTime(t, 750), Vector(651, 2486, -321), Vector(0, 270, 20))
	cin:AddSplineCamPoint(CinTime(t, 500), Vector(659, 2249, -318), Vector(0, 270, 0))
	cin:AddSplineCamPoint(CinTime(t, 500), Vector(610, 2009, -319), Vector(0, 270, 0))
	cin:AddSplineCamPoint(CinTime(t, 500), Vector(574, 1874, -317), Vector(0, 270, -20))
	cin:AddSplineCamPoint(CinTime(t, 500), Vector(604, 1691, -351), Vector(0, 270, 0))
	cin:AddSplineCamPoint(CinTime(t, 750), Vector(628, 1312, -355), Vector(0, 240, 0))
	cin:AddColorModBC(t.time, -0.3, 1, 0, 1, 750)
	cin:AddSplineCamPoint(CinTime(t, 250), Vector(573, 1133, -346), Vector(0, 200, 50))
	cin:AddSplineCamPoint(CinTime(t, 250), Vector(334, 1103, -348), Vector(0, 180, 0))
	cin:AddSplineCamPoint(CinTime(t, 500), Vector(-157, 1110, -343), Vector(0, 180, 0))
	cin:AddCode(t.time-300, OpenDoor, doors[2])
	cin:AddPVSLock(t.time-300, Vector(-305, 1159, -400))
	cin:AddMotionBlur(t.time, 0, 600, 250)
	cin:AddFov(t.time, -1, 130, 250)
	cin:AddSplineCamPoint(CinTime(t, 250), Vector(-345, 1159, -400), Vector(0, 226, 0))
	cin:AddSplineCamPoint(CinTime(t, 500), Vector(-365, 356, -38), Vector(-26, 270, 0))
	cin:AddMotionBlur(t.time, 600, 0, 250)
	cin:AddFov(t.time, 130, -1, 250)
	cin:AddPVSLock(t.time, nil)
	cin:AddSplineCamPoint(CinTime(t, 500), Vector(-362, 226, 38), Vector(0, 299, 0))
	cin:AddSplineCamPoint(CinTime(t, 500), Vector(-326, 29, 41), Vector(0, 360, 0))
	cin:AddSplineCamPoint(CinTime(t, 500), Vector(248, 29, 38), Vector(0, 360, 0))
	cin:AddColorModScale(t.time, 3, 2, 1, 1, 1 ,1, 750)
	cin:AddSplineCamPoint(CinTime(t, 500), Vector(396, 52, 40), Vector(0, 425, 0))
	cin:AddSplineCamPoint(CinTime(t, 500), Vector(430, 230, 37), Vector(0, 510, 0))
	cin:AddSplineCamPoint(CinTime(t, 500), Vector(273, 254, 34), Vector(0, 540, 0))
	cin:AddSplineCamPoint(CinTime(t, 750), Vector(110, 248, 41), Vector(0, 496, 0))
	cin:AddSplineCamPoint(CinTime(t, 750), Vector(35, 365, 44), Vector(0, 450, 0))
	cin:AddCode(t.time-200, OpenDoor, doors[3])
	cin:AddSplineCamPoint(CinTime(t, 1500), Vector(19, 795, 26), Vector(-10, 457, 0))
	cin:AddColorModInv(t.time, 1, 0, 400)
	cin:AddColorModBC(t.time, 0, 5, 0, 1,1200)
	cin:AddSplineCamPoint(CinTime(t, 1500), Vector(19, 795, 26), Vector(-10, 457, 0))
	cin:AddSplineCamPoint(CinTime(t, 1000), Vector(-106, 1345, -40), Vector(-14, 457, 0))
	cin:AddSplineCamPoint(CinTime(t, 1000), Vector(-142, 1690, 159), Vector(-19, 450, 0))
	cin:AddCode(t.time, OpenDoor, doors[4])
	cin:AddSplineCamPoint(CinTime(t, 750), Vector(-162, 1880, 265), Vector(0, 400, 0))
	cin:AddSplineCamPoint(CinTime(t, 1250), Vector(-117, 1975, 262), Vector(0, 532, 0))
	cin:FinishSplineCam()
	
	--[[
		SHOT 16:		Shots inside falcon (5 of em)
		Start: 155.75  seconds
		Duration:  18.75 seconds	(3.75 each)
		End Time:  174.5 seconds
	]]
	
	cin:AddColorModInv(155750, 1, 0, 250)
	
	t = {time = 155750}
	cin:StartSplineCam(155750)
	cin:SetSplineCamAlgos(1,1,1,1,1,1)
	cin:AddSplineCamPoint(CinTime(t, 0), Vector(-494, 2085, 250), Vector(0, 316, 0))
	cin:AddSplineCamPoint(CinTime(t, 1250), Vector(-383, 1976, 259), Vector(0, 342, 0))
	cin:AddSplineCamPoint(CinTime(t, 2500), Vector(-98, 1947, 254), Vector(0, 360, 0))
	cin:FinishSplineCam()
	
	cin:AddFadeIn(t.time, 250)
	cin:StartSplineCam(CinTime(t, 0))
	cin:SetSplineCamAlgos(1,1,1,1,1,1)
	cin:AddSplineCamPoint(CinTime(t, 0), Vector(56, 2021, 257), Vector(0, 293, 0))
	cin:AddSplineCamPoint(CinTime(t, 3000), Vector(254, 1610, 309), Vector(0, 310, 0))
	cin:AddSplineCamPoint(CinTime(t, 750), Vector(327, 1490, 325), Vector(0, 344, 0))
	cin:FinishSplineCam()
	
	CinTime(t, -250)
	cin:AddColorModBC(t.time, 1, 1, 0, 1, 250)
	cin:StartLinearCam(CinTime(t, 0))
	cin:AddLinearCamPoint(CinTime(t, 0), Vector(567, 1377, 333), Vector(-345, 110, 0))
	cin:AddLinearCamPoint(CinTime(t, 3750), Vector(577, 1515, 333), Vector(-345, -120, 0))
	cin:FinishLinearCam()
	
	
	cin:AddFadeIn(t.time, 250)
	cin:StartSplineCam(CinTime(t, 0))
	cin:SetSplineCamAlgos(1,1,1,1,1,1)
	cin:AddSplineCamPoint(CinTime(t, 0), Vector(124, 2564, 266), Vector(0, 377, 0))
	cin:AddSplineCamPoint(CinTime(t, 1875), Vector(180, 2636, 261), Vector(0, 306, 0))
	cin:AddSplineCamPoint(CinTime(t, 1875), Vector(305, 2626, 260), Vector(0, 226, 0))
	cin:FinishSplineCam()
	
	cin:AddColorModBC(t.time, 1, 1, 0, 1, 250)
	cin:StartSplineCam(CinTime(t, 0))
	cin:SetSplineCamAlgos(1,1,1,1,1,1)
	cin:AddSplineCamPoint(CinTime(t, 0), Vector(-333, 2531, 255), Vector(0, 160, 0))
	cin:AddSplineCamPoint(CinTime(t, 937), Vector(-421, 2448, 260), Vector(0, 177, 0))
	cin:AddSplineCamPoint(CinTime(t, 938), Vector(-425, 2320, 260), Vector(0, 182, 0))
	cin:AddSplineCamPoint(CinTime(t, 937), Vector(-399, 2197, 259), Vector(0, 193, 0))
	cin:AddSplineCamPoint(CinTime(t, 938), Vector(-381, 2104, 258), Vector(0, 205, 0))
	cin:FinishSplineCam()
	
	cin:StartSkip(174250, 174250)
	cin:FinishSkip()
	
	cin:AddFadeIn(174250, 500)
	
	cin:AddColorModBC(175250, 0, 1, 1, 1, 500)
	cin:AddColorModScale(175750, 3, 2, 1)
	cin:AddColorModBC(175750, 1, 1, -0.3, 1, 250)
	cin:StartSplineCam(CinTime(t, 0))
	cin:SetSplineCamAlgos(1,1,1,1,1,1)
	cin:AddSplineCamPoint(CinTime(t, 0), Vector(-1495, -2787, 496), Vector(40, 44, 0))
	cin:AddSplineCamPoint(CinTime(t, 2000), Vector(-545, -1953, 154), Vector(5, 44, 0))
	cin:AddSplineCamPoint(CinTime(t, 1500), Vector(-118, -1607, 136), Vector(3, 69, 0))
	cin:AddSplineCamPoint(CinTime(t, 2000), Vector(-91, -1057, 105), Vector(0, 90, 0))
	cin:AddSplineCamPoint(CinTime(t, 750), Vector(-91, -789, 93), Vector(0, 60, 0))
	cin:AddSplineCamPoint(CinTime(t, 1000), Vector(23, -512, 83), Vector(0, 0, 0))
	cin:AddColorModFX(t.time, 2, 0, 1, 1, 1, 1000)
	cin:AddMotionBlur(t.time, 0, 600, 750)
	cin:AddFov(t.time, -1, 130, 750)
	cin:AddSplineCamPoint(CinTime(t, 1000), Vector(2167, -544, 56), Vector(0, 0, 0))
	cin:AddSplineCamPoint(CinTime(t, 1000), Vector(4757, -483, 1787), Vector(-40, 0, 0))
	cin:AddColorModBC(t.time-750,  0, 1, 1, 1, 500)
	cin:AddFadeOut(t.time- 250, 250)
	cin:FinishSplineCam()
	
	cin:StartSkip(188000, 188000)
	cin:FinishSkip()
	cin:AddFinish(188000)
	
	self.CinObj = cin

	cin:PlayCinematic(ply)
end

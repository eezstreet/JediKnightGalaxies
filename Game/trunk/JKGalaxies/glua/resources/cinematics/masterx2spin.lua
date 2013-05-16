--[[ ------------------------------------------------
	Jedi Knight Galaxies Cinematic
	
	Written by BobaFett
--------------------------------------------------]]

CIN.Name = "masterx2spin"
CIN.Description = "MasterX's spin cinematic"


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
		
	local function CinTime(t, time)
		t.time = t.time + time
		return t.time
	end
	local cin = sys.CreateCinematic()
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
	t = {time = 0}
	cin:StartSplineCam(t.time)
	cin:SetSplineCamAlgos(1, 1, 1, 1, 1, 1)
	cin:AddSplineCamPoint(CinTime(t, 0), Vector(2303.962, 3851.820, -19.854), Vector(-0.659, 254.372, 0.000))
	cin:AddFadeIn(t.time, 1000)
	cin:AddColorModScale(t.time, 1.7, 1.4, 1)
	cin:AddColorModBC(t.time, 0, 1, 0, 20, 1000)
	cin:AddColorModBC(t.time+1000, 0, 20, 0, 1, 3000)
	cin:AddMotionBlur(t.time, 0, 6000, 1500)
	cin:AddMotionBlur(t.time+2500, 6000, 0, 2500)
	cin:AddSplineCamPoint(CinTime(t, 1000), Vector(5142.327, -1205.356, -20.141), Vector(0.110, 175.501, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 1000), Vector(439.032, -5287.018, -10.141), Vector(-0.879, 82.002, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 1000), Vector(-3407.263, 155.919, -20.141), Vector(-0.330, -13.694, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 1000), Vector(906.769, 3648.309, -20.141), Vector(-0.439, -89.929, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 1000), Vector(5123.377, 1259.063, -16.262), Vector(-0.220, -152.518, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 1000), Vector(3628.171, -3276.160, -24.017), Vector(0.000, -224.786, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 1000), Vector(341.015, -3504.665, -17.530), Vector(-0.659, -284.134, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 1000), Vector(-962.943, 879.153, 82.930), Vector(1.208, -403.813, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 1000), Vector(2394.060, 119.900, 356.865), Vector(22.549, -512.655, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 1000), Vector(797.760, -1178.950, 692.512), Vector(59.178, -648.616, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 2000), Vector(-32.580, -165.750, 1617.770), Vector(81.727, -743.214, 0.000))
	cin:AddColorModInv(t.time, 1, 0, 500)
	cin:AddMotionBlur(t.time, 0, 2000, 1500)
	cin:AddMotionBlur(t.time+1500, 2000, 0, 1500)
	cin:AddColorModFX(t.time, 1, 1, 1, 0, 1, 3000)
	cin:AddSplineCamPoint(CinTime(t, 1000), Vector(603.246, 228.384, 953.233), Vector(65.006, -830.775, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 1500), Vector(665.768, -694.444, 212.273), Vector(34.865, -854.978, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 1500), Vector(296.465, -1133.947, -177.540), Vector(21.006, -801.848, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 2000), Vector(425.879, -1454.176, -256.766), Vector(10.558, -883.575, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 2500), Vector(209.642, -1476.068, -270.195), Vector(10.228, -1042.965, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 3000), Vector(279.911, -1372.504, -266.331), Vector(3.516, -1144.611, 0.000))
	cin:AddColorModScale(t.time+2000, 1.7, 1.4, 1, 3, 1.7, 0.6, 1000)
	cin:AddColorModBC(t.time+2000, 0, 1, 0, 4, 1500)
	cin:AddSplineCamPoint(CinTime(t, 4000), Vector(296.245, -1397.244, -256.902), Vector(16.826, -1203.135, 0.000))
	cin:FinishSplineCam()
	cin:AddFadeOut(t.time, 1000)
	-- End offset: 27500

	
	cin:AddRestoreCam(28500)
	cin:AddFadeIn(28500, 1000)
	cin:AddFinish(29500)
	
	self.CinObj = cin

	cin:PlayCinematic(ply)
end

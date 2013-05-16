--[[ ------------------------------------------------
	Jedi Knight Galaxies Cinematic
	
	Written by BobaFett
--------------------------------------------------]]

CIN.Name = "masterxroll"
CIN.Description = "MasterX's roll cinematic"


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
	t = {time = 0 }
	
	cin:StartSplineCam(t.time)
	cin:SetSplineCamAlgos(1, 1, 1, 1, 1, 1)
	cin:AddSplineCamPoint(CinTime(t, 0), Vector(1675.839, -1486.198, 1348.511), Vector(-0.665, 117.933, 0.000))
	cin:AddFadeIn(t.time, 1500)
	cin:AddColorModScale(t.time, 1.7, 1.4, 1)
	cin:AddSplineCamPoint(CinTime(t, 8000), Vector(1662.042, -1070.294, 43.859), Vector(2.900, 139.800, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 4000), Vector(1583.691, -1008.276, 55.585), Vector(1.648, 143.108, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 2000), Vector(1328.149, -933.896, 23.681), Vector(17.820, 173.578, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 2000), Vector(942.426, -911.043, -111.305), Vector(21.226, 177.869, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 1000), Vector(770.705, -893.835, -202.599), Vector(16.500, 115.400, 15.000))
	cin:AddSplineCamPoint(CinTime(t, 1000), Vector(756.209, -720.826, -233.860), Vector(-10.300, 162.600, -20.000))
	cin:AddSplineCamPoint(CinTime(t, 3000), Vector(474.653, -686.454, -227.940), Vector(9.700, 190.700, -50.000))
	cin:AddSplineCamPoint(CinTime(t, 2000), Vector(336.124, -731.938, -237.310), Vector(9.900, 148.400, 20.000))
	cin:AddSplineCamPoint(CinTime(t, 1000), Vector(244.977, -598.640, -194.585), Vector(-9.900, 84.300, 10.000))
	cin:AddSplineCamPoint(CinTime(t, 3000), Vector(341.344, 168.810, -129.046), Vector(2.747, 261.942, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 2000), Vector(221.444, 177.916, -191.053), Vector(1.648, 277.971, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 2000), Vector(191.049, -52.151, -262.054), Vector(-0.439, 305.799, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 2000), Vector(375.016, -129.586, -257.401), Vector(-1.653, 224.462, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 2000), Vector(486.472, 72.301, -283.593), Vector(10.228, 290.182, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 3000), Vector(556.658, -37.433, -269.412), Vector(5.389, 281.821, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 4500), Vector(557.668, -60.049, -266.551), Vector(7.696, 289.704, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 2000), Vector(603.266, -207.466, -285.081), Vector(-4.180, 151.518, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 2000), Vector(491.484, -53.449, -268.597), Vector(-2.642, 159.324, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 2000), Vector(380.587, 137.213, -200.082), Vector(-5.834, 169.338, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 3000), Vector(123.585, 154.704, -148.304), Vector(-3.521, 169.008, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 3000), Vector(84.938, 166.091, 25.012), Vector(1.978, 158.445, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 2000), Vector(-64.659, 271.460, 19.443), Vector(8.026, 138.428, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 3500), Vector(-367.282, 623.675, 43.109), Vector(10.558, 2.247, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 3500), Vector(-195.888, 824.230, 206.266), Vector(23.538, -44.500, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 2000), Vector(73.760, 968.685, 428.931), Vector(33.986, -72.328, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 2000), Vector(137.397, 1013.012, 452.025), Vector(18.182, -75.361, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 2000), Vector(253.114, 978.932, 394.910), Vector(21.594, -76.459, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 2000), Vector(572.093, 491.462, 195.887), Vector(20.385, -89.000, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 2000), Vector(581.805, -12.598, 70.473), Vector(11.030, -86.358, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 2000), Vector(595.516, -292.280, 73.186), Vector(4.768, -86.248, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 2000), Vector(631.200, -791.500, 21.289), Vector(2.791, -88.231, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 4500), Vector(633.065, -851.899, 18.343), Vector(2.791, -88.231, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 4000), Vector(752.502, -226.674, -8.588), Vector(1.467, -116.060, 0.000))
	cin:AddFadeOut(t.time, 2000)
	cin:AddSplineCamPoint(CinTime(t, 2000), Vector(776.212, -182.196, 25.416), Vector(9.712, -107.809, 0.000))
	cin:FinishSplineCam()
	-- End offset: 88000

	cin:StartSkip(89000, 89000)
	cin:FinishSkip()
	cin:AddRestoreCam(89000)
	cin:AddFadeIn(89000, 1000)
	cin:AddFinish(90000)
	
	self.CinObj = cin

	cin:PlayCinematic(ply)
end

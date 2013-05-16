--[[ ------------------------------------------------
	Jedi Knight Galaxies Cinematic
	
	Written by BobaFett
--------------------------------------------------]]

CIN.Name = "masterx2"
CIN.Description = "MasterX's first cinematic"


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
	cin:SetSplineCamAlgos(0, 0, 0, 0, 0, 0)
	cin:AddSplineCamPoint(CinTime(t, 0), Vector(3371.662, -2120.019, 944.133), Vector(-87.891, 141.443, 0.000))
	cin:AddFadeIn(t.time, 1000)
	cin:AddSplineCamPoint(CinTime(t, 5000), Vector(2407.778, -1486.834, 618.753), Vector(-27.614, 145.294, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 3000), Vector(1910.050, -1306.536, 294.124), Vector(2.411, 140.735, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 3000), Vector(1503.147, -1091.169, 39.225), Vector(-0.780, 147.222, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 2000), Vector(1518.737, -625.948, 70.417), Vector(1.862, 158.225, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 1500), Vector(1265.208, -346.264, 64.206), Vector(0.544, 156.682, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 1600), Vector(900.313, -369.998, 122.949), Vector(12.640, 145.794, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 2000), Vector(538.651, -273.352, 48.162), Vector(41.682, 129.073, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 2000), Vector(262.367, -333.412, 30.379), Vector(37.062, 80.013, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 1800), Vector(104.002, -116.937, -82.862), Vector(30.899, 41.182, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 1800), Vector(206.275, 99.398, -194.889), Vector(21.220, 45.582, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 2700), Vector(386.791, 227.950, -248.919), Vector(10.992, 90.681, 0.000))
	cin:AddFadeOut(t.time+4000, 1000)
	cin:AddSplineCamPoint(CinTime(t, 5000), Vector(438.237, 506.811, -275.584), Vector(-5.620, 123.574, 0.000))
	cin:FinishSplineCam()
	-- End offset: 31400

	
	cin:AddRestoreCam(31400)
	cin:AddFadeIn(31400, 1000)
	cin:AddFinish(32400)
	
	self.CinObj = cin

	cin:PlayCinematic(ply)
end

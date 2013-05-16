--[[ ------------------------------------------------
	Jedi Knight Galaxies Cinematic
	
	Written by BobaFett
--------------------------------------------------]]

CIN.Name = "masterx"
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
	cin:AddFadeIn(t.time, 500)
	cin:AddSplineCamPoint(CinTime(t, 0), Vector(3307.613, -165.120, 744.594), Vector(22.000, 318.895, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 1000), Vector(3596.960, -390.404, 689.813), Vector(5.504, 235.223, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 1000), Vector(3560.841, -948.425, 434.468), Vector(13.310, 251.609, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 1000), Vector(3573.468, -1213.754, 372.538), Vector(-11.217, 258.981, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 1000), Vector(3836.765, -1653.704, 652.067), Vector(-16.276, 246.874, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 1000), Vector(4331.513, -2519.439, 861.912), Vector(-6.378, 183.840, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 2000), Vector(3195.642, -3226.060, 846.819), Vector(-6.817, 76.152, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 2000), Vector(2822.047, -2902.366, 851.633), Vector(-14.738, 60.315, 0.000))
	cin:AddFadeOut(t.time, 2000)
	cin:AddSplineCamPoint(CinTime(t, 2000), Vector(2824.432, -2675.976, 890.922), Vector(-15.068, 63.951, 0.000))
	cin:FinishSplineCam()
	-- End offset: 11000
	
	cin:AddRestoreCam(13000)
	cin:AddFadeIn(13000, 1000)
	cin:AddFinish(14000)
	
	self.CinObj = cin

	cin:PlayCinematic(ply)
end

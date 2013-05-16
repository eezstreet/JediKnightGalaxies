--[[ ------------------------------------------------
	Jedi Knight Galaxies Cinematic

	Cinematic Test
	
	Written by BobaFett
--------------------------------------------------]]

CIN.Name = "dollytest"
CIN.Description = "Tests the hitchcock effect"


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
	
	cin:AddFadeOut(0, 500)
	cin:AddFadeIn(500, 500)
	cin:StartSplineCam(500)
	cin:SetSplineCamAlgos(0,0,0,1,1,1)
	cin:AddSplineCamPoint(500, Vector(8425, 4860, -2070), Vector(-80, 0, 0))
	cin:AddSplineCamPoint(3000, Vector(8425, 4860, 0), Vector(-60, 180, 0))
	cin:AddSplineCamPoint(4000, Vector(8425, 4860, 250), Vector(-20, 180, 0))
	cin:AddSplineCamPoint(5000, Vector(7915, 4860, 200), Vector(0, 180, 0))
	cin:AddSplineCamPoint(6000, Vector(7600, 4815, 200), Vector(0, 250, -90))
	cin:AddSplineCamPoint(7000, Vector(7450, 4380, 200), Vector(0, 290, 180))
	cin:AddSplineCamPoint(8000, Vector(7580, 3850, 180), Vector(0, 315, 360))
	cin:AddSplineCamPoint(9000, Vector(7870, 3585, 200), Vector(0, 315, 360))
	cin:FinishSplineCam()
	cin:AddColorMod(500, 0.5, 0, 0)
	cin:AddFov(500, 120)
	cin:AddColorMod(2500, 0.5, 0, 0, 1, 1, 1, 1500)
	cin:AddFov(3500, 120, 100, 750)
	cin:AddMotionBlur(500,600)
	cin:AddMotionBlur(3500, 600, 0, 750)
	cin:AddColorMod(6000, 1, 1, 1, 0.3, 0.8, 0.3, 1000)
	cin:AddColorMod(8000, 0.3, 0.8, 0.3, 1, 1, 1, 2500)
	
	cin:AddHitchcockEffect(10000, Vector(7870, 3585, 200), Vector(0, 315, 0), 90, 100, 10, 3000)
	
	cin:AddFadeOut(14000,500)
	cin:AddFov(14500, 140)
	cin:AddMotionBlur(14500, 300)
	
	cin:AddFadeIn(14500, 500)
	
	cin:StartSplineCam(14500)
	cin:SetSplineCamAlgos(0,0,0,1,1,1)
	cin:AddSplineCamPoint(14500, Vector(9475, 1350, 90), Vector(0, 180, 0))
	cin:AddSplineCamPoint(16500, Vector(8615, 1330, 70), Vector(0, 180, 0))
	cin:AddSplineCamPoint(18500, Vector(7925, 1650, 70), Vector(0, 90, 0))
	cin:AddSplineCamPoint(20500, Vector(7820, 2040, 105), Vector(0, 90, 0))
	cin:AddSplineCamPoint(21500, Vector(7900, 2360, 100), Vector(0, 45, 0))
	cin:AddSplineCamPoint(22500, Vector(8300, 2515, 95), Vector(0, 0, 0))
	cin:AddSplineCamPoint(23500, Vector(9000, 2515, 150), Vector(0, 0, 0))
	cin:FinishSplineCam()
	
	cin:AddFadeOut(23000,500)
	cin:AddFov(24000, -1)
	cin:AddMotionBlur(24000, 0)
	cin:AddFadeIn(24000, 500)
	
	cin:StartSplineCam(24000)
	cin:SetSplineCamAlgos(2,2,2,1,1,1)
	cin:AddSplineCamPoint(24000, Vector(6830, 1745, 210), Vector(0, 45, 0))
	cin:AddSplineCamPoint(29000, Vector(8220, 3121, 200), Vector(0, 35, 0))
	cin:AddSplineCamPoint(30000, Vector(8225, 3170, 275), Vector(0, 0, 0))
	cin:FinishSplineCam()
	cin:AddMotionBlur(24000, 1500)
	cin:AddColorMod(24000, 1, 1, 1, 1, 0, 0, 1000)
	cin:AddColorMod(25000, 1, 0, 0, 1, 1, 0, 1000)
	cin:AddColorMod(26000, 1, 1, 0, 0, 1, 0, 1000)
	cin:AddColorMod(27000, 0, 1, 0, 0, 1, 1, 1000)
	cin:AddColorMod(28000, 0, 1, 1, 0, 0, 1, 1000)
	cin:AddColorMod(29000, 0, 0, 1, 1, 1, 1, 1000)
	cin:AddMotionBlur(30000, 1500, 0, 250)
	
	
	cin:AddHitchcockEffect(30000, Vector(8225, 3170, 275), Vector(0, 0, 0), 90, -1, 15, 2000)
	
	cin:AddFadeOut(33000,500)
	cin:AddFov(33500, -1)
	cin:AddFadeIn(33500, 500)
	cin:AddColorMod(33500)
	
	cin:AddRestoreCam(33500)
	cin:AddFinish(33500)
	
	self.CinObj = cin

	cin:PlayCinematic(ply)
end

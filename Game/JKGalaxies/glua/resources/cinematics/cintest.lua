--[[ ------------------------------------------------
	Jedi Knight Galaxies Cinematic

	Cinematic Test
	
	Written by BobaFett
--------------------------------------------------]]

CIN.Name = "cintest"
CIN.Description = "Test of the cinematic system"
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
	
	cin:AddFadeOut(0, 500)
	cin:AddStaticCam(500, Vector(9300, 2000, 460), Vector(30, -140, 0))
	cin:AddFadeIn(500, 500)
	cin:StartLinearCam(2500, Vector(8300, 1350, 20))
	cin:AddLinearCamPoint(2500, Vector(9280, 2675, 760))
	cin:AddLinearCamPoint(5000, Vector(9280, 1350, 760))
	cin:AddLinearCamPoint(5750, Vector(9280, 1350, 760))
	cin:AddLinearCamPoint(6250, Vector(7875, 1315, 25))
	cin:FinishLinearCam()
	cin:AddMotionBlur(5500, 1000)
	cin:AddMotionBlur(6500, 0)
	cin:AddFadeOut(5500,500)
	cin:StartSkip(5500, 5500)
	cin:FinishSkip()
	cin:AddFadeIn(6500,500)
	
	cin:AddColorModFX(6500, 1, 1, 40, 1, 1, 1000)
	cin:AddColorModFX(8500, 1, 1, 1, 0, 1, 500)
	cin:StartSplineCam(6500)
	cin:SetSplineCamAlgos(0,0,0,1,1,1) -- Origin on cubic, angles on b-spline
	cin:AddSplineCamPoint(6500, Vector(8000, 1095, 100), Vector(0,0,0))
	cin:AddSplineCamPoint(8500, Vector(8410, 1095, 80), Vector(0,90,0))
	cin:AddSplineCamPoint(10500, Vector(8410, 1435, 60), Vector(0,180,0))
	cin:AddSplineCamPoint(12500, Vector(8010, 1435, 40), Vector(0,180,0))
	cin:AddSplineCamPoint(13500, Vector(7840, 1300, 80), Vector(0,180,0))
	cin:AddSplineCamPoint(15500, Vector(7250, 1300, 150), Vector(0,90,0))
	cin:AddSplineCamPoint(17500, Vector(6950, 1675, 180), Vector(0,45,0))
	cin:AddSplineCamPoint(19500, Vector(7415, 2325, 230), Vector(0,45,0))
	cin:AddSplineCamPoint(20000, Vector(7900, 2800, 260), Vector(0,45,0))
	cin:AddSplineCamPoint(21000, Vector(8400, 3160, 310), Vector(0,120,0))
	cin:AddSplineCamPoint(23000, Vector(8030, 3160, 400), Vector(0,180,0))
	cin:FinishSplineCam()
	
	
	cin:AddColorModBC(18500, 0, 1, -0.2, 1, 750)
	cin:AddColorModScale(18500, 1, 1, 1, 1, 1, 4, 750)
	cin:AddColorModScale(20000, 1, 1, 4, 1, 1, 1, 500)
	cin:AddColorModBC(20000, -0.2, 1, 0, 1, 500)
	
	cin:AddMotionBlur(18500, 0, 750, 750)
	cin:AddMotionBlur(20000, 750, 0, 500)
	
	cin:AddFov(18500, -1, 130, 750)
	cin:AddFov(20000, 130, -1, 500)
	
	cin:AddFadeOut(24000, 500)
	cin:StartSkip(24000, 24000)
		cin:AddMotionBlur(24500, 0)
		cin:AddFov(24500, -1)
	cin:FinishSkip()
	cin:AddFadeIn(24500, 500)
	cin:AddStaticCam(24500, Vector(8390, 3165, 300), Vector(0,0,0))
	
	cin:StartSplineCam(26000)
	cin:SetSplineCamAlgos(0,0,0,1,1,1)
	cin:AddSplineCamPoint(26000, Vector(8390, 3165, 300), Vector(0,0,0))
	cin:AddSplineCamPoint(28000, Vector(8900, 3165, 300), Vector(0,0,0))
	cin:AddSplineCamPoint(30000, Vector(9280, 3450, 300), Vector(0,90,0))
	
	cin:AddSplineCamPoint(30500, Vector(9280, 4035, 300), Vector(0,135,360))
	cin:AddSplineCamPoint(31000, Vector(8850, 4475, 300), Vector(0,135,0))
	cin:AddSplineCamPoint(33000, Vector(8540, 4800, 300), Vector(0,150,0))
	cin:AddSplineCamPoint(36000, Vector(8021, 4850, 300), Vector(0,180,0))
	cin:AddSplineCamPoint(38000, Vector(7450, 4850, 400), Vector(0,180,0))
	cin:AddSplineCamPoint(39000, Vector(7260, 4950, 400), Vector(40,150,0))
	cin:AddSplineCamPoint(40000, Vector(7160, 5030, 200), Vector(80,150,0))
	cin:AddSplineCamPoint(41000, Vector(7160, 5030, 0), Vector(80,190,0))
	cin:AddSplineCamPoint(42000, Vector(7140, 4940, -400), Vector(60,270,0))
	cin:AddSplineCamPoint(44000, Vector(7150, 4460, -900), Vector(0,270,0))
	cin:AddSplineCamPoint(47000, Vector(6870, 3640, -500), Vector(-30,250,0))
	cin:AddSplineCamPoint(49000, Vector(6700, 3350, -300), Vector(-60,230,0))
	cin:AddSplineCamPoint(50000, Vector(6490, 3100, -100), Vector(-30,230,0))
	cin:AddSplineCamPoint(51000, Vector(6325, 3000, -40), Vector(-20,180,0))
	cin:AddSplineCamPoint(54000, Vector(5700, 2950, 500), Vector(0,180,0))
	cin:AddSplineCamPoint(56000, Vector(5364, 2972, 580), Vector(0,180,0))
	cin:FinishSplineCam()
	
	
	cin:AddColorModScale(29500, 1, 1, 1, .5, .3, .4, 500)
	cin:AddColorModScale(31000, .5, .3, .4, 1, 1, 1, 500)
	cin:AddMotionBlur(29500, 0, 750, 500)
	cin:AddMotionBlur(31500, 750, 0, 800)
	
	cin:AddFov(29500, -1, 130, 500)
	cin:AddFov(31000, 130, -1, 300)
	
	cin:AddColorModScale(40000, 1, 1, 1, .7, .3, .4, 2500)
	cin:AddColorModScale(45000, .7, .3, .4, 1, 1, 1, 2500)
	
	
	cin:AddColorModScale(54000, 1, 1, 1, 2, 0, 0, 3000)
	cin:AddColorModBC(54000, 0, 1, -0.2, 1, 3000)
	cin:AddColorModScale(57000, 2, 0, 0, 1, 1, 1, 250)
	cin:AddColorModBC(57000, -0.2, 1, 0, 1, 250)
	
	cin:StartLinearCam(57000)
	cin:AddLinearCamPoint(57000, Vector(5364, 2972, 580), Vector(0,180,0))
	cin:AddLinearCamPoint(58000, Vector(7639, 2950, 600), Vector(0,180,0))
	cin:FinishLinearCam()
	
	
	cin:AddMotionBlur(57000, 0, 750, 500)
	cin:AddMotionBlur(58000, 0)
	
	cin:AddFov(57000, -1, 130, 500)
	cin:AddFov(58000, -1)
	
	cin:AddFadeOut(57500, 500)
	cin:StartSkip(57500, 57500)
		cin:AddColorModScale(58000)
	cin:FinishSkip()
	cin:AddRestoreCam(58000)
	cin:AddFadeIn(58000, 1000)
	
	cin:AddFinish(60000)
	
	self.CinObj = cin

	cin:PlayCinematic(ply)
end

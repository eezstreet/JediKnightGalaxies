--[[ ------------------------------------------------
	Jedi Knight Galaxies Cinematic

	Cinematic Test
	
	Written by BobaFett
--------------------------------------------------]]

CIN.Name = "heviltest"
CIN.Description = "Cinematic of hevil spaceport"
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
	cin:AddStaticCam(500, Vector(-160, -3215, 357), Vector(10, 75, 0))
	cin:AddFadeIn(500, 1000)
	cin:AddFadeOut(3000,1200)
	cin:AddFadeIn(4000, 500)
	cin:StartSplineCam(4000)
	cin:SetSplineCamAlgos(1,1,1,2,2,2)
	cin:AddSplineCamPoint(4000, Vector(-785, -1000, 200), Vector(30, 55, -10))
	cin:AddSplineCamPoint(6000, Vector(450, -1300, 300), Vector(30, 90, 0))
	cin:AddSplineCamPoint(8000, Vector(1685, -1000, 200), Vector(30, 125, 10))
	cin:FinishSplineCam()
	cin:AddFadeOut(7500,500)
	cin:AddRestoreCam(8000)
	cin:AddFadeIn(8000, 500)
	cin:StartSplineCam(8000)
	cin:SetSplineCamAlgos(2,2,2,1,1,1)
	cin:AddSplineCamPoint(8000, Vector(500, -2140, -743), Vector(0, 90, 0))
	cin:AddSplineCamPoint(9000, Vector(500, -1800, -763), Vector(0, 90, 0))
	cin:AddSplineCamPoint(10000, Vector(500, -888, -733), Vector(0, 90, 0))
	cin:AddSplineCamPoint(10250, Vector(500, -715, -733), Vector(0, 90, 0))
	cin:AddSplineCamPoint(10750, Vector(660, -650, -733), Vector(0, 90, -5))
	cin:AddSplineCamPoint(11250, Vector(735, -512, -733), Vector(0, 90, -10))
	cin:AddSplineCamPoint(11750, Vector(650, -368, -733), Vector(0, 90, -5))
	cin:AddSplineCamPoint(12250, Vector(500, -310, -733), Vector(0, 90, 5))
	cin:AddSplineCamPoint(14000, Vector(500, 400, -733), Vector(0, 90, 0))
	cin:AddSplineCamPoint(14500, Vector(500, 575, -733), Vector(-80, 90, 0))
	cin:AddSplineCamPoint(16000, Vector(500, 575, -215), Vector(-80, 270, 0))
	cin:AddSplineCamPoint(16500, Vector(500, 575, -115), Vector(0, 270, 0))
	cin:FinishSplineCam()
	
	cin:AddFadeOut(9800,100)
	cin:AddFadeIn(10250,100)
	
	cin:AddFinish(17000)
	
	self.CinObj = cin

	cin:PlayCinematic(ply)
end

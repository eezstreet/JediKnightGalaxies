--[[ ------------------------------------------------
	Jedi Knight Galaxies Cinematic

	Cinematic Test
	
	Written by BobaFett
--------------------------------------------------]]

CIN.Name = "macetest"
CIN.Description = "Cinematic for mace"
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
	
	cin:StartLinearCam(0)
	cin:AddLinearCamPoint(0, Vector(2515, 875, -520), Vector(0,0,0))
	cin:AddLinearCamPoint(9000, Vector(2615, 960, -520), Vector(0,0,0))
	cin:FinishLinearCam()
	
	cin:StartSkip(9000, 9000)
	cin:FinishSkip()
	
	cin:StartLinearCam(9000)
	cin:AddLinearCamPoint(9000, Vector(2615, 960, -520), Vector(0,0,0))
	cin:AddLinearCamPoint(10000, Vector(2900, 950, -570), Vector(-10,-160,0))
	cin:FinishLinearCam()
	
	cin:StartSkip(22000, 22000)
	cin:FinishSkip()
	
	cin:StartLinearCam(22000)
	cin:AddLinearCamPoint(22000, Vector(2900, 950, -570), Vector(-10,-160,0))
	cin:AddLinearCamPoint(23000, Vector(2665, 925, -555), Vector(-6,-32,0))
	cin:FinishLinearCam()
	
	cin:StartSkip(26000, 26000)
	cin:FinishSkip()
	
	cin:StartLinearCam(26000)
	cin:AddLinearCamPoint(26000, Vector(2665, 925, -555), Vector(-6,-32,0))
	cin:AddLinearCamPoint(27000, Vector(2870, 770, -565), Vector(-14,25,0))
	cin:FinishLinearCam()

	cin:StartSkip(31000, 31000)
	cin:FinishSkip()

	cin:StartLinearCam(31000)
	cin:AddLinearCamPoint(31000, Vector(2870, 770, -565), Vector(-14,25,0))
	cin:AddLinearCamPoint(32000, Vector(2870, 770, -565), Vector(0,40,0))
	cin:FinishLinearCam()
	
	cin:StartSkip(35000, 35000)
	cin:FinishSkip()

	
	cin:StartLinearCam(35000)
	cin:AddLinearCamPoint(35000, Vector(2870, 770, -565), Vector(0,40,0))
	cin:AddLinearCamPoint(36000, Vector(3262, 814, -543), Vector(0,15,0))
	cin:FinishLinearCam()
	
	cin:StartSkip(39000, 39000)
	cin:FinishSkip()
	
	cin:StartLinearCam(39000)
	cin:AddLinearCamPoint(39000, Vector(3262, 814, -543), Vector(0,15,0))
	cin:AddLinearCamPoint(41000, Vector(3340, 835, -535), Vector(0,35,0))
	cin:FinishLinearCam()
	
	cin:StartSkip(43500, 43500)
	cin:FinishSkip()
	
	cin:StartLinearCam(43500)
	cin:AddLinearCamPoint(43500, Vector(3340, 835, -535), Vector(0,35,0))
	cin:AddLinearCamPoint(45000, Vector(3363, 995, -550), Vector(-1,-51,0))
	cin:FinishLinearCam()
	
	cin:StartSkip(51000, 51000)
	cin:FinishSkip()
	
	cin:StartLinearCam(51000)
	cin:AddLinearCamPoint(51000, Vector(3363, 995, -550), Vector(-1,-51,0))
	cin:AddLinearCamPoint(53000, Vector(3346, 876, -570), Vector(0,-120,0))
	cin:FinishLinearCam()
	
	cin:StartSkip(58000, 58000)
	cin:FinishSkip()
	
	cin:StartLinearCam(58000)
	cin:AddLinearCamPoint(58000, Vector(3346, 876, -570), Vector(0,-120,0))
	cin:AddLinearCamPoint(59000, Vector(3346, 876, -570), Vector(-7,-120,0))
	cin:FinishLinearCam()
	
	cin:StartSkip(65000, 65000)
	cin:FinishSkip()
	
	cin:StartLinearCam(65000)
	cin:AddLinearCamPoint(65000, Vector(3346, 876, -570), Vector(-7,-120,0))
	cin:AddLinearCamPoint(67000, Vector(3342, 745, -543), Vector(4,-211,0))
	cin:FinishLinearCam()
	
	cin:StartSkip(72000, 72000)
	cin:FinishSkip()
	cin:AddFinish(72000)
	
	self.CinObj = cin

	cin:PlayCinematic(ply)
end

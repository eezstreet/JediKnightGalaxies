--[[ ------------------------------------------------
	Jedi Knight Galaxies Cinematic

	Cinematic Test
	
	Written by BobaFett
--------------------------------------------------]]

CIN.Name = "rolltest"
CIN.Description = "Tests rolling with a skybox"


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
	cin:StartLinearCam(500)
	cin:AddLinearCamPoint(500, Vector(-639, -1488, 536), Vector(-15, 50, 0))
	cin:AddLinearCamPoint(3500, Vector(1241, 904, 1288), Vector(-15, 50, 360))
	cin:FinishLinearCam()
	
	cin:AddRestoreCam(4000)
	cin:AddFinish(4000)
	
	self.CinObj = cin

	cin:PlayCinematic(ply)
end

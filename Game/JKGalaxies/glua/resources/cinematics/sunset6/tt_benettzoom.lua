--[[ ------------------------------------------------
	Jedi Knight Galaxies Cinematic
	Quest Cinematic
	
	Quest: Homestead alpha quest: Tusken Troubles
	
	Written by BobaFett
--------------------------------------------------]]

CIN.Name = "tt_benettzoom"
CIN.Description = "Tusken Troubles: Benett Zoom"


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
	cin:StartLinearCam(t.time)
	cin:AddLinearCamPoint(CinTime(t, 0), Vector(212.992, 582.810, 324.168), Vector(39.265, 294.445, 0.000))
	cin:AddLinearCamPoint(CinTime(t, 3000), Vector(532.800, -95.098, -254.156), Vector(8.575, 303.464, 0.000))
	cin:FinishLinearCam()
	-- End offset: 3000
	cin:AddFadeIn(0, 500)
	
	cin:AddFadeOut(4000, 1000)
	
	cin:AddRestoreCam(5000)
	cin:AddFadeIn(5000, 1000)
	cin:AddFinish(6000)
	
	self.CinObj = cin

	cin:PlayCinematic(ply)
end

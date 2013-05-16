--[[ ------------------------------------------------
	Jedi Knight Galaxies Cinematic
	
	Written by BobaFett
--------------------------------------------------]]

CIN.Name = "tram1_mid2to1"
CIN.Description = "Tram 1 - Midsection 2 to 1"


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
	cin:AddFov(t.time, 110)
	cin:AddFadeIn(t.time, 1000)
	cin:StartLinearCam(t.time)
	cin:AddLinearCamPoint(CinTime(t, 0), Vector(-4065.875, -727.172, 690.580), Vector(0.994, 3.702, 0.000))
	cin:AddLinearCamPoint(CinTime(t, 4000), Vector(-7439.051, -732.939, 670.969), Vector(1.653, 4.801, 0.000))
	cin:AddFadeOut(t.time-1250, 1000)
	cin:AddFov(t.time, -1)
	cin:FinishLinearCam()
	-- End offset: 4000
	cin:AddFinish(4000)
	
	self.CinObj = cin

	cin:PlayCinematic(ply)
end

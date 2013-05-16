--[[ ------------------------------------------------
	Jedi Knight Galaxies Cinematic
	
	Written by BobaFett
--------------------------------------------------]]

CIN.Name = "leviathanbridge2"
CIN.Description = "Leviathan Bridge 2"


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
	cin:AddLinearCamPoint(CinTime(t, 0), Vector(896.592, 954.325, 58.426), Vector(5.499, 251.362, 0.000))
	cin:AddLinearCamPoint(CinTime(t, 6000), Vector(893.202, 944.276, 57.405), Vector(5.499, 251.362, 0.000))
	cin:FinishLinearCam()
	-- End offset: 6000


	cin:AddRestoreCam(6000)
	cin:AddFinish(6000)
	
	self.CinObj = cin

	cin:PlayCinematic(ply)
end

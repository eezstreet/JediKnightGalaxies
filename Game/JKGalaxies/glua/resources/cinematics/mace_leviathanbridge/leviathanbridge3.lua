--[[ ------------------------------------------------
	Jedi Knight Galaxies Cinematic
	
	Written by BobaFett
--------------------------------------------------]]

CIN.Name = "leviathanbridge3"
CIN.Description = "Leviathan Bridge 3"


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
	cin:AddLinearCamPoint(CinTime(t, 0), Vector(896.571, 871.000, 58.103), Vector(7.795, 106.820, 0.000))
	cin:AddLinearCamPoint(CinTime(t, 5000), Vector(891.178, 887.744, 56.366), Vector(5.592, 107.924, 0.000))
	cin:FinishLinearCam()


	cin:AddRestoreCam(5000)
	cin:AddFinish(5000)
	
	self.CinObj = cin

	cin:PlayCinematic(ply)
end

--[[ ------------------------------------------------
	Jedi Knight Galaxies Cinematic
	
	Written by BobaFett
--------------------------------------------------]]

CIN.Name = "leviathanbridge1"
CIN.Description = "Leviathan Bridge 1"


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
	cin:AddStaticCam(0, Vector(1429.889, -1651.209, 209.232), Vector(4.839, 117.147, 0.000))
	t = {time = 1000}
	cin:StartSplineCam(t.time)
	cin:SetSplineCamAlgos(0, 0, 0, 0, 0, 0)
	cin:AddSplineCamPoint(CinTime(t, 0), Vector(1429.889, -1651.209, 209.232), Vector(4.839, 117.147, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 3000), Vector(1068.960, -826.225, 119.942), Vector(4.070, 98.366, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 3000), Vector(954.077, 1052.848, 148.819), Vector(4.730, 101.151, 0.000))
	cin:FinishSplineCam()
	-- End offset: 6000


	cin:AddRestoreCam(13000)
	cin:AddFinish(13000)
	
	self.CinObj = cin

	cin:PlayCinematic(ply)
end

--[[ ------------------------------------------------
	Jedi Knight Galaxies Cinematic
	
	Written by BobaFett
--------------------------------------------------]]

CIN.Name = "tram1_arrive2"
CIN.Description = "Tram 1 - Arrival Station 2"


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
	cin:StartSplineCam(t.time)
	cin:SetSplineCamAlgos(1, 1, 1, 1, 1, 1)
	cin:AddSplineCamPoint(CinTime(t, 0), Vector(-2107.511, -2080.714, 645.396), Vector(1.099, 168.794, 0.000))
	cin:AddFadeIn(t.time, 1000)
	cin:AddSplineCamPoint(CinTime(t, 3000), Vector(82.110, -2134.119, 677.374), Vector(1.538, 166.564, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 5000), Vector(774.670, -2346.029, 746.883), Vector(6.158, 145.113, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 5000), Vector(867.608, -2354.950, 748.780), Vector(6.158, 145.113, 0.000))
	cin:AddFadeOut(t.time-1000, 1000)
	cin:FinishSplineCam()
	-- End offset: 13000
	
	cin:AddRestoreCam(13000)
	cin:AddFadeIn(13000, 1000)
	cin:AddFinish(14000)
	
	self.CinObj = cin

	cin:PlayCinematic(ply)
end

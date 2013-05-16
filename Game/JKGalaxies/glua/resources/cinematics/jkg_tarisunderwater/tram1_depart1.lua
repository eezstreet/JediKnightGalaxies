--[[ ------------------------------------------------
	Jedi Knight Galaxies Cinematic
	
	Written by BobaFett
--------------------------------------------------]]

CIN.Name = "tram1_depart1"
CIN.Description = "Tram 1 - Departure Station 1"


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
	t = {time = 1000}
	cin:AddFadeOut(0, 1000)
	cin:StartSplineCam(t.time)
	cin:SetSplineCamAlgos(0, 0, 0, 0, 0, 0)
	cin:AddSplineCamPoint(CinTime(t, 0), Vector(329.053, 466.827, 703.880), Vector(10.887, 129.661, 0.000))
	cin:AddFadeIn(t.time, 1000)
	cin:AddSplineCamPoint(CinTime(t, 4000), Vector(780.500, 535.477, 752.255), Vector(10.118, 149.464, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 4000), Vector(845.182, 866.798, 753.963), Vector(0.769, 179.604, 0.000))
	cin:AddFadeOut(t.time+3000, 2000)
	cin:AddSplineCamPoint(CinTime(t, 5000), Vector(688.310, 946.490, 713.606), Vector(-0.549, 183.565, 0.000))
	cin:FinishSplineCam()
	-- End offset: 13000

	cin:AddFinish(14000)
	
	self.CinObj = cin

	cin:PlayCinematic(ply)
end

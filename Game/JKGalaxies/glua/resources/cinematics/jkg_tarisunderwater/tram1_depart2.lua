--[[ ------------------------------------------------
	Jedi Knight Galaxies Cinematic
	
	Written by BobaFett
--------------------------------------------------]]

CIN.Name = "tram1_depart2"
CIN.Description = "Tram 1 - Departure Station 2"


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
	cin:AddSplineCamPoint(CinTime(t, 0), Vector(203.002, -2240.644, 701.403), Vector(4.735, 146.261, 0.000))
	cin:AddFadeIn(t.time, 1000)
	cin:AddSplineCamPoint(CinTime(t, 4000), Vector(805.040, -2329.443, 744.519), Vector(8.696, 143.954, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 4000), Vector(962.639, -1910.877, 748.744), Vector(6.713, 177.940, 0.000))
	cin:AddFadeOut(t.time+3000, 2000)
	cin:AddSplineCamPoint(CinTime(t, 5000), Vector(758.635, -1805.180, 726.437), Vector(5.724, 185.971, 0.000))
	cin:FinishSplineCam()
	-- End offset: 13000
	cin:AddFinish(14000)
	
	
	self.CinObj = cin

	cin:PlayCinematic(ply)
end

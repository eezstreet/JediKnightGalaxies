--[[ ------------------------------------------------
	Jedi Knight Galaxies Cinematic

	Cinematic Test
	
	Written by BobaFett
--------------------------------------------------]]

CIN.Name = "revolvetest"
CIN.Description = "Revolves the cam around yourself"


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
	cin:StartSplineCam(500, ply:GetEntity())
	local basevec = ply:GetPos() + Vector(0, 0, 50) -- 50 units above the player
	local radius = 125
	
	-- Make 1.125 rounds around the player, so we have time to fade out
	local i
	local xoffs
	local yoffs
	for i = 0 , 9 do
		xoffs = math.sin(math.rad(45 * i)) * radius
		yoffs = math.cos(math.rad(45 * i)) * radius
		cin:AddSplineCamPoint(500 + (500 * i), Vector(xoffs, yoffs, 0) + basevec)
	end
	cin:FinishSplineCam()
	
	cin:AddFadeOut(4500, 500)
	cin:AddRestoreCam(5000)
	cin:AddFadeIn(5000, 500)
	
	cin:AddFinish(5500)
	
	self.CinObj = cin

	cin:PlayCinematic(ply)
end

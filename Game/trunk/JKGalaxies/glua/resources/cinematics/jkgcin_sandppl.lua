--[[ ------------------------------------------------
	Jedi Knight Galaxies Cinematic

	Tatooine Cinematic Teaser - Homestead
	
	Written by BobaFett
--------------------------------------------------]]

CIN.Name = "jkgcin_sandppl"
CIN.Description = "Official cinematic - Sand people"
CIN.AllowSkip = true


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
	--[[
		CinTime is a little helper function that makes working with camera trajectories easier
		With it, you can just provide the duration and it'll automatically work out the offset
	]]
	local function CinTime(t, time)
		t.time = t.time + time
		return t.time
	end
	cin:SetAllowAbort(true)
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
	
	-- Setup
	cin:AddFadeOut(0, 500)
	--[[
		SHOT 1
		Start: 0.5 seconds
		Duration: 9 seconds	
		End Time: 9.5 seconds
	]]
	cin:AddFadeIn(500, 0)
	t = {time = 500}
	cin:StartSplineCam(500)
	cin:SetSplineCamAlgos(0,0,0,0,0,0)
	cin:AddSplineCamPoint(CinTime(t, 0), Vector(4421, 7824, 226), Vector(1, -119, 0))
	cin:AddSplineCamPoint(CinTime(t, 3000), Vector(3399, 5791, 331), Vector(1, -99, 0))
	cin:AddSplineCamPoint(CinTime(t, 3000), Vector(3278, 2670, 542), Vector(10, -129, 0))
	cin:AddSplineCamPoint(CinTime(t, 3000), Vector(2707, 2229, 397), Vector(15, -122, 0))
	cin:FinishSplineCam()
	
	--[[
		SHOT 2
		Start: 14.5 seconds
		Duration: 4.5 seconds	
		End Time: 5 seconds
	]]
	
	cin:AddFadeOut(14500, 500)
	
	cin:AddFinish(15000)
	
	self.CinObj = cin

	cin:PlayCinematic(ply)
end

--[[ ------------------------------------------------
	Jedi Knight Galaxies Cinematic

	Stargate waiting cinematic (Korriban)
	
	Written by BobaFett
--------------------------------------------------]]

CIN.Name = "sg_korriban"
CIN.Description = "Stargate waiting cinematic (Korriban)"


function CIN:SetUpCinematic(cin)
	local function CinTime(t, time)
		t.time = t.time + time
		return t.time
	end

	t = {time = 0 }
	
	cin:AddColorModFX(t.time, 1, 1, 1)
	cin:AddFadeIn(t.time, 500)
	cin:StartLinearCam(t.time)
	cin:AddLinearCamPoint(CinTime(t, 0), Vector(-14025.424, 1824.917, -898.822), Vector(6.405, 324.377, 0.000))
	cin:AddLinearCamPoint(CinTime(t, 10000), Vector(-14277.604, 1400.687, -898.822), Vector(6.185, 344.504, 0.000))
	cin:FinishLinearCam()
	
	cin:AddFadeIn(t.time, 500)
	cin:StartLinearCam(t.time)
	cin:AddLinearCamPoint(CinTime(t, 0), Vector(-14101.880, 1319.230, -859.286), Vector(6.405, 342.746, 0.000))
	cin:AddLinearCamPoint(CinTime(t, 5000), Vector(-13996.486, 1698.517, -843.541), Vector(14.216, 298.416, 0.000))
	cin:FinishLinearCam()
	
	cin:AddFadeIn(t.time, 500)
	cin:StartLinearCam(t.time)
	cin:AddLinearCamPoint(CinTime(t, 0), Vector(-14134.930, 941.797, -660.036), Vector(37.535, 398.958, 0.000))
	cin:AddLinearCamPoint(CinTime(t, 12000), Vector(-14106.352, 1839.215, -662.661), Vector(29.180, 304.470, 0.000))
	cin:FinishLinearCam()
	
	cin:AddFadeIn(t.time, 500)
	cin:StartLinearCam(t.time)
	cin:AddLinearCamPoint(CinTime(t, 0), Vector(-13478.804, 1836.736, -695.837), Vector(19.825, 219.441, 0.000))
	cin:AddLinearCamPoint(CinTime(t, 9000), Vector(-13495.837, 930.265, -695.837), Vector(19.715, 138.373, 0.000))
	cin:FinishLinearCam()
	
	cin:AddFadeIn(t.time, 500)
	cin:StartLinearCam(t.time)
	cin:AddLinearCamPoint(CinTime(t, 0), Vector(-14328.737, 1775.877, -994.555), Vector(-3.598, -32.437, 0.000))
	cin:AddLinearCamPoint(CinTime(t, 8000), Vector(-14244.550, 1098.857, -839.218), Vector(11.470, 33.673, 0.000))
	cin:FinishLinearCam()
		
	cin:AddFinish(t.time)
end



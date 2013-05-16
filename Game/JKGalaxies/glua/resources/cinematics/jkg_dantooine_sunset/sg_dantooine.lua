--[[ ------------------------------------------------
	Jedi Knight Galaxies Cinematic

	Stargate waiting cinematic (Dantooine)
	
	Written by BobaFett
--------------------------------------------------]]

CIN.Name = "sg_dantooine"
CIN.Description = "Stargate waiting cinematic (Dantooine)"


function CIN:SetUpCinematic(cin)
	local function CinTime(t, time)
		t.time = t.time + time
		return t.time
	end

	t = {time = 0 }
	
	cin:AddColorModFX(t.time, 1, 1, 1)
	cin:AddFadeIn(t.time, 500)
	cin:StartLinearCam(t.time)
	cin:AddLinearCamPoint(CinTime(t, 0), Vector(3913.438, -838.166, 433.094), Vector(1.648, 322.971, 0.000))
	cin:AddLinearCamPoint(CinTime(t, 10000), Vector(3864.985, -1356.744, 433.094), Vector(0.549, 394.036, 0.000))
	cin:FinishLinearCam()
	
	cin:AddFadeIn(t.time, 500)
	cin:StartLinearCam(t.time)
	cin:AddLinearCamPoint(CinTime(t, 0), Vector(3978.676, -1305.790, 506.087), Vector(13.640, 382.374, 0.000))
	cin:AddLinearCamPoint(CinTime(t, 8000), Vector(3960.812, -883.154, 506.087), Vector(14.299, 327.700, 0.000))
	cin:FinishLinearCam()
	
	cin:AddFadeIn(t.time, 500)
	cin:StartLinearCam(t.time)
	cin:AddLinearCamPoint(CinTime(t, 0), Vector(3730.160, -782.857, 544.628), Vector(11.217, 327.590, 0.000))
	cin:AddLinearCamPoint(CinTime(t, 12000), Vector(3898.872, -1531.812, 544.628), Vector(19.468, 415.701, 0.000))
	cin:FinishLinearCam()
	
	cin:AddFadeIn(t.time, 500)
	cin:StartLinearCam(t.time)
	cin:AddLinearCamPoint(CinTime(t, 0), Vector(4375.964, -1687.632, 671.742), Vector(28.817, 487.205, 0.000))
	cin:AddLinearCamPoint(CinTime(t, 14000), Vector(4455.634, -591.988, 671.742), Vector(22.549, 591.367, 0.000))
	cin:FinishLinearCam()
	
	cin:AddFadeIn(t.time, 500)
	cin:StartLinearCam(t.time)
	cin:AddLinearCamPoint(CinTime(t, 0), Vector(3867.690, -714.723, 649.541), Vector(23.318, 697.187, 0.000))
	cin:AddLinearCamPoint(CinTime(t, 9000), Vector(3738.602, -1382.000, 665.047), Vector(21.995, 388.449, 0.000))
	cin:FinishLinearCam()
		
	cin:AddFadeIn(t.time, 500)
	cin:StartLinearCam(t.time)
	cin:AddLinearCamPoint(CinTime(t, 0), Vector(3951.231, -1394.003, 544.585), Vector(15.397, 383.280, 0.000))
	cin:AddLinearCamPoint(CinTime(t, 7000), Vector(3974.753, -805.170, 544.585), Vector(23.318, 310.457, 0.000))
	cin:FinishLinearCam()
	
	cin:AddFinish(t.time)
end
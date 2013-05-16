--[[ ------------------------------------------------
	Jedi Knight Galaxies Cinematic

	Stargate waiting cinematic (Tatooine)
	
	Written by BobaFett
--------------------------------------------------]]

CIN.Name = "sg_tatooine"
CIN.Description = "Stargate waiting cinematic (Tatooine)"


function CIN:SetUpCinematic(cin)
	local function CinTime(t, time)
		t.time = t.time + time
		return t.time
	end

	t = {time = 0 }
	
	cin:AddColorModFX(t.time, 1, 1, 1)
	cin:AddFadeIn(t.time, 500)
	cin:StartLinearCam(t.time)
	cin:AddLinearCamPoint(CinTime(t, 0), Vector(2317.675, -181.580, 106.877), Vector(4.927, 325.294, 0.000))
	cin:AddLinearCamPoint(CinTime(t, 8000), Vector(2207.223, -736.321, 103.510), Vector(5.037, 367.866, 0.000))
	cin:FinishLinearCam()
	
	cin:AddFadeIn(t.time, 500)
	cin:StartLinearCam(t.time)
	cin:AddLinearCamPoint(CinTime(t, 0), Vector(2356.924, -801.899, 147.278), Vector(13.618, 372.371, 0.000))
	cin:AddLinearCamPoint(CinTime(t, 9000), Vector(2420.490, -240.055, 147.278), Vector(15.705, 328.041, 0.000))
	cin:FinishLinearCam()
	
	cin:AddFadeIn(t.time, 500)
	cin:StartLinearCam(t.time)
	cin:AddLinearCamPoint(CinTime(t, 0), Vector(2307.281, -91.405, 314.891), Vector(28.685, 333.984, 0.000))
	cin:AddLinearCamPoint(CinTime(t, 12000), Vector(2151.035, -863.784, 314.891), Vector(27.367, 375.458, 0.000))
	cin:FinishLinearCam()
	
	cin:AddFadeIn(t.time, 500)
	cin:StartLinearCam(t.time)
	cin:AddLinearCamPoint(CinTime(t, 0), Vector(2499.637, -1075.348, 304.957), Vector(33.305, 389.647, 0.000))
	cin:AddLinearCamPoint(CinTime(t, 15000), Vector(2602.003, 48.853, 304.957), Vector(40.128, 308.024, 0.000))
	cin:FinishLinearCam()
	
	cin:AddFadeIn(t.time, 500)
	cin:StartLinearCam(t.time)
	cin:AddLinearCamPoint(CinTime(t, 0), Vector(3122.448, -65.257, 318.363), Vector(38.035, 206.164, 0.000))
	cin:AddLinearCamPoint(CinTime(t, 9000), Vector(2991.085, -1036.246, 352.096), Vector(39.029, 130.265, 0.000))
	cin:FinishLinearCam()
		
	cin:AddFadeIn(t.time, 500)
	cin:StartLinearCam(t.time)
	cin:AddLinearCamPoint(CinTime(t, 0), Vector(2356.645, -813.621, 12.147), Vector(-8.597, 24.005, 0.000))
	cin:AddLinearCamPoint(CinTime(t, 7000), Vector(2368.237, -167.865, 1.672), Vector(-5.845, -35.837, 0.000))
	cin:FinishLinearCam()
	
	cin:AddFinish(t.time)
end
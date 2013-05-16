--[[ ------------------------------------------------
	Jedi Knight Galaxies Cinematic

	Stargate warptunnel
	
	Written by BobaFett
--------------------------------------------------]]

CIN.Name = "sg_warptunnel"
CIN.Description = "Stargate warptunnel (sg_test)"


function CIN:SetUpCinematic(cin)
	local function CinTime(t, time)
		t.time = t.time + time
		return t.time
	end

	cin:AddMotionBlur(0, 500)
	cin:AddFadeIn(0, 500)
	cin:AddColorModBC(0, .5, 1, -0.15, 3, 1000)
	cin:AddColorModInv(0, 1, 0, 500)
	t = {time = 0 }
	cin:StartSplineCam(t.time)
	cin:SetSplineCamAlgos(1, 1, 1, 1, 1, 1)
	cin:AddSplineCamPoint(CinTime(t, 0), Vector(494.082, 2.076, 0.048), Vector(1.648, 0.357, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 380), Vector(1799.766, 269.875, 18.644), Vector(-2.747, 17.408, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 187), Vector(2410.116, 496.083, 92.113), Vector(-6.707, 18.726, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 190), Vector(3019.501, 737.654, 213.748), Vector(-22.879, 33.580, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 122), Vector(3303.857, 976.890, 427.304), Vector(-32.228, 42.709, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 122), Vector(3522.193, 1279.292, 641.148), Vector(-18.040, 95.180, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 103), Vector(3545.573, 1610.908, 784.324), Vector(-15.068, 95.070, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 211), Vector(3317.564, 2316.516, 758.817), Vector(14.409, 123.008, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 108), Vector(3084.257, 2609.338, 691.696), Vector(2.747, 129.825, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 144), Vector(2760.090, 2998.101, 667.070), Vector(2.417, 128.727, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 175), Vector(2431.783, 3518.389, 651.943), Vector(-5.389, 110.248, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 160), Vector(2447.969, 4009.015, 926.691), Vector(-33.657, 84.287, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 137), Vector(2342.376, 4426.437, 1144.483), Vector(1.428, 51.400, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 280), Vector(3176.348, 4929.616, 1010.037), Vector(13.310, -9.432, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 148), Vector(3639.577, 4747.380, 865.119), Vector(38.386, -26.812, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 111), Vector(3849.608, 4601.364, 571.181), Vector(41.468, -48.483, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 161), Vector(4068.561, 4218.084, 219.970), Vector(44.769, -60.139, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 248), Vector(4597.958, 3872.408, -377.166), Vector(38.826, -9.102, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 151), Vector(4983.554, 3802.937, -736.184), Vector(42.347, -17.243, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 142), Vector(5337.183, 3667.755, -1063.156), Vector(27.828, -33.409, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 194), Vector(5837.439, 3262.762, -1279.976), Vector(9.020, -51.888, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 198), Vector(6160.815, 2653.976, -1198.563), Vector(-8.355, -62.672, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 202), Vector(6427.820, 2028.792, -993.234), Vector(-44.769, -87.643, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 143), Vector(6448.246, 1676.389, -635.218), Vector(-43.116, -86.429, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 206), Vector(6523.421, 1090.793, -218.232), Vector(-19.468, -76.201, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 154), Vector(6802.184, 651.989, -73.635), Vector(-8.910, -51.010, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 200), Vector(7343.935, 220.378, 35.555), Vector(-7.476, -29.559, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 118), Vector(7715.054, 64.015, 128.235), Vector(-14.848, -19.990, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 143), Vector(8130.706, -141.025, 322.872), Vector(-26.949, -30.004, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 148), Vector(8601.385, -336.963, 430.239), Vector(-6.487, -19.880, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 131), Vector(9056.243, -290.963, 476.178), Vector(11.217, 17.078, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 97), Vector(9374.509, -175.536, 432.757), Vector(7.696, 21.918, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 148), Vector(9864.188, -57.633, 300.095), Vector(20.237, 6.075, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 126), Vector(10285.517, -35.824, 162.942), Vector(16.496, 1.016, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 136), Vector(10749.622, -10.687, 50.611), Vector(10.887, 4.867, 0.000))
	cin:AddSplineCamPoint(CinTime(t, 143), Vector(11246.498, 13.089, -22.166), Vector(5.718, 0.687, 0.000))
	cin:FinishSplineCam()
	-- End offset: 5787
	
	cin:AddFadeOut(t.time - 2000, 1500)
	cin:AddColorModBC(t.time, 0, 1)
	cin:AddMotionBlur(t.time, 0)
	
	cin:AddFinish(t.time)
end
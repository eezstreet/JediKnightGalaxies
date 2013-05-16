--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Code
	Tram controls - Prototype
	
	Written by BobaFett
--------------------------------------------------]]

if RELOADING then
	return
end

--[[ Tram states:

	0:	In Station 1
	1: 	Preparing to leave station 1
	2:	Moving towards Tunnel 1
	3:	Teleporting to MidSection1
	4:	Moving from MidSection1 to MidSection2
	5:	Teleporting to Tunnel2
	6:	Moving from Tunnel2 towards station
	7:	Arrived in station 2
	8:	In station 2
	9: 	Preparing to leave station 2
	10:	Moving towards Tunnel 2
	11:	Teleporting to MidSection2
	12:	Moving from MidSection2 to MidSection1
	13:	Teleporting to Tunnel1
	14:	Moving from Tunnel1 towards station
	15:	Arrived in station 1
]]

local function TramDisplayUpdate()
	-- Refresh all display panels
	local k,v
	local modeS1, modeS2 -- 0 = 'arrival', 1 = 'departure'
	local timeS1, timeS2
	local minsS1, secsS1
	local minsS2, secsS2
	
	local secT, secS
	for k,v in pairs(JKG.Trams) do
		if v.DisplayData.Time == -1 then
			-- Clear the display
			v.Displays.S1Arrival:SetShaderFrame(0)
			v.Displays.S1Min:SetShaderFrame(0)
			v.Displays.S1SecT:SetShaderFrame(0)
			v.Displays.S1Sec:SetShaderFrame(0)
				
			v.Displays.S2Arrival:SetShaderFrame(0)
			v.Displays.S2Min:SetShaderFrame(0)
			v.Displays.S2SecT:SetShaderFrame(0)
			v.Displays.S2Sec:SetShaderFrame(0)
		else
			-- Determine the modes and times
			if v.InMotion then
				modeS1 = 0
				modeS2 = 0
				if v.DisplayData.Station == 1 then
					timeS1 = v.DisplayData.Time - sys.Time()
					timeS2 = timeS1 + v.TravelTime*2 + v.StationTime
				else
					timeS2 = v.DisplayData.Time - sys.Time()
					timeS1 = timeS2 + v.TravelTime*2 + v.StationTime
				end
			else
				if v.DisplayData.Station == 1 then
					modeS1 = 1
					modeS2 = 0
					timeS1 = v.DisplayData.Time - sys.Time()
					timeS2 = timeS1 + v.TravelTime
				else
					modeS1 = 0
					modeS2 = 1
					timeS2 = v.DisplayData.Time - sys.Time()
					timeS1 = timeS2 + v.TravelTime
				end
			end
			-- And process them 'n update the displays
			v.Displays.S1Arrival:SetShaderFrame(modeS1)
			v.Displays.S2Arrival:SetShaderFrame(modeS2)
			
			if timeS1 < 0 then timeS1 = 0 end
			if timeS2 < 0 then timeS2 = 0 end
			
			timeS1 = math.ceil(timeS1/1000)
			timeS2 = math.ceil(timeS2/1000)
			
			minsS1 = math.floor(timeS1/60)
			secsS1 = timeS1 - (minsS1*60)
			
			if minsS1 > 9 then minsS1 = 9 end
			secT = math.floor(secsS1/10)
			secS = secsS1 - (secT*10)
			
			v.Displays.S1Min:SetShaderFrame(minsS1)
			v.Displays.S1SecT:SetShaderFrame(secT)
			v.Displays.S1Sec:SetShaderFrame(secS)
			
			
			minsS2 = math.floor(timeS2/60)
			secsS2 = timeS2 - (minsS2*60)
			
			if minsS2 > 9 then minsS2 = 9 end
			secT = math.floor(secsS2/10)
			secS = secsS2 - (secT*10)
			
			v.Displays.S2Min:SetShaderFrame(minsS2)
			v.Displays.S2SecT:SetShaderFrame(secT)
			v.Displays.S2Sec:SetShaderFrame(secS)		
		end
	end
	timer.Simple(200, TramDisplayUpdate)
end

local function TramPlayCinematic(tram, cinematic, finalcin, initialcin)
	local cin, k, v
	cin = cinematics.Get(cinematic)
	if cin == nil then
		print("TRAM ERROR: Could not load cinematic ", cinematic)
		return
	end
	for k,v in pairs(tram.Passengers) do
		cin = cinematics.Get(cinematic)
		if finalcin then
			cin:SetFinishCallback(function(cin) cin:GetPlayer():StopCinematic() end)
		end
		if initialcin then
			v:StartCinematic()
		end
		cin:PlayCinematic(v)
	end
end

local function TramWarningSigns(tram, station)
	-- Toggle the tram warning signs
	if station == 1 then
		tram.Displays.S1Warning:Use(nil, nil)
	else
		tram.Displays.S2Warning:Use(nil, nil)
	end
end

local function TramAction(tram)
	if tram.State == -1 then
		-- Setup
		
		tram.State = 0
		tram.DisplayData.Station = 1
		tram.DisplayData.Time = sys.Time() + tram.StationTime
		
		timer.Simple(tram.StationTime, TramAction, tram)
		timer.Simple(tram.StationTime-3000, TramWarningSigns, tram, 1)
	elseif tram.State == 0 then
		tram.InMotion = true
		tram.DisplayData.Station = 2
		tram.DisplayData.Time = tram.TravelTime + sys.Time()

		-- First, check who's aboard
		local mins = tram.Tram:GetPos() + tram.TramBrushDelta + tram.Mins
		local maxs = tram.Tram:GetPos() + tram.TramBrushDelta + tram.Maxs
		local passengers = ents.FindInBox(mins, maxs)
		tram.Passengers = {}
		local k,v 
		for k,v in pairs(passengers) do
			if v:IsPlayer() then		-- Dont include anything other than players
				table.insert(tram.Passengers, v:ToPlayer())
				v:ToPlayer():SetNoMove(true)
			end
		end
	
		tram.State = 1
		TramPlayCinematic(tram, tram.Cinematics.Depart1, false, true)

		-- Make it depart, first close the doors
		tram.DoorsFront:Move(tram.Tram:GetPos(), 1000, TR_NONLINEAR_STOP)
		tram.DoorsBack:Move(tram.Tram:GetPos(), 1000, TR_NONLINEAR_STOP)
		-- Queue the next action
		timer.Simple(2000, TramAction, tram)
	elseif tram.State == 1 then
		-- We just closed the doors, start moving into the tunnel
		TramWarningSigns(tram, 1)
		tram.Tram:Move(tram.Positions.Tunnel1, 12000, TR_HARDEASEIN)
		tram.DoorsBack:Move(tram.Positions.Tunnel1, 12000, TR_HARDEASEIN)
		tram.DoorsFront:Move(tram.Positions.Tunnel1, 12000, TR_HARDEASEIN)
		tram.State = 2
		timer.Simple(12000, TramAction, tram)
	elseif tram.State == 2 then
		-- We reached the end of tunnel 1, tele the whole thing to midsection1 and start moving
		local trampos = tram.Tram:GetPos()
		local posdelta, k, v
		
		local oldtrampos = tram.Tram:GetPos()
		tram.Tram:SetPos(tram.Positions.MidSection1)
		tram.DoorsBack:SetPos(tram.Positions.MidSection1)
		tram.DoorsFront:SetPos(tram.Positions.MidSection1)
		
		for k,v in pairs(tram.Passengers) do
			-- Teleport our passengers with the tram
			if v:IsValid() then
				if tram.FlipPoint == 1 then
					-- Time to flip the people around
					local rotpoint =  v.Pos - (oldtrampos + tram.TramBrushDelta)
					local rotangs
					rotpoint = rotpoint * -1	-- Flip it around
					rotpoint = rotpoint + (oldtrampos + tram.TramBrushDelta)
					rotpoint.z = v.Pos.z	-- Cancel z-axis movement
					rotangs = v.Angles
					rotangs.y = rotangs.y + 180
					if rotangs.y > 360 then rotangs.y = rotangs.y - 360 end
					v.Angles = rotangs
					posdelta = rotpoint - trampos
				else
					posdelta = v.Pos - trampos
				end
				v:SetPos(tram.Positions.MidSection1 + posdelta, true)
			end
		end
		
		tram.State = 3
		timer.Simple(500, TramAction, tram)
	elseif tram.State == 3 then
		tram.Tram:Move(tram.Positions.MidSection2, 4000, TR_LINEAR_STOP)
		tram.DoorsBack:Move(tram.Positions.MidSection2, 4000, TR_LINEAR_STOP)
		tram.DoorsFront:Move(tram.Positions.MidSection2, 4000, TR_LINEAR_STOP)
		tram.State = 4
		TramPlayCinematic(tram, tram.Cinematics.Mid1to2, false, false)
		timer.Simple(4000, TramAction, tram)
	elseif tram.State == 4 then
		-- We reached the end of the midsection, teleport to tunnel 2
		local trampos = tram.Tram:GetPos()
		local posdelta, k, v
		
		local oldtrampos = tram.Tram:GetPos()
		tram.Tram:SetPos(tram.Positions.Tunnel2)
		tram.DoorsBack:SetPos(tram.Positions.Tunnel2)
		tram.DoorsFront:SetPos(tram.Positions.Tunnel2)
		
		for k,v in pairs(tram.Passengers) do
			-- Teleport our passengers with the tram
			if v:IsValid() then
				if tram.FlipPoint == 2 then
					-- Time to flip the people around
					local rotpoint =  v.Pos - (oldtrampos + tram.TramBrushDelta)
					local rotangs
					rotpoint = rotpoint * -1	-- Flip it around
					rotpoint = rotpoint + (oldtrampos + tram.TramBrushDelta)
					rotpoint.z = v.Pos.z	-- Cancel z-axis movement
					rotangs = v.Angles
					rotangs.y = rotangs.y + 180
					if rotangs.y > 360 then rotangs.y = rotangs.y - 360 end
					v.Angles = rotangs
					posdelta = rotpoint - trampos
				else
					posdelta = v.Pos - trampos
				end
				v:SetPos(tram.Positions.Tunnel2 + posdelta, true)
			end
		end
		tram.State = 5
		timer.Simple(500, TramAction, tram)
	elseif tram.State == 5 then
		tram.Tram:Move(tram.Positions.Station2, 12000, TR_HARDEASEOUT)
		tram.DoorsBack:Move(tram.Positions.Station2, 12000, TR_HARDEASEOUT)
		tram.DoorsFront:Move(tram.Positions.Station2, 12000, TR_HARDEASEOUT)
		tram.State = 6
		TramPlayCinematic(tram, tram.Cinematics.Arrive2, true, false)
		timer.Simple(13000, TramAction, tram)
	elseif tram.State == 6 then
		-- We just arrived in station 2, open the doors
		tram.DoorsFront:Move(tram.Positions.Station2 + tram.Positions.OffsetDoorFront, 1000, TR_NONLINEAR_STOP)
		tram.DoorsBack:Move(tram.Positions.Station2 + tram.Positions.OffsetDoorBack, 1000, TR_NONLINEAR_STOP)
		tram.State = 7
		timer.Simple(1000, TramAction, tram)
	elseif tram.State == 7 then
		tram.State = 8
		tram.InMotion = false
		tram.DisplayData.Station = 2
		tram.DisplayData.Time = tram.StationTime + sys.Time()
		-- And unfreeze the players
		for k,v in pairs(tram.Passengers) do
			v:SetNoMove(false)
		end
		timer.Simple(tram.StationTime, TramAction, tram)
		timer.Simple(tram.StationTime-3000, TramWarningSigns, tram, 2)
	elseif tram.State == 8 then
		tram.InMotion = true
		tram.DisplayData.Station = 1
		tram.DisplayData.Time = tram.TravelTime + sys.Time()

		-- First, check who's aboard
		local mins = tram.Tram:GetPos() + tram.TramBrushDelta + tram.Mins
		local maxs = tram.Tram:GetPos() + tram.TramBrushDelta + tram.Maxs
		local passengers = ents.FindInBox(mins, maxs)
		tram.Passengers = {}
		local k,v 
		for k,v in pairs(passengers) do
			if v:IsPlayer() then		-- Dont include anything other than players
				table.insert(tram.Passengers, v:ToPlayer())
				v:ToPlayer():SetNoMove(true)
			end
		end
	
		tram.State = 9
		TramPlayCinematic(tram, tram.Cinematics.Depart2, false, true)

		-- Make it depart, first close the doors
		tram.DoorsFront:Move(tram.Tram:GetPos(), 1000, TR_NONLINEAR_STOP)
		tram.DoorsBack:Move(tram.Tram:GetPos(), 1000, TR_NONLINEAR_STOP)
		-- Queue the next action
		timer.Simple(2000, TramAction, tram)
	elseif tram.State == 9 then
		-- We just closed the doors, start moving into the tunnel
		TramWarningSigns(tram, 2)
		tram.Tram:Move(tram.Positions.Tunnel2, 12000, TR_HARDEASEIN)
		tram.DoorsBack:Move(tram.Positions.Tunnel2, 12000, TR_HARDEASEIN)
		tram.DoorsFront:Move(tram.Positions.Tunnel2, 12000, TR_HARDEASEIN)
		tram.State = 10
		timer.Simple(12000, TramAction, tram)
	elseif tram.State == 10 then
		-- We reached the end of tunnel 1, tele the whole thing to midsection1 and start moving
		local trampos = tram.Tram:GetPos()
		local posdelta, k, v
		
		local oldtrampos = tram.Tram:GetPos()
		tram.Tram:SetPos(tram.Positions.MidSection2)
		tram.DoorsBack:SetPos(tram.Positions.MidSection2)
		tram.DoorsFront:SetPos(tram.Positions.MidSection2)
		
		for k,v in pairs(tram.Passengers) do
			-- Teleport our passengers with the tram
			if v:IsValid() then
				if tram.FlipPoint == 2 then
					-- Time to flip the people around
					local rotpoint =  v.Pos - (oldtrampos + tram.TramBrushDelta)
					local rotangs
					rotpoint = rotpoint * -1	-- Flip it around
					rotpoint = rotpoint + (oldtrampos + tram.TramBrushDelta)
					rotpoint.z = v.Pos.z	-- Cancel z-axis movement
					rotangs = v.Angles
					rotangs.y = rotangs.y + 180
					if rotangs.y > 360 then rotangs.y = rotangs.y - 360 end
					v.Angles = rotangs
					posdelta = rotpoint - trampos
				else
					posdelta = v.Pos - trampos
				end
				v:SetPos(tram.Positions.MidSection2 + posdelta, true)
			end
		end
		
		tram.State = 11
		timer.Simple(500, TramAction, tram)
	elseif tram.State == 11 then
		tram.Tram:Move(tram.Positions.MidSection1, 4000, TR_LINEAR_STOP)
		tram.DoorsBack:Move(tram.Positions.MidSection1, 4000, TR_LINEAR_STOP)
		tram.DoorsFront:Move(tram.Positions.MidSection1, 4000, TR_LINEAR_STOP)
		tram.State = 12
		TramPlayCinematic(tram, tram.Cinematics.Mid2to1, false, false)
		timer.Simple(4000, TramAction, tram)
	elseif tram.State == 12 then
		-- We reached the end of the midsection, teleport to tunnel 2
		local trampos = tram.Tram:GetPos()
		local posdelta, k, v
		
		local oldtrampos = tram.Tram:GetPos()
		tram.Tram:SetPos(tram.Positions.Tunnel1)
		tram.DoorsBack:SetPos(tram.Positions.Tunnel1)
		tram.DoorsFront:SetPos(tram.Positions.Tunnel1)
	
		for k,v in pairs(tram.Passengers) do
			-- Teleport our passengers with the tram
			if v:IsValid() then
				if tram.FlipPoint == 1 then
					-- Time to flip the people around
					local rotpoint =  v.Pos - (oldtrampos + tram.TramBrushDelta)
					local rotangs
					rotpoint = rotpoint * -1	-- Flip it around
					rotpoint = rotpoint + (oldtrampos + tram.TramBrushDelta)
					rotpoint.z = v.Pos.z	-- Cancel z-axis movement
					rotangs = v.Angles
					rotangs.y = rotangs.y + 180
					if rotangs.y > 360 then rotangs.y = rotangs.y - 360 end
					v.Angles = rotangs
					posdelta = rotpoint - trampos
				else
					posdelta = v.Pos - trampos
				end
				v:SetPos(tram.Positions.Tunnel1 + posdelta, true)
			end
		end
		
		tram.State = 13
		timer.Simple(500, TramAction, tram)
	elseif tram.State == 13 then
		tram.Tram:Move(tram.Positions.Station1, 12000, TR_HARDEASEOUT)
		tram.DoorsBack:Move(tram.Positions.Station1, 12000, TR_HARDEASEOUT)
		tram.DoorsFront:Move(tram.Positions.Station1, 12000, TR_HARDEASEOUT)
		tram.State = 14
		TramPlayCinematic(tram, tram.Cinematics.Arrive1, true, false)
		timer.Simple(13000, TramAction, tram)
	elseif tram.State == 14 then
		-- We just arrived in station 2, open the doors
		tram.DoorsFront:Move(tram.Positions.Station1 + tram.Positions.OffsetDoorFront, 1000, TR_NONLINEAR_STOP)
		tram.DoorsBack:Move(tram.Positions.Station1 + tram.Positions.OffsetDoorBack, 1000, TR_NONLINEAR_STOP)
		tram.State = 15
		timer.Simple(1000, TramAction, tram)
	elseif tram.State == 15 then
		tram.State = 0
		tram.InMotion = false
		tram.DisplayData.Station = 1
		tram.DisplayData.Time = tram.StationTime + sys.Time()
		-- And unfreeze the players
		for k,v in pairs(tram.Passengers) do
			v:SetNoMove(false)
		end
		timer.Simple(tram.StationTime, TramAction, tram)
		timer.Simple(tram.StationTime-3000, TramWarningSigns, tram, 1)
		
	end
end

local function InitTrams()
	
	print("Initializing trams...")
	JKG.Trams = {
		[1] = {
			Tram = ents.GetByName("Tram1")[1] or error("Could not find tram"),
			DoorsBack = ents.GetByName("Tram1Door1")[1] or error("Could not find tram back doors"),
			DoorsFront = ents.GetByName("Tram1Door2")[1] or error("Could not find tram front doors"),
			Displays = {
				S1Arrival = ents.GetByName("OceanicT1Arrive")[1] or error("Could not find Station 1 arrival display"),
				S1Min = ents.GetByName("OceanicT1Min")[1] or error("Could not find Station 1 minutes display"),
				S1SecT = ents.GetByName("OceanicT1Sec1")[1] or error("Could not find Station 1 secondsT display"),
				S1Sec = ents.GetByName("OceanicT1Sec2")[1] or error("Could not find Station 1 seconds display"),
				S1Warning = ents.GetByName("OceanicT1Warning")[1] or error("Could not find Station 1 warning signs"),
				
				S2Arrival = ents.GetByName("OceanicT2Arrive")[1] or error("Could not find Station 2 arrival display"),
				S2Min = ents.GetByName("OceanicT2Min")[1] or error("Could not find Station 2 minutes display"),
				S2SecT = ents.GetByName("OceanicT2Sec1")[1] or error("Could not find Station 2 secondsT display"),
				S2Sec = ents.GetByName("OceanicT2Sec2")[1] or error("Could not find Station 2 seconds display"),
				S2Warning = ents.GetByName("OceanicT2Warning")[1] or error("Could not find Station 2 warning signs"),
			},
			DisplayData = {
				Station = 1,	-- Station it's at or headed to (based on tram.InMotion)
				Time = -1,		-- Timestamp of arrival or departure
			},
			Positions = {
				Station1 = Vector(0,0,0),
				Tunnel1 = Vector(-7150,0,0),
				MidSection1 = Vector(-7150, -1368, 0),
				MidSection2 = Vector(-1100, -1368, 0),
				Tunnel2 = Vector(-7150, -2752, 0),
				Station2 = Vector(0, -2752, 0),
				OffsetDoorBack = Vector(60,0,0),
				OffsetDoorFront = Vector(-60,0,0),
			},
			Cinematics = {
				Depart1 = "tram1_depart1",
				Arrive1 = "tram1_arrive1",
				Mid1to2 = "tram1_mid1to2",
				Mid2to1 = "tram1_mid2to1",
				Depart2 = "tram1_depart2",
				Arrive2 = "tram1_arrive2",
			},
			TramBrushDelta = Vector(112, 860, 672),
			StationTime = 75000, --75000, -- 1 min, 15 secs
			TravelTime = 31000, -- 31 seconds
			Mins = Vector(-430,-148,-92),
			Maxs = Vector(430,148,92),
			FlipPoint = 1, -- Point of flipping (where the tram turns around): 0 = none, 1 = between tunnel 1 and midsection, 2 = between tunnel 2 and midsection
			Station = 1,
			Direction = 0,
			InMotion = false,
			State = -1,
			Passengers = nil,
		}
	}
		
	-- Reset the train to its default position
	
	local k,v 
	for k,v in pairs(JKG.Trams) do
		v.Tram:SetPos(v.Positions.Station1)
		v.DoorsBack:SetPos(v.Positions.Station1 + v.Positions.OffsetDoorBack)
		v.DoorsFront:SetPos(v.Positions.Station1 + v.Positions.OffsetDoorFront)
		TramAction(v)
	end
	TramDisplayUpdate()
end

hook.Add("MapLoaded", "InitTrams", InitTrams)


if RELOADING then
	InitTrams()
end

--[[
local function TramDepart(ply, argc, argv)
	local id = tonumber(argv[1] or "0")
	local tram = JKG.Trams[id]
	if not tram then
		ply:SendPrint("Invalid Tram ID specified")
		return
	end
	if tram.InMotion then
		ply:SendPrint("Tram is currently in motion")
		return
	end
	-- Mark the tram as in-motion so we cant trigger it again until it reaches the other station
	tram.InMotion = true

	-- First, check who's aboard
	local mins = tram.Tram:GetPos() + tram.TramBrushDelta + tram.Mins
	local maxs = tram.Tram:GetPos() + tram.TramBrushDelta + tram.Maxs
	local passengers = ents.FindInBox(mins, maxs)
	tram.Passengers = {}
	local k,v 
	for k,v in pairs(passengers) do
		if v:IsPlayer() then		-- Dont include anything other than players
			table.insert(tram.Passengers, v:ToPlayer())
			v:ToPlayer():SetFreeze(true)
		end
	end
	-- Check which station we're at
	if tram.State == 0 then		
		tram.State = 1
		TramPlayCinematic(tram, tram.Cinematics.Depart1, false, true)
	else
		tram.State = 7
		TramPlayCinematic(tram, tram.Cinematics.Depart2, false, true)
	end

	-- Make it depart, first close the doors
	tram.DoorsFront:Move(tram.Tram:GetPos(), 1000, false)
	tram.DoorsBack:Move(tram.Tram:GetPos(), 1000, false)
	-- Queue the next action
	timer.Simple(2000, TramAction, tram)
end

cmds.Add("tramdepart", TramDepart)
]]--

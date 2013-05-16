-- Exported by JKG Dialogue Creator

DLG.Name = "PzkTourneyNode"
DLG.RootNode = "E1"
DLG.Nodes = {
	E1 = {
		Type = 1,
		SubNode = "T2",
		NextNode = "E19",
		HasCondition = true,
		ConditionFunc = function(owner, ply)
			if not JKG.PzkTourney then
			    return true
			end
			
			if not JKG.PzkTourney[owner.tname] then
			    return true
			end
			
			return false 
		end,
	},
	T2 = {
		Type = 2,
		SubNode = "D3",
		Text = "This terminal is locked.",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	D3 = {
		Type = 5,
	},
	E19 = {
		Type = 1,
		SubNode = "L20",
		NextNode = "E4",
		HasCondition = true,
		ConditionFunc = function(owner, ply)
			return JKG.PzkTourney[owner.tname].Mode == 0
		end,
	},
	L20 = {
		Type = 4,
		Target = "T2",
	},
	E4 = {
		Type = 1,
		SubNode = "T5",
		NextNode = "E24",
		HasCondition = true,
		ConditionFunc = function(owner, ply)
			return JKG.PzkTourney[owner.tname].Mode == 1
		end,
	},
	T5 = {
		Type = 2,
		SubNode = "T77",
		Text = "This pazaak tournament terminal is currently in free-play mode.",
		Duration = 4000,
		HasCondition = false,
		HasResolver = false,
	},
	T77 = {
		Type = 2,
		SubNode = "T78",
		NextNode = "T6",
		Text = "This terminal supports versus matches against other players.",
		Duration = 4000,
		HasCondition = true,
		ConditionFunc = function(owner, ply)
			return owner.mpenabled
		end,
		HasResolver = false,
	},
	T78 = {
		Type = 2,
		SubNode = "O74",
		Text = "What do you wish to do?",
		Duration = 1500,
		HasCondition = false,
		HasResolver = false,
	},
	O74 = {
		Type = 3,
		SubNode = "L79",
		NextNode = "O75",
		Text = "Play against AI",
		HasCondition = false,
		HasResolver = false,
	},
	L79 = {
		Type = 4,
		Target = "W10",
	},
	O75 = {
		Type = 3,
		SubNode = "T89",
		NextNode = "O76",
		Text = "Play against another player",
		HasCondition = false,
		HasResolver = false,
	},
	T89 = {
		Type = 2,
		SubNode = "L90",
		NextNode = "S91",
		Text = "^1Linking fault!\nAn error has occoured, cannot start multiplayer match.\n",
		Duration = 5000,
		HasCondition = true,
		ConditionFunc = function(owner, ply)
			return owner.mplink.mplink ~= owner
		end,
		HasResolver = false,
	},
	L90 = {
		Type = 4,
		Target = "T78",
	},
	S91 = {
		Type = 6,
		SubNode = "T83",
		ScriptFunc = function(owner, ply)
			owner.pendingplayer = nil
			owner.ready = false
			owner.tries = 0
			owner.resumefunc = nil
			
			owner.wonmatch = false
			owner.pazaakerror = false
				
		end,
		HasCondition = false,
	},
	T83 = {
		Type = 2,
		SubNode = "S84",
		Text = "Waiting for opponent...",
		Duration = 500,
		HasCondition = false,
		HasResolver = false,
	},
	S84 = {
		Type = 6,
		SubNode = "W85",
		ScriptFunc = function(owner, ply)
			if owner.tries == 0 then
			    owner.pendingplayer = ply
			end
			
			if owner.mplink.pendingplayer then
			    -- We got one
			    owner.ready = true
			    owner.tries = 0
			    return
			end
			
			if owner.tries == 10 then
			    owner.tries = 0
			    owner.ready = false
			    owner.pendingplayer = nil
			    return
			end
			
			owner.tries = owner.tries + 1
		end,
		HasCondition = false,
	},
	W85 = {
		Type = 7,
		SubNode = "T92",
		NextNode = "T86",
		ScriptFunc = function(owner, ply, resumefunc)
			print("Found opponent!")
			owner.resumefunc = resumefunc
			
			-- If the opponent is not ready yet, we wait
			if not owner.mplink.resumefunc then
			    return true
			end
			
			-- If the opponent is ready, we start the match
			local function pzkFinish(winner)
			    if winner == owner.pendingplayer then
			        owner.wonmatch = true
			    else
			        owner.wonmatch = false
			    end
			    owner.mplink.resumefunc()
			    owner.resumefunc()
			end
			
			local pzk = sys.CreatePazaakGame()
			local cards = JKG.Pazaak.Cards
			if aigame then
			    pzk:SetPlayers(cpl.Player1, nil)
			else
			    pzk:SetPlayers(cpl.Player1, cpl.Player2)
			end
			
			pzk:SetFinishCallback(pzkFinish)
			
			pzk:SetCards(1, {10,10,10,10,10,10,
			             10,10,10,10,10,10,
			             10,10,10,10,10,10,
			             10,10,10,10,10 } )
			
			pzk:SetCards(2, {10,10,10,10,10,10,
			             10,10,10,10,10,10,
			             10,10,10,10,10,10,
			             10,10,10,10,10 } )
			            
			pzk:ShowCardSelection(true)
			
			
			local success, reason = pzk:StartGame()
			    if not success then
			    print("ERROR STARTING PAZAAK: ", reason)
			
			    owner.mplink.pazaakerror = true
			    owner.pazaakerror = true
			
			    owner.mplink.resumefunc()
			    owner.resumefunc()
			end
			
			return true
		end,
		HasCondition = true,
		ConditionFunc = function(owner, ply)
			return owner.ready
		end,
	},
	T92 = {
		Type = 2,
		SubNode = "S94",
		NextNode = "T93",
		Text = "An error has occoured starting the pazaak match, please try again.",
		Duration = 5000,
		HasCondition = true,
		ConditionFunc = function(owner, ply)
			return owner.pazaakerror
		end,
		HasResolver = false,
	},
	S94 = {
		Type = 6,
		SubNode = "L95",
		ScriptFunc = function(owner, ply)
			owner.pendingplayer = nil
			owner.ready = false
			owner.tries = 0
			owner.resumefunc = nil
			
			owner.wonmatch = false
			owner.pazaakerror = false
		end,
		HasCondition = false,
	},
	L95 = {
		Type = 4,
		Target = "T78",
	},
	T93 = {
		Type = 2,
		SubNode = "L96",
		NextNode = "T97",
		Text = "You have won the match, congratulations!",
		Duration = 3000,
		HasCondition = true,
		ConditionFunc = function(owner, ply)
			return owner.wonmatch
		end,
		HasResolver = false,
	},
	L96 = {
		Type = 4,
		Target = "S94",
	},
	T97 = {
		Type = 2,
		SubNode = "L98",
		Text = "You have lost the match, better luck next time!",
		Duration = 4000,
		HasCondition = false,
		HasResolver = false,
	},
	L98 = {
		Type = 4,
		Target = "S94",
	},
	T86 = {
		Type = 2,
		SubNode = "L87",
		NextNode = "L88",
		Text = "No opponent could be found, please try again.",
		Duration = 4000,
		HasCondition = true,
		ConditionFunc = function(owner, ply)
			-- Tries can only be 0 if we stop trying
			-- This can either be due to an opponent being found
			-- or due to the max amount of tries being exceeded
			-- Since finding an opponent sets owner.ready to true
			-- and that condition is checked before this one
			-- we know that, if tries is 0, we exceeded our amount of retries
			
			return owner.tries == 0
		end,
		HasResolver = false,
	},
	L87 = {
		Type = 4,
		Target = "T78",
	},
	L88 = {
		Type = 4,
		Target = "T83",
	},
	O76 = {
		Type = 3,
		SubNode = "L80",
		Text = "Leave",
		HasCondition = false,
		HasResolver = false,
	},
	L80 = {
		Type = 4,
		Target = "T18",
	},
	T6 = {
		Type = 2,
		SubNode = "O7",
		Text = "Play a game of pazaak?",
		Duration = 2000,
		HasCondition = false,
		HasResolver = false,
	},
	O7 = {
		Type = 3,
		SubNode = "W10",
		NextNode = "O8",
		Text = "Yes",
		HasCondition = false,
		HasResolver = false,
	},
	W10 = {
		Type = 7,
		SubNode = "T11",
		ScriptFunc = function(owner, ply, resumefunc)
			local function pzkFinish(winner)
			    if not winner then
			        -- The AI won
			        owner.pzkaiwin = true
			    else
			        owner.pzkaiwin = false
			    end
			    resumefunc()
			end
			
			local pzk = sys.CreatePazaakGame()
			local cards = JKG.Pazaak.Cards
			pzk:SetPlayers(ply, nil)
			
			if not opp then
			    pzk:SetAILevel(1)
			    pzk:SetAIName("Practice Bot")
			end
			
			pzk:SetFinishCallback(pzkFinish)
			
			pzk:SetCards(1, {10,10,10,10,10,10,
			                 10,10,10,10,10,10,
			                 10,10,10,10,10,10,
			                 10,10,10,10,10 } )
			
			pzk:SetCards(2, {10,10,10,10,10,10,
			                 10,10,10,10,10,10,
			                 10,10,10,10,10,10,
			                 10,10,10,10,10 } )
			
			local function CreateRandomDeck()
			    local result = {}
			    local i
			    local newcard
			    for i=1, 10 do
			        newcard = math.floor(math.random(cards.PZCARD_PLUS_1, cards.PZCARD_FLIP_6))
			        table.insert(result, newcard)
			    end
			    return result
			end
			
			pzk:SetSideDeck(2, CreateRandomDeck() )
			                
			pzk:ShowCardSelection(true)
			
			local success, reason = pzk:StartGame()
			if not success then
			    resumefunc()
			end
			
			-- We want to retain the mouse, so return true
			return true 
		end,
		HasCondition = false,
	},
	T11 = {
		Type = 2,
		SubNode = "T21",
		NextNode = "T12",
		Text = "You have lost the match.",
		Duration = 2500,
		HasCondition = true,
		ConditionFunc = function(owner, ply)
			return owner.pzkaiwin
		end,
		HasResolver = false,
	},
	T21 = {
		Type = 2,
		SubNode = "L16",
		NextNode = "L22",
		Text = "Would you like to try again?",
		Duration = 2000,
		HasCondition = true,
		ConditionFunc = function(owner, ply)
			return JKG.PzkTourney[owner.tname].Mode == 1
		end,
		HasResolver = false,
	},
	L16 = {
		Type = 4,
		NextNode = "L17",
		Target = "O7",
	},
	L17 = {
		Type = 4,
		Target = "O8",
	},
	L22 = {
		Type = 4,
		Target = "T18",
	},
	T12 = {
		Type = 2,
		SubNode = "T13",
		Text = "Congratulations, you won the match.",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	T13 = {
		Type = 2,
		SubNode = "L14",
		NextNode = "L23",
		Text = "Would you like to play again?",
		Duration = 2000,
		HasCondition = true,
		ConditionFunc = function(owner, ply)
			return JKG.PzkTourney[owner.tname].Mode == 1
		end,
		HasResolver = false,
	},
	L14 = {
		Type = 4,
		NextNode = "L15",
		Target = "O7",
	},
	L15 = {
		Type = 4,
		Target = "O8",
	},
	L23 = {
		Type = 4,
		Target = "T18",
	},
	O8 = {
		Type = 3,
		SubNode = "T18",
		Text = "No",
		HasCondition = false,
		HasResolver = false,
	},
	T18 = {
		Type = 2,
		SubNode = "D9",
		Text = "Come back anytime.",
		Duration = 2000,
		HasCondition = false,
		HasResolver = false,
	},
	D9 = {
		Type = 5,
	},
	E24 = {
		Type = 1,
		SubNode = "T25",
		NextNode = "E43",
		HasCondition = true,
		ConditionFunc = function(owner, ply)
			return JKG.PzkTourney[owner.tname].Mode == 2
		end,
	},
	T25 = {
		Type = 2,
		SubNode = "T34",
		Text = "This terminal is currently registering players for the tournament.",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	T34 = {
		Type = 2,
		SubNode = "O35",
		NextNode = "T41",
		Text = "You are already registered in this tournament.\nDo you want to unregister yourself?",
		Duration = 5000,
		HasCondition = true,
		ConditionFunc = function(owner, ply)
			local data = JKG.PzkTourney[owner.tname]
			
			local k,v
			for k,v in pairs(data.TourneyPlayers) do
			    if v == ply then
			        return true
			    end
			end
			
			return false
		end,
		HasResolver = false,
	},
	O35 = {
		Type = 3,
		SubNode = "S37",
		NextNode = "O36",
		Text = "Yes, unregister me",
		HasCondition = false,
		HasResolver = false,
	},
	S37 = {
		Type = 6,
		SubNode = "T38",
		ScriptFunc = function(owner, ply)
			local data = JKG.PzkTourney[owner.tname]
			
			local k,v
			for k,v in pairs(data.TourneyPlayers) do
			    if v == ply then
			        data.TourneyPlayers[k] = nil
			        data.TourneyPlayerCount = data.TourneyPlayerCount - 1
			    end
			end
			
			data:SendNotify(ply:GetName() .. " has unregistered from the tournament")
		end,
		HasCondition = false,
	},
	T38 = {
		Type = 2,
		SubNode = "D39",
		Text = "You are no longer registered for this tournament",
		Duration = 4000,
		HasCondition = false,
		HasResolver = false,
	},
	D39 = {
		Type = 5,
	},
	O36 = {
		Type = 3,
		SubNode = "D40",
		Text = "No, I want to stay registered",
		HasCondition = false,
		HasResolver = false,
	},
	D40 = {
		Type = 5,
	},
	T41 = {
		Type = 2,
		SubNode = "D42",
		NextNode = "T27",
		Text = "There is no more room for further registration",
		Duration = 2000,
		HasCondition = true,
		ConditionFunc = function(owner, ply)
			local data = JKG.PzkTourney[owner.tname]
			
			if data.TourneyPlayerCount >= data.MaxPlayers then
			    return true
			end
			
			return false
		end,
		HasResolver = false,
	},
	D42 = {
		Type = 5,
	},
	T27 = {
		Type = 2,
		SubNode = "O28",
		Text = "Do you want to register?",
		Duration = 1500,
		HasCondition = false,
		HasResolver = false,
	},
	O28 = {
		Type = 3,
		SubNode = "S31",
		NextNode = "O29",
		Text = "Yes, register me for this tournament",
		HasCondition = false,
		HasResolver = false,
	},
	S31 = {
		Type = 6,
		SubNode = "T32",
		ScriptFunc = function(owner, ply)
			local data = JKG.PzkTourney[owner.tname]
			
			table.insert(data.TourneyPlayers, ply)
			data.TourneyPlayerCount = data.TourneyPlayerCount + 1
			data:SendNotify(ply:GetName() .. " has registered for the tournament")
		end,
		HasCondition = false,
	},
	T32 = {
		Type = 2,
		SubNode = "D33",
		Text = "You are now registered for this tournament!",
		Duration = 2000,
		HasCondition = false,
		HasResolver = false,
	},
	D33 = {
		Type = 5,
	},
	O29 = {
		Type = 3,
		SubNode = "D30",
		Text = "No thanks",
		HasCondition = false,
		HasResolver = false,
	},
	D30 = {
		Type = 5,
	},
	E43 = {
		Type = 1,
		SubNode = "T44",
		NextNode = "E46",
		HasCondition = true,
		ConditionFunc = function(owner, ply)
			return JKG.PzkTourney[owner.tname].Mode == 3
		end,
	},
	T44 = {
		Type = 2,
		SubNode = "D45",
		Text = "Sorry, registration for the tournament is currently closed",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	D45 = {
		Type = 5,
	},
	E46 = {
		Type = 1,
		SubNode = "T50",
		NextNode = "E47",
		HasCondition = true,
		ConditionFunc = function(owner, ply)
			return JKG.PzkTourney[owner.tname].Mode == 4
		end,
	},
	T50 = {
		Type = 2,
		SubNode = "D51",
		NextNode = "T52",
		Text = "You are not (or no longer) participating in this tournament.",
		Duration = 3000,
		HasCondition = true,
		ConditionFunc = function(owner, ply)
			local data = JKG.PzkTourney[owner.tname]
			
			local k,v
			for k,v in pairs(data.TourneyPlayers) do
			    if v == ply then
			        return false
			    end
			end
			
			return true
		end,
		HasResolver = false,
	},
	D51 = {
		Type = 5,
	},
	T52 = {
		Type = 2,
		SubNode = "O53",
		Text = "Ready to play?",
		Duration = 1000,
		HasCondition = false,
		HasResolver = false,
	},
	O53 = {
		Type = 3,
		SubNode = "T56",
		NextNode = "O54",
		Text = "Let's go!",
		HasCondition = false,
		HasResolver = false,
	},
	T56 = {
		Type = 2,
		SubNode = "S57",
		Text = "Searching for opponent... Please wait",
		Duration = 500,
		HasCondition = false,
		HasResolver = false,
	},
	S57 = {
		Type = 6,
		SubNode = "W63",
		NextNode = "T58",
		ScriptFunc = function(owner, ply)
			local tries = ply.tmpPzkTry or 0
			local data = JKG.PzkTourney[owner.tname]
			
			print("DEBUG: In S57")
			local opp = data:GetOpponent(ply)
			if not opp then
			    tries = tries + 1
			    ply.tmpPzkTry = tries
			else
			    ply.tmpPzkTry = -1
			end
		end,
		HasCondition = true,
		ConditionFunc = function(owner, ply)
			local tries = ply.tmpPzkTry or 0
			
			if tries > 20 then
			    return false
			else
			    return true
			end
		end,
	},
	W63 = {
		Type = 7,
		SubNode = "T65",
		NextNode = "L64",
		ScriptFunc = function(owner, ply, resumefunc)
			local data = JKG.PzkTourney[owner.tname]
			ply.tmpPzkTry = nil
			
			
			print("DEBUG: In W63")
			
			data:ReadyToPlay(ply, resumefunc)
			
			return true
		end,
		HasCondition = true,
		ConditionFunc = function(owner, ply)
			local tries = ply.tmpPzkTry or 0
			if tries == -1 then
			    -- We're set
			    return true
			else
			    return false
			end
		end,
	},
	T65 = {
		Type = 2,
		SubNode = "D66",
		NextNode = "T67",
		Text = "You lost the match.",
		Duration = 1000,
		HasCondition = true,
		ConditionFunc = function(owner, ply)
			local data = JKG.PzkTourney[owner.tname]
			
			local k,v
			for k,v in pairs(data.TourneyPlayers) do
			    if v == ply then
			        return false
			    end
			end
			
			return true
		end,
		HasResolver = false,
	},
	D66 = {
		Type = 5,
	},
	T67 = {
		Type = 2,
		SubNode = "T68",
		NextNode = "T70",
		Text = "You have won the match, congratulations!",
		Duration = 3000,
		HasCondition = true,
		ConditionFunc = function(owner, ply)
			local data = JKG.PzkTourney[owner.tname]
			
			return data.TourneyPlayerCount > 1
		end,
		HasResolver = false,
	},
	T68 = {
		Type = 2,
		SubNode = "D69",
		Text = "Wait for the remaining players to finish to proceed to the next round.",
		Duration = 4000,
		HasCondition = false,
		HasResolver = false,
	},
	D69 = {
		Type = 5,
	},
	T70 = {
		Type = 2,
		SubNode = "D71",
		Text = "You have won the tournament! Congratulations!",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	D71 = {
		Type = 5,
	},
	L64 = {
		Type = 4,
		Target = "T56",
	},
	T58 = {
		Type = 2,
		SubNode = "O59",
		Text = "The search for an opponent is taking too long...\nTry again?",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	O59 = {
		Type = 3,
		SubNode = "L60",
		NextNode = "O61",
		Text = "Yes",
		HasCondition = false,
		HasResolver = false,
	},
	L60 = {
		Type = 4,
		Target = "T56",
	},
	O61 = {
		Type = 3,
		SubNode = "D62",
		Text = "No",
		HasCondition = false,
		HasResolver = false,
	},
	D62 = {
		Type = 5,
	},
	O54 = {
		Type = 3,
		SubNode = "D55",
		Text = "Not yet...",
		HasCondition = false,
		HasResolver = false,
	},
	D55 = {
		Type = 5,
	},
	E47 = {
		Type = 1,
		SubNode = "T48",
		HasCondition = true,
		ConditionFunc = function(owner, ply)
			return JKG.PzkTourney[owner.tname].Mode == 5
		end,
	},
	T48 = {
		Type = 2,
		SubNode = "D49",
		Text = "The tournament has finished.\nThe winner of the tournament is $winner$.",
		Duration = 5000,
		HasCondition = false,
		HasResolver = false,
	},
	D49 = {
		Type = 5,
	},
}

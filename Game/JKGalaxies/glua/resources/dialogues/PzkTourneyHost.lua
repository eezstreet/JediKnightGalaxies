-- Exported by JKG Dialogue Creator

DLG.Name = "PzkTourneyHost"
DLG.RootNode = "E1"
DLG.Nodes = {
	E1 = {
		Type = 1,
		SubNode = "T2",
		NextNode = "E4",
		HasCondition = true,
		ConditionFunc = function(owner, ply)
			return (not ply.IsAdmin)
		end,
	},
	T2 = {
		Type = 2,
		SubNode = "D3",
		Text = "This is a restricted terminal. Access Denied.",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	D3 = {
		Type = 5,
	},
	E4 = {
		Type = 1,
		SubNode = "T19",
		HasCondition = false,
	},
	T19 = {
		Type = 2,
		SubNode = "T20",
		NextNode = "T5",
		Text = "The pazaak tournament system has not been initialized",
		Duration = 5000,
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
		HasResolver = false,
	},
	T20 = {
		Type = 2,
		SubNode = "O21",
		Text = "Do you wish to initialize it now?",
		Duration = 2000,
		HasCondition = false,
		HasResolver = false,
	},
	O21 = {
		Type = 3,
		SubNode = "S24",
		NextNode = "O22",
		Text = "Initialize it now",
		HasCondition = false,
		HasResolver = false,
	},
	S24 = {
		Type = 6,
		SubNode = "T25",
		ScriptFunc = function(owner, ply)
			if not JKG.PzkTourney then
			    JKG.PzkTourney = {}
			end
			
			if not JKG.PzkTourney[owner.tname] then
			    JKG.PzkTourney[owner.tname] = {}
			end
			
			local data = JKG.PzkTourney[owner.tname]
			
			-- Modes:
			-- 0 = Standby
			-- 1 = Free-play
			-- 2 = Registering
			-- 3 = Closed registration
			-- 4 = In Tournament
			-- 5 = Tournament finished
			
			data.Mode = 0
			
			data.TourneyPlayers = {}
			data.TourneyPlayerCount = 0
			data.TourneyRound = 0
			
			data.NotifyPlayers = {}
			
			function data:SendNotify(msg)
			    local k,v
			    for k,v in pairs(self.NotifyPlayers) do
			        v:SendChat("^5Pazaak Tourney^7: ^2 " .. msg)
			    end
			end
		end,
		HasCondition = false,
	},
	T25 = {
		Type = 2,
		SubNode = "L26",
		Text = "Successfully initialized, accessing control panel",
		Duration = 4000,
		HasCondition = false,
		HasResolver = false,
	},
	L26 = {
		Type = 4,
		Target = "T5",
	},
	O22 = {
		Type = 3,
		SubNode = "D23",
		Text = "Leave it as it is",
		HasCondition = false,
		HasResolver = false,
	},
	D23 = {
		Type = 5,
	},
	T5 = {
		Type = 2,
		SubNode = "O27",
		Text = "- Pazaak Tournament Control -\nPlease choose your action:",
		Duration = 5000,
		HasCondition = false,
		HasResolver = false,
	},
	O27 = {
		Type = 3,
		SubNode = "T28",
		NextNode = "O7",
		Text = "Access terminal controls",
		HasCondition = false,
		HasResolver = false,
	},
	T28 = {
		Type = 2,
		SubNode = "O29",
		Text = "The terminals are currently $state$\nChoose your action:",
		Duration = 5000,
		HasCondition = false,
		HasResolver = true,
		ResolveFunc = function(owner, ply, var)
			if var == "state" then
			    local data = JKG.PzkTourney[owner.tname]
			
			    if data.Mode == 0 then
			        return "on standby"
			    elseif data.Mode == 1 then
			        return "on free-play mode"
			    elseif data.Mode == 2 then
			        return "enlisting players"
			    elseif data.Mode == 3 then
			        return "running a tournament"
			    end
			end
			
			return ""
		end,
	},
	O29 = {
		Type = 3,
		SubNode = "S35",
		NextNode = "O30",
		Text = "Switch terminals to standby",
		HasCondition = true,
		ConditionFunc = function(owner, ply)
			return JKG.PzkTourney[owner.tname].Mode == 1
		end,
		HasResolver = false,
	},
	S35 = {
		Type = 6,
		SubNode = "L36",
		ScriptFunc = function(owner, ply)
			JKG.PzkTourney[owner.tname].Mode = 0
		end,
		HasCondition = false,
	},
	L36 = {
		Type = 4,
		Target = "T28",
	},
	O30 = {
		Type = 3,
		SubNode = "S37",
		NextNode = "L31",
		Text = "Switch terminals to free-play mode",
		HasCondition = true,
		ConditionFunc = function(owner, ply)
			return JKG.PzkTourney[owner.tname].Mode == 0
		end,
		HasResolver = false,
	},
	S37 = {
		Type = 6,
		SubNode = "L38",
		ScriptFunc = function(owner, ply)
			JKG.PzkTourney[owner.tname].Mode = 1
		end,
		HasCondition = false,
	},
	L38 = {
		Type = 4,
		Target = "T28",
	},
	L31 = {
		Type = 4,
		NextNode = "L69",
		Target = "O7",
	},
	L69 = {
		Type = 4,
		NextNode = "L32",
		Target = "O42",
	},
	L32 = {
		Type = 4,
		Target = "O8",
	},
	O7 = {
		Type = 3,
		SubNode = "T77",
		NextNode = "O42",
		Text = "Access tournament controls",
		HasCondition = false,
		HasResolver = false,
	},
	T77 = {
		Type = 2,
		SubNode = "O78",
		NextNode = "T117",
		Text = "There is no tournament going right now.",
		Duration = 2000,
		HasCondition = true,
		ConditionFunc = function(owner, ply)
			return JKG.PzkTourney[owner.tname].Mode < 2
		end,
		HasResolver = false,
	},
	O78 = {
		Type = 3,
		SubNode = "T79",
		NextNode = "L114",
		Text = "Start a new tournament",
		HasCondition = false,
		HasResolver = false,
	},
	T79 = {
		Type = 2,
		SubNode = "T80",
		Text = "Starting a new tournament will switch the terminals to registration mode.",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	T80 = {
		Type = 2,
		SubNode = "T81",
		Text = "Once everyone has registered, please close registration to begin the actual tournament.",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	T81 = {
		Type = 2,
		SubNode = "O82",
		Text = "Continue?",
		Duration = 1000,
		HasCondition = false,
		HasResolver = false,
	},
	O82 = {
		Type = 3,
		SubNode = "T86",
		NextNode = "O83",
		Text = "Yes",
		HasCondition = false,
		HasResolver = false,
	},
	T86 = {
		Type = 2,
		SubNode = "O87",
		Text = "How many players can participate?\n(This should be equal to the amount of present nodes)",
		Duration = 5000,
		HasCondition = false,
		HasResolver = false,
	},
	O87 = {
		Type = 3,
		SubNode = "S96",
		NextNode = "O88",
		Text = "2",
		HasCondition = false,
		HasResolver = false,
	},
	S96 = {
		Type = 6,
		SubNode = "T105",
		ScriptFunc = function(owner, ply)
			local data = JKG.PzkTourney[owner.tname]
			
			data.Mode = 2
			
			data.TourneyPlayers = {}
			data.TourneyPlayerCount = 0
			data.MaxPlayers = 2
			data.TourneyRound = 0
		end,
		HasCondition = false,
	},
	T105 = {
		Type = 2,
		SubNode = "L126",
		Text = "Tournament started (max players: $max$)",
		Duration = 3000,
		HasCondition = false,
		HasResolver = true,
		ResolveFunc = function(owner, ply, var)
			if var == "max" then
			    return JKG.PzkTourney[owner.tname].MaxPlayers
			end
			
			return ""
		end,
	},
	L126 = {
		Type = 4,
		Target = "T117",
	},
	O88 = {
		Type = 3,
		SubNode = "S97",
		NextNode = "O89",
		Text = "4",
		HasCondition = false,
		HasResolver = false,
	},
	S97 = {
		Type = 6,
		SubNode = "L106",
		ScriptFunc = function(owner, ply)
			local data = JKG.PzkTourney[owner.tname]
			
			data.Mode = 2
			
			data.TourneyPlayers = {}
			data.TourneyPlayerCount = 0
			data.MaxPlayers = 4
			data.TourneyRound = 0
		end,
		HasCondition = false,
	},
	L106 = {
		Type = 4,
		Target = "T105",
	},
	O89 = {
		Type = 3,
		SubNode = "S98",
		NextNode = "O90",
		Text = "8",
		HasCondition = false,
		HasResolver = false,
	},
	S98 = {
		Type = 6,
		SubNode = "L107",
		ScriptFunc = function(owner, ply)
			local data = JKG.PzkTourney[owner.tname]
			
			data.Mode = 2
			
			data.TourneyPlayers = {}
			data.TourneyPlayerCount = 0
			data.MaxPlayers = 8
			data.TourneyRound = 0
		end,
		HasCondition = false,
	},
	L107 = {
		Type = 4,
		Target = "T105",
	},
	O90 = {
		Type = 3,
		SubNode = "S99",
		NextNode = "O91",
		Text = "12",
		HasCondition = false,
		HasResolver = false,
	},
	S99 = {
		Type = 6,
		SubNode = "L108",
		ScriptFunc = function(owner, ply)
			local data = JKG.PzkTourney[owner.tname]
			
			data.Mode = 2
			
			data.TourneyPlayers = {}
			data.TourneyPlayerCount = 0
			data.MaxPlayers = 12
			data.TourneyRound = 0
		end,
		HasCondition = false,
	},
	L108 = {
		Type = 4,
		Target = "T105",
	},
	O91 = {
		Type = 3,
		SubNode = "S100",
		NextNode = "O92",
		Text = "16",
		HasCondition = false,
		HasResolver = false,
	},
	S100 = {
		Type = 6,
		SubNode = "L109",
		ScriptFunc = function(owner, ply)
			local data = JKG.PzkTourney[owner.tname]
			
			data.Mode = 2
			
			data.TourneyPlayers = {}
			data.TourneyPlayerCount = 0
			data.MaxPlayers = 16
			data.TourneyRound = 0
		end,
		HasCondition = false,
	},
	L109 = {
		Type = 4,
		Target = "T105",
	},
	O92 = {
		Type = 3,
		SubNode = "S101",
		NextNode = "O93",
		Text = "24",
		HasCondition = false,
		HasResolver = false,
	},
	S101 = {
		Type = 6,
		SubNode = "L110",
		ScriptFunc = function(owner, ply)
			local data = JKG.PzkTourney[owner.tname]
			
			data.Mode = 2
			
			data.TourneyPlayers = {}
			data.TourneyPlayerCount = 0
			data.MaxPlayers = 24
			data.TourneyRound = 0
		end,
		HasCondition = false,
	},
	L110 = {
		Type = 4,
		Target = "T105",
	},
	O93 = {
		Type = 3,
		SubNode = "S102",
		NextNode = "O94",
		Text = "32",
		HasCondition = false,
		HasResolver = false,
	},
	S102 = {
		Type = 6,
		SubNode = "L111",
		ScriptFunc = function(owner, ply)
			local data = JKG.PzkTourney[owner.tname]
			
			data.Mode = 2
			
			data.TourneyPlayers = {}
			data.TourneyPlayerCount = 0
			data.MaxPlayers = 32
			data.TourneyRound = 0
		end,
		HasCondition = false,
	},
	L111 = {
		Type = 4,
		Target = "T105",
	},
	O94 = {
		Type = 3,
		SubNode = "S103",
		NextNode = "O95",
		Text = "48",
		HasCondition = false,
		HasResolver = false,
	},
	S103 = {
		Type = 6,
		SubNode = "L112",
		ScriptFunc = function(owner, ply)
			local data = JKG.PzkTourney[owner.tname]
			
			data.Mode = 2
			
			data.TourneyPlayers = {}
			data.TourneyPlayerCount = 0
			data.MaxPlayers = 48
			data.TourneyRound = 0
		end,
		HasCondition = false,
	},
	L112 = {
		Type = 4,
		Target = "T105",
	},
	O95 = {
		Type = 3,
		SubNode = "S104",
		Text = "64",
		HasCondition = false,
		HasResolver = false,
	},
	S104 = {
		Type = 6,
		SubNode = "L113",
		ScriptFunc = function(owner, ply)
			local data = JKG.PzkTourney[owner.tname]
			
			data.Mode = 2
			
			data.TourneyPlayers = {}
			data.TourneyPlayerCount = 0
			data.MaxPlayers = 64
			data.TourneyRound = 0
		end,
		HasCondition = false,
	},
	L113 = {
		Type = 4,
		Target = "T105",
	},
	O83 = {
		Type = 3,
		SubNode = "L84",
		Text = "No",
		HasCondition = false,
		HasResolver = false,
	},
	L84 = {
		Type = 4,
		Target = "T77",
	},
	L114 = {
		Type = 4,
		NextNode = "L115",
		Target = "O27",
	},
	L115 = {
		Type = 4,
		NextNode = "L116",
		Target = "O42",
	},
	L116 = {
		Type = 4,
		Target = "O8",
	},
	T117 = {
		Type = 2,
		SubNode = "O118",
		NextNode = "T123",
		Text = "The tournament is currently open for registration.\nSo far $pl$ players have registered. (Max: $max$)",
		Duration = 3000,
		HasCondition = true,
		ConditionFunc = function(owner, ply)
			return JKG.PzkTourney[owner.tname].Mode == 2
		end,
		HasResolver = true,
		ResolveFunc = function(owner, ply, var)
			local data = JKG.PzkTourney[owner.tname]
			if var == "pl" then
			    return data.TourneyPlayerCount
			elseif var == "max" then
			    return data.MaxPlayers
			end
			
			return ""
		end,
	},
	O118 = {
		Type = 3,
		SubNode = "S132",
		NextNode = "O119",
		Text = "Close registration",
		HasCondition = false,
		HasResolver = false,
	},
	S132 = {
		Type = 6,
		SubNode = "T133",
		ScriptFunc = function(owner, ply)
			JKG.PzkTourney[owner.tname].Mode = 3
		end,
		HasCondition = false,
	},
	T133 = {
		Type = 2,
		SubNode = "L134",
		Text = "Registration has been closed",
		Duration = 2000,
		HasCondition = false,
		HasResolver = false,
	},
	L134 = {
		Type = 4,
		Target = "T123",
	},
	O119 = {
		Type = 3,
		SubNode = "T135",
		NextNode = "L120",
		Text = "Stop Tournament",
		HasCondition = false,
		HasResolver = false,
	},
	T135 = {
		Type = 2,
		SubNode = "O136",
		Text = "Are you sure you wish to stop the tournament?",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	O136 = {
		Type = 3,
		SubNode = "S139",
		NextNode = "O137",
		Text = "Yes",
		HasCondition = false,
		HasResolver = false,
	},
	S139 = {
		Type = 6,
		SubNode = "T140",
		ScriptFunc = function(owner, ply)
			local data = JKG.PzkTourney[owner.tname]
			
			data.Mode = 0
			
			data.TourneyPlayers = {}
			data.TourneyPlayerCount = 0
			data.TourneyRound = 0
		end,
		HasCondition = false,
	},
	T140 = {
		Type = 2,
		SubNode = "L141",
		Text = "The tournament has been stopped",
		Duration = 1000,
		HasCondition = false,
		HasResolver = false,
	},
	L141 = {
		Type = 4,
		Target = "T77",
	},
	O137 = {
		Type = 3,
		SubNode = "L138",
		Text = "No",
		HasCondition = false,
		HasResolver = false,
	},
	L138 = {
		Type = 4,
		Target = "T117",
	},
	L120 = {
		Type = 4,
		NextNode = "L121",
		Target = "O27",
	},
	L121 = {
		Type = 4,
		NextNode = "L122",
		Target = "O42",
	},
	L122 = {
		Type = 4,
		Target = "O8",
	},
	T123 = {
		Type = 2,
		SubNode = "O127",
		NextNode = "T124",
		Text = "The tournament is ready to start.\n$pl$/$max$ players are participating.",
		Duration = 4000,
		HasCondition = true,
		ConditionFunc = function(owner, ply)
			return JKG.PzkTourney[owner.tname].Mode == 3
		end,
		HasResolver = true,
		ResolveFunc = function(owner, ply, var)
			local data = JKG.PzkTourney[owner.tname]
			
			if var == "pl" then
			    return data.TourneyPlayerCount
			elseif var == "max" then
			    return data.MaxPlayers
			end
			
			return ""
		end,
	},
	O127 = {
		Type = 3,
		SubNode = "S142",
		NextNode = "O128",
		Text = "Open registration",
		HasCondition = false,
		HasResolver = false,
	},
	S142 = {
		Type = 6,
		SubNode = "T143",
		ScriptFunc = function(owner, ply)
			JKG.PzkTourney[owner.tname].Mode = 2
		end,
		HasCondition = false,
	},
	T143 = {
		Type = 2,
		SubNode = "L144",
		Text = "Registration has been re-opened",
		Duration = 1000,
		HasCondition = false,
		HasResolver = false,
	},
	L144 = {
		Type = 4,
		Target = "T117",
	},
	O128 = {
		Type = 3,
		SubNode = "S154",
		NextNode = "L129",
		Text = "Start the tournament",
		HasCondition = false,
		HasResolver = false,
	},
	S154 = {
		Type = 6,
		SubNode = "T155",
		ScriptFunc = function(owner, ply)
			local data = JKG.PzkTourney[owner.tname]
			
			data.Mode = 4
			data.TourneyRound = 1
			-- Player who've finished the round, gets reset when everyone's done
			
			-- Couples made in this tourney
			-- Couple data:
			-- cpl.Player1
			-- cpl.Player2 (can be "AI")
			-- cpl.ReturnFunc1
			-- cpl.ReturnFunc2
			-- cpl.ReadyCount
			
			data.TourneyCouples = {}
			-- Player awaiting someone to play against
			data.TourneyPlayerPending = nil
			
			-- People who have finished the round
			data.TourneyRoundFinished = {}
			-- Players still remaining
			data.TourneyPlayersWaiting = data.TourneyPlayerCount
			-- Players who are playing pazaak
			data.TourneyPlayersIngame = 0
			
			
			function data:InCouple(ply)
			    local k,v
			    for k,v in pairs(self.TourneyCouples) do
			        if v.Player1 == ply or v.Player2 == ply then
			            return v
			        end
			    end
			    return nil
			end
			
			function data:GetOpponent(ply)
			    local cpl
			    
			    cpl = self:InCouple(ply)
			    if not cpl then
			        -- He's not in a couple yet
			        if self.TourneyPlayerPending then
			            -- We got someone pending! Awesome
			            local tbl = {
			                Player1 = ply,
			                Player2 = self.TourneyPlayerPending,
			                ReturnFunc1 = nil,
			                ReturnFunc2 = nil,
			                ReadyCount = 0,
			            }
			            table.insert(self.TourneyCouples, tbl)
			            self.TourneyPlayerPending = nil
			            self.TourneyPlayersWaiting = self.TourneyPlayersWaiting - 1
			            return tbl.Player2
			        else
			            -- Dont have anyone pending either
			            -- Check if i'm the last one, if so, NPC time
			            -- Otherwise, make me the pending one
			            if self.TourneyPlayersWaiting == 1 then
			                -- Yep, NPC time
			                local tbl = {
			                    Player1 = ply,
			                    Player2 = "AI",
			                    ReturnFunc1 = nil,
			                    ReturnFunc2 = nil,
			                    ReadyCount = 1,
			                }
			                table.insert(self.TourneyCouples, tbl)
			                self.TourneyPlayersWaiting = self.TourneyPlayersWaiting - 1
			                return "AI"
			            else
			                self.TourneyPlayerPending = ply
			                self.TourneyPlayersWaiting = self.TourneyPlayersWaiting - 1
			                return nil
			            end
			        end
			    else 
			        if cpl.Player1 == ply then
			            return cpl.Player2
			        else
			            return cpl.Player1
			        end
			    end
			end
			
			function data:RemovePlayer(ply)
			    local k,v
			    for k,v in pairs(self.TourneyPlayers) do
			        if v == ply then
			            self.TourneyPlayers[k] = nil
			            self.TourneyPlayerCount = self.TourneyPlayerCount -1
			        end
			    end
			end
			
			function data:FinishRound()
			    self:SendNotify("Tournament round completed")
			    if self.TourneyPlayerCount == 0 then
			        self:SendNotify("AI has won the tournament!")
			        -- AI won
			        self.Mode = 5
			        return
			    elseif self.TourneyPlayerCount == 1 then
			        local k,v
			        for k,v in pairs(self.TourneyPlayers) do
			            self:SendNotify(v:GetName() .. " has won the tournament!")
			        end
			        self.Mode = 5
			        return
			    else
			        self:SendNotify("Starting next round!")
			
			        self.TourneyRoundFinished = {}
			        self.TourneyPlayersWaiting = self.TourneyPlayerCount
			        self.TourneyPlayersIngame = 0
			        self.TourneyRound = self.TourneyRound + 1
			        local k,v
			        for k,v in pairs(self.TourneyPlayers) do
			            v:SendChat("Pazaak Tournament: A new round has started!")
			        end
			    end
			end
			
			function data:ReadyToPlay(ply, returnfunc)
			    local cpl
			    cpl = self:InCouple(ply)
			    if not cpl then
			        return false
			    end
			    -- Check who this is
			    if cpl.Player1 == ply then
			        -- It's player 1
			        if not cpl.ReturnFunc1 then
			            cpl.ReadyCount = cpl.ReadyCount + 1
			        end
			        cpl.ReturnFunc1 = returnfunc
			    else
			        -- It's player 2
			        if not cpl.ReturnFunc2 then
			            cpl.ReadyCount = cpl.ReadyCount + 1
			        end
			        cpl.ReturnFunc2 = returnfunc
			    end
			    if cpl.ReadyCount == 2 then
			        -- Time to go!
			        self.TourneyPlayersIngame = self.TourneyPlayersIngame + 2
			        
			        local function pzkFinish(winner)
			            if winner == nil then
			                -- AI wins
			                self:RemovePlayer(cpl.Player1)
			                self:SendNotify(cpl.Player1:GetName() .. " has been defeated by AI")
			            else
			                table.insert(self.TourneyRoundFinished, winner)
			                if winner == cpl.Player1 then
			                    self:RemovePlayer(cpl.Player2)
			                    self:SendNotify(cpl.Player2:GetName() .. " has been defeated by " .. cpl.Player1:GetName())
			                else
			                    self:RemovePlayer(cpl.Player1)
			                    self:SendNotify(cpl.Player1:GetName() .. " has been defeated by " .. cpl.Player2:GetName())
			                end
			            end
			            self.TourneyPlayersIngame = self.TourneyPlayersIngame - 2
			            
			            if self.TourneyPlayersIngame == 0 and self.TourneyPlayersWaiting == 0 then
			                -- Round finished
			                self:FinishRound()
			            end
			            
			            cpl.ReturnFunc1()
			            if cpl.ReturnFunc2 then
			                cpl.ReturnFunc2()
			            end
			        end
			
			        local pzk = sys.CreatePazaakGame()
			        local aigame = cpl.Player2 == "AI"
			        local cards = JKG.Pazaak.Cards
			        if aigame then
			            pzk:SetPlayers(cpl.Player1, nil)
			        else
			            pzk:SetPlayers(cpl.Player1, cpl.Player2)
			        end
			
			        if aigame then
			            pzk:SetAILevel(1)
			            pzk:SetAIName("Tournament Bot")
			            self:SendNotify(cpl.Player1:GetName() .. " is now playing against AI")
			        else
			            self:SendNotify(cpl.Player1:GetName() .. " is now playing against " .. cpl.Player2:GetName())
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
			
			        if aigame then
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
			        end
			                        
			        pzk:ShowCardSelection(true)
			
			    
			        local success, reason = pzk:StartGame()
			        if not success then
			            print("ERROR STARTING PAZAAK: ", reason)
			            cpl.ReturnFunc1()
			            if not aigame then
			                cpl.ReturnFunc2()
			            end
			        end
			    end
			end
		end,
		HasCondition = false,
	},
	T155 = {
		Type = 2,
		SubNode = "L157",
		Text = "The tournament has started.\nThe players can now use the terminals to start playing.",
		Duration = 6000,
		HasCondition = false,
		HasResolver = false,
	},
	L157 = {
		Type = 4,
		Target = "T124",
	},
	L129 = {
		Type = 4,
		NextNode = "L130",
		Target = "O27",
	},
	L130 = {
		Type = 4,
		NextNode = "L131",
		Target = "O42",
	},
	L131 = {
		Type = 4,
		Target = "O8",
	},
	T124 = {
		Type = 2,
		SubNode = "O156",
		NextNode = "T125",
		Text = "The tournament is in progress\n$rempl$ players are ingame.\nThe game's currently on round $round$.\n$remig$ players are participating.",
		Duration = 4000,
		HasCondition = true,
		ConditionFunc = function(owner, ply)
			return JKG.PzkTourney[owner.tname].Mode == 4
		end,
		HasResolver = true,
		ResolveFunc = function(owner, ply, var)
			local data = JKG.PzkTourney[owner.tname]
			
			if var == "rempl" then
			    return data.TourneyPlayersIngame
			elseif var == "round" then
			    return data.TourneyRound
			elseif var == "remig" then
			    return data.TourneyPlayerCount
			end
			
			return ""
		end,
	},
	O156 = {
		Type = 3,
		SubNode = "T163",
		NextNode = "L169",
		Text = "Cancel the tournament",
		HasCondition = false,
		HasResolver = false,
	},
	T163 = {
		Type = 2,
		SubNode = "O164",
		Text = "Are you absolutely sure you wish to abort the tournament?",
		Duration = 4000,
		HasCondition = false,
		HasResolver = false,
	},
	O164 = {
		Type = 3,
		SubNode = "L168",
		NextNode = "O165",
		Text = "Yes, abort it",
		HasCondition = false,
		HasResolver = false,
	},
	L168 = {
		Type = 4,
		Target = "S151",
	},
	O165 = {
		Type = 3,
		SubNode = "L166",
		Text = "No, never mind",
		HasCondition = false,
		HasResolver = false,
	},
	L166 = {
		Type = 4,
		Target = "T124",
	},
	L169 = {
		Type = 4,
		NextNode = "L170",
		Target = "O27",
	},
	L170 = {
		Type = 4,
		NextNode = "L171",
		Target = "O42",
	},
	L171 = {
		Type = 4,
		Target = "O8",
	},
	T125 = {
		Type = 2,
		SubNode = "L146",
		NextNode = "T39",
		Text = "The tournament has finished.\nThe winner is $winner$",
		Duration = 4000,
		HasCondition = true,
		ConditionFunc = function(owner, ply)
			return JKG.PzkTourney[owner.tname].Mode == 5
		end,
		HasResolver = true,
		ResolveFunc = function(owner, ply, var)
			local data = JKG.PzkTourney[owner.tname]
			
			if var == "winner"  then
			    if data.TourneyPlayerCount == 0 then
			        return "AI"
			    elseif self.TourneyPlayerCount == 1 then
			        local k,v
			        for k,v in pairs(self.TourneyPlayers) do
			            return sys.StripColorcodes(v:GetName())
			        end
			    end
			end
			
			return ""
		end,
	},
	L146 = {
		Type = 4,
		NextNode = "O147",
		Target = "O78",
	},
	O147 = {
		Type = 3,
		SubNode = "S151",
		NextNode = "L148",
		Text = "Reset the tournament",
		HasCondition = false,
		HasResolver = false,
	},
	S151 = {
		Type = 6,
		SubNode = "T152",
		ScriptFunc = function(owner, ply)
			local data = JKG.PzkTourney[owner.tname]
			
			data.TourneyRound = 0
			data.TourneyCouples = {}
			data.TourneyPlayerPending = nil
			data.TourneyRoundFinished = {}
			data.TourneyPlayersWaiting = data.TourneyPlayerCount
			data.TourneyPlayersIngame = 0
			data.Mode = 0
		end,
		HasCondition = false,
	},
	T152 = {
		Type = 2,
		SubNode = "L153",
		Text = "The tournament has been reset and the terminals are on standby",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	L153 = {
		Type = 4,
		Target = "T77",
	},
	L148 = {
		Type = 4,
		NextNode = "L149",
		Target = "O27",
	},
	L149 = {
		Type = 4,
		NextNode = "L150",
		Target = "O42",
	},
	L150 = {
		Type = 4,
		Target = "O8",
	},
	T39 = {
		Type = 2,
		SubNode = "O158",
		Text = "Internal error: Status unknown",
		Duration = 1000,
		HasCondition = false,
		HasResolver = false,
	},
	O158 = {
		Type = 3,
		SubNode = "S159",
		NextNode = "L40",
		Text = "Reset",
		HasCondition = false,
		HasResolver = false,
	},
	S159 = {
		Type = 6,
		SubNode = "T160",
		ScriptFunc = function(owner, ply)
			JKG.PzkTourney[owner.tname].Mode = 0
		end,
		HasCondition = false,
	},
	T160 = {
		Type = 2,
		SubNode = "D161",
		Text = "Status has been reset, please relogin",
		Duration = 2000,
		HasCondition = false,
		HasResolver = false,
	},
	D161 = {
		Type = 5,
	},
	L40 = {
		Type = 4,
		NextNode = "L70",
		Target = "O27",
	},
	L70 = {
		Type = 4,
		NextNode = "L41",
		Target = "O42",
	},
	L41 = {
		Type = 4,
		Target = "O8",
	},
	O42 = {
		Type = 3,
		SubNode = "T53",
		NextNode = "O43",
		Text = "Access notification controls",
		HasCondition = false,
		HasResolver = false,
	},
	T53 = {
		Type = 2,
		SubNode = "O55",
		NextNode = "T54",
		Text = "You are currently being notified of events.",
		Duration = 2500,
		HasCondition = true,
		ConditionFunc = function(owner, ply)
			local k,v
			for k,v in pairs(JKG.PzkTourney[owner.tname].NotifyPlayers) do
			    if v == ply then
			        return true
			    end
			end
			return false
		end,
		HasResolver = false,
	},
	O55 = {
		Type = 3,
		SubNode = "S58",
		NextNode = "L71",
		Text = "Stop notifying me of events",
		HasCondition = false,
		HasResolver = false,
	},
	S58 = {
		Type = 6,
		SubNode = "T59",
		ScriptFunc = function(owner, ply)
			local k,v
			for k,v in pairs(JKG.PzkTourney[owner.tname].NotifyPlayers) do
			    if v == ply then
			        JKG.PzkTourney[owner.tname].NotifyPlayers[k] = nil
			    end
			end
		end,
		HasCondition = false,
	},
	T59 = {
		Type = 2,
		SubNode = "L60",
		Text = "You will no longer be informed of events",
		Duration = 2000,
		HasCondition = false,
		HasResolver = false,
	},
	L60 = {
		Type = 4,
		Target = "T5",
	},
	L71 = {
		Type = 4,
		NextNode = "L73",
		Target = "O27",
	},
	L73 = {
		Type = 4,
		NextNode = "L75",
		Target = "O42",
	},
	L75 = {
		Type = 4,
		Target = "O8",
	},
	T54 = {
		Type = 2,
		SubNode = "O61",
		Text = "You are currently not being notified of events.",
		Duration = 2500,
		HasCondition = false,
		HasResolver = false,
	},
	O61 = {
		Type = 3,
		SubNode = "S64",
		NextNode = "L72",
		Text = "Notify me of events",
		HasCondition = false,
		HasResolver = false,
	},
	S64 = {
		Type = 6,
		SubNode = "T65",
		ScriptFunc = function(owner, ply)
			table.insert(JKG.PzkTourney[owner.tname].NotifyPlayers, ply)
		end,
		HasCondition = false,
	},
	T65 = {
		Type = 2,
		SubNode = "L66",
		Text = "You will now be notified of events",
		Duration = 1000,
		HasCondition = false,
		HasResolver = false,
	},
	L66 = {
		Type = 4,
		Target = "T5",
	},
	L72 = {
		Type = 4,
		NextNode = "L74",
		Target = "O27",
	},
	L74 = {
		Type = 4,
		NextNode = "L76",
		Target = "O42",
	},
	L76 = {
		Type = 4,
		Target = "O8",
	},
	O43 = {
		Type = 3,
		SubNode = "T44",
		NextNode = "O8",
		Text = "Terminate system",
		HasCondition = false,
		HasResolver = false,
	},
	T44 = {
		Type = 2,
		SubNode = "T45",
		Text = "Warning, this will completely shut down the pazaak tournament system",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	T45 = {
		Type = 2,
		SubNode = "O46",
		Text = "All data will be lost, are you sure you wish to proceed?",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	O46 = {
		Type = 3,
		SubNode = "S49",
		NextNode = "O47",
		Text = "Yes",
		HasCondition = false,
		HasResolver = false,
	},
	S49 = {
		Type = 6,
		SubNode = "T50",
		ScriptFunc = function(owner, ply)
			JKG.PzkTourney[owner.tname] = nil
		end,
		HasCondition = false,
	},
	T50 = {
		Type = 2,
		SubNode = "L67",
		Text = "Systems Terminated...",
		Duration = 2000,
		HasCondition = false,
		HasResolver = false,
	},
	L67 = {
		Type = 4,
		Target = "T33",
	},
	O47 = {
		Type = 3,
		SubNode = "L48",
		Text = "No",
		HasCondition = false,
		HasResolver = false,
	},
	L48 = {
		Type = 4,
		Target = "T5",
	},
	O8 = {
		Type = 3,
		SubNode = "T33",
		Text = "Log out",
		HasCondition = false,
		HasResolver = false,
	},
	T33 = {
		Type = 2,
		SubNode = "D34",
		Text = "Logging out...",
		Duration = 1500,
		HasCondition = false,
		HasResolver = false,
	},
	D34 = {
		Type = 5,
	},
}

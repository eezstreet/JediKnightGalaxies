-- Exported by JKG Dialogue Creator

DLG.Name = "ArenaControl"
DLG.RootNode = "E1"
DLG.Nodes = {
	E1 = {
		Type = 1,
		SubNode = "T157",
		NextNode = "E2",
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			--[[-----------------------------------------------
				Conditional Script
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				data - Local storage table
			
				Wanted return value: Bool (true/false)
			--]]-----------------------------------------------
			
			return ply.IsAdmin
		end,
	},
	T157 = {
		Type = 2,
		SubNode = "T5",
		Text = "This terminal is restricted to GM's.\nAccess Granted.",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	T5 = {
		Type = 2,
		SubNode = "O6",
		Text = "Arena control terminal.\nPlease choose the desired action:",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	O6 = {
		Type = 3,
		SubNode = "T12",
		NextNode = "O17",
		Text = "Arena status",
		HasCondition = false,
		HasResolver = false,
	},
	T12 = {
		Type = 2,
		SubNode = "L61",
		Text = "The arena status is currently: $status$.",
		Duration = 3000,
		HasCondition = false,
		HasResolver = true,
		ResolveFunc = function(owner, ply, var, data)
			--[[-----------------------------------------------
				Resolver Script
			
				This allows the use of variables in the text
				(single word enclosed in $ signs)
				For example: 'Hello there $name$, how are you.',
				where name is the variable here.
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				var - Variable to resolve (string)
				data - Local storage table
			
				Wanted return value: The text to use in place of
				the variable
			--]]-----------------------------------------------
			
			local MS_NONE = 0			 -- No matches in progress
			local MS_AWAITINGPLAYERS = 1 -- Players are being summoned to the waiting rooms
			local MS_STARTING = 2		 -- Players are in the waiting rooms, doing 10 sec countdown
			local MS_INPROGRESS = 3		 -- The match is in progress
			local MS_FINISHED = 4		 -- The match finished, people can now leave the arena
			
			if var == "status" then
			    local state = JKG.ArenaBackend.GetState(1)
			    if state == MS_NONE then
			        return "Idle"
			    elseif state == MS_AWAITINGPLAYERS then
			        return "Awaiting players"
			    elseif state == MS_STARTING then
			        return "Starting match"
			    elseif state == MS_INPROGRESS then
			        return "Match in progress"
			    elseif state == MS_FINISHED then
			        return "Match is finished, cleaning up"
			    else
			        return "Unknown"
			    end
			end
			
			return ""
		end,
	},
	L61 = {
		Type = 4,
		Target = "T5",
	},
	O17 = {
		Type = 3,
		SubNode = "T26",
		NextNode = "O19",
		Text = "Setup match",
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			return not JKG.ArenaBackend.GetPending(1)
		end,
		HasResolver = false,
	},
	T26 = {
		Type = 2,
		SubNode = "O27",
		Text = "This will set up a new match, do you wish to continue?",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	O27 = {
		Type = 3,
		SubNode = "T30",
		NextNode = "O28",
		Text = "Yes",
		HasCondition = false,
		HasResolver = false,
	},
	T30 = {
		Type = 2,
		SubNode = "O35",
		Text = "Please choose the game type",
		Duration = 2000,
		HasCondition = false,
		HasResolver = false,
	},
	O35 = {
		Type = 3,
		SubNode = "T36",
		NextNode = "O34",
		Text = "Normal mode",
		HasCondition = false,
		HasResolver = false,
	},
	T36 = {
		Type = 2,
		SubNode = "N37",
		Text = "Please specify a kill limit.\nValid range: 5 to 99",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	N37 = {
		Type = 8,
		SubNode = "T38",
		Caption = "Specify the kill limit (5-99)",
		DefVal = "20",
		Flags = 3,
		ScriptFunc = function(owner, ply, response, data)
			--[[-----------------------------------------------
				Text Entry Processing Script
			
				The response to a text entry node can be processed here.
				Use the local storage table to save information and
				adjust the dialogue flow (conditional scripts)
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				response - The response the player gave, nil if cancel was clicked
				data - Local storage table
			
			--]]-----------------------------------------------
			
			if not response then
			    data.Cancel = true
			else
			    data.Cancel = false
			    data.GameMode = 2
			    data.Limit = tonumber(response)
			    if data.Limit < 5 or data.Limit > 99 then
			        data.Retry = true
			    else
			        data.Retry = false
			    end
			end
		end,
		HasCondition = false,
		HasResolver = false,
	},
	T38 = {
		Type = 2,
		SubNode = "L39",
		NextNode = "T40",
		Text = "Invalid kill limit entered.\nPlease choose a kill limit between 5 and 99.",
		Duration = 4000,
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			--[[-----------------------------------------------
				Conditional Script
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				data - Local storage table
			
				Wanted return value: Bool (true/false)
			--]]-----------------------------------------------
			
			return data.Retry
		end,
		HasResolver = false,
	},
	L39 = {
		Type = 4,
		Target = "N37",
	},
	T40 = {
		Type = 2,
		SubNode = "N48",
		NextNode = "L41",
		Text = "Please specify a time limit (in seconds)\nValid range: (30 - 3600)",
		Duration = 3000,
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			--[[-----------------------------------------------
				Conditional Script
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				data - Local storage table
			
				Wanted return value: Bool (true/false)
			--]]-----------------------------------------------
			
			return not data.Cancel
		end,
		HasResolver = false,
	},
	N48 = {
		Type = 8,
		SubNode = "T50",
		Caption = "Specify the time limit (30 - 3600)",
		DefVal = "300",
		Flags = 3,
		ScriptFunc = function(owner, ply, response, data)
			--[[-----------------------------------------------
				Text Entry Processing Script
			
				The response to a text entry node can be processed here.
				Use the local storage table to save information and
				adjust the dialogue flow (conditional scripts)
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				response - The response the player gave, nil if cancel was clicked
				data - Local storage table
			
			--]]-----------------------------------------------
			
			if not response then
			    data.Cancel = true
			else
			    data.Cancel = false
			    data.TimeLimit = tonumber(response)
			    if data.TimeLimit < 30 or data.TimeLimit > 3600 then
			        data.Retry = true
			    else
			        data.Retry = false
			    end
			end
		end,
		HasCondition = false,
		HasResolver = false,
	},
	T50 = {
		Type = 2,
		SubNode = "L65",
		NextNode = "S51",
		Text = "Invalid time limit specified.\nPlease specify a time limit between 30 and 3600 seconds.",
		Duration = 4000,
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			return data.Retry
		end,
		HasResolver = false,
	},
	L65 = {
		Type = 4,
		Target = "N48",
	},
	S51 = {
		Type = 6,
		SubNode = "T53",
		NextNode = "L52",
		ScriptFunc = function(owner, ply, data)
			--[[-----------------------------------------------
				Dialogue Script
			
				Available Variables:
				owner - Entity that runs this conversation
				ply	  - Player the owner is talking to
				data - Local storage table
			--]]-----------------------------------------------
			
			data.SetupOk, data.SetupFailReason = JKG.ArenaBackend.SetupMatch(1, data.GameMode, data.Limit, data.TimeLimit)
		end,
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			--[[-----------------------------------------------
				Conditional Script
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				data - Local storage table
			
				Wanted return value: Bool (true/false)
			--]]-----------------------------------------------
			
			return not data.Cancel
		end,
	},
	T53 = {
		Type = 2,
		SubNode = "L55",
		NextNode = "T54",
		Text = "Failed to register match.\nReason: $error$\nReturning to main menu...",
		Duration = 5000,
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			--[[-----------------------------------------------
				Conditional Script
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				data - Local storage table
			
				Wanted return value: Bool (true/false)
			--]]-----------------------------------------------
			
			return not data.SetupOk
		end,
		HasResolver = true,
		ResolveFunc = function(owner, ply, var, data)
			--[[-----------------------------------------------
				Resolver Script
			
				This allows the use of variables in the text
				(single word enclosed in $ signs)
				For example: 'Hello there $name$, how are you.',
				where name is the variable here.
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				var - Variable to resolve (string)
				data - Local storage table
			
				Wanted return value: The text to use in place of
				the variable
			--]]-----------------------------------------------
			
			if var == "error" then
			    return data.SetupFailReason
			end
			
			return ""
		end,
	},
	L55 = {
		Type = 4,
		Target = "T5",
	},
	T54 = {
		Type = 2,
		SubNode = "L56",
		Text = "Match successfully registered. Returning to main menu...",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	L56 = {
		Type = 4,
		Target = "T5",
	},
	L52 = {
		Type = 4,
		Target = "T5",
	},
	L41 = {
		Type = 4,
		Target = "T5",
	},
	O34 = {
		Type = 3,
		SubNode = "T42",
		NextNode = "O31",
		Text = "Reinforcements mode",
		HasCondition = false,
		HasResolver = false,
	},
	T42 = {
		Type = 2,
		SubNode = "N43",
		Text = "Please specify the amount of reinforcements.\nValid range: 5 to 99",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	N43 = {
		Type = 8,
		SubNode = "T44",
		Caption = "Specify the reinforcement count",
		DefVal = "20",
		Flags = 3,
		ScriptFunc = function(owner, ply, response, data)
			--[[-----------------------------------------------
				Text Entry Processing Script
			
				The response to a text entry node can be processed here.
				Use the local storage table to save information and
				adjust the dialogue flow (conditional scripts)
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				response - The response the player gave, nil if cancel was clicked
				data - Local storage table
			
			--]]-----------------------------------------------
			
			if not response then
			    data.Cancel = true
			else
			    data.Cancel = false
			    data.GameMode = 1
			    data.Limit = tonumber(response)
			    if data.Limit < 5 or data.Limit > 99 then
			        data.Retry = true
			    else
			        data.Retry = false
			    end
			end
		end,
		HasCondition = false,
		HasResolver = false,
	},
	T44 = {
		Type = 2,
		SubNode = "L45",
		NextNode = "T46",
		Text = "Invalid reinforcement count entered.\nPlease choose a kill limit between 5 and 99.",
		Duration = 4000,
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			return data.Retry
		end,
		HasResolver = false,
	},
	L45 = {
		Type = 4,
		Target = "N43",
	},
	T46 = {
		Type = 2,
		SubNode = "L49",
		NextNode = "L47",
		Text = "Please specify a time limit (in seconds)\nValid range: (30 - 3600)",
		Duration = 3000,
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			return not data.Cancel
		end,
		HasResolver = false,
	},
	L49 = {
		Type = 4,
		Target = "N48",
	},
	L47 = {
		Type = 4,
		Target = "T5",
	},
	O31 = {
		Type = 3,
		SubNode = "L33",
		Text = "Cancel",
		HasCondition = false,
		HasResolver = false,
	},
	L33 = {
		Type = 4,
		Target = "T5",
	},
	O28 = {
		Type = 3,
		SubNode = "L29",
		Text = "No",
		HasCondition = false,
		HasResolver = false,
	},
	L29 = {
		Type = 4,
		Target = "T5",
	},
	O19 = {
		Type = 3,
		SubNode = "T62",
		NextNode = "O7",
		Text = "Adjust match parameters",
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			return JKG.ArenaBackend.GetPending(1)
		end,
		HasResolver = false,
	},
	T62 = {
		Type = 2,
		SubNode = "L63",
		NextNode = "T88",
		Text = "Please create a match first.",
		Duration = 2000,
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			--[[-----------------------------------------------
				Conditional Script
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				data - Local storage table
			
				Wanted return value: Bool (true/false)
			--]]-----------------------------------------------
			
			return not JKG.ArenaBackend.GetPending(1)
		end,
		HasResolver = false,
	},
	L63 = {
		Type = 4,
		Target = "T5",
	},
	T88 = {
		Type = 2,
		SubNode = "O89",
		Text = "What do you want to change?",
		Duration = 2000,
		HasCondition = false,
		HasResolver = false,
	},
	O89 = {
		Type = 3,
		SubNode = "T94",
		NextNode = "O90",
		Text = "Game mode ($current$)",
		HasCondition = false,
		HasResolver = true,
		ResolveFunc = function(owner, ply, var, data)
			local MGM_REINFORCEMENTS = 1
			local MGM_KILLS = 2
			
			if var == "current" then
			    local state, gamemode = JKG.ArenaBackend.GetPendingMatch()
			    if not state then
			        return "Unknown"
			    else
			        if gamemode == MGM_REINFORCEMENTS then
			            return "Reinforcements mode"
			        elseif gamemode == MGM_KILLS then
			            return "Normal mode mode"
			        else
			            return "Unknown"
			        end
			    end
			end
			return ""
		end,
	},
	T94 = {
		Type = 2,
		SubNode = "O95",
		Text = "Please choose the new game mode",
		Duration = 2000,
		HasCondition = false,
		HasResolver = false,
	},
	O95 = {
		Type = 3,
		SubNode = "S99",
		NextNode = "O96",
		Text = "Reinforcements mode",
		HasCondition = false,
		HasResolver = false,
	},
	S99 = {
		Type = 6,
		SubNode = "T101",
		ScriptFunc = function(owner, ply, data)
			--[[-----------------------------------------------
				Dialogue Script
			
				Available Variables:
				owner - Entity that runs this conversation
				ply	  - Player the owner is talking to
				data - Local storage table
			--]]-----------------------------------------------
			
			--local MGM_REINFORCEMENTS = 1
			
			data.ChangeOk, data.ChangeError = JKG.ArenaBackend.ChangeMatch(1, 1)
		end,
		HasCondition = false,
	},
	T101 = {
		Type = 2,
		SubNode = "L103",
		NextNode = "T102",
		Text = "Failed to change game mode.\nReason: $error$",
		Duration = 3000,
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			--[[-----------------------------------------------
				Conditional Script
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				data - Local storage table
			
				Wanted return value: Bool (true/false)
			--]]-----------------------------------------------
			
			return not data.ChangeOk
		end,
		HasResolver = true,
		ResolveFunc = function(owner, ply, var, data)
			--[[-----------------------------------------------
				Resolver Script
			
				This allows the use of variables in the text
				(single word enclosed in $ signs)
				For example: 'Hello there $name$, how are you.',
				where name is the variable here.
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				var - Variable to resolve (string)
				data - Local storage table
			
				Wanted return value: The text to use in place of
				the variable
			--]]-----------------------------------------------
			
			if var == "error" then
			    return data.ChangeError
			end
			
			return ""
		end,
	},
	L103 = {
		Type = 4,
		Target = "T88",
	},
	T102 = {
		Type = 2,
		SubNode = "L104",
		Text = "Game mode changed successfully",
		Duration = 2000,
		HasCondition = false,
		HasResolver = false,
	},
	L104 = {
		Type = 4,
		Target = "T88",
	},
	O96 = {
		Type = 3,
		SubNode = "S100",
		NextNode = "O97",
		Text = "Normal mode",
		HasCondition = false,
		HasResolver = false,
	},
	S100 = {
		Type = 6,
		SubNode = "L105",
		ScriptFunc = function(owner, ply, data)
			--[[-----------------------------------------------
				Dialogue Script
			
				Available Variables:
				owner - Entity that runs this conversation
				ply	  - Player the owner is talking to
				data - Local storage table
			--]]-----------------------------------------------
			
			-- local MGM_KILLS = 2
			data.ChangeOk, data.ChangeError = JKG.ArenaBackend.ChangeMatch(1, 2)
		end,
		HasCondition = false,
	},
	L105 = {
		Type = 4,
		NextNode = "L106",
		Target = "T101",
	},
	L106 = {
		Type = 4,
		Target = "T102",
	},
	O97 = {
		Type = 3,
		SubNode = "L98",
		Text = "Cancel",
		HasCondition = false,
		HasResolver = false,
	},
	L98 = {
		Type = 4,
		Target = "T88",
	},
	O90 = {
		Type = 3,
		SubNode = "N107",
		NextNode = "O91",
		Text = "Limit ($current$)",
		HasCondition = false,
		HasResolver = true,
		ResolveFunc = function(owner, ply, var, data)
			local MGM_REINFORCEMENTS = 1
			local MGM_KILLS = 2
			
			if var == "current" then
			    local state, gamemode, limit = JKG.ArenaBackend.GetPendingMatch()
			    if not state then
			        return "Unknown"
			    else
			        if gamemode == MGM_REINFORCEMENTS then
			            return string.format("%i reinforcements", limit)
			        elseif gamemode == MGM_KILLS then
			            return string.format("%i kills", limit)
			        else
			            return "Unknown"
			        end
			    end
			end
			return ""
		end,
	},
	N107 = {
		Type = 8,
		SubNode = "S108",
		Caption = "Please enter the new limit (5-99)",
		DefVal = "$current$",
		Flags = 3,
		ScriptFunc = function(owner, ply, response, data)
			--[[-----------------------------------------------
				Text Entry Processing Script
			
				The response to a text entry node can be processed here.
				Use the local storage table to save information and
				adjust the dialogue flow (conditional scripts)
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				response - The response the player gave, nil if cancel was clicked
				data - Local storage table
			
			--]]-----------------------------------------------
			
			if not response then
			    data.Cancel = true
			else
			    data.Cancel = false
			    data.NewLimit = tonumber(response)
			    if data.NewLimit > 99 then data.NewLimit = 99 end
			    if data.NewLimit < 5 then data.NewLimit = 5 end
			end
		end,
		HasCondition = false,
		HasResolver = true,
		ResolveFunc = function(owner, ply, var, data)
			--[[-----------------------------------------------
				Resolver Script
			
				This allows the use of variables in the text
				(single word enclosed in $ signs)
				For example: 'Hello there $name$, how are you.',
				where name is the variable here.
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				var - Variable to resolve (string)
				data - Local storage table
			
				Wanted return value: The text to use in place of
				the variable
			--]]-----------------------------------------------
			if var == "current" then
			    local ok, _, limit = JKG.ArenaBackend.GetPendingMatch()
			    if not ok then
			        return 20
			    else
			        return limit
			    end
			end
			
			return ""
		end,
	},
	S108 = {
		Type = 6,
		SubNode = "T110",
		NextNode = "L109",
		ScriptFunc = function(owner, ply, data)
			--[[-----------------------------------------------
				Dialogue Script
			
				Available Variables:
				owner - Entity that runs this conversation
				ply	  - Player the owner is talking to
				data - Local storage table
			--]]-----------------------------------------------
			
			data.ChangeOk, data.ChangeError = JKG.ArenaBackend.ChangeMatch(1, nil, data.NewLimit)
		end,
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			--[[-----------------------------------------------
				Conditional Script
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				data - Local storage table
			
				Wanted return value: Bool (true/false)
			--]]-----------------------------------------------
			
			return not data.Cancel
		end,
	},
	T110 = {
		Type = 2,
		SubNode = "L112",
		NextNode = "T111",
		Text = "Failed to change limit.\nReason: $error$",
		Duration = 3000,
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			--[[-----------------------------------------------
				Conditional Script
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				data - Local storage table
			
				Wanted return value: Bool (true/false)
			--]]-----------------------------------------------
			
			return not data.ChangeOk
		end,
		HasResolver = true,
		ResolveFunc = function(owner, ply, var, data)
			--[[-----------------------------------------------
				Resolver Script
			
				This allows the use of variables in the text
				(single word enclosed in $ signs)
				For example: 'Hello there $name$, how are you.',
				where name is the variable here.
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				var - Variable to resolve (string)
				data - Local storage table
			
				Wanted return value: The text to use in place of
				the variable
			--]]-----------------------------------------------
			
			if var == "error" then
			    return data.ChangeError
			end
			
			return ""
		end,
	},
	L112 = {
		Type = 4,
		Target = "T88",
	},
	T111 = {
		Type = 2,
		SubNode = "L113",
		Text = "Limit successfully changed",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	L113 = {
		Type = 4,
		Target = "T88",
	},
	L109 = {
		Type = 4,
		Target = "T88",
	},
	O91 = {
		Type = 3,
		SubNode = "N114",
		NextNode = "O92",
		Text = "Time limit ($current$)",
		HasCondition = false,
		HasResolver = true,
		ResolveFunc = function(owner, ply, var, data)
			local MGM_REINFORCEMENTS = 1
			local MGM_KILLS = 2
			
			if var == "current" then
			    local state, _, _, timelimit = JKG.ArenaBackend.GetPendingMatch()
			    if not state then
			        return "Unknown"
			    else
			        return string.format("%i seconds", timelimit)
			    end
			end
			return ""
		end,
	},
	N114 = {
		Type = 8,
		SubNode = "S115",
		Caption = "Please enter the new time limit (30-3600)",
		DefVal = "$current$",
		Flags = 3,
		ScriptFunc = function(owner, ply, response, data)
			--[[-----------------------------------------------
				Text Entry Processing Script
			
				The response to a text entry node can be processed here.
				Use the local storage table to save information and
				adjust the dialogue flow (conditional scripts)
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				response - The response the player gave, nil if cancel was clicked
				data - Local storage table
			
			--]]-----------------------------------------------
			
			if not response then
			    data.Cancel = true
			else
			    data.Cancel = false
			    data.NewTime = tonumber(response)
			    if data.NewTime > 3600 then data.NewTime = 3600 end
			    if data.NewTime < 30 then data.NewTime = 30 end
			end
		end,
		HasCondition = false,
		HasResolver = true,
		ResolveFunc = function(owner, ply, var, data)
			--[[-----------------------------------------------
				Resolver Script
			
				This allows the use of variables in the text
				(single word enclosed in $ signs)
				For example: 'Hello there $name$, how are you.',
				where name is the variable here.
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				var - Variable to resolve (string)
				data - Local storage table
			
				Wanted return value: The text to use in place of
				the variable
			--]]-----------------------------------------------
			
			if var == "current" then
			    local ok, _, _, timelimit = JKG.ArenaBackend.GetPendingMatch()
			    if not ok then
			        return 300
			    else
			        return timelimit
			    end
			end
			
			return ""
		end,
	},
	S115 = {
		Type = 6,
		SubNode = "T117",
		NextNode = "L116",
		ScriptFunc = function(owner, ply, data)
			--[[-----------------------------------------------
				Dialogue Script
			
				Available Variables:
				owner - Entity that runs this conversation
				ply	  - Player the owner is talking to
				data - Local storage table
			--]]-----------------------------------------------
			
			data.ChangeOk, data.ChangeError = JKG.ArenaBackend.ChangeMatch(1, nil, nil, data.NewTime)
		end,
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			--[[-----------------------------------------------
				Conditional Script
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				data - Local storage table
			
				Wanted return value: Bool (true/false)
			--]]-----------------------------------------------
			
			return not data.Cancel
		end,
	},
	T117 = {
		Type = 2,
		SubNode = "L119",
		NextNode = "T118",
		Text = "Failed to change time limit.\nReason: $error$",
		Duration = 3000,
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			--[[-----------------------------------------------
				Conditional Script
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				data - Local storage table
			
				Wanted return value: Bool (true/false)
			--]]-----------------------------------------------
			
			return not data.ChangeOk
		end,
		HasResolver = true,
		ResolveFunc = function(owner, ply, var, data)
			--[[-----------------------------------------------
				Resolver Script
			
				This allows the use of variables in the text
				(single word enclosed in $ signs)
				For example: 'Hello there $name$, how are you.',
				where name is the variable here.
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				var - Variable to resolve (string)
				data - Local storage table
			
				Wanted return value: The text to use in place of
				the variable
			--]]-----------------------------------------------
			
			if var == "error" then
			    return data.ChangeError
			end
			
			return ""
		end,
	},
	L119 = {
		Type = 4,
		Target = "T88",
	},
	T118 = {
		Type = 2,
		SubNode = "L120",
		Text = "Time limit successfully changed",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	L120 = {
		Type = 4,
		Target = "T88",
	},
	L116 = {
		Type = 4,
		Target = "T88",
	},
	O92 = {
		Type = 3,
		SubNode = "L93",
		Text = "Go back",
		HasCondition = false,
		HasResolver = false,
	},
	L93 = {
		Type = 4,
		Target = "T5",
	},
	O7 = {
		Type = 3,
		SubNode = "T18",
		NextNode = "O8",
		Text = "Team management",
		HasCondition = false,
		HasResolver = false,
	},
	T18 = {
		Type = 2,
		SubNode = "L66",
		NextNode = "T121",
		Text = "Team management unavailable, please set up a match first.",
		Duration = 3000,
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			--[[-----------------------------------------------
				Conditional Script
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				data - Local storage table
			
				Wanted return value: Bool (true/false)
			--]]-----------------------------------------------
			
			return not JKG.ArenaBackend.GetPending(1)
		end,
		HasResolver = false,
	},
	L66 = {
		Type = 4,
		Target = "T5",
	},
	T121 = {
		Type = 2,
		SubNode = "O122",
		Text = "There are currently $data$.\nPlease choose a team to edit:",
		Duration = 4000,
		HasCondition = false,
		HasResolver = true,
		ResolveFunc = function(owner, ply, var, data)
			--[[-----------------------------------------------
				Resolver Script
			
				This allows the use of variables in the text
				(single word enclosed in $ signs)
				For example: 'Hello there $name$, how are you.',
				where name is the variable here.
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				var - Variable to resolve (string)
				data - Local storage table
			
				Wanted return value: The text to use in place of
				the variable
			--]]-----------------------------------------------
			
			if var == "data" then
			    local redteam = JKG.ArenaBackend.GetPendingTeam(1)
			    local blueteam = JKG.ArenaBackend.GetPendingTeam(2)
			
			    local total = #redteam + #blueteam
			
			    if total == 0 then
			        return "no players signed up."
			    else
			        return string.format("%i player(s) signed up. (%i in red, %i in blue)", total, #redteam, #blueteam)
			    end
			    return true
			end
			
			return ""
		end,
	},
	O122 = {
		Type = 3,
		SubNode = "T128",
		NextNode = "O123",
		Text = "Red team",
		HasCondition = false,
		HasResolver = false,
	},
	T128 = {
		Type = 2,
		SubNode = "O129",
		Text = "The red team has $count$ member(s):\n$members$\n",
		Duration = 3000,
		HasCondition = false,
		HasResolver = true,
		ResolveFunc = function(owner, ply, var, data)
			--[[-----------------------------------------------
				Resolver Script
			
				This allows the use of variables in the text
				(single word enclosed in $ signs)
				For example: 'Hello there $name$, how are you.',
				where name is the variable here.
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				var - Variable to resolve (string)
				data - Local storage table
			
				Wanted return value: The text to use in place of
				the variable
			--]]-----------------------------------------------
			
			if var == "count" then
			    local team = JKG.ArenaBackend.GetPendingTeam(1)
			    return #team
			elseif var == "members" then
			    local team = JKG.ArenaBackend.GetPendingTeam(1)
			    local names = ""
			    local k,v
			    for k,v in pairs(team) do
			        names = names .. sys.StripColorcodes(v.Name) .. " "
			    end
			    return names
			end
			
			return ""
		end,
	},
	O129 = {
		Type = 3,
		SubNode = "N158",
		NextNode = "O130",
		Text = "Add new member",
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			--[[-----------------------------------------------
				Conditional Script
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				data - Local storage table
			
				Wanted return value: Bool (true/false)
			--]]-----------------------------------------------
			
			local team = JKG.ArenaBackend.GetPendingTeam(1)
			return #team < 3
		end,
		HasResolver = false,
	},
	N158 = {
		Type = 8,
		SubNode = "T159",
		Caption = "Enter the name of the player to add.",
		DefVal = "",
		Flags = 1,
		ScriptFunc = function(owner, ply, response, data)
			--[[-----------------------------------------------
				Text Entry Processing Script
			
				The response to a text entry node can be processed here.
				Use the local storage table to save information and
				adjust the dialogue flow (conditional scripts)
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				response - The response the player gave, nil if cancel was clicked
				data - Local storage table
			
			--]]-----------------------------------------------
			
			if not response then
			    data.Cancel = true
			else
			    data.Cancel = false
			    data.PlayerText = response
			    data.PlayerToAdd = players.GetByName(response)
			end
		end,
		HasCondition = false,
		HasResolver = false,
	},
	T159 = {
		Type = 2,
		SubNode = "L161",
		NextNode = "T160",
		Text = "Player '$name$' not found.",
		Duration = 3000,
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			--[[-----------------------------------------------
				Conditional Script
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				data - Local storage table
			
				Wanted return value: Bool (true/false)
			--]]-----------------------------------------------
			
			return not data.PlayerToAdd and not data.Cancel
		end,
		HasResolver = true,
		ResolveFunc = function(owner, ply, var, data)
			--[[-----------------------------------------------
				Resolver Script
			
				This allows the use of variables in the text
				(single word enclosed in $ signs)
				For example: 'Hello there $name$, how are you.',
				where name is the variable here.
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				var - Variable to resolve (string)
				data - Local storage table
			
				Wanted return value: The text to use in place of
				the variable
			--]]-----------------------------------------------
			
			if var == "name" then
			    return data.PlayerText
			end
			
			return ""
		end,
	},
	L161 = {
		Type = 4,
		Target = "T128",
	},
	T160 = {
		Type = 2,
		SubNode = "O163",
		NextNode = "L162",
		Text = "Do you want to add $name$ to the team?",
		Duration = 3000,
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			--[[-----------------------------------------------
				Conditional Script
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				data - Local storage table
			
				Wanted return value: Bool (true/false)
			--]]-----------------------------------------------
			
			return data.PlayerToAdd ~= nil and not data.Cancel
		end,
		HasResolver = true,
		ResolveFunc = function(owner, ply, var, data)
			--[[-----------------------------------------------
				Resolver Script
			
				This allows the use of variables in the text
				(single word enclosed in $ signs)
				For example: 'Hello there $name$, how are you.',
				where name is the variable here.
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				var - Variable to resolve (string)
				data - Local storage table
			
				Wanted return value: The text to use in place of
				the variable
			--]]-----------------------------------------------
			
			if var == "name" then
			    return sys.StripColorcodes(data.PlayerToAdd:GetName())
			end
			
			return ""
		end,
	},
	O163 = {
		Type = 3,
		SubNode = "S166",
		NextNode = "O164",
		Text = "Yes",
		HasCondition = false,
		HasResolver = false,
	},
	S166 = {
		Type = 6,
		SubNode = "T167",
		ScriptFunc = function(owner, ply, data)
			--[[-----------------------------------------------
				Dialogue Script
			
				Available Variables:
				owner - Entity that runs this conversation
				ply	  - Player the owner is talking to
				data - Local storage table
			--]]-----------------------------------------------
			
			data.AddOk, data.AddError = JKG.ArenaBackend.SignUpPlayer(1, 1, data.PlayerToAdd)
		end,
		HasCondition = false,
	},
	T167 = {
		Type = 2,
		SubNode = "L168",
		NextNode = "T169",
		Text = "Could not add player to the team.\nReason: $error$",
		Duration = 3000,
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			--[[-----------------------------------------------
				Conditional Script
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				data - Local storage table
			
				Wanted return value: Bool (true/false)
			--]]-----------------------------------------------
			
			return not data.AddOk
		end,
		HasResolver = true,
		ResolveFunc = function(owner, ply, var, data)
			--[[-----------------------------------------------
				Resolver Script
			
				This allows the use of variables in the text
				(single word enclosed in $ signs)
				For example: 'Hello there $name$, how are you.',
				where name is the variable here.
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				var - Variable to resolve (string)
				data - Local storage table
			
				Wanted return value: The text to use in place of
				the variable
			--]]-----------------------------------------------
			
			if var == "error" then
			    return data.AddError
			end
			
			return ""
		end,
	},
	L168 = {
		Type = 4,
		Target = "T128",
	},
	T169 = {
		Type = 2,
		SubNode = "L170",
		Text = "The player has been successfully added to the team",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	L170 = {
		Type = 4,
		Target = "T128",
	},
	O164 = {
		Type = 3,
		SubNode = "L165",
		Text = "No",
		HasCondition = false,
		HasResolver = false,
	},
	L165 = {
		Type = 4,
		Target = "T128",
	},
	L162 = {
		Type = 4,
		Target = "T128",
	},
	O130 = {
		Type = 3,
		SubNode = "T171",
		NextNode = "O138",
		Text = "Remove member",
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			--[[-----------------------------------------------
				Conditional Script
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				data - Local storage table
			
				Wanted return value: Bool (true/false)
			--]]-----------------------------------------------
			
			local team = JKG.ArenaBackend.GetPendingTeam(1)
			return #team > 0
		end,
		HasResolver = false,
	},
	T171 = {
		Type = 2,
		SubNode = "Y172",
		Text = "Who do you want to remove from the team?",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	Y172 = {
		Type = 9,
		SubNode = "S175",
		NextNode = "O173",
		Host = "Y172",
		SetupFunc = function(owner, ply, AddOption, data)
			--[[-----------------------------------------------
				Dynamic Options Setup Script
			
				This script is where the actual options are to be defined
				To define an option call AddOption("Choice text", "Tag")
				When an option is chosen, the tag associated with that option
				will be passed to the processing script
			
				The tag must be a number or a string!
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				data - Local storage table
			
			--]]-----------------------------------------------
			
			local team = JKG.ArenaBackend.GetPendingTeam(1)
			local k,v
			
			for k,v in pairs(team) do
			    AddOption(sys.StripColorcodes(v:GetName()), k)
			end
		end,
		ProcessFunc = function(owner, ply, tag, data)
			--[[-----------------------------------------------
				Dynamic Options Process Script
			
				The chosen option from a Dynamic Options node can be processed here
				Use the local storage table to save information and
				adjust the dialogue flow (conditional scripts)
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				tag - The tag of the chosen option
				data - Local storage table
			
			--]]-----------------------------------------------
			
			data.SelectedIndex = tag
		end,
	},
	S175 = {
		Type = 6,
		SubNode = "T176",
		ScriptFunc = function(owner, ply, data)
			--[[-----------------------------------------------
				Dialogue Script
			
				Available Variables:
				owner - Entity that runs this conversation
				ply	  - Player the owner is talking to
				data - Local storage table
			--]]-----------------------------------------------
			
			local team = JKG.ArenaBackend.GetPendingTeam(1)
			data.RemoveOk, data.RemoveError = JKG.ArenaBackend.RemovePlayer(1, 1, team[data.SelectedIndex])
		end,
		HasCondition = false,
	},
	T176 = {
		Type = 2,
		SubNode = "L177",
		NextNode = "T178",
		Text = "Failed to remove the player from the team.\nReason: $error$",
		Duration = 3000,
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			--[[-----------------------------------------------
				Conditional Script
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				data - Local storage table
			
				Wanted return value: Bool (true/false)
			--]]-----------------------------------------------
			
			return not data.RemoveOk
		end,
		HasResolver = true,
		ResolveFunc = function(owner, ply, var, data)
			--[[-----------------------------------------------
				Resolver Script
			
				This allows the use of variables in the text
				(single word enclosed in $ signs)
				For example: 'Hello there $name$, how are you.',
				where name is the variable here.
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				var - Variable to resolve (string)
				data - Local storage table
			
				Wanted return value: The text to use in place of
				the variable
			--]]-----------------------------------------------
			
			if var == "error" then
			    return data.RemoveError
			end
			
			return ""
		end,
	},
	L177 = {
		Type = 4,
		Target = "T128",
	},
	T178 = {
		Type = 2,
		SubNode = "L179",
		Text = "The player has been successfully removed from the team",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	L179 = {
		Type = 4,
		Target = "T128",
	},
	O173 = {
		Type = 3,
		SubNode = "L174",
		Text = "Go back",
		HasCondition = false,
		HasResolver = false,
	},
	L174 = {
		Type = 4,
		Target = "T128",
	},
	O138 = {
		Type = 3,
		SubNode = "S142",
		NextNode = "O140",
		Text = "Clear team",
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			local team = JKG.ArenaBackend.GetPendingTeam(1)
			return #team > 0
		end,
		HasResolver = false,
	},
	S142 = {
		Type = 6,
		SubNode = "T143",
		ScriptFunc = function(owner, ply, data)
			--[[-----------------------------------------------
				Dialogue Script
			
				Available Variables:
				owner - Entity that runs this conversation
				ply	  - Player the owner is talking to
				data - Local storage table
			--]]-----------------------------------------------
			
			JKG.ArenaBackend.ClearTeam(1, 1)
		end,
		HasCondition = false,
	},
	T143 = {
		Type = 2,
		SubNode = "L144",
		Text = "All team members have been removed",
		Duration = 2000,
		HasCondition = false,
		HasResolver = false,
	},
	L144 = {
		Type = 4,
		Target = "T128",
	},
	O140 = {
		Type = 3,
		SubNode = "S145",
		NextNode = "O131",
		Text = "Remove invalid players from team",
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			local team = JKG.ArenaBackend.GetPendingTeam(1)
			return #team > 0
		end,
		HasResolver = false,
	},
	S145 = {
		Type = 6,
		SubNode = "T146",
		ScriptFunc = function(owner, ply, data)
			--[[-----------------------------------------------
				Dialogue Script
			
				Available Variables:
				owner - Entity that runs this conversation
				ply	  - Player the owner is talking to
				data - Local storage table
			--]]-----------------------------------------------
			
			JKG.ArenaBackend.CheckTeam(1, 1)
		end,
		HasCondition = false,
	},
	T146 = {
		Type = 2,
		SubNode = "L147",
		Text = "All invalid players have been removed from the team.",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	L147 = {
		Type = 4,
		Target = "T128",
	},
	O131 = {
		Type = 3,
		SubNode = "L132",
		Text = "Go back",
		HasCondition = false,
		HasResolver = false,
	},
	L132 = {
		Type = 4,
		Target = "T121",
	},
	O123 = {
		Type = 3,
		SubNode = "T133",
		NextNode = "O124",
		Text = "Blue team",
		HasCondition = false,
		HasResolver = false,
	},
	T133 = {
		Type = 2,
		SubNode = "O134",
		Text = "The blue team has $count$ member(s):\n$members$\n",
		Duration = 3000,
		HasCondition = false,
		HasResolver = true,
		ResolveFunc = function(owner, ply, var, data)
			--[[-----------------------------------------------
				Resolver Script
			
				This allows the use of variables in the text
				(single word enclosed in $ signs)
				For example: 'Hello there $name$, how are you.',
				where name is the variable here.
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				var - Variable to resolve (string)
				data - Local storage table
			
				Wanted return value: The text to use in place of
				the variable
			--]]-----------------------------------------------
			
			if var == "count" then
			    local team = JKG.ArenaBackend.GetPendingTeam(2)
			    return #team
			elseif var == "members" then
			    local team = JKG.ArenaBackend.GetPendingTeam(2)
			    local names = ""
			    local k,v
			    for k,v in pairs(team) do
			        names = names .. sys.StripColorcodes(v.Name) .. "^7 "
			    end
			    return names
			end
			
			return ""
		end,
	},
	O134 = {
		Type = 3,
		SubNode = "N182",
		NextNode = "O135",
		Text = "Add new member",
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			--[[-----------------------------------------------
				Conditional Script
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				data - Local storage table
			
				Wanted return value: Bool (true/false)
			--]]-----------------------------------------------
			
			local team = JKG.ArenaBackend.GetPendingTeam(2)
			return #team < 3
		end,
		HasResolver = false,
	},
	N182 = {
		Type = 8,
		SubNode = "T183",
		Caption = "Enter the name of the player to add.",
		DefVal = "",
		Flags = 1,
		ScriptFunc = function(owner, ply, response, data)
			--[[-----------------------------------------------
				Text Entry Processing Script
			
				The response to a text entry node can be processed here.
				Use the local storage table to save information and
				adjust the dialogue flow (conditional scripts)
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				response - The response the player gave, nil if cancel was clicked
				data - Local storage table
			
			--]]-----------------------------------------------
			
			if not response then
			    data.Cancel = true
			else
			    data.Cancel = false
			    data.PlayerText = response
			    data.PlayerToAdd = players.GetByName(response)
			end
		end,
		HasCondition = false,
		HasResolver = false,
	},
	T183 = {
		Type = 2,
		SubNode = "L184",
		NextNode = "T185",
		Text = "Player '$name$' not found.",
		Duration = 3000,
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			--[[-----------------------------------------------
				Conditional Script
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				data - Local storage table
			
				Wanted return value: Bool (true/false)
			--]]-----------------------------------------------
			
			return not data.PlayerToAdd and not data.Cancel
		end,
		HasResolver = true,
		ResolveFunc = function(owner, ply, var, data)
			--[[-----------------------------------------------
				Resolver Script
			
				This allows the use of variables in the text
				(single word enclosed in $ signs)
				For example: 'Hello there $name$, how are you.',
				where name is the variable here.
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				var - Variable to resolve (string)
				data - Local storage table
			
				Wanted return value: The text to use in place of
				the variable
			--]]-----------------------------------------------
			
			if var == "name" then
			    return data.PlayerText
			end
			
			return ""
		end,
	},
	L184 = {
		Type = 4,
		Target = "T133",
	},
	T185 = {
		Type = 2,
		SubNode = "O187",
		NextNode = "L186",
		Text = "Do you want to add $name$ to the team?",
		Duration = 3000,
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			--[[-----------------------------------------------
				Conditional Script
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				data - Local storage table
			
				Wanted return value: Bool (true/false)
			--]]-----------------------------------------------
			
			return data.PlayerToAdd ~= nil and not data.Cancel
		end,
		HasResolver = true,
		ResolveFunc = function(owner, ply, var, data)
			--[[-----------------------------------------------
				Resolver Script
			
				This allows the use of variables in the text
				(single word enclosed in $ signs)
				For example: 'Hello there $name$, how are you.',
				where name is the variable here.
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				var - Variable to resolve (string)
				data - Local storage table
			
				Wanted return value: The text to use in place of
				the variable
			--]]-----------------------------------------------
			
			if var == "name" then
			    return sys.StripColorcodes(data.PlayerToAdd:GetName())
			end
			
			return ""
		end,
	},
	O187 = {
		Type = 3,
		SubNode = "S190",
		NextNode = "O188",
		Text = "Yes",
		HasCondition = false,
		HasResolver = false,
	},
	S190 = {
		Type = 6,
		SubNode = "T191",
		ScriptFunc = function(owner, ply, data)
			--[[-----------------------------------------------
				Dialogue Script
			
				Available Variables:
				owner - Entity that runs this conversation
				ply	  - Player the owner is talking to
				data - Local storage table
			--]]-----------------------------------------------
			
			data.AddOk, data.AddError = JKG.ArenaBackend.SignUpPlayer(1, 2, data.PlayerToAdd)
		end,
		HasCondition = false,
	},
	T191 = {
		Type = 2,
		SubNode = "L192",
		NextNode = "T193",
		Text = "Could not add player to the team.\nReason: $error$",
		Duration = 3000,
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			--[[-----------------------------------------------
				Conditional Script
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				data - Local storage table
			
				Wanted return value: Bool (true/false)
			--]]-----------------------------------------------
			
			return not data.AddOk
		end,
		HasResolver = true,
		ResolveFunc = function(owner, ply, var, data)
			--[[-----------------------------------------------
				Resolver Script
			
				This allows the use of variables in the text
				(single word enclosed in $ signs)
				For example: 'Hello there $name$, how are you.',
				where name is the variable here.
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				var - Variable to resolve (string)
				data - Local storage table
			
				Wanted return value: The text to use in place of
				the variable
			--]]-----------------------------------------------
			
			if var == "error" then
			    return data.AddError
			end
			
			return ""
			
		end,
	},
	L192 = {
		Type = 4,
		Target = "T133",
	},
	T193 = {
		Type = 2,
		SubNode = "L194",
		Text = "The player has been successfully added to the team",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	L194 = {
		Type = 4,
		Target = "T133",
	},
	O188 = {
		Type = 3,
		SubNode = "L189",
		Text = "No",
		HasCondition = false,
		HasResolver = false,
	},
	L189 = {
		Type = 4,
		Target = "T133",
	},
	L186 = {
		Type = 4,
		Target = "T133",
	},
	O135 = {
		Type = 3,
		SubNode = "T203",
		NextNode = "O139",
		Text = "Remove member",
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			local team = JKG.ArenaBackend.GetPendingTeam(2)
			return #team > 0
		end,
		HasResolver = false,
	},
	T203 = {
		Type = 2,
		SubNode = "Y195",
		Text = "Who do you want to remove from the team?",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	Y195 = {
		Type = 9,
		SubNode = "S198",
		NextNode = "O196",
		Host = "Y195",
		SetupFunc = function(owner, ply, AddOption, data)
			--[[-----------------------------------------------
				Dynamic Options Setup Script
			
				This script is where the actual options are to be defined
				To define an option call AddOption("Choice text", "Tag")
				When an option is chosen, the tag associated with that option
				will be passed to the processing script
			
				The tag must be a number or a string!
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				data - Local storage table
			
			--]]-----------------------------------------------
			
			local team = JKG.ArenaBackend.GetPendingTeam(2)
			local k,v
			
			for k,v in pairs(team) do
			    AddOption(sys.StripColorcodes(v:GetName()), k)
			end
		end,
		ProcessFunc = function(owner, ply, tag, data)
			--[[-----------------------------------------------
				Dynamic Options Process Script
			
				The chosen option from a Dynamic Options node can be processed here
				Use the local storage table to save information and
				adjust the dialogue flow (conditional scripts)
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				tag - The tag of the chosen option
				data - Local storage table
			
			--]]-----------------------------------------------
			
			data.SelectedIndex = tag
		end,
	},
	S198 = {
		Type = 6,
		SubNode = "T199",
		ScriptFunc = function(owner, ply, data)
			--[[-----------------------------------------------
				Dialogue Script
			
				Available Variables:
				owner - Entity that runs this conversation
				ply	  - Player the owner is talking to
				data - Local storage table
			--]]-----------------------------------------------
			
			local team = JKG.ArenaBackend.GetPendingTeam(2)
			data.RemoveOk, data.RemoveError = JKG.ArenaBackend.RemovePlayer(1, 2, team[data.SelectedIndex])
		end,
		HasCondition = false,
	},
	T199 = {
		Type = 2,
		SubNode = "L200",
		NextNode = "T201",
		Text = "Failed to remove the player from the team.\nReason: $error$",
		Duration = 3000,
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			--[[-----------------------------------------------
				Conditional Script
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				data - Local storage table
			
				Wanted return value: Bool (true/false)
			--]]-----------------------------------------------
			
			return not data.RemoveOk
		end,
		HasResolver = true,
		ResolveFunc = function(owner, ply, var, data)
			--[[-----------------------------------------------
				Resolver Script
			
				This allows the use of variables in the text
				(single word enclosed in $ signs)
				For example: 'Hello there $name$, how are you.',
				where name is the variable here.
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				var - Variable to resolve (string)
				data - Local storage table
			
				Wanted return value: The text to use in place of
				the variable
			--]]-----------------------------------------------
			
			if var == "error" then
			    return data.RemoveError
			end
			
			return ""
		end,
	},
	L200 = {
		Type = 4,
		Target = "T133",
	},
	T201 = {
		Type = 2,
		SubNode = "L202",
		Text = "The player has been successfully removed from the team.",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	L202 = {
		Type = 4,
		Target = "T133",
	},
	O196 = {
		Type = 3,
		SubNode = "L197",
		Text = "Go back",
		HasCondition = false,
		HasResolver = false,
	},
	L197 = {
		Type = 4,
		Target = "T133",
	},
	O139 = {
		Type = 3,
		SubNode = "S148",
		NextNode = "O141",
		Text = "Clear team",
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			local team = JKG.ArenaBackend.GetPendingTeam(2)
			return #team > 0
		end,
		HasResolver = false,
	},
	S148 = {
		Type = 6,
		SubNode = "T149",
		ScriptFunc = function(owner, ply, data)
			--[[-----------------------------------------------
				Dialogue Script
			
				Available Variables:
				owner - Entity that runs this conversation
				ply	  - Player the owner is talking to
				data - Local storage table
			--]]-----------------------------------------------
			
			JKG.ArenaBackend.ClearTeam(1, 2)
		end,
		HasCondition = false,
	},
	T149 = {
		Type = 2,
		SubNode = "L150",
		Text = "All team members have been removed",
		Duration = 2000,
		HasCondition = false,
		HasResolver = false,
	},
	L150 = {
		Type = 4,
		Target = "T133",
	},
	O141 = {
		Type = 3,
		SubNode = "S151",
		NextNode = "O136",
		Text = "Remove invalid players from team",
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			local team = JKG.ArenaBackend.GetPendingTeam(2)
			return #team > 0
		end,
		HasResolver = false,
	},
	S151 = {
		Type = 6,
		SubNode = "T152",
		ScriptFunc = function(owner, ply, data)
			--[[-----------------------------------------------
				Dialogue Script
			
				Available Variables:
				owner - Entity that runs this conversation
				ply	  - Player the owner is talking to
				data - Local storage table
			--]]-----------------------------------------------
			
			JKG.ArenaBackend.CheckTeam(1, 2)
		end,
		HasCondition = false,
	},
	T152 = {
		Type = 2,
		SubNode = "L153",
		Text = "All invalid players have been removed from the team",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	L153 = {
		Type = 4,
		Target = "T133",
	},
	O136 = {
		Type = 3,
		SubNode = "L137",
		Text = "Go back",
		HasCondition = false,
		HasResolver = false,
	},
	L137 = {
		Type = 4,
		Target = "T121",
	},
	O124 = {
		Type = 3,
		SubNode = "L125",
		Text = "Go back",
		HasCondition = false,
		HasResolver = false,
	},
	L125 = {
		Type = 4,
		Target = "T5",
	},
	O8 = {
		Type = 3,
		SubNode = "T20",
		NextNode = "O9",
		Text = "Arena controls",
		HasCondition = false,
		HasResolver = false,
	},
	T20 = {
		Type = 2,
		SubNode = "O60",
		Text = "Please choose your action",
		Duration = 1000,
		HasCondition = false,
		HasResolver = false,
	},
	O60 = {
		Type = 3,
		SubNode = "S204",
		NextNode = "O24",
		Text = "Start the match",
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			return not JKG.ArenaBackend.GetInUse(1) and JKG.ArenaBackend.GetPending(1)
		end,
		HasResolver = false,
	},
	S204 = {
		Type = 6,
		SubNode = "T205",
		ScriptFunc = function(owner, ply, data)
			--[[-----------------------------------------------
				Dialogue Script
			
				Available Variables:
				owner - Entity that runs this conversation
				ply	  - Player the owner is talking to
				data - Local storage table
			--]]-----------------------------------------------
			
			local ok, err = JKG.ArenaBackend.FinalizeMatch(1)
			if not ok then
			    data.StartOk, data.StartError = ok, err
			    return
			end
			data.StartOk, data.StartError = JKG.ArenaBackend.StartMatch(1)
		end,
		HasCondition = false,
	},
	T205 = {
		Type = 2,
		SubNode = "L206",
		NextNode = "T207",
		Text = "Failed to start the match.\nReason: $error$",
		Duration = 3000,
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			--[[-----------------------------------------------
				Conditional Script
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				data - Local storage table
			
				Wanted return value: Bool (true/false)
			--]]-----------------------------------------------
			
			return not data.StartOk
		end,
		HasResolver = true,
		ResolveFunc = function(owner, ply, var, data)
			--[[-----------------------------------------------
				Resolver Script
			
				This allows the use of variables in the text
				(single word enclosed in $ signs)
				For example: 'Hello there $name$, how are you.',
				where name is the variable here.
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				var - Variable to resolve (string)
				data - Local storage table
			
				Wanted return value: The text to use in place of
				the variable
			--]]-----------------------------------------------
			
			if var == "error" then
			    return data.StartError
			end
			
			return ""
		end,
	},
	L206 = {
		Type = 4,
		Target = "T5",
	},
	T207 = {
		Type = 2,
		SubNode = "L208",
		Text = "Match started successfully",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	L208 = {
		Type = 4,
		Target = "T5",
	},
	O24 = {
		Type = 3,
		SubNode = "T180",
		NextNode = "O21",
		Text = "Abort the current match",
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			return JKG.ArenaBackend.GetInUse(1)
		end,
		HasResolver = false,
	},
	T180 = {
		Type = 2,
		SubNode = "L181",
		NextNode = "T68",
		Text = "The match has already been aborted.\nPlease wait a few seconds for the arena to reset.",
		Duration = 3000,
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			return JKG.ArenaBackend.GetStage(1) == 4
		end,
		HasResolver = false,
	},
	L181 = {
		Type = 4,
		Target = "T5",
	},
	T68 = {
		Type = 2,
		SubNode = "O69",
		Text = "Are you sure you want to abort the current match?",
		Duration = 2000,
		HasCondition = false,
		HasResolver = false,
	},
	O69 = {
		Type = 3,
		SubNode = "S154",
		NextNode = "O70",
		Text = "Yes",
		HasCondition = false,
		HasResolver = false,
	},
	S154 = {
		Type = 6,
		SubNode = "T155",
		ScriptFunc = function(owner, ply, data)
			--[[-----------------------------------------------
				Dialogue Script
			
				Available Variables:
				owner - Entity that runs this conversation
				ply	  - Player the owner is talking to
				data - Local storage table
			--]]-----------------------------------------------
			
			JKG.ArenaBackend.GameFinished(1, 3)
		end,
		HasCondition = false,
	},
	T155 = {
		Type = 2,
		SubNode = "L156",
		Text = "The match has been aborted",
		Duration = 2000,
		HasCondition = false,
		HasResolver = false,
	},
	L156 = {
		Type = 4,
		Target = "T5",
	},
	O70 = {
		Type = 3,
		SubNode = "L71",
		Text = "No",
		HasCondition = false,
		HasResolver = false,
	},
	L71 = {
		Type = 4,
		Target = "T20",
	},
	O21 = {
		Type = 3,
		SubNode = "L22",
		Text = "Go back",
		HasCondition = false,
		HasResolver = false,
	},
	L22 = {
		Type = 4,
		Target = "T5",
	},
	O9 = {
		Type = 3,
		SubNode = "T25",
		NextNode = "O10",
		Text = "Display controls",
		HasCondition = false,
		HasResolver = false,
	},
	T25 = {
		Type = 2,
		SubNode = "L64",
		NextNode = "T67",
		Text = "You cannot override the displays while the arena is in use!",
		Duration = 3000,
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			--[[-----------------------------------------------
				Conditional Script
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				data - Local storage table
			
				Wanted return value: Bool (true/false)
			--]]-----------------------------------------------
			
			return JKG.ArenaBackend.GetInUse(1)
		end,
		HasResolver = false,
	},
	L64 = {
		Type = 4,
		Target = "T5",
	},
	T67 = {
		Type = 2,
		SubNode = "O73",
		Text = "Please choose the desired action",
		Duration = 2000,
		HasCondition = false,
		HasResolver = false,
	},
	O73 = {
		Type = 3,
		SubNode = "S77",
		NextNode = "O72",
		Text = "Initialize display",
		HasCondition = false,
		HasResolver = false,
	},
	S77 = {
		Type = 6,
		SubNode = "T78",
		ScriptFunc = function(owner, ply, data)
			--[[-----------------------------------------------
				Dialogue Script
			
				Available Variables:
				owner - Entity that runs this conversation
				ply	  - Player the owner is talking to
				data - Local storage table
			--]]-----------------------------------------------
			
			JKG.ArenaBackend.Display.InitDisplay(1)
		end,
		HasCondition = false,
	},
	T78 = {
		Type = 2,
		SubNode = "L79",
		Text = "Display has been initialized",
		Duration = 2000,
		HasCondition = false,
		HasResolver = false,
	},
	L79 = {
		Type = 4,
		Target = "T67",
	},
	O72 = {
		Type = 3,
		SubNode = "S80",
		NextNode = "O74",
		Text = "Disable display",
		HasCondition = false,
		HasResolver = false,
	},
	S80 = {
		Type = 6,
		SubNode = "L81",
		ScriptFunc = function(owner, ply, data)
			--[[-----------------------------------------------
				Dialogue Script
			
				Available Variables:
				owner - Entity that runs this conversation
				ply	  - Player the owner is talking to
				data - Local storage table
			--]]-----------------------------------------------
			
			JKG.ArenaBackend.Display.ClearDisplay(1)
		end,
		HasCondition = false,
	},
	L81 = {
		Type = 4,
		Target = "T67",
	},
	O74 = {
		Type = 3,
		SubNode = "T82",
		NextNode = "O75",
		Text = "Test countdown",
		HasCondition = false,
		HasResolver = false,
	},
	T82 = {
		Type = 2,
		SubNode = "N83",
		Text = "Specify the start time (in seconds)",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	N83 = {
		Type = 8,
		SubNode = "S84",
		Caption = "Start time (seconds)",
		DefVal = "300",
		Flags = 3,
		ScriptFunc = function(owner, ply, response, data)
			--[[-----------------------------------------------
				Text Entry Processing Script
			
				The response to a text entry node can be processed here.
				Use the local storage table to save information and
				adjust the dialogue flow (conditional scripts)
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				response - The response the player gave, nil if cancel was clicked
				data - Local storage table
			
			--]]-----------------------------------------------
			
			if not response then
			    data.Cancel = true
			else
			    data.Cancel = false
			    data.StartTime = tonumber(response)
			    if data.StartTime > 5999 then 
			        data.StartTime = 5999
			    end
			end
		end,
		HasCondition = false,
		HasResolver = false,
	},
	S84 = {
		Type = 6,
		SubNode = "T85",
		NextNode = "L87",
		ScriptFunc = function(owner, ply, data)
			--[[-----------------------------------------------
				Dialogue Script
			
				Available Variables:
				owner - Entity that runs this conversation
				ply	  - Player the owner is talking to
				data - Local storage table
			--]]-----------------------------------------------
			
			JKG.ArenaBackend.Display.TestCountdown(1, data.StartTime)
		end,
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			--[[-----------------------------------------------
				Conditional Script
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				data - Local storage table
			
				Wanted return value: Bool (true/false)
			--]]-----------------------------------------------
			
			return not data.Cancel
		end,
	},
	T85 = {
		Type = 2,
		SubNode = "L86",
		Text = "Countdown test started...",
		Duration = 2000,
		HasCondition = false,
		HasResolver = false,
	},
	L86 = {
		Type = 4,
		Target = "T67",
	},
	L87 = {
		Type = 4,
		Target = "T67",
	},
	O75 = {
		Type = 3,
		SubNode = "L76",
		Text = "Go back",
		HasCondition = false,
		HasResolver = false,
	},
	L76 = {
		Type = 4,
		Target = "T5",
	},
	O10 = {
		Type = 3,
		SubNode = "D11",
		Text = "Log off",
		HasCondition = false,
		HasResolver = false,
	},
	D11 = {
		Type = 5,
	},
	E2 = {
		Type = 1,
		SubNode = "T3",
		HasCondition = false,
	},
	T3 = {
		Type = 2,
		SubNode = "D4",
		Text = "This terminal is restricted to GM's.\nAccess Denied.",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	D4 = {
		Type = 5,
	},
}

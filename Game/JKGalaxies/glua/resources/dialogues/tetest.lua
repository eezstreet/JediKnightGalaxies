-- Exported by JKG Dialogue Creator

DLG.Name = "tetest"
DLG.RootNode = "E1"
DLG.Nodes = {
	E1 = {
		Type = 1,
		SubNode = "T2",
		HasCondition = false,
	},
	T2 = {
		Type = 2,
		SubNode = "T3",
		Text = "This is a test dialogue to test out the text entry node",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	T3 = {
		Type = 2,
		SubNode = "O4",
		Text = "Would you like to continue?",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	O4 = {
		Type = 3,
		SubNode = "T7",
		NextNode = "O5",
		Text = "Yes, continue",
		HasCondition = false,
		HasResolver = false,
	},
	T7 = {
		Type = 2,
		SubNode = "N8",
		Text = "Please enter any text in the text field",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	N8 = {
		Type = 8,
		SubNode = "T9",
		Caption = "Type something",
		DefVal = "",
		Flags = 0,
		ScriptFunc = function(owner, ply, response, data)
			data.EnteredText = response
		end,
		HasCondition = false,
		HasResolver = false,
	},
	T9 = {
		Type = 2,
		SubNode = "T10",
		Text = "You typed in '$text$'.",
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
			
			if var == "text" then
			    return data.EnteredText
			end
			
			return ""
		end,
	},
	T10 = {
		Type = 2,
		SubNode = "N11",
		Text = "Now, please type a number",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	N11 = {
		Type = 8,
		SubNode = "T12",
		Caption = "Type a number",
		DefVal = "",
		Flags = 14,
		ScriptFunc = function(owner, ply, response, data)
			data.Number = response
		end,
		HasCondition = false,
		HasResolver = false,
	},
	T12 = {
		Type = 2,
		SubNode = "T13",
		Text = "You typed $test$",
		Duration = 3000,
		HasCondition = false,
		HasResolver = true,
		ResolveFunc = function(owner, ply, var, data)
			if var == "test" then
			    return data.Number
			end
		end,
	},
	T13 = {
		Type = 2,
		SubNode = "T14",
		Text = "Password entry test",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	T14 = {
		Type = 2,
		SubNode = "N15",
		Text = "Please type the password 'test'",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	N15 = {
		Type = 8,
		SubNode = "T16",
		Caption = "Please enter the password",
		DefVal = "",
		Flags = 17,
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
			
			if response == nil then
			    data.NextNode = 0
			elseif response == "test" then
			    data.NextNode = 1
			else
			    data.nextNode = 2
			end
		end,
		HasCondition = false,
		HasResolver = false,
	},
	T16 = {
		Type = 2,
		SubNode = "T19",
		NextNode = "T17",
		Text = "You have clicked on cancel",
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
			
			return data.NextNode == 0
		end,
		HasResolver = false,
	},
	T19 = {
		Type = 2,
		SubNode = "D20",
		Text = "This concludes the text entry test",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	D20 = {
		Type = 5,
	},
	T17 = {
		Type = 2,
		SubNode = "L21",
		NextNode = "T18",
		Text = "You entered the right password!",
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
			
			return data.NextNode == 1
		end,
		HasResolver = false,
	},
	L21 = {
		Type = 4,
		Target = "T19",
	},
	T18 = {
		Type = 2,
		SubNode = "L22",
		Text = "You have entered the wrong password!",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	L22 = {
		Type = 4,
		Target = "T19",
	},
	O5 = {
		Type = 3,
		SubNode = "D6",
		Text = "No thanks",
		HasCondition = false,
		HasResolver = false,
	},
	D6 = {
		Type = 5,
	},
}

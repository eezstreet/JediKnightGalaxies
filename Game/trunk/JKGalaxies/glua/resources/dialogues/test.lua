-- Dialogue template

DLG.Name = "test"

DLG.RootNode = "E1"
DLG.Nodes = {
	E1 = {	
		Type = 1,
		SubNode = "T3",
		NextNode = nil,
		HasCondition = false,
	},
	T3 = {	
		Type = 2,
		SubNode = "O4",
		Text = "Hello there $name$",
		Duration = 1000,
		HasCondition = false,
		HasResolver = true,
		ResolveFunc = function(owner, ply, var)
			if var == "name" then
				return sys.StripColorcodes(ply:GetName())
			end
		end,
	},
	O4 = {	
		Type = 3,
		SubNode = "T10",
		NextNode = "O5",
		Text = "WIN!",
		HasCondition = false,
		HasResolver = false,
	},
	O5 = {	
		Type = 3,
		SubNode = "T11",
		NextNode = "O6",
		Text = "o.o",
		HasCondition = false,
		HasResolver = false,
	},
	O6 = {	
		Type = 3,
		SubNode = "T12",
		NextNode = "O7",
		Text = "Failure!",
		HasCondition = false,
		HasResolver = false,
	},
	O7 = {	
		Type = 3,
		SubNode = "W8",
		NextNode = nil,
		Text = "Pazaaaak!!!",
		HasCondition = false,
		HasResolver = false,
	},
	W8 = {	
		Type = 7,
		SubNode = "T9",
		NextNode = nil,
		HasCondition = false,
		ScriptFunc = function(owner, ply, resumefunc)
			local function pzkFinish(winner)
				resumefunc()
			end
			
			local pzk = sys.CreatePazaakGame()
			local cards = JKG.Pazaak.Cards
			pzk:SetPlayers(ply, nil)
			if not opp then
				pzk:SetAILevel(1)
				pzk:SetAIName("Tournament Bot")
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

			pzk:SetSideDeck(2, {cards.PZCARD_FLIP_1,
							cards.PZCARD_FLIP_2,
							cards.PZCARD_FLIP_3,
							cards.PZCARD_FLIP_4,
							cards.PZCARD_FLIP_5,
							cards.PZCARD_FLIP_6,
							cards.PZCARD_FLIP_2,
							cards.PZCARD_FLIP_3,
							cards.PZCARD_FLIP_4,
							cards.PZCARD_FLIP_5	
							} )
							
			pzk:ShowCardSelection(true)
			
			timer.Simple(50, function()
				local success, reason = pzk:StartGame()
				if not success then
					resumefunc()
				end
			end)
			return true
		end,
	},
	T9 = {	
		Type = 2,
		SubNode = "L16",
		Text = "Good game!",
		Duration = 2000,
		HasCondition = false,
		HasResolver = false,
	},
	T10 = {	
		Type = 2,
		SubNode = "D13",
		Text = "You picked 'WIN!'",
		Duration = 2000,
		HasCondition = false,
		HasResolver = false,
	},
	T11 = {	
		Type = 2,
		SubNode = "D14",
		Text = "You picked 'o.o'",
		Duration = 2000,
		HasCondition = false,
		HasResolver = false,
	},
	T12 = {	
		Type = 2,
		SubNode = "D15",
		Text = "You picked 'Failure!'",
		Duration = 2000,
		HasCondition = false,
		HasResolver = false,
	},
	D13 = {
		Type = 5,
	},
	D14 = {
		Type = 5,
	},
	D15 = {
		Type = 5,
	},
	L16 = {
		Type = 4,
		Target = "O4",
		NextNode = "L17",
	},
	L17 = {
		Type = 4,
		Target = "O5",
		NextNode = "L18",
	},
	L18 = {
		Type = 4,
		Target = "O6",
		NextNode = "L19",
	},
	L19 = {
		Type = 4,
		Target = "O7",
		NextNode = nil,
	},
}
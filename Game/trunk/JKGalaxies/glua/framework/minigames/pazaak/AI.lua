--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Code
	Pazaak (Artificial Intelligence code)
	
	Written by BobaFett
--------------------------------------------------]]

--[[----------------------------------------------------------------------------------------
	JKG.Pazaak.AI.Normal() 
	
	Normal difficulty AI routine
	
	Pretty much a replica of the Kotor AI
	
	WARNING: This AI is not yet capable of using special cards!
	Only use Plus, Minus and Flip cards!
-------------------------------------------------------------------------------------------]]
JKG.Pazaak.AI = {}

function JKG.Pazaak.AI.Normal(PazaakGame)
	local self = PazaakGame.Players[2]	-- AI is always player two
	local opponent = PazaakGame.Players[1] -- And the player always player 1 ^_^
	local cards = JKG.Pazaak.Cards -- Easier/faster access to the card ID list
	
	local HandCard = 0		-- Card to draw (0 = nothing)
	local CardParam = 0		-- Param for the card to draw
	local Stand = false		-- Stand (after using the hand card)?
	
	local LowestPossibleTotal = self.Points -- Establish the lowest total we can currently get using our hand cards
	local LPTCard = 0						-- Card that will get us the lowest possible total
	local CurrentPossibleTotal = self.Points -- Establish the closest we can get to 20 using the current hand cards
	local CPTCard = 0						-- Card that will get us the current possible total
	local i
	
	-- First thing we do, is run through all our cards and calculate the lowest and current possible total
	local temp
	for i = 1,4 do
		temp = self.Points + cards.MinValue(self.Hand[i])
		if temp < LowestPossibleTotal then
			LowestPossibleTotal = temp
			LPTCard = i
		end
		temp = self.Points + cards.MaxValue(self.Hand[i])
		if temp <= 20 and temp > CurrentPossibleTotal then
			CurrentPossibleTotal = temp
			CPTCard = i
		end
	end
	
	-- Now check the circumstances
	if CurrentPossibleTotal > 20 then
		-- We're bust, see if our lowest possible total is under 21
		if LowestPossibleTotal < 21 then
			-- We can recover, run through our cards again and recalculate CurrentPossibleTotal, but now with negative cards
			local CurrentPossibleTotal = 0
			for i = 1,4 do
				temp = self.Points + cards.MinValue(self.Hand[i])
				if temp <= 20 and temp > CurrentPossibleTotal then
					CurrentPossibleTotal = temp
					CPTCard = i
				end
			end
			if opponent.Points > 17 then
				-- Only play the recovery card if this can give us an advantage
				if opponent.Score == 2 then
					-- If we dont, the player will win, so do it
					HandCard = CPTCard
					temp = self.Hand[HandCard]
					if temp >= cards.PZCARD_FLIP_1 and temp <= cards.PZCARD_FLIP_6 then
						CardParam = 1
					else
						CardParam = 0
					end
				elseif CurrentPossibleTotal >= opponent.Points then
					HandCard = CPTCard
					temp = self.Hand[HandCard]
					if temp >= cards.PZCARD_FLIP_1 and temp <= cards.PZCARD_FLIP_6 then
						CardParam = 1
					else
						CardParam = 0
					end
				end
			else
				HandCard = CPTCard
				temp = self.Hand[HandCard]
				if temp >= cards.PZCARD_FLIP_1 and temp <= cards.PZCARD_FLIP_6 then
					CardParam = 1
				else
					CardParam = 0
				end
			end
		end
	elseif CurrentPossibleTotal >= 18 then
		-- Check if we should use this card
		if CurrentPossibleTotal == 20 then
			-- Use it!
			HandCard = CPTCard
			CardParam = 0
		elseif CurrentPossibleTotal == 19 then
			if opponent.Points == 20 then
				-- Dont bother
				HandCard = 0
			elseif LowestPossibleTotal <= 11 then
				HandCard = CPTCard
				CardParam = 0
			end
		elseif CurrentPossibleTotal == 18 then
			if opponent.Points > 18 then
				HandCard = 0
			elseif LowestPossibleTotal <= 13 then
				HandCard = CPTCard
				CardParam = 0
			end
		end		
	end
	
	if HandCard == 0 then
		-- We are not using a card, so set CurrentPossibleTotal back to self.Points
		CurrentPossibleTotal = self.Points
	end
		
	-- Now that we determined whether we'll use the card or not, determine if we're going to stand or not
	if CurrentPossibleTotal == 20 then
		Stand = true
	elseif opponent.Points > CurrentPossibleTotal then
		Stand = false -- Never stand if a player has a higher score
	elseif CurrentPossibleTotal > 17 and opponent.Points < CurrentPossibleTotal then
		Stand = true
	end
	
	return HandCard, CardParam, Stand	
end

--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Code
	Pazaak (Utility funtions)
	
	Written by BobaFett
--------------------------------------------------]]

--[[----------------------------------------------------------------------------------------
	array JKG.Pazaak.GenerateDeck()
	
	Creates a freshly shuffled 40-card pazaak deck
-------------------------------------------------------------------------------------------]]
function JKG.Pazaak.GenerateDeck()
	local cardstoshuffle = 40				-- Amount of cards we still need to add
	local cardlist = {4,4,4,4,4,4,4,4,4,4}	-- Amount of each number available (4 of each at the start)
	local cards = {1,2,3,4,5,6,7,8,9,10}	-- Numbers still present in remaining cards (1-10 at start)
	local carddeck = {}						-- Eventual deck, starts empty of course
	
	while cardstoshuffle ~= 0 do			-- Repeat till all cards are in
		local cardid = math.random(1, #cards)	-- Pick a random number from the available ones to add
		local cardnum = cards[cardid]			-- Get the actual card number
		table.insert(carddeck, cardnum)			-- Add this to our deck
		cardlist[cardnum] = cardlist[cardnum] - 1	-- Subtract it from the remaining cards
		if cardlist[cardnum] == 0 then			-- If we took the last card of that number...
			table.remove(cards, cardid)			-- Remove it from our number list
		end
		cardstoshuffle = cardstoshuffle - 1		-- Added one card, so lower the remaining card count
	end
	return carddeck							-- Return the deck we just made
end

--[[----------------------------------------------------------------------------------------
	array JKG.Pazaak.GenerateHand()
	
	Gets 4 cards for the hand based on a 10-card sidedeck
-------------------------------------------------------------------------------------------]]
function JKG.Pazaak.GenerateHand(sidedeck)
	-- Draws 4 random cards from the side-deck and puts them in the hand
	local cards = {}
	local hand = {}		-- our hand cards
	local cardsremaining = 4
	local k,v
	-- First make a clone of the sidedeck, since we'll be removing entries from here
	for k,v in pairs(sidedeck) do
		cards[k] = v
	end
	
	-- Lets create our hand
	while cardsremaining ~= 0 do				-- Repeat till all cards are in
		local cardid = math.random(1, #cards)	-- Pick a random card from the available ones
		local cardnum = cards[cardid]			-- Get the actual card number
		table.insert(hand, cardnum)			-- Add this to our hand
		table.remove(cards, cardid)			-- Remove it from our remaining cards
		cardsremaining = cardsremaining - 1		-- Added one card, so lower the remaining card count
	end
	return hand
end

--[[----------------------------------------------------------------------------------------
	array JKG.Pazaak.DefaultCardDeck()
	
	Returns a default card deck
-------------------------------------------------------------------------------------------]]
function JKG.Pazaak.DefaultCardDeck()
		-- Order of side deck cards:
		-- 6x plus cards	(1 of each default)
		-- 6x minus cards	(1 of each default)
		-- 6x flip cards
		-- Tie breaker
		-- Double
		-- +/- 1/2
		-- 2&4
		-- 3&6
		return {
				1, 1, 1, 1, 1, 1,
				1, 1, 1, 1, 1, 1,
				0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0
			   }
end

--[[----------------------------------------------------------------------------------------
	array JKG.Pazaak.DefaultSideDeck()
	
	Returns a default side deck
-------------------------------------------------------------------------------------------]]
function JKG.Pazaak.DefaultCardDeck()
		-- 6x plus cards	(1 of each)
		-- 6x minus cards	(1 to 4)
		local cards = JKG.Pazaak.Cards
		return {
			cards.PZCARD_PLUS_1,
			cards.PZCARD_PLUS_2,
			cards.PZCARD_PLUS_3,
			cards.PZCARD_PLUS_4,
			cards.PZCARD_PLUS_5,
			cards.PZCARD_PLUS_6,
			cards.PZCARD_MINUS_1,
			cards.PZCARD_MINUS_2,
			cards.PZCARD_MINUS_3,
			cards.PZCARD_MINUS_4
			}
end

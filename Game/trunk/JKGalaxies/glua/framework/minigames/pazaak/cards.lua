--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Code
	Pazaak (Card IDs)
	
	Written by BobaFett
--------------------------------------------------]]
JKG.Pazaak.Cards = {
-- ID For face-down cards (opponent hand)
PZCARD_BACK = -2,
PZCARD_NONE = -1,

-- ID's for normal (green) cards (params: 1 = flipped)
PZCARD_NORMAL_1 = 0,
PZCARD_NORMAL_2 = 1,
PZCARD_NORMAL_3 = 2,
PZCARD_NORMAL_4 = 3,
PZCARD_NORMAL_5 = 4,
PZCARD_NORMAL_6 = 5,
PZCARD_NORMAL_7 = 6,
PZCARD_NORMAL_8 = 7,
PZCARD_NORMAL_9 = 8,
PZCARD_NORMAL_10 = 9,

-- ID's for + cards  (params: 1 = flipped)
PZCARD_PLUS_1 = 10,
PZCARD_PLUS_2 = 11,
PZCARD_PLUS_3 = 12,
PZCARD_PLUS_4 = 13,
PZCARD_PLUS_5 = 14,
PZCARD_PLUS_6 = 15,

-- ID's for - cards  (params: 1 = flipped)
PZCARD_MINUS_1 = 16,
PZCARD_MINUS_2 = 17,
PZCARD_MINUS_3 = 18,
PZCARD_MINUS_4 = 19,
PZCARD_MINUS_5 = 20,
PZCARD_MINUS_6 = 21,

-- ID's for +/- cards
-- Params: 0 = Positive state, 1 = Negative state
-- Params: 2 = Positive state, flipped, 3 = Negative state, flipped
PZCARD_FLIP_1 = 22,
PZCARD_FLIP_2 = 23,
PZCARD_FLIP_3 = 24,
PZCARD_FLIP_4 = 25,
PZCARD_FLIP_5 = 26,
PZCARD_FLIP_6 = 27,

-- ID's for special (gold) cards
-- Params: 0 = +1T, 1 = -1T
PZCARD_TIEBREAKER = 28,
-- Params: 0 = D, anything else = Value of card on the field (1 to 10)
PZCARD_DOUBLE = 29,
-- Params: 0 = +1, 1 = -1, 2 = +2, 3 = -2
PZCARD_FLIP12 = 30,
-- Params: 1 = on the field (show 0)
PZCARD_2N4 = 31,
PZCARD_3N6 = 32,
}

local PZCARD_BACK = -2
local PZCARD_NONE = -1
local PZCARD_NORMAL_1 = 0
local PZCARD_NORMAL_2 = 1
local PZCARD_NORMAL_3 = 2
local PZCARD_NORMAL_4 = 3
local PZCARD_NORMAL_5 = 4
local PZCARD_NORMAL_6 = 5
local PZCARD_NORMAL_7 = 6
local PZCARD_NORMAL_8 = 7
local PZCARD_NORMAL_9 = 8
local PZCARD_NORMAL_10 = 9
local PZCARD_PLUS_1 = 10
local PZCARD_PLUS_2 = 11
local PZCARD_PLUS_3 = 12
local PZCARD_PLUS_4 = 13
local PZCARD_PLUS_5 = 14
local PZCARD_PLUS_6 = 15
local PZCARD_MINUS_1 = 16
local PZCARD_MINUS_2 = 17
local PZCARD_MINUS_3 = 18
local PZCARD_MINUS_4 = 19
local PZCARD_MINUS_5 = 20
local PZCARD_MINUS_6 = 21
local PZCARD_FLIP_1 = 22
local PZCARD_FLIP_2 = 23
local PZCARD_FLIP_3 = 24
local PZCARD_FLIP_4 = 25
local PZCARD_FLIP_5 = 26
local PZCARD_FLIP_6 = 27
local PZCARD_TIEBREAKER = 28
local PZCARD_DOUBLE = 29
local PZCARD_FLIP12 = 30
local PZCARD_2N4 = 31
local PZCARD_3N6 = 32

--[[----------------------------------------------------------------------------------------
	int JKG.Pazaak.Cards.CardValue(ID, param)
	
	Obtains the value of the specified card
	A return value of 0 means the card has no direct value 
	(eg. backsides, double cards, or 3&6/2&4 cards)
-------------------------------------------------------------------------------------------]]
function JKG.Pazaak.Cards.CardValue(ID, param)
	if ID == PZCARD_BACK then
		return 0		-- 0 is invalid, so use it as error code (used for cards with no direct value)
	elseif ID >= PZCARD_NORMAL_1 and ID <= PZCARD_NORMAL_10 then
		if param == 0 then
			return (ID - PZCARD_NORMAL_1) + 1
		else
			return ((ID - PZCARD_NORMAL_1) + 1) * -1	-- Flipped by a 2&4 or 3&6 card
		end
	elseif ID >= PZCARD_PLUS_1 and ID <= PZCARD_PLUS_6 then
		if param == 0 then
			return (ID - PZCARD_PLUS_1) + 1
		else 
			return ((ID - PZCARD_PLUS_1) + 1) * -1
		end
	elseif ID >= PZCARD_MINUS_1 and ID <= PZCARD_MINUS_6 then
		if param == 0 then
			return ((ID - PZCARD_MINUS_1) + 1) * -1
			
		else 
			return (ID - PZCARD_MINUS_1) + 1
		end
	elseif ID >= PZCARD_FLIP_1 and ID <= PZCARD_FLIP_6 then
		if param == 1 or param == 2 then
			return ((ID - PZCARD_FLIP_1) + 1) * -1
		else 
			return (ID - PZCARD_FLIP_1) + 1
		end
	elseif ID == PZCARD_TIEBREAKER then
		if param == 0 then
			return 1
		elseif param == 1 then
			return -1
		end
	elseif ID == PZCARD_DOUBLE then
		return param	-- Special case
	elseif ID == PZCARD_FLIP12 then
		if param == 0 then
			return 1
		elseif param == 1 then
			return -1
		elseif param == 2 then
			return 2
		elseif param == 3 then
			return -2
		end
		return 0
	elseif ID == PZCARD_3N6 or ID == PZCARD_2N4 then
		return 0	-- Special case
	end
	return 0
end

--[[----------------------------------------------------------------------------------------
	bool JKG.Pazaak.Cards.CanFlip(ID)
	
	Returns whether or not the current card can be sign flipped
-------------------------------------------------------------------------------------------]]
function JKG.Pazaak.Cards.CanFlip(ID)
	if ID >= PZCARD_NORMAL_1 and ID <= PZCARD_NORMAL_10 then
		return true
	elseif ID >= PZCARD_FLIP_1 and ID <= PZCARD_FLIP_6 then
		return true
	elseif ID == PZCARD_TIEBREAKER or ID == PZCARD_FLIP12 then
		return true
	else
		return false
	end
end

--[[----------------------------------------------------------------------------------------
	bool JKG.Pazaak.Cards.CanFlipValue(ID)
	
	Returns whether or not the current card can be value flipped
-------------------------------------------------------------------------------------------]]
function JKG.Pazaak.Cards.CanFlipValue(ID)
	if ID == PZCARD_FLIP12 then
		return true
	else
		return false
	end
end

--[[----------------------------------------------------------------------------------------
	int JKG.Pazaak.Cards.MinValue(ID)
	
	Returns the lowest value the card can have (in case it's flippable)
-------------------------------------------------------------------------------------------]]
function JKG.Pazaak.Cards.MinValue(ID)
	if ID == PZCARD_BACK then
		return 0
	elseif ID >= PZCARD_NORMAL_1 and ID <= PZCARD_NORMAL_10 then
		return (ID - PZCARD_NORMAL_1) + 1
	elseif ID >= PZCARD_PLUS_1 and ID <= PZCARD_PLUS_6 then
		return (ID - PZCARD_PLUS_1) + 1
	elseif ID >= PZCARD_MINUS_1 and ID <= PZCARD_MINUS_6 then
		return ((ID - PZCARD_MINUS_1) + 1) * -1
	elseif ID >= PZCARD_FLIP_1 and ID <= PZCARD_FLIP_6 then
		return ((ID - PZCARD_FLIP_1) + 1) * -1
	elseif ID == PZCARD_TIEBREAKER then
		return -1
	elseif ID == PZCARD_DOUBLE then
		return 0	-- Special case
	elseif ID == PZCARD_FLIP12 then
		return -2
	elseif ID == PZCARD_3N6 or ID == PZCARD_2N4 then
		return 0	-- Special case
	end
	return 0
end

--[[----------------------------------------------------------------------------------------
	int JKG.Pazaak.Cards.MaxValue(ID)
	
	Returns the highest value the card can have (in case it's flippable)
-------------------------------------------------------------------------------------------]]
function JKG.Pazaak.Cards.MaxValue(ID)
	if ID == PZCARD_BACK then
		return 0
	elseif ID >= PZCARD_NORMAL_1 and ID <= PZCARD_NORMAL_10 then
		return (ID - PZCARD_NORMAL_1) + 1
	elseif ID >= PZCARD_PLUS_1 and ID <= PZCARD_PLUS_6 then
		return (ID - PZCARD_PLUS_1) + 1
	elseif ID >= PZCARD_MINUS_1 and ID <= PZCARD_MINUS_6 then
		return ((ID - PZCARD_MINUS_1) + 1) * -1
	elseif ID >= PZCARD_FLIP_1 and ID <= PZCARD_FLIP_6 then
		return (ID - PZCARD_FLIP_1) + 1
	elseif ID == PZCARD_TIEBREAKER then
		return 1
	elseif ID == PZCARD_DOUBLE then
		return 0	-- Special case
	elseif ID == PZCARD_FLIP12 then
		return 2
	elseif ID == PZCARD_3N6 or ID == PZCARD_2N4 then
		return 0	-- Special case
	end
	return 0
end

--[[----------------------------------------------------------------------------------------
	int JKG.Pazaak.Cards.FlipCard(ID, param)
	
	Flips the card's sign and returns its new param
-------------------------------------------------------------------------------------------]]
function JKG.Pazaak.Cards.FlipCard(ID, param)
	if ID >= PZCARD_FLIP_1 and ID <= PZCARD_FLIP_6 then
		if param == 1 then
			return 0
		else
			return 1
		end
	elseif ID == PZCARD_TIEBREAKER then
		if param == 1 then
			return 0
		else
			return 1
		end
	elseif ID == PZCARD_FLIP12 then
		if bitwise.And(param, 1) then	-- 2 bits: lowest bit is sign, highest bit is value
			return bitwise.And(param, 2)
		else
			return bitwise.Or(param, 1)
		end
	end
	return param
end

--[[----------------------------------------------------------------------------------------
	int JKG.Pazaak.Cards.FlipCard(ID, param)
	
	Flips the card's value and returns its new param
-------------------------------------------------------------------------------------------]]
function JKG.Pazaak.Cards.FlipCardValue(ID, param)
	if ID == PZCARD_FLIP12 then
		if bitwise.And(param, 2) then	-- 2 bits: lowest bit is sign, highest bit is value
			return bitwise.And(param, 1)
		else
			return bitwise.Or(param, 2)
		end
	end
	return param
end

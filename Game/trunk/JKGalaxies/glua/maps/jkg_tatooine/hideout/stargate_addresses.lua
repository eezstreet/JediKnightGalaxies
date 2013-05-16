--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Code
	Stargate address table - Tatooine GM Hideout
	
	Written by BobaFett
--------------------------------------------------]]

local AddressTable = JKG.GMH.StarGate.AddressTable

--[[ -------
--|| Entries:
--||
--|| Type: Address type
--|| 
--|| 1 - Local teleport
--||     Target should be a table: { position, angles }
--||
--|| 2 - Remote teleport (server redirect)
--||     Target should be the server to connect to
--||
--||
--|| Description: Description (as to be shown in the DHD
--]] -------

-- Interserver addresses
include("/glua/shared/gmh/stargate_addresses.lua")

-- Intraserver addresses
AddressTable["@homes#"] = { Type = 1, Description = "Homestead", Target = {Vector(921, -27930, 51), Vector(0, 150, 0)} }
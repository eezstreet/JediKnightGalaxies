--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Code
	Stargate address table - Korriban GM Hideout
	
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
AddressTable["@spawn#"] = { Type = 1, Description = "Spawn area", Target = {Vector(2890, -1080, 388), Vector(0, 0, 0)} }
AddressTable["@couni#"] = { Type = 1, Description = "Council chamber", Target = {Vector(240, -1905, 68), Vector(0, -45, 0)} }
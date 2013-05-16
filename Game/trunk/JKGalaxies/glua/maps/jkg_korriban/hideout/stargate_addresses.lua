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
AddressTable["@spawn#"] = { Type = 1, Description = "Spawn area", Target = {Vector(-36075, 5245, 60), Vector(0, 0, 0)} }
AddressTable["@gmhos#"] = { Type = 1, Description = "Outside GM Vault", Target = {Vector(-15045, 4500, -690), Vector(0, 90, 0)} }
AddressTable["@siacd#"] = { Type = 1, Description = "Outside Sith Academy", Target = {Vector(-25530, 8280, -410), Vector(0, 0, 0)} }
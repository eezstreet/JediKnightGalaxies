--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Code
	Korriban GM Hideout init
	
	Written by BobaFett
--------------------------------------------------]]

-- Initialize the GM Hideout on korriban
JKG.GMH = {}

include("vaultdoor.lua")
include("stargate.lua")

local function SpawnVaultControls()
	local entfact = ents.CreateEntityFactory("jkg_vaultcontrol")
	entfact:SetSpawnVar("targetname", "vaultoutside")
	entfact:SetSpawnVar("outside", "1")
	entfact:Create()
	entfact:SetSpawnVar("targetname", "vaultinside")
	entfact:SetSpawnVar("outside", "0")
	entfact:Create()
end

hook.Add("MapLoaded", "GMH.SpawnVaultControls", SpawnVaultControls)
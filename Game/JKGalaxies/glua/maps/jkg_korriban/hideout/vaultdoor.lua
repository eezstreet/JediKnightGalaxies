--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Code
	Vault door controls - Korriban GM Hideout
	
	Written by BobaFett
--------------------------------------------------]]

JKG.GMH.VaultDoor = {}
local vault = JKG.GMH.VaultDoor

vault.Password = "lemmein"  -- Required to open the door from the outside
vault.GMNoPass = false		-- Whether GM's can open the vault without a password
vault.AllowNonGM = true		-- Whether non-GM's are allowed to open and close the door

local DoorInUse = false
local DoorOpen = false
local DoorEnt = nil

-- Local thread function to open the vault door
local function DoorOpenSequence()
	if not DoorEnt then 
		print("GMH-Vault: Attempted to open the vault door without valid door entity")
		return
	end
	DoorInUse = true
	-- Move the door back in 2.5 seconds using TR_EASEINOUT
	DoorEnt:Move( Vector( -15200, 3925, -648 ), 2500, 8 )
	thread.Wait(3000)
	-- Roll it aside in 4 seconds (using TR_EASEINOUT for both rotation and movement)
	DoorEnt:Move( Vector( -14926.68144, 3925, -648 ), 4000, 8)
	DoorEnt:Rotate( Vector( 180, 0, 0 ), 4000, 8)
	thread.Wait(4000)
	
	DoorOpen = true
	DoorInUse = false
end

-- Local thread function to close the vault door
local function DoorCloseSequence()
	if not DoorEnt then 
		print("GMH-Vault: Attempted to open the vault door without valid door entity")
		return
	end
	DoorInUse = true

	-- Roll it back in 4 seconds (using TR_EASEINOUT for both rotation and movement)
	DoorEnt:Move( Vector(  -15200, 3925, -648 ), 4000, 8)
	DoorEnt:Rotate( Vector( 0, 0, 0 ), 4000, 8)
	thread.Wait(4500)
	
	-- Move the door back in 2.5 seconds using TR_EASEINOUT
	DoorEnt:Move( Vector( -15200, 3998, -648 ), 2500, 8 )
	thread.Wait(2500)
	
	DoorOpen = false
	DoorInUse = false
end


local function InitVaultDoor()
	DoorEnt = ents.GetByName('vaulttecdoor')[1]
end

hook.Add("MapLoaded", "GMH.VaultDoor", InitVaultDoor)

--[[ -----
--|| Vault-Door Function
--|| Open ( )
--|| 
--|| Opens the GM Hideout's outer vault door
--]] -----
function vault.Open()
	if DoorInUse or DoorOpen then
		return false
	end
	thread.Create("gmhideoutvaultdoor", DoorOpenSequence)
	return true
end

--[[ -----
--|| Vault-Door Function
--|| Close ( )
--|| 
--|| Closes the GM Hideout's outer vault door
--]] -----
function vault.Close()
	if DoorInUse or not DoorOpen then
		return false
	end
	thread.Create("gmhideoutvaultdoor", DoorCloseSequence)
	return true
end

--[[ -----
--|| Vault-Door Function
--|| bool IsOpen ( )
--|| 
--|| Returns whether or not the vault door is open
--]] -----
function vault.IsOpen()
	return DoorOpen
end

--[[ -----
--|| Vault-Door Function
--|| bool IsInUse ( )
--|| 
--|| Returns whether or not the vault door is currently in use (in motion)
--]] -----
function vault.IsInUse()
	return DoorInUse
end


if RELOADING then
	InitVaultDoor()
end
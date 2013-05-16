--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Code
	Entity commands
	
	Written by BobaFett
--------------------------------------------------]]

local function ShowSpawnVars(ply, argc, argv)
	local ent
	if argc == 2 then
		ent = ents.GetByIndex(argv[1])
	else
		ent = ply:GetEyeTrace().Entity
	end
	if not ent or not ent:IsValid() then
		ply:SendPrint("No entity targetted")
		return
	end
	local SB = sys.CreateStringBuilder()
	SB:Append("Targetted: " .. tostring(ent) .. "\n\nSpawn vars:\n")
	if ent:HasSpawnVars() then
		local k,v
		for k,v in pairs(ent:GetSpawnVars()) do
			SB:Append(k .. " = " .. v .. "\n")
		end
	else
		SB:Append("^1Entity has no spawn vars\n")
	end
	ply:SendPrint(SB:ToString())
end

cmds.Add("showspawnvars", ShowSpawnVars)

local function BBTest(ply, argc, argv)
	local ent = ply:GetEyeTrace().Entity
	if not ent or not ent:IsValid() then
		ply:SendChat("^1No entity targetted")
	end
	ent:SetBoundingBox(Vector(-15, -85, -20), Vector(15, 85, 90))
	ply:SendChat("^5Bounding box set")
end

cmds.Add("bbtest", BBTest)

-- Replica of lugormod's delent~
local function delent(ply, argc, argv)
	if ply.AdminRank < ADMRANK_ADMIN then
		ply:SendPrint("^1Admin ^5- ^7You are not allowed to use this command")
		return
	end
	if argc < 2 then
		ply:SendPrint("Usage: /delent <entity index> (use with caution)")
		return
	else
		local ent = ents.GetByIndex(argv[1])
		if ent:IsPlayer() then
			ply:SendPrint("You cannot delete player entities")
			return
		end
		if ent:IsValid() then
			ply:SendPrint("Freeing entity " .. tostring(ent) .. "...")
			ent:Free()
		else
			ply:SendPrint("Specified entity does not exist")
		end
	end
end

cmds.Add("delent", delent)

-- Replica of lugormod's place command
-- Syntax: /place <entity name> <distance from aimed surface (* for explicit origin)> [key,value,key,value etc..]
local function place(ply, argc, argv)
	if ply.AdminRank < ADMRANK_ADMIN then
		ply:SendPrint("^1Admin ^5- ^7You are not allowed to use this command")
		return
	end
	if argc < 3 then
		ply:SendPrint("Usage: /place <entity name> <distance (* for explicit origin)> [keys/values, separated by ,]")
		return
	end
	local entname = argv[1]
	local offset = argv[2]
	local entfact = ents.CreateEntityFactory(entname)
	if offset ~= "*" then
		-- Determine the spawn location
		local tr = ply:GetEyeTrace()
		local entpos = tr.EndPos + (tr.HitNormal * (tonumber(offset) or 0))
		entfact:SetSpawnVar("origin", string.format("%i %i %i", entpos.x, entpos.y, entpos.z))
	end
	local tokens = table.concat(argv," ",3, argc-1)
	if string.trim(tokens) ~= "" then
		local tokens = string.split(tokens,",")
		local tokencount = #tokens % 2
		if tokencount ~= 0 then
			ply:SendPrint("Invalid spawnvars provided, uneven count")
			return
		end
		local i = 1
		local k,v
		while (i <= #tokens) do
			k = string.trim(tokens[i])
			v = string.trim(tokens[i+1])
			entfact:SetSpawnVar(k,v)
			i = i + 2
		end
	end
	local ent = entfact:Create()
	if ent:IsValid() then
		ply:SendPrint("Entity spawned successfully: " .. tostring(ent))
	else
		ply:SendPrint("Failed to spawn entity")
	end
end

cmds.Add("place", place)

local function entcount(ply, argc, argv)
	ply:SendPrint(string.format("Entity count - Normal: %i (%i slots allocated) / Logical: %i (%i slots allocated)", ents.EntCount(), ents.EntCountAllocated(), ents.LogicalEntCount(), ents.LogicalEntCountAllocated()))
end

cmds.Add("entcount", entcount)

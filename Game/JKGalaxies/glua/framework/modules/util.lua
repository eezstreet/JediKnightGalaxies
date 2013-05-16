--[[ ------------------------------------------------
	Ultra Utility Lua Framework
	Utility Module
	
	DO NOT MODIFY THIS FILE
	
	This module provides some handy utility functions, such as (de)serialization of tables
--------------------------------------------------]]


-- Locals (for faster access, and because we're about to lose access to the globals)
local pairs = pairs
local pcall = pcall
local print = print

-- Start the util module, all subsequent functions are variables, are stored within the util. table, and access to the global environment is gone
module("util")

--[[----------------------------------------------------------------------------------------
	util.SerializeTable
	
	Converts a table into a tokenized string
-------------------------------------------------------------------------------------------]]

local function SerAddToken(s, token) -- Helper function
	if s == "" then return token
	return s .. " " .. token
end
	
function SerializeTable(t)
	local str = ""

	for k, v in pairs( t ) do
		if type( k ) == "number" then
			SerAddToken(tostring(k))
		else
			SerAddToken(string.format("%q", tostring(k)))
		end
		
		if type( v ) == "table" then
			SerAddToken("{") -- Table start token
			SerAddToken(SerializeTable( v ))
			SerAddToken("}") -- Table end token
		elseif type( v ) == "string" then
			SerAddToken(string.format( "%q", v))
		else
			SerAddToken(tostring( v ))
		end
	end
	return str
end

--[[----------------------------------------------------------------------------------------
	hook.DeserializeTable
	
	Converts a serialized table (tokenized string) into a table
-------------------------------------------------------------------------------------------]]

function DeserializeTable(str)

end





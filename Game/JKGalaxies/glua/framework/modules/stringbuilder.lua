--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Framework
	Stringbuilder module
	
	DO NOT MODIFY THIS FILE
	
	This module lets you easilly construct and manage a string
	
	Written by BobaFett
--------------------------------------------------]]

local StringBuilder = {}

function sys.CreateStringBuilder()
	local o = {}
	setmetatable(o, StringBuilder)
	o.buffer = {}
	o.length = 0
	return o
end

StringBuilder.__index = StringBuilder

function StringBuilder:Append(str)
	local s = str
	if type(s) ~= "string" then
		s = tostring(str)
	end
	self.buffer[#self.buffer+1] = s
	self.length = self.length + string.len(s)
end

function StringBuilder:Length()
	return self.length
end

function StringBuilder:Clear()
	self.buffer = {}
	self.length = 0
end

function StringBuilder:ToString()
	return table.concat(self.buffer)
end
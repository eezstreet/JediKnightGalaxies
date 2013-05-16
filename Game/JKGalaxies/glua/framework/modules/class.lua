--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Framework
	Class manager
	
	DO NOT MODIFY THIS FILE
	
	This module manages classes
	
	Written by BobaFett
--------------------------------------------------]]

-- Locals (for faster access, and because we're about to lose access to the globals)
local pairs = pairs
local pcall = pcall
local Timer = sys.Milliseconds
local print = print
local table = table
local unpack = unpack
local tostring = tostring
local setmetatable = setmetatable
local coroutine = coroutine

-- Start the class module, all subsequent functions are variables, are stored within the class.* table, and access to the global environment is gone
module("class")

-- TODO
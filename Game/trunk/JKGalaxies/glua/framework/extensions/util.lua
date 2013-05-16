--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Framework
	Utility extensions
	
	DO NOT MODIFY THIS FILE
	
	Written by BobaFett
--------------------------------------------------]]

function PrintTable ( t, indent, done, SB ) 
	local key, value
 	local done = done or {} 
 	local indent = indent or 0
	local t = t
	local SB = SB or sys.CreateStringBuilder()
 	for key, value in pairs (t) do 
   
 		SB:Append( string.rep ("  ", indent)) 
   
 		if type (value) == "table" and not done [value] then 
   
 	      		done [value] = true 
 	      		SB:Append (tostring (key) .. ":" .. "\n"); 
 	     		PrintTable (value, indent + 2, done, SB) 
   
 	    	else 
   
 	      		SB:Append (tostring (key) .. "  =  ") 
 	      		SB:Append (tostring(value) .. "\n") 
   
 	    	end 
   
 	end 
	if indent == 0 then -- root table
		print(SB:ToString())
	end
 end
 
 -- Dumps a callstack into the console, use for debugging purposes only!
 function CallStack()
	local level = 2
	print("Call stack:")
	while true do
		local info = debug.getinfo(level, "Sl")
		if not info then break end
		if info.what == "C" then   -- is a C function?
			print(level, " C function")
		else   -- a Lua function
			print(string.format("%i [%s]:%d", level, info.short_src, info.currentline))
		end
		level = level + 1
	end
 end
 
 function printf(...)
	print(string.format(...))
 end
 
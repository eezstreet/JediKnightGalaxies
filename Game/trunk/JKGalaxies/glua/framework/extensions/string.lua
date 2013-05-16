--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Framework
	String Library Extentions
	
	DO NOT MODIFY THIS FILE
	
	Written by BobaFett
--------------------------------------------------]]


--[[----------------------------------------------------------------------------------------
	string.toTable
	
	Converts a string to a table ("hello" -> { "h", "e", "l", "l", "o"} )
-------------------------------------------------------------------------------------------]]
 
function string.toTable ( str ) 
   
 	local tab = {} 
 	 
 	for i=1, string.len( str ) do 
 		table.insert( tab, string.sub( str, i, i ) ) 
 	end 
 	 
 	return tab 
   
end 
   
   
--[[----------------------------------------------------------------------------------------
	string.split
	
	Convert a string into a table, using a specific seperator
	string.split("This is a string"," ") returns {"This", "is", "a", "string"}
-------------------------------------------------------------------------------------------]]
 
function string.split ( str, sep ) 

	if ( seperator == "" ) then 
		return string.toTable( str ) 
	end 

	local tbl = {} -- Table to store the results in
	local sloc = 0 -- Search location
	local mloc = 0 -- Match location
	 
	while (true) do 
	 
		mloc = string.find( str, sep, sloc, true ) 
		 
		if (mloc ~= nil) then 
			table.insert(tbl, string.sub(str,sloc,mloc-1))  
			sloc=mloc+1 
		else 
			table.insert(tbl, string.sub(str,sloc)) 
			break 
		end 
		 
	end 
	 
	return tbl 
 
end 

--[[----------------------------------------------------------------------------------------
	string.join
	
	Joins a (string) table together, using the specified seperator
	string.join( {"This","is","a","string"} ," ") returns "This is a string"
-------------------------------------------------------------------------------------------]]

function string.join(tbl, sep)
	return table.concat(tbl, sep)  
end 

--[[----------------------------------------------------------------------------------------
	string.left
	
	Returns the first xx characters of a string
	string.left("This is my string", 3) returns "Thi"
-------------------------------------------------------------------------------------------]]

function string.left(str, num) 
	return string.sub(str, 1, num) 
end 

--[[----------------------------------------------------------------------------------------
	string.right
	
	Returns the last xx characters of a string
	string.left("This is my string", 3) returns "ing"
-------------------------------------------------------------------------------------------]]
   
function string.right(str, num) 
	return string.sub(str, -num) 
end

--[[----------------------------------------------------------------------------------------
	string.replace
	
	Replaces a specific part of a string with something else
	string.replace("This is pure fail", "fail", "win") returns "This is pure win"
-------------------------------------------------------------------------------------------]]
   
function string.replace(str, tofind, toreplace) 
 	local start = 1 
 	while (true) do 
 		local pos = string.find(str, tofind, start, true) 
 	 
 		if (pos == nil) then 
 			break 
 		end 
 		 
 		local left = string.sub(str, 1, pos-1) 
 		local right = string.sub(str, pos + #tofind) 
 		 
 		str = left .. toreplace .. right 
 		start = pos + #toreplace 
 	end 
 	return str 
end 
   
--[[----------------------------------------------------------------------------------------
	string.trim
	
	Removes spaces (or the specified chars) from the beginning and ending of the provided string
	string.trim("          hey there      ") returns "hey there"
-------------------------------------------------------------------------------------------]]

function string.trim( s, char ) 
 	if (char==nil) then char = "%s" end 
 	return string.gsub(s, "^".. char.."*(.-)"..char.."*$", "%1") 
end 

--[[----------------------------------------------------------------------------------------
	string.trimRight
	
	Removes spaces (or the specified chars) from the ending of the provided string
	string.trim("          hey there      ") returns "          hey there"
-------------------------------------------------------------------------------------------]]
   
function string.trimRight( s, char ) 
 	 
 	if (char==nil) then char = " " end	 
 	 
 	if ( string.sub( s, -1 ) == char ) then 
 		s = string.sub( s, 0, -2 ) 
 		s = string.trimRight( s, char ) 
 	end 
 	 
 	return s 
 	 
end 

--[[----------------------------------------------------------------------------------------
	string.trimLeft
	
	Removes spaces (or the specified chars) from the beginning of the provided string
	string.trim("          hey there      ") returns "hey there      "
-------------------------------------------------------------------------------------------]]

function string.trimLeft( s, char ) 
   
 	if (char==nil) then char = " " end
	
	local startpos = 1
	
 	while true do
		
		if ( string.sub( s, startpos ) == char ) then 
			startpos = startpos + 1
		else
			return string.sub( s, startpos )
		end
	end
end  
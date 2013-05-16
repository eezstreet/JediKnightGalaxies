--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Framework
	Dialogue Manager
	
	DO NOT MODIFY THIS FILE
	
	Written by BobaFett
--------------------------------------------------]]
local _G = _G
local include = include
local print = print
local pairs = pairs
local tostring = tostring
local tonumber = tonumber
local setmetatable = setmetatable
local file = file
local sys = sys
local string = string
local Vector = Vector
local printf = printf
local type = type

module("dialogue")

local Dialogues = {}
local DlgCount = 0

function Init()
	Dialogues = {}
	DlgCount = 0
end
	
local function RegisterDialogue(path) -- path must be rooted! (/glua/xxxx)
	_G.DLG = {} -- Declare global namespace
	include(path) -- This will fill in the DLG namespace
	local name = _G.DLG.Name
	if name == nil then
		print(string.format("Error: RegisterDialogue: Failed to load %s, no name specified", path))
		_G.DLG = nil
		return false
	end
	if Dialogues[name] == nil then
		DlgCount = DlgCount + 1
	end
	Dialogues[name] = {}
	Dialogues[name].path = path
	Dialogues[name].data = _G.DLG
	_G.DLG = nil
	return true
end

function RegisterDir(dir)
	local files = file.ListFiles(dir, ".lua")
	if #files == 0 then return end
	for k, v in pairs(files) do
		RegisterDialogue(string.format("/%s/%s", dir, v))
	end
end

-- NodeTypes
--local NT_ROOT = 0		-- Not used
local NT_ENTRYPOINT = 1
local NT_TEXT = 2
local NT_OPTION = 3
local NT_LINK = 4
local NT_END = 5
local NT_SCRIPT = 6
local NT_WAITSCRIPT = 7
local NT_TEXTENTRY = 8
local NT_DYNAMICOPTIONS = 9

local DialogueRunner = {}		-- The dialogue runner object
DialogueRunner.__index = DialogueRunner

local function CreateDialogueRunner(Dialogue)
	local o = {}
	setmetatable(o, DialogueRunner)
	o.Dlg = Dialogue
	o.Convo = nil
	o.Player = nil
	o.Owner = nil
	o.CurrentNodeID = nil
	o.CurrentNode = nil
	o.ConvoPending = false
	o.WaitingForScript = false
	o.WaitScriptFinished = true
	o.Callback = nil
	o.InOptions = false
	o.Data = nil
	return o
end

function DialogueRunner:RunDialogue(owner, ply)
	self.Player = ply
	self.Owner = owner
	self.Convo = sys.CreateConversation()
	self.ConvoActive = false
	self.Data = {}
	self.CurrentNodeID = self.Dlg.RootNode
	if not self.CurrentNodeID then
		return false
	end
	self.CurrentNode = self.Dlg.Nodes[self.CurrentNodeID]
	if not self.CurrentNode then
		return false
	end
	self:ResumeDialogue()
	return true
end

function DialogueRunner:SetCallback(func)
	self.Callback = func
end

function DialogueRunner:ConditionPassed(node)
	if node.HasCondition then
		if node.ConditionFunc(self.Owner, self.Player, self.Data) then
			return true
		else
			return false
		end
	else
		return true
	end
end

function DialogueRunner:ResolveText(node, element)
	local res
	if node.Type ~= NT_TEXT and node.Type ~= NT_OPTION and node.Type ~= NT_TEXTENTRY then	
		return ""
	else
		if node.Type == NT_TEXTENTRY then
			-- Special treatment
			if not node.HasResolver then
				if element == 1 then
					return node.Caption
				else 
					return node.DefVal
				end
			else
				local function resolver(n)
					return node.ResolveFunc(self.Owner, self.Player, n, self.Data) or ""
				end
				if element == 1 then
					res = string.gsub(node.Caption, "%$(%w+)%$", resolver)
					return res
				else 
					res = string.gsub(node.DefVal, "%$(%w+)%$", resolver)
					return res
				end
			end
		else
			if not node.HasResolver then
				return node.Text
			else
				local function resolver(n)
					return node.ResolveFunc(self.Owner, self.Player, n, self.Data) or ""
				end
				res = string.gsub(node.Text, "%$(%w+)%$", resolver)
				return res
			end
		end
	end
end

function DialogueRunner:RunWaitScript(node)
	-- Function for the script to call to resume
	local function resumefunc()
		self.WaitScriptFinished = true
		if self.WaitingForScript then
			if self.CurrentNode.SubNode then
				self.CurrentNode = self.Dlg.Nodes[self.CurrentNode.SubNode]
			else
				self.CurrentNode = nil
			end
			if not self.CurrentNode then
				self:DoTerminate()
				return
			end
			self.ConvoPending = false
			self.InOptions = false
			self:ResumeDialogue()
		end
	end
	self.WaitingForScript = false
	self.WaitScriptFinished = false
	-- First, we run the script with WaitingForScript set to false, that way, if resumefunc gets called before the next call returns,
	-- we wont get everything derailed due to the call to ResumeDialogue
	local showmouse
	showmouse = node.ScriptFunc(self.Owner, self.Player, resumefunc, self.Data)
	if self.WaitScriptFinished then
		return false
	end
	self.WaitingForScript = true
	return true, showmouse
end

-- Process the current node
-- Return values:
-- 0 = Error (Error in question is passed as second return value)
-- 1 = Proceed to subnode
-- 2 = Proceed to nextnode
-- 3 = Execute convo with options
-- 4 = Execute convo without options and resume at current node
-- 5 = Execute convo without options and terminate
-- 6 = Suspend (wait for ResumeDialogue call)
-- 7 = Terminate
-- 8 = Switch execution to specified node (Node is passed as second return value)
-- 9 = Execute text entry

function DialogueRunner:ProcessNode(node)
	local nodetype = node.Type
	
	if self.InOptions and (nodetype ~= NT_OPTION and nodetype ~= NT_LINK and nodetype ~= NT_DYNAMICOPTIONS) then
		return 0, "Non-option node while processing options"
	end
	
	if nodetype == NT_ENTRYPOINT then
		if self:ConditionPassed(node) then
			return 1
		else
			return 2
		end
	elseif nodetype == NT_TEXT then
		if self:ConditionPassed(node) then
			self.ConvoPending = true
			self.Convo:AddText(self:ResolveText(node), node.Duration)
			return 1
		else 
			return 2
		end
	elseif nodetype == NT_OPTION then
		if self:ConditionPassed(node) then
			self.InOptions = true
			self.ConvoPending = true
			if not node.SubNode then
				return 0, "Option node without subnodes found"
			end
			self.Convo:AddChoice(self:ResolveText(node), node.SubNode)
		end
		if node.NextNode == nil then
			return 3
		else 
			return 2
		end
	elseif nodetype == NT_LINK then
		if not node.Target then
			return 0, "Link node without target found"
		end
		local linknode = self.Dlg.Nodes[node.Target]
		if not linknode then
			return 0, "Link node with invalid target found"
		end
		if linknode.Type == NT_OPTION then
			-- Special handling here
			local linkret
			linkret = self:ProcessNode(linknode)
			if linkret == 0 then
				return 0, "Option node without subnodes found"
			end
			if node.NextNode == nil then
				return 3
			else 
				return 2
			end 
		else
			return 8, linknode
		end
	elseif nodetype == NT_END then
		if self.ConvoPending then
			return 5
		else
			return 7
		end
	elseif nodetype == NT_SCRIPT then
		if self:ConditionPassed(node) then
			if self.ConvoPending then
				return 4
			else
				node.ScriptFunc(self.Owner, self.Player, self.Data)
				return 1
			end
		else
			return 2
		end
	elseif nodetype == NT_WAITSCRIPT then
		if self:ConditionPassed(node) then
			if self.ConvoPending then
				return 4
			else
				local pending, showmouse = self:RunWaitScript(node)
				if pending then
					return 6, showmouse	-- Script hasnt called the return func yet
				else
					return 1		-- Script called the return func right away, proceed
				end
			end
		else
			return 2
		end
	elseif nodetype == NT_TEXTENTRY then
		if self:ConditionPassed(node) then
			self.ConvoPending = true
			self.Convo:AddTextEntry(self:ResolveText(node, 1), self:ResolveText(node, 2), node.Flags)
			return 9
		else
			return 2
		end
	elseif nodetype == NT_DYNAMICOPTIONS then
		self.InOptions = true
		self.ConvoPending = true
		if not node.SubNode then
			return 0, "Dynamic options node without subnodes found"
		end
		local function addchoice(text, tag)
			local nodetype
			local tagtype
			if type(tag) == "number" then tagtype = "N" else tagtype = "S" end
			if type(node.Host) == "number" then nodetype = "N" else nodetype = "S" end
			self.Convo:AddChoice(text, string.format("!%s%s-%s%s", nodetype, tostring(node.Host), tagtype, tostring(tag)))
		end
		node.SetupFunc(self.Owner, self.Player, addchoice, self.Data)
		if node.NextNode == nil then
			return 3
		else 
			return 2
		end	
	end
	return 0, "Invalid node type"
end

function DialogueRunner:ResumeDialogue()
	local node
	local action, data
	-- Loop until we're interrupted
	while (true) do
		node = self.CurrentNode
		if (not node) then
			return self:DoTerminate()
		end
		action, data = self:ProcessNode(node)
		if action == 0 then -- 0 = Error (Error in question is passed as second return value)
			print("Dialogue error: ", data)
			return self:DoTerminate()
		elseif action == 1 then -- 1 = Proceed to subnode
			if not node.SubNode then
				-- Shouldn't happen, but let's try dealing with this
				if self.ConvoPending then
					if self.InOptions then
						return self:DoOptions()
					else
						return self:DoConvoAndTerminate()
					end
				else
					return self:DoTerminate()
				end
			else
				self.CurrentNode = self.Dlg.Nodes[node.SubNode]
			end
		elseif action == 2 then -- 2 = Proceed to nextnode
			if not node.NextNode then
				-- Shouldn't happen, but let's try dealing with this
				if self.ConvoPending then
					if self.InOptions then
						return self:DoOptions()
					else
						return self:DoConvoAndTerminate()
					end
				else
					return self:DoTerminate()
				end
			else
				self.CurrentNode = self.Dlg.Nodes[node.NextNode]
			end
		elseif action == 3 then -- 3 = Execute convo with options
			return self:DoOptions()
		elseif action == 4 then -- 4 = Execute convo without options and resume at current node
			return self:DoConvoAndResume()
		elseif action == 5 then -- 5 = Execute convo without options and terminate
			return self:DoConvoAndTerminate()
		elseif action == 6 then -- 6 = Suspend (wait for ResumeDialogue call)
			self.Convo:Halt(data)
			return
		elseif action == 7 then -- 7 = Terminate
			return self:DoTerminate()
		elseif action == 8 then -- 8 = Switch execution to specified node (Node is passed as second return value)
			self.CurrentNode = data
		elseif action == 9 then -- 9 = Execute text entry
			return self:DoTextEntry()
		else
			print("Invalid response from DialogueRunner:ProcessNode")
			return self:DoTerminate()
		end	
	end
end

function DialogueRunner:DoOptions()
	local function callback(conv, ply, convid, resp, resptag)
		if not resptag then
			self:DoTerminate()
		end
		if string.left(tostring(resptag),1) == "!" then
			-- Dynamic option, parse it
			local nodetype, node, tagtype, tag
			_,_,nodetype,node,tagtype,tag = string.find(resptag, "!(%w)(%w+)-(%w)(.+)")
			if nodetype == "N" then node = tonumber(node) end
			if tagtype == "N" then tag = tonumber(tag) end
			node = self.Dlg.Nodes[node]
			if not node then -- Should never happen
				return self:DoTerminate()
			end
			node.ProcessFunc(self.Owner, self.Player, tag, self.Data)
			self.CurrentNode = self.Dlg.Nodes[node.SubNode]
			if not self.CurrentNode then
				return self:DoTerminate()
			end
			self.ConvoPending = false
			self.InOptions = false
			self:ResumeDialogue()
		else	
			self.CurrentNode = self.Dlg.Nodes[resptag]
			if not self.CurrentNode then
				print("Error: Node ", resptag, " not found")
				return self:DoTerminate()
			end
			self.ConvoPending = false
			self.InOptions = false
			self:ResumeDialogue()
		end
	end
	
	self.Convo:SetChoiceCallback(callback)
	if self.ConvoActive then
		self.Convo:Continue()
	else
		self.ConvoActive = true
		self.Convo:RunConvo(self.Player)
	end		
end

function DialogueRunner:DoTextEntry()
	local function callback(conv, ply, convid, cancelled, response)
		if cancelled then
			self.CurrentNode.ScriptFunc(self.Owner, self.Player, nil, self.Data)
		else
			self.CurrentNode.ScriptFunc(self.Owner, self.Player, response, self.Data)
		end
		if not self.CurrentNode.SubNode then
			return self:DoTerminate()
		else
			self.CurrentNode = self.Dlg.Nodes[self.CurrentNode.SubNode]
		end
		self.ConvoPending = false
		self.InOptions = false
		self:ResumeDialogue()
	end
	
	self.Convo:SetTextEntryCallback(callback)
	if self.ConvoActive then
		self.Convo:Continue()
	else
		self.ConvoActive = true
		self.Convo:RunConvo(self.Player)
	end		
end

function DialogueRunner:DoConvoAndTerminate()
	local function callback()
		self:DoTerminate()
	end
	
	self.Convo:SetFinishCallback(callback)
	if self.ConvoActive then
		self.Convo:Continue()
	else
		self.ConvoActive = true
		self.Convo:RunConvo(self.Player)
	end		
end

function DialogueRunner:DoConvoAndResume()
	local function callback()
		self.ConvoPending = false
		self.InOptions = false
		self:ResumeDialogue()
		return true
	end
	
	self.Convo:SetFinishCallback(callback)
	if self.ConvoActive then
		self.Convo:Continue()
	else
		self.ConvoActive = true
		self.Convo:RunConvo(self.Player)
	end	
end

function DialogueRunner:DoTerminate()
	self.Convo:Clear()
	-- If we never used the convo object, it means the client hasnt activated convo mode yet, so dont bother doing anything in that case
	self.Convo:SetFinishCallback(nil)
	if self.ConvoActive then
		self.Convo:Continue()
	end
	if self.Callback then
		self.Callback(self.Owner, self.Player)
	end
end

function CreateDialogueObject(name)
	if Dialogues[name] == nil then
		return nil
	end
	-- Return dialogue object here
	
	return CreateDialogueRunner(Dialogues[name].data)
end

function ReloadDialogue(name)
	if Dialogues[name] == nil then
		return false
	end
	return RegisterDialogue(Dialogues[name].path)
end

function GetDlgCount()
	return DlgCount
end
		
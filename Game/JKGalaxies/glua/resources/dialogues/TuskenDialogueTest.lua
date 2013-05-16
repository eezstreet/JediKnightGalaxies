-- Exported by JKG Dialogue Creator

DLG.Name = "TuskenDialogueTest"
DLG.RootNode = "E1"
DLG.Nodes = {
	E1 = {
		Type = 1,
		SubNode = "T2",
		NextNode = "E25",
		HasCondition = true,
		ConditionFunc = function(owner, ply)
			local state = 0
			
			if ply.Quests["myquest"] then
			    state = ply.Quests["myquest"].state
			end
			
			if state == 0 then
			    return true
			else
			    return false
			end 
		end,
	},
	T2 = {
		Type = 2,
		SubNode = "O3",
		Text = "What are we going to do... we will all die! Oh, what are we going to do?",
		Duration = 5000,
		HasCondition = false,
		HasResolver = false,
	},
	O3 = {
		Type = 3,
		SubNode = "T6",
		NextNode = "O4",
		Text = "Get your act together and spit it out before I cut you in pieces",
		HasCondition = false,
		HasResolver = false,
	},
	T6 = {
		Type = 2,
		SubNode = "O7",
		Text = "I-I, I'm sorry... It's just the tuskens!\nThose damn beasts have been raiding this place daily for almost a week now!",
		Duration = 6000,
		HasCondition = false,
		HasResolver = false,
	},
	O7 = {
		Type = 3,
		SubNode = "T15",
		NextNode = "O8",
		Text = "Why would they attack you?",
		HasCondition = false,
		HasResolver = false,
	},
	T15 = {
		Type = 2,
		SubNode = "L16",
		Text = "Cause they are beasts, Savage animal-like beasts I tell you!",
		Duration = 4000,
		HasCondition = false,
		HasResolver = false,
	},
	L16 = {
		Type = 4,
		NextNode = "L17",
		Target = "O8",
	},
	L17 = {
		Type = 4,
		NextNode = "L18",
		Target = "O10",
	},
	L18 = {
		Type = 4,
		Target = "O11",
	},
	O8 = {
		Type = 3,
		SubNode = "T19",
		NextNode = "O10",
		Text = "Who can I talk to, about helping with this problem?",
		HasCondition = false,
		HasResolver = false,
	},
	T19 = {
		Type = 2,
		SubNode = "T20",
		Text = "You should talk to Bennett.",
		Duration = 2000,
		HasCondition = false,
		HasResolver = false,
	},
	T20 = {
		Type = 2,
		SubNode = "T21",
		Text = "He's in charge of maintaining the security around here.",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	T21 = {
		Type = 2,
		SubNode = "S54",
		Text = "But lately it seems he isn't even doing his job!",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	S54 = {
		Type = 6,
		SubNode = "D22",
		ScriptFunc = function(owner, ply)
			if not ply.Quests["myquest"] then
			    ply.Quests["myquest"] = {}
			end
			
			ply.Quests["myquest"].state = 1
			ply.Quests["myquest"].spokenTo = owner
		end,
		HasCondition = false,
	},
	D22 = {
		Type = 5,
	},
	O10 = {
		Type = 3,
		SubNode = "L23",
		NextNode = "O11",
		Text = "If you are too weak to fight, you deserve it. Bye.",
		HasCondition = false,
		HasResolver = false,
	},
	L23 = {
		Type = 4,
		Target = "T13",
	},
	O11 = {
		Type = 3,
		SubNode = "L24",
		Text = "Maybe I'll be back later",
		HasCondition = false,
		HasResolver = false,
	},
	L24 = {
		Type = 4,
		Target = "T13",
	},
	O4 = {
		Type = 3,
		SubNode = "L12",
		NextNode = "O5",
		Text = "Calm down, Maybe I can help. What's the problem?",
		HasCondition = false,
		HasResolver = false,
	},
	L12 = {
		Type = 4,
		Target = "T6",
	},
	O5 = {
		Type = 3,
		SubNode = "T13",
		Text = "You bore me, bye.",
		HasCondition = false,
		HasResolver = false,
	},
	T13 = {
		Type = 2,
		SubNode = "D14",
		Text = "No one can help us anyways! We're doomed, doomed I tell you!",
		Duration = 4000,
		HasCondition = false,
		HasResolver = false,
	},
	D14 = {
		Type = 5,
	},
	E25 = {
		Type = 1,
		SubNode = "T26",
		NextNode = "E32",
		HasCondition = true,
		ConditionFunc = function(owner, ply)
			local state = 0
			
			if ply.Quests["myquest"] then
			    state = ply.Quests["myquest"].state
			end
			
			if state == 1 then
			    return true
			else
			    return false
			end 
		end,
	},
	T26 = {
		Type = 2,
		SubNode = "D29",
		NextNode = "T27",
		Text = "You should talk to benett.",
		Duration = 2000,
		HasCondition = true,
		ConditionFunc = function(owner, ply)
			return ply.Quests["myquest"].spokenTo == owner
		end,
		HasResolver = false,
	},
	D29 = {
		Type = 5,
	},
	T27 = {
		Type = 2,
		SubNode = "D30",
		NextNode = "T28",
		Text = "I don't want to die!",
		Duration = 2000,
		HasCondition = true,
		ConditionFunc = function(owner, ply)
			return math.random(0,10) > 5
		end,
		HasResolver = false,
	},
	D30 = {
		Type = 5,
	},
	T28 = {
		Type = 2,
		SubNode = "D31",
		Text = "What are we going to do?!",
		Duration = 1000,
		HasCondition = false,
		HasResolver = false,
	},
	D31 = {
		Type = 5,
	},
	E32 = {
		Type = 1,
		SubNode = "T33",
		NextNode = "E51",
		HasCondition = true,
		ConditionFunc = function(owner, ply)
			local state = 0
			
			if ply.Quests["myquest"] then
			    state = ply.Quests["myquest"].state
			end
			
			if state == 2 then
			    return true
			else
			    return false
			end 
		end,
	},
	T33 = {
		Type = 2,
		SubNode = "O34",
		Text = "Why are you bothering me now? I have to find somewhere to hide!",
		Duration = 5000,
		HasCondition = false,
		HasResolver = false,
	},
	O34 = {
		Type = 3,
		SubNode = "T37",
		NextNode = "O35",
		Text = "Have you seen some tools around here somewhere?",
		HasCondition = false,
		HasResolver = false,
	},
	T37 = {
		Type = 2,
		SubNode = "T38",
		Text = "Why are you bothering me with a question like that?",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	T38 = {
		Type = 2,
		SubNode = "L39",
		Text = "The only one who would know where the tools are, is Bennett so go ask him!",
		Duration = 5000,
		HasCondition = false,
		HasResolver = false,
	},
	L39 = {
		Type = 4,
		NextNode = "L40",
		Target = "O35",
	},
	L40 = {
		Type = 4,
		Target = "O36",
	},
	O35 = {
		Type = 3,
		SubNode = "T41",
		NextNode = "O36",
		Text = "Did you notice anything suspicious going on lately?",
		HasCondition = false,
		HasResolver = false,
	},
	T41 = {
		Type = 2,
		SubNode = "T42",
		Text = "NO I haven't and would...",
		Duration = 2000,
		HasCondition = false,
		HasResolver = false,
	},
	T42 = {
		Type = 2,
		SubNode = "T43",
		Text = "Wait, yes, now that you say it, for the past week I haven't seen\nBennett's kid Siran running around.",
		Duration = 6000,
		HasCondition = false,
		HasResolver = false,
	},
	T43 = {
		Type = 2,
		SubNode = "T44",
		Text = "He usually plays around in the halls.",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	T44 = {
		Type = 2,
		SubNode = "O45",
		Text = "Now that I come to think of it, he disappeared about the same time\nBennett suddenly started stuttering!",
		Duration = 6000,
		HasCondition = false,
		HasResolver = false,
	},
	O45 = {
		Type = 3,
		SubNode = "T46",
		Text = "He didn't always stutter like that?",
		HasCondition = false,
		HasResolver = false,
	},
	T46 = {
		Type = 2,
		SubNode = "T47",
		Text = "Stutter? Bennett? He used to be as confident and stubborn as a bantha!",
		Duration = 4000,
		HasCondition = false,
		HasResolver = false,
	},
	T47 = {
		Type = 2,
		SubNode = "T48",
		Text = "But these days he has been quiet and stuttering whenever anyone has talked to him...",
		Duration = 5000,
		HasCondition = false,
		HasResolver = false,
	},
	T48 = {
		Type = 2,
		SubNode = "S55",
		Text = "But would you please leave me alone now? I got nothing more to say.",
		Duration = 4000,
		HasCondition = false,
		HasResolver = false,
	},
	S55 = {
		Type = 6,
		SubNode = "D49",
		ScriptFunc = function(owner, ply)
			ply.Quests["myquest"].state = 3
		end,
		HasCondition = false,
	},
	D49 = {
		Type = 5,
	},
	O36 = {
		Type = 3,
		SubNode = "D50",
		Text = "I'm sorry, I'll leave now.",
		HasCondition = false,
		HasResolver = false,
	},
	D50 = {
		Type = 5,
	},
	E51 = {
		Type = 1,
		SubNode = "D52",
		HasCondition = false,
	},
	D52 = {
		Type = 5,
	},
}

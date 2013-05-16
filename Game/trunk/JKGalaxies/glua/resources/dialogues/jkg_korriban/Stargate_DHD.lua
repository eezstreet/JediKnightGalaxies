DLG.Name="Stargate_DHD"DLG.RootNode=1 DLG.Nodes={{Type=1,SubNode=2,NextNode=55,HasCondition=true,ConditionFunc=function(t,e,t)if JKG.GMH.StarGate.GMOnly then
return e.AdminRank>=ADMRANK_DEVELOPER
else
return true
end
end,},{Type=2,SubNode=3,Text="Stargate Dial-Home Device\n$state$",Duration=3e3,HasCondition=false,HasResolver=true,ResolveFunc=function(t,t,e,t)if e=="state"then
local e=JKG.GMH.StarGate.GetState()if e==0 then
return"The stargate is currently idle"elseif e==1 then
return"The stargate is currently dialing or being dialed"elseif e==2 then
return"The stargate is dialed ("..JKG.GMH.StarGate.GetAddress()..")"elseif e==3 then
return"The stargate has been dialed from another location"elseif e==4 then
return"The stargate is shutting down"else
return"The stargate's state is unknown ("..e..")"end
end
return""end,},{Type=3,SubNode=4,NextNode=15,Text="Dial Stargate",HasCondition=false,HasResolver=false,},{Type=2,SubNode=5,NextNode=13,Text="Enter address to dial (7 characters, 0-9, a-z and @#$ allowed)",Duration=3e3,HasCondition=true,ConditionFunc=function(e,e,e)return JKG.GMH.StarGate.GetState()==0 end,HasResolver=false,},{Type=8,SubNode=6,Caption="Enter address (7 characters, 0-9, a-z and @#$ allowed)",DefVal="",Flags=1,ScriptFunc=function(a,a,t,e)if not t then
e.Cancel=true
else
e.Cancel=false
e.Address=t
e.AddressOk,e.Error=JKG.GMH.StarGate.IsValidAddress(t)end
end,HasCondition=false,HasResolver=false,},{Type=6,SubNode=7,NextNode=9,ScriptFunc=function(t,t,e)JKG.GMH.StarGate.Dial(e.Address)end,HasCondition=true,ConditionFunc=function(t,t,e)return e.Cancel==false and e.AddressOk end,},{Type=2,SubNode=8,Text="Dialing $address$...",Duration=3e3,HasCondition=false,HasResolver=true,ResolveFunc=function(a,a,e,t)if e=="address"then
return t.Address
end
end,},{Type=4,Target=2,},{Type=2,SubNode=10,NextNode=12,Text="Bad address: $error$",Duration=3e3,HasCondition=true,ConditionFunc=function(t,t,e)return e.Cancel==false and e.AddressOk==false
end,HasResolver=true,ResolveFunc=function(a,a,e,t)if e=="error"then
return t.Error
end
end,},{Type=4,NextNode=11,Target=4,},{Type=4,Target=13,},{Type=4,Target=2,},{Type=2,SubNode=14,Text="Cannot dial the stargate at this moment. Wait for the gate to be off first.",Duration=3e3,HasCondition=false,HasResolver=false,},{Type=4,Target=2,},{Type=3,SubNode=16,NextNode=33,Text="Show Address List",HasCondition=false,HasResolver=false,},{Type=2,SubNode=17,Text="Select address category",Duration=2e3,HasCondition=false,HasResolver=false,},{Type=3,SubNode=18,NextNode=24,Text="Remote addresses",HasCondition=false,HasResolver=false,},{Type=2,SubNode=19,Text="Please choose a location",Duration=2e3,HasCondition=false,HasResolver=false,},{Type=3,SubNode=20,NextNode=21,Text="Go back",HasCondition=false,HasResolver=false,},{Type=4,Target=2,},{Type=9,SubNode=22,Host=21,SetupFunc=function(e,e,t,e)local e,e
for a,e in pairs(JKG.GMH.StarGate.AddressTable)do
if e.Type==2 then
t(e.Description,a)end
end end,ProcessFunc=function(a,a,t,e)e.Tag=t end,},{Type=2,SubNode=23,Text="Location: $loc$\nAddress: $address$",Duration=5e3,HasCondition=false,HasResolver=true,ResolveFunc=function(a,a,t,e)if t=="loc"then
return JKG.GMH.StarGate.AddressTable[e.Tag].Description
elseif t=="address"then
return e.Tag
end
end,},{Type=4,Target=18,},{Type=3,SubNode=25,NextNode=31,Text="Local addresses",HasCondition=false,HasResolver=false,},{Type=2,SubNode=26,Text="Please choose a location",Duration=2e3,HasCondition=false,HasResolver=false,},{Type=3,SubNode=27,NextNode=28,Text="Go back",HasCondition=false,HasResolver=false,},{Type=4,Target=2,},{Type=9,SubNode=29,Host=28,SetupFunc=function(e,e,a,e)local e,e
for t,e in pairs(JKG.GMH.StarGate.AddressTable)do
if e.Type==1 then
a(e.Description,t)end
end end,ProcessFunc=function(a,a,e,t)t.Tag=e end,},{Type=2,SubNode=30,Text="Location: $loc$\nAddress: $address$",Duration=5e3,HasCondition=false,HasResolver=true,ResolveFunc=function(a,a,e,t)if e=="loc"then
return JKG.GMH.StarGate.AddressTable[t.Tag].Description
elseif e=="address"then
return t.Tag
end
end,},{Type=4,Target=25,},{Type=3,SubNode=32,Text="Go back",HasCondition=false,HasResolver=false,},{Type=4,Target=2,},{Type=3,SubNode=34,NextNode=43,Text="Shut Down Stargate",HasCondition=false,HasResolver=false,},{Type=2,SubNode=35,NextNode=36,Text="The stargate is busy, cannot shut down right now",Duration=2e3,HasCondition=true,ConditionFunc=function(e,e,e)local e=JKG.GMH.StarGate.GetState()return e==1 or e==4
end,HasResolver=false,},{Type=4,Target=2,},{Type=2,SubNode=37,NextNode=38,Text="The stargate is already shut down",Duration=2e3,HasCondition=true,ConditionFunc=function(e,e,e)local e=JKG.GMH.StarGate.GetState()return e==0
end,HasResolver=false,},{Type=4,Target=2,},{Type=2,SubNode=39,NextNode=40,Text="The stargate has an incoming connection, cannot shut down.",Duration=2e3,HasCondition=true,ConditionFunc=function(e,e,e)local e=JKG.GMH.StarGate.GetState()return e==3
end,HasResolver=false,},{Type=4,Target=2,},{Type=6,SubNode=41,ScriptFunc=function(e,e,e)JKG.GMH.StarGate.Shutdown()end,HasCondition=false,},{Type=2,SubNode=42,Text="The stargate is shutting down...",Duration=2e3,HasCondition=false,HasResolver=false,},{Type=4,Target=2,},{Type=3,SubNode=44,NextNode=45,Text="Refresh state",HasCondition=false,HasResolver=false,},{Type=4,Target=2,},{Type=3,SubNode=46,NextNode=49,Text="Restrict DHD usage to GMs",HasCondition=true,ConditionFunc=function(t,e,t)return e.AdminRank>=ADMRANK_ADMIN and not JKG.GMH.StarGate.GMOnly
end,HasResolver=false,},{Type=6,SubNode=47,ScriptFunc=function(e,e,e)JKG.GMH.StarGate.GMOnly=true
end,HasCondition=false,},{Type=2,SubNode=48,Text="DHD usage is now restricted to GM's only",Duration=2e3,HasCondition=false,HasResolver=false,},{Type=4,Target=2,},{Type=3,SubNode=50,NextNode=53,Text="Allow DHD usage by non-GMs",HasCondition=true,ConditionFunc=function(t,e,t)return e.AdminRank>=ADMRANK_ADMIN and JKG.GMH.StarGate.GMOnly end,HasResolver=false,},{Type=6,SubNode=51,ScriptFunc=function(e,e,e)JKG.GMH.StarGate.GMOnly=false
end,HasCondition=false,},{Type=2,SubNode=52,Text="DHD usage is now unrestricted",Duration=3e3,HasCondition=false,HasResolver=false,},{Type=4,Target=2,},{Type=3,SubNode=54,Text="Leave",HasCondition=false,HasResolver=false,},{Type=5,},{Type=1,SubNode=56,HasCondition=false,},{Type=2,SubNode=57,Text="This DHD is restricted to GM use only.",Duration=2e3,HasCondition=false,HasResolver=false,},{Type=5,},}
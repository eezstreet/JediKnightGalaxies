DLG.Name="gmh_vaultcontrol"DLG.RootNode=1 DLG.Nodes={{Type=1,SubNode=2,NextNode=42,HasCondition=true,ConditionFunc=function(o,e,o)if JKG.GMH.VaultDoor.AllowNonGM then
return true
else
return e.IsAdmin
end
end,},{Type=2,SubNode=3,Text="Vault door controls.\nChoose your desired action:",Duration=3e3,HasCondition=false,HasResolver=false,},{Type=3,SubNode=4,NextNode=14,Text="Open vault door",HasCondition=true,ConditionFunc=function(e,e,e)return not JKG.GMH.VaultDoor.IsOpen()and not JKG.GMH.VaultDoor.IsInUse()end,HasResolver=false,},{Type=2,SubNode=5,NextNode=11,Text="Please enter password",Duration=2e3,HasCondition=true,ConditionFunc=function(o,e,n)if not o.IsOutside then
return false
else
if JKG.GMH.VaultDoor.GMNoPass and e.IsAdmin then
return false
else
return true
end
end end,HasResolver=false,},{Type=8,SubNode=6,Caption="Enter password",DefVal="",Flags=17,ScriptFunc=function(n,n,o,e)if not o then
e.Cancel=true
else
e.Cancel=false
e.PassOk=o==JKG.GMH.VaultDoor.Password
end
end,HasCondition=false,HasResolver=false,},{Type=2,SubNode=7,NextNode=8,Text="Incorrect password",Duration=1e3,HasCondition=true,ConditionFunc=function(o,o,e)return not e.Cancel and not e.PassOk
end,HasResolver=false,},{Type=4,Target=5,},{Type=2,SubNode=9,NextNode=10,Text="Password valid",Duration=1e3,HasCondition=true,ConditionFunc=function(o,o,e)return not e.Cancel and e.PassOk
end,HasResolver=false,},{Type=4,Target=11,},{Type=4,Target=2,},{Type=6,SubNode=12,ScriptFunc=function(e,e,e)JKG.GMH.VaultDoor.Open()end,HasCondition=false,},{Type=2,SubNode=13,Text="Opening vault door...",Duration=2e3,HasCondition=false,HasResolver=false,},{Type=5,},{Type=3,SubNode=15,NextNode=18,Text="Close vault door",HasCondition=true,ConditionFunc=function(e,e,e)return JKG.GMH.VaultDoor.IsOpen()and not JKG.GMH.VaultDoor.IsInUse()end,HasResolver=false,},{Type=6,SubNode=16,ScriptFunc=function(e,e,e)JKG.GMH.VaultDoor.Close()end,HasCondition=false,},{Type=2,SubNode=17,Text="Closing vault door...",Duration=2e3,HasCondition=false,HasResolver=false,},{Type=5,},{Type=3,SubNode=19,NextNode=21,Text="Access security gate override",HasCondition=true,ConditionFunc=function(e,o,n)return not e.IsOutside and o.IsAdmin end,HasResolver=false,},{Type=2,SubNode=20,Text="Currently not available",Duration=2e3,HasCondition=false,HasResolver=false,},{Type=4,Target=2,},{Type=3,SubNode=22,NextNode=40,Text="Access configuration",HasCondition=true,ConditionFunc=function(o,e,o)return e.IsAdmin end,HasResolver=false,},{Type=2,SubNode=23,Text="Choose setting to modify",Duration=2e3,HasCondition=false,HasResolver=false,},{Type=3,SubNode=24,NextNode=28,Text="Allow non-GM access - $state$",HasCondition=false,HasResolver=true,ResolveFunc=function(o,o,e,o)if e=="state"then
return JKG.GMH.VaultDoor.AllowNonGM and"Enabled"or"Disabled"end
end,},{Type=2,SubNode=25,NextNode=26,Text="Non-GM access is now disabled",Duration=2e3,HasCondition=true,ConditionFunc=function(e,e,e)JKG.GMH.VaultDoor.AllowNonGM=not JKG.GMH.VaultDoor.AllowNonGM
return not JKG.GMH.VaultDoor.AllowNonGM end,HasResolver=false,},{Type=4,Target=22,},{Type=2,SubNode=27,Text="Non-GM access is now enabled",Duration=3e3,HasCondition=false,HasResolver=false,},{Type=4,Target=22,},{Type=3,SubNode=29,NextNode=33,Text="GM's don't need to enter a password - $state$",HasCondition=false,HasResolver=true,ResolveFunc=function(o,o,e,o)if e=="state"then
return JKG.GMH.VaultDoor.GMNoPass and"Enabled"or"Disabled"end
end,},{Type=2,SubNode=30,NextNode=31,Text="GM password entry is now enabled",Duration=2e3,HasCondition=true,ConditionFunc=function(e,e,e)JKG.GMH.VaultDoor.GMNoPass=not JKG.GMH.VaultDoor.GMNoPass
return not JKG.GMH.VaultDoor.GMNoPass end,HasResolver=false,},{Type=4,Target=22,},{Type=2,SubNode=32,Text="GM password entry is now disabled",Duration=2e3,HasCondition=false,HasResolver=false,},{Type=4,Target=22,},{Type=3,SubNode=34,NextNode=38,Text="Access Password - $state$",HasCondition=false,HasResolver=true,ResolveFunc=function(o,o,e,o)if e=="state"then
return JKG.GMH.VaultDoor.Password
end
end,},{Type=8,SubNode=35,Caption="Enter new password",DefVal="",Flags=17,ScriptFunc=function(n,n,o,e)if not o then
e.Cancel=true
else
e.Cancel=false
JKG.GMH.VaultDoor.Password=o
end
end,HasCondition=false,HasResolver=false,},{Type=2,SubNode=36,NextNode=37,Text="The password has been changed to '$state$'",Duration=3e3,HasCondition=true,ConditionFunc=function(o,o,e)return not e.Cancel
end,HasResolver=true,ResolveFunc=function(o,o,e,o)if e=="state"then
return JKG.GMH.VaultDoor.Password
end
end,},{Type=4,Target=22,},{Type=4,Target=22,},{Type=3,SubNode=39,Text="Go back",HasCondition=false,HasResolver=false,},{Type=4,Target=2,},{Type=3,SubNode=41,Text="Leave",HasCondition=false,HasResolver=false,},{Type=5,},{Type=1,SubNode=43,HasCondition=false,},{Type=2,SubNode=44,Text="Access denied",Duration=1e3,HasCondition=false,HasResolver=false,},{Type=5,},}
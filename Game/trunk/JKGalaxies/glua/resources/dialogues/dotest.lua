DLG.Name="dotest"DLG.RootNode=1 DLG.Nodes={{Type=1,SubNode=2,HasCondition=false,},{Type=2,SubNode=3,Text="You are full of lulz!",Duration=3e3,HasCondition=false,HasResolver=false,},{Type=9,SubNode=4,NextNode=7,Host=3,SetupFunc=function(e,e,o,e)local e
for e=1,10 do
if e==2||e==5 then continue end
o(string.format("I wantz %i",e),e)end
end,ProcessFunc=function(n,n,o,e)e.Picked=o end,},{Type=2,SubNode=5,Text="You picked $num$!",Duration=2e3,HasCondition=false,HasResolver=true,ResolveFunc=function(n,n,o,e)if o=="num"then
return e.Picked
end
return""end,},{Type=2,SubNode=6,Text="Hmkai, bai!",Duration=2e3,HasCondition=false,HasResolver=false,},{Type=5,},{Type=3,SubNode=8,Text="I gotta go",HasCondition=false,HasResolver=false,},{Type=2,SubNode=9,Text="Hmkai!",Duration=1e3,HasCondition=false,HasResolver=false,},{Type=5,},}
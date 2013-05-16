NPC.NPCName = "weaponstest3"
 
function NPC:OnSpawn()
        print("Test NPC OnSpawn")
end
 
function NPC:OnInit(spawner)
        print("Test NPC OnInit (spawned by " .. tostring(spawner) .. ")" )
        self.DeathLootIndex = 5
end
 
function NPC:OnPain(attacker, damage)
        print(string.format("Test NPC OnPain (%i damage by %s)", damage, tostring(attacker)))
end
 
function NPC:OnDie(inflictor, attacker, damage, meansofdeath)
        print(string.format("Test NPC OnDie (Killed by %s, doing %i damage, using %i)", tostring(attacker), damage, meansofdeath))
end
 
function NPC:OnUse(other, activator)
        print(string.format("Test NPC OnUse (Used by %s)", tostring(activator)))
end
 
function NPC:OnThink()
        --print("Test NPC OnThink")
end
 
function NPC:OnTouch(other)
        print("Test NPC OnTouch (touched by " .. tostring(other) .. ")")
end
 
function NPC:OnRemove()
        print("Test NPC OnRemove")
end
 
function NPC:OnBlocked(blocker)
        print("Test NPC OnBlocked (blocked by " .. tostring(blocker) .. ")")
end
 
function NPC:OnAwake()
        print("Test NPC OnAwake")
end
 
function NPC:OnAnger(enemy)
        print("Test NPC OnAnger (targetted " .. tostring(enemy) .. ")")
end
 
function NPC:OnAttack()
        print("Test NPC OnAttack")
end
 
function NPC:OnVictory()
        print("Test NPC OnVictory")
end
 
function NPC:OnLostEnemy()
        print("Test NPC OnLostEnemy")
end
 
function NPC:OnMindTrick(user)
        print("Test NPC OnMindTrick (mindtricked by " .. tostring(user) .. ")")
end
 
function NPC:OnReached()
        print("Test NPC OnReached")
end
 
function NPC:OnStuck()
        print("Test NPC OnStuck")
end
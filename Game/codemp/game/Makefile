# Makefile for JKA

CC = gcc
CFLAGS = -DNDEBUG -DMISSION_PACK -DQAGAME -D_JK2 -fPIC -shared -m32 -masm=intel -O3 -static -shared -rdynamic -nostdlib -lgcc -Wall -fno-strict-aliasing -I../shared/ -I../libraries/ -I../libraries/libcurl/include/
LDFLAGS = -m32 -shared

TARGET = jampgamei386.so
# Game files
OBJECTS = ai_main.o ai_util.o ai_wpnav.o AnimalNPC.o bg_g2_utils.o bg_lib.o \
bg_misc.o bg_panimate.o bg_pmove.o bg_saber.o bg_saberLoad.o \
bg_saga.o bg_slidemove.o bg_vehicleLoad.o bg_weapons.o FighterNPC.o \
g_active.o g_arenas.o g_bot.o g_client.o g_cmds.o g_combat.o g_crash.o \
g_engine.o g_exphysics.o g_ICARUScb.o g_items.o g_log.o g_main.o \
g_mem.o g_misc.o g_missile.o g_mover.o g_nav.o g_navnew.o g_object.o \
g_saga.o g_session.o g_spawn.o g_strap.o g_svcmds.o g_syscalls.o \
g_target.o g_team.o g_timer.o g_trigger.o g_turret.o g_turret_G2.o \
g_utils.o g_vehicles.o g_vehicleTurret.o g_weapon.o jkg_accounts.o \
jkg_admin.o jkg_auxlib.o jkg_bans.o jkg_chatcmds.o jkg_custombounds.o \
jkg_dynarrays.o jkg_keypairs.o jkg_navmesh_creator.o jkg_playerisolation.o \
jkg_rconbans.o jkg_team.o jkg_threading.o jkg_utilityfunc.o NPC_AI_Atst.o \
NPC_AI_Default.o NPC_AI_Droid.o NPC_AI_GalakMech.o NPC_AI_Grenadier.o \
NPC_AI_Howler.o NPC_AI_ImperialProbe.o NPC_AI_Interrogator.o NPC_AI_Jedi.o \
NPC_AI_Mark1.o NPC_AI_Mark2.o NPC_AI_MineMonster.o NPC_AI_Rancor.o \
NPC_AI_Remote.o NPC_AI_Seeker.o NPC_AI_Sentry.o NPC_AI_Sniper.o \
NPC_AI_Stormtrooper.o NPC_AI_Utils.o NPC_AI_Wampa.o NPC_behavior.o \
NPC.o NPC_combat.o NPC_goal.o NPC_misc.o NPC_move.o NPC_reactions.o \
NPC_senses.o NPC_sounds.o NPC_spawn.o NPC_stats.o NPC_utils.o q_math.o \
q_shared.o SpeederNPC.o tri_coll_test.o WalkerNPC.o w_force.o w_saber.o

# GLua files
OBJECTS += ../GLua/glua_bitstream.o ../GLua/glua_bitwise.o \
../GLua/glua_cryptography.o ../GLua/glua_cvar.o ../GLua/glua_engine.o \
../GLua/glua_encoding.o ../GLua/glua_entity.o ../GLua/glua_files.o \
../GLua/glua_main.o ../GLua/glua_mem.o ../GLua/glua_npc.o \
../GLua/glua_player.o ../GLua/glua_sys.o ../GLua/glua_vector.o
 
# Base128 files
OBJECTS += ../libraries/encoding/base128.o

# DisASM files
OBJECTS += ../libraries/disasm/asmserv.o ../libraries/disasm/assembl.o \
../libraries/disasm/disasm.o

# cJSON files
OBJECTS += ../libraries/json/cJSON.o

# Lua files
OBJECTS += ../libraries/Lua/lapi.o ../libraries/Lua/lauxlib.o \
../libraries/Lua/lbaselib.o ../libraries/Lua/lcode.o \
../libraries/Lua/ldblib.o ../libraries/Lua/ldebug.o ../libraries/Lua/ldo.o \
../libraries/Lua/ldump.o ../libraries/Lua/lfunc.o ../libraries/Lua/lgc.o \
../libraries/Lua/linit.o ../libraries/Lua/liolib.o ../libraries/Lua/llex.o \
../libraries/Lua/lmathlib.o ../libraries/Lua/lmem.o \
../libraries/Lua/loadlib.o ../libraries/Lua/lobject.o \
../libraries/Lua/lopcodes.o ../libraries/Lua/loslib.o \
../libraries/Lua/lparser.o ../libraries/Lua/lstate.o \
../libraries/Lua/lstring.o ../libraries/Lua/lstrlib.o \
../libraries/Lua/ltable.o ../libraries/Lua/ltablib.o \
../libraries/Lua/ltm.o ../libraries/Lua/lundump.o ../libraries/Lua/lvm.o \
 ../libraries/Lua/lzio.o ../libraries/Lua/print.o

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJECTS)


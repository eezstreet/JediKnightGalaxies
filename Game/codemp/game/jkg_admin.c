/***********************************************
 *
 *	JKG Administration System 
 *
 *  This system assumes that the args are
 *  tokenized and available through Cmd_Argc 
 *  and Cmd_Argv
 *
 ***********************************************/

#include "g_local.h"
#include "jkg_admin.h"
#include "jkg_bans.h"

static const char	*NET_AdrToStringNoPort (netadr_t a)
{
	static	char	s[64];

	if (a.type == NA_LOOPBACK) {
		Com_sprintf (s, sizeof(s), "loopback");
	} else if (a.type == NA_BOT) {
		Com_sprintf (s, sizeof(s), "bot");
	} else if (a.type == NA_IP) {
		Com_sprintf (s, sizeof(s), "%i.%i.%i.%i",
			a.ip[0], a.ip[1], a.ip[2], a.ip[3]);
	} else {
		Com_sprintf (s, sizeof(s), "%02x%02x%02x%02x.%02x%02x%02x%02x%02x%02x",
		a.ipx[0], a.ipx[1], a.ipx[2], a.ipx[3], a.ipx[4], a.ipx[5], a.ipx[6], a.ipx[7], a.ipx[8], a.ipx[9]);
	}

	return s;
}

const char *adminRanks[] = { "None", "VIP Access", "Developer", "Lite Administrator", "Administrator", "Operator" };
const char *adminRanksShort[] = { "None", "VIP", "Dev", "LAdm", "Adm", "Op" }; /* Max len: 4 chars */

/* Includes from other sources */
int G_ClientNumberFromArg ( const char* name);

/*      Local defines          */

typedef void (* admCmdFunc_t)(gentity_t *ent, int clientNum, int rank);
typedef void (* admRconCmdFunc_t)(void);

typedef struct admCmd_s {
	const char			*name;		/* Command name */
	int					minRank;	/* Minimal admin rank required */
	admCmdFunc_t		func;		/* Function to call */
	
	struct admCmd_s		*next;		/* Next item in hashtable */
} admCmd_t;

typedef struct admRconCmd_s {
	const char			*name;		/* Command name */
	admRconCmdFunc_t	func;		/* Function to call */
	
	struct admRconCmd_s	*next;		/* Next item in hashtable */
} admRconCmd_t;

/* Convenience functions */
#ifdef _WIN32

static int __inline Cmd_Argc() {return *(int *)0x4DC188;}

static const char *Cmd_Argv( int arg ) {
	if ( /*(unsigned)*/arg >= Cmd_Argc() ) {
		return "";
	}
	return ((const char **)0x4D8D88)[arg];
}
#else

static int __inline Cmd_Argc() {return *(int *)0x8260E20;}

static const char *Cmd_Argv( int arg ) {
	if ( /*(unsigned)*/arg >= Cmd_Argc() ) {
		return "";
	}
	return ((const char **)0x8260E40)[arg];
}

#endif

void SanitizeString2( char *in, char *out );

static const char *SanitizeName( const char *name ) {
	static int idx = 0;
	static char clean[4][64];

	idx++;
	idx &= 3;
	SanitizeString2((char *)name, &clean[idx][0]);
	return clean[idx];
}

static const char	*Cmd_ConcatArgs( int start ) {
	int		i, c, tlen;
	static char	line[MAX_STRING_CHARS];
	int		len;
	const char *arg;

	len = 0;
	c = Cmd_Argc();
	for ( i = start ; i < c ; i++ ) {
		arg = Cmd_Argv(i);
		tlen = strlen( arg );
		if ( len + tlen >= MAX_STRING_CHARS - 1 ) {
			break;
		}
		memcpy( line + len, arg, tlen );
		len += tlen;
		if ( i != c - 1 ) {
			line[len] = ' ';
			len++;
		}
	}

	line[len] = 0;

	return line;
}

/*   Utility function: Response buffering  
|*
|* In situations where a response length varies
|* this system can be used.
|*
|* It works like a string-builder, where parts
|* of the response can be added to form the full response.
|*
|* Once the cache buffer fills up, the message so-far will
|* be sent to the player and the buffer is cleared.
|*
|* How to use:
|*
|* First, call InitBuffer, specifying the client to send the response to
|* Second, for all parts of the response, call AddToBuffer
|* Lastly, when the response is finished, call FlushBuffer to send out the remaining part
|*
|* NOTE: There is no trailing \n included, so dont forget to add one
\*/

static char RespBuffer[1024];
static int RespLen = 0;
static int RespTarget = -1;

static void InitBuffer(int clientNum)
{
	RespTarget = clientNum;
	RespLen = 0;
	RespBuffer[0] = 0;
}

static void FlushBuffer()
{
	if (RespTarget != -1) {
		trap_SendServerCommand(RespTarget, va("print \"%s\"", RespBuffer));
	}
	InitBuffer(-1);
}

static void AddToBuffer(const char *text)
{
	int len = strlen(text);
	if (len + RespLen > 960) {
		// Buffer is full
		if (RespTarget != -1) {
			trap_SendServerCommand(RespTarget, va("print \"%s\"", RespBuffer));
		}
		RespBuffer[0] = 0;
		RespLen = 0;
	}
	Q_strcat(&RespBuffer[RespLen], 1024 - RespLen, text);
	RespLen += len;
}

/* Forward declaration of the command arrays, in case a command wants to access it */
#ifdef _WIN32
// HACKHACKHACKHACKHACKHACK: this is just ew imo ~ Xycaleth
admCmd_t adminCmds[];
admRconCmd_t rconCmds[];
#else
static admCmd_t adminCmds[];
static admRconCmd_t rconCmds[];
#endif

/***************************************************************/
/*          Define admin commands below this point             */
/***************************************************************/

/******************************************************\
|* AmInfo
|*
|* Required rank: VIP access
|* Description:
|* Displays the current access level of the user
|* and the commands available to them
|*
|* Syntax: aminfo
\******************************************************/
static void AdmCmd_AmHelp(gentity_t *ent, int clientNum, int rank)
{
	// Show admin rank
	admCmd_t *cmd;

	InitBuffer(clientNum);
	AddToBuffer(va("^5Your administration/access rank is ^7%i ^5(^7%s^5)\n", rank, adminRanks[rank]));
	AddToBuffer("\n^5== Available commands ==\n\n^7");

	for (cmd = &adminCmds[0]; cmd->name; cmd++) {
		if (cmd->minRank > rank) continue;
		AddToBuffer(va("%s ",cmd->name));
	}
	AddToBuffer("\n\n^5========================\n");
	FlushBuffer();
}

/******************************************************\
|* AmPlayers
|*
|* Required rank: Developer
|* Description:
|* Shows a list of all currently present players
|* including their base stats and admin rank (short version)
|*
|* Syntax: amplayers
\******************************************************/
static void AdmCmd_AmPlayers(gentity_t *ent, int clientNum, int rank)
{
	// List all players on the server
	int i;
	InitBuffer(clientNum);
	AddToBuffer("\n^5== Player List ==\n\n^7");
	AddToBuffer(va("\nID | %-40s | Lvl | Rank | Ping | IP\n", "Player name"));
	AddToBuffer("---+------------------------------------------+-----+------+------+-----------------\n");

	for (i = 0; i < MAX_CLIENTS; i++) {
		if (level.clients[i].pers.connected == CON_CONNECTED) {
			// FIXME: This is broken
			//AddToBuffer(va("^5%-2i ^7| ^5%-40s ^7| ^5%-3i ^7| ^5%-4s ^7| ^5%-4i ^7| ^5%s\n", i, SanitizeName(level.clients[i].pers.netname), 0, adminRanksShort[level.clients[i].sess.adminRank], svs->clients[i].ping, NET_AdrToStringNoPort(svs->clients[i].netchan.remoteAddress) ));
		}
	}
	AddToBuffer("^7---+------------------------------------------+-----+------+------+-----------------\n\n");
	FlushBuffer();
}

/******************************************************\
|* AmKick
|*
|* Required rank: Lite Administrator
|* Description:
|* Kicks the specified client off the server
|*
|* Optionally a reason for the kick can be provided
|* 
|* Syntax: amkick <name/id> [reason] 
\******************************************************/
static void AdmCmd_AmKick(gentity_t *ent, int clientNum, int rank)
{
	int target;
	const char *reason;
	if (Cmd_Argc() < 2) {
		trap_SendServerCommand(clientNum, "print \"Syntax: amkick <name/id> [reason]\n\"");
		return;
	}
	target = G_ClientNumberFromArg(Cmd_Argv(1));
	if (target == -1) {
		trap_SendServerCommand(clientNum, va("print \"Cannot find player '%s'\n\"", Cmd_Argv(1)));
		return;
	} else if (target == -2) {
		trap_SendServerCommand(clientNum, "print \"Ambiguous name specified\n\"");
		return;
	}

	if (rank != ADMRANK_OPERATOR && level.clients[target].sess.adminRank > level.clients[clientNum].sess.adminRank) {
		trap_SendServerCommand(clientNum, va("print \"You are not allowed to use this command on %s\n\"", SanitizeName(level.clients[target].pers.netname)));
		return;
	}
	if (Cmd_Argc() > 2) {
		reason = Cmd_ConcatArgs(2);
		G_LogPrintf("Admin: %s has kicked %s (%s)\n", SanitizeName(ent->client->pers.netname), SanitizeName(level.clients[target].pers.netname), reason); 
		trap_DropClient(target, va("was kicked: %s", reason));
	} else {
		G_LogPrintf("Admin: %s has kicked %s\n", SanitizeName(ent->client->pers.netname), SanitizeName(level.clients[target].pers.netname));
#ifndef __MMO__ // UQ1: clear up some spam when kicking mass bots. Could add bot check if we really need this to display...
		trap_DropClient(target, "was kicked");
#endif //__MMO__
	}
}

/******************************************************\
|* AmSabotage
|*
|* Required rank: Administrator
|* Description:
|* Enables the self-sabotage mechanism on
|* the specified player.
|*
|* This will cause the client to crash randomly with a
|* packet parsing error. Once the error has occoured,
|* the sabotage mechanism will be deactivated again.
|*
|* WARNING: This *cannot* be disabled once activated!
|*
|* !! DO NOT ABUSE !!
|* 
|* Syntax: amsabotage <name/id> 
\******************************************************/
static void AdmCmd_AmSabotage(gentity_t *ent, int clientNum, int rank)
{
	int target;
	if (Cmd_Argc() < 2) {
		trap_SendServerCommand(clientNum, "print \"Syntax: amsabotage <name/id>\n\"");
		return;
	}
	target = G_ClientNumberFromArg(Cmd_Argv(1));
	if (target == -1) {
		trap_SendServerCommand(clientNum, va("print \"Cannot find player '%s'\n\"", Cmd_Argv(1)));
		return;
	} else if (target == -2) {
		trap_SendServerCommand(clientNum, "print \"Ambiguous name specified\n\"");
		return;
	}

	if (rank != ADMRANK_OPERATOR && level.clients[target].sess.adminRank > level.clients[clientNum].sess.adminRank) {
		trap_SendServerCommand(clientNum, va("print \"You are not allowed to use this command on %s\n\"", SanitizeName(level.clients[target].pers.netname)));
		return;
	}
	G_LogPrintf("Admin: %s has activaged %s's self-sabotage mechanism\n", SanitizeName(ent->client->pers.netname), SanitizeName(level.clients[target].pers.netname));
	trap_SendServerCommand(target, "ss 2");
	trap_SendServerCommand(clientNum, va("print \"The self-sabotage mechanism on player %s has been activated\n\"", SanitizeName(level.clients[target].pers.netname)));
}

/******************************************************\
|* AmGrantVIP
|*
|* Required rank: Administrator
|* Description:
|* Grants the specified player temporary VIP access
|* 
|* Syntax: amgrantvip <name/id> 
\******************************************************/
static void AdmCmd_AmGrantVIP(gentity_t *ent, int clientNum, int rank)
{
	int target;
	if (Cmd_Argc() < 2) {
		trap_SendServerCommand(clientNum, "print \"Syntax: amgrantvip <name/id>\n\"");
		return;
	}
	target = G_ClientNumberFromArg(Cmd_Argv(1));
	if (target == -1) {
		trap_SendServerCommand(clientNum, va("print \"Cannot find player '%s'\n\"", Cmd_Argv(1)));
		return;
	} else if (target == -2) {
		trap_SendServerCommand(clientNum, "print \"Ambiguous name specified\n\"");
		return;
	}

	if (level.clients[target].sess.adminRank == ADMRANK_VIP) {
		trap_SendServerCommand(clientNum, va("print \"%s already has VIP access\n\"", SanitizeName(level.clients[target].pers.netname)));
		return;
	} else if (level.clients[target].sess.adminRank > ADMRANK_VIP) {
		trap_SendServerCommand(clientNum, va("print \"%s cannot be given VIP access\n\"", SanitizeName(level.clients[target].pers.netname)));
		return;
	} 
	G_LogPrintf("Admin: %s has granted VIP access to %s\n", SanitizeName(ent->client->pers.netname), SanitizeName(level.clients[target].pers.netname));
	level.clients[target].sess.adminRank = ADMRANK_VIP;
	trap_SendServerCommand(clientNum, va("print \"%s has been given VIP access\n\"", SanitizeName(level.clients[target].pers.netname)));
}

/******************************************************\
|* AmRevokeVIP
|*
|* Required rank: Administrator
|* Description:
|* Revokes the specified player's VIP access
|* 
|* Syntax: amrevokevip <name/id> 
\******************************************************/
static void AdmCmd_AmRevokeVIP(gentity_t *ent, int clientNum, int rank)
{
	int target;
	if (Cmd_Argc() < 2) {
		trap_SendServerCommand(clientNum, "print \"Syntax: amgrantvip <name/id>\n\"");
		return;
	}
	target = G_ClientNumberFromArg(Cmd_Argv(1));
	if (target == -1) {
		trap_SendServerCommand(clientNum, va("print \"Cannot find player '%s'\n\"", Cmd_Argv(1)));
		return;
	} else if (target == -2) {
		trap_SendServerCommand(clientNum, "print \"Ambiguous name specified\n\"");
		return;
	}

	if (level.clients[target].sess.adminRank != ADMRANK_VIP) {
		trap_SendServerCommand(clientNum, va("print \"%s does not have VIP access\n\"", SanitizeName(level.clients[target].pers.netname)));
		return;
	} 

	G_LogPrintf("Admin: %s has revoked %s's VIP access\n", SanitizeName(ent->client->pers.netname), SanitizeName(level.clients[target].pers.netname));
	level.clients[target].sess.adminRank = 0;
	trap_SendServerCommand(clientNum, va("print \"%s's VIP access has been revoked\n\"", SanitizeName(level.clients[target].pers.netname)));
}

/******************************************************\
|* AmTele
|*
|* Required rank: Developers
|* Description:
|* Teleports yourself or other players.
|* 
|* Possible ways to use:
|* - No arguments: Teleports yourself to your reticule
|* - 1 argument: Teleports yourself to the specified player
|* - 2 arguments: Teleports client 1 to client 2
|* - 3 arguments: Teleports you to the specified coordinates
|* - 4 arguments: Teleports client 1 to the specified coordinates
|*
|* NOTE: Developers cannot teleport other players,
|*       only themselves!
|*
|* Syntax: amtele [client1] [client2 or coordinates]
\******************************************************/
qboolean SpotWouldTelefrag2( gentity_t *mover, vec3_t dest );
static void AdmCmd_AmTele(gentity_t *ent, int clientNum, int rank)
{
	int client1;
	gentity_t* ent1;
	int client2;
	gentity_t* ent2;
	vec3_t src;
	vec3_t dest;
	vec3_t vf;
	vec3_t target;
	vec3_t angs;
	trace_t tr;

	switch (Cmd_Argc() - 1) {
		case 0:
			// Travel to the reticule
			VectorCopy(ent->client->ps.origin, src);
			src[2] += ent->client->ps.viewheight;
			AngleVectors( ent->client->ps.viewangles, vf, NULL, NULL );
			VectorMA( src, 131072, vf, dest );
			trap_Trace( &tr, src, vec3_origin, vec3_origin, dest, ent->s.number, MASK_OPAQUE|CONTENTS_SOLID|CONTENTS_BODY|CONTENTS_ITEM|CONTENTS_CORPSE );
			VectorMA ( tr.endpos, 32, tr.plane.normal, target );

			if (SpotWouldTelefrag2(ent, target)) {
				trap_SendServerCommand(clientNum, "print \"Target area blocked\n\"");
				return;
			}
			G_LogPrintf("Admin: %s has teleported to %i %i %i\n", SanitizeName(ent->client->pers.netname), target[0], target[1], target[2]);
			TeleportPlayer2(ent, target, ent->client->ps.viewangles);
			return;
		case 1:
			// Teleport self to target player
			client1 = G_ClientNumberFromArg(Cmd_Argv(1));
			if (client1 == -1) {
				trap_SendServerCommand(clientNum, va("print \"Cannot find player '%s'\n\"", Cmd_Argv(1)));
				return;
			} else if (client1 == -2) {
				trap_SendServerCommand(clientNum, "print \"Ambiguous name specified\n\"");
				return;
			}

			ent1 = &g_entities[client1];
			VectorCopy(ent1->client->ps.origin, src);
			src[2] += ent1->client->ps.viewheight;
			AngleVectors( ent1->client->ps.viewangles, vf, NULL, NULL );
			vf[PITCH] = vf[ROLL] = 0;
			VectorMA( src, 48, vf, target );
			target[2] += 48;

			if (SpotWouldTelefrag2(ent, target)) {
				trap_SendServerCommand(clientNum, "print \"Target area blocked\n\"");
				return;
			}
			VectorCopy(ent1->client->ps.viewangles, angs);
			angs[YAW] -= 180;
			G_LogPrintf("Admin: %s has teleported to %s (at %i %i %i)\n", SanitizeName(ent->client->pers.netname), SanitizeName(ent1->client->pers.netname), target[0], target[1], target[2]);
			TeleportPlayer2(ent, target, angs);
			return;
		case 2:
			// Teleport client 1 to client 2
			client1 = G_ClientNumberFromArg(Cmd_Argv(1));
			if (client1 == -1) {
				trap_SendServerCommand(clientNum, va("print \"Cannot find player '%s'\n\"", Cmd_Argv(1)));
				return;
			}  else if (client1 == -2) {
				trap_SendServerCommand(clientNum, "print \"Ambiguous name specified for client 1\n\"");
				return;
			}

			ent1 = &g_entities[client1];

			client2 = G_ClientNumberFromArg(Cmd_Argv(2));
			if (client2 == -1) {
				trap_SendServerCommand(clientNum, va("print \"Cannot find player '%s'\n\"", Cmd_Argv(1)));
				return;
			}  else if (client2 == -2) {
				trap_SendServerCommand(clientNum, "print \"Ambiguous name specified for client 2\n\"");
				return;
			}
			ent2 = &g_entities[client2];

			if (client1 != clientNum && rank == ADMRANK_DEVELOPER) {
				trap_SendServerCommand(clientNum, "print \"You are not allowed to teleport other players\n\"");
				return;
			}


			VectorCopy(ent2->client->ps.origin, src);
			src[2] += ent2->client->ps.viewheight;
			AngleVectors( ent2->client->ps.viewangles, vf, NULL, NULL );
			vf[PITCH] = vf[ROLL] = 0;
			VectorMA( src, 48, vf, target );
			target[2] += 48;

			if (SpotWouldTelefrag2(ent1, target)) {
				trap_SendServerCommand(clientNum, "print \"Target area blocked\n\"");
				return;
			}
			VectorCopy(ent2->client->ps.viewangles, angs);
			angs[YAW] -= 180;
			G_LogPrintf("Admin: %s has teleported %s to %s (at %i %i %i)\n", SanitizeName(ent->client->pers.netname), SanitizeName(ent1->client->pers.netname), SanitizeName(ent2->client->pers.netname), target[0], target[1], target[2]);
			TeleportPlayer2(ent1, target, angs);
			return;
		case 3:
			// Teleport self to coordinates
			target[0] = atof(Cmd_Argv(1));
			target[1] = atof(Cmd_Argv(2));
			target[2] = atof(Cmd_Argv(3));

			if (SpotWouldTelefrag2(ent, target)) {
				trap_SendServerCommand(clientNum, "print \"Target area blocked\n\"");
				return;
			}
			G_LogPrintf("Admin: %s has teleported to %i %i %i)\n", SanitizeName(ent->client->pers.netname), target[0], target[1], target[2]);
			TeleportPlayer2(ent, target, ent->client->ps.viewangles);
			return;
		case 4:
			// Teleport client to coordinates
			client1 = G_ClientNumberFromArg(Cmd_Argv(1));
			if (client1 == -1) {
				trap_SendServerCommand(clientNum, va("print \"Cannot find player '%s'\n\"", Cmd_Argv(1)));
				return;
			} else if (client1 == -2) {
				trap_SendServerCommand(clientNum, "print \"Ambiguous name specified\n\"");
				return;
			}

			ent1 = &g_entities[client1];

			if (client1 != clientNum && rank == ADMRANK_DEVELOPER) {
				trap_SendServerCommand(clientNum, "print \"You are not allowed to teleport other players\n\"");
				return;
			}

			target[0] = atof(Cmd_Argv(2));
			target[1] = atof(Cmd_Argv(3));
			target[2] = atof(Cmd_Argv(4));

			if (SpotWouldTelefrag2(ent1, target)) {
				trap_SendServerCommand(clientNum, "print \"Target area blocked\n\"");
				return;
			}
			G_LogPrintf("Admin: %s has teleported %s to %i %i %i\n", SanitizeName(ent->client->pers.netname), SanitizeName(ent1->client->pers.netname), target[0], target[1], target[2]);
			TeleportPlayer2(ent1, target, ent1->client->ps.viewangles);
			return;
		default:
			trap_SendServerCommand(clientNum, "print \"Syntax: amtele [client 1] [client 2 or coordinates]\n\"");
			return;
	}
}

/******************************************************\
|* AmGod
|*
|* Required rank: Administrator
|* Description:
|* Grants of revokes god-mode
|* 
|* Syntax: amgod [name/id]
\******************************************************/
static void AdmCmd_AmGod(gentity_t *ent, int clientNum, int rank)
{
	const char *msg;
	gentity_t *targ;
	int target;
	if (Cmd_Argc() < 2) {
		target = clientNum;
	} else {
		target = G_ClientNumberFromArg(Cmd_Argv(1));
		if (target == -1) {
			trap_SendServerCommand(clientNum, va("print \"Cannot find player '%s'\n\"", Cmd_Argv(1)));
			return;
		}  else if (target == -2) {
			trap_SendServerCommand(clientNum, "print \"Ambiguous name specified\n\"");
			return;
		}
	}
	targ = &g_entities[target];
	targ->flags ^= FL_GODMODE;
	if (!(targ->flags & FL_GODMODE) )
		msg = "God-mode disabled";
	else
		msg = "God-mode enabled";

	G_LogPrintf("Admin: %s has %s %s\n", SanitizeName(ent->client->pers.netname), targ->flags & FL_GODMODE ? "granted god-mode to" : "revoked god-mode from", SanitizeName(level.clients[target].pers.netname));
	
	trap_SendServerCommand(clientNum, va("print \"%s (%s)\n\"", msg, SanitizeName(level.clients[target].pers.netname)));
}

/******************************************************\
|* AmNoClip
|*
|* Required rank: Administrator
|* Description:
|* Grants of revokes noclip
|* 
|* Syntax: amnoclip [name/id]
\******************************************************/
static void AdmCmd_AmNoClip(gentity_t *ent, int clientNum, int rank)
{
	const char *msg;
	gentity_t *targ;
	int target;
	if (Cmd_Argc() < 2) {
		target = clientNum;
	} else {
		target = G_ClientNumberFromArg(Cmd_Argv(1));
		if (target == -1) {
			trap_SendServerCommand(clientNum, va("print \"Cannot find player '%s'\n\"", Cmd_Argv(1)));
			return;
		} else if (target == -2) {
			trap_SendServerCommand(clientNum, "print \"Ambiguous name specified\n\"");
			return;
		}
	}
	targ = &g_entities[target];
	targ->client->noclip = !targ->client->noclip;
	if (!(targ->client->noclip) )
		msg = "No-clip disabled";
	else
		msg = "No-clip enabled";

	G_LogPrintf("Admin: %s has %s %s\n", SanitizeName(ent->client->pers.netname), targ->client->noclip ? "granted no-clip to" : "revoked no-clip from", SanitizeName(level.clients[target].pers.netname));

	trap_SendServerCommand(clientNum, va("print \"%s (%s)\n\"", msg, SanitizeName(level.clients[target].pers.netname)));
}

/******************************************************\
|* AmNoTarget
|*
|* Required rank: Administrator
|* Description:
|* Grants of revokes notarget
|* 
|* Syntax: amnoclip [name/id]
\******************************************************/
static void AdmCmd_AmNoTarget(gentity_t *ent, int clientNum, int rank)
{
	const char *msg;
	gentity_t *targ;
	int target;
	if (Cmd_Argc() < 2) {
		target = clientNum;
	} else {
		target = G_ClientNumberFromArg(Cmd_Argv(1));
		if (target == -1) {
			trap_SendServerCommand(clientNum, va("print \"Cannot find player '%s'\n\"", Cmd_Argv(1)));
			return;
		} else if (target == -2) {
			trap_SendServerCommand(clientNum, "print \"Ambiguous name specified\n\"");
			return;
		}
	}
	targ = &g_entities[target];
	targ->flags ^= FL_NOTARGET;
	if (!(targ->flags & FL_NOTARGET) )
		msg = "No-target disabled";
	else
		msg = "No-target enabled";

	G_LogPrintf("Admin: %s has %s %s\n", SanitizeName(ent->client->pers.netname), targ->flags & FL_NOTARGET ? "granted no-target to" : "revoked no-target from", SanitizeName(level.clients[target].pers.netname));

	trap_SendServerCommand(clientNum, va("print \"%s (%s)\n\"", msg, SanitizeName(level.clients[target].pers.netname)));
}

/******************************************************\
|* AmSilence
|*
|* Required rank: Lite Administrator
|* Description:
|* Silences (or unsilences) the specified player
|* (Makes him unable to chat)
|* 
|* Syntax: amsilence <name/id>
\******************************************************/
static void AdmCmd_AmSilence(gentity_t *ent, int clientNum, int rank)
{
	const char *msg;
	gentity_t *targ;
	int target;
	if (Cmd_Argc() < 2) {
		trap_SendServerCommand(clientNum, "print \"Syntax: amsilence <name/id>\n\"");
		return;
	}
	target = G_ClientNumberFromArg(Cmd_Argv(1));
	if (target == -1) {
		trap_SendServerCommand(clientNum, va("print \"Cannot find player '%s'\n\"", Cmd_Argv(1)));
		return;
	} else if (target == -2) {
		trap_SendServerCommand(clientNum, "print \"Ambiguous name specified\n\"");
		return;
	}

	targ = &g_entities[target];
	targ->client->pers.silenced = !targ->client->pers.silenced;
	if (targ->client->pers.silenced)
		msg = "has been silenced";
	else
		msg = "has been unsilenced";


	G_LogPrintf("Admin: %s has %s %s\n", SanitizeName(ent->client->pers.netname), targ->client->pers.silenced ? "silenced" : "un-silenced", SanitizeName(level.clients[target].pers.netname));

	trap_SendServerCommand(clientNum, va("print \"%s %s\n\"", SanitizeName(level.clients[target].pers.netname), msg));
}

/******************************************************\
|* AmBan
|*
|* Required rank: Administrator
|* Description:
|* (Temporarily) bans the specified person.
|* 
|* Syntax: amban <name/id> <duration> [reason]
|*
|* Duration syntax:
|* <amount><specifier> (ie. 12h for a 12 hour ban)
|*
|* Valid specifiers:
|*
|* m = Minutes
|* h = Hours
|* d = Days
|* n = Months
|* y = Years
|*
|* Note: A duration of '0' makes the ban permament!
\******************************************************/
static void AdmCmd_AmBan(gentity_t *ent, int clientNum, int rank)
{
	// FIXME: This is broken
	/*int target;
	int ret;
	const char *reason;

	if (Cmd_Argc() < 3) {
		trap_SendServerCommand(clientNum, "print \"Syntax: amban <name/id> <duration> [reason]\n\"");
		return;
	}
	target = G_ClientNumberFromArg(Cmd_Argv(1));
	if (target == -1) {
		trap_SendServerCommand(clientNum, va("print \"Cannot find player '%s'\n\"", Cmd_Argv(1)));
		return;
	} else if (target == -2) {
		trap_SendServerCommand(clientNum, "print \"Ambiguous name specified\n\"");
		return;
	}

	if (rank != ADMRANK_OPERATOR && level.clients[target].sess.adminRank > level.clients[clientNum].sess.adminRank) {
		trap_SendServerCommand(clientNum, va("print \"You are not allowed to use this command on %s\n\"", SanitizeName(level.clients[target].pers.netname)));
		return;
	}
	if (Cmd_Argc() > 3) {
		reason = Cmd_ConcatArgs(3);
		ret = JKG_Bans_AddBan(svs->clients[target].netchan.remoteAddress, Cmd_Argv(2), reason);
		if (ret == -1) {
			trap_SendServerCommand(clientNum, va("print \"Could not ban %s\n\"", SanitizeName(level.clients[target].pers.netname)));
			return;
		}
		G_LogPrintf("Admin: %s has banned %s, duration: '%s' (%s) - Ban ID: %i\n", SanitizeName(ent->client->pers.netname), SanitizeName(level.clients[target].pers.netname), Cmd_Argv(2), reason, ret); 
		trap_DropClient(target, va("was banned: %s", reason));
		trap_SendServerCommand(clientNum, va("print \"Player %s has been banned. Ban id: %i\n\"", SanitizeName(level.clients[target].pers.netname), ret));
	} else {
		ret = JKG_Bans_AddBan(svs->clients[target].netchan.remoteAddress, Cmd_Argv(2), NULL);
		if (ret == -1) {
			trap_SendServerCommand(clientNum, va("print \"Could not ban %s\n\"", SanitizeName(level.clients[target].pers.netname)));
			return;
		}
		G_LogPrintf("Admin: %s has banned %s, duration: '%s' - Ban ID: %i\n", SanitizeName(ent->client->pers.netname), SanitizeName(level.clients[target].pers.netname), Cmd_Argv(2), ret);
		trap_DropClient(target, "was banned");
		trap_SendServerCommand(clientNum, va("print \"Player %s has been banned. Ban id: %i\n\"", SanitizeName(level.clients[target].pers.netname), ret));
	}*/
}

/******************************************************\
|* AmUnBan
|*
|* Required rank: Administrator
|* Description:
|* Unbans a previously banned IP.
|* 
|* Syntax: amunban <ban id>
\******************************************************/
static void AdmCmd_AmUnBan(gentity_t *ent, int clientNum, int rank)
{
	qboolean ret;
	char ip[16];
	char reason[64];
	char duration[32];
	unsigned int id;
	const char *unbanreason;

	if (Cmd_Argc() < 2) {
		trap_SendServerCommand(clientNum, "print \"Syntax: amunban <ban id> [reason]\n\"");
		return;
	}
	id = atoi(Cmd_Argv(1));
	if (Cmd_Argc() > 2) {
		unbanreason = Cmd_ConcatArgs(2);
	} else {
		unbanreason = "Not specified";
	}

	ret = JKG_Bans_GetBanInfo(id, ip, sizeof(ip), duration, sizeof(duration), reason, sizeof(reason));
	if (!ret) {
		trap_SendServerCommand(clientNum, "print \"Invalid ban ID specified\n\"");
	}

	ret = JKG_Bans_RemoveBan(id);
	if (ret) {
		G_LogPrintf("Admin: %s has unbanned %s, reason: %s. Ban info: Duration: %s - Ban ID: %i- Ban reason: %s\n", SanitizeName(ent->client->pers.netname), ip, unbanreason, duration, id, reason);
		trap_SendServerCommand(clientNum, "print \"Ban successfully lifted\n\"");
		return;
	} else {
		trap_SendServerCommand(clientNum, "print \"Could not lift ban\n\"");
	}
}

#ifdef _DEBUG
//NOTENOTE: Not in final release
/******************************************************\
|* AmLoot
|*
|* Required rank: Administrator
|* Description:
|* Generates an item with quality, and random properties where applicable
|* 
|* Syntax: amloot <itemid> <quality>
\******************************************************/
extern itemData_t itemLookupTable[MAX_ITEM_TABLE_SIZE];
static void AdmCmd_AmLoot(gentity_t *ent, int clientNum, int rank)
{
	unsigned int id;
	unsigned int quality;
	itemInstance_t item;
	int i = 0, j = 0;

	if (Cmd_Argc() < 3) 
	{
		trap_SendServerCommand(clientNum, "print \"Syntax: amloot <itemid> <quality>\n\"");
		return;
	}
	if(level.clients[clientNum].pers.connected != CON_CONNECTED)
	{
		trap_SendServerCommand(clientNum, "print \"You are not connected to a server.\n\"");
		return;
	}

	id = atoi(Cmd_Argv(1));
	quality = atoi(Cmd_Argv(2));

    memset (&item, 0, sizeof (item));
	item.id = &itemLookupTable[id];
	item.itemQuality = quality;

	if((ent->client->coreStats.weight - item.id->weight) <= 0)
	{
		trap_SendServerCommand(clientNum, "print \"You cannot carry any more items.\n\"");
		return;
	}

	while(ent->inventory->items[i].id)
	{
	    i++;
	}

	ent->inventory->items[i] = item;

	G_LogPrintf("Admin: %s /amloot %s [Item ID %d quality %d]", ent->s.number, item.id->displayName, item.id->itemID, item.itemQuality);
}


#endif

/***************************************************************/
/*           Define rcon commands below this point             */
/***************************************************************/

/* Returns the IP of the currently executing Rcon command 
|* If the command is executed by the console, the response will be 'console'
\*/

static const char *NET_RconAdrToString()
{
	netadr_t a;
	void *flushfunc;
	static	char	s[64];

	/* Determine if this is a console command or not by checking rd_flush */

#ifdef __linux__
	flushfunc = *(void **)0x81E90C8;
	if (!flushfunc) {
		Com_sprintf(s, sizeof(s), "console");
		return s;
	} else {
		a = *(netadr_t *)0x831C200;
	}
#else
	flushfunc = *(void **)0x4DC73C;
	if (!flushfunc) {
		Com_sprintf(s, sizeof(s), "console");
		return s;
	} else {
		a = *(netadr_t *)0x610238;
	}
#endif

	if (a.type == NA_LOOPBACK) {
		Com_sprintf (s, sizeof(s), "loopback");
	} else if (a.type == NA_BOT) {
		Com_sprintf (s, sizeof(s), "bot");
	} else if (a.type == NA_IP) {
		Com_sprintf (s, sizeof(s), "%i.%i.%i.%i:%hu",
			a.ip[0], a.ip[1], a.ip[2], a.ip[3], BigShort(a.port));
	} else {
		Com_sprintf (s, sizeof(s), "%02x%02x%02x%02x.%02x%02x%02x%02x%02x%02x:%hu",
		a.ipx[0], a.ipx[1], a.ipx[2], a.ipx[3], a.ipx[4], a.ipx[5], a.ipx[6], a.ipx[7], a.ipx[8], a.ipx[9], 
		BigShort(a.port));
	}

	return s;

}

/******************************************************\
|* AmGrant
|*
|* Description:
|* Changes the admin rank of the specified client
|*
|* The rank is to be provided in numeric form, where
|* 0 is None, and 5 is Operator.
|* 
|* Syntax: amgrant <name/id> <rank>
\******************************************************/
static void RconCmd_AmGrant(void)
{
	int target;
	int rank;
	
	if (Cmd_Argc() < 3) {
		G_Printf("Syntax: amgrant <name/id> <rank>\n");
		return;
	}
	target = G_ClientNumberFromArg(Cmd_Argv(1));
	if (target == -1) {
		G_Printf("Cannot find player '%s'\n", Cmd_Argv(1));
		return;
	}
	rank = atoi(Cmd_Argv(2));

	if (rank < ADMRANK_NONE || rank > ADMRANK_OPERATOR) {
		G_Printf("Invalid rank specified\n");
		return;
	}

	G_LogPrintf("RconAdmin: (%s) %s has been granted %s rank\n", NET_RconAdrToString(), SanitizeName(level.clients[target].pers.netname), adminRanks[rank]);

	level.clients[target].sess.adminRank = rank;
	G_Printf("Player %i (%s) now has admin rank %s\n", target, SanitizeName(level.clients[target].pers.netname), adminRanks[rank]);
}

/******************************************************\
|* AmSpeak
|*
|* Description:
|* Talks via chat
|*
|* 
|* Syntax: amspeak "message"
\******************************************************/
static void RconCmd_AmSpeak(void)
{
	char message[1024];
	
	if (Cmd_Argc() < 2) {
		G_Printf("Syntax: amspeak \"message\"\n");
		return;
	}
	
	strcpy(message, ConcatArgs(1));
	trap_SendServerCommand(-1, va("chat 100 \"^3[SERVER] %s\"", message));
}

/******************************************************\
|* AmCP
|*
|* Description:
|* Centerprint.
|*
|* 
|* Syntax: amcp "message"
\******************************************************/
static void RconCmd_AmCP(void)
{
	char message[1024];
	
	if (Cmd_Argc() < 2) {
		G_Printf("Syntax: amcp \"message\"\n");
		return;
	}
	
	strcpy(message, Cmd_Argv(1));
	trap_SendServerCommand(-1, va("cp \"%s\"", message));
}


/******************************************************\
|* AmBan
|*
|* Description:
|* (Temporarily) bans the specified person.
|* 
|* Syntax: amban <name/id> <duration> [reason]
|*
|* Duration syntax:
|* <amount><specifier> (ie. 12h for a 12 hour ban)
|*
|* Valid specifiers:
|*
|* m = Minutes
|* h = Hours
|* d = Days
|* n = Months
|* y = Years
|*
|* Note: A duration of '0' makes the ban permament!
\******************************************************/
static void RconCmd_AmBan(void)
{
	/*int target;
	const char *reason;
	int ret;
	
	if (Cmd_Argc() < 3) {
		G_Printf("Syntax: amban <name/id> <duration> [reason]\n");
		return;
	}
	target = G_ClientNumberFromArg(Cmd_Argv(1));
	if (target == -1) {
		G_Printf("Cannot find player '%s'\n", Cmd_Argv(1));
		return;
	} else if (target == -2) {
		G_Printf("Ambiguous name specified\n");
		return;
	}

	if (Cmd_Argc() > 3) {
		reason = Cmd_ConcatArgs(3);
		ret = JKG_Bans_AddBan(svs->clients[target].netchan.remoteAddress, Cmd_Argv(2), reason);
		if (ret == -1) {
			G_Printf("Could not ban %s\n", SanitizeName(level.clients[target].pers.netname));
			return;
		}
		G_LogPrintf("RconAdmin: (%s) %s has been banned, duration: '%s' (%s) - Ban ID: %i\n", NET_RconAdrToString(), SanitizeName(level.clients[target].pers.netname), Cmd_Argv(2), reason, ret); 
		trap_DropClient(target, va("was banned: %s", reason));
		G_Printf("Player %s has been banned. Ban id: %i\n", SanitizeName(level.clients[target].pers.netname), ret);
	} else {
		ret = JKG_Bans_AddBan(svs->clients[target].netchan.remoteAddress, Cmd_Argv(2), NULL);
		if (ret == -1) {
			G_Printf("Could not ban %s\n", SanitizeName(level.clients[target].pers.netname));
			return;
		}

		G_LogPrintf("RconAdmin: (%s) %s has been banned, duration: '%s' - Ban ID: %i\n", NET_RconAdrToString(), SanitizeName(level.clients[target].pers.netname), Cmd_Argv(2), ret);
		trap_DropClient(target, "was banned");
		G_Printf("Player %s has been banned. Ban id: %i\n", SanitizeName(level.clients[target].pers.netname), ret);
	}*/
}

/******************************************************\
|* AmBanIP
|*
|* Description:
|* (Temporarily) bans the specified IP.
|* 
|* Syntax: amban <ip address> <duration> [reason]
|*
|* IP addresses can contain * as wildcards to ban IP ranges
|* (ie. 123.213.92.*)
|*
|* Duration syntax:
|* <amount><specifier> (ie. 12h for a 12 hour ban)
|*
|* Valid specifiers:
|*
|* m = Minutes
|* h = Hours
|* d = Days
|* n = Months
|* y = Years
|*
|* Note: A duration of '0' makes the ban permament!
\******************************************************/
static void RconCmd_AmBanIP(void)
{
	const char *ip;
	const char *reason;
	int ret;
	
	if (Cmd_Argc() < 3) {
		G_Printf("Syntax: ambanip <ip address> <duration> [reason]\n");
		return;
	}
	
	ip = Cmd_Argv(1);

	if (Cmd_Argc() > 3) {
		reason = Cmd_ConcatArgs(3);
		ret = JKG_Bans_AddBanString(ip, Cmd_Argv(2), reason);
		if (ret == -1) {
			G_Printf("Could not ban %s\n", ip);
			return;
		}
		G_LogPrintf("RconAdmin: (%s) the IP %s has been banned, duration: '%s' (%s) - Ban ID: %i\n", NET_RconAdrToString(), ip, Cmd_Argv(2), reason, ret); 
		G_Printf("IP %s has been banned. Ban id: %i\n", ip, ret);
	} else {
		ret = JKG_Bans_AddBanString(ip, Cmd_Argv(2), NULL);
		if (ret == -1) {
			G_Printf("Could not ban %s\n", ip);
			return;
		}

		G_LogPrintf("RconAdmin: (%s) the IP %s has been banned, duration: '%s' - Ban ID: %i\n", NET_RconAdrToString(), ip, Cmd_Argv(2), ret);
		G_Printf("IP %s has been banned. Ban id: %i\n", ip, ret);
	}
}

/******************************************************\
|* AmUnBan
|*
|* Description:
|* Unbans a previously banned IP.
|* 
|* Syntax: amunban <ban id>
\******************************************************/
static void RconCmd_AmUnBan(void)
{
	qboolean ret;
	char ip[16];
	char reason[64];
	char duration[32];
	unsigned int id;
	const char *unbanreason;

	if (Cmd_Argc() < 2) {
		G_Printf("Syntax: amunban <id> [reason]\n");
		return;
	}
	id = atoi(Cmd_Argv(1));
	if (Cmd_Argc() > 2) {
		unbanreason = Cmd_ConcatArgs(2);
	} else {
		unbanreason = "Not specified";
	}

	ret = JKG_Bans_GetBanInfo(id, ip, sizeof(ip), duration, sizeof(duration), reason, sizeof(reason));
	if (!ret) {
		G_Printf("Invalid ban ID specified\n");
	}

	ret = JKG_Bans_RemoveBan(id);
	if (ret) {
		G_LogPrintf("RconAdmin: (%s) %s has been unbanned, reason: %s. Ban info: Duration: %s - Ban ID: %i- Ban reason: %s\n",  NET_RconAdrToString(), ip, unbanreason, duration, id, reason);
		G_Printf("Ban successfully lifted\n");
		return;
	} else {
		G_Printf("Could not lift ban\n");
	}
}

/******************************************************\
|* AmListBans
|*
|* Description:
|* Lists all bans matching a specified mask
|* 
|* Syntax: amlistbans [ban mask]
|*
|* If no banmask is provided, *.*.*.* will be used.
\******************************************************/
static void RconCmd_AmListBans(void)
{
	if (Cmd_Argc() < 2) {
		JKG_Bans_ListBans("*.*.*.*");
	} else {
		JKG_Bans_ListBans(Cmd_Argv(1));
	}	
}

/******************************************************\
|* AmClearBans
|*
|* Description:
|* Clears all bans (Use with caution!)
|* 
|* Syntax: amclearbans
\******************************************************/
static void RconCmd_AmClearBans(void)
{
	JKG_Bans_Clear();
}

/******************************************************/
/*        Define rcon/admin command tables            */
/******************************************************/

static admCmd_t adminCmds[] = {
	{"amhelp",		ADMRANK_VIP,		AdmCmd_AmHelp},
	{"amkick",		ADMRANK_LITEADMIN,	AdmCmd_AmKick},
	{"amsabotage",	ADMRANK_ADMIN,		AdmCmd_AmSabotage},
	{"amgrantvip",	ADMRANK_ADMIN,		AdmCmd_AmGrantVIP},
	{"amrevokevip",	ADMRANK_ADMIN,		AdmCmd_AmRevokeVIP},
	{"amtele",		ADMRANK_DEVELOPER,	AdmCmd_AmTele},
	{"amgod",		ADMRANK_ADMIN,		AdmCmd_AmGod},
	{"amnoclip",	ADMRANK_ADMIN,		AdmCmd_AmNoClip},
	{"amnotarget",	ADMRANK_ADMIN,		AdmCmd_AmNoTarget},
	{"amsilence",	ADMRANK_LITEADMIN,	AdmCmd_AmSilence},
	{"amplayers",	ADMRANK_DEVELOPER,	AdmCmd_AmPlayers},
	{"amban",		ADMRANK_ADMIN,		AdmCmd_AmBan},
	{"amunban",		ADMRANK_ADMIN,		AdmCmd_AmUnBan},

#ifdef _DEBUG
	//eezstreet add
	{"amloot",		ADMRANK_ADMIN,		AdmCmd_AmLoot},
#endif

	/* Sentinel to terminate the table */
	{NULL,			0,					NULL},
};

static admRconCmd_t rconCmds[] = {
	{"amgrant",		RconCmd_AmGrant},
	{"amban",		RconCmd_AmBan},
	{"ambanip",		RconCmd_AmBanIP},
	{"amunban",		RconCmd_AmUnBan},
	{"amlistbans",	RconCmd_AmListBans},
	{"amclearbans", RconCmd_AmClearBans},
	
	{"amspeak",		RconCmd_AmSpeak},
	{"amprint",		RconCmd_AmCP},
	/* Sentinel to terminate the table */
	{NULL,			NULL},
};


/******************************************************/
/*                Internal functions                  */
/******************************************************/

static admCmd_t			*adminCmdTable[64];
static admRconCmd_t		*rconCmdTable[64];

static long GenerateHashValue( const char *name, const int size ) {
	int		i;
	long	hash;
	char	letter;

	hash = 0;
	i = 0;
	while (name[i] != '\0') {
		letter = tolower(name[i]);
		hash+=(long)(letter)*(i+119);
		i++;
	}
	hash = (hash ^ (hash >> 10) ^ (hash >> 20));
	hash &= (size-1);
	return hash;
}

/* Initializes the admin system */
void JKG_Admin_Init()
{
	long hash;
	admCmd_t *cmd;
	admRconCmd_t *rcmd;

	// Wipe the root tables
	memset( &adminCmdTable, 0, sizeof(adminCmdTable) );
	memset( &rconCmdTable, 0, sizeof(rconCmdTable) );

	// Link in the admin commands
	for (cmd = &adminCmds[0]; cmd->name; cmd++) {
		hash = GenerateHashValue(cmd->name, 64);
		cmd->next = adminCmdTable[hash];
		adminCmdTable[hash] = cmd;
	}

	// Link in the rcon commands
	for (rcmd = &rconCmds[0]; rcmd->name; rcmd++) {
		hash = GenerateHashValue(rcmd->name, 64);
		rcmd->next = rconCmdTable[hash];
		rconCmdTable[hash] = rcmd;
	}
}

qboolean JKG_Admin_Execute(const char *command, gentity_t *ent)
{
	long hash;
	admCmd_t *cmd;

	if (!ent || !ent->client) {
		return qfalse;
	}

	hash = GenerateHashValue(command, 64);
	for (cmd = adminCmdTable[hash]; cmd; cmd = cmd->next) {
		if (!Q_stricmp(cmd->name, command)) {
			if (cmd->minRank > ent->client->sess.adminRank) {
				return qfalse;
			}
			cmd->func(ent, ent-g_entities, ent->client->sess.adminRank);
			return qtrue;
		}
	}
	return qfalse;
}


qboolean JKG_Admin_ExecuteRcon(const char *command)
{
	long hash;
	admRconCmd_t *cmd;

	hash = GenerateHashValue(command, 64);
	for (cmd = rconCmdTable[hash]; cmd; cmd = cmd->next) {
		if (!Q_stricmp(cmd->name, command)) {
			cmd->func();
			return qtrue;
		}
	}
	return qfalse;
}

void JKG_AdminNotify( admrank_e rank, const char *fmt, ... )
{
	
	int i;
	gclient_t	*cl;
	va_list		argptr;
	static char		temp_buffer[1024];
	
	va_start (argptr, fmt);
	Q_vsnprintf(temp_buffer, 1024, fmt, argptr);
	va_end (argptr);

	for (i = 0; i < MAX_CLIENTS; i++) {
		cl = &level.clients[i];
		if (cl->pers.connected != CON_CONNECTED) {
			continue;
		}
		if (cl->sess.adminRank >= rank) {
			trap_SendServerCommand(i, va("chat 100 \"%s\"\n", temp_buffer));
		}
	}
}
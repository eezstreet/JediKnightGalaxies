#include "g_local.h"
#include "ai_main.h"

#ifdef __DOMINANCE_NPC__

extern gentity_t	*NPC;

extern qboolean NPC_FacePosition( vec3_t position, qboolean doPitch );
extern void G_SoundOnEnt( gentity_t *ent, int channel, const char *soundPath );

void NPC_EndConversation()
{
	NPC->NPC->conversationRole = 0;
	NPC->NPC->conversationPartner->NPC->conversationRole = 0;
	NPC->NPC->conversationSection = 1;
	NPC->NPC->conversationPartner->NPC->conversationSection = 1;
	NPC->NPC->conversationPart = 1;
	NPC->NPC->conversationPartner->NPC->conversationPart = 1;

	NPC->NPC->conversationReplyTime = level.time + 60000;
	NPC->NPC->conversationPartner->NPC->conversationReplyTime = level.time + 45000;

	NPC->NPC->conversationPartner->NPC->conversationPartner = NULL;
	NPC->NPC->conversationPartner = NULL;
}

void NPC_SetStormtrooperConversationReplyTimer()
{
	NPC->NPC->conversationPart++;
	NPC->NPC->conversationPartner->NPC->conversationPart++;
	NPC->NPC->conversationReplyTime = level.time + 14000;
	NPC->NPC->conversationPartner->NPC->conversationReplyTime = level.time + 7000;
}

extern void NPC_SetAnim(gentity_t *ent, int setAnimParts, int anim, int setAnimFlags);

void NPC_ConversationAnimation(gentity_t *NPC)
{
	int randAnim = irand(1,10);

	switch (randAnim)
	{
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
		NPC_SetAnim(NPC, SETANIM_BOTH, BOTH_TALK2, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
		break;
	case 6:
	case 7:
	case 8:
	case 9:
	default:
		NPC_SetAnim(NPC, SETANIM_BOTH, BOTH_TALK1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
		break;
	}
}

void NPC_StormTrooperConversation()
{
	int				role = NPC->NPC->conversationRole;
	int				section = NPC->NPC->conversationSection;
	int				part = NPC->NPC->conversationPart;
	vec3_t			origin, angles;
	fileHandle_t	f;
	char			filename[256];

	if (NPC->enemy || NPC->NPC->conversationPartner->enemy)
	{// Exit early if they get a target...
		NPC_EndConversation();
		return;
	}

	// Look at our partner...
	VectorCopy(NPC->NPC->conversationPartner->r.currentOrigin, origin);
	VectorSubtract( origin, NPC->r.currentOrigin , NPC->move_vector );
	vectoangles( NPC->move_vector, angles );
	G_SetAngles(NPC, angles);
	VectorCopy(angles, NPC->client->ps.viewangles);
	NPC_FacePosition( NPC->NPC->conversationPartner->r.currentOrigin, qfalse );

	if (NPC->NPC->conversationReplyTime > level.time)
		return; // Wait...

	if (section < 10)
	{
		if (part < 10)
			strcpy(filename, va("sound/conversation/stormtrooper/MST_0%iL0%i.mp3", section, part));
		else
			strcpy(filename, va("sound/conversation/stormtrooper/MST_0%iL%i.mp3", section, part));
	}
	else
	{
		if (part < 10)
			strcpy(filename, va("sound/conversation/stormtrooper/MST_%iL0%i.mp3", section, part));
		else
			strcpy(filename, va("sound/conversation/stormtrooper/MST_%iL%i.mp3", section, part));
	}

	trap_FS_FOpenFile( filename, &f, FS_READ );

	if ( !f )
	{
		trap_FS_FCloseFile( f );

		//G_Printf("File %s does not exist.\n", filename);

		NPC->NPC->conversationSection++;
		NPC->NPC->conversationPart = 1;
		NPC->NPC->conversationPartner->NPC->conversationSection++;
		NPC->NPC->conversationPartner->NPC->conversationPart = 1;

		if (section < 10)
		{
			if (part < 10)
				strcpy(filename, va("sound/conversation/stormtrooper/MST_0%iL0%i.mp3", section, part));
			else
				strcpy(filename, va("sound/conversation/stormtrooper/MST_0%iL%i.mp3", section, part));
		}
		else
		{
			if (part < 10)
				strcpy(filename, va("sound/conversation/stormtrooper/MST_%iL0%i.mp3", section, part));
			else
				strcpy(filename, va("sound/conversation/stormtrooper/MST_%iL%i.mp3", section, part));
		}

		trap_FS_FOpenFile( filename, &f, FS_READ );

		if ( !f )
		{// End of conversation...
			trap_FS_FCloseFile( f );

			//G_Printf("File %s does not exist. Aborting conversation.\n", filename);

			if (NPC->NPC->conversationSection > 15)
				NPC_EndConversation();

			return;
		}
	}
	//CHAN_VOICE_ATTEN
	trap_FS_FCloseFile( f );

	//G_Printf("NPC %i playing sound file %s.\n", NPC->s.number, filename);

	G_SoundOnEnt( NPC, CHAN_VOICE_ATTEN/*CHAN_AUTO*/, filename );
	NPC_SetStormtrooperConversationReplyTimer();
	NPC_ConversationAnimation(NPC);
}

void NPC_StormtrooperFindConversationPartner()
{
	if (NPC->client->NPC_class != CLASS_STORMTROOPER) return;

	if (VectorLength(NPC->client->ps.velocity) <= 16)
	{// I'm not mooving... Conversaion possible...
		int i = 0;
		gentity_t *partner = NULL;

		// Make sure there are no other conversations going on...
		for (i = MAX_CLIENTS; i < MAX_GENTITIES; i++)
		{
			partner = &g_entities[i];

			if (!partner) continue;
			if (partner == NPC) continue;
			if (partner->s.eType != ET_NPC) continue;
			if (!partner->client) continue;
			if (partner->client->NPC_class != CLASS_STORMTROOPER) continue;
			if (partner->NPC->conversationPartner || partner->NPC->conversationReplyTime > level.time)
				if (Distance(partner->r.currentOrigin, NPC->r.currentOrigin) < 2048)
					return; // We don't want them talking too close to others having the same conversations :)
		}

		for (i = MAX_CLIENTS; i < MAX_GENTITIES; i++)
		{
			partner = &g_entities[i];

			if (!partner) continue;
			if (partner == NPC) continue;
			if (partner->s.eType != ET_NPC) continue;
			if (!partner->client) continue;
			if (partner->client->NPC_class != CLASS_STORMTROOPER) continue;
			if (VectorLength(partner->client->ps.velocity) > 16) continue;

			if (partner->NPC->conversationPartner || partner->NPC->conversationReplyTime > level.time)
			{// this one already in a convo...
				if (Distance(partner->r.currentOrigin, NPC->r.currentOrigin) < 2048)
					return; // We don't want them talking too close to others having the same conversations :)

				continue;
			}

			if (Distance(partner->r.currentOrigin, NPC->r.currentOrigin) > 128) continue;

			// Looks good! Start a convo...
			NPC->NPC->conversationSection = 1;
			NPC->NPC->conversationRole = 1;
			NPC->NPC->conversationPart = 1;
			NPC->NPC->conversationPartner = partner;
			NPC->NPC->conversationPartner->NPC->conversationPartner = NPC;
			NPC->NPC->conversationPartner->NPC->conversationRole = 2;
			NPC->NPC->conversationPartner->NPC->conversationSection = 1;
			NPC->NPC->conversationPartner->NPC->conversationPart = 1;
			NPC->NPC->conversationPartner->NPC->conversationReplyTime = level.time + 8000;

			G_Printf(">> NPC %i (%s) enterred a conversation with NPC %i.\n", NPC->s.number, NPC->NPC_type, NPC->NPC->conversationPartner->s.number);
			NPC_StormTrooperConversation();
			return;
		}
	}
}

qboolean NPC_HasConversationSounds(gentity_t *conversationalist)
{
	fileHandle_t	f;
	char			filename[256];

	// For faster checking without FS wear...
	if (conversationalist->NPC->conversationAvailable == 1) return qfalse;
	if (conversationalist->NPC->conversationAvailable == 2) return qtrue;

	//G_Printf("Testing %s for conversation sounds.\n", conversationalist->NPC_type);

	strcpy(filename, va("sound/conversation/%s/conversation00.mp3", conversationalist->NPC_type));

	trap_FS_FOpenFile( filename, &f, FS_READ );

	if ( !f )
	{// End of conversation...
		trap_FS_FCloseFile( f );
		conversationalist->NPC->conversationAvailable = 1; // checked but has none!
		//G_Printf("%s has NO conversation sounds.\n", conversationalist->NPC_type);
		return qfalse;
	}

	trap_FS_FCloseFile( f );

	conversationalist->NPC->conversationAvailable = 2; // checked and has some!
	//G_Printf("%s has conversation sounds.\n", conversationalist->NPC_type);
	return qtrue;
}

qboolean NPC_VendorHasConversationSounds(gentity_t *conversationalist)
{
	fileHandle_t	f;
	char			filename[256];

	// For faster checking without FS wear...
	if (conversationalist->NPC->conversationAvailable == 1) return qfalse;
	if (conversationalist->NPC->conversationAvailable == 2) return qtrue;

	//G_Printf("Testing %s for conversation sounds.\n", conversationalist->NPC_type);

	strcpy(filename, va("sound/conversation/civilian_%s/conversation00.mp3", conversationalist->NPC_type));

	trap_FS_FOpenFile( filename, &f, FS_READ );

	if ( !f )
	{// End of conversation...
		trap_FS_FCloseFile( f );
		conversationalist->NPC->conversationAvailable = 1; // checked but has none!
		//G_Printf("%s has NO conversation sounds.\n", conversationalist->NPC_type);
		return qfalse;
	}

	trap_FS_FCloseFile( f );

	conversationalist->NPC->conversationAvailable = 2; // checked and has some!
	//G_Printf("%s has conversation sounds.\n", conversationalist->NPC_type);
	return qtrue;
}

qboolean NPC_VendorHasVendorSound(gentity_t *conversationalist, char *name)
{
	fileHandle_t	f;
	char			filename[256];

	strcpy(filename, va("sound/vendor/%s/%s.mp3", conversationalist->NPC_type, name));

	trap_FS_FOpenFile( filename, &f, FS_READ );

	if ( !f )
	{// End of conversation...
		trap_FS_FCloseFile( f );
		return qfalse;
	}

	trap_FS_FCloseFile( f );

	return qtrue;
}

void NPC_SetConversationReplyTimer()
{
	NPC->NPC->conversationPart++;
	NPC->NPC->conversationPartner->NPC->conversationPart++;
	NPC->NPC->conversationReplyTime = level.time + 10000;//18000;
	NPC->NPC->conversationPartner->NPC->conversationReplyTime = level.time + 5000;//8000;
}

void NPC_NPCConversation()
{
	int				part = NPC->NPC->conversationPart-1;
//	vec3_t			origin, angles;
	fileHandle_t	f;
	char			filename[256];

	if (NPC->client->NPC_class == CLASS_STORMTROOPER)
	{
		NPC_StormTrooperConversation();
		return;
	}

	if (NPC->enemy || NPC->NPC->conversationPartner->enemy)
	{// Exit early if they get a target...
		NPC_EndConversation();
		return;
	}

	// Look at our partner...
	//VectorCopy(NPC->NPC->conversationPartner->r.currentOrigin, origin);
	//VectorSubtract( origin, NPC->r.currentOrigin , NPC->move_vector );
	//vectoangles( NPC->move_vector, angles );
	//G_SetAngles(NPC, angles);
	//VectorCopy(angles, NPC->client->ps.viewangles);
	NPC_FacePosition( NPC->NPC->conversationPartner->r.currentOrigin, qfalse );

	if (NPC->NPC->conversationReplyTime > level.time)
		return; // Wait...

	if (part < 10)
		strcpy(filename, va("sound/conversation/%s/conversation0%i.mp3", NPC->NPC_type, part));
	else
		strcpy(filename, va("sound/conversation/%s/conversation%i.mp3", NPC->NPC_type, part));

	trap_FS_FOpenFile( filename, &f, FS_READ );

	if ( !f )
	{
		trap_FS_FCloseFile( f );

		//G_Printf("File %s does not exist. Aborting conversation.\n", filename);

		NPC_EndConversation();

		return;
	}
	//CHAN_VOICE_ATTEN
	trap_FS_FCloseFile( f );

	//G_Printf("NPC %i (%s) playing sound file %s.\n", NPC->s.number, NPC->NPC_type, filename);

	G_SoundOnEnt( NPC, CHAN_VOICE_ATTEN/*CHAN_AUTO*/, filename );
	NPC_SetConversationReplyTimer();
	NPC_ConversationAnimation(NPC);

	if (NPC->NPC->conversationPart > 50)
		NPC_EndConversation();
}

void NPC_FindConversationPartner()
{
	if (NPC->NPC->conversationSearchTime > level.time) return;

	NPC->NPC->conversationSearchTime = level.time + 2000 + irand(0, 3000);

	if (NPC->client->NPC_class == CLASS_STORMTROOPER)
	{
		NPC_StormtrooperFindConversationPartner();
		return;
	}

	if (!NPC_HasConversationSounds(NPC)) 
		return;

	if (VectorLength(NPC->client->ps.velocity) <= 16)
	{// I'm not mooving... Conversaion possible...
		int i = 0;
		gentity_t *partner = NULL;

		// Make sure there are no other conversations going on...
		for (i = MAX_CLIENTS; i < MAX_GENTITIES; i++)
		{
			partner = &g_entities[i];

			if (!partner) continue;
			if (partner == NPC) continue;
			if (partner->s.eType != ET_NPC) continue;
			if (!partner->client) continue;
			if (partner->client->NPC_class == CLASS_STORMTROOPER) continue;
			//if (!Q_stricmpn(partner->NPC_type, NPC->NPC_type, strlen(partner->NPC_type)-1)) continue; // never talk to the same race... (they would repeat eachother)
			if (partner->NPC->conversationPartner || partner->NPC->conversationReplyTime > level.time)
				if (Distance(partner->r.currentOrigin, NPC->r.currentOrigin) < 1024)//2048)
					return; // We don't want them talking too close to others having the same conversations :)
		}

		for (i = MAX_CLIENTS; i < MAX_GENTITIES; i++)
		{
			partner = &g_entities[i];

			if (!partner) continue;
			if (partner == NPC) continue;
			if (partner->s.eType != ET_NPC) continue;
			if (!partner->client) continue;
			if (partner->client->NPC_class == CLASS_STORMTROOPER) continue;
			//if (!Q_stricmpn(partner->NPC_type, NPC->NPC_type, strlen(partner->NPC_type)-1)) continue; // never talk to the same race... (they would repeat eachother)
			if (VectorLength(partner->client->ps.velocity) > 16) continue;
			if (!NPC_HasConversationSounds(partner)) continue;

			if (partner->NPC->conversationPartner || partner->NPC->conversationReplyTime > level.time)
			{// this one already in a convo...
				if (Distance(partner->r.currentOrigin, NPC->r.currentOrigin) < 1024)//2048)
					return; // We don't want them talking too close to others having the same conversations :)

				continue;
			}

			if (Distance(partner->r.currentOrigin, NPC->r.currentOrigin) > 128) continue;

			// Looks good! Start a convo...
			NPC->NPC->conversationSection = 1;
			NPC->NPC->conversationRole = 1;
			NPC->NPC->conversationPart = 1;
			NPC->NPC->conversationPartner = partner;
			NPC->NPC->conversationPartner->NPC->conversationPartner = NPC;
			NPC->NPC->conversationPartner->NPC->conversationRole = 2;
			NPC->NPC->conversationPartner->NPC->conversationSection = 1;
			NPC->NPC->conversationPartner->NPC->conversationPart = 1;
			NPC->NPC->conversationPartner->NPC->conversationReplyTime = level.time + 8000;

			G_Printf(">> NPC %i (%s) enterred a conversation with NPC %i (%s).\n", NPC->s.number, NPC->NPC_type, NPC->NPC->conversationPartner->s.number, NPC->NPC->conversationPartner->NPC_type);
			NPC_NPCConversation();
			return;
		}
	}
}

#endif //__DOMINANCE_NPC__
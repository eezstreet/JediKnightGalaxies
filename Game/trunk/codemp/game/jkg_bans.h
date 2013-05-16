/************************************************
|*
|* JKG Ban system
|*
\*/

void		JKG_Bans_Clear();
void		JKG_Bans_LoadBans();
void		JKG_Bans_Init();
void		JKG_Bans_SaveBans();
int			JKG_Bans_AddBan(netadr_t adr, const char *duration, const char *reason);
qboolean	JKG_Bans_AddBanString( const char *ip, const char *duration, const char *reason);
const char *JKG_Bans_IsBanned(netadr_t adr);
qboolean	JKG_Bans_RemoveBan(unsigned int id);
void		JKG_Bans_ListBans(const char *ip);
qboolean	JKG_Bans_GetBanInfo(unsigned int id, char *ip, size_t ipsize, char *duration, size_t durationsize, char *reason, size_t reasonsize);
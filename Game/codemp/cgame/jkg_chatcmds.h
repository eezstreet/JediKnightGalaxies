//       ____ ___________________   ___           ____  __ _______   ___  ________  ___ ______________
//      |    |\_   _____/\______ \ |   |         |    |/ _|\      \ |   |/  _____/ /   |   \__    ___/
//      |    | |    __)_  |    |  \|   |         |      <  /   |   \|   /   \  ___/    ~    \|    |   
//  /\__|    | |        \ |    `   \   |         |    |  \/    |    \   \    \_\  \    Y    /|    |   
//  \________|/_______  //_______  /___|         |____|__ \____|__  /___|\______  /\___|_  / |____|   
//                    \/         \/                      \/       \/            \/       \/           
//                         ________    _____   ____       _____  ____  ___ ______________ _________   
//                        /  _____/   /  _  \ |    |     /  _  \ \   \/  /|   \_   _____//   _____/   
//                       /   \  ___  /  /_\  \|    |    /  /_\  \ \     / |   ||    __)_ \_____  \    
//                       \    \_\  \/    |    \    |___/    |    \/     \ |   ||        \/        \   
//                        \______  /\____|__  /_______ \____|__  /___/\  \|___/_______  /_______  /   
//                               \/         \/        \/	   \/	   \_/			  \/        \/ (c)
// jkg_chatcmds.h
// JKG - Chat command processing
// Heavily based on the engine's command tokenizer
// Copyright (c) 2013 Jedi Knight Galaxies

typedef void (*xccommand_t) ( void );


int		CCmd_Argc( void );
char	*CCmd_Argv( int arg );
void	CCmd_ArgvBuffer( int arg, char *buffer, int bufferLength );
char	*CCmd_Args( void );
char *CCmd_ArgsFrom( int arg );
void	CCmd_ArgsBuffer( char *buffer, int bufferLength );
char *CCmd_Cmd();
qboolean CCmd_Execute(const char *command);
void CCmd_TokenizeString( const char *text_in );
void	CCmd_AddCommand( const char *cmd_name, xccommand_t function );
void	CCmd_RemoveCommand( const char *cmd_name );
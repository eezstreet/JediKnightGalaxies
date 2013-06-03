// This is a JKG system, not basejka

#include "ui_local.h"

// Do want access to argc, argv and Cmd_TokenizeString
static	int			*cmd_argc = (int *)0xB3CC08;
static	char		**cmd_argv = (char **)0xB39808;		// points into cmd_tokenized

// ENGINE: void __usercall Cmd_TokenizeString(const char *text_in<eax>)
static void Cmd_TokenizeString ( const char *text_in )
{
	_asm
	{
		pushad
		mov		eax, text_in
		mov		ebx, 0x436C50
		call	ebx
		popad
	}
}

/*
============
Cmd_Argc
============
*/
int		Cmd_Argc( void ) {
	return *cmd_argc;
}

/*
============
Cmd_Argv
============
*/
char	*Cmd_Argv( int arg ) {
	if ( (unsigned)arg >= *cmd_argc ) {
		return "";
	}
	return *(cmd_argv + arg);
}

static void UI_ServerRedirect( void )
{
	// clc.serverMessage will be no more than 1024 bytes so there's
	// no need to buffer these, we can just pass the argv pointers :D
	char *connAddress = Cmd_Argv(1);

	// We must have the server address at least
	if ( Cmd_Argc() < 2 ) {
		return;
	}

	// Because we're executing a command containing bits of the server command, we need to filter for \n, \r, ;, and "
	if ( strchr(connAddress,'\n') || strchr(connAddress,'\r') || strchr(connAddress,';') || strchr(connAddress,'"') ) {
		return;
	}
	// Well actually, on further research, the command breaking symbols above won't affect Cbuf_ExecuteText here because
	// the EXEC_NOW that's specified will call Cmd_ExecuteString not Cbuf_AddText, and will only execute one command.
	// Regardless, we'll filter them anyway because they shouldn't be sent from the server at all.
	// For confirmation on that, follow line 827 of cl_ui.c in the Quake 3 source code.

	trap_Cvar_Set( "cflag", Cmd_Argv(2) );
	trap_Cvar_Set( "connmsg", Cmd_Argv(3) );
	trap_Cmd_ExecuteText( EXEC_NOW, va( "connect \"%s\"", connAddress ) ); // Also with EXEC_NOW, no need to pass a trailing \n

	return;
}

qboolean UI_ServerCommand( const char *cmd_string )
{
	char *cmd;

	if ( !cmd_string[0] ) {
		return qfalse;
	}

	if ( cmd_string[0] == '@' && cmd_string[1] == '@' ) {
		// Oh god... server returned a localized string ref for some reason, ignore
		return qfalse;
	}

	Cmd_TokenizeString( cmd_string );

	if ( Cmd_Argc() == 0 ) {
		return qfalse;
	}

	cmd = Cmd_Argv(0);

	if ( !strcmp( cmd, "svr" ) ) {
		UI_ServerRedirect();
		return qtrue;
	} else if ( !strcmp( cmd, "update" ) ) {
		//UI_PromptUpdate(); // Just an example of a UI Server Command that we could have
		//return qtrue;
	}

	return qfalse;
}
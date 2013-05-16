
#include <Qsci/qscilexerlua.h>
#include "glualexer.h"

#include <qcolor.h>
#include <qfont.h>
#include <qsettings.h>


// The ctor.
QsciLexerGLua::QsciLexerGLua(QObject *parent)
    : QsciLexerLua(parent)
{
}


// The dtor.
QsciLexerGLua::~QsciLexerGLua()
{
}


// Returns the language name.
const char *QsciLexerGLua::language() const
{
    return "Lua";
}


// Returns the lexer name.
const char *QsciLexerGLua::lexer() const
{
    return "lua";
}


// Return the set of character sequences that can separate auto-completion
// words.
QStringList QsciLexerGLua::autoCompletionWordSeparators() const
{
    QStringList wl;

    wl << "." << ":";

    return wl;
}


// Return the list of characters that can start a block.
const char *QsciLexerGLua::blockStart(int *style) const
{
    if (style)
        *style = Operator;

    return "";
}


// Return the style used for braces.
int QsciLexerGLua::braceStyle() const
{
    return Operator;
}


// Returns the foreground colour of the text for a style.
QColor QsciLexerGLua::defaultColor(int style) const
{
    switch (style)
    {
    case Default:
        return QColor(0x00,0x00,0x00);

    case Comment:
    case LineComment:
        return QColor(0x00,0x7f,0x00);

    case Number:
        return QColor(0xFF,0x80,0x00);

    case Keyword:
        return QColor(0x00,0x00,0xFF);

    case String:
    case Character:
        return QColor(0x7F,0x7F,0x7F);
	
	case LiteralString:
		return QColor(0x95,0x00,0x4A);

    case Preprocessor:
        return QColor(0x80,0x40,0x00);

    case Operator:
    case Identifier:
		return QColor(0x00,0x00,0x80);
        break;

	case KeywordSet2:
		return QColor(0x00,0x80,0xC0);
	case KeywordSet3:
		return QColor(0x80,0x00,0xFF);
	case KeywordSet4:
		return QColor(0x00,0x00,0xA0);
	case KeywordSet5:
		return QColor(0x00,0x40,0xA0);
	case KeywordSet6:
		return QColor(0x80,0x00,0x40);
    }

    return QsciLexer::defaultColor(style);
}


// Returns the end-of-line fill for a style.
bool QsciLexerGLua::defaultEolFill(int style) const
{
    if (style == Comment || style == UnclosedString)
        return true;

    return QsciLexer::defaultEolFill(style);
}


// Returns the font of the text for a style.
QFont QsciLexerGLua::defaultFont(int style) const
{
    QFont f = QsciLexer::defaultFont(style);

    switch (style)
    {
    case Comment:
    case LineComment:
		f.setItalic(true);
		break;

	case Keyword:
	case KeywordSet2:
	case KeywordSet3:
	case KeywordSet4:
	case KeywordSet5:
	case KeywordSet6:
		f.setBold(true);
		break;

    default:
		break;
    }

    return f;
}


// Returns the set of keywords.
const char *QsciLexerGLua::keywords(int set) const
{
    if (set == 1)
        // Keywords.
        return
            "and break do else elseif end false for function if "
            "in local nil not or repeat return then true until "
            "while";

    if (set == 2)
        // Basic functions.
        return
            "_ALERT _ERRORMESSAGE _INPUT _PROMPT _OUTPUT _STDERR "
            "_STDIN _STDOUT call dostring foreach foreachi getn "
            "globals newtype rawget rawset require sort tinsert "
            "tremove "

            "G getfenv getmetatable ipairs loadlib next pairs "
            "pcall rawegal rawget rawset require setfenv "
            "setmetatable xpcall string table math coroutine io "
            "os debug"

			// GLua
			"findmetatable hook timer cmds Json thread chatcmds npcmanager cinematics entmanager CreateCvar ents players Vector bit file sys";

    if (set == 3)
        // Main libraries
        return
            "string.byte string.char string.dump string.find "
			"string.len string.lower string.rep string.sub string.upper string.format "
			"string.gfind string.gsub string.toTable string.split string.join string.left string.right "
			"string.replace string.trim string.trimRight string.trimLeft table.concat table.foreach table.foreachi table.getn "
			"table.sort table.insert table.remove table.setn math.abs math.acos math.asin "
			"math.atan math.atan2 math.ceil math.cos math.deg math.exp math.floor math.frexp "
			"math.ldexp math.log math.log10 math.max math.min math.mod math.pi math.rad "
			"math.random math.randomseed math.sin math.sqrt math.tan coroutine.create coroutine.resume "
			"coroutine.status coroutine.wrap coroutine.yield os.clock os.date os.difftime os.getenv os.time";

    if (set == 4)
        // GLua libraries
        return
            "hook.Add hook.Remove hook.GetHooks timer.Create timer.Simple timer.GetFreeTimer timer.Remove "
			"timer.Change timer.Reset cmds.Add cmds.Remove cmds.AddRcon cmds.RemoveRcon Json.Encode Json.Decode Json.Null "
			"thread.Create thread.CreateObject thread.Signal thread.Terminate chatcmds.Add chatcmds.Remove chatcmds.Ignore "
			"npcmanager.Init npcmanager.RegisterDir npcmanager.GetCount npcmanager.NPCExists cinematics.Init cinematics.RegisterDir "
			"cinematics.Get cinematics.ReloadCinematic cinematics.GetCinCount cinematics.GetCinList cinematics.AbortCinematic "
			"entmanager.Init entmanager.RegisterDir entmanager.GetEntType entmanager.GetEntTypeID entmanager.GetCount "
			"bit.Or bit.And bit.Xor bit.Not bit.ShiftL bit.ShiftR bit.ShiftRZ ents.GetByIndex ents.GetAll ents.GetByClass "
			"ents.GetByName ents.FindInBox ents.CreateEntityFactory ents.EntCount ents.EntCountAllocated ents.LogicalEntCount "
			"ents.LogicalEntCountAllocated file.Read file.Write file.Exists file.ListFiles players.GetByID players.GetByName "
			"players.GetByArg players.GetAll sys.GetCvarString sys.GetCvarInt sys.GetCvarFloat sys.SetCvar sys.GetConfigString "
			"sys.SetConfigString sys.Milliseconds sys.Time sys.Command sys.RemapShader sys.EffectIndex sys.PlayEffect sys.RadiusDamage "
			"sys.PlaySound sys.PlaySoundIdx sys.SoundIndex sys.WeaponClipSize sys.AmmoLimit sys.MapName sys.SpotWouldTelefrag "
			"sys.StripColorcodes sys.CreateStringBuilder sys.CreatePazaakGame sys.CreateConversation sys.CreateCinematic";
	if (set == 5)
		// GLua Constants
		return 
			"CVAR_ARCHIVE CVAR_USERINFO CVAR_SERVERINFO CVAR_SYSTEMINFO CVAR_INIT CVAR_LATCH CVAR_ROM CVAR_USER_CREATED "
			"CVAR_TEMP CVAR_CHEAT CVAR_NORESTART CVAR_INTERNAL CVAR_PARENTAL FP_FIRST FP_HEAL FP_LEVITATION FP_SPEED FP_PUSH "
			"FP_PULL FP_TELEPATHY FP_GRIP FP_LIGHTNING FP_RAGE FP_PROTECT FP_ABSORB FP_TEAM_HEAL FP_TEAM_FORCE FP_DRAIN FP_SEE "
			"FP_SABER_OFFENSE FP_SABER_DEFENSE FP_SABERTHROW CONTENTS_SOLID CONTENTS_LAVA CONTENTS_WATER CONTENTS_FOG "
			"CONTENTS_PLAYERCLIP CONTENTS_MONSTERCLIP CONTENTS_BOTCLIP CONTENTS_SHOTCLIP CONTENTS_BODY CONTENTS_CORPSE "
			"CONTENTS_TRIGGER CONTENTS_NODROP CONTENTS_TERRAIN CONTENTS_LADDER CONTENTS_ABSEIL CONTENTS_OPAQUE CONTENTS_OUTSIDE "
			"CONTENTS_INSIDE CONTENTS_SLIME CONTENTS_LIGHTSABER CONTENTS_TELEPORTER CONTENTS_ITEM CONTENTS_NOSHOT CONTENTS_DETAIL "
			"CONTENTS_TRANSLUCENT MASK_ALL MASK_SOLID MASK_PLAYERSOLID MASK_NPCSOLID MASK_DEADSOLID MASK_WATER MASK_OPAQUE MASK_SHOT "
			"WP_NONE WP_STUN_BATON WP_MELEE WP_SABER WP_BRYAR_PISTOL WP_BLASTER WP_DISRUPTOR WP_BOWCASTER WP_REPEATER WP_DEMP2 "
			"WP_FLECHETTE WP_ROCKET_LAUNCHER WP_THERMAL WP_TRIP_MINE WP_DET_PACK WP_CONCUSSION WP_BRYAR_OLD WP_EMPLACED_GUN WP_TURRET "
			"WP_NUM_WEAPONS AMMO_NONE AMMO_FORCE AMMO_BLASTER AMMO_POWERCELL AMMO_METAL_BOLTS AMMO_ROCKETS AMMO_EMPLACED AMMO_THERMAL "
			"AMMO_TRIPMINE AMMO_DETPACK AMMO_MAX";
	if (set == 6)
		// GLua methods
		return
			"ToRight Scale GetAngles Copy Normalize Length Sub GetUp DotProduct GetRight CrossProduct "
			"GetNormalized ToUp ToForward GetForward ToAngles Add Set IsModified GetString Update GetFloat "
			"GetBool GetName GetInteger IsNPC GetModel Free SetHealth SetTakeDamage GetHealth IsLinked IsPlayer UnlinkEntity GetTable "
			"GetPos SetTarget SetShaderFrame SetActive EnableShaderAnim GetTarget SetTargetName GetClassName ToNPC IsPlayerUsable GetClipmask "
			"UseEnt AutoBox CanTakeDamage SetPlayerUsable RotateSine GetTargetName SetPos MoveSine SetClipmask SetModel SetContents ToPlayer "
			"PlaySoundIdx GetContents IsValid Move HasSpawnVars GetBoundingBox SetBoundingBox IsActive PlaySound GetIndex LinkEntity SetAngles "
			"GetSpawnVars Rotate SetNextThink GetSpawnVar GetClassname SetSpawnVar RemoveSpawnVar Create GetVarCount SetClassname "
			"ClearSpawnVars GetActiveForce GetEvasion SetMaxHealth GetChaseEnemies GetEarShot SetNPCFreeze HasForce Damage GetLeader GetEyeTrace "
			"SetNoMindTrick Teleport GetFollowDist SetBehaviorState SetFollowDist SetEarShot StripWeapons MaxHealth GetVFov GiveForce "
			"GetNoForce GetVisRange SetVFov SetAnimUpper GetAggression SetReactions GetEnemy SetVigilance GetOrigin SetVisRange SetDontFire "
			"SetAnimHoldTime SetNoTarget SetAnimBoth SetAnimLower Remove SetWeapon GetNoAvoid SetWatchTarget HasNoTarget SetForceLevel "
			"GetDPitch SetNoForce GetDYaw SetFireWeapon SetChaseEnemies GetWeapon SetLeader GetRunSpeed GetFireWeapon SetRunning GetAim "
			"GetForceLevel SetHFov SetCrouched GiveWeapon SetNavGoal GetNPCFreeze GetWalkSpeed GetWatchTarget TakeForce GetAltFire HasWeapon "
			"SetAltFire GetNoAcrobatics SetRunSpeed GetLookForEnemies Health SetAim SetAggression GetHFov HasGodMode SetEnemy GetDontFire "
			"GetForcedMarch Kill SetArmor GetNoMindTrick SetDPitch SetDYaw GetRunning Armor SetForcedMarch SetLookForEnemies GetCrouched "
			"GetWalking SetWalking SetGodMode SetWalkSpeed SetNoAcrobatics GetReactions GetEntity SetOrigin SetEvasion GetVigilance SetMaxArmor "
			"SetViewTarget TakeWeapon MaxArmor SetNoAvoid SetTeam AddVelocity SetEscapeFunc SetKnockback SendChat StripAmmo GetIP StopConversation "
			"StopCinematic GetID SendCommand FinishedHacking HasHoldable TakeHoldable GiveHoldable GetNoMove GetUndying SetNoMove GetFreeze "
			"SetFreeze BroadcastEntity GetInvulnerable SetInvulnerable SetAmmo SetIsolate SetUndying Spawn SetGravity StartCinematic GetGravity "
			"IsConnected SetClipAmmo SetNoClip SendFadedChat GetAmmo StartHacking SetVelocity SetCinematicMode SendCenterPrint GetTeam "
			"StripClipAmmo SendPrint HasNoClip SetActiveForce Kick GetClipAmmo IsHacking";

    return 0;
}


// Returns the user name of a style.
QString QsciLexerGLua::description(int style) const
{
    switch (style)
    {
    case Default:
        return tr("Default");

    case Comment:
        return tr("Comment");

    case LineComment:
        return tr("Line comment");

    case Number:
        return tr("Number");

    case Keyword:
        return tr("Keyword");

    case String:
        return tr("String");

    case Character:
        return tr("Character");

    case LiteralString:
        return tr("Literal string");

    case Preprocessor:
        return tr("Preprocessor");

    case Operator:
        return tr("Operator");

    case Identifier:
        return tr("Identifier");

    case UnclosedString:
        return tr("Unclosed string");

    case KeywordSet2:
        return tr("Basic functions");

    case KeywordSet3:
        return tr("Main libraries");

    case KeywordSet4:
        return tr("GLua libraries");

    case KeywordSet5:
        return tr("GLua Constants");

    case KeywordSet6:
        return tr("GLua methods");

    case KeywordSet7:
        return tr("N/A");

    case KeywordSet8:
        return tr("N/A");
    }

    return QString();
}


// Returns the background colour of the text for a style.
QColor QsciLexerGLua::defaultPaper(int style) const
{
    switch (style)
    {
    case Comment:
        return QColor(0xd0,0xf0,0xd0);

    case LiteralString:
        return QColor(0xe0,0xff,0xff);

    case UnclosedString:
        return QColor(0xe0,0xc0,0xe0);

    }

    return QsciLexer::defaultPaper(style);
}
-- Entity related definitions

CONTENTS_SOLID			= 0x00000001	-- Default setting. An eye is never valid in a solid
CONTENTS_LAVA			= 0x00000002
CONTENTS_WATER			= 0x00000004
CONTENTS_FOG			= 0x00000008
CONTENTS_PLAYERCLIP		= 0x00000010
CONTENTS_MONSTERCLIP	= 0x00000020	-- Physically block bots
CONTENTS_BOTCLIP		= 0x00000040	-- A hint for bots - do not enter this brush by navigation (if possible)
CONTENTS_SHOTCLIP		= 0x00000080
CONTENTS_BODY			= 0x00000100	-- should never be on a brush, only in game
CONTENTS_CORPSE			= 0x00000200	-- should never be on a brush, only in game
CONTENTS_TRIGGER		= 0x00000400
CONTENTS_NODROP			= 0x00000800	-- don't leave bodies or items (death fog, lava)
CONTENTS_TERRAIN		= 0x00001000	-- volume contains terrain data
CONTENTS_LADDER			= 0x00002000
CONTENTS_ABSEIL			= 0x00004000  	-- (SOF2) used like ladder to define where an NPC can abseil
CONTENTS_OPAQUE			= 0x00008000	-- defaults to on, when off, solid can be seen through
CONTENTS_OUTSIDE		= 0x00010000	-- volume is considered to be in the outside (i.e. not indoors)
CONTENTS_INSIDE			= 0x10000000	-- volume is considered to be inside (i.e. indoors)
CONTENTS_SLIME			= 0x00020000	-- CHC needs this since we use same tools
CONTENTS_LIGHTSABER		= 0x00040000	-- ""
CONTENTS_TELEPORTER		= 0x00080000	-- ""
CONTENTS_ITEM			= 0x00100000	-- ""
CONTENTS_NOSHOT			= 0x00200000	-- shots pass through me
CONTENTS_DETAIL			= 0x08000000	-- brushes not used for the bsp
CONTENTS_TRANSLUCENT	= 0x80000000	-- don't consume surface fragments inside


MASK_ALL				= (-1)
MASK_SOLID				= (CONTENTS_SOLID + CONTENTS_TERRAIN)
MASK_PLAYERSOLID		= (CONTENTS_SOLID + CONTENTS_PLAYERCLIP + CONTENTS_BODY + CONTENTS_TERRAIN)
MASK_NPCSOLID			= (CONTENTS_SOLID + CONTENTS_MONSTERCLIP + CONTENTS_BODY + CONTENTS_TERRAIN)
MASK_DEADSOLID			= (CONTENTS_SOLID + CONTENTS_PLAYERCLIP + CONTENTS_TERRAIN)
MASK_WATER				= (CONTENTS_WATER + CONTENTS_LAVA + CONTENTS_SLIME)
MASK_OPAQUE				= (CONTENTS_SOLID + CONTENTS_SLIME + CONTENTS_LAVA + CONTENTS_TERRAIN)
MASK_SHOT				= (CONTENTS_SOLID + CONTENTS_BODY + CONTENTS_CORPSE + CONTENTS_TERRAIN)

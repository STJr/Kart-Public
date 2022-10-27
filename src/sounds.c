// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 1999-2018 by Sonic Team Junior.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  sounds.c
/// \brief music/sound tables, and related sound routines

#include "doomtype.h"
#include "i_sound.h"
#include "sounds.h"
#include "r_defs.h"
#include "r_things.h"
#include "z_zone.h"
#include "w_wad.h"
#include "lua_script.h"

//
// Information about all the sfx
//

sfxinfo_t S_sfx[NUMSFX] =
{

/*****
	Legacy doesn't use the PITCH variable, so now it is used for
	various flags. See soundflags_t.
*****/
  // S_sfx[0] needs to be a dummy for odd reasons. (don't modify this comment)
//  name, singularity, priority, pitch, volume, data, length, skinsound, usefulness, lumpnum
  {"none" ,  false,   0,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},

  // Skin Sounds
  {"altdi1", false, 192, 16, -1, NULL, 0, SKSPLDET1,  -1, LUMPERROR},
  {"altdi2", false, 192, 16, -1, NULL, 0, SKSPLDET2,  -1, LUMPERROR},
  {"altdi3", false, 192, 16, -1, NULL, 0, SKSPLDET3,  -1, LUMPERROR},
  {"altdi4", false, 192, 16, -1, NULL, 0, SKSPLDET4,  -1, LUMPERROR},
  {"altow1", false, 192, 16, -1, NULL, 0, SKSPLPAN1,  -1, LUMPERROR},
  {"altow2", false, 192, 16, -1, NULL, 0, SKSPLPAN2,  -1, LUMPERROR},
  {"altow3", false, 192, 16, -1, NULL, 0, SKSPLPAN3,  -1, LUMPERROR},
  {"altow4", false, 192, 16, -1, NULL, 0, SKSPLPAN4,  -1, LUMPERROR},
  {"victr1", false,  64, 16, -1, NULL, 0, SKSPLVCT1,  -1, LUMPERROR},
  {"victr2", false,  64, 16, -1, NULL, 0, SKSPLVCT2,  -1, LUMPERROR},
  {"victr3", false,  64, 16, -1, NULL, 0, SKSPLVCT3,  -1, LUMPERROR},
  {"victr4", false,  64, 16, -1, NULL, 0, SKSPLVCT4,  -1, LUMPERROR},
  {"gasp" ,  false,  64,  0, -1, NULL, 0,   SKSGASP,  -1, LUMPERROR},
  {"jump" ,  false, 140,  0, -1, NULL, 0,   SKSJUMP,  -1, LUMPERROR},
  {"pudpud", false,  64,  0, -1, NULL, 0, SKSPUDPUD,  -1, LUMPERROR},
  {"putput", false,  64,  0, -1, NULL, 0, SKSPUTPUT,  -1, LUMPERROR}, // not as high a priority
  {"spin" ,  false, 100,  0, -1, NULL, 0,   SKSSPIN,  -1, LUMPERROR},
  {"spndsh", false,  64,  1, -1, NULL, 0, SKSSPNDSH,  -1, LUMPERROR},
  {"thok" ,  false,  96,  0, -1, NULL, 0,   SKSTHOK,  -1, LUMPERROR},
  {"zoom" ,  false, 120,  1, -1, NULL, 0,   SKSZOOM,  -1, LUMPERROR},
  {"skid",   false,  64, 32, -1, NULL, 0,   SKSSKID,  -1, LUMPERROR},

  // Ambience/background objects/etc
  {"ambint",  true,  32,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},

  {"alarm",  false,  32,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"buzz1",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"buzz2",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"buzz3",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"buzz4",  false,   8,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"crumbl",  true, 127,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Platform Crumble Tails 03-16-2001
  {"fire",   false,   8, 32, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"grind",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"laser",   true,  16,  2, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"mswing", false,  16,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"pstart", false, 100,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"pstop",  false, 100,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"steam1", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Tails 06-19-2001
  {"steam2", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Tails 06-19-2001
  {"wbreak", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},

  {"rainin",  true,  24,  4, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"litng1", false,  16,  2, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"litng2", false,  16,  2, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"litng3", false,  16,  2, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"litng4", false,  16,  2, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"athun1", false,  16,  2, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"athun2", false,  16,  2, -1, NULL, 0,        -1,  -1, LUMPERROR},

  {"amwtr1", false,  12,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"amwtr2", false,  12,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"amwtr3", false,  12,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"amwtr4", false,  12,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"amwtr5", false,  12,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"amwtr6", false,  12,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"amwtr7", false,  12,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"amwtr8", false,  12,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"bubbl1", false,  11,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"bubbl2", false,  11,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"bubbl3", false,  11,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"bubbl4", false,  11,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"bubbl5", false,  11,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"floush", false,  16,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"splash", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"splish", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Splish Tails 12-08-2000
  {"wdrip1", false,   8,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"wdrip2", false,   8 , 0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"wdrip3", false,   8,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"wdrip4", false,   8,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"wdrip5", false,   8,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"wdrip6", false,   8,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"wdrip7", false,   8,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"wdrip8", false,   8,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"wslap",  false,  32,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Water Slap Tails 12-13-2000

  {"doora1", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"doorb1", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"doorc1", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"doorc2", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"doord1", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"doord2", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"eleva1", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"eleva2", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"eleva3", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"elevb1", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"elevb2", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"elevb3", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},

  {"ambin2", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"lavbub", false,  64,  8, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"rocks1", false,  64, 64, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"rocks2", false,  64, 64, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"rocks3", false,  64, 64, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"rocks4", false,  64, 64, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"rumbam", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"rumble", false,  64, 24, -1, NULL, 0,        -1,  -1, LUMPERROR},

  // Game objects, etc
  {"appear", false, 127,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"bkpoof", false,  70,  8, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"bnce1",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Boing!
  {"bnce2",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Boing!
  {"cannon", false,  64,  8, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cgot" ,   true, 120,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Got Emerald! Tails 09-02-2001
  {"cybdth", false,  32,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"deton",   true,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"ding",   false, 127,  8, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"dmpain", false,  96,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"drown",  false, 192,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"fizzle", false, 127,  8, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"gbeep",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Grenade beep
  {"ghit" ,  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"gloop",  false,  60,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"gspray", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"gravch", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"itemup",  true, 255,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"jet",    false,   8,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"jshard",  true, 167,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"lose" ,  false, 127,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"lvpass", false,  96,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"mindig", false,   8, 64, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"mixup",   true, 127,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"pogo" ,  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"pop"  ,  false,  78,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"rail1",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"rail2",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"rlaunc", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"shield", false,  60,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"shldls", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"spdpad", false, 127,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"spkdth", false, 127,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"spring", false, 112,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"statu1",  true,  64,  2, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"statu2",  true,  64,  2, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"strpst",  true, 192,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Starpost Sound Tails 07-04-2002
  {"supert",  true, 127,  2, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"telept", false,  32,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"tink" ,  false,  60,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"token" ,  true, 224,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // SS token
  {"trfire",  true,  60,  8, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"trpowr",  true, 127,  8, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"turhit", false,  40,  8, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"wdjump", false,  60,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"mswarp", false,  60, 16, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"mspogo", false,  60,  8, -1, NULL, 0,        -1,  -1, LUMPERROR},

  // Menu, interface
  {"chchng", false, 120,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"dwnind", false, 212,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"emfind", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"flgcap", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"menu1",   true,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"oneup",   true, 192,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"ptally",  true,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Point tally is identical to menu for now
  {"radio",  false,  60,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"wepchg",  true,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Weapon switch is identical to menu for now
  {"wtrdng",  true, 212,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // make sure you can hear the DING DING! Tails 03-08-2000
  {"zelda",  false, 120,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},

  // NiGHTS
  {"ideya",  false, 127,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"xideya", false, 127,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Xmas
  {"nbmper", false,  96,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"nxbump", false,  96,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Xmas
  {"ncitem", false, 204,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"nxitem", false, 204,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Xmas
  {"ngdone",  true, 127,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"nxdone",  true, 127,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Xmas
  {"drill1", false,  48,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"drill2", false,  48,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"ncspec", false, 204,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Tails 12-15-2003
  {"nghurt", false,  96,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"ngskid", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"hoop1",  false, 192,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"hoop2",  false, 192,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"hoop3",  false, 192,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"hidden", false, 204,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"prloop", false, 104,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"timeup",  true, 256,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},

  // Mario
  {"koopfr" , true, 127,  8, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"mario1", false,  60,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"mario2", false, 127,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"mario3", false,  60,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"mario4",  true,  78,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"mario5", false,  78,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"mario6", false,  60,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"mario7", false,  32,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"mario8", false,  48,  8, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"mario9",  true, 120,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"marioa",  true, 127,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"thwomp",  true, 127,  8, -1, NULL, 0,        -1,  -1, LUMPERROR},

  // Black Eggman
  {"bebomb", false,  64, 64, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"bechrg", false,  64, 64, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"becrsh", false,  64, 64, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"bedeen", false,  64, 64, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"bedie1", false,  64, 64, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"bedie2", false,  64, 64, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"beeyow", false,  64, 64, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"befall", false,  64, 64, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"befire", false,  64, 64, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"beflap", false,  64, 64, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"begoop", false,  64, 64, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"begrnd", false,  64, 64, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"behurt", false,  64, 64, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"bejet1", false,  64, 64, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"belnch", false,  64, 64, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"beoutb", false,  64, 64, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"beragh", false,  64, 64, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"beshot", false,  64, 64, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"bestep", false,  64, 64, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"bestp2", false,  64, 64, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"bewar1", false,  64, 64, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"bewar2", false,  64, 64, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"bewar3", false,  64, 64, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"bewar4", false,  64, 64, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"bexpld", false,  64, 64, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"bgxpld", false,  64, 64, -1, NULL, 0,        -1,  -1, LUMPERROR},

  // Cybrakdemon
  {"beelec", false,  64, 64, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"brakrl", false,  64, 64, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"brakrx", false,  64, 64, -1, NULL, 0,        -1,  -1, LUMPERROR},

  // Sonic 1 sounds
  {"s1a0",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s1a1",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s1a2",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Monty Mole attach
  {"s1a3",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s1a4",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s1a5",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s1a6",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s1a7",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s1a8",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s1a9",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s1aa",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s1ab",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Monty Mole twitch
  {"s1ac",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Misc. obstacle destroyed
  {"s1ad",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s1ae",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s1af",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s1b0",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s1b1",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s1b2",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s1b3",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s1b4",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Toggle all items
  {"s1b5",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s1b6",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s1b7",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s1b8",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s1b9",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s1ba",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Toggle item
  {"s1bb",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Thwomp drop
  {"s1bc",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s1bd",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Ballhog bounce
  {"s1be",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s1bf",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s1c0",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s1c1",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Monokuma death
  {"s1c2",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s1c3",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s1c4",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s1c5",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s1c6",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s1c7",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s1c8",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s1c9",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Item hit w/ voices off
  {"s1ca",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s1cb",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s1cc",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s1cd",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s1ce",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s1cf",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},

  // Sonic 2 sounds
  {"s220",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s221",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Lap sound
  {"s222",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s223",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s224",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s225",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s226",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s227",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s228",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s229",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s22a",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s22b",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s22c",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s22d",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s22e",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s22f",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s230",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s231",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s232",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s233",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s234",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s235",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s236",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s237",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s238",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s239",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s23a",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s23b",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s23c",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Drift boost
  {"s23d",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s23e",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Title screen opening
  {"s23f",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s240",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s241",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s242",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s243",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s244",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s245",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s246",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s247",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s248",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s249",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s24a",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s24b",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s24c",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s24d",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s24e",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s24f",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s250",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s251",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s252",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s253",   false, 255,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // 1st place finish
  {"s254",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s255",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s256",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s257",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s258",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s259",   false,  96,  8, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Item clashing
  {"s25a",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s25b",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s25c",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s25d",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s25e",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s25f",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Perfect start boost
  {"s260",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s261",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s262",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s263",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s264",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s265",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s266",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s267",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s268",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s269",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s26a",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s26b",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s26c",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s26d",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Missed checkpoint
  {"s26e",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s26f",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s270",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},

  // S3&K sounds
  {"s3k33",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k34",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k35",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k36",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k37",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k38",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k39",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k3a",  false,  96,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Kart item shield
  {"s3k3b",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k3c",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k3d",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k3e",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k3f",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k40",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k41",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k42",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Kart Thunder Shield spawned
  {"s3k43",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k44",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k45",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k46",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k47",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Kart AIZ dust
  {"s3k48",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k49",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Kart bump sound
  {"s3k4a",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k4b",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k4c",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k4d",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k4e",  false,  64,  8, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Kart explosion
  {"s3k4f",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k50",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k51",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k52",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k53",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k54",  false,  64, 64, -1, NULL, 0,        -1,  -1, LUMPERROR}, // MetalSonic shot fire
  {"s3k55",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k56",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k57",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k58",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k59",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k5a",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k5b",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k5c",  false,  96,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Kart Mine tick
  {"s3k5d",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k5e",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k5f",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k60",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k61",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k62",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k63",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k64",  false,  64,  2, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k65",  false, 255,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Blue Spheres
  {"s3k66",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k67",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k68",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Kart final lap
  {"s3k69",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k6a",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Kart finish
  {"s3k6b",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k6c",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k6d",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k6e",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k6f",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k70",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k71",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k72",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k73",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k74",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k75",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k76",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k77",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k78",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k79",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k7a",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k7b",  false,  96, 16, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Kart successful hit
  {"s3k7c",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k7d",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k7e",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k7f",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k80",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k81",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k82",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k83",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Overcharged start boost
  {"s3k84",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k85",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k86",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k87",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k88",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k89",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Kart Jawz target
  {"s3k8a",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k8b",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k8c",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k8d",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k8e",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k8f",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k90",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k91",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k92",  false,  64,  8, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Kart Hyudoro use
  {"s3k93",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k94",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k95",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k96",  false, 128,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Kart Orbinaut
  {"s3k97",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k98",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k99",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k9a",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k9b",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k9c",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k9d",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k9e",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3k9f",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3ka0",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3ka1",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3ka2",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Kart drift spark gain
  {"s3ka3",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3ka4",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3ka5",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3ka6",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3ka7",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Kart 3,2,1
  {"s3ka8",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3ka9",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kaa",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kab",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kac",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kad",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Kart GO!
  {"s3kae",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kaf",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kb0",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kb1",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kb2",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kb3",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kb4",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kb5",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kb6",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kb7",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kb8",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kb9",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kba",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kbb",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kbcs", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kbcl", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kbds", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kbdl", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kbes", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kbel", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kbfs", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kbfl", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kc0s", false, 192,  8, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Kart Jawz
  {"s3kc0l", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kc1s", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kc1l", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kc2s", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kc2l", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kc3s", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kc3l", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kc4s", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kc4l", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kc5s", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kc5l", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kc6s", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kc6l", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kc7s", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kc7l", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kc8s", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kc8l", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kc9s", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kc9l", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kcas", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Kart respawn
  {"s3kcal", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kcbs", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kcbl", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kccs", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kccl", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kcds", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kcdl", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kces", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kcel", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kcfs", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kcfl", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kd0s", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kd0l", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kd1s", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kd1l", false,  48,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kd2s", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kd2l", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kd3s", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kd3l", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kd4s", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kd4l", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kd5s", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kd5l", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kd6s", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kd6l", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kd7s", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kd7l", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kd8s", false,  64, 64, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Sharp Spin (maybe use the long/L version?)
  {"s3kd8l", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kd9s", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kd9l", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kdas", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kdal", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kdbs", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"s3kdbl", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},

  // 3D Blast sounds (the "missing" ones are direct copies of S3K's, no minor differences what-so-ever)
  {"3db06",  false,  96,  8, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Bumper stolen
  {"3db09",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"3db14",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"3db16",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},

  // Sonic CD sounds
  {"cdfm00", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Undercharged start boost
  {"cdfm01", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Sneaker boost
  {"cdfm02", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm03", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm04", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm05", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm06", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm07", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm08", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm09", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm10", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm11", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm12", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm13", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm14", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm15", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm16", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm17", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Brake drift ambient
  {"cdfm18", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm19", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // CD SS1 UFO death
  {"cdfm20", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm21", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm22", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm23", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm24", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm25", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm26", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm27", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm28", false,  96,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Eggman item landing
  {"cdfm29", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm30", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm31", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm32", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm33", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm34", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm35", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm36", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm37", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm38", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm39", false, 128,  8, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Mine deployed
  {"cdfm40", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm41", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm42", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm43", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm44", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm45", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm46", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm47", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm48", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm49", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm50", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm51", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm52", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm53", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm54", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm55", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm56", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm57", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm58", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm59", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm60", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm61", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm62", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm63", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm64", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm65", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm66", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm67", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm68", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm69", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm70", false,  96,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Trail item dragging
  {"cdfm71", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm72", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm73", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // CD SS1 UFO hit
  {"cdfm74", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm75", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm76", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm77", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm78", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdfm79", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdpcm0", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdpcm1", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdpcm2", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdpcm3", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdpcm4", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdpcm5", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdpcm6", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdpcm7", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdpcm8", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"cdpcm9", false,  96,  8, -1, NULL, 0,        -1,  -1, LUMPERROR}, // No damage taken

  // Knuckles Chaotix sounds
  {"kc2a",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"kc2b",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"kc2c",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"kc2d",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"kc2e",   false,  96,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Item box gotten
  {"kc2f",   false,  96,  8, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Spring pad
  {"kc30",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"kc31",   false,  64, 64, -1, NULL, 0,        -1,  -1, LUMPERROR}, // NO CONTEST explosion
  {"kc32",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"kc33",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"kc34",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"kc35",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"kc36",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"kc37",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"kc38",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"kc39",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Voting roulette
  {"kc3a",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"kc3b",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"kc3c",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"kc3d",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"kc3e",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"kc3f",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"kc40",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"kc41",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"kc42",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Unlock everything cheat
  {"kc43",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"kc44",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"kc45",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"kc46",   false,  96,  8, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Shrink use
  {"kc47",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"kc48",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Vote picked
  {"kc49",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"kc4a",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Voting beep
  {"kc4b",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"kc4c",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"kc4d",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"kc4e",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"kc4f",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"kc50",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"kc51",   false,  64, 64, -1, NULL, 0,        -1,  -1, LUMPERROR}, // NO CONTEST debris ambience
  {"kc52",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"kc53",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"kc54",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"kc55",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"kc56",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"kc57",   false, 128,  8, -1, NULL, 0,        -1,  -1, LUMPERROR}, // SPB locked in
  {"kc58",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"kc59",   false, 128,  8, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Shrink
  {"kc5a",   false, 128,  8, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Grow
  {"kc5b",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"kc5c",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"kc5d",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"kc5e",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"kc5f",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"kc60",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"kc61",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"kc62",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"kc63",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"kc64",   false,  96,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // SPB ambient
  {"kc65",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"kc66",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"kc67",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"kc68",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"kc69",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"kc6b",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"kc6c",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"kc6d",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"kc6e",   false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},

  // SRB2kart
  {"slip",   false,  96,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Spinout
  {"screec", false,  48,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Tight turning screech
  {"drift",  false,  48,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Drifting ambient
  {"ruburn", false,  48,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Rubber-burn turn ambient
  {"ddash",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Respawn Drop Dash
  {"tossed", false, 192,  8, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Item fired
  {"itpick", false, 128,  8, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Pick up HUD drop
  {"peel",   false,  96,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Edited S25A for banana landing
  {"hogbom", false,  96,  8, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Ballhog explosions
  {"zio3",   false, 128,  8, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Thunder Shield use
  {"kpogos", false,  96,  8, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Pogo Spring use
  {"alarmi", false, 255,  8, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Invincibility alarm
  {"alarmg", false, 255,  8, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Grow alarm
  {"kinvnc", false, 255,  8, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Invincibility music
  {"kgrow",  false, 255,  8, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Grow music
  {"itrol1",  true,  96,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Roulette spinning
  {"itrol2",  true,  96,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"itrol3",  true,  96,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"itrol4",  true,  96,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"itrol5",  true,  96,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"itrol6",  true,  96,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"itrol7",  true,  96,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"itrol8",  true,  96,  0, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"itrolf",  true,  96,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Roulette end
  {"itrolm",  true,  96,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Roulette end (mashed)
  {"itrolk",  true,  96,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Roulette end (karma enhanced)
  {"itrole",  true,  96,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Roulette end (Eggman)
  {"vroom",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Kart Krew opening vroom
  {"chaooo", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Chao audience cheer
  {"yeeeah", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Voting picks you
  {"noooo1", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Voting picks hell (by chance)
  {"noooo2", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Voting picks hell (on purpose)
  {"ruby1",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Phantom Ruby charge up
  {"ruby2",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Phantom Ruby teleport
  {"tcart",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Twinkle Cart
  {"bfare",  false, 128,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Pleasure Castle fanfare
  {"merry",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Pleasure Castle merry-go-round ambient
  {"bowlh",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Pleasure Castle pins
  {"tppop",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Pleasure Castle bombs
  {"hsdoor", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Red Barrage Area door
  {"hstrn",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Red Barrage Area train
  {"aspkb",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Red Barrage Area spikeballs
  {"wind1",  false,  64,  8, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Midnight Channel monsters
  {"fire2",  false,  64,  8, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"chain",  false, 255,  8, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Mementos Reaper
  {"mkuma",  false,  96,  8, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Trigger Happy Havoc Monokuma
  {"toada",  false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Arid Sands Toad scream
  {"gemhit", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Opulence gem/coin tumbling
  {"wrink", false,  64,  0, -1, NULL, 0,         -1,  -1, LUMPERROR}, // Some sort of ghoulie?
  {"bsnipe", false,  96,  8, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Banana sniping
  {"join",    true,  96,  8, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Player joined server
  {"leave",   true,  96,  8, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Player left server
  {"requst",  true,  96,  8, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Got a Discord join request
  {"syfail",  true,  96,  8, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Funny sync failure
  {"itfree", false,  64,  0, -1, NULL, 0,        -1,  -1, LUMPERROR}, // :shitsfree:
  {"dbgsal", false, 255,  8, -1, NULL, 0,        -1,  -1, LUMPERROR}, // Debug notification

  // SRB2Kart - Engine sounds
  // Engine class A
  {"krta00", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krta01", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krta02", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krta03", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krta04", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krta05", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krta06", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krta07", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krta08", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krta09", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krta10", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krta11", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krta12", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  // Engine class B
  {"krtb00", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtb01", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtb02", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtb03", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtb04", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtb05", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtb06", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtb07", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtb08", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtb09", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtb10", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtb11", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtb12", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  // Engine class C
  {"krtc00", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtc01", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtc02", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtc03", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtc04", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtc05", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtc06", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtc07", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtc08", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtc09", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtc10", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtc11", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtc12", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  // Engine class D
  {"krtd00", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtd01", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtd02", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtd03", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtd04", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtd05", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtd06", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtd07", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtd08", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtd09", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtd10", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtd11", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtd12", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  // Engine class E
  {"krte00", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krte01", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krte02", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krte03", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krte04", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krte05", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krte06", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krte07", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krte08", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krte09", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krte10", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krte11", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krte12", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  // Engine class F
  {"krtf00", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtf01", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtf02", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtf03", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtf04", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtf05", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtf06", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtf07", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtf08", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtf09", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtf10", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtf11", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtf12", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  // Engine class G
  {"krtg00", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtg01", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtg02", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtg03", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtg04", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtg05", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtg06", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtg07", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtg08", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtg09", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtg10", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtg11", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krtg12", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  // Engine class H
  {"krth00", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krth01", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krth02", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krth03", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krth04", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krth05", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krth06", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krth07", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krth08", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krth09", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krth10", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krth11", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krth12", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  // Engine class I
  {"krti00", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krti01", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krti02", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krti03", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krti04", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krti05", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krti06", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krti07", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krti08", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krti09", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krti10", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krti11", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},
  {"krti12", false,  48, 65, -1, NULL, 0,        -1,  -1, LUMPERROR},

  // SRB2kart - Skin sounds
  {"kwin",   false,  64, 96, -1, NULL, 0,   SKSKWIN,  -1, LUMPERROR},
  {"klose",  false,  64, 96, -1, NULL, 0,  SKSKLOSE,  -1, LUMPERROR},
  {"khurt1", false,  64, 96, -1, NULL, 0,  SKSKPAN1,  -1, LUMPERROR},
  {"khurt2", false,  64, 96, -1, NULL, 0,  SKSKPAN2,  -1, LUMPERROR},
  {"kattk1", false,  64, 96, -1, NULL, 0,  SKSKATK1,  -1, LUMPERROR},
  {"kattk2", false,  64, 96, -1, NULL, 0,  SKSKATK2,  -1, LUMPERROR},
  {"kbost1", false,  64, 96, -1, NULL, 0,  SKSKBST1,  -1, LUMPERROR},
  {"kbost2", false,  64, 96, -1, NULL, 0,  SKSKBST2,  -1, LUMPERROR},
  {"kslow",  false,  64, 32, -1, NULL, 0,  SKSKSLOW,  -1, LUMPERROR},
  {"khitem", false, 128, 32, -1, NULL, 0,  SKSKHITM,  -1, LUMPERROR},
  {"kgloat", false,  64, 48, -1, NULL, 0,  SKSKPOWR,  -1, LUMPERROR},

  // skin sounds free slots to add sounds at run time (Boris HACK!!!)
  // initialized to NULL
};

char freeslotnames[sfx_freeslot0 + NUMSFXFREESLOTS + NUMSKINSFXSLOTS][7];

// Prepare free sfx slots to add sfx at run time
void S_InitRuntimeSounds (void)
{
	sfxenum_t i;
	INT32 value;
	char soundname[10];

	for (i = sfx_freeslot0; i <= sfx_lastskinsoundslot; i++)
	{
		value = (i+1) - sfx_freeslot0;

		if (value < 10)
			sprintf(soundname, "fre00%d", value);
		else if (value < 100)
			sprintf(soundname, "fre0%d", value);
		else if (value < 1000)
			sprintf(soundname, "fre%d", value);
		else
			sprintf(soundname, "fr%d", value);

		strcpy(freeslotnames[value-1], soundname);

		S_sfx[i].name = freeslotnames[value-1];
		S_sfx[i].singularity = false;
		S_sfx[i].priority = 0;
		S_sfx[i].pitch = 0;
		S_sfx[i].volume = -1;
		S_sfx[i].data = NULL;
		S_sfx[i].length = 0;
		S_sfx[i].skinsound = -1;
		S_sfx[i].usefulness = -1;
		S_sfx[i].lumpnum = LUMPERROR;
	}
}

// Add a new sound fx into a free sfx slot.
//
sfxenum_t S_AddSoundFx(const char *name, boolean singular, INT32 flags, boolean skinsound)
{
	sfxenum_t i, slot;

	if (skinsound)
		slot = sfx_skinsoundslot0;
	else
		slot = sfx_freeslot0;

	for (i = slot; i < NUMSFX; i++)
	{
		if (!S_sfx[i].priority)
		{
			strncpy(freeslotnames[i-sfx_freeslot0], name, 6);
			S_sfx[i].singularity = singular;
			S_sfx[i].priority = 60;
			S_sfx[i].pitch = flags;
			S_sfx[i].volume = -1;
			S_sfx[i].lumpnum = LUMPERROR;
			S_sfx[i].skinsound = -1;
			S_sfx[i].usefulness = -1;

			/// \todo if precached load it here
			S_sfx[i].data = NULL;
			return i;
		}
	}
	I_Error("Out of Sound Freeslots while allocating \"%s\"\nLoad less addons to fix this.", name);
	return 0;
}

void S_RemoveSoundFx(sfxenum_t id)
{
	if (id >= sfx_freeslot0 && id <= sfx_lastskinsoundslot
		&& S_sfx[id].priority != 0)
	{
		S_sfx[id].lumpnum = LUMPERROR;
		I_FreeSfx(&S_sfx[id]);
		S_sfx[id].priority = 0;
	}
}

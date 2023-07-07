// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 2012-2016 by Matthew "Inuyasha" Walsh.
// Copyright (C) 2012-2018 by Sonic Team Junior.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  m_cond.c
/// \brief Unlockable condition system for SRB2 version 2.1

#include "m_cond.h"
#include "doomstat.h"
#include "z_zone.h"

#include "hu_stuff.h" // CEcho
#include "v_video.h" // video flags

#include "g_game.h" // record info
#include "r_things.h" // numskins
//#include "r_draw.h" // R_GetColorByName
#include "k_kart.h" // K_GetKartColorByName

// Map triggers for linedef executors
// 32 triggers, one bit each
UINT32 unlocktriggers;

// The meat of this system lies in condition sets
conditionset_t conditionSets[MAXCONDITIONSETS];

// Default Emblem locations
emblem_t emblemlocations[MAXEMBLEMS] =
{
	// GOLD DEV MEDALS
	// These values are directly lifted from the Champion ghost, through the power of hex editing!
	{ET_TIME, 0,0,0,  1, 'A', SKINCOLOR_GOLD, 3021, "", 0}, // Green Hills Zone - 1'26"31
	{ET_TIME, 0,0,0,  2, 'A', SKINCOLOR_GOLD, 4466, "", 0}, // Dark Race - 2'07"60
	{ET_TIME, 0,0,0,  3, 'A', SKINCOLOR_GOLD, 4337, "", 0}, // Northern District Zone - 2'03"91
	{ET_TIME, 0,0,0,  4, 'A', SKINCOLOR_GOLD, 3452, "", 0}, // Darkvile Garden Zone - 1'38"62
	{ET_TIME, 0,0,0,  5, 'A', SKINCOLOR_GOLD, 3525, "", 0}, // Daytona Speedway Zone - 1'40"71
	{ET_TIME, 0,0,0,  6, 'A', SKINCOLOR_GOLD, 4047, "", 0}, // Egg Zeppelin Zone - 1'55"62
	{ET_TIME, 0,0,0,  7, 'A', SKINCOLOR_GOLD, 4041, "", 0}, // Sonic Speedway Zone - 1'55"45
	{ET_TIME, 0,0,0,  8, 'A', SKINCOLOR_GOLD, 3281, "", 0}, // Hill Top Zone - 1'33"74
	{ET_TIME, 0,0,0,  9, 'A', SKINCOLOR_GOLD, 4764, "", 0}, // Misty Maze Zone - 2'16"11
	{ET_TIME, 0,0,0, 10, 'A', SKINCOLOR_GOLD, 4378, "", 0}, // Grand Metropolis - 2'05"08
	{ET_TIME, 0,0,0, 11, 'A', SKINCOLOR_GOLD, 3678, "", 0}, // Sunbeam Paradise Zone - 1'45"08
	{ET_TIME, 0,0,0, 12, 'A', SKINCOLOR_GOLD, 3928, "", 0}, // Diamond Square Zone - 1'52"22
	{ET_TIME, 0,0,0, 13, 'A', SKINCOLOR_GOLD, 3846, "", 0}, // Midnight Meadow Zone - 1'49"88
	{ET_TIME, 0,0,0, 14, 'A', SKINCOLOR_GOLD, 3278, "", 0}, // Twinkle Cart - 1'33"65
	{ET_TIME, 0,0,0, 15, 'A', SKINCOLOR_GOLD, 3591, "", 0}, // Pleasure Castle - 1'42"60
	{ET_TIME, 0,0,0, 16, 'A', SKINCOLOR_GOLD, 5187, "", 0}, // Paradise Hill Zone - 2'28"20
	{ET_TIME, 0,0,0, 17, 'A', SKINCOLOR_GOLD, 4976, "", 0}, // Sub-Zero Peak Zone - 2'22"17
	{ET_TIME, 0,0,0, 18, 'A', SKINCOLOR_GOLD, 3696, "", 0}, // Sapphire Coast Zone - 1'45"60
	{ET_TIME, 0,0,0, 19, 'A', SKINCOLOR_GOLD, 4931, "", 0}, // Sand Valley Zone - 2'20"88
	{ET_TIME, 0,0,0, 20, 'A', SKINCOLOR_GOLD, 4220, "", 0}, // Megablock Castle Zone - 2'00"57
	{ET_TIME, 0,0,0, 21, 'A', SKINCOLOR_GOLD, 4053, "", 0}, // Canyon Rush Zone - 1'55"80
	{ET_TIME, 0,0,0, 22, 'A', SKINCOLOR_GOLD, 3613, "", 0}, // Casino Resort Zone - 1'43"22
	{ET_TIME, 0,0,0, 23, 'A', SKINCOLOR_GOLD, 4385, "", 0}, // Silvercloud Island Zone - 2'05"28
	{ET_TIME, 0,0,0, 24, 'A', SKINCOLOR_GOLD, 5321, "", 0}, // Blue Mountain Zone - 2'32"02
	{ET_TIME, 0,0,0, 25, 'A', SKINCOLOR_GOLD, 4476, "", 0}, // Petroleum Refinery Zone - 2'07"88
	{ET_TIME, 0,0,0, 26, 'A', SKINCOLOR_GOLD, 3295, "", 0}, // Desert Palace Zone - 1'34"14
	{ET_TIME, 0,0,0, 27, 'A', SKINCOLOR_GOLD, 4765, "", 0}, // Aurora Atoll Zone - 2'16"14
	{ET_TIME, 0,0,0, 28, 'A', SKINCOLOR_GOLD, 4670, "", 0}, // Barren Badlands Zone - 2'13"42
	{ET_TIME, 0,0,0, 29, 'A', SKINCOLOR_GOLD, 5888, "", 0}, // Red Barrage Area - 2'48"22
	{ET_TIME, 0,0,0, 30, 'A', SKINCOLOR_GOLD, 4047, "", 0}, // Midnight Channel - 1'55"62
	{ET_TIME, 0,0,0, 31, 'A', SKINCOLOR_GOLD, 4944, "", 0}, // Vanilla Hotel Zone - 2'21"25
	{ET_TIME, 0,0,0, 32, 'A', SKINCOLOR_GOLD, 4638, "", 0}, // Toxic Palace Zone - 2'12"51
	{ET_TIME, 0,0,0, 33, 'A', SKINCOLOR_GOLD, 4036, "", 0}, // Ancient Tomb Zone - 1'55"31
	{ET_TIME, 0,0,0, 34, 'A', SKINCOLOR_GOLD, 3595, "", 0}, // Cloud Cradle Zone K - 1'42"71
	{ET_TIME, 0,0,0, 35, 'A', SKINCOLOR_GOLD, 4705, "", 0}, // Volcanic Valley Zone - 2'14"42
	{ET_TIME, 0,0,0, 36, 'A', SKINCOLOR_GOLD, 3314, "", 0}, // Kodachrome Void Zone - 1'34"68
	{ET_TIME, 0,0,0, 37, 'A', SKINCOLOR_GOLD, 3501, "", 0}, // Boiling Bedrock Zone - 1'40"02
	{ET_TIME, 0,0,0, 38, 'A', SKINCOLOR_GOLD, 4920, "", 0}, // Egg Quarters - 2'20"57
	{ET_TIME, 0,0,0, 39, 'A', SKINCOLOR_GOLD, 4318, "", 0}, // Virtual Highway Zone - 2'03"37
	{ET_TIME, 0,0,0, 40, 'A', SKINCOLOR_GOLD, 3550, "", 0}, // Eggman's Nightclub Zone - 1'41"42
	{ET_TIME, 0,0,0, 41, 'A', SKINCOLOR_GOLD, 2572, "", 0}, // KKR Ganbare Dochu 2 - 1'13"48
	{ET_TIME, 0,0,0, 42, 'A', SKINCOLOR_GOLD, 3091, "", 0}, // CK Chao Circuit 1 - 1'28"31
	{ET_TIME, 0,0,0, 43, 'A', SKINCOLOR_GOLD, 3454, "", 0}, // CK Chao Circuit 2 - 1'38"68
	{ET_TIME, 0,0,0, 44, 'A', SKINCOLOR_GOLD, 2958, "", 0}, // CK Cloud Tops 2 - 1'24"51
	{ET_TIME, 0,0,0, 45, 'A', SKINCOLOR_GOLD, 3693, "", 0}, // CK Regal Raceway - 1'45"51
	{ET_TIME, 0,0,0, 46, 'A', SKINCOLOR_GOLD, 3437, "", 0}, // SD2 Balloon Panic - 1'38"20
	{ET_TIME, 0,0,0, 47, 'A', SKINCOLOR_GOLD, 3238, "", 0}, // SM Dimension Heist - 1'32"51
	{ET_TIME, 0,0,0, 48, 'A', SKINCOLOR_GOLD, 3063, "", 0}, // MKSC Sky Garden - 1'27"51
	{ET_TIME, 0,0,0, 49, 'A', SKINCOLOR_GOLD, 2980, "", 0}, // MKDS Peach Gardens - 1'25"14
	{ET_TIME, 0,0,0, 50, 'A', SKINCOLOR_GOLD, 2914, "", 0}, // MKSC Rainbow Road - 1'23"25
	{ET_TIME, 0,0,0, 51, 'A', SKINCOLOR_GOLD, 3003, "", 0}, // SMK Donut Plains 1 - 1'25"80
	{ET_TIME, 0,0,0, 52, 'A', SKINCOLOR_GOLD, 2930, "", 0}, // SMK Mario Circuit 2 - 1'23"71
	{ET_TIME, 0,0,0, 53, 'A', SKINCOLOR_GOLD, 2389, "", 0}, // SMK Ghost Valley 2 - 1'08"25
	{ET_TIME, 0,0,0, 54, 'A', SKINCOLOR_GOLD, 4292, "", 0}, // SMK Bowser Castle 3 - 2'02"62
	{ET_TIME, 0,0,0, 55, 'A', SKINCOLOR_GOLD, 2346, "", 0}, // SMK Vanilla Lake 2 - 1'07"02

	// SILVER NORMAL MEDALS
	// The general "guideline" of how good we want a player to be at a map.
	{ET_TIME, 0,0,0,  1, 'B', SKINCOLOR_GREY, 110*TICRATE, "", 0}, // Green Hills Zone - 1'50"00
	{ET_TIME, 0,0,0,  2, 'B', SKINCOLOR_GREY, 160*TICRATE, "", 0}, // Dark Race - 2'40"00
	{ET_TIME, 0,0,0,  3, 'B', SKINCOLOR_GREY, 160*TICRATE, "", 0}, // Northern District Zone - 2'40"00
	{ET_TIME, 0,0,0,  4, 'B', SKINCOLOR_GREY, 120*TICRATE, "", 0}, // Darkvile Garden Zone - 2'00"00
	{ET_TIME, 0,0,0,  5, 'B', SKINCOLOR_GREY, 130*TICRATE, "", 0}, // Daytona Speedway Zone - 2'10"00
	{ET_TIME, 0,0,0,  6, 'B', SKINCOLOR_GREY, 140*TICRATE, "", 0}, // Egg Zeppelin Zone - 2'20"00
	{ET_TIME, 0,0,0,  7, 'B', SKINCOLOR_GREY, 140*TICRATE, "", 0}, // Sonic Speedway Zone - 2'20"00
	{ET_TIME, 0,0,0,  8, 'B', SKINCOLOR_GREY, 110*TICRATE, "", 0}, // Hill Top Zone - 1'50"00
	{ET_TIME, 0,0,0,  9, 'B', SKINCOLOR_GREY, 170*TICRATE, "", 0}, // Misty Maze Zone - 2'50"00
	{ET_TIME, 0,0,0, 10, 'B', SKINCOLOR_GREY, 140*TICRATE, "", 0}, // Grand Metropolis - 2'20"00
	{ET_TIME, 0,0,0, 11, 'B', SKINCOLOR_GREY, 120*TICRATE, "", 0}, // Sunbeam Paradise Zone - 2'00"00
	{ET_TIME, 0,0,0, 12, 'B', SKINCOLOR_GREY, 130*TICRATE, "", 0}, // Diamond Square Zone - 2'10"00
	{ET_TIME, 0,0,0, 13, 'B', SKINCOLOR_GREY, 135*TICRATE, "", 0}, // Midnight Meadow Zone - 2'15"00
	{ET_TIME, 0,0,0, 14, 'B', SKINCOLOR_GREY, 120*TICRATE, "", 0}, // Twinkle Cart - 2'00"00
	{ET_TIME, 0,0,0, 15, 'B', SKINCOLOR_GREY, 120*TICRATE, "", 0}, // Pleasure Castle - 2'00"00
	{ET_TIME, 0,0,0, 16, 'B', SKINCOLOR_GREY, 160*TICRATE, "", 0}, // Paradise Hill Zone - 2'40"00
	{ET_TIME, 0,0,0, 17, 'B', SKINCOLOR_GREY, 170*TICRATE, "", 0}, // Sub-Zero Peak Zone - 2'50"00
	{ET_TIME, 0,0,0, 18, 'B', SKINCOLOR_GREY, 130*TICRATE, "", 0}, // Sapphire Coast Zone - 2'10"00
	{ET_TIME, 0,0,0, 19, 'B', SKINCOLOR_GREY, 170*TICRATE, "", 0}, // Sand Valley Zone - 2'50"00
	{ET_TIME, 0,0,0, 20, 'B', SKINCOLOR_GREY, 145*TICRATE, "", 0}, // Megablock Castle Zone - 2'25"00
	{ET_TIME, 0,0,0, 21, 'B', SKINCOLOR_GREY, 140*TICRATE, "", 0}, // Canyon Rush Zone - 2'20"00
	{ET_TIME, 0,0,0, 22, 'B', SKINCOLOR_GREY, 120*TICRATE, "", 0}, // Casino Resort Zone - 2'00"00
	{ET_TIME, 0,0,0, 23, 'B', SKINCOLOR_GREY, 150*TICRATE, "", 0}, // Silvercloud Island Zone - 2'30"00
	{ET_TIME, 0,0,0, 24, 'B', SKINCOLOR_GREY, 170*TICRATE, "", 0}, // Blue Mountain Zone - 2'50"00
	{ET_TIME, 0,0,0, 25, 'B', SKINCOLOR_GREY, 150*TICRATE, "", 0}, // Petroleum Refinery Zone - 2'30"00
	{ET_TIME, 0,0,0, 26, 'B', SKINCOLOR_GREY, 120*TICRATE, "", 0}, // Desert Palace Zone - 2'00"00
	{ET_TIME, 0,0,0, 27, 'B', SKINCOLOR_GREY, 160*TICRATE, "", 0}, // Aurora Atoll Zone - 2'40"00
	{ET_TIME, 0,0,0, 28, 'B', SKINCOLOR_GREY, 150*TICRATE, "", 0}, // Barren Badlands Zone - 2'30"00
	{ET_TIME, 0,0,0, 29, 'B', SKINCOLOR_GREY, 170*TICRATE, "", 0}, // Red Barrage Area - 2'50"00
	{ET_TIME, 0,0,0, 30, 'B', SKINCOLOR_GREY, 135*TICRATE, "", 0}, // Midnight Channel - 2'15"00
	{ET_TIME, 0,0,0, 31, 'B', SKINCOLOR_GREY, 160*TICRATE, "", 0}, // Vanilla Hotel Zone - 2'40"00
	{ET_TIME, 0,0,0, 32, 'B', SKINCOLOR_GREY, 160*TICRATE, "", 0}, // Toxic Palace Zone - 2'40"00
	{ET_TIME, 0,0,0, 33, 'B', SKINCOLOR_GREY, 150*TICRATE, "", 0}, // Ancient Tomb Zone - 2'30"00
	{ET_TIME, 0,0,0, 34, 'B', SKINCOLOR_GREY, 120*TICRATE, "", 0}, // Cloud Cradle Zone K - 2'00"00
	{ET_TIME, 0,0,0, 35, 'B', SKINCOLOR_GREY, 165*TICRATE, "", 0}, // Volcanic Valley Zone - 2'45"00
	{ET_TIME, 0,0,0, 36, 'B', SKINCOLOR_GREY, 110*TICRATE, "", 0}, // Kodachrome Void Zone - 1'50"00
	{ET_TIME, 0,0,0, 37, 'B', SKINCOLOR_GREY, 130*TICRATE, "", 0}, // Boiling Bedrock Zone - 2'10"00
	{ET_TIME, 0,0,0, 38, 'B', SKINCOLOR_GREY, 165*TICRATE, "", 0}, // Egg Quarters - 2'45"00
	{ET_TIME, 0,0,0, 39, 'B', SKINCOLOR_GREY, 145*TICRATE, "", 0}, // Virtual Highway Zone - 2'25"00
	{ET_TIME, 0,0,0, 40, 'B', SKINCOLOR_GREY, 120*TICRATE, "", 0}, // Eggman's Nightclub Zone - 2'00"00
	{ET_TIME, 0,0,0, 41, 'B', SKINCOLOR_GREY, 100*TICRATE, "", 0}, // KKR Ganbare Dochu 2 - 1'40"00
	{ET_TIME, 0,0,0, 42, 'B', SKINCOLOR_GREY, 110*TICRATE, "", 0}, // CK Chao Circuit 1 - 1'50"00
	{ET_TIME, 0,0,0, 43, 'B', SKINCOLOR_GREY, 120*TICRATE, "", 0}, // CK Chao Circuit 2 - 2'00"00
	{ET_TIME, 0,0,0, 44, 'B', SKINCOLOR_GREY, 110*TICRATE, "", 0}, // CK Cloud Tops 2 - 1'50"00
	{ET_TIME, 0,0,0, 45, 'B', SKINCOLOR_GREY, 130*TICRATE, "", 0}, // CK Regal Raceway - 2'10"00
	{ET_TIME, 0,0,0, 46, 'B', SKINCOLOR_GREY, 110*TICRATE, "", 0}, // SD2 Balloon Panic - 1'50"00
	{ET_TIME, 0,0,0, 47, 'B', SKINCOLOR_GREY, 130*TICRATE, "", 0}, // SM Dimension Heist - 2'10"00
	{ET_TIME, 0,0,0, 48, 'B', SKINCOLOR_GREY, 110*TICRATE, "", 0}, // MKSC Sky Garden - 1'50"00
	{ET_TIME, 0,0,0, 49, 'B', SKINCOLOR_GREY, 105*TICRATE, "", 0}, // MKDS Peach Gardens - 1'45"00
	{ET_TIME, 0,0,0, 50, 'B', SKINCOLOR_GREY, 120*TICRATE, "", 0}, // MKSC Rainbow Road - 2'00"00
	{ET_TIME, 0,0,0, 51, 'B', SKINCOLOR_GREY, 100*TICRATE, "", 0}, // SMK Donut Plains 1 - 1'40"00
	{ET_TIME, 0,0,0, 52, 'B', SKINCOLOR_GREY, 105*TICRATE, "", 0}, // SMK Mario Circuit 2 - 1'45"00
	{ET_TIME, 0,0,0, 53, 'B', SKINCOLOR_GREY,  90*TICRATE, "", 0}, // SMK Ghost Valley 2 - 1'30"00
	{ET_TIME, 0,0,0, 54, 'B', SKINCOLOR_GREY, 150*TICRATE, "", 0}, // SMK Bowser Castle 3 - 2'30"00
	{ET_TIME, 0,0,0, 55, 'B', SKINCOLOR_GREY,  90*TICRATE, "", 0}  // SMK Vanilla Lake 2 - 1'30"00
};

// Default Extra Emblems
extraemblem_t extraemblems[MAXEXTRAEMBLEMS] =
{
	{"Novice",    "Play 100 matches",  10, 'C', SKINCOLOR_RED, 0},
	{"Standard",  "Play 250 matches",  11, 'C', SKINCOLOR_RED, 0},
	{"Expert",    "Play 500 matches",  12, 'C', SKINCOLOR_RED, 0},
	{"Master",    "Play 750 matches",  13, 'C', SKINCOLOR_RED, 0},
	{"Nightmare", "Play 1000 matches", 14, 'C', SKINCOLOR_RED, 0},
};

// Default Unlockables
unlockable_t unlockables[MAXUNLOCKABLES] =
{
	// Name, Objective, Showing Conditionset, ConditionSet, Unlock Type, Variable, NoCecho, NoChecklist
	/* 01 */ {"Egg Cup",  "", -1, 1, SECRET_NONE, 0, false, false, 0},
	/* 02 */ {"Chao Cup", "", -1, 2, SECRET_NONE, 0, false, false, 0},
	/* 03 */ {"SMK Cup",  "",  2, 3, SECRET_NONE, 0, false, false, 0},

	/* 04 */ {"Hard Game Speed", "", -1, 4, SECRET_HARDSPEED,  0, false, false, 0},
	/* 05 */ {"Encore Mode",     "",  4, 5, SECRET_ENCORE,     0, false, false, 0},
	/* 06 */ {"Hell Attack",     "",  6, 6, SECRET_HELLATTACK, 0, false, false, 0},

	/* 07 */ {"Record Attack", "", -1, -1, SECRET_RECORDATTACK, 0, true, true, 0},
};

// Default number of emblems and extra emblems
INT32 numemblems = 110;
INT32 numextraemblems = 5;

// DEFAULT CONDITION SETS FOR SRB2KART:
void M_SetupDefaultConditionSets(void)
{
	memset(conditionSets, 0, sizeof(conditionSets));

	// UNLOCKABLES
	// -- 1: Collect 5 medals OR play 25 matches
	M_AddRawCondition(1, 1, UC_TOTALEMBLEMS, 5, 0, 0);
	M_AddRawCondition(1, 2, UC_MATCHESPLAYED, 25, 0, 0);

	// -- 2: Collect 15 medals OR play 50 matches
	M_AddRawCondition(2, 1, UC_TOTALEMBLEMS, 15, 0, 0);
	M_AddRawCondition(2, 2, UC_MATCHESPLAYED, 50, 0, 0);

	// -- 3: Collect 30 medals OR play 150 matches
	M_AddRawCondition(3, 1, UC_TOTALEMBLEMS, 30, 0, 0);
	M_AddRawCondition(3, 2, UC_MATCHESPLAYED, 150, 0, 0);

	// -- 4: Collect 50 medals OR play 200 matches
	M_AddRawCondition(4, 1, UC_TOTALEMBLEMS, 50, 0, 0);
	M_AddRawCondition(4, 2, UC_MATCHESPLAYED, 200, 0, 0);

	// -- 5: Collect 70 medals OR play 300 matches
	M_AddRawCondition(5, 1, UC_TOTALEMBLEMS, 70, 0, 0);
	M_AddRawCondition(5, 2, UC_MATCHESPLAYED, 300, 0, 0);

	// -- 6: Collect 110 medals ONLY
	M_AddRawCondition(6, 1, UC_TOTALEMBLEMS, 110, 0, 0);

	// MILESTONES
	// -- 10: Play 100 matches
	M_AddRawCondition(10, 1, UC_MATCHESPLAYED, 100, 0, 0);

	// -- 11: Play 250 matches
	M_AddRawCondition(11, 1, UC_MATCHESPLAYED, 250, 0, 0);

	// -- 12: Play 500 matches
	M_AddRawCondition(12, 1, UC_MATCHESPLAYED, 500, 0, 0);

	// -- 13: Play 750 matches
	M_AddRawCondition(13, 1, UC_MATCHESPLAYED, 750, 0, 0);

	// -- 14: Play 1000 matches
	M_AddRawCondition(14, 1, UC_MATCHESPLAYED, 1000, 0, 0);
}

void M_AddRawCondition(UINT8 set, UINT8 id, conditiontype_t c, INT32 r, INT16 x1, INT16 x2)
{
	condition_t *cond;
	UINT32 num, wnum;

	I_Assert(set && set <= MAXCONDITIONSETS);

	wnum = conditionSets[set - 1].numconditions;
	num = ++conditionSets[set - 1].numconditions;

	conditionSets[set - 1].condition = Z_Realloc(conditionSets[set - 1].condition, sizeof(condition_t)*num, PU_STATIC, 0);

	cond = conditionSets[set - 1].condition;

	cond[wnum].id = id;
	cond[wnum].type = c;
	cond[wnum].requirement = r;
	cond[wnum].extrainfo1 = x1;
	cond[wnum].extrainfo2 = x2;
}

void M_ClearConditionSet(UINT8 set)
{
	if (conditionSets[set - 1].numconditions)
	{
		Z_Free(conditionSets[set - 1].condition);
		conditionSets[set - 1].condition = NULL;
		conditionSets[set - 1].numconditions = 0;
	}
	conditionSets[set - 1].achieved = false;
}

// Clear ALL secrets.
void M_ClearSecrets(void)
{
	INT32 i;

	memset(mapvisited, 0, sizeof(mapvisited));

	for (i = 0; i < MAXEMBLEMS; ++i)
		emblemlocations[i].collected = false;
	for (i = 0; i < MAXEXTRAEMBLEMS; ++i)
		extraemblems[i].collected = false;
	for (i = 0; i < MAXUNLOCKABLES; ++i)
		unlockables[i].unlocked = false;
	for (i = 0; i < MAXCONDITIONSETS; ++i)
		conditionSets[i].achieved = false;

	timesBeaten = timesBeatenWithEmeralds = 0;

	// Re-unlock any always unlocked things
	M_SilentUpdateUnlockablesAndEmblems();
}

// ----------------------
// Condition set checking
// ----------------------
UINT8 M_CheckCondition(condition_t *cn)
{
	switch (cn->type)
	{
		case UC_PLAYTIME: // Requires total playing time >= x
			return (totalplaytime >= (unsigned)cn->requirement);
		case UC_MATCHESPLAYED: // Requires any level completed >= x times
			return (matchesplayed >= (unsigned)cn->requirement);
		case UC_GAMECLEAR: // Requires game beaten >= x times
			return (timesBeaten >= (unsigned)cn->requirement);
		case UC_ALLEMERALDS: // Requires game beaten with all 7 emeralds >= x times
			return (timesBeatenWithEmeralds >= (unsigned)cn->requirement);
		/*case UC_ULTIMATECLEAR: // Requires game beaten on ultimate >= x times (in other words, never)
			return (timesBeatenUltimate >= (unsigned)cn->requirement);*/
		case UC_OVERALLTIME: // Requires overall time <= x
			return (M_GotLowEnoughTime(cn->requirement));
		/*case UC_OVERALLSCORE: // Requires overall score >= x
			return (M_GotHighEnoughScore(cn->requirement));
		case UC_OVERALLRINGS: // Requires overall rings >= x
			return (M_GotHighEnoughRings(cn->requirement));*/
		case UC_MAPVISITED: // Requires map x to be visited
			return ((mapvisited[cn->requirement - 1] & MV_VISITED) == MV_VISITED);
		case UC_MAPBEATEN: // Requires map x to be beaten
			return ((mapvisited[cn->requirement - 1] & MV_BEATEN) == MV_BEATEN);
		case UC_MAPALLEMERALDS: // Requires map x to be beaten with all emeralds in possession
			return ((mapvisited[cn->requirement - 1] & MV_ALLEMERALDS) == MV_ALLEMERALDS);
		/*case UC_MAPULTIMATE: // Requires map x to be beaten on ultimate
			return ((mapvisited[cn->requirement - 1] & MV_ULTIMATE) == MV_ULTIMATE);
		case UC_MAPPERFECT: // Requires map x to be beaten with a perfect bonus
			return ((mapvisited[cn->requirement - 1] & MV_PERFECT) == MV_PERFECT);*/
		case UC_MAPTIME: // Requires time on map <= x
			return (G_GetBestTime(cn->extrainfo1) <= (unsigned)cn->requirement);
		/*case UC_MAPSCORE: // Requires score on map >= x
			return (G_GetBestScore(cn->extrainfo1) >= (unsigned)cn->requirement);
		case UC_MAPRINGS: // Requires rings on map >= x
			return (G_GetBestRings(cn->extrainfo1) >= cn->requirement);
		case UC_NIGHTSSCORE:
			return (G_GetBestNightsScore(cn->extrainfo1, (UINT8)cn->extrainfo2) >= (unsigned)cn->requirement);
		case UC_NIGHTSTIME:
			return (G_GetBestNightsTime(cn->extrainfo1, (UINT8)cn->extrainfo2) <= (unsigned)cn->requirement);
		case UC_NIGHTSGRADE:
			return (G_GetBestNightsGrade(cn->extrainfo1, (UINT8)cn->extrainfo2) >= cn->requirement);*/
		case UC_TRIGGER: // requires map trigger set
			return !!(unlocktriggers & (1 << cn->requirement));
		case UC_TOTALEMBLEMS: // Requires number of emblems >= x
			return (M_GotEnoughEmblems(cn->requirement));
		case UC_EMBLEM: // Requires emblem x to be obtained
			return emblemlocations[cn->requirement-1].collected;
		case UC_EXTRAEMBLEM: // Requires extra emblem x to be obtained
			return extraemblems[cn->requirement-1].collected;
		case UC_CONDITIONSET: // requires condition set x to already be achieved
			return M_Achieved(cn->requirement-1);
	}
	return false;
}

static UINT8 M_CheckConditionSet(conditionset_t *c)
{
	UINT32 i;
	UINT32 lastID = 0;
	condition_t *cn;
	UINT8 achievedSoFar = true;

	for (i = 0; i < c->numconditions; ++i)
	{
		cn = &c->condition[i];

		// If the ID is changed and all previous statements of the same ID were true
		// then this condition has been successfully achieved
		if (lastID && lastID != cn->id && achievedSoFar)
			return true;

		// Skip future conditions with the same ID if one fails, for obvious reasons
		else if (lastID && lastID == cn->id && !achievedSoFar)
			continue;

		lastID = cn->id;
		achievedSoFar = M_CheckCondition(cn);
	}

	return achievedSoFar;
}

void M_CheckUnlockConditions(void)
{
	INT32 i;
	conditionset_t *c;

	for (i = 0; i < MAXCONDITIONSETS; ++i)
	{
		c = &conditionSets[i];
		if (!c->numconditions || c->achieved)
			continue;

		c->achieved = (M_CheckConditionSet(c));
	}
}

UINT8 M_UpdateUnlockablesAndExtraEmblems(boolean force)
{
	INT32 i;
	char cechoText[992] = "";
	UINT8 cechoLines = 0;

	if (majormods && !force) // SRB2Kart: for enabling unlocks online in modified servers
		return false;

	M_CheckUnlockConditions();

	// Go through extra emblems
	for (i = 0; i < numextraemblems; ++i)
	{
		if (extraemblems[i].collected || !extraemblems[i].conditionset)
			continue;
		if ((extraemblems[i].collected = M_Achieved(extraemblems[i].conditionset - 1)) != false)
		{
			strcat(cechoText, va(M_GetText("Got \"%s\" medal!\\"), extraemblems[i].name));
			++cechoLines;
		}
	}

	// Fun part: if any of those unlocked we need to go through the
	// unlock conditions AGAIN just in case an emblem reward was reached
	if (cechoLines)
		M_CheckUnlockConditions();

	// Go through unlockables
	for (i = 0; i < MAXUNLOCKABLES; ++i)
	{
		if (unlockables[i].unlocked || !unlockables[i].conditionset)
			continue;
		if ((unlockables[i].unlocked = M_Achieved(unlockables[i].conditionset - 1)) != false)
		{
			if (unlockables[i].nocecho)
				continue;
			strcat(cechoText, va(M_GetText("\"%s\" unlocked!\\"), unlockables[i].name));
			++cechoLines;
		}
	}

	// Announce
	if (cechoLines)
	{
		char slashed[1024] = "";
		for (i = 0; (i < 21) && (i < 24 - cechoLines); ++i)
			slashed[i] = '\\';
		slashed[i] = 0;

		strcat(slashed, cechoText);

		HU_SetCEchoFlags(V_YELLOWMAP|V_RETURN8);
		HU_SetCEchoDuration(6);
		HU_DoCEcho(slashed);
		return true;
	}
	return false;
}

// Used when loading gamedata to make sure all unlocks are synched with conditions
void M_SilentUpdateUnlockablesAndEmblems(void)
{
	INT32 i;
	boolean checkAgain = false;

	// Just in case they aren't to sync
	M_CheckUnlockConditions();
	M_CheckLevelEmblems();

	// Go through extra emblems
	for (i = 0; i < numextraemblems; ++i)
	{
		if (extraemblems[i].collected || !extraemblems[i].conditionset)
			continue;
		if ((extraemblems[i].collected = M_Achieved(extraemblems[i].conditionset - 1)) != false)
			checkAgain = true;
	}

	// check again if extra emblems unlocked, blah blah, etc
	if (checkAgain)
		M_CheckUnlockConditions();

	// Go through unlockables
	for (i = 0; i < MAXUNLOCKABLES; ++i)
	{
		if (unlockables[i].unlocked || !unlockables[i].conditionset)
			continue;
		unlockables[i].unlocked = M_Achieved(unlockables[i].conditionset - 1);
	}
}

// Emblem unlocking shit
UINT8 M_CheckLevelEmblems(void)
{
	INT32 i;
	INT32 valToReach;
	INT16 levelnum;
	UINT8 res;
	UINT8 somethingUnlocked = 0;

	// Update Score, Time, Rings emblems
	for (i = 0; i < numemblems; ++i)
	{
		if (emblemlocations[i].type <= ET_SKIN || emblemlocations[i].collected)
			continue;

		levelnum = emblemlocations[i].level;
		valToReach = emblemlocations[i].var;

		switch (emblemlocations[i].type)
		{
			/*case ET_SCORE: // Requires score on map >= x
				res = (G_GetBestScore(levelnum) >= (unsigned)valToReach);
				break;*/
			case ET_TIME: // Requires time on map <= x
				res = (G_GetBestTime(levelnum) <= (unsigned)valToReach);
				break;
			/*case ET_RINGS: // Requires rings on map >= x
				res = (G_GetBestRings(levelnum) >= valToReach);
				break;
			case ET_NGRADE: // Requires NiGHTS grade on map >= x
				res = (G_GetBestNightsGrade(levelnum, 0) >= valToReach);
				break;
			case ET_NTIME: // Requires NiGHTS time on map <= x
				res = (G_GetBestNightsTime(levelnum, 0) <= (unsigned)valToReach);
				break;*/
			default: // unreachable but shuts the compiler up.
				continue;
		}

		emblemlocations[i].collected = res;
		if (res)
			++somethingUnlocked;
	}
	return somethingUnlocked;
}

// -------------------
// Quick unlock checks
// -------------------
UINT8 M_AnySecretUnlocked(void)
{
	INT32 i;
	for (i = 0; i < MAXUNLOCKABLES; ++i)
	{
		if (!unlockables[i].nocecho && unlockables[i].unlocked)
			return true;
	}
	return false;
}

UINT8 M_SecretUnlocked(INT32 type)
{
	INT32 i;

#if 1
	if (dedicated)
		return true;
#endif

	for (i = 0; i < MAXUNLOCKABLES; ++i)
	{
		if (unlockables[i].type == type && unlockables[i].unlocked)
			return true;
	}
	return false;
}

UINT8 M_MapLocked(INT32 mapnum)
{
	if (!mapheaderinfo[mapnum-1] || mapheaderinfo[mapnum-1]->unlockrequired < 0)
		return false;
	if (!unlockables[mapheaderinfo[mapnum-1]->unlockrequired].unlocked)
		return true;
	return false;
}

INT32 M_CountEmblems(void)
{
	INT32 found = 0, i;
	for (i = 0; i < numemblems; ++i)
	{
		if (emblemlocations[i].collected)
			found++;
	}
	for (i = 0; i < numextraemblems; ++i)
	{
		if (extraemblems[i].collected)
			found++;
	}
	return found;
}

// --------------------------------------
// Quick functions for calculating things
// --------------------------------------

// Theoretically faster than using M_CountEmblems()
// Stops when it reaches the target number of emblems.
UINT8 M_GotEnoughEmblems(INT32 number)
{
	INT32 i, gottenemblems = 0;
	for (i = 0; i < numemblems; ++i)
	{
		if (emblemlocations[i].collected)
			if (++gottenemblems >= number) return true;
	}
	for (i = 0; i < numextraemblems; ++i)
	{
		if (extraemblems[i].collected)
			if (++gottenemblems >= number) return true;
	}
	return false;
}

/*UINT8 M_GotHighEnoughScore(INT32 tscore)
{
	INT32 mscore = 0;
	INT32 i;

	for (i = 0; i < NUMMAPS; ++i)
	{
		if (!mapheaderinfo[i] || !(mapheaderinfo[i]->menuflags & LF2_RECORDATTACK))
			continue;
		if (!mainrecords[i])
			continue;

		if ((mscore += mainrecords[i]->score) > tscore)
			return true;
	}
	return false;
}*/

UINT8 M_GotLowEnoughTime(INT32 tictime)
{
	INT32 curtics = 0;
	INT32 i;

	for (i = 0; i < NUMMAPS; ++i)
	{
		if (!mapheaderinfo[i] || !(mapheaderinfo[i]->menuflags & LF2_RECORDATTACK))
			continue;

		if (!mainrecords[i] || !mainrecords[i]->time)
			return false;
		else if ((curtics += mainrecords[i]->time) > tictime)
			return false;
	}
	return true;
}

/*UINT8 M_GotHighEnoughRings(INT32 trings)
{
	INT32 mrings = 0;
	INT32 i;

	for (i = 0; i < NUMMAPS; ++i)
	{
		if (!mapheaderinfo[i] || !(mapheaderinfo[i]->menuflags & LF2_RECORDATTACK))
			continue;
		if (!mainrecords[i])
			continue;

		if ((mrings += mainrecords[i]->rings) > trings)
			return true;
	}
	return false;
}*/

// ----------------
// Misc Emblem shit
// ----------------

// Returns pointer to an emblem if an emblem exists for that level.
// Pass -1 mapnum to continue from last emblem.
// NULL if not found.
// note that this goes in reverse!!
emblem_t *M_GetLevelEmblems(INT32 mapnum)
{
	static INT32 map = -1;
	static INT32 i = -1;

	if (mapnum > 0)
	{
		map = mapnum;
		i = numemblems;
	}

	while (--i >= 0)
	{
		if (emblemlocations[i].level == map)
			return &emblemlocations[i];
	}
	return NULL;
}

skincolors_t M_GetEmblemColor(emblem_t *em)
{
	if (!em || em->color >= MAXSKINCOLORS)
		return SKINCOLOR_NONE;
	return em->color;
}

const char *M_GetEmblemPatch(emblem_t *em)
{
	static char pnamebuf[7] = "GOTITn";

	I_Assert(em->sprite >= 'A' && em->sprite <= 'Z');
	pnamebuf[5] = em->sprite;
	return pnamebuf;
}

skincolors_t M_GetExtraEmblemColor(extraemblem_t *em)
{
	if (!em || em->color >= MAXSKINCOLORS)
		return SKINCOLOR_NONE;
	return em->color;
}

const char *M_GetExtraEmblemPatch(extraemblem_t *em)
{
	static char pnamebuf[7] = "GOTITn";

	I_Assert(em->sprite >= 'A' && em->sprite <= 'Z');
	pnamebuf[5] = em->sprite;
	return pnamebuf;
}

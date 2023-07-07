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
/// \file  d_ticcmd.h
/// \brief Button/action code definitions, ticcmd_t

#ifndef __D_TICCMD__
#define __D_TICCMD__

#include "m_fixed.h"
#include "doomtype.h"

#ifdef __GNUG__
#pragma interface
#endif

#define MAXPREDICTTICS 12

// Button/action code definitions.
typedef enum
{
	BT_ACCELERATE	= 1,		// Accelerate
	BT_DRIFT		= 1<<2,		// Drift (direction is cmd->driftturn)
	BT_BRAKE		= 1<<3,		// Brake
	BT_ATTACK		= 1<<4,		// Use Item
	BT_FORWARD		= 1<<5,		// Aim Item Forward
	BT_BACKWARD		= 1<<6,		// Aim Item Backward

	// free: 1<<7 to 1<<12

	// Lua garbage
	BT_CUSTOM1		= 1<<13,
	BT_CUSTOM2		= 1<<14,
	BT_CUSTOM3		= 1<<15,
} buttoncode_t;

// The data sampled per tick (single player)
// and transmitted to other peers (multiplayer).
// Mainly movements/button commands per game tick,
// plus a checksum for internal state consistency.

// bits in angleturn
#define TICCMD_RECEIVED 1
#define TICCMD_XY 2

#if defined(_MSC_VER)
#pragma pack(1)
#endif

typedef struct
{
	SINT8 forwardmove; // -MAXPLMOVE to MAXPLMOVE (50)
	SINT8 sidemove; // -MAXPLMOVE to MAXPLMOVE (50)
	INT16 angleturn; // <<16 for angle delta - saved as 1 byte into demos
	INT16 aiming; // vertical aiming, see G_BuildTicCmd
	UINT16 buttons;
	INT16 driftturn; // SRB2Kart: Used for getting drift turn speed
	UINT8 latency; // Netgames: how many tics ago was this ticcmd generated from this player's end?
} ATTRPACK ticcmd_t;

#if defined(_MSC_VER)
#pragma pack()
#endif

#endif

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
/// \file  p_pspr.h
/// \brief Sprite animation
///        Frame flags:
///        handles maximum brightness (torches, muzzle flare, light sources)

// we use the upper 16 bits of the frame field for effects.
#ifndef __P_PSPR__
#define __P_PSPR__

// Basic data types.
// Needs fixed point, and BAM angles.
#include "m_fixed.h"
#include "tables.h"

//
// Needs to include the precompiled
//  sprite animation tables.
// Header generated by multigen utility.
// This includes all the data for thing animation,
// i.e. the Thing Atrributes table
// and the Frame Sequence table.
#include "info.h"

#ifdef __GNUG__
#pragma interface
#endif

/// \brief Frame flags: only the frame number
#define FF_FRAMEMASK 0x1ff
/// \brief Frame flags: Thin, paper-like sprite (for collision equivalent, see
/// MF_PAPERCOLLISION)
#define FF_PAPERSPRITE 0x800
/// \brief Frame flags: Simple stateless animation
#define FF_ANIMATE 0x4000
/// \brief Frame flags: frame always appears full bright
#define FF_FULLBRIGHT 0x8000
/// \brief Frame flags: 0 = no trans(opaque), 1-15 = transl. table
#define FF_TRANSMASK 0xf0000
/// \brief shift for FF_TRANSMASK
#define FF_TRANSSHIFT 16
/// \brief preshifted translucency flags
#define FF_TRANS10 (tr_trans10 << FF_TRANSSHIFT)
#define FF_TRANS20 (tr_trans20 << FF_TRANSSHIFT)
#define FF_TRANS30 (tr_trans30 << FF_TRANSSHIFT)
#define FF_TRANS40 (tr_trans40 << FF_TRANSSHIFT)
#define FF_TRANS50 (tr_trans50 << FF_TRANSSHIFT)
#define FF_TRANS60 (tr_trans60 << FF_TRANSSHIFT)
#define FF_TRANS70 (tr_trans70 << FF_TRANSSHIFT)
#define FF_TRANS80 (tr_trans80 << FF_TRANSSHIFT)
#define FF_TRANS90 (tr_trans90 << FF_TRANSSHIFT)

/**	\brief translucency tables

        \todo add another asm routine which use the fg and bg indexes in the
        inverse order so the 20-80 becomes 80-20 translucency, no need
        for other tables (thus 1090,2080,5050,8020,9010, and fire special)

        \todo render all this obsolete by writing some goddamn realtime
        translucency and 32-bit color support
*/
typedef enum {
  tr_trans10 = 1,
  tr_trans20,
  tr_trans30,
  tr_trans40,
  tr_trans50,
  tr_trans60,
  tr_trans70,
  tr_trans80,
  tr_trans90,
  NUMTRANSMAPS
} transnum_t;

#endif

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
/// \file  m_swap.h
/// \brief Endianess handling, swapping 16bit and 32bit

#ifndef __M_SWAP__
#define __M_SWAP__

#include "endian.h"

#define SWAP_SHORT(x) ((INT16)(\
(((UINT16)(x) & (UINT16)0x00ffU) << 8) \
| \
(((UINT16)(x) & (UINT16)0xff00U) >> 8))) \

#define SWAP_LONG(x) ((INT32)(\
(((UINT32)(x) & (UINT32)0x000000ffUL) << 24) \
| \
(((UINT32)(x) & (UINT32)0x0000ff00UL) <<  8) \
| \
(((UINT32)(x) & (UINT32)0x00ff0000UL) >>  8) \
| \
(((UINT32)(x) & (UINT32)0xff000000UL) >> 24)))

// Endianess handling.
// WAD files are stored little endian.
#ifdef SRB2_BIG_ENDIAN
#define SHORT SWAP_SHORT
#define LONG SWAP_LONG
#define MSBF_SHORT(x) ((INT16)(x))
#define MSBF_LONG(x) ((INT32)(x))
#else
#define SHORT(x) ((INT16)(x))
#define LONG(x)	((INT32)(x))
#define MSBF_SHORT SWAP_SHORT
#define MSBF_LONG SWAP_LONG
#endif

#endif

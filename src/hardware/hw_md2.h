// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 1999-2019 by Sonic Team Junior.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file
/// \brief MD2 Handling
///	Inspired from md2.h by Mete Ciragan (mete@swissquake.ch)

#ifndef _HW_MD2_H_
#define _HW_MD2_H_

#include "hw_glob.h"
#include "hw_model.h"

#if defined(_MSC_VER)
#pragma pack()
#endif

typedef struct
{
	char        filename[32];
	float       scale;
	float       offset;
	model_t     *model;
	void        *grpatch;
	void        *blendgrpatch;
	boolean     notfound;
	INT32       skin;
	boolean     error;
} md2_t;

extern md2_t md2_models[NUMSPRITES];
extern md2_t md2_playermodels[MAXSKINS];

void HWR_InitMD2(void);
void HWR_DrawMD2(gr_vissprite_t *spr);
void HWR_AddPlayerMD2(INT32 skin);
void HWR_AddSpriteMD2(size_t spritenum);

#endif // _HW_MD2_H_

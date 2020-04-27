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
/// \brief globals (shared data & code) for hw_ modules

#ifndef _HWR_GLOB_H_
#define _HWR_GLOB_H_

#include "hw_defs.h"
#include "../m_misc.h"
#include "../r_defs.h"

// Uncomment this to enable the OpenGL loading screen
//#define HWR_LOADING_SCREEN

// --------
// hw_bsp.c
// --------
extern extrasubsector_t *extrasubsectors;
extern size_t addsubsector;

// --------
// hw_cache.c
// --------
void HWR_InitTextureCache(void);
void HWR_FreeTextureCache(void);
void HWR_FreeExtraSubsectors(void);

void HWR_GetFlat(lumpnum_t flatlumpnum, boolean noencoremap);
// ^ some flats must NOT be remapped to encore, since we remap them as we cache them for ease, adding a toggle here seems wise.

GLTexture_t *HWR_GetTexture(INT32 tex);
void HWR_GetPatch(GLPatch_t *gpatch);
void HWR_GetMappedPatch(GLPatch_t *gpatch, const UINT8 *colormap);
void HWR_MakePatch(patch_t *patch, GLPatch_t *grPatch, GLMipmap_t *grMipmap, boolean makebitmap);
void HWR_UnlockCachedPatch(GLPatch_t *gpatch);
void HWR_SetPalette(RGBA_t *palette);
GLPatch_t *HWR_GetCachedGLPatchPwad(UINT16 wad, UINT16 lump);
GLPatch_t *HWR_GetCachedGLPatch(lumpnum_t lumpnum);
void HWR_GetFadeMask(lumpnum_t fademasklumpnum);

// --------
// hw_draw.c
// --------
extern consvar_t cv_grrounddown; // on/off

extern INT32 patchformat;
extern INT32 textureformat;

#endif //_HW_GLOB_
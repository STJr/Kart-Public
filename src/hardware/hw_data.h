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
/// \brief defines structures and exports for the standard GPU driver

#ifndef _HWR_DATA_
#define _HWR_DATA_

#include "../doomdef.h"
#include "../screen.h"

// ==========================================================================
//                                                               TEXTURE INFO
// ==========================================================================

//
// hw_glide.h
//

typedef long GLAspectRatio_t;
#define GR_ASPECT_LOG2_8x1        3       /* 8W x 1H */
#define GR_ASPECT_LOG2_4x1        2       /* 4W x 1H */
#define GR_ASPECT_LOG2_2x1        1       /* 2W x 1H */
#define GR_ASPECT_LOG2_1x1        0       /* 1W x 1H */
#define GR_ASPECT_LOG2_1x2       -1       /* 1W x 2H */
#define GR_ASPECT_LOG2_1x4       -2       /* 1W x 4H */
#define GR_ASPECT_LOG2_1x8       -3       /* 1W x 8H */

typedef long GLlod_t;
#define GR_LOD_LOG2_256         0x8
#define GR_LOD_LOG2_128         0x7
#define GR_LOD_LOG2_64          0x6
#define GR_LOD_LOG2_32          0x5
#define GR_LOD_LOG2_16          0x4
#define GR_LOD_LOG2_8           0x3
#define GR_LOD_LOG2_4           0x2
#define GR_LOD_LOG2_2           0x1
#define GR_LOD_LOG2_1           0x0

typedef long GLTextureFormat_t;
#define GR_TEXFMT_ALPHA_8               0x2 /* (0..0xFF) alpha     */
#define GR_TEXFMT_INTENSITY_8           0x3 /* (0..0xFF) intensity */
#define GR_TEXFMT_ALPHA_INTENSITY_44    0x4
#define GR_TEXFMT_P_8                   0x5 /* 8-bit palette */
#define GR_TEXFMT_RGB_565               0xa
#define GR_TEXFMT_ARGB_1555             0xb
#define GR_TEXFMT_ARGB_4444             0xc
#define GR_TEXFMT_ALPHA_INTENSITY_88    0xd
#define GR_TEXFMT_AP_88                 0xe /* 8-bit alpha 8-bit palette */
#define GR_RGBA                         0x6 // 32 bit RGBA !

typedef struct
{
	GLlod_t           smallLodLog2;
	GLlod_t           largeLodLog2;
	GLAspectRatio_t   aspectRatioLog2;
	GLTextureFormat_t format;
	void              *data;
} GLTexInfo;

// grInfo.data holds the address of the graphics data cached in heap memory
//                NULL if the texture is not in Doom heap cache.
struct GLMipmap_s
{
	GLTexInfo 				grInfo;
	unsigned long 			flags;
	UINT16 					width, height;
	UINT32 					downloaded;		// tex_downloaded

	struct	GLMipmap_s		*nextmipmap;
	struct	GLMipmap_s 		*nextcolormap;
	const 	UINT8 			*colormap;
};
typedef struct GLMipmap_s GLMipmap_t;

//
// Doom texture info, as cached for hardware rendering
//
struct GLTexture_s
{
	GLMipmap_t		mipmap;
	float			scaleX;             //used for scaling textures on walls
	float			scaleY;
};
typedef struct GLTexture_s GLTexture_t;

// a cached patch as converted to hardware format, holding the original patch_t
// header so that the existing code can retrieve ->width, ->height as usual
// This is returned by W_CachePatchNum()/W_CachePatchName(), when rendermode
// is 'render_opengl'. Else it returns the normal patch_t data.
struct GLPatch_s
{
	// the 4 first fields come right away from the original patch_t
	INT16               width;
	INT16               height;
	INT16               leftoffset;     // pixels to the left of origin
	INT16               topoffset;      // pixels below the origin
	//
	float               max_s,max_t;
	UINT16              wadnum;      // the software patch lump num for when the hardware patch
	UINT16              lumpnum;     // was flushed, and we need to re-create it
	GLMipmap_t         *mipmap;
	
	boolean             notfound; // if the texture file was not found, mark it here (used in model texture loading)
} ATTRPACK;
typedef struct GLPatch_s GLPatch_t;

#endif //_HWR_DATA_

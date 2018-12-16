// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// Copyright (C) 1998-2000 by DooM Legacy Team.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//-----------------------------------------------------------------------------
/// \file
/// \brief MD2 Handling
///	Inspired from md2.c by Mete Ciragan (mete@swissquake.ch)


#ifdef __GNUC__
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../doomdef.h"
#include "../doomstat.h"

#ifdef HWRENDER
#include "hw_drv.h"
#include "hw_light.h"
#include "hw_md2.h"
#include "../d_main.h"
#include "../r_bsp.h"
#include "../r_main.h"
#include "../m_misc.h"
#include "../w_wad.h"
#include "../z_zone.h"
#include "../r_things.h"
#include "../r_draw.h"
#include "../p_tick.h"
#include "../k_kart.h" // colortranslations
#include "hw_model.h"

#include "hw_main.h"
#include "../v_video.h"
#ifdef HAVE_PNG

#ifndef _MSC_VER
#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif
#endif

#ifndef _LFS64_LARGEFILE
#define _LFS64_LARGEFILE
#endif

#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 0
#endif

 #include "png.h"
 #ifndef PNG_READ_SUPPORTED
 #undef HAVE_PNG
 #endif
 #if PNG_LIBPNG_VER < 100207
 //#undef HAVE_PNG
 #endif
#endif

#ifndef errno
#include "errno.h"
#endif

md2_t md2_models[NUMSPRITES];
md2_t md2_playermodels[MAXSKINS];


/*
 * free model
 */
static void md2_freeModel (model_t *model)
{
	UnloadModel(model);
}


//
// load model
//
// Hurdler: the current path is the Legacy.exe path
static model_t *md2_readModel(const char *filename)
{
	//Filename checking fixed ~Monster Iestyn and Golden
	return LoadModel(va("%s"PATHSEP"%s", srb2home, filename), PU_STATIC);
}

static inline void md2_printModelInfo (model_t *model)
{
#if 0
	INT32 i;

	CONS_Debug(DBG_RENDER, "magic:\t\t\t%c%c%c%c\n", model->header.magic>>24,
	            (model->header.magic>>16)&0xff,
	            (model->header.magic>>8)&0xff,
	             model->header.magic&0xff);
	CONS_Debug(DBG_RENDER, "version:\t\t%d\n", model->header.version);
	CONS_Debug(DBG_RENDER, "skinWidth:\t\t%d\n", model->header.skinWidth);
	CONS_Debug(DBG_RENDER, "skinHeight:\t\t%d\n", model->header.skinHeight);
	CONS_Debug(DBG_RENDER, "frameSize:\t\t%d\n", model->header.frameSize);
	CONS_Debug(DBG_RENDER, "numSkins:\t\t%d\n", model->header.numSkins);
	CONS_Debug(DBG_RENDER, "numVertices:\t\t%d\n", model->header.numVertices);
	CONS_Debug(DBG_RENDER, "numTexCoords:\t\t%d\n", model->header.numTexCoords);
	CONS_Debug(DBG_RENDER, "numTriangles:\t\t%d\n", model->header.numTriangles);
	CONS_Debug(DBG_RENDER, "numGlCommands:\t\t%d\n", model->header.numGlCommands);
	CONS_Debug(DBG_RENDER, "numFrames:\t\t%d\n", model->header.numFrames);
	CONS_Debug(DBG_RENDER, "offsetSkins:\t\t%d\n", model->header.offsetSkins);
	CONS_Debug(DBG_RENDER, "offsetTexCoords:\t%d\n", model->header.offsetTexCoords);
	CONS_Debug(DBG_RENDER, "offsetTriangles:\t%d\n", model->header.offsetTriangles);
	CONS_Debug(DBG_RENDER, "offsetFrames:\t\t%d\n", model->header.offsetFrames);
	CONS_Debug(DBG_RENDER, "offsetGlCommands:\t%d\n", model->header.offsetGlCommands);
	CONS_Debug(DBG_RENDER, "offsetEnd:\t\t%d\n", model->header.offsetEnd);

	for (i = 0; i < model->header.numFrames; i++)
		CONS_Debug(DBG_RENDER, "%s ", model->frames[i].name);
	CONS_Debug(DBG_RENDER, "\n");
#else
	(void)model;
#endif
}

#ifdef HAVE_PNG
static void PNG_error(png_structp PNG, png_const_charp pngtext)
{
	CONS_Debug(DBG_RENDER, "libpng error at %p: %s", PNG, pngtext);
	//I_Error("libpng error at %p: %s", PNG, pngtext);
}

static void PNG_warn(png_structp PNG, png_const_charp pngtext)
{
	CONS_Debug(DBG_RENDER, "libpng warning at %p: %s", PNG, pngtext);
}

static GrTextureFormat_t PNG_Load(const char *filename, int *w, int *h, GLPatch_t *grpatch)
{
	png_structp png_ptr;
	png_infop png_info_ptr;
	png_uint_32 width, height;
	int bit_depth, color_type;
#ifdef PNG_SETJMP_SUPPORTED
#ifdef USE_FAR_KEYWORD
	jmp_buf jmpbuf;
#endif
#endif
	png_FILE_p png_FILE;
	//Filename checking fixed ~Monster Iestyn and Golden
	char *pngfilename = va("%s"PATHSEP"md2"PATHSEP"%s", srb2home, filename);

	FIL_ForceExtension(pngfilename, ".png");
	png_FILE = fopen(pngfilename, "rb");
	if (!png_FILE)
	{
		//CONS_Debug(DBG_RENDER, "M_SavePNG: Error on opening %s for loading\n", filename);
		return 0;
	}

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL,
		PNG_error, PNG_warn);
	if (!png_ptr)
	{
		CONS_Debug(DBG_RENDER, "PNG_Load: Error on initialize libpng\n");
		fclose(png_FILE);
		return 0;
	}

	png_info_ptr = png_create_info_struct(png_ptr);
	if (!png_info_ptr)
	{
		CONS_Debug(DBG_RENDER, "PNG_Load: Error on allocate for libpng\n");
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		fclose(png_FILE);
		return 0;
	}

#ifdef USE_FAR_KEYWORD
	if (setjmp(jmpbuf))
#else
	if (setjmp(png_jmpbuf(png_ptr)))
#endif
	{
		//CONS_Debug(DBG_RENDER, "libpng load error on %s\n", filename);
		png_destroy_read_struct(&png_ptr, &png_info_ptr, NULL);
		fclose(png_FILE);
		Z_Free(grpatch->mipmap.grInfo.data);
		return 0;
	}
#ifdef USE_FAR_KEYWORD
	png_memcpy(png_jmpbuf(png_ptr), jmpbuf, sizeof jmp_buf);
#endif

	png_init_io(png_ptr, png_FILE);

#ifdef PNG_SET_USER_LIMITS_SUPPORTED
	png_set_user_limits(png_ptr, 2048, 2048);
#endif

	png_read_info(png_ptr, png_info_ptr);

	png_get_IHDR(png_ptr, png_info_ptr, &width, &height, &bit_depth, &color_type,
	 NULL, NULL, NULL);

	if (bit_depth == 16)
		png_set_strip_16(png_ptr);

	if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(png_ptr);
	else if (color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(png_ptr);

	if (png_get_valid(png_ptr, png_info_ptr, PNG_INFO_tRNS))
		png_set_tRNS_to_alpha(png_ptr);
	else if (color_type != PNG_COLOR_TYPE_RGB_ALPHA && color_type != PNG_COLOR_TYPE_GRAY_ALPHA)
	{
#if PNG_LIBPNG_VER < 10207
		png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);
#else
		png_set_add_alpha(png_ptr, 0xFF, PNG_FILLER_AFTER);
#endif
	}

	png_read_update_info(png_ptr, png_info_ptr);

	{
		png_uint_32 i, pitch = png_get_rowbytes(png_ptr, png_info_ptr);
		png_bytep PNG_image = Z_Malloc(pitch*height, PU_HWRCACHE, &grpatch->mipmap.grInfo.data);
		png_bytepp row_pointers = png_malloc(png_ptr, height * sizeof (png_bytep));
		for (i = 0; i < height; i++)
			row_pointers[i] = PNG_image + i*pitch;
		png_read_image(png_ptr, row_pointers);
		png_free(png_ptr, (png_voidp)row_pointers);
	}

	png_destroy_read_struct(&png_ptr, &png_info_ptr, NULL);

	fclose(png_FILE);
	*w = (int)width;
	*h = (int)height;
	return GR_RGBA;
}
#endif

typedef struct
{
	UINT8 manufacturer;
	UINT8 version;
	UINT8 encoding;
	UINT8 bitsPerPixel;
	INT16 xmin;
	INT16 ymin;
	INT16 xmax;
	INT16 ymax;
	INT16 hDpi;
	INT16 vDpi;
	UINT8 colorMap[48];
	UINT8 reserved;
	UINT8 numPlanes;
	INT16 bytesPerLine;
	INT16 paletteInfo;
	INT16 hScreenSize;
	INT16 vScreenSize;
	UINT8 filler[54];
} PcxHeader;

static GrTextureFormat_t PCX_Load(const char *filename, int *w, int *h,
	GLPatch_t *grpatch)
{
	PcxHeader header;
#define PALSIZE 768
	UINT8 palette[PALSIZE];
	const UINT8 *pal;
	RGBA_t *image;
	size_t pw, ph, size, ptr = 0;
	INT32 ch, rep;
	FILE *file;
	//Filename checking fixed ~Monster Iestyn and Golden
	char *pcxfilename = va("%s"PATHSEP"md2"PATHSEP"%s", srb2home, filename);

	FIL_ForceExtension(pcxfilename, ".pcx");
	file = fopen(pcxfilename, "rb");
	if (!file)
		return 0;

	if (fread(&header, sizeof (PcxHeader), 1, file) != 1)
	{
		fclose(file);
		return 0;
	}

	if (header.bitsPerPixel != 8)
	{
		fclose(file);
		return 0;
	}

	fseek(file, -PALSIZE, SEEK_END);

	pw = *w = header.xmax - header.xmin + 1;
	ph = *h = header.ymax - header.ymin + 1;
	image = Z_Malloc(pw*ph*4, PU_HWRCACHE, &grpatch->mipmap.grInfo.data);

	if (fread(palette, sizeof (UINT8), PALSIZE, file) != PALSIZE)
	{
		Z_Free(image);
		fclose(file);
		return 0;
	}
	fseek(file, sizeof (PcxHeader), SEEK_SET);

	size = pw * ph;
	while (ptr < size)
	{
		ch = fgetc(file);  //Hurdler: beurk
		if (ch >= 192)
		{
			rep = ch - 192;
			ch = fgetc(file);
		}
		else
		{
			rep = 1;
		}
		while (rep--)
		{
			pal = palette + ch*3;
			image[ptr].s.red   = *pal++;
			image[ptr].s.green = *pal++;
			image[ptr].s.blue  = *pal++;
			image[ptr].s.alpha = 0xFF;
			ptr++;
		}
	}
	fclose(file);
	return GR_RGBA;
}

// -----------------+
// md2_loadTexture  : Download a pcx or png texture for MD2 models
// -----------------+
static void md2_loadTexture(md2_t *model)
{
	GLPatch_t *grpatch;
	const char *filename = model->filename;

	if (model->grpatch)
	{
		grpatch = model->grpatch;
		Z_Free(grpatch->mipmap.grInfo.data);
	}
	else
		grpatch = Z_Calloc(sizeof *grpatch, PU_HWRPATCHINFO,
		                   &(model->grpatch));

	if (!grpatch->mipmap.downloaded && !grpatch->mipmap.grInfo.data)
	{
		int w = 0, h = 0;
#ifdef HAVE_PNG
		grpatch->mipmap.grInfo.format = PNG_Load(filename, &w, &h, grpatch);
		if (grpatch->mipmap.grInfo.format == 0)
#endif
		grpatch->mipmap.grInfo.format = PCX_Load(filename, &w, &h, grpatch);
		if (grpatch->mipmap.grInfo.format == 0)
			return;

		grpatch->mipmap.downloaded = 0;
		grpatch->mipmap.flags = 0;

		grpatch->width = (INT16)w;
		grpatch->height = (INT16)h;
		grpatch->mipmap.width = (UINT16)w;
		grpatch->mipmap.height = (UINT16)h;

		// not correct!
		grpatch->mipmap.grInfo.smallLodLog2 = GR_LOD_LOG2_256;
		grpatch->mipmap.grInfo.largeLodLog2 = GR_LOD_LOG2_256;
		grpatch->mipmap.grInfo.aspectRatioLog2 = GR_ASPECT_LOG2_1x1;
	}
	HWD.pfnSetTexture(&grpatch->mipmap);
	HWR_UnlockCachedPatch(grpatch);
}

// -----------------+
// md2_loadBlendTexture  : Download a pcx or png texture for blending MD2 models
// -----------------+
static void md2_loadBlendTexture(md2_t *model)
{
	GLPatch_t *grpatch;
	char *filename = Z_Malloc(strlen(model->filename)+7, PU_STATIC, NULL);
	strcpy(filename, model->filename);

	FIL_ForceExtension(filename, "_blend.png");

	if (model->blendgrpatch)
	{
		grpatch = model->blendgrpatch;
		Z_Free(grpatch->mipmap.grInfo.data);
	}
	else
		grpatch = Z_Calloc(sizeof *grpatch, PU_HWRPATCHINFO,
		                   &(model->blendgrpatch));

	if (!grpatch->mipmap.downloaded && !grpatch->mipmap.grInfo.data)
	{
		int w = 0, h = 0;
#ifdef HAVE_PNG
		grpatch->mipmap.grInfo.format = PNG_Load(filename, &w, &h, grpatch);
		if (grpatch->mipmap.grInfo.format == 0)
#endif
		grpatch->mipmap.grInfo.format = PCX_Load(filename, &w, &h, grpatch);
		if (grpatch->mipmap.grInfo.format == 0)
		{
			Z_Free(filename);
			return;
		}

		grpatch->mipmap.downloaded = 0;
		grpatch->mipmap.flags = 0;

		grpatch->width = (INT16)w;
		grpatch->height = (INT16)h;
		grpatch->mipmap.width = (UINT16)w;
		grpatch->mipmap.height = (UINT16)h;

		// not correct!
		grpatch->mipmap.grInfo.smallLodLog2 = GR_LOD_LOG2_256;
		grpatch->mipmap.grInfo.largeLodLog2 = GR_LOD_LOG2_256;
		grpatch->mipmap.grInfo.aspectRatioLog2 = GR_ASPECT_LOG2_1x1;
	}
	HWD.pfnSetTexture(&grpatch->mipmap); // We do need to do this so that it can be cleared and knows to recreate it when necessary
	HWR_UnlockCachedPatch(grpatch);

	Z_Free(filename);
}

// Don't spam the console, or the OS with fopen requests!
static boolean nomd2s = false;

void HWR_InitMD2(void)
{
	size_t i;
	INT32 s;
	FILE *f;
	char name[18], filename[32];
	float scale, offset;

	CONS_Printf("InitMD2()...\n");
	for (s = 0; s < MAXSKINS; s++)
	{
		md2_playermodels[s].scale = -1.0f;
		md2_playermodels[s].model = NULL;
		md2_playermodels[s].grpatch = NULL;
		md2_playermodels[s].skin = -1;
		md2_playermodels[s].notfound = true;
		md2_playermodels[s].error = false;
	}
	for (i = 0; i < NUMSPRITES; i++)
	{
		md2_models[i].scale = -1.0f;
		md2_models[i].model = NULL;
		md2_models[i].grpatch = NULL;
		md2_models[i].skin = -1;
		md2_models[i].notfound = true;
		md2_models[i].error = false;
	}

	// read the md2.dat file
	//Filename checking fixed ~Monster Iestyn and Golden
	f = fopen(va("%s"PATHSEP"%s", srb2home, "kmd2.dat"), "rt");

	if (!f)
	{
		CONS_Printf("%s %s\n", M_GetText("Error while loading kmd2.dat:"), strerror(errno));
		nomd2s = true;
		return;
	}
	while (fscanf(f, "%19s %31s %f %f", name, filename, &scale, &offset) == 4)
	{
		if (stricmp(name, "PLAY") == 0)
		{
			CONS_Printf("MD2 for sprite PLAY detected in kmd2.dat, use a player skin instead!\n");
			continue;
		}

		for (i = 0; i < NUMSPRITES; i++)
		{
			if (stricmp(name, sprnames[i]) == 0)
			{
				//if (stricmp(name, "PLAY") == 0)
					//continue;

				//CONS_Debug(DBG_RENDER, "  Found: %s %s %f %f\n", name, filename, scale, offset);
				md2_models[i].scale = scale;
				md2_models[i].offset = offset;
				md2_models[i].notfound = false;
				strcpy(md2_models[i].filename, filename);
				goto md2found;
			}
		}

		for (s = 0; s < MAXSKINS; s++)
		{
			if (stricmp(name, skins[s].name) == 0)
			{
				//CONS_Printf("  Found: %s %s %f %f\n", name, filename, scale, offset);
				md2_playermodels[s].skin = s;
				md2_playermodels[s].scale = scale;
				md2_playermodels[s].offset = offset;
				md2_playermodels[s].notfound = false;
				strcpy(md2_playermodels[s].filename, filename);
				goto md2found;
			}
		}
		// no sprite/player skin name found?!?
		CONS_Printf("Unknown sprite/player skin %s detected in kmd2.dat\n", name);
md2found:
		// move on to next line...
		continue;
	}
	fclose(f);
}

void HWR_AddPlayerMD2(int skin) // For MD2's that were added after startup
{
	FILE *f;
	char name[18], filename[32];
	float scale, offset;

	if (nomd2s)
		return;

	CONS_Printf("AddPlayerMD2()...\n");

	// read the md2.dat file
	//Filename checking fixed ~Monster Iestyn and Golden
	f = fopen(va("%s"PATHSEP"%s", srb2home, "kmd2.dat"), "rt");

	if (!f)
	{
		CONS_Printf("Error while loading kmd2.dat\n");
		nomd2s = true;
		return;
	}

	// Check for any MD2s that match the names of player skins!
	while (fscanf(f, "%19s %31s %f %f", name, filename, &scale, &offset) == 4)
	{
		if (stricmp(name, skins[skin].name) == 0)
		{
			md2_playermodels[skin].skin = skin;
			md2_playermodels[skin].scale = scale;
			md2_playermodels[skin].offset = offset;
			md2_playermodels[skin].notfound = false;
			strcpy(md2_playermodels[skin].filename, filename);
			goto playermd2found;
		}
	}

	//CONS_Printf("MD2 for player skin %s not found\n", skins[skin].name);
	md2_playermodels[skin].notfound = true;
playermd2found:
	fclose(f);
}


void HWR_AddSpriteMD2(size_t spritenum) // For MD2s that were added after startup
{
	FILE *f;
	// name[18] is used to check for names in the kmd2.dat file that match with sprites or player skins
	// sprite names are always 4 characters long, and names is for player skins can be up to 19 characters long
	char name[18], filename[32];
	float scale, offset;

	if (nomd2s)
		return;

	if (spritenum == SPR_PLAY) // Handled already NEWMD2: Per sprite, per-skin check
		return;

	// Read the md2.dat file
	//Filename checking fixed ~Monster Iestyn and Golden
	f = fopen(va("%s"PATHSEP"%s", srb2home, "kmd2.dat"), "rt");

	if (!f)
	{
		CONS_Printf("Error while loading kmd2.dat\n");
		nomd2s = true;
		return;
	}

	// Check for any MD2s that match the names of sprite names!
	while (fscanf(f, "%19s %31s %f %f", name, filename, &scale, &offset) == 4)
	{
		if (stricmp(name, sprnames[spritenum]) == 0)
		{
			md2_models[spritenum].scale = scale;
			md2_models[spritenum].offset = offset;
			md2_models[spritenum].notfound = false;
			strcpy(md2_models[spritenum].filename, filename);
			goto spritemd2found;
		}
	}

	//CONS_Printf("MD2 for sprite %s not found\n", sprnames[spritenum]);
	md2_models[spritenum].notfound = true;
spritemd2found:
	fclose(f);
}

// Define for getting accurate color brightness readings according to how the human eye sees them.
// https://en.wikipedia.org/wiki/Relative_luminance
// 0.2126 to red
// 0.7152 to green
// 0.0722 to blue
// (See this same define in k_kart.c!)
#define SETBRIGHTNESS(brightness,r,g,b) \
	brightness = (UINT8)(((1063*((UINT16)r)/5000) + (3576*((UINT16)g)/5000) + (361*((UINT16)b)/5000)) / 3)

static void HWR_CreateBlendedTexture(GLPatch_t *gpatch, GLPatch_t *blendgpatch, GLMipmap_t *grmip, INT32 skinnum, skincolors_t color)
{
	UINT8 i;
	UINT16 w = gpatch->width, h = gpatch->height;
	UINT32 size = w*h;
	RGBA_t *image, *blendimage, *cur, blendcolor;

	if (grmip->width == 0)
	{

		grmip->width = gpatch->width;
		grmip->height = gpatch->height;

		// no wrap around, no chroma key
		grmip->flags = 0;
		// setup the texture info
		grmip->grInfo.format = GR_RGBA;
	}

	Z_Free(grmip->grInfo.data);
	grmip->grInfo.data = NULL;

	cur = Z_Malloc(size*4, PU_HWRCACHE, &grmip->grInfo.data);
	memset(cur, 0x00, size*4);

	image = gpatch->mipmap.grInfo.data;
	blendimage = blendgpatch->mipmap.grInfo.data;

	// Average all of the translation's colors
	{
		const UINT8 div = 6;
		const UINT8 start = 4;
		UINT32 r, g, b;

		blendcolor = V_GetColor(colortranslations[color][start]);
		r = (UINT32)(blendcolor.s.red*blendcolor.s.red);
		g = (UINT32)(blendcolor.s.green*blendcolor.s.green);
		b = (UINT32)(blendcolor.s.blue*blendcolor.s.blue);

		for (i = 1; i < div; i++)
		{
			RGBA_t nextcolor = V_GetColor(colortranslations[color][start+i]);
			r += (UINT32)(nextcolor.s.red*nextcolor.s.red);
			g += (UINT32)(nextcolor.s.green*nextcolor.s.green);
			b += (UINT32)(nextcolor.s.blue*nextcolor.s.blue);
		}

		blendcolor.s.red = (UINT8)(FixedSqrt((r/div)<<FRACBITS)>>FRACBITS);
		blendcolor.s.green = (UINT8)(FixedSqrt((g/div)<<FRACBITS)>>FRACBITS);
		blendcolor.s.blue = (UINT8)(FixedSqrt((b/div)<<FRACBITS)>>FRACBITS);
	}

	// rainbow support, could theoretically support boss ones too
	if (skinnum == TC_RAINBOW)
	{
		while (size--)
		{
			if (image->s.alpha == 0 && blendimage->s.alpha == 0)
			{
				// Don't bother with blending the pixel if the alpha of the blend pixel is 0
				cur->rgba = image->rgba;
			}
			else
			{
				UINT32 tempcolor;
				UINT16 imagebright, blendbright, finalbright, colorbright;
				SETBRIGHTNESS(imagebright,image->s.red,image->s.green,image->s.blue);
				SETBRIGHTNESS(blendbright,blendimage->s.red,blendimage->s.green,blendimage->s.blue);
				// slightly dumb average between the blend image color and base image colour, usually one or the other will be fully opaque anyway
				finalbright = (imagebright*(255-blendimage->s.alpha))/255 + (blendbright*blendimage->s.alpha)/255;
				SETBRIGHTNESS(colorbright,blendcolor.s.red,blendcolor.s.green,blendcolor.s.blue);

				tempcolor = (finalbright*blendcolor.s.red)/colorbright;
				tempcolor = min(255, tempcolor);
				cur->s.red = (UINT8)tempcolor;
				tempcolor = (finalbright*blendcolor.s.green)/colorbright;
				tempcolor = min(255, tempcolor);
				cur->s.green = (UINT8)tempcolor;
				tempcolor = (finalbright*blendcolor.s.blue)/colorbright;
				tempcolor = min(255, tempcolor);
				cur->s.blue = (UINT8)tempcolor;
				cur->s.alpha = image->s.alpha;
			}

			cur++; image++; blendimage++;
		}
	}
	else
	{
		while (size--)
		{
			if (blendimage->s.alpha == 0)
			{
				// Don't bother with blending the pixel if the alpha of the blend pixel is 0
				cur->rgba = image->rgba;
			}
			else
			{
				INT32 tempcolor;
				INT16 tempmult, tempalpha;
				tempalpha = -(abs(blendimage->s.red-127)-127)*2;
				if (tempalpha > 255)
					tempalpha = 255;
				else if (tempalpha < 0)
					tempalpha = 0;

				tempmult = (blendimage->s.red-127)*2;
				if (tempmult > 255)
					tempmult = 255;
				else if (tempmult < 0)
					tempmult = 0;

				tempcolor = (image->s.red*(255-blendimage->s.alpha))/255 + ((tempmult + ((tempalpha*blendcolor.s.red)/255)) * blendimage->s.alpha)/255;
				cur->s.red = (UINT8)tempcolor;
				tempcolor = (image->s.green*(255-blendimage->s.alpha))/255 + ((tempmult + ((tempalpha*blendcolor.s.green)/255)) * blendimage->s.alpha)/255;
				cur->s.green = (UINT8)tempcolor;
				tempcolor = (image->s.blue*(255-blendimage->s.alpha))/255 + ((tempmult + ((tempalpha*blendcolor.s.blue)/255)) * blendimage->s.alpha)/255;
				cur->s.blue = (UINT8)tempcolor;
				cur->s.alpha = image->s.alpha;
			}

			cur++; image++; blendimage++;
		}
	}

	return;
}

#undef SETBRIGHTNESS

static void HWR_GetBlendedTexture(GLPatch_t *gpatch, GLPatch_t *blendgpatch, INT32 skinnum, const UINT8 *colormap, skincolors_t color)
{
	// mostly copied from HWR_GetMappedPatch, hence the similarities and comment
	GLMipmap_t *grmip, *newmip;

	if (colormap == colormaps || colormap == NULL)
	{
		// Don't do any blending
		HWD.pfnSetTexture(&gpatch->mipmap);
		return;
	}

	// search for the mimmap
	// skip the first (no colormap translated)
	for (grmip = &gpatch->mipmap; grmip->nextcolormap; )
	{
		grmip = grmip->nextcolormap;
		if (grmip->colormap == colormap)
		{
			if (grmip->downloaded && grmip->grInfo.data)
			{
				HWD.pfnSetTexture(grmip); // found the colormap, set it to the correct texture
				Z_ChangeTag(grmip->grInfo.data, PU_HWRCACHE_UNLOCKED);
				return;
			}
		}
	}

	// If here, the blended texture has not been created
	// So we create it

	//BP: WARNING: don't free it manually without clearing the cache of harware renderer
	//              (it have a liste of mipmap)
	//    this malloc is cleared in HWR_FreeTextureCache
	//    (...) unfortunately z_malloc fragment alot the memory :(so malloc is better
	newmip = calloc(1, sizeof (*newmip));
	if (newmip == NULL)
		I_Error("%s: Out of memory", "HWR_GetMappedPatch");
	grmip->nextcolormap = newmip;
	newmip->colormap = colormap;

	HWR_CreateBlendedTexture(gpatch, blendgpatch, newmip, skinnum, color);

	HWD.pfnSetTexture(newmip);
	Z_ChangeTag(newmip->grInfo.data, PU_HWRCACHE_UNLOCKED);
}


// -----------------+
// HWR_DrawMD2      : Draw MD2
//                  : (monsters, bonuses, weapons, lights, ...)
// Returns          :
// -----------------+
	/*
	wait/stand
	death
	pain
	walk
	shoot/fire

	die?
	atka?
	atkb?
	attacka/b/c/d?
	res?
	run?
	*/
#define NORMALFOG 0x00000000
#define FADEFOG 0x19000000
void HWR_DrawMD2(gr_vissprite_t *spr)
{
	FSurfaceInfo Surf;

	char filename[64];
	INT32 frame;
	FTransform p;
	md2_t *md2;
	UINT8 color[4];

	if (!cv_grmd2.value)
		return;

	if (spr->precip)
		return;

	// MD2 colormap fix
	// colormap test
	{
		sector_t *sector = spr->mobj->subsector->sector;
		UINT8 lightlevel = 255;
		extracolormap_t *colormap = sector->extra_colormap;

		if (sector->numlights)
		{
			INT32 light;

			light = R_GetPlaneLight(sector, spr->mobj->z + spr->mobj->height, false); // Always use the light at the top instead of whatever I was doing before

			if (!(spr->mobj->frame & FF_FULLBRIGHT))
				lightlevel = *sector->lightlist[light].lightlevel;

			if (sector->lightlist[light].extra_colormap)
				colormap = sector->lightlist[light].extra_colormap;
		}
		else
		{
			if (!(spr->mobj->frame & FF_FULLBRIGHT))
				lightlevel = sector->lightlevel;

			if (sector->extra_colormap)
				colormap = sector->extra_colormap;
		}

		if (colormap)
			Surf.FlatColor.rgba = HWR_Lighting(lightlevel, colormap->rgba, colormap->fadergba, false, false);
		else
			Surf.FlatColor.rgba = HWR_Lighting(lightlevel, NORMALFOG, FADEFOG, false, false);
	}

	// Look at HWR_ProjectSprite for more
	{
		GLPatch_t *gpatch;
		INT32 durs = spr->mobj->state->tics;
		INT32 tics = spr->mobj->tics;
		mdlframe_t *curr, *next = NULL;
		const UINT8 flip = (UINT8)((spr->mobj->eflags & MFE_VERTICALFLIP) == MFE_VERTICALFLIP);
		spritedef_t *sprdef;
		spriteframe_t *sprframe;
		float finalscale;

		// Apparently people don't like jump frames like that, so back it goes
		//if (tics > durs)
			//durs = tics;

		if (spr->mobj->flags2 & MF2_SHADOW)
			Surf.FlatColor.s.alpha = 0x40;
		else if (spr->mobj->frame & FF_TRANSMASK)
			HWR_TranstableToAlpha((spr->mobj->frame & FF_TRANSMASK)>>FF_TRANSSHIFT, &Surf);
		else
			Surf.FlatColor.s.alpha = 0xFF;

		// dont forget to enabled the depth test because we can't do this like
		// before: polygons models are not sorted

		// 1. load model+texture if not already loaded
		// 2. draw model with correct position, rotation,...
		if (spr->mobj->skin && spr->mobj->sprite == SPR_PLAY) // Use the player MD2 list if the mobj has a skin and is using the player sprites
		{
			md2 = &md2_playermodels[(skin_t*)spr->mobj->skin-skins];
			md2->skin = (skin_t*)spr->mobj->skin-skins;
		}
		else
			md2 = &md2_models[spr->mobj->sprite];

		if (md2->error)
			return; // we already failed loading this before :(
		if (!md2->model)
		{
			//CONS_Debug(DBG_RENDER, "Loading MD2... (%s)", sprnames[spr->mobj->sprite]);
			sprintf(filename, "md2/%s", md2->filename);
			md2->model = md2_readModel(filename);

			if (md2->model)
			{
				md2_printModelInfo(md2->model);
			}
			else
			{
				//CONS_Debug(DBG_RENDER, " FAILED\n");
				md2->error = true; // prevent endless fail
				return;
			}
		}
		//HWD.pfnSetBlend(blend); // This seems to actually break translucency?
		finalscale = md2->scale;
		//Hurdler: arf, I don't like that implementation at all... too much crappy
		gpatch = md2->grpatch;
		if (!gpatch || !gpatch->mipmap.grInfo.format || !gpatch->mipmap.downloaded)
			md2_loadTexture(md2);
		gpatch = md2->grpatch; // Load it again, because it isn't being loaded into gpatch after md2_loadtexture...

		if ((gpatch && gpatch->mipmap.grInfo.format) // don't load the blend texture if the base texture isn't available
			&& (!md2->blendgrpatch || !((GLPatch_t *)md2->blendgrpatch)->mipmap.grInfo.format || !((GLPatch_t *)md2->blendgrpatch)->mipmap.downloaded))
			md2_loadBlendTexture(md2);

		if (gpatch && gpatch->mipmap.grInfo.format) // else if meant that if a texture couldn't be loaded, it would just end up using something else's texture
		{
			if ((skincolors_t)spr->mobj->color != SKINCOLOR_NONE &&
				md2->blendgrpatch && ((GLPatch_t *)md2->blendgrpatch)->mipmap.grInfo.format
				&& gpatch->width == ((GLPatch_t *)md2->blendgrpatch)->width && gpatch->height == ((GLPatch_t *)md2->blendgrpatch)->height)
			{
				INT32 skinnum;
				if ((spr->mobj->flags & MF_BOSS) && (spr->mobj->flags2 & MF2_FRET) && (leveltime & 1)) // Bosses "flash"
				{
					if (spr->mobj->type == MT_CYBRAKDEMON)
						skinnum = TC_ALLWHITE;
					else if (spr->mobj->type == MT_METALSONIC_BATTLE)
						skinnum = TC_METALSONIC;
					else
						skinnum = TC_BOSS;
				}
				else if (spr->mobj->color)
				{
					if (spr->mobj->skin && spr->mobj->sprite == SPR_PLAY)
					{
						if (spr->mobj->colorized)
							skinnum = TC_RAINBOW;
						else
						{
							skinnum = (INT32)((skin_t*)spr->mobj->skin-skins);
						}
					}
					else skinnum = TC_DEFAULT;
				}
				HWR_GetBlendedTexture(gpatch, (GLPatch_t *)md2->blendgrpatch, skinnum, spr->colormap, (skincolors_t)spr->mobj->color);
			}
			else
			{
				// This is safe, since we know the texture has been downloaded
				HWD.pfnSetTexture(&gpatch->mipmap);
			}
		}
		else
		{
			// Sprite
			gpatch = W_CachePatchNum(spr->patchlumpnum, PU_CACHE);
			HWR_GetMappedPatch(gpatch, spr->colormap);
		}

		if (spr->mobj->frame & FF_ANIMATE)
		{
			// set duration and tics to be the correct values for FF_ANIMATE states
			durs = spr->mobj->state->var2;
			tics = spr->mobj->anim_duration;
		}

		//FIXME: this is not yet correct
		frame = (spr->mobj->frame & FF_FRAMEMASK) % md2->model->meshes[0].numFrames;
		curr = &md2->model->meshes[0].frames[frame];
#if 0
		if (cv_grmd2.value == 1 && tics <= durs)
		{
			// frames are handled differently for states with FF_ANIMATE, so get the next frame differently for the interpolation
			if (spr->mobj->frame & FF_ANIMATE)
			{
				UINT32 nextframe = (spr->mobj->frame & FF_FRAMEMASK) + 1;
				if (nextframe >= (UINT32)spr->mobj->state->var1)
					nextframe = (spr->mobj->state->frame & FF_FRAMEMASK);
				nextframe %= md2->model->header.numFrames;
				next = &md2->model->frames[nextframe];
			}
			else
			{
				if (spr->mobj->state->nextstate != S_NULL && states[spr->mobj->state->nextstate].sprite != SPR_NULL)
				{
					const UINT32 nextframe = (states[spr->mobj->state->nextstate].frame & FF_FRAMEMASK) % md2->model->header.numFrames;
					next = &md2->model->frames[nextframe];
				}
			}
		}
#endif

		//Hurdler: it seems there is still a small problem with mobj angle
		p.x = FIXED_TO_FLOAT(spr->mobj->x);
		p.y = FIXED_TO_FLOAT(spr->mobj->y)+md2->offset;

		if (spr->mobj->eflags & MFE_VERTICALFLIP)
			p.z = FIXED_TO_FLOAT(spr->mobj->z + spr->mobj->height);
		else
			p.z = FIXED_TO_FLOAT(spr->mobj->z);

		if (spr->mobj->skin && spr->mobj->sprite == SPR_PLAY)
			sprdef = &((skin_t *)spr->mobj->skin)->spritedef;
		else
			sprdef = &sprites[spr->mobj->sprite];

		sprframe = &sprdef->spriteframes[spr->mobj->frame & FF_FRAMEMASK];

		if (sprframe->rotate)
		{
			fixed_t anglef;
			if (spr->mobj->player)
				anglef = AngleFixed(spr->mobj->player->frameangle);
			else
				anglef = AngleFixed(spr->mobj->angle);
			p.angley = FIXED_TO_FLOAT(anglef);
		}
		else
		{
			const fixed_t anglef = AngleFixed((R_PointToAngle(spr->mobj->x, spr->mobj->y))-ANGLE_180);
			p.angley = FIXED_TO_FLOAT(anglef);
		}
		p.anglex = 0.0f;
		p.anglez = 0.0f;
		if (spr->mobj->standingslope)
		{
			fixed_t tempz = spr->mobj->standingslope->normal.z;
			fixed_t tempy = spr->mobj->standingslope->normal.y;
			fixed_t tempx = spr->mobj->standingslope->normal.x;
			fixed_t tempangle = AngleFixed(R_PointToAngle2(0, 0, FixedSqrt(FixedMul(tempy, tempy) + FixedMul(tempz, tempz)), tempx));
			p.anglez = FIXED_TO_FLOAT(tempangle);
			tempangle = -AngleFixed(R_PointToAngle2(0, 0, tempz, tempy));
			p.anglex = FIXED_TO_FLOAT(tempangle);
		}

		color[0] = Surf.FlatColor.s.red;
		color[1] = Surf.FlatColor.s.green;
		color[2] = Surf.FlatColor.s.blue;
		color[3] = Surf.FlatColor.s.alpha;

		// SRB2CBTODO: MD2 scaling support
		finalscale *= FIXED_TO_FLOAT(spr->mobj->scale);

		p.flip = atransform.flip;
		p.mirror = atransform.mirror;

		HWD.pfnDrawModel(md2->model, curr, durs, tics, next, &p, finalscale, flip, color);
	}
}

#endif //HWRENDER

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
/// \brief hardware renderer, using the standard HardWareRender driver DLL for SRB2

#include <math.h>

#include "../doomstat.h"

#ifdef HWRENDER
#include "hw_main.h"
#include "hw_glob.h"
#include "hw_drv.h"
#include "hw_md2.h"
#include "hw_clip.h"
#include "hw_light.h"

#include "../i_video.h" // for rendermode == render_glide
#include "../v_video.h"
#include "../p_local.h"
#include "../p_setup.h"
#include "../r_local.h"
#include "../r_bsp.h"	// R_NoEncore
#include "../r_main.h"	// cv_fov
#include "../d_clisrv.h"
#include "../w_wad.h"
#include "../z_zone.h"
#include "../r_splats.h"
#include "../g_game.h"
#include "../st_stuff.h"
#include "../i_system.h"
#include "../m_cheat.h"

#ifdef ESLOPE
#include "../p_slopes.h"
#endif

#include <stdlib.h> // qsort

#define ABS(x) ((x) < 0 ? -(x) : (x))

// ==========================================================================
// the hardware driver object
// ==========================================================================
struct hwdriver_s hwdriver;

// ==========================================================================
// Commands and console variables
// ==========================================================================

static void CV_filtermode_ONChange(void);
static void CV_anisotropic_ONChange(void);

static CV_PossibleValue_t grfiltermode_cons_t[]= {{HWD_SET_TEXTUREFILTER_POINTSAMPLED, "Nearest"},
	{HWD_SET_TEXTUREFILTER_BILINEAR, "Bilinear"}, {HWD_SET_TEXTUREFILTER_TRILINEAR, "Trilinear"},
	{HWD_SET_TEXTUREFILTER_MIXED1, "Linear_Nearest"},
	{HWD_SET_TEXTUREFILTER_MIXED2, "Nearest_Linear"},
	{HWD_SET_TEXTUREFILTER_MIXED3, "Nearest_Mipmap"},
	{0, NULL}};
CV_PossibleValue_t granisotropicmode_cons_t[] = {{1, "MIN"}, {16, "MAX"}, {0, NULL}};

consvar_t cv_grrounddown = {"gr_rounddown", "Off", 0, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL};


consvar_t cv_grfiltermode = {"gr_filtermode", "Nearest", CV_CALL|CV_SAVE, grfiltermode_cons_t,
                             CV_filtermode_ONChange, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_granisotropicmode = {"gr_anisotropicmode", "1", CV_CALL|CV_SAVE, granisotropicmode_cons_t,
                             CV_anisotropic_ONChange, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_grcorrecttricks = {"gr_correcttricks", "Off", 0, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_grsolvetjoin = {"gr_solvetjoin", "On", 0, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL};

consvar_t cv_grbatching = {"gr_batching", "On", 0, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL};

static void CV_filtermode_ONChange(void)
{
	HWD.pfnSetSpecialState(HWD_SET_TEXTUREFILTERMODE, cv_grfiltermode.value);
}

static void CV_anisotropic_ONChange(void)
{
	HWD.pfnSetSpecialState(HWD_SET_TEXTUREANISOTROPICMODE, cv_granisotropicmode.value);
}

// ==========================================================================
// Globals
// ==========================================================================

// base values set at SetViewSize
static float gr_basecentery;
static float gr_basecenterx;

float gr_baseviewwindowy, gr_basewindowcentery;
float gr_baseviewwindowx, gr_basewindowcenterx;
float gr_viewwidth, gr_viewheight; // viewport clipping boundaries (screen coords)

static float gr_centerx;
static float gr_viewwindowx;
static float gr_windowcenterx; // center of view window, for projection

static float gr_centery;
static float gr_viewwindowy; // top left corner of view window
static float gr_windowcentery;

static float gr_pspritexscale, gr_pspriteyscale;


static seg_t *gr_curline;
static side_t *gr_sidedef;
static line_t *gr_linedef;
static sector_t *gr_frontsector;
static sector_t *gr_backsector;

boolean gr_shadersavailable = true;

// ==========================================================================
// View position
// ==========================================================================

FTransform atransform;

// Float variants of viewx, viewy, viewz, etc.
static float gr_viewx, gr_viewy, gr_viewz;
static float gr_viewsin, gr_viewcos;

static angle_t gr_aimingangle;
static float gr_viewludsin, gr_viewludcos;

static INT32 drawcount = 0;

// ==========================================================================
// Lighting
// ==========================================================================

void HWR_Lighting(FSurfaceInfo *Surface, INT32 light_level, extracolormap_t *colormap)
{
	RGBA_t poly_color, tint_color, fade_color;

	poly_color.rgba = 0xFFFFFFFF;
	tint_color.rgba = (colormap != NULL) ? (UINT32)colormap->rgba : GL_DEFAULTMIX;
	fade_color.rgba = (colormap != NULL) ? (UINT32)colormap->fadergba : GL_DEFAULTFOG;

	// Crappy backup coloring if you can't do shaders
	if (!(cv_grshaders.value && gr_shadersavailable))
	{
		// be careful, this may get negative for high lightlevel values.
		float tint_alpha, fade_alpha;
		float red, green, blue;

		red = (float)poly_color.s.red;
		green = (float)poly_color.s.green;
		blue = (float)poly_color.s.blue;

		// 48 is just an arbritrary value that looked relatively okay.
		tint_alpha = (float)(sqrt(tint_color.s.alpha) * 48) / 255.0f;

		// 8 is roughly the brightness of the "close" color in Software, and 16 the brightness of the "far" color.
		// 8 is too bright for dark levels, and 16 is too dark for bright levels.
		// 12 is the compromise value. It doesn't look especially good anywhere, but it's the most balanced.
		// (Also, as far as I can tell, fade_color's alpha is actually not used in Software, so we only use light level.)
		fade_alpha = (float)(sqrt(255-light_level) * 12) / 255.0f;

		// Clamp the alpha values
		tint_alpha = min(max(tint_alpha, 0.0f), 1.0f);
		fade_alpha = min(max(fade_alpha, 0.0f), 1.0f);

		red = (tint_color.s.red * tint_alpha) + (red * (1.0f - tint_alpha));
		green = (tint_color.s.green * tint_alpha) + (green * (1.0f - tint_alpha));
		blue = (tint_color.s.blue * tint_alpha) + (blue * (1.0f - tint_alpha));

		red = (fade_color.s.red * fade_alpha) + (red * (1.0f - fade_alpha));
		green = (fade_color.s.green * fade_alpha) + (green * (1.0f - fade_alpha));
		blue = (fade_color.s.blue * fade_alpha) + (blue * (1.0f - fade_alpha));

		poly_color.s.red = (UINT8)red;
		poly_color.s.green = (UINT8)green;
		poly_color.s.blue = (UINT8)blue;
	}

	Surface->PolyColor.rgba = poly_color.rgba;
	Surface->TintColor.rgba = tint_color.rgba;
	Surface->FadeColor.rgba = fade_color.rgba;
	Surface->LightInfo.light_level = light_level;
	Surface->LightInfo.fade_start = (colormap != NULL) ? colormap->fadestart : 0;
	Surface->LightInfo.fade_end = (colormap != NULL) ? colormap->fadeend : 31;
}

UINT8 HWR_FogBlockAlpha(INT32 light, extracolormap_t *colormap) // Let's see if this can work
{
	RGBA_t realcolor, surfcolor;
	INT32 alpha;

	realcolor.rgba = (colormap != NULL) ? colormap->rgba : GL_DEFAULTMIX;

	if (!(cv_grshaders.value && gr_shadersavailable))
	{
		light = light - (255 - light);

		// Don't go out of bounds
		if (light < 0)
			light = 0;
		else if (light > 255)
			light = 255;

		alpha = (realcolor.s.alpha*255)/25;

		// at 255 brightness, alpha is between 0 and 127, at 0 brightness alpha will always be 255
		surfcolor.s.alpha = (alpha*light) / (2*256) + 255-light;
	}
	else
	{
		surfcolor.s.alpha = (255 - light);
	}

	return surfcolor.s.alpha;
}

static FUINT HWR_CalcWallLight(FUINT lightnum, fixed_t v1x, fixed_t v1y, fixed_t v2x, fixed_t v2y)
{
	INT16 finallight = lightnum;

	if (cv_grfakecontrast.value != 0)
	{
		const UINT8 contrast = 8;
		fixed_t extralight = 0;

		if (cv_grfakecontrast.value == 2) // Smooth setting
		{
			extralight = (-(contrast<<FRACBITS) +
			FixedDiv(AngleFixed(R_PointToAngle2(0, 0,
				abs(v1x - v2x),
				abs(v1y - v2y))), 90<<FRACBITS)
			* (contrast * 2)) >> FRACBITS;
		}
		else
		{
			if (v1y == v2y)
				extralight = -contrast;
			else if (v1x == v2x)
				extralight = contrast;
		}

		if (extralight != 0)
		{
			finallight += extralight;

			if (finallight < 0)
				finallight = 0;
			if (finallight > 255)
				finallight = 255;
		}
	}

	return (FUINT)finallight;
}

// ==========================================================================
// Floor and ceiling generation from subsectors
// ==========================================================================

// HWR_RenderPlane
// Render a floor or ceiling convex polygon
void HWR_RenderPlane(extrasubsector_t *xsub, boolean isceiling, fixed_t fixedheight, FBITFIELD PolyFlags, INT32 lightlevel, lumpnum_t lumpnum, sector_t *FOFsector, UINT8 alpha, extracolormap_t *planecolormap)
{
	polyvertex_t *  pv;
	float           height; //constant y for all points on the convex flat polygon
	FOutVector      *v3d;
	INT32             nrPlaneVerts;   //verts original define of convex flat polygon
	INT32             i;
	float           flatxref,flatyref;
	float fflatsize;
	INT32 flatflag;
	size_t len;
	float scrollx = 0.0f, scrolly = 0.0f;
	angle_t angle = 0;
	FSurfaceInfo    Surf;
	fixed_t tempxsow, tempytow;
#ifdef ESLOPE
	pslope_t *slope = NULL;
#endif

	static FOutVector *planeVerts = NULL;
	static UINT16 numAllocedPlaneVerts = 0;

	// no convex poly were generated for this subsector
	if (!xsub->planepoly)
		return;

#ifdef ESLOPE
	// Get the slope pointer to simplify future code
	if (FOFsector)
	{
		if (FOFsector->f_slope && !isceiling)
			slope = FOFsector->f_slope;
		else if (FOFsector->c_slope && isceiling)
			slope = FOFsector->c_slope;
	}
	else
	{
		if (gr_frontsector->f_slope && !isceiling)
			slope = gr_frontsector->f_slope;
		else if (gr_frontsector->c_slope && isceiling)
			slope = gr_frontsector->c_slope;
	}

	// Set fixedheight to the slope's height from our viewpoint, if we have a slope
	if (slope)
		fixedheight = P_GetZAt(slope, viewx, viewy);
#endif

	height = FIXED_TO_FLOAT(fixedheight);

	pv  = xsub->planepoly->pts;
	nrPlaneVerts = xsub->planepoly->numpts;

	if (nrPlaneVerts < 3)   //not even a triangle ?
		return;

	// Allocate plane-vertex buffer if we need to
	if (!planeVerts || nrPlaneVerts > numAllocedPlaneVerts)
	{
		numAllocedPlaneVerts = (UINT16)nrPlaneVerts;
		Z_Free(planeVerts);
		Z_Malloc(numAllocedPlaneVerts * sizeof (FOutVector), PU_LEVEL, &planeVerts);
	}

	len = W_LumpLength(lumpnum);

	switch (len)
	{
		case 4194304: // 2048x2048 lump
			fflatsize = 2048.0f;
			flatflag = 2047;
			break;
		case 1048576: // 1024x1024 lump
			fflatsize = 1024.0f;
			flatflag = 1023;
			break;
		case 262144:// 512x512 lump
			fflatsize = 512.0f;
			flatflag = 511;
			break;
		case 65536: // 256x256 lump
			fflatsize = 256.0f;
			flatflag = 255;
			break;
		case 16384: // 128x128 lump
			fflatsize = 128.0f;
			flatflag = 127;
			break;
		case 1024: // 32x32 lump
			fflatsize = 32.0f;
			flatflag = 31;
			break;
		default: // 64x64 lump
			fflatsize = 64.0f;
			flatflag = 63;
			break;
	}

	// reference point for flat texture coord for each vertex around the polygon
	flatxref = (float)(((fixed_t)pv->x & (~flatflag)) / fflatsize);
	flatyref = (float)(((fixed_t)pv->y & (~flatflag)) / fflatsize);

	// transform
	v3d = planeVerts;

	if (FOFsector != NULL)
	{
		if (!isceiling) // it's a floor
		{
			scrollx = FIXED_TO_FLOAT(FOFsector->floor_xoffs)/fflatsize;
			scrolly = FIXED_TO_FLOAT(FOFsector->floor_yoffs)/fflatsize;
			angle = FOFsector->floorpic_angle;
		}
		else // it's a ceiling
		{
			scrollx = FIXED_TO_FLOAT(FOFsector->ceiling_xoffs)/fflatsize;
			scrolly = FIXED_TO_FLOAT(FOFsector->ceiling_yoffs)/fflatsize;
			angle = FOFsector->ceilingpic_angle;
		}
	}
	else if (gr_frontsector)
	{
		if (!isceiling) // it's a floor
		{
			scrollx = FIXED_TO_FLOAT(gr_frontsector->floor_xoffs)/fflatsize;
			scrolly = FIXED_TO_FLOAT(gr_frontsector->floor_yoffs)/fflatsize;
			angle = gr_frontsector->floorpic_angle;
		}
		else // it's a ceiling
		{
			scrollx = FIXED_TO_FLOAT(gr_frontsector->ceiling_xoffs)/fflatsize;
			scrolly = FIXED_TO_FLOAT(gr_frontsector->ceiling_yoffs)/fflatsize;
			angle = gr_frontsector->ceilingpic_angle;
		}
	}


	if (angle) // Only needs to be done if there's an altered angle
	{

		angle = InvAngle(angle)>>ANGLETOFINESHIFT;

		// This needs to be done so that it scrolls in a different direction after rotation like software
		/*tempxsow = FLOAT_TO_FIXED(scrollx);
		tempytow = FLOAT_TO_FIXED(scrolly);
		scrollx = (FIXED_TO_FLOAT(FixedMul(tempxsow, FINECOSINE(angle)) - FixedMul(tempytow, FINESINE(angle))));
		scrolly = (FIXED_TO_FLOAT(FixedMul(tempxsow, FINESINE(angle)) + FixedMul(tempytow, FINECOSINE(angle))));*/

		// This needs to be done so everything aligns after rotation
		// It would be done so that rotation is done, THEN the translation, but I couldn't get it to rotate AND scroll like software does
		tempxsow = FLOAT_TO_FIXED(flatxref);
		tempytow = FLOAT_TO_FIXED(flatyref);
		flatxref = (FIXED_TO_FLOAT(FixedMul(tempxsow, FINECOSINE(angle)) - FixedMul(tempytow, FINESINE(angle))));
		flatyref = (FIXED_TO_FLOAT(FixedMul(tempxsow, FINESINE(angle)) + FixedMul(tempytow, FINECOSINE(angle))));
	}


	for (i = 0; i < nrPlaneVerts; i++,v3d++,pv++)
	{
		// Hurdler: add scrolling texture on floor/ceiling
		v3d->s = (float)((pv->x / fflatsize) - flatxref + scrollx);
		v3d->t = (float)(flatyref - (pv->y / fflatsize) + scrolly);

		//v3d->s = (float)(pv->x / fflatsize);
		//v3d->t = (float)(pv->y / fflatsize);

		// Need to rotate before translate
		if (angle) // Only needs to be done if there's an altered angle
		{
			tempxsow = FLOAT_TO_FIXED(v3d->s);
			tempytow = FLOAT_TO_FIXED(v3d->t);
			v3d->s = (FIXED_TO_FLOAT(FixedMul(tempxsow, FINECOSINE(angle)) - FixedMul(tempytow, FINESINE(angle))));
			v3d->t = (FIXED_TO_FLOAT(FixedMul(tempxsow, FINESINE(angle)) + FixedMul(tempytow, FINECOSINE(angle))));
		}

		//v3d->s = (float)(v3d->s - flatxref + scrollx);
		//v3d->t = (float)(flatyref - v3d->t + scrolly);

		v3d->x = pv->x;
		v3d->y = height;
		v3d->z = pv->y;

#ifdef ESLOPE
		if (slope)
		{
			fixedheight = P_GetZAt(slope, FLOAT_TO_FIXED(pv->x), FLOAT_TO_FIXED(pv->y));
			v3d->y = FIXED_TO_FLOAT(fixedheight);
		}
#endif
	}

	HWR_Lighting(&Surf, lightlevel, planecolormap);

	if (PolyFlags & (PF_Translucent|PF_Fog))
	{
		Surf.PolyColor.s.alpha = (UINT8)alpha;
		PolyFlags |= PF_Modulated;
	}
	else
		PolyFlags |= PF_Masked|PF_Modulated;

	if (PolyFlags & PF_Fog)
		HWD.pfnSetShader(6);	// fog shader
	else if (PolyFlags & PF_Ripple)
		HWD.pfnSetShader(5);	// water shader
	else
		HWD.pfnSetShader(1);	// floor shader

	HWD.pfnDrawPolygon(&Surf, planeVerts, nrPlaneVerts, PolyFlags);
}

#ifdef WALLSPLATS
static void HWR_DrawSegsSplats(FSurfaceInfo * pSurf)
{
	FOutVector wallVerts[4];
	wallsplat_t *splat;
	GLPatch_t *gpatch;
	fixed_t i;
	// seg bbox
	fixed_t segbbox[4];

	M_ClearBox(segbbox);
	M_AddToBox(segbbox,
		FLOAT_TO_FIXED(((polyvertex_t *)gr_curline->pv1)->x),
		FLOAT_TO_FIXED(((polyvertex_t *)gr_curline->pv1)->y));
	M_AddToBox(segbbox,
		FLOAT_TO_FIXED(((polyvertex_t *)gr_curline->pv2)->x),
		FLOAT_TO_FIXED(((polyvertex_t *)gr_curline->pv2)->y));

	splat = (wallsplat_t *)gr_curline->linedef->splats;
	for (; splat; splat = splat->next)
	{
		//BP: don't draw splat extern to this seg
		//    this is quick fix best is explain in logboris.txt at 12-4-2000
		if (!M_PointInBox(segbbox,splat->v1.x,splat->v1.y) && !M_PointInBox(segbbox,splat->v2.x,splat->v2.y))
			continue;

		gpatch = W_CachePatchNum(splat->patch, PU_CACHE);
		HWR_GetPatch(gpatch);

		wallVerts[0].x = wallVerts[3].x = FIXED_TO_FLOAT(splat->v1.x);
		wallVerts[0].z = wallVerts[3].z = FIXED_TO_FLOAT(splat->v1.y);
		wallVerts[2].x = wallVerts[1].x = FIXED_TO_FLOAT(splat->v2.x);
		wallVerts[2].z = wallVerts[1].z = FIXED_TO_FLOAT(splat->v2.y);

		i = splat->top;
		if (splat->yoffset)
			i += *splat->yoffset;

		wallVerts[2].y = wallVerts[3].y = FIXED_TO_FLOAT(i)+(gpatch->height>>1);
		wallVerts[0].y = wallVerts[1].y = FIXED_TO_FLOAT(i)-(gpatch->height>>1);

		wallVerts[3].s = wallVerts[3].t = wallVerts[2].s = wallVerts[0].t = 0.0f;
		wallVerts[1].s = wallVerts[1].t = wallVerts[2].t = wallVerts[0].s = 1.0f;

		switch (splat->flags & SPLATDRAWMODE_MASK)
		{
			case SPLATDRAWMODE_OPAQUE :
				pSurf.PolyColor.s.alpha = 0xff;
				i = PF_Translucent;
				break;
			case SPLATDRAWMODE_TRANS :
				pSurf.PolyColor.s.alpha = 128;
				i = PF_Translucent;
				break;
			case SPLATDRAWMODE_SHADE :
				pSurf.PolyColor.s.alpha = 0xff;
				i = PF_Substractive;
				break;
		}

		HWD.pfnSetShader(2);	// wall shader
		HWD.pfnDrawPolygon(&pSurf, wallVerts, 4, i|PF_Modulated|PF_Decal);
	}
}
#endif

FBITFIELD HWR_TranstableToAlpha(INT32 transtablenum, FSurfaceInfo *pSurf)
{
	switch (transtablenum)
	{
		case tr_trans10 : pSurf->PolyColor.s.alpha = 0xe6;return  PF_Translucent;
		case tr_trans20 : pSurf->PolyColor.s.alpha = 0xcc;return  PF_Translucent;
		case tr_trans30 : pSurf->PolyColor.s.alpha = 0xb3;return  PF_Translucent;
		case tr_trans40 : pSurf->PolyColor.s.alpha = 0x99;return  PF_Translucent;
		case tr_trans50 : pSurf->PolyColor.s.alpha = 0x80;return  PF_Translucent;
		case tr_trans60 : pSurf->PolyColor.s.alpha = 0x66;return  PF_Translucent;
		case tr_trans70 : pSurf->PolyColor.s.alpha = 0x4c;return  PF_Translucent;
		case tr_trans80 : pSurf->PolyColor.s.alpha = 0x33;return  PF_Translucent;
		case tr_trans90 : pSurf->PolyColor.s.alpha = 0x19;return  PF_Translucent;
	}
	return PF_Translucent;
}

// ==========================================================================
// Wall generation from subsector segs
// ==========================================================================

//
// HWR_ProjectWall
//
void HWR_ProjectWall(FOutVector *wallVerts, FSurfaceInfo *pSurf, FBITFIELD blendmode, INT32 lightlevel, extracolormap_t *wallcolormap)
{
	HWR_Lighting(pSurf, lightlevel, wallcolormap);

	HWD.pfnSetShader(2);	// wall shader
	HWD.pfnDrawPolygon(pSurf, wallVerts, 4, blendmode|PF_Modulated|PF_Occlude);

#ifdef WALLSPLATS
	if (gr_curline->linedef->splats && cv_splats.value)
		HWR_DrawSegsSplats(pSurf);
#endif
}

//
// HWR_SplitWall
//
void HWR_SplitWall(sector_t *sector, FOutVector *wallVerts, INT32 texnum, FSurfaceInfo* Surf, INT32 cutflag, ffloor_t *pfloor)
{
	/* SoM: split up and light walls according to the
	 lightlist. This may also include leaving out parts
	 of the wall that can't be seen */

	float realtop, realbot, top, bot;
	float pegt, pegb, pegmul;
	float height = 0.0f, bheight = 0.0f;

#ifdef ESLOPE
	float endrealtop, endrealbot, endtop, endbot;
	float endpegt, endpegb, endpegmul;
	float endheight = 0.0f, endbheight = 0.0f;

	fixed_t v1x = FLOAT_TO_FIXED(wallVerts[0].x);
	fixed_t v1y = FLOAT_TO_FIXED(wallVerts[0].z); // not a typo
	fixed_t v2x = FLOAT_TO_FIXED(wallVerts[1].x);
	fixed_t v2y = FLOAT_TO_FIXED(wallVerts[1].z); // not a typo
	// compiler complains when P_GetZAt is used in FLOAT_TO_FIXED directly
	// use this as a temp var to store P_GetZAt's return value each time
	fixed_t temp;
#endif

	INT32   solid, i;
	lightlist_t *  list = sector->lightlist;
	const UINT8 alpha = Surf->PolyColor.s.alpha;
	FUINT lightnum = HWR_CalcWallLight(sector->lightlevel, v1x, v1y, v2x, v2y);
	extracolormap_t *colormap = NULL;

	realtop = top = wallVerts[3].y;
	realbot = bot = wallVerts[0].y;
	pegt = wallVerts[3].t;
	pegb = wallVerts[0].t;
	pegmul = (pegb - pegt) / (top - bot);

#ifdef ESLOPE
	endrealtop = endtop = wallVerts[2].y;
	endrealbot = endbot = wallVerts[1].y;
	endpegt = wallVerts[2].t;
	endpegb = wallVerts[1].t;
	endpegmul = (endpegb - endpegt) / (endtop - endbot);
#endif

	for (i = 0; i < sector->numlights; i++)
	{
#ifdef ESLOPE
		if (endtop < endrealbot)
#endif
		if (top < realbot)
			return;

		if (!(list[i].flags & FF_NOSHADE))
		{
			if (pfloor && (pfloor->flags & FF_FOG))
			{
				lightnum = HWR_CalcWallLight(pfloor->master->frontsector->lightlevel, v1x, v1y, v2x, v2y);
				colormap = pfloor->master->frontsector->extra_colormap;
			}
			else
			{
				lightnum = HWR_CalcWallLight(*list[i].lightlevel, v1x, v1y, v2x, v2y);
				colormap = list[i].extra_colormap;
			}
		}

		solid = false;

		if ((sector->lightlist[i].flags & FF_CUTSOLIDS) && !(cutflag & FF_EXTRA))
			solid = true;
		else if ((sector->lightlist[i].flags & FF_CUTEXTRA) && (cutflag & FF_EXTRA))
		{
			if (sector->lightlist[i].flags & FF_EXTRA)
			{
				if ((sector->lightlist[i].flags & (FF_FOG|FF_SWIMMABLE)) == (cutflag & (FF_FOG|FF_SWIMMABLE))) // Only merge with your own types
					solid = true;
			}
			else
				solid = true;
		}
		else
			solid = false;

#ifdef ESLOPE
		if (list[i].slope)
		{
			temp = P_GetZAt(list[i].slope, v1x, v1y);
			height = FIXED_TO_FLOAT(temp);
			temp = P_GetZAt(list[i].slope, v2x, v2y);
			endheight = FIXED_TO_FLOAT(temp);
		}
		else
			height = endheight = FIXED_TO_FLOAT(list[i].height);
		if (solid)
		{
			if (*list[i].caster->b_slope)
			{
				temp = P_GetZAt(*list[i].caster->b_slope, v1x, v1y);
				bheight = FIXED_TO_FLOAT(temp);
				temp = P_GetZAt(*list[i].caster->b_slope, v2x, v2y);
				endbheight = FIXED_TO_FLOAT(temp);
			}
			else
				bheight = endbheight = FIXED_TO_FLOAT(*list[i].caster->bottomheight);
		}
#else
		height = FIXED_TO_FLOAT(list[i].height);
		if (solid)
			bheight = FIXED_TO_FLOAT(*list[i].caster->bottomheight);
#endif

#ifdef ESLOPE
		if (endheight >= endtop)
#endif
		if (height >= top)
		{
			if (solid && top > bheight)
				top = bheight;
#ifdef ESLOPE
			if (solid && endtop > endbheight)
				endtop = endbheight;
#endif
		}

#ifdef ESLOPE
		if (i + 1 < sector->numlights)
		{
			if (list[i+1].slope)
			{
				temp = P_GetZAt(list[i+1].slope, v1x, v1y);
				bheight = FIXED_TO_FLOAT(temp);
				temp = P_GetZAt(list[i+1].slope, v2x, v2y);
				endbheight = FIXED_TO_FLOAT(temp);
			}
			else
				bheight = endbheight = FIXED_TO_FLOAT(list[i+1].height);
		}
		else
		{
			bheight = realbot;
			endbheight = endrealbot;
		}
#else
		if (i + 1 < sector->numlights)
		{
			bheight = FIXED_TO_FLOAT(list[i+1].height);
		}
		else
		{
			bheight = realbot;
		}
#endif

#ifdef ESLOPE
		if (endbheight >= endtop)
#endif
		if (bheight >= top)
			continue;

		//Found a break;
		bot = bheight;

		if (bot < realbot)
			bot = realbot;

#ifdef ESLOPE
		endbot = endbheight;

		if (endbot < endrealbot)
			endbot = endrealbot;
#endif
		Surf->PolyColor.s.alpha = alpha;

#ifdef ESLOPE
		wallVerts[3].t = pegt + ((realtop - top) * pegmul);
		wallVerts[2].t = endpegt + ((endrealtop - endtop) * endpegmul);
		wallVerts[0].t = pegt + ((realtop - bot) * pegmul);
		wallVerts[1].t = endpegt + ((endrealtop - endbot) * endpegmul);

		// set top/bottom coords
		wallVerts[3].y = top;
		wallVerts[2].y = endtop;
		wallVerts[0].y = bot;
		wallVerts[1].y = endbot;
#else
		wallVerts[3].t = wallVerts[2].t = pegt + ((realtop - top) * pegmul);
		wallVerts[0].t = wallVerts[1].t = pegt + ((realtop - bot) * pegmul);

		// set top/bottom coords
		wallVerts[2].y = wallVerts[3].y = top;
		wallVerts[0].y = wallVerts[1].y = bot;
#endif

		if (cutflag & FF_FOG)
			HWR_AddTransparentWall(wallVerts, Surf, texnum, PF_Fog|PF_NoTexture, true, lightnum, colormap);
		else if (cutflag & FF_TRANSLUCENT)
			HWR_AddTransparentWall(wallVerts, Surf, texnum, PF_Translucent, false, lightnum, colormap);
		else
			HWR_ProjectWall(wallVerts, Surf, PF_Masked, lightnum, colormap);

		top = bot;
#ifdef ESLOPE
		endtop = endbot;
#endif
	}

	bot = realbot;
#ifdef ESLOPE
	endbot = endrealbot;
	if (endtop <= endrealbot)
#endif
	if (top <= realbot)
		return;

	Surf->PolyColor.s.alpha = alpha;

#ifdef ESLOPE
	wallVerts[3].t = pegt + ((realtop - top) * pegmul);
	wallVerts[2].t = endpegt + ((endrealtop - endtop) * endpegmul);
	wallVerts[0].t = pegt + ((realtop - bot) * pegmul);
	wallVerts[1].t = endpegt + ((endrealtop - endbot) * endpegmul);

	// set top/bottom coords
	wallVerts[3].y = top;
	wallVerts[2].y = endtop;
	wallVerts[0].y = bot;
	wallVerts[1].y = endbot;
#else
    wallVerts[3].t = wallVerts[2].t = pegt + ((realtop - top) * pegmul);
    wallVerts[0].t = wallVerts[1].t = pegt + ((realtop - bot) * pegmul);

    // set top/bottom coords
    wallVerts[2].y = wallVerts[3].y = top;
    wallVerts[0].y = wallVerts[1].y = bot;
#endif

	if (cutflag & FF_FOG)
		HWR_AddTransparentWall(wallVerts, Surf, texnum, PF_Fog|PF_NoTexture, true, lightnum, colormap);
	else if (cutflag & FF_TRANSLUCENT)
		HWR_AddTransparentWall(wallVerts, Surf, texnum, PF_Translucent, false, lightnum, colormap);
	else
		HWR_ProjectWall(wallVerts, Surf, PF_Masked, lightnum, colormap);
}

// HWR_DrawSkyWalls
// Draw walls into the depth buffer so that anything behind is culled properly
void HWR_DrawSkyWall(FOutVector *wallVerts, FSurfaceInfo *Surf)
{
	HWD.pfnSetTexture(NULL);
	// no texture
	wallVerts[3].t = wallVerts[2].t = 0;
	wallVerts[0].t = wallVerts[1].t = 0;
	wallVerts[0].s = wallVerts[3].s = 0;
	wallVerts[2].s = wallVerts[1].s = 0;

	HWR_ProjectWall(wallVerts, Surf, PF_Invisible|PF_NoTexture, 255, NULL);
	// PF_Invisible so it's not drawn into the colour buffer
	// PF_NoTexture for no texture
	// PF_Occlude is set in HWR_ProjectWall to draw into the depth buffer
}

//
// HWR_ProcessSeg
// A portion or all of a wall segment will be drawn, from startfrac to endfrac,
//  where 0 is the start of the segment, 1 the end of the segment
// Anything between means the wall segment has been clipped with solidsegs,
//  reducing wall overdraw to a minimum
//
void HWR_ProcessSeg(void) // Sort of like GLWall::Process in GZDoom
{
	FOutVector wallVerts[4];
	v2d_t vs, ve; // start, end vertices of 2d line (view from above)

	fixed_t worldtop, worldbottom;
	fixed_t worldhigh = 0, worldlow = 0;
#ifdef ESLOPE
	fixed_t worldtopslope, worldbottomslope;
	fixed_t worldhighslope = 0, worldlowslope = 0;
	fixed_t v1x, v1y, v2x, v2y;
#endif

	GLTexture_t *grTex = NULL;
	float cliplow = 0.0f, cliphigh = 0.0f;
	INT32 gr_midtexture;
	fixed_t h, l; // 3D sides and 2s middle textures
#ifdef ESLOPE
	fixed_t hS, lS;
#endif

	FUINT lightnum = 0; // shut up compiler
	extracolormap_t *colormap;
	FSurfaceInfo Surf;

	gr_sidedef = gr_curline->sidedef;
	gr_linedef = gr_curline->linedef;

	vs.x = ((polyvertex_t *)gr_curline->pv1)->x;
	vs.y = ((polyvertex_t *)gr_curline->pv1)->y;
	ve.x = ((polyvertex_t *)gr_curline->pv2)->x;
	ve.y = ((polyvertex_t *)gr_curline->pv2)->y;

#ifdef ESLOPE
	v1x = FLOAT_TO_FIXED(vs.x);
	v1y = FLOAT_TO_FIXED(vs.y);
	v2x = FLOAT_TO_FIXED(ve.x);
	v2y = FLOAT_TO_FIXED(ve.y);
#endif
#ifdef ESLOPE

#define SLOPEPARAMS(slope, end1, end2, normalheight) \
	if (slope) { \
		end1 = P_GetZAt(slope, v1x, v1y); \
		end2 = P_GetZAt(slope, v2x, v2y); \
	} else \
		end1 = end2 = normalheight;

	SLOPEPARAMS(gr_frontsector->c_slope, worldtop,    worldtopslope,    gr_frontsector->ceilingheight)
	SLOPEPARAMS(gr_frontsector->f_slope, worldbottom, worldbottomslope, gr_frontsector->floorheight)
#else
	worldtop    = gr_frontsector->ceilingheight;
	worldbottom = gr_frontsector->floorheight;
#endif

	// remember vertices ordering
	//  3--2
	//  | /|
	//  |/ |
	//  0--1
	// make a wall polygon (with 2 triangles), using the floor/ceiling heights,
	// and the 2d map coords of start/end vertices
	wallVerts[0].x = wallVerts[3].x = vs.x;
	wallVerts[0].z = wallVerts[3].z = vs.y;
	wallVerts[2].x = wallVerts[1].x = ve.x;
	wallVerts[2].z = wallVerts[1].z = ve.y;

	// x offset the texture
	{
		fixed_t texturehpeg = gr_sidedef->textureoffset + gr_curline->offset;
		cliplow = (float)texturehpeg;
		cliphigh = (float)(texturehpeg + (gr_curline->flength*FRACUNIT));
	}

	lightnum = HWR_CalcWallLight(gr_frontsector->lightlevel, vs.x, vs.y, ve.x, ve.y);
	colormap = gr_frontsector->extra_colormap;

	if (gr_frontsector)
		Surf.PolyColor.s.alpha = 255;

	if (gr_backsector)
	{
		INT32 gr_toptexture, gr_bottomtexture;
		// two sided line

#ifdef ESLOPE
		SLOPEPARAMS(gr_backsector->c_slope, worldhigh, worldhighslope, gr_backsector->ceilingheight)
		SLOPEPARAMS(gr_backsector->f_slope, worldlow,  worldlowslope,  gr_backsector->floorheight)
#undef SLOPEPARAMS
#else
		worldhigh = gr_backsector->ceilingheight;
		worldlow  = gr_backsector->floorheight;
#endif

		// Sky culling
		if (!gr_curline->polyseg) // Don't do it for polyobjects
		{
			// Sky Ceilings
			wallVerts[3].y = wallVerts[2].y = FIXED_TO_FLOAT(INT32_MAX);

			if (gr_frontsector->ceilingpic == skyflatnum)
			{
				if (gr_backsector->ceilingpic == skyflatnum)
				{
					// Both front and back sectors are sky, needs skywall from the frontsector's ceiling, but only if the
					// backsector is lower
					if ((worldhigh <= worldtop && worldhighslope <= worldtopslope)// Assuming ESLOPE is always on with my changes
					&& (worldhigh != worldtop || worldhighslope != worldtopslope))
					// Removing the second line above will render more rarely visible skywalls. Example: Cave garden ceiling in Dark race
					{
#ifdef ESLOPE
						wallVerts[0].y = FIXED_TO_FLOAT(worldhigh);
						wallVerts[1].y = FIXED_TO_FLOAT(worldhighslope);
#else
						wallVerts[0].y = wallVerts[1].y = FIXED_TO_FLOAT(worldhigh);
#endif
						HWR_DrawSkyWall(wallVerts, &Surf);
					}
				}
				else
				{
					// Only the frontsector is sky, just draw a skywall from the front ceiling
#ifdef ESLOPE
					wallVerts[0].y = FIXED_TO_FLOAT(worldtop);
					wallVerts[1].y = FIXED_TO_FLOAT(worldtopslope);
#else
					wallVerts[0].y = wallVerts[1].y = FIXED_TO_FLOAT(worldtop);
#endif
					HWR_DrawSkyWall(wallVerts, &Surf);
				}
			}
			else if (gr_backsector->ceilingpic == skyflatnum)
			{
				// Only the backsector is sky, just draw a skywall from the front ceiling
#ifdef ESLOPE
				wallVerts[0].y = FIXED_TO_FLOAT(worldtop);
				wallVerts[1].y = FIXED_TO_FLOAT(worldtopslope);
#else
				wallVerts[0].y = wallVerts[1].y = FIXED_TO_FLOAT(worldtop);
#endif
				HWR_DrawSkyWall(wallVerts, &Surf);
			}


			// Sky Floors
			wallVerts[0].y = wallVerts[1].y = FIXED_TO_FLOAT(INT32_MIN);

			if (gr_frontsector->floorpic == skyflatnum)
			{
				if (gr_backsector->floorpic == skyflatnum)
				{
					// Both front and back sectors are sky, needs skywall from the backsector's floor, but only if the
					// it's higher, also needs to check for bottomtexture as the floors don't usually move down
					// when both sides are sky floors
					if ((worldlow >= worldbottom && worldlowslope >= worldbottomslope)
					&& (worldlow != worldbottom || worldlowslope != worldbottomslope)
					// Removing the second line above will render more rarely visible skywalls. Example: Cave garden ceiling in Dark race
					&& !(gr_sidedef->bottomtexture))
					{
#ifdef ESLOPE
						wallVerts[3].y = FIXED_TO_FLOAT(worldlow);
						wallVerts[2].y = FIXED_TO_FLOAT(worldlowslope);
#else
						wallVerts[3].y = wallVerts[2].y = FIXED_TO_FLOAT(worldlow);
#endif

						HWR_DrawSkyWall(wallVerts, &Surf);
					}
				}
				else
				{
					// Only the backsector has sky, just draw a skywall from the back floor
#ifdef ESLOPE
					wallVerts[3].y = FIXED_TO_FLOAT(worldbottom);
					wallVerts[2].y = FIXED_TO_FLOAT(worldbottomslope);
#else
					wallVerts[3].y = wallVerts[2].y = FIXED_TO_FLOAT(worldbottom);
#endif

					HWR_DrawSkyWall(wallVerts, &Surf);
				}
			}
			else if ((gr_backsector->floorpic == skyflatnum) && !(gr_sidedef->bottomtexture))
			{
				// Only the backsector has sky, just draw a skywall from the back floor if there's no bottomtexture
#ifdef ESLOPE
				wallVerts[3].y = FIXED_TO_FLOAT(worldlow);
				wallVerts[2].y = FIXED_TO_FLOAT(worldlowslope);
#else
				wallVerts[3].y = wallVerts[2].y = FIXED_TO_FLOAT(worldlow);
#endif

				HWR_DrawSkyWall(wallVerts, &Surf);
			}

		}

		// hack to allow height changes in outdoor areas
		// This is what gets rid of the upper textures if there should be sky
		if (gr_frontsector->ceilingpic == skyflatnum &&
			gr_backsector->ceilingpic  == skyflatnum)
		{
			worldtop = worldhigh;
#ifdef ESLOPE
			worldtopslope = worldhighslope;
#endif
		}

		gr_toptexture = R_GetTextureNum(gr_sidedef->toptexture);
		gr_bottomtexture = R_GetTextureNum(gr_sidedef->bottomtexture);

		// check TOP TEXTURE
		if ((
#ifdef ESLOPE
			worldhighslope < worldtopslope ||
#endif
            worldhigh < worldtop
            ) && gr_toptexture)
		{
			{
				fixed_t texturevpegtop; // top

				grTex = HWR_GetTexture(gr_toptexture);

				// PEGGING
				if (gr_linedef->flags & ML_DONTPEGTOP)
					texturevpegtop = 0;
#ifdef ESLOPE
				else if (gr_linedef->flags & ML_EFFECT1)
					texturevpegtop = worldhigh + textureheight[gr_sidedef->toptexture] - worldtop;
				else
					texturevpegtop = gr_backsector->ceilingheight + textureheight[gr_sidedef->toptexture] - gr_frontsector->ceilingheight;
#else
                else
                    texturevpegtop = worldhigh + textureheight[gr_sidedef->toptexture] - worldtop;
#endif

				texturevpegtop += gr_sidedef->rowoffset;

				// This is so that it doesn't overflow and screw up the wall, it doesn't need to go higher than the texture's height anyway
				texturevpegtop %= SHORT(textures[gr_toptexture]->height)<<FRACBITS;

				wallVerts[3].t = wallVerts[2].t = texturevpegtop * grTex->scaleY;
				wallVerts[0].t = wallVerts[1].t = (texturevpegtop + gr_frontsector->ceilingheight - gr_backsector->ceilingheight) * grTex->scaleY;
				wallVerts[0].s = wallVerts[3].s = cliplow * grTex->scaleX;
				wallVerts[2].s = wallVerts[1].s = cliphigh * grTex->scaleX;

#ifdef ESLOPE
				// Adjust t value for sloped walls
				if (!(gr_linedef->flags & ML_EFFECT1))
				{
					// Unskewed
					wallVerts[3].t -= (worldtop - gr_frontsector->ceilingheight) * grTex->scaleY;
					wallVerts[2].t -= (worldtopslope - gr_frontsector->ceilingheight) * grTex->scaleY;
					wallVerts[0].t -= (worldhigh - gr_backsector->ceilingheight) * grTex->scaleY;
					wallVerts[1].t -= (worldhighslope - gr_backsector->ceilingheight) * grTex->scaleY;
				}
				else if (gr_linedef->flags & ML_DONTPEGTOP)
				{
					// Skewed by top
					wallVerts[0].t = (texturevpegtop + worldtop - worldhigh) * grTex->scaleY;
					wallVerts[1].t = (texturevpegtop + worldtopslope - worldhighslope) * grTex->scaleY;
				}
				else
				{
					// Skewed by bottom
					wallVerts[0].t = wallVerts[1].t = (texturevpegtop + worldtop - worldhigh) * grTex->scaleY;
					wallVerts[3].t = wallVerts[0].t - (worldtop - worldhigh) * grTex->scaleY;
					wallVerts[2].t = wallVerts[1].t - (worldtopslope - worldhighslope) * grTex->scaleY;
				}
#endif
			}

			// set top/bottom coords
#ifdef ESLOPE
			wallVerts[3].y = FIXED_TO_FLOAT(worldtop);
			wallVerts[0].y = FIXED_TO_FLOAT(worldhigh);
			wallVerts[2].y = FIXED_TO_FLOAT(worldtopslope);
			wallVerts[1].y = FIXED_TO_FLOAT(worldhighslope);
#else
			wallVerts[2].y = wallVerts[3].y = FIXED_TO_FLOAT(worldtop);
			wallVerts[0].y = wallVerts[1].y = FIXED_TO_FLOAT(worldhigh);
#endif

			if (gr_frontsector->numlights)
				HWR_SplitWall(gr_frontsector, wallVerts, gr_toptexture, &Surf, FF_CUTLEVEL, NULL);
			else if (grTex->mipmap.flags & TF_TRANSPARENT)
				HWR_AddTransparentWall(wallVerts, &Surf, gr_toptexture, PF_Environment, false, lightnum, colormap);
			else
				HWR_ProjectWall(wallVerts, &Surf, PF_Masked, lightnum, colormap);
		}

		// check BOTTOM TEXTURE
		if ((
#ifdef ESLOPE
			worldlowslope > worldbottomslope ||
#endif
            worldlow > worldbottom) && gr_bottomtexture) //only if VISIBLE!!!
		{
			{
				fixed_t texturevpegbottom = 0; // bottom

				grTex = HWR_GetTexture(gr_bottomtexture);

				// PEGGING
#ifdef ESLOPE
				if (!(gr_linedef->flags & ML_DONTPEGBOTTOM))
					texturevpegbottom = 0;
				else if (gr_linedef->flags & ML_EFFECT1)
					texturevpegbottom = worldbottom - worldlow;
				else
					texturevpegbottom = gr_frontsector->floorheight - gr_backsector->floorheight;
#else
				if (gr_linedef->flags & ML_DONTPEGBOTTOM)
					texturevpegbottom = worldbottom - worldlow;
                else
                    texturevpegbottom = 0;
#endif

				texturevpegbottom += gr_sidedef->rowoffset;

				// This is so that it doesn't overflow and screw up the wall, it doesn't need to go higher than the texture's height anyway
				texturevpegbottom %= SHORT(textures[gr_bottomtexture]->height)<<FRACBITS;

				wallVerts[3].t = wallVerts[2].t = texturevpegbottom * grTex->scaleY;
				wallVerts[0].t = wallVerts[1].t = (texturevpegbottom + gr_backsector->floorheight - gr_frontsector->floorheight) * grTex->scaleY;
				wallVerts[0].s = wallVerts[3].s = cliplow * grTex->scaleX;
				wallVerts[2].s = wallVerts[1].s = cliphigh * grTex->scaleX;

#ifdef ESLOPE
				// Adjust t value for sloped walls
				if (!(gr_linedef->flags & ML_EFFECT1))
				{
					// Unskewed
					wallVerts[0].t -= (worldbottom - gr_frontsector->floorheight) * grTex->scaleY;
					wallVerts[1].t -= (worldbottomslope - gr_frontsector->floorheight) * grTex->scaleY;
					wallVerts[3].t -= (worldlow - gr_backsector->floorheight) * grTex->scaleY;
					wallVerts[2].t -= (worldlowslope - gr_backsector->floorheight) * grTex->scaleY;
				}
				else if (gr_linedef->flags & ML_DONTPEGBOTTOM)
				{
					// Skewed by bottom
					wallVerts[0].t = wallVerts[1].t = (texturevpegbottom + worldlow - worldbottom) * grTex->scaleY;
					//wallVerts[3].t = wallVerts[0].t - (worldlow - worldbottom) * grTex->scaleY; // no need, [3] is already this
					wallVerts[2].t = wallVerts[1].t - (worldlowslope - worldbottomslope) * grTex->scaleY;
				}
				else
				{
					// Skewed by top
					wallVerts[0].t = (texturevpegbottom + worldlow - worldbottom) * grTex->scaleY;
					wallVerts[1].t = (texturevpegbottom + worldlowslope - worldbottomslope) * grTex->scaleY;
				}
#endif
			}

			// set top/bottom coords
#ifdef ESLOPE
			wallVerts[3].y = FIXED_TO_FLOAT(worldlow);
			wallVerts[0].y = FIXED_TO_FLOAT(worldbottom);
			wallVerts[2].y = FIXED_TO_FLOAT(worldlowslope);
			wallVerts[1].y = FIXED_TO_FLOAT(worldbottomslope);
#else
			wallVerts[2].y = wallVerts[3].y = FIXED_TO_FLOAT(worldlow);
			wallVerts[0].y = wallVerts[1].y = FIXED_TO_FLOAT(worldbottom);
#endif

			if (gr_frontsector->numlights)
				HWR_SplitWall(gr_frontsector, wallVerts, gr_bottomtexture, &Surf, FF_CUTLEVEL, NULL);
			else if (grTex->mipmap.flags & TF_TRANSPARENT)
				HWR_AddTransparentWall(wallVerts, &Surf, gr_bottomtexture, PF_Environment, false, lightnum, colormap);
			else
				HWR_ProjectWall(wallVerts, &Surf, PF_Masked, lightnum, colormap);
		}
		gr_midtexture = R_GetTextureNum(gr_sidedef->midtexture);
		if (gr_midtexture)
		{
			FBITFIELD blendmode;
			sector_t *front, *back;
			fixed_t  popentop, popenbottom, polytop, polybottom, lowcut, highcut;
			fixed_t     texturevpeg = 0;
			INT32 repeats;

			if (gr_linedef->frontsector->heightsec != -1)
				front = &sectors[gr_linedef->frontsector->heightsec];
			else
				front = gr_linedef->frontsector;

			if (gr_linedef->backsector->heightsec != -1)
				back = &sectors[gr_linedef->backsector->heightsec];
			else
				back = gr_linedef->backsector;

			if (gr_sidedef->repeatcnt)
				repeats = 1 + gr_sidedef->repeatcnt;
			else if (gr_linedef->flags & ML_EFFECT5)
			{
				fixed_t high, low;

				if (front->ceilingheight > back->ceilingheight)
					high = back->ceilingheight;
				else
					high = front->ceilingheight;

				if (front->floorheight > back->floorheight)
					low = front->floorheight;
				else
					low = back->floorheight;

				repeats = (high - low)/textureheight[gr_sidedef->midtexture];
				if ((high-low)%textureheight[gr_sidedef->midtexture])
					repeats++; // tile an extra time to fill the gap -- Monster Iestyn
			}
			else
				repeats = 1;

			// SoM: a little note: This code re-arranging will
			// fix the bug in Nimrod map02. popentop and popenbottom
			// record the limits the texture can be displayed in.
			// polytop and polybottom, are the ideal (i.e. unclipped)
			// heights of the polygon, and h & l, are the final (clipped)
			// poly coords.

#ifdef POLYOBJECTS
			// NOTE: With polyobjects, whenever you need to check the properties of the polyobject sector it belongs to,
			// you must use the linedef's backsector to be correct
			// From CB
			if (gr_curline->polyseg)
			{
				popentop = back->ceilingheight;
				popenbottom = back->floorheight;
			}
			else
#endif
            {
#ifdef ESLOPE
				popentop = min(worldtop, worldhigh);
				popenbottom = max(worldbottom, worldlow);
#else
				popentop = min(front->ceilingheight, back->ceilingheight);
				popenbottom = max(front->floorheight, back->floorheight);
#endif
			}

#ifdef ESLOPE
			if (gr_linedef->flags & ML_EFFECT2)
			{
				if (!!(gr_linedef->flags & ML_DONTPEGBOTTOM) ^ !!(gr_linedef->flags & ML_EFFECT3))
				{
					polybottom = max(front->floorheight, back->floorheight) + gr_sidedef->rowoffset;
					polytop = polybottom + textureheight[gr_midtexture]*repeats;
				}
				else
				{
					polytop = min(front->ceilingheight, back->ceilingheight) + gr_sidedef->rowoffset;
					polybottom = polytop - textureheight[gr_midtexture]*repeats;
				}
			}
			else if (!!(gr_linedef->flags & ML_DONTPEGBOTTOM) ^ !!(gr_linedef->flags & ML_EFFECT3))
#else
            if (gr_linedef->flags & ML_DONTPEGBOTTOM)
#endif
			{
				polybottom = popenbottom + gr_sidedef->rowoffset;
				polytop = polybottom + textureheight[gr_midtexture]*repeats;
			}
			else
			{
				polytop = popentop + gr_sidedef->rowoffset;
				polybottom = polytop - textureheight[gr_midtexture]*repeats;
			}
			// CB
#ifdef POLYOBJECTS
			// NOTE: With polyobjects, whenever you need to check the properties of the polyobject sector it belongs to,
			// you must use the linedef's backsector to be correct
			if (gr_curline->polyseg)
			{
				lowcut = polybottom;
				highcut = polytop;
			}
#endif
			else
			{
				// The cut-off values of a linedef can always be constant, since every line has an absoulute front and or back sector
				lowcut = popenbottom;
				highcut = popentop;
			}

			h = min(highcut, polytop);
			l = max(polybottom, lowcut);

			{
				// PEGGING
#ifdef ESLOPE
				if (!!(gr_linedef->flags & ML_DONTPEGBOTTOM) ^ !!(gr_linedef->flags & ML_EFFECT3))
#else
				if (gr_linedef->flags & ML_DONTPEGBOTTOM)
#endif
					texturevpeg = textureheight[gr_sidedef->midtexture]*repeats - h + polybottom;
				else
					texturevpeg = polytop - h;

				grTex = HWR_GetTexture(gr_midtexture);

				wallVerts[3].t = wallVerts[2].t = texturevpeg * grTex->scaleY;
				wallVerts[0].t = wallVerts[1].t = (h - l + texturevpeg) * grTex->scaleY;
				wallVerts[0].s = wallVerts[3].s = cliplow * grTex->scaleX;
				wallVerts[2].s = wallVerts[1].s = cliphigh * grTex->scaleX;
			}

			// set top/bottom coords
			// Take the texture peg into account, rather than changing the offsets past
			// where the polygon might not be.
			wallVerts[2].y = wallVerts[3].y = FIXED_TO_FLOAT(h);
			wallVerts[0].y = wallVerts[1].y = FIXED_TO_FLOAT(l);

#ifdef ESLOPE
			// Correct to account for slopes
			{
				fixed_t midtextureslant;

				if (gr_linedef->flags & ML_EFFECT2)
					midtextureslant = 0;
				else if (!!(gr_linedef->flags & ML_DONTPEGBOTTOM) ^ !!(gr_linedef->flags & ML_EFFECT3))
					midtextureslant = worldlow < worldbottom
							  ? worldbottomslope-worldbottom
							  : worldlowslope-worldlow;
				else
					midtextureslant = worldtop < worldhigh
							  ? worldtopslope-worldtop
							  : worldhighslope-worldhigh;

				polytop += midtextureslant;
				polybottom += midtextureslant;

				highcut += worldtop < worldhigh
						 ? worldtopslope-worldtop
						 : worldhighslope-worldhigh;
				lowcut += worldlow < worldbottom
						? worldbottomslope-worldbottom
						: worldlowslope-worldlow;

				// Texture stuff
				h = min(highcut, polytop);
				l = max(polybottom, lowcut);

				{
					// PEGGING
					if (!!(gr_linedef->flags & ML_DONTPEGBOTTOM) ^ !!(gr_linedef->flags & ML_EFFECT3))
						texturevpeg = textureheight[gr_sidedef->midtexture]*repeats - h + polybottom;
					else
						texturevpeg = polytop - h;
					wallVerts[2].t = texturevpeg * grTex->scaleY;
					wallVerts[1].t = (h - l + texturevpeg) * grTex->scaleY;
				}

				wallVerts[2].y = FIXED_TO_FLOAT(h);
				wallVerts[1].y = FIXED_TO_FLOAT(l);
			}
#endif

			// set alpha for transparent walls (new boom and legacy linedef types)
			// ooops ! this do not work at all because render order we should render it in backtofront order
			switch (gr_linedef->special)
			{
				case 900:
					blendmode = HWR_TranstableToAlpha(tr_trans10, &Surf);
					break;
				case 901:
					blendmode = HWR_TranstableToAlpha(tr_trans20, &Surf);
					break;
				case 902:
					blendmode = HWR_TranstableToAlpha(tr_trans30, &Surf);
					break;
				case 903:
					blendmode = HWR_TranstableToAlpha(tr_trans40, &Surf);
					break;
				case 904:
					blendmode = HWR_TranstableToAlpha(tr_trans50, &Surf);
					break;
				case 905:
					blendmode = HWR_TranstableToAlpha(tr_trans60, &Surf);
					break;
				case 906:
					blendmode = HWR_TranstableToAlpha(tr_trans70, &Surf);
					break;
				case 907:
					blendmode = HWR_TranstableToAlpha(tr_trans80, &Surf);
					break;
				case 908:
					blendmode = HWR_TranstableToAlpha(tr_trans90, &Surf);
					break;
				//  Translucent
				case 102:
				case 121:
				case 123:
				case 124:
				case 125:
				case 141:
				case 142:
				case 144:
				case 145:
				case 174:
				case 175:
				case 192:
				case 195:
				case 221:
				case 253:
				case 256:
					blendmode = PF_Translucent;
					break;
				default:
					blendmode = PF_Masked;
					break;
			}

#ifdef POLYOBJECTS
			if (gr_curline->polyseg && gr_curline->polyseg->translucency > 0)
			{
				if (gr_curline->polyseg->translucency >= NUMTRANSMAPS) // wall not drawn
				{
					Surf.PolyColor.s.alpha = 0x00; // This shouldn't draw anything regardless of blendmode
					blendmode = PF_Masked;
				}
				else
					blendmode = HWR_TranstableToAlpha(gr_curline->polyseg->translucency, &Surf);
			}
#endif

			if (gr_frontsector->numlights)
			{
				if (!(blendmode & PF_Masked))
					HWR_SplitWall(gr_frontsector, wallVerts, gr_midtexture, &Surf, FF_TRANSLUCENT, NULL);
				else
				{
					HWR_SplitWall(gr_frontsector, wallVerts, gr_midtexture, &Surf, FF_CUTLEVEL, NULL);
				}
			}
			else if (!(blendmode & PF_Masked))
				HWR_AddTransparentWall(wallVerts, &Surf, gr_midtexture, blendmode, false, lightnum, colormap);
			else
				HWR_ProjectWall(wallVerts, &Surf, blendmode, lightnum, colormap);
		}
	}
	else
	{
		// Single sided line... Deal only with the middletexture (if one exists)
		gr_midtexture = R_GetTextureNum(gr_sidedef->midtexture);
		if (gr_midtexture && gr_linedef->special != 41) // (Ignore horizon line for OGL)
		{
			{
				fixed_t     texturevpeg;
				// PEGGING
#ifdef ESLOPE
				if ((gr_linedef->flags & (ML_DONTPEGBOTTOM|ML_EFFECT2)) == (ML_DONTPEGBOTTOM|ML_EFFECT2))
					texturevpeg = gr_frontsector->floorheight + textureheight[gr_sidedef->midtexture] - gr_frontsector->ceilingheight + gr_sidedef->rowoffset;
				else
#endif
				if (gr_linedef->flags & ML_DONTPEGBOTTOM)
					texturevpeg = worldbottom + textureheight[gr_sidedef->midtexture] - worldtop + gr_sidedef->rowoffset;
				else
					// top of texture at top
					texturevpeg = gr_sidedef->rowoffset;

				grTex = HWR_GetTexture(gr_midtexture);

				wallVerts[3].t = wallVerts[2].t = texturevpeg * grTex->scaleY;
				wallVerts[0].t = wallVerts[1].t = (texturevpeg + gr_frontsector->ceilingheight - gr_frontsector->floorheight) * grTex->scaleY;
				wallVerts[0].s = wallVerts[3].s = cliplow * grTex->scaleX;
				wallVerts[2].s = wallVerts[1].s = cliphigh * grTex->scaleX;

#ifdef ESLOPE
				// Texture correction for slopes
				if (gr_linedef->flags & ML_EFFECT2) {
					wallVerts[3].t += (gr_frontsector->ceilingheight - worldtop) * grTex->scaleY;
					wallVerts[2].t += (gr_frontsector->ceilingheight - worldtopslope) * grTex->scaleY;
					wallVerts[0].t += (gr_frontsector->floorheight - worldbottom) * grTex->scaleY;
					wallVerts[1].t += (gr_frontsector->floorheight - worldbottomslope) * grTex->scaleY;
				} else if (gr_linedef->flags & ML_DONTPEGBOTTOM) {
					wallVerts[3].t = wallVerts[0].t + (worldbottom-worldtop) * grTex->scaleY;
					wallVerts[2].t = wallVerts[1].t + (worldbottomslope-worldtopslope) * grTex->scaleY;
				} else {
					wallVerts[0].t = wallVerts[3].t - (worldbottom-worldtop) * grTex->scaleY;
					wallVerts[1].t = wallVerts[2].t - (worldbottomslope-worldtopslope) * grTex->scaleY;
				}
#endif
			}
#ifdef ESLOPE
			//Set textures properly on single sided walls that are sloped
			wallVerts[3].y = FIXED_TO_FLOAT(worldtop);
			wallVerts[0].y = FIXED_TO_FLOAT(worldbottom);
			wallVerts[2].y = FIXED_TO_FLOAT(worldtopslope);
			wallVerts[1].y = FIXED_TO_FLOAT(worldbottomslope);
#else
			// set top/bottom coords
			wallVerts[2].y = wallVerts[3].y = FIXED_TO_FLOAT(worldtop);
			wallVerts[0].y = wallVerts[1].y = FIXED_TO_FLOAT(worldbottom);
#endif

			if (gr_frontsector->numlights)
				HWR_SplitWall(gr_frontsector, wallVerts, gr_midtexture, &Surf, FF_CUTLEVEL, NULL);
			// I don't think that solid walls can use translucent linedef types...
			else
			{
				if (grTex->mipmap.flags & TF_TRANSPARENT)
					HWR_AddTransparentWall(wallVerts, &Surf, gr_midtexture, PF_Environment, false, lightnum, colormap);
				else
					HWR_ProjectWall(wallVerts, &Surf, PF_Masked, lightnum, colormap);
			}
		}
		else
		{
#ifdef ESLOPE
			//Set textures properly on single sided walls that are sloped
			wallVerts[3].y = FIXED_TO_FLOAT(worldtop);
			wallVerts[0].y = FIXED_TO_FLOAT(worldbottom);
			wallVerts[2].y = FIXED_TO_FLOAT(worldtopslope);
			wallVerts[1].y = FIXED_TO_FLOAT(worldbottomslope);
#else
			// set top/bottom coords
			wallVerts[2].y = wallVerts[3].y = FIXED_TO_FLOAT(worldtop);
			wallVerts[0].y = wallVerts[1].y = FIXED_TO_FLOAT(worldbottom);
#endif

			// When there's no midtexture, draw a skywall to prevent rendering behind it
			HWR_DrawSkyWall(wallVerts, &Surf);
		}


		// Single sided lines are simple for skywalls, just need to draw from the top or bottom of the sector if there's
		// a sky flat
		if (!gr_curline->polyseg)
		{
			if (gr_frontsector->ceilingpic == skyflatnum) // It's a single-sided line with sky for its sector
			{
				wallVerts[3].y = wallVerts[2].y = FIXED_TO_FLOAT(INT32_MAX);
#ifdef ESLOPE
				wallVerts[0].y = FIXED_TO_FLOAT(worldtop);
				wallVerts[1].y = FIXED_TO_FLOAT(worldtopslope);
#else
				wallVerts[0].y = wallVerts[1].y = FIXED_TO_FLOAT(worldtop);
#endif
				HWR_DrawSkyWall(wallVerts, &Surf);
			}
			if (gr_frontsector->floorpic == skyflatnum)
			{
#ifdef ESLOPE
				wallVerts[3].y = FIXED_TO_FLOAT(worldbottom);
				wallVerts[2].y = FIXED_TO_FLOAT(worldbottomslope);
#else
				wallVerts[3].y = wallVerts[2].y = FIXED_TO_FLOAT(worldbottom);
#endif
				wallVerts[0].y = wallVerts[1].y = FIXED_TO_FLOAT(INT32_MIN);

				HWR_DrawSkyWall(wallVerts, &Surf);
			}
		}
	}


	//Hurdler: 3d-floors test
	if (gr_frontsector && gr_backsector && gr_frontsector->tag != gr_backsector->tag && (gr_backsector->ffloors || gr_frontsector->ffloors))
	{
		ffloor_t * rover;
		fixed_t    highcut = 0, lowcut = 0;

		INT32 texnum;
		line_t * newline = NULL; // Multi-Property FOF

        ///TODO add slope support (fixing cutoffs, proper wall clipping) - maybe just disable highcut/lowcut if either sector or FOF has a slope
        ///     to allow fun plane intersecting in OGL? But then people would abuse that and make software look bad. :C
		highcut = gr_frontsector->ceilingheight < gr_backsector->ceilingheight ? gr_frontsector->ceilingheight : gr_backsector->ceilingheight;
		lowcut = gr_frontsector->floorheight > gr_backsector->floorheight ? gr_frontsector->floorheight : gr_backsector->floorheight;

		if (gr_backsector->ffloors)
		{
			for (rover = gr_backsector->ffloors; rover; rover = rover->next)
			{
				if (!(rover->flags & FF_EXISTS) || !(rover->flags & FF_RENDERSIDES) || (rover->flags & FF_INVERTSIDES))
					continue;
				if (*rover->topheight < lowcut || *rover->bottomheight > highcut)
					continue;

				texnum = R_GetTextureNum(sides[rover->master->sidenum[0]].midtexture);

				if (rover->master->flags & ML_TFERLINE)
				{
					size_t linenum = gr_curline->linedef-gr_backsector->lines[0];
					newline = rover->master->frontsector->lines[0] + linenum;
					texnum = R_GetTextureNum(sides[newline->sidenum[0]].midtexture);
				}

#ifdef ESLOPE
				h  = *rover->t_slope ? P_GetZAt(*rover->t_slope, v1x, v1y) : *rover->topheight;
				hS = *rover->t_slope ? P_GetZAt(*rover->t_slope, v2x, v2y) : *rover->topheight;
				l  = *rover->b_slope ? P_GetZAt(*rover->b_slope, v1x, v1y) : *rover->bottomheight;
				lS = *rover->b_slope ? P_GetZAt(*rover->b_slope, v2x, v2y) : *rover->bottomheight;
				if (!(*rover->t_slope) && !gr_frontsector->c_slope && !gr_backsector->c_slope && h > highcut)
					h = hS = highcut;
				if (!(*rover->b_slope) && !gr_frontsector->f_slope && !gr_backsector->f_slope && l < lowcut)
					l = lS = lowcut;
				//Hurdler: HW code starts here
				//FIXME: check if peging is correct
				// set top/bottom coords

				wallVerts[3].y = FIXED_TO_FLOAT(h);
				wallVerts[2].y = FIXED_TO_FLOAT(hS);
				wallVerts[0].y = FIXED_TO_FLOAT(l);
				wallVerts[1].y = FIXED_TO_FLOAT(lS);
#else
				h = *rover->topheight;
				l = *rover->bottomheight;
				if (h > highcut)
					h = highcut;
				if (l < lowcut)
					l = lowcut;
				//Hurdler: HW code starts here
				//FIXME: check if peging is correct
				// set top/bottom coords
				wallVerts[2].y = wallVerts[3].y = FIXED_TO_FLOAT(h);
				wallVerts[0].y = wallVerts[1].y = FIXED_TO_FLOAT(l);
#endif
				if (rover->flags & FF_FOG)
				{
					wallVerts[3].t = wallVerts[2].t = 0;
					wallVerts[0].t = wallVerts[1].t = 0;
					wallVerts[0].s = wallVerts[3].s = 0;
					wallVerts[2].s = wallVerts[1].s = 0;
				}
				else
				{
					fixed_t texturevpeg;
					boolean attachtobottom = false;
#ifdef ESLOPE
					boolean slopeskew = false; // skew FOF walls with slopes?
#endif

					// Wow, how was this missing from OpenGL for so long?
					// ...Oh well, anyway, Lower Unpegged now changes pegging of FOFs like in software
					// -- Monster Iestyn 26/06/18
					if (newline)
					{
						texturevpeg = sides[newline->sidenum[0]].rowoffset;
						attachtobottom = !!(newline->flags & ML_DONTPEGBOTTOM);
#ifdef ESLOPE
						slopeskew = !!(newline->flags & ML_DONTPEGTOP);
#endif
					}
					else
					{
						texturevpeg = sides[rover->master->sidenum[0]].rowoffset;
						attachtobottom = !!(gr_linedef->flags & ML_DONTPEGBOTTOM);
#ifdef ESLOPE
						slopeskew = !!(rover->master->flags & ML_DONTPEGTOP);
#endif
					}

					grTex = HWR_GetTexture(texnum);

#ifdef ESLOPE
					if (!slopeskew) // no skewing
					{
						if (attachtobottom)
							texturevpeg -= *rover->topheight - *rover->bottomheight;
						wallVerts[3].t = (*rover->topheight - h + texturevpeg) * grTex->scaleY;
						wallVerts[2].t = (*rover->topheight - hS + texturevpeg) * grTex->scaleY;
						wallVerts[0].t = (*rover->topheight - l + texturevpeg) * grTex->scaleY;
						wallVerts[1].t = (*rover->topheight - lS + texturevpeg) * grTex->scaleY;
					}
					else
					{
						if (!attachtobottom) // skew by top
						{
							wallVerts[3].t = wallVerts[2].t = texturevpeg * grTex->scaleY;
							wallVerts[0].t = (h - l + texturevpeg) * grTex->scaleY;
							wallVerts[1].t = (hS - lS + texturevpeg) * grTex->scaleY;
						}
						else // skew by bottom
						{
							wallVerts[0].t = wallVerts[1].t = texturevpeg * grTex->scaleY;
							wallVerts[3].t = wallVerts[0].t - (h - l) * grTex->scaleY;
							wallVerts[2].t = wallVerts[1].t - (hS - lS) * grTex->scaleY;
						}
					}
#else
					if (attachtobottom)
						texturevpeg -= *rover->topheight - *rover->bottomheight;
					wallVerts[3].t = wallVerts[2].t = (*rover->topheight - h + texturevpeg) * grTex->scaleY;
					wallVerts[0].t = wallVerts[1].t = (*rover->topheight - l + texturevpeg) * grTex->scaleY;
#endif

					wallVerts[0].s = wallVerts[3].s = cliplow * grTex->scaleX;
					wallVerts[2].s = wallVerts[1].s = cliphigh * grTex->scaleX;
				}
				if (rover->flags & FF_FOG)
				{
					FBITFIELD blendmode;

					blendmode = PF_Fog|PF_NoTexture;

					lightnum = HWR_CalcWallLight(rover->master->frontsector->lightlevel, vs.x, vs.y, ve.x, ve.y);
					colormap = rover->master->frontsector->extra_colormap;

					Surf.PolyColor.s.alpha = HWR_FogBlockAlpha(rover->master->frontsector->lightlevel, rover->master->frontsector->extra_colormap);

					if (gr_frontsector->numlights)
						HWR_SplitWall(gr_frontsector, wallVerts, 0, &Surf, rover->flags, rover);
					else
						HWR_AddTransparentWall(wallVerts, &Surf, 0, blendmode, true, lightnum, colormap);
				}
				else
				{
					FBITFIELD blendmode = PF_Masked;

					if (rover->flags & FF_TRANSLUCENT && rover->alpha < 256)
					{
						blendmode = PF_Translucent;
						Surf.PolyColor.s.alpha = (UINT8)rover->alpha-1 > 255 ? 255 : rover->alpha-1;
					}

					if (gr_frontsector->numlights)
						HWR_SplitWall(gr_frontsector, wallVerts, texnum, &Surf, rover->flags, rover);
					else
					{
						if (blendmode != PF_Masked)
							HWR_AddTransparentWall(wallVerts, &Surf, texnum, blendmode, false, lightnum, colormap);
						else
							HWR_ProjectWall(wallVerts, &Surf, PF_Masked, lightnum, colormap);
					}
				}
			}
		}

		if (gr_frontsector->ffloors) // Putting this seperate should allow 2 FOF sectors to be connected without too many errors? I think?
		{
			for (rover = gr_frontsector->ffloors; rover; rover = rover->next)
			{
				if (!(rover->flags & FF_EXISTS) || !(rover->flags & FF_RENDERSIDES) || !(rover->flags & FF_ALLSIDES))
					continue;
				if (*rover->topheight < lowcut || *rover->bottomheight > highcut)
					continue;

				texnum = R_GetTextureNum(sides[rover->master->sidenum[0]].midtexture);

				if (rover->master->flags & ML_TFERLINE)
				{
					size_t linenum = gr_curline->linedef-gr_backsector->lines[0];
					newline = rover->master->frontsector->lines[0] + linenum;
					texnum = R_GetTextureNum(sides[newline->sidenum[0]].midtexture);
				}
#ifdef ESLOPE //backsides
				h  = *rover->t_slope ? P_GetZAt(*rover->t_slope, v1x, v1y) : *rover->topheight;
				hS = *rover->t_slope ? P_GetZAt(*rover->t_slope, v2x, v2y) : *rover->topheight;
				l  = *rover->b_slope ? P_GetZAt(*rover->b_slope, v1x, v1y) : *rover->bottomheight;
				lS = *rover->b_slope ? P_GetZAt(*rover->b_slope, v2x, v2y) : *rover->bottomheight;
				if (!(*rover->t_slope) && !gr_frontsector->c_slope && !gr_backsector->c_slope && h > highcut)
					h = hS = highcut;
				if (!(*rover->b_slope) && !gr_frontsector->f_slope && !gr_backsector->f_slope && l < lowcut)
					l = lS = lowcut;
				//Hurdler: HW code starts here
				//FIXME: check if peging is correct
				// set top/bottom coords

				wallVerts[3].y = FIXED_TO_FLOAT(h);
				wallVerts[2].y = FIXED_TO_FLOAT(hS);
				wallVerts[0].y = FIXED_TO_FLOAT(l);
				wallVerts[1].y = FIXED_TO_FLOAT(lS);
#else
				h = *rover->topheight;
				l = *rover->bottomheight;
				if (h > highcut)
					h = highcut;
				if (l < lowcut)
					l = lowcut;
				//Hurdler: HW code starts here
				//FIXME: check if peging is correct
				// set top/bottom coords
				wallVerts[2].y = wallVerts[3].y = FIXED_TO_FLOAT(h);
				wallVerts[0].y = wallVerts[1].y = FIXED_TO_FLOAT(l);
#endif
				if (rover->flags & FF_FOG)
				{
					wallVerts[3].t = wallVerts[2].t = 0;
					wallVerts[0].t = wallVerts[1].t = 0;
					wallVerts[0].s = wallVerts[3].s = 0;
					wallVerts[2].s = wallVerts[1].s = 0;
				}
				else
				{
					grTex = HWR_GetTexture(texnum);

					if (newline)
					{
						wallVerts[3].t = wallVerts[2].t = (*rover->topheight - h + sides[newline->sidenum[0]].rowoffset) * grTex->scaleY;
						wallVerts[0].t = wallVerts[1].t = (h - l + (*rover->topheight - h + sides[newline->sidenum[0]].rowoffset)) * grTex->scaleY;
					}
					else
					{
						wallVerts[3].t = wallVerts[2].t = (*rover->topheight - h + sides[rover->master->sidenum[0]].rowoffset) * grTex->scaleY;
						wallVerts[0].t = wallVerts[1].t = (h - l + (*rover->topheight - h + sides[rover->master->sidenum[0]].rowoffset)) * grTex->scaleY;
					}

					wallVerts[0].s = wallVerts[3].s = cliplow * grTex->scaleX;
					wallVerts[2].s = wallVerts[1].s = cliphigh * grTex->scaleX;
				}

				if (rover->flags & FF_FOG)
				{
					FBITFIELD blendmode;

					blendmode = PF_Fog|PF_NoTexture;

					lightnum = HWR_CalcWallLight(rover->master->frontsector->lightlevel, vs.x, vs.y, ve.x, ve.y);
					colormap = rover->master->frontsector->extra_colormap;

					Surf.PolyColor.s.alpha = HWR_FogBlockAlpha(rover->master->frontsector->lightlevel, rover->master->frontsector->extra_colormap);

					if (gr_backsector->numlights)
						HWR_SplitWall(gr_backsector, wallVerts, 0, &Surf, rover->flags, rover);
					else
						HWR_AddTransparentWall(wallVerts, &Surf, 0, blendmode, true, lightnum, colormap);
				}
				else
				{
					FBITFIELD blendmode = PF_Masked;

					if (rover->flags & FF_TRANSLUCENT && rover->alpha < 256)
					{
						blendmode = PF_Translucent;
						Surf.PolyColor.s.alpha = (UINT8)rover->alpha-1 > 255 ? 255 : rover->alpha-1;
					}

					if (gr_backsector->numlights)
						HWR_SplitWall(gr_backsector, wallVerts, texnum, &Surf, rover->flags, rover);
					else
					{
						if (blendmode != PF_Masked)
							HWR_AddTransparentWall(wallVerts, &Surf, texnum, blendmode, false, lightnum, colormap);
						else
							HWR_ProjectWall(wallVerts, &Surf, PF_Masked, lightnum, colormap);
					}
				}
			}
		}
	}
//Hurdler: end of 3d-floors test
}

// From PrBoom:
//
// e6y: Check whether the player can look beyond this line, rturns true if we can't
//

boolean checkforemptylines = true;
// Don't modify anything here, just check
// Kalaron: Modified for sloped linedefs
static boolean CheckClip(sector_t * afrontsector, sector_t * abacksector)
{
	fixed_t frontf1,frontf2, frontc1, frontc2; // front floor/ceiling ends
	fixed_t backf1, backf2, backc1, backc2; // back floor ceiling ends

	// GZDoom method of sloped line clipping

#ifdef ESLOPE
	if (afrontsector->f_slope || afrontsector->c_slope || abacksector->f_slope || abacksector->c_slope)
	{
		fixed_t v1x, v1y, v2x, v2y; // the seg's vertexes as fixed_t
		v1x = FLOAT_TO_FIXED(((polyvertex_t *)gr_curline->pv1)->x);
		v1y = FLOAT_TO_FIXED(((polyvertex_t *)gr_curline->pv1)->y);
		v2x = FLOAT_TO_FIXED(((polyvertex_t *)gr_curline->pv2)->x);
		v2y = FLOAT_TO_FIXED(((polyvertex_t *)gr_curline->pv2)->y);
#define SLOPEPARAMS(slope, end1, end2, normalheight) \
		if (slope) { \
			end1 = P_GetZAt(slope, v1x, v1y); \
			end2 = P_GetZAt(slope, v2x, v2y); \
		} else \
			end1 = end2 = normalheight;

		SLOPEPARAMS(afrontsector->f_slope, frontf1, frontf2, afrontsector->floorheight)
		SLOPEPARAMS(afrontsector->c_slope, frontc1, frontc2, afrontsector->ceilingheight)
		SLOPEPARAMS( abacksector->f_slope, backf1,  backf2,  abacksector->floorheight)
		SLOPEPARAMS( abacksector->c_slope, backc1,  backc2,  abacksector->ceilingheight)
#undef SLOPEPARAMS
	}
	else
#endif
	{
		frontf1 = frontf2 = afrontsector->floorheight;
		frontc1 = frontc2 = afrontsector->ceilingheight;
		backf1 = backf2 = abacksector->floorheight;
		backc1 = backc2 = abacksector->ceilingheight;
	}


	// now check for closed sectors!

	// here we're talking about a CEILING lower than a floor. ...yeah we don't even need to bother.
	if (backc1 <= frontf1 && backc2 <= frontf2)
	{
		checkforemptylines = false;
		return true;
	}

	// here we're talking about floors higher than ceilings, don't even bother either.
	if (backf1 >= frontc1 && backf2 >= frontc2)
	{
		checkforemptylines = false;
		return true;
	}

	// Lat: Ok, here's what we need to do, we want to draw thok barriers. Let's define what a thok barrier is;
	// -Must have ceilheight <= floorheight
	// -ceilpic must be skyflatnum
	// -an adjacant sector needs to have a ceilingheight or a floor height different than the one we have, otherwise, it's just a huge ass wall, we shouldn't render past it.
	// -said adjacant sector cannot also be a thok barrier, because that's also dumb and we could render far more than we need to as a result :V

	if (backc1 <= backf1 && backc2 <= backf2)
	{
		checkforemptylines = false;

		// before we do anything, if both sectors are thok barriers, GET ME OUT OF HERE!
		if (frontc1 <= backc1 && frontc2 <= backc2)
			return true;	// STOP RENDERING.

		// draw floors at the top of thok barriers:
		if (backc1 < frontc1 || backc2 < frontc2)
			return false;

		if (backf1 > frontf1 || backf2 > frontf2)
			return false;

		return true;
	}

	// Window.
	// We know it's a window when the above isn't true and the back and front sectors don't match
	if (backc1 != frontc1 || backc2 != frontc2
		|| backf1 != frontf1 || backf2 != frontf2)
	{
		checkforemptylines = false;
		return false;
	}

	// In this case we just need to check whether there is actually a need to render any lines, so checkforempty lines
	// stays true
	return false;
}

// HWR_AddLine
// Clips the given segment and adds any visible pieces to the line list.
void HWR_AddLine(seg_t *line)
{
	angle_t angle1, angle2;

	// SoM: Backsector needs to be run through R_FakeFlat
	static sector_t tempsec;

	fixed_t v1x, v1y, v2x, v2y; // the seg's vertexes as fixed_t
#ifdef POLYOBJECTS
	if (line->polyseg && !(line->polyseg->flags & POF_RENDERSIDES))
		return;
#endif

	gr_curline = line;

	v1x = FLOAT_TO_FIXED(((polyvertex_t *)gr_curline->pv1)->x);
	v1y = FLOAT_TO_FIXED(((polyvertex_t *)gr_curline->pv1)->y);
	v2x = FLOAT_TO_FIXED(((polyvertex_t *)gr_curline->pv2)->x);
	v2y = FLOAT_TO_FIXED(((polyvertex_t *)gr_curline->pv2)->y);

	// OPTIMIZE: quickly reject orthogonal back sides.
	angle1 = R_PointToAngleEx(viewx, viewy, v1x, v1y);
	angle2 = R_PointToAngleEx(viewx, viewy, v2x, v2y);

	 // PrBoom: Back side, i.e. backface culling - read: endAngle >= startAngle!
	if (angle2 - angle1 < ANGLE_180)
		return;

	// PrBoom: use REAL clipping math YAYYYYYYY!!!
	if (!gld_clipper_SafeCheckRange(angle2, angle1))
		return;

	checkforemptylines = true;

	gr_backsector = line->backsector;

	if (!line->backsector)
		gld_clipper_SafeAddClipRange(angle2, angle1);
	else
	{
		gr_backsector = R_FakeFlat(gr_backsector, &tempsec, NULL, NULL, true);
		if (CheckClip(gr_frontsector, gr_backsector))
		{
			gld_clipper_SafeAddClipRange(angle2, angle1);
			checkforemptylines = false;
		}
		// Reject empty lines used for triggers and special events.
		// Identical floor and ceiling on both sides,
		//  identical light levels on both sides,
		//  and no middle texture.
		if (checkforemptylines && R_IsEmptyLine(line, gr_frontsector, gr_backsector))
			return;
    }

	HWR_ProcessSeg(); // Doesn't need arguments because they're defined globally :D
	return;
}

// HWR_CheckBBox
// Checks BSP node/subtree bounding box.
// Returns true
//  if some part of the bbox might be visible.
//
// modified to use local variables

boolean HWR_CheckBBox(fixed_t *bspcoord)
{
	INT32 boxpos;
	fixed_t px1, py1, px2, py2;
	angle_t angle1, angle2;

	// Find the corners of the box
	// that define the edges from current viewpoint.
	if (viewx <= bspcoord[BOXLEFT])
		boxpos = 0;
	else if (viewx < bspcoord[BOXRIGHT])
		boxpos = 1;
	else
		boxpos = 2;

	if (viewy >= bspcoord[BOXTOP])
		boxpos |= 0;
	else if (viewy > bspcoord[BOXBOTTOM])
		boxpos |= 1<<2;
	else
		boxpos |= 2<<2;

	if (boxpos == 5)
		return true;

	px1 = bspcoord[checkcoord[boxpos][0]];
	py1 = bspcoord[checkcoord[boxpos][1]];
	px2 = bspcoord[checkcoord[boxpos][2]];
	py2 = bspcoord[checkcoord[boxpos][3]];

	angle1 = R_PointToAngleEx(viewx, viewy, px1, py1);
	angle2 = R_PointToAngleEx(viewx, viewy, px2, py2);
	return gld_clipper_SafeCheckRange(angle2, angle1);
}

#ifdef POLYOBJECTS

//
// HWR_AddPolyObjectSegs
//
// haleyjd 02/19/06
// Adds all segs in all polyobjects in the given subsector.
// Modified for hardware rendering.
//
void HWR_AddPolyObjectSegs(void)
{
	size_t i, j;
	seg_t *gr_fakeline = Z_Calloc(sizeof(seg_t), PU_STATIC, NULL);
	polyvertex_t *pv1 = Z_Calloc(sizeof(polyvertex_t), PU_STATIC, NULL);
	polyvertex_t *pv2 = Z_Calloc(sizeof(polyvertex_t), PU_STATIC, NULL);

	// Sort through all the polyobjects
	for (i = 0; i < numpolys; ++i)
	{
		// Render the polyobject's lines
		for (j = 0; j < po_ptrs[i]->segCount; ++j)
		{
			// Copy the info of a polyobject's seg, then convert it to OpenGL floating point
			M_Memcpy(gr_fakeline, po_ptrs[i]->segs[j], sizeof(seg_t));

			// Now convert the line to float and add it to be rendered
			pv1->x = FIXED_TO_FLOAT(gr_fakeline->v1->x);
			pv1->y = FIXED_TO_FLOAT(gr_fakeline->v1->y);
			pv2->x = FIXED_TO_FLOAT(gr_fakeline->v2->x);
			pv2->y = FIXED_TO_FLOAT(gr_fakeline->v2->y);

			gr_fakeline->pv1 = pv1;
			gr_fakeline->pv2 = pv2;

			HWR_AddLine(gr_fakeline);
		}
	}

	// Free temporary data no longer needed
	Z_Free(pv2);
	Z_Free(pv1);
	Z_Free(gr_fakeline);
}

#ifdef POLYOBJECTS_PLANES
void HWR_RenderPolyObjectPlane(polyobj_t *polysector, boolean isceiling, fixed_t fixedheight, FBITFIELD blendmode, UINT8 lightlevel, lumpnum_t lumpnum, sector_t *FOFsector, UINT8 alpha, extracolormap_t *planecolormap)
{
	float           height; //constant y for all points on the convex flat polygon
	FOutVector      *v3d;
	INT32             i;
	float           flatxref,flatyref;
	float fflatsize;
	INT32 flatflag;
	size_t len;
	float scrollx = 0.0f, scrolly = 0.0f;
	angle_t angle = 0;
	FSurfaceInfo    Surf;
	fixed_t tempxsow, tempytow;
	size_t nrPlaneVerts;

	static FOutVector *planeVerts = NULL;
	static UINT16 numAllocedPlaneVerts = 0;

	nrPlaneVerts = polysector->numVertices;

	height = FIXED_TO_FLOAT(fixedheight);

	if (nrPlaneVerts < 3)   //not even a triangle ?
		return;

	if (nrPlaneVerts > INT16_MAX) // FIXME: exceeds plVerts size
	{
		CONS_Debug(DBG_RENDER, "polygon size of %s exceeds max value of %d vertices\n", sizeu1(nrPlaneVerts), UINT16_MAX);
		return;
	}

	// Allocate plane-vertex buffer if we need to
	if (!planeVerts || nrPlaneVerts > numAllocedPlaneVerts)
	{
		numAllocedPlaneVerts = (UINT16)nrPlaneVerts;
		Z_Free(planeVerts);
		Z_Malloc(numAllocedPlaneVerts * sizeof (FOutVector), PU_LEVEL, &planeVerts);
	}

	len = W_LumpLength(lumpnum);

	switch (len)
	{
		case 4194304: // 2048x2048 lump
			fflatsize = 2048.0f;
			flatflag = 2047;
			break;
		case 1048576: // 1024x1024 lump
			fflatsize = 1024.0f;
			flatflag = 1023;
			break;
		case 262144:// 512x512 lump
			fflatsize = 512.0f;
			flatflag = 511;
			break;
		case 65536: // 256x256 lump
			fflatsize = 256.0f;
			flatflag = 255;
			break;
		case 16384: // 128x128 lump
			fflatsize = 128.0f;
			flatflag = 127;
			break;
		case 1024: // 32x32 lump
			fflatsize = 32.0f;
			flatflag = 31;
			break;
		default: // 64x64 lump
			fflatsize = 64.0f;
			flatflag = 63;
			break;
	}

	// reference point for flat texture coord for each vertex around the polygon
	flatxref = (float)(((fixed_t)FIXED_TO_FLOAT(polysector->origVerts[0].x) & (~flatflag)) / fflatsize);
	flatyref = (float)(((fixed_t)FIXED_TO_FLOAT(polysector->origVerts[0].y) & (~flatflag)) / fflatsize);

	// transform
	v3d = planeVerts;

	if (FOFsector != NULL)
	{
		if (!isceiling) // it's a floor
		{
			scrollx = FIXED_TO_FLOAT(FOFsector->floor_xoffs)/fflatsize;
			scrolly = FIXED_TO_FLOAT(FOFsector->floor_yoffs)/fflatsize;
			angle = FOFsector->floorpic_angle>>ANGLETOFINESHIFT;
		}
		else // it's a ceiling
		{
			scrollx = FIXED_TO_FLOAT(FOFsector->ceiling_xoffs)/fflatsize;
			scrolly = FIXED_TO_FLOAT(FOFsector->ceiling_yoffs)/fflatsize;
			angle = FOFsector->ceilingpic_angle>>ANGLETOFINESHIFT;
		}
	}
	else if (gr_frontsector)
	{
		if (!isceiling) // it's a floor
		{
			scrollx = FIXED_TO_FLOAT(gr_frontsector->floor_xoffs)/fflatsize;
			scrolly = FIXED_TO_FLOAT(gr_frontsector->floor_yoffs)/fflatsize;
			angle = gr_frontsector->floorpic_angle>>ANGLETOFINESHIFT;
		}
		else // it's a ceiling
		{
			scrollx = FIXED_TO_FLOAT(gr_frontsector->ceiling_xoffs)/fflatsize;
			scrolly = FIXED_TO_FLOAT(gr_frontsector->ceiling_yoffs)/fflatsize;
			angle = gr_frontsector->ceilingpic_angle>>ANGLETOFINESHIFT;
		}
	}

	if (angle) // Only needs to be done if there's an altered angle
	{
		// This needs to be done so that it scrolls in a different direction after rotation like software
		tempxsow = FLOAT_TO_FIXED(scrollx);
		tempytow = FLOAT_TO_FIXED(scrolly);
		scrollx = (FIXED_TO_FLOAT(FixedMul(tempxsow, FINECOSINE(angle)) - FixedMul(tempytow, FINESINE(angle))));
		scrolly = (FIXED_TO_FLOAT(FixedMul(tempxsow, FINESINE(angle)) + FixedMul(tempytow, FINECOSINE(angle))));

		// This needs to be done so everything aligns after rotation
		// It would be done so that rotation is done, THEN the translation, but I couldn't get it to rotate AND scroll like software does
		tempxsow = FLOAT_TO_FIXED(flatxref);
		tempytow = FLOAT_TO_FIXED(flatyref);
		flatxref = (FIXED_TO_FLOAT(FixedMul(tempxsow, FINECOSINE(angle)) - FixedMul(tempytow, FINESINE(angle))));
		flatyref = (FIXED_TO_FLOAT(FixedMul(tempxsow, FINESINE(angle)) + FixedMul(tempytow, FINECOSINE(angle))));
	}

	for (i = 0; i < (INT32)nrPlaneVerts; i++,v3d++)
	{
		// Hurdler: add scrolling texture on floor/ceiling
		v3d->s = (float)((FIXED_TO_FLOAT(polysector->origVerts[i].x) / fflatsize) - flatxref + scrollx); // Go from the polysector's original vertex locations
		v3d->t = (float)(flatyref - (FIXED_TO_FLOAT(polysector->origVerts[i].y) / fflatsize) + scrolly); // Means the flat is offset based on the original vertex locations

		// Need to rotate before translate
		if (angle) // Only needs to be done if there's an altered angle
		{
			tempxsow = FLOAT_TO_FIXED(v3d->s);
			tempytow = FLOAT_TO_FIXED(v3d->t);
			v3d->s = (FIXED_TO_FLOAT(FixedMul(tempxsow, FINECOSINE(angle)) - FixedMul(tempytow, FINESINE(angle))));
			v3d->t = (FIXED_TO_FLOAT(-FixedMul(tempxsow, FINESINE(angle)) - FixedMul(tempytow, FINECOSINE(angle))));
		}

		v3d->x = FIXED_TO_FLOAT(polysector->vertices[i]->x);
		v3d->y = height;
		v3d->z = FIXED_TO_FLOAT(polysector->vertices[i]->y);
	}

	HWR_Lighting(&Surf, lightlevel, planecolormap);

	if (blendmode & PF_Translucent)
	{
		Surf.PolyColor.s.alpha = (UINT8)alpha;
		blendmode |= PF_Modulated|PF_Occlude;
	}
	else
		blendmode |= PF_Masked|PF_Modulated;

	HWD.pfnSetShader(1);	// floor shader
	HWD.pfnDrawPolygon(&Surf, planeVerts, nrPlaneVerts, blendmode);
}

void HWR_AddPolyObjectPlanes(void)
{
	size_t i;
	sector_t *polyobjsector;

	// Polyobject Planes need their own function for drawing because they don't have extrasubsectors by themselves
	// It should be okay because polyobjects should always be convex anyway

	for (i  = 0; i < numpolys; i++)
	{
		polyobjsector = po_ptrs[i]->lines[0]->backsector; // the in-level polyobject sector

		if (!(po_ptrs[i]->flags & POF_RENDERPLANES)) // Only render planes when you should
			continue;

		if (po_ptrs[i]->translucency >= NUMTRANSMAPS)
			continue;

		if (polyobjsector->floorheight <= gr_frontsector->ceilingheight
			&& polyobjsector->floorheight >= gr_frontsector->floorheight
			&& (viewz < polyobjsector->floorheight))
		{
			if (po_ptrs[i]->translucency > 0)
			{
				FSurfaceInfo Surf;
				FBITFIELD blendmode = HWR_TranstableToAlpha(po_ptrs[i]->translucency, &Surf);
				HWR_AddTransparentPolyobjectFloor(levelflats[polyobjsector->floorpic].lumpnum, po_ptrs[i], false, polyobjsector->floorheight,
													polyobjsector->lightlevel, Surf.PolyColor.s.alpha, polyobjsector, blendmode, NULL);
			}
			else
			{
				HWR_GetFlat(levelflats[polyobjsector->floorpic].lumpnum, R_NoEncore(polyobjsector, false));
				HWR_RenderPolyObjectPlane(po_ptrs[i], false, polyobjsector->floorheight, PF_Occlude,
										polyobjsector->lightlevel, levelflats[polyobjsector->floorpic].lumpnum,
										polyobjsector, 255, NULL);
			}
		}

		if (polyobjsector->ceilingheight >= gr_frontsector->floorheight
			&& polyobjsector->ceilingheight <= gr_frontsector->ceilingheight
			&& (viewz > polyobjsector->ceilingheight))
		{
			if (po_ptrs[i]->translucency > 0)
			{
				FSurfaceInfo Surf;
				FBITFIELD blendmode;
				memset(&Surf, 0x00, sizeof(Surf));
				blendmode = HWR_TranstableToAlpha(po_ptrs[i]->translucency, &Surf);
				HWR_AddTransparentPolyobjectFloor(levelflats[polyobjsector->ceilingpic].lumpnum, po_ptrs[i], true, polyobjsector->ceilingheight,
				                                  polyobjsector->lightlevel, Surf.PolyColor.s.alpha, polyobjsector, blendmode, NULL);
			}
			else
			{
				HWR_GetFlat(levelflats[polyobjsector->ceilingpic].lumpnum, R_NoEncore(polyobjsector, true));
				HWR_RenderPolyObjectPlane(po_ptrs[i], true, polyobjsector->ceilingheight, PF_Occlude,
				                          polyobjsector->lightlevel, levelflats[polyobjsector->floorpic].lumpnum,
				                          polyobjsector, 255, NULL);
			}
		}
	}
}
#endif
#endif

// -----------------+
// HWR_Subsector    : Determine floor/ceiling planes.
//                  : Add sprites of things in sector.
//                  : Draw one or more line segments.
// -----------------+
void HWR_Subsector(size_t num)
{
	INT16 count;
	seg_t *line;
	subsector_t *sub;
	static sector_t tempsec; //SoM: 4/7/2000
	INT32 floorlightlevel;
	INT32 ceilinglightlevel;
	INT32 locFloorHeight, locCeilingHeight;
	INT32 cullFloorHeight, cullCeilingHeight;
	INT32 light = 0;
	extracolormap_t *floorcolormap;
	extracolormap_t *ceilingcolormap;

#ifdef PARANOIA //no risk while developing, enough debugging nights!
	if (num >= addsubsector)
		I_Error("HWR_Subsector: ss %s with numss = %s, addss = %s\n",
			sizeu1(num), sizeu2(numsubsectors), sizeu3(addsubsector));
#endif

	if (num < numsubsectors)
	{
		// subsector
		sub = &subsectors[num];
		// sector
		gr_frontsector = sub->sector;
		// how many linedefs
		count = sub->numlines;
		// first line seg
		line = &segs[sub->firstline];
	}
	else
	{
		// there are no segs but only planes
		sub = &subsectors[0];
		gr_frontsector = sub->sector;
		count = 0;
		line = NULL;
	}

	//SoM: 4/7/2000: Test to make Boom water work in Hardware mode.
	gr_frontsector = R_FakeFlat(gr_frontsector, &tempsec, &floorlightlevel,
								&ceilinglightlevel, false);
	//FIXME: Use floorlightlevel and ceilinglightlevel insted of lightlevel.

	floorcolormap = ceilingcolormap = gr_frontsector->extra_colormap;

// ----- for special tricks with HW renderer -----
	if (gr_frontsector->pseudoSector)
	{
		cullFloorHeight = locFloorHeight = gr_frontsector->virtualFloorheight;
		cullCeilingHeight = locCeilingHeight = gr_frontsector->virtualCeilingheight;
	}
	else if (gr_frontsector->virtualFloor)
	{
		///@TODO Is this whole virtualFloor mess even useful? I don't think it even triggers ever.
		cullFloorHeight = locFloorHeight = gr_frontsector->virtualFloorheight;
		if (gr_frontsector->virtualCeiling)
			cullCeilingHeight = locCeilingHeight = gr_frontsector->virtualCeilingheight;
		else
			cullCeilingHeight = locCeilingHeight = gr_frontsector->ceilingheight;
	}
	else if (gr_frontsector->virtualCeiling)
	{
		cullCeilingHeight = locCeilingHeight = gr_frontsector->virtualCeilingheight;
		cullFloorHeight   = locFloorHeight   = gr_frontsector->floorheight;
	}
	else
	{
		cullFloorHeight   = locFloorHeight   = gr_frontsector->floorheight;
		cullCeilingHeight = locCeilingHeight = gr_frontsector->ceilingheight;

#ifdef ESLOPE
		if (gr_frontsector->f_slope)
		{
			cullFloorHeight = P_GetZAt(gr_frontsector->f_slope, viewx, viewy);
			locFloorHeight = P_GetZAt(gr_frontsector->f_slope, gr_frontsector->soundorg.x, gr_frontsector->soundorg.y);
		}

		if (gr_frontsector->c_slope)
		{
			cullCeilingHeight = P_GetZAt(gr_frontsector->c_slope, viewx, viewy);
			locCeilingHeight = P_GetZAt(gr_frontsector->c_slope, gr_frontsector->soundorg.x, gr_frontsector->soundorg.y);
		}
#endif
	}
// ----- end special tricks -----

	if (gr_frontsector->ffloors)
	{
		if (gr_frontsector->moved)
		{
			gr_frontsector->numlights = sub->sector->numlights = 0;
			R_Prep3DFloors(gr_frontsector);
			sub->sector->lightlist = gr_frontsector->lightlist;
			sub->sector->numlights = gr_frontsector->numlights;
			sub->sector->moved = gr_frontsector->moved = false;
		}

		light = R_GetPlaneLight(gr_frontsector, locFloorHeight, false);
		if (gr_frontsector->floorlightsec == -1)
			floorlightlevel = *gr_frontsector->lightlist[light].lightlevel;
		floorcolormap = gr_frontsector->lightlist[light].extra_colormap;

		light = R_GetPlaneLight(gr_frontsector, locCeilingHeight, false);
		if (gr_frontsector->ceilinglightsec == -1)
			ceilinglightlevel = *gr_frontsector->lightlist[light].lightlevel;
		ceilingcolormap = gr_frontsector->lightlist[light].extra_colormap;
	}

	sub->sector->extra_colormap = gr_frontsector->extra_colormap;

	// render floor ?
	// yeah, easy backface cull! :)
	if (cullFloorHeight < viewz)
	{
		if (gr_frontsector->floorpic != skyflatnum)
		{
			if (sub->validcount != validcount)
			{
				HWR_GetFlat(levelflats[gr_frontsector->floorpic].lumpnum, R_NoEncore(gr_frontsector, false));
				HWR_RenderPlane(&extrasubsectors[num], false,
					// Hack to make things continue to work around slopes.
					locFloorHeight == cullFloorHeight ? locFloorHeight : gr_frontsector->floorheight,
					// We now return you to your regularly scheduled rendering.
					PF_Occlude, floorlightlevel, levelflats[gr_frontsector->floorpic].lumpnum, NULL, 255, floorcolormap);
			}
		}
	}

	if (cullCeilingHeight > viewz)
	{
		if (gr_frontsector->ceilingpic != skyflatnum)
		{
			if (sub->validcount != validcount)
			{
				HWR_GetFlat(levelflats[gr_frontsector->ceilingpic].lumpnum, R_NoEncore(gr_frontsector, true));
				HWR_RenderPlane(&extrasubsectors[num], true,
					// Hack to make things continue to work around slopes.
					locCeilingHeight == cullCeilingHeight ? locCeilingHeight : gr_frontsector->ceilingheight,
					// We now return you to your regularly scheduled rendering.
					PF_Occlude, ceilinglightlevel, levelflats[gr_frontsector->ceilingpic].lumpnum,NULL, 255, ceilingcolormap);
			}
		}
	}

	if (gr_frontsector->ffloors)
	{
		/// \todo fix light, xoffs, yoffs, extracolormap ?
		ffloor_t * rover;
		for (rover = gr_frontsector->ffloors;
			rover; rover = rover->next)
		{
			fixed_t cullHeight, centerHeight;

            // bottom plane
#ifdef ESLOPE
			if (*rover->b_slope)
			{
				cullHeight = P_GetZAt(*rover->b_slope, viewx, viewy);
				centerHeight = P_GetZAt(*rover->b_slope, gr_frontsector->soundorg.x, gr_frontsector->soundorg.y);
			}
			else
#endif
		    cullHeight = centerHeight = *rover->bottomheight;

			if (!(rover->flags & FF_EXISTS) || !(rover->flags & FF_RENDERPLANES))
				continue;
			if (sub->validcount == validcount)
				continue;

			if (centerHeight <= locCeilingHeight &&
			    centerHeight >= locFloorHeight &&
			    ((viewz < cullHeight && !(rover->flags & FF_INVERTPLANES)) ||
			     (viewz > cullHeight && (rover->flags & FF_BOTHPLANES || rover->flags & FF_INVERTPLANES))))
			{
				if (rover->flags & FF_FOG)
				{
					UINT8 alpha;

					light = R_GetPlaneLight(gr_frontsector, centerHeight, viewz < cullHeight ? true : false);

					alpha = HWR_FogBlockAlpha(*gr_frontsector->lightlist[light].lightlevel, rover->master->frontsector->extra_colormap);

					HWR_AddTransparentFloor(0,
					                       &extrasubsectors[num],
										   false,
					                       *rover->bottomheight,
					                       *gr_frontsector->lightlist[light].lightlevel,
					                       alpha, rover->master->frontsector, PF_Fog|PF_NoTexture,
										   true, rover->master->frontsector->extra_colormap);
				}
				else if (rover->flags & FF_TRANSLUCENT && rover->alpha < 256) // SoM: Flags are more efficient
				{
					light = R_GetPlaneLight(gr_frontsector, centerHeight, viewz < cullHeight ? true : false);
					HWR_AddTransparentFloor(levelflats[*rover->bottompic].lumpnum,
					                       &extrasubsectors[num],
										   false,
					                       *rover->bottomheight,
					                       *gr_frontsector->lightlist[light].lightlevel,
					                       rover->alpha-1 > 255 ? 255 : rover->alpha-1, rover->master->frontsector, (rover->flags & FF_RIPPLE ? PF_Ripple : 0)|PF_Translucent,
					                       false, gr_frontsector->lightlist[light].extra_colormap);
				}
				else
				{
					HWR_GetFlat(levelflats[*rover->bottompic].lumpnum, R_NoEncore(gr_frontsector, false));
					light = R_GetPlaneLight(gr_frontsector, centerHeight, viewz < cullHeight ? true : false);
					HWR_RenderPlane(&extrasubsectors[num], false, *rover->bottomheight, (rover->flags & FF_RIPPLE ? PF_Ripple : 0)|PF_Occlude, *gr_frontsector->lightlist[light].lightlevel, levelflats[*rover->bottompic].lumpnum,
					                rover->master->frontsector, 255, gr_frontsector->lightlist[light].extra_colormap);
				}
			}

			// top plane
#ifdef ESLOPE
			if (*rover->t_slope)
			{
				cullHeight = P_GetZAt(*rover->t_slope, viewx, viewy);
				centerHeight = P_GetZAt(*rover->t_slope, gr_frontsector->soundorg.x, gr_frontsector->soundorg.y);
			}
			else
#endif
		    cullHeight = centerHeight = *rover->topheight;

			if (centerHeight >= locFloorHeight &&
			    centerHeight <= locCeilingHeight &&
			    ((viewz > cullHeight && !(rover->flags & FF_INVERTPLANES)) ||
			     (viewz < cullHeight && (rover->flags & FF_BOTHPLANES || rover->flags & FF_INVERTPLANES))))
			{
				if (rover->flags & FF_FOG)
				{
					UINT8 alpha;

					light = R_GetPlaneLight(gr_frontsector, centerHeight, viewz < cullHeight ? true : false);

					alpha = HWR_FogBlockAlpha(*gr_frontsector->lightlist[light].lightlevel, rover->master->frontsector->extra_colormap);

					HWR_AddTransparentFloor(0,
					                       &extrasubsectors[num],
										   true,
					                       *rover->topheight,
					                       *gr_frontsector->lightlist[light].lightlevel,
					                       alpha, rover->master->frontsector, PF_Fog|PF_NoTexture,
										   true, rover->master->frontsector->extra_colormap);
				}
				else if (rover->flags & FF_TRANSLUCENT && rover->alpha < 256)
				{
					light = R_GetPlaneLight(gr_frontsector, centerHeight, viewz < cullHeight ? true : false);
					HWR_AddTransparentFloor(levelflats[*rover->toppic].lumpnum,
					                        &extrasubsectors[num],
											true,
					                        *rover->topheight,
					                        *gr_frontsector->lightlist[light].lightlevel,
					                        rover->alpha-1 > 255 ? 255 : rover->alpha-1, rover->master->frontsector, (rover->flags & FF_RIPPLE ? PF_Ripple : 0)|PF_Translucent,
					                        false, gr_frontsector->lightlist[light].extra_colormap);
				}
				else
				{
					HWR_GetFlat(levelflats[*rover->toppic].lumpnum, R_NoEncore(gr_frontsector, true));
					light = R_GetPlaneLight(gr_frontsector, centerHeight, viewz < cullHeight ? true : false);
					HWR_RenderPlane(&extrasubsectors[num], true, *rover->topheight, (rover->flags & FF_RIPPLE ? PF_Ripple : 0)|PF_Occlude, *gr_frontsector->lightlist[light].lightlevel, levelflats[*rover->toppic].lumpnum,
					                  rover->master->frontsector, 255, gr_frontsector->lightlist[light].extra_colormap);
				}
			}
		}
	}

#ifdef POLYOBJECTS
	// Draw all the polyobjects in this subsector
	if (sub->polyList)
	{
		polyobj_t *po = sub->polyList;

		numpolys = 0;

		// Count all the polyobjects, reset the list, and recount them
		while (po)
		{
			++numpolys;
			po = (polyobj_t *)(po->link.next);
		}

		// Sort polyobjects
		R_SortPolyObjects(sub);

		// Draw polyobject lines.
		HWR_AddPolyObjectSegs();

#ifdef POLYOBJECTS_PLANES
		if (sub->validcount != validcount) // This validcount situation seems to let us know that the floors have already been drawn.
		{
			// Draw polyobject planes
			HWR_AddPolyObjectPlanes();
		}
#endif
	}
#endif

// Hurder ici se passe les choses INT32�essantes!
// on vient de tracer le sol et le plafond
// on trace �pr�ent d'abord les sprites et ensuite les murs
// hurdler: faux: on ajoute seulement les sprites, le murs sont trac� d'abord
	if (line)
	{
		// draw sprites first, coz they are clipped to the solidsegs of
		// subsectors more 'in front'
		HWR_AddSprites(gr_frontsector);

		//Hurdler: at this point validcount must be the same, but is not because
		//         gr_frontsector doesn't point anymore to sub->sector due to
		//         the call gr_frontsector = R_FakeFlat(...)
		//         if it's not done, the sprite is drawn more than once,
		//         what looks really bad with translucency or dynamic light,
		//         without talking about the overdraw of course.
		sub->sector->validcount = validcount;/// \todo fix that in a better way

		while (count--)
		{
#ifdef POLYOBJECTS
				if (!line->polyseg) // ignore segs that belong to polyobjects
#endif
				HWR_AddLine(line);
				line++;
		}
	}

	sub->validcount = validcount;
}

//
// Renders all subsectors below a given node,
//  traversing subtree recursively.
// Just call with BSP root.

void HWR_RenderBSPNode(INT32 bspnum)
{
	node_t *bsp = &nodes[bspnum];

	// Decide which side the view point is on
	INT32 side;

	// Found a subsector?
	if (bspnum & NF_SUBSECTOR)
	{
		if (bspnum != -1)
			HWR_Subsector(bspnum&(~NF_SUBSECTOR));
		return;
	}

	// Decide which side the view point is on.
	side = R_PointOnSide(viewx, viewy, bsp);

	// Recursively divide front space.
	HWR_RenderBSPNode(bsp->children[side]);

	// Possibly divide back space.
	if (HWR_CheckBBox(bsp->bbox[side^1]))
		HWR_RenderBSPNode(bsp->children[side^1]);
}

// ==========================================================================
// gr_things.c
// ==========================================================================

// sprites are drawn after all wall and planes are rendered, so that
// sprite translucency effects apply on the rendered view (instead of the background sky!!)

static UINT32 gr_visspritecount;
static gr_vissprite_t *gr_visspritechunks[MAXVISSPRITES >> VISSPRITECHUNKBITS] = {NULL};

// --------------------------------------------------------------------------
// HWR_ClearSprites
// Called at frame start.
// --------------------------------------------------------------------------
static void HWR_ClearSprites(void)
{
	gr_visspritecount = 0;
}

// --------------------------------------------------------------------------
// HWR_NewVisSprite
// --------------------------------------------------------------------------
static gr_vissprite_t gr_overflowsprite;

static gr_vissprite_t *HWR_GetVisSprite(UINT32 num)
{
		UINT32 chunk = num >> VISSPRITECHUNKBITS;

		// Allocate chunk if necessary
		if (!gr_visspritechunks[chunk])
			Z_Malloc(sizeof(gr_vissprite_t) * VISSPRITESPERCHUNK, PU_LEVEL, &gr_visspritechunks[chunk]);

		return gr_visspritechunks[chunk] + (num & VISSPRITEINDEXMASK);
}

static gr_vissprite_t *HWR_NewVisSprite(void)
{
	if (gr_visspritecount == MAXVISSPRITES)
		return &gr_overflowsprite;

	return HWR_GetVisSprite(gr_visspritecount++);
}

// Finds a floor through which light does not pass.
static fixed_t HWR_OpaqueFloorAtPos(fixed_t x, fixed_t y, fixed_t z, fixed_t height)
{
	const sector_t *sec = R_PointInSubsector(x, y)->sector;
	fixed_t floorz = sec->floorheight;

	if (sec->ffloors)
	{
		ffloor_t *rover;
		fixed_t delta1, delta2;
		const fixed_t thingtop = z + height;

		for (rover = sec->ffloors; rover; rover = rover->next)
		{
			if (!(rover->flags & FF_EXISTS)
			|| !(rover->flags & FF_RENDERPLANES)
			|| rover->flags & FF_TRANSLUCENT
			|| rover->flags & FF_FOG
			|| rover->flags & FF_INVERTPLANES)
				continue;

			delta1 = z - (*rover->bottomheight + ((*rover->topheight - *rover->bottomheight)/2));
			delta2 = thingtop - (*rover->bottomheight + ((*rover->topheight - *rover->bottomheight)/2));
			if (*rover->topheight > floorz && abs(delta1) < abs(delta2))
				floorz = *rover->topheight;
		}
	}

	return floorz;
}

//
// HWR_DoCulling
// Hardware version of R_DoCulling
// (see r_main.c)
static boolean HWR_DoCulling(line_t *cullheight, line_t *viewcullheight, float vz, float bottomh, float toph)
{
	float cullplane;

	if (!cullheight)
		return false;

	cullplane = FIXED_TO_FLOAT(cullheight->frontsector->floorheight);
	if (cullheight->flags & ML_NOCLIMB) // Group culling
	{
		if (!viewcullheight)
			return false;

		// Make sure this is part of the same group
		if (viewcullheight->frontsector == cullheight->frontsector)
		{
			// OK, we can cull
			if (vz > cullplane && toph < cullplane) // Cull if below plane
				return true;

			if (bottomh > cullplane && vz <= cullplane) // Cull if above plane
				return true;
		}
	}
	else // Quick culling
	{
		if (vz > cullplane && toph < cullplane) // Cull if below plane
			return true;

		if (bottomh > cullplane && vz <= cullplane) // Cull if above plane
			return true;
	}

	return false;
}

static void HWR_DrawSpriteShadow(gr_vissprite_t *spr, GLPatch_t *gpatch, float this_scale)
{
	FOutVector swallVerts[4];
	FSurfaceInfo sSurf;
	fixed_t floorheight, mobjfloor;
	float offset = 0;

	mobjfloor = HWR_OpaqueFloorAtPos(
		spr->mobj->x, spr->mobj->y,
		spr->mobj->z, spr->mobj->height);
	if (cv_shadowoffs.value)
	{
		angle_t shadowdir;

		// Set direction
		if (splitscreen && stplyr == &players[displayplayers[1]])
			shadowdir = localangle[1] + FixedAngle(cv_cam2_rotate.value);
		else if (splitscreen > 1 && stplyr == &players[displayplayers[2]])
			shadowdir = localangle[2] + FixedAngle(cv_cam3_rotate.value);
		else if (splitscreen > 2 && stplyr == &players[displayplayers[3]])
			shadowdir = localangle[3] + FixedAngle(cv_cam4_rotate.value);
		else
			shadowdir = localangle[0] + FixedAngle(cv_cam_rotate.value);

		// Find floorheight
		floorheight = HWR_OpaqueFloorAtPos(
			spr->mobj->x + P_ReturnThrustX(spr->mobj, shadowdir, spr->mobj->z - mobjfloor),
			spr->mobj->y + P_ReturnThrustY(spr->mobj, shadowdir, spr->mobj->z - mobjfloor),
			spr->mobj->z, spr->mobj->height);

		// The shadow is falling ABOVE it's mobj?
		// Don't draw it, then!
		if (spr->mobj->z < floorheight)
			return;
		else
		{
			fixed_t floorz;
			floorz = HWR_OpaqueFloorAtPos(
				spr->mobj->x + P_ReturnThrustX(spr->mobj, shadowdir, spr->mobj->z - floorheight),
				spr->mobj->y + P_ReturnThrustY(spr->mobj, shadowdir, spr->mobj->z - floorheight),
				spr->mobj->z, spr->mobj->height);
			// The shadow would be falling on a wall? Don't draw it, then.
			// Would draw midair otherwise.
			if (floorz < floorheight)
				return;
		}

		floorheight = FixedInt(spr->mobj->z - floorheight);

		offset = floorheight;
	}
	else
		floorheight = FixedInt(spr->mobj->z - mobjfloor);

	// create the sprite billboard
	//
	//  3--2
	//  | /|
	//  |/ |
	//  0--1

	// x1/x2 were already scaled in HWR_ProjectSprite
	// First match the normal sprite
	swallVerts[0].x = swallVerts[3].x = spr->x1;
	swallVerts[2].x = swallVerts[1].x = spr->x2;
	swallVerts[0].z = swallVerts[3].z = spr->z1;
	swallVerts[2].z = swallVerts[1].z = spr->z2;

	if (spr->mobj && fabsf(this_scale - 1.0f) > 1.0E-36f)
	{
		// Always a pixel above the floor, perfectly flat.
		swallVerts[0].y = swallVerts[1].y = swallVerts[2].y = swallVerts[3].y = spr->ty - gpatch->topoffset * this_scale - (floorheight+3);

		// Now transform the TOP vertices along the floor in the direction of the camera
		swallVerts[3].x = spr->x1 + ((gpatch->height * this_scale) + offset) * gr_viewcos;
		swallVerts[2].x = spr->x2 + ((gpatch->height * this_scale) + offset) * gr_viewcos;
		swallVerts[3].z = spr->z1 + ((gpatch->height * this_scale) + offset) * gr_viewsin;
		swallVerts[2].z = spr->z2 + ((gpatch->height * this_scale) + offset) * gr_viewsin;
	}
	else
	{
		// Always a pixel above the floor, perfectly flat.
		swallVerts[0].y = swallVerts[1].y = swallVerts[2].y = swallVerts[3].y = spr->ty - gpatch->topoffset - (floorheight+3);

		// Now transform the TOP vertices along the floor in the direction of the camera
		swallVerts[3].x = spr->x1 + (gpatch->height + offset) * gr_viewcos;
		swallVerts[2].x = spr->x2 + (gpatch->height + offset) * gr_viewcos;
		swallVerts[3].z = spr->z1 + (gpatch->height + offset) * gr_viewsin;
		swallVerts[2].z = spr->z2 + (gpatch->height + offset) * gr_viewsin;
	}

	// We also need to move the bottom ones away when shadowoffs is on
	if (cv_shadowoffs.value)
	{
		swallVerts[0].x = spr->x1 + offset * gr_viewcos;
		swallVerts[1].x = spr->x2 + offset * gr_viewcos;
		swallVerts[0].z = spr->z1 + offset * gr_viewsin;
		swallVerts[1].z = spr->z2 + offset * gr_viewsin;
	}

	if (spr->flip)
	{
		swallVerts[0].s = swallVerts[3].s = gpatch->max_s;
		swallVerts[2].s = swallVerts[1].s = 0;
	}
	else
	{
		swallVerts[0].s = swallVerts[3].s = 0;
		swallVerts[2].s = swallVerts[1].s = gpatch->max_s;
	}

	// flip the texture coords (look familiar?)
	if (spr->vflip)
	{
		swallVerts[3].t = swallVerts[2].t = gpatch->max_t;
		swallVerts[0].t = swallVerts[1].t = 0;
	}
	else
	{
		swallVerts[3].t = swallVerts[2].t = 0;
		swallVerts[0].t = swallVerts[1].t = gpatch->max_t;
	}

	sSurf.PolyColor.s.red = 0x00;
	sSurf.PolyColor.s.blue = 0x00;
	sSurf.PolyColor.s.green = 0x00;

	// shadow is always half as translucent as the sprite itself
	if (!cv_translucency.value) // use default translucency (main sprite won't have any translucency)
		sSurf.PolyColor.s.alpha = 0x80; // default
	else if (spr->mobj->flags2 & MF2_SHADOW)
		sSurf.PolyColor.s.alpha = 0x20;
	else if (spr->mobj->frame & FF_TRANSMASK)
	{
		HWR_TranstableToAlpha((spr->mobj->frame & FF_TRANSMASK)>>FF_TRANSSHIFT, &sSurf);
		sSurf.PolyColor.s.alpha /= 2; //cut alpha in half!
	}
	else
		sSurf.PolyColor.s.alpha = 0x80; // default

	if (sSurf.PolyColor.s.alpha > floorheight/4)
	{
		sSurf.PolyColor.s.alpha = (UINT8)(sSurf.PolyColor.s.alpha - floorheight/4);
		HWD.pfnSetShader(1);	// floor shader
		HWD.pfnDrawPolygon(&sSurf, swallVerts, 4, PF_Translucent|PF_Modulated);
	}
}

// This is expecting a pointer to an array containing 4 wallVerts for a sprite
static void HWR_RotateSpritePolyToAim(gr_vissprite_t *spr, FOutVector *wallVerts)
{
	if (cv_grspritebillboarding.value && spr && spr->mobj && !(spr->mobj->frame & FF_PAPERSPRITE) && wallVerts)
	{
		float basey = FIXED_TO_FLOAT(spr->mobj->z);
		float lowy = wallVerts[0].y;
		if (P_MobjFlip(spr->mobj) == -1)
		{
			basey = FIXED_TO_FLOAT(spr->mobj->z + spr->mobj->height);
		}
		// Rotate sprites to fully billboard with the camera
		// X, Y, AND Z need to be manipulated for the polys to rotate around the
		// origin, because of how the origin setting works I believe that should
		// be mobj->z or mobj->z + mobj->height
		wallVerts[2].y = wallVerts[3].y = (spr->ty - basey) * gr_viewludsin + basey;
		wallVerts[0].y = wallVerts[1].y = (lowy - basey) * gr_viewludsin + basey;
		// translate back to be around 0 before translating back
		wallVerts[3].x += ((spr->ty - basey) * gr_viewludcos) * gr_viewcos;
		wallVerts[2].x += ((spr->ty - basey) * gr_viewludcos) * gr_viewcos;

		wallVerts[0].x += ((lowy - basey) * gr_viewludcos) * gr_viewcos;
		wallVerts[1].x += ((lowy - basey) * gr_viewludcos) * gr_viewcos;

		wallVerts[3].z += ((spr->ty - basey) * gr_viewludcos) * gr_viewsin;
		wallVerts[2].z += ((spr->ty - basey) * gr_viewludcos) * gr_viewsin;

		wallVerts[0].z += ((lowy - basey) * gr_viewludcos) * gr_viewsin;
		wallVerts[1].z += ((lowy - basey) * gr_viewludcos) * gr_viewsin;
	}
}

static void HWR_SplitSprite(gr_vissprite_t *spr)
{
	float this_scale = 1.0f;
	FOutVector wallVerts[4];
	FOutVector baseWallVerts[4]; // This is what the verts should end up as
	GLPatch_t *gpatch;
	FSurfaceInfo Surf;
	const boolean hires = (spr->mobj && spr->mobj->skin && ((skin_t *)spr->mobj->skin)->flags & SF_HIRES);
	extracolormap_t *colormap;
	FUINT lightlevel;
	FBITFIELD blend = 0;
	UINT8 alpha;

	INT32 i;
	float realtop, realbot, top, bot;
	float towtop, towbot, towmult;
	float bheight;
	float realheight, heightmult;
	const sector_t *sector = spr->mobj->subsector->sector;
	const lightlist_t *list = sector->lightlist;
#ifdef ESLOPE
	float endrealtop, endrealbot, endtop, endbot;
	float endbheight;
	float endrealheight;
	fixed_t temp;
	fixed_t v1x, v1y, v2x, v2y;
#endif

	this_scale = FIXED_TO_FLOAT(spr->mobj->scale);

	if (hires)
		this_scale = this_scale * FIXED_TO_FLOAT(((skin_t *)spr->mobj->skin)->highresscale);

	gpatch = W_CachePatchNum(spr->patchlumpnum, PU_CACHE);

	// cache the patch in the graphics card memory
	//12/12/99: Hurdler: same comment as above (for md2)
	//Hurdler: 25/04/2000: now support colormap in hardware mode
	HWR_GetMappedPatch(gpatch, spr->colormap);

	// Draw shadow BEFORE sprite
	if (cv_shadow.value // Shadows enabled
		&& (spr->mobj->flags & (MF_SCENERY|MF_SPAWNCEILING|MF_NOGRAVITY)) != (MF_SCENERY|MF_SPAWNCEILING|MF_NOGRAVITY) // Ceiling scenery have no shadow.
		&& !(spr->mobj->flags2 & MF2_DEBRIS) // Debris have no corona or shadow.
		&& (spr->mobj->z >= spr->mobj->floorz)) // Without this, your shadow shows on the floor, even after you die and fall through the ground.
	{
		////////////////////
		// SHADOW SPRITE! //
		////////////////////
		HWR_DrawSpriteShadow(spr, gpatch, this_scale);
	}

	baseWallVerts[0].x = baseWallVerts[3].x = spr->x1;
	baseWallVerts[2].x = baseWallVerts[1].x = spr->x2;
	baseWallVerts[0].z = baseWallVerts[3].z = spr->z1;
	baseWallVerts[1].z = baseWallVerts[2].z = spr->z2;

	baseWallVerts[2].y = baseWallVerts[3].y = spr->ty;
	if (spr->mobj && fabsf(this_scale - 1.0f) > 1.0E-36f)
		baseWallVerts[0].y = baseWallVerts[1].y = spr->ty - gpatch->height * this_scale;
	else
		baseWallVerts[0].y = baseWallVerts[1].y = spr->ty - gpatch->height;

	v1x = FLOAT_TO_FIXED(spr->x1);
	v1y = FLOAT_TO_FIXED(spr->z1);
	v2x = FLOAT_TO_FIXED(spr->x2);
	v2y = FLOAT_TO_FIXED(spr->z2);

	if (spr->flip)
	{
		baseWallVerts[0].s = baseWallVerts[3].s = gpatch->max_s;
		baseWallVerts[2].s = baseWallVerts[1].s = 0;
	}
	else
	{
		baseWallVerts[0].s = baseWallVerts[3].s = 0;
		baseWallVerts[2].s = baseWallVerts[1].s = gpatch->max_s;
	}

	// flip the texture coords (look familiar?)
	if (spr->vflip)
	{
		baseWallVerts[3].t = baseWallVerts[2].t = gpatch->max_t;
		baseWallVerts[0].t = baseWallVerts[1].t = 0;
	}
	else
	{
		baseWallVerts[3].t = baseWallVerts[2].t = 0;
		baseWallVerts[0].t = baseWallVerts[1].t = gpatch->max_t;
	}

	// Let dispoffset work first since this adjust each vertex
	HWR_RotateSpritePolyToAim(spr, baseWallVerts);

	// push it toward the camera to mitigate floor-clipping sprites
	{
		float sprdist = sqrtf((spr->x1 - gr_viewx)*(spr->x1 - gr_viewx) + (spr->z1 - gr_viewy)*(spr->z1 - gr_viewy) + (spr->ty - gr_viewz)*(spr->ty - gr_viewz));
		float distfact = ((2.0f*spr->dispoffset) + 20.0f) / sprdist;
		for (i = 0; i < 4; i++)
		{
			baseWallVerts[i].x += (gr_viewx - baseWallVerts[i].x)*distfact;
			baseWallVerts[i].z += (gr_viewy - baseWallVerts[i].z)*distfact;
			baseWallVerts[i].y += (gr_viewz - baseWallVerts[i].y)*distfact;
		}
	}

	realtop = top = baseWallVerts[3].y;
	realbot = bot = baseWallVerts[0].y;
	towtop = baseWallVerts[3].t;
	towbot = baseWallVerts[0].t;
	towmult = (towbot - towtop) / (top - bot);

#ifdef ESLOPE
	endrealtop = endtop = baseWallVerts[2].y;
	endrealbot = endbot = baseWallVerts[1].y;
#endif

	// copy the contents of baseWallVerts into the drawn wallVerts array
	// baseWallVerts is used to know the final shape to easily get the vertex
	// co-ordinates
	memcpy(wallVerts, baseWallVerts, sizeof(baseWallVerts));

	if (!cv_translucency.value) // translucency disabled
	{
		Surf.PolyColor.s.alpha = 0xFF;
		blend = PF_Translucent|PF_Occlude;
	}
	else if (spr->mobj->flags2 & MF2_SHADOW)
	{
		Surf.PolyColor.s.alpha = 0x40;
		blend = PF_Translucent;
	}
	else if (spr->mobj->frame & FF_TRANSMASK)
		blend = HWR_TranstableToAlpha((spr->mobj->frame & FF_TRANSMASK)>>FF_TRANSSHIFT, &Surf);
	else
	{
		// BP: i agree that is little better in environement but it don't
		//     work properly under glide nor with fogcolor to ffffff :(
		// Hurdler: PF_Environement would be cool, but we need to fix
		//          the issue with the fog before
		Surf.PolyColor.s.alpha = 0xFF;
		blend = PF_Translucent|PF_Occlude;
	}

	alpha = Surf.PolyColor.s.alpha;

	// Start with the lightlevel and colormap from the top of the sprite
	lightlevel = *list[sector->numlights - 1].lightlevel;
	colormap = list[sector->numlights - 1].extra_colormap;
	i = 0;
	temp = FLOAT_TO_FIXED(realtop);

	if (spr->mobj->frame & FF_FULLBRIGHT)
		lightlevel = 255;

#ifdef ESLOPE
	for (i = 1; i < sector->numlights; i++)
	{
		fixed_t h = sector->lightlist[i].slope ? P_GetZAt(sector->lightlist[i].slope, spr->mobj->x, spr->mobj->y)
					: sector->lightlist[i].height;
		if (h <= temp)
		{
			if (!(spr->mobj->frame & FF_FULLBRIGHT))
				lightlevel = *list[i-1].lightlevel;
			colormap = list[i-1].extra_colormap;
			break;
		}
	}
#else
	i = R_GetPlaneLight(sector, temp, false);
	if (!(spr->mobj->frame & FF_FULLBRIGHT))
		lightlevel = *list[i].lightlevel;
	colormap = list[i].extra_colormap;
#endif

	for (i = 0; i < sector->numlights; i++)
	{
#ifdef ESLOPE
		if (endtop < endrealbot)
#endif
		if (top < realbot)
			return;

		// even if we aren't changing colormap or lightlevel, we still need to continue drawing down the sprite
		if (!(list[i].flags & FF_NOSHADE) && (list[i].flags & FF_CUTSPRITES))
		{
			if (!(spr->mobj->frame & FF_FULLBRIGHT))
				lightlevel = *list[i].lightlevel;
			colormap = list[i].extra_colormap;
		}

#ifdef ESLOPE
		if (i + 1 < sector->numlights)
		{
			if (list[i+1].slope)
			{
				temp = P_GetZAt(list[i+1].slope, v1x, v1y);
				bheight = FIXED_TO_FLOAT(temp);
				temp = P_GetZAt(list[i+1].slope, v2x, v2y);
				endbheight = FIXED_TO_FLOAT(temp);
			}
			else
				bheight = endbheight = FIXED_TO_FLOAT(list[i+1].height);
		}
		else
		{
			bheight = realbot;
			endbheight = endrealbot;
		}
#else
		if (i + 1 < sector->numlights)
		{
			bheight = FIXED_TO_FLOAT(list[i+1].height);
		}
		else
		{
			bheight = realbot;
		}
#endif

#ifdef ESLOPE
		if (endbheight >= endtop)
#endif
		if (bheight >= top)
			continue;

		bot = bheight;

		if (bot < realbot)
			bot = realbot;

#ifdef ESLOPE
		endbot = endbheight;

		if (endbot < endrealbot)
			endbot = endrealbot;
#endif

#ifdef ESLOPE
		wallVerts[3].t = towtop + ((realtop - top) * towmult);
		wallVerts[2].t = towtop + ((endrealtop - endtop) * towmult);
		wallVerts[0].t = towtop + ((realtop - bot) * towmult);
		wallVerts[1].t = towtop + ((endrealtop - endbot) * towmult);

		wallVerts[3].y = top;
		wallVerts[2].y = endtop;
		wallVerts[0].y = bot;
		wallVerts[1].y = endbot;

		// The x and y only need to be adjusted in the case that it's not a papersprite
		if (cv_grspritebillboarding.value && spr->mobj && !(spr->mobj->frame & FF_PAPERSPRITE))
		{
			// Get the x and z of the vertices so billboarding draws correctly
			realheight = realbot - realtop;
			endrealheight = endrealbot - endrealtop;
			heightmult = (realtop - top) / realheight;
			wallVerts[3].x = baseWallVerts[3].x + (baseWallVerts[3].x - baseWallVerts[0].x) * heightmult;
			wallVerts[3].z = baseWallVerts[3].z + (baseWallVerts[3].z - baseWallVerts[0].z) * heightmult;

			heightmult = (endrealtop - endtop) / endrealheight;
			wallVerts[2].x = baseWallVerts[2].x + (baseWallVerts[2].x - baseWallVerts[1].x) * heightmult;
			wallVerts[2].z = baseWallVerts[2].z + (baseWallVerts[2].z - baseWallVerts[1].z) * heightmult;

			heightmult = (realtop - bot) / realheight;
			wallVerts[0].x = baseWallVerts[3].x + (baseWallVerts[3].x - baseWallVerts[0].x) * heightmult;
			wallVerts[0].z = baseWallVerts[3].z + (baseWallVerts[3].z - baseWallVerts[0].z) * heightmult;

			heightmult = (endrealtop - endbot) / endrealheight;
			wallVerts[1].x = baseWallVerts[2].x + (baseWallVerts[2].x - baseWallVerts[1].x) * heightmult;
			wallVerts[1].z = baseWallVerts[2].z + (baseWallVerts[2].z - baseWallVerts[1].z) * heightmult;
		}
#else
		wallVerts[3].t = wallVerts[2].t = towtop + ((realtop - top) * towmult);
		wallVerts[0].t = wallVerts[1].t = towtop + ((realtop - bot) * towmult);

		wallVerts[2].y = wallVerts[3].y = top;
		wallVerts[0].y = wallVerts[1].y = bot;

		// The x and y only need to be adjusted in the case that it's not a papersprite
		if (cv_grspritebillboarding.value && spr->mobj && !(spr->mobj->frame & FF_PAPERSPRITE))
		{
			// Get the x and z of the vertices so billboarding draws correctly
			realheight = realbot - realtop;
			heightmult = (realtop - top) / realheight;
			wallVerts[3].x = baseWallVerts[3].x + (baseWallVerts[3].x - baseWallVerts[0].x) * heightmult;
			wallVerts[3].z = baseWallVerts[3].z + (baseWallVerts[3].z - baseWallVerts[0].z) * heightmult;
			wallVerts[2].x = baseWallVerts[2].x + (baseWallVerts[2].x - baseWallVerts[1].x) * heightmult;
			wallVerts[2].z = baseWallVerts[2].z + (baseWallVerts[2].z - baseWallVerts[1].z) * heightmult;

			heightmult = (realtop - bot) / realheight;
			wallVerts[0].x = baseWallVerts[3].x + (baseWallVerts[3].x - baseWallVerts[0].x) * heightmult;
			wallVerts[0].z = baseWallVerts[3].z + (baseWallVerts[3].z - baseWallVerts[0].z) * heightmult;
			wallVerts[1].x = baseWallVerts[2].x + (baseWallVerts[2].x - baseWallVerts[1].x) * heightmult;
			wallVerts[1].z = baseWallVerts[2].z + (baseWallVerts[2].z - baseWallVerts[1].z) * heightmult;
		}
#endif

		HWR_Lighting(&Surf, lightlevel, colormap);

		Surf.PolyColor.s.alpha = alpha;

		HWD.pfnSetShader(3);	// sprite shader
		HWD.pfnDrawPolygon(&Surf, wallVerts, 4, blend|PF_Modulated);

		top = bot;
#ifdef ESLOPE
		endtop = endbot;
#endif
	}

	bot = realbot;
#ifdef ESLOPE
	endbot = endrealbot;
	if (endtop <= endrealbot)
#endif
	if (top <= realbot)
		return;

	// If we're ever down here, somehow the above loop hasn't draw all the light levels of sprite
#ifdef ESLOPE
	wallVerts[3].t = towtop + ((realtop - top) * towmult);
	wallVerts[2].t = towtop + ((endrealtop - endtop) * towmult);
	wallVerts[0].t = towtop + ((realtop - bot) * towmult);
	wallVerts[1].t = towtop + ((endrealtop - endbot) * towmult);

	wallVerts[3].y = top;
	wallVerts[2].y = endtop;
	wallVerts[0].y = bot;
	wallVerts[1].y = endbot;
#else
	wallVerts[3].t = wallVerts[2].t = towtop + ((realtop - top) * towmult);
	wallVerts[0].t = wallVerts[1].t = towtop + ((realtop - bot) * towmult);

	wallVerts[2].y = wallVerts[3].y = top;
	wallVerts[0].y = wallVerts[1].y = bot;
#endif

	HWR_Lighting(&Surf, lightlevel, colormap);

	Surf.PolyColor.s.alpha = alpha;

	HWD.pfnSetShader(3);	// sprite shader
	HWD.pfnDrawPolygon(&Surf, wallVerts, 4, blend|PF_Modulated);
}

// -----------------+
// HWR_DrawSprite   : Draw flat sprites
//                  : (monsters, bonuses, weapons, lights, ...)
// Returns          :
// -----------------+
static void HWR_DrawSprite(gr_vissprite_t *spr)
{
	float this_scale = 1.0f;
	FOutVector wallVerts[4];
	GLPatch_t *gpatch; // sprite patch converted to hardware
	FSurfaceInfo Surf;
	const boolean hires = (spr->mobj && spr->mobj->skin && ((skin_t *)spr->mobj->skin)->flags & SF_HIRES);
	if (spr->mobj)
		this_scale = FIXED_TO_FLOAT(spr->mobj->scale);
	if (hires)
		this_scale = this_scale * FIXED_TO_FLOAT(((skin_t *)spr->mobj->skin)->highresscale);

	if (!spr->mobj)
		return;

	if (!spr->mobj->subsector)
		return;

	if (spr->mobj->subsector->sector->numlights)
	{
		HWR_SplitSprite(spr);
		return;
	}

	// cache sprite graphics
	//12/12/99: Hurdler:
	//          OK, I don't change anything for MD2 support because I want to be
	//          sure to do it the right way. So actually, we keep normal sprite
	//          in memory and we add the md2 model if it exists for that sprite

	gpatch = W_CachePatchNum(spr->patchlumpnum, PU_CACHE);

	// create the sprite billboard
	//
	//  3--2
	//  | /|
	//  |/ |
	//  0--1

	// these were already scaled in HWR_ProjectSprite
	wallVerts[0].x = wallVerts[3].x = spr->x1;
	wallVerts[2].x = wallVerts[1].x = spr->x2;
	wallVerts[2].y = wallVerts[3].y = spr->ty;
	if (spr->mobj && fabsf(this_scale - 1.0f) > 1.0E-36f)
		wallVerts[0].y = wallVerts[1].y = spr->ty - gpatch->height * this_scale;
	else
		wallVerts[0].y = wallVerts[1].y = spr->ty - gpatch->height;

	// make a wall polygon (with 2 triangles), using the floor/ceiling heights,
	// and the 2d map coords of start/end vertices
	wallVerts[0].z = wallVerts[3].z = spr->z1;
	wallVerts[1].z = wallVerts[2].z = spr->z2;

	if (spr->flip)
	{
		wallVerts[0].s = wallVerts[3].s = gpatch->max_s;
		wallVerts[2].s = wallVerts[1].s = 0;
	}else{
		wallVerts[0].s = wallVerts[3].s = 0;
		wallVerts[2].s = wallVerts[1].s = gpatch->max_s;
	}

	// flip the texture coords (look familiar?)
	if (spr->vflip)
	{
		wallVerts[3].t = wallVerts[2].t = gpatch->max_t;
		wallVerts[0].t = wallVerts[1].t = 0;
	}else{
		wallVerts[3].t = wallVerts[2].t = 0;
		wallVerts[0].t = wallVerts[1].t = gpatch->max_t;
	}

	// cache the patch in the graphics card memory
	//12/12/99: Hurdler: same comment as above (for md2)
	//Hurdler: 25/04/2000: now support colormap in hardware mode
	HWR_GetMappedPatch(gpatch, spr->colormap);

	// Draw shadow BEFORE sprite
	if (cv_shadow.value // Shadows enabled
		&& (spr->mobj->flags & (MF_SCENERY|MF_SPAWNCEILING|MF_NOGRAVITY)) != (MF_SCENERY|MF_SPAWNCEILING|MF_NOGRAVITY) // Ceiling scenery have no shadow.
		&& !(spr->mobj->flags2 & MF2_DEBRIS) // Debris have no corona or shadow.
		&& (spr->mobj->z >= spr->mobj->floorz)) // Without this, your shadow shows on the floor, even after you die and fall through the ground.
	{
		////////////////////
		// SHADOW SPRITE! //
		////////////////////
		HWR_DrawSpriteShadow(spr, gpatch, this_scale);
	}

	// Let dispoffset work first since this adjust each vertex
	// ...nah
	HWR_RotateSpritePolyToAim(spr, wallVerts);

	// push it toward the camera to mitigate floor-clipping sprites
	{
		float sprdist = sqrtf((spr->x1 - gr_viewx)*(spr->x1 - gr_viewx) + (spr->z1 - gr_viewy)*(spr->z1 - gr_viewy) + (spr->ty - gr_viewz)*(spr->ty - gr_viewz));
		float distfact = ((2.0f*spr->dispoffset) + 20.0f) / sprdist;
		size_t i;
		for (i = 0; i < 4; i++)
		{
			wallVerts[i].x += (gr_viewx - wallVerts[i].x)*distfact;
			wallVerts[i].z += (gr_viewy - wallVerts[i].z)*distfact;
			wallVerts[i].y += (gr_viewz - wallVerts[i].y)*distfact;
		}
	}

	// This needs to be AFTER the shadows so that the regular sprites aren't drawn completely black.
	// sprite lighting by modulating the RGB components
	/// \todo coloured

	// colormap test
	{
		sector_t *sector = spr->mobj->subsector->sector;
		UINT8 lightlevel = 255;
		extracolormap_t *colormap = sector->extra_colormap;

		if (!(spr->mobj->frame & FF_FULLBRIGHT))
			lightlevel = sector->lightlevel;

		HWR_Lighting(&Surf, lightlevel, colormap);
	}

	{
		FBITFIELD blend = 0;
		if (!cv_translucency.value) // translucency disabled
		{
			Surf.PolyColor.s.alpha = 0xFF;
			blend = PF_Translucent|PF_Occlude;
		}
		else if (spr->mobj->flags2 & MF2_SHADOW)
		{
			Surf.PolyColor.s.alpha = 0x40;
			blend = PF_Translucent;
		}
		else if (spr->mobj->frame & FF_TRANSMASK)
			blend = HWR_TranstableToAlpha((spr->mobj->frame & FF_TRANSMASK)>>FF_TRANSSHIFT, &Surf);
		else
		{
			// BP: i agree that is little better in environement but it don't
			//     work properly under glide nor with fogcolor to ffffff :(
			// Hurdler: PF_Environement would be cool, but we need to fix
			//          the issue with the fog before
			Surf.PolyColor.s.alpha = 0xFF;
			blend = PF_Translucent|PF_Occlude;
		}

		HWD.pfnSetShader(3);	// sprite shader
		HWD.pfnDrawPolygon(&Surf, wallVerts, 4, blend|PF_Modulated);
	}
}

// Sprite drawer for precipitation
static inline void HWR_DrawPrecipitationSprite(gr_vissprite_t *spr)
{
	FBITFIELD blend = 0;
	FOutVector wallVerts[4];
	GLPatch_t *gpatch; // sprite patch converted to hardware
	FSurfaceInfo Surf;

	if (!spr->mobj)
		return;

	if (!spr->mobj->subsector)
		return;

	// cache sprite graphics
	gpatch = W_CachePatchNum(spr->patchlumpnum, PU_CACHE);

	// create the sprite billboard
	//
	//  3--2
	//  | /|
	//  |/ |
	//  0--1
	wallVerts[0].x = wallVerts[3].x = spr->x1;
	wallVerts[2].x = wallVerts[1].x = spr->x2;
	wallVerts[2].y = wallVerts[3].y = spr->ty;
	wallVerts[0].y = wallVerts[1].y = spr->ty - gpatch->height;

	// make a wall polygon (with 2 triangles), using the floor/ceiling heights,
	// and the 2d map coords of start/end vertices
	wallVerts[0].z = wallVerts[3].z = spr->z1;
	wallVerts[1].z = wallVerts[2].z = spr->z2;

	// Let dispoffset work first since this adjust each vertex
	HWR_RotateSpritePolyToAim(spr, wallVerts);

	wallVerts[0].s = wallVerts[3].s = 0;
	wallVerts[2].s = wallVerts[1].s = gpatch->max_s;

	wallVerts[3].t = wallVerts[2].t = 0;
	wallVerts[0].t = wallVerts[1].t = gpatch->max_t;

	// cache the patch in the graphics card memory
	//12/12/99: Hurdler: same comment as above (for md2)
	//Hurdler: 25/04/2000: now support colormap in hardware mode
	HWR_GetMappedPatch(gpatch, spr->colormap);

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

		HWR_Lighting(&Surf, lightlevel, colormap);
	}

	if (spr->mobj->flags2 & MF2_SHADOW)
	{
		Surf.PolyColor.s.alpha = 0x40;
		blend = PF_Translucent;
	}
	else if (spr->mobj->frame & FF_TRANSMASK)
		blend = HWR_TranstableToAlpha((spr->mobj->frame & FF_TRANSMASK)>>FF_TRANSSHIFT, &Surf);
	else
	{
		// BP: i agree that is little better in environement but it don't
		//     work properly under glide nor with fogcolor to ffffff :(
		// Hurdler: PF_Environement would be cool, but we need to fix
		//          the issue with the fog before
		Surf.PolyColor.s.alpha = 0xFF;
		blend = PF_Translucent|PF_Occlude;
	}

	HWD.pfnSetShader(3);	// sprite shader
	HWD.pfnDrawPolygon(&Surf, wallVerts, 4, blend|PF_Modulated);
}

// --------------------------------------------------------------------------
// Sort vissprites by distance
// --------------------------------------------------------------------------

gr_vissprite_t* gr_vsprorder[MAXVISSPRITES];

// For more correct transparency the transparent sprites would need to be
// sorted and drawn together with transparent surfaces.
static int CompareVisSprites(const void *p1, const void *p2)
{
	gr_vissprite_t* spr1 = *(gr_vissprite_t*const*)p1;
	gr_vissprite_t* spr2 = *(gr_vissprite_t*const*)p2;
	int idiff;
	float fdiff;
	
	// make transparent sprites last
	// "boolean to int"
	
	int transparency1 = (spr1->mobj->flags2 & MF2_SHADOW) || (spr1->mobj->frame & FF_TRANSMASK);
	int transparency2 = (spr2->mobj->flags2 & MF2_SHADOW) || (spr2->mobj->frame & FF_TRANSMASK);
	idiff = transparency1 - transparency2;
	if (idiff != 0) return idiff;

	fdiff = spr2->tz - spr1->tz;// this order seems correct when checking with apitrace. Back to front.
	if (fabsf(fdiff) < 1.0E-36f)
		return spr1->dispoffset - spr2->dispoffset;// smallest dispoffset first if sprites are at (almost) same location.
	else if (fdiff > 0)
		return 1;
	else
		return -1;
}


static void HWR_SortVisSprites(void)
{
	UINT32 i;
	for (i = 0; i < gr_visspritecount; i++)
	{
		gr_vsprorder[i] = HWR_GetVisSprite(i);
	}
	qsort(gr_vsprorder, gr_visspritecount, sizeof(gr_vissprite_t*), CompareVisSprites);
}

// A drawnode is something that points to a 3D floor, 3D side, or masked
// middle texture. This is used for sorting with sprites.
typedef struct
{
	FOutVector    wallVerts[4];
	FSurfaceInfo  Surf;
	INT32         texnum;
	FBITFIELD     blend;
	INT32         drawcount;
	boolean fogwall;
	INT32 lightlevel;
	extracolormap_t *wallcolormap; // Doing the lighting in HWR_RenderWall now for correct fog after sorting
} wallinfo_t;

static wallinfo_t *wallinfo = NULL;
static size_t numwalls = 0; // a list of transparent walls to be drawn

typedef struct
{
	extrasubsector_t *xsub;
	boolean isceiling;
	fixed_t fixedheight;
	INT32 lightlevel;
	lumpnum_t lumpnum;
	INT32 alpha;
	sector_t *FOFSector;
	FBITFIELD blend;
	boolean fogplane;
	extracolormap_t *planecolormap;
	INT32 drawcount;
} planeinfo_t;

static size_t numplanes = 0; // a list of transparent floors to be drawn
static planeinfo_t *planeinfo = NULL;

typedef struct
{
	polyobj_t *polysector;
	boolean isceiling;
	fixed_t fixedheight;
	INT32 lightlevel;
	lumpnum_t lumpnum;
	INT32 alpha;
	sector_t *FOFSector;
	FBITFIELD blend;
	extracolormap_t *planecolormap;
	INT32 drawcount;
} polyplaneinfo_t;

static size_t numpolyplanes = 0; // a list of transparent poyobject floors to be drawn
static polyplaneinfo_t *polyplaneinfo = NULL;

typedef struct gr_drawnode_s
{
	planeinfo_t *plane;
	polyplaneinfo_t *polyplane;
	wallinfo_t *wall;
	gr_vissprite_t *sprite;
} gr_drawnode_t;

#define MAX_TRANSPARENTWALL 256
#define MAX_TRANSPARENTFLOOR 512

// This will likely turn into a copy of HWR_Add3DWater and replace it.
void HWR_AddTransparentFloor(lumpnum_t lumpnum, extrasubsector_t *xsub, boolean isceiling, fixed_t fixedheight, INT32 lightlevel, INT32 alpha, sector_t *FOFSector, FBITFIELD blend, boolean fogplane, extracolormap_t *planecolormap)
{
	static size_t allocedplanes = 0;

	// Force realloc if buffer has been freed
	if (!planeinfo)
		allocedplanes = 0;

	if (allocedplanes < numplanes + 1)
	{
		allocedplanes += MAX_TRANSPARENTFLOOR;
		Z_Realloc(planeinfo, allocedplanes * sizeof (*planeinfo), PU_LEVEL, &planeinfo);
	}

	planeinfo[numplanes].isceiling = isceiling;
	planeinfo[numplanes].fixedheight = fixedheight;
	planeinfo[numplanes].lightlevel = lightlevel;
	planeinfo[numplanes].lumpnum = lumpnum;
	planeinfo[numplanes].xsub = xsub;
	planeinfo[numplanes].alpha = alpha;
	planeinfo[numplanes].FOFSector = FOFSector;
	planeinfo[numplanes].blend = blend;
	planeinfo[numplanes].fogplane = fogplane;
	planeinfo[numplanes].planecolormap = planecolormap;
	planeinfo[numplanes].drawcount = drawcount++;

	numplanes++;
}

// Adding this for now until I can create extrasubsector info for polyobjects
// When that happens it'll just be done through HWR_AddTransparentFloor and HWR_RenderPlane
void HWR_AddTransparentPolyobjectFloor(lumpnum_t lumpnum, polyobj_t *polysector, boolean isceiling, fixed_t fixedheight, INT32 lightlevel, INT32 alpha, sector_t *FOFSector, FBITFIELD blend, extracolormap_t *planecolormap)
{
	static size_t allocedpolyplanes = 0;

	// Force realloc if buffer has been freed
	if (!polyplaneinfo)
		allocedpolyplanes = 0;

	if (allocedpolyplanes < numpolyplanes + 1)
	{
		allocedpolyplanes += MAX_TRANSPARENTFLOOR;
		Z_Realloc(polyplaneinfo, allocedpolyplanes * sizeof (*polyplaneinfo), PU_LEVEL, &polyplaneinfo);
	}

	polyplaneinfo[numpolyplanes].isceiling = isceiling;
	polyplaneinfo[numpolyplanes].fixedheight = fixedheight;
	polyplaneinfo[numpolyplanes].lightlevel = lightlevel;
	polyplaneinfo[numpolyplanes].lumpnum = lumpnum;
	polyplaneinfo[numpolyplanes].polysector = polysector;
	polyplaneinfo[numpolyplanes].alpha = alpha;
	polyplaneinfo[numpolyplanes].FOFSector = FOFSector;
	polyplaneinfo[numpolyplanes].blend = blend;
	polyplaneinfo[numpolyplanes].planecolormap = planecolormap;
	polyplaneinfo[numpolyplanes].drawcount = drawcount++;
	numpolyplanes++;
}

// putting sortindex and sortnode here so the comparator function can see them
gr_drawnode_t *sortnode;
size_t *sortindex;

static int CompareDrawNodes(const void *p1, const void *p2)
{
	size_t n1 = *(const size_t*)p1;
	size_t n2 = *(const size_t*)p2;
	INT32 v1 = 0;
	INT32 v2 = 0;
	INT32 diff;
	if (sortnode[n1].plane)
		v1 = sortnode[n1].plane->drawcount;
	else if (sortnode[n1].polyplane)
		v1 = sortnode[n1].polyplane->drawcount;
	else if (sortnode[n1].wall)
		v1 = sortnode[n1].wall->drawcount;
	else I_Error("n1 unknown");

	if (sortnode[n2].plane)
		v2 = sortnode[n2].plane->drawcount;
	else if (sortnode[n2].polyplane)
		v2 = sortnode[n2].polyplane->drawcount;
	else if (sortnode[n2].wall)
		v2 = sortnode[n2].wall->drawcount;
	else I_Error("n2 unknown");

	diff = v2 - v1;
	if (diff == 0) I_Error("diff is zero");
	return diff;
}

static int CompareDrawNodePlanes(const void *p1, const void *p2)
{
	size_t n1 = *(const size_t*)p1;
	size_t n2 = *(const size_t*)p2;
	if (!sortnode[n1].plane) I_Error("Uh.. This isn't a plane! (n1)");
	if (!sortnode[n2].plane) I_Error("Uh.. This isn't a plane! (n2)");
	return ABS(sortnode[n2].plane->fixedheight - viewz) - ABS(sortnode[n1].plane->fixedheight - viewz);
}

// HWR_RenderDrawNodes
// Creates, sorts and renders a list of drawnodes for the current frame.
void HWR_RenderDrawNodes(void)
{
	UINT32 i = 0, p = 0;
	size_t run_start = 0;

	// Dump EVERYTHING into a huge drawnode list. Then we'll sort it!
	// Could this be optimized into _AddTransparentWall/_AddTransparentPlane?
	// Hell yes! But sort algorithm must be modified to use a linked list.
	sortnode = Z_Calloc((sizeof(planeinfo_t)*numplanes)
									+ (sizeof(polyplaneinfo_t)*numpolyplanes)
									+ (sizeof(wallinfo_t)*numwalls)
									,PU_STATIC, NULL);
	// todo:
	// However, in reality we shouldn't be re-copying and shifting all this information
	// that is already lying around. This should all be in some sort of linked list or lists.
	sortindex = Z_Calloc(sizeof(size_t) * (numplanes + numpolyplanes + numwalls), PU_STATIC, NULL);

	for (i = 0; i < numplanes; i++, p++)
	{
		sortnode[p].plane = &planeinfo[i];
		sortindex[p] = p;
	}

	for (i = 0; i < numpolyplanes; i++, p++)
	{
		sortnode[p].polyplane = &polyplaneinfo[i];
		sortindex[p] = p;
	}

	for (i = 0; i < numwalls; i++, p++)
	{
		sortnode[p].wall = &wallinfo[i];
		sortindex[p] = p;
	}

	// p is the number of stuff to sort

	// Add the 3D floors, thicksides, and masked textures...
	// Instead of going through drawsegs, we need to iterate
	// through the lists of masked textures and
	// translucent ffloors being drawn.

	// im not sure if this sort on the next line is needed.
	// it sorts the list based on the value of the 'drawcount' member of the drawnodes.
	// im thinking the list might already be in that order, but i havent bothered to check yet.
	// anyway doing this sort does not hurt and does not take much time.
	// the while loop after this sort is important however!
	qsort(sortindex, p, sizeof(size_t), CompareDrawNodes);

	// try solving floor order here. for each consecutive run of floors in the list, sort that run.
	while (run_start < p-1)// p-1 because a 1 plane run at the end of the list does not count
	{
		// locate run start
		if (sortnode[sortindex[run_start]].plane)
		{
			// found it, now look for run end
			size_t run_end;// (inclusive)
			for (i = run_start+1; i < p; i++)// size_t and UINT32 being used mixed here... shouldnt break anything though..
			{
				if (!sortnode[sortindex[i]].plane) break;
			}
			run_end = i-1;
			if (run_end > run_start)// if there are multiple consecutive planes, not just one
			{
				// consecutive run of planes found, now sort it
				// not sure how long these runs can be in reality...
				qsort(sortindex + run_start, run_end - run_start + 1, sizeof(size_t), CompareDrawNodePlanes);
			}
			run_start = run_end + 1;// continue looking for runs coming right after this one
		}
		else
		{
			// this wasnt the run start, try next one
			run_start++;
		}
	}

	// Okay! Let's draw it all! Woo!
	HWD.pfnSetTransform(&atransform);
	HWD.pfnSetShader(0);

	for (i = 0; i < p; i++)
	{
		if (sortnode[sortindex[i]].plane)
		{
			// We aren't traversing the BSP tree, so make gr_frontsector null to avoid crashes.
			gr_frontsector = NULL;

			if (!(sortnode[sortindex[i]].plane->blend & PF_NoTexture))
				HWR_GetFlat(sortnode[sortindex[i]].plane->lumpnum, R_NoEncore(sortnode[sortindex[i]].plane->FOFSector, sortnode[sortindex[i]].plane->isceiling));
			HWR_RenderPlane(sortnode[sortindex[i]].plane->xsub, sortnode[sortindex[i]].plane->isceiling, sortnode[sortindex[i]].plane->fixedheight, sortnode[sortindex[i]].plane->blend, sortnode[sortindex[i]].plane->lightlevel,
				sortnode[sortindex[i]].plane->lumpnum, sortnode[sortindex[i]].plane->FOFSector, sortnode[sortindex[i]].plane->alpha, /*sortnode[sortindex[i]].plane->fogplane,*/ sortnode[sortindex[i]].plane->planecolormap);
		}
		else if (sortnode[sortindex[i]].polyplane)
		{
			// We aren't traversing the BSP tree, so make gr_frontsector null to avoid crashes.
			gr_frontsector = NULL;

			if (!(sortnode[sortindex[i]].polyplane->blend & PF_NoTexture))
				HWR_GetFlat(sortnode[sortindex[i]].polyplane->lumpnum, R_NoEncore(sortnode[sortindex[i]].polyplane->FOFSector, sortnode[sortindex[i]].polyplane->isceiling));
			HWR_RenderPolyObjectPlane(sortnode[sortindex[i]].polyplane->polysector, sortnode[sortindex[i]].polyplane->isceiling, sortnode[sortindex[i]].polyplane->fixedheight, sortnode[sortindex[i]].polyplane->blend, sortnode[sortindex[i]].polyplane->lightlevel,
				sortnode[sortindex[i]].polyplane->lumpnum, sortnode[sortindex[i]].polyplane->FOFSector, sortnode[sortindex[i]].polyplane->alpha, sortnode[sortindex[i]].polyplane->planecolormap);
		}
		else if (sortnode[sortindex[i]].wall)
		{
			if (!(sortnode[sortindex[i]].wall->blend & PF_NoTexture))
				HWR_GetTexture(sortnode[sortindex[i]].wall->texnum);
			HWR_RenderWall(sortnode[sortindex[i]].wall->wallVerts, &sortnode[sortindex[i]].wall->Surf, sortnode[sortindex[i]].wall->blend, sortnode[sortindex[i]].wall->fogwall,
				sortnode[sortindex[i]].wall->lightlevel, sortnode[sortindex[i]].wall->wallcolormap);
		}
	}

	numwalls = 0;
	numplanes = 0;
	numpolyplanes = 0;

	// No mem leaks, please.
	Z_Free(sortnode);
	Z_Free(sortindex);
}

// --------------------------------------------------------------------------
//  Draw all vissprites
// --------------------------------------------------------------------------
void HWR_DrawSprites(void)
{
	UINT32 i;
	for (i = 0; i < gr_visspritecount; i++)
	{
		gr_vissprite_t *spr = gr_vsprorder[i];
		if (spr->precip)
			HWR_DrawPrecipitationSprite(spr);
		else
			if (spr->mobj && spr->mobj->skin && spr->mobj->sprite == SPR_PLAY)
			{
				// 8/1/19: Only don't display player models if no default SPR_PLAY is found.
				if (!cv_grmdls.value || ((md2_playermodels[(skin_t*)spr->mobj->skin-skins].notfound || md2_playermodels[(skin_t*)spr->mobj->skin-skins].scale < 0.0f) && ((!cv_grfallbackplayermodel.value) || md2_models[SPR_PLAY].notfound || md2_models[SPR_PLAY].scale < 0.0f)) || spr->mobj->state == &states[S_PLAY_SIGN])
					HWR_DrawSprite(spr);
				else
					HWR_DrawMD2(spr);
			}
			else
			{
				if (!cv_grmdls.value || md2_models[spr->mobj->sprite].notfound || md2_models[spr->mobj->sprite].scale < 0.0f)
					HWR_DrawSprite(spr);
				else
					HWR_DrawMD2(spr);
			}
	}
}

// --------------------------------------------------------------------------
// HWR_AddSprites
// During BSP traversal, this adds sprites by sector.
// --------------------------------------------------------------------------
void HWR_AddSprites(sector_t *sec)
{
	mobj_t *thing;
	precipmobj_t *precipthing;
	fixed_t approx_dist, limit_dist;

	INT32 splitflags;
	boolean split_drawsprite;	// drawing with splitscreen flags

	// BSP is traversed by subsector.
	// A sector might have been split into several
	//  subsectors during BSP building.
	// Thus we check whether its already added.
	if (sec->validcount == validcount)
		return;

	// Well, now it will be done.
	sec->validcount = validcount;

	// Handle all things in sector.
	// If a limit exists, handle things a tiny bit different.
	if ((limit_dist = (fixed_t)(/*(maptol & TOL_NIGHTS) ? cv_drawdist_nights.value : */cv_drawdist.value) << FRACBITS))
	{
		for (thing = sec->thinglist; thing; thing = thing->snext)
		{

			split_drawsprite = false;

			if (thing->sprite == SPR_NULL || thing->flags2 & MF2_DONTDRAW)
				continue;

			splitflags = thing->eflags & (MFE_DRAWONLYFORP1|MFE_DRAWONLYFORP2|MFE_DRAWONLYFORP3|MFE_DRAWONLYFORP4);

			if (splitscreen && splitflags)
			{
				if (thing->eflags & MFE_DRAWONLYFORP1)
					if (viewssnum == 0)
						split_drawsprite = true;

				if (thing->eflags & MFE_DRAWONLYFORP2)
					if (viewssnum == 1)
						split_drawsprite = true;

				if (thing->eflags & MFE_DRAWONLYFORP3 && splitscreen > 1)
					if (viewssnum == 2)
						split_drawsprite = true;

				if (thing->eflags & MFE_DRAWONLYFORP4 && splitscreen > 2)
					if (viewssnum == 3)
						split_drawsprite = true;
			}
			else
				split_drawsprite = true;

			if (!split_drawsprite)
				continue;

			approx_dist = P_AproxDistance(viewx-thing->x, viewy-thing->y);

			if (approx_dist > limit_dist)
				continue;

			HWR_ProjectSprite(thing);
		}
	}
	else
	{
		// Draw everything in sector, no checks
		for (thing = sec->thinglist; thing; thing = thing->snext)
		{

			split_drawsprite = false;

			if (thing->sprite == SPR_NULL || thing->flags2 & MF2_DONTDRAW)
				continue;

			splitflags = thing->eflags & (MFE_DRAWONLYFORP1|MFE_DRAWONLYFORP2|MFE_DRAWONLYFORP3|MFE_DRAWONLYFORP4);

			if (splitscreen && splitflags)
			{
				if (thing->eflags & MFE_DRAWONLYFORP1)
					if (viewssnum == 0)
						split_drawsprite = true;

				if (thing->eflags & MFE_DRAWONLYFORP2)
					if (viewssnum == 1)
						split_drawsprite = true;

				if (thing->eflags & MFE_DRAWONLYFORP3 && splitscreen > 1)
					if (viewssnum == 2)
						split_drawsprite = true;

				if (thing->eflags & MFE_DRAWONLYFORP4 && splitscreen > 2)
					if (viewssnum == 3)
						split_drawsprite = true;
			}
			else
				split_drawsprite = true;

			if (!split_drawsprite)
				continue;

			HWR_ProjectSprite(thing);
		}
	}

	// No to infinite precipitation draw distance.
	if ((limit_dist = (fixed_t)cv_drawdist_precip.value << FRACBITS))
	{
		for (precipthing = sec->preciplist; precipthing; precipthing = precipthing->snext)
		{
			if (precipthing->precipflags & PCF_INVISIBLE)
				continue;

			approx_dist = P_AproxDistance(viewx-precipthing->x, viewy-precipthing->y);

			if (approx_dist > limit_dist)
				continue;

			HWR_ProjectPrecipitationSprite(precipthing);
		}
	}
}

// --------------------------------------------------------------------------
// HWR_ProjectSprite
//  Generates a vissprite for a thing if it might be visible.
// --------------------------------------------------------------------------
// BP why not use xtoviexangle/viewangletox like in bsp ?....
void HWR_ProjectSprite(mobj_t *thing)
{
	gr_vissprite_t *vis;
	float tr_x, tr_y;
	float tz;
	float x1, x2;
	float z1, z2;
	float rightsin, rightcos;
	float this_scale;
	float gz, gzt;
	spritedef_t *sprdef;
	spriteframe_t *sprframe;
	size_t lumpoff;
	unsigned rot;
	UINT8 flip;
	angle_t ang;
	const boolean papersprite = (thing->frame & FF_PAPERSPRITE);
	INT32 heightsec, phs;

	if (!thing)
		return;
	else
		this_scale = FIXED_TO_FLOAT(thing->scale);

	// transform the origin point
	tr_x = FIXED_TO_FLOAT(thing->x) - gr_viewx;
	tr_y = FIXED_TO_FLOAT(thing->y) - gr_viewy;

	// rotation around vertical axis
	tz = (tr_x * gr_viewcos) + (tr_y * gr_viewsin);

	// thing is behind view plane?
	if (tz < ZCLIP_PLANE && !papersprite && (!cv_grmdls.value || md2_models[thing->sprite].notfound == true)) //Yellow: Only MD2's dont disappear
		return;

	// The above can stay as it works for cutting sprites that are too close
	tr_x = FIXED_TO_FLOAT(thing->x);
	tr_y = FIXED_TO_FLOAT(thing->y);

	// decide which patch to use for sprite relative to player
#ifdef RANGECHECK
	if ((unsigned)thing->sprite >= numsprites)
		I_Error("HWR_ProjectSprite: invalid sprite number %i ", thing->sprite);
#endif

	rot = thing->frame&FF_FRAMEMASK;

	//Fab : 02-08-98: 'skin' override spritedef currently used for skin
	if (thing->skin && thing->sprite == SPR_PLAY)
		sprdef = &((skin_t *)thing->skin)->spritedef;
	else
		sprdef = &sprites[thing->sprite];

	if (rot >= sprdef->numframes)
	{
		CONS_Alert(CONS_ERROR, M_GetText("HWR_ProjectSprite: invalid sprite frame %s/%s for %s\n"),
			sizeu1(rot), sizeu2(sprdef->numframes), sprnames[thing->sprite]);
		thing->sprite = states[S_UNKNOWN].sprite;
		thing->frame = states[S_UNKNOWN].frame;
		sprdef = &sprites[thing->sprite];
		rot = thing->frame&FF_FRAMEMASK;
		thing->state->sprite = thing->sprite;
		thing->state->frame = thing->frame;
	}

	sprframe = &sprdef->spriteframes[rot];

#ifdef PARANOIA
	if (!sprframe)
		I_Error("sprframes NULL for sprite %d\n", thing->sprite);
#endif

	if (thing->player)
		ang = R_PointToAngle (thing->x, thing->y) - thing->player->frameangle;
	else
		ang = R_PointToAngle (thing->x, thing->y) - thing->angle;

	if (sprframe->rotate == SRF_SINGLE)
	{
		// use single rotation for all views
		rot = 0;                        //Fab: for vis->patch below
		lumpoff = sprframe->lumpid[0];     //Fab: see note above
		flip = sprframe->flip; // Will only be 0x00 or 0xFF

		if (papersprite && ang < ANGLE_180)
		{
			if (flip)
				flip = 0;
			else
				flip = 255;
		}
	}
	else
	{
		// choose a different rotation based on player view
		if ((ang < ANGLE_180) && (sprframe->rotate & SRF_RIGHT)) // See from right
			rot = 6; // F7 slot
		else if ((ang >= ANGLE_180) && (sprframe->rotate & SRF_LEFT)) // See from left
			rot = 2; // F3 slot
		else // Normal behaviour
			rot = (ang+ANGLE_202h)>>29;

		//Fab: lumpid is the index for spritewidth,spriteoffset... tables
		lumpoff = sprframe->lumpid[rot];
		flip = sprframe->flip & (1<<rot);

		if (papersprite && ang < ANGLE_180)
		{
			if (flip)
				flip = 0;
			else
				flip = 1<<rot;
		}
	}

	if (thing->skin && ((skin_t *)thing->skin)->flags & SF_HIRES)
		this_scale = this_scale * FIXED_TO_FLOAT(((skin_t *)thing->skin)->highresscale);

	if (papersprite)
	{
		rightsin = FIXED_TO_FLOAT(FINESINE((thing->angle)>>ANGLETOFINESHIFT));
		rightcos = FIXED_TO_FLOAT(FINECOSINE((thing->angle)>>ANGLETOFINESHIFT));
	}
	else
	{
		rightsin = FIXED_TO_FLOAT(FINESINE((viewangle + ANGLE_90)>>ANGLETOFINESHIFT));
		rightcos = FIXED_TO_FLOAT(FINECOSINE((viewangle + ANGLE_90)>>ANGLETOFINESHIFT));
	}

	if (flip)
	{
		x1 = (FIXED_TO_FLOAT(spritecachedinfo[lumpoff].width - spritecachedinfo[lumpoff].offset) * this_scale);
		x2 = (FIXED_TO_FLOAT(spritecachedinfo[lumpoff].offset) * this_scale);
	}
	else
	{
		x1 = (FIXED_TO_FLOAT(spritecachedinfo[lumpoff].offset) * this_scale);
		x2 = (FIXED_TO_FLOAT(spritecachedinfo[lumpoff].width - spritecachedinfo[lumpoff].offset) * this_scale);
	}

	z1 = tr_y + x1 * rightsin;
	z2 = tr_y - x2 * rightsin;
	x1 = tr_x + x1 * rightcos;
	x2 = tr_x - x2 * rightcos;


	if (thing->eflags & MFE_VERTICALFLIP)
	{
		gz = FIXED_TO_FLOAT(thing->z+thing->height) - FIXED_TO_FLOAT(spritecachedinfo[lumpoff].topoffset) * this_scale;
		gzt = gz + FIXED_TO_FLOAT(spritecachedinfo[lumpoff].height) * this_scale;
	}
	else
	{
		gzt = FIXED_TO_FLOAT(thing->z) + FIXED_TO_FLOAT(spritecachedinfo[lumpoff].topoffset) * this_scale;
		gz = gzt - FIXED_TO_FLOAT(spritecachedinfo[lumpoff].height) * this_scale;
	}

	if (thing->subsector->sector->cullheight)
	{
		if (HWR_DoCulling(thing->subsector->sector->cullheight, viewsector->cullheight, gr_viewz, gz, gzt))
			return;
	}

	heightsec = thing->subsector->sector->heightsec;
	if (viewplayer->mo && viewplayer->mo->subsector)
		phs = viewplayer->mo->subsector->sector->heightsec;
	else
		phs = -1;

	if (heightsec != -1 && phs != -1) // only clip things which are in special sectors
	{
		if (gr_viewz < FIXED_TO_FLOAT(sectors[phs].floorheight) ?
		FIXED_TO_FLOAT(thing->z) >= FIXED_TO_FLOAT(sectors[heightsec].floorheight) :
		gzt < FIXED_TO_FLOAT(sectors[heightsec].floorheight))
			return;
		if (gr_viewz > FIXED_TO_FLOAT(sectors[phs].ceilingheight) ?
		gzt < FIXED_TO_FLOAT(sectors[heightsec].ceilingheight) && gr_viewz >= FIXED_TO_FLOAT(sectors[heightsec].ceilingheight) :
		FIXED_TO_FLOAT(thing->z) >= FIXED_TO_FLOAT(sectors[heightsec].ceilingheight))
			return;
	}

	// store information in a vissprite
	vis = HWR_NewVisSprite();
	vis->x1 = x1;
	vis->x2 = x2;
	vis->z1 = z1;
	vis->z2 = z2;
	vis->tz = tz; // Keep tz for the simple sprite sorting that happens
	vis->dispoffset = thing->info->dispoffset; // Monster Iestyn: 23/11/15: HARDWARE SUPPORT AT LAST
	vis->patchlumpnum = sprframe->lumppat[rot];
	vis->flip = flip;
	vis->mobj = thing;


	//Hurdler: 25/04/2000: now support colormap in hardware mode
	if ((vis->mobj->flags & MF_BOSS) && (vis->mobj->flags2 & MF2_FRET) && (leveltime & 1)) // Bosses "flash"
	{
		if (vis->mobj->type == MT_CYBRAKDEMON)
			vis->colormap = R_GetTranslationColormap(TC_ALLWHITE, 0, GTC_CACHE);
		else if (vis->mobj->type == MT_METALSONIC_BATTLE)
			vis->colormap = R_GetTranslationColormap(TC_METALSONIC, 0, GTC_CACHE);
		else
			vis->colormap = R_GetTranslationColormap(TC_BOSS, 0, GTC_CACHE);
	}
	else if (thing->color)
	{
		// New colormap stuff for skins Tails 06-07-2002
		if (thing->colorized)
			vis->colormap = R_GetTranslationColormap(TC_RAINBOW, thing->color, GTC_CACHE);
		else if (thing->skin && thing->sprite == SPR_PLAY) // This thing is a player!
		{
			size_t skinnum = (skin_t*)thing->skin-skins;
			vis->colormap = R_GetTranslationColormap((INT32)skinnum, thing->color, GTC_CACHE);
		}
		else
			vis->colormap = R_GetTranslationColormap(TC_DEFAULT, thing->color, GTC_CACHE);
	}
	else
	{
		vis->colormap = colormaps;
#ifdef GLENCORE
		if (encoremap && (thing->flags & (MF_SCENERY|MF_NOTHINK)) && !(thing->flags & MF_DONTENCOREMAP))
			vis->colormap += (256*32);
#endif
	}

	// set top/bottom coords
	vis->ty = gzt;

	//CONS_Debug(DBG_RENDER, "------------------\nH: sprite  : %d\nH: frame   : %x\nH: type    : %d\nH: sname   : %s\n\n",
	//            thing->sprite, thing->frame, thing->type, sprnames[thing->sprite]);

	if (thing->eflags & MFE_VERTICALFLIP)
		vis->vflip = true;
	else
		vis->vflip = false;

	vis->precip = false;
}

// Precipitation projector for hardware mode
void HWR_ProjectPrecipitationSprite(precipmobj_t *thing)
{
	gr_vissprite_t *vis;
	float tr_x, tr_y;
	float tz;
	float x1, x2;
	float z1, z2;
	float rightsin, rightcos;
	spritedef_t *sprdef;
	spriteframe_t *sprframe;
	size_t lumpoff;
	unsigned rot = 0;
	UINT8 flip;

	// transform the origin point
	tr_x = FIXED_TO_FLOAT(thing->x) - gr_viewx;
	tr_y = FIXED_TO_FLOAT(thing->y) - gr_viewy;

	// rotation around vertical axis
	tz = (tr_x * gr_viewcos) + (tr_y * gr_viewsin);

	// thing is behind view plane?
	if (tz < ZCLIP_PLANE)
		return;

	tr_x = FIXED_TO_FLOAT(thing->x);
	tr_y = FIXED_TO_FLOAT(thing->y);

	// decide which patch to use for sprite relative to player
	if ((unsigned)thing->sprite >= numsprites)
#ifdef RANGECHECK
		I_Error("HWR_ProjectPrecipitationSprite: invalid sprite number %i ",
		        thing->sprite);
#else
		return;
#endif

	sprdef = &sprites[thing->sprite];

	if ((size_t)(thing->frame&FF_FRAMEMASK) >= sprdef->numframes)
#ifdef RANGECHECK
		I_Error("HWR_ProjectPrecipitationSprite: invalid sprite frame %i : %i for %s",
		        thing->sprite, thing->frame, sprnames[thing->sprite]);
#else
		return;
#endif

	sprframe = &sprdef->spriteframes[ thing->frame & FF_FRAMEMASK];

	// use single rotation for all views
	lumpoff = sprframe->lumpid[0];
	flip = sprframe->flip; // Will only be 0x00 or 0xFF

	rightsin = FIXED_TO_FLOAT(FINESINE((viewangle + ANGLE_90)>>ANGLETOFINESHIFT));
	rightcos = FIXED_TO_FLOAT(FINECOSINE((viewangle + ANGLE_90)>>ANGLETOFINESHIFT));
	if (flip)
	{
		x1 = FIXED_TO_FLOAT(spritecachedinfo[lumpoff].width - spritecachedinfo[lumpoff].offset);
		x2 = FIXED_TO_FLOAT(spritecachedinfo[lumpoff].offset);
	}
	else
	{
		x1 = FIXED_TO_FLOAT(spritecachedinfo[lumpoff].offset);
		x2 = FIXED_TO_FLOAT(spritecachedinfo[lumpoff].width - spritecachedinfo[lumpoff].offset);
	}

	z1 = tr_y + x1 * rightsin;
	z2 = tr_y - x2 * rightsin;
	x1 = tr_x + x1 * rightcos;
	x2 = tr_x - x2 * rightcos;

	// okay, we can't return now... this is a hack, but weather isn't networked, so it should be ok
	if (!(thing->precipflags & PCF_THUNK))
	{
		if (thing->precipflags & PCF_RAIN)
			P_RainThinker(thing);
		else
			P_SnowThinker(thing);
		thing->precipflags |= PCF_THUNK;
	}

	//
	// store information in a vissprite
	//
	vis = HWR_NewVisSprite();
	vis->x1 = x1;
	vis->x2 = x2;
	vis->z1 = z1;
	vis->z2 = z2;
	vis->tz = tz;
	vis->dispoffset = 0; // Monster Iestyn: 23/11/15: HARDWARE SUPPORT AT LAST
	vis->patchlumpnum = sprframe->lumppat[rot];
	vis->flip = flip;
	vis->mobj = (mobj_t *)thing;

	vis->colormap = colormaps;

#ifdef GLENCORE
	if (encoremap && !(thing->flags & MF_DONTENCOREMAP))
		vis->colormap += (256*32);
#endif

	// set top/bottom coords
	vis->ty = FIXED_TO_FLOAT(thing->z + spritecachedinfo[lumpoff].topoffset);

	vis->precip = true;
}

static boolean drewsky = false;

void HWR_DrawSkyBackground(float fpov)
{
	FTransform dometransform;

	if (drewsky)
		return;

	memset(&dometransform, 0x00, sizeof(FTransform));

	//04/01/2000: Hurdler: added for T&L
	//                     It should replace all other gr_viewxxx when finished
	if (!atransform.shearing)
		dometransform.anglex = (float)(aimingangle>>ANGLETOFINESHIFT)*(360.0f/(float)FINEANGLES);
	dometransform.angley = (float)((viewangle-ANGLE_270)>>ANGLETOFINESHIFT)*(360.0f/(float)FINEANGLES);

	dometransform.flip = atransform.flip;
	dometransform.mirror = atransform.mirror;
	dometransform.shearing = atransform.shearing;
	dometransform.viewaiming = atransform.viewaiming;

	dometransform.scalex = 1;
	dometransform.scaley = (float)vid.width/vid.height;
	dometransform.scalez = 1;
	dometransform.fovxangle = fpov; // Tails
	dometransform.fovyangle = fpov; // Tails
	dometransform.splitscreen = splitscreen;

	HWR_GetTexture(texturetranslation[skytexture]);
	HWD.pfnSetShader(7);	// sky shader
	HWD.pfnRenderSkyDome(skytexture, textures[skytexture]->width, textures[skytexture]->height, dometransform);
	HWD.pfnSetShader(0);
}


// -----------------+
// HWR_ClearView : clear the viewwindow, with maximum z value
// -----------------+
static inline void HWR_ClearView(void)
{
	HWD.pfnGClipRect((INT32)gr_viewwindowx,
	                 (INT32)gr_viewwindowy,
	                 (INT32)(gr_viewwindowx + gr_viewwidth),
	                 (INT32)(gr_viewwindowy + gr_viewheight),
	                 ZCLIP_PLANE);
	HWD.pfnClearBuffer(false, true, 0);
}


// -----------------+
// HWR_SetViewSize  : set projection and scaling values
// -----------------+
void HWR_SetViewSize(void)
{
	// setup view size
	gr_viewwidth = (float)vid.width;
	gr_viewheight = (float)vid.height;

	if (splitscreen)
		gr_viewheight /= 2;

	if (splitscreen > 1)
		gr_viewwidth /= 2;

	gr_basecenterx = gr_viewwidth / 2;
	gr_basecentery = gr_viewheight / 2;

	gr_baseviewwindowy = 0;
	gr_basewindowcentery = (float)(gr_viewheight / 2);

	gr_baseviewwindowx = 0;
	gr_basewindowcenterx = (float)(gr_viewwidth / 2);

	gr_pspritexscale = ((vid.width*gr_pspriteyscale*BASEVIDHEIGHT)/BASEVIDWIDTH)/vid.height;
	gr_pspriteyscale = ((vid.height*gr_pspritexscale*BASEVIDWIDTH)/BASEVIDHEIGHT)/vid.width;

	HWD.pfnFlushScreenTextures();
}


// ==========================================================================
// Render the current frame.
// ==========================================================================
void HWR_RenderFrame(INT32 viewnumber, player_t *player, boolean skybox)
{
	angle_t a1;
	const float fpov = FIXED_TO_FLOAT(cv_fov.value+player->fovadd);
	postimg_t *postprocessor = &postimgtype[0];
	INT32 i;

	// set window position
	gr_centerx = gr_basecenterx;
	gr_viewwindowx = gr_baseviewwindowx;
	gr_windowcenterx = gr_basewindowcenterx;
	gr_centery = gr_basecentery;
	gr_viewwindowy = gr_baseviewwindowy;
	gr_windowcentery = gr_basewindowcentery;

	if ((splitscreen == 1 && viewnumber == 1) || (splitscreen > 1 && viewnumber > 1))
	{
		gr_viewwindowy += gr_viewheight;
		gr_windowcentery += gr_viewheight;
	}

	if (splitscreen > 1 && viewnumber & 1)
	{
		gr_viewwindowx += gr_viewwidth;
		gr_windowcenterx += gr_viewwidth;
	}

	// check for new console commands.
	NetUpdate();

	gr_viewx = FIXED_TO_FLOAT(viewx);
	gr_viewy = FIXED_TO_FLOAT(viewy);
	gr_viewz = FIXED_TO_FLOAT(viewz);
	gr_viewsin = FIXED_TO_FLOAT(viewsin);
	gr_viewcos = FIXED_TO_FLOAT(viewcos);

	// Set T&L transform
	atransform.x = gr_viewx;
	atransform.y = gr_viewy;
	atransform.z = gr_viewz;

	atransform.scalex = 1;
	atransform.scaley = (float)vid.width/vid.height;
	atransform.scalez = 1;

	// 14042019
	gr_aimingangle = aimingangle;
	atransform.shearing = false;
	atransform.viewaiming = aimingangle;

	if (cv_grshearing.value)
	{
		gr_aimingangle = 0;
		atransform.shearing = true;
	}

	gr_viewludsin = FIXED_TO_FLOAT(FINECOSINE(gr_aimingangle>>ANGLETOFINESHIFT));
	gr_viewludcos = FIXED_TO_FLOAT(-FINESINE(gr_aimingangle>>ANGLETOFINESHIFT));

	atransform.anglex = (float)(gr_aimingangle>>ANGLETOFINESHIFT)*(360.0f/(float)FINEANGLES);
	atransform.angley = (float)(viewangle>>ANGLETOFINESHIFT)*(360.0f/(float)FINEANGLES);

	atransform.fovxangle = fpov; // Tails
	atransform.fovyangle = fpov; // Tails
	atransform.splitscreen = splitscreen;

	for (i = 0; i <= splitscreen; i++)
	{
		if (player == &players[displayplayers[i]])
			postprocessor = &postimgtype[i];
	}

	atransform.flip = false;
	if (*postprocessor == postimg_flip)
		atransform.flip = true;

	atransform.mirror = false;
	if (*postprocessor == postimg_mirror)
		atransform.mirror = true;

	// Clear view, set viewport (glViewport), set perspective...
	HWR_ClearView();
	HWR_ClearSprites();

	ST_doPaletteStuff();

	// Draw the sky background.
	HWR_DrawSkyBackground(fpov);
	if (skybox)
		drewsky = true;

	a1 = gld_FrustumAngle(gr_aimingangle);
	gld_clipper_Clear();
	gld_clipper_SafeAddClipRange(viewangle + a1, viewangle - a1);
#ifdef HAVE_SPHEREFRUSTRUM
	gld_FrustrumSetup();
#endif

	// Set transform.
	HWD.pfnSetTransform(&atransform);

	// Reset the shader state.
	HWD.pfnSetSpecialState(HWD_SET_SHADERS, cv_grshaders.value);
	HWD.pfnSetShader(0);

	if (cv_grbatching.value)
		HWD.pfnStartBatching();

	drawcount = 0;
	validcount++;

	// Recursively "render" the BSP tree.
	HWR_RenderBSPNode((INT32)numnodes-1);

	if (cv_grbatching.value)
	{
		int dummy = 0;// the vars in RenderBatches are meant for render stats. But we don't have that stuff in this branch
					// so that stuff could be removed...
		HWD.pfnRenderBatches(&dummy, &dummy, &dummy, &dummy, &dummy, &dummy, &dummy);
	}

	// Check for new console commands.
	// this was removed since it caused crashes on leaving record attack with models on since it was removing mobjs that were about to be rendered
	//NetUpdate();

	// Draw MD2 and sprites
	HWR_SortVisSprites();
	HWR_DrawSprites();

	if (numplanes || numpolyplanes || numwalls) // Render FOFs and translucent walls after everything
		HWR_RenderDrawNodes();

	// Unset transform and shader
	HWD.pfnSetTransform(NULL);
	HWD.pfnUnSetShader();

	// Run post processor effects
	if (!skybox)
		HWR_DoPostProcessor(player);

	// Check for new console commands.
	NetUpdate();

	// added by Hurdler for correct splitscreen
	// moved here by hurdler so it works with the new near clipping plane
	HWD.pfnGClipRect(0, 0, vid.width, vid.height, NZCLIP_PLANE);
}

// ==========================================================================
// Render the player view.
// ==========================================================================
void HWR_RenderPlayerView(INT32 viewnumber, player_t *player)
{
	const boolean skybox = (skyboxmo[0] && cv_skybox.value); // True if there's a skybox object and skyboxes are on

	// Clear the color buffer, stops HOMs. Also seems to fix the skybox issue on Intel GPUs.
	if (viewnumber == 0) // Only do it if it's the first screen being rendered
	{
		FRGBAFloat ClearColor;

		ClearColor.red = 0.0f;
		ClearColor.green = 0.0f;
		ClearColor.blue = 0.0f;
		ClearColor.alpha = 1.0f;

		HWD.pfnClearBuffer(true, false, &ClearColor);
	}

	if (viewnumber > 3)
		return;

	// Render the skybox if there is one.
	drewsky = false;
	if (skybox)
	{
		R_SkyboxFrame(player);
		HWR_RenderFrame(viewnumber, player, true);
	}

	R_SetupFrame(player, false); // This can stay false because it is only used to set viewsky in r_main.c, which isn't used here
	framecount++; // for timedemo
	HWR_RenderFrame(viewnumber, player, false);
}

// ==========================================================================
//                                                         3D ENGINE COMMANDS
// ==========================================================================

// **************************************************************************
//                                                            3D ENGINE SETUP
// **************************************************************************

// --------------------------------------------------------------------------
// Add hardware engine commands & consvars
// --------------------------------------------------------------------------
//added by Hurdler: console varibale that are saved
void HWR_AddCommands(void)
{
	CV_RegisterVar(&cv_grrounddown);
	CV_RegisterVar(&cv_grfiltermode);
	CV_RegisterVar(&cv_granisotropicmode);
	CV_RegisterVar(&cv_grcorrecttricks);
	CV_RegisterVar(&cv_grsolvetjoin);

	CV_RegisterVar(&cv_grbatching);
}

// --------------------------------------------------------------------------
// Setup the hardware renderer
// --------------------------------------------------------------------------
void HWR_Startup(void)
{
	static boolean startupdone = false;

	// do this once
	if (!startupdone)
	{
		CONS_Printf("HWR_Startup()...\n");
		HWR_InitTextureCache();
		HWR_InitMD2();
	}

	if (rendermode == render_opengl)
		textureformat = patchformat =
#ifdef _NDS
			GR_TEXFMT_P_8;
#else
			GR_RGBA;
#endif

	startupdone = true;

	// jimita
	HWD.pfnKillShaders();
	if (!HWD.pfnLoadShaders())
		gr_shadersavailable = false;
}


// --------------------------------------------------------------------------
// Free resources allocated by the hardware renderer
// --------------------------------------------------------------------------
void HWR_Shutdown(void)
{
	CONS_Printf("HWR_Shutdown()\n");
	HWR_FreeExtraSubsectors();
	HWR_FreeTextureCache();
	HWD.pfnFlushScreenTextures();
}

void HWR_AddTransparentWall(FOutVector *wallVerts, FSurfaceInfo *pSurf, INT32 texnum, FBITFIELD blend, boolean fogwall, INT32 lightlevel, extracolormap_t *wallcolormap)
{
	static size_t allocedwalls = 0;

	// Force realloc if buffer has been freed
	if (!wallinfo)
		allocedwalls = 0;

	if (allocedwalls < numwalls + 1)
	{
		allocedwalls += MAX_TRANSPARENTWALL;
		Z_Realloc(wallinfo, allocedwalls * sizeof (*wallinfo), PU_LEVEL, &wallinfo);
	}

	M_Memcpy(wallinfo[numwalls].wallVerts, wallVerts, sizeof (wallinfo[numwalls].wallVerts));
	M_Memcpy(&wallinfo[numwalls].Surf, pSurf, sizeof (FSurfaceInfo));
	wallinfo[numwalls].texnum = texnum;
	wallinfo[numwalls].blend = blend;
	wallinfo[numwalls].drawcount = drawcount++;
	wallinfo[numwalls].fogwall = fogwall;
	wallinfo[numwalls].lightlevel = lightlevel;
	wallinfo[numwalls].wallcolormap = wallcolormap;
	numwalls++;
}

void HWR_RenderWall(FOutVector *wallVerts, FSurfaceInfo *pSurf, FBITFIELD blend, boolean fogwall, INT32 lightlevel, extracolormap_t *wallcolormap)
{
	FBITFIELD blendmode = blend;
	UINT8 alpha = pSurf->PolyColor.s.alpha; // retain the alpha

	// Lighting is done here instead so that fog isn't drawn incorrectly on transparent walls after sorting
	HWR_Lighting(pSurf, lightlevel, wallcolormap);

	pSurf->PolyColor.s.alpha = alpha; // put the alpha back after lighting

	HWD.pfnSetShader(2);	// wall shader

	if (blend & PF_Environment)
		blendmode |= PF_Occlude;	// PF_Occlude must be used for solid objects

	if (fogwall)
	{
		blendmode |= PF_Fog;
		HWD.pfnSetShader(6);	// fog shader
	}

	blendmode |= PF_Modulated;	// No PF_Occlude means overlapping (incorrect) transparency

	HWD.pfnDrawPolygon(pSurf, wallVerts, 4, blendmode);

#ifdef WALLSPLATS
	if (gr_curline->linedef->splats && cv_splats.value)
		HWR_DrawSegsSplats(pSurf);
#endif
}

void HWR_DoPostProcessor(player_t *player)
{
	postimg_t *type = &postimgtype[0];
	UINT8 i;

	HWD.pfnUnSetShader();

	for (i = splitscreen; i > 0; i--)
	{
		if (player == &players[displayplayers[i]])
		{
			type = &postimgtype[i];
			break;
		}
	}

	// Armageddon Blast Flash!
	// Could this even be considered postprocessor?
	if (player->flashcount)
	{
		FOutVector      v[4];
		FSurfaceInfo Surf;

		v[0].x = v[2].y = v[3].x = v[3].y = -4.0f;
		v[0].y = v[1].x = v[1].y = v[2].x = 4.0f;
		v[0].z = v[1].z = v[2].z = v[3].z = 4.0f; // 4.0 because of the same reason as with the sky, just after the screen is cleared so near clipping plane is 3.99

		// This won't change if the flash palettes are changed unfortunately, but it works for its purpose
		if (player->flashpal == PAL_NUKE)
		{
			Surf.PolyColor.s.red = 0xff;
			Surf.PolyColor.s.green = Surf.PolyColor.s.blue = 0x7F; // The nuke palette is kind of pink-ish
		}
		else
			Surf.PolyColor.s.red = Surf.PolyColor.s.green = Surf.PolyColor.s.blue = 0xff;

		Surf.PolyColor.s.alpha = 0xc0; // match software mode

		HWD.pfnDrawPolygon(&Surf, v, 4, PF_Modulated|PF_Translucent|PF_NoTexture|PF_NoDepthTest);
	}

	// Capture the screen for intermission and screen waving
	if(gamestate != GS_INTERMISSION)
		HWD.pfnMakeScreenTexture();

	if (splitscreen) // Not supported in splitscreen - someone want to add support?
		return;

	// Drunken vision! WooOOooo~
	if (*type == postimg_water || *type == postimg_heat)
	{
		// 10 by 10 grid. 2 coordinates (xy)
		float v[SCREENVERTS][SCREENVERTS][2];
		double disStart = leveltime;
		UINT8 x, y;
		INT32 WAVELENGTH;
		INT32 AMPLITUDE;
		INT32 FREQUENCY;

		// Modifies the wave.
		if (*type == postimg_water)
		{
			WAVELENGTH = 20; // Lower is longer
			AMPLITUDE = 20; // Lower is bigger
			FREQUENCY = 16; // Lower is faster
		}
		else
		{
			WAVELENGTH = 10; // Lower is longer
			AMPLITUDE = 30; // Lower is bigger
			FREQUENCY = 4; // Lower is faster
		}

		for (x = 0; x < SCREENVERTS; x++)
		{
			for (y = 0; y < SCREENVERTS; y++)
			{
				// Change X position based on its Y position.
				v[x][y][0] = (x/((float)(SCREENVERTS-1.0f)/9.0f))-4.5f + (float)sin((disStart+(y*WAVELENGTH))/FREQUENCY)/AMPLITUDE;
				v[x][y][1] = (y/((float)(SCREENVERTS-1.0f)/9.0f))-4.5f;
			}
		}
		HWD.pfnPostImgRedraw(v);

		// Capture the screen again for screen waving on the intermission
		if(gamestate != GS_INTERMISSION)
			HWD.pfnMakeScreenTexture();
	}
	// Flipping of the screen isn't done here anymore
}

void HWR_StartScreenWipe(void)
{
	HWD.pfnStartScreenWipe();
}

void HWR_EndScreenWipe(void)
{
	HWD.pfnEndScreenWipe();
}

void HWR_DrawIntermissionBG(void)
{
	HWD.pfnDrawIntermissionBG();
}

void HWR_DoWipe(UINT8 wipenum, UINT8 scrnnum)
{
	static char lumpname[9] = "FADEmmss";
	lumpnum_t lumpnum;
	size_t lsize;

	if (wipenum > 99 || scrnnum > 99) // not a valid wipe number
		return; // shouldn't end up here really, the loop should've stopped running beforehand

	// puts the numbers into the lumpname
	sprintf(&lumpname[4], "%.2hu%.2hu", (UINT16)wipenum, (UINT16)scrnnum);
	lumpnum = W_CheckNumForName(lumpname);

	if (lumpnum == LUMPERROR) // again, shouldn't be here really
		return;

	lsize = W_LumpLength(lumpnum);

	if (!(lsize == 256000 || lsize == 64000 || lsize == 16000 || lsize == 4000))
	{
		CONS_Alert(CONS_WARNING, "Fade mask lump %s of incorrect size, ignored\n", lumpname);
		return; // again, shouldn't get here if it is a bad size
	}

	HWR_GetFadeMask(lumpnum);
	HWD.pfnDoScreenWipe();
}

void HWR_MakeScreenFinalTexture(void)
{
    HWD.pfnMakeScreenFinalTexture();
}

void HWR_DrawScreenFinalTexture(int width, int height)
{
    HWD.pfnDrawScreenFinalTexture(width, height);
}

// jimita 18032019
typedef struct
{
	char type[16];
	INT32 id;
} shaderxlat_t;

static inline UINT16 HWR_CheckShader(UINT16 wadnum)
{
	UINT16 i;
	lumpinfo_t *lump_p;

	lump_p = wadfiles[wadnum]->lumpinfo;
	for (i = 0; i < wadfiles[wadnum]->numlumps; i++, lump_p++)
		if (memcmp(lump_p->name, "SHADERS", 7) == 0)
			return i;

	return INT16_MAX;
}

void HWR_LoadShaders(UINT16 wadnum, boolean PK3)
{
	UINT16 lump;
	char *shaderdef, *line;
	char *stoken;
	char *value;
	size_t size;
	int linenum = 1;
	int shadertype = 0;
	int i;

	#define SHADER_TYPES 7
	shaderxlat_t shaderxlat[SHADER_TYPES] =
	{
		{"Flat", 1},
		{"WallTexture", 2},
		{"Sprite", 3},
		{"Model", 4},
		{"WaterRipple", 5},
		{"Fog", 6},
		{"Sky", 7},
	};

	lump = HWR_CheckShader(wadnum);
	if (lump == INT16_MAX)
		return;

	shaderdef = W_CacheLumpNumPwad(wadnum, lump, PU_CACHE);
	size = W_LumpLengthPwad(wadnum, lump);

	line = Z_Malloc(size+1, PU_STATIC, NULL);
	if (!line)
		I_Error("HWR_LoadShaders: No more free memory\n");

	M_Memcpy(line, shaderdef, size);
	line[size] = '\0';

	stoken = strtok(line, "\r\n ");
	while (stoken)
	{
		if ((stoken[0] == '/' && stoken[1] == '/')
			|| (stoken[0] == '#'))// skip comments
		{
			stoken = strtok(NULL, "\r\n");
			goto skip_field;
		}

		if (!stricmp(stoken, "GLSL"))
		{
			value = strtok(NULL, "\r\n ");
			if (!value)
			{
				CONS_Alert(CONS_WARNING, "HWR_LoadShaders: Missing shader type (file %s, line %d)\n", wadfiles[wadnum]->filename, linenum);
				stoken = strtok(NULL, "\r\n"); // skip end of line
				goto skip_lump;
			}

			if (!stricmp(value, "VERTEX"))
				shadertype = 1;
			else if (!stricmp(value, "FRAGMENT"))
				shadertype = 2;

skip_lump:
			stoken = strtok(NULL, "\r\n ");
			linenum++;
		}
		else
		{
			value = strtok(NULL, "\r\n= ");
			if (!value)
			{
				CONS_Alert(CONS_WARNING, "HWR_LoadShaders: Missing shader target (file %s, line %d)\n", wadfiles[wadnum]->filename, linenum);
				stoken = strtok(NULL, "\r\n"); // skip end of line
				goto skip_field;
			}

			if (!shadertype)
			{
				CONS_Alert(CONS_ERROR, "HWR_LoadShaders: Missing shader type (file %s, line %d)\n", wadfiles[wadnum]->filename, linenum);
				Z_Free(line);
				return;
			}

			for (i = 0; i < SHADER_TYPES; i++)
			{
				if (!stricmp(shaderxlat[i].type, stoken))
				{
					size_t shader_size;
					char *shader_source;
					char *shader_lumpname;
					UINT16 shader_lumpnum;

					if (PK3)
					{
						shader_lumpname = Z_Malloc(strlen(value) + 12, PU_STATIC, NULL);
						strcpy(shader_lumpname, "Shaders/sh_");
						strcat(shader_lumpname, value);
						shader_lumpnum = W_CheckNumForFullNamePK3(shader_lumpname, wadnum, 0);
					}
					else
					{
						shader_lumpname = Z_Malloc(strlen(value) + 4, PU_STATIC, NULL);
						strcpy(shader_lumpname, "SH_");
						strcat(shader_lumpname, value);
						shader_lumpnum = W_CheckNumForNamePwad(shader_lumpname, wadnum, 0);
					}

					if (shader_lumpnum == INT16_MAX)
					{
						CONS_Alert(CONS_ERROR, "HWR_LoadShaders: Missing shader source %s (file %s, line %d)\n", shader_lumpname, wadfiles[wadnum]->filename, linenum);
						Z_Free(shader_lumpname);
						continue;
					}

					shader_size = W_LumpLengthPwad(wadnum, shader_lumpnum);
					shader_source = Z_Malloc(shader_size, PU_STATIC, NULL);
					W_ReadLumpPwad(wadnum, shader_lumpnum, shader_source);

					HWD.pfnLoadCustomShader(shaderxlat[i].id, shader_source, shader_size, (shadertype == 2));

					Z_Free(shader_source);
					Z_Free(shader_lumpname);
				}
			}

skip_field:
			stoken = strtok(NULL, "\r\n= ");
			linenum++;
		}
	}

	HWD.pfnInitCustomShaders();

	Z_Free(line);
	return;
}

#endif // HWRENDER

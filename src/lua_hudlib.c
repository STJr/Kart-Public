// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 2014-2016 by John "JTE" Muniz.
// Copyright (C) 2014-2018 by Sonic Team Junior.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  lua_hudlib.c
/// \brief custom HUD rendering library for Lua scripting

#include "doomdef.h"
#ifdef HAVE_BLUA
#include "r_defs.h"
#include "r_local.h"
#include "st_stuff.h" // hudinfo[]
#include "g_game.h"
#include "i_video.h" // rendermode
#include "p_local.h" // camera_t
#include "screen.h" // screen width/height
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

#include "lua_script.h"
#include "lua_libs.h"
#include "lua_hud.h"

#define HUDONLY if (!hud_running) return luaL_error(L, "HUD rendering code should not be called outside of rendering hooks!");

boolean hud_running = false;
static UINT8 hud_enabled[(hud_MAX/8)+1];

static UINT8 hudAvailable; // hud hooks field

static UINT8 camnum = 1;

// must match enum hud in lua_hud.h
static const char *const hud_disable_options[] = {
	"stagetitle",
	"textspectator",

	"time",
	"gametypeinfo",	// Bumpers / Karma / Laps depending on gametype
	"minimap",
	"item",
	"position",
	"check",		// "CHECK" f-zero indicator
	"minirankings",	// Gametype rankings to the left
	"battlerankingsbumpers",	// bumper drawer for battle. Useful if you want to make a custom battle gamemode without bumpers being involved.
	"battlefullscreen",			// battlefullscreen func (WAIT, ATTACK OR PROTECT ...)
	"battlecomebacktimer",		// come back timer in battlefullscreen
	"wanted",
	"speedometer",
	"freeplay",
	"rankings",
	NULL};

enum hudinfo {
	hudinfo_x = 0,
	hudinfo_y
};

static const char *const hudinfo_opt[] = {
	"x",
	"y",
	NULL};

enum patch {
	patch_valid = 0,
	patch_width,
	patch_height,
	patch_leftoffset,
	patch_topoffset
};
static const char *const patch_opt[] = {
	"valid",
	"width",
	"height",
	"leftoffset",
	"topoffset",
	NULL};

enum hudhook {
	hudhook_game = 0,
	hudhook_scores
};
static const char *const hudhook_opt[] = {
	"game",
	"scores",
	NULL};

// alignment types for v.drawString
enum align {
	align_left = 0,
	align_center,
	align_right,
	align_fixed,
	align_small,
	align_smallright,
	align_thin,
	align_thinright
};
static const char *const align_opt[] = {
	"left",
	"center",
	"right",
	"fixed",
	"small",
	"small-right",
	"thin",
	"thin-right",
	NULL};

// width types for v.stringWidth
enum widtht {
	widtht_normal = 0,
	widtht_small,
	widtht_thin
};
static const char *const widtht_opt[] = {
	"normal",
	"small",
	"thin",
	NULL};

enum cameraf {
	camera_chase = 0,
	camera_aiming,
	camera_x,
	camera_y,
	camera_z,
	camera_angle,
	camera_subsector,
	camera_floorz,
	camera_ceilingz,
	camera_radius,
	camera_height,
	camera_momx,
	camera_momy,
	camera_momz,
	camera_pnum
};


static const char *const camera_opt[] = {
	"chase",
	"aiming",
	"x",
	"y",
	"z",
	"angle",
	"subsector",
	"floorz",
	"ceilingz",
	"radius",
	"height",
	"momx",
	"momy",
	"momz",
	"pnum",
	NULL};

static int lib_getHudInfo(lua_State *L)
{
	UINT32 i;
	lua_remove(L, 1);

	i = luaL_checkinteger(L, 1);
	if (i >= NUMHUDITEMS)
		return luaL_error(L, "hudinfo[] index %d out of range (0 - %d)", i, NUMHUDITEMS-1);
	LUA_PushUserdata(L, &hudinfo[i], META_HUDINFO);
	return 1;
}

static int lib_hudinfolen(lua_State *L)
{
	lua_pushinteger(L, NUMHUDITEMS);
	return 1;
}

static int hudinfo_get(lua_State *L)
{
	hudinfo_t *info = *((hudinfo_t **)luaL_checkudata(L, 1, META_HUDINFO));
	enum hudinfo field = luaL_checkoption(L, 2, hudinfo_opt[0], hudinfo_opt);
	I_Assert(info != NULL); // huditems are always valid

	switch(field)
	{
	case hudinfo_x:
		lua_pushinteger(L, info->x);
		break;
	case hudinfo_y:
		lua_pushinteger(L, info->y);
		break;
	}
	return 1;
}

static int hudinfo_set(lua_State *L)
{
	hudinfo_t *info = *((hudinfo_t **)luaL_checkudata(L, 1, META_HUDINFO));
	enum hudinfo field = luaL_checkoption(L, 2, hudinfo_opt[0], hudinfo_opt);
	I_Assert(info != NULL);

	switch(field)
	{
	case hudinfo_x:
		info->x = (INT32)luaL_checkinteger(L, 3);
		break;
	case hudinfo_y:
		info->y = (INT32)luaL_checkinteger(L, 3);
		break;
	}
	return 0;
}

static int hudinfo_num(lua_State *L)
{
	hudinfo_t *info = *((hudinfo_t **)luaL_checkudata(L, 1, META_HUDINFO));
	lua_pushinteger(L, info-hudinfo);
	return 1;
}

static int colormap_get(lua_State *L)
{
	return luaL_error(L, "colormap is not a struct.");
}

static int patch_get(lua_State *L)
{
	patch_t *patch = *((patch_t **)luaL_checkudata(L, 1, META_PATCH));
	enum patch field = luaL_checkoption(L, 2, NULL, patch_opt);

	// patches are CURRENTLY always valid, expected to be cached with PU_STATIC
	// this may change in the future, so patch.valid still exists
	I_Assert(patch != NULL);

	switch (field)
	{
	case patch_valid:
		lua_pushboolean(L, patch != NULL);
		break;
	case patch_width:
		lua_pushinteger(L, SHORT(patch->width));
		break;
	case patch_height:
		lua_pushinteger(L, SHORT(patch->height));
		break;
	case patch_leftoffset:
		lua_pushinteger(L, SHORT(patch->leftoffset));
		break;
	case patch_topoffset:
		lua_pushinteger(L, SHORT(patch->topoffset));
		break;
	}
	return 1;
}

static int patch_set(lua_State *L)
{
	return luaL_error(L, LUA_QL("patch_t") " struct cannot be edited by Lua.");
}

static int camera_get(lua_State *L)
{
	camera_t *cam = *((camera_t **)luaL_checkudata(L, 1, META_CAMERA));
	enum cameraf field = luaL_checkoption(L, 2, NULL, camera_opt);

	// cameras should always be valid unless I'm a nutter
	I_Assert(cam != NULL);

	switch (field)
	{
	case camera_chase:
		lua_pushboolean(L, cam->chase);
		break;
	case camera_aiming:
		lua_pushinteger(L, cam->aiming);
		break;
	case camera_x:
		lua_pushinteger(L, cam->x);
		break;
	case camera_y:
		lua_pushinteger(L, cam->y);
		break;
	case camera_z:
		lua_pushinteger(L, cam->z);
		break;
	case camera_angle:
		lua_pushinteger(L, cam->angle);
		break;
	case camera_subsector:
		LUA_PushUserdata(L, cam->subsector, META_SUBSECTOR);
		break;
	case camera_floorz:
		lua_pushinteger(L, cam->floorz);
		break;
	case camera_ceilingz:
		lua_pushinteger(L, cam->ceilingz);
		break;
	case camera_radius:
		lua_pushinteger(L, cam->radius);
		break;
	case camera_height:
		lua_pushinteger(L, cam->height);
		break;
	case camera_momx:
		lua_pushinteger(L, cam->momx);
		break;
	case camera_momy:
		lua_pushinteger(L, cam->momy);
		break;
	case camera_momz:
		lua_pushinteger(L, cam->momz);
		break;
	case camera_pnum:
		lua_pushinteger(L, camnum);
		break;
	}
	return 1;
}

//
// lib_draw
//

static int libd_patchExists(lua_State *L)
{
	HUDONLY
	lua_pushboolean(L, W_LumpExists(luaL_checkstring(L, 1)));
	return 1;
}

static int libd_cachePatch(lua_State *L)
{
	HUDONLY
	LUA_PushUserdata(L, W_CachePatchName(luaL_checkstring(L, 1), PU_STATIC), META_PATCH);
	return 1;
}

static int libd_draw(lua_State *L)
{
	INT32 x, y, flags;
	patch_t *patch;
	UINT8 *colormap = NULL;
	huddrawlist_h list;

	HUDONLY
	x = luaL_checkinteger(L, 1);
	y = luaL_checkinteger(L, 2);
	patch = *((patch_t **)luaL_checkudata(L, 3, META_PATCH));
	flags = luaL_optinteger(L, 4, 0);
	if (!lua_isnoneornil(L, 5))
		colormap = *((UINT8 **)luaL_checkudata(L, 5, META_COLORMAP));

	flags &= ~V_PARAMMASK; // Don't let crashes happen.

	lua_getfield(L, LUA_REGISTRYINDEX, "HUD_DRAW_LIST");
	list = (huddrawlist_h) lua_touserdata(L, -1);
	lua_pop(L, 1);

	if (LUA_HUD_IsDrawListValid(list))
		LUA_HUD_AddDraw(list, x, y, patch, flags, colormap);
	else
		V_DrawFixedPatch(x<<FRACBITS, y<<FRACBITS, FRACUNIT, flags, patch, colormap);
	return 0;
}

static int libd_drawScaled(lua_State *L)
{
	fixed_t x, y, scale;
	INT32 flags;
	patch_t *patch;
	UINT8 *colormap = NULL;
	huddrawlist_h list;

	HUDONLY
	x = luaL_checkinteger(L, 1);
	y = luaL_checkinteger(L, 2);
	scale = luaL_checkinteger(L, 3);
	if (scale < 0)
		return luaL_error(L, "negative scale");
	patch = *((patch_t **)luaL_checkudata(L, 4, META_PATCH));
	flags = luaL_optinteger(L, 5, 0);
	if (!lua_isnoneornil(L, 6))
		colormap = *((UINT8 **)luaL_checkudata(L, 6, META_COLORMAP));

	flags &= ~V_PARAMMASK; // Don't let crashes happen.

	lua_getfield(L, LUA_REGISTRYINDEX, "HUD_DRAW_LIST");
	list = (huddrawlist_h) lua_touserdata(L, -1);
	lua_pop(L, 1);

	if (LUA_HUD_IsDrawListValid(list))
		LUA_HUD_AddDrawScaled(list, x, y, scale, patch, flags, colormap);
	else
		V_DrawFixedPatch(x, y, scale, flags, patch, colormap);
	return 0;
}

// KART: draw patch on minimap from x, y coordinates on the map
static int libd_drawOnMinimap(lua_State *L)
{
	fixed_t x, y, scale;	// coordinates of the object
	patch_t *patch;	// patch we want to draw
	UINT8 *colormap = NULL;	// do we want to colormap this patch?
	boolean centered;	// the patch is centered and doesn't need readjusting on x/y coordinates.
	huddrawlist_h list;

	// variables used to replicate k_kart's mmap drawer:
	INT32 lumpnum;
	patch_t *AutomapPic;
	INT32 mx, my;
	INT32 splitflags, minimaptrans;

	// base position of the minimap which also takes splits into account:
	INT32 MM_X, MM_Y;

	// variables used for actually drawing the icon:
	fixed_t amnumxpos, amnumypos;
	INT32 amxpos, amypos;

	node_t *bsp = &nodes[numnodes-1];
	fixed_t maxx, minx, maxy, miny;

	fixed_t mapwidth, mapheight;
	fixed_t xoffset, yoffset;
	fixed_t xscale, yscale, zoom;
	fixed_t patchw, patchh;

	HUDONLY	// only run this function in hud hooks
	x = luaL_checkinteger(L, 1);
	y = luaL_checkinteger(L, 2);
	scale = luaL_checkinteger(L, 3);
	patch = *((patch_t **)luaL_checkudata(L, 4, META_PATCH));
	if (!lua_isnoneornil(L, 5))
		colormap = *((UINT8 **)luaL_checkudata(L, 5, META_COLORMAP));
	centered = lua_optboolean(L, 6);

	// replicate exactly what source does for its minimap drawer; AKA hardcoded garbo.

	// first, check what position the mmap is supposed to be in (pasted from k_kart.c):
	MM_X = BASEVIDWIDTH - 50;		// 270
	MM_Y = (BASEVIDHEIGHT/2)-16; //  84
	if (splitscreen)
	{
		MM_Y = (BASEVIDHEIGHT/2);
		if (splitscreen > 1)	// 3P : bottom right
		{
			MM_X = (3*BASEVIDWIDTH/4);
			MM_Y = (3*BASEVIDHEIGHT/4);

			if (splitscreen > 2) // 4P: centered
			{
				MM_X = (BASEVIDWIDTH/2);
				MM_Y = (BASEVIDHEIGHT/2);
			}
		}
	}

	// splitscreen flags
	splitflags = (splitscreen == 3 ? 0 : V_SNAPTORIGHT);	// flags should only be 0 when it's centered (4p split)

	// translucency:
	if (timeinmap > 105)
	{
		minimaptrans = cv_kartminimap.value;
		if (timeinmap <= 113)
			minimaptrans = ((((INT32)timeinmap) - 105)*minimaptrans)/(113-105);
		if (!minimaptrans)
			return 0;
	}
	else
		return 0;


	minimaptrans = ((10-minimaptrans)<<FF_TRANSSHIFT);
	splitflags |= minimaptrans;

	if (!(splitscreen == 2))
	{
		splitflags &= ~minimaptrans;
		splitflags |= V_HUDTRANSHALF;
	}

	splitflags &= ~V_HUDTRANSHALF;
	splitflags |= V_HUDTRANS;

	// Draw the HUD only when playing in a level.
	// hu_stuff needs this, unlike st_stuff.
	if (gamestate != GS_LEVEL)
		return 0;

	if (stplyr != &players[displayplayers[0]])
		return 0;

	lumpnum = W_CheckNumForName(va("%sR", G_BuildMapName(gamemap)));

	if (lumpnum != -1)
		AutomapPic = W_CachePatchName(va("%sR", G_BuildMapName(gamemap)), PU_HUDGFX);
	else
		return 0; // no pic, just get outta here

	mx = MM_X - (AutomapPic->width/2);
	my = MM_Y - (AutomapPic->height/2);

	// let offsets transfer to the heads, too!
	if (encoremode)
		mx += SHORT(AutomapPic->leftoffset);
	else
		mx -= SHORT(AutomapPic->leftoffset);
	my -= SHORT(AutomapPic->topoffset);

	// now that we have replicated this behavior, we can draw an icon from our supplied x, y coordinates by replicating k_kart.c's totally understandable uncommented code!!!

	// get map boundaries using nodes
	maxx = maxy = INT32_MAX;
	minx = miny = INT32_MIN;
	minx = bsp->bbox[0][BOXLEFT];
	maxx = bsp->bbox[0][BOXRIGHT];
	miny = bsp->bbox[0][BOXBOTTOM];
	maxy = bsp->bbox[0][BOXTOP];

	if (bsp->bbox[1][BOXLEFT] < minx)
		minx = bsp->bbox[1][BOXLEFT];
	if (bsp->bbox[1][BOXRIGHT] > maxx)
		maxx = bsp->bbox[1][BOXRIGHT];
	if (bsp->bbox[1][BOXBOTTOM] < miny)
		miny = bsp->bbox[1][BOXBOTTOM];
	if (bsp->bbox[1][BOXTOP] > maxy)
		maxy = bsp->bbox[1][BOXTOP];

	// You might be wondering why these are being bitshift here
	// it's because mapwidth and height would otherwise overflow for maps larger than half the size possible...
	// map boundaries and sizes will ALWAYS be whole numbers thankfully
	// later calculations take into consideration that these are actually not in terms of FRACUNIT though
	minx >>= FRACBITS;
	maxx >>= FRACBITS;
	miny >>= FRACBITS;
	maxy >>= FRACBITS;

	// these are our final map boundaries:
	mapwidth = maxx - minx;
	mapheight = maxy - miny;

	// These should always be small enough to be bitshift back right now
	xoffset = (minx + mapwidth/2)<<FRACBITS;
	yoffset = (miny + mapheight/2)<<FRACBITS;

	xscale = FixedDiv(AutomapPic->width, mapwidth);
	yscale = FixedDiv(AutomapPic->height, mapheight);
	zoom = FixedMul(min(xscale, yscale), FRACUNIT-FRACUNIT/20);

	amnumxpos = (FixedMul(x, zoom) - FixedMul(xoffset, zoom));
	amnumypos = -(FixedMul(y, zoom) - FixedMul(yoffset, zoom));

	if (encoremode)
		amnumxpos = -amnumxpos;

	// scale patch coords
	patchw = patch->width*scale /2;
	patchh = patch->height*scale /2;

	if (centered)
		patchw = patchh = 0;	// patch is supposedly already centered, don't butt in.

	amxpos = amnumxpos + ((mx + AutomapPic->width/2)<<FRACBITS) - patchw;
	amypos = amnumypos + ((my + AutomapPic->height/2)<<FRACBITS) - patchh;

	// and NOW we can FINALLY DRAW OUR GOD DAMN PATCH :V
	lua_getfield(L, LUA_REGISTRYINDEX, "HUD_DRAW_LIST");
	list = (huddrawlist_h) lua_touserdata(L, -1);
	lua_pop(L, 1);

	if (LUA_HUD_IsDrawListValid(list))
		LUA_HUD_AddDrawScaled(list, amxpos, amypos, scale, patch, splitflags, colormap);
	else
		V_DrawFixedPatch(amxpos, amypos, scale, splitflags, patch, colormap);

	
	return 0;
}

static int libd_drawNum(lua_State *L)
{
	INT32 x, y, flags, num;
	huddrawlist_h list;

	HUDONLY
	x = luaL_checkinteger(L, 1);
	y = luaL_checkinteger(L, 2);
	num = luaL_checkinteger(L, 3);
	flags = luaL_optinteger(L, 4, 0);
	flags &= ~V_PARAMMASK; // Don't let crashes happen.

	lua_getfield(L, LUA_REGISTRYINDEX, "HUD_DRAW_LIST");
	list = (huddrawlist_h) lua_touserdata(L, -1);
	lua_pop(L, 1);

	if (LUA_HUD_IsDrawListValid(list))
		LUA_HUD_AddDrawNum(list, x, y, num, flags);
	else
		V_DrawTallNum(x, y, flags, num);
	return 0;
}

static int libd_drawPaddedNum(lua_State *L)
{
	INT32 x, y, flags, num, digits;
	huddrawlist_h list;

	HUDONLY
	x = luaL_checkinteger(L, 1);
	y = luaL_checkinteger(L, 2);
	num = labs(luaL_checkinteger(L, 3));
	digits = luaL_optinteger(L, 4, 2);
	flags = luaL_optinteger(L, 5, 0);
	flags &= ~V_PARAMMASK; // Don't let crashes happen.

	lua_getfield(L, LUA_REGISTRYINDEX, "HUD_DRAW_LIST");
	list = (huddrawlist_h) lua_touserdata(L, -1);
	lua_pop(L, 1);

	if (LUA_HUD_IsDrawListValid(list))
		LUA_HUD_AddDrawPaddedNum(list, x, y, num, digits, flags);
	else
		V_DrawPaddedTallNum(x, y, flags, num, digits);
	return 0;
}


static int libd_drawPingNum(lua_State *L)
{
	INT32 x, y, flags, num;
	const UINT8 *colormap = NULL;
	HUDONLY
	x = luaL_checkinteger(L, 1);
	y = luaL_checkinteger(L, 2);
	num = luaL_checkinteger(L, 3);
	flags = luaL_optinteger(L, 4, 0);
	flags &= ~V_PARAMMASK; // Don't let crashes happen.
	if (!lua_isnoneornil(L, 5))
		colormap = *((UINT8 **)luaL_checkudata(L, 5, META_COLORMAP));

	V_DrawPingNum(x, y, flags, num, colormap);
	return 0;
}

static int libd_drawFill(lua_State *L)
{
	huddrawlist_h list;
	INT32 x = luaL_optinteger(L, 1, 0);
	INT32 y = luaL_optinteger(L, 2, 0);
	INT32 w = luaL_optinteger(L, 3, BASEVIDWIDTH);
	INT32 h = luaL_optinteger(L, 4, BASEVIDHEIGHT);
	INT32 c = luaL_optinteger(L, 5, 31);

	HUDONLY

	lua_getfield(L, LUA_REGISTRYINDEX, "HUD_DRAW_LIST");
	list = (huddrawlist_h) lua_touserdata(L, -1);
	lua_pop(L, 1);

	if (LUA_HUD_IsDrawListValid(list))
		LUA_HUD_AddDrawFill(list, x, y, w, h, c);
	else
		V_DrawFill(x, y, w, h, c);
	return 0;
}

static int libd_fadeScreen(lua_State *L)
{
	UINT16 color = luaL_checkinteger(L, 1);
	UINT8 strength = luaL_checkinteger(L, 2);
	const UINT8 maxstrength = ((color & 0xFF00) ? 32 : 10);
	huddrawlist_h list;

	HUDONLY

	if (!strength)
		return 0;

	if (strength > maxstrength)
		return luaL_error(L, "%s fade strength %d out of range (0 - %d)", ((color & 0xFF00) ? "COLORMAP" : "TRANSMAP"), strength, maxstrength);

	lua_getfield(L, LUA_REGISTRYINDEX, "HUD_DRAW_LIST");
	list = (huddrawlist_h) lua_touserdata(L, -1);
	lua_pop(L, 1);

	if (strength == maxstrength) // Allow as a shortcut for drawfill...
	{
		if (LUA_HUD_IsDrawListValid(list))
			LUA_HUD_AddDrawFill(list, 0, 0, BASEVIDWIDTH, BASEVIDHEIGHT, ((color & 0xFF00) ? 31 : color));
		else
			V_DrawFill(0, 0, BASEVIDWIDTH, BASEVIDHEIGHT, ((color & 0xFF00) ? 31 : color));

		return 0;
	}

	if (LUA_HUD_IsDrawListValid(list))
		LUA_HUD_AddFadeScreen(list, color, strength);
	else
		V_DrawFadeScreen(color, strength);

	return 0;
}

static int libd_drawString(lua_State *L)
{
	huddrawlist_h list;
	fixed_t x = luaL_checkinteger(L, 1);
	fixed_t y = luaL_checkinteger(L, 2);
	const char *str = luaL_checkstring(L, 3);
	INT32 flags = luaL_optinteger(L, 4, V_ALLOWLOWERCASE);
	enum align align = luaL_checkoption(L, 5, "left", align_opt);

	flags &= ~V_PARAMMASK; // Don't let crashes happen.

	HUDONLY

	lua_getfield(L, LUA_REGISTRYINDEX, "HUD_DRAW_LIST");
	list = (huddrawlist_h) lua_touserdata(L, -1);
	lua_pop(L, 1);

	// okay, sorry, this is kind of ugly
	if (LUA_HUD_IsDrawListValid(list))
		LUA_HUD_AddDrawString(list, x, y, str, flags, align);
	else
	switch(align)
	{
	// hu_font
	case align_left:
		V_DrawString(x, y, flags, str);
		break;
	case align_center:
		V_DrawCenteredString(x, y, flags, str);
		break;
	case align_right:
		V_DrawRightAlignedString(x, y, flags, str);
		break;
	case align_fixed:
		V_DrawStringAtFixed(x, y, flags, str);
		break;
	// hu_font, 0.5x scale
	case align_small:
		V_DrawSmallString(x, y, flags, str);
		break;
	case align_smallright:
		V_DrawRightAlignedSmallString(x, y, flags, str);
		break;
	// tny_font
	case align_thin:
		V_DrawThinString(x, y, flags, str);
		break;
	case align_thinright:
		V_DrawRightAlignedThinString(x, y, flags, str);
		break;
	}
	return 0;
}

static int libd_drawKartString(lua_State *L)
{
	fixed_t x = luaL_checkinteger(L, 1);
	fixed_t y = luaL_checkinteger(L, 2);
	const char *str = luaL_checkstring(L, 3);
	INT32 flags = luaL_optinteger(L, 4, V_ALLOWLOWERCASE);
	huddrawlist_h list;

	flags &= ~V_PARAMMASK; // Don't let crashes happen.

	HUDONLY

	lua_getfield(L, LUA_REGISTRYINDEX, "HUD_DRAW_LIST");
	list = (huddrawlist_h) lua_touserdata(L, -1);
	lua_pop(L, 1);

	if (LUA_HUD_IsDrawListValid(list))
		LUA_HUD_AddDrawKartString(list, x, y, str, flags);
	else
		V_DrawKartString(x, y, flags, str);
	return 0;
}

static int libd_stringWidth(lua_State *L)
{
	const char *str = luaL_checkstring(L, 1);
	INT32 flags = luaL_optinteger(L, 2, V_ALLOWLOWERCASE);
	enum widtht widtht = luaL_checkoption(L, 3, "normal", widtht_opt);

	HUDONLY
	switch(widtht)
	{
	case widtht_normal: // hu_font
		lua_pushinteger(L, V_StringWidth(str, flags));
		break;
	case widtht_small: // hu_font, 0.5x scale
		lua_pushinteger(L, V_SmallStringWidth(str, flags));
		break;
	case widtht_thin: // tny_font
		lua_pushinteger(L, V_ThinStringWidth(str, flags));
		break;
	}
	return 1;
}

static int libd_getColormap(lua_State *L)
{
	INT32 skinnum = TC_DEFAULT;
	skincolors_t color = luaL_optinteger(L, 2, 0);
	UINT8* colormap = NULL;
	HUDONLY
	if (lua_isnoneornil(L, 1))
		; // defaults to TC_DEFAULT
	else if (lua_type(L, 1) == LUA_TNUMBER) // skin number
	{
		skinnum = (INT32)luaL_checkinteger(L, 1);
		if (skinnum < TC_BLINK || skinnum >= MAXSKINS)
			return luaL_error(L, "skin number %d is out of range (%d - %d)", skinnum, TC_BLINK, MAXSKINS-1);
	}
	else // skin name
	{
		const char *skinname = luaL_checkstring(L, 1);
		INT32 i = R_SkinAvailable(skinname);
		if (i != -1) // if -1, just default to TC_DEFAULT as above
			skinnum = i;
	}

	// all was successful above, now we generate the colormap at last!

	colormap = R_GetTranslationColormap(skinnum, color, GTC_CACHE);
	LUA_PushUserdata(L, colormap, META_COLORMAP); // push as META_COLORMAP userdata, specifically for patches to use!
	return 1;
}

static int libd_width(lua_State *L)
{
	HUDONLY
	lua_pushinteger(L, vid.width); // push screen width
	return 1;
}

static int libd_height(lua_State *L)
{
	HUDONLY
	lua_pushinteger(L, vid.height); // push screen height
	return 1;
}

static int libd_dupx(lua_State *L)
{
	HUDONLY
	lua_pushinteger(L, vid.dupx); // push integral scale (patch scale)
	lua_pushfixed(L, vid.fdupx); // push fixed point scale (position scale)
	return 2;
}

static int libd_dupy(lua_State *L)
{
	HUDONLY
	lua_pushinteger(L, vid.dupy); // push integral scale (patch scale)
	lua_pushfixed(L, vid.fdupy); // push fixed point scale (position scale)
	return 2;
}

static int libd_renderer(lua_State *L)
{
	HUDONLY
	switch (rendermode) {
		case render_opengl: lua_pushliteral(L, "opengl");   break; // OpenGL renderer
		case render_soft:   lua_pushliteral(L, "software"); break; // Software renderer
		default:            lua_pushliteral(L, "none");     break; // render_none (for dedicated), in case there's any reason this should be run
	}
	return 1;
}

// 30/10/18 Lat': Get cv_translucenthud's value for HUD rendering as a normal V_xxTRANS int
// Could as well be thrown in global vars for ease of access but I guess it makes sense for it to be a HUD fn
static int libd_getlocaltransflag(lua_State *L)
{
	HUDONLY
	lua_pushinteger(L, (10-cv_translucenthud.value)*V_10TRANS);	// A bit weird that it's called "translucenthud" yet 10 is fully opaque :V
	return 1;
}

static luaL_Reg lib_draw[] = {
	{"patchExists", libd_patchExists},
	{"cachePatch", libd_cachePatch},
	{"draw", libd_draw},
	{"drawScaled", libd_drawScaled},
	{"drawNum", libd_drawNum},
	{"drawPaddedNum", libd_drawPaddedNum},
	{"drawPingNum", libd_drawPingNum},
	{"drawFill", libd_drawFill},
	{"fadeScreen", libd_fadeScreen},
	{"drawString", libd_drawString},
	{"drawKartString", libd_drawKartString},
	{"stringWidth", libd_stringWidth},
	{"getColormap", libd_getColormap},
	{"width", libd_width},
	{"height", libd_height},
	{"dupx", libd_dupx},
	{"dupy", libd_dupy},
	{"renderer", libd_renderer},
	{"localTransFlag", libd_getlocaltransflag},
	{"drawOnMinimap", libd_drawOnMinimap},
	{NULL, NULL}
};

//
// lib_hud
//

// enable vanilla HUD element
static int lib_hudenable(lua_State *L)
{
	enum hud option = luaL_checkoption(L, 1, NULL, hud_disable_options);
	hud_enabled[option/8] |= 1<<(option%8);
	return 0;
}

// disable vanilla HUD element
static int lib_huddisable(lua_State *L)
{
	enum hud option = luaL_checkoption(L, 1, NULL, hud_disable_options);
	hud_enabled[option/8] &= ~(1<<(option%8));
	return 0;
}

// 30/10/18: Lat': How come this wasn't here before?
static int lib_hudenabled(lua_State *L)
{
	enum hud option = luaL_checkoption(L, 1, NULL, hud_disable_options);
	if (hud_enabled[option/8] & (1<<(option%8)))
		lua_pushboolean(L, true);
	else
		lua_pushboolean(L, false);

	return 1;
}

// add a HUD element for rendering
static int lib_hudadd(lua_State *L)
{
	enum hudhook field;

	luaL_checktype(L, 1, LUA_TFUNCTION);
	field = luaL_checkoption(L, 2, "game", hudhook_opt);

	lua_getfield(L, LUA_REGISTRYINDEX, "HUD");
	I_Assert(lua_istable(L, -1));
	lua_rawgeti(L, -1, field+2); // HUD[2+]
	I_Assert(lua_istable(L, -1));
	lua_remove(L, -2);

	lua_pushvalue(L, 1);
	lua_rawseti(L, -2, (int)(lua_objlen(L, -2) + 1));

	hudAvailable |= 1<<field;
	return 0;
}

static luaL_Reg lib_hud[] = {
	{"enable", lib_hudenable},
	{"disable", lib_huddisable},
	{"enabled", lib_hudenabled},
	{"add", lib_hudadd},
	{NULL, NULL}
};

//
//
//

int LUA_HudLib(lua_State *L)
{
	memset(hud_enabled, 0xff, (hud_MAX/8)+1);

	lua_newtable(L); // HUD registry table
		lua_newtable(L);
		luaL_register(L, NULL, lib_draw);
		lua_rawseti(L, -2, 1); // HUD[1] = lib_draw

		lua_newtable(L);
		lua_rawseti(L, -2, 2); // HUD[2] = game rendering functions array

		lua_newtable(L);
		lua_rawseti(L, -2, 3); // HUD[2] = scores rendering functions array
	lua_setfield(L, LUA_REGISTRYINDEX, "HUD");

	luaL_newmetatable(L, META_HUDINFO);
		lua_pushcfunction(L, hudinfo_get);
		lua_setfield(L, -2, "__index");

		lua_pushcfunction(L, hudinfo_set);
		lua_setfield(L, -2, "__newindex");

		lua_pushcfunction(L, hudinfo_num);
		lua_setfield(L, -2, "__len");
	lua_pop(L,1);

	lua_newuserdata(L, 0);
		lua_createtable(L, 0, 2);
			lua_pushcfunction(L, lib_getHudInfo);
			lua_setfield(L, -2, "__index");

			lua_pushcfunction(L, lib_hudinfolen);
			lua_setfield(L, -2, "__len");
		lua_setmetatable(L, -2);
	lua_setglobal(L, "hudinfo");

	luaL_newmetatable(L, META_COLORMAP);
		lua_pushcfunction(L, colormap_get);
		lua_setfield(L, -2, "__index");
	lua_pop(L,1);

	luaL_newmetatable(L, META_PATCH);
		lua_pushcfunction(L, patch_get);
		lua_setfield(L, -2, "__index");

		lua_pushcfunction(L, patch_set);
		lua_setfield(L, -2, "__newindex");
	lua_pop(L,1);

	luaL_newmetatable(L, META_CAMERA);
		lua_pushcfunction(L, camera_get);
		lua_setfield(L, -2, "__index");
	lua_pop(L,1);

	luaL_register(L, "hud", lib_hud);
	return 0;
}

boolean LUA_HudEnabled(enum hud option)
{
	if (!gL || hud_enabled[option/8] & (1<<(option%8)))
		return true;
	return false;
}

// Hook for HUD rendering
void LUAh_GameHUD(player_t *stplayr, huddrawlist_h list)
{
	if (!gL || !(hudAvailable & (1<<hudhook_game)))
		return;
	
	lua_pushlightuserdata(gL, list);
	lua_setfield(gL, LUA_REGISTRYINDEX, "HUD_DRAW_LIST");

	hud_running = true;
	lua_pop(gL, -1);

	lua_getfield(gL, LUA_REGISTRYINDEX, "HUD");
	I_Assert(lua_istable(gL, -1));
	lua_rawgeti(gL, -1, 2); // HUD[2] = rendering funcs
	I_Assert(lua_istable(gL, -1));

	lua_rawgeti(gL, -2, 1); // HUD[1] = lib_draw
	I_Assert(lua_istable(gL, -1));
	lua_remove(gL, -3); // pop HUD
	LUA_PushUserdata(gL, stplayr, META_PLAYER);

	if (splitscreen > 2 && stplayr == &players[displayplayers[3]])
	{
		LUA_PushUserdata(gL, &camera[3], META_CAMERA);
		camnum = 4;
	}
	else if (splitscreen > 1 && stplayr == &players[displayplayers[2]])
	{
		LUA_PushUserdata(gL, &camera[2], META_CAMERA);
		camnum = 3;
	}
	else if (splitscreen && stplayr == &players[displayplayers[1]])
	{
		LUA_PushUserdata(gL, &camera[1], META_CAMERA);
		camnum = 2;
	}
	else
	{
		LUA_PushUserdata(gL, &camera[0], META_CAMERA);
		camnum = 1;
	}

	lua_pushnil(gL);
	while (lua_next(gL, -5) != 0) {
		lua_pushvalue(gL, -5); // graphics library (HUD[1])
		lua_pushvalue(gL, -5); // stplayr
		lua_pushvalue(gL, -5); // camera
		LUA_Call(gL, 3);
	}
	lua_pop(gL, -1);
	hud_running = false;

	lua_pushlightuserdata(gL, NULL);
	lua_setfield(gL, LUA_REGISTRYINDEX, "HUD_DRAW_LIST");
}

void LUAh_ScoresHUD(huddrawlist_h list)
{
	if (!gL || !(hudAvailable & (1<<hudhook_scores)))
		return;
	
	lua_pushlightuserdata(gL, list);
	lua_setfield(gL, LUA_REGISTRYINDEX, "HUD_DRAW_LIST");

	hud_running = true;
	lua_pop(gL, -1);

	lua_getfield(gL, LUA_REGISTRYINDEX, "HUD");
	I_Assert(lua_istable(gL, -1));
	lua_rawgeti(gL, -1, 3); // HUD[3] = rendering funcs
	I_Assert(lua_istable(gL, -1));

	lua_rawgeti(gL, -2, 1); // HUD[1] = lib_draw
	I_Assert(lua_istable(gL, -1));
	lua_remove(gL, -3); // pop HUD
	lua_pushnil(gL);
	while (lua_next(gL, -3) != 0) {
		lua_pushvalue(gL, -3); // graphics library (HUD[1])
		LUA_Call(gL, 1);
	}
	lua_pop(gL, -1);
	hud_running = false;

	lua_pushlightuserdata(gL, NULL);
	lua_setfield(gL, LUA_REGISTRYINDEX, "HUD_DRAW_LIST");
}

#endif

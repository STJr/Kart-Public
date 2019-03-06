// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 2012-2016 by John "JTE" Muniz.
// Copyright (C) 2012-2018 by Sonic Team Junior.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  lua_script.h
/// \brief Lua scripting basics

#ifdef HAVE_BLUA

#include "m_fixed.h"
#include "doomtype.h"
#include "d_player.h"

#include "blua/lua.h"
#include "blua/lualib.h"
#include "blua/lauxlib.h"

#define lua_optboolean(L, i) (!lua_isnoneornil(L, i) && lua_toboolean(L, i))
#define lua_opttrueboolean(L, i) (lua_isnoneornil(L, i) || lua_toboolean(L, i))

// fixed_t casting
// TODO add some distinction between fixed numbers and integer numbers
// for at least the purpose of printing and maybe math.
#define luaL_checkfixed(L, i) luaL_checkinteger(L, i)
#define lua_pushfixed(L, f) lua_pushinteger(L, f)

// angle_t casting
// TODO deal with signedness
#define luaL_checkangle(L, i) ((angle_t)luaL_checkinteger(L, i))
#define lua_pushangle(L, a) lua_pushinteger(L, a)

#ifdef _DEBUG
void LUA_ClearExtVars(void);
#endif

void LUA_LoadLump(UINT16 wad, UINT16 lump);
#ifdef LUA_ALLOW_BYTECODE
void LUA_DumpFile(const char *filename);
#endif
fixed_t LUA_EvalMath(const char *word);
void LUA_PushUserdata(lua_State *L, void *data, const char *meta);
void LUA_InvalidateUserdata(void *data);
void LUA_InvalidateLevel(void);
void LUA_InvalidateMapthings(void);
void LUA_InvalidatePlayer(player_t *player);
void LUA_Step(void);
void LUA_Archive(void);
void LUA_UnArchive(void);
void Got_Luacmd(UINT8 **cp, INT32 playernum); // lua_consolelib.c
void LUA_CVarChanged(const char *name); // lua_consolelib.c
int Lua_optoption(lua_State *L, int narg,
	const char *def, const char *const lst[]);
void LUAh_NetArchiveHook(lua_CFunction archFunc);

// Console wrapper
void COM_Lua_f(void);

#define LUA_Call(L,a)\
{\
	if (lua_pcall(L, a, 0, 0)) {\
		CONS_Alert(CONS_WARNING,"%s\n",lua_tostring(L,-1));\
		lua_pop(L, 1);\
	}\
}

/* 	Invalidity error, happens when you access something that doesn't exist. (eg: "player.mo.variable" when "player.mo" doesn't exist)
	Except it's a pretty stupid error because the game knows to stop proceeding when something doesn't exist (it would crash otherwise)
	These P_MobjWasRemoved (.valid in Lua equivalent) checks are NECESSARY in hardcode because the game would crash otherwise, but obviously NOT HERE since the game does it FOR US.
	And also has no impact on the script aside of returning nil when something dosn't exist, obviously.
	All it does is turn something like
	"if player.mo.mobj.variable" into "if player.mo and player.mo.valid and player.mo.mobj and player.mo.mobj.valid and player.mo.mobj.variable"
	No, I am not stretching that, and yes, it looks absolutely ridiculous.
	Thus, it's hidden under debug now, and returns 0 otherwise (this is what lua_error ends up doing)
*/
#define LUA_ErrInvalid(L, type) (cv_debug & DBG_LUA) ? (luaL_error(L, "accessed " type " doesn't exist anymore, returning nil.")) : (0);

// Deprecation warnings
// Shows once upon use. Then doesn't show again.
#define LUA_Deprecated(L,this_func,use_instead)\
{\
	static UINT8 seen = 0;\
	if (!seen) {\
		seen = 1;\
		CONS_Alert(CONS_WARNING,"\"%s\" is deprecated and will be removed.\nUse \"%s\" instead.\n", this_func, use_instead);\
	}\
}

// Warnings about incorrect function usage.
// Shows once, then never again, like deprecation
#define LUA_UsageWarning(L, warningmsg)\
{\
	static UINT8 seen = 0;\
	if (!seen) {\
		seen = 1;\
		CONS_Alert(CONS_WARNING,"%s\n", warningmsg);\
	}\
}

#endif

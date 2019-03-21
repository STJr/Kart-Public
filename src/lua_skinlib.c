// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 2014-2016 by John "JTE" Muniz.
// Copyright (C) 2014-2018 by Sonic Team Junior.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  lua_skinlib.c
/// \brief player skin structure library for Lua scripting

#include "doomdef.h"
#ifdef HAVE_BLUA
#include "fastcmp.h"
#include "r_things.h"
#include "sounds.h"

#include "lua_script.h"
#include "lua_libs.h"

enum skin {
	skin_valid = 0,
	skin_name,
	skin_spritedef,
	skin_wadnum,
	skin_flags,
	skin_realname,
	skin_hudname,
	skin_facerank,
	skin_facewant,
	skin_facemmap,
	// SRB2kart
	skin_kartspeed,
	skin_kartweight,
	//
	skin_starttranscolor,
	skin_prefcolor,
	skin_highresscale,
	skin_soundsid
};
static const char *const skin_opt[] = {
	"valid",
	"name",
	"spritedef",
	"wadnum",
	"flags",
	"realname",
	"hudname",
	"facerank",
	"facewant",
	"facemmap",
	// SRB2kart
	"kartspeed",
	"kartweight",
	//
	"starttranscolor",
	"prefcolor",
	"highresscale",
	"soundsid",
	NULL};

#define UNIMPLEMENTED luaL_error(L, LUA_QL("skin_t") " field " LUA_QS " is not implemented for Lua and cannot be accessed.", skin_opt[field])

static int skin_get(lua_State *L)
{
	skin_t *skin = *((skin_t **)luaL_checkudata(L, 1, META_SKIN));
	enum skin field = luaL_checkoption(L, 2, NULL, skin_opt);
	INT32 i;

	// skins are always valid, only added, never removed
	I_Assert(skin != NULL);

	switch (field)
	{
	case skin_valid:
		lua_pushboolean(L, skin != NULL);
		break;
	case skin_name:
		lua_pushstring(L, skin->name);
		break;
	case skin_spritedef:
		return UNIMPLEMENTED;
	case skin_wadnum:
		// !!WARNING!! May differ between clients due to music wads, therefore NOT NETWORK SAFE
		return UNIMPLEMENTED;
	case skin_flags:
		lua_pushinteger(L, skin->flags);
		break;
	case skin_realname:
		lua_pushstring(L, skin->realname);
		break;
	case skin_hudname:
		lua_pushstring(L, skin->hudname);
		break;
	case skin_facerank:
		for (i = 0; i < 8; i++)
			if (!skin->facerank[i])
				break;
		lua_pushlstring(L, skin->facerank, i);
		break;
	case skin_facewant:
		for (i = 0; i < 8; i++)
			if (!skin->facewant[i])
				break;
		lua_pushlstring(L, skin->facewant, i);
		break;
	case skin_facemmap:
		for (i = 0; i < 8; i++)
			if (!skin->facemmap[i])
				break;
		lua_pushlstring(L, skin->facemmap, i);
		break;
	// SRB2kart
	case skin_kartspeed:
		lua_pushinteger(L, skin->kartspeed);
		break;
	case skin_kartweight:
		lua_pushinteger(L, skin->kartweight);
		break;
	//
	case skin_starttranscolor:
		lua_pushinteger(L, skin->starttranscolor);
		break;
	case skin_prefcolor:
		lua_pushinteger(L, skin->prefcolor);
		break;
	case skin_highresscale:
		lua_pushinteger(L, skin->highresscale);
		break;
	case skin_soundsid:
		LUA_PushUserdata(L, skin->soundsid, META_SOUNDSID);
		break;
	}
	return 1;
}

static int skin_set(lua_State *L)
{
	return luaL_error(L, LUA_QL("skin_t") " struct cannot be edited by Lua.");
}

static int skin_num(lua_State *L)
{
	skin_t *skin = *((skin_t **)luaL_checkudata(L, 1, META_SKIN));

	// skins are always valid, only added, never removed
	I_Assert(skin != NULL);

	lua_pushinteger(L, skin-skins);
	return 1;
}

static int lib_iterateSkins(lua_State *L)
{
	INT32 i;

	if (lua_gettop(L) < 2)
	{
		//return luaL_error(L, "Don't call skins.iterate() directly, use it as 'for skin in skins.iterate do <block> end'.");
		lua_pushcfunction(L, lib_iterateSkins);
		return 1;
	}

	lua_settop(L, 2);
	lua_remove(L, 1); // state is unused.

	if (!lua_isnil(L, 1))
		i = (INT32)(*((skin_t **)luaL_checkudata(L, 1, META_SKIN)) - skins) + 1;
	else
		i = 0;

	// skins are always valid, only added, never removed
	if (i < numskins)
	{
		LUA_PushUserdata(L, &skins[i], META_SKIN);
		return 1;
	}

	return 0;
}

static int lib_getSkin(lua_State *L)
{
	const char *field;
	INT32 i;

	// find skin by number
	if (lua_type(L, 2) == LUA_TNUMBER)
	{
		i = luaL_checkinteger(L, 2);
		if (i < 0 || i >= MAXSKINS)
			return luaL_error(L, "skins[] index %d out of range (0 - %d)", i, MAXSKINS-1);
		if (i >= numskins)
			return 0;
		LUA_PushUserdata(L, &skins[i], META_SKIN);
		return 1;
	}

	field = luaL_checkstring(L, 2);

	// special function iterate
	if (fastcmp(field,"iterate"))
	{
		lua_pushcfunction(L, lib_iterateSkins);
		return 1;
	}

	// find skin by name
	for (i = 0; i < numskins; i++)
		if (fastcmp(skins[i].name, field))
		{
			LUA_PushUserdata(L, &skins[i], META_SKIN);
			return 1;
		}

	return 0;
}

static int lib_numSkins(lua_State *L)
{
	lua_pushinteger(L, numskins);
	return 1;
}

// soundsid, i -> soundsid[i]
static int soundsid_get(lua_State *L)
{
	sfxenum_t *soundsid = *((sfxenum_t **)luaL_checkudata(L, 1, META_SOUNDSID));
	skinsound_t i = luaL_checkinteger(L, 2);
	if (i >= NUMSKINSOUNDS)
		return luaL_error(L, LUA_QL("skinsound_t") " cannot be %u", i);
	lua_pushinteger(L, soundsid[i]);
	return 1;
}

// #soundsid -> NUMSKINSOUNDS
static int soundsid_num(lua_State *L)
{
	lua_pushinteger(L, NUMSKINSOUNDS);
	return 1;
}

int LUA_SkinLib(lua_State *L)
{
	luaL_newmetatable(L, META_SKIN);
		lua_pushcfunction(L, skin_get);
		lua_setfield(L, -2, "__index");

		lua_pushcfunction(L, skin_set);
		lua_setfield(L, -2, "__newindex");

		lua_pushcfunction(L, skin_num);
		lua_setfield(L, -2, "__len");
	lua_pop(L,1);

	luaL_newmetatable(L, META_SOUNDSID);
		lua_pushcfunction(L, soundsid_get);
		lua_setfield(L, -2, "__index");

		lua_pushcfunction(L, soundsid_num);
		lua_setfield(L, -2, "__len");
	lua_pop(L,1);

	lua_newuserdata(L, 0);
		lua_createtable(L, 0, 2);
			lua_pushcfunction(L, lib_getSkin);
			lua_setfield(L, -2, "__index");

			lua_pushcfunction(L, lib_numSkins);
			lua_setfield(L, -2, "__len");
		lua_setmetatable(L, -2);
	lua_setglobal(L, "skins");

	return 0;
}

#endif

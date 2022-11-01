// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 1999-2018 by Sonic Team Junior.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  d_clisrv.c
/// \brief SRB2 Network game communication and protocol, all OS independent parts.

#if !defined (UNDER_CE)
#include <time.h>
#endif
#ifdef __GNUC__
#include <unistd.h> //for unlink
#endif

#include "i_time.h"
#include "i_net.h"
#include "i_system.h"
#include "i_video.h"
#include "d_net.h"
#include "d_netfil.h" // fileneedednum
#include "d_main.h"
#include "g_game.h"
#include "hu_stuff.h"
#include "keys.h"
#include "g_input.h" // JOY1
#include "m_menu.h"
#include "console.h"
#include "d_netfil.h"
#include "byteptr.h"
#include "p_saveg.h"
#include "z_zone.h"
#include "p_local.h"
#include "m_misc.h"
#include "am_map.h"
#include "m_random.h"
#include "mserv.h"
#include "y_inter.h"
#include "r_local.h"
#include "m_argv.h"
#include "p_setup.h"
#include "lzf.h"
#include "lua_script.h"
#include "lua_hook.h"
#include "k_kart.h"
#include "s_sound.h" // sfx_syfail

#ifdef CLIENT_LOADINGSCREEN
// cl loading screen
#include "v_video.h"
#include "f_finale.h"
#endif

#ifdef _XBOX
#include "sdl12/SRB2XBOX/xboxhelp.h"
#endif

#ifdef HAVE_DISCORDRPC
#include "discord.h"
#endif

//
// NETWORKING
//
// gametic is the tic about to (or currently being) run
// Server:
//   maketic is the tic that hasn't had control made for it yet
//   nettics is the tic for each node
//   firstticstosend is the lowest value of nettics
// Client:
//   neededtic is the tic needed by the client to run the game
//   firstticstosend is used to optimize a condition
// Normally maketic >= gametic > 0

#define MAX_REASONLENGTH 30
#define FORCECLOSE 0x8000

boolean server = true; // true or false but !server == client
#define client (!server)
boolean nodownload = false;
boolean serverrunning = false;
INT32 serverplayer = 0;
char motd[254], server_context[8]; // Message of the Day, Unique Context (even without Mumble support)

// Server specific vars
UINT8 playernode[MAXPLAYERS];

// Minimum timeout for sending the savegame
// The actual timeout will be longer depending on the savegame length
tic_t jointimeout = (3*TICRATE);
static boolean sendingsavegame[MAXNETNODES]; // Are we sending the savegame?
static tic_t freezetimeout[MAXNETNODES]; // Until when can this node freeze the server before getting a timeout?

UINT16 pingmeasurecount = 1;
UINT32 realpingtable[MAXPLAYERS]; //the base table of ping where an average will be sent to everyone.
UINT32 playerpingtable[MAXPLAYERS]; //table of player latency values.
SINT8 nodetoplayer[MAXNETNODES];
SINT8 nodetoplayer2[MAXNETNODES]; // say the numplayer for this node if any (splitscreen)
SINT8 nodetoplayer3[MAXNETNODES]; // say the numplayer for this node if any (splitscreen == 2)
SINT8 nodetoplayer4[MAXNETNODES]; // say the numplayer for this node if any (splitscreen == 3)
UINT8 playerpernode[MAXNETNODES]; // used specialy for splitscreen
boolean nodeingame[MAXNETNODES]; // set false as nodes leave game

tic_t servermaxping = 20; // server's max delay, in frames. Defaults to 20
static tic_t nettics[MAXNETNODES]; // what tic the client have received
static tic_t supposedtics[MAXNETNODES]; // nettics prevision for smaller packet
static UINT8 nodewaiting[MAXNETNODES];
static tic_t firstticstosend; // min of the nettics
static tic_t tictoclear = 0; // optimize d_clearticcmd
static tic_t maketic;

static INT16 consistancy[TICQUEUE];

// Resynching shit!
static UINT32 resynch_score[MAXNETNODES]; // "score" for kicking -- if this gets too high then cfail kick
static UINT16 resynch_delay[MAXNETNODES]; // delay time before the player can be considered to have desynched
static UINT32 resynch_status[MAXNETNODES]; // 0 bit means synched for that player, 1 means possibly desynched
static UINT8 resynch_sent[MAXNETNODES][MAXPLAYERS]; // what synch packets have we attempted to send to the player
static UINT8 resynch_inprogress[MAXNETNODES];
static UINT8 resynch_local_inprogress = false; // WE are desynched and getting packets to fix it.
static UINT8 player_joining = false;
UINT8 hu_resynching = 0;

// kart, true when a player is connecting or disconnecting so that the gameplay has stopped in its tracks
UINT8 hu_stopped = 0;

// Client specific
static ticcmd_t localcmds;
static ticcmd_t localcmds2;
static ticcmd_t localcmds3;
static ticcmd_t localcmds4;
static boolean cl_packetmissed;
// here it is for the secondary local player (splitscreen)
static UINT8 mynode; // my address pointofview server

static UINT8 localtextcmd[MAXTEXTCMD];
static UINT8 localtextcmd2[MAXTEXTCMD]; // splitscreen
static UINT8 localtextcmd3[MAXTEXTCMD]; // splitscreen == 2
static UINT8 localtextcmd4[MAXTEXTCMD]; // splitscreen == 3
static tic_t neededtic;
SINT8 servernode = 0; // the number of the server node
char connectedservername[MAXSERVERNAME];
/// \brief do we accept new players?
/// \todo WORK!
boolean acceptnewnode = true;

boolean serverisfull = false; //lets us be aware if the server was full after we check files, but before downloading, so we can ask if the user still wants to download or not
tic_t firstconnectattempttime = 0;

// engine

// Must be a power of two
#define TEXTCMD_HASH_SIZE 4

typedef struct textcmdplayer_s
{
	INT32 playernum;
	UINT8 cmd[MAXTEXTCMD];
	struct textcmdplayer_s *next;
} textcmdplayer_t;

typedef struct textcmdtic_s
{
	tic_t tic;
	textcmdplayer_t *playercmds[TEXTCMD_HASH_SIZE];
	struct textcmdtic_s *next;
} textcmdtic_t;

ticcmd_t netcmds[TICQUEUE][MAXPLAYERS];
static textcmdtic_t *textcmds[TEXTCMD_HASH_SIZE] = {NULL};


consvar_t cv_showjoinaddress = {"showjoinaddress", "On", CV_SAVE, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL};

static CV_PossibleValue_t playbackspeed_cons_t[] = {{1, "MIN"}, {10, "MAX"}, {0, NULL}};
consvar_t cv_playbackspeed = {"playbackspeed", "1", 0, playbackspeed_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};

consvar_t cv_httpsource = {"http_source", "", CV_SAVE, NULL, NULL, 0, NULL, NULL, 0, 0, NULL};

consvar_t cv_kicktime = {"kicktime", "10", CV_SAVE, CV_Unsigned, NULL, 0, NULL, NULL, 0, 0, NULL};

static inline void *G_DcpyTiccmd(void* dest, const ticcmd_t* src, const size_t n)
{
	const size_t d = n / sizeof(ticcmd_t);
	const size_t r = n % sizeof(ticcmd_t);
	UINT8 *ret = dest;

	if (r)
		M_Memcpy(dest, src, n);
	else if (d)
		G_MoveTiccmd(dest, src, d);
	return ret+n;
}

static inline void *G_ScpyTiccmd(ticcmd_t* dest, void* src, const size_t n)
{
	const size_t d = n / sizeof(ticcmd_t);
	const size_t r = n % sizeof(ticcmd_t);
	UINT8 *ret = src;

	if (r)
		M_Memcpy(dest, src, n);
	else if (d)
		G_MoveTiccmd(dest, src, d);
	return ret+n;
}



// Some software don't support largest packet
// (original sersetup, not exactely, but the probability of sending a packet
// of 512 bytes is like 0.1)
UINT16 software_MAXPACKETLENGTH;

/** Guesses the value of a tic from its lowest byte and from maketic
  *
  * \param low The lowest byte of the tic value
  * \param basetic The last full tic value to compare against
  * \return The full tic value
  *
  */
tic_t ExpandTics(INT32 low, tic_t basetic)
{
	INT32 delta;

	delta = low - (basetic & UINT8_MAX);

	if (delta >= -64 && delta <= 64)
		return (basetic & ~UINT8_MAX) + low;
	else if (delta > 64)
		return (basetic & ~UINT8_MAX) - 256 + low;
	else //if (delta < -64)
		return (basetic & ~UINT8_MAX) + 256 + low;
}

// -----------------------------------------------------------------
// Some extra data function for handle textcmd buffer
// -----------------------------------------------------------------

static void (*listnetxcmd[MAXNETXCMD])(UINT8 **p, INT32 playernum);

void RegisterNetXCmd(netxcmd_t id, void (*cmd_f)(UINT8 **p, INT32 playernum))
{
#ifdef PARANOIA
	if (id >= MAXNETXCMD)
		I_Error("Command id %d too big", id);
	if (listnetxcmd[id] != 0)
		I_Error("Command id %d already used", id);
#endif
	listnetxcmd[id] = cmd_f;
}

void SendNetXCmd(netxcmd_t id, const void *param, size_t nparam)
{
	if (localtextcmd[0]+2+nparam > MAXTEXTCMD)
	{
		// for future reference: if (cv_debug) != debug disabled.
		CONS_Alert(CONS_ERROR, M_GetText("NetXCmd buffer full, cannot add netcmd %d! (size: %d, needed: %s)\n"), id, localtextcmd[0], sizeu1(nparam));
		return;
	}
	localtextcmd[0]++;
	localtextcmd[localtextcmd[0]] = (UINT8)id;
	if (param && nparam)
	{
		M_Memcpy(&localtextcmd[localtextcmd[0]+1], param, nparam);
		localtextcmd[0] = (UINT8)(localtextcmd[0] + (UINT8)nparam);
	}
}

// splitscreen player
void SendNetXCmd2(netxcmd_t id, const void *param, size_t nparam)
{
	if (localtextcmd2[0]+2+nparam > MAXTEXTCMD)
	{
		I_Error("No more place in the buffer for netcmd %d\n",id);
		return;
	}
	localtextcmd2[0]++;
	localtextcmd2[localtextcmd2[0]] = (UINT8)id;
	if (param && nparam)
	{
		M_Memcpy(&localtextcmd2[localtextcmd2[0]+1], param, nparam);
		localtextcmd2[0] = (UINT8)(localtextcmd2[0] + (UINT8)nparam);
	}
}

void SendNetXCmd3(netxcmd_t id, const void *param, size_t nparam)
{
	if (localtextcmd3[0]+2+nparam > MAXTEXTCMD)
	{
		I_Error("No more place in the buffer for netcmd %d\n",id);
		return;
	}
	localtextcmd3[0]++;
	localtextcmd3[localtextcmd3[0]] = (UINT8)id;
	if (param && nparam)
	{
		M_Memcpy(&localtextcmd3[localtextcmd3[0]+1], param, nparam);
		localtextcmd3[0] = (UINT8)(localtextcmd3[0] + (UINT8)nparam);
	}
}

void SendNetXCmd4(netxcmd_t id, const void *param, size_t nparam)
{
	if (localtextcmd4[0]+2+nparam > MAXTEXTCMD)
	{
		I_Error("No more place in the buffer for netcmd %d\n",id);
		return;
	}
	localtextcmd4[0]++;
	localtextcmd4[localtextcmd4[0]] = (UINT8)id;
	if (param && nparam)
	{
		M_Memcpy(&localtextcmd4[localtextcmd4[0]+1], param, nparam);
		localtextcmd4[0] = (UINT8)(localtextcmd4[0] + (UINT8)nparam);
	}
}

UINT8 GetFreeXCmdSize(void)
{
	// -1 for the size and another -1 for the ID.
	return (UINT8)(localtextcmd[0] - 2);
}

// Frees all textcmd memory for the specified tic
static void D_FreeTextcmd(tic_t tic)
{
	textcmdtic_t **tctprev = &textcmds[tic & (TEXTCMD_HASH_SIZE - 1)];
	textcmdtic_t *textcmdtic = *tctprev;

	while (textcmdtic && textcmdtic->tic != tic)
	{
		tctprev = &textcmdtic->next;
		textcmdtic = textcmdtic->next;
	}

	if (textcmdtic)
	{
		INT32 i;

		// Remove this tic from the list.
		*tctprev = textcmdtic->next;

		// Free all players.
		for (i = 0; i < TEXTCMD_HASH_SIZE; i++)
		{
			textcmdplayer_t *textcmdplayer = textcmdtic->playercmds[i];

			while (textcmdplayer)
			{
				textcmdplayer_t *tcpnext = textcmdplayer->next;
				Z_Free(textcmdplayer);
				textcmdplayer = tcpnext;
			}
		}

		// Free this tic's own memory.
		Z_Free(textcmdtic);
	}
}

// Gets the buffer for the specified ticcmd, or NULL if there isn't one
static UINT8* D_GetExistingTextcmd(tic_t tic, INT32 playernum)
{
	textcmdtic_t *textcmdtic = textcmds[tic & (TEXTCMD_HASH_SIZE - 1)];
	while (textcmdtic && textcmdtic->tic != tic) textcmdtic = textcmdtic->next;

	// Do we have an entry for the tic? If so, look for player.
	if (textcmdtic)
	{
		textcmdplayer_t *textcmdplayer = textcmdtic->playercmds[playernum & (TEXTCMD_HASH_SIZE - 1)];
		while (textcmdplayer && textcmdplayer->playernum != playernum) textcmdplayer = textcmdplayer->next;

		if (textcmdplayer) return textcmdplayer->cmd;
	}

	return NULL;
}

// Gets the buffer for the specified ticcmd, creating one if necessary
static UINT8* D_GetTextcmd(tic_t tic, INT32 playernum)
{
	textcmdtic_t *textcmdtic = textcmds[tic & (TEXTCMD_HASH_SIZE - 1)];
	textcmdtic_t **tctprev = &textcmds[tic & (TEXTCMD_HASH_SIZE - 1)];
	textcmdplayer_t *textcmdplayer, **tcpprev;

	// Look for the tic.
	while (textcmdtic && textcmdtic->tic != tic)
	{
		tctprev = &textcmdtic->next;
		textcmdtic = textcmdtic->next;
	}

	// If we don't have an entry for the tic, make it.
	if (!textcmdtic)
	{
		textcmdtic = *tctprev = Z_Calloc(sizeof (textcmdtic_t), PU_STATIC, NULL);
		textcmdtic->tic = tic;
	}

	tcpprev = &textcmdtic->playercmds[playernum & (TEXTCMD_HASH_SIZE - 1)];
	textcmdplayer = *tcpprev;

	// Look for the player.
	while (textcmdplayer && textcmdplayer->playernum != playernum)
	{
		tcpprev = &textcmdplayer->next;
		textcmdplayer = textcmdplayer->next;
	}

	// If we don't have an entry for the player, make it.
	if (!textcmdplayer)
	{
		textcmdplayer = *tcpprev = Z_Calloc(sizeof (textcmdplayer_t), PU_STATIC, NULL);
		textcmdplayer->playernum = playernum;
	}

	return textcmdplayer->cmd;
}

static void ExtraDataTicker(void)
{
	INT32 i;

	for (i = 0; i < MAXPLAYERS; i++)
		if (playeringame[i] || i == 0)
		{
			UINT8 *bufferstart = D_GetExistingTextcmd(gametic, i);

			if (bufferstart)
			{
				UINT8 *curpos = bufferstart;
				UINT8 *bufferend = &curpos[curpos[0]+1];

				curpos++;
				while (curpos < bufferend)
				{
					if (*curpos < MAXNETXCMD && listnetxcmd[*curpos])
					{
						const UINT8 id = *curpos;
						curpos++;
						DEBFILE(va("executing x_cmd %s ply %u ", netxcmdnames[id - 1], i));
						(listnetxcmd[id])(&curpos, i);
						DEBFILE("done\n");
					}
					else
					{
						if (server)
						{
							XBOXSTATIC UINT8 buf[3];

							buf[0] = (UINT8)i;
							buf[1] = KICK_MSG_CON_FAIL;
							SendNetXCmd(XD_KICK, &buf, 2);
							DEBFILE(va("player %d kicked [gametic=%u] reason as follows:\n", i, gametic));
						}
						CONS_Alert(CONS_WARNING, M_GetText("Got unknown net command [%s]=%d (max %d)\n"), sizeu1(curpos - bufferstart), *curpos, bufferstart[0]);
						break;
					}
				}
			}
		}

	// If you are a client, you can safely forget the net commands for this tic
	// If you are the server, you need to remember them until every client has been aknowledged,
	// because if you need to resend a PT_SERVERTICS packet, you need to put the commands in it
	if (client)
		D_FreeTextcmd(gametic);
}

static void D_Clearticcmd(tic_t tic)
{
	INT32 i;

	D_FreeTextcmd(tic);

	for (i = 0; i < MAXPLAYERS; i++)
		netcmds[tic%TICQUEUE][i].angleturn = 0;

	DEBFILE(va("clear tic %5u (%2u)\n", tic, tic%TICQUEUE));
}

void D_ResetTiccmds(void)
{
	INT32 i;

	memset(&localcmds, 0, sizeof(ticcmd_t));
	memset(&localcmds2, 0, sizeof(ticcmd_t));
	memset(&localcmds3, 0, sizeof(ticcmd_t));
	memset(&localcmds4, 0, sizeof(ticcmd_t));

	// Reset the net command list
	for (i = 0; i < TEXTCMD_HASH_SIZE; i++)
		while (textcmds[i])
			D_Clearticcmd(textcmds[i]->tic);
}

// -----------------------------------------------------------------
// end of extra data function
// -----------------------------------------------------------------

// -----------------------------------------------------------------
// extra data function for lmps
// -----------------------------------------------------------------

// if extradatabit is set, after the ziped tic you find this:
//
//   type   |  description
// ---------+--------------
//   byte   | size of the extradata
//   byte   | the extradata (xd) bits: see XD_...
//            with this byte you know what parameter folow
// if (xd & XDNAMEANDCOLOR)
//   byte   | color
//   char[MAXPLAYERNAME] | name of the player
// endif
// if (xd & XD_WEAPON_PREF)
//   byte   | original weapon switch: boolean, true if use the old
//          | weapon switch methode
//   char[NUMWEAPONS] | the weapon switch priority
//   byte   | autoaim: true if use the old autoaim system
// endif
/*boolean AddLmpExtradata(UINT8 **demo_point, INT32 playernum)
{
	UINT8 *textcmd = D_GetExistingTextcmd(gametic, playernum);

	if (!textcmd)
		return false;

	M_Memcpy(*demo_point, textcmd, textcmd[0]+1);
	*demo_point += textcmd[0]+1;
	return true;
}

void ReadLmpExtraData(UINT8 **demo_pointer, INT32 playernum)
{
	UINT8 nextra;
	UINT8 *textcmd;

	if (!demo_pointer)
		return;

	textcmd = D_GetTextcmd(gametic, playernum);
	nextra = **demo_pointer;
	M_Memcpy(textcmd, *demo_pointer, nextra + 1);
	// increment demo pointer
	*demo_pointer += nextra + 1;
}*/

// -----------------------------------------------------------------
// end extra data function for lmps
// -----------------------------------------------------------------

// -----------------------------------------------------------------
// resynch player data
// -----------------------------------------------------------------
static inline void resynch_write_player(resynch_pak *rsp, const size_t i)
{
	size_t j;

	rsp->playernum = (UINT8)i;

	// Do not send anything visual related.
	// Only send data that we need to know for physics.
	rsp->playerstate = (UINT8)players[i].playerstate; //playerstate_t
	rsp->pflags = (UINT32)LONG(players[i].pflags); //pflags_t
	rsp->panim  = (UINT8)players[i].panim; //panim_t

	rsp->aiming = (angle_t)LONG(players[i].aiming);
	rsp->currentweapon = LONG(players[i].currentweapon);
	rsp->ringweapons = LONG(players[i].ringweapons);

	for (j = 0; j < NUMPOWERS; ++j)
		rsp->powers[j] = (UINT16)SHORT(players[i].powers[j]);
	for (j = 0; j < NUMKARTSTUFF; ++j)
		rsp->kartstuff[j] = LONG(players[i].kartstuff[j]); // SRB2kart

	rsp->frameangle = (angle_t)LONG(players[i].frameangle); // SRB2kart

	// Score is resynched in the rspfirm resync packet
	rsp->health = 0; // resynched with mo health
	rsp->lives = players[i].lives;
	rsp->continues = players[i].continues;
	rsp->scoreadd = players[i].scoreadd;
	rsp->xtralife = players[i].xtralife;
	rsp->pity = players[i].pity;

	rsp->skincolor = players[i].skincolor;
	rsp->skin = LONG(players[i].skin);
	// Just in case Lua does something like
	// modify these at runtime
	// SRB2kart
	rsp->kartspeed = (UINT8)players[i].kartspeed;
	rsp->kartweight = (UINT8)players[i].kartweight;
	//
	rsp->charflags = (UINT32)LONG(players[i].charflags);

	rsp->speed = (fixed_t)LONG(players[i].speed);
	rsp->jumping = players[i].jumping;
	rsp->secondjump = players[i].secondjump;
	rsp->fly1 = players[i].fly1;
	rsp->glidetime = (tic_t)LONG(players[i].glidetime);
	rsp->climbing = players[i].climbing;
	rsp->deadtimer = players[i].deadtimer;
	rsp->exiting = (tic_t)LONG(players[i].exiting);
	rsp->homing = players[i].homing;
	rsp->skidtime = (tic_t)LONG(players[i].skidtime);
	rsp->cmomx = (fixed_t)LONG(players[i].cmomx);
	rsp->cmomy = (fixed_t)LONG(players[i].cmomy);
	rsp->rmomx = (fixed_t)LONG(players[i].rmomx);
	rsp->rmomy = (fixed_t)LONG(players[i].rmomy);

	rsp->weapondelay = LONG(players[i].weapondelay);
	rsp->tossdelay = LONG(players[i].tossdelay);

	rsp->starpostx = SHORT(players[i].starpostx);
	rsp->starposty = SHORT(players[i].starposty);
	rsp->starpostz = SHORT(players[i].starpostz);
	rsp->starpostnum = LONG(players[i].starpostnum);
	rsp->starposttime = (tic_t)LONG(players[i].starposttime);
	rsp->starpostangle = (angle_t)LONG(players[i].starpostangle);

	rsp->maxlink = LONG(players[i].maxlink);
	rsp->dashspeed = (fixed_t)LONG(players[i].dashspeed);
	rsp->dashtime = LONG(players[i].dashtime);
	rsp->angle_pos = (angle_t)LONG(players[i].angle_pos);
	rsp->old_angle_pos = (angle_t)LONG(players[i].old_angle_pos);
	rsp->bumpertime = (tic_t)LONG(players[i].bumpertime);
	rsp->flyangle = LONG(players[i].flyangle);
	rsp->drilltimer = (tic_t)LONG(players[i].drilltimer);
	rsp->linkcount = LONG(players[i].linkcount);
	rsp->linktimer = (tic_t)LONG(players[i].linktimer);
	rsp->anotherflyangle = LONG(players[i].anotherflyangle);
	rsp->nightstime = (tic_t)LONG(players[i].nightstime);
	rsp->drillmeter = LONG(players[i].drillmeter);
	rsp->drilldelay = players[i].drilldelay;
	rsp->bonustime = players[i].bonustime;
	rsp->mare = players[i].mare;
	rsp->lastsidehit = SHORT(players[i].lastsidehit);
	rsp->lastlinehit = SHORT(players[i].lastlinehit);

	rsp->losstime = (tic_t)LONG(players[i].losstime);
	rsp->timeshit = players[i].timeshit;
	rsp->onconveyor = LONG(players[i].onconveyor);

	rsp->jointime = (tic_t)LONG(players[i].jointime);
	rsp->spectatorreentry = (tic_t)LONG(players[i].spectatorreentry);

	rsp->grieftime = (tic_t)LONG(players[i].grieftime);
	rsp->griefstrikes = players[i].griefstrikes;

	rsp->splitscreenindex = players[i].splitscreenindex;

	rsp->hasmo = false;
	//Transfer important mo information if the player has a body.
	//This lets us resync players even if they are dead.
	if (!players[i].mo)
		return;
	rsp->hasmo = true;

	rsp->health = LONG(players[i].mo->health);

	rsp->angle = (angle_t)LONG(players[i].mo->angle);
	rsp->x = (fixed_t)LONG(players[i].mo->x);
	rsp->y = (fixed_t)LONG(players[i].mo->y);
	rsp->z = (fixed_t)LONG(players[i].mo->z);
	rsp->momx = (fixed_t)LONG(players[i].mo->momx);
	rsp->momy = (fixed_t)LONG(players[i].mo->momy);
	rsp->momz = (fixed_t)LONG(players[i].mo->momz);
	rsp->friction = (fixed_t)LONG(players[i].mo->friction);
	rsp->movefactor = (fixed_t)LONG(players[i].mo->movefactor);

	rsp->tics = LONG(players[i].mo->tics);
	rsp->statenum = (statenum_t)LONG(players[i].mo->state-states); // :(
	rsp->flags = (UINT32)LONG(players[i].mo->flags);
	rsp->flags2 = (UINT32)LONG(players[i].mo->flags2);
	rsp->eflags = (UINT16)SHORT(players[i].mo->eflags);

	rsp->radius = (fixed_t)LONG(players[i].mo->radius);
	rsp->height = (fixed_t)LONG(players[i].mo->height);
	rsp->scale = (fixed_t)LONG(players[i].mo->scale);
	rsp->destscale = (fixed_t)LONG(players[i].mo->destscale);
	rsp->scalespeed = (fixed_t)LONG(players[i].mo->scalespeed);
}

static void resynch_read_player(resynch_pak *rsp)
{
	INT32 i = rsp->playernum, j;
	mobj_t *savedmo = players[i].mo;

	// Do not send anything visual related.
	// Only send data that we need to know for physics.
	players[i].playerstate = (UINT8)rsp->playerstate; //playerstate_t
	players[i].pflags = (UINT32)LONG(rsp->pflags); //pflags_t
	players[i].panim  = (UINT8)rsp->panim; //panim_t

	players[i].aiming = (angle_t)LONG(rsp->aiming);
	players[i].currentweapon = LONG(rsp->currentweapon);
	players[i].ringweapons = LONG(rsp->ringweapons);

	for (j = 0; j < NUMPOWERS; ++j)
		players[i].powers[j] = (UINT16)SHORT(rsp->powers[j]);
	for (j = 0; j < NUMKARTSTUFF; ++j)
		players[i].kartstuff[j] = LONG(rsp->kartstuff[j]); // SRB2kart

	players[i].frameangle = (angle_t)LONG(rsp->frameangle); // SRB2kart

	// Score is resynched in the rspfirm resync packet
	players[i].health = rsp->health;
	players[i].lives = rsp->lives;
	players[i].continues = rsp->continues;
	players[i].scoreadd = rsp->scoreadd;
	players[i].xtralife = rsp->xtralife;
	players[i].pity = rsp->pity;

	players[i].skincolor = rsp->skincolor;
	players[i].skin = LONG(rsp->skin);
	// Just in case Lua does something like
	// modify these at runtime
	players[i].kartspeed = (UINT8)rsp->kartspeed;
	players[i].kartweight = (UINT8)rsp->kartweight;

	players[i].charflags = (UINT32)LONG(rsp->charflags);

	players[i].speed = (fixed_t)LONG(rsp->speed);
	players[i].jumping = rsp->jumping;
	players[i].secondjump = rsp->secondjump;
	players[i].fly1 = rsp->fly1;
	players[i].glidetime = (tic_t)LONG(rsp->glidetime);
	players[i].climbing = rsp->climbing;
	players[i].deadtimer = rsp->deadtimer;
	players[i].exiting = (tic_t)LONG(rsp->exiting);
	players[i].homing = rsp->homing;
	players[i].skidtime = (tic_t)LONG(rsp->skidtime);
	players[i].cmomx = (fixed_t)LONG(rsp->cmomx);
	players[i].cmomy = (fixed_t)LONG(rsp->cmomy);
	players[i].rmomx = (fixed_t)LONG(rsp->rmomx);
	players[i].rmomy = (fixed_t)LONG(rsp->rmomy);

	players[i].weapondelay = LONG(rsp->weapondelay);
	players[i].tossdelay = LONG(rsp->tossdelay);

	players[i].starpostx = SHORT(rsp->starpostx);
	players[i].starposty = SHORT(rsp->starposty);
	players[i].starpostz = SHORT(rsp->starpostz);
	players[i].starpostnum = LONG(rsp->starpostnum);
	players[i].starposttime = (tic_t)LONG(rsp->starposttime);
	players[i].starpostangle = (angle_t)LONG(rsp->starpostangle);

	players[i].maxlink = LONG(rsp->maxlink);
	players[i].dashspeed = (fixed_t)LONG(rsp->dashspeed);
	players[i].dashtime = LONG(rsp->dashtime);
	players[i].angle_pos = (angle_t)LONG(rsp->angle_pos);
	players[i].old_angle_pos = (angle_t)LONG(rsp->old_angle_pos);
	players[i].bumpertime = (tic_t)LONG(rsp->bumpertime);
	players[i].flyangle = LONG(rsp->flyangle);
	players[i].drilltimer = (tic_t)LONG(rsp->drilltimer);
	players[i].linkcount = LONG(rsp->linkcount);
	players[i].linktimer = (tic_t)LONG(rsp->linktimer);
	players[i].anotherflyangle = LONG(rsp->anotherflyangle);
	players[i].nightstime = (tic_t)LONG(rsp->nightstime);
	players[i].drillmeter = LONG(rsp->drillmeter);
	players[i].drilldelay = rsp->drilldelay;
	players[i].bonustime = rsp->bonustime;
	players[i].mare = rsp->mare;
	players[i].lastsidehit = SHORT(rsp->lastsidehit);
	players[i].lastlinehit = SHORT(rsp->lastlinehit);

	players[i].losstime = (tic_t)LONG(rsp->losstime);
	players[i].timeshit = rsp->timeshit;
	players[i].onconveyor = LONG(rsp->onconveyor);

	players[i].jointime = (tic_t)LONG(rsp->jointime);
	players[i].spectatorreentry = (tic_t)LONG(rsp->spectatorreentry);

	players[i].grieftime = (tic_t)LONG(rsp->grieftime);
	players[i].griefstrikes = rsp->griefstrikes;

	players[i].splitscreenindex = rsp->splitscreenindex;

	//We get a packet for each player in game.
	if (!playeringame[i])
		return;

	//...but keep old mo even if it is corrupt or null!
	players[i].mo = savedmo;

	//Transfer important mo information if they have a valid mo.
	if (!rsp->hasmo)
		return;

	//server thinks player has a body.
	//Give them a new body that can be then manipulated by the server's info.
	if (!players[i].mo) //client thinks it has no body.
		P_SpawnPlayer(i);

	//At this point, the player should have a body, whether they were respawned or not.
	P_UnsetThingPosition(players[i].mo);
	players[i].mo->health = LONG(rsp->health);

	players[i].mo->angle = (angle_t)LONG(rsp->angle);
	players[i].mo->x = (fixed_t)LONG(rsp->x);
	players[i].mo->y = (fixed_t)LONG(rsp->y);
	players[i].mo->z = (fixed_t)LONG(rsp->z);
	players[i].mo->momx = (fixed_t)LONG(rsp->momx);
	players[i].mo->momy = (fixed_t)LONG(rsp->momy);
	players[i].mo->momz = (fixed_t)LONG(rsp->momz);
	players[i].mo->friction = (fixed_t)LONG(rsp->friction);
	players[i].mo->movefactor = (fixed_t)LONG(rsp->movefactor);

	players[i].mo->tics = LONG(rsp->tics);
	P_SetMobjStateNF(players[i].mo, (statenum_t)LONG(rsp->statenum));
	players[i].mo->flags = (UINT32)LONG(rsp->flags);
	players[i].mo->flags2 = (UINT32)LONG(rsp->flags2);
	players[i].mo->eflags = (UINT16)SHORT(rsp->eflags);

	players[i].mo->radius = (fixed_t)LONG(rsp->radius);
	players[i].mo->height = (fixed_t)LONG(rsp->height);
	// P_SetScale is redundant for this, as all related variables are already restored properly.
	players[i].mo->scale = (fixed_t)LONG(rsp->scale);
	players[i].mo->destscale = (fixed_t)LONG(rsp->destscale);
	players[i].mo->scalespeed = (fixed_t)LONG(rsp->scalespeed);

	// And finally, SET THE MOBJ SKIN damn it.
	players[i].mo->skin = &skins[players[i].skin];
	players[i].mo->color = players[i].skincolor;

	P_SetThingPosition(players[i].mo);
}

static inline void resynch_write_ctf(resynchend_pak *rst)
{
	mobj_t *mflag;
	UINT8 i, j;

	for (i = 0, mflag = redflag; i < 2; ++i, mflag = blueflag)
	{
		rst->flagx[i] = rst->flagy[i] = rst->flagz[i] = 0;
		rst->flagloose[i] = rst->flagflags[i] = 0;
		rst->flagplayer[i] = -1;

		if (!mflag)
		{
			// Should be held by a player
			for (j = 0; j < MAXPLAYERS; ++j)
			{
				// GF_REDFLAG is 1, GF_BLUEFLAG is 2
				// redflag handling is i=0, blueflag is i=1
				// so check for gotflag == (i+1)
				if (!playeringame[j] || players[j].gotflag != (i+1))
					continue;
				rst->flagplayer[i] = (SINT8)j;
				break;
			}
			if (j == MAXPLAYERS) // fine, no I_Error
			{
				CONS_Alert(CONS_ERROR, "One of the flags has gone completely missing...\n");
				rst->flagplayer[i] = -2;
			}
			continue;
		}

		rst->flagx[i] = (fixed_t)LONG(mflag->x);
		rst->flagy[i] = (fixed_t)LONG(mflag->y);
		rst->flagz[i] = (fixed_t)LONG(mflag->z);
		rst->flagflags[i] = LONG(mflag->flags2);
		rst->flagloose[i] = LONG(mflag->fuse); // Dropped or not?
	}
}

static inline void resynch_read_ctf(resynchend_pak *p)
{
	UINT8 i;

	for (i = 0; i < MAXPLAYERS; ++i)
		players[i].gotflag = 0;

	// Red flag
	if (p->flagplayer[0] == -2)
		; // The server doesn't even know what happened to it...
	else if (p->flagplayer[0] != -1) // Held by a player
	{
		if (!playeringame[p->flagplayer[0]])
			 I_Error("Invalid red flag player %d who isn't in the game!", (INT32)p->flagplayer[0]);
		players[p->flagplayer[0]].gotflag = GF_REDFLAG;
		if (redflag)
		{
			P_RemoveMobj(redflag);
			redflag = NULL;
		}
	}
	else
	{
		if (!redflag)
			redflag = P_SpawnMobj(0,0,0,MT_REDFLAG);

		P_UnsetThingPosition(redflag);
		redflag->x = (fixed_t)LONG(p->flagx[0]);
		redflag->y = (fixed_t)LONG(p->flagy[0]);
		redflag->z = (fixed_t)LONG(p->flagz[0]);
		redflag->flags2 = LONG(p->flagflags[0]);
		redflag->fuse = LONG(p->flagloose[0]);
		P_SetThingPosition(redflag);
	}

	// Blue flag
	if (p->flagplayer[1] == -2)
		; // The server doesn't even know what happened to it...
	else if (p->flagplayer[1] != -1) // Held by a player
	{
		if (!playeringame[p->flagplayer[1]])
			 I_Error("Invalid blue flag player %d who isn't in the game!", (INT32)p->flagplayer[1]);
		players[p->flagplayer[1]].gotflag = GF_BLUEFLAG;
		if (blueflag)
		{
			P_RemoveMobj(blueflag);
			blueflag = NULL;
		}
	}
	else
	{
		if (!blueflag)
			blueflag = P_SpawnMobj(0,0,0,MT_BLUEFLAG);

		P_UnsetThingPosition(blueflag);
		blueflag->x = (fixed_t)LONG(p->flagx[1]);
		blueflag->y = (fixed_t)LONG(p->flagy[1]);
		blueflag->z = (fixed_t)LONG(p->flagz[1]);
		blueflag->flags2 = LONG(p->flagflags[1]);
		blueflag->fuse = LONG(p->flagloose[1]);
		P_SetThingPosition(blueflag);
	}
}

static inline void resynch_write_others(resynchend_pak *rst)
{
	UINT8 i;

	rst->ingame = 0;

	for (i = 0; i < MAXPLAYERS; ++i)
	{
		if (!playeringame[i])
		{
			rst->ctfteam[i] = 0;
			rst->score[i] = 0;
			rst->marescore[i] = 0;
			rst->realtime[i] = 0;
			rst->laps[i] = 0;
			continue;
		}

		if (!players[i].spectator)
			rst->ingame |= (1<<i);
		rst->ctfteam[i] = (INT32)LONG(players[i].ctfteam);
		rst->score[i] = (UINT32)LONG(players[i].score);
		rst->marescore[i] = (UINT32)LONG(players[i].marescore);
		rst->realtime[i] = (tic_t)LONG(players[i].realtime);
		rst->laps[i] = players[i].laps;
	}

	// endian safeness
	rst->ingame = (UINT32)LONG(rst->ingame);
}

static inline void resynch_read_others(resynchend_pak *p)
{
	UINT8 i;
	UINT32 loc_ingame = (UINT32)LONG(p->ingame);

	for (i = 0; i < MAXPLAYERS; ++i)
	{
		// We don't care if they're in the game or not, just write all the data.
		players[i].spectator = !(loc_ingame & (1<<i));
		players[i].ctfteam = (INT32)LONG(p->ctfteam[i]); // no, 0 does not mean spectator, at least not in Match
		players[i].score = (UINT32)LONG(p->score[i]);
		players[i].marescore = (UINT32)LONG(p->marescore[i]);
		players[i].realtime = (tic_t)LONG(p->realtime[i]);
		players[i].laps = p->laps[i];
	}
}

static void SV_InitResynchVars(INT32 node)
{
	resynch_delay[node] = TICRATE; // initial one second delay
	resynch_score[node] = 0; // clean slate
	resynch_status[node] = 0x00;
	resynch_inprogress[node] = false;
	memset(resynch_sent[node], 0, MAXPLAYERS);
}

static void SV_RequireResynch(INT32 node)
{
	INT32 i;

	resynch_delay[node] = 10; // Delay before you can fail sync again
	resynch_score[node] += 200; // Add score for initial desynch
	for (i = 0; i < MAXPLAYERS; ++i)
		resynch_status[node] |= (1<<i); // No players assumed synched
	resynch_inprogress[node] = true; // so we know to send a PT_RESYNCHEND after sync

	// Initial setup
	memset(resynch_sent[node], 0, MAXPLAYERS);
	for (i = 0; i < MAXPLAYERS; ++i)
	{
		if (!playeringame[i]) // Player not in game so just drop it from required synch
			resynch_status[node] &= ~(1<<i);
		else if (playernode[i] == node); // instantly update THEIR position
		else // Send at random times based on num players
			resynch_sent[node][i] = M_RandomKey(D_NumPlayers()>>1)+1;
	}
}

static void SV_SendResynch(INT32 node)
{
	INT32 i, j;

	if (!nodeingame[node])
	{
		// player left during resynch
		// so obviously we don't need to do any of this anymore
		resynch_inprogress[node] = false;
		return;
	}

	// resynched?
	if (!resynch_status[node])
	{
		// you are now synched
		resynch_inprogress[node] = false;

		netbuffer->packettype = PT_RESYNCHEND;

		netbuffer->u.resynchend.randomseed = P_GetRandSeed();
		if (gametype == GT_CTF)
			resynch_write_ctf(&netbuffer->u.resynchend);
		resynch_write_others(&netbuffer->u.resynchend);

		HSendPacket(node, true, 0, (sizeof(resynchend_pak)));
		return;
	}

	netbuffer->packettype = PT_RESYNCHING;
	for (i = 0, j = 0; i < MAXPLAYERS; ++i)
	{
		// if already synched don't bother
		if (!(resynch_status[node] & 1<<i))
			continue;

		// waiting for a reply or just waiting in general
		if (resynch_sent[node][i])
		{
			--resynch_sent[node][i];
			continue;
		}

		resynch_write_player(&netbuffer->u.resynchpak, i);
		HSendPacket(node, false, 0, (sizeof(resynch_pak)));

		resynch_sent[node][i] = TICRATE;
		resynch_score[node] += 2; // penalty for send

		if (++j > 3)
			break;
	}

	if (resynch_score[node] > (unsigned)cv_resynchattempts.value*250)
	{
		XBOXSTATIC UINT8 buf[2];
		buf[0] = (UINT8)nodetoplayer[node];
		buf[1] = KICK_MSG_CON_FAIL;
		SendNetXCmd(XD_KICK, &buf, 2);
		resynch_score[node] = 0;
	}
}

static void CL_AcknowledgeResynch(resynch_pak *rsp)
{
	resynch_read_player(rsp);

	netbuffer->packettype = PT_RESYNCHGET;
	netbuffer->u.resynchgot = rsp->playernum;
	HSendPacket(servernode, true, 0, sizeof(UINT8));
}

static void SV_AcknowledgeResynchAck(INT32 node, UINT8 rsg)
{
	if (rsg >= MAXPLAYERS)
		resynch_score[node] += 16384; // lol.
	else
	{
		resynch_status[node] &= ~(1<<rsg);
		--resynch_score[node]; // unpenalize
	}

	// Don't let resynch cause a timeout
	freezetimeout[node] = I_GetTime() + connectiontimeout;
}
// -----------------------------------------------------------------
// end resynch
// -----------------------------------------------------------------

static INT16 Consistancy(void);

#ifndef NONET
#define JOININGAME
#endif

typedef enum
{
	CL_SEARCHING,
	CL_CHECKFILES,
	CL_DOWNLOADFILES,
	CL_ASKJOIN,
	CL_LOADFILES,
	CL_SETUPFILES,
	CL_WAITJOINRESPONSE,
#ifdef JOININGAME
	CL_DOWNLOADSAVEGAME,
#endif
	CL_CONNECTED,
	CL_ABORTED,
	CL_ASKFULLFILELIST,
	CL_CONFIRMCONNECT,
#ifdef HAVE_CURL
	CL_PREPAREHTTPFILES,
	CL_DOWNLOADHTTPFILES,
#endif
	CL_LEGACYREQUESTFAILED,
} cl_mode_t;

static void GetPackets(void);

static cl_mode_t cl_mode = CL_SEARCHING;

#ifdef HAVE_CURL
char http_source[MAX_MIRROR_LENGTH];
#endif

static UINT16 cl_lastcheckedfilecount = 0;	// used for full file list

// Player name send/load

static void CV_SavePlayerNames(UINT8 **p)
{
	INT32 i = 0;
	// Players in game only.
	for (; i < MAXPLAYERS; ++i)
	{
		if (!playeringame[i])
		{
			WRITEUINT8(*p, 0);
			continue;
		}
		WRITESTRING(*p, player_names[i]);
	}
}

static void CV_LoadPlayerNames(UINT8 **p)
{
	INT32 i = 0;
	char tmp_name[MAXPLAYERNAME+1];
	tmp_name[MAXPLAYERNAME] = 0;

	for (; i < MAXPLAYERS; ++i)
	{
		READSTRING(*p, tmp_name);
		if (tmp_name[0] == 0)
			continue;
		if (tmp_name[MAXPLAYERNAME]) // overflow detected
			I_Error("Received bad server config packet when trying to join");
		memcpy(player_names[i], tmp_name, MAXPLAYERNAME+1);
	}
}

#ifdef CLIENT_LOADINGSCREEN

//
// CL_DrawConnectionStatus
//
// Keep the local client informed of our status.
//
static inline void CL_DrawConnectionStatus(void)
{
	INT32 ccstime = I_GetTime();

	// Draw background fade
	V_DrawFadeScreen(0xFF00, 16);

	if (cl_mode != CL_DOWNLOADFILES && cl_mode != CL_LOADFILES && cl_mode != CL_CHECKFILES
#ifdef HAVE_CURL
	&& cl_mode != CL_DOWNLOADHTTPFILES
#endif
	)
	{
		INT32 i, animtime = ((ccstime / 4) & 15) + 16;
		UINT8 palstart = (cl_mode == CL_SEARCHING) ? 128 : 160;
		// 15 pal entries total.
		const char *cltext;

		//Draw bottom box
		M_DrawTextBox(BASEVIDWIDTH/2-128-8, BASEVIDHEIGHT-24-8, 32, 1);
		V_DrawCenteredString(BASEVIDWIDTH/2, BASEVIDHEIGHT-24-24, V_YELLOWMAP, "Press ESC to abort");

		for (i = 0; i < 16; ++i)
			V_DrawFill((BASEVIDWIDTH/2-128) + (i * 16), BASEVIDHEIGHT-24, 16, 8, palstart + ((animtime - i) & 15));

		switch (cl_mode)
		{
#ifdef JOININGAME
			case CL_DOWNLOADSAVEGAME:
				if (lastfilenum != -1)
				{
					cltext = M_GetText("Downloading game state...");
					Net_GetNetStat();
					V_DrawString(BASEVIDWIDTH/2-128, BASEVIDHEIGHT-24, V_20TRANS|V_MONOSPACE,
						va(" %4uK",fileneeded[lastfilenum].currentsize>>10));
					V_DrawRightAlignedString(BASEVIDWIDTH/2+128, BASEVIDHEIGHT-24, V_20TRANS|V_MONOSPACE,
						va("%3.1fK/s ", ((double)getbps)/1024));
				}
				else
					cltext = M_GetText("Waiting to download game state...");
				break;
#endif
			case CL_ASKFULLFILELIST:
			case CL_CONFIRMCONNECT:
			case CL_LEGACYREQUESTFAILED:
				cltext = "";
				break;
			case CL_SETUPFILES:
				cltext = M_GetText("Configuring addons...");
				break;
			case CL_ASKJOIN:
			case CL_WAITJOINRESPONSE:
				if (serverisfull)
					cltext = M_GetText("Server full, waiting for a slot...");
				else
					cltext = M_GetText("Requesting to join...");

				break;
#ifdef HAVE_CURL
			case CL_PREPAREHTTPFILES:
				cltext = M_GetText("Waiting to download files...");
				break;
#endif
			default:
				cltext = M_GetText("Connecting to server...");
				break;
		}
		V_DrawCenteredString(BASEVIDWIDTH/2, BASEVIDHEIGHT-24-32, V_YELLOWMAP, cltext);
	}
	else
	{
		if (cl_mode == CL_CHECKFILES)
		{
			INT32 totalfileslength;
			INT32 checkednum = 0;
			INT32 i;

			V_DrawCenteredString(BASEVIDWIDTH/2, BASEVIDHEIGHT-24-24, V_YELLOWMAP, "Press ESC to abort");

			//ima just count files here
			for (i = 0; i < fileneedednum; i++)
				if (fileneeded[i].status != FS_NOTCHECKED)
					checkednum++;

			// Loading progress
			V_DrawCenteredString(BASEVIDWIDTH/2, BASEVIDHEIGHT-24-32, V_YELLOWMAP, "Checking server addons...");
			totalfileslength = (INT32)((checkednum/(double)(fileneedednum)) * 256);
			M_DrawTextBox(BASEVIDWIDTH/2-128-8, BASEVIDHEIGHT-24-8, 32, 1);
			V_DrawFill(BASEVIDWIDTH/2-128, BASEVIDHEIGHT-24, 256, 8, 175);
			V_DrawFill(BASEVIDWIDTH/2-128, BASEVIDHEIGHT-24, totalfileslength, 8, 160);
			V_DrawCenteredString(BASEVIDWIDTH/2, BASEVIDHEIGHT-24, V_20TRANS|V_MONOSPACE,
				va(" %2u/%2u Files",checkednum,fileneedednum));
		}
		else if (cl_mode == CL_LOADFILES)
		{
			INT32 totalfileslength;
			INT32 loadcompletednum = 0;
			INT32 i;

			V_DrawCenteredString(BASEVIDWIDTH/2, BASEVIDHEIGHT-24-24, V_YELLOWMAP, "Press ESC to abort");

			//ima just count files here
			for (i = 0; i < fileneedednum; i++)
				if (fileneeded[i].status == FS_OPEN)
					loadcompletednum++;

			// Loading progress
			V_DrawCenteredString(BASEVIDWIDTH/2, BASEVIDHEIGHT-24-32, V_YELLOWMAP, "Loading server addons...");
			totalfileslength = (INT32)((loadcompletednum/(double)(fileneedednum)) * 256);
			M_DrawTextBox(BASEVIDWIDTH/2-128-8, BASEVIDHEIGHT-24-8, 32, 1);
			V_DrawFill(BASEVIDWIDTH/2-128, BASEVIDHEIGHT-24, 256, 8, 175);
			V_DrawFill(BASEVIDWIDTH/2-128, BASEVIDHEIGHT-24, totalfileslength, 8, 160);
			V_DrawCenteredString(BASEVIDWIDTH/2, BASEVIDHEIGHT-24, V_20TRANS|V_MONOSPACE,
				va(" %2u/%2u Files",loadcompletednum,fileneedednum));
		}
		else if (lastfilenum != -1)
		{
			INT32 dldlength;
			INT32 totalfileslength;
			UINT32 totaldldsize;
			static char tempname[28];
			fileneeded_t *file = &fileneeded[lastfilenum];
			char *filename = file->filename;

			// Draw the bottom box.
			M_DrawTextBox(BASEVIDWIDTH/2-128-8, BASEVIDHEIGHT-58-8, 32, 1);
			V_DrawCenteredString(BASEVIDWIDTH/2, BASEVIDHEIGHT-58-14, V_YELLOWMAP, "Press ESC to abort");

			Net_GetNetStat();
			dldlength = (INT32)((file->currentsize/(double)file->totalsize) * 256);
			if (dldlength > 256)
				dldlength = 256;
			V_DrawFill(BASEVIDWIDTH/2-128, BASEVIDHEIGHT-58, 256, 8, 175);
			V_DrawFill(BASEVIDWIDTH/2-128, BASEVIDHEIGHT-58, dldlength, 8, 160);

			memset(tempname, 0, sizeof(tempname));
			// offset filename to just the name only part
			filename += strlen(filename) - nameonlylength(filename);

			if (strlen(filename) > sizeof(tempname)-1) // too long to display fully
			{
				size_t endhalfpos = strlen(filename)-10;
				// display as first 14 chars + ... + last 10 chars
				// which should add up to 27 if our math(s) is correct
				snprintf(tempname, sizeof(tempname), "%.14s...%.10s", filename, filename+endhalfpos);
			}
			else // we can copy the whole thing in safely
			{
				strncpy(tempname, filename, sizeof(tempname)-1);
			}

			V_DrawCenteredString(BASEVIDWIDTH/2, BASEVIDHEIGHT-58-30, 0,
				va(M_GetText("%s downloading"), ((cl_mode == CL_DOWNLOADHTTPFILES) ? "\x82""HTTP" : "\x85""Direct")));
			V_DrawCenteredString(BASEVIDWIDTH/2, BASEVIDHEIGHT-58-22, V_YELLOWMAP,
				va(M_GetText("\"%s\""), tempname));
			V_DrawString(BASEVIDWIDTH/2-128, BASEVIDHEIGHT-58, V_20TRANS|V_MONOSPACE,
				va(" %4uK/%4uK",fileneeded[lastfilenum].currentsize>>10,file->totalsize>>10));
			V_DrawRightAlignedString(BASEVIDWIDTH/2+128, BASEVIDHEIGHT-58, V_20TRANS|V_MONOSPACE,
				va("%3.1fK/s ", ((double)getbps)/1024));

			// Download progress

			if (fileneeded[lastfilenum].currentsize != fileneeded[lastfilenum].totalsize)
				totaldldsize = downloadcompletedsize+fileneeded[lastfilenum].currentsize; //Add in single file progress download if applicable
			else
				totaldldsize = downloadcompletedsize;

			V_DrawCenteredString(BASEVIDWIDTH/2, BASEVIDHEIGHT-24-14, V_YELLOWMAP, "Overall Download Progress");
			totalfileslength = (INT32)((totaldldsize/(double)totalfilesrequestedsize) * 256);
			M_DrawTextBox(BASEVIDWIDTH/2-128-8, BASEVIDHEIGHT-24-8, 32, 1);
			V_DrawFill(BASEVIDWIDTH/2-128, BASEVIDHEIGHT-24, 256, 8, 175);
			V_DrawFill(BASEVIDWIDTH/2-128, BASEVIDHEIGHT-24, totalfileslength, 8, 160);

			if (totalfilesrequestedsize>>20 >= 10) //display in MB if over 10MB
				V_DrawString(BASEVIDWIDTH/2-128, BASEVIDHEIGHT-24, V_20TRANS|V_MONOSPACE,
					va(" %4uM/%4uM",totaldldsize>>20,totalfilesrequestedsize>>20));
			else
				V_DrawString(BASEVIDWIDTH/2-128, BASEVIDHEIGHT-24, V_20TRANS|V_MONOSPACE,
					va(" %4uK/%4uK",totaldldsize>>10,totalfilesrequestedsize>>10));

			V_DrawRightAlignedString(BASEVIDWIDTH/2+128, BASEVIDHEIGHT-24, V_20TRANS|V_MONOSPACE,
					va("%2u/%2u Files ",downloadcompletednum,totalfilesrequestednum));
		}
		else
		{
			INT32 i, animtime = ((ccstime / 4) & 15) + 16;
			UINT8 palstart = (cl_mode == CL_SEARCHING) ? 128 : 160;
			// 15 pal entries total.

			//Draw bottom box
			M_DrawTextBox(BASEVIDWIDTH/2-128-8, BASEVIDHEIGHT-24-8, 32, 1);
			V_DrawCenteredString(BASEVIDWIDTH/2, BASEVIDHEIGHT-24-24, V_YELLOWMAP, "Press ESC to abort");

			for (i = 0; i < 16; ++i)
				V_DrawFill((BASEVIDWIDTH/2-128) + (i * 16), BASEVIDHEIGHT-24, 16, 8, palstart + ((animtime - i) & 15));

			V_DrawCenteredString(BASEVIDWIDTH/2, BASEVIDHEIGHT-24-32, V_YELLOWMAP,
				M_GetText("Waiting to download files..."));
		}
	}
}
#endif

static boolean CL_AskFileList(INT32 firstfile)
{
	netbuffer->packettype = PT_TELLFILESNEEDED;
	netbuffer->u.filesneedednum = firstfile;

	return HSendPacket(servernode, false, 0, sizeof (INT32));
}

/** Sends a special packet to declare how many players in local
  * Used only in arbitratrenetstart()
  * Sends a PT_CLIENTJOIN packet to the server
  *
  * \return True if the packet was successfully sent
  * \todo Improve the description...
  *       Because to be honest, I have no idea what arbitratrenetstart is...
  *       Is it even used...?
  *
  */
static boolean CL_SendJoin(void)
{
	UINT8 localplayers = 1;
	if (netgame)
		CONS_Printf(M_GetText("Sending join request...\n"));
	netbuffer->packettype = PT_CLIENTJOIN;

	if (splitscreen)
		localplayers += splitscreen;
	else if (botingame)
		localplayers++;

	netbuffer->u.clientcfg.localplayers = localplayers;
	netbuffer->u.clientcfg._255 = 255;
	netbuffer->u.clientcfg.packetversion = PACKETVERSION;
	netbuffer->u.clientcfg.version = VERSION;
	netbuffer->u.clientcfg.subversion = SUBVERSION;
	strncpy(netbuffer->u.clientcfg.application, SRB2APPLICATION,
			sizeof netbuffer->u.clientcfg.application);

	return HSendPacket(servernode, false, 0, sizeof (clientconfig_pak));
}

static void
CopyCaretColors (char *p, const char *s, int n)
{
	char *t;
	int   m;
	int   c;
	if (!n)
		return;
	while (( t = strchr(s, '^') ))
	{
		m = ( t - s );

		if (m >= n)
		{
			memcpy(p, s, n);
			return;
		}
		else
			memcpy(p, s, m);

		p += m;
		n -= m;
		s += m;

		if (!n)
			return;

		if (s[1])
		{
			c = toupper(s[1]);
			if (isdigit(c))
				c = 0x80 + ( c - '0' );
			else if (c >= 'A' && c <= 'F')
				c = 0x80 + ( c - 'A' );
			else
				c = 0;

			if (c)
			{
				*p++ = c;
				n--;

				if (!n)
					return;
			}
			else
			{
				if (n < 2)
					break;

				memcpy(p, s, 2);

				p += 2;
				n -= 2;
			}

			s += 2;
		}
		else
			break;
	}
	strncpy(p, s, n);
}

static void SV_SendServerInfo(INT32 node, tic_t servertime)
{
	UINT8 *p;
	size_t mirror_length;
	const char *httpurl = cv_httpsource.string;
	UINT8 gt = (cv_kartgametypepreference.value == -1)
		? gametype
		: cv_kartgametypepreference.value;

	netbuffer->packettype = PT_SERVERINFO;
	netbuffer->u.serverinfo._255 = 255;
	netbuffer->u.serverinfo.packetversion = PACKETVERSION;
	netbuffer->u.serverinfo.version = VERSION;
	netbuffer->u.serverinfo.subversion = SUBVERSION;
	strncpy(netbuffer->u.serverinfo.application, SRB2APPLICATION,
			sizeof netbuffer->u.serverinfo.application);
	// return back the time value so client can compute their ping
	netbuffer->u.serverinfo.time = (tic_t)LONG(servertime);
	netbuffer->u.serverinfo.leveltime = (tic_t)LONG(leveltime);

	netbuffer->u.serverinfo.numberofplayer = (UINT8)D_NumPlayers();
	netbuffer->u.serverinfo.maxplayer = (UINT8)(min((dedicated ? MAXPLAYERS-1 : MAXPLAYERS), cv_maxplayers.value));

	// SRB2Kart: Vanilla's gametype constants for MS support
	netbuffer->u.serverinfo.gametype = (UINT8)((gt == GT_MATCH) ? VANILLA_GT_MATCH : VANILLA_GT_RACE);

	netbuffer->u.serverinfo.modifiedgame = (UINT8)modifiedgame;
	netbuffer->u.serverinfo.cheatsenabled = CV_CheatsEnabled();

	netbuffer->u.serverinfo.kartvars = (UINT8) (
		(cv_kartspeed.value & SV_SPEEDMASK) |
		(dedicated ? SV_DEDICATED : 0)
	);

	CopyCaretColors(netbuffer->u.serverinfo.servername, cv_servername.string,
		MAXSERVERNAME);
	strncpy(netbuffer->u.serverinfo.mapname, G_BuildMapName(gamemap), 7);

	M_Memcpy(netbuffer->u.serverinfo.mapmd5, mapmd5, 16);

	if (!(mapheaderinfo[gamemap-1]->levelflags & LF_NOZONE) && !(mapheaderinfo[prevmap]->zonttl[0]))
		netbuffer->u.serverinfo.iszone = 1;
	else
		netbuffer->u.serverinfo.iszone = 0;

	memset(netbuffer->u.serverinfo.maptitle, 0, 33);
	memset(netbuffer->u.serverinfo.httpsource, 0, MAX_MIRROR_LENGTH);

	if (!(mapheaderinfo[gamemap-1]->menuflags & LF2_HIDEINMENU) && mapheaderinfo[gamemap-1]->lvlttl[0])
	{
		//strncpy(netbuffer->u.serverinfo.maptitle, (char *)mapheaderinfo[gamemap-1]->lvlttl, 33);
		// set up the levelstring
		if (netbuffer->u.serverinfo.iszone || (mapheaderinfo[gamemap-1]->levelflags & LF_NOZONE))
		{
			if (mapheaderinfo[gamemap-1]->actnum[0])
				snprintf(netbuffer->u.serverinfo.maptitle,
					33,
					"%s %s",
					mapheaderinfo[gamemap-1]->lvlttl, mapheaderinfo[gamemap-1]->actnum);
			else
				snprintf(netbuffer->u.serverinfo.maptitle,
					33,
					"%s",
					mapheaderinfo[gamemap-1]->lvlttl);
		}
		else
		{
			if (mapheaderinfo[gamemap-1]->actnum[0])
			{
				if (snprintf(netbuffer->u.serverinfo.maptitle,
					33,
					"%s %s %s",
					mapheaderinfo[gamemap-1]->lvlttl, mapheaderinfo[gamemap-1]->zonttl, mapheaderinfo[gamemap-1]->actnum) < 0)
				{
					// If there's an encoding error, send UNKNOWN, we accept that the above may be truncated
					strncpy(netbuffer->u.serverinfo.maptitle, "Unknown", 33);
				}
			}
			else
			{
				if (snprintf(netbuffer->u.serverinfo.maptitle,
					33,
					"%s %s",
					mapheaderinfo[gamemap-1]->lvlttl, mapheaderinfo[gamemap-1]->zonttl) < 0)
				{
					// If there's an encoding error, send UNKNOWN, we accept that the above may be truncated
					strncpy(netbuffer->u.serverinfo.maptitle, "Unknown", 33);
				}
			}
		}
	}
	else
		strncpy(netbuffer->u.serverinfo.maptitle, "Unknown", 33);

	netbuffer->u.serverinfo.maptitle[32] = '\0';

	netbuffer->u.serverinfo.actnum = 0; //mapheaderinfo[gamemap-1]->actnum

	mirror_length = strlen(httpurl);
	if (mirror_length > MAX_MIRROR_LENGTH)
		mirror_length = MAX_MIRROR_LENGTH;

	if (snprintf(netbuffer->u.serverinfo.httpsource, mirror_length+1, "%s", httpurl) < 0)
		// If there's an encoding error, send nothing, we accept that the above may be truncated
		strncpy(netbuffer->u.serverinfo.httpsource, "", mirror_length);

	netbuffer->u.serverinfo.httpsource[MAX_MIRROR_LENGTH-1] = '\0';

	p = PutFileNeeded(0);

	HSendPacket(node, false, 0, p - ((UINT8 *)&netbuffer->u));
}

static void SV_SendPlayerInfo(INT32 node)
{
	UINT8 i;
	netbuffer->packettype = PT_PLAYERINFO;

	for (i = 0; i < MSCOMPAT_MAXPLAYERS; i++)
	{
		if (i >= MAXPLAYERS)
		{
			netbuffer->u.playerinfo[i].node = 255;
			continue;
		}

		if (!playeringame[i])
		{
			netbuffer->u.playerinfo[i].node = 255; // This slot is empty.
			continue;
		}

		netbuffer->u.playerinfo[i].node = i;
		strncpy(netbuffer->u.playerinfo[i].name, (const char *)&player_names[i], MAXPLAYERNAME+1);
		netbuffer->u.playerinfo[i].name[MAXPLAYERNAME] = '\0';

		//fetch IP address
		//No, don't do that, you fuckface.
		memset(netbuffer->u.playerinfo[i].address, 0, 4);

		if (G_GametypeHasTeams())
		{
			if (!players[i].ctfteam)
				netbuffer->u.playerinfo[i].team = 255;
			else
				netbuffer->u.playerinfo[i].team = (UINT8)players[i].ctfteam;
		}
		else
		{
			if (players[i].spectator)
				netbuffer->u.playerinfo[i].team = 255;
			else
				netbuffer->u.playerinfo[i].team = 0;
		}

		netbuffer->u.playerinfo[i].score = LONG(players[i].score);
		netbuffer->u.playerinfo[i].timeinserver = SHORT((UINT16)(players[i].jointime / TICRATE));
		netbuffer->u.playerinfo[i].skin = (UINT8)players[i].skin;

		// Extra data
		// Kart has extra skincolors, so we can't use this
		netbuffer->u.playerinfo[i].data = 0; //netbuffer->u.playerinfo[i].data = players[i].skincolor;

		if (players[i].pflags & PF_TAGIT)
			netbuffer->u.playerinfo[i].data |= 0x20;

		if (players[i].gotflag)
			netbuffer->u.playerinfo[i].data |= 0x40;

		if (players[i].powers[pw_super])
			netbuffer->u.playerinfo[i].data |= 0x80;
	}

	HSendPacket(node, false, 0, sizeof(plrinfo) * MSCOMPAT_MAXPLAYERS);
}

/** Sends a PT_SERVERCFG packet
  *
  * \param node The destination
  * \return True if the packet was successfully sent
  *
  */
static boolean SV_SendServerConfig(INT32 node)
{
	INT32 i;
	UINT8 *p, *op;
	boolean waspacketsent;

	netbuffer->packettype = PT_SERVERCFG;

	netbuffer->u.servercfg.version = VERSION;
	netbuffer->u.servercfg.subversion = SUBVERSION;

	netbuffer->u.servercfg.serverplayer = (UINT8)serverplayer;
	netbuffer->u.servercfg.totalslotnum = (UINT8)(doomcom->numslots);
	netbuffer->u.servercfg.gametic = (tic_t)LONG(gametic);
	netbuffer->u.servercfg.clientnode = (UINT8)node;
	netbuffer->u.servercfg.gamestate = (UINT8)gamestate;
	netbuffer->u.servercfg.gametype = (UINT8)gametype;
	netbuffer->u.servercfg.modifiedgame = (UINT8)modifiedgame;

	// we fill these structs with FFs so that any players not in game get sent as 0xFFFF
	// which is nice and easy for us to detect
	memset(netbuffer->u.servercfg.playerskins, 0xFF, sizeof(netbuffer->u.servercfg.playerskins));
	memset(netbuffer->u.servercfg.playercolor, 0xFF, sizeof(netbuffer->u.servercfg.playercolor));

	memset(netbuffer->u.servercfg.adminplayers, -1, sizeof(netbuffer->u.servercfg.adminplayers));

	for (i = 0; i < MAXPLAYERS; i++)
	{
		netbuffer->u.servercfg.adminplayers[i] = (SINT8)adminplayers[i];

		if (!playeringame[i])
			continue;

		netbuffer->u.servercfg.playerskins[i] = (UINT8)players[i].skin;
		netbuffer->u.servercfg.playercolor[i] = (UINT8)players[i].skincolor;
	}

	netbuffer->u.servercfg.maxplayer = (UINT8)(min((dedicated ? MAXPLAYERS-1 : MAXPLAYERS), cv_maxplayers.value));
	netbuffer->u.servercfg.allownewplayer = cv_allownewplayer.value;
	netbuffer->u.servercfg.discordinvites = (boolean)cv_discordinvites.value;

	memcpy(netbuffer->u.servercfg.server_context, server_context, 8);
	op = p = netbuffer->u.servercfg.varlengthinputs;

	CV_SavePlayerNames(&p);
	CV_SaveNetVars(&p, false);
	{
		const size_t len = sizeof (serverconfig_pak) + (size_t)(p - op);

#ifdef DEBUGFILE
		if (debugfile)
		{
			fprintf(debugfile, "ServerConfig Packet about to be sent, size of packet:%s to node:%d\n",
				sizeu1(len), node);
		}
#endif

		waspacketsent = HSendPacket(node, true, 0, len);
	}

#ifdef DEBUGFILE
	if (debugfile)
	{
		if (waspacketsent)
		{
			fprintf(debugfile, "ServerConfig Packet was sent\n");
		}
		else
		{
			fprintf(debugfile, "ServerConfig Packet could not be sent right now\n");
		}
	}
#endif

	return waspacketsent;
}

#ifdef JOININGAME
#define SAVEGAMESIZE (768*1024)

static void SV_SendSaveGame(INT32 node)
{
	size_t length, compressedlen;
	UINT8 *savebuffer;
	UINT8 *compressedsave;
	UINT8 *buffertosend;

	// first save it in a malloced buffer
	savebuffer = (UINT8 *)malloc(SAVEGAMESIZE);
	if (!savebuffer)
	{
		CONS_Alert(CONS_ERROR, M_GetText("No more free memory for savegame\n"));
		return;
	}

	// Leave room for the uncompressed length.
	save_p = savebuffer + sizeof(UINT32);

	P_SaveNetGame();

	length = save_p - savebuffer;
	if (length > SAVEGAMESIZE)
	{
		free(savebuffer);
		save_p = NULL;
		I_Error("Savegame buffer overrun");
	}

	// Allocate space for compressed save: one byte fewer than for the
	// uncompressed data to ensure that the compression is worthwhile.
	compressedsave = malloc(length - 1);
	if (!compressedsave)
	{
		CONS_Alert(CONS_ERROR, M_GetText("No more free memory for savegame\n"));
		return;
	}

	// Attempt to compress it.
	if((compressedlen = lzf_compress(savebuffer + sizeof(UINT32), length - sizeof(UINT32), compressedsave + sizeof(UINT32), length - sizeof(UINT32) - 1)))
	{
		// Compressing succeeded; send compressed data

		free(savebuffer);

		// State that we're compressed.
		buffertosend = compressedsave;
		WRITEUINT32(compressedsave, length - sizeof(UINT32));
		length = compressedlen + sizeof(UINT32);
	}
	else
	{
		// Compression failed to make it smaller; send original

		free(compressedsave);

		// State that we're not compressed
		buffertosend = savebuffer;
		WRITEUINT32(savebuffer, 0);
	}

	SV_SendRam(node, buffertosend, length, SF_RAM, 0);
	save_p = NULL;

	// Remember when we started sending the savegame so we can handle timeouts
	sendingsavegame[node] = true;
	freezetimeout[node] = I_GetTime() + jointimeout + length / 1024; // 1 extra tic for each kilobyte
}

#ifdef DUMPCONSISTENCY
#define TMPSAVENAME "badmath.sav"
static consvar_t cv_dumpconsistency = {"dumpconsistency", "Off", CV_NETVAR, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL};

static void SV_SavedGame(void)
{
	size_t length;
	UINT8 *savebuffer;
	XBOXSTATIC char tmpsave[264];

	if (!cv_dumpconsistency.value)
		return;

	sprintf(tmpsave, "%s" PATHSEP TMPSAVENAME, srb2home);

	// first save it in a malloced buffer
	save_p = savebuffer = (UINT8 *)malloc(SAVEGAMESIZE);
	if (!save_p)
	{
		CONS_Alert(CONS_ERROR, M_GetText("No more free memory for savegame\n"));
		return;
	}

	P_SaveNetGame();

	length = save_p - savebuffer;
	if (length > SAVEGAMESIZE)
	{
		free(savebuffer);
		save_p = NULL;
		I_Error("Savegame buffer overrun");
	}

	// then save it!
	if (!FIL_WriteFile(tmpsave, savebuffer, length))
		CONS_Printf(M_GetText("Didn't save %s for netgame"), tmpsave);

	free(savebuffer);
	save_p = NULL;
}

#undef  TMPSAVENAME
#endif
#define TMPSAVENAME "$$$.sav"


static void CL_LoadReceivedSavegame(void)
{
	UINT8 *savebuffer = NULL;
	size_t length, decompressedlen;
	XBOXSTATIC char tmpsave[264];

	sprintf(tmpsave, "%s" PATHSEP TMPSAVENAME, srb2home);

	length = FIL_ReadFile(tmpsave, &savebuffer);

	CONS_Printf(M_GetText("Loading savegame length %s\n"), sizeu1(length));
	if (!length)
	{
		I_Error("Can't read savegame sent");
		return;
	}

	save_p = savebuffer;

	// Decompress saved game if necessary.
	decompressedlen = READUINT32(save_p);
	if(decompressedlen > 0)
	{
		UINT8 *decompressedbuffer = Z_Malloc(decompressedlen, PU_STATIC, NULL);
		lzf_decompress(save_p, length - sizeof(UINT32), decompressedbuffer, decompressedlen);
		Z_Free(savebuffer);
		save_p = savebuffer = decompressedbuffer;
	}

	paused = false;
	demo.playback = false;
	demo.title = false;
	automapactive = false;

	// load a base level
	if (P_LoadNetGame())
	{
		CON_LogMessage(va(M_GetText("Map is now \"%s"), G_BuildMapName(gamemap)));
		if (strlen(mapheaderinfo[gamemap-1]->lvlttl) > 0)
		{
			CON_LogMessage(va(": %s", mapheaderinfo[gamemap-1]->lvlttl));
			if (strlen(mapheaderinfo[gamemap-1]->zonttl) > 0)
				CON_LogMessage(va(" %s", mapheaderinfo[gamemap-1]->zonttl));
			else if (!(mapheaderinfo[gamemap-1]->levelflags & LF_NOZONE))
				CON_LogMessage(M_GetText(" Zone"));
			if (strlen(mapheaderinfo[gamemap-1]->actnum) > 0)
				CON_LogMessage(va(" %s", mapheaderinfo[gamemap-1]->actnum));
		}
		CON_LogMessage("\"\n");
	}
	else
	{
		CONS_Alert(CONS_ERROR, M_GetText("Can't load the level!\n"));
		Z_Free(savebuffer);
		save_p = NULL;
		if (unlink(tmpsave) == -1)
			CONS_Alert(CONS_ERROR, M_GetText("Can't delete %s\n"), tmpsave);
		return;
	}

	// done
	Z_Free(savebuffer);
	save_p = NULL;
	if (unlink(tmpsave) == -1)
		CONS_Alert(CONS_ERROR, M_GetText("Can't delete %s\n"), tmpsave);
	consistancy[gametic%TICQUEUE] = Consistancy();
	CON_ToggleOff();
}
#endif

#ifndef NONET
static void SendAskInfo(INT32 node)
{
	tic_t asktime;

	if (node != 0 && node != BROADCASTADDR &&
			cv_rendezvousserver.string[0])
	{
		I_NetRequestHolePunch(node);
	}

	asktime = I_GetTime();

	netbuffer->packettype = PT_ASKINFO;
	netbuffer->u.askinfo.version = VERSION;
	netbuffer->u.askinfo.time = (tic_t)LONG(asktime);

	// Even if this never arrives due to the host being firewalled, we've
	// now allowed traffic from the host to us in, so once the MS relays
	// our address to the host, it'll be able to speak to us.
	HSendPacket(node, false, 0, sizeof (askinfo_pak));
}

serverelem_t serverlist[MAXSERVERLIST];
UINT32 serverlistcount = 0;
UINT32 serverlistultimatecount = 0;

static boolean resendserverlistnode[MAXNETNODES];
static tic_t serverlistepoch;

static void SL_ClearServerList(INT32 connectedserver)
{
	UINT32 i;

	for (i = 0; i < serverlistcount; i++)
		if (connectedserver != serverlist[i].node)
		{
			Net_CloseConnection(serverlist[i].node|FORCECLOSE);
			serverlist[i].node = 0;
		}
	serverlistcount = 0;

	memset(resendserverlistnode, 0, sizeof resendserverlistnode);
}

static UINT32 SL_SearchServer(INT32 node)
{
	UINT32 i;
	for (i = 0; i < serverlistcount; i++)
		if (serverlist[i].node == node)
			return i;

	return UINT32_MAX;
}

static void SL_InsertServer(serverinfo_pak* info, SINT8 node)
{
	UINT32 i;

	resendserverlistnode[node] = false;

	// search if not already on it
	i = SL_SearchServer(node);
	if (i == UINT32_MAX)
	{
		// not found add it
		if (serverlistcount >= MAXSERVERLIST)
			return; // list full

		if (info->_255 != 255)
			return;/* old packet format */

		if (info->packetversion != PACKETVERSION)
			return;/* old new packet format */

		if (info->version != VERSION)
			return; // Not same version.

		if (info->subversion != SUBVERSION)
			return; // Close, but no cigar.

		if (strcmp(info->application, SRB2APPLICATION))
			return;/* that's a different mod */

		i = serverlistcount++;
	}

	serverlist[i].info = *info;
	serverlist[i].node = node;

	// resort server list
	M_SortServerList();
}

void CL_UpdateServerList (void)
{
	SL_ClearServerList(0);

	if (!netgame && I_NetOpenSocket)
	{
		if (I_NetOpenSocket())
		{
			netgame = true;
			multiplayer = true;
		}
	}

	// search for local servers
	if (netgame)
		SendAskInfo(BROADCASTADDR);
}

void CL_QueryServerList (msg_server_t *server_list)
{
	INT32 i;

	CL_UpdateServerList();

	serverlistepoch = I_GetTime();

	for (i = 0; server_list[i].header.buffer[0]; i++)
	{
		// Make sure MS version matches our own, to
		// thwart nefarious servers who lie to the MS.

		/* lol bruh, that version COMES from the servers */
		//if (strcmp(version, server_list[i].version) == 0)
		{
			INT32 node = I_NetMakeNodewPort(server_list[i].ip, server_list[i].port);
			if (node == -1)
				break; // no more node free
			SendAskInfo(node);
			resendserverlistnode[node] = true;
			// Leave this node open. It'll be closed if the
			// request times out (CL_TimeoutServerList).
		}
	}

	serverlistultimatecount = i;
}

#define SERVERLISTRESENDRATE NEWTICRATE

void CL_TimeoutServerList(void)
{
	if (netgame && serverlistultimatecount > serverlistcount)
	{
		const tic_t timediff = I_GetTime() - serverlistepoch;
		const tic_t timetoresend = timediff % SERVERLISTRESENDRATE;
		const boolean timedout = timediff > connectiontimeout;

		if (timedout || (timediff > 0 && timetoresend == 0))
		{
			INT32 node;

			for (node = 1; node < MAXNETNODES; ++node)
			{
				if (resendserverlistnode[node])
				{
					if (timedout)
						Net_CloseConnection(node|FORCECLOSE);
					else
						SendAskInfo(node);
				}
			}

			if (timedout)
				serverlistultimatecount = serverlistcount;
		}
	}
}
#endif // ifndef NONET

static void M_ConfirmConnect(event_t *ev)
{
#ifndef NONET
	if (ev->type == ev_keydown)
	{
		if (ev->data1 == ' ' || ev->data1 == 'y' || ev->data1 == KEY_ENTER || ev->data1 == gamecontrol[gc_accelerate][0] || ev->data1 == gamecontrol[gc_accelerate][1])
		{
			if (totalfilesrequestednum > 0)
			{
#ifdef HAVE_CURL
				if (http_source[0] == '\0' || curl_failedwebdownload)
#endif
				{
					if (CL_SendRequestFile())
					{
						cl_mode = CL_DOWNLOADFILES;
					}
					else
					{
						cl_mode = CL_LEGACYREQUESTFAILED;
					}
				}
#ifdef HAVE_CURL
				else
					cl_mode = CL_PREPAREHTTPFILES;
#endif
			}
			else
				cl_mode = CL_LOADFILES;

			M_ClearMenus(true);
		}
		else if (ev->data1 == 'n' || ev->data1 == KEY_ESCAPE|| ev->data1 == gamecontrol[gc_brake][0] || ev->data1 == gamecontrol[gc_brake][1])
		{
			cl_mode = CL_ABORTED;
			M_ClearMenus(true);
		}
	}
#else
	(void)ev;
#endif
}

static boolean CL_FinishedFileList(void)
{
	INT32 i;
	char *downloadsize = NULL;
	//CONS_Printf(M_GetText("Checking files...\n"));
	i = CL_CheckFiles();
	if (i == 4) // still checking ...
	{
		return true;
	}
	else if (i == 3) // too many files
	{
		D_QuitNetGame();
		CL_Reset();
		D_StartTitle();
		M_StartMessage(M_GetText(
			"You have too many WAD files loaded\n"
			"to add ones the server is using.\n"
			"Please restart SRB2Kart before connecting.\n\n"
			"Press ESC\n"
		), NULL, MM_NOTHING);
		return false;
	}
	else if (i == 2) // cannot join for some reason
	{
		D_QuitNetGame();
		CL_Reset();
		D_StartTitle();
		M_StartMessage(M_GetText(
			"You have the wrong addons loaded.\n\n"
			"To play on this server, restart\n"
			"the game and don't load any addons.\n"
			"SRB2Kart will automatically add\n"
			"everything you need when you join.\n\n"
			"Press ESC\n"
		), NULL, MM_NOTHING);
		return false;
	}
	else if (i == 1)
	{
		if (serverisfull)
		{
			M_StartMessage(M_GetText(
				"This server is full!\n"
				"\n"
				"You may load server addons (if any), and wait for a slot.\n"
				"\n"
				"Press ACCEL to continue or BRAKE to cancel.\n\n"
			), M_ConfirmConnect, MM_EVENTHANDLER);
			cl_mode = CL_CONFIRMCONNECT;
		}
		else
			cl_mode = CL_LOADFILES;
	}
	else
	{
		// must download something
		// can we, though?
#ifdef HAVE_CURL
		if (http_source[0] == '\0' || curl_failedwebdownload)
#endif
		{
			if (!CL_CheckDownloadable()) // nope!
			{
				D_QuitNetGame();
				CL_Reset();
				D_StartTitle();
				M_StartMessage(M_GetText(
					"An error occured when trying to\n"
					"download missing addons.\n"
					"(This is almost always a problem\n"
					"with the server, not your game.)\n\n"
					"See the console or log file\n"
					"for additional details.\n\n"
					"Press ESC\n"
				), NULL, MM_NOTHING);
				return false;
			}
		}

#ifdef HAVE_CURL
		if (!curl_failedwebdownload)
#endif
		{
#ifndef NONET
			downloadcompletednum = 0;
			downloadcompletedsize = 0;
			totalfilesrequestednum = 0;
			totalfilesrequestedsize = 0;
#endif

			for (i = 0; i < fileneedednum; i++)
				if (fileneeded[i].status == FS_NOTFOUND || fileneeded[i].status == FS_MD5SUMBAD)
				{
#ifndef NONET
					totalfilesrequestednum++;
					totalfilesrequestedsize += fileneeded[i].totalsize;
#endif
				}

#ifndef NONET
			if (totalfilesrequestedsize>>20 >= 10)
				downloadsize = Z_StrDup(va("%uM",totalfilesrequestedsize>>20));
			else
				downloadsize = Z_StrDup(va("%uK",totalfilesrequestedsize>>10));
#endif

			if (serverisfull)
				M_StartMessage(va(M_GetText(
					"This server is full!\n"
					"Download of %s additional content is required to join.\n"
					"\n"
					"You may download, load server addons, and wait for a slot.\n"
					"\n"
					"Press ACCEL to continue or BRAKE to cancel.\n\n"
				), downloadsize), M_ConfirmConnect, MM_EVENTHANDLER);
			else
				M_StartMessage(va(M_GetText(
					"Download of %s additional content is required to join.\n"
					"\n"
					"Press ACCEL to continue or BRAKE to cancel.\n\n"
				), downloadsize), M_ConfirmConnect, MM_EVENTHANDLER);

			Z_Free(downloadsize);
			cl_mode = CL_CONFIRMCONNECT;
		}
#ifdef HAVE_CURL
		else
		{
			if (CL_SendRequestFile())
			{
				cl_mode = CL_DOWNLOADFILES;
			}
			else
			{
				cl_mode = CL_LEGACYREQUESTFAILED;
			}
		}
#endif
	}
	return true;
}

/** Called by CL_ServerConnectionTicker
  *
  * \param asksent The last time we asked the server to join. We re-ask every second in case our request got lost in transmit.
  * \return False if the connection was aborted
  * \sa CL_ServerConnectionTicker
  * \sa CL_ConnectToServer
  *
  */
static boolean CL_ServerConnectionSearchTicker(tic_t *asksent)
{
#ifndef NONET
	INT32 i;

	// serverlist is updated by GetPacket function
	if (serverlistcount > 0)
	{
		// this can be a responce to our broadcast request
		if (servernode == -1 || servernode >= MAXNETNODES)
		{
			i = 0;
			servernode = serverlist[i].node;
			CONS_Printf(M_GetText("Found, "));
		}
		else
		{
			i = SL_SearchServer(servernode);
			if (i < 0)
				return true;
		}

		// Quit here rather than downloading files and being refused later.
		if (serverlist[i].info.numberofplayer >= serverlist[i].info.maxplayer)
		{
			serverisfull = true;
		}

		if (client)
		{
#ifdef HAVE_CURL
			if (serverlist[i].info.httpsource[0])
				strncpy(http_source, serverlist[i].info.httpsource, MAX_MIRROR_LENGTH);
			else
				http_source[0] = '\0';
#else
			if (serverlist[i].info.httpsource[0])
				CONS_Printf("We received a http url from the server, however it will not be used as this build lacks curl support (%s)\n", serverlist[i].info.httpsource);
#endif
			D_ParseFileneeded(serverlist[i].info.fileneedednum, serverlist[i].info.fileneeded, 0);
			if (serverlist[i].info.kartvars & SV_LOTSOFADDONS)
			{
				cl_mode = CL_ASKFULLFILELIST;
				cl_lastcheckedfilecount = 0;
				return true;
			}

			cl_mode = CL_CHECKFILES;
		}
		else
		{
			cl_mode = CL_ASKJOIN; // files need not be checked for the server.
			*asksent = 0;
		}

		return true;
	}

	// Ask the info to the server (askinfo packet)
	if (I_GetTime() >= *asksent)
	{
		SendAskInfo(servernode);
		*asksent = I_GetTime() + NEWTICRATE;
	}
#else
	(void)asksent;
	// No netgames, so we skip this state.
	cl_mode = CL_ASKJOIN;
#endif // ifndef NONET/else

	return true;
}

/** Called by CL_ConnectToServer
  *
  * \param tmpsave The name of the gamestate file???
  * \param oldtic Used for knowing when to poll events and redraw
  * \param asksent The last time we asked the server to join. We re-ask every second in case our request got lost in transmit.
  * \return False if the connection was aborted
  * \sa CL_ServerConnectionSearchTicker
  * \sa CL_ConnectToServer
  *
  */
static boolean CL_ServerConnectionTicker(const char *tmpsave, tic_t *oldtic, tic_t *asksent)
{
	boolean waitmore;
	INT32 i;

#ifdef NONET
	(void)tmpsave;
#endif

	switch (cl_mode)
	{
		case CL_SEARCHING:
			if (!CL_ServerConnectionSearchTicker(asksent))
				return false;
			break;

		case CL_ASKFULLFILELIST:
			if (cl_lastcheckedfilecount == UINT16_MAX) // All files retrieved
				cl_mode = CL_CHECKFILES;
			else if (fileneedednum != cl_lastcheckedfilecount || I_GetTime() >= *asksent)
			{
				if (CL_AskFileList(fileneedednum))
				{
					cl_lastcheckedfilecount = fileneedednum;
					*asksent = I_GetTime() + NEWTICRATE;
				}
			}
			break;
		case CL_CHECKFILES:
			if (!CL_FinishedFileList())
				return false;
			break;
#ifdef HAVE_CURL
		case CL_PREPAREHTTPFILES:
			if (http_source[0])
			{
				for (i = 0; i < fileneedednum; i++)
					if (fileneeded[i].status == FS_NOTFOUND || fileneeded[i].status == FS_MD5SUMBAD)
					{
						curl_transfers++;
					}

				cl_mode = CL_DOWNLOADHTTPFILES;
			}
			break;

		case CL_DOWNLOADHTTPFILES:
			waitmore = false;
			for (i = 0; i < fileneedednum; i++)
				if (fileneeded[i].status == FS_NOTFOUND || fileneeded[i].status == FS_MD5SUMBAD)
				{
					if (!curl_running)
						CURLPrepareFile(http_source, i);
					waitmore = true;
					break;
				}

			if (curl_running)
				CURLGetFile();

			if (waitmore)
				break; // exit the case

			if (curl_failedwebdownload && !curl_transfers)
			{
				CONS_Printf("One or more files failed to download, falling back to internal downloader\n");
				cl_mode = CL_CHECKFILES;
				break;
			}

			if (!curl_transfers)
				cl_mode = CL_LOADFILES;

			break;
#endif
		case CL_DOWNLOADFILES:
			waitmore = false;
			for (i = 0; i < fileneedednum; i++)
				if (fileneeded[i].status == FS_DOWNLOADING
					|| fileneeded[i].status == FS_REQUESTED)
				{
					waitmore = true;
					break;
				}
			if (waitmore)
				break; // exit the case

			cl_mode = CL_LOADFILES;
			break;
		case CL_LEGACYREQUESTFAILED:
			{
				CONS_Printf(M_GetText("Legacy downloader request packet failed.\n"));
				CONS_Printf(M_GetText("Network game synchronization aborted.\n"));
				D_QuitNetGame();
				CL_Reset();
				D_StartTitle();
				M_StartMessage(M_GetText(
					"The direct download encountered an error.\n"
					"See the logfile for more info.\n"
					"\n"
					"Press ESC\n"
				), NULL, MM_NOTHING);
				return false;
			}
		case CL_LOADFILES:
			if (CL_LoadServerFiles()) 
				cl_mode = CL_SETUPFILES;

			break;
		case CL_SETUPFILES:
			if (P_PartialAddGetStage() < 0 || P_MultiSetupWadFiles(false))
			{
				*asksent = 0; //This ensure the first join ask is right away
				firstconnectattempttime = I_GetTime();
				cl_mode = CL_ASKJOIN;
			}
			break;
		case CL_ASKJOIN:
			if (firstconnectattempttime + NEWTICRATE*300 < I_GetTime() && !server)
			{
				CONS_Printf(M_GetText("5 minute wait time exceeded.\n"));
				CONS_Printf(M_GetText("Network game synchronization aborted.\n"));
				D_QuitNetGame();
				CL_Reset();
				D_StartTitle();
				M_StartMessage(M_GetText(
					"5 minute wait time exceeded.\n"
					"You may retry connection.\n"
					"\n"
					"Press ESC\n"
				), NULL, MM_NOTHING);
				return false;
			}
#ifdef JOININGAME
			// prepare structures to save the file
			// WARNING: this can be useless in case of server not in GS_LEVEL
			// but since the network layer doesn't provide ordered packets...
			CL_PrepareDownloadSaveGame(tmpsave);
#endif
			if (I_GetTime() >= *asksent && CL_SendJoin())
			{
				*asksent = I_GetTime() + NEWTICRATE*3;
				cl_mode = CL_WAITJOINRESPONSE;
			}
			break;
		case CL_WAITJOINRESPONSE:
			if (I_GetTime() >= *asksent)
			{
				cl_mode = CL_ASKJOIN;
			}
			break;
#ifdef JOININGAME
		case CL_DOWNLOADSAVEGAME:
			// At this state, the first (and only) needed file is the gamestate
			if (fileneeded[0].status == FS_FOUND)
			{
				// Gamestate is now handled within CL_LoadReceivedSavegame()
				CL_LoadReceivedSavegame();
				cl_mode = CL_CONNECTED;
				break;
			} // don't break case continue to CL_CONNECTED
			else
				break;
#endif
		case CL_CONNECTED:
		case CL_CONFIRMCONNECT: //logic is handled by M_ConfirmConnect
		default:
			break;

		// Connection closed by cancel, timeout or refusal.
		case CL_ABORTED:
			cl_mode = CL_SEARCHING;
			return false;

	}

	GetPackets();
	Net_AckTicker();

	// Call it only once by tic
	if (*oldtic != I_GetTime())
	{

		INT32 key;

		I_OsPolling();

		if (cl_mode == CL_CONFIRMCONNECT)
			D_ProcessEvents(); //needed for menu system to receive inputs

		key = I_GetKey();
		// Only ESC and non-keyboard keys abort connection
		if (!modeattacking && (key == KEY_ESCAPE || key >= KEY_MOUSE1 || cl_mode == CL_ABORTED))
		{
			CONS_Printf(M_GetText("Network game synchronization aborted.\n"));
			D_QuitNetGame();
			CL_Reset();
			D_StartTitle();

			return false;
		}
		*oldtic = I_GetTime();

#ifdef CLIENT_LOADINGSCREEN
		if (client && cl_mode != CL_CONNECTED && cl_mode != CL_ABORTED)
		{
			F_TitleScreenTicker(true);
			F_TitleScreenDrawer();
			CL_DrawConnectionStatus();
#ifdef HAVE_THREADS
			I_lock_mutex(&m_menu_mutex);
#endif
			M_Drawer(); //Needed for drawing messageboxes on the connection screen
#ifdef HAVE_THREADS
			I_unlock_mutex(m_menu_mutex);
#endif
			I_UpdateNoVsync(); // page flip or blit buffer
			if (moviemode)
				M_SaveFrame();
		}
#else
		CON_Drawer();
		I_UpdateNoVsync();
#endif
	}
	else
	{
		I_Sleep(cv_sleep.value);
		I_UpdateTime(cv_timescale.value);
	}

	return true;
}

/** Use adaptive send using net_bandwidth and stat.sendbytes
  *
  * \todo Better description...
  *
  */
static void CL_ConnectToServer(void)
{
	INT32 pnumnodes, nodewaited = doomcom->numnodes, i;
	tic_t oldtic;
#ifndef NONET
	tic_t asksent;
#endif
#ifdef JOININGAME
	XBOXSTATIC char tmpsave[264];

	sprintf(tmpsave, "%s" PATHSEP TMPSAVENAME, srb2home);
#endif

	cl_mode = CL_SEARCHING;

#ifdef CLIENT_LOADINGSCREEN
	lastfilenum = -1;
#endif

#ifdef JOININGAME
	// Don't get a corrupt savegame error because tmpsave already exists
	if (FIL_FileExists(tmpsave) && unlink(tmpsave) == -1)
		I_Error("Can't delete %s\n", tmpsave);
#endif

	if (netgame)
	{
		if (servernode < 0 || servernode >= MAXNETNODES)
			CONS_Printf(M_GetText("Searching for a server...\n"));
		else
			CONS_Printf(M_GetText("Contacting the server...\n"));
	}

	if (gamestate == GS_INTERMISSION)
		Y_EndIntermission(); // clean up intermission graphics etc
	if (gamestate == GS_VOTING)
		Y_EndVote();

	DEBFILE(va("waiting %d nodes\n", doomcom->numnodes));
	G_SetGamestate(GS_WAITINGPLAYERS);
	wipegamestate = GS_WAITINGPLAYERS;

	ClearAdminPlayers();
	pnumnodes = 1;
	oldtic = 0;
#ifndef NONET
	asksent = 0;
	firstconnectattempttime = I_GetTime();

	i = SL_SearchServer(servernode);

	if (i != -1)
	{
		UINT8 num = serverlist[i].info.gametype;
		const char *gametypestr = NULL;

		strncpy(connectedservername, serverlist[i].info.servername, MAXSERVERNAME);

		CONS_Printf(M_GetText("Connecting to: %s\n"), serverlist[i].info.servername);
		if (num < NUMGAMETYPES)
			gametypestr = Gametype_Names[num];
		if (gametypestr)
			CONS_Printf(M_GetText("Gametype: %s\n"), gametypestr);
		CONS_Printf(M_GetText("Version: %d.%d\n"),
		 serverlist[i].info.version, serverlist[i].info.subversion);
	}
	SL_ClearServerList(servernode);
#endif

	do
	{
		// If the connection was aborted for some reason, leave
#ifndef NONET
		if (!CL_ServerConnectionTicker(tmpsave, &oldtic, &asksent))
#else
		if (!CL_ServerConnectionTicker((char*)NULL, &oldtic, (tic_t *)NULL))
#endif
		{
			if (P_PartialAddGetStage() >= 0)
				P_MultiSetupWadFiles(true); // in case any partial adds were done
			return;
		}

		if (server)
		{
			pnumnodes = 0;
			for (i = 0; i < MAXNETNODES; i++)
				if (nodeingame[i])
					pnumnodes++;
		}
	}
	while (!(cl_mode == CL_CONNECTED && (client || (server && nodewaited <= pnumnodes))));

#ifndef NONET
	if (netgame)
		F_StartWaitingPlayers();
#endif
	DEBFILE(va("Synchronisation Finished\n"));

	displayplayers[0] = consoleplayer;
}

#ifndef NONET
static void Command_ShowBan(void) //Print out ban list
{
	size_t i;
	const char *address, *mask, *reason, *username;
	time_t unbanTime = NO_BAN_TIME;
	const time_t curTime = time(NULL);

	if (I_GetBanAddress)
		CONS_Printf(M_GetText("Ban List:\n"));
	else
		return;

	for (i = 0; (address = I_GetBanAddress(i)) != NULL; i++)
	{
		unbanTime = NO_BAN_TIME;
		if (I_GetUnbanTime)
			unbanTime = I_GetUnbanTime(i);

		if (unbanTime != NO_BAN_TIME && curTime >= unbanTime)
			continue;

		CONS_Printf("%s: ", sizeu1(i+1));

		if (I_GetBanUsername && (username = I_GetBanUsername(i)) != NULL)
			CONS_Printf("%s - ", username);

		if (!I_GetBanMask || (mask = I_GetBanMask(i)) == NULL)
			CONS_Printf("%s", address);
		else
			CONS_Printf("%s/%s", address, mask);

		if (I_GetBanReason && (reason = I_GetBanReason(i)) != NULL)
			CONS_Printf(" - %s", reason);

		if (unbanTime != NO_BAN_TIME)
		{
			 // these are fudged a little to match what a joiner sees
			int minutes = ((unbanTime - curTime) + 30) / 60;
			int hours = (minutes + 1) / 60;
			int days = (hours + 1) / 24;
			if (days)
				CONS_Printf(" (%d day%s)", days, days > 1 ? "s" : "");
			else if (hours)
				CONS_Printf(" (%d hour%s)", hours, hours > 1 ? "s" : "");
			else if (minutes)
				CONS_Printf(" (%d minute%s)", minutes, minutes > 1 ? "s" : "");
			else
				CONS_Printf(" (<1 minute)");
		}

		CONS_Printf("\n");
	}

	if (i == 0 && !address)
		CONS_Printf(M_GetText("(empty)\n"));
}

static boolean bansLoaded = false;
// If you're a community contributor looking to improve how bans are written, please
// offer your changes back to our Git repository. Kart Krew reserve the right to
// utilise format numbers in use by community builds for different layouts.
#define BANFORMAT 1

void D_SaveBan(void)
{
	FILE *f;
	size_t i;
	const char *address, *mask;
	const char *username, *reason;
	const time_t curTime = time(NULL);
	time_t unbanTime = NO_BAN_TIME;
	const char *path = va("%s"PATHSEP"%s", srb2home, "ban.txt");

	if (bansLoaded != true)
	{
		// You didn't even get to ATTEMPT to load bans.txt.
		// Don't immediately save nothing over it.
		return;
	}

	f = fopen(path, "w");

	if (!f)
	{
		CONS_Alert(CONS_WARNING, M_GetText("Could not save ban list into ban.txt\n"));
		return;
	}

	// Add header.
	fprintf(f, "BANFORMAT %d\n", BANFORMAT);

	for (i = 0; (address = I_GetBanAddress(i)) != NULL; i++)
	{
		if (I_GetUnbanTime)
		{
			unbanTime = I_GetUnbanTime(i);
		}
		else
		{
			unbanTime = NO_BAN_TIME;
		}

		if (unbanTime != NO_BAN_TIME && curTime >= unbanTime)
		{
			// This one has served their sentence.
			// We don't need to save them in the file anymore.
			continue;
		}

		mask = NULL;
		if (!I_GetBanMask || (mask = I_GetBanMask(i)) == NULL)
			fprintf(f, "%s/0", address);
		else
			fprintf(f, "%s/%s", address, mask);

		fprintf(f, " %ld", (long)unbanTime);

		username = NULL;
		if (I_GetBanUsername && (username = I_GetBanUsername(i)) != NULL)
			fprintf(f, " \"%s\"", username);
		else
			fprintf(f, " \"%s\"", "Direct IP ban");

		reason = NULL;
		if (I_GetBanReason && (reason = I_GetBanReason(i)) != NULL)
			fprintf(f, " \"%s\"\n", reason);
		else
			fprintf(f, " \"%s\"\n", "No reason given");
	}

	fclose(f);
}

static void Command_ClearBans(void)
{
	if (!I_ClearBans)
		return;

	I_ClearBans();
	D_SaveBan();
}

void D_LoadBan(boolean warning)
{
	FILE *f;
	size_t i, j;
	char *address, *mask;
	char *username, *reason;
	time_t unbanTime = NO_BAN_TIME;
	char buffer[MAX_WADPATH];
	UINT8 banmode = 0;
	boolean malformed = false;

	if (!I_ClearBans)
		return;

	// We at least attempted loading bans.txt
	bansLoaded = true;

	f = fopen(va("%s"PATHSEP"%s", srb2home, "ban.txt"), "r");

	if (!f)
	{
		if (warning)
			CONS_Alert(CONS_WARNING, M_GetText("Could not open ban.txt for ban list\n"));
		return;
	}

	I_ClearBans();

	for (i = 0; fgets(buffer, (int)sizeof(buffer), f); i++)
	{
		address = strtok(buffer, " /\t\r\n");
		mask = strtok(NULL, " \t\r\n");

		if (i == 0 && !strncmp(address, "BANFORMAT", 9))
		{
			if (mask)
			{
				banmode = atoi(mask);
			}
			switch (banmode)
			{
				case BANFORMAT: // currently supported format
				//case 0: -- permitted only when BANFORMAT string not present
					break;
				default:
				{
					fclose(f);
					CONS_Alert(CONS_WARNING, "Could not load unknown ban.txt for ban list (BANFORMAT %s, expected %d)\n", mask, BANFORMAT);
					return;
				}
			}
			continue;
		}

		if (I_SetBanAddress(address, mask) == false) // invalid IP input?
		{
			CONS_Alert(CONS_WARNING, "\"%s/%s\" is not a valid IP address, discarding...\n", address, mask);
			continue;
		}

		// One-way legacy format conversion -- the game will crash otherwise
		if (banmode == 0)
		{
			unbanTime = NO_BAN_TIME;
			username = NULL; // not guaranteed to be accurate, but only sane substitute
			reason = strtok(NULL, "\r\n");
			if (reason && reason[0] == 'N' && reason[1] == 'A' && reason[2] == '\0')
			{
				reason = NULL;
			}
		}
		else
		{
			reason = strtok(NULL, " \"\t\r\n");
			if (reason)
			{
				unbanTime = atoi(reason);
				reason = NULL;
			}
			else
			{
				unbanTime = NO_BAN_TIME;
				malformed = true;
			}

			username = strtok(NULL, "\"\t\r\n"); // go until next "
			if (!username)
			{
				malformed = true;
			}

			strtok(NULL, "\"\t\r\n"); // remove first "
			reason = strtok(NULL, "\"\r\n"); // go until next "
			if (!reason)
			{
				malformed = true;
			}
		}

		// Enforce MAX_REASONLENGTH.
		if (reason)
		{
			j = 0;
			while (reason[j] != '\0')
			{
				if ((j++) < MAX_REASONLENGTH)
					continue;
				reason[j] = '\0';
				break;
			}
		}

		if (I_SetUnbanTime)
			I_SetUnbanTime(unbanTime);

		if (I_SetBanUsername)
			I_SetBanUsername(username);

		if (I_SetBanReason)
			I_SetBanReason(reason);
	}

	if (malformed)
	{
		CONS_Alert(CONS_WARNING, "One or more lines of ban.txt are malformed. The game can correct for this, but some data may be lost.\n");
	}

	fclose(f);
}

#undef BANFORMAT

static void Command_ReloadBan(void)  //recheck ban.txt
{
	D_LoadBan(true);
}

static void Command_connect(void)
{
	if (COM_Argc() < 2 || *COM_Argv(1) == 0)
	{
		CONS_Printf(M_GetText(
			"Connect <serveraddress> (port): connect to a server\n"
			"Connect ANY: connect to the first lan server found\n"
			"Connect SELF: connect to your own server.\n"));
		return;
	}

	if (Playing() || demo.title)
	{
		CONS_Printf(M_GetText("You cannot connect while in a game. End this game first.\n"));
		return;
	}

	// modified game check: no longer handled
	// we don't request a restart unless the filelist differs

	server = false;

	if (!stricmp(COM_Argv(1), "self"))
	{
		servernode = 0;
		server = true;
		/// \bug should be but...
		//SV_SpawnServer();
	}
	else
	{
		// used in menu to connect to a server in the list
		if (netgame && !stricmp(COM_Argv(1), "node"))
		{
			servernode = (SINT8)atoi(COM_Argv(2));
		}
		else if (netgame)
		{
			CONS_Printf(M_GetText("You cannot connect while in a game. End this game first.\n"));
			return;
		}
		else if (I_NetOpenSocket)
		{
			I_NetOpenSocket();
			netgame = true;
			multiplayer = true;

			if (!stricmp(COM_Argv(1), "any"))
				servernode = BROADCASTADDR;
			else if (I_NetMakeNodewPort && COM_Argc() >= 3)
				servernode = I_NetMakeNodewPort(COM_Argv(1), COM_Argv(2));
			else if (I_NetMakeNodewPort)
				servernode = I_NetMakeNode(COM_Argv(1));
			else
			{
				CONS_Alert(CONS_ERROR, M_GetText("There is no server identification with this network driver\n"));
				D_CloseConnection();
				return;
			}
		}
		else
			CONS_Alert(CONS_ERROR, M_GetText("There is no network driver\n"));
	}

	if (splitscreen != cv_splitplayers.value-1)
	{
		splitscreen = cv_splitplayers.value-1;
		SplitScreen_OnChange();
	}
	botingame = false;
	botskin = 0;
	CL_ConnectToServer();
}
#endif

static void ResetNode(INT32 node);

//
// CL_ClearPlayer
//
// Clears the player data so that a future client can use this slot
//
void CL_ClearPlayer(INT32 playernum)
{
	if (players[playernum].mo)
	{
		// Don't leave a NiGHTS ghost!
		if ((players[playernum].pflags & PF_NIGHTSMODE) && players[playernum].mo->tracer)
			P_RemoveMobj(players[playernum].mo->tracer);
		P_RemoveMobj(players[playernum].mo);
	}
	memset(&players[playernum], 0, sizeof (player_t));
}

//
// CL_RemovePlayer
//
// Removes a player from the current game
//
void CL_RemovePlayer(INT32 playernum, INT32 reason)
{
	// Sanity check: exceptional cases (i.e. c-fails) can cause multiple
	// kick commands to be issued for the same player.
	if (!playeringame[playernum])
		return;

	demo_extradata[playernum] |= DXD_PLAYSTATE;

	if (server && !demo.playback)
	{
		INT32 node = playernode[playernum];
		//playerpernode[node] = 0; // It'd be better to remove them all at once, but ghosting happened, so continue to let CL_RemovePlayer do it one-by-one
		playerpernode[node]--;
		if (playerpernode[node] <= 0)
		{
			// If a resynch was in progress, well, it no longer needs to be.
			SV_InitResynchVars(node);

			nodeingame[node] = false;
			Net_CloseConnection(node);
			ResetNode(node);
		}
	}

	if (K_IsPlayerWanted(&players[playernum]))
		K_CalculateBattleWanted();

	if (gametype == GT_CTF)
		P_PlayerFlagBurst(&players[playernum], false); // Don't take the flag with you!

	// If in a special stage, redistribute the player's rings across
	// the remaining players.
	if (G_IsSpecialStage(gamemap))
	{
		INT32 i, count, increment, rings;

		for (i = 0, count = 0; i < MAXPLAYERS; i++)
		{
			if (playeringame[i])
				count++;
		}

		count--;
		rings = players[playernum].health - 1;
		increment = rings/count;

		for (i = 0; i < MAXPLAYERS; i++)
		{
			if (playeringame[i] && i != playernum)
			{
				if (rings < increment)
					P_GivePlayerRings(&players[i], rings);
				else
					P_GivePlayerRings(&players[i], increment);

				rings -= increment;
			}
		}
	}

#ifdef HAVE_BLUA
	LUAh_PlayerQuit(&players[playernum], reason); // Lua hook for player quitting
#else
	(void)reason;
#endif

	// Reset player data
	CL_ClearPlayer(playernum);

	// remove avatar of player
	playeringame[playernum] = false;
	playernode[playernum] = UINT8_MAX;
	while (!playeringame[doomcom->numslots-1] && doomcom->numslots > 1)
		doomcom->numslots--;

	// Reset the name
	sprintf(player_names[playernum], "Player %d", playernum+1);

	player_name_changes[playernum] = 0;

	if (IsPlayerAdmin(playernum))
	{
		RemoveAdminPlayer(playernum); // don't stay admin after you're gone
	}

	if (playernum == displayplayers[0] && !demo.playback)
		displayplayers[0] = consoleplayer; // don't look through someone's view who isn't there

#ifdef HAVE_BLUA
	LUA_InvalidatePlayer(&players[playernum]);
#endif

	/*if (G_TagGametype()) //Check if you still have a game. Location flexible. =P
		P_CheckSurvivors();
	else*/ if (G_BattleGametype()) // SRB2Kart
		K_CheckBumpers();
	else if (G_RaceGametype())
		P_CheckRacers();

	// Reset startedInFreePlay
	{
		INT32 i;

		for (i = 0; i < MAXPLAYERS; i++)
		{
			if (playeringame[i] && !players[i].spectator)
				break;
		}

		if (i == MAXPLAYERS)
		{
			// Server was emptied, consider it FREE PLAY.
			startedInFreePlay = true;
		}
	}
}

void CL_Reset(void)
{
	if (metalrecording)
		G_StopMetalRecording();
	if (metalplayback)
		G_StopMetalDemo();
	if (demo.recording)
		G_CheckDemoStatus();

	// reset client/server code
	DEBFILE(va("\n-=-=-=-=-=-=-= Client reset =-=-=-=-=-=-=-\n\n"));

	if (servernode > 0 && servernode < MAXNETNODES)
	{
		nodeingame[(UINT8)servernode] = false;
		Net_CloseConnection(servernode);
	}
	D_CloseConnection(); // netgame = false
	multiplayer = false;
	servernode = 0;
	server = true;
	doomcom->numnodes = 1;
	doomcom->numslots = 1;
	SV_StopServer();
	SV_ResetServer();

	// make sure we don't leave any fileneeded gunk over from a failed join
	fileneedednum = 0;
	memset(fileneeded, 0, sizeof(fileneeded));

#ifndef NONET
	totalfilesrequestednum = 0;
	totalfilesrequestedsize = 0;
#endif
	firstconnectattempttime = 0;
	serverisfull = false;
	connectiontimeout = (tic_t)cv_nettimeout.value; //reset this temporary hack

#ifdef HAVE_CURL
	curl_failedwebdownload = false;
	curl_transfers = 0;
	curl_running = false;
	http_source[0] = '\0';
#endif

	// D_StartTitle should get done now, but the calling function will handle it
}

#ifndef NONET
static void Command_GetPlayerNum(void)
{
	INT32 i;

	for (i = 0; i < MAXPLAYERS; i++)
		if (playeringame[i])
		{
			if (serverplayer == i)
				CONS_Printf(M_GetText("num:%2d  node:%2d  %s\n"), i, playernode[i], player_names[i]);
			else
				CONS_Printf(M_GetText("\x82num:%2d  node:%2d  %s\n"), i, playernode[i], player_names[i]);
		}
}

SINT8 nametonum(const char *name)
{
	INT32 playernum, i;

	if (!strcmp(name, "0"))
		return 0;

	playernum = (SINT8)atoi(name);

	if (playernum < 0 || playernum >= MAXPLAYERS)
		return -1;

	if (playernum)
	{
		if (playeringame[playernum])
			return (SINT8)playernum;
		else
			return -1;
	}

	for (i = 0; i < MAXPLAYERS; i++)
		if (playeringame[i] && !stricmp(player_names[i], name))
			return (SINT8)i;

	CONS_Printf(M_GetText("There is no player named \"%s\"\n"), name);

	return -1;
}

/** Lists all players and their player numbers.
  *
  * \sa Command_GetPlayerNum
  */
static void Command_Nodes(void)
{
	INT32 i;
	size_t maxlen = 0;
	const char *address;

	for (i = 0; i < MAXPLAYERS; i++)
	{
		const size_t plen = strlen(player_names[i]);
		if (playeringame[i] && plen > maxlen)
			maxlen = plen;
	}

	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (playeringame[i])
		{
			CONS_Printf("%.2u: %*s", i, (int)maxlen, player_names[i]);
			CONS_Printf(" - %.2d", playernode[i]);
			if (I_GetNodeAddress && (address = I_GetNodeAddress(playernode[i])) != NULL)
				CONS_Printf(" - %s", address);

			if (IsPlayerAdmin(i))
				CONS_Printf(M_GetText(" (verified admin)"));

			if (players[i].spectator)
				CONS_Printf(M_GetText(" (spectator)"));

			CONS_Printf("\n");
		}
	}
}

static void Command_Ban(void)
{
	if (COM_Argc() < 2)
	{
		CONS_Printf(M_GetText("ban <playername/playernum> <reason>: ban and kick a player\n"));
		return;
	}

	if (!netgame) // Don't kick Tails in splitscreen!
	{
		CONS_Printf(M_GetText("This only works in a netgame.\n"));
		return;
	}

	if (server || IsPlayerAdmin(consoleplayer))
	{
		XBOXSTATIC UINT8 buf[3 + MAX_REASONLENGTH];
		UINT8 *p = buf;
		const SINT8 pn = nametonum(COM_Argv(1));

		if (pn == -1 || pn == 0)
			return;

		WRITEUINT8(p, pn);

		if (COM_Argc() == 2)
		{
			WRITEUINT8(p, KICK_MSG_BANNED);
			SendNetXCmd(XD_KICK, &buf, 2);
		}
		else
		{
			size_t i, j = COM_Argc();
			char message[MAX_REASONLENGTH];

			//Steal from the motd code so you don't have to put the reason in quotes.
			strlcpy(message, COM_Argv(2), sizeof message);
			for (i = 3; i < j; i++)
			{
				strlcat(message, " ", sizeof message);
				strlcat(message, COM_Argv(i), sizeof message);
			}

			WRITEUINT8(p, KICK_MSG_CUSTOM_BAN);
			WRITESTRINGN(p, message, MAX_REASONLENGTH);
			SendNetXCmd(XD_KICK, &buf, p - buf);
		}
	}
	else
		CONS_Printf(M_GetText("Only the server or a remote admin can use this.\n"));
}

static void Command_BanIP(void)
{
	size_t ac = COM_Argc();

	if (ac < 2)
	{
		CONS_Printf(M_GetText("banip <ip> [<reason>]: ban an ip address\n"));
		return;
	}

	if (server) // Only the server can use this, otherwise does nothing.
	{
		char *addressInput = Z_StrDup(COM_Argv(1));

		const char *address = NULL;
		const char *mask = NULL;

		const char *reason = NULL;

		address = strtok(addressInput, "/");
		mask = strtok(NULL, "");

		if (ac > 2)
		{
			reason = COM_Argv(2);
		}

		if (I_SetBanAddress && I_SetBanAddress(address, mask))
		{
			if (reason)
			{
				CONS_Printf(
					"Banned IP address %s%s for: %s\n",
					address,
					(mask && (strlen(mask) > 0)) ? va("/%s", mask) : "",
					reason
				);
			}
			else
			{
				CONS_Printf(
					"Banned IP address %s%s\n",
					address,
					(mask && (strlen(mask) > 0)) ? va("/%s", mask) : ""
				);
			}

			if (I_SetUnbanTime)
				I_SetUnbanTime(NO_BAN_TIME);

			if (I_SetBanUsername)
				I_SetBanUsername(NULL);

			if (I_SetBanReason)
				I_SetBanReason(reason);

			D_SaveBan();
		}
		else
		{
			return;
		}
	}
}

static void Command_Kick(void)
{
	if (COM_Argc() < 2)
	{
		CONS_Printf(M_GetText("kick <playername/playernum> <reason>: kick a player\n"));
		return;
	}

	if (!netgame) // Don't kick Tails in splitscreen!
	{
		CONS_Printf(M_GetText("This only works in a netgame.\n"));
		return;
	}

	if (server || IsPlayerAdmin(consoleplayer))
	{
		XBOXSTATIC UINT8 buf[3 + MAX_REASONLENGTH];
		UINT8 *p = buf;
		const SINT8 pn = nametonum(COM_Argv(1));

		if (pn == -1 || pn == 0)
			return;

		if (server)
		{
			// Special case if we are trying to kick a player who is downloading the game state:
			// trigger a timeout instead of kicking them, because a kick would only
			// take effect after they have finished downloading
			if (sendingsavegame[playernode[pn]])
			{
				Net_ConnectionTimeout(playernode[pn]);
				return;
			}
		}

		WRITESINT8(p, pn);

		if (COM_Argc() == 2)
		{
			WRITEUINT8(p, KICK_MSG_GO_AWAY);
			SendNetXCmd(XD_KICK, &buf, 2);
		}
		else
		{
			size_t i, j = COM_Argc();
			char message[MAX_REASONLENGTH];

			//Steal from the motd code so you don't have to put the reason in quotes.
			strlcpy(message, COM_Argv(2), sizeof message);
			for (i = 3; i < j; i++)
			{
				strlcat(message, " ", sizeof message);
				strlcat(message, COM_Argv(i), sizeof message);
			}

			WRITEUINT8(p, KICK_MSG_CUSTOM_KICK);
			WRITESTRINGN(p, message, MAX_REASONLENGTH);
			SendNetXCmd(XD_KICK, &buf, p - buf);
		}
	}
	else
		CONS_Printf(M_GetText("Only the server or a remote admin can use this.\n"));
}
#endif

static void Got_KickCmd(UINT8 **p, INT32 playernum)
{
	INT32 pnum, msg;
	XBOXSTATIC char buf[3 + MAX_REASONLENGTH];
	char *reason = buf;
	kickreason_t kickreason = KR_KICK;
	UINT32 banMinutes = 0;

	pnum = READUINT8(*p);
	msg = READUINT8(*p);

	if (pnum == serverplayer && IsPlayerAdmin(playernum))
	{
		CONS_Printf(M_GetText("Server is being shut down remotely. Goodbye!\n"));

		if (server)
			COM_BufAddText("quit\n");

		return;
	}

	// Is playernum authorized to make this kick?
	if (playernum != serverplayer && !IsPlayerAdmin(playernum)
		/*&& !(playerpernode[playernode[playernum]] == 2
		//&& nodetoplayer2[playernode[playernum]] == pnum)*/)
	{
		// We received a kick command from someone who isn't the
		// server or admin, and who isn't in splitscreen removing
		// player 2. Thus, it must be someone with a modified
		// binary, trying to kick someone but without having
		// authorization.

		// We deal with this by changing the kick reason to
		// "consistency failure" and kicking the offending user
		// instead.

		CONS_Alert(CONS_WARNING, M_GetText("Illegal kick command received from %s for player %d\n"), player_names[playernum], pnum);

		// In debug, print a longer message with more details.
		// TODO Callum: Should we translate this?
/*
		CONS_Debug(DBG_NETPLAY,
			"So, you must be asking, why is this an illegal kick?\n"
			"Well, let's take a look at the facts, shall we?\n"
			"\n"
			"playernum (this is the guy who did it), he's %d.\n"
			"pnum (the guy he's trying to kick) is %d.\n"
			"playernum's node is %d.\n"
			"That node has %d players.\n"
			"Player 2 on that node is %d.\n"
			"pnum's node is %d.\n"
			"That node has %d players.\n"
			"Player 2 on that node is %d.\n"
			"\n"
			"If you think this is a bug, please report it, including all of the details above.\n",
				playernum, pnum,
				playernode[playernum], playerpernode[playernode[playernum]],
				nodetoplayer2[playernode[playernum]],
				playernode[pnum], playerpernode[playernode[pnum]],
				nodetoplayer2[playernode[pnum]]);
*/
		pnum = playernum;
		msg = KICK_MSG_CON_FAIL;
	}

	if (msg == KICK_MSG_CUSTOM_BAN || msg == KICK_MSG_CUSTOM_KICK)
	{
		READSTRINGN(*p, reason, MAX_REASONLENGTH+1);
	}

	//CONS_Printf("\x82%s ", player_names[pnum]);

	// Save bans here. Used to be split between here and the actual command, depending on
	// whenever the server did it or a remote admin did it, but it's simply more convenient
	// to keep it all in one place.
	if (server)
	{
		if (msg == KICK_MSG_GO_AWAY || msg == KICK_MSG_CUSTOM_KICK)
		{
			// Kick as a temporary ban.
			banMinutes = cv_kicktime.value;
		}

		if (msg == KICK_MSG_BANNED || msg == KICK_MSG_CUSTOM_BAN || banMinutes)
		{
			if (I_Ban && !I_Ban(playernode[(INT32)pnum]))
			{
				CONS_Alert(CONS_WARNING, M_GetText("Ban failed. Invalid node?\n"));
			}
			else
			{
				if (I_SetBanUsername)
					I_SetBanUsername(player_names[pnum]);

				if (I_SetBanReason)
					I_SetBanReason(reason);

				if (I_SetUnbanTime)
				{
					if (banMinutes)
						I_SetUnbanTime(time(NULL) + (banMinutes * 60));
					else
						I_SetUnbanTime(NO_BAN_TIME);
				}

				D_SaveBan();
			}
		}
	}

	if (msg == KICK_MSG_PLAYER_QUIT)
		S_StartSound(NULL, sfx_leave); // intended leave
	else
		S_StartSound(NULL, sfx_syfail); // he he he

	switch (msg)
	{
		case KICK_MSG_GO_AWAY:
			HU_AddChatText(va("\x82*%s has been kicked (Go away)", player_names[pnum]), false);
			kickreason = KR_KICK;
			break;
		case KICK_MSG_PING_HIGH:
			HU_AddChatText(va("\x82*%s left the game (Broke delay limit)", player_names[pnum]), false);
			kickreason = KR_PINGLIMIT;
			break;
		case KICK_MSG_CON_FAIL:
			HU_AddChatText(va("\x82*%s left the game (Synch Failure)", player_names[pnum]), false);
			kickreason = KR_SYNCH;

			if (M_CheckParm("-consisdump")) // Helps debugging some problems
			{
				INT32 i;

				CONS_Printf(M_GetText("Player kicked is #%d, dumping consistency...\n"), pnum);

				for (i = 0; i < MAXPLAYERS; i++)
				{
					if (!playeringame[i])
						continue;
					CONS_Printf("-------------------------------------\n");
					CONS_Printf("Player %d: %s\n", i, player_names[i]);
					CONS_Printf("Skin: %d\n", players[i].skin);
					CONS_Printf("Color: %d\n", players[i].skincolor);
					CONS_Printf("Speed: %d\n",players[i].speed>>FRACBITS);
					if (players[i].mo)
					{
						if (!players[i].mo->skin)
							CONS_Printf("Mobj skin: NULL!\n");
						else
							CONS_Printf("Mobj skin: %s\n", ((skin_t *)players[i].mo->skin)->name);
						CONS_Printf("Position: %d, %d, %d\n", players[i].mo->x, players[i].mo->y, players[i].mo->z);
						if (!players[i].mo->state)
							CONS_Printf("State: S_NULL\n");
						else
							CONS_Printf("State: %d\n", (statenum_t)(players[i].mo->state-states));
					}
					else
						CONS_Printf("Mobj: NULL\n");
					CONS_Printf("-------------------------------------\n");
				}
			}
			break;
		case KICK_MSG_TIMEOUT:
			HU_AddChatText(va("\x82*%s left the game (Connection timeout)", player_names[pnum]), false);
			kickreason = KR_TIMEOUT;
			break;
		case KICK_MSG_PLAYER_QUIT:
			if (netgame) // not splitscreen/bots
				HU_AddChatText(va("\x82*%s left the game", player_names[pnum]), false);
			kickreason = KR_LEAVE;
			break;
		case KICK_MSG_GRIEF:
			HU_AddChatText(va("\x82*%s has been kicked (Automatic grief detection)", player_names[pnum]), false);
			kickreason = KR_KICK;
			break;
		case KICK_MSG_BANNED:
			HU_AddChatText(va("\x82*%s has been banned (Don't come back)", player_names[pnum]), false);
			kickreason = KR_BAN;
			break;
		case KICK_MSG_CUSTOM_KICK:
			HU_AddChatText(va("\x82*%s has been kicked (%s)", player_names[pnum], reason), false);
			kickreason = KR_KICK;
			break;
		case KICK_MSG_CUSTOM_BAN:
			HU_AddChatText(va("\x82*%s has been banned (%s)", player_names[pnum], reason), false);
			kickreason = KR_BAN;
			break;
	}

	if (playernode[pnum] == playernode[consoleplayer])
	{
#ifdef DUMPCONSISTENCY
		if (msg == KICK_MSG_CON_FAIL) SV_SavedGame();
#endif
		D_QuitNetGame();
		CL_Reset();
		D_StartTitle();

		if (msg == KICK_MSG_CON_FAIL)
			M_StartMessage(M_GetText("Server closed connection\n(Synch failure)\nPress ESC\n"), NULL, MM_NOTHING);
		else if (msg == KICK_MSG_PING_HIGH)
			M_StartMessage(M_GetText("Server closed connection\n(Broke delay limit)\nPress ESC\n"), NULL, MM_NOTHING);
		else if (msg == KICK_MSG_BANNED)
			M_StartMessage(M_GetText("You have been banned by the server\n\nPress ESC\n"), NULL, MM_NOTHING);
		else if (msg == KICK_MSG_CUSTOM_KICK)
			M_StartMessage(va(M_GetText("You have been kicked\n(%s)\n\nPress ESC\n"), reason), NULL, MM_NOTHING);
		else if (msg == KICK_MSG_CUSTOM_BAN)
			M_StartMessage(va(M_GetText("You have been banned\n(%s)\n\nPress ESC\n"), reason), NULL, MM_NOTHING);
		else
			M_StartMessage(M_GetText("You have been kicked by the server\n\nPress ESC\n"), NULL, MM_NOTHING);
	}
	else if (server)
	{
		// Sal: Because kicks (and a lot of other commands) are player-based, we can't tell which player pnum is on the node from a glance.
		// When we want to remove everyone from a node, we have to get the kicked player's node, then remove everyone on that node manually so we don't miss any.
		// This avoids the bugs with older SRB2 version's online splitscreen kicks, specifically ghosting.
		// On top of this, it can't just be a CL_RemovePlayer call; it has to be a server-sided.
		// Clients don't bother setting any nodes for anything but THE server player (even ignoring the server's extra players!), so it'll often remove everyone because they all have node -1/255, insta-desync!
		// And yes. This is a netxcmd wrap for just CL_RemovePlayer! :V

#define removethisplayer(otherp) \
	if (otherp >= 0) \
	{ \
		buf[0] = (UINT8)otherp; \
		if (otherp != pnum) \
		{ \
			HU_AddChatText(va("\x82*%s left the game (Joined with %s)", player_names[otherp], player_names[pnum]), false); \
			buf[1] = KR_LEAVE; \
		} \
		else \
			buf[1] = (UINT8)kickreason; \
		SendNetXCmd(XD_REMOVEPLAYER, &buf, 2); \
		otherp = -1; \
	}
		removethisplayer(nodetoplayer[playernode[pnum]])
		removethisplayer(nodetoplayer2[playernode[pnum]])
		removethisplayer(nodetoplayer3[playernode[pnum]])
		removethisplayer(nodetoplayer4[playernode[pnum]])
#undef removethisplayer
	}
}

#ifdef HAVE_CURL
/** Add a login for HTTP downloads. If the
  * user/password is missing, remove it.
  *
  * \sa Command_list_http_logins
  */
static void Command_set_http_login (void)
{
	HTTP_login  *login;
	HTTP_login **prev_next;

	if (COM_Argc() < 2)
	{
		CONS_Printf(
				"set_http_login <URL> [user:password]: Set or remove a login to "
				"authenticate HTTP downloads.\n"
		);
		return;
	}

	login = CURLGetLogin(COM_Argv(1), &prev_next);

	if (COM_Argc() == 2)
	{
		if (login)
		{
			(*prev_next) = login->next;
			CONS_Printf("Login for '%s' removed.\n", login->url);
			Z_Free(login);
		}
	}
	else
	{
		if (login)
			Z_Free(login->auth);
		else
		{
			login = ZZ_Alloc(sizeof *login);
			login->url  = Z_StrDup(COM_Argv(1));
		}

		login->auth = Z_StrDup(COM_Argv(2));

		login->next = curl_logins;
		curl_logins = login;
	}
}

/** List logins for HTTP downloads.
  *
  * \sa Command_set_http_login
  */
static void Command_list_http_logins (void)
{
	HTTP_login *login;

	for (
			login = curl_logins;
			login;
			login = login->next
	){
		CONS_Printf(
				"'%s' -> '%s'\n",
				login->url,
				login->auth
		);
	}
}
#endif/*HAVE_CURL*/

static CV_PossibleValue_t netticbuffer_cons_t[] = {{0, "MIN"}, {3, "MAX"}, {0, NULL}};
consvar_t cv_netticbuffer = {"netticbuffer", "1", CV_SAVE, netticbuffer_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};

static void Joinable_OnChange(void);

consvar_t cv_allownewplayer = {"allowjoin", "On", CV_SAVE|CV_CALL, CV_OnOff, Joinable_OnChange, 0, NULL, NULL, 0, 0, NULL};

#ifdef VANILLAJOINNEXTROUND
consvar_t cv_joinnextround = {"joinnextround", "Off", CV_SAVE|CV_NETVAR, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL}; /// \todo not done
#endif

static CV_PossibleValue_t maxplayers_cons_t[] = {{2, "MIN"}, {MAXPLAYERS, "MAX"}, {0, NULL}};
consvar_t cv_maxplayers = {"maxplayers", "8", CV_SAVE|CV_CALL, maxplayers_cons_t, Joinable_OnChange, 0, NULL, NULL, 0, 0, NULL};

// Here for dedicated servers
static CV_PossibleValue_t discordinvites_cons_t[] = {{0, "Admins Only"}, {1, "Everyone"}, {0, NULL}};
consvar_t cv_discordinvites = {"discordinvites", "Everyone", CV_SAVE|CV_CALL, discordinvites_cons_t, Joinable_OnChange, 0, NULL, NULL, 0, 0, NULL};

static CV_PossibleValue_t resynchattempts_cons_t[] = {{0, "MIN"}, {20, "MAX"}, {0, NULL}};
consvar_t cv_resynchattempts = {"resynchattempts", "2", CV_SAVE, resynchattempts_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL	};
consvar_t cv_blamecfail = {"blamecfail", "Off", CV_SAVE, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL	};

// max file size to send to a player (in kilobytes)
static CV_PossibleValue_t maxsend_cons_t[] = {{0, "MIN"}, {51200, "MAX"}, {0, NULL}};
consvar_t cv_maxsend = {"maxsend", "MAX", CV_SAVE, maxsend_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_noticedownload = {"noticedownload", "Off", CV_SAVE, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL};

// Speed of file downloading (in packets per tic)
static CV_PossibleValue_t downloadspeed_cons_t[] = {{0, "MIN"}, {32, "MAX"}, {0, NULL}};
consvar_t cv_downloadspeed = {"downloadspeed", "MAX", CV_SAVE, downloadspeed_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};

static void Got_AddPlayer(UINT8 **p, INT32 playernum);
static void Got_RemovePlayer(UINT8 **p, INT32 playernum);

static void Joinable_OnChange(void)
{
	UINT8 buf[3];
	UINT8 *p = buf;
	UINT8 maxplayer;

	if (!server)
		return;

	maxplayer = (UINT8)(min((dedicated ? MAXPLAYERS-1 : MAXPLAYERS), cv_maxplayers.value));

	WRITEUINT8(p, maxplayer);
	WRITEUINT8(p, cv_allownewplayer.value);
	WRITEUINT8(p, cv_discordinvites.value);

	SendNetXCmd(XD_DISCORD, &buf, 3);
}

// called one time at init
void D_ClientServerInit(void)
{
	DEBFILE(va("- - -== SRB2Kart v%d.%d "VERSIONSTRING" debugfile ==- - -\n",
		VERSION, SUBVERSION));

#ifndef NONET
	COM_AddCommand("getplayernum", Command_GetPlayerNum);
	COM_AddCommand("kick", Command_Kick);
	COM_AddCommand("ban", Command_Ban);
	COM_AddCommand("banip", Command_BanIP);
	COM_AddCommand("clearbans", Command_ClearBans);
	COM_AddCommand("showbanlist", Command_ShowBan);
	COM_AddCommand("reloadbans", Command_ReloadBan);
	COM_AddCommand("connect", Command_connect);
	COM_AddCommand("nodes", Command_Nodes);
#ifdef HAVE_CURL
	COM_AddCommand("set_http_login", Command_set_http_login);
	COM_AddCommand("list_http_logins", Command_list_http_logins);
#endif
#ifdef PACKETDROP
	COM_AddCommand("drop", Command_Drop);
	COM_AddCommand("droprate", Command_Droprate);
#endif
#ifdef _DEBUG
	COM_AddCommand("numnodes", Command_Numnodes);
#endif
#endif

	RegisterNetXCmd(XD_KICK, Got_KickCmd);
	RegisterNetXCmd(XD_ADDPLAYER, Got_AddPlayer);
	RegisterNetXCmd(XD_REMOVEPLAYER, Got_RemovePlayer);
#ifndef NONET
#ifdef DUMPCONSISTENCY
	CV_RegisterVar(&cv_dumpconsistency);
#endif
	D_LoadBan(false);
#endif

	gametic = 0;
	localgametic = 0;

	// do not send anything before the real begin
	SV_StopServer();
	SV_ResetServer();
	if (dedicated)
		SV_SpawnServer();
}

static void ResetNode(INT32 node)
{
	nodeingame[node] = false;
	nodetoplayer[node] = -1;
	nodetoplayer2[node] = -1;
	nodetoplayer3[node] = -1;
	nodetoplayer4[node] = -1;
	nettics[node] = gametic;
	supposedtics[node] = gametic;
	nodewaiting[node] = 0;
	playerpernode[node] = 0;
	sendingsavegame[node] = false;
	bannednode[node].banid = SIZE_MAX;
	bannednode[node].timeleft = NO_BAN_TIME;
}

void SV_ResetServer(void)
{
	INT32 i;

	// +1 because this command will be executed in com_executebuffer in
	// tryruntic so gametic will be incremented, anyway maketic > gametic
	// is not an issue

	maketic = gametic + 1;
	neededtic = maketic;
	tictoclear = maketic;

	for (i = 0; i < MAXNETNODES; i++)
	{
		ResetNode(i);

		// Make sure resynch status doesn't get carried over!
		SV_InitResynchVars(i);
	}

	for (i = 0; i < MAXPLAYERS; i++)
	{
#ifdef HAVE_BLUA
		LUA_InvalidatePlayer(&players[i]);
#endif
		playeringame[i] = false;
		playernode[i] = UINT8_MAX;
		sprintf(player_names[i], "Player %d", i + 1);
		adminplayers[i] = -1; // Populate the entire adminplayers array with -1.
	}

	memset(player_name_changes, 0, sizeof player_name_changes);

	mynode = 0;
	cl_packetmissed = false;

	if (dedicated)
	{
		nodeingame[0] = true;
		serverplayer = 0;
	}
	else
		serverplayer = consoleplayer;

	if (server)
		servernode = 0;

	doomcom->numslots = 0;

	// clear server_context
	memset(server_context, '-', 8);

	DEBFILE("\n-=-=-=-=-=-=-= Server Reset =-=-=-=-=-=-=-\n\n");
}

static inline void SV_GenContext(void)
{
	UINT8 i;
	// generate server_context, as exactly 8 bytes of randomly mixed A-Z and a-z
	// (hopefully M_Random is initialized!! if not this will be awfully silly!)
	for (i = 0; i < 8; i++)
	{
		const char a = M_RandomKey(26*2);
		if (a < 26) // uppercase
			server_context[i] = 'A'+a;
		else // lowercase
			server_context[i] = 'a'+(a-26);
	}
}

//
// D_QuitNetGame
// Called before quitting to leave a net game
// without hanging the other players
//
void D_QuitNetGame(void)
{
	if (!netgame || !netbuffer)
		return;

	DEBFILE("===========================================================================\n"
	        "                  Quitting Game, closing connection\n"
	        "===========================================================================\n");

	// abort send/receive of files
	CloseNetFile();

	if (server)
	{
		INT32 i;

		netbuffer->packettype = PT_SERVERSHUTDOWN;
		for (i = 0; i < MAXNETNODES; i++)
			if (nodeingame[i])
				HSendPacket(i, true, 0, 0);
#ifdef MASTERSERVER
		if (serverrunning && netgame && cv_advertise.value) // see mserv.c Online()
			UnregisterServer();
#endif
	}
	else if (servernode > 0 && servernode < MAXNETNODES && nodeingame[(UINT8)servernode])
	{
		netbuffer->packettype = PT_CLIENTQUIT;
		HSendPacket(servernode, true, 0, 0);
	}

	D_CloseConnection();
	ClearAdminPlayers();

	DEBFILE("===========================================================================\n"
	        "                         Log finish\n"
	        "===========================================================================\n");
#ifdef DEBUGFILE
	if (debugfile)
	{
		fclose(debugfile);
		debugfile = NULL;
	}
#endif
}

// Adds a node to the game (player will follow at map change or at savegame....)
static inline void SV_AddNode(INT32 node)
{
	nettics[node] = gametic;
	supposedtics[node] = gametic;
	// little hack because the server connects to itself and puts
	// nodeingame when connected not here
	if (node)
		nodeingame[node] = true;
}

// Xcmd XD_ADDPLAYER
static void Got_AddPlayer(UINT8 **p, INT32 playernum)
{
	INT16 node, newplayernum;
	UINT8 splitscreenplayer = 0;
	UINT8 i;

	if (playernum != serverplayer && !IsPlayerAdmin(playernum))
	{
		// protect against hacked/buggy client
		CONS_Alert(CONS_WARNING, M_GetText("Illegal add player command received from %s\n"), player_names[playernum]);
		if (server)
		{
			XBOXSTATIC UINT8 buf[2];

			buf[0] = (UINT8)playernum;
			buf[1] = KICK_MSG_CON_FAIL;
			SendNetXCmd(XD_KICK, &buf, 2);
		}
		return;
	}

	node = READUINT8(*p);
	newplayernum = READUINT8(*p);
	splitscreenplayer = newplayernum/MAXPLAYERS;
	newplayernum %= MAXPLAYERS;

	// Clear player before joining, lest some things get set incorrectly
	CL_ClearPlayer(newplayernum);

	playeringame[newplayernum] = true;
	G_AddPlayer(newplayernum);
	if (newplayernum+1 > doomcom->numslots)
		doomcom->numslots = (INT16)(newplayernum+1);

	// the server is creating my player
	if (node == mynode)
	{
		playernode[newplayernum] = 0; // for information only

		if (splitscreenplayer)
		{
			displayplayers[splitscreenplayer] = newplayernum;
			DEBFILE(va("spawning one of my sister number %d\n", splitscreenplayer));
			if (splitscreenplayer == 1 && botingame)
				players[newplayernum].bot = 1;
		}
		else
		{
			consoleplayer = newplayernum;
			for (i = 0; i < MAXSPLITSCREENPLAYERS; i++)
				displayplayers[i] = newplayernum;
			DEBFILE("spawning me\n");
		}
		D_SendPlayerConfig();
		addedtogame = true;
	}

	players[newplayernum].splitscreenindex = splitscreenplayer;

	if (netgame)
	{
		if (node != mynode)
			S_StartSound(NULL, sfx_join);

		if (server && cv_showjoinaddress.value)
		{
			const char *address;
			if (I_GetNodeAddress && (address = I_GetNodeAddress(node)) != NULL)
				HU_AddChatText(va("\x82*Player %d has joined the game (node %d) (%s)", newplayernum+1, node, address), false);	// merge join notification + IP to avoid clogging console/chat.
		}
		else
			HU_AddChatText(va("\x82*Player %d has joined the game (node %d)", newplayernum+1, node), false);	// if you don't wanna see the join address.
	}

	if (server && multiplayer && motd[0] != '\0')
		COM_BufAddText(va("sayto %d %s\n", newplayernum, motd));

#ifdef HAVE_BLUA
	LUAh_PlayerJoin(newplayernum);
#endif

#ifdef HAVE_DISCORDRPC
	DRPC_UpdatePresence();
#endif
}

// Xcmd XD_REMOVEPLAYER
static void Got_RemovePlayer(UINT8 **p, INT32 playernum)
{
	SINT8 pnum, reason;

	if (playernum != serverplayer && !IsPlayerAdmin(playernum))
	{
		// protect against hacked/buggy client
		CONS_Alert(CONS_WARNING, M_GetText("Illegal remove player command received from %s\n"), player_names[playernum]);
		if (server)
		{
			XBOXSTATIC UINT8 buf[2];

			buf[0] = (UINT8)playernum;
			buf[1] = KICK_MSG_CON_FAIL;
			SendNetXCmd(XD_KICK, &buf, 2);
		}
		return;
	}

	pnum = READUINT8(*p);
	reason = READUINT8(*p);

	CL_RemovePlayer(pnum, reason);

#ifdef HAVE_DISCORDRPC
	DRPC_UpdatePresence();
#endif
}

static boolean SV_AddWaitingPlayers(void)
{
	INT32 node, n, newplayer = false;
	XBOXSTATIC UINT8 buf[2];
	UINT8 newplayernum = 0;

	// What is the reason for this? Why can't newplayernum always be 0?
	// Sal: Because the dedicated player is stupidly forced into players[0].....
	if (dedicated)
		newplayernum = 1;

	for (node = 0; node < MAXNETNODES; node++)
	{
		// splitscreen can allow 2+ players in one node
		for (; nodewaiting[node] > 0; nodewaiting[node]--)
		{
			newplayer = true;

			// search for a free playernum
			// we can't use playeringame since it is not updated here
			for (; newplayernum < MAXPLAYERS; newplayernum++)
			{
				for (n = 0; n < MAXNETNODES; n++)
					if (nodetoplayer[n] == newplayernum || nodetoplayer2[n] == newplayernum
						|| nodetoplayer3[n] == newplayernum || nodetoplayer4[n] == newplayernum)
						break;
				if (n == MAXNETNODES)
					break;
			}

			// should never happen since we check the playernum
			// before accepting the join
			I_Assert(newplayernum < MAXPLAYERS);

			playernode[newplayernum] = (UINT8)node;

			buf[0] = (UINT8)node;
			buf[1] = newplayernum;
			if (playerpernode[node] < 1)
				nodetoplayer[node] = newplayernum;
			else if (playerpernode[node] < 2)
			{
				nodetoplayer2[node] = newplayernum;
				buf[1] += MAXPLAYERS;
			}
			else if (playerpernode[node] < 3)
			{
				nodetoplayer3[node] = newplayernum;
				buf[1] += MAXPLAYERS*2;
			}
			else
			{
				nodetoplayer4[node] = newplayernum;
				buf[1] += MAXPLAYERS*3;
			}
			playerpernode[node]++;

			SendNetXCmd(XD_ADDPLAYER, &buf, 2);

			DEBFILE(va("Server added player %d node %d\n", newplayernum, node));
			// use the next free slot (we can't put playeringame[newplayernum] = true here)
			newplayernum++;
		}
	}

	return newplayer;
}

void CL_AddSplitscreenPlayer(void)
{
	if (cl_mode == CL_CONNECTED)
		CL_SendJoin();
}

void CL_RemoveSplitscreenPlayer(UINT8 p)
{
	XBOXSTATIC UINT8 buf[2];

	if (cl_mode != CL_CONNECTED)
		return;

	buf[0] = p;
	buf[1] = KICK_MSG_PLAYER_QUIT;
	SendNetXCmd(XD_KICK, &buf, 2);
}

// is there a game running
boolean Playing(void)
{
	return (server && serverrunning) || (client && cl_mode == CL_CONNECTED);
}

boolean SV_SpawnServer(void)
{
	if (demo.playback)
		G_StopDemo(); // reset engine parameter
	if (metalplayback)
		G_StopMetalDemo();

	if (!serverrunning)
	{
		CONS_Printf(M_GetText("Starting Server....\n"));
		serverrunning = true;
		SV_ResetServer();
		SV_GenContext();
		if (netgame && I_NetOpenSocket)
		{
			I_NetOpenSocket();
		}

		// non dedicated server just connect to itself
		if (!dedicated)
			CL_ConnectToServer();
		else doomcom->numslots = 1;
	}

	return SV_AddWaitingPlayers();
}

void SV_StopServer(void)
{
	tic_t i;

	if (gamestate == GS_INTERMISSION)
		Y_EndIntermission();
	if (gamestate == GS_VOTING)
		Y_EndVote();
	gamestate = wipegamestate = GS_NULL;

	localtextcmd[0] = 0;
	localtextcmd2[0] = 0;
	localtextcmd3[0] = 0;
	localtextcmd4[0] = 0;

	for (i = firstticstosend; i < firstticstosend + TICQUEUE; i++)
		D_Clearticcmd(i);

	consoleplayer = 0;
	cl_mode = CL_ABORTED;
	maketic = gametic+1;
	neededtic = maketic;
	serverrunning = false;
}

// called at singleplayer start and stopdemo
void SV_StartSinglePlayerServer(void)
{
	server = true;
	netgame = false;
	multiplayer = false;
	gametype = GT_RACE; //srb2kart

	// no more tic the game with this settings!
	SV_StopServer();

	if (splitscreen)
		multiplayer = true;
}

static void SV_SendRefuse(INT32 node, const char *reason)
{
	strcpy(netbuffer->u.serverrefuse.reason, reason);

	netbuffer->packettype = PT_SERVERREFUSE;
	HSendPacket(node, false, 0, strlen(netbuffer->u.serverrefuse.reason) + 1);
	Net_CloseConnection(node);
}

// used at txtcmds received to check packetsize bound
static size_t TotalTextCmdPerTic(tic_t tic)
{
	INT32 i;
	size_t total = 1; // num of textcmds in the tic (ntextcmd byte)

	for (i = 0; i < MAXPLAYERS; i++)
	{
		UINT8 *textcmd = D_GetExistingTextcmd(tic, i);
		if ((!i || playeringame[i]) && textcmd)
			total += 2 + textcmd[0]; // "+2" for size and playernum
	}

	return total;
}

/** Called when a PT_CLIENTJOIN packet is received
  *
  * \param node The packet sender
  *
  */
static void HandleConnect(SINT8 node)
{
	// Sal: Dedicated mode is INCREDIBLY hacked together.
	// If a server filled out, then it'd overwrite the host and turn everyone into weird husks.....
	// It's too much effort to legimately fix right now. Just prevent it from reaching that state.
	UINT8 maxplayers = min((dedicated ? MAXPLAYERS-1 : MAXPLAYERS), cv_maxplayers.value);
	UINT8 connectedplayers = 0;

	for (UINT8 i = dedicated ? 1 : 0; i < MAXPLAYERS; i++)
		if (playernode[i] != UINT8_MAX) // We use this to count players because it is affected by SV_AddWaitingPlayers when more than one client joins on the same tic, unlike playeringame and D_NumPlayers. UINT8_MAX denotes no node for that player
			connectedplayers++;

	if (bannednode && bannednode[node].banid != SIZE_MAX)
	{
		const char *reason = NULL;

		// Get the reason...
		if (!I_GetBanReason || (reason = I_GetBanReason(bannednode[node].banid)) == NULL)
			reason = "No reason given";

		if (bannednode[node].timeleft != NO_BAN_TIME)
		{
			 // these are fudged a little to allow it to sink in for impatient rejoiners
			int minutes = (bannednode[node].timeleft + 30) / 60;
			int hours = (minutes + 1) / 60;
			int days = (hours + 1) / 24;

			if (days)
			{
				SV_SendRefuse(node, va("K|%s\n(Time remaining: %d day%s)", reason, days, days > 1 ? "s" : ""));
			}
			else if (hours)
			{
				SV_SendRefuse(node, va("K|%s\n(Time remaining: %d hour%s)", reason, hours, hours > 1 ? "s" : ""));
			}
			else if (minutes)
			{
				SV_SendRefuse(node, va("K|%s\n(Time remaining: %d minute%s)", reason, minutes, minutes > 1 ? "s" : ""));
			}
			else
			{
				SV_SendRefuse(node, va("K|%s\n(Time remaining: <1 minute)", reason));
			}
		}
		else
		{
			SV_SendRefuse(node, va("B|%s", reason));
		}
	}
	else if (netbuffer->u.clientcfg._255 != 255 ||
			netbuffer->u.clientcfg.packetversion != PACKETVERSION)
	{
		SV_SendRefuse(node, "Incompatible packet formats.");
	}
	else if (strncmp(netbuffer->u.clientcfg.application, SRB2APPLICATION,
				sizeof netbuffer->u.clientcfg.application))
	{
		SV_SendRefuse(node, "Different SRB2Kart modifications\nare not compatible.");
	}
	else if (netbuffer->u.clientcfg.version != VERSION
		|| netbuffer->u.clientcfg.subversion != SUBVERSION)
	{
		SV_SendRefuse(node, va(M_GetText("Different SRB2Kart versions cannot\nplay a netgame!\n(server version %d.%d)"), VERSION, SUBVERSION));
	}
	else if (!cv_allownewplayer.value && node)
	{
		SV_SendRefuse(node, M_GetText("The server is not accepting\njoins for the moment."));
	}
	else if (connectedplayers >= maxplayers)
	{
		SV_SendRefuse(node, va(M_GetText("Maximum players reached: %d"), maxplayers));
	}
	else if (netgame && netbuffer->u.clientcfg.localplayers > 4) // Hacked client?
	{
		SV_SendRefuse(node, M_GetText("Too many players from\nthis node."));
	}
	else if (netgame && connectedplayers + netbuffer->u.clientcfg.localplayers > maxplayers)
	{
		SV_SendRefuse(node, va(M_GetText("Number of local players\nwould exceed maximum: %d"), maxplayers));
	}
	else if (netgame && !netbuffer->u.clientcfg.localplayers) // Stealth join?
	{
		SV_SendRefuse(node, M_GetText("No players from\nthis node."));
	}
	else
	{
#ifndef NONET
		boolean newnode = false;
#endif

		// client authorised to join
		nodewaiting[node] = (UINT8)(netbuffer->u.clientcfg.localplayers - playerpernode[node]);
		if (!nodeingame[node])
		{
			gamestate_t backupstate = gamestate;
#ifndef NONET
			newnode = true;
#endif

			SV_AddNode(node);

			/// \note Wait what???
			///       What if the gamestate takes more than one second to get downloaded?
			///       Or if a lagspike happens?
			// you get a free second before desynch checks. use it wisely.
			SV_InitResynchVars(node);

#ifdef VANILLAJOINNEXTROUND
			if (cv_joinnextround.value && gameaction == ga_nothing)
				G_SetGamestate(GS_WAITINGPLAYERS);
#endif
			if (!SV_SendServerConfig(node))
			{
				G_SetGamestate(backupstate);
				/// \note Shouldn't SV_SendRefuse be called before ResetNode?
				SV_SendRefuse(node, M_GetText("Server couldn't send info, please try again"));
				ResetNode(node); // Yeah, lets try it!
				/// \todo fix this !!!
				return; // restart the while
			}
			//if (gamestate != GS_LEVEL) // GS_INTERMISSION, etc?
			//	SV_SendPlayerConfigs(node); // send bare minimum player info
			G_SetGamestate(backupstate);
			DEBFILE("new node joined\n");
		}
#ifdef JOININGAME
		if (nodewaiting[node])
		{
			if (node && newnode)
			{
				SV_SendSaveGame(node); // send a complete game state
				DEBFILE("send savegame\n");
			}
			SV_AddWaitingPlayers();
			player_joining = true;
		}
#else
#ifndef NONET
		// I guess we have no use for this if we aren't doing mid-level joins?
		(void)newnode;
#endif
#endif
	}
}

/** Called when a PT_SERVERSHUTDOWN packet is received
  *
  * \param node The packet sender (should be the server)
  *
  */
static void HandleShutdown(SINT8 node)
{
	(void)node;
	D_QuitNetGame();
	CL_Reset();
	D_StartTitle();
	M_StartMessage(M_GetText("Server has shutdown\n\nPress Esc\n"), NULL, MM_NOTHING);
}

/** Called when a PT_NODETIMEOUT packet is received
  *
  * \param node The packet sender (should be the server)
  *
  */
static void HandleTimeout(SINT8 node)
{
	(void)node;
	D_QuitNetGame();
	CL_Reset();
	D_StartTitle();
	M_StartMessage(M_GetText("Server Timeout\n\nPress Esc\n"), NULL, MM_NOTHING);
}

#ifndef NONET
/** Called when a PT_SERVERINFO packet is received
  *
  * \param node The packet sender
  * \note What happens if the packet comes from a client or something like that?
  *
  */
static void HandleServerInfo(SINT8 node)
{
	char servername[MAXSERVERNAME];
	// compute ping in ms
	const tic_t ticnow = I_GetTime();
	const tic_t ticthen = (tic_t)LONG(netbuffer->u.serverinfo.time);
	const tic_t ticdiff = (ticnow - ticthen)*1000/NEWTICRATE;
	netbuffer->u.serverinfo.time = (tic_t)LONG(ticdiff);
	netbuffer->u.serverinfo.servername[MAXSERVERNAME-1] = 0;
	netbuffer->u.serverinfo.application
		[sizeof netbuffer->u.serverinfo.application - 1] = '\0';
	memcpy(servername, netbuffer->u.serverinfo.servername, MAXSERVERNAME);
	CopyCaretColors(netbuffer->u.serverinfo.servername, servername, MAXSERVERNAME);
	netbuffer->u.serverinfo.gametype = (UINT8)((netbuffer->u.serverinfo.gametype == VANILLA_GT_MATCH) ? GT_MATCH : GT_RACE);

	SL_InsertServer(&netbuffer->u.serverinfo, node);
}
#endif

/** Handles a packet received from a node that isn't in game
  *
  * \param node The packet sender
  * \todo Choose a better name, as the packet can also come from the server apparently?
  * \sa HandlePacketFromPlayer
  * \sa GetPackets
  *
  */
static void HandlePacketFromAwayNode(SINT8 node)
{
	if (node != servernode)
		DEBFILE(va("Received packet from unknown host %d\n", node));

// macro for packets that should only be sent by the server
// if it is NOT from the server, bail out and close the connection!
#define SERVERONLY \
			if (node != servernode) \
			{ \
				Net_CloseConnection(node); \
				break; \
			}
	switch (netbuffer->packettype)
	{
		case PT_ASKINFOVIAMS:
#if 0
			if (server && serverrunning)
			{
				INT32 clientnode;
				if (ms_RoomId < 0) // ignore if we're not actually on the MS right now
				{
					Net_CloseConnection(node); // and yes, close connection
					return;
				}
				clientnode = I_NetMakeNode(netbuffer->u.msaskinfo.clientaddr);
				if (clientnode != -1)
				{
					SV_SendServerInfo(clientnode, (tic_t)LONG(netbuffer->u.msaskinfo.time));
					SV_SendPlayerInfo(clientnode); // Send extra info
					Net_CloseConnection(clientnode);
					// Don't close connection to MS...
				}
				else
					Net_CloseConnection(node); // ...unless the IP address is not valid
			}
			else
				Net_CloseConnection(node); // you're not supposed to get it, so ignore it
#else
			Net_CloseConnection(node);
#endif
			break;

		case PT_TELLFILESNEEDED:
			if (server && serverrunning)
			{
				UINT8 *p;
				INT32 firstfile = netbuffer->u.filesneedednum;

				netbuffer->packettype = PT_MOREFILESNEEDED;
				netbuffer->u.filesneededcfg.first = firstfile;
				netbuffer->u.filesneededcfg.more = 0;

				p = PutFileNeeded(firstfile);

				HSendPacket(node, false, 0, p - ((UINT8 *)&netbuffer->u));
			}
			else // Shouldn't get this if you aren't the server...?
				Net_CloseConnection(node);
			break;

		case PT_MOREFILESNEEDED:
			if (server && serverrunning)
			{ // But wait I thought I'm the server?
				Net_CloseConnection(node);
				break;
			}
			SERVERONLY
			if (cl_mode == CL_ASKFULLFILELIST && netbuffer->u.filesneededcfg.first == fileneedednum)
			{
				D_ParseFileneeded(netbuffer->u.filesneededcfg.num, netbuffer->u.filesneededcfg.files, netbuffer->u.filesneededcfg.first);
				if (!netbuffer->u.filesneededcfg.more)
					cl_lastcheckedfilecount = UINT16_MAX; // Got the whole file list
			}
			break;

		case PT_ASKINFO:
			if (server && serverrunning)
			{
				SV_SendServerInfo(node, (tic_t)LONG(netbuffer->u.askinfo.time));
				SV_SendPlayerInfo(node); // Send extra info
			}
			Net_CloseConnection(node);
			break;

		case PT_SERVERREFUSE: // Negative response of client join request
			if (server && serverrunning)
			{ // But wait I thought I'm the server?
				Net_CloseConnection(node);
				break;
			}
			SERVERONLY
			if (cl_mode == CL_WAITJOINRESPONSE)
			{
				// Save the reason so it can be displayed after quitting the netgame
				char *reason = strdup(netbuffer->u.serverrefuse.reason);
				if (!reason)
					I_Error("Out of memory!\n");

				if (strstr(reason, "Maximum players reached"))
				{
					serverisfull = true;
					//Special timeout for when refusing due to player cap. The client will wait 3 seconds between join requests when waiting for a slot, so we need this to be much longer
					//We set it back to the value of cv_nettimeout.value in CL_Reset
					connectiontimeout = NEWTICRATE*7;
					cl_mode = CL_ASKJOIN;
					free(reason);
					break;
				}

				D_QuitNetGame();
				CL_Reset();
				D_StartTitle();

				if (reason[1] == '|')
				{
					M_StartMessage(va("You have been %sfrom the server\n\nReason:\n%s",
						(reason[0] == 'B') ? "banned\n" : "temporarily\nkicked ",
						reason+2), NULL, MM_NOTHING);
				}
				else
				{
					M_StartMessage(va(M_GetText("Server refuses connection\n\nReason:\n%s"),
						reason), NULL, MM_NOTHING);
				}

				free(reason);

				// Will be reset by caller. Signals refusal.
				cl_mode = CL_ABORTED;
			}
			break;

		case PT_SERVERCFG: // Positive response of client join request
		{
			INT32 j;
			UINT8 *scp;

			if (server && serverrunning && node != servernode)
			{ // but wait I thought I'm the server?
				Net_CloseConnection(node);
				break;
			}
			SERVERONLY
			/// \note how would this happen? and is it doing the right thing if it does?
			if (!(cl_mode == CL_WAITJOINRESPONSE || cl_mode == CL_ASKJOIN))
				break;

			if (client)
			{
				maketic = gametic = neededtic = (tic_t)LONG(netbuffer->u.servercfg.gametic);
				if ((gametype = netbuffer->u.servercfg.gametype) >= NUMGAMETYPES)
					I_Error("Bad gametype in cliserv!");
				modifiedgame = netbuffer->u.servercfg.modifiedgame;
				for (j = 0; j < MAXPLAYERS; j++)
					adminplayers[j] = netbuffer->u.servercfg.adminplayers[j];
				memcpy(server_context, netbuffer->u.servercfg.server_context, 8);
			}

#ifdef HAVE_DISCORDRPC
			discordInfo.maxPlayers = netbuffer->u.servercfg.maxplayer;
			discordInfo.joinsAllowed = netbuffer->u.servercfg.allownewplayer;
			discordInfo.everyoneCanInvite = netbuffer->u.servercfg.discordinvites;
#endif

			nodeingame[(UINT8)servernode] = true;
			serverplayer = netbuffer->u.servercfg.serverplayer;
			doomcom->numslots = SHORT(netbuffer->u.servercfg.totalslotnum);
			mynode = netbuffer->u.servercfg.clientnode;
			if (serverplayer >= 0)
				playernode[(UINT8)serverplayer] = servernode;

			if (netgame)
#ifdef JOININGAME
				CONS_Printf(M_GetText("Join accepted, waiting for complete game state...\n"));
#else
				CONS_Printf(M_GetText("Join accepted, waiting for next level change...\n"));
#endif
			DEBFILE(va("Server accept join gametic=%u mynode=%d\n", gametic, mynode));

			memset(playeringame, 0, sizeof(playeringame));
			for (j = 0; j < MAXPLAYERS; j++)
			{
				if (netbuffer->u.servercfg.playerskins[j] == 0xFF
				 && netbuffer->u.servercfg.playercolor[j] == 0xFF)
					continue; // not in game

				playeringame[j] = true;
				SetPlayerSkinByNum(j, (INT32)netbuffer->u.servercfg.playerskins[j]);
				players[j].skincolor = netbuffer->u.servercfg.playercolor[j];
			}

			scp = netbuffer->u.servercfg.varlengthinputs;
			CV_LoadPlayerNames(&scp);
			CV_LoadNetVars(&scp);
#ifdef JOININGAME
			/// \note Wait. What if a Lua script uses some global custom variables synched with the NetVars hook?
			///       Shouldn't them be downloaded even at intermission time?
			///       Also, according to HandleConnect, the server will send the savegame even during intermission...
			/// Sryder 2018-07-05: If we don't want to send the player config another way we need to send the gamestate
			///                    At almost any gamestate there could be joiners... So just always send gamestate?
			cl_mode = ((server) ? CL_CONNECTED : CL_DOWNLOADSAVEGAME);
#else
			cl_mode = CL_CONNECTED;
#endif
			break;
		}

		// Handled in d_netfil.c
		case PT_FILEFRAGMENT:
			if (server)
			{ // But wait I thought I'm the server?
				Net_CloseConnection(node);
				break;
			}
			SERVERONLY
			Got_Filetxpak();
			break;

		case PT_REQUESTFILE:
			if (server)
			{
				if (!cv_downloading.value || !Got_RequestFilePak(node))
					Net_CloseConnection(node); // close connection if one of the requested files could not be sent, or you disabled downloading anyway
			}
			else
				Net_CloseConnection(node); // nope
			break;

		case PT_NODETIMEOUT:
		case PT_CLIENTQUIT:
			if (server)
				Net_CloseConnection(node);
			break;

		case PT_CLIENTCMD:
			break; // This is not an "unknown packet"

		case PT_SERVERTICS:
			// Do not remove my own server (we have just get a out of order packet)
			if (node == servernode)
				break;
			/* FALLTHRU */

		default:
			DEBFILE(va("unknown packet received (%d) from unknown host\n",netbuffer->packettype));
			Net_CloseConnection(node);
			break; // Ignore it

	}
#undef SERVERONLY
}

/** Checks ticcmd for "speed hacks"
  *
  * \param p Which player
  * \return True if player is hacking
  * \sa HandlePacketFromPlayer
  *
  */
static boolean CheckForSpeedHacks(UINT8 p)
{
	if (netcmds[maketic%TICQUEUE][p].forwardmove > MAXPLMOVE || netcmds[maketic%TICQUEUE][p].forwardmove < -MAXPLMOVE
		|| netcmds[maketic%TICQUEUE][p].sidemove > MAXPLMOVE || netcmds[maketic%TICQUEUE][p].sidemove < -MAXPLMOVE
		|| netcmds[maketic%TICQUEUE][p].driftturn > KART_FULLTURN || netcmds[maketic%TICQUEUE][p].driftturn < -KART_FULLTURN)
	{
		XBOXSTATIC char buf[2];
		CONS_Alert(CONS_WARNING, M_GetText("Illegal movement value received from node %d\n"), playernode[p]);
		//D_Clearticcmd(k);

		buf[0] = (char)p;
		buf[1] = KICK_MSG_CON_FAIL;
		SendNetXCmd(XD_KICK, &buf, 2);
		return true;
	}

	return false;
}

/** Handles a packet received from a node that is in game
  *
  * \param node The packet sender
  * \todo Choose a better name
  * \sa HandlePacketFromAwayNode
  * \sa GetPackets
  *
  */
static void HandlePacketFromPlayer(SINT8 node)
{FILESTAMP
	XBOXSTATIC INT32 netconsole;
	XBOXSTATIC tic_t realend, realstart;
	XBOXSTATIC UINT8 *pak, *txtpak, numtxtpak;
FILESTAMP

	txtpak = NULL;

	if (dedicated && node == 0)
		netconsole = 0;
	else
		netconsole = nodetoplayer[node];
#ifdef PARANOIA
	if (netconsole >= MAXPLAYERS)
		I_Error("bad table nodetoplayer: node %d player %d", doomcom->remotenode, netconsole);
#endif

	switch (netbuffer->packettype)
	{
// -------------------------------------------- SERVER RECEIVE ----------
		case PT_RESYNCHGET:
			if (client)
				break;
			SV_AcknowledgeResynchAck(node, netbuffer->u.resynchgot);
			break;
		case PT_CLIENTCMD:
		case PT_CLIENT2CMD:
		case PT_CLIENT3CMD:
		case PT_CLIENT4CMD:
		case PT_CLIENTMIS:
		case PT_CLIENT2MIS:
		case PT_CLIENT3MIS:
		case PT_CLIENT4MIS:
		case PT_NODEKEEPALIVE:
		case PT_NODEKEEPALIVEMIS:
			if (client)
				break;

			// Ignore tics from those not synched
			if (resynch_inprogress[node])
				break;

			// To save bytes, only the low byte of tic numbers are sent
			// Use ExpandTics to figure out what the rest of the bytes are
			realstart = ExpandTics(netbuffer->u.clientpak.client_tic, nettics[node]);
			realend = ExpandTics(netbuffer->u.clientpak.resendfrom, nettics[node]);

			if (netbuffer->packettype == PT_CLIENTMIS || netbuffer->packettype == PT_CLIENT2MIS
				|| netbuffer->packettype == PT_CLIENT3MIS || netbuffer->packettype == PT_CLIENT4MIS
				|| netbuffer->packettype == PT_NODEKEEPALIVEMIS
				|| supposedtics[node] < realend)
			{
				supposedtics[node] = realend;
			}
			// Discard out of order packet
			if (nettics[node] > realend)
			{
				DEBFILE(va("out of order ticcmd discarded nettics = %u\n", nettics[node]));
				break;
			}

			// Update the nettics
			nettics[node] = realend;

			// This should probably still timeout though, as the node should always have a player 1 number
			if (netconsole == -1)
				break;

			// If a client sends a ticcmd it should mean they are done receiving the savegame
			sendingsavegame[node] = false;

			// As long as clients send valid ticcmds, the server can keep running, so reset the timeout
			/// \todo Use a separate cvar for that kind of timeout?
			freezetimeout[node] = I_GetTime() + connectiontimeout;

			// Don't do anything for packets of type NODEKEEPALIVE?
			// Sryder 2018/07/01: Update the freezetimeout still!
			if (netbuffer->packettype == PT_NODEKEEPALIVE
				|| netbuffer->packettype == PT_NODEKEEPALIVEMIS)
				break;

			// Copy ticcmd
			G_MoveTiccmd(&netcmds[maketic%TICQUEUE][netconsole], &netbuffer->u.clientpak.cmd, 1);

			// Check ticcmd for "speed hacks"
			if (CheckForSpeedHacks((UINT8)netconsole))
				break;

			// Splitscreen cmd
			if (((netbuffer->packettype == PT_CLIENT2CMD || netbuffer->packettype == PT_CLIENT2MIS)
				|| (netbuffer->packettype == PT_CLIENT3CMD || netbuffer->packettype == PT_CLIENT3MIS)
				|| (netbuffer->packettype == PT_CLIENT4CMD || netbuffer->packettype == PT_CLIENT4MIS))
				&& (nodetoplayer2[node] >= 0))
			{
				G_MoveTiccmd(&netcmds[maketic%TICQUEUE][(UINT8)nodetoplayer2[node]],
					&netbuffer->u.client2pak.cmd2, 1);

				if (CheckForSpeedHacks((UINT8)nodetoplayer2[node]))
					break;
			}

			if (((netbuffer->packettype == PT_CLIENT3CMD || netbuffer->packettype == PT_CLIENT3MIS)
				|| (netbuffer->packettype == PT_CLIENT4CMD || netbuffer->packettype == PT_CLIENT4MIS))
				&& (nodetoplayer3[node] >= 0))
			{
				G_MoveTiccmd(&netcmds[maketic%TICQUEUE][(UINT8)nodetoplayer3[node]],
					&netbuffer->u.client3pak.cmd3, 1);

				if (CheckForSpeedHacks((UINT8)nodetoplayer3[node]))
					break;
			}

			if ((netbuffer->packettype == PT_CLIENT4CMD || netbuffer->packettype == PT_CLIENT4MIS)
				&& (nodetoplayer4[node] >= 0))
			{
				G_MoveTiccmd(&netcmds[maketic%TICQUEUE][(UINT8)nodetoplayer4[node]],
					&netbuffer->u.client4pak.cmd4, 1);

				if (CheckForSpeedHacks((UINT8)nodetoplayer4[node]))
					break;
			}

			// A delay before we check resynching
			// Used on join or just after a synch fail
			if (resynch_delay[node])
			{
				--resynch_delay[node];
				break;
			}
			// Check player consistancy during the level
			if (realstart <= gametic && realstart > gametic - TICQUEUE+1 && gamestate == GS_LEVEL
				&& consistancy[realstart%TICQUEUE] != SHORT(netbuffer->u.clientpak.consistancy))
			{
				SV_RequireResynch(node);

				if (cv_resynchattempts.value && resynch_score[node] <= (unsigned)cv_resynchattempts.value*250)
				{
					if (cv_blamecfail.value)
						CONS_Printf(M_GetText("Synch failure for player %d (%s); expected %hd, got %hd\n"),
							netconsole+1, player_names[netconsole],
							consistancy[realstart%TICQUEUE],
							SHORT(netbuffer->u.clientpak.consistancy));
					DEBFILE(va("Restoring player %d (synch failure) [%update] %d!=%d\n",
						netconsole, realstart, consistancy[realstart%TICQUEUE],
						SHORT(netbuffer->u.clientpak.consistancy)));
					break;
				}
				else
				{
					XBOXSTATIC UINT8 buf[3];

					buf[0] = (UINT8)netconsole;
					buf[1] = KICK_MSG_CON_FAIL;
					SendNetXCmd(XD_KICK, &buf, 2);
					DEBFILE(va("player %d kicked (synch failure) [%u] %d!=%d\n",
						netconsole, realstart, consistancy[realstart%TICQUEUE],
						SHORT(netbuffer->u.clientpak.consistancy)));
					break;
				}
			}
			else if (resynch_score[node])
				--resynch_score[node];
			break;
		case PT_BASICKEEPALIVE:
			if (client)
				break;

			// This should probably still timeout though, as the node should always have a player 1 number
			if (netconsole == -1)
				break;

			// If a client sends this it should mean they are done receiving the savegame
			sendingsavegame[node] = false;

			// As long as clients send keep alives, the server can keep running, so reset the timeout
			/// \todo Use a separate cvar for that kind of timeout?
			freezetimeout[node] = I_GetTime() + connectiontimeout;
			break;
		case PT_TEXTCMD:
		case PT_TEXTCMD2:
		case PT_TEXTCMD3:
		case PT_TEXTCMD4:
			if (netbuffer->packettype == PT_TEXTCMD2) // splitscreen special
				netconsole = nodetoplayer2[node];
			else if (netbuffer->packettype == PT_TEXTCMD3)
				netconsole = nodetoplayer3[node];
			else if (netbuffer->packettype == PT_TEXTCMD4)
				netconsole = nodetoplayer4[node];

			if (client)
				break;

			if (netconsole < 0 || netconsole >= MAXPLAYERS)
				Net_UnAcknowledgePacket(node);
			else
			{
				size_t j;
				tic_t tic = maketic;
				UINT8 *textcmd;

				// ignore if the textcmd has a reported size of zero
				// this shouldn't be sent at all
				if (!netbuffer->u.textcmd[0])
				{
					DEBFILE(va("GetPacket: Textcmd with size 0 detected! (node %u, player %d)\n",
						node, netconsole));
					Net_UnAcknowledgePacket(node);
					break;
				}

				// ignore if the textcmd size var is actually larger than it should be
				// BASEPACKETSIZE + 1 (for size) + textcmd[0] should == datalength
				if (netbuffer->u.textcmd[0] > (size_t)doomcom->datalength-BASEPACKETSIZE-1)
				{
					DEBFILE(va("GetPacket: Bad Textcmd packet size! (expected %d, actual %s, node %u, player %d)\n",
					netbuffer->u.textcmd[0], sizeu1((size_t)doomcom->datalength-BASEPACKETSIZE-1),
						node, netconsole));
					Net_UnAcknowledgePacket(node);
					break;
				}

				// check if tic that we are making isn't too large else we cannot send it :(
				// doomcom->numslots+1 "+1" since doomcom->numslots can change within this time and sent time
				j = software_MAXPACKETLENGTH
					- (netbuffer->u.textcmd[0]+2+BASESERVERTICSSIZE
					+ (doomcom->numslots+1)*sizeof(ticcmd_t));

				// search a tic that have enougth space in the ticcmd
				while ((textcmd = D_GetExistingTextcmd(tic, netconsole)),
					(TotalTextCmdPerTic(tic) > j || netbuffer->u.textcmd[0] + (textcmd ? textcmd[0] : 0) > MAXTEXTCMD)
					&& tic < firstticstosend + TICQUEUE)
					tic++;

				if (tic >= firstticstosend + TICQUEUE)
				{
					DEBFILE(va("GetPacket: Textcmd too long (max %s, used %s, mak %d, "
						"tosend %u, node %u, player %d)\n", sizeu1(j), sizeu2(TotalTextCmdPerTic(maketic)),
						maketic, firstticstosend, node, netconsole));
					Net_UnAcknowledgePacket(node);
					break;
				}

				// Make sure we have a buffer
				if (!textcmd) textcmd = D_GetTextcmd(tic, netconsole);

				DEBFILE(va("textcmd put in tic %u at position %d (player %d) ftts %u mk %u\n",
					tic, textcmd[0]+1, netconsole, firstticstosend, maketic));

				M_Memcpy(&textcmd[textcmd[0]+1], netbuffer->u.textcmd+1, netbuffer->u.textcmd[0]);
				textcmd[0] += (UINT8)netbuffer->u.textcmd[0];
			}
			break;
		case PT_NODETIMEOUT:
		case PT_CLIENTQUIT:
			if (client)
				break;

			// nodeingame will be put false in the execution of kick command
			// this allow to send some packets to the quitting client to have their ack back
			nodewaiting[node] = 0;
			if (netconsole != -1 && playeringame[netconsole])
			{
				XBOXSTATIC UINT8 buf[2];
				buf[0] = (UINT8)netconsole;
				if (netbuffer->packettype == PT_NODETIMEOUT)
					buf[1] = KICK_MSG_TIMEOUT;
				else
					buf[1] = KICK_MSG_PLAYER_QUIT;
				SendNetXCmd(XD_KICK, &buf, 2);
				//nodetoplayer[node] = -1;

				/*if (nodetoplayer2[node] != -1 && nodetoplayer2[node] >= 0
					&& playeringame[(UINT8)nodetoplayer2[node]])
				{
					buf[0] = nodetoplayer2[node];
					SendNetXCmd(XD_KICK, &buf, 2);
					nodetoplayer2[node] = -1;
				}

				if (nodetoplayer3[node] != -1 && nodetoplayer3[node] >= 0
					&& playeringame[(UINT8)nodetoplayer3[node]])
				{
					buf[0] = nodetoplayer3[node];
					SendNetXCmd(XD_KICK, &buf, 2);
					nodetoplayer3[node] = -1;
				}

				if (nodetoplayer4[node] != -1 && nodetoplayer4[node] >= 0
					&& playeringame[(UINT8)nodetoplayer4[node]])
				{
					buf[0] = nodetoplayer4[node];
					SendNetXCmd(XD_KICK, &buf, 2);
					nodetoplayer4[node] = -1;
				}*/
			}
			Net_CloseConnection(node);
			nodeingame[node] = false;
			break;
// -------------------------------------------- CLIENT RECEIVE ----------
		case PT_RESYNCHEND:
			// Only accept PT_RESYNCHEND from the server.
			if (node != servernode)
			{
				CONS_Alert(CONS_WARNING, M_GetText("%s received from non-host %d\n"), "PT_RESYNCHEND", node);

				if (server)
				{
					XBOXSTATIC UINT8 buf[2];
					buf[0] = (UINT8)node;
					buf[1] = KICK_MSG_CON_FAIL;
					SendNetXCmd(XD_KICK, &buf, 2);
				}

				break;
			}
			resynch_local_inprogress = false;

			P_SetRandSeed(netbuffer->u.resynchend.randomseed);

			if (gametype == GT_CTF)
				resynch_read_ctf(&netbuffer->u.resynchend);
			resynch_read_others(&netbuffer->u.resynchend);

			break;
		case PT_SERVERTICS:
			// Only accept PT_SERVERTICS from the server.
			if (node != servernode)
			{
				CONS_Alert(CONS_WARNING, M_GetText("%s received from non-host %d\n"), "PT_SERVERTICS", node);

				if (server)
				{
					XBOXSTATIC UINT8 buf[2];
					buf[0] = (UINT8)node;
					buf[1] = KICK_MSG_CON_FAIL;
					SendNetXCmd(XD_KICK, &buf, 2);
				}

				break;
			}

			realstart = ExpandTics(netbuffer->u.serverpak.starttic, maketic);
			realend = realstart + netbuffer->u.serverpak.numtics;

			if (!txtpak)
				txtpak = (UINT8 *)&netbuffer->u.serverpak.cmds[netbuffer->u.serverpak.numslots
					* netbuffer->u.serverpak.numtics];

			if (realend > gametic + BACKUPTICS)
				realend = gametic + BACKUPTICS;
			cl_packetmissed = realstart > neededtic;

			if (realstart <= neededtic && realend > neededtic)
			{
				tic_t i, j;
				pak = (UINT8 *)&netbuffer->u.serverpak.cmds;

				for (i = realstart; i < realend; i++)
				{
					// clear first
					D_Clearticcmd(i);

					// copy the tics
					pak = G_ScpyTiccmd(netcmds[i%TICQUEUE], pak,
						netbuffer->u.serverpak.numslots*sizeof (ticcmd_t));

					// copy the textcmds
					numtxtpak = *txtpak++;
					for (j = 0; j < numtxtpak; j++)
					{
						INT32 k = *txtpak++; // playernum
						const size_t txtsize = txtpak[0]+1;

						if (i >= gametic) // Don't copy old net commands
							M_Memcpy(D_GetTextcmd(i, k), txtpak, txtsize);
						txtpak += txtsize;
					}
				}

				neededtic = realend;
			}
			else
			{
				DEBFILE(va("frame not in bound: %u\n", neededtic));
				/*if (realend < neededtic - 2 * TICRATE || neededtic + 2 * TICRATE < realstart)
					I_Error("Received an out of order PT_SERVERTICS packet!\n"
							"Got tics %d-%d, needed tic %d\n\n"
							"Please report this crash on the Master Board,\n"
							"IRC or Discord so it can be fixed.\n", (INT32)realstart, (INT32)realend, (INT32)neededtic);*/
			}
			break;
		case PT_RESYNCHING:
			// Only accept PT_RESYNCHING from the server.
			if (node != servernode)
			{
				CONS_Alert(CONS_WARNING, M_GetText("%s received from non-host %d\n"), "PT_RESYNCHING", node);

				if (server)
				{
					XBOXSTATIC char buf[2];
					buf[0] = (char)node;
					buf[1] = KICK_MSG_CON_FAIL;
					SendNetXCmd(XD_KICK, &buf, 2);
				}

				break;
			}
			resynch_local_inprogress = true;
			CL_AcknowledgeResynch(&netbuffer->u.resynchpak);
			break;
		case PT_PING:
			// Only accept PT_PING from the server.
			if (node != servernode)
			{
				CONS_Alert(CONS_WARNING, M_GetText("%s received from non-host %d\n"), "PT_PING", node);

				if (server)
				{
					XBOXSTATIC char buf[2];
					buf[0] = (char)node;
					buf[1] = KICK_MSG_CON_FAIL;
					SendNetXCmd(XD_KICK, &buf, 2);
				}

				break;
			}

			//Update client ping table from the server.
			if (client)
			{
				UINT8 i;
				for (i = 0; i < MAXPLAYERS; i++)
					if (playeringame[i])
						playerpingtable[i] = (tic_t)netbuffer->u.pingtable[i];

				servermaxping = (tic_t)netbuffer->u.pingtable[MAXPLAYERS];
			}

			break;
		case PT_SERVERCFG:
			break;
		case PT_FILEFRAGMENT:
			// Only accept PT_FILEFRAGMENT from the server.
			if (node != servernode)
			{
				CONS_Alert(CONS_WARNING, M_GetText("%s received from non-host %d\n"), "PT_FILEFRAGMENT", node);

				if (server)
				{
					XBOXSTATIC UINT8 buf[2];
					buf[0] = (UINT8)node;
					buf[1] = KICK_MSG_CON_FAIL;
					SendNetXCmd(XD_KICK, &buf, 2);
				}

				break;
			}
			if (client)
				Got_Filetxpak();
			break;
		default:
			DEBFILE(va("UNKNOWN PACKET TYPE RECEIVED %d from host %d\n",
				netbuffer->packettype, node));
	} // end switch
}

/**	Handles all received packets, if any
  *
  * \todo Add details to this description (lol)
  *
  */
static void GetPackets(void)
{FILESTAMP
	XBOXSTATIC SINT8 node; // The packet sender
FILESTAMP

	player_joining = false;

	while (HGetPacket())
	{
		node = (SINT8)doomcom->remotenode;

		if (netbuffer->packettype == PT_CLIENTJOIN && server && !levelloading)
		{
			HandleConnect(node);
			continue;
		}
		if (node == servernode && client && cl_mode != CL_SEARCHING)
		{
			if (netbuffer->packettype == PT_SERVERSHUTDOWN)
			{
				HandleShutdown(node);
				continue;
			}
			if (netbuffer->packettype == PT_NODETIMEOUT)
			{
				HandleTimeout(node);
				continue;
			}
		}

#ifndef NONET
		if (netbuffer->packettype == PT_SERVERINFO)
		{
			HandleServerInfo(node);
			continue;
		}
#endif

		if (netbuffer->packettype == PT_PLAYERINFO)
			continue; // We do nothing with PLAYERINFO, that's for the MS browser.

		// Packet received from someone already playing
		if (nodeingame[node])
			HandlePacketFromPlayer(node);
		// Packet received from someone not playing
		else
			HandlePacketFromAwayNode(node);
	}
}

//
// NetUpdate
// Builds ticcmds for console player,
// sends out a packet
//
// no more use random generator, because at very first tic isn't yet synchronized
// Note: It is called consistAncy on purpose.
//
static INT16 Consistancy(void)
{
	INT32 i;
	UINT32 ret = 0;
#ifdef MOBJCONSISTANCY
	thinker_t *th;
	mobj_t *mo;
#endif

	DEBFILE(va("TIC %u ", gametic));

	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (!playeringame[i])
			ret ^= 0xCCCC;
		else if (!players[i].mo || gamestate != GS_LEVEL);
		else
		{
			ret += players[i].mo->x;
			ret -= players[i].mo->y;
			ret += players[i].kartstuff[k_itemtype]; // powers[pw_shield]
			ret *= i+1;
		}
	}
	// I give up
	// Coop desynching enemies is painful
	if (gamestate == GS_LEVEL)
		ret += P_GetRandSeed();

#ifdef MOBJCONSISTANCY
	if (!thinkercap.next)
	{
		DEBFILE(va("Consistancy = %u\n", ret));
		return ret;
	}
	if (gamestate == GS_LEVEL)
	{
		for (th = thinkercap.next; th != &thinkercap; th = th->next)
		{
			if (th->function.acp1 != (actionf_p1)P_MobjThinker)
				continue;

			mo = (mobj_t *)th;

			if (mo->flags & (MF_SPECIAL | MF_SOLID | MF_PUSHABLE | MF_BOSS | MF_MISSILE | MF_SPRING | MF_MONITOR | MF_FIRE | MF_ENEMY | MF_PAIN | MF_STICKY))
			{
				ret -= mo->type;
				ret += mo->x;
				ret -= mo->y;
				ret += mo->z;
				ret -= mo->momx;
				ret += mo->momy;
				ret -= mo->momz;
				ret += mo->angle;
				ret -= mo->flags;
				ret += mo->flags2;
				ret -= mo->eflags;
				if (mo->target)
				{
					ret += mo->target->type;
					ret -= mo->target->x;
					ret += mo->target->y;
					ret -= mo->target->z;
					ret += mo->target->momx;
					ret -= mo->target->momy;
					ret += mo->target->momz;
					ret -= mo->target->angle;
					ret += mo->target->flags;
					ret -= mo->target->flags2;
					ret += mo->target->eflags;
					ret -= mo->target->state - states;
					ret += mo->target->tics;
					ret -= mo->target->sprite;
					ret += mo->target->frame;
				}
				else
					ret ^= 0x3333;
				if (mo->tracer && mo->tracer->type != MT_OVERLAY)
				{
					ret += mo->tracer->type;
					ret -= mo->tracer->x;
					ret += mo->tracer->y;
					ret -= mo->tracer->z;
					ret += mo->tracer->momx;
					ret -= mo->tracer->momy;
					ret += mo->tracer->momz;
					ret -= mo->tracer->angle;
					ret += mo->tracer->flags;
					ret -= mo->tracer->flags2;
					ret += mo->tracer->eflags;
					ret -= mo->tracer->state - states;
					ret += mo->tracer->tics;
					ret -= mo->tracer->sprite;
					ret += mo->tracer->frame;
				}
				else
					ret ^= 0xAAAA;
				// SRB2Kart: We use hnext & hprev very extensively
				if (mo->hnext)
				{
					ret += mo->hnext->type;
					ret -= mo->hnext->x;
					ret += mo->hnext->y;
					ret -= mo->hnext->z;
					ret += mo->hnext->momx;
					ret -= mo->hnext->momy;
					ret += mo->hnext->momz;
					ret -= mo->hnext->angle;
					ret += mo->hnext->flags;
					ret -= mo->hnext->flags2;
					ret += mo->hnext->eflags;
					ret -= mo->hnext->state - states;
					ret += mo->hnext->tics;
					ret -= mo->hnext->sprite;
					ret += mo->hnext->frame;
				}
				else
					ret ^= 0x5555;
				if (mo->hprev)
				{
					ret += mo->hprev->type;
					ret -= mo->hprev->x;
					ret += mo->hprev->y;
					ret -= mo->hprev->z;
					ret += mo->hprev->momx;
					ret -= mo->hprev->momy;
					ret += mo->hprev->momz;
					ret -= mo->hprev->angle;
					ret += mo->hprev->flags;
					ret -= mo->hprev->flags2;
					ret += mo->hprev->eflags;
					ret -= mo->hprev->state - states;
					ret += mo->hprev->tics;
					ret -= mo->hprev->sprite;
					ret += mo->hprev->frame;
				}
				else
					ret ^= 0xCCCC;
				ret -= mo->state - states;
				ret += mo->tics;
				ret -= mo->sprite;
				ret += mo->frame;
			}
		}
	}
#endif

	DEBFILE(va("Consistancy = %u\n", (ret & 0xFFFF)));

	return (INT16)(ret & 0xFFFF);
}

// confusing, but this DOESN'T send PT_NODEKEEPALIVE, it sends PT_BASICKEEPALIVE
// used during wipes to tell the server that a node is still connected
static void CL_SendClientKeepAlive(void)
{
	netbuffer->packettype = PT_BASICKEEPALIVE;

	HSendPacket(servernode, false, 0, 0);
}

static void SV_SendServerKeepAlive(void)
{
	INT32 n;

	for (n = 1; n < MAXNETNODES; n++)
	{
		if (nodeingame[n])
		{
			netbuffer->packettype = PT_BASICKEEPALIVE;
			HSendPacket(n, false, 0, 0);
		}
	}
}

// send the client packet to the server
static void CL_SendClientCmd(void)
{
	size_t packetsize = 0;
	boolean mis = false;

	netbuffer->packettype = PT_CLIENTCMD;

	if (cl_packetmissed)
	{
		netbuffer->packettype = PT_CLIENTMIS;
		mis = true;
	}

	netbuffer->u.clientpak.resendfrom = (UINT8)(neededtic & UINT8_MAX);
	netbuffer->u.clientpak.client_tic = (UINT8)(gametic & UINT8_MAX);

	if (gamestate == GS_WAITINGPLAYERS)
	{
		// Send PT_NODEKEEPALIVE packet
		netbuffer->packettype = (mis ? PT_NODEKEEPALIVEMIS : PT_NODEKEEPALIVE);
		packetsize = sizeof (clientcmd_pak) - sizeof (ticcmd_t) - sizeof (INT16);
		HSendPacket(servernode, false, 0, packetsize);
	}
	else if (gamestate != GS_NULL)
	{
		packetsize = sizeof (clientcmd_pak);
		G_MoveTiccmd(&netbuffer->u.clientpak.cmd, &localcmds, 1);
		netbuffer->u.clientpak.consistancy = SHORT(consistancy[gametic%TICQUEUE]);

		if (splitscreen || botingame) // Send a special packet with 2 cmd for splitscreen
		{
			netbuffer->packettype = (mis ? PT_CLIENT2MIS : PT_CLIENT2CMD);
			packetsize = sizeof (client2cmd_pak);
			G_MoveTiccmd(&netbuffer->u.client2pak.cmd2, &localcmds2, 1);

			if (splitscreen > 1)
			{
				netbuffer->packettype = (mis ? PT_CLIENT3MIS : PT_CLIENT3CMD);
				packetsize = sizeof (client3cmd_pak);
				G_MoveTiccmd(&netbuffer->u.client3pak.cmd3, &localcmds3, 1);

				if (splitscreen > 2)
				{
					netbuffer->packettype = (mis ? PT_CLIENT4MIS : PT_CLIENT4CMD);
					packetsize = sizeof (client4cmd_pak);
					G_MoveTiccmd(&netbuffer->u.client4pak.cmd4, &localcmds4, 1);
				}
			}
		}

		HSendPacket(servernode, false, 0, packetsize);
	}

	if (cl_mode == CL_CONNECTED || dedicated)
	{
		// Send extra data if needed
		if (localtextcmd[0])
		{
			netbuffer->packettype = PT_TEXTCMD;
			M_Memcpy(netbuffer->u.textcmd,localtextcmd, localtextcmd[0]+1);
			// All extra data have been sent
			if (HSendPacket(servernode, true, 0, localtextcmd[0]+1)) // Send can fail...
				localtextcmd[0] = 0;
		}

		// Send extra data if needed for player 2 (splitscreen == 1)
		if (localtextcmd2[0])
		{
			netbuffer->packettype = PT_TEXTCMD2;
			M_Memcpy(netbuffer->u.textcmd, localtextcmd2, localtextcmd2[0]+1);
			// All extra data have been sent
			if (HSendPacket(servernode, true, 0, localtextcmd2[0]+1)) // Send can fail...
				localtextcmd2[0] = 0;
		}

		// Send extra data if needed for player 3 (splitscreen == 2)
		if (localtextcmd3[0])
		{
			netbuffer->packettype = PT_TEXTCMD3;
			M_Memcpy(netbuffer->u.textcmd, localtextcmd3, localtextcmd3[0]+1);
			// All extra data have been sent
			if (HSendPacket(servernode, true, 0, localtextcmd3[0]+1)) // Send can fail...
				localtextcmd3[0] = 0;
		}

		// Send extra data if needed for player 4 (splitscreen == 3)
		if (localtextcmd4[0])
		{
			netbuffer->packettype = PT_TEXTCMD4;
			M_Memcpy(netbuffer->u.textcmd, localtextcmd4, localtextcmd4[0]+1);
			// All extra data have been sent
			if (HSendPacket(servernode, true, 0, localtextcmd4[0]+1)) // Send can fail...
				localtextcmd4[0] = 0;
		}
	}
}

// send the server packet
// send tic from firstticstosend to maketic-1
static void SV_SendTics(void)
{
	tic_t realfirsttic, lasttictosend, i;
	UINT32 n;
	INT32 j;
	size_t packsize;
	UINT8 *bufpos;
	UINT8 *ntextcmd;

	// send to all client but not to me
	// for each node create a packet with x tics and send it
	// x is computed using supposedtics[n], max packet size and maketic
	for (n = 1; n < MAXNETNODES; n++)
		if (nodeingame[n])
		{
			// assert supposedtics[n]>=nettics[n]
			realfirsttic = supposedtics[n];
			lasttictosend = maketic;

			if (lasttictosend - nettics[n] >= BACKUPTICS)
				lasttictosend = nettics[n] + BACKUPTICS-1;

			if (realfirsttic >= lasttictosend)
			{
				// well we have sent all tics we will so use extrabandwidth
				// to resent packet that are supposed lost (this is necessary since lost
				// packet detection work when we have received packet with firsttic > neededtic
				// (getpacket servertics case)
				DEBFILE(va("Nothing to send node %u mak=%u sup=%u net=%u \n",
					n, lasttictosend, supposedtics[n], nettics[n]));
				realfirsttic = nettics[n];
				if (realfirsttic >= lasttictosend || (I_GetTime() + n)&3)
					// all tic are ok
					continue;
				DEBFILE(va("Sent %d anyway\n", realfirsttic));
			}
			if (realfirsttic < firstticstosend)
				realfirsttic = firstticstosend;

			// compute the length of the packet and cut it if too large
			packsize = BASESERVERTICSSIZE;
			for (i = realfirsttic; i < lasttictosend; i++)
			{
				packsize += sizeof (ticcmd_t) * doomcom->numslots;
				packsize += TotalTextCmdPerTic(i);

				if (packsize > software_MAXPACKETLENGTH)
				{
					DEBFILE(va("packet too large (%s) at tic %d (should be from %d to %d)\n",
						sizeu1(packsize), i, realfirsttic, lasttictosend));
					lasttictosend = i;

					// too bad: too much player have send extradata and there is too
					//          much data in one tic.
					// To avoid it put the data on the next tic. (see getpacket
					// textcmd case) but when numplayer changes the computation can be different
					if (lasttictosend == realfirsttic)
					{
						if (packsize > MAXPACKETLENGTH)
							I_Error("Too many players: can't send %s data for %d players to node %d\n"
							        "Well sorry nobody is perfect....\n",
							        sizeu1(packsize), doomcom->numslots, n);
						else
						{
							lasttictosend++; // send it anyway!
							DEBFILE("sending it anyway\n");
						}
					}
					break;
				}
			}

			// Send the tics
			netbuffer->packettype = PT_SERVERTICS;
			netbuffer->u.serverpak.starttic = (UINT8)realfirsttic;
			netbuffer->u.serverpak.numtics = (UINT8)(lasttictosend - realfirsttic);
			netbuffer->u.serverpak.numslots = (UINT8)SHORT(doomcom->numslots);
			bufpos = (UINT8 *)&netbuffer->u.serverpak.cmds;

			for (i = realfirsttic; i < lasttictosend; i++)
			{
				bufpos = G_DcpyTiccmd(bufpos, netcmds[i%TICQUEUE], doomcom->numslots * sizeof (ticcmd_t));
			}

			// add textcmds
			for (i = realfirsttic; i < lasttictosend; i++)
			{
				ntextcmd = bufpos++;
				*ntextcmd = 0;
				for (j = 0; j < MAXPLAYERS; j++)
				{
					UINT8 *textcmd = D_GetExistingTextcmd(i, j);
					INT32 size = textcmd ? textcmd[0] : 0;

					if ((!j || playeringame[j]) && size)
					{
						(*ntextcmd)++;
						WRITEUINT8(bufpos, j);
						M_Memcpy(bufpos, textcmd, size + 1);
						bufpos += size + 1;
					}
				}
			}
			packsize = bufpos - (UINT8 *)&(netbuffer->u);

			HSendPacket(n, false, 0, packsize);
			// when tic are too large, only one tic is sent so don't go backward!
			if (lasttictosend-doomcom->extratics > realfirsttic)
				supposedtics[n] = lasttictosend-doomcom->extratics;
			else
				supposedtics[n] = lasttictosend;
			if (supposedtics[n] < nettics[n]) supposedtics[n] = nettics[n];
		}
	// node 0 is me!
	supposedtics[0] = maketic;
}

//
// TryRunTics
//
static void Local_Maketic(INT32 realtics)
{
	I_OsPolling(); // I_Getevent
	D_ProcessEvents(); // menu responder, cons responder,
	                   // game responder calls HU_Responder, AM_Responder, F_Responder,
	                   // and G_MapEventsToControls
	if (!dedicated) rendergametic = gametic;
	// translate inputs (keyboard/mouse/joystick) into game controls
	G_BuildTiccmd(&localcmds, realtics, 1);
	if (splitscreen || botingame)
	{
		G_BuildTiccmd(&localcmds2, realtics, 2);
		if (splitscreen > 1)
		{
			G_BuildTiccmd(&localcmds3, realtics, 3);
			if (splitscreen > 2)
				G_BuildTiccmd(&localcmds4, realtics, 4);
		}
	}

	localcmds.angleturn |= TICCMD_RECEIVED;
}

void SV_SpawnPlayer(INT32 playernum, INT32 x, INT32 y, angle_t angle)
{
	tic_t tic;
	UINT16 numadjust = 0;

	(void)x;
	(void)y;

	// Revisionist history: adjust the angles in the ticcmds received
	// for this player, because they actually preceded the player
	// spawning, but will be applied afterwards.

	for (tic = server ? maketic : (neededtic - 1); tic >= gametic; tic--)
	{
		if (numadjust++ == TICQUEUE)
		{
			DEBFILE(va("SV_SpawnPlayer: All netcmds for player %d adjusted!\n", playernum));
			// We already adjusted them all, waste of time doing the same thing over and over
			// This shouldn't happen normally though, either gametic was 0 (which is handled now anyway)
			// or maketic >= gametic + TICQUEUE
			// -- Monster Iestyn 16/01/18
			break;
		}
		netcmds[tic%TICQUEUE][playernum].angleturn = (INT16)((angle>>16) | TICCMD_RECEIVED);

		if (!tic) // failsafe for gametic == 0 -- Monster Iestyn 16/01/18
			break;
	}
}

// create missed tic
static void SV_Maketic(void)
{
	INT32 j;

	for (j = 0; j < MAXNETNODES; j++)
		if (playerpernode[j])
		{
			INT32 player = nodetoplayer[j];
			if ((netcmds[maketic%TICQUEUE][player].angleturn & TICCMD_RECEIVED) == 0)
			{ // we didn't receive this tic
				INT32 i;

				DEBFILE(va("MISS tic%4d for node %d\n", maketic, j));
#if defined(PARANOIA) && 0
				CONS_Debug(DBG_NETPLAY, "Client Misstic %d\n", maketic);
#endif
				// copy the old tic
				for (i = 0; i < playerpernode[j]; i++)
				{
					if (i == 0) player = nodetoplayer[j];
					else if (i == 1) player = nodetoplayer2[j];
					else if (i == 2) player = nodetoplayer3[j];
					else if (i == 3) player = nodetoplayer4[j];
					netcmds[maketic%TICQUEUE][player] = netcmds[(maketic-1)%TICQUEUE][player];
					netcmds[maketic%TICQUEUE][player].angleturn &= ~TICCMD_RECEIVED;
				}
			}
		}

	// all tic are now proceed make the next
	maketic++;
}

boolean TryRunTics(tic_t realtics)
{
	boolean ticking;

	// the machine has lagged but it is not so bad
	if (realtics > TICRATE/7) // FIXME: consistency failure!!
	{
		if (server)
			realtics = 1;
		else
			realtics = TICRATE/7;
	}

	if (singletics)
		realtics = 1;

	if (realtics >= 1)
	{
		COM_BufTicker();
		if (mapchangepending)
			D_MapChange(-1, 0, encoremode, false, 2, false, fromlevelselect); // finish the map change
	}

	NetUpdate();

	if (demo.playback)
	{
		neededtic = gametic + realtics * (gamestate == GS_LEVEL ? cv_playbackspeed.value : 1);
		// start a game after a demo
		maketic += realtics;
		firstticstosend = maketic;
		tictoclear = firstticstosend;
	}

	GetPackets();

#ifdef DEBUGFILE
	if (debugfile && (realtics || neededtic > gametic))
	{
		//SoM: 3/30/2000: Need long INT32 in the format string for args 4 & 5.
		//Shut up stupid warning!
		fprintf(debugfile, "------------ Tryruntic: REAL:%d NEED:%d GAME:%d LOAD: %d\n",
			realtics, neededtic, gametic, debugload);
		debugload = 100000;
	}
#endif

	if (neededtic > gametic)
	{
		hu_stopped = false;
	}

	if (player_joining)
	{
		hu_stopped = true;
		return false;
	}

	ticking = neededtic > gametic;

	if (ticking)
	{
		// run the count * tics
		while (neededtic > gametic)
		{
			DEBFILE(va("============ Running tic %d (local %d)\n", gametic, localgametic));

			G_Ticker((gametic % NEWTICRATERATIO) == 0);
			ExtraDataTicker();
			gametic++;
			consistancy[gametic%TICQUEUE] = Consistancy();

			// Leave a certain amount of tics present in the net buffer as long as we've ran at least one tic this frame.
			if (client && gamestate == GS_LEVEL && leveltime > 3 && neededtic <= gametic + cv_netticbuffer.value)
				break;
		}
	}
	else
	{
		hu_stopped = true;
	}

	return ticking;
}


/* 	Ping Update except better:
	We call this once per second and check for people's pings. If their ping happens to be too high, we increment some timer and kick them out.
	If they're not lagging, decrement the timer by 1. Of course, reset all of this if they leave.

	Why do we do that? Well, I'm a person with unfortunately sometimes unstable internet and happen to keep getting kicked very unconveniently for very short high spikes. (700+ ms)
	Because my spikes are so high, the average ping is exponentially higher too (700s really add up...!) which leads me to getting kicked for a short burst of spiking.
	With this change here, this doesn't happen anymore as it checks if my ping has been CONSISTENTLY bad for long enough before killing me.
*/

static INT32 pingtimeout[MAXPLAYERS];

#define PINGKICK_TICQUEUE 2
#define PINGKICK_LIMIT 1

static inline void PingUpdate(void)
{
	INT32 i;
	UINT8 pingkick[MAXPLAYERS];
	UINT8 nonlaggers = 0;
	memset(pingkick, 0, sizeof(pingkick));

	netbuffer->packettype = PT_PING;

	//check for ping limit breakage.
	//if (cv_maxping.value) -- always check for TICQUEUE overrun
	{
		for (i = 0; i < MAXPLAYERS; i++)
		{
			if (!playeringame[i] || P_IsLocalPlayer(&players[i])) // should be P_IsMachineLocalPlayer for DRRR
			{
				pingtimeout[i] = 0;
				continue;
			}

			if ((maketic + 5) >= nettics[playernode[i]] + (TICQUEUE-(2*TICRATE)))
			{
				// Anyone who's gobbled most of the TICQUEUE and is likely to halt the server the next few times this runs has to die *right now*. (See also NetUpdate)
				pingkick[i] = PINGKICK_TICQUEUE;
			}
			else if ((cv_maxping.value)
				&& (realpingtable[i] / pingmeasurecount > (unsigned)cv_maxping.value))
			{
				if (players[i].jointime > 10 * TICRATE)
				{
					pingkick[i] = PINGKICK_LIMIT;
				}
			}
			else
			{
				nonlaggers++;

				// you aren't lagging, but you aren't free yet. In case you'll keep spiking, we just make the timer go back down. (Very unstable net must still get kicked).
				if (pingtimeout[i] > 0)
					pingtimeout[i]--;
			}

		}

		//kick lagging players... unless everyone but the server's ping sucks.
		//in that case, it is probably the server's fault.
		// Always kick TICQUEUE-overrunners, too.
		{
			UINT8 minimumkicklevel = (nonlaggers > 0) ? PINGKICK_LIMIT : PINGKICK_TICQUEUE;
			for (i = 0; i < MAXPLAYERS; i++)
			{
				XBOXSTATIC char buf[2];

				if (!playeringame[i] || pingkick[i] < minimumkicklevel)
					continue;

				if (pingkick[i] == PINGKICK_LIMIT)
				{
					// Don't kick on ping alone if we haven't reached our threshold yet.
					if (++pingtimeout[i] < cv_pingtimeout.value)
						continue;
				}

				pingtimeout[i] = 0;

				buf[0] = (char)i;
				buf[1] = KICK_MSG_PING_HIGH;
				SendNetXCmd(XD_KICK, &buf, 2);
			}
		}
	}

	//make the ping packet and clear server data for next one
	for (i = 0; i < MAXPLAYERS; i++)
	{
		//CONS_Printf("player %d - total pings: %d\n", i, realpingtable[i]);

		netbuffer->u.pingtable[i] = realpingtable[i] / pingmeasurecount;
		//server takes a snapshot of the real ping for display.
		//otherwise, pings fluctuate a lot and would be odd to look at.
		playerpingtable[i] = realpingtable[i] / pingmeasurecount;
		realpingtable[i] = 0; //Reset each as we go.
	}

	// send the server's maxping as last element of our ping table. This is useful to let us know when we're about to get kicked.
	netbuffer->u.pingtable[MAXPLAYERS] = cv_maxping.value;

	//send out our ping packets
	for (i = 0; i < MAXNETNODES; i++)
		if (nodeingame[i])
			HSendPacket(i, true, 0, sizeof(INT32) * (MAXPLAYERS+1));

	pingmeasurecount = 0; //Reset count
}

#undef PINGKICK_DANGER
#undef PINGKICK_LIMIT

static tic_t gametime = 0;

static void UpdatePingTable(void)
{
	INT32 i;
	if (server)
	{
		if (netgame && !(gametime % 35))	// update once per second.
			PingUpdate();
		// update node latency values so we can take an average later.
		for (i = 0; i < MAXPLAYERS; i++)
			if (playeringame[i])
				realpingtable[i] += GetLag(playernode[i]);
		pingmeasurecount++;
	}
}

static void RenewHolePunch(void)
{
	if (cv_rendezvousserver.string[0])
	{
		static time_t past;

		const time_t now = time(NULL);

		if ((now - past) > 20)
		{
			I_NetRegisterHolePunch();
			past = now;
		}
	}
}

// Handle timeouts to prevent definitive freezes from happenning
static void HandleNodeTimeouts(void)
{
	INT32 i;
	if (server)
		for (i = 1; i < MAXNETNODES; i++)
			if (nodeingame[i] && freezetimeout[i] < I_GetTime())
				Net_ConnectionTimeout(i);
}

// Keep the network alive while not advancing tics!
void NetKeepAlive(void)
{
	tic_t nowtime;
	INT32 realtics;

	nowtime = I_GetTime();
	realtics = nowtime - gametime;

	// return if there's no time passed since the last call
	if (realtics <= 0) // nothing new to update
		return;

	UpdatePingTable();

// Sryder: What is FILESTAMP???
FILESTAMP
	GetPackets();
FILESTAMP

#ifdef MASTERSERVER
	MasterClient_Ticker();
#endif

	if (netgame && serverrunning)
	{
		RenewHolePunch();
	}

	if (client)
	{
		// send keep alive
		CL_SendClientKeepAlive();
		// No need to check for resynch because we aren't running any tics
	}
	else
	{
		SV_SendServerKeepAlive();
	}

	// No else because no tics are being run and we can't resynch during this

	Net_AckTicker();
	HandleNodeTimeouts();
	SV_FileSendTicker();
}

// If a tree falls in the forest but nobody is around to hear it, does it make a tic?
#define DEDICATEDIDLETIME (10*TICRATE)

void NetUpdate(void)
{
	static tic_t resptime = 0;
	tic_t nowtime;
	INT32 i;
	INT32 realtics;

	nowtime = I_GetTime();
	realtics = nowtime - gametime;

	if (realtics <= 0) // nothing new to update
		return;

#ifdef DEDICATEDIDLETIME
	if (server && dedicated && gamestate == GS_LEVEL)
	{
		static tic_t dedicatedidle = 0;

		for (i = 1; i < MAXNETNODES; ++i)
			if (nodeingame[i])
			{
				if (dedicatedidle == DEDICATEDIDLETIME)
				{
					CONS_Printf("DEDICATED: Awakening from idle (Node %d detected...)\n", i);
					dedicatedidle = 0;
				}
				break;
			}

		if (i == MAXNETNODES)
		{
			if (leveltime == 2)
			{
				// On next tick...
				dedicatedidle = DEDICATEDIDLETIME-1;
			}
			else if (dedicatedidle == DEDICATEDIDLETIME)
			{
				if (D_GetExistingTextcmd(gametic, 0) || D_GetExistingTextcmd(gametic+1, 0))
				{
					CONS_Printf("DEDICATED: Awakening from idle (Netxcmd detected...)\n");
					dedicatedidle = 0;
				}
				else
				{
					realtics = 0;
				}
			}
			else if (++dedicatedidle == DEDICATEDIDLETIME)
			{
				const char *idlereason = "at round start";
				if (leveltime > 3)
					idlereason = va("for %d seconds", dedicatedidle/TICRATE);

				CONS_Printf("DEDICATED: No nodes %s, idling...\n", idlereason);
				realtics = 0;
			}
		}
	}
#endif

	if (realtics > 5)
	{
		if (server)
			realtics = 1;
		else
			realtics = 5;
	}

	gametime = nowtime;

	UpdatePingTable();

	if (client)
		maketic = neededtic;

	Local_Maketic(realtics); // make local tic, and call menu?

	if (server)
		CL_SendClientCmd(); // send it
FILESTAMP
	GetPackets(); // get packet from client or from server
FILESTAMP
	// client send the command after a receive of the server
	// the server send before because in single player is beter

#ifdef MASTERSERVER
	MasterClient_Ticker(); // Acking the Master Server
#endif

	if (netgame && serverrunning)
	{
		RenewHolePunch();
	}

	if (client)
	{
		if (!resynch_local_inprogress)
			CL_SendClientCmd(); // Send tic cmd
		hu_resynching = resynch_local_inprogress;
	}
	else
	{
		if (!demo.playback && realtics > 0)
		{
			INT32 counts;

			hu_resynching = false;

			firstticstosend = gametic;
			for (i = 0; i < MAXNETNODES; i++)
				if (nodeingame[i] && nettics[i] < firstticstosend)
					firstticstosend = nettics[i];

			// Don't erase tics not acknowledged
			counts = realtics;

			for (i = 0; i < MAXNETNODES; ++i)
				if (resynch_inprogress[i])
				{
					SV_SendResynch(i);
					counts = -666;
				}

			// Do not make tics while resynching
			if (counts != -666)
			{
				// See also PingUpdate
				if (maketic + counts >= firstticstosend + TICQUEUE)
					counts = firstticstosend+TICQUEUE-maketic-1;

				for (i = 0; i < counts; i++)
					SV_Maketic(); // Create missed tics and increment maketic

				for (; tictoclear < firstticstosend; tictoclear++) // Clear only when acknowledged
					D_Clearticcmd(tictoclear);                    // Clear the maketic the new tic

				SV_SendTics();

				neededtic = maketic; // The server is a client too
			}
			else
				hu_resynching = true;
		}
	}
	Net_AckTicker();
	HandleNodeTimeouts();
	if (nowtime > resptime)
	{
		resptime = nowtime;
#ifdef HAVE_THREADS
		I_lock_mutex(&m_menu_mutex);
#endif
		M_Ticker();
#ifdef HAVE_THREADS
		I_unlock_mutex(m_menu_mutex);
#endif
		CON_Ticker();
	}
	SV_FileSendTicker();
}

/** Returns the number of players playing.
  * \return Number of players. Can be zero if we're running a ::dedicated
  *         server.
  * \author Graue <graue@oceanbase.org>
  */
INT32 D_NumPlayers(void)
{
	INT32 num = 0, ix;
	for (ix = 0; ix < MAXPLAYERS; ix++)
		if (playeringame[ix])
			num++;
	return num;
}

tic_t GetLag(INT32 node)
{
	// If the client has caught up to the server -- say, during a wipe -- lag is meaningless.
	if (nettics[node] > gametic)
		return 0;
	return gametic - nettics[node];
}

#define REWIND_POINT_INTERVAL 4*TICRATE + 16
rewind_t *rewindhead;

void CL_ClearRewinds(void)
{
	rewind_t *head;
	while ((head = rewindhead))
	{
		rewindhead = rewindhead->next;
		free(head);
	}
}

rewind_t *CL_SaveRewindPoint(size_t demopos)
{
	rewind_t *rewind;

	if (rewindhead && rewindhead->leveltime + REWIND_POINT_INTERVAL > leveltime)
		return NULL;

	rewind = (rewind_t *)malloc(sizeof (rewind_t));
	if (!rewind)
		return NULL;

	save_p = rewind->savebuffer;
	P_SaveNetGame();
	rewind->leveltime = leveltime;
	rewind->next = rewindhead;
	rewind->demopos = demopos;
	rewindhead = rewind;

	return rewind;
}

rewind_t *CL_RewindToTime(tic_t time)
{
	rewind_t *rewind;

	while (rewindhead && rewindhead->leveltime > time)
	{
		rewind = rewindhead->next;
		free(rewindhead);
		rewindhead = rewind;
	}

	if (!rewindhead)
		return NULL;

	save_p = rewindhead->savebuffer;
	P_LoadNetGame();
	wipegamestate = gamestate; // No fading back in!
	timeinmap = leveltime;

	return rewindhead;
}

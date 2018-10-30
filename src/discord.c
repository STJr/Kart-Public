// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 2012-2018 by Sally "TehRealSalt" Cochenour.
// Copyright (C) 2012-2016 by Sonic Team Junior.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  discord.h
/// \brief Discord Rich Presence handling

#ifdef HAVE_DISCORDRPC

#include "i_system.h"
#include "d_clisrv.h"
#include "d_netcmd.h"
#include "i_net.h"
#include "g_game.h"
#include "p_tick.h"
#include "m_menu.h" // gametype_cons_t
#include "r_things.h" // skins
#include "mserv.h" // ms_RoomId

#include "discord.h"
#include "doomdef.h"

#define DISCORD_APPID "503531144395096085" // Feel free to provide your own, if you care.

//
// DRPC_Handle's
//
static inline void DRPC_HandleReady(const DiscordUser *user)
{
	CONS_Printf("Discord: connected to %s#%s - %s\n", user->username, user->discriminator, user->userId);
}

static inline void DRPC_HandleDisconnect(int err, const char *msg)
{
	CONS_Printf("Discord: disconnected (%d: %s)\n", err, msg);
}

static inline void DRPC_HandleError(int err, const char *msg)
{
	CONS_Printf("Discord: error (%d, %s)\n", err, msg);
}

static inline void DRPC_HandleJoin(const char *secret)
{
	CONS_Printf("Discord: connecting to %s\n", secret);
	COM_BufAddText(va("connect \"%s\"\n", secret));
}

//
// DRPC_Init: starting up the handles, call Discord_initalize
//
void DRPC_Init(void)
{
	DiscordEventHandlers handlers;
	memset(&handlers, 0, sizeof(handlers));

	handlers.ready = DRPC_HandleReady;
	handlers.disconnected = DRPC_HandleDisconnect;
	handlers.errored = DRPC_HandleError;
	handlers.joinGame = DRPC_HandleJoin;

	Discord_Initialize(DISCORD_APPID, &handlers, 1, NULL);
	I_AddExitFunc(Discord_Shutdown);
	DRPC_UpdatePresence();
}

//
// DRPC_UpdatePresence: Called whenever anything changes about server info
//
void DRPC_UpdatePresence(void)
{
	char mapimg[8];
	char mapname[48];
	char charimg[21];
	char charname[28];
	DiscordRichPresence discordPresence;
	memset(&discordPresence, 0, sizeof(discordPresence));

	// Server info
	if (netgame)
	{
		const char *address;

		switch (ms_RoomId)
		{
			case -1: discordPresence.state = "Private"; break; // Private server
			case 33: discordPresence.state = "Standard"; break;
			case 28: discordPresence.state = "Casual"; break;
			default: discordPresence.state = "???"; break; // How?
		}

		discordPresence.partyId = server_context; // Thanks, whoever gave us Mumble support, for implementing the EXACT thing Discord wanted for this field!

		// Grab the host's IP for joining.
		if (I_GetNodeAddress && (address = I_GetNodeAddress(servernode)) != NULL)
		{
			discordPresence.joinSecret = address;
			CONS_Printf("%s\n", address);
		}

		discordPresence.partySize = D_NumPlayers(); // Players in server
		discordPresence.partyMax = cv_maxplayers.value; // Max players (turned into a netvar for this, FOR NOW!)
	}
	else if (Playing())
		discordPresence.state = "Offline";
	else if (demoplayback)
		discordPresence.state = "Watching Demo";
	else
		discordPresence.state = "Menu";

	// Gametype info
	if (gamestate == GS_LEVEL || gamestate == GS_INTERMISSION || gamestate == GS_VOTING)
	{
		if (modeattacking)
			discordPresence.details = "Record Attack";
		else
			discordPresence.details = gametype_cons_t[gametype].strvalue;
	}

	if (gamestate == GS_LEVEL || gamestate == GS_INTERMISSION) // Map info
	{
		if ((gamemap >= 1 && gamemap <= 55) // supported race maps
			|| (gamemap >= 136 && gamemap <= 164) // supported battle maps
			//|| (gamemap >= 352 && gamemap <= 367) // supported hell maps (none of them)
			)
		{
			snprintf(mapimg, 8, "%s", G_BuildMapName(gamemap));
			strlwr(mapimg);
			discordPresence.largeImageKey = mapimg; // Map image
		}
		else // Fallback, since no image looks crappy!
			discordPresence.largeImageKey = "miscdice";

		if (mapheaderinfo[gamemap-1]->menuflags & LF2_HIDEINMENU) // hell map, hide the name
			discordPresence.largeImageText = "Map: ???";
		else
		{
			snprintf(mapname, 48, "Map: %s%s%s",
				mapheaderinfo[gamemap-1]->lvlttl,
				(strlen(mapheaderinfo[gamemap-1]->zonttl) > 0) ? va(" %s",mapheaderinfo[gamemap-1]->zonttl) : // SRB2kart
				((mapheaderinfo[gamemap-1]->levelflags & LF_NOZONE) ? "" : " Zone"),
				(strlen(mapheaderinfo[gamemap-1]->actnum) > 0) ? va(" %s",mapheaderinfo[gamemap-1]->actnum) : "");
			discordPresence.largeImageText = mapname; // Map name
		}

		// discordPresence.startTimestamp & endTimestamp could be used to show leveltime & timelimit respectively,
		// but would need converted to epoch seconds somehow
	}
	else if (gamestate == GS_VOTING)
	{
		discordPresence.largeImageKey = (G_BattleGametype() ? "miscredplanet" : "miscblueplanet");
		discordPresence.largeImageText = "Voting";
	}
	else
	{
		discordPresence.largeImageKey = "misctitle";
		discordPresence.largeImageText = "Title Screen";
	}

	// Character info
	if (Playing() && playeringame[consoleplayer] && !players[consoleplayer].spectator)
	{
		if (players[consoleplayer].skin < 5) // supported skins
		{
			snprintf(charimg, 21, "char%s", skins[players[consoleplayer].skin].name);
			discordPresence.smallImageKey = charimg; // Character image
		}
		else
			discordPresence.smallImageKey = "charnull"; // Just so that you can still see the name of custom chars 

		snprintf(charname, 28, "Character: %s", skins[players[consoleplayer].skin].realname);
		discordPresence.smallImageText = charname; // Character name
	}

	Discord_UpdatePresence(&discordPresence);
}

#endif

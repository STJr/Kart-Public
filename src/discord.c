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
#include "discord_pass.h" // .gitignore'd file for volitile information; DO NOT push this info
#include "doomdef.h"

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

	if (!discordappid)
		return;

	handlers.ready = DRPC_HandleReady;
	handlers.disconnected = DRPC_HandleDisconnect;
	handlers.errored = DRPC_HandleError;
	handlers.joinGame = DRPC_HandleJoin;

	Discord_Initialize(discordappid, &handlers, 1, NULL);
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

	if (discordappid)
	{
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

			discordPresence.partyId = "1"; // We don't really have "party" IDs, so to make invites expire we just let it reset to 0 outside of servers

			// Grab the host's IP for joining.
			if (I_GetNodeAddress && (address = I_GetNodeAddress(servernode)) != NULL)
			{
				discordPresence.joinSecret = address;
				CONS_Printf("%s\n", address);
			}

			discordPresence.partySize = D_NumPlayers(); // Players in server
			discordPresence.partyMax = cv_maxplayers.value; // Max players
		}
		else
			discordPresence.state = "Offline";

		// Gametype info
		if (gamestate == GS_LEVEL || gamestate == GS_INTERMISSION || gamestate == GS_VOTING)
		{
			if (modeattacking)
				discordPresence.details = "Record Attack";
			else
				discordPresence.details = gametype_cons_t[gametype].strvalue;
		}

		// Map info
		if (gamestate == GS_LEVEL || gamestate == GS_INTERMISSION)
		{
			snprintf(mapimg, 8, "%s", G_BuildMapName(gamemap));
			strlwr(mapimg);

			discordPresence.largeImageKey = mapimg; // Map image

			if (mapheaderinfo[gamemap-1]->lvlttl[0] != '\0')
				snprintf(mapname, 48, "Map: %s%s%s",
					mapheaderinfo[gamemap-1]->lvlttl,
					(strlen(mapheaderinfo[gamemap-1]->zonttl) > 0) ? va(" %s",mapheaderinfo[gamemap-1]->zonttl) : // SRB2kart
					((mapheaderinfo[gamemap-1]->levelflags & LF_NOZONE) ? "" : " ZONE"),
					(strlen(mapheaderinfo[gamemap-1]->actnum) > 0) ? va(" %s",mapheaderinfo[gamemap-1]->actnum) : "");
			else
				snprintf(mapname, 48, "???");

			discordPresence.largeImageText = mapname; // Map name

			//if (cv_timelimit.value)
				//discordPresence.endTimestamp = levelstarttime + (cv_timelimit.value*60); // Time limit if applicable
		}
		else if (gamestate == GS_VOTING)
		{
			discordPresence.largeImageText = "Voting";
		}

		// Player info
		if (playeringame[consoleplayer])
		{
			//discordPresence.startTimestamp = levelstarttime; // Time in level
			if (!players[consoleplayer].spectator)
			{
				snprintf(charimg, 21, "char%s", skins[players[consoleplayer].skin].name);
				discordPresence.smallImageKey = charimg; // Character image

				snprintf(charname, 28, "Character: %s", skins[players[consoleplayer].skin].realname);
				discordPresence.smallImageText = charname; // Character name
			}
		}
	}

	Discord_UpdatePresence(&discordPresence);
}

#endif

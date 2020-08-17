// SONIC ROBO BLAST 2 KART
//-----------------------------------------------------------------------------
// Copyright (C) 2018-2020 by Sally "TehRealSalt" Cochenour.
// Copyright (C) 2018-2020 by Kart Krew.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  discord.h
/// \brief Discord Rich Presence handling

#ifdef HAVE_DISCORDRPC

#ifdef HAVE_CURL
#include <curl/curl.h>
#endif

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

// Feel free to provide your own, if you care enough to create another Discord app for this :P
#define DISCORD_APPID "503531144395096085"

consvar_t cv_discordrp = {"discordrp", "On", CV_SAVE|CV_CALL, CV_OnOff, DRPC_UpdatePresence, 0, NULL, NULL, 0, 0, NULL};

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
	CONS_Alert(CONS_WARNING, "Discord: error (%d, %s)\n", err, msg);
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

#ifdef HAVE_CURL
#define IP_SIZE 16
static char self_ip[IP_SIZE];

struct SelfIPbuffer
{
	CURL *curl;
	char *pointer;
	size_t length;
};

static size_t DRPC_WriteServerIP(char *s, size_t size, size_t n, void *userdata )
{
	struct SelfIPbuffer *buffer;
	size_t newlength;

	buffer = userdata;

	newlength = buffer->length + size*n;
	buffer->pointer = realloc(buffer->pointer, newlength+1);

	memcpy(buffer->pointer + buffer->length, s, size*n);

	buffer->pointer[newlength] = '\0';
	buffer->length = newlength;

	return size*n;
}
#endif

//
// DRPC_GetServerIP: Gets the server's IP address, used to 
//
static const char *DRPC_GetServerIP(void)
{
	const char *address; 

	// If you're connected
	if (I_GetNodeAddress && (address = I_GetNodeAddress(servernode)) != NULL)
	{
		if (strcmp(address, "self"))
			return address; // We're not the server, so we could successfully get the IP! No problem here :)
	}

#ifdef HAVE_CURL
	// This is a little bit goofy, but
	// there's practically no good way to get your own public IP address,
	// so we've gotta break out curl for this :V
	if (!self_ip[0])
	{
		CURL *curl;

		curl_global_init(CURL_GLOBAL_ALL);
		curl = curl_easy_init();

		if (curl)
		{
			const char *api = "http://ip4only.me/api/"; // API to get your public IP address from
			struct SelfIPbuffer buffer;
			CURLcode success;

			buffer.length = 0;
			buffer.pointer = malloc(buffer.length+1);
			buffer.pointer[0] = '\0';

			curl_easy_setopt(curl, CURLOPT_URL, api);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, DRPC_WriteServerIP);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

			success = curl_easy_perform(curl);

			if (success == CURLE_OK)
			{
				char *tmp;
				tmp = strtok(buffer.pointer, ",");

				if (!strcmp(tmp, "IPv4")) // ensure correct type of IP
				{
					tmp = strtok(NULL, ",");
					strncpy(self_ip, tmp, IP_SIZE); // Yay, we have the IP :)
				}
			}

			free(buffer.pointer);
			curl_easy_cleanup(curl);
		}
	}

	if (self_ip[0])
		return self_ip;
	else
#endif
		return NULL; // Could not get your IP for whatever reason, so we cannot do Discord invites
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

	if (!cv_discordrp.value)
	{
		// User doesn't want to show their game information, so update with empty presence.
		// This just shows that they're playing SRB2Kart. (If that's too much, then they should disable game activity :V)
		Discord_UpdatePresence(&discordPresence);
		return;
	}

	// Server info
	if (netgame)
	{
		const char *join;

		switch (ms_RoomId)
		{
			case -1: discordPresence.state = "Private"; break; // Private server
			case 33: discordPresence.state = "Standard"; break;
			case 28: discordPresence.state = "Casual"; break;
			case 38: discordPresence.state = "Custom Gametypes"; break;
			case 31: discordPresence.state = "OLDC"; break;
			default: discordPresence.state = "Unknown Room"; break; // HOW
		}

		discordPresence.partyId = server_context; // Thanks, whoever gave us Mumble support, for implementing the EXACT thing Discord wanted for this field!
		discordPresence.partySize = D_NumPlayers(); // Players in server
		discordPresence.partyMax = cv_maxplayers.value; // Max players (TODO: another variable should hold this, so that maxplayers doesn't have to be a netvar)

		// Grab the host's IP for joining.
		if ((join = DRPC_GetServerIP()) != NULL)
			discordPresence.joinSecret = join;
	}
	else
	{
		// Offline info
		if (Playing())
			discordPresence.state = "Offline";
		else if (demo.playback && !demo.title)
			discordPresence.state = "Watching Replay";
		else
			discordPresence.state = "Menu";
	}

	// Gametype info
	if (gamestate == GS_LEVEL || gamestate == GS_INTERMISSION || gamestate == GS_VOTING)
	{
		if (modeattacking)
			discordPresence.details = "Time Attack";
		else
			discordPresence.details = gametype_cons_t[gametype].strvalue;
	}

	if (gamestate == GS_LEVEL || gamestate == GS_INTERMISSION) // Map info
	{
		if ((gamemap >= 1 && gamemap <= 60) // supported race maps
			|| (gamemap >= 136 && gamemap <= 164)) // supported battle maps
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

		if (gamestate == GS_LEVEL)
		{
			const time_t currentTime = time(NULL);
			const time_t mapTimeStart = currentTime - (leveltime / TICRATE);

			discordPresence.startTimestamp = mapTimeStart;

			if (timelimitintics > 0)
			{
				const time_t mapTimeEnd = mapTimeStart + ((timelimitintics + starttime + 1) / TICRATE);
				discordPresence.endTimestamp = mapTimeEnd;
			}
		}
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

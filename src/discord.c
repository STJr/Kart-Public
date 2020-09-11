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
#endif // HAVE_CURL

#include "i_system.h"
#include "d_clisrv.h"
#include "d_netcmd.h"
#include "i_net.h"
#include "g_game.h"
#include "p_tick.h"
#include "m_menu.h" // gametype_cons_t
#include "r_things.h" // skins
#include "mserv.h" // cv_advertise
#include "z_zone.h"
#include "byteptr.h"

#include "discord.h"
#include "doomdef.h"

// Feel free to provide your own, if you care enough to create another Discord app for this :P
#define DISCORD_APPID "503531144395096085"

// length of IP strings
#define IP_SIZE 21

consvar_t cv_discordrp = {"discordrp", "On", CV_SAVE|CV_CALL, CV_OnOff, DRPC_UpdatePresence, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_discordstreamer = {"discordstreamer", "Off", CV_SAVE, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_discordasks = {"discordasks", "Yes", CV_SAVE|CV_CALL, CV_YesNo, DRPC_UpdatePresence, 0, NULL, NULL, 0, 0, NULL};

struct discordInfo_s discordInfo;

discordRequest_t *discordRequestList = NULL;

#ifdef HAVE_CURL
struct SelfIPbuffer
{
	CURL *curl;
	char *pointer;
	size_t length;
};

static char self_ip[IP_SIZE];
#endif // HAVE_CURL

/*--------------------------------------------------
	static char *DRPC_XORIPString(const char *input)

		Simple XOR encryption/decryption. Not complex or
		very secretive because we aren't sending anything
		that isn't easily accessible via our Master Server anyway.
--------------------------------------------------*/
static char *DRPC_XORIPString(const char *input)
{
	const UINT8 xor[IP_SIZE] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21};
	char *output = malloc(sizeof(char) * (IP_SIZE+1));
	UINT8 i;

	for (i = 0; i < IP_SIZE; i++)
	{
		char xorinput;

		if (!input[i])
			break;

		xorinput = input[i] ^ xor[i];

		if (xorinput < 32 || xorinput > 126)
		{
			xorinput = input[i];
		}

		output[i] = xorinput;
	}

	output[i] = '\0';

	return output;
}

/*--------------------------------------------------
	static void DRPC_HandleReady(const DiscordUser *user)

		Callback function, ran when the game connects to Discord.

	Input Arguments:-
		user - Struct containing Discord user info.

	Return:-
		None
--------------------------------------------------*/
static void DRPC_HandleReady(const DiscordUser *user)
{
	if (cv_discordstreamer.value)
	{
		CONS_Printf("Discord: connected to %s\n", user->username);
	}
	else
	{
		CONS_Printf("Discord: connected to %s#%s (%s)\n", user->username, user->discriminator, user->userId);
	}
}

/*--------------------------------------------------
	static void DRPC_HandleDisconnect(int err, const char *msg)

		Callback function, ran when disconnecting from Discord.

	Input Arguments:-
		err - Error type
		msg - Error message

	Return:-
		None
--------------------------------------------------*/
static void DRPC_HandleDisconnect(int err, const char *msg)
{
	CONS_Printf("Discord: disconnected (%d: %s)\n", err, msg);
}

/*--------------------------------------------------
	static void DRPC_HandleError(int err, const char *msg)

		Callback function, ran when Discord outputs an error.

	Input Arguments:-
		err - Error type
		msg - Error message

	Return:-
		None
--------------------------------------------------*/
static void DRPC_HandleError(int err, const char *msg)
{
	CONS_Alert(CONS_WARNING, "Discord error (%d: %s)\n", err, msg);
}

/*--------------------------------------------------
	static void DRPC_HandleJoin(const char *secret)

		Callback function, ran when Discord wants to
		connect a player to the game via a channel invite
		or a join request.

	Input Arguments:-
		secret - Value that links you to the server.

	Return:-
		None
--------------------------------------------------*/
static void DRPC_HandleJoin(const char *secret)
{
	char *ip = DRPC_XORIPString(secret);
	CONS_Printf("Connecting to %s via Discord\n", ip);
	COM_BufAddText(va("connect \"%s\"\n", ip));
	free(ip);
}

/*--------------------------------------------------
	static boolean DRPC_InvitesAreAllowed(void)

		Determines whenever or not invites or
		ask to join requests are allowed.

	Input Arguments:-
		None

	Return:-
		true if invites are allowed, false otherwise.
--------------------------------------------------*/
static boolean DRPC_InvitesAreAllowed(void)
{
	if (!Playing())
	{
		// We're not playing, so we should not be getting invites.
		return false;
	}

	if (cv_discordasks.value == 0)
	{
		// Client has the CVar set to off, so never allow invites from this client.
		return false;
	}

	if (discordInfo.joinsAllowed == true)
	{
		if (discordInfo.everyoneCanInvite == true)
		{
			// Everyone's allowed!
			return true;
		}
		else if (consoleplayer == serverplayer || IsPlayerAdmin(consoleplayer))
		{
			// Only admins are allowed!
			return true;
		}
	}

	// Did not pass any of the checks
	return false;
}

/*--------------------------------------------------
	static void DRPC_HandleJoinRequest(const DiscordUser *requestUser)

		Callback function, ran when Discord wants to
		ask the player if another Discord user can join
		or not.

	Input Arguments:-
		requestUser - DiscordUser struct for the user trying to connect.

	Return:-
		None
--------------------------------------------------*/
static void DRPC_HandleJoinRequest(const DiscordUser *requestUser)
{
	discordRequest_t *append = discordRequestList;
	discordRequest_t *newRequest;

	if (DRPC_InvitesAreAllowed() == false)
	{
		// Something weird happened if this occurred...
		Discord_Respond(requestUser->userId, DISCORD_REPLY_IGNORE);
		return;
	}

	newRequest = Z_Calloc(sizeof(discordRequest_t), PU_STATIC, NULL);

	newRequest->username = Z_Calloc(344, PU_STATIC, NULL);
	snprintf(newRequest->username, 344, "%s", requestUser->username);

	newRequest->discriminator = Z_Calloc(8, PU_STATIC, NULL);
	snprintf(newRequest->discriminator, 8, "%s", requestUser->discriminator);

	newRequest->userID = Z_Calloc(32, PU_STATIC, NULL);
	snprintf(newRequest->userID, 32, "%s", requestUser->userId);

	if (append != NULL)
	{
		discordRequest_t *prev = NULL;

		while (append != NULL)
		{
			// CHECK FOR DUPES!! Ignore any that already exist from the same user.
			if (!strcmp(newRequest->userID, append->userID))
			{
				Discord_Respond(newRequest->userID, DISCORD_REPLY_IGNORE);
				DRPC_RemoveRequest(newRequest);
				return;
			}

			prev = append;
			append = append->next;
		}

		newRequest->prev = prev;
		prev->next = newRequest;
	}
	else
	{
		discordRequestList = newRequest;
		M_RefreshPauseMenu();
	}

	// Made it to the end, request was valid, so play the request sound :)
	S_StartSound(NULL, sfx_requst);
}

/*--------------------------------------------------
	void DRPC_RemoveRequest(discordRequest_t *removeRequest)

		See header file for description.
--------------------------------------------------*/
void DRPC_RemoveRequest(discordRequest_t *removeRequest)
{
	if (removeRequest->prev != NULL)
	{
		removeRequest->prev->next = removeRequest->next;
	}

	if (removeRequest->next != NULL)
	{
		removeRequest->next->prev = removeRequest->prev;

		if (removeRequest == discordRequestList)
		{
			discordRequestList = removeRequest->next;
		}
	}
	else
	{
		if (removeRequest == discordRequestList)
		{
			discordRequestList = NULL;
		}
	}

	Z_Free(removeRequest->username);
	Z_Free(removeRequest->userID);
	Z_Free(removeRequest);
}

/*--------------------------------------------------
	void DRPC_Init(void)

		See header file for description.
--------------------------------------------------*/
void DRPC_Init(void)
{
	DiscordEventHandlers handlers;
	memset(&handlers, 0, sizeof(handlers));

	handlers.ready = DRPC_HandleReady;
	handlers.disconnected = DRPC_HandleDisconnect;
	handlers.errored = DRPC_HandleError;
	handlers.joinGame = DRPC_HandleJoin;
	handlers.joinRequest = DRPC_HandleJoinRequest;

	Discord_Initialize(DISCORD_APPID, &handlers, 1, NULL);
	I_AddExitFunc(Discord_Shutdown);
	DRPC_UpdatePresence();
}

#ifdef HAVE_CURL
/*--------------------------------------------------
	static size_t DRPC_WriteServerIP(char *s, size_t size, size_t n, void *userdata)

		Writing function for use with curl. Only intended to be used with simple text.

	Input Arguments:-
		s - Data to write
		size - Always 1.
		n - Length of data
		userdata - Passed in from CURLOPT_WRITEDATA, intended to be SelfIPbuffer

	Return:-
		Number of bytes wrote in this pass.
--------------------------------------------------*/
static size_t DRPC_WriteServerIP(char *s, size_t size, size_t n, void *userdata)
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
#endif // HAVE_CURL

/*--------------------------------------------------
	static const char *DRPC_GetServerIP(void)

		Retrieves the IP address of the server that you're
		connected to. Will attempt to use curl for getting your
		own IP address, if it's not yours.
--------------------------------------------------*/
static const char *DRPC_GetServerIP(void)
{
	const char *address; 

	// If you're connected
	if (I_GetNodeAddress && (address = I_GetNodeAddress(servernode)) != NULL)
	{
		if (strcmp(address, "self"))
		{
			// We're not the server, so we could successfully get the IP!
			// No need to do anything else :)
			return address; 
		}
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
			// The API to get your public IP address from.
			// Picked because it's stupid simple and it's been up for a long time.
			const char *api = "http://ip4only.me/api/"; 

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

		curl_global_cleanup();
	}

	if (self_ip[0])
		return self_ip;
	else
#endif // HAVE_CURL
		return NULL; // Could not get your IP for whatever reason, so we cannot do Discord invites
}

/*--------------------------------------------------
	void DRPC_EmptyRequests(void)

		Empties the request list. Any existing requests
		will get an ignore reply.
--------------------------------------------------*/
static void DRPC_EmptyRequests(void)
{
	while (discordRequestList != NULL)
	{
		Discord_Respond(discordRequestList->userID, DISCORD_REPLY_IGNORE);
		DRPC_RemoveRequest(discordRequestList);
	}
}

/*--------------------------------------------------
	void DRPC_UpdatePresence(void)

		See header file for description.
--------------------------------------------------*/
void DRPC_UpdatePresence(void)
{
	char detailstr[48+1];

	char mapimg[8+1];
	char mapname[5+21+21+2+1];

	char charimg[4+SKINNAMESIZE+1];
	char charname[11+SKINNAMESIZE+1];

	boolean joinSecretSet = false;

	DiscordRichPresence discordPresence;
	memset(&discordPresence, 0, sizeof(discordPresence));

	if (!cv_discordrp.value)
	{
		// User doesn't want to show their game information, so update with empty presence.
		// This just shows that they're playing SRB2Kart. (If that's too much, then they should disable game activity :V)
		DRPC_EmptyRequests();
		Discord_UpdatePresence(&discordPresence);
		return;
	}

#ifdef DEVELOP
	// This way, we can use the invite feature in-dev, but not have snoopers seeing any potential secrets! :P
	discordPresence.largeImageKey = "miscdevelop";
	discordPresence.largeImageText = "No peeking!";
	discordPresence.state = "Testing the game";

	DRPC_EmptyRequests();
	Discord_UpdatePresence(&discordPresence);
	return;
#endif // DEVELOP

	// Server info
	if (netgame)
	{
		if (cv_advertise.value)
		{
			discordPresence.state = "Public";
		}
		else
		{
			discordPresence.state = "Private";
		}

		discordPresence.partyId = server_context; // Thanks, whoever gave us Mumble support, for implementing the EXACT thing Discord wanted for this field!
		discordPresence.partySize = D_NumPlayers(); // Players in server
		discordPresence.partyMax = discordInfo.maxPlayers; // Max players

		if (DRPC_InvitesAreAllowed() == true)
		{
			const char *join;

			// Grab the host's IP for joining.
			if ((join = DRPC_GetServerIP()) != NULL)
			{
				char *xorjoin = DRPC_XORIPString(join);
				discordPresence.joinSecret = xorjoin;
				free(xorjoin);

				joinSecretSet = true;
			}
		}
	}
	else
	{
		// Reset discord info if you're not in a place that uses it!
		// Important for if you join a server that compiled without HAVE_DISCORDRPC,
		// so that you don't ever end up using bad information from another server.
		memset(&discordInfo, 0, sizeof(discordInfo));

		// Offline info
		if (Playing())
			discordPresence.state = "Offline";
		else if (demo.playback && !demo.title)
			discordPresence.state = "Watching Replay";
		else
			discordPresence.state = "Menu";
	}

	// Gametype info
	if ((gamestate == GS_LEVEL || gamestate == GS_INTERMISSION || gamestate == GS_VOTING) && Playing())
	{
		if (modeattacking)
			discordPresence.details = "Time Attack";
		else
		{
			snprintf(detailstr, 48, "%s%s%s",
				gametype_cons_t[gametype].strvalue,
				(gametype == GT_RACE) ? va(" | %s", kartspeed_cons_t[gamespeed].strvalue) : "",
				(encoremode == true) ? " | Encore" : ""
			);
			discordPresence.details = detailstr;
		}
	}

	if ((gamestate == GS_LEVEL || gamestate == GS_INTERMISSION) // Map info
		&& !(demo.playback && demo.title))
	{
		if ((gamemap >= 1 && gamemap <= 60) // supported race maps
			|| (gamemap >= 136 && gamemap <= 164)) // supported battle maps
		{
			snprintf(mapimg, 8, "%s", G_BuildMapName(gamemap));
			strlwr(mapimg);
			discordPresence.largeImageKey = mapimg; // Map image
		}
		else if (mapheaderinfo[gamemap-1]->menuflags & LF2_HIDEINMENU)
		{
			// Hell map, use the method that got you here :P
			discordPresence.largeImageKey = "miscdice";
		}
		else
		{
			// This is probably a custom map!
			discordPresence.largeImageKey = "mapcustom";
		}

		if (mapheaderinfo[gamemap-1]->menuflags & LF2_HIDEINMENU)
		{
			// Hell map, hide the name
			discordPresence.largeImageText = "Map: ???";
		}
		else
		{
			// Map name on tool tip
			snprintf(mapname, 48, "Map: %s", G_BuildMapTitle(gamemap));
			discordPresence.largeImageText = mapname;
		}

		if (gamestate == GS_LEVEL && Playing())
		{
			const time_t currentTime = time(NULL);
			const time_t mapTimeStart = currentTime - ((leveltime + (modeattacking ? starttime : 0)) / TICRATE);

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
		// Supported skin names
		static const char *supportedSkins[] = {
			// base game
			"sonic",
			"tails",
			"knuckles",
			"eggman",
			"metalsonic",
			// bonus chars
			"flicky",
			"motobug",
			"amy",
			"mighty",
			"ray",
			"espio",
			"vector",
			"chao",
			"gamma",
			"chaos",
			"shadow",
			"rouge",
			"herochao",
			"darkchao",
			"cream",
			"omega",
			"blaze",
			"silver",
			"wonderboy",
			"arle",
			"nights",
			"sakura",
			"ulala",
			"beat",
			"vyse",
			"aiai",
			"kiryu",
			"aigis",
			"miku",
			"doom",
			NULL
		};

		boolean customChar = true;
		UINT8 checkSkin = 0;

		// Character image
		while (supportedSkins[checkSkin] != NULL)
		{
			if (!strcmp(skins[players[consoleplayer].skin].name, supportedSkins[checkSkin]))
			{
				snprintf(charimg, 21, "char%s", supportedSkins[checkSkin]);
				discordPresence.smallImageKey = charimg;
				customChar = false;
				break;
			}

			checkSkin++;
		}

		if (customChar == true)
		{
			// Use the custom character icon!
			discordPresence.smallImageKey = "charcustom";
		}

		snprintf(charname, 28, "Character: %s", skins[players[consoleplayer].skin].realname);
		discordPresence.smallImageText = charname; // Character name
	}

	if (joinSecretSet == false)
	{
		// Not able to join? Flush the request list, if it exists.
		DRPC_EmptyRequests();
	}

	Discord_UpdatePresence(&discordPresence);
}

#endif // HAVE_DISCORDRPC

// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 1999-2018 by Sonic Team Junior.
// Copyright (C)      2020 by James R.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  mserv.c
/// \brief Commands used to communicate with the master server

#if !defined (UNDER_CE)
#include <time.h>
#endif

#include "doomstat.h"
#include "doomdef.h"
#include "command.h"
#include "i_threads.h"
#include "mserv.h"
#include "m_menu.h"
#include "z_zone.h"

#ifdef HAVE_DISCORDRPC
#include "discord.h"
#endif

#ifdef MASTERSERVER

static int     MSId;
static int     MSRegisteredId = -1;

static boolean MSRegistered;
static boolean MSInProgress;
static boolean MSUpdateAgain;

static time_t  MSLastPing;

static char *MSRules;

#ifdef HAVE_THREADS
static I_mutex MSMutex;
static I_cond  MSCond;

#  define Lock_state()   I_lock_mutex  (&MSMutex)
#  define Unlock_state() I_unlock_mutex (MSMutex)
#else/*HAVE_THREADS*/
#  define Lock_state()
#  define Unlock_state()
#endif/*HAVE_THREADS*/

#ifndef NONET
static void Command_Listserv_f(void);
#endif

#endif/*MASTERSERVER*/

static void Update_parameters (void);

static void MasterServer_OnChange(void);

static void Advertise_OnChange(void);

static CV_PossibleValue_t masterserver_update_rate_cons_t[] = {
	{2,  "MIN"},
	{60, "MAX"},
	{0, NULL}
};

consvar_t cv_masterserver = {"masterserver", "https://ms.kartkrew.org/ms/api", CV_SAVE|CV_CALL, NULL, MasterServer_OnChange, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_rendezvousserver = {"holepunchserver", "relay.kartkrew.org", CV_SAVE, NULL, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_servername = {"servername", "SRB2Kart server", CV_SAVE|CV_CALL|CV_NOINIT, NULL, Update_parameters, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_server_contact = {"server_contact", "", CV_SAVE|CV_CALL|CV_NOINIT, NULL, Update_parameters, 0, NULL, NULL, 0, 0, NULL};

consvar_t cv_masterserver_update_rate = {"masterserver_update_rate", "15", CV_SAVE|CV_CALL|CV_NOINIT, masterserver_update_rate_cons_t, MasterClient_Ticker, 0, NULL, NULL, 0, 0, NULL};

consvar_t cv_advertise = {"advertise", "No", CV_NETVAR|CV_CALL|CV_NOINIT, CV_YesNo, Advertise_OnChange, 0, NULL, NULL, 0, 0, NULL};

consvar_t cv_masterserver_nagattempts = {"masterserver_nagattempts", "5", CV_SAVE, CV_Unsigned, NULL, 0, NULL, NULL, 0, 0, NULL};

#if defined (MASTERSERVER) && defined (HAVE_THREADS)
int           ms_QueryId;
I_mutex       ms_QueryId_mutex;

msg_server_t *ms_ServerList;
I_mutex       ms_ServerList_mutex;
#endif

UINT16 current_port = 0;

/** Adds variables and commands relating to the master server.
  *
  * \sa cv_masterserver, cv_servername,
  *     Command_Listserv_f
  */
void AddMServCommands(void)
{
#ifndef NONET
	CV_RegisterVar(&cv_masterserver);
	CV_RegisterVar(&cv_masterserver_update_rate);
	CV_RegisterVar(&cv_masterserver_timeout);
	CV_RegisterVar(&cv_masterserver_debug);
	CV_RegisterVar(&cv_masterserver_token);
	CV_RegisterVar(&cv_masterserver_nagattempts);
	CV_RegisterVar(&cv_advertise);
	CV_RegisterVar(&cv_rendezvousserver);
	CV_RegisterVar(&cv_servername);
	CV_RegisterVar(&cv_server_contact);
#ifdef MASTERSERVER
	COM_AddCommand("listserv", Command_Listserv_f);
#endif
#endif
}

#ifdef MASTERSERVER

static void WarnGUI (void)
{
#ifdef HAVE_THREADS
	I_lock_mutex(&m_menu_mutex);
#endif
	M_StartMessage(M_GetText("There was a problem connecting to\nthe Master Server\n\nCheck the console for details.\n"), NULL, MM_NOTHING);
#ifdef HAVE_THREADS
	I_unlock_mutex(m_menu_mutex);
#endif
}

#define NUM_LIST_SERVER MAXSERVERLIST
msg_server_t *GetShortServersList(int id)
{
	msg_server_t *server_list;

	// +1 for easy test
	server_list = malloc(( NUM_LIST_SERVER + 1 ) * sizeof *server_list);

	if (HMS_fetch_servers(server_list, id))
		return server_list;
	else
	{
		free(server_list);
		WarnGUI();
		return NULL;
	}
}

#ifdef UPDATE_ALERT
char *GetMODVersion(int id)
{
	char *buffer;
	int c;

	(void)id;

	buffer = malloc(16);

	c = HMS_compare_mod_version(buffer, 16);

#ifdef HAVE_THREADS
	I_lock_mutex(&ms_QueryId_mutex);
	{
		if (id != ms_QueryId)
			c = -1;
	}
	I_unlock_mutex(ms_QueryId_mutex);
#endif

	if (c > 0)
		return buffer;
	else
	{
		free(buffer);

		if (! c)
			WarnGUI();

		return NULL;
	}
}
#endif

#ifndef NONET
/** Gets a list of game servers. Called from console.
  */
static void Command_Listserv_f(void)
{
	CONS_Printf(M_GetText("Retrieving server list...\n"));

	{
		HMS_list_servers();
	}
}
#endif

static void
Finish_registration (void)
{
	int registered;
	char *rules = GetMasterServerRules();

	CONS_Printf("Registering this server on the master server...\n");

	if (rules)
	{
		CONS_Printf("\n");
		CONS_Alert(CONS_NOTICE, "%s\n", rules);
	}

	registered = HMS_register();

	Lock_state();
	{
		MSRegistered = registered;
		MSRegisteredId = MSId;

		time(&MSLastPing);
	}
	Unlock_state();

	if (registered)
		CONS_Printf("Master server registration successful.\n");
}

static void
Finish_update (void)
{
	int registered;
	int done;

	Lock_state();
	{
		registered = MSRegistered;
		MSUpdateAgain = false;/* this will happen anyway */
	}
	Unlock_state();

	if (registered)
	{
		if (HMS_update())
		{
			Lock_state();
			{
				time(&MSLastPing);
				MSRegistered = true;
			}
			Unlock_state();

			CONS_Printf("Updated master server listing.\n");
		}
		else
			Finish_registration();
	}
	else
		Finish_registration();

	Lock_state();
	{
		done = ! MSUpdateAgain;

		if (done)
			MSInProgress = false;
	}
	Unlock_state();

	if (! done)
		Finish_update();
}

static void
Finish_unlist (void)
{
	int registered;

	Lock_state();
	{
		registered = MSRegistered;

		if (MSId == MSRegisteredId)
			MSId++;
	}
	Unlock_state();

	if (registered)
	{
		CONS_Printf("Removing this server from the master server...\n");

		if (HMS_unlist())
			CONS_Printf("Server deregistration request successfully sent.\n");

		Lock_state();
		{
			MSRegistered = false;
		}
		Unlock_state();

#ifdef HAVE_THREADS
		I_wake_all_cond(&MSCond);
#endif
	}
}

static void
Finish_masterserver_change (char *api)
{
	char rules[256];

	HMS_set_api(api);

	if (HMS_fetch_rules(rules, sizeof rules))
	{
		Lock_state();
		Z_Free(MSRules);
		MSRules = Z_StrDup(rules);
		Unlock_state();
	}
}

#ifdef HAVE_THREADS
static int *
Server_id (void)
{
	int *id;
	id = malloc(sizeof *id);
	Lock_state();
	{
		*id = MSId;
	}
	Unlock_state();
	return id;
}

static int *
New_server_id (void)
{
	int *id;
	id = malloc(sizeof *id);
	Lock_state();
	{
		*id = ++MSId;
		I_wake_all_cond(&MSCond);
	}
	Unlock_state();
	return id;
}

static void
Register_server_thread (int *id)
{
	int same;

	Lock_state();
	{
		/* wait for previous unlist to finish */
		while (*id == MSId && MSRegistered)
			I_hold_cond(&MSCond, MSMutex);

		same = ( *id == MSId );/* it could have been a while */
	}
	Unlock_state();

	if (same)/* it could have been a while */
		Finish_registration();

	free(id);
}

static void
Update_server_thread (int *id)
{
	int same;

	Lock_state();
	{
		same = ( *id == MSRegisteredId );
	}
	Unlock_state();

	if (same)
		Finish_update();

	free(id);
}

static void
Unlist_server_thread (int *id)
{
	int same;

	Lock_state();
	{
		same = ( *id == MSRegisteredId );
	}
	Unlock_state();

	if (same)
		Finish_unlist();

	free(id);
}

static void
Change_masterserver_thread (char *api)
{
	Lock_state();
	{
		while (MSRegistered)
			I_hold_cond(&MSCond, MSMutex);
	}
	Unlock_state();

	Finish_masterserver_change(api);
}
#endif/*HAVE_THREADS*/

void RegisterServer(void)
{
#ifdef MASTERSERVER
#ifdef HAVE_THREADS
	I_spawn_thread(
			"register-server",
			(I_thread_fn)Register_server_thread,
			New_server_id()
	);
#else
	Finish_registration();
#endif
#endif/*MASTERSERVER*/
}

static void UpdateServer(void)
{
#ifdef HAVE_THREADS
	I_spawn_thread(
			"update-server",
			(I_thread_fn)Update_server_thread,
			Server_id()
	);
#else
	Finish_update();
#endif
}

void UnregisterServer(void)
{
#ifdef MASTERSERVER
#ifdef HAVE_THREADS
	I_spawn_thread(
			"unlist-server",
			(I_thread_fn)Unlist_server_thread,
			Server_id()
	);
#else
	Finish_unlist();
#endif
#endif/*MASTERSERVER*/
}

char *GetMasterServerRules(void)
{
	char *rules;

	Lock_state();
	rules = MSRules ? Z_StrDup(MSRules) : NULL;
	Unlock_state();

	return rules;
}

static boolean
Online (void)
{
	return ( serverrunning && netgame && cv_advertise.value );
}

static inline void SendPingToMasterServer(void)
{
	int ready;
	time_t now;

	if (Online())
	{
		time(&now);

		Lock_state();
		{
			ready = (
					MSRegisteredId == MSId &&
					! MSInProgress &&
					now >= ( MSLastPing + 60 * cv_masterserver_update_rate.value )
			);

			if (ready)
				MSInProgress = true;
		}
		Unlock_state();

		if (ready)
			UpdateServer();
	}
}

void MasterClient_Ticker(void)
{
#ifdef MASTERSERVER
	SendPingToMasterServer();
#endif
}

static void
Set_api (const char *api)
{
#ifdef HAVE_THREADS
	I_spawn_thread(
			"change-masterserver",
			(I_thread_fn)Change_masterserver_thread,
			strdup(api)
	);
#else
	Finish_masterserver_change(strdup(api));
#endif
}

#endif/*MASTERSERVER*/

static void
Update_parameters (void)
{
#ifdef MASTERSERVER
	int registered;
	int delayed;

	if (Online())
	{
		Lock_state();
		{
			delayed = MSInProgress;

			if (delayed)/* do another update after the current one */
				MSUpdateAgain = true;
			else
				registered = MSRegistered;
		}
		Unlock_state();

		if (! delayed && registered)
			UpdateServer();
	}
#endif/*MASTERSERVER*/
}

static void MasterServer_OnChange(void)
{
#ifdef MASTERSERVER
	UnregisterServer();

	Set_api(cv_masterserver.string);

	if (Online())
		RegisterServer();
#endif/*MASTERSERVER*/
}

static void
Advertise_OnChange(void)
{
	int different;

	if (cv_advertise.value)
	{
		if (serverrunning && netgame)
		{
			Lock_state();
			{
				different = ( MSId != MSRegisteredId );
			}
			Unlock_state();

			if (different)
			{
				RegisterServer();
			}
		}
	}
	else
	{
		UnregisterServer();
	}

#ifdef HAVE_DISCORDRPC
	DRPC_UpdatePresence();
#endif

	if (!dedicated)
	{
		M_PopupMasterServerRules();
	}
}

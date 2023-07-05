// SONIC ROBO BLAST 2 KART
//-----------------------------------------------------------------------------
// Copyright (C) 2020 by James R.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
// \brief HTTP based master server

/*
Documentation available here.

                     <https://ms.kartkrew.org/tools/api/2.2/>
*/

#ifdef HAVE_CURL
#include <curl/curl.h>
#endif

#include "doomdef.h"
#include "d_clisrv.h"
#include "command.h"
#include "console.h"
#include "m_argv.h"
#include "m_menu.h"
#include "mserv.h"
#include "i_tcp.h"/* for current_port */
#include "i_threads.h"

/* reasonable default I guess?? */
#define DEFAULT_BUFFER_SIZE (4096)

/* I just stop myself from making macros anymore. */
#define Blame( ... ) \
	CONS_Printf("\x85" __VA_ARGS__)

static void MasterServer_Debug_OnChange (void);

consvar_t cv_masterserver_timeout = {
	"masterserver_timeout", "5", CV_SAVE, CV_Unsigned,
	NULL, 0, NULL, NULL, 0, 0, NULL/* C90 moment */
};

consvar_t cv_masterserver_debug = {
	"masterserver_debug", "Off", CV_SAVE|CV_CALL, CV_OnOff,
	MasterServer_Debug_OnChange, 0, NULL, NULL, 0, 0, NULL/* C90 moment */
};

consvar_t cv_masterserver_token = {
	"masterserver_token", "", CV_SAVE, NULL,
	NULL, 0, NULL, NULL, 0, 0, NULL/* C90 moment */
};

#define HMS_QUERY_VERSION "?v=2.2"

#ifdef MASTERSERVER

static int hms_started;

static char *hms_api;
static I_mutex hms_api_mutex;

static char *hms_server_token;

struct HMS_buffer
{
	CURL *curl;
	char *buffer;
	int   needle;
	int    end;
};

static void
Contact_error (void)
{
	CONS_Alert(CONS_ERROR,
			"There was a problem contacting the master server...\n"
	);
}

static void
Printf_url (const char *url)
{
	boolean startup;

	I_lock_mutex(&con_mutex);
	startup = con_startup;
	I_unlock_mutex(con_mutex);

	(startup ? I_OutputMsg : CONS_Printf)(
			"HMS: connecting '%s'...\n", url);
}

static size_t
HMS_on_read (char *s, size_t _1, size_t n, void *userdata)
{
	struct HMS_buffer *buffer;
	size_t blocks;

	(void)_1;

	buffer = userdata;

	if (n >= (size_t)( buffer->end - buffer->needle ))
	{
		/* resize to next multiple of buffer size */
		blocks = ( n / DEFAULT_BUFFER_SIZE + 1 );
		buffer->end += ( blocks * DEFAULT_BUFFER_SIZE );

		buffer->buffer = realloc(buffer->buffer, buffer->end);
	}

	memcpy(&buffer->buffer[buffer->needle], s, n);
	buffer->needle += n;

	return n;
}

static struct HMS_buffer *
HMS_connect (const char *format, ...)
{
	va_list ap;
	CURL *curl;
	char *url;
	char *quack_token;
	size_t seek;
	size_t token_length;
	struct HMS_buffer *buffer;

	if (! hms_started)
	{
		if (curl_global_init(CURL_GLOBAL_ALL) != 0)
		{
			Contact_error();
			Blame("From curl_global_init.\n");
			return NULL;
		}
		else
		{
			atexit(curl_global_cleanup);
			hms_started = 1;
		}
	}

	curl = curl_easy_init();

	if (! curl)
	{
		Contact_error();
		Blame("From curl_easy_init.\n");
		return NULL;
	}

	if (cv_masterserver_token.string && cv_masterserver_token.string[0])
	{
		quack_token = curl_easy_escape(curl, cv_masterserver_token.string, 0);
		token_length = ( sizeof "&token="-1 )+ strlen(quack_token);
	}
	else
	{
		quack_token = NULL;
		token_length = 0;
	}

	I_lock_mutex(&hms_api_mutex);

	seek = strlen(hms_api) + 1;/* + '/' */

	va_start (ap, format);
	url = malloc(seek + vsnprintf(0, 0, format, ap) +
			sizeof HMS_QUERY_VERSION - 1 +
			token_length + 1);
	va_end (ap);

	sprintf(url, "%s/", hms_api);

	I_unlock_mutex(hms_api_mutex);

	va_start (ap, format);
	seek += vsprintf(&url[seek], format, ap);
	va_end (ap);

	strcpy(&url[seek], HMS_QUERY_VERSION);
	seek += sizeof HMS_QUERY_VERSION - 1;

	if (quack_token)
		sprintf(&url[seek], "&token=%s", quack_token);

	Printf_url(url);

	buffer = malloc(sizeof *buffer);
	buffer->curl = curl;
	buffer->end = DEFAULT_BUFFER_SIZE;
	buffer->buffer = malloc(buffer->end);
	buffer->needle = 0;

	if (cv_masterserver_debug.value)
	{
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		curl_easy_setopt(curl, CURLOPT_STDERR, logstream);
	}

	if (M_CheckParm("-bindaddr") && M_IsNextParm())
	{
		curl_easy_setopt(curl, CURLOPT_INTERFACE, M_GetNextParm());
	}

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);

	curl_easy_setopt(curl, CURLOPT_TIMEOUT, cv_masterserver_timeout.value);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, HMS_on_read);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, buffer);

	curl_free(quack_token);
	free(url);

	return buffer;
}

static int
HMS_do (struct HMS_buffer *buffer)
{
	CURLcode cc;
	long status;

	char *p;

	cc = curl_easy_perform(buffer->curl);

	if (cc != CURLE_OK)
	{
		Contact_error();
		Blame(
				"From curl_easy_perform: %s\n",
				curl_easy_strerror(cc)
		);
		return 0;
	}

	buffer->buffer[buffer->needle] = '\0';

	curl_easy_getinfo(buffer->curl, CURLINFO_RESPONSE_CODE, &status);

	if (status != 200)
	{
		p = strchr(buffer->buffer, '\n');

		if (p)
			*p = '\0';

		Contact_error();
		Blame(
				"Master server error %ld: %s%s\n",
				status,
				buffer->buffer,
				( (p) ? "" : " (malformed)" )
		);

		return 0;
	}
	else
		return 1;
}

static void
HMS_end (struct HMS_buffer *buffer)
{
	curl_easy_cleanup(buffer->curl);
	free(buffer->buffer);
	free(buffer);
}

int
HMS_register (void)
{
	struct HMS_buffer *hms;
	int ok;

	char post[256];

	char *contact;

	hms = HMS_connect(
			"games/%s/%d/servers/register", SRB2APPLICATION, MODVERSION);

	if (! hms)
		return 0;

	contact = curl_easy_escape(hms->curl, cv_server_contact.string, 0);

	snprintf(post, sizeof post,
			"port=%d&"
			"contact=%s",

			current_port,

			contact
	);

	curl_free(contact);

	curl_easy_setopt(hms->curl, CURLOPT_POSTFIELDS, post);

	ok = HMS_do(hms);

	if (ok)
	{
		hms_server_token = strdup(strtok(hms->buffer, "\n"));
	}

	HMS_end(hms);

	return ok;
}

int
HMS_unlist (void)
{
	struct HMS_buffer *hms;
	int ok;

	hms = HMS_connect("servers/%s/unlist", hms_server_token);

	if (! hms)
		return 0;

	curl_easy_setopt(hms->curl, CURLOPT_CUSTOMREQUEST, "POST");

	ok = HMS_do(hms);
	HMS_end(hms);

	free(hms_server_token);

	return ok;
}

int
HMS_update (void)
{
	struct HMS_buffer *hms;
	int ok;

	char post[256];

	char *title;

	hms = HMS_connect("servers/%s/update", hms_server_token);

	if (! hms)
		return 0;

	title = curl_easy_escape(hms->curl, cv_servername.string, 0);

	snprintf(post, sizeof post,
			"title=%s",
			title
	);

	curl_free(title);

	curl_easy_setopt(hms->curl, CURLOPT_POSTFIELDS, post);

	ok = HMS_do(hms);
	HMS_end(hms);

	return ok;
}

void
HMS_list_servers (void)
{
	struct HMS_buffer *hms;

	hms = HMS_connect("games/%s/%d/servers", SRB2APPLICATION, MODVERSION);

	if (! hms)
		return;

	if (HMS_do(hms))
	{
		CONS_Printf("%s\n", hms->buffer);
	}

	HMS_end(hms);
}

msg_server_t *
HMS_fetch_servers (msg_server_t *list, int query_id)
{
	struct HMS_buffer *hms;

	int doing_shit;

	char *address;
	char *port;
	char *contact;

	char *end;
	char *p;

	int i;

	(void)query_id;

	hms = HMS_connect("games/%s/%d/servers", SRB2APPLICATION, MODVERSION);

	if (! hms)
		return NULL;

	if (HMS_do(hms))
	{
		doing_shit = 1;

		p = hms->buffer;
		i = 0;

		while (i < MAXSERVERLIST && ( end = strchr(p, '\n') ))
		{
			*end = '\0';

			address = strtok(p, " ");
			port    = strtok(0, " ");
			contact = strtok(0, "");

			if (address && port)
			{
				I_lock_mutex(&ms_QueryId_mutex);
				{
					if (query_id != ms_QueryId)
						doing_shit = 0;
				}
				I_unlock_mutex(ms_QueryId_mutex);

				if (! doing_shit)
					break;

				strlcpy(list[i].ip,      address, sizeof list[i].ip);
				strlcpy(list[i].port,    port,    sizeof list[i].port);

				if (contact)
				{
					strlcpy(list[i].contact, contact, sizeof list[i].contact);
				}

				list[i].header.buffer[0] = 1;

				i++;

				p = ( end + 1 );/* skip server delimiter */
			}
			else
			{
				/* malformed so quit the parsing */
				break;
			}
		}

		if (doing_shit)
			list[i].header.buffer[0] = 0;
	}
	else
		list = NULL;

	HMS_end(hms);

	return list;
}

int
HMS_compare_mod_version (char *buffer, size_t buffer_size)
{
	struct HMS_buffer *hms;
	int ok;

	char *version;
	char *version_name;

	hms = HMS_connect("games/%s/version", SRB2APPLICATION);

	if (! hms)
		return 0;

	ok = 0;

	if (HMS_do(hms))
	{
		version      = strtok(hms->buffer, " ");
		version_name = strtok(0, "\n");

		if (version && version_name)
		{
			if (atoi(version) != MODVERSION)
			{
				strlcpy(buffer, version_name, buffer_size);
				ok = 1;
			}
			else
				ok = -1;
		}
	}

	HMS_end(hms);

	return ok;
}

const char *
HMS_fetch_rules (char *buffer, size_t buffer_size)
{
	struct HMS_buffer *hms;

	hms = HMS_connect("rules");

	if (! hms)
		return NULL;

	if (HMS_do(hms))
	{
		char *p = strstr(hms->buffer, "\n\n");

		if (p)
		{
			p[1] = '\0';

			strlcpy(buffer, hms->buffer, buffer_size);
		}
		else
			buffer = NULL;
	}

	HMS_end(hms);

	return buffer;
}

static char *
Strip_trailing_slashes (char *api)
{
	char * p = &api[strlen(api)];

	while (*--p == '/')
		;

	p[1] = '\0';

	return api;
}

void
HMS_set_api (char *api)
{
	I_lock_mutex(&hms_api_mutex);
	{
		free(hms_api);
		hms_api = Strip_trailing_slashes(api);
	}
	I_unlock_mutex(hms_api_mutex);
}

#endif/*MASTERSERVER*/

static void
MasterServer_Debug_OnChange (void)
{
#ifdef MASTERSERVER
	/* TODO: change to 'latest-log.txt' for log files revision. */
	if (cv_masterserver_debug.value)
		CONS_Printf("Master server debug messages will appear in log.txt\n");
#endif
}

// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 1999-2018 by Sonic Team Junior.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  d_netfil.c
/// \brief Transfer a file using HSendPacket.

#include <stdio.h>
#ifndef _WIN32_WCE
#ifdef __OS2__
#include <sys/types.h>
#endif // __OS2__
#include <sys/stat.h>
#endif

#if !defined (UNDER_CE)
#include <time.h>
#endif

#if ((defined (_WIN32) && !defined (_WIN32_WCE)) || defined (__DJGPP__)) && !defined (_XBOX)
#include <io.h>
#include <direct.h>
#elif !defined (_WIN32_WCE) && !(defined (_XBOX) && !defined (__GNUC__))
#include <sys/types.h>
#include <dirent.h>
#include <utime.h>
#endif

#ifdef __GNUC__
#include <unistd.h>
#include <limits.h>
#elif defined (_WIN32) && !defined (_WIN32_WCE)
#include <sys/utime.h>
#endif
#ifdef __DJGPP__
#include <dir.h>
#include <utime.h>
#endif

#ifdef HAVE_CURL
#include "curl/curl.h"
#endif

#include "doomdef.h"
#include "doomstat.h"
#include "d_main.h"
#include "g_game.h"
#include "i_time.h"
#include "i_net.h"
#include "i_system.h"
#include "m_argv.h"
#include "d_net.h"
#include "w_wad.h"
#include "d_netfil.h"
#include "z_zone.h"
#include "byteptr.h"
#include "p_setup.h"
#include "m_misc.h"
#include "m_menu.h"
#include "md5.h"
#include "filesrch.h"

#include <errno.h>

// Prototypes
static boolean SV_SendFile(INT32 node, const char *filename, UINT8 fileid);

#ifdef HAVE_CURL
size_t curlwrite_data(void *ptr, size_t size, size_t nmemb, FILE *stream);
int curlprogress_callback(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow);
#endif

// Sender structure
typedef struct filetx_s
{
	INT32 ram;
	union {
		char *filename; // Name of the file
		char *ram; // Pointer to the data in RAM
	} id;
	UINT32 size; // Size of the file
	UINT8 fileid;
	INT32 node; // Destination
	struct filetx_s *next; // Next file in the list
} filetx_t;

// Current transfers (one for each node)
typedef struct filetran_s
{
	filetx_t *txlist; // Linked list of all files for the node
	UINT32 position; // The current position in the file
	FILE *currentfile; // The file currently being sent/received
} filetran_t;
static filetran_t transfer[MAXNETNODES];

// Read time of file: stat _stmtime
// Write time of file: utime

// Receiver structure
INT32 fileneedednum; // Number of files needed to join the server
fileneeded_t fileneeded[MAX_WADFILES]; // List of needed files
char downloaddir[512] = "DOWNLOAD";

#ifdef CLIENT_LOADINGSCREEN
// for cl loading screen
INT32 lastfilenum = -1;
INT32 downloadcompletednum = 0;
UINT32 downloadcompletedsize = 0;
INT32 totalfilesrequestednum = 0;
UINT32 totalfilesrequestedsize = 0;
#endif

#ifdef HAVE_CURL
static CURL *http_handle;
static CURLM *multi_handle;
boolean curl_running = false;
boolean curl_failedwebdownload = false;
static double curl_dlnow;
static double curl_dltotal;
static time_t curl_starttime;
INT32 curl_transfers = 0;
static int curl_runninghandles = 0;
static UINT32 curl_origfilesize;
static UINT32 curl_origtotalfilesize;
static char *curl_realname = NULL;
fileneeded_t *curl_curfile = NULL;
HTTP_login *curl_logins;
#endif

/** Fills a serverinfo packet with information about wad files loaded.
  *
  * \todo Give this function a better name since it is in global scope.
  * Used to have size limiting built in - now handed via W_LoadWadFile in w_wad.c
  *
  */
UINT8 *PutFileNeeded(UINT16 firstfile)
{
	size_t i;
	UINT8 count = 0;
	UINT8 *p_start = netbuffer->packettype == PT_MOREFILESNEEDED ? netbuffer->u.filesneededcfg.files : netbuffer->u.serverinfo.fileneeded;
	UINT8 *p = p_start;
	char wadfilename[MAX_WADPATH] = "";
	UINT8 filestatus;

	for (i = mainwads+1; i < numwadfiles; i++) //mainwads+1, otherwise we start on the first mainwad
	{
		// If it has only music/sound lumps, don't put it in the list
		if (!wadfiles[i]->important)
			continue;

		if (firstfile)
		{ // Skip files until we reach the first file.
			firstfile--;
			continue;
		}

		nameonly(strcpy(wadfilename, wadfiles[i]->filename));

		// Look below at the WRITE macros to understand what these numbers mean.
		if (p + 1 + 4 + min(strlen(wadfilename) + 1, MAX_WADPATH) + 16 > p_start + MAXFILENEEDED)
		{
			// Too many files to send all at once
			if (netbuffer->packettype == PT_MOREFILESNEEDED)
				netbuffer->u.filesneededcfg.more = 1;
			else
				netbuffer->u.serverinfo.kartvars |= SV_LOTSOFADDONS;
			break;
		}

		filestatus = 1; // Importance - not really used any more, holds 1 by default for backwards compat with MS

		// Store in the upper four bits
		if (!cv_downloading.value)
			filestatus += (2 << 4); // Won't send
		else if ((wadfiles[i]->filesize <= (UINT32)cv_maxsend.value * 1024))
			filestatus += (1 << 4); // Will send if requested
		// else
			// filestatus += (0 << 4); -- Won't send, too big

		WRITEUINT8(p, filestatus);

		count++;
		WRITEUINT32(p, wadfiles[i]->filesize);
		WRITESTRINGN(p, wadfilename, MAX_WADPATH);
		WRITEMEM(p, wadfiles[i]->md5sum, 16);
	}
	if (netbuffer->packettype == PT_MOREFILESNEEDED)
		netbuffer->u.filesneededcfg.num = count;
	else
		netbuffer->u.serverinfo.fileneedednum = count;

	return p;
}

/** Parses the serverinfo packet and fills the fileneeded table on client
  *
  * \param fileneedednum_parm The number of files (sent in this page) needed to join the server
  * \param fileneededstr The memory block containing the list of needed files
  * \param firstfile The first file index to read from
  */
void D_ParseFileneeded(INT32 fileneedednum_parm, UINT8 *fileneededstr, UINT16 firstfile)
{
	INT32 i;
	UINT8 *p;
	UINT8 filestatus;

	fileneedednum = firstfile + fileneedednum_parm;
	p = (UINT8 *)fileneededstr;
	for (i = firstfile; i < fileneedednum; i++)
	{
		fileneeded[i].status = FS_NOTCHECKED; // We haven't even started looking for the file yet
		filestatus = READUINT8(p); // The first byte is the file status
		fileneeded[i].willsend = (UINT8)(filestatus >> 4);
		fileneeded[i].totalsize = READUINT32(p); // The four next bytes are the file size
		fileneeded[i].file = NULL; // The file isn't open yet
		READSTRINGN(p, fileneeded[i].filename, MAX_WADPATH); // The next bytes are the file name
		READMEM(p, fileneeded[i].md5sum, 16); // The last 16 bytes are the file checksum
	}
}

void CL_PrepareDownloadSaveGame(const char *tmpsave)
{
	fileneedednum = 1;
	fileneeded[0].status = FS_REQUESTED;
	fileneeded[0].totalsize = UINT32_MAX;
	fileneeded[0].file = NULL;
	memset(fileneeded[0].md5sum, 0, 16);
	strcpy(fileneeded[0].filename, tmpsave);
}

/** Checks the server to see if we CAN download all the files,
  * before starting to create them and requesting.
  *
  * \return True if we can download all the files
  *
  */
boolean CL_CheckDownloadable(void)
{
	UINT8 i,dlstatus = 0;

	for (i = 0; i < fileneedednum; i++)
		if (fileneeded[i].status != FS_FOUND && fileneeded[i].status != FS_OPEN)
		{
			if (fileneeded[i].willsend == 1)
				continue;

			if (fileneeded[i].willsend == 0)
				dlstatus = 1;
			else //if (fileneeded[i].willsend == 2)
				dlstatus = 2;
		}

	// Downloading locally disabled
	if (!dlstatus && M_CheckParm("-nodownload"))
		dlstatus = 3;

	if (!dlstatus)
		return true;

	// not downloadable, put reason in console
	CONS_Alert(CONS_NOTICE, M_GetText("You need additional files to connect to this server:\n"));
	for (i = 0; i < fileneedednum; i++)
		if (fileneeded[i].status != FS_FOUND && fileneeded[i].status != FS_OPEN)
		{
			CONS_Printf(" * \"%s\" (%dK)", fileneeded[i].filename, fileneeded[i].totalsize >> 10);

				if (fileneeded[i].status == FS_MD5SUMBAD)
					CONS_Printf(M_GetText(" wrong version, md5: "));
				else
					CONS_Printf(M_GetText(" not found, md5: "));

			{
				INT32 j;
				char md5tmp[33];
				for (j = 0; j < 16; j++)
					sprintf(&md5tmp[j*2], "%02x", fileneeded[i].md5sum[j]);
				CONS_Printf("%s", md5tmp);
			}
			CONS_Printf("\n");
		}

	switch (dlstatus)
	{
		case 1:
			CONS_Printf(M_GetText("Some files are larger than the server is willing to send.\n"));
			break;
		case 2:
			CONS_Printf(M_GetText("The server is not allowing download requests.\n"));
			break;
		case 3:
			CONS_Printf(M_GetText("All files downloadable, but you have chosen to disable downloading locally.\n"));
			break;
	}
	return false;
}

// The following was written and, against all odds, works.
#define MORELEGACYDOWNLOADER

/** Sends requests for files in the ::fileneeded table with a status of
  * ::FS_NOTFOUND.
  *
  * \return True if the packet was successfully sent
  * \note Sends a PT_REQUESTFILE packet
  *
  */
boolean CL_SendRequestFile(void)
{
	char *p;
	INT32 i;
	INT64 totalfreespaceneeded = 0, availablefreespace;
	INT32 skippedafile = -1;
#ifdef MORELEGACYDOWNLOADER
	boolean firstloop = true;
#endif

#ifdef PARANOIA
	if (M_CheckParm("-nodownload"))
	{
		CONS_Printf("Direct download - Attempted to download files in -nodownload mode");
		return false;
	}
#endif

	for (i = 0; i < fileneedednum; i++)
	{
		if (fileneeded[i].status != FS_FOUND && fileneeded[i].status != FS_OPEN
			&& (fileneeded[i].willsend == 0 || fileneeded[i].willsend == 2))
		{
			CONS_Printf("Direct download - attempted to download files that were not sendable\n");
			return false;
		}

		if ((fileneeded[i].status == FS_NOTFOUND || fileneeded[i].status == FS_MD5SUMBAD || fileneeded[i].status == FS_FALLBACK))
		{
			// Error check for the first time around.
			totalfreespaceneeded += fileneeded[i].totalsize;
		}
	}

	I_GetDiskFreeSpace(&availablefreespace);
	if (totalfreespaceneeded > availablefreespace)
	{
		CONS_Printf("Direct download -\n"
			" To play on this server you must download %s KB,\n"
			" but you have only %s KB free space on this drive\n",
			sizeu1((size_t)(totalfreespaceneeded>>10)), sizeu2((size_t)(availablefreespace>>10)));
		return false;
	}

#ifdef MORELEGACYDOWNLOADER
tryagain:
	skippedafile = -1;
#endif

#ifdef VERBOSEREQUESTFILE
	CONS_Printf("Preparing packet\n");
#endif

	netbuffer->packettype = PT_REQUESTFILE;
	p = (char *)netbuffer->u.textcmd;

	for (i = 0; i < fileneedednum; i++)
	{
		if ((fileneeded[i].status == FS_NOTFOUND || fileneeded[i].status == FS_MD5SUMBAD || fileneeded[i].status == FS_FALLBACK))
		{
			// Pre-prepare.
			size_t checklen;
			nameonly(fileneeded[i].filename);

			// Figure out if we'd overrun our buffer.
			checklen = strlen(fileneeded[i].filename)+2; // plus the fileid (and terminator, in case this is last)
			if ((UINT8 *)(p + checklen) >= netbuffer->u.textcmd + MAXTEXTCMD)
			{
				skippedafile = i;
				// we might have a shorter file that can fit in the remaining space, and file ID permits out-of-order data
				continue;
			}

			// Now write.
			WRITEUINT8(p, i); // fileid
			WRITESTRINGN(p, fileneeded[i].filename, MAX_WADPATH);

#ifdef VERBOSEREQUESTFILE
			CONS_Printf(" file \"%s\" (id %d)\n", i, fileneeded[i].filename);
#endif

			// put it in download dir
			strcatbf(fileneeded[i].filename, downloaddir, "/");
			fileneeded[i].status = FS_REQUESTED;
		}
	}

#ifdef MORELEGACYDOWNLOADER
	if (firstloop)
#else
	// If we're not trying extralong legacy download requests, gotta bail.
	if (skippedafile != -1)
	{
		CONS_Printf("Direct download - missing files are as follows:\n");
		for (i = 0; i < fileneedednum; i++)
		{
			if ((fileneeded[i].status == FS_NOTFOUND || fileneeded[i].status == FS_MD5SUMBAD || fileneeded[i].status == FS_FALLBACK || fileneeded[i].status == FS_REQUESTED)) // FS_REQUESTED added
				CONS_Printf(" %s\n", fileneeded[i].filename);
		}
		return false;
	}
#endif
		I_mkdir(downloaddir, 0755);

	// Couldn't fit a single one in?
	if (p == (char *)netbuffer->u.textcmd)
	{
		CONS_Printf("Direct download - fileneeded name for %s (fileneeded[%d]) too long??\n", (skippedafile != -1 ? fileneeded[skippedafile].filename : NULL), skippedafile);
		return false;
	}

	WRITEUINT8(p, 0xFF); // terminator
	if (!HSendPacket(servernode, true, 0, p - (char *)netbuffer->u.textcmd))
	{
		CONS_Printf("Direct download - unable to send packet.\n");
		return false;
	}

#ifdef MORELEGACYDOWNLOADER
	if (skippedafile != -1)
	{
		firstloop = false;
		goto tryagain;
	}
#endif

#ifdef VERBOSEREQUESTFILE
	CONS_Printf("Returning true\n");
#endif

	return true;
}

// get request filepak and put it on the send queue
// returns false if a requested file was not found or cannot be sent
boolean Got_RequestFilePak(INT32 node)
{
	char wad[MAX_WADPATH+1];
	UINT8 *p = netbuffer->u.textcmd;
	UINT8 id;
	while (p < netbuffer->u.textcmd + MAXTEXTCMD) // Don't allow hacked client to overflow
	{
		id = READUINT8(p);
		if (id == 0xFF)
			break;
		READSTRINGN(p, wad, MAX_WADPATH);
		if (p >= netbuffer->u.textcmd + MAXTEXTCMD || !SV_SendFile(node, wad, id))
		{
			if (cv_noticedownload.value)
				CONS_Printf("Bad PT_REQUESTFILE from node %d!\n", node);
			SV_AbortSendFiles(node);
			return false; // don't read any more
		}
	}
	return true; // no problems with any files
}

/** Checks if the files needed aren't already loaded or on the disk
  *
  * \return 0 if some files are missing
  *         1 if all files exist
  *         2 if some already loaded files are not requested or are in a different order
  * 		3 too many files, over WADLIMIT
  * 		4 still checking, continuing next tic
  *
  */
INT32 CL_CheckFiles(void)
{
	INT32 i, j;
	char wadfilename[MAX_WADPATH];
	size_t packetsize = 0;
	size_t filestoload = 0;
	boolean downloadrequired = false;

//	if (M_CheckParm("-nofiles"))
//		return 1;

	// the first is the iwad (the main wad file)
	// we don't care if it's called srb2.srb or srb2.wad.
	// Never download the IWAD, just assume it's there and identical
	// ...No! Why were we sending the base wads to begin with??
	//fileneeded[0].status = FS_OPEN;

	// Modified game handling -- check for an identical file list
	// must be identical in files loaded AND in order
	// Return 2 on failure -- disconnect from server
	if (modifiedgame)
	{
		CONS_Debug(DBG_NETPLAY, "game is modified; only doing basic checks\n");
		for (i = 0, j = mainwads+1; i < fileneedednum || j < numwadfiles;)
		{
			if (j < numwadfiles && !wadfiles[j]->important)
			{
				// Unimportant on our side. still don't care.
				++j;
				continue;
			}

			// If this test is true, we've reached the end of one file list
			// and the other still has a file that's important
			if (i >= fileneedednum || j >= numwadfiles)
				return 2;

			// For the sake of speed, only bother with a md5 check
			if (memcmp(wadfiles[j]->md5sum, fileneeded[i].md5sum, 16))
				return 2;

			// It's accounted for! let's keep going.
			CONS_Debug(DBG_NETPLAY, "'%s' accounted for\n", fileneeded[i].filename);
			fileneeded[i].status = FS_OPEN;
			++i;
			++j;
		}
		return 1;
	}

	for (i = 0; i < fileneedednum; i++)
	{
		if (fileneeded[i].status == FS_NOTFOUND || fileneeded[i].status == FS_MD5SUMBAD || fileneeded[i].status == FS_FALLBACK)
			downloadrequired = true;
		
		if (fileneeded[i].status != FS_OPEN)
			filestoload++;

		if (fileneeded[i].status != FS_NOTCHECKED) //since we're running this over multiple tics now, its possible for us to come across files checked in previous tics
			continue;
		
		CONS_Debug(DBG_NETPLAY, "searching for '%s' ", fileneeded[i].filename);

		// Check in already loaded files
		for (j = mainwads+1; wadfiles[j]; j++)
		{
			nameonly(strcpy(wadfilename, wadfiles[j]->filename));
			if (!stricmp(wadfilename, fileneeded[i].filename) &&
				!memcmp(wadfiles[j]->md5sum, fileneeded[i].md5sum, 16))
			{
				CONS_Debug(DBG_NETPLAY, "already loaded\n");
				fileneeded[i].status = FS_OPEN;
				return 4;
			}
		}

		packetsize += nameonlylength(fileneeded[i].filename) + 22;

		fileneeded[i].status = findfile(fileneeded[i].filename, fileneeded[i].md5sum, true);
		CONS_Debug(DBG_NETPLAY, "found %d\n", fileneeded[i].status);
		return 4;
	}

	//now making it here means we've checked the entire list and no FS_NOTCHECKED files remain
	if (numwadfiles+filestoload > MAX_WADFILES)
		return 3;
	else if (downloadrequired)
		return 0; //some stuff is FS_NOTFOUND, needs download
	else
		return 1; //everything is FS_OPEN or FS_FOUND, proceed to loading
}

// Load it now
boolean CL_LoadServerFiles(void)
{
	INT32 i;

//	if (M_CheckParm("-nofiles"))
//		return;

	for (i = 0; i < fileneedednum; i++)
	{
		if (fileneeded[i].status == FS_OPEN)
			continue; // Already loaded
		else if (fileneeded[i].status == FS_FOUND)
		{
			P_PartialAddWadFile(fileneeded[i].filename);
			G_SetGameModified(true, false);
			fileneeded[i].status = FS_OPEN;
			return false;
		}
		else if (fileneeded[i].status == FS_MD5SUMBAD)
			I_Error("Wrong version of file %s", fileneeded[i].filename);
		else
		{
			const char *s;
			switch(fileneeded[i].status)
			{
			case FS_NOTFOUND:
				s = "FS_NOTFOUND";
				break;
			case FS_REQUESTED:
				s = "FS_REQUESTED";
				break;
			case FS_DOWNLOADING:
				s = "FS_DOWNLOADING";
				break;
			default:
				s = "unknown";
				break;
			}
			I_Error("Try to load file \"%s\" with status of %d (%s)\n", fileneeded[i].filename,
				fileneeded[i].status, s);
		}
	}
	return true;
}

// Number of files to send
// Little optimization to quickly test if there is a file in the queue
static INT32 filestosend = 0;

/** Adds a file to the file list for a node
  *
  * \param node The node to send the file to
  * \param filename The file to send
  * \param fileid ???
  * \sa SV_SendRam
  *
  */
static boolean SV_SendFile(INT32 node, const char *filename, UINT8 fileid)
{
	filetx_t **q; // A pointer to the "next" field of the last file in the list
	filetx_t *p; // The new file request
	INT32 i;
	char wadfilename[MAX_WADPATH];

	if (cv_noticedownload.value)
		CONS_Printf("Sending file \"%s\" (id %d) to node %d (%s)\n", filename, fileid, node, I_GetNodeAddress(node));

	// Find the last file in the list and set a pointer to its "next" field
	q = &transfer[node].txlist;
	while (*q)
		q = &((*q)->next);

	// Allocate a file request and append it to the file list
	p = *q = (filetx_t *)malloc(sizeof (filetx_t));
	if (!p)
		I_Error("SV_SendFile: No more memory\n");

	// Initialise with zeros
	memset(p, 0, sizeof (filetx_t));

	// Allocate the file name
	p->id.filename = (char *)malloc(MAX_WADPATH);
	if (!p->id.filename)
		I_Error("SV_SendFile: No more memory\n");

	// Set the file name and get rid of the path
	strlcpy(p->id.filename, filename, MAX_WADPATH);
	nameonly(p->id.filename);

	// Look for the requested file through all loaded files
	for (i = 0; wadfiles[i]; i++)
	{
		strlcpy(wadfilename, wadfiles[i]->filename, MAX_WADPATH);
		nameonly(wadfilename);
		if (!stricmp(wadfilename, p->id.filename))
		{
			// Copy file name with full path
			strlcpy(p->id.filename, wadfiles[i]->filename, MAX_WADPATH);
			break;
		}
	}

	// Handle non-loaded file requests
	if (!wadfiles[i])
	{
		DEBFILE(va("%s not found in wadfiles\n", filename));
		// This formerly checked if (!findfile(p->id.filename, NULL, true))

		// Not found
		// Don't inform client (probably someone who thought they could leak 2.2 ACZ)
		DEBFILE(va("Client %d request %s: not found\n", node, filename));
		free(p->id.filename);
		free(p);
		*q = NULL;
		return false; // cancel the rest of the requests
	}

	// Handle huge file requests (i.e. bigger than cv_maxsend.value KB)
	if (wadfiles[i]->filesize > (UINT32)cv_maxsend.value * 1024)
	{
		// Too big
		// Don't inform client (client sucks, man)
		DEBFILE(va("Client %d request %s: file too big, not sending\n", node, filename));
		free(p->id.filename);
		free(p);
		*q = NULL;
		return false; // cancel the rest of the requests
	}

	DEBFILE(va("Sending file %s (id=%d) to %d\n", filename, fileid, node));
	p->ram = SF_FILE; // It's a file, we need to close it and free its name once we're done sending it
	p->fileid = fileid;
	p->next = NULL; // End of list
	filestosend++;
	return true;
}

/** Adds a memory block to the file list for a node
  *
  * \param node The node to send the memory block to
  * \param data The memory block to send
  * \param size The size of the block in bytes
  * \param freemethod How to free the block after it has been sent
  * \param fileid ???
  * \sa SV_SendFile
  *
  */
void SV_SendRam(INT32 node, void *data, size_t size, freemethod_t freemethod, UINT8 fileid)
{
	filetx_t **q; // A pointer to the "next" field of the last file in the list
	filetx_t *p; // The new file request

	// Find the last file in the list and set a pointer to its "next" field
	q = &transfer[node].txlist;
	while (*q)
		q = &((*q)->next);

	// Allocate a file request and append it to the file list
	p = *q = (filetx_t *)malloc(sizeof (filetx_t));
	if (!p)
		I_Error("SV_SendRam: No more memory\n");

	// Initialise with zeros
	memset(p, 0, sizeof (filetx_t));

	p->ram = freemethod; // Remember how to free the memory block for when we're done sending it
	p->id.ram = data;
	p->size = (UINT32)size;
	p->fileid = fileid;
	p->next = NULL; // End of list

	DEBFILE(va("Sending ram %p(size:%u) to %d (id=%u)\n",p->id.ram,p->size,node,fileid));

	filestosend++;
}

/** Stops sending a file for a node, and removes the file request from the list,
  * either because the file has been fully sent or because the node was disconnected
  *
  * \param node The destination
  *
  */
static void SV_EndFileSend(INT32 node)
{
	filetx_t *p = transfer[node].txlist;

	// Free the file request according to the freemethod parameter used with SV_SendFile/Ram
	switch (p->ram)
	{
		case SF_FILE: // It's a file, close it and free its filename
			if (cv_noticedownload.value)
				CONS_Printf("Ending file transfer (id %d) for node %d\n", p->fileid, node);
			if (transfer[node].currentfile)
				fclose(transfer[node].currentfile);
			free(p->id.filename);
			break;
		case SF_Z_RAM: // It's a memory block allocated with Z_Alloc or the likes, use Z_Free
			Z_Free(p->id.ram);
			break;
		case SF_RAM: // It's a memory block allocated with malloc, use free
			free(p->id.ram);
		case SF_NOFREERAM: // Nothing to free
			break;
	}

	// Remove the file request from the list
	transfer[node].txlist = p->next;
	free(p);

	// Indicate that the transmission is over
	transfer[node].currentfile = NULL;

	filestosend--;
}

#define PACKETPERTIC net_bandwidth/(TICRATE*software_MAXPACKETLENGTH)

/** Handles file transmission
  *
  * \todo Use an acknowledging method more adapted to file transmission
  *       The current download speed suffers from lack of ack packets,
  *       especially when the one downloading has high latency
  *
  */
void SV_FileSendTicker(void)
{
	static INT32 currentnode = 0;
	filetx_pak *p;
	size_t size;
	filetx_t *f;
	INT32 packetsent, ram, i, j;
	INT32 maxpacketsent;

	if (!filestosend) // No file to send
		return;

	if (cv_downloadspeed.value) // New (and experimental) behavior
	{
		packetsent = cv_downloadspeed.value;
		// Don't send more packets than we have free acks
#ifndef NONET
		maxpacketsent = Net_GetFreeAcks(false) - 5; // Let 5 extra acks just in case
#else
		maxpacketsent = 1;
#endif
		if (packetsent > maxpacketsent && maxpacketsent > 0) // Send at least one packet
			packetsent = maxpacketsent;
	}
	else // Old behavior
	{
		packetsent = PACKETPERTIC;
		if (!packetsent)
			packetsent = 1;
	}

	netbuffer->packettype = PT_FILEFRAGMENT;

	// (((sendbytes-nowsentbyte)*TICRATE)/(I_GetTime()-starttime)<(UINT32)net_bandwidth)
	while (packetsent-- && filestosend != 0)
	{
		for (i = currentnode, j = 0; j < MAXNETNODES;
			i = (i+1) % MAXNETNODES, j++)
		{
			if (transfer[i].txlist)
				goto found;
		}
		// no transfer to do
		I_Error("filestosend=%d but no file to send found\n", filestosend);
	found:
		currentnode = (i+1) % MAXNETNODES;
		f = transfer[i].txlist;
		ram = f->ram;

		// Open the file if it isn't open yet, or
		if (!transfer[i].currentfile)
		{
			if (!ram) // Sending a file
			{
				long filesize;

				transfer[i].currentfile =
					fopen(f->id.filename, "rb");

				if (!transfer[i].currentfile)
					I_Error("File %s does not exist",
						f->id.filename);

				fseek(transfer[i].currentfile, 0, SEEK_END);
				filesize = ftell(transfer[i].currentfile);

				// Nobody wants to transfer a file bigger
				// than 4GB!
				if (filesize >= LONG_MAX)
					I_Error("filesize of %s is too large", f->id.filename);
				if (filesize == -1)
					I_Error("Error getting filesize of %s", f->id.filename);

				f->size = (UINT32)filesize;
				fseek(transfer[i].currentfile, 0, SEEK_SET);
			}
			else // Sending RAM
				transfer[i].currentfile = (FILE *)1; // Set currentfile to a non-null value to indicate that it is open
			transfer[i].position = 0;
		}

		// Build a packet containing a file fragment
		p = &netbuffer->u.filetxpak;
		size = software_MAXPACKETLENGTH - (FILETXHEADER + BASEPACKETSIZE);
		if (f->size-transfer[i].position < size)
			size = f->size-transfer[i].position;
		if (ram)
			M_Memcpy(p->data, &f->id.ram[transfer[i].position], size);
		else if (fread(p->data, 1, size, transfer[i].currentfile) != size)
			I_Error("SV_FileSendTicker: can't read %s byte on %s at %d because %s", sizeu1(size), f->id.filename, transfer[i].position, M_FileError(transfer[i].currentfile));
		p->position = LONG(transfer[i].position);
		// Put flag so receiver knows the total size
		if (transfer[i].position + size == f->size)
			p->position |= LONG(0x80000000);
		p->fileid = f->fileid;
		p->size = SHORT((UINT16)size);

		// Send the packet
		if (HSendPacket(i, true, 0, FILETXHEADER + size)) // Reliable SEND
		{ // Success
			transfer[i].position = (UINT32)(transfer[i].position + size);
			if (transfer[i].position == f->size) // Finish?
				SV_EndFileSend(i);
		}
		else
		{ // Not sent for some odd reason, retry at next call
			if (!ram)
				fseek(transfer[i].currentfile,transfer[i].position, SEEK_SET);
			// Exit the while (can't send this one so why should i send the next?)
			break;
		}
	}
}

void Got_Filetxpak(void)
{
	INT32 filenum = netbuffer->u.filetxpak.fileid;
	fileneeded_t *file = &fileneeded[filenum];
	char *filename = file->filename;
	static INT32 filetime = 0;

	if (!(strcmp(filename, "srb2.srb")
		&& strcmp(filename, "srb2.wad")
		&& strcmp(filename, "patch.dta")
		//&& strcmp(filename, "music.dta")
		&& strcmp(filename, "gfx.kart")
		&& strcmp(filename, "textures.kart")
		&& strcmp(filename, "chars.kart")
		&& strcmp(filename, "maps.kart")
		&& strcmp(filename, "sounds.kart")
		&& strcmp(filename, "music.kart")
		&& strcmp(filename, "patch.kart")
		))
		I_Error("Tried to download \"%s\"", filename);

	if (filenum >= fileneedednum)
	{
		DEBFILE(va("fileframent not needed %d>%d\n", filenum, fileneedednum));
		//I_Error("Received an unneeded file fragment (file id received: %d, file id needed: %d)\n", filenum, fileneedednum);
		return;
	}

	if (file->status == FS_REQUESTED)
	{
		if (file->file)
			I_Error("Got_Filetxpak: already open file\n");
		file->file = fopen(filename, "wb");
		if (!file->file)
			I_Error("Can't create file %s: %s", filename, strerror(errno));
		CONS_Printf("\r%s...\n",filename);
		file->currentsize = 0;
		file->status = FS_DOWNLOADING;
	}

	if (file->status == FS_DOWNLOADING)
	{
		UINT32 pos = LONG(netbuffer->u.filetxpak.position);
		UINT16 size = SHORT(netbuffer->u.filetxpak.size);
		// Use a special trick to know when the file is complete (not always used)
		// WARNING: file fragments can arrive out of order so don't stop yet!
		if (pos & 0x80000000)
		{
			pos &= ~0x80000000;
			file->totalsize = pos + size;
		}
		// We can receive packet in the wrong order, anyway all os support gaped file
		fseek(file->file, pos, SEEK_SET);
		if (fwrite(netbuffer->u.filetxpak.data,size,1,file->file) != 1)
			I_Error("Can't write to %s: %s\n",filename, M_FileError(file->file));
		file->currentsize += size;

		// Finished?
		if (file->currentsize == file->totalsize)
		{
			fclose(file->file);
			file->file = NULL;
			file->status = FS_FOUND;
			CONS_Printf(M_GetText("Downloading %s...(done)\n"),
				filename);
#ifndef NONET
			downloadcompletednum++;
			downloadcompletedsize += file->totalsize;
#endif
		}
	}
	else
	{
		const char *s;
		switch(file->status)
		{
		case FS_NOTFOUND:
			s = "FS_NOTFOUND";
			break;
		case FS_FOUND:
			s = "FS_FOUND";
			break;
		case FS_OPEN:
			s = "FS_OPEN";
			break;
		case FS_MD5SUMBAD:
			s = "FS_MD5SUMBAD";
			break;
		default:
			s = "unknown";
			break;
		}
		I_Error("Received a file not requested (file id: %d, file status: %s)\n", filenum, s);
	}
	// Send ack back quickly
	if (++filetime == 3)
	{
		Net_SendAcks(servernode);
		filetime = 0;
	}

#ifdef CLIENT_LOADINGSCREEN
	lastfilenum = filenum;
#endif
}

/** \brief Checks if a node is downloading a file
 *
 * \param node The node to check for
 * \return True if the node is downloading a file
 *
 */
boolean SV_SendingFile(INT32 node)
{
	return transfer[node].txlist != NULL;
}

/** Cancels all file requests for a node
  *
  * \param node The destination
  * \sa SV_EndFileSend
  *
  */
void SV_AbortSendFiles(INT32 node)
{
	while (transfer[node].txlist)
		SV_EndFileSend(node);
}

void CloseNetFile(void)
{
	INT32 i;
	// Is sending?
	for (i = 0; i < MAXNETNODES; i++)
		SV_AbortSendFiles(i);

	// Receiving a file?
	for (i = 0; i < MAX_WADFILES; i++)
		if (fileneeded[i].status == FS_DOWNLOADING && fileneeded[i].file)
		{
			fclose(fileneeded[i].file);
			// File is not complete delete it
			remove(fileneeded[i].filename);
		}

	// Remove PT_FILEFRAGMENT from acknowledge list
	Net_AbortPacketType(PT_FILEFRAGMENT);
}

// Functions cut and pasted from Doomatic :)

void nameonly(char *s)
{
	size_t j, len;
	void *ns;

	for (j = strlen(s); j != (size_t)-1; j--)
		if ((s[j] == '\\') || (s[j] == ':') || (s[j] == '/'))
		{
			ns = &(s[j+1]);
			len = strlen(ns);
#if 0
				M_Memcpy(s, ns, len+1);
#else
				memmove(s, ns, len+1);
#endif
			return;
		}
}

// Returns the length in characters of the last element of a path.
size_t nameonlylength(const char *s)
{
	size_t j, len = strlen(s);

	for (j = len; j != (size_t)-1; j--)
		if ((s[j] == '\\') || (s[j] == ':') || (s[j] == '/'))
			return len - j - 1;

	return len;
}

#ifndef O_BINARY
#define O_BINARY 0
#endif

filestatus_t checkfilemd5(char *filename, const UINT8 *wantedmd5sum)
{
#if defined (NOMD5) || defined (_arch_dreamcast)
	(void)wantedmd5sum;
	(void)filename;
#else
	FILE *fhandle;
	UINT8 md5sum[16];

	if (!wantedmd5sum)
		return FS_FOUND;

	fhandle = fopen(filename, "rb");
	if (fhandle)
	{
		md5_stream(fhandle,md5sum);
		fclose(fhandle);
		if (!memcmp(wantedmd5sum, md5sum, 16))
			return FS_FOUND;
		return FS_MD5SUMBAD;
	}

	I_Error("Couldn't open %s for md5 check", filename);
#endif
	return FS_FOUND; // will never happen, but makes the compiler shut up
}

// Rewritten by Monster Iestyn to be less stupid
// Note: if completepath is true, "filename" is modified, but only if FS_FOUND is going to be returned
// (Don't worry about WinCE's version of filesearch, nobody cares about that OS anymore)
filestatus_t findfile(char *filename, const UINT8 *wantedmd5sum, boolean completepath)
{
	filestatus_t homecheck; // store result of last file search
	boolean badmd5 = false; // store whether md5 was bad from either of the first two searches (if nothing was found in the third)

	// first, check SRB2's "home" directory
	homecheck = filesearch(filename, srb2home, wantedmd5sum, completepath, 10);

	if (homecheck == FS_FOUND) // we found the file, so return that we have :)
		return FS_FOUND;
	else if (homecheck == FS_MD5SUMBAD) // file has a bad md5; move on and look for a file with the right md5
		badmd5 = true;
	// if not found at all, just move on without doing anything

	// next, check SRB2's "path" directory
	homecheck = filesearch(filename, srb2path, wantedmd5sum, completepath, 10);

	if (homecheck == FS_FOUND) // we found the file, so return that we have :)
		return FS_FOUND;
	else if (homecheck == FS_MD5SUMBAD) // file has a bad md5; move on and look for a file with the right md5
		badmd5 = true;
	// if not found at all, just move on without doing anything

	// finally check "." directory
#ifdef _arch_dreamcast
	homecheck = filesearch(filename, "/cd", wantedmd5sum, completepath, 10);
#else
	homecheck = filesearch(filename, ".", wantedmd5sum, completepath, 10);
#endif

	if (homecheck != FS_NOTFOUND) // if not found this time, fall back on the below return statement
		return homecheck; // otherwise return the result we got

	return (badmd5 ? FS_MD5SUMBAD : FS_NOTFOUND); // md5 sum bad or file not found
}

#ifdef HAVE_CURL
size_t curlwrite_data(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    size_t written;
    written = fwrite(ptr, size, nmemb, stream);
    return written;
}

int curlprogress_callback(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
	(void)clientp;
	(void)ultotal;
	(void)ulnow; // Function prototype requires these but we won't use, so just discard
	curl_dlnow = dlnow;
	curl_dltotal = dltotal;
	getbytes = curl_dlnow / (time(NULL) - curl_starttime); // To-do: Make this more accurate???
	return 0;
}

void CURLPrepareFile(const char* url, int dfilenum)
{
	HTTP_login *login;

#ifdef PARANOIA
	if (M_CheckParm("-nodownload"))
		I_Error("Attempted to download files in -nodownload mode");
#endif

	curl_global_init(CURL_GLOBAL_ALL);

	http_handle = curl_easy_init();
	multi_handle = curl_multi_init();

	if (http_handle && multi_handle)
	{
		I_mkdir(downloaddir, 0755);

		curl_curfile = &fileneeded[dfilenum];
		curl_realname = curl_curfile->filename;
		nameonly(curl_realname);

		curl_origfilesize = curl_curfile->currentsize;
		curl_origtotalfilesize = curl_curfile->totalsize;

		curl_easy_setopt(http_handle, CURLOPT_URL, va("%s/%s", url, curl_realname));

		// Only allow HTTP and HTTPS
		curl_easy_setopt(http_handle, CURLOPT_PROTOCOLS, CURLPROTO_HTTP|CURLPROTO_HTTPS);

		curl_easy_setopt(http_handle, CURLOPT_USERAGENT, va("SRB2Kart/v%d.%d", VERSION, SUBVERSION)); // Set user agent as some servers won't accept invalid user agents.

		// Authenticate if the user so wishes
		login = CURLGetLogin(url, NULL);

		if (login)
		{
			curl_easy_setopt(http_handle, CURLOPT_USERPWD, login->auth);
		}

		// Follow a redirect request, if sent by the server.
		curl_easy_setopt(http_handle, CURLOPT_FOLLOWLOCATION, 1L);

		curl_easy_setopt(http_handle, CURLOPT_FAILONERROR, 1L);

		CONS_Printf("Downloading %s from %s\n", curl_realname, url);

		strcatbf(curl_curfile->filename, downloaddir, "/");
		curl_curfile->file = fopen(curl_curfile->filename, "wb");
		curl_easy_setopt(http_handle, CURLOPT_WRITEDATA, curl_curfile->file);
		curl_easy_setopt(http_handle, CURLOPT_WRITEFUNCTION, curlwrite_data);
		curl_easy_setopt(http_handle, CURLOPT_NOPROGRESS, 0L);
		curl_easy_setopt(http_handle, CURLOPT_PROGRESSFUNCTION, curlprogress_callback);

		curl_curfile->status = FS_DOWNLOADING;
		lastfilenum = dfilenum;
		curl_multi_add_handle(multi_handle, http_handle);

		curl_multi_perform(multi_handle, &curl_runninghandles);
		curl_starttime = time(NULL);
		curl_running = true;
	}
}

void CURLGetFile(void)
{
	CURLMcode mc; /* return code used by curl_multi_wait() */
	CURLcode easyres; /* Return from easy interface */
	int numfds;
	CURLMsg *m; /* for picking up messages with the transfer status */
	CURL *e;
	int msgs_left; /* how many messages are left */
	const char *easy_handle_error;
	long response_code = 0;
	static char *filename;

    if (curl_runninghandles)
    {
    	curl_multi_perform(multi_handle, &curl_runninghandles);

		/* wait for activity, timeout or "nothing" */
		mc = curl_multi_wait(multi_handle, NULL, 0, 1000, &numfds);

		if (mc != CURLM_OK)
		{
			CONS_Alert(CONS_WARNING, "curl_multi_wait() failed, code %d.\n", mc);
			return;
		}
		curl_curfile->currentsize = curl_dlnow;
		curl_curfile->totalsize = curl_dltotal;
    }

    /* See how the transfers went */
	while ((m = curl_multi_info_read(multi_handle, &msgs_left)))
	{
		if (m && (m->msg == CURLMSG_DONE))
		{
			e = m->easy_handle;
			easyres = m->data.result;
			filename = Z_StrDup(curl_realname);
			nameonly(filename);
			if (easyres != CURLE_OK)
			{
				if (easyres == CURLE_HTTP_RETURNED_ERROR)
					curl_easy_getinfo(e, CURLINFO_RESPONSE_CODE, &response_code);

				easy_handle_error = (response_code) ? va("HTTP reponse code %ld", response_code) : curl_easy_strerror(easyres);
				curl_curfile->status = FS_FALLBACK;
				curl_curfile->currentsize = curl_origfilesize;
				curl_curfile->totalsize = curl_origtotalfilesize;
				curl_failedwebdownload = true;
				fclose(curl_curfile->file);
				remove(curl_curfile->filename);
				CONS_Printf(M_GetText("Failed to download %s (%s)\n"), filename, easy_handle_error);
			}
			else
			{
				fclose(curl_curfile->file);

				if (checkfilemd5(curl_curfile->filename, curl_curfile->md5sum) == FS_MD5SUMBAD)
				{
					CONS_Alert(CONS_ERROR, M_GetText("HTTP Download of %s finished but is corrupt or has been modified\n"), filename);
					curl_curfile->status = FS_FALLBACK;
					curl_failedwebdownload = true;
				}
				else
				{
					CONS_Printf(M_GetText("Finished HTTP download of %s\n"), filename);
					downloadcompletednum++;
					downloadcompletedsize += curl_curfile->totalsize;
					curl_curfile->status = FS_FOUND;
				}
			}


			Z_Free(filename);
			curl_curfile->file = NULL;
			curl_running = false;
			curl_transfers--;
			curl_multi_remove_handle(multi_handle, e);
			curl_easy_cleanup(e);

			if (!curl_transfers)
				break;
		}
	}

    if (!curl_transfers)
    {
		curl_multi_cleanup(multi_handle);
		curl_global_cleanup();
    }
}

HTTP_login *
CURLGetLogin (const char *url, HTTP_login ***return_prev_next)
{
	HTTP_login  * login;
	HTTP_login ** prev_next;

	for (
			prev_next = &curl_logins;
			( login = (*prev_next));
			prev_next = &login->next
	){
		if (strcmp(login->url, url) == 0)
		{
			if (return_prev_next)
				(*return_prev_next) = prev_next;

			return login;
		}
	}

	return NULL;
}
#endif

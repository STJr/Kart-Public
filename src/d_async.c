// SONIC ROBO BLAST 2 KART
//-----------------------------------------------------------------------------
// Copyright (C) 2020 by James R.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  d_async.c
/// \brief Add wad/exec config asynchronously to the main thread, but in order

#include "doomdef.h"
#include "d_main.h"
#include "d_async.h"
#include "filesrch.h"
#include "i_threads.h"
#include "m_misc.h"
#include "p_setup.h"
#include "z_zone.h"

struct filename_queue;
struct async_addfile;

typedef struct filename_queue filename_queue_t;
typedef struct async_addfile async_addfile_t;

struct filename_queue
{
	int                type;
	char             * name;
	void             * u;/* exec command has some parameters that need know */
	filename_queue_t * next;
};

struct async_addfile
{
	int               done;
	int               query_size;
	int               query_position;
	filequery_t     * query;
	int             * type;
	void           ** u;
	async_addfile_t * next;
};

static filename_queue_t * queue_head;
static filename_queue_t * queue_tail;
static int                queue_size;

static I_mutex            async_mutex;

static async_addfile_t  * async_head;
static async_addfile_t  * async_tail;

static void
Async_addfile_thread (async_addfile_t *file)
{
	findmultiplefiles(file->query_size, file->query, false, true, &async_mutex);

	I_lock_mutex(&async_mutex);
	{
		file->done = 1;
	}
	I_unlock_mutex(async_mutex);
}

static void
Do_addfile (int type, filequery_t *q, void *ctx)
{
	switch (type)
	{
		case ASYNC_EXEC:
			COM_ExecuteFile(q, ctx);
			break;
	}
}

void
Finish_async_addfile (void)
{
	async_addfile_t * file = async_head;
	async_addfile_t * next;

	filequery_t * q;

	filestatus_t status;

	int type;
	int done;

	while (file != NULL)
	{
		while (file->query_position < file->query_size)
		{
			q    = &file->query [file->query_position];
			type =  file->type  [file->query_position];

			I_lock_mutex(&async_mutex);
			{
				status = q->status;
			}
			I_unlock_mutex(async_mutex);

			if (status == FS_NOTCHECKED)
			{
				break;
			}
			else
			{
				Do_addfile(type, q, file->u[file->query_position]);
			}

			Z_Free(q->filename);

			file->query_position++;
		}

		if (file->query_position < file->query_size)
		{
			break;
		}

		I_lock_mutex(&async_mutex);
		{
			done = file->done;
		}
		I_unlock_mutex(async_mutex);

		if (done)
		{
			next = file->next;

			Z_Free(file->u);
			Z_Free(file->type);
			Z_Free(file->query);
			Z_Free(file);

			file = next;
		}
		else
		{
			break;
		}
	}

	async_head = file;

	if (file == NULL)
	{
		async_tail = NULL;
	}
}

void
Append_async_addfile (int type, const char * filename, void * userdata)
{
	char filenamebuf[MAX_WADPATH];

	filename_queue_t * q;

	filequery_t file;

	if (FIL_FileOK(va(pandf,srb2home,filename)))
	{
		file.status   = FS_FOUND;
		file.filename = filenamebuf;

		strcpy(file.filename, filename);

		Do_addfile(type, &file, userdata);
	}
	else
	{
		q = ZZ_Alloc(sizeof *q);

		if (queue_tail != NULL)
		{
			queue_tail->next = q;
		}
		else
		{
			queue_head = q;
		}

		queue_tail = q;
		queue_size++;

		q->type = type;
		q->name = strcpy(ZZ_Alloc(MAX_WADPATH), filename);
		q->u    = userdata;
		q->next = NULL;
	}
}

void
Detach_async_addfile (void)
{
	filename_queue_t * q = queue_head;
	filename_queue_t * next;

	async_addfile_t  * file;

	filequery_t      * query;
	int              * type;
	void            ** udata;

	if (q != NULL)
	{
		file = ZZ_Alloc(sizeof *file);

		if (async_tail != NULL)
		{
			async_tail->next = file;
		}
		else
		{
			async_head = file;
		}

		async_tail = file;

		file->done = 0;
		file->next = NULL;

		file->query_size     = queue_size;
		file->query_position = 0;

		file->query = ZZ_Alloc(queue_size * sizeof (*file->query));
		file->type  = ZZ_Alloc(queue_size * sizeof (*file->type));
		file->u     = ZZ_Alloc(queue_size * sizeof (*file->u));

		query = file->query;
		type  = file->type;
		udata = file->u;

		do
		{
			*type++  = q->type;
			*udata++ = q->u;

			query->status       = FS_NOTCHECKED;
			query->wantedmd5sum = NULL;
			query->filename     = q->name;
			query++;

			next = q->next;

			Z_Free(q);
		}
		while (( q = next ) != NULL) ;

		I_spawn_thread("async-addfile",
				(I_thread_fn)Async_addfile_thread, file);

		queue_head = NULL;
		queue_tail = NULL;
		queue_size = 0;
	}
}

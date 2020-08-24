// SONIC ROBO BLAST 2 KART
//-----------------------------------------------------------------------------
// Copyright (C) 2020 by James R.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  d_async.h
/// \brief Add wad/exec config asynchronously to the main thread, but in order

#ifndef D_ASYNC_H
#define D_ASYNC_H

enum
{
	ASYNC_ADDFILE,
	ASYNC_EXEC,
};

/*
Queue a file to be added. Detach_async_addfile will spawn a thread that
searches for any files if necessary.
*/

void Append_async_addfile (int type, const char * filename, void * ctx);

/*
Although the file searching is done on another thread, the actual adding of the
wad or executing the config must be done on the main thread. This function is
called every tic to check if a file was found so it can perform that last step.
It does this in order.
*/

void Finish_async_addfile (void);

/*
Spawns a thread that searches for files that were queued.
Called at the end of each tic.
*/

void Detach_async_addfile (void);

#endif/*D_ASYNC_H*/

// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2000 by DooM Legacy Team.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//-----------------------------------------------------------------------------
/// \file
/// \brief Main program, simply calls D_SRB2Main and D_SRB2Loop, the high level loop.

#include "../doomdef.h"
#include "../m_argv.h"
#include "../d_main.h"
#include "../i_system.h"

#ifdef __GNUC__
#include <unistd.h>
#endif




#ifdef HAVE_TTF
#include "SDL.h"
#include "i_ttf.h"
#endif

#ifdef SDLMAIN
#include "SDL_main.h"
#elif defined(FORCESDLMAIN)
extern int SDL_main(int argc, char *argv[]);
#endif

#ifdef LOGMESSAGES
FILE *logstream = NULL;
#endif

#ifndef DOXYGEN
#ifndef O_TEXT
#define O_TEXT 0
#endif

#ifndef O_SEQUENTIAL
#define O_SEQUENTIAL 0
#endif
#endif





/**	\brief	The main function

	\param	argc	number of arg
	\param	*argv	string table

	\return	int
*/
FUNCNORETURN
#ifdef FORCESDLMAIN
int SDL_main(int argc, char **argv)
#else
int main(int argc, char **argv)
#endif
{
	const char *logdir = NULL;
	myargc = argc;
	myargv = argv; /// \todo pull out path to exe from this string

#ifdef HAVE_TTF
	I_StartupTTF(FONTPOINTSIZE, SDL_INIT_VIDEO, SDL_SWSURFACE);
#endif


// init Wii-specific stuff

	logdir = D_Home();

#ifdef LOGMESSAGES
#if defined(_WIN32_WCE) || defined(GP2X)
	logstream = fopen(va("%s.log",argv[0]), "a");
#elif defined (DEFAULTDIR)
	if (logdir)
		logstream = fopen(va("%s/"DEFAULTDIR"/srb2log.txt",logdir), "a");
	else
#endif
		logstream = fopen("./srb2log.txt", "a");
#endif

	//I_OutputMsg("I_StartupSystem() ...\n");
	I_StartupSystem();
	// startup SRB2
	CONS_Printf("%s", M_GetText("Setting up SRB2...\n"));
	D_SRB2Main();
	CONS_Printf("%s", M_GetText("Entering main game loop...\n"));
	// never return
	D_SRB2Loop();

#ifdef BUGTRAP
	// This is safe even if BT didn't start.
	ShutdownBugTrap();
#endif

	// return to OS
#ifndef __GNUC__
	return 0;
#endif
}

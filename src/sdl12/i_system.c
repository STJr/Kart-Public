// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Portions Copyright (C) 1998-2000 by DooM Legacy Team.
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
//
// Changes by Graue <graue@oceanbase.org> are in the public domain.
//
//-----------------------------------------------------------------------------
/// \file
/// \brief SRB2 system stuff for SDL

#include <signal.h>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __GNUC__
#include <unistd.h>
#elif defined (_MSC_VER)
#include <direct.h>
#endif
#if defined (__unix__) || defined (UNIXCOMMON)
#include <fcntl.h>
#endif

#include <stdio.h>

#ifdef _MSC_VER
#pragma warning(disable : 4214 4244)
#endif


#include "SDL.h"

#ifdef HAVE_TTF
#include "i_ttf.h"
#endif

#ifdef _MSC_VER
#pragma warning(default : 4214 4244)
#endif

#if SDL_VERSION_ATLEAST(1,2,7) && !defined (DC)
#include "SDL_cpuinfo.h" // 1.2.7 or greater
#define HAVE_SDLCPUINFO
#endif

#if defined (__unix__) || defined(__APPLE__) || (defined (UNIXCOMMON) && !defined (_arch_dreamcast) && !defined (__HAIKU__) && !defined (_WII))
#if defined (__linux__)
#include <sys/vfs.h>
#else
#include <sys/param.h>
#include <sys/mount.h>
/*For meminfo*/
#include <sys/types.h>
#ifdef FREEBSD
#include <kvm.h>
#endif
#include <nlist.h>
#include <sys/vmmeter.h>
#endif
#endif

#if defined (__linux__) || (defined (UNIXCOMMON) && !defined (_arch_dreamcast) && !defined (_PSP) && !defined (__HAIKU__) && !defined (_WII))
#ifndef NOTERMIOS
#include <termios.h>
#include <sys/ioctl.h> // ioctl
#define HAVE_TERMIOS
#endif
#endif

#ifndef NOMUMBLE
#if defined (__linux__) && !defined(_PS3) // need -lrt
#include <sys/mman.h>
#ifdef MAP_FAILED
#define HAVE_SHM
#endif
#include <wchar.h>
#endif

#if   defined (HAVE_SHM)
#define HAVE_MUMBLE
#endif
#endif // NOMUMBLE


#ifndef O_BINARY
#define O_BINARY 0
#endif

// Locations for searching the srb2.srb
#if   defined (GP2X)
#define DEFAULTWADLOCATION1 "/mnt/sd"
#define DEFAULTWADLOCATION2 "/mnt/sd/SRB2Kart"
#define DEFAULTWADLOCATION3 "/tmp/mnt/sd"
#define DEFAULTWADLOCATION4 "/tmp/mnt/sd/SRB2Kart"
#define DEFAULTSEARCHPATH1 "/mnt/sd"
#define DEFAULTSEARCHPATH2 "/tmp/mnt/sd"
#elif defined (__unix__) || defined(__APPLE__) || defined (UNIXCOMMON)
#define DEFAULTWADLOCATION1 "/usr/local/share/games/SRB2Kart"
#define DEFAULTWADLOCATION2 "/usr/local/games/SRB2Kart"
#define DEFAULTWADLOCATION3 "/usr/share/games/SRB2Kart"
#define DEFAULTWADLOCATION4 "/usr/games/SRB2Kart"
#define DEFAULTSEARCHPATH1 "/usr/local/games"
#define DEFAULTSEARCHPATH2 "/usr/games"
#define DEFAULTSEARCHPATH3 "/usr/local"
#endif

/**	\brief WAD file to look for
*/
#define WADKEYWORD1 "srb2.srb"
#define WADKEYWORD2 "srb2.wad"
/**	\brief holds wad path
*/
static char returnWadPath[256];

//Alam_GBC: SDL

#include "../doomdef.h"
#include "../m_misc.h"
#include "../i_video.h"
#include "../i_sound.h"
#include "../i_system.h"
#include "../screen.h" //vid.WndParent
#include "../d_net.h"
#include "../g_game.h"
#include "../filesrch.h"
#include "endtxt.h"
#include "sdlmain.h"

#include "../i_joy.h"

#include "../m_argv.h"

#ifdef MAC_ALERT
#include "macosx/mac_alert.h"
#endif

#include "../d_main.h"

#if !defined(NOMUMBLE) && defined(HAVE_MUMBLE)
// Mumble context string
#include "../d_clisrv.h"
#include "../byteptr.h"
#endif

/**	\brief	The JoyReset function

	\param	JoySet	Joystick info to reset

	\return	void
*/
static void JoyReset(SDLJoyInfo_t *JoySet)
{
	if (JoySet->dev)
	{
#ifdef GP2X //GP2X's SDL does an illegal free on the 1st joystick...
		if (SDL_JoystickIndex(JoySet->dev) != 0)
#endif
		SDL_JoystickClose(JoySet->dev);
	}
	JoySet->dev = NULL;
	JoySet->oldjoy = -1;
	JoySet->axises = JoySet->buttons = JoySet->hats = JoySet->balls = 0;
	//JoySet->scale
}

/**	\brief First joystick up and running
*/
static INT32 joystick_started  = 0;

/**	\brief SDL info about joystick 1
*/
SDLJoyInfo_t JoyInfo;


/**	\brief Second joystick up and running
*/
static INT32 joystick2_started = 0;

/**	\brief SDL inof about joystick 2
*/
SDLJoyInfo_t JoyInfo2;


/**	\brief Third joystick up and running
*/
static INT32 joystick3_started = 0;

/**	\brief SDL inof about joystick 3
*/
SDLJoyInfo_t JoyInfo3;


/**	\brief Fourth joystick up and running
*/
static INT32 joystick4_started = 0;

/**	\brief SDL inof about joystick 4
*/
SDLJoyInfo_t JoyInfo4;


#ifdef HAVE_TERMIOS
static INT32 fdmouse2 = -1;
static INT32 mouse2_started = 0;
#endif

SDL_bool consolevent = SDL_FALSE;
SDL_bool framebuffer = SDL_FALSE;

UINT8 keyboard_started = false;

#if 0
static void signal_handler(INT32 num)
{
	//static char msg[] = "oh no! back to reality!\r\n";
	char *      sigmsg;
	char        sigdef[32];

	switch (num)
	{
	case SIGINT:
		sigmsg = "interrupt";
		break;
	case SIGILL:
		sigmsg = "illegal instruction - invalid function image";
		break;
	case SIGFPE:
		sigmsg = "floating point exception";
		break;
	case SIGSEGV:
		sigmsg = "segment violation";
		break;
	case SIGTERM:
		sigmsg = "Software termination signal from kill";
		break;
#if !(defined (__unix_) || defined (UNIXCOMMON))
	case SIGBREAK:
		sigmsg = "Ctrl-Break sequence";
		break;
#endif
	case SIGABRT:
		sigmsg = "abnormal termination triggered by abort call";
		break;
	default:
		sprintf(sigdef,"signal number %d", num);
		sigmsg = sigdef;
	}

	I_OutputMsg("signal_handler() error: %s\n", sigmsg);
	signal(num, SIG_DFL);               //default signal action
	raise(num);
	I_Quit();
}
#endif

#if defined (NDEBUG) && !defined (DC) && !defined (_WIN32_WCE)
FUNCNORETURN static ATTRNORETURN void quit_handler(int num)
{
	signal(num, SIG_DFL); //default signal action
	raise(num);
	I_Quit();
}
#endif

#ifdef HAVE_TERMIOS
// TERMIOS console code from Quake3: thank you!
SDL_bool stdin_active = SDL_TRUE;

typedef struct
{
	size_t cursor;
	char buffer[256];
} feild_t;

feild_t tty_con;

// when printing general stuff to stdout stderr (Sys_Printf)
//   we need to disable the tty console stuff
// this increments so we can recursively disable
static INT32 ttycon_hide = 0;
// some key codes that the terminal may be using
// TTimo NOTE: I'm not sure how relevant this is
static INT32 tty_erase;
static INT32 tty_eof;

static struct termios tty_tc;

// =============================================================
// tty console routines
// NOTE: if the user is editing a line when something gets printed to the early console then it won't look good
//   so we provide tty_Clear and tty_Show to be called before and after a stdout or stderr output
// =============================================================

// flush stdin, I suspect some terminals are sending a LOT of garbage
// FIXME TTimo relevant?
#if 0
static inline void tty_FlushIn(void)
{
	char key;
	while (read(STDIN_FILENO, &key, 1)!=-1);
}
#endif

// do a backspace
// TTimo NOTE: it seems on some terminals just sending '\b' is not enough
//   so for now, in any case we send "\b \b" .. yeah well ..
//   (there may be a way to find out if '\b' alone would work though)
static void tty_Back(void)
{
	char key;
	ssize_t d;
	key = '\b';
	d = write(STDOUT_FILENO, &key, 1);
	key = ' ';
	d = write(STDOUT_FILENO, &key, 1);
	key = '\b';
	d = write(STDOUT_FILENO, &key, 1);
	(void)d;
}

static void tty_Clear(void)
{
	size_t i;
	if (tty_con.cursor>0)
	{
		for (i=0; i<tty_con.cursor; i++)
		{
			tty_Back();
		}
	}

}

// clear the display of the line currently edited
// bring cursor back to beginning of line
static inline void tty_Hide(void)
{
	//I_Assert(consolevent);
	if (ttycon_hide)
	{
		ttycon_hide++;
		return;
	}
	tty_Clear();
	ttycon_hide++;
}

// show the current line
// FIXME TTimo need to position the cursor if needed??
static inline void tty_Show(void)
{
	size_t i;
	ssize_t d;
	//I_Assert(consolevent);
	I_Assert(ttycon_hide>0);
	ttycon_hide--;
	if (ttycon_hide == 0 && tty_con.cursor)
	{
		for (i=0; i<tty_con.cursor; i++)
		{
			d = write(STDOUT_FILENO, tty_con.buffer+i, 1);
		}
	}
	(void)d;
}

// never exit without calling this, or your terminal will be left in a pretty bad state
static void I_ShutdownConsole(void)
{
	if (consolevent)
	{
		I_OutputMsg("Shutdown tty console\n");
		consolevent = SDL_FALSE;
		tcsetattr (STDIN_FILENO, TCSADRAIN, &tty_tc);
	}
}

static void I_StartupConsole(void)
{
	struct termios tc;

	// TTimo
	// https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=390 (404)
	// then SIGTTIN or SIGTOU is emitted, if not catched, turns into a SIGSTP
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);

#if !defined(GP2X) //read is bad on GP2X
	consolevent = !M_CheckParm("-noconsole");
#endif
	framebuffer = M_CheckParm("-framebuffer");

	if (framebuffer)
		consolevent = SDL_FALSE;

	if (!consolevent) return;

	if (isatty(STDIN_FILENO)!=1)
	{
		I_OutputMsg("stdin is not a tty, tty console mode failed\n");
		consolevent = SDL_FALSE;
		return;
	}
	memset(&tty_con, 0x00, sizeof(tty_con));
	tcgetattr (0, &tty_tc);
	tty_erase = tty_tc.c_cc[VERASE];
	tty_eof = tty_tc.c_cc[VEOF];
	tc = tty_tc;
	/*
	 ECHO: don't echo input characters
	 ICANON: enable canonical mode.  This  enables  the  special
	  characters  EOF,  EOL,  EOL2, ERASE, KILL, REPRINT,
	  STATUS, and WERASE, and buffers by lines.
	 ISIG: when any of the characters  INTR,  QUIT,  SUSP,  or
	  DSUSP are received, generate the corresponding signal
	*/
	tc.c_lflag &= ~(ECHO | ICANON);
	/*
	 ISTRIP strip off bit 8
	 INPCK enable input parity checking
	 */
	tc.c_iflag &= ~(ISTRIP | INPCK);
	tc.c_cc[VMIN] = 0; //1?
	tc.c_cc[VTIME] = 0;
	tcsetattr (0, TCSADRAIN, &tc);
}

void I_GetConsoleEvents(void)
{
	// we use this when sending back commands
	event_t ev = {0,0,0,0};
	char key = 0;
	ssize_t d;

	if (!consolevent)
		return;

	ev.type = ev_console;
	if (read(STDIN_FILENO, &key, 1) == -1 || !key)
		return;

	// we have something
	// backspace?
	// NOTE TTimo testing a lot of values .. seems it's the only way to get it to work everywhere
	if ((key == tty_erase) || (key == 127) || (key == 8))
	{
		if (tty_con.cursor > 0)
		{
			tty_con.cursor--;
			tty_con.buffer[tty_con.cursor] = '\0';
			tty_Back();
		}
		ev.data1 = KEY_BACKSPACE;
	}
	else if (key < ' ') // check if this is a control char
	{
		if (key == '\n')
		{
			tty_Clear();
			tty_con.cursor = 0;
			ev.data1 = KEY_ENTER;
		}
		else return;
	}
	else
	{
		// push regular character
		ev.data1 = tty_con.buffer[tty_con.cursor] = key;
		tty_con.cursor++;
		// print the current line (this is differential)
		d = write(STDOUT_FILENO, &key, 1);
	}
	if (ev.data1) D_PostEvent(&ev);
	//tty_FlushIn();
	(void)d;
}

#else
void I_GetConsoleEvents(void){}
static inline void I_StartupConsole(void)
{
#ifdef _DEBUG
	consolevent = !M_CheckParm("-noconsole");
#else
	consolevent = M_CheckParm("-console");
#endif

	framebuffer = M_CheckParm("-framebuffer");

	if (framebuffer)
		consolevent = SDL_FALSE;
}
static inline void I_ShutdownConsole(void){}
#endif

//
// StartupKeyboard
//
void I_StartupKeyboard (void)
{
#if defined (NDEBUG) && !defined (DC)
#ifdef SIGILL
//	signal(SIGILL , signal_handler);
#endif
#ifdef SIGINT
	signal(SIGINT , quit_handler);
#endif
#ifdef SIGSEGV
//	signal(SIGSEGV , signal_handler);
#endif
#ifdef SIGBREAK
	signal(SIGBREAK , quit_handler);
#endif
#ifdef SIGABRT
//	signal(SIGABRT , signal_handler);
#endif
#ifdef SIGTERM
	signal(SIGTERM , quit_handler);
#endif
#endif
}

//
//I_OutputMsg
//
void I_OutputMsg(const char *fmt, ...)
{
	size_t len;
	XBOXSTATIC char txt[8192];
	va_list  argptr;


	va_start(argptr,fmt);
	vsprintf(txt, fmt, argptr);
	va_end(argptr);

#ifdef HAVE_TTF
	if (TTF_WasInit()) I_TTFDrawText(currentfont, solid, DEFAULTFONTFGR, DEFAULTFONTFGG, DEFAULTFONTFGB,  DEFAULTFONTFGA,
	DEFAULTFONTBGR, DEFAULTFONTBGG, DEFAULTFONTBGB, DEFAULTFONTBGA, txt);
#endif


	len = strlen(txt);

#ifdef LOGMESSAGES
	if (logstream)
	{
		size_t d = fwrite(txt, len, 1, logstream);
		fflush(logstream);
		(void)d;
	}
#endif

#ifdef HAVE_TERMIOS
	if (consolevent)
	{
		tty_Hide();
	}
#endif

	if (!framebuffer)
		fprintf(stderr, "%s", txt);
#ifdef HAVE_TERMIOS
	if (consolevent)
	{
		tty_Show();
	}
#endif

	// 2004-03-03 AJR Since not all messages end in newline, some were getting displayed late.
	if (!framebuffer)
		fflush(stderr);

}

//
// I_GetKey
//
INT32 I_GetKey (void)
{
	// Warning: I_GetKey empties the event queue till next keypress
	event_t *ev;
	INT32 rc = 0;

	// return the first keypress from the event queue
	for (; eventtail != eventhead; eventtail = (eventtail+1)&(MAXEVENTS-1))
	{
		ev = &events[eventtail];
		if (ev->type == ev_keydown || ev->type == ev_console)
		{
			rc = ev->data1;
			continue;
		}
	}

	return rc;
}

//
// I_JoyScale
//
void I_JoyScale(void)
{
#ifdef GP2X
	if (JoyInfo.dev && SDL_JoystickIndex(JoyInfo.dev) == 0)
		Joystick.bGamepadStyle = true;
	else
#endif
	Joystick.bGamepadStyle = cv_joyscale.value==0;
	JoyInfo.scale = Joystick.bGamepadStyle?1:cv_joyscale.value;
}

void I_JoyScale2(void)
{
#ifdef GP2X
	if (JoyInfo2.dev && SDL_JoystickIndex(JoyInfo2.dev) == 0)
		Joystick.bGamepadStyle = true;
	else
#endif
	Joystick2.bGamepadStyle = cv_joyscale2.value==0;
	JoyInfo2.scale = Joystick2.bGamepadStyle?1:cv_joyscale2.value;
}

void I_JoyScale3(void)
{
#ifdef GP2X
	if (JoyInfo3.dev && SDL_JoystickIndex(JoyInfo3.dev) == 0)
		Joystick.bGamepadStyle = true;
	else
#endif
	Joystick3.bGamepadStyle = cv_joyscale3.value==0;
	JoyInfo3.scale = Joystick3.bGamepadStyle?1:cv_joyscale3.value;
}

void I_JoyScale4(void)
{
#ifdef GP2X
	if (JoyInfo4.dev && SDL_JoystickIndex(JoyInfo4.dev) == 0)
		Joystick.bGamepadStyle = true;
	else
#endif
	Joystick4.bGamepadStyle = cv_joyscale4.value==0;
	JoyInfo4.scale = Joystick4.bGamepadStyle?1:cv_joyscale4.value;
}

/**	\brief Joystick 1 buttons states
*/
static UINT64 lastjoybuttons = 0;

/**	\brief Joystick 1 hats state
*/
static UINT64 lastjoyhats = 0;

/**	\brief	Shuts down joystick 1


	\return void


*/
static void I_ShutdownJoystick(void)
{
	INT32 i;
	event_t event;
	event.type=ev_keyup;
	event.data2 = 0;
	event.data3 = 0;

	lastjoybuttons = lastjoyhats = 0;

	// emulate the up of all joystick buttons
	for (i=0;i<JOYBUTTONS;i++)
	{
		event.data1=KEY_JOY1+i;
		D_PostEvent(&event);
	}

	// emulate the up of all joystick hats
	for (i=0;i<JOYHATS*4;i++)
	{
		event.data1=KEY_HAT1+i;
		D_PostEvent(&event);
	}

	// reset joystick position
	event.type = ev_joystick;
	for (i=0;i<JOYAXISSET; i++)
	{
		event.data1 = i;
		D_PostEvent(&event);
	}

	joystick_started = 0;
	JoyReset(&JoyInfo);
	if (!joystick_started && !joystick2_started && !joystick3_started && !joystick4_started
		&& SDL_WasInit(SDL_INIT_JOYSTICK) == SDL_INIT_JOYSTICK)
	{
		SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
		if (cv_usejoystick.value == 0)
		{
			I_OutputMsg("I_Joystick: SDL's Joystick system has been shutdown\n");
		}
	}
}

void I_GetJoystickEvents(void)
{
	static event_t event = {0,0,0,0};
	INT32 i = 0;
	UINT64 joyhats = 0;
#if 0
	UINT64 joybuttons = 0;
	Sint16 axisx, axisy;
#endif

	if (!joystick_started) return;

	if (!JoyInfo.dev) //I_ShutdownJoystick();
		return;

#if 0
	//faB: look for as much buttons as g_input code supports,
	//  we don't use the others
	for (i = JoyInfo.buttons - 1; i >= 0; i--)
	{
		joybuttons <<= 1;
		if (SDL_JoystickGetButton(JoyInfo.dev,i))
			joybuttons |= 1;
	}

	if (joybuttons != lastjoybuttons)
	{
		INT64 j = 1; // keep only bits that changed since last time
		INT64 newbuttons = joybuttons ^ lastjoybuttons;
		lastjoybuttons = joybuttons;

		for (i = 0; i < JOYBUTTONS; i++, j <<= 1)
		{
			if (newbuttons & j) // button changed state?
			{
				if (joybuttons & j)
					event.type = ev_keydown;
				else
					event.type = ev_keyup;
				event.data1 = KEY_JOY1 + i;
				D_PostEvent(&event);
			}
		}
	}
#endif

	for (i = JoyInfo.hats - 1; i >= 0; i--)
	{
		Uint8 hat = SDL_JoystickGetHat(JoyInfo.dev, i);

		if (hat & SDL_HAT_UP   ) joyhats|=(UINT64)0x1<<(0 + 4*i);
		if (hat & SDL_HAT_DOWN ) joyhats|=(UINT64)0x1<<(1 + 4*i);
		if (hat & SDL_HAT_LEFT ) joyhats|=(UINT64)0x1<<(2 + 4*i);
		if (hat & SDL_HAT_RIGHT) joyhats|=(UINT64)0x1<<(3 + 4*i);
	}

	if (joyhats != lastjoyhats)
	{
		INT64 j = 1; // keep only bits that changed since last time
		INT64 newhats = joyhats ^ lastjoyhats;
		lastjoyhats = joyhats;

		for (i = 0; i < JOYHATS*4; i++, j <<= 1)
		{
			if (newhats & j) // hat changed state?
			{
				if (joyhats & j)
					event.type = ev_keydown;
				else
					event.type = ev_keyup;
				event.data1 = KEY_HAT1 + i;
				D_PostEvent(&event);
			}
		}
	}

#if 0
	// send joystick axis positions
	event.type = ev_joystick;

	for (i = JOYAXISSET - 1; i >= 0; i--)
	{
		event.data1 = i;
		if (i*2 + 1 <= JoyInfo.axises)
			axisx = SDL_JoystickGetAxis(JoyInfo.dev, i*2 + 0);
		else axisx = 0;
		if (i*2 + 2 <= JoyInfo.axises)
			axisy = SDL_JoystickGetAxis(JoyInfo.dev, i*2 + 1);
		else axisy = 0;

		axisx = axisx/32;
		axisy = axisy/32;

		if (Joystick.bGamepadStyle)
		{
			// gamepad control type, on or off, live or die
			if (axisx < -(JOYAXISRANGE/2))
				event.data2 = -1;
			else if (axisx > (JOYAXISRANGE/2))
				event.data2 = 1;
			else event.data2 = 0;
			if (axisy < -(JOYAXISRANGE/2))
				event.data3 = -1;
			else if (axisy > (JOYAXISRANGE/2))
				event.data3 = 1;
			else event.data3 = 0;
		}
		else
		{

			axisx = JoyInfo.scale?((axisx/JoyInfo.scale)*JoyInfo.scale):axisx;
			axisy = JoyInfo.scale?((axisy/JoyInfo.scale)*JoyInfo.scale):axisy;

#ifdef SDL_JDEADZONE
			if (-SDL_JDEADZONE <= axisx && axisx <= SDL_JDEADZONE) axisx = 0;
			if (-SDL_JDEADZONE <= axisy && axisy <= SDL_JDEADZONE) axisy = 0;
#endif

			// analog control style , just send the raw data
			event.data2 = axisx; // x axis
			event.data3 = axisy; // y axis
		}
		D_PostEvent(&event);
	}
#endif
}

/**	\brief	Open joystick handle

	\param	fname	name of joystick

	\return	axises


*/
static int joy_open(const char *fname)
{
	int joyindex = atoi(fname);
	int num_joy = 0;
	int i;

	if (joystick_started == 0 && joystick2_started == 0 && joystick3_started == 0 && joystick4_started == 0)
	{
		if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) == -1)
		{
			CONS_Printf(M_GetText("Couldn't initialize joystick: %s\n"), SDL_GetError());
			return -1;
		}
		else
		{
			num_joy = SDL_NumJoysticks();
		}

		if (num_joy < joyindex)
		{
			CONS_Printf(M_GetText("Cannot use joystick #%d/(%s), it doesn't exist\n"),joyindex,fname);
			for (i = 0; i < num_joy; i++)
				CONS_Printf("#%d/(%s)\n", i+1, SDL_JoystickName(i));
			I_ShutdownJoystick();
			return -1;
		}
	}
	else
	{
		JoyReset(&JoyInfo);
		//I_ShutdownJoystick();
		//joy_open(fname);
	}

	num_joy = SDL_NumJoysticks();

	if (joyindex <= 0 || num_joy == 0 || JoyInfo.oldjoy == joyindex)
	{
//		I_OutputMsg("Unable to use that joystick #(%s), non-number\n",fname);
		if (num_joy != 0)
		{
			CONS_Printf(M_GetText("Found %d joysticks on this system\n"), num_joy);
			for (i = 0; i < num_joy; i++)
				CONS_Printf("#%d/(%s)\n", i+1, SDL_JoystickName(i));
		}
		else
			CONS_Printf("%s", M_GetText("Found no joysticks on this system\n"));
		if (joyindex <= 0 || num_joy == 0) return 0;
	}

	JoyInfo.dev = SDL_JoystickOpen(joyindex-1);
	CONS_Printf(M_GetText("Joystick: %s\n"), SDL_JoystickName(joyindex-1));

	if (JoyInfo.dev == NULL)
	{
		CONS_Printf(M_GetText("Couldn't open joystick: %s\n"), SDL_GetError());
		I_ShutdownJoystick();
		return -1;
	}
	else
	{
		JoyInfo.axises = SDL_JoystickNumAxes(JoyInfo.dev);
		if (JoyInfo.axises > JOYAXISSET*2)
			JoyInfo.axises = JOYAXISSET*2;
/*		if (joyaxes<2)
		{
			I_OutputMsg("Not enought axes?\n");
			I_ShutdownJoystick();
			return 0;
		}*/

		JoyInfo.buttons = SDL_JoystickNumButtons(JoyInfo.dev);
		if (JoyInfo.buttons > JOYBUTTONS)
			JoyInfo.buttons = JOYBUTTONS;

		JoyInfo.hats = SDL_JoystickNumHats(JoyInfo.dev);
		if (JoyInfo.hats > JOYHATS)
			JoyInfo.hats = JOYHATS;

		JoyInfo.balls = SDL_JoystickNumBalls(JoyInfo.dev);

		//Joystick.bGamepadStyle = !stricmp(SDL_JoystickName(SDL_JoystickIndex(JoyInfo.dev)), "pad");

		return JoyInfo.axises;
	}
}

//Joystick2

/**	\brief Joystick 2 buttons states
*/
static UINT64 lastjoy2buttons = 0;

/**	\brief Joystick 2 hats state
*/
static UINT64 lastjoy2hats = 0;

/**	\brief	Shuts down joystick 2


	\return	void
*/
static void I_ShutdownJoystick2(void)
{
	INT32 i;
	event_t event;
	event.type = ev_keyup;
	event.data2 = 0;
	event.data3 = 0;

	lastjoy2buttons = lastjoy2hats = 0;

	// emulate the up of all joystick buttons
	for (i = 0; i < JOYBUTTONS; i++)
	{
		event.data1 = KEY_2JOY1 + i;
		D_PostEvent(&event);
	}

	// emulate the up of all joystick hats
	for (i = 0; i < JOYHATS*4; i++)
	{
		event.data1 = KEY_2HAT1 + i;
		D_PostEvent(&event);
	}

	// reset joystick position
	event.type = ev_joystick2;
	for (i = 0; i < JOYAXISSET; i++)
	{
		event.data1 = i;
		D_PostEvent(&event);
	}

	JoyReset(&JoyInfo2);
	if (!joystick_started && !joystick2_started && !joystick3_started && !joystick4_started
		&& SDL_WasInit(SDL_INIT_JOYSTICK) == SDL_INIT_JOYSTICK)
	{
		SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
		if (cv_usejoystick2.value == 0)
		{
			DEBFILE("I_Joystick2: SDL's Joystick system has been shutdown\n");
		}
	}
}

void I_GetJoystick2Events(void)
{
	static event_t event = {0,0,0,0};
	INT32 i = 0;
	UINT64 joyhats = 0;
#if 0
	INT64 joybuttons = 0;
	INT32 axisx, axisy;
#endif

	if (!joystick2_started)
		return;

	if (!JoyInfo2.dev) //I_ShutdownJoystick2();
		return;


#if 0
	//faB: look for as much buttons as g_input code supports,
	//  we don't use the others
	for (i = JoyInfo2.buttons - 1; i >= 0; i--)
	{
		joybuttons <<= 1;
		if (SDL_JoystickGetButton(JoyInfo2.dev,i))
			joybuttons |= 1;
	}

	if (joybuttons != lastjoy2buttons)
	{
		INT64 j = 1; // keep only bits that changed since last time
		INT64 newbuttons = joybuttons ^ lastjoy2buttons;
		lastjoy2buttons = joybuttons;

		for (i = 0; i < JOYBUTTONS; i++, j <<= 1)
		{
			if (newbuttons & j) // button changed state?
			{
				if (joybuttons & j)
					event.type = ev_keydown;
				else
					event.type = ev_keyup;
				event.data1 = KEY_2JOY1 + i;
				D_PostEvent(&event);
			}
		}
	}
#endif

	for (i = JoyInfo2.hats - 1; i >= 0; i--)
	{
		Uint8 hat = SDL_JoystickGetHat(JoyInfo2.dev, i);

		if (hat & SDL_HAT_UP   ) joyhats|=(UINT64)0x1<<(0 + 4*i);
		if (hat & SDL_HAT_DOWN ) joyhats|=(UINT64)0x1<<(1 + 4*i);
		if (hat & SDL_HAT_LEFT ) joyhats|=(UINT64)0x1<<(2 + 4*i);
		if (hat & SDL_HAT_RIGHT) joyhats|=(UINT64)0x1<<(3 + 4*i);
	}

	if (joyhats != lastjoy2hats)
	{
		INT64 j = 1; // keep only bits that changed since last time
		INT64 newhats = joyhats ^ lastjoy2hats;
		lastjoy2hats = joyhats;

		for (i = 0; i < JOYHATS*4; i++, j <<= 1)
		{
			if (newhats & j) // hat changed state?
			{
				if (joyhats & j)
					event.type = ev_keydown;
				else
					event.type = ev_keyup;
				event.data1 = KEY_2HAT1 + i;
				D_PostEvent(&event);
			}
		}
	}

#if 0
	// send joystick axis positions
	event.type = ev_joystick2;

	for (i = JOYAXISSET - 1; i >= 0; i--)
	{
		event.data1 = i;
		if (i*2 + 1 <= JoyInfo2.axises)
			axisx = SDL_JoystickGetAxis(JoyInfo2.dev, i*2 + 0);
		else axisx = 0;
		if (i*2 + 2 <= JoyInfo2.axises)
			axisy = SDL_JoystickGetAxis(JoyInfo2.dev, i*2 + 1);
		else axisy = 0;

		axisx = axisx/32;
		axisy = axisy/32;

		if (Joystick2.bGamepadStyle)
		{
			// gamepad control type, on or off, live or die
			if (axisx < -(JOYAXISRANGE/2))
				event.data2 = -1;
			else if (axisx > (JOYAXISRANGE/2))
				event.data2 = 1;
			else
				event.data2 = 0;
			if (axisy < -(JOYAXISRANGE/2))
				event.data3 = -1;
			else if (axisy > (JOYAXISRANGE/2))
				event.data3 = 1;
			else
				event.data3 = 0;
		}
		else
		{

			axisx = JoyInfo2.scale?((axisx/JoyInfo2.scale)*JoyInfo2.scale):axisx;
			axisy = JoyInfo2.scale?((axisy/JoyInfo2.scale)*JoyInfo2.scale):axisy;

#ifdef SDL_JDEADZONE
			if (-SDL_JDEADZONE <= axisx && axisx <= SDL_JDEADZONE) axisx = 0;
			if (-SDL_JDEADZONE <= axisy && axisy <= SDL_JDEADZONE) axisy = 0;
#endif

			// analog control style , just send the raw data
			event.data2 = axisx; // x axis
			event.data3 = axisy; // y axis
		}
		D_PostEvent(&event);
	}
#endif

}

/**	\brief	Open joystick handle

	\param	fname	name of joystick

	\return	axises


*/
static int joy_open2(const char *fname)
{
	int joyindex = atoi(fname);
	int num_joy = 0;
	int i;

	if (joystick_started == 0 && joystick2_started == 0 && joystick3_started == 0 && joystick4_started == 0)
	{
		if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) == -1)
		{
			CONS_Printf(M_GetText("Couldn't initialize joystick: %s\n"), SDL_GetError());
			return -1;
		}
		else
			num_joy = SDL_NumJoysticks();

		if (num_joy < joyindex)
		{
			CONS_Printf(M_GetText("Cannot use joystick #%d/(%s), it doesn't exist\n"),joyindex,fname);
			for (i = 0; i < num_joy; i++)
				CONS_Printf("#%d/(%s)\n", i+1, SDL_JoystickName(i));
			I_ShutdownJoystick2();
			return -1;
		}
	}
	else
	{
		JoyReset(&JoyInfo2);
		//I_ShutdownJoystick();
		//joy_open2(fname);
	}

	num_joy = SDL_NumJoysticks();

	if (joyindex <= 0 || num_joy == 0 || JoyInfo2.oldjoy == joyindex)
	{
//		I_OutputMsg("Unable to use that joystick #(%s), non-number\n",fname);
		if (num_joy != 0)
		{
			CONS_Printf(M_GetText("Found %d joysticks on this system\n"), num_joy);
			for (i = 0; i < num_joy; i++)
				CONS_Printf("#%d/(%s)\n", i+1, SDL_JoystickName(i));
		}
		else
			CONS_Printf("%s", M_GetText("Found no joysticks on this system\n"));
		if (joyindex <= 0 || num_joy == 0) return 0;
	}

	JoyInfo2.dev = SDL_JoystickOpen(joyindex-1);
	CONS_Printf(M_GetText("Joystick2: %s\n"), SDL_JoystickName(joyindex-1));

	if (!JoyInfo2.dev)
	{
		CONS_Printf(M_GetText("Couldn't open joystick2: %s\n"), SDL_GetError());
		I_ShutdownJoystick2();
		return -1;
	}
	else
	{
		JoyInfo2.axises = SDL_JoystickNumAxes(JoyInfo2.dev);
		if (JoyInfo2.axises > JOYAXISSET*2)
			JoyInfo2.axises = JOYAXISSET*2;
/*		if (joyaxes < 2)
		{
			I_OutputMsg("Not enought axes?\n");
			I_ShutdownJoystick2();
			return 0;
		}*/

		JoyInfo2.buttons = SDL_JoystickNumButtons(JoyInfo2.dev);
		if (JoyInfo2.buttons > JOYBUTTONS)
			JoyInfo2.buttons = JOYBUTTONS;

		JoyInfo2.hats = SDL_JoystickNumHats(JoyInfo2.dev);
		if (JoyInfo2.hats > JOYHATS)
			JoyInfo2.hats = JOYHATS;

		JoyInfo2.balls = SDL_JoystickNumBalls(JoyInfo2.dev);

		//Joystick.bGamepadStyle = !stricmp(SDL_JoystickName(SDL_JoystickIndex(JoyInfo2.dev)), "pad");

		return JoyInfo2.axises;
	}
}

//Joystick3

/**	\brief Joystick 3 buttons states
*/
static UINT64 lastjoy3buttons = 0;

/**	\brief Joystick 3 hats state
*/
static UINT64 lastjoy3hats = 0;

/**	\brief	Shuts down joystick 3


	\return	void
*/
static void I_ShutdownJoystick3(void)
{
	INT32 i;
	event_t event;
	event.type = ev_keyup;
	event.data2 = 0;
	event.data3 = 0;

	lastjoy3buttons = lastjoy3hats = 0;

	// emulate the up of all joystick buttons
	for (i = 0; i < JOYBUTTONS; i++)
	{
		event.data1 = KEY_3JOY1 + i;
		D_PostEvent(&event);
	}

	// emulate the up of all joystick hats
	for (i = 0; i < JOYHATS*4; i++)
	{
		event.data1 = KEY_3HAT1 + i;
		D_PostEvent(&event);
	}

	// reset joystick position
	event.type = ev_joystick3;
	for (i = 0; i < JOYAXISSET; i++)
	{
		event.data1 = i;
		D_PostEvent(&event);
	}

	JoyReset(&JoyInfo3);
	if (!joystick_started && !joystick2_started && !joystick3_started && !joystick4_started
		&& SDL_WasInit(SDL_INIT_JOYSTICK) == SDL_INIT_JOYSTICK)
	{
		SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
		if (cv_usejoystick3.value == 0)
		{
			DEBFILE("I_Joystick3: SDL's Joystick system has been shutdown\n");
		}
	}
}

void I_GetJoystick3Events(void)
{
	static event_t event = {0,0,0,0};
	INT32 i = 0;
	UINT64 joyhats = 0;
#if 0
	INT64 joybuttons = 0;
	INT32 axisx, axisy;
#endif

	if (!joystick3_started)
		return;

	if (!JoyInfo3.dev) //I_ShutdownJoystick3();
		return;


#if 0
	//faB: look for as much buttons as g_input code supports,
	//  we don't use the others
	for (i = JoyInfo3.buttons - 1; i >= 0; i--)
	{
		joybuttons <<= 1;
		if (SDL_JoystickGetButton(JoyInfo3.dev,i))
			joybuttons |= 1;
	}

	if (joybuttons != lastjoy3buttons)
	{
		INT64 j = 1; // keep only bits that changed since last time
		INT64 newbuttons = joybuttons ^ lastjoy3buttons;
		lastjoy3buttons = joybuttons;

		for (i = 0; i < JOYBUTTONS; i++, j <<= 1)
		{
			if (newbuttons & j) // button changed state?
			{
				if (joybuttons & j)
					event.type = ev_keydown;
				else
					event.type = ev_keyup;
				event.data1 = KEY_3JOY1 + i;
				D_PostEvent(&event);
			}
		}
	}
#endif

	for (i = JoyInfo3.hats - 1; i >= 0; i--)
	{
		Uint8 hat = SDL_JoystickGetHat(JoyInfo3.dev, i);

		if (hat & SDL_HAT_UP   ) joyhats|=(UINT64)0x1<<(0 + 4*i);
		if (hat & SDL_HAT_DOWN ) joyhats|=(UINT64)0x1<<(1 + 4*i);
		if (hat & SDL_HAT_LEFT ) joyhats|=(UINT64)0x1<<(2 + 4*i);
		if (hat & SDL_HAT_RIGHT) joyhats|=(UINT64)0x1<<(3 + 4*i);
	}

	if (joyhats != lastjoy3hats)
	{
		INT64 j = 1; // keep only bits that changed since last time
		INT64 newhats = joyhats ^ lastjoy3hats;
		lastjoy3hats = joyhats;

		for (i = 0; i < JOYHATS*4; i++, j <<= 1)
		{
			if (newhats & j) // hat changed state?
			{
				if (joyhats & j)
					event.type = ev_keydown;
				else
					event.type = ev_keyup;
				event.data1 = KEY_3HAT1 + i;
				D_PostEvent(&event);
			}
		}
	}

#if 0
	// send joystick axis positions
	event.type = ev_joystick3;

	for (i = JOYAXISSET - 1; i >= 0; i--)
	{
		event.data1 = i;
		if (i*2 + 1 <= JoyInfo3.axises)
			axisx = SDL_JoystickGetAxis(JoyInfo3.dev, i*2 + 0);
		else axisx = 0;
		if (i*2 + 2 <= JoyInfo3.axises)
			axisy = SDL_JoystickGetAxis(JoyInfo3.dev, i*2 + 1);
		else axisy = 0;

		axisx = axisx/32;
		axisy = axisy/32;

		if (Joystick3.bGamepadStyle)
		{
			// gamepad control type, on or off, live or die
			if (axisx < -(JOYAXISRANGE/2))
				event.data2 = -1;
			else if (axisx > (JOYAXISRANGE/2))
				event.data2 = 1;
			else
				event.data2 = 0;
			if (axisy < -(JOYAXISRANGE/2))
				event.data3 = -1;
			else if (axisy > (JOYAXISRANGE/2))
				event.data3 = 1;
			else
				event.data3 = 0;
		}
		else
		{

			axisx = JoyInfo3.scale?((axisx/JoyInfo3.scale)*JoyInfo3.scale):axisx;
			axisy = JoyInfo3.scale?((axisy/JoyInfo3.scale)*JoyInfo3.scale):axisy;

#ifdef SDL_JDEADZONE
			if (-SDL_JDEADZONE <= axisx && axisx <= SDL_JDEADZONE) axisx = 0;
			if (-SDL_JDEADZONE <= axisy && axisy <= SDL_JDEADZONE) axisy = 0;
#endif

			// analog control style , just send the raw data
			event.data2 = axisx; // x axis
			event.data3 = axisy; // y axis
		}
		D_PostEvent(&event);
	}
#endif

}

/**	\brief	Open joystick handle

	\param	fname	name of joystick

	\return	axises


*/
static int joy_open3(const char *fname)
{
	int joyindex = atoi(fname);
	int num_joy = 0;
	int i;

	if (joystick_started == 0 && joystick2_started == 0 && joystick3_started == 0 && joystick4_started == 0)
	{
		if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) == -1)
		{
			CONS_Printf(M_GetText("Couldn't initialize joystick: %s\n"), SDL_GetError());
			return -1;
		}
		else
			num_joy = SDL_NumJoysticks();

		if (num_joy < joyindex)
		{
			CONS_Printf(M_GetText("Cannot use joystick #%d/(%s), it doesn't exist\n"),joyindex,fname);
			for (i = 0; i < num_joy; i++)
				CONS_Printf("#%d/(%s)\n", i+1, SDL_JoystickName(i));
			I_ShutdownJoystick3();
			return -1;
		}
	}
	else
	{
		JoyReset(&JoyInfo3);
		//I_ShutdownJoystick();
		//joy_open3(fname);
	}

	num_joy = SDL_NumJoysticks();

	if (joyindex <= 0 || num_joy == 0 || JoyInfo3.oldjoy == joyindex)
	{
//		I_OutputMsg("Unable to use that joystick #(%s), non-number\n",fname);
		if (num_joy != 0)
		{
			CONS_Printf(M_GetText("Found %d joysticks on this system\n"), num_joy);
			for (i = 0; i < num_joy; i++)
				CONS_Printf("#%d/(%s)\n", i+1, SDL_JoystickName(i));
		}
		else
			CONS_Printf("%s", M_GetText("Found no joysticks on this system\n"));
		if (joyindex <= 0 || num_joy == 0) return 0;
	}

	JoyInfo3.dev = SDL_JoystickOpen(joyindex-1);
	CONS_Printf(M_GetText("Joystick3: %s\n"), SDL_JoystickName(joyindex-1));

	if (!JoyInfo3.dev)
	{
		CONS_Printf(M_GetText("Couldn't open joystick3: %s\n"), SDL_GetError());
		I_ShutdownJoystick3();
		return -1;
	}
	else
	{
		JoyInfo3.axises = SDL_JoystickNumAxes(JoyInfo3.dev);
		if (JoyInfo3.axises > JOYAXISSET*2)
			JoyInfo3.axises = JOYAXISSET*2;
/*		if (joyaxes < 2)
		{
			I_OutputMsg("Not enought axes?\n");
			I_ShutdownJoystick3();
			return 0;
		}*/

		JoyInfo3.buttons = SDL_JoystickNumButtons(JoyInfo3.dev);
		if (JoyInfo3.buttons > JOYBUTTONS)
			JoyInfo3.buttons = JOYBUTTONS;

		JoyInfo3.hats = SDL_JoystickNumHats(JoyInfo3.dev);
		if (JoyInfo3.hats > JOYHATS)
			JoyInfo3.hats = JOYHATS;

		JoyInfo3.balls = SDL_JoystickNumBalls(JoyInfo3.dev);

		//Joystick.bGamepadStyle = !stricmp(SDL_JoystickName(SDL_JoystickIndex(JoyInfo3.dev)), "pad");

		return JoyInfo3.axises;
	}
}

//Joystick4

/**	\brief Joystick 4 buttons states
*/
static UINT64 lastjoy4buttons = 0;

/**	\brief Joystick 4 hats state
*/
static UINT64 lastjoy4hats = 0;

/**	\brief	Shuts down joystick 4


	\return	void
*/
static void I_ShutdownJoystick4(void)
{
	INT32 i;
	event_t event;
	event.type = ev_keyup;
	event.data2 = 0;
	event.data3 = 0;

	lastjoy4buttons = lastjoy4hats = 0;

	// emulate the up of all joystick buttons
	for (i = 0; i < JOYBUTTONS; i++)
	{
		event.data1 = KEY_4JOY1 + i;
		D_PostEvent(&event);
	}

	// emulate the up of all joystick hats
	for (i = 0; i < JOYHATS*4; i++)
	{
		event.data1 = KEY_4HAT1 + i;
		D_PostEvent(&event);
	}

	// reset joystick position
	event.type = ev_joystick4;
	for (i = 0; i < JOYAXISSET; i++)
	{
		event.data1 = i;
		D_PostEvent(&event);
	}

	JoyReset(&JoyInfo4);
	if (!joystick_started && !joystick2_started && !joystick3_started && !joystick4_started
		&& SDL_WasInit(SDL_INIT_JOYSTICK) == SDL_INIT_JOYSTICK)
	{
		SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
		if (cv_usejoystick4.value == 0)
		{
			DEBFILE("I_Joystick3: SDL's Joystick system has been shutdown\n");
		}
	}
}

void I_GetJoystick4Events(void)
{
	static event_t event = {0,0,0,0};
	INT32 i = 0;
	UINT64 joyhats = 0;
#if 0
	INT64 joybuttons = 0;
	INT32 axisx, axisy;
#endif

	if (!joystick4_started)
		return;

	if (!JoyInfo4.dev) //I_ShutdownJoystick4();
		return;


#if 0
	//faB: look for as much buttons as g_input code supports,
	//  we don't use the others
	for (i = JoyInfo4.buttons - 1; i >= 0; i--)
	{
		joybuttons <<= 1;
		if (SDL_JoystickGetButton(JoyInfo4.dev,i))
			joybuttons |= 1;
	}

	if (joybuttons != lastjoy4buttons)
	{
		INT64 j = 1; // keep only bits that changed since last time
		INT64 newbuttons = joybuttons ^ lastjoy4buttons;
		lastjoy4buttons = joybuttons;

		for (i = 0; i < JOYBUTTONS; i++, j <<= 1)
		{
			if (newbuttons & j) // button changed state?
			{
				if (joybuttons & j)
					event.type = ev_keydown;
				else
					event.type = ev_keyup;
				event.data1 = KEY_4JOY1 + i;
				D_PostEvent(&event);
			}
		}
	}
#endif

	for (i = JoyInfo4.hats - 1; i >= 0; i--)
	{
		Uint8 hat = SDL_JoystickGetHat(JoyInfo4.dev, i);

		if (hat & SDL_HAT_UP   ) joyhats|=(UINT64)0x1<<(0 + 4*i);
		if (hat & SDL_HAT_DOWN ) joyhats|=(UINT64)0x1<<(1 + 4*i);
		if (hat & SDL_HAT_LEFT ) joyhats|=(UINT64)0x1<<(2 + 4*i);
		if (hat & SDL_HAT_RIGHT) joyhats|=(UINT64)0x1<<(3 + 4*i);
	}

	if (joyhats != lastjoy4hats)
	{
		INT64 j = 1; // keep only bits that changed since last time
		INT64 newhats = joyhats ^ lastjoy4hats;
		lastjoy4hats = joyhats;

		for (i = 0; i < JOYHATS*4; i++, j <<= 1)
		{
			if (newhats & j) // hat changed state?
			{
				if (joyhats & j)
					event.type = ev_keydown;
				else
					event.type = ev_keyup;
				event.data1 = KEY_4HAT1 + i;
				D_PostEvent(&event);
			}
		}
	}

#if 0
	// send joystick axis positions
	event.type = ev_joystick4;

	for (i = JOYAXISSET - 1; i >= 0; i--)
	{
		event.data1 = i;
		if (i*2 + 1 <= JoyInfo4.axises)
			axisx = SDL_JoystickGetAxis(JoyInfo4.dev, i*2 + 0);
		else axisx = 0;
		if (i*2 + 2 <= JoyInfo4.axises)
			axisy = SDL_JoystickGetAxis(JoyInfo4.dev, i*2 + 1);
		else axisy = 0;

		axisx = axisx/32;
		axisy = axisy/32;

		if (Joystick4.bGamepadStyle)
		{
			// gamepad control type, on or off, live or die
			if (axisx < -(JOYAXISRANGE/2))
				event.data2 = -1;
			else if (axisx > (JOYAXISRANGE/2))
				event.data2 = 1;
			else
				event.data2 = 0;
			if (axisy < -(JOYAXISRANGE/2))
				event.data3 = -1;
			else if (axisy > (JOYAXISRANGE/2))
				event.data3 = 1;
			else
				event.data3 = 0;
		}
		else
		{

			axisx = JoyInfo4.scale?((axisx/JoyInfo4.scale)*JoyInfo4.scale):axisx;
			axisy = JoyInfo4.scale?((axisy/JoyInfo4.scale)*JoyInfo4.scale):axisy;

#ifdef SDL_JDEADZONE
			if (-SDL_JDEADZONE <= axisx && axisx <= SDL_JDEADZONE) axisx = 0;
			if (-SDL_JDEADZONE <= axisy && axisy <= SDL_JDEADZONE) axisy = 0;
#endif

			// analog control style , just send the raw data
			event.data2 = axisx; // x axis
			event.data3 = axisy; // y axis
		}
		D_PostEvent(&event);
	}
#endif

}

/**	\brief	Open joystick handle

	\param	fname	name of joystick

	\return	axises


*/
static int joy_open4(const char *fname)
{
	int joyindex = atoi(fname);
	int num_joy = 0;
	int i;

	if (joystick_started == 0 && joystick2_started == 0 && joystick3_started == 0 && joystick4_started == 0)
	{
		if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) == -1)
		{
			CONS_Printf(M_GetText("Couldn't initialize joystick: %s\n"), SDL_GetError());
			return -1;
		}
		else
			num_joy = SDL_NumJoysticks();

		if (num_joy < joyindex)
		{
			CONS_Printf(M_GetText("Cannot use joystick #%d/(%s), it doesn't exist\n"),joyindex,fname);
			for (i = 0; i < num_joy; i++)
				CONS_Printf("#%d/(%s)\n", i+1, SDL_JoystickName(i));
			I_ShutdownJoystick3();
			return -1;
		}
	}
	else
	{
		JoyReset(&JoyInfo4);
		//I_ShutdownJoystick();
		//joy_open4(fname);
	}

	num_joy = SDL_NumJoysticks();

	if (joyindex <= 0 || num_joy == 0 || JoyInfo4.oldjoy == joyindex)
	{
//		I_OutputMsg("Unable to use that joystick #(%s), non-number\n",fname);
		if (num_joy != 0)
		{
			CONS_Printf(M_GetText("Found %d joysticks on this system\n"), num_joy);
			for (i = 0; i < num_joy; i++)
				CONS_Printf("#%d/(%s)\n", i+1, SDL_JoystickName(i));
		}
		else
			CONS_Printf("%s", M_GetText("Found no joysticks on this system\n"));
		if (joyindex <= 0 || num_joy == 0) return 0;
	}

	JoyInfo4.dev = SDL_JoystickOpen(joyindex-1);
	CONS_Printf(M_GetText("Joystick4: %s\n"), SDL_JoystickName(joyindex-1));

	if (!JoyInfo4.dev)
	{
		CONS_Printf(M_GetText("Couldn't open joystick4: %s\n"), SDL_GetError());
		I_ShutdownJoystick4();
		return -1;
	}
	else
	{
		JoyInfo4.axises = SDL_JoystickNumAxes(JoyInfo4.dev);
		if (JoyInfo4.axises > JOYAXISSET*2)
			JoyInfo4.axises = JOYAXISSET*2;
/*		if (joyaxes < 2)
		{
			I_OutputMsg("Not enought axes?\n");
			I_ShutdownJoystick4();
			return 0;
		}*/

		JoyInfo4.buttons = SDL_JoystickNumButtons(JoyInfo4.dev);
		if (JoyInfo4.buttons > JOYBUTTONS)
			JoyInfo4.buttons = JOYBUTTONS;

		JoyInfo4.hats = SDL_JoystickNumHats(JoyInfo4.dev);
		if (JoyInfo4.hats > JOYHATS)
			JoyInfo4.hats = JOYHATS;

		JoyInfo4.balls = SDL_JoystickNumBalls(JoyInfo4.dev);

		//Joystick.bGamepadStyle = !stricmp(SDL_JoystickName(SDL_JoystickIndex(JoyInfo4.dev)), "pad");

		return JoyInfo4.axises;
	}
}

//
// I_InitJoystick
//
void I_InitJoystick(void)
{
	I_ShutdownJoystick();
	if (!strcmp(cv_usejoystick.string, "0") || M_CheckParm("-nojoy"))
		return;
	if (joy_open(cv_usejoystick.string) != -1)
		JoyInfo.oldjoy = atoi(cv_usejoystick.string);
	else
	{
		cv_usejoystick.value = 0;
		return;
	}
	joystick_started = 1;
}

void I_InitJoystick2(void)
{
	I_ShutdownJoystick2();
	if (!strcmp(cv_usejoystick2.string, "0") || M_CheckParm("-nojoy"))
		return;
	if (joy_open2(cv_usejoystick2.string) != -1)
		JoyInfo2.oldjoy = atoi(cv_usejoystick2.string);
	else
	{
		cv_usejoystick2.value = 0;
		return;
	}
	joystick2_started = 1;
}

void I_InitJoystick3(void)
{
	I_ShutdownJoystick3();
	if (!strcmp(cv_usejoystick3.string, "0") || M_CheckParm("-nojoy"))
		return;
	if (joy_open3(cv_usejoystick3.string) != -1)
		JoyInfo3.oldjoy = atoi(cv_usejoystick3.string);
	else
	{
		cv_usejoystick3.value = 0;
		return;
	}
	joystick3_started = 1;
}

void I_InitJoystick4(void)
{
	I_ShutdownJoystick4();
	if (!strcmp(cv_usejoystick4.string, "0") || M_CheckParm("-nojoy"))
		return;
	if (joy_open4(cv_usejoystick4.string) != -1)
		JoyInfo4.oldjoy = atoi(cv_usejoystick4.string);
	else
	{
		cv_usejoystick4.value = 0;
		return;
	}
	joystick4_started = 1;
}

static void I_ShutdownInput(void)
{
	if (SDL_WasInit(SDL_INIT_JOYSTICK) == SDL_INIT_JOYSTICK)
	{
		JoyReset(&JoyInfo);
		JoyReset(&JoyInfo2);
		JoyReset(&JoyInfo3);
		JoyReset(&JoyInfo4);
		SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
	}

}

INT32 I_NumJoys(void)
{
	INT32 numjoy = 0;
	if (SDL_WasInit(SDL_INIT_JOYSTICK) == 0)
	{
		if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) != -1)
			numjoy = SDL_NumJoysticks();
		SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
	}
	else
		numjoy = SDL_NumJoysticks();
	return numjoy;
}

static char joyname[255]; // MAX_PATH; joystick name is straight from the driver

const char *I_GetJoyName(INT32 joyindex)
{
	const char *tempname = NULL;
	joyindex--; //SDL's Joystick System starts at 0, not 1
	if (SDL_WasInit(SDL_INIT_JOYSTICK) == 0)
	{
		if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) != -1)
		{
			tempname = SDL_JoystickNameForIndex(joyindex);
			if (tempname)
				strncpy(joyname, tempname, 255);
		}
		SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
	}
	else
	{
		tempname = SDL_JoystickNameForIndex(joyindex);
		if (tempname)
			strncpy(joyname, tempname, 255);
	}
	return joyname;
}

#ifndef NOMUMBLE
#ifdef HAVE_MUMBLE
// Best Mumble positional audio settings:
// Minimum distance 3.0 m
// Bloom 175%
// Maximum distance 80.0 m
// Minimum volume 50%
#define DEG2RAD (0.017453292519943295769236907684883l) // TAU/360 or PI/180
#define MUMBLEUNIT (64.0f) // FRACUNITS in a Meter

static struct {
#ifdef WINMUMBLE
	UINT32 uiVersion;
	DWORD uiTick;
#else
	Uint32 uiVersion;
	Uint32 uiTick;
#endif
	float fAvatarPosition[3];
	float fAvatarFront[3];
	float fAvatarTop[3]; // defaults to Y-is-up (only used for leaning)
	wchar_t name[256]; // game name
	float fCameraPosition[3];
	float fCameraFront[3];
	float fCameraTop[3]; // defaults to Y-is-up (only used for leaning)
	wchar_t identity[256]; // player id
#ifdef WINMUMBLE
	UINT32 context_len;
#else
	Uint32 context_len;
#endif
	unsigned char context[256]; // server/team
	wchar_t description[2048]; // game description
} *mumble = NULL;
#endif // HAVE_MUMBLE

static void I_SetupMumble(void)
{
#ifdef WINMUMBLE
	HANDLE hMap = OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, L"MumbleLink");
	if (!hMap)
		return;

	mumble = MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(*mumble));
	if (!mumble)
		CloseHandle(hMap);
#elif defined (HAVE_SHM)
	int shmfd;
	char memname[256];

	snprintf(memname, 256, "/MumbleLink.%d", getuid());
	shmfd = shm_open(memname, O_RDWR, S_IRUSR | S_IWUSR);

	if(shmfd < 0)
		return;

	mumble = mmap(NULL, sizeof(*mumble), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
	if (mumble == MAP_FAILED)
		mumble = NULL;
#endif
}

void I_UpdateMumble(const mobj_t *mobj, const listener_t listener)
{
#ifdef HAVE_MUMBLE
	double angle;
	fixed_t anglef;

	if (!mumble)
		return;

	if(mumble->uiVersion != 2) {
		wcsncpy(mumble->name, L"SRB2Kart "VERSIONSTRING, 256);
		wcsncpy(mumble->description, L"Sonic Robo Blast 2 Kart with integrated Mumble Link support.", 2048);
		mumble->uiVersion = 2;
	}
	mumble->uiTick++;

	if (!netgame || gamestate != GS_LEVEL) { // Zero out, but never delink.
		mumble->fAvatarPosition[0] = mumble->fAvatarPosition[1] = mumble->fAvatarPosition[2] = 0.0f;
		mumble->fAvatarFront[0] = 1.0f;
		mumble->fAvatarFront[1] = mumble->fAvatarFront[2] = 0.0f;
		mumble->fCameraPosition[0] = mumble->fCameraPosition[1] = mumble->fCameraPosition[2] = 0.0f;
		mumble->fCameraFront[0] = 1.0f;
		mumble->fCameraFront[1] = mumble->fCameraFront[2] = 0.0f;
		return;
	}

	{
		UINT8 *p = mumble->context;
		WRITEMEM(p, server_context, 8);
		WRITEINT16(p, gamemap);
		mumble->context_len = p - mumble->context;
	}

	if (mobj) {
		mumble->fAvatarPosition[0] = FIXED_TO_FLOAT(mobj->x) / MUMBLEUNIT;
		mumble->fAvatarPosition[1] = FIXED_TO_FLOAT(mobj->z) / MUMBLEUNIT;
		mumble->fAvatarPosition[2] = FIXED_TO_FLOAT(mobj->y) / MUMBLEUNIT;

		anglef = AngleFixed(mobj->angle);
		angle = FIXED_TO_FLOAT(anglef)*DEG2RAD;
		mumble->fAvatarFront[0] = (float)cos(angle);
		mumble->fAvatarFront[1] = 0.0f;
		mumble->fAvatarFront[2] = (float)sin(angle);
	} else {
		mumble->fAvatarPosition[0] = mumble->fAvatarPosition[1] = mumble->fAvatarPosition[2] = 0.0f;
		mumble->fAvatarFront[0] = 1.0f;
		mumble->fAvatarFront[1] = mumble->fAvatarFront[2] = 0.0f;
	}

	mumble->fCameraPosition[0] = FIXED_TO_FLOAT(listener.x) / MUMBLEUNIT;
	mumble->fCameraPosition[1] = FIXED_TO_FLOAT(listener.z) / MUMBLEUNIT;
	mumble->fCameraPosition[2] = FIXED_TO_FLOAT(listener.y) / MUMBLEUNIT;

	anglef = AngleFixed(listener.angle);
	angle = FIXED_TO_FLOAT(anglef)*DEG2RAD;
	mumble->fCameraFront[0] = (float)cos(angle);
	mumble->fCameraFront[1] = 0.0f;
	mumble->fCameraFront[2] = (float)sin(angle);
#else
	(void)mobj;
	(void)listener;
#endif // HAVE_MUMBLE
}
#undef WINMUMBLE
#endif // NOMUMBLE

#ifdef HAVE_TERMIOS

void I_GetMouseEvents(void)
{
	static UINT8 mdata[5];
	static INT32 i = 0,om2b = 0;
	INT32 di, j, mlp, button;
	event_t event;
	const INT32 mswap[8] = {0, 4, 1, 5, 2, 6, 3, 7};

	if (!mouse2_started) return;
	for (mlp = 0; mlp < 20; mlp++)
	{
		for (; i < 5; i++)
		{
			di = read(fdmouse2, mdata+i, 1);
			if (di == -1) return;
		}
		if ((mdata[0] & 0xf8) != 0x80)
		{
			for (j = 1; j < 5; j++)
				if ((mdata[j] & 0xf8) == 0x80)
					for (i = 0; i < 5-j; i++) // shift
						mdata[i] = mdata[i+j];
			if (i < 5) continue;
		}
		else
		{
			button = mswap[~mdata[0] & 0x07];
			for (j = 0; j < MOUSEBUTTONS; j++)
			{
				if (om2b & (1<<j))
				{
					if (!(button & (1<<j))) //keyup
					{
						event.type = ev_keyup;
						event.data1 = KEY_2MOUSE1+j;
						D_PostEvent(&event);
						om2b ^= 1 << j;
					}
				}
				else
				{
					if (button & (1<<j))
					{
						event.type = ev_keydown;
						event.data1 = KEY_2MOUSE1+j;
						D_PostEvent(&event);
						om2b ^= 1 << j;
					}
				}
			}
			event.data2 = ((SINT8)mdata[1])+((SINT8)mdata[3]);
			event.data3 = ((SINT8)mdata[2])+((SINT8)mdata[4]);
			if (event.data2 && event.data3)
			{
				event.type = ev_mouse2;
				event.data1 = 0;
				D_PostEvent(&event);
			}
		}
		i = 0;
	}
}

//
// I_ShutdownMouse2
//
static void I_ShutdownMouse2(void)
{
	if (fdmouse2 != -1) close(fdmouse2);
	mouse2_started = 0;
}
#else
void I_GetMouseEvents(void){};
#endif

//
// I_StartupMouse2
//
void I_StartupMouse2(void)
{
#ifdef HAVE_TERMIOS
	struct termios m2tio;
	size_t i;
	INT32 dtr = -1, rts = -1;;
	I_ShutdownMouse2();
	if (cv_usemouse2.value == 0) return;
	if ((fdmouse2 = open(cv_mouse2port.string, O_RDONLY|O_NONBLOCK|O_NOCTTY)) == -1)
	{
		CONS_Printf(M_GetText("Error opening %s!\n"), cv_mouse2port.string);
		return;
	}
	tcflush(fdmouse2, TCIOFLUSH);
	m2tio.c_iflag = IGNBRK;
	m2tio.c_oflag = 0;
	m2tio.c_cflag = CREAD|CLOCAL|HUPCL|CS8|CSTOPB|B1200;
	m2tio.c_lflag = 0;
	m2tio.c_cc[VTIME] = 0;
	m2tio.c_cc[VMIN] = 1;
	tcsetattr(fdmouse2, TCSANOW, &m2tio);
	for (i = 0; i < strlen(cv_mouse2opt.string); i++)
	{
		if (toupper(cv_mouse2opt.string[i]) == 'D')
		{
			if (cv_mouse2opt.string[i+1] == '-')
				dtr = 0;
			else
				dtr = 1;
		}
		if (toupper(cv_mouse2opt.string[i]) == 'R')
		{
			if (cv_mouse2opt.string[i+1] == '-')
				rts = 0;
			else
				rts = 1;
		}
		if (dtr != -1 || rts != -1)
		{
			INT32 c;
			if (!ioctl(fdmouse2, TIOCMGET, &c))
			{
				if (!dtr)
					c &= ~TIOCM_DTR;
				else if (dtr > 0)
					c |= TIOCM_DTR;
			}
			if (!rts)
				c &= ~TIOCM_RTS;
			else if (rts > 0)
				c |= TIOCM_RTS;
			ioctl(fdmouse2, TIOCMSET, &c);
		}
	}
	mouse2_started = 1;
	I_AddExitFunc(I_ShutdownMouse2);
#endif
}

//
// I_Tactile
//
void I_Tactile(FFType pFFType, const JoyFF_t *FFEffect)
{
	// UNUSED.
	(void)pFFType;
	(void)FFEffect;
}

void I_Tactile2(FFType pFFType, const JoyFF_t *FFEffect)
{
	// UNUSED.
	(void)pFFType;
	(void)FFEffect;
}

void I_Tactile3(FFType pFFType, const JoyFF_t *FFEffect)
{
	// UNUSED.
	(void)pFFType;
	(void)FFEffect;
}

void I_Tactile4(FFType pFFType, const JoyFF_t *FFEffect)
{
	// UNUSED.
	(void)pFFType;
	(void)FFEffect;
}

/**	\brief empty ticcmd for player 1
*/
static ticcmd_t emptycmd;

ticcmd_t *I_BaseTiccmd(void)
{
	return &emptycmd;
}

/**	\brief empty ticcmd for player 2
*/
static ticcmd_t emptycmd2;

ticcmd_t *I_BaseTiccmd2(void)
{
	return &emptycmd2;
}

/**	\brief empty ticcmd for player 3
*/
static ticcmd_t emptycmd3;

ticcmd_t *I_BaseTiccmd3(void)
{
	return &emptycmd3;
}

/**	\brief empty ticcmd for player 4
*/
static ticcmd_t emptycmd4;

ticcmd_t *I_BaseTiccmd4(void)
{
	return &emptycmd4;
}

//
// I_GetTime
// returns time in 1/TICRATE second tics
//
tic_t I_GetTime (void)
{
	static Uint32 basetime = 0;
	       Uint32 ticks = SDL_GetTicks();

	if (!basetime)
		basetime = ticks;

	ticks -= basetime;

	ticks = (ticks*TICRATE);

#if 0 //#ifdef _WIN32_WCE
	ticks = (ticks/10);
#else
	ticks = (ticks/1000);
#endif

	return (tic_t)ticks;
}

//
//I_StartupTimer
//
void I_StartupTimer(void)
{
#if   0 //#elif !defined (_arch_dreamcast) && !defined(GP2X) // the DC have it own timer and GP2X have broken pthreads?
	if (SDL_InitSubSystem(SDL_INIT_TIMER) < 0)
		I_Error("SRB2: Needs SDL_Timer, Error: %s", SDL_GetError());
#endif
}



void I_Sleep(void)
{
	if (cv_sleep.value > 0)
		SDL_Delay(cv_sleep.value);
}

INT32 I_StartupSystem(void)
{
	SDL_version SDLcompiled;
	const SDL_version *SDLlinked;
	SDL_VERSION(&SDLcompiled)
	SDLlinked = SDL_Linked_Version();
	I_StartupConsole();
	I_OutputMsg("Compiled for SDL version: %d.%d.%d\n",
	 SDLcompiled.major, SDLcompiled.minor, SDLcompiled.patch);
	I_OutputMsg("Linked with SDL version: %d.%d.%d\n",
	 SDLlinked->major, SDLlinked->minor, SDLlinked->patch);
#if 0 //#ifdef GP2X //start up everything
	if (SDL_Init(SDL_INIT_NOPARACHUTE|SDL_INIT_EVERYTHING) < 0)
#else
	if (SDL_Init(SDL_INIT_NOPARACHUTE) < 0)
#endif
		I_Error("SRB2: SDL System Error: %s", SDL_GetError()); //Alam: Oh no....
#ifndef NOMUMBLE
	I_SetupMumble();
#endif
	return 0;
}


//
// I_Quit
//
void I_Quit(void)
{
	static SDL_bool quiting = SDL_FALSE;

	/* prevent recursive I_Quit() */
	if (quiting) goto death;
	SDLforceUngrabMouse();
	quiting = SDL_FALSE;
	I_ShutdownConsole();
	M_SaveConfig(NULL); //save game config, cvars..
#ifndef NONET
	D_SaveBan(); // save the ban list
#endif
	G_SaveGameData(); // Tails 12-08-2002
	//added:16-02-98: when recording a demo, should exit using 'q' key,
	//        but sometimes we forget and use 'F10'.. so save here too.

	if (demorecording)
		G_CheckDemoStatus();
	if (metalrecording)
		G_StopMetalRecording();

	D_QuitNetGame();
	I_ShutdownMusic();
	I_ShutdownSound();
	I_ShutdownCD();
	// use this for 1.28 19990220 by Kin
	I_ShutdownGraphics();
	I_ShutdownInput();
	I_ShutdownSystem();
	SDL_Quit();
	/* if option -noendtxt is set, don't print the text */
	if (!M_CheckParm("-noendtxt") && W_CheckNumForName("ENDOOM") != LUMPERROR)
	{
		printf("\r");
		ShowEndTxt();
	}
	if (myargmalloc)
		free(myargv); // Deallocate allocated memory
death:
	W_Shutdown();
#ifdef GP2X
	chdir("/usr/gp2x");
	execl("/usr/gp2x/gp2xmenu", "/usr/gp2x/gp2xmenu", NULL);
#endif
	exit(0);
}

void I_WaitVBL(INT32 count)
{
	count = 1;
	SDL_Delay(count);
}

void I_BeginRead(void)
{
}

void I_EndRead(void)
{
}

//
// I_Error
//
/**	\brief phuck recursive errors
*/
static INT32 errorcount = 0;

/**	\brief recursive error detecting
*/
static boolean shutdowning = false;

void I_Error(const char *error, ...)
{
	va_list argptr;
#if (defined (MAC_ALERT) || defined (_WIN32) || (defined (_WIN32_WCE) && !defined (__GNUC__))) && !defined (_XBOX)
	char buffer[8192];
#endif

	// recursive error detecting
	if (shutdowning)
	{
		errorcount++;
		if (errorcount == 1)
			SDLforceUngrabMouse();
		// try to shutdown each subsystem separately
		if (errorcount == 2)
			I_ShutdownMusic();
		if (errorcount == 3)
			I_ShutdownSound();
		if (errorcount == 4)
			I_ShutdownCD();
		if (errorcount == 5)
			I_ShutdownGraphics();
		if (errorcount == 6)
			I_ShutdownInput();
		if (errorcount == 7)
			I_ShutdownSystem();
		if (errorcount == 8)
			SDL_Quit();
		if (errorcount == 9)
		{
			M_SaveConfig(NULL);
			G_SaveGameData();
		}
		if (errorcount > 20)
		{
#ifdef MAC_ALERT
			va_start(argptr, error);
			vsprintf(buffer, error, argptr);
			va_end(argptr);
			// 2004-03-03 AJR Since the Mac user is most likely double clicking to run the game, give them a panel.
			MacShowAlert("Recursive Error", buffer, "Quit", NULL, NULL);
#else
			// Don't print garbage
			va_start(argptr, error);
			if (!framebuffer)
				vfprintf (stderr, error, argptr);
			va_end(argptr);
#endif
			W_Shutdown();
#ifdef GP2X
			chdir("/usr/gp2x");
			execl("/usr/gp2x/gp2xmenu", "/usr/gp2x/gp2xmenu", NULL);
#endif
			exit(-1); // recursive errors detected
		}
	}
	shutdowning = true;
	I_ShutdownConsole();
#ifndef MAC_ALERT
	// Message first.
	va_start(argptr,error);
	if (!framebuffer)
	{
		fprintf(stderr, "Error: ");
		vfprintf(stderr,error,argptr);
		fprintf(stderr, "\n");
	}
	va_end(argptr);

	if (!framebuffer)
		fflush(stderr);
#endif
	M_SaveConfig(NULL); // save game config, cvars..
#ifndef NONET
	D_SaveBan(); // save the ban list
#endif
	G_SaveGameData(); // Tails 12-08-2002

	// Shutdown. Here might be other errors.
	if (demorecording)
		G_CheckDemoStatus();
	if (metalrecording)
		G_StopMetalRecording();

	D_QuitNetGame();
	I_ShutdownMusic();
	I_ShutdownSound();
	I_ShutdownCD();
	// use this for 1.28 19990220 by Kin
	I_ShutdownGraphics();
	I_ShutdownInput();
	I_ShutdownSystem();
	SDL_Quit();
#ifdef MAC_ALERT
	va_start(argptr, error);
	vsprintf(buffer, error, argptr);
	va_end(argptr);
	// 2004-03-03 AJR Since the Mac user is most likely double clicking to run the game, give them a panel.
	MacShowAlert("Critical Error", buffer, "Quit", NULL, NULL);
#endif
	W_Shutdown();
#ifdef GP2X
	chdir("/usr/gp2x");
	execl("/usr/gp2x/gp2xmenu", "/usr/gp2x/gp2xmenu", NULL);
#endif
	exit(-1);
}

/**	\brief quit function table
*/
static quitfuncptr quit_funcs[MAX_QUIT_FUNCS]; /* initialized to all bits 0 */

//
//  Adds a function to the list that need to be called by I_SystemShutdown().
//
void I_AddExitFunc(void (*func)())
{
	INT32 c;

	for (c = 0; c < MAX_QUIT_FUNCS; c++)
	{
		if (!quit_funcs[c])
		{
			quit_funcs[c] = func;
			break;
		}
	}
}


//
//  Removes a function from the list that need to be called by
//   I_SystemShutdown().
//
void I_RemoveExitFunc(void (*func)())
{
	INT32 c;

	for (c = 0; c < MAX_QUIT_FUNCS; c++)
	{
		if (quit_funcs[c] == func)
		{
			while (c < MAX_QUIT_FUNCS-1)
			{
				quit_funcs[c] = quit_funcs[c+1];
				c++;
			}
			quit_funcs[MAX_QUIT_FUNCS-1] = NULL;
			break;
		}
	}
}

//
//  Closes down everything. This includes restoring the initial
//  palette and video mode, and removing whatever mouse, keyboard, and
//  timer routines have been installed.
//
//  NOTE: Shutdown user funcs are effectively called in reverse order.
//
void I_ShutdownSystem(void)
{
	INT32 c;

	for (c = MAX_QUIT_FUNCS-1; c >= 0; c--)
		if (quit_funcs[c])
			(*quit_funcs[c])();
#ifdef  LOGMESSAGES
	if (logstream)
	{
		fclose(logstream);
		logstream = NULL;
	}
#endif

}

void I_GetDiskFreeSpace(INT64 *freespace)
{
#if   defined (__unix__) || defined(__APPLE__) || defined (UNIXCOMMON)
#if defined (SOLARIS) || defined (__HAIKU__) || defined (_WII) || defined (_PS3)
	*freespace = INT32_MAX;
	return;
#else // Both Linux and BSD have this, apparently.
	struct statfs stfs;
	if (statfs(".", &stfs) == -1)
	{
		*freespace = INT32_MAX;
		return;
	}
	*freespace = stfs.f_bavail * stfs.f_bsize;
#endif
#else // Dummy for platform independent; 1GB should be enough
	*freespace = 1024*1024*1024;
#endif
}

char *I_GetUserName(void)
{
#ifdef GP2X
	static char username[MAXPLAYERNAME] = "GP2XUSER";
	return username;
#else
	static char username[MAXPLAYERNAME];
	char *p;
	{
		p = I_GetEnv("USER");
		if (!p)
		{
			p = I_GetEnv("user");
			if (!p)
			{
				p = I_GetEnv("USERNAME");
				if (!p)
				{
					p = I_GetEnv("username");
					if (!p)
					{
						return NULL;
					}
				}
			}
		}
		strncpy(username, p, MAXPLAYERNAME);
	}


	if (strcmp(username, "") != 0)
		return username;
#endif
	return NULL; // dummy for platform independent version
}

INT32 I_mkdir(const char *dirname, INT32 unixright)
{
//[segabor]
#if defined (__unix__) || defined(__APPLE__) || defined (UNIXCOMMON) || defined (__CYGWIN__) || defined (__OS2__)
	return mkdir(dirname, unixright);
#else
	(void)dirname;
	(void)unixright;
	return false;
#endif
}

char *I_GetEnv(const char *name)
{
#ifdef NEED_SDL_GETENV
	return SDL_getenv(name);
#else
	return getenv(name);
#endif
}

INT32 I_PutEnv(char *variable)
{
#ifdef NEED_SDL_GETENV
	return SDL_putenv(variable);
#else
	return putenv(variable);
#endif
}

INT32 I_ClipboardCopy(const char *data, size_t size)
{
	(void)data;
	(void)size;
	return -1;
}

char *I_ClipboardPaste(void)
{
	return NULL;
}

/**	\brief	The isWadPathOk function

	\param	path	string path to check

	\return if true, wad file found


*/
static boolean isWadPathOk(const char *path)
{
	char *wad3path = malloc(256);

	if (!wad3path)
		return false;

	sprintf(wad3path, pandf, path, WADKEYWORD1);

	if (FIL_ReadFileOK(wad3path))
	{
		free(wad3path);
		return true;
	}

	sprintf(wad3path, pandf, path, WADKEYWORD2);

	if (FIL_ReadFileOK(wad3path))
	{
		free(wad3path);
		return true;
	}

	free(wad3path);
	return false;
}

static void pathonly(char *s)
{
	size_t j;

	for (j = strlen(s); j != (size_t)-1; j--)
		if ((s[j] == '\\') || (s[j] == ':') || (s[j] == '/'))
		{
			if (s[j] == ':') s[j+1] = 0;
			else s[j] = 0;
			return;
		}
}

/**	\brief	search for srb2.srb in the given path

	\param	searchDir	starting path

	\return	WAD path if not NULL


*/
static const char *searchWad(const char *searchDir)
{
	static char tempsw[256] = "";
	filestatus_t fstemp;

	strcpy(tempsw, WADKEYWORD1);
	fstemp = filesearch(tempsw,searchDir,NULL,true,20);
	if (fstemp == FS_FOUND)
	{
		pathonly(tempsw);
		return tempsw;
	}

	strcpy(tempsw, WADKEYWORD2);
	fstemp = filesearch(tempsw, searchDir, NULL, true, 20);
	if (fstemp == FS_FOUND)
	{
		pathonly(tempsw);
		return tempsw;
	}
	return NULL;
}

/**	\brief go through all possible paths and look for srb2.srb

  \return path to srb2.srb if any
*/
static const char *locateWad(void)
{
	const char *envstr;
	const char *WadPath;

	I_OutputMsg("SRB2WADDIR");
	// does SRB2WADDIR exist?
	if (((envstr = I_GetEnv("SRB2WADDIR")) != NULL) && isWadPathOk(envstr))
		return envstr;


#ifndef NOCWD
	I_OutputMsg(",.");
	// examine current dir
	strcpy(returnWadPath, ".");
	if (isWadPathOk(returnWadPath))
		return NULL;
#endif

	// examine default dirs
#ifdef DEFAULTWADLOCATION1
	I_OutputMsg(","DEFAULTWADLOCATION1);
	strcpy(returnWadPath, DEFAULTWADLOCATION1);
	if (isWadPathOk(returnWadPath))
		return returnWadPath;
#endif
#ifdef DEFAULTWADLOCATION2
	I_OutputMsg(","DEFAULTWADLOCATION2);
	strcpy(returnWadPath, DEFAULTWADLOCATION2);
	if (isWadPathOk(returnWadPath))
		return returnWadPath;
#endif
#ifdef DEFAULTWADLOCATION3
	I_OutputMsg(","DEFAULTWADLOCATION3);
	strcpy(returnWadPath, DEFAULTWADLOCATION3);
	if (isWadPathOk(returnWadPath))
		return returnWadPath;
#endif
#ifdef DEFAULTWADLOCATION4
	I_OutputMsg(","DEFAULTWADLOCATION4);
	strcpy(returnWadPath, DEFAULTWADLOCATION4);
	if (isWadPathOk(returnWadPath))
		return returnWadPath;
#endif
#ifdef DEFAULTWADLOCATION5
	I_OutputMsg(","DEFAULTWADLOCATION5);
	strcpy(returnWadPath, DEFAULTWADLOCATION5);
	if (isWadPathOk(returnWadPath))
		return returnWadPath;
#endif
#ifdef DEFAULTWADLOCATION6
	I_OutputMsg(","DEFAULTWADLOCATION6);
	strcpy(returnWadPath, DEFAULTWADLOCATION6);
	if (isWadPathOk(returnWadPath))
		return returnWadPath;
#endif
#ifdef DEFAULTWADLOCATION7
	I_OutputMsg(","DEFAULTWADLOCATION7);
	strcpy(returnWadPath, DEFAULTWADLOCATION7);
	if (isWadPathOk(returnWadPath))
		return returnWadPath;
#endif
#ifndef NOHOME
	// find in $HOME
	I_OutputMsg(",HOME");
	if ((envstr = I_GetEnv("HOME")) != NULL)
	{
		WadPath = searchWad(envstr);
		if (WadPath)
			return WadPath;
	}
#endif
#ifdef DEFAULTSEARCHPATH1
	// find in /usr/local
	I_OutputMsg(", in:"DEFAULTSEARCHPATH1);
	WadPath = searchWad(DEFAULTSEARCHPATH1);
	if (WadPath)
		return WadPath;
#endif
#ifdef DEFAULTSEARCHPATH2
	// find in /usr/games
	I_OutputMsg(", in:"DEFAULTSEARCHPATH2);
	WadPath = searchWad(DEFAULTSEARCHPATH2);
	if (WadPath)
		return WadPath;
#endif
#ifdef DEFAULTSEARCHPATH3
	// find in ???
	I_OutputMsg(", in:"DEFAULTSEARCHPATH3);
	WadPath = searchWad(DEFAULTSEARCHPATH3);
	if (WadPath)
		return WadPath;
#endif
	// if nothing was found
	return NULL;
}

const char *I_LocateWad(void)
{
	const char *waddir;

	I_OutputMsg("Looking for WADs in: ");
	waddir = locateWad();
	I_OutputMsg("\n");

	if (waddir)
	{
		// change to the directory where we found srb2.srb
		if (chdir(waddir) == -1)
			I_OutputMsg("Couldn't change working directory\n");
	}
	return waddir;
}

#define MEMINFO_FILE "/proc/meminfo"
#define MEMTOTAL "MemTotal:"
#define MEMFREE "MemFree:"

// quick fix for compil
UINT32 I_GetFreeMem(UINT32 *total)
{
#if   defined (FREEBSD)
	struct vmmeter sum;
	kvm_t *kd;
	struct nlist namelist[] =
	{
#define X_SUM   0
		{"_cnt"},
		{NULL}
	};
	if ((kd = kvm_open(NULL, NULL, NULL, O_RDONLY, "kvm_open")) == NULL)
	{
		*total = 0L;
		return 0;
	}
	if (kvm_nlist(kd, namelist) != 0)
	{
		kvm_close (kd);
		*total = 0L;
		return 0;
	}
	if (kvm_read(kd, namelist[X_SUM].n_value, &sum,
		sizeof (sum)) != sizeof (sum))
	{
		kvm_close(kd);
		*total = 0L;
		return 0;
	}
	kvm_close(kd);

	if (total)
		*total = sum.v_page_count * sum.v_page_size;
	return sum.v_free_count * sum.v_page_size;
#else
	/* Linux */
	char buf[1024];
	char *memTag;
	UINT32 freeKBytes;
	UINT32 totalKBytes;
	INT32 n;
	INT32 meminfo_fd = -1;

	meminfo_fd = open(MEMINFO_FILE, O_RDONLY);
	n = read(meminfo_fd, buf, 1023);
	close(meminfo_fd);

	if (n < 0)
	{
		// Error
		*total = 0L;
		return 0;
	}

	buf[n] = '\0';
	if (NULL == (memTag = strstr(buf, MEMTOTAL)))
	{
		// Error
		*total = 0L;
		return 0;
	}

	memTag += sizeof (MEMTOTAL);
	totalKBytes = atoi(memTag);

	if (NULL == (memTag = strstr(buf, MEMFREE)))
	{
		// Error
		*total = 0L;
		return 0;
	}

	memTag += sizeof (MEMFREE);
	freeKBytes = atoi(memTag);

	if (total)
		*total = totalKBytes << 10;
	return freeKBytes << 10;
#endif
}

const CPUInfoFlags *I_CPUInfo(void)
{
#if   defined (HAVE_SDLCPUINFO)
	static CPUInfoFlags SDL_CPUInfo;
	memset(&SDL_CPUInfo,0,sizeof (CPUInfoFlags));
	SDL_CPUInfo.RDTSC       = SDL_HasRDTSC();
	SDL_CPUInfo.MMX         = SDL_HasMMX();
	SDL_CPUInfo.MMXExt      = SDL_HasMMXExt();
	SDL_CPUInfo.AMD3DNow    = SDL_Has3DNow();
	SDL_CPUInfo.AMD3DNowExt = SDL_Has3DNowExt();
	SDL_CPUInfo.SSE         = SDL_HasSSE();
	SDL_CPUInfo.SSE2        = SDL_HasSSE2();
	SDL_CPUInfo.AltiVec     = SDL_HasAltiVec();
	return &SDL_CPUInfo;
#else
	return NULL; /// \todo CPUID asm
#endif
}


void I_RegisterSysCommands(void)
{
}

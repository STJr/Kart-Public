// Emacs style mode select   -*- C++ -*-
// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Portions Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 2014-2018 by Sonic Team Junior.
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
/// \brief SRB2 graphics stuff for SDL

#include <stdlib.h>
#include <errno.h>

#include <signal.h>

#ifdef _MSC_VER
#pragma warning(disable : 4214 4244)
#endif

#ifdef HAVE_SDL
#define _MATH_DEFINES_DEFINED
#include "SDL.h"

#ifdef _MSC_VER
#pragma warning(default : 4214 4244)
#endif

#ifdef HAVE_TTF
#include "i_ttf.h"
#endif

#ifdef HAVE_IMAGE
#include "SDL_image.h"
#elif 1
#define LOAD_XPM //I want XPM!
#include "IMG_xpm.c" //Alam: I don't want to add SDL_Image.dll/so
#define HAVE_IMAGE //I have SDL_Image, sortof
#endif

#ifdef HAVE_IMAGE
#include "SDL_icon.xpm"
#endif

#include "../doomdef.h"

#ifdef _WIN32
#include "SDL_syswm.h"
#endif

#include "../doomstat.h"
#include "../i_system.h"
#include "../v_video.h"
#include "../m_argv.h"
#include "../m_menu.h"
#include "../d_main.h"
#include "../s_sound.h"
#include "../i_sound.h"  	// midi pause/unpause
#include "../i_joy.h"
#include "../st_stuff.h"
#include "../g_game.h"
#include "../i_video.h"
#include "../console.h"
#include "../command.h"
#include "sdlmain.h"
#include "../i_system.h"
#ifdef HWRENDER
#include "../hardware/hw_main.h"
#include "../hardware/hw_drv.h"
// For dynamic referencing of HW rendering functions
#include "hwsym_sdl.h"
#include "ogl_sdl.h"
#endif

#ifdef HAVE_DISCORDRPC
#include "../discord.h"
#endif

// maximum number of windowed modes (see windowedModes[][])
#define MAXWINMODES (18)

/**	\brief
*/
static INT32 numVidModes = -1;

/**	\brief
*/
static char vidModeName[33][32]; // allow 33 different modes

rendermode_t rendermode = render_none;

boolean highcolor = false;

static void Impl_SetVsync(void);

// synchronize page flipping with screen refresh
consvar_t cv_vidwait = {"vid_wait", "Off", CV_SAVE|CV_CALL|CV_NOINIT, CV_OnOff, Impl_SetVsync, 0, NULL, NULL, 0, 0, NULL};
static consvar_t cv_stretch = {"stretch", "Off", CV_SAVE|CV_NOSHOWHELP, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL};

UINT8 graphics_started = 0; // Is used in console.c and screen.c

// To disable fullscreen at startup; is set in VID_PrepareModeList
boolean allow_fullscreen = false;
static SDL_bool disable_fullscreen = SDL_FALSE;
#define USE_FULLSCREEN (disable_fullscreen||!allow_fullscreen)?0:cv_fullscreen.value
static SDL_bool disable_mouse = SDL_FALSE;
#define USE_MOUSEINPUT (!disable_mouse && cv_usemouse.value && havefocus)
#define MOUSE_MENU false //(!disable_mouse && cv_usemouse.value && menuactive && !USE_FULLSCREEN)
#define MOUSEBUTTONS_MAX MOUSEBUTTONS

// first entry in the modelist which is not bigger than MAXVIDWIDTHxMAXVIDHEIGHT
static      INT32          firstEntry = 0;

// Total mouse motion X/Y offsets
static      INT32        mousemovex = 0, mousemovey = 0;

// SDL vars
static      SDL_Surface *vidSurface = NULL;
static      SDL_Surface *bufSurface = NULL;
static      SDL_Surface *icoSurface = NULL;
static      SDL_Color    localPalette[256];
#if 0
static      SDL_Rect   **modeList = NULL;
static       Uint8       BitsPerPixel = 16;
#endif
Uint16      realwidth = BASEVIDWIDTH;
Uint16      realheight = BASEVIDHEIGHT;
static       SDL_bool    mousegrabok = SDL_TRUE;
static       SDL_bool    wrapmouseok = SDL_FALSE;
#define HalfWarpMouse(x,y) if (wrapmouseok) SDL_WarpMouseInWindow(window, (Uint16)(x/2),(Uint16)(y/2))
static       SDL_bool    videoblitok = SDL_FALSE;
static       SDL_bool    exposevideo = SDL_FALSE;
static       SDL_bool    usesdl2soft = SDL_FALSE;
static       SDL_bool    borderlesswindow = SDL_FALSE;

// SDL2 vars
SDL_Window   *window;
SDL_Renderer *renderer;
static SDL_Texture  *texture;
static SDL_bool      havefocus = SDL_TRUE;
static const char *fallback_resolution_name = "Fallback";

// windowed video modes from which to choose from.
static INT32 windowedModes[MAXWINMODES][2] =
{
	{1920,1200}, // 1.60,6.00
	{1920,1080}, // 1.66
	{1680,1050}, // 1.60,5.25
	{1600,1200}, // 1.33
	{1600, 900}, // 1.66
	{1366, 768}, // 1.66
	{1440, 900}, // 1.60,4.50
	{1280,1024}, // 1.33?
	{1280, 960}, // 1.33,4.00
	{1280, 800}, // 1.60,4.00
	{1280, 720}, // 1.66
	{1152, 864}, // 1.33,3.60
	{1024, 768}, // 1.33,3.20
	{ 800, 600}, // 1.33,2.50
	{ 640, 480}, // 1.33,2.00
	{ 640, 400}, // 1.60,2.00
	{ 320, 240}, // 1.33,1.00
	{ 320, 200}, // 1.60,1.00
};

static void Impl_VideoSetupSDLBuffer(void);
static void Impl_VideoSetupBuffer(void);
static SDL_bool Impl_CreateWindow(SDL_bool fullscreen);
//static void Impl_SetWindowName(const char *title);
static void Impl_SetWindowIcon(void);

static void SDLSetMode(INT32 width, INT32 height, SDL_bool fullscreen)
{
	static SDL_bool wasfullscreen = SDL_FALSE;
	Uint32 rmask;
	Uint32 gmask;
	Uint32 bmask;
	Uint32 amask;
	int bpp = 16;
	int sw_texture_format = SDL_PIXELFORMAT_ABGR8888;

	realwidth = vid.width;
	realheight = vid.height;

	if (window)
	{
		if (fullscreen)
		{
			wasfullscreen = SDL_TRUE;
			SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
		}
		else // windowed mode
		{
			if (wasfullscreen)
			{
				wasfullscreen = SDL_FALSE;
				SDL_SetWindowFullscreen(window, 0);
			}
			// Reposition window only in windowed mode
			SDL_SetWindowSize(window, width, height);
			SDL_SetWindowPosition(window,
				SDL_WINDOWPOS_CENTERED_DISPLAY(SDL_GetWindowDisplayIndex(window)),
				SDL_WINDOWPOS_CENTERED_DISPLAY(SDL_GetWindowDisplayIndex(window))
			);
		}
	}
	else
	{
		Impl_CreateWindow(fullscreen);
		Impl_SetWindowIcon();
		wasfullscreen = fullscreen;
		SDL_SetWindowSize(window, width, height);
		if (fullscreen)
		{
			SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
		}
	}

#ifdef HWRENDER
	if (rendermode == render_opengl)
	{
		OglSdlSurface(vid.width, vid.height);
	}
#endif

	if (rendermode == render_soft)
	{
		SDL_RenderClear(renderer);
		SDL_RenderSetLogicalSize(renderer, width, height);
		// Set up Texture
		realwidth = width;
		realheight = height;
		if (texture != NULL)
		{
			SDL_DestroyTexture(texture);
		}

		if (!usesdl2soft)
		{
			sw_texture_format = SDL_PIXELFORMAT_RGB565;
		}
		else
		{
			bpp = 32;
			sw_texture_format = SDL_PIXELFORMAT_RGBA8888;
		}

		texture = SDL_CreateTexture(renderer, sw_texture_format, SDL_TEXTUREACCESS_STREAMING, width, height);

		// Set up SW surface
		if (vidSurface != NULL)
		{
			SDL_FreeSurface(vidSurface);
		}
		if (vid.buffer)
		{
			free(vid.buffer);
			vid.buffer = NULL;
		}
		SDL_PixelFormatEnumToMasks(sw_texture_format, &bpp, &rmask, &gmask, &bmask, &amask);
		vidSurface = SDL_CreateRGBSurface(0, width, height, bpp, rmask, gmask, bmask, amask);
	}
}

static INT32 Impl_SDL_Scancode_To_Keycode(SDL_Scancode code)
{
	if (code >= SDL_SCANCODE_A && code <= SDL_SCANCODE_Z)
	{
		// get lowercase ASCII
		return code - SDL_SCANCODE_A + 'a';
	}
	if (code >= SDL_SCANCODE_1 && code <= SDL_SCANCODE_9)
	{
		return code - SDL_SCANCODE_1 + '1';
	}
	else if (code == SDL_SCANCODE_0)
	{
		return '0';
	}
	if (code >= SDL_SCANCODE_F1 && code <= SDL_SCANCODE_F10)
	{
		return KEY_F1 + (code - SDL_SCANCODE_F1);
	}
	switch (code)
	{
		// F11 and F12 are separated from the rest of the function keys
		case SDL_SCANCODE_F11: return KEY_F11;
		case SDL_SCANCODE_F12: return KEY_F12;

		case SDL_SCANCODE_KP_0: return KEY_KEYPAD0;
		case SDL_SCANCODE_KP_1: return KEY_KEYPAD1;
		case SDL_SCANCODE_KP_2: return KEY_KEYPAD2;
		case SDL_SCANCODE_KP_3: return KEY_KEYPAD3;
		case SDL_SCANCODE_KP_4: return KEY_KEYPAD4;
		case SDL_SCANCODE_KP_5: return KEY_KEYPAD5;
		case SDL_SCANCODE_KP_6: return KEY_KEYPAD6;
		case SDL_SCANCODE_KP_7: return KEY_KEYPAD7;
		case SDL_SCANCODE_KP_8: return KEY_KEYPAD8;
		case SDL_SCANCODE_KP_9: return KEY_KEYPAD9;

		case SDL_SCANCODE_RETURN:         return KEY_ENTER;
		case SDL_SCANCODE_ESCAPE:         return KEY_ESCAPE;
		case SDL_SCANCODE_BACKSPACE:      return KEY_BACKSPACE;
		case SDL_SCANCODE_TAB:            return KEY_TAB;
		case SDL_SCANCODE_SPACE:          return KEY_SPACE;
		case SDL_SCANCODE_MINUS:          return KEY_MINUS;
		case SDL_SCANCODE_EQUALS:         return KEY_EQUALS;
		case SDL_SCANCODE_LEFTBRACKET:    return '[';
		case SDL_SCANCODE_RIGHTBRACKET:   return ']';
		case SDL_SCANCODE_BACKSLASH:      return '\\';
		case SDL_SCANCODE_NONUSHASH:      return '#';
		case SDL_SCANCODE_SEMICOLON:      return ';';
		case SDL_SCANCODE_APOSTROPHE:     return '\'';
		case SDL_SCANCODE_GRAVE:          return '`';
		case SDL_SCANCODE_COMMA:          return ',';
		case SDL_SCANCODE_PERIOD:         return '.';
		case SDL_SCANCODE_SLASH:          return '/';
		case SDL_SCANCODE_CAPSLOCK:       return KEY_CAPSLOCK;
		case SDL_SCANCODE_PRINTSCREEN:    return 0; // undefined?
		case SDL_SCANCODE_SCROLLLOCK:     return KEY_SCROLLLOCK;
		case SDL_SCANCODE_PAUSE:          return KEY_PAUSE;
		case SDL_SCANCODE_INSERT:         return KEY_INS;
		case SDL_SCANCODE_HOME:           return KEY_HOME;
		case SDL_SCANCODE_PAGEUP:         return KEY_PGUP;
		case SDL_SCANCODE_DELETE:         return KEY_DEL;
		case SDL_SCANCODE_END:            return KEY_END;
		case SDL_SCANCODE_PAGEDOWN:       return KEY_PGDN;
		case SDL_SCANCODE_RIGHT:          return KEY_RIGHTARROW;
		case SDL_SCANCODE_LEFT:           return KEY_LEFTARROW;
		case SDL_SCANCODE_DOWN:           return KEY_DOWNARROW;
		case SDL_SCANCODE_UP:             return KEY_UPARROW;
		case SDL_SCANCODE_NUMLOCKCLEAR:   return KEY_NUMLOCK;
		case SDL_SCANCODE_KP_DIVIDE:      return KEY_KPADSLASH;
		case SDL_SCANCODE_KP_MULTIPLY:    return '*'; // undefined?
		case SDL_SCANCODE_KP_MINUS:       return KEY_MINUSPAD;
		case SDL_SCANCODE_KP_PLUS:        return KEY_PLUSPAD;
		case SDL_SCANCODE_KP_ENTER:       return KEY_ENTER;
		case SDL_SCANCODE_KP_PERIOD:      return KEY_KPADDEL;
		case SDL_SCANCODE_NONUSBACKSLASH: return '\\';

		case SDL_SCANCODE_LSHIFT: return KEY_LSHIFT;
		case SDL_SCANCODE_RSHIFT: return KEY_RSHIFT;
		case SDL_SCANCODE_LCTRL:  return KEY_LCTRL;
		case SDL_SCANCODE_RCTRL:  return KEY_RCTRL;
		case SDL_SCANCODE_LALT:   return KEY_LALT;
		case SDL_SCANCODE_RALT:   return KEY_RALT;
		case SDL_SCANCODE_LGUI:   return KEY_LEFTWIN;
		case SDL_SCANCODE_RGUI:   return KEY_RIGHTWIN;
		default:                  break;
	}
	return 0;
}

static void SDLdoGrabMouse(void)
{
	SDL_ShowCursor(SDL_DISABLE);
	SDL_SetWindowGrab(window, SDL_TRUE);
	if (SDL_SetRelativeMouseMode(SDL_TRUE) == 0) // already warps mouse if successful
		wrapmouseok = SDL_TRUE; // TODO: is wrapmouseok or HalfWarpMouse needed anymore?
}

static void SDLdoUngrabMouse(void)
{
	SDL_ShowCursor(SDL_ENABLE);
	SDL_SetWindowGrab(window, SDL_FALSE);
	wrapmouseok = SDL_FALSE;
	SDL_SetRelativeMouseMode(SDL_FALSE);
}

void SDLforceUngrabMouse(void)
{
	if (SDL_WasInit(SDL_INIT_VIDEO)==SDL_INIT_VIDEO && window != NULL)
	{
		SDL_ShowCursor(SDL_ENABLE);
		SDL_SetWindowGrab(window, SDL_FALSE);
		wrapmouseok = SDL_FALSE;
		SDL_SetRelativeMouseMode(SDL_FALSE);
	}
}

static void VID_Command_NumModes_f (void)
{
	CONS_Printf(M_GetText("%d video mode(s) available(s)\n"), VID_NumModes());
}

// SDL2 doesn't have SDL_GetVideoSurface or a lot of the SDL_Surface flags that SDL 1.2 had
static void SurfaceInfo(const SDL_Surface *infoSurface, const char *SurfaceText)
{
	INT32 vfBPP;

	if (!infoSurface)
		return;

	if (!SurfaceText)
		SurfaceText = M_GetText("Unknown Surface");

	vfBPP = infoSurface->format?infoSurface->format->BitsPerPixel:0;

	CONS_Printf("\x82" "%s\n", SurfaceText);
	CONS_Printf(M_GetText(" %ix%i at %i bit color\n"), infoSurface->w, infoSurface->h, vfBPP);

	if (infoSurface->flags&SDL_PREALLOC)
		CONS_Printf("%s", M_GetText(" Uses preallocated memory\n"));
	else
		CONS_Printf("%s", M_GetText(" Stored in system memory\n"));
	if (infoSurface->flags&SDL_RLEACCEL)
		CONS_Printf("%s", M_GetText(" Colorkey RLE acceleration blit\n"));
}

static void VID_Command_Info_f (void)
{
#if 0
	SDL2STUB();
#else
#if 0
	const SDL_VideoInfo *videoInfo;
	videoInfo = SDL_GetVideoInfo(); //Alam: Double-Check
	if (videoInfo)
	{
		CONS_Printf("%s", M_GetText("Video Interface Capabilities:\n"));
		if (videoInfo->hw_available)
			CONS_Printf("%s", M_GetText(" Hardware surfaces\n"));
		if (videoInfo->wm_available)
			CONS_Printf("%s", M_GetText(" Window manager\n"));
		//UnusedBits1  :6
		//UnusedBits2  :1
		if (videoInfo->blit_hw)
			CONS_Printf("%s", M_GetText(" Accelerated blits HW-2-HW\n"));
		if (videoInfo->blit_hw_CC)
			CONS_Printf("%s", M_GetText(" Accelerated blits HW-2-HW with Colorkey\n"));
		if (videoInfo->wm_available)
			CONS_Printf("%s", M_GetText(" Accelerated blits HW-2-HW with Alpha\n"));
		if (videoInfo->blit_sw)
		{
			CONS_Printf("%s", M_GetText(" Accelerated blits SW-2-HW\n"));
			if (!M_CheckParm("-noblit")) videoblitok = SDL_TRUE;
		}
		if (videoInfo->blit_sw_CC)
			CONS_Printf("%s", M_GetText(" Accelerated blits SW-2-HW with Colorkey\n"));
		if (videoInfo->blit_sw_A)
			CONS_Printf("%s", M_GetText(" Accelerated blits SW-2-HW with Alpha\n"));
		if (videoInfo->blit_fill)
			CONS_Printf("%s", M_GetText(" Accelerated Color filling\n"));
		//UnusedBits3  :16
		if (videoInfo->video_mem)
			CONS_Printf(M_GetText(" There is %i KB of video memory\n"), videoInfo->video_mem);
		else
			CONS_Printf("%s", M_GetText(" There no video memory for SDL\n"));
		//*vfmt
	}
#else
	if (!M_CheckParm("-noblit")) videoblitok = SDL_TRUE;
#endif
	SurfaceInfo(bufSurface, M_GetText("Current Engine Mode"));
	SurfaceInfo(vidSurface, M_GetText("Current Video Mode"));
#endif
}

static void VID_Command_ModeList_f(void)
{
	// List windowed modes
	INT32 i = 0;
	CONS_Printf("NOTE: Under SDL2, all modes are supported on all platforms.\n");
	CONS_Printf("Under opengl, fullscreen only supports native desktop resolution.\n");
	CONS_Printf("Under software, the mode is stretched up to desktop resolution.\n");
	for (i = 0; i < MAXWINMODES; i++)
	{
		CONS_Printf("%2d: %dx%d\n", i, windowedModes[i][0], windowedModes[i][1]);
	}

}

static void VID_Command_Mode_f (void)
{
	INT32 modenum;

	if (COM_Argc()!= 2)
	{
		CONS_Printf(M_GetText("vid_mode <modenum> : set video mode, current video mode %i\n"), vid.modenum);
		return;
	}

	modenum = atoi(COM_Argv(1));

	if (modenum >= VID_NumModes())
		CONS_Printf(M_GetText("Video mode not present\n"));
	else
		setmodeneeded = modenum+1; // request vid mode change
}

static inline void SDLJoyRemap(event_t *event)
{
	(void)event;
}

static INT32 SDLJoyAxis(const Sint16 axis, evtype_t which)
{
	// -32768 to 32767
	INT32 raxis = axis/32;
	if (which == ev_joystick)
	{
		if (Joystick.bGamepadStyle)
		{
			// gamepad control type, on or off, live or die
			if (raxis < -(JOYAXISRANGE/2))
				raxis = -1;
			else if (raxis > (JOYAXISRANGE/2))
				raxis = 1;
			else
				raxis = 0;
		}
		else
		{
			raxis = JoyInfo.scale!=1?((raxis/JoyInfo.scale)*JoyInfo.scale):raxis;

#ifdef SDL_JDEADZONE
			if (-SDL_JDEADZONE <= raxis && raxis <= SDL_JDEADZONE)
				raxis = 0;
#endif
		}
	}
	else if (which == ev_joystick2)
	{
		if (Joystick2.bGamepadStyle)
		{
			// gamepad control type, on or off, live or die
			if (raxis < -(JOYAXISRANGE/2))
				raxis = -1;
			else if (raxis > (JOYAXISRANGE/2))
				raxis = 1;
			else raxis = 0;
		}
		else
		{
			raxis = JoyInfo2.scale!=1?((raxis/JoyInfo2.scale)*JoyInfo2.scale):raxis;

#ifdef SDL_JDEADZONE
			if (-SDL_JDEADZONE <= raxis && raxis <= SDL_JDEADZONE)
				raxis = 0;
#endif
		}
	}
	else if (which == ev_joystick3)
	{
		if (Joystick3.bGamepadStyle)
		{
			// gamepad control type, on or off, live or die
			if (raxis < -(JOYAXISRANGE/2))
				raxis = -1;
			else if (raxis > (JOYAXISRANGE/2))
				raxis = 1;
			else raxis = 0;
		}
		else
		{
			raxis = JoyInfo3.scale!=1?((raxis/JoyInfo3.scale)*JoyInfo3.scale):raxis;

#ifdef SDL_JDEADZONE
			if (-SDL_JDEADZONE <= raxis && raxis <= SDL_JDEADZONE)
				raxis = 0;
#endif
		}
	}
	else if (which == ev_joystick4)
	{
		if (Joystick4.bGamepadStyle)
		{
			// gamepad control type, on or off, live or die
			if (raxis < -(JOYAXISRANGE/2))
				raxis = -1;
			else if (raxis > (JOYAXISRANGE/2))
				raxis = 1;
			else raxis = 0;
		}
		else
		{
			raxis = JoyInfo4.scale!=1?((raxis/JoyInfo4.scale)*JoyInfo4.scale):raxis;

#ifdef SDL_JDEADZONE
			if (-SDL_JDEADZONE <= raxis && raxis <= SDL_JDEADZONE)
				raxis = 0;
#endif
		}
	}
	return raxis;
}

static void Impl_HandleWindowEvent(SDL_WindowEvent evt)
{
	static SDL_bool firsttimeonmouse = SDL_TRUE;
	static SDL_bool mousefocus = SDL_TRUE;
	static SDL_bool kbfocus = SDL_TRUE;

	switch (evt.event)
	{
		case SDL_WINDOWEVENT_ENTER:
			mousefocus = SDL_TRUE;
			break;
		case SDL_WINDOWEVENT_LEAVE:
			mousefocus = SDL_FALSE;
			break;
		case SDL_WINDOWEVENT_FOCUS_GAINED:
			kbfocus = SDL_TRUE;
			mousefocus = SDL_TRUE;
			break;
		case SDL_WINDOWEVENT_FOCUS_LOST:
			kbfocus = SDL_FALSE;
			mousefocus = SDL_FALSE;
			break;
		case SDL_WINDOWEVENT_MAXIMIZED:
			break;
	}

	if (mousefocus && kbfocus)
	{
		// Tell game we got focus back, resume music if necessary
		window_notinfocus = false;

		S_InitMusicVolume();

		if (cv_gamesounds.value)
			S_EnableSound();

		if (!firsttimeonmouse)
		{
			if (cv_usemouse.value) I_StartupMouse();
		}
		//else firsttimeonmouse = SDL_FALSE;
	}
	else if (!mousefocus && !kbfocus)
	{
		// Tell game we lost focus, pause music
		window_notinfocus = true;
		if (!cv_playmusicifunfocused.value)
			I_SetMusicVolume(0);
		if (!cv_playsoundifunfocused.value)
			S_DisableSound();

		if (!disable_mouse)
		{
			SDLforceUngrabMouse();
		}
		memset(gamekeydown, 0, NUMKEYS); // TODO this is a scary memset

		if (MOUSE_MENU)
		{
			SDLdoUngrabMouse();
		}
	}

}

static void Impl_HandleKeyboardEvent(SDL_KeyboardEvent evt, Uint32 type)
{
	event_t event;
	if (type == SDL_KEYUP)
	{
		event.type = ev_keyup;
	}
	else if (type == SDL_KEYDOWN)
	{
		event.type = ev_keydown;
	}
	else
	{
		return;
	}
	event.data1 = Impl_SDL_Scancode_To_Keycode(evt.keysym.scancode);
	if (event.data1) D_PostEvent(&event);
}

static void Impl_HandleMouseMotionEvent(SDL_MouseMotionEvent evt)
{
	if (USE_MOUSEINPUT)
	{
		if ((SDL_GetMouseFocus() != window && SDL_GetKeyboardFocus() != window))
		{
			SDLdoUngrabMouse();
			return;
		}

		// If using relative mouse mode, don't post an event_t just now,
		// add on the offsets so we can make an overall event later.
		if (SDL_GetRelativeMouseMode())
		{
			if (SDL_GetMouseFocus() == window && SDL_GetKeyboardFocus() == window)
			{
				mousemovex +=  evt.xrel;
				mousemovey += -evt.yrel;
				SDL_SetWindowGrab(window, SDL_TRUE);
			}
			return;
		}

		// If the event is from warping the pointer to middle
		// of the screen then ignore it.
		if ((evt.x == realwidth/2) && (evt.y == realheight/2))
		{
			return;
		}

		// Don't send an event_t if not in relative mouse mode anymore,
		// just grab and set relative mode
		// this fixes the stupid camera jerk on mouse entering bug
		// -- Monster Iestyn
		if (SDL_GetMouseFocus() == window && SDL_GetKeyboardFocus() == window)
		{
			SDLdoGrabMouse();
		}
	}
}

static void Impl_HandleMouseButtonEvent(SDL_MouseButtonEvent evt, Uint32 type)
{
	event_t event;

	SDL_memset(&event, 0, sizeof(event_t));

	// Ignore the event if the mouse is not actually focused on the window.
	// This can happen if you used the mouse to restore keyboard focus;
	// this apparently makes a mouse button down event but not a mouse button up event,
	// resulting in whatever key was pressed down getting "stuck" if we don't ignore it.
	// -- Monster Iestyn (28/05/18)
	if (SDL_GetMouseFocus() != window)
		return;

	/// \todo inputEvent.button.which
	if (USE_MOUSEINPUT)
	{
		if (type == SDL_MOUSEBUTTONUP)
		{
			event.type = ev_keyup;
		}
		else if (type == SDL_MOUSEBUTTONDOWN)
		{
			event.type = ev_keydown;
		}
		else return;
		if (evt.button == SDL_BUTTON_MIDDLE)
			event.data1 = KEY_MOUSE1+2;
		else if (evt.button == SDL_BUTTON_RIGHT)
			event.data1 = KEY_MOUSE1+1;
		else if (evt.button == SDL_BUTTON_LEFT)
			event.data1 = KEY_MOUSE1;
		else if (evt.button == SDL_BUTTON_X1)
			event.data1 = KEY_MOUSE1+3;
		else if (evt.button == SDL_BUTTON_X2)
			event.data1 = KEY_MOUSE1+4;
		if (event.type == ev_keyup || event.type == ev_keydown)
		{
			D_PostEvent(&event);
		}
	}
}

static void Impl_HandleMouseWheelEvent(SDL_MouseWheelEvent evt)
{
	event_t event;

	SDL_memset(&event, 0, sizeof(event_t));

	if (evt.y > 0)
	{
		event.data1 = KEY_MOUSEWHEELUP;
		event.type = ev_keydown;
	}
	if (evt.y < 0)
	{
		event.data1 = KEY_MOUSEWHEELDOWN;
		event.type = ev_keydown;
	}
	if (evt.y == 0)
	{
		event.data1 = 0;
		event.type = ev_keyup;
	}
	if (event.type == ev_keyup || event.type == ev_keydown)
	{
		D_PostEvent(&event);
	}
}

static void Impl_HandleControllerAxisEvent(SDL_ControllerAxisEvent evt)
{
	event_t event;
	SDL_JoystickID joyid[4];
	INT32 value;

	// Determine the Joystick IDs for each current open joystick
	joyid[0] = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(JoyInfo.dev));
	joyid[1] = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(JoyInfo2.dev));
	joyid[2] = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(JoyInfo3.dev));
	joyid[3] = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(JoyInfo4.dev));

	event.data1 = event.data2 = event.data3 = INT32_MAX;

	if (evt.which == joyid[0])
	{
		event.type = ev_joystick;
	}
	else if (evt.which == joyid[1])
	{
		event.type = ev_joystick2;
	}
	else if (evt.which == joyid[2])
	{
		event.type = ev_joystick3;
	}
	else if (evt.which == joyid[3])
	{
		event.type = ev_joystick4;
	}
	else return;
	//axis
	if (evt.axis > JOYAXISSET*2)
		return;
	//vaule
	value = SDLJoyAxis(evt.value, event.type);
	switch (evt.axis)
	{
		case SDL_CONTROLLER_AXIS_LEFTX:
			event.data1 = 0;
			event.data2 = value;
			break;
		case SDL_CONTROLLER_AXIS_LEFTY:
			event.data1 = 0;
			event.data3 = value;
			break;
		case SDL_CONTROLLER_AXIS_RIGHTX:
			event.data1 = 1;
			event.data2 = value;
			break;
		case SDL_CONTROLLER_AXIS_RIGHTY:
			event.data1 = 1;
			event.data3 = value;
			break;
		case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
			event.data1 = 2;
			event.data2 = value;
			break;
		case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
			event.data1 = 2;
			event.data3 = value;
			break;
		default:
			return;
	}
	D_PostEvent(&event);
}

#if 0
static void Impl_HandleJoystickHatEvent(SDL_JoyHatEvent evt)
{
	event_t event;
	SDL_JoystickID joyid[2];

	// Determine the Joystick IDs for each current open joystick
	joyid[0] = SDL_JoystickInstanceID(JoyInfo.dev);
	joyid[1] = SDL_JoystickInstanceID(JoyInfo2.dev);

	if (evt.hat >= JOYHATS)
		return; // ignore hats with too high an index

	if (evt.which == joyid[0])
	{
		event.data1 = KEY_HAT1 + (evt.hat*4);
	}
	else if (evt.which == joyid[1])
	{
		event.data1 = KEY_2HAT1 + (evt.hat*4);
	}
	else return;

	// NOTE: UNFINISHED
}
#endif

static void Impl_HandleControllerButtonEvent(SDL_ControllerButtonEvent evt, Uint32 type)
{
	event_t event;
	SDL_JoystickID joyid[4];

	// Determine the Joystick IDs for each current open joystick
	joyid[0] = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(JoyInfo.dev));
	joyid[1] = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(JoyInfo2.dev));
	joyid[2] = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(JoyInfo3.dev));
	joyid[3] = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(JoyInfo4.dev));

	if (evt.button == SDL_CONTROLLER_BUTTON_DPAD_UP
		|| evt.button == SDL_CONTROLLER_BUTTON_DPAD_DOWN
		|| evt.button == SDL_CONTROLLER_BUTTON_DPAD_LEFT
		|| evt.button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT)
	{
		// dpad buttons are mapped as the hat instead
		return;
	}

	if (evt.which == joyid[0])
	{
		event.data1 = KEY_JOY1;
	}
	else if (evt.which == joyid[1])
	{
		event.data1 = KEY_2JOY1;
	}
	else if (evt.which == joyid[2])
	{
		event.data1 = KEY_3JOY1;
	}
	else if (evt.which == joyid[3])
	{
		event.data1 = KEY_4JOY1;
	}
	else return;
	if (type == SDL_CONTROLLERBUTTONUP)
	{
		event.type = ev_keyup;
	}
	else if (type == SDL_CONTROLLERBUTTONDOWN)
	{
		event.type = ev_keydown;
	}
	else return;
	if (evt.button < JOYBUTTONS)
	{
		event.data1 += evt.button;
	}
	else return;

	SDLJoyRemap(&event);
	if (event.type != ev_console) D_PostEvent(&event);
}



void I_GetEvent(void)
{
	SDL_Event evt;
	// We only want the first motion event,
	// otherwise we'll end up catching the warp back to center.
	//int mouseMotionOnce = 0;

	if (!graphics_started)
	{
		return;
	}

	mousemovex = mousemovey = 0;

	while (SDL_PollEvent(&evt))
	{
		switch (evt.type)
		{
			case SDL_WINDOWEVENT:
				Impl_HandleWindowEvent(evt.window);
				break;
			case SDL_KEYUP:
			case SDL_KEYDOWN:
				Impl_HandleKeyboardEvent(evt.key, evt.type);
				break;
			case SDL_MOUSEMOTION:
				//if (!mouseMotionOnce)
				Impl_HandleMouseMotionEvent(evt.motion);
				//mouseMotionOnce = 1;
				break;
			case SDL_MOUSEBUTTONUP:
			case SDL_MOUSEBUTTONDOWN:
				Impl_HandleMouseButtonEvent(evt.button, evt.type);
				break;
			case SDL_MOUSEWHEEL:
				Impl_HandleMouseWheelEvent(evt.wheel);
				break;
			case SDL_CONTROLLERAXISMOTION:
				Impl_HandleControllerAxisEvent(evt.caxis);
				break;
#if 0
			case SDL_JOYHATMOTION:
				Impl_HandleJoystickHatEvent(evt.jhat)
				break;
#endif
			case SDL_CONTROLLERBUTTONUP:
			case SDL_CONTROLLERBUTTONDOWN:
				Impl_HandleControllerButtonEvent(evt.cbutton, evt.type);
				break;

			////////////////////////////////////////////////////////////

			case SDL_CONTROLLERDEVICEADDED:
				{
					// OH BOY are you in for a good time! #abominationstation

					SDL_GameController *newcontroller = SDL_GameControllerOpen(evt.cdevice.which);

					CONS_Debug(DBG_GAMELOGIC, "Joystick device index %d added\n", evt.jdevice.which + 1);

					////////////////////////////////////////////////////////////
					// Because SDL's device index is unstable, we're going to cheat here a bit:
					// For the first joystick setting that is NOT active:
					//
					// 1. Set cv_usejoystickX.value to the new device index (this does not change what is written to config.cfg)
					//
					// 2. Set OTHERS' cv_usejoystickX.value to THEIR new device index, because it likely changed
					//    * If device doesn't exist, switch cv_usejoystick back to default value (.string)
					//      * BUT: If that default index is being occupied, use ANOTHER cv_usejoystick's default value!
					////////////////////////////////////////////////////////////

					//////////////////////////////
					// PLAYER 1
					//////////////////////////////

					if (newcontroller && (!JoyInfo.dev || !SDL_GameControllerGetAttached(JoyInfo.dev))
						&& JoyInfo2.dev != newcontroller && JoyInfo3.dev != newcontroller && JoyInfo4.dev != newcontroller) // don't override a currently active device
					{
						cv_usejoystick.value = evt.cdevice.which + 1;
						I_UpdateJoystickDeviceIndices(1);
					}

					//////////////////////////////
					// PLAYER 2
					//////////////////////////////

					else if (newcontroller && (!JoyInfo2.dev || !SDL_GameControllerGetAttached(JoyInfo2.dev))
						&& JoyInfo.dev != newcontroller && JoyInfo3.dev != newcontroller && JoyInfo4.dev != newcontroller) // don't override a currently active device
					{
						cv_usejoystick2.value = evt.cdevice.which + 1;
						I_UpdateJoystickDeviceIndices(2);
					}

					//////////////////////////////
					// PLAYER 3
					//////////////////////////////

					else if (newcontroller && (!JoyInfo3.dev || !SDL_GameControllerGetAttached(JoyInfo3.dev))
						&& JoyInfo.dev != newcontroller && JoyInfo2.dev != newcontroller && JoyInfo4.dev != newcontroller) // don't override a currently active device
					{
						cv_usejoystick3.value = evt.cdevice.which + 1;
						I_UpdateJoystickDeviceIndices(3);
					}

					//////////////////////////////
					// PLAYER 4
					//////////////////////////////

					else if (newcontroller && (!JoyInfo4.dev || !SDL_GameControllerGetAttached(JoyInfo4.dev))
						&& JoyInfo.dev != newcontroller && JoyInfo2.dev != newcontroller && JoyInfo3.dev != newcontroller) // don't override a currently active device
					{
						cv_usejoystick4.value = evt.cdevice.which + 1;
						I_UpdateJoystickDeviceIndices(4);
					}

					////////////////////////////////////////////////////////////
					// Was cv_usejoystick disabled in settings?
					////////////////////////////////////////////////////////////

					if (!strcmp(cv_usejoystick.string, "0") || !cv_usejoystick.value)
						cv_usejoystick.value = 0;
					else if (atoi(cv_usejoystick.string) <= I_NumJoys() // don't mess if we intentionally set higher than NumJoys
						     && cv_usejoystick.value) // update the cvar ONLY if a device exists
						CV_SetValue(&cv_usejoystick, cv_usejoystick.value);

					if (!strcmp(cv_usejoystick2.string, "0") || !cv_usejoystick2.value)
						cv_usejoystick2.value = 0;
					else if (atoi(cv_usejoystick2.string) <= I_NumJoys() // don't mess if we intentionally set higher than NumJoys
					         && cv_usejoystick2.value) // update the cvar ONLY if a device exists
						CV_SetValue(&cv_usejoystick2, cv_usejoystick2.value);

					if (!strcmp(cv_usejoystick3.string, "0") || !cv_usejoystick3.value)
						cv_usejoystick3.value = 0;
					else if (atoi(cv_usejoystick3.string) <= I_NumJoys() // don't mess if we intentionally set higher than NumJoys
						&& cv_usejoystick3.value) // update the cvar ONLY if a device exists
						CV_SetValue(&cv_usejoystick3, cv_usejoystick3.value);

					if (!strcmp(cv_usejoystick4.string, "0") || !cv_usejoystick4.value)
						cv_usejoystick4.value = 0;
					else if (atoi(cv_usejoystick4.string) <= I_NumJoys() // don't mess if we intentionally set higher than NumJoys
						&& cv_usejoystick4.value) // update the cvar ONLY if a device exists
						CV_SetValue(&cv_usejoystick4, cv_usejoystick4.value);

					////////////////////////////////////////////////////////////
					// Update all joysticks' init states
					// This is a little wasteful since cv_usejoystick already calls this, but
					// we need to do this in case CV_SetValue did nothing because the string was already same.
					// if the device is already active, this should do nothing, effectively.
					////////////////////////////////////////////////////////////

					I_InitJoystick();
					I_InitJoystick2();
					I_InitJoystick3();
					I_InitJoystick4();

					////////////////////////////////////////////////////////////

					CONS_Debug(DBG_GAMELOGIC, "Joystick1 device index: %d\n", JoyInfo.oldjoy);
					CONS_Debug(DBG_GAMELOGIC, "Joystick2 device index: %d\n", JoyInfo2.oldjoy);
					CONS_Debug(DBG_GAMELOGIC, "Joystick3 device index: %d\n", JoyInfo3.oldjoy);
					CONS_Debug(DBG_GAMELOGIC, "Joystick4 device index: %d\n", JoyInfo4.oldjoy);

					// update the menu
					if (currentMenu == &OP_JoystickSetDef)
						M_SetupJoystickMenu(0);

					if (JoyInfo.dev != newcontroller && JoyInfo2.dev != newcontroller && JoyInfo3.dev != newcontroller && JoyInfo4.dev != newcontroller)
						SDL_GameControllerClose(newcontroller);
				}
				break;

			////////////////////////////////////////////////////////////

			case SDL_CONTROLLERDEVICEREMOVED:
				if (JoyInfo.dev && !SDL_GameControllerGetAttached(JoyInfo.dev))
				{
					CONS_Debug(DBG_GAMELOGIC, "Joystick1 removed, device index: %d\n", JoyInfo.oldjoy);
					I_ShutdownJoystick();
				}

				if (JoyInfo2.dev && !SDL_GameControllerGetAttached(JoyInfo2.dev))
				{
					CONS_Debug(DBG_GAMELOGIC, "Joystick2 removed, device index: %d\n", JoyInfo2.oldjoy);
					I_ShutdownJoystick2();
				}

				if (JoyInfo3.dev && !SDL_GameControllerGetAttached(JoyInfo3.dev))
				{
					CONS_Debug(DBG_GAMELOGIC, "Joystick3 removed, device index: %d\n", JoyInfo3.oldjoy);
					I_ShutdownJoystick3();
				}

				if (JoyInfo4.dev && !SDL_GameControllerGetAttached(JoyInfo4.dev))
				{
					CONS_Debug(DBG_GAMELOGIC, "Joystick4 removed, device index: %d\n", JoyInfo4.oldjoy);
					I_ShutdownJoystick4();
				}

				////////////////////////////////////////////////////////////
				// Update the device indexes, because they likely changed
				// * If device doesn't exist, switch cv_usejoystick back to default value (.string)
				//   * BUT: If that default index is being occupied, use ANOTHER cv_usejoystick's default value!
				////////////////////////////////////////////////////////////

				if (JoyInfo.dev)
					cv_usejoystick.value = JoyInfo.oldjoy = I_GetJoystickDeviceIndex(JoyInfo.dev) + 1;
				else if (atoi(cv_usejoystick.string) != JoyInfo2.oldjoy
					&& atoi(cv_usejoystick.string) != JoyInfo3.oldjoy
					&& atoi(cv_usejoystick.string) != JoyInfo4.oldjoy)
					cv_usejoystick.value = atoi(cv_usejoystick.string);
				else if (atoi(cv_usejoystick2.string) != JoyInfo2.oldjoy
					&& atoi(cv_usejoystick2.string) != JoyInfo3.oldjoy
					&& atoi(cv_usejoystick2.string) != JoyInfo4.oldjoy)
					cv_usejoystick.value = atoi(cv_usejoystick2.string);
				else if (atoi(cv_usejoystick3.string) != JoyInfo2.oldjoy
					&& atoi(cv_usejoystick3.string) != JoyInfo3.oldjoy
					&& atoi(cv_usejoystick3.string) != JoyInfo4.oldjoy)
					cv_usejoystick.value = atoi(cv_usejoystick3.string);
				else if (atoi(cv_usejoystick4.string) != JoyInfo2.oldjoy
					&& atoi(cv_usejoystick4.string) != JoyInfo3.oldjoy
					&& atoi(cv_usejoystick4.string) != JoyInfo4.oldjoy)
					cv_usejoystick.value = atoi(cv_usejoystick4.string);
				else // we tried...
					cv_usejoystick.value = 0;

				if (JoyInfo2.dev)
					cv_usejoystick2.value = JoyInfo2.oldjoy = I_GetJoystickDeviceIndex(JoyInfo2.dev) + 1;
				else if (atoi(cv_usejoystick.string) != JoyInfo.oldjoy
					&& atoi(cv_usejoystick.string) != JoyInfo3.oldjoy
					&& atoi(cv_usejoystick.string) != JoyInfo4.oldjoy)
					cv_usejoystick2.value = atoi(cv_usejoystick.string);
				else if (atoi(cv_usejoystick2.string) != JoyInfo.oldjoy
					&& atoi(cv_usejoystick2.string) != JoyInfo3.oldjoy
					&& atoi(cv_usejoystick2.string) != JoyInfo4.oldjoy)
					cv_usejoystick2.value = atoi(cv_usejoystick2.string);
				else if (atoi(cv_usejoystick3.string) != JoyInfo.oldjoy
					&& atoi(cv_usejoystick3.string) != JoyInfo3.oldjoy
					&& atoi(cv_usejoystick3.string) != JoyInfo4.oldjoy)
					cv_usejoystick2.value = atoi(cv_usejoystick3.string);
				else if (atoi(cv_usejoystick4.string) != JoyInfo.oldjoy
					&& atoi(cv_usejoystick4.string) != JoyInfo3.oldjoy
					&& atoi(cv_usejoystick4.string) != JoyInfo4.oldjoy)
					cv_usejoystick2.value = atoi(cv_usejoystick4.string);
				else // we tried...
					cv_usejoystick2.value = 0;

				if (JoyInfo3.dev)
					cv_usejoystick3.value = JoyInfo3.oldjoy = I_GetJoystickDeviceIndex(JoyInfo3.dev) + 1;
				else if (atoi(cv_usejoystick.string) != JoyInfo.oldjoy
					&& atoi(cv_usejoystick.string) != JoyInfo2.oldjoy
					&& atoi(cv_usejoystick.string) != JoyInfo4.oldjoy)
					cv_usejoystick3.value = atoi(cv_usejoystick.string);
				else if (atoi(cv_usejoystick2.string) != JoyInfo.oldjoy
					&& atoi(cv_usejoystick2.string) != JoyInfo2.oldjoy
					&& atoi(cv_usejoystick2.string) != JoyInfo4.oldjoy)
					cv_usejoystick3.value = atoi(cv_usejoystick2.string);
				else if (atoi(cv_usejoystick3.string) != JoyInfo.oldjoy
					&& atoi(cv_usejoystick3.string) != JoyInfo2.oldjoy
					&& atoi(cv_usejoystick3.string) != JoyInfo4.oldjoy)
					cv_usejoystick3.value = atoi(cv_usejoystick3.string);
				else if (atoi(cv_usejoystick4.string) != JoyInfo.oldjoy
					&& atoi(cv_usejoystick4.string) != JoyInfo2.oldjoy
					&& atoi(cv_usejoystick4.string) != JoyInfo4.oldjoy)
					cv_usejoystick3.value = atoi(cv_usejoystick4.string);
				else // we tried...
					cv_usejoystick3.value = 0;

				if (JoyInfo4.dev)
					cv_usejoystick4.value = JoyInfo4.oldjoy = I_GetJoystickDeviceIndex(JoyInfo4.dev) + 1;
				else if (atoi(cv_usejoystick.string) != JoyInfo.oldjoy
					&& atoi(cv_usejoystick.string) != JoyInfo2.oldjoy
					&& atoi(cv_usejoystick.string) != JoyInfo3.oldjoy)
					cv_usejoystick4.value = atoi(cv_usejoystick.string);
				else if (atoi(cv_usejoystick2.string) != JoyInfo.oldjoy
					&& atoi(cv_usejoystick2.string) != JoyInfo2.oldjoy
					&& atoi(cv_usejoystick2.string) != JoyInfo3.oldjoy)
					cv_usejoystick4.value = atoi(cv_usejoystick2.string);
				else if (atoi(cv_usejoystick3.string) != JoyInfo.oldjoy
					&& atoi(cv_usejoystick3.string) != JoyInfo2.oldjoy
					&& atoi(cv_usejoystick3.string) != JoyInfo3.oldjoy)
					cv_usejoystick4.value = atoi(cv_usejoystick3.string);
				else if (atoi(cv_usejoystick4.string) != JoyInfo.oldjoy
					&& atoi(cv_usejoystick4.string) != JoyInfo2.oldjoy
					&& atoi(cv_usejoystick4.string) != JoyInfo3.oldjoy)
					cv_usejoystick4.value = atoi(cv_usejoystick4.string);
				else // we tried...
					cv_usejoystick4.value = 0;

				////////////////////////////////////////////////////////////
				// Was cv_usejoystick disabled in settings?
				////////////////////////////////////////////////////////////

				if (!strcmp(cv_usejoystick.string, "0"))
					cv_usejoystick.value = 0;
				else if (atoi(cv_usejoystick.string) <= I_NumJoys() // don't mess if we intentionally set higher than NumJoys
						 && cv_usejoystick.value) // update the cvar ONLY if a device exists
					CV_SetValue(&cv_usejoystick, cv_usejoystick.value);

				if (!strcmp(cv_usejoystick2.string, "0"))
					cv_usejoystick2.value = 0;
				else if (atoi(cv_usejoystick2.string) <= I_NumJoys() // don't mess if we intentionally set higher than NumJoys
						 && cv_usejoystick2.value) // update the cvar ONLY if a device exists
					CV_SetValue(&cv_usejoystick2, cv_usejoystick2.value);

				if (!strcmp(cv_usejoystick3.string, "0"))
					cv_usejoystick3.value = 0;
				else if (atoi(cv_usejoystick3.string) <= I_NumJoys() // don't mess if we intentionally set higher than NumJoys
					&& cv_usejoystick3.value) // update the cvar ONLY if a device exists
					CV_SetValue(&cv_usejoystick3, cv_usejoystick3.value);

				if (!strcmp(cv_usejoystick4.string, "0"))
					cv_usejoystick4.value = 0;
				else if (atoi(cv_usejoystick4.string) <= I_NumJoys() // don't mess if we intentionally set higher than NumJoys
					&& cv_usejoystick4.value) // update the cvar ONLY if a device exists
					CV_SetValue(&cv_usejoystick4, cv_usejoystick4.value);

				////////////////////////////////////////////////////////////

				CONS_Debug(DBG_GAMELOGIC, "Joystick1 device index: %d\n", JoyInfo.oldjoy);
				CONS_Debug(DBG_GAMELOGIC, "Joystick2 device index: %d\n", JoyInfo2.oldjoy);
				CONS_Debug(DBG_GAMELOGIC, "Joystick3 device index: %d\n", JoyInfo3.oldjoy);
				CONS_Debug(DBG_GAMELOGIC, "Joystick4 device index: %d\n", JoyInfo4.oldjoy);

				// update the menu
				if (currentMenu == &OP_JoystickSetDef)
					M_SetupJoystickMenu(0);
				break;
			case SDL_QUIT:
				I_Quit();
				M_QuitResponse('y');
				break;
		}
	}

	// Send all relative mouse movement as one single mouse event.
	if (mousemovex || mousemovey)
	{
		event_t event;
		int wwidth, wheight;
		SDL_GetWindowSize(window, &wwidth, &wheight);
		//SDL_memset(&event, 0, sizeof(event_t));
		event.type = ev_mouse;
		event.data1 = 0;
		event.data2 = (INT32)lround(mousemovex * ((float)wwidth / (float)realwidth));
		event.data3 = (INT32)lround(mousemovey * ((float)wheight / (float)realheight));
		D_PostEvent(&event);
	}

	// In order to make wheels act like buttons, we have to set their state to Up.
	// This is because wheel messages don't have an up/down state.
	gamekeydown[KEY_MOUSEWHEELDOWN] = gamekeydown[KEY_MOUSEWHEELUP] = 0;
}

void I_StartupMouse(void)
{
	static SDL_bool firsttimeonmouse = SDL_TRUE;

	if (disable_mouse)
		return;

	if (!firsttimeonmouse)
	{
		HalfWarpMouse(realwidth, realheight); // warp to center
	}
	else
		firsttimeonmouse = SDL_FALSE;
	if (cv_usemouse.value)
		SDLdoGrabMouse();
	else
		SDLdoUngrabMouse();
}

//
// I_OsPolling
//
void I_OsPolling(void)
{
	SDL_Keymod mod;

	if (consolevent)
		I_GetConsoleEvents();
	if (SDL_WasInit(SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) == (SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER))
	{
		SDL_GameControllerUpdate();
		I_GetJoystickEvents();
		I_GetJoystick2Events();
		I_GetJoystick3Events();
		I_GetJoystick4Events();
	}

	I_GetMouseEvents();

	I_GetEvent();

	mod = SDL_GetModState();
	/* Handle here so that our state is always synched with the system. */
	shiftdown = ctrldown = altdown = 0;
	capslock = false;
	if (mod & KMOD_LSHIFT) shiftdown |= 1;
	if (mod & KMOD_RSHIFT) shiftdown |= 2;
	if (mod & KMOD_LCTRL)   ctrldown |= 1;
	if (mod & KMOD_RCTRL)   ctrldown |= 2;
	if (mod & KMOD_LALT)     altdown |= 1;
	if (mod & KMOD_RALT)     altdown |= 2;
	if (mod & KMOD_CAPS) capslock = true;
}

//
// I_UpdateNoBlit
//
void I_UpdateNoBlit(void)
{
	if (rendermode == render_none)
		return;
	if (exposevideo)
	{
#ifdef HWRENDER
		if (rendermode == render_opengl)
		{
			OglSdlFinishUpdate(cv_vidwait.value);
		}
		else
#endif
		if (rendermode == render_soft)
		{
			SDL_RenderCopy(renderer, texture, NULL, NULL);
			SDL_RenderPresent(renderer);
		}
	}
	exposevideo = SDL_FALSE;
}

// I_SkipFrame
//
// Returns true if it thinks we can afford to skip this frame
// from PrBoom's src/SDL/i_video.c
static inline boolean I_SkipFrame(void)
{
#if 1
	// While I fixed the FPS counter bugging out with this,
	// I actually really like being able to pause and
	// use perfstats to measure rendering performance
	// without game logic changes.
	return false;
#else
	static boolean skip = false;

	skip = !skip;

	switch (gamestate)
	{
		case GS_LEVEL:
			if (!paused)
				return false;
			/* FALLTHRU */
		case GS_WAITINGPLAYERS:
			return skip; // Skip odd frames
		default:
			return false;
	}
#endif
}

//
// I_FinishUpdate
//
static SDL_Rect src_rect = { 0, 0, 0, 0 };

void I_FinishUpdate(void)
{
	if (rendermode == render_none)
		return; //Alam: No software or OpenGl surface

	SCR_CalculateFPS();

	if (I_SkipFrame())
		return;

	if (cv_ticrate.value)
		SCR_DisplayTicRate();

	if (cv_showping.value && netgame && consoleplayer != serverplayer)
		SCR_DisplayLocalPing();

#ifdef HAVE_DISCORDRPC
	if (discordRequestList != NULL)
		ST_AskToJoinEnvelope();
#endif

	if (rendermode == render_soft && screens[0])
	{
		if (!bufSurface) //Double-Check
		{
			Impl_VideoSetupSDLBuffer();
		}

		if (bufSurface)
		{
			SDL_BlitSurface(bufSurface, &src_rect, vidSurface, &src_rect);
			// Fury -- there's no way around UpdateTexture, the GL backend uses it anyway
			SDL_LockSurface(vidSurface);
			SDL_UpdateTexture(texture, &src_rect, vidSurface->pixels, vidSurface->pitch);
			SDL_UnlockSurface(vidSurface);
		}

		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, &src_rect, NULL);
		SDL_RenderPresent(renderer);
	}

#ifdef HWRENDER
	else if (rendermode == render_opengl)
	{
		OglSdlFinishUpdate(cv_vidwait.value);
	}
#endif

	exposevideo = SDL_FALSE;
}

//
// I_UpdateNoVsync
//
void I_UpdateNoVsync(void)
{
	INT32 real_vidwait = cv_vidwait.value;
	cv_vidwait.value = 0;
	I_FinishUpdate();
	cv_vidwait.value = real_vidwait;
}

//
// I_ReadScreen
//
void I_ReadScreen(UINT8 *scr)
{
	if (rendermode != render_soft)
		I_Error ("I_ReadScreen: called while in non-software mode");
	else
		VID_BlitLinearScreen(screens[0], scr,
			vid.width*vid.bpp, vid.height,
			vid.rowbytes, vid.rowbytes);
}

//
// I_SetPalette
//
void I_SetPalette(RGBA_t *palette)
{
	size_t i;
	for (i=0; i<256; i++)
	{
		localPalette[i].r = palette[i].s.red;
		localPalette[i].g = palette[i].s.green;
		localPalette[i].b = palette[i].s.blue;
	}
	//if (vidSurface) SDL_SetPaletteColors(vidSurface->format->palette, localPalette, 0, 256);
	// Fury -- SDL2 vidSurface is a 32-bit surface buffer copied to the texture. It's not palletized, like bufSurface.
	if (bufSurface) SDL_SetPaletteColors(bufSurface->format->palette, localPalette, 0, 256);
}

// return number of fullscreen + X11 modes
INT32 VID_NumModes(void)
{
	if (USE_FULLSCREEN && numVidModes != -1)
		return numVidModes - firstEntry;
	else
		return MAXWINMODES;
}

const char *VID_GetModeName(INT32 modeNum)
{
#if 0
	if (USE_FULLSCREEN && numVidModes != -1) // fullscreen modes
	{
		modeNum += firstEntry;
		if (modeNum >= numVidModes)
			return NULL;

		sprintf(&vidModeName[modeNum][0], "%dx%d",
			modeList[modeNum]->w,
			modeList[modeNum]->h);
	}
	else // windowed modes
	{
#endif
	if (modeNum == -1)
	{
		return fallback_resolution_name;
	}
		if (modeNum > MAXWINMODES)
			return NULL;

		sprintf(&vidModeName[modeNum][0], "%dx%d",
			windowedModes[modeNum][0],
			windowedModes[modeNum][1]);
	//}
	return &vidModeName[modeNum][0];
}

INT32 VID_GetModeForSize(INT32 w, INT32 h)
{
	int i;
	for (i = 0; i < MAXWINMODES; i++)
	{
		if (windowedModes[i][0] == w && windowedModes[i][1] == h)
		{
			return i;
		}
	}
	return -1;
#if 0
	INT32 matchMode = -1, i;
	VID_PrepareModeList();
	if (USE_FULLSCREEN && numVidModes != -1)
	{
		for (i=firstEntry; i<numVidModes; i++)
		{
			if (modeList[i]->w == w &&
			    modeList[i]->h == h)
			{
				matchMode = i;
				break;
			}
		}
		if (-1 == matchMode) // use smaller mode
		{
			w -= w%BASEVIDWIDTH;
			h -= h%BASEVIDHEIGHT;
			for (i=firstEntry; i<numVidModes; i++)
			{
				if (modeList[i]->w == w &&
				    modeList[i]->h == h)
				{
					matchMode = i;
					break;
				}
			}
			if (-1 == matchMode) // use smallest mode
				matchMode = numVidModes-1;
		}
		matchMode -= firstEntry;
	}
	else
	{
		for (i=0; i<MAXWINMODES; i++)
		{
			if (windowedModes[i][0] == w &&
			    windowedModes[i][1] == h)
			{
				matchMode = i;
				break;
			}
		}
		if (-1 == matchMode) // use smaller mode
		{
			w -= w%BASEVIDWIDTH;
			h -= h%BASEVIDHEIGHT;
			for (i=0; i<MAXWINMODES; i++)
			{
				if (windowedModes[i][0] == w &&
				    windowedModes[i][1] == h)
				{
					matchMode = i;
					break;
				}
			}
			if (-1 == matchMode) // use smallest mode
				matchMode = MAXWINMODES-1;
		}
	}
	return matchMode;
#endif
}

void VID_PrepareModeList(void)
{
	// Under SDL2, we just use the windowed modes list, and scale in windowed fullscreen.
	allow_fullscreen = true;
#if 0
	INT32 i;

	firstEntry = 0;

#ifdef HWRENDER
	if (rendermode == render_opengl)
		modeList = SDL_ListModes(NULL, SDL_OPENGL|SDL_FULLSCREEN);
	else
#endif
	modeList = SDL_ListModes(NULL, surfaceFlagsF|SDL_HWSURFACE); //Alam: At least hardware surface

	if (disable_fullscreen?0:cv_fullscreen.value) // only fullscreen needs preparation
	{
		if (-1 != numVidModes)
		{
			for (i=0; i<numVidModes; i++)
			{
				if (modeList[i]->w <= MAXVIDWIDTH &&
					modeList[i]->h <= MAXVIDHEIGHT)
				{
					firstEntry = i;
					break;
				}
			}
		}
	}
	allow_fullscreen = true;
#endif
}

static UINT32 refresh_rate;
static UINT32 VID_GetRefreshRate(void)
{
	int index = SDL_GetWindowDisplayIndex(window);
	SDL_DisplayMode m;

	if (SDL_WasInit(SDL_INIT_VIDEO) == 0)
	{
		// Video not init yet.
		return 0;
	}

	if (SDL_GetCurrentDisplayMode(index, &m) != 0)
	{
		// Error has occurred.
		return 0;
	}

	return m.refresh_rate;
}

INT32 VID_SetMode(INT32 modeNum)
{
	SDLdoUngrabMouse();

	vid.recalc = 1;
	vid.bpp = 1;

	if (modeNum >= 0 && modeNum < MAXWINMODES)
	{
		vid.width = windowedModes[modeNum][0];
		vid.height = windowedModes[modeNum][1];
		vid.modenum = modeNum;
	}
	else
	{
		// just set the desktop resolution as a fallback
		SDL_DisplayMode mode;
		SDL_GetWindowDisplayMode(window, &mode);
		if (mode.w >= 2048)
		{
			vid.width = 1920;
			vid.height = 1200;
		}
		else
		{
			vid.width = mode.w;
			vid.height = mode.h;
		}
		vid.modenum = -1;
	}
	//Impl_SetWindowName("SRB2Kart "VERSIONSTRING);

	SDLSetMode(vid.width, vid.height, USE_FULLSCREEN);
	Impl_VideoSetupBuffer();

	if (rendermode == render_soft)
	{
		if (bufSurface)
		{
			SDL_FreeSurface(bufSurface);
			bufSurface = NULL;
		}
	}

	src_rect.w = vid.width;
	src_rect.h = vid.height;

	refresh_rate = VID_GetRefreshRate();

	return SDL_TRUE;
}

static SDL_bool Impl_CreateWindow(SDL_bool fullscreen)
{
	int flags = 0;

	if (rendermode == render_none) // dedicated
		return SDL_TRUE; // Monster Iestyn -- not sure if it really matters what we return here tbh

	if (window != NULL)
		return SDL_FALSE;

	if (fullscreen)
		flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

	if (borderlesswindow)
		flags |= SDL_WINDOW_BORDERLESS;

#ifdef HWRENDER
	if (rendermode == render_opengl)
		flags |= SDL_WINDOW_OPENGL;
#endif

	// Create a window
	window = SDL_CreateWindow("SRB2Kart "VERSIONSTRING, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			realwidth, realheight, flags);

	if (window == NULL)
	{
		CONS_Printf(M_GetText("Couldn't create window: %s\n"), SDL_GetError());
		return SDL_FALSE;
	}

	// Renderer-specific stuff
#ifdef HWRENDER
	if (rendermode == render_opengl)
	{
		sdlglcontext = SDL_GL_CreateContext(window);
		if (sdlglcontext == NULL)
		{
			SDL_DestroyWindow(window);
			I_Error("Failed to create a GL context: %s\n", SDL_GetError());
		}
		SDL_GL_MakeCurrent(window, sdlglcontext);
	}
	else
#endif
	if (rendermode == render_soft)
	{
		flags = 0; // Use this to set SDL_RENDERER_* flags now
		if (usesdl2soft)
			flags |= SDL_RENDERER_SOFTWARE;
		else if (cv_vidwait.value)
			flags |= SDL_RENDERER_PRESENTVSYNC;

		// 3 August 2022
		// Possibly a Windows 11 issue; the default
		// "direct3d" driver (D3D9) causes Drmingw exchndl
		// to not write RPT files. Every other driver
		// seems fine.
		SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");

		renderer = SDL_CreateRenderer(window, -1, flags);
		if (renderer == NULL)
		{
			CONS_Printf(M_GetText("Couldn't create rendering context: %s\n"), SDL_GetError());
			return SDL_FALSE;
		}
		SDL_RenderSetLogicalSize(renderer, BASEVIDWIDTH, BASEVIDHEIGHT);
	}

	return SDL_TRUE;
}

/*
static void Impl_SetWindowName(const char *title)
{
	if (window == NULL)
	{
		return;
	}
	SDL_SetWindowTitle(window, title);
}
*/

static void Impl_SetWindowIcon(void)
{
	if (window == NULL || icoSurface == NULL)
	{
		return;
	}
	//SDL2STUB(); // Monster Iestyn: why is this stubbed?
	SDL_SetWindowIcon(window, icoSurface);
}

static void Impl_VideoSetupSDLBuffer(void)
{
	if (bufSurface != NULL)
	{
		SDL_FreeSurface(bufSurface);
		bufSurface = NULL;
	}
	// Set up the SDL palletized buffer (copied to vidbuffer before being rendered to texture)
	if (vid.bpp == 1)
	{
		bufSurface = SDL_CreateRGBSurfaceFrom(screens[0],vid.width,vid.height,8,
			(int)vid.rowbytes,0x00000000,0x00000000,0x00000000,0x00000000); // 256 mode
	}
	else if (vid.bpp == 2) // Fury -- don't think this is used at all anymore
	{
		bufSurface = SDL_CreateRGBSurfaceFrom(screens[0],vid.width,vid.height,15,
			(int)vid.rowbytes,0x00007C00,0x000003E0,0x0000001F,0x00000000); // 555 mode
	}
	if (bufSurface)
	{
		SDL_SetPaletteColors(bufSurface->format->palette, localPalette, 0, 256);
	}
	else
	{
		I_Error("%s", M_GetText("No system memory for SDL buffer surface\n"));
	}
}

static void Impl_VideoSetupBuffer(void)
{
	// Set up game's software render buffer
	//if (rendermode == render_soft)
	{
		vid.rowbytes = vid.width * vid.bpp;
		vid.direct = NULL;
		if (vid.buffer)
			free(vid.buffer);
		vid.buffer = calloc(vid.rowbytes*vid.height, NUMSCREENS);
		if (!vid.buffer)
		{
			I_Error("%s", M_GetText("Not enough memory for video buffer\n"));
		}
	}
}

static FILE *
OpenRendererFile (const char * mode)
{
	char * path = va(pandf,srb2home,"renderer.txt");
	return fopen(path, mode);
}

void I_StartupGraphics(void)
{
	if (dedicated)
	{
		rendermode = render_none;
		return;
	}
	if (graphics_started)
		return;

	COM_AddCommand ("vid_nummodes", VID_Command_NumModes_f);
	COM_AddCommand ("vid_info", VID_Command_Info_f);
	COM_AddCommand ("vid_modelist", VID_Command_ModeList_f);
	COM_AddCommand ("vid_mode", VID_Command_Mode_f);
	CV_RegisterVar (&cv_vidwait);
	CV_RegisterVar (&cv_stretch);
	disable_mouse = M_CheckParm("-nomouse");
	disable_fullscreen = M_CheckParm("-win") ? 1 : 0;

	keyboard_started = true;

#if !defined(HAVE_TTF)
	// Previously audio was init here for questionable reasons?
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
	{
		CONS_Printf(M_GetText("Couldn't initialize SDL's Video System: %s\n"), SDL_GetError());
		return;
	}
#endif
	{
		const char *vd = SDL_GetCurrentVideoDriver();
		//CONS_Printf(M_GetText("Starting up with video driver: %s\n"), vd);
		if (vd && (
			strncasecmp(vd, "gcvideo", 8) == 0 ||
			strncasecmp(vd, "fbcon", 6) == 0 ||
			strncasecmp(vd, "wii", 4) == 0 ||
			strncasecmp(vd, "psl1ght", 8) == 0
		))
			framebuffer = SDL_TRUE;
	}
	if (M_CheckParm("-software"))
		rendermode = render_soft;
#ifdef HWRENDER
	else if (M_CheckParm("-opengl"))
		rendermode = render_opengl;
#endif

	if (rendermode == render_none)
	{
#ifdef HWRENDER
		char   line[16];
		char * word;
		FILE * file = OpenRendererFile("r");
		if (file != NULL)
		{
			if (fgets(line, sizeof line, file) != NULL)
			{
				word = strtok(line, "\n");

				if (strcasecmp(word, "software") == 0)
				{
					rendermode = render_soft;
				}
				else if (strcasecmp(word, "opengl") == 0)
				{
					rendermode = render_opengl;
				}

				if (rendermode != render_none)
				{
					CONS_Printf("Using last known renderer: %s\n", line);
				}
			}
			fclose(file);
		}
#endif
		if (rendermode == render_none)
		{
			rendermode = render_soft;
			CONS_Printf("Using default software renderer.\n");
		}
	}
	else
	{
		FILE * file = OpenRendererFile("w");
		if (file != NULL)
		{
			if (rendermode == render_soft)
			{
				fputs("software\n", file);
			}
			else if (rendermode == render_opengl)
			{
				fputs("opengl\n", file);
			}
			fclose(file);
		}
		else
		{
			CONS_Printf("Could not save renderer to file: %s\n", strerror(errno));
		}
	}

	usesdl2soft = M_CheckParm("-softblit");
	borderlesswindow = M_CheckParm("-borderless");

	//SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY>>1,SDL_DEFAULT_REPEAT_INTERVAL<<2);
	VID_Command_ModeList_f();
#ifdef HWRENDER
	if (rendermode == render_opengl)
	{
		HWD.pfnInit             = hwSym("Init",NULL);
		HWD.pfnFinishUpdate     = NULL;
		HWD.pfnDraw2DLine       = hwSym("Draw2DLine",NULL);
		HWD.pfnDrawPolygon      = hwSym("DrawPolygon",NULL);
		HWD.pfnSetBlend         = hwSym("SetBlend",NULL);
		HWD.pfnClearBuffer      = hwSym("ClearBuffer",NULL);
		HWD.pfnSetTexture       = hwSym("SetTexture",NULL);
		HWD.pfnReadRect         = hwSym("ReadRect",NULL);
		HWD.pfnGClipRect        = hwSym("GClipRect",NULL);
		HWD.pfnClearMipMapCache = hwSym("ClearMipMapCache",NULL);
		HWD.pfnSetSpecialState  = hwSym("SetSpecialState",NULL);
		HWD.pfnSetPalette       = hwSym("SetPalette",NULL);
		HWD.pfnGetTextureUsed   = hwSym("GetTextureUsed",NULL);
		HWD.pfnDrawModel        = hwSym("DrawModel",NULL);
		HWD.pfnCreateModelVBOs  = hwSym("CreateModelVBOs",NULL);
		HWD.pfnSetTransform     = hwSym("SetTransform",NULL);
		HWD.pfnPostImgRedraw    = hwSym("PostImgRedraw",NULL);
		HWD.pfnFlushScreenTextures=hwSym("FlushScreenTextures",NULL);
		HWD.pfnStartScreenWipe  = hwSym("StartScreenWipe",NULL);
		HWD.pfnEndScreenWipe    = hwSym("EndScreenWipe",NULL);
		HWD.pfnDoScreenWipe     = hwSym("DoScreenWipe",NULL);
		HWD.pfnDrawIntermissionBG=hwSym("DrawIntermissionBG",NULL);
		HWD.pfnMakeScreenTexture= hwSym("MakeScreenTexture",NULL);
		HWD.pfnMakeScreenFinalTexture=hwSym("MakeScreenFinalTexture",NULL);
		HWD.pfnDrawScreenFinalTexture=hwSym("DrawScreenFinalTexture",NULL);

		HWD.pfnRenderSkyDome = hwSym("RenderSkyDome",NULL);

		HWD.pfnLoadShaders = hwSym("LoadShaders",NULL);
		HWD.pfnKillShaders = hwSym("KillShaders",NULL);
		HWD.pfnSetShader = hwSym("SetShader",NULL);
		HWD.pfnUnSetShader = hwSym("UnSetShader",NULL);

		HWD.pfnLoadCustomShader = hwSym("LoadCustomShader",NULL);
		HWD.pfnInitCustomShaders = hwSym("InitCustomShaders",NULL);

		HWD.pfnStartBatching = hwSym("StartBatching",NULL);
		HWD.pfnRenderBatches = hwSym("RenderBatches",NULL);

		if (!HWD.pfnInit()) // load the OpenGL library
			rendermode = render_soft;
	}
#endif

	// Fury: we do window initialization after GL setup to allow
	// SDL_GL_LoadLibrary to work well on Windows

	// Create window
	//Impl_CreateWindow(USE_FULLSCREEN);
	//Impl_SetWindowName("SRB2Kart "VERSIONSTRING);
	VID_SetMode(VID_GetModeForSize(BASEVIDWIDTH, BASEVIDHEIGHT));

	vid.width = BASEVIDWIDTH; // Default size for startup
	vid.height = BASEVIDHEIGHT; // BitsPerPixel is the SDL interface's
	vid.recalc = true; // Set up the console stufff
	vid.direct = NULL; // Maybe direct access?
	vid.bpp = 1; // This is the game engine's Bpp
	vid.WndParent = NULL; //For the window?

#ifdef HAVE_TTF
	I_ShutdownTTF();
#endif
	// Window icon
#ifdef HAVE_IMAGE
	icoSurface = IMG_ReadXPMFromArray(SDL_icon_xpm);
#endif
	Impl_SetWindowIcon();

	VID_SetMode(VID_GetModeForSize(BASEVIDWIDTH, BASEVIDHEIGHT));

	if (M_CheckParm("-nomousegrab"))
		mousegrabok = SDL_FALSE;
#if 0 // defined (_DEBUG)
	else
	{
		char videodriver[4] = {'S','D','L',0};
		if (!M_CheckParm("-mousegrab") &&
		    *strncpy(videodriver, SDL_GetCurrentVideoDriver(), 4) != '\0' &&
		    strncasecmp("x11",videodriver,4) == 0)
			mousegrabok = SDL_FALSE; //X11's XGrabPointer not good
	}
#endif
	realwidth = (Uint16)vid.width;
	realheight = (Uint16)vid.height;

	VID_Command_Info_f();
	SDLdoUngrabMouse();

	SDL_RaiseWindow(window);

	if (mousegrabok && !disable_mouse)
	{
		SDL_ShowCursor(SDL_DISABLE);
		SDL_SetRelativeMouseMode(SDL_TRUE);
		wrapmouseok = SDL_TRUE;
		SDL_SetWindowGrab(window, SDL_TRUE);
	}

	graphics_started = true;
}

void I_ShutdownGraphics(void)
{
	const rendermode_t oldrendermode = rendermode;

	rendermode = render_none;
	if (icoSurface) SDL_FreeSurface(icoSurface);
	icoSurface = NULL;
	if (oldrendermode == render_soft)
	{
		if (vidSurface) SDL_FreeSurface(vidSurface);
		vidSurface = NULL;
		if (vid.buffer) free(vid.buffer);
		vid.buffer = NULL;
		if (bufSurface) SDL_FreeSurface(bufSurface);
		bufSurface = NULL;
	}

	I_OutputMsg("I_ShutdownGraphics(): ");

	// was graphics initialized anyway?
	if (!graphics_started)
	{
		I_OutputMsg("graphics never started\n");
		return;
	}
	graphics_started = false;
	I_OutputMsg("shut down\n");

#ifdef HWRENDER
	if (GLUhandle)
		hwClose(GLUhandle);
	if (sdlglcontext)
	{
		SDL_GL_DeleteContext(sdlglcontext);
	}
#endif
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	framebuffer = SDL_FALSE;
}

UINT32 I_GetRefreshRate(void)
{
	// Moved to VID_GetRefreshRate.
	// Precalculating it like that won't work as
	// well for windowed mode since you can drag
	// the window around, but very slow PCs might have
	// trouble querying mode over and over again.
	return refresh_rate;
}

static void Impl_SetVsync(void)
{
#if SDL_VERSION_ATLEAST(2,0,18)
	if (renderer)
		SDL_RenderSetVSync(renderer, cv_vidwait.value);
#endif
}
#endif

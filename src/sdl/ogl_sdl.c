// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
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
//
//-----------------------------------------------------------------------------
/// \file
/// \brief SDL specific part of the OpenGL API for SRB2

#ifdef _MSC_VER
#pragma warning(disable : 4214 4244)
#endif

#define _MATH_DEFINES_DEFINED

#include "SDL.h"

#include "sdlmain.h"

#ifdef _MSC_VER
#pragma warning(default : 4214 4244)
#endif

#include "../d_main.h"
#include "../doomdef.h"

#include "../hardware/hw_main.h"
#include "../hardware/r_opengl/r_opengl.h"
#include "../i_system.h"
#include "../m_argv.h"
#include "hwsym_sdl.h"
#include "ogl_sdl.h"

#ifdef DEBUG_TO_FILE
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#ifdef USE_WGL_SWAP
PFNWGLEXTSWAPCONTROLPROC wglSwapIntervalEXT = NULL;
#else
typedef int (*PFNGLXSWAPINTERVALPROC)(int);
PFNGLXSWAPINTERVALPROC glXSwapIntervalSGIEXT = NULL;
#endif

#ifndef STATIC_OPENGL
PFNglClear pglClear;
PFNglGetIntegerv pglGetIntegerv;
PFNglGetString pglGetString;
#endif

/**	\brief SDL video display surface
 */
INT32 oglflags = 0;
void *GLUhandle = NULL;
SDL_GLContext sdlglcontext = 0;

void *GetGLFunc(const char *proc) {
  if (strncmp(proc, "glu", 3) == 0) {
    if (GLUhandle)
      return hwSym(proc, GLUhandle);
    else
      return NULL;
  }
  return SDL_GL_GetProcAddress(proc);
}

boolean LoadGL(void) {
#ifndef STATIC_OPENGL
  const char *OGLLibname = NULL;
  const char *GLULibname = NULL;

  if (M_CheckParm("-OGLlib") && M_IsNextParm())
    OGLLibname = M_GetNextParm();

  if (SDL_GL_LoadLibrary(OGLLibname) != 0) {
    CONS_Alert(CONS_ERROR,
               "Could not load OpenGL Library: %s\n"
               "Falling back to Software mode.\n",
               SDL_GetError());
    if (!M_CheckParm("-OGLlib"))
      CONS_Printf(
          "If you know what is the OpenGL library's name, use -OGLlib\n");
    return 0;
  }

#if 0
	GLULibname = "/proc/self/exe";
#elif defined(__MACH__)
  GLULibname =
      "/System/Library/Frameworks/OpenGL.framework/Libraries/libGLU.dylib";
#elif defined(__unix__)
  GLULibname = "libGLU.so.1";
#elif defined(__HAIKU__)
  GLULibname = "libGLU.so";
#else
  GLULibname = NULL;
#endif

  if (M_CheckParm("-GLUlib") && M_IsNextParm())
    GLULibname = M_GetNextParm();

  if (GLULibname) {
    GLUhandle = hwOpen(GLULibname);
    if (GLUhandle)
      return SetupGLfunc();
    else {
      CONS_Alert(CONS_ERROR, "Could not load GLU Library: %s\n", GLULibname);
      if (!M_CheckParm("-GLUlib"))
        CONS_Printf(
            "If you know what is the GLU library's name, use -GLUlib\n");
    }
  } else {
    CONS_Alert(CONS_ERROR, "Could not load GLU Library\n");
    CONS_Printf("If you know what is the GLU library's name, use -GLUlib\n");
  }
#endif
  return SetupGLfunc();
}

/**	\brief	The OglSdlSurface function

        \param	w	width
        \param	h	height
        \param	isFullscreen	if true, go fullscreen

        \return	if true, changed video mode
*/
boolean OglSdlSurface(INT32 w, INT32 h) {
  INT32 cbpp = cv_scr_depth.value < 16 ? 16 : cv_scr_depth.value;
  static boolean first_init = false;
  const char *gllogdir = NULL;

  oglflags = 0;

  if (!first_init) {
    if (!gllogstream) {
      gllogdir = D_Home();

#ifdef DEBUG_TO_FILE
#ifdef DEFAULTDIR
      if (gllogdir)
        gllogstream = fopen(va("%s/" DEFAULTDIR "/ogllog.txt", gllogdir), "wt");
      else
#endif
        gllogstream = fopen("./ogllog.txt", "wt");
#endif
    }

    gl_version = pglGetString(GL_VERSION);
    gl_renderer = pglGetString(GL_RENDERER);
    gl_extensions = pglGetString(GL_EXTENSIONS);

    GL_DBG_Printf("OpenGL %s\n", gl_version);
    GL_DBG_Printf("GPU: %s\n", gl_renderer);
    GL_DBG_Printf("Extensions: %s\n", gl_extensions);
  }
  first_init = true;

  if (isExtAvailable("GL_EXT_texture_filter_anisotropic", gl_extensions))
    pglGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maximumAnisotropy);
  else
    maximumAnisotropy = 1;

  SetupGLFunc4();

  granisotropicmode_cons_t[1].value = maximumAnisotropy;

  SDL_GL_SetSwapInterval(cv_vidwait.value ? 1 : 0);

  SetModelView(w, h);
  SetStates();
  pglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  HWR_Startup();
  textureformatGL = cbpp > 16 ? GL_RGBA : GL_RGB5_A1;

  return true;
}

/**	\brief	The OglSdlFinishUpdate function

        \param	vidwait	wait for video sync

        \return	void
*/
void OglSdlFinishUpdate(boolean waitvbl) {
  static boolean oldwaitvbl = false;
  int sdlw, sdlh;
  if (oldwaitvbl != waitvbl) {
    SDL_GL_SetSwapInterval(waitvbl ? 1 : 0);
  }

  oldwaitvbl = waitvbl;

  SDL_GetWindowSize(window, &sdlw, &sdlh);

  HWR_MakeScreenFinalTexture();
  HWR_DrawScreenFinalTexture(sdlw, sdlh);
  SDL_GL_SwapWindow(window);

  GClipRect(0, 0, realwidth, realheight, NZCLIP_PLANE);

  // Sryder:	We need to draw the final screen texture again into the other
  // buffer in the original position so that
  //			effects that want to take the old screen can do so after
  // this
  HWR_DrawScreenFinalTexture(realwidth, realheight);
}

EXPORT void HWRAPI(OglSdlSetPalette)(RGBA_t *palette, RGBA_t *pgamma) {
  INT32 i = -1;
  UINT32 redgamma = pgamma->s.red, greengamma = pgamma->s.green,
         bluegamma = pgamma->s.blue;

  for (i = 0; i < 256; i++) {
    myPaletteData[i].s.red =
        (UINT8)MIN((palette[i].s.red * redgamma) / 127, 255);
    myPaletteData[i].s.green =
        (UINT8)MIN((palette[i].s.green * greengamma) / 127, 255);
    myPaletteData[i].s.blue =
        (UINT8)MIN((palette[i].s.blue * bluegamma) / 127, 255);
    myPaletteData[i].s.alpha = palette[i].s.alpha;
  }
  Flush();
}

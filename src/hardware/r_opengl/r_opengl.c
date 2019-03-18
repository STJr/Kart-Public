// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 1999-2019 by Sonic Team Junior.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file
/// \brief OpenGL API for Sonic Robo Blast 2

#if defined (_WIN32)
//#define WIN32_LEAN_AND_MEAN
#define RPC_NO_WINDOWS_H
#include <windows.h>
#endif
#undef GETTEXT
#ifdef __GNUC__
#include <unistd.h>
#endif

#include <stdarg.h>
#include <math.h>
#include "r_opengl.h"

#if defined (HWRENDER) && !defined (NOROPENGL)
// for KOS: GL_TEXTURE_ENV, glAlphaFunc, glColorMask, glPolygonOffset, glReadPixels, GL_ALPHA_TEST, GL_POLYGON_OFFSET_FILL

struct GLRGBAFloat
{
	GLfloat red;
	GLfloat green;
	GLfloat blue;
	GLfloat alpha;
};
typedef struct GLRGBAFloat GLRGBAFloat;

// ==========================================================================
//                                                                  CONSTANTS
// ==========================================================================

// With OpenGL 1.1+, the first texture should be 1
#define NOTEXTURE_NUM     1     // small white texture
#define FIRST_TEX_AVAIL   (NOTEXTURE_NUM + 1)

#define      N_PI_DEMI               (M_PIl/2.0f) //(1.5707963268f)

#define      ASPECT_RATIO            (1.0f)  //(320.0f/200.0f)
#define      FAR_CLIPPING_PLANE      32768.0f // Draw further! Tails 01-21-2001
static float NEAR_CLIPPING_PLANE =   NZCLIP_PLANE;

// **************************************************************************
//                                                                    GLOBALS
// **************************************************************************


static  GLuint      NextTexAvail    = FIRST_TEX_AVAIL;
static  GLuint      tex_downloaded  = 0;
static  GLfloat     fov             = 90.0f;
static  FBITFIELD   CurrentPolyFlags;

static  FTextureInfo*  gr_cachetail = NULL;
static  FTextureInfo*  gr_cachehead = NULL;

RGBA_t  myPaletteData[256];
GLint   screen_width    = 0;               // used by Draw2DLine()
GLint   screen_height   = 0;
GLbyte  screen_depth    = 0;
GLint   textureformatGL = 0;
GLint maximumAnisotropy = 0;
static GLboolean MipMap = GL_FALSE;
static GLint min_filter = GL_LINEAR;
static GLint mag_filter = GL_LINEAR;
static GLint anisotropic_filter = 0;
static FTransform  md2_transform;

const GLubyte *gl_extensions = NULL;

//Hurdler: 04/10/2000: added for the kick ass coronas as Boris wanted;-)
static GLfloat modelMatrix[16];
static GLfloat projMatrix[16];
static GLint viewport[4];

#ifdef USE_PALETTED_TEXTURE
	PFNGLCOLORTABLEEXTPROC  glColorTableEXT = NULL;
	GLubyte                 palette_tex[256*3];
#endif

// Yay for arbitrary  numbers! NextTexAvail is buggy for some reason.
// Sryder:	NextTexAvail is broken for these because palette changes or changes to the texture filter or antialiasing
//			flush all of the stored textures, leaving them unavailable at times such as between levels
//			These need to start at 0 and be set to their number, and be reset to 0 when deleted so that Intel GPUs
//			can know when the textures aren't there, as textures are always considered resident in their virtual memory
// TODO:	Store them in a more normal way
#define SCRTEX_SCREENTEXTURE 65535
#define SCRTEX_STARTSCREENWIPE 65534
#define SCRTEX_ENDSCREENWIPE 65533
#define SCRTEX_FINALSCREENTEXTURE 65532
static GLuint screentexture = 0;
static GLuint startScreenWipe = 0;
static GLuint endScreenWipe = 0;
static GLuint finalScreenTexture = 0;

// shortcut for ((float)1/i)
static const GLfloat byte2float[256] = {
	0.000000f, 0.003922f, 0.007843f, 0.011765f, 0.015686f, 0.019608f, 0.023529f, 0.027451f,
	0.031373f, 0.035294f, 0.039216f, 0.043137f, 0.047059f, 0.050980f, 0.054902f, 0.058824f,
	0.062745f, 0.066667f, 0.070588f, 0.074510f, 0.078431f, 0.082353f, 0.086275f, 0.090196f,
	0.094118f, 0.098039f, 0.101961f, 0.105882f, 0.109804f, 0.113725f, 0.117647f, 0.121569f,
	0.125490f, 0.129412f, 0.133333f, 0.137255f, 0.141176f, 0.145098f, 0.149020f, 0.152941f,
	0.156863f, 0.160784f, 0.164706f, 0.168627f, 0.172549f, 0.176471f, 0.180392f, 0.184314f,
	0.188235f, 0.192157f, 0.196078f, 0.200000f, 0.203922f, 0.207843f, 0.211765f, 0.215686f,
	0.219608f, 0.223529f, 0.227451f, 0.231373f, 0.235294f, 0.239216f, 0.243137f, 0.247059f,
	0.250980f, 0.254902f, 0.258824f, 0.262745f, 0.266667f, 0.270588f, 0.274510f, 0.278431f,
	0.282353f, 0.286275f, 0.290196f, 0.294118f, 0.298039f, 0.301961f, 0.305882f, 0.309804f,
	0.313726f, 0.317647f, 0.321569f, 0.325490f, 0.329412f, 0.333333f, 0.337255f, 0.341176f,
	0.345098f, 0.349020f, 0.352941f, 0.356863f, 0.360784f, 0.364706f, 0.368627f, 0.372549f,
	0.376471f, 0.380392f, 0.384314f, 0.388235f, 0.392157f, 0.396078f, 0.400000f, 0.403922f,
	0.407843f, 0.411765f, 0.415686f, 0.419608f, 0.423529f, 0.427451f, 0.431373f, 0.435294f,
	0.439216f, 0.443137f, 0.447059f, 0.450980f, 0.454902f, 0.458824f, 0.462745f, 0.466667f,
	0.470588f, 0.474510f, 0.478431f, 0.482353f, 0.486275f, 0.490196f, 0.494118f, 0.498039f,
	0.501961f, 0.505882f, 0.509804f, 0.513726f, 0.517647f, 0.521569f, 0.525490f, 0.529412f,
	0.533333f, 0.537255f, 0.541177f, 0.545098f, 0.549020f, 0.552941f, 0.556863f, 0.560784f,
	0.564706f, 0.568627f, 0.572549f, 0.576471f, 0.580392f, 0.584314f, 0.588235f, 0.592157f,
	0.596078f, 0.600000f, 0.603922f, 0.607843f, 0.611765f, 0.615686f, 0.619608f, 0.623529f,
	0.627451f, 0.631373f, 0.635294f, 0.639216f, 0.643137f, 0.647059f, 0.650980f, 0.654902f,
	0.658824f, 0.662745f, 0.666667f, 0.670588f, 0.674510f, 0.678431f, 0.682353f, 0.686275f,
	0.690196f, 0.694118f, 0.698039f, 0.701961f, 0.705882f, 0.709804f, 0.713726f, 0.717647f,
	0.721569f, 0.725490f, 0.729412f, 0.733333f, 0.737255f, 0.741177f, 0.745098f, 0.749020f,
	0.752941f, 0.756863f, 0.760784f, 0.764706f, 0.768627f, 0.772549f, 0.776471f, 0.780392f,
	0.784314f, 0.788235f, 0.792157f, 0.796078f, 0.800000f, 0.803922f, 0.807843f, 0.811765f,
	0.815686f, 0.819608f, 0.823529f, 0.827451f, 0.831373f, 0.835294f, 0.839216f, 0.843137f,
	0.847059f, 0.850980f, 0.854902f, 0.858824f, 0.862745f, 0.866667f, 0.870588f, 0.874510f,
	0.878431f, 0.882353f, 0.886275f, 0.890196f, 0.894118f, 0.898039f, 0.901961f, 0.905882f,
	0.909804f, 0.913726f, 0.917647f, 0.921569f, 0.925490f, 0.929412f, 0.933333f, 0.937255f,
	0.941177f, 0.945098f, 0.949020f, 0.952941f, 0.956863f, 0.960784f, 0.964706f, 0.968628f,
	0.972549f, 0.976471f, 0.980392f, 0.984314f, 0.988235f, 0.992157f, 0.996078f, 1.000000f
};

// -----------------+
// GL_DBG_Printf    : Output debug messages to debug log if DEBUG_TO_FILE is defined,
//                  : else do nothing
// Returns          :
// -----------------+

#ifdef DEBUG_TO_FILE
FILE *gllogstream;
#endif

FUNCPRINTF void GL_DBG_Printf(const char *format, ...)
{
#ifdef DEBUG_TO_FILE
	char str[4096] = "";
	va_list arglist;

	if (!gllogstream)
		gllogstream = fopen("ogllog.txt", "w");

	va_start(arglist, format);
	vsnprintf(str, 4096, format, arglist);
	va_end(arglist);

	fwrite(str, strlen(str), 1, gllogstream);
#else
	(void)format;
#endif
}

#ifdef STATIC_OPENGL
/* 1.0 functions */
/* Miscellaneous */
#define pglClearColor glClearColor
//glClear
#define pglColorMask glColorMask
#define pglAlphaFunc glAlphaFunc
#define pglBlendFunc glBlendFunc
#define pglCullFace glCullFace
#define pglPolygonOffset glPolygonOffset
#define pglScissor glScissor
#define pglEnable glEnable
#define pglDisable glDisable
#define pglGetFloatv glGetFloatv
//glGetIntegerv
//glGetString

/* Depth Buffer */
#define pglClearDepth glClearDepth
#define pglDepthFunc glDepthFunc
#define pglDepthMask glDepthMask
#define pglDepthRange glDepthRange

/* Transformation */
#define pglMatrixMode glMatrixMode
#define pglViewport glViewport
#define pglPushMatrix glPushMatrix
#define pglPopMatrix glPopMatrix
#define pglLoadIdentity glLoadIdentity
#define pglMultMatrixd glMultMatrixd
#define pglRotatef glRotatef
#define pglScalef glScalef
#define pglTranslatef glTranslatef

/* Drawing Functions */
#define pglBegin glBegin
#define pglEnd glEnd
#define pglVertex3f glVertex3f
#define pglNormal3f glNormal3f
#define pglColor4f glColor4f
#define pglColor4fv glColor4fv
#define pglTexCoord2f glTexCoord2f

/* Lighting */
#define pglShadeModel glShadeModel
#define pglLightfv glLightfv
#define pglLightModelfv glLightModelfv
#define pglMaterialfv glMaterialfv

/* Raster functions */
#define pglPixelStorei glPixelStorei
#define pglReadPixels glReadPixels

/* Texture mapping */
#define pglTexEnvi glTexEnvi
#define pglTexParameteri glTexParameteri
#define pglTexImage2D glTexImage2D

/* Fog */
#define pglFogf glFogf
#define pglFogfv glFogfv

/* 1.1 functions */
/* texture objects */ //GL_EXT_texture_object
#define pglDeleteTextures glDeleteTextures
#define pglBindTexture glBindTexture
/* texture mapping */ //GL_EXT_copy_texture
#define pglCopyTexImage2D glCopyTexImage2D
#define pglCopyTexSubImage2D glCopyTexSubImage2D

#else //!STATIC_OPENGL

/* 1.0 functions */
/* Miscellaneous */
typedef void (APIENTRY * PFNglClearColor) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
static PFNglClearColor pglClearColor;
//glClear
typedef void (APIENTRY * PFNglColorMask) (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
static PFNglColorMask pglColorMask;
typedef void (APIENTRY * PFNglAlphaFunc) (GLenum func, GLclampf ref);
static PFNglAlphaFunc pglAlphaFunc;
typedef void (APIENTRY * PFNglBlendFunc) (GLenum sfactor, GLenum dfactor);
static PFNglBlendFunc pglBlendFunc;
typedef void (APIENTRY * PFNglCullFace) (GLenum mode);
static PFNglCullFace pglCullFace;
typedef void (APIENTRY * PFNglPolygonOffset) (GLfloat factor, GLfloat units);
static PFNglPolygonOffset pglPolygonOffset;
typedef void (APIENTRY * PFNglScissor) (GLint x, GLint y, GLsizei width, GLsizei height);
static PFNglScissor pglScissor;
typedef void (APIENTRY * PFNglEnable) (GLenum cap);
static PFNglEnable pglEnable;
typedef void (APIENTRY * PFNglDisable) (GLenum cap);
static PFNglDisable pglDisable;
typedef void (APIENTRY * PFNglGetFloatv) (GLenum pname, GLfloat *params);
static PFNglGetFloatv pglGetFloatv;
//glGetIntegerv
//glGetString

/* Depth Buffer */
typedef void (APIENTRY * PFNglClearDepth) (GLclampd depth);
static PFNglClearDepth pglClearDepth;
typedef void (APIENTRY * PFNglDepthFunc) (GLenum func);
static PFNglDepthFunc pglDepthFunc;
typedef void (APIENTRY * PFNglDepthMask) (GLboolean flag);
static PFNglDepthMask pglDepthMask;
typedef void (APIENTRY * PFNglDepthRange) (GLclampd near_val, GLclampd far_val);
static PFNglDepthRange pglDepthRange;

/* Transformation */
typedef void (APIENTRY * PFNglMatrixMode) (GLenum mode);
static PFNglMatrixMode pglMatrixMode;
typedef void (APIENTRY * PFNglViewport) (GLint x, GLint y, GLsizei width, GLsizei height);
static PFNglViewport pglViewport;
typedef void (APIENTRY * PFNglPushMatrix) (void);
static PFNglPushMatrix pglPushMatrix;
typedef void (APIENTRY * PFNglPopMatrix) (void);
static PFNglPopMatrix pglPopMatrix;
typedef void (APIENTRY * PFNglLoadIdentity) (void);
static PFNglLoadIdentity pglLoadIdentity;
typedef void (APIENTRY * PFNglMultMatrixf) (const GLfloat *m);
static PFNglMultMatrixf pglMultMatrixf;
typedef void (APIENTRY * PFNglRotatef) (GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
static PFNglRotatef pglRotatef;
typedef void (APIENTRY * PFNglScalef) (GLfloat x, GLfloat y, GLfloat z);
static PFNglScalef pglScalef;
typedef void (APIENTRY * PFNglTranslatef) (GLfloat x, GLfloat y, GLfloat z);
static PFNglTranslatef pglTranslatef;

/* Drawing Functions */
typedef void (APIENTRY * PFNglBegin) (GLenum mode);
static PFNglBegin pglBegin;
typedef void (APIENTRY * PFNglEnd) (void);
static PFNglEnd pglEnd;
typedef void (APIENTRY * PFNglVertex3f) (GLfloat x, GLfloat y, GLfloat z);
static PFNglVertex3f pglVertex3f;
typedef void (APIENTRY * PFNglNormal3f) (GLfloat x, GLfloat y, GLfloat z);
static PFNglNormal3f pglNormal3f;
typedef void (APIENTRY * PFNglColor4f) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
static PFNglColor4f pglColor4f;
typedef void (APIENTRY * PFNglColor4fv) (const GLfloat *v);
static PFNglColor4fv pglColor4fv;
typedef void (APIENTRY * PFNglTexCoord2f) (GLfloat s, GLfloat t);
static PFNglTexCoord2f pglTexCoord2f;

/* Lighting */
typedef void (APIENTRY * PFNglShadeModel) (GLenum mode);
static PFNglShadeModel pglShadeModel;
typedef void (APIENTRY * PFNglLightfv) (GLenum light, GLenum pname, GLfloat *params);
static PFNglLightfv pglLightfv;
typedef void (APIENTRY * PFNglLightModelfv) (GLenum pname, GLfloat *params);
static PFNglLightModelfv pglLightModelfv;
typedef void (APIENTRY * PFNglMaterialfv) (GLint face, GLenum pname, GLfloat *params);
static PFNglMaterialfv pglMaterialfv;

/* Raster functions */
typedef void (APIENTRY * PFNglPixelStorei) (GLenum pname, GLint param);
static PFNglPixelStorei pglPixelStorei;
typedef void (APIENTRY  * PFNglReadPixels) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
static PFNglReadPixels pglReadPixels;

/* Texture mapping */
typedef void (APIENTRY * PFNglTexEnvi) (GLenum target, GLenum pname, GLint param);
static PFNglTexEnvi pglTexEnvi;
typedef void (APIENTRY * PFNglTexParameteri) (GLenum target, GLenum pname, GLint param);
static PFNglTexParameteri pglTexParameteri;
typedef void (APIENTRY * PFNglTexImage2D) (GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
static PFNglTexImage2D pglTexImage2D;

/* Fog */
typedef void (APIENTRY * PFNglFogf) (GLenum pname, GLfloat param);
static PFNglFogf pglFogf;
typedef void (APIENTRY * PFNglFogfv) (GLenum pname, const GLfloat *params);
static PFNglFogfv pglFogfv;

/* 1.1 functions */
/* texture objects */ //GL_EXT_texture_object
typedef void (APIENTRY * PFNglDeleteTextures) (GLsizei n, const GLuint *textures);
static PFNglDeleteTextures pglDeleteTextures;
typedef void (APIENTRY * PFNglBindTexture) (GLenum target, GLuint texture);
static PFNglBindTexture pglBindTexture;
/* texture mapping */ //GL_EXT_copy_texture
typedef void (APIENTRY * PFNglCopyTexImage2D) (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
static PFNglCopyTexImage2D pglCopyTexImage2D;
typedef void (APIENTRY * PFNglCopyTexSubImage2D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
static PFNglCopyTexSubImage2D pglCopyTexSubImage2D;
#endif
/* GLU functions */
typedef GLint (APIENTRY * PFNgluBuild2DMipmaps) (GLenum target, GLint internalFormat, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *data);
static PFNgluBuild2DMipmaps pgluBuild2DMipmaps;

/* 1.3 functions for multitexturing */
typedef void (APIENTRY *PFNglActiveTexture) (GLenum);
static PFNglActiveTexture pglActiveTexture;
typedef void (APIENTRY *PFNglMultiTexCoord2f) (GLenum, GLfloat, GLfloat);
static PFNglMultiTexCoord2f pglMultiTexCoord2f;

/* 1.2 Parms */
/* GL_CLAMP_TO_EDGE_EXT */
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif
#ifndef GL_TEXTURE_MIN_LOD
#define GL_TEXTURE_MIN_LOD 0x813A
#endif
#ifndef GL_TEXTURE_MAX_LOD
#define GL_TEXTURE_MAX_LOD 0x813B
#endif

/* 1.3 GL_TEXTUREi */
#ifndef GL_TEXTURE0
#define GL_TEXTURE0 0x84C0
#endif
#ifndef GL_TEXTURE1
#define GL_TEXTURE1 0x84C1
#endif

boolean SetupGLfunc(void)
{
#ifndef STATIC_OPENGL
#define GETOPENGLFUNC(func, proc) \
	func = GetGLFunc(#proc); \
	if (!func) \
	{ \
		GL_DBG_Printf("failed to get OpenGL function: %s", #proc); \
	} \

	GETOPENGLFUNC(pglClearColor, glClearColor)

	GETOPENGLFUNC(pglClear, glClear)
	GETOPENGLFUNC(pglColorMask, glColorMask)
	GETOPENGLFUNC(pglAlphaFunc, glAlphaFunc)
	GETOPENGLFUNC(pglBlendFunc, glBlendFunc)
	GETOPENGLFUNC(pglCullFace, glCullFace)
	GETOPENGLFUNC(pglPolygonOffset, glPolygonOffset)
	GETOPENGLFUNC(pglScissor, glScissor)
	GETOPENGLFUNC(pglEnable, glEnable)
	GETOPENGLFUNC(pglDisable, glDisable)
	GETOPENGLFUNC(pglGetFloatv, glGetFloatv)
	GETOPENGLFUNC(pglGetIntegerv, glGetIntegerv)
	GETOPENGLFUNC(pglGetString, glGetString)

	GETOPENGLFUNC(pglClearDepth, glClearDepth)
	GETOPENGLFUNC(pglDepthFunc, glDepthFunc)
	GETOPENGLFUNC(pglDepthMask, glDepthMask)
	GETOPENGLFUNC(pglDepthRange, glDepthRange)

	GETOPENGLFUNC(pglMatrixMode, glMatrixMode)
	GETOPENGLFUNC(pglViewport, glViewport)
	GETOPENGLFUNC(pglPushMatrix, glPushMatrix)
	GETOPENGLFUNC(pglPopMatrix, glPopMatrix)
	GETOPENGLFUNC(pglLoadIdentity, glLoadIdentity)
	GETOPENGLFUNC(pglMultMatrixf, glMultMatrixf)
	GETOPENGLFUNC(pglRotatef, glRotatef)
	GETOPENGLFUNC(pglScalef, glScalef)
	GETOPENGLFUNC(pglTranslatef, glTranslatef)

	GETOPENGLFUNC(pglBegin, glBegin)
	GETOPENGLFUNC(pglEnd, glEnd)
	GETOPENGLFUNC(pglVertex3f, glVertex3f)
	GETOPENGLFUNC(pglNormal3f, glNormal3f)
	GETOPENGLFUNC(pglColor4f, glColor4f)
	GETOPENGLFUNC(pglColor4fv, glColor4fv)
	GETOPENGLFUNC(pglTexCoord2f, glTexCoord2f)

	GETOPENGLFUNC(pglShadeModel, glShadeModel)
	GETOPENGLFUNC(pglLightfv, glLightfv)
	GETOPENGLFUNC(pglLightModelfv, glLightModelfv)
	GETOPENGLFUNC(pglMaterialfv, glMaterialfv)

	GETOPENGLFUNC(pglPixelStorei, glPixelStorei)
	GETOPENGLFUNC(pglReadPixels, glReadPixels)

	GETOPENGLFUNC(pglTexEnvi, glTexEnvi)
	GETOPENGLFUNC(pglTexParameteri, glTexParameteri)
	GETOPENGLFUNC(pglTexImage2D, glTexImage2D)

	GETOPENGLFUNC(pglFogf, glFogf)
	GETOPENGLFUNC(pglFogfv, glFogfv)

	GETOPENGLFUNC(pglDeleteTextures, glDeleteTextures)
	GETOPENGLFUNC(pglBindTexture, glBindTexture)

	GETOPENGLFUNC(pglCopyTexImage2D, glCopyTexImage2D)
	GETOPENGLFUNC(pglCopyTexSubImage2D, glCopyTexSubImage2D)

#undef GETOPENGLFUNC
#endif
	return true;
}

// jtc

// glstate
static INT32 glstate_fog_mode = 0;
static float glstate_fog_density = 0;

// glEXT
boolean GLEXT_legacy = false;
boolean GLEXT_shaders = false;

// hw_glob.h
INT32 gl_leveltime = 0;

//#define GL_RETAINED_MODE			// Immediate mode is faster. Go fucking figure.
#define USE_SHADERS

typedef void (APIENTRY *PFNglDrawArrays) (GLenum, GLint, GLsizei);
typedef void (APIENTRY *PFNglDrawElements) (GLenum, GLsizei, GLenum, const GLvoid*);
typedef void (APIENTRY *PFNglVertexAttribPointer) (GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid*);
typedef void (APIENTRY *PFNglEnableClientState) (GLenum);
typedef void (APIENTRY *PFNglDisableClientState) (GLenum);
typedef void (APIENTRY *PFNglBindVertexArray) (GLuint);
typedef void (APIENTRY *PFNglGenVertexArrays) (GLsizei, GLuint*);
typedef void (APIENTRY *PFNglGenBuffers) (GLsizei, GLuint*);
typedef void (APIENTRY *PFNglBindBuffer) (GLenum, GLuint);
typedef void (APIENTRY *PFNglBufferData) (GLenum, GLsizeiptr, GLvoid*, GLenum);
typedef void (APIENTRY *PFNglBufferSubData) (GLenum, GLintptr, GLsizeiptr, GLvoid*);
typedef void (APIENTRY *PFNglDeleteBuffers) (GLsizei, GLuint*);
typedef void (APIENTRY *PFNglDeleteVertexArrays) (GLsizei, GLuint*);
typedef void (APIENTRY *PFNglEnableVertexAttribArray) (GLuint);
typedef void (APIENTRY *PFNglDisableVertexAttribArray) (GLuint);

static PFNglDrawArrays pglDrawArrays;
static PFNglDrawElements pglDrawElements;
static PFNglVertexAttribPointer pglVertexAttribPointer;
static PFNglEnableClientState pglEnableClientState;
static PFNglDisableClientState pglDisableClientState;
static PFNglBindVertexArray pglBindVertexArray;
static PFNglGenVertexArrays pglGenVertexArrays;
static PFNglGenBuffers pglGenBuffers;
static PFNglBindBuffer pglBindBuffer;
static PFNglBufferData pglBufferData;
static PFNglBufferSubData pglBufferSubData;
static PFNglDeleteBuffers pglDeleteBuffers;
static PFNglDeleteVertexArrays pglDeleteVertexArrays;
static PFNglEnableVertexAttribArray pglEnableVertexAttribArray;
static PFNglDisableVertexAttribArray pglDisableVertexAttribArray;

typedef void (APIENTRY *PFNglVertexPointer) (GLint, GLenum, GLsizei, const GLvoid*);
typedef void (APIENTRY *PFNglNormalPointer) (GLenum, GLsizei, const GLvoid*);
typedef void (APIENTRY *PFNglTexCoordPointer) (GLint, GLenum, GLsizei, const GLvoid*);

static PFNglVertexPointer pglVertexPointer;
static PFNglNormalPointer pglNormalPointer;
static PFNglTexCoordPointer pglTexCoordPointer;

// shaders
#ifdef USE_SHADERS

#define MAXSHADERS 16
#define MAXSHADERPROGRAMS 16

static GLuint gl_shaders[MAXSHADERS];
static GLint gl_totalshaders = 0;

static boolean gl_shadersenabled = false;
static GLuint gl_currentshaderprogram = 0;
static GLuint gl_shaderprograms[MAXSHADERPROGRAMS];

typedef GLuint 	(APIENTRY *PFNglCreateShader)		(GLenum);
typedef void 	(APIENTRY *PFNglShaderSource)		(GLuint, GLsizei, GLchar**, GLint*);
typedef void 	(APIENTRY *PFNglCompileShader)		(GLuint);
typedef void 	(APIENTRY *PFNglGetShaderiv)		(GLuint, GLenum, GLint*);
typedef void 	(APIENTRY *PFNglDeleteShader)		(GLuint);
typedef GLuint 	(APIENTRY *PFNglCreateProgram)		(void);
typedef void 	(APIENTRY *PFNglAttachShader)		(GLuint, GLuint);
typedef void 	(APIENTRY *PFNglLinkProgram)		(GLuint);
typedef void 	(APIENTRY *PFNglGetProgramiv)		(GLuint, GLenum, GLint*);
typedef void 	(APIENTRY *PFNglUseProgram)			(GLuint);
typedef void 	(APIENTRY *PFNglUniform1i)			(GLint, GLint);
typedef void 	(APIENTRY *PFNglUniform1f)			(GLint, GLfloat);
typedef void 	(APIENTRY *PFNglUniform2f)			(GLint, GLfloat, GLfloat);
typedef void 	(APIENTRY *PFNglUniform3f)			(GLint, GLfloat, GLfloat, GLfloat);
typedef void 	(APIENTRY *PFNglUniform4f)			(GLint, GLfloat, GLfloat, GLfloat, GLfloat);
typedef void 	(APIENTRY *PFNglUniform1fv)			(GLint, GLsizei, const GLfloat*);
typedef void 	(APIENTRY *PFNglUniform2fv)			(GLint, GLsizei, const GLfloat*);
typedef void 	(APIENTRY *PFNglUniform3fv)			(GLint, GLsizei, const GLfloat*);
typedef GLint 	(APIENTRY *PFNglGetUniformLocation)	(GLuint, GLchar*);

static PFNglCreateShader pglCreateShader;
static PFNglShaderSource pglShaderSource;
static PFNglCompileShader pglCompileShader;
static PFNglGetShaderiv pglGetShaderiv;
static PFNglDeleteShader pglDeleteShader;
static PFNglCreateProgram pglCreateProgram;
static PFNglAttachShader pglAttachShader;
static PFNglLinkProgram pglLinkProgram;
static PFNglGetProgramiv pglGetProgramiv;
static PFNglUseProgram pglUseProgram;
static PFNglUniform1i pglUniform1i;
static PFNglUniform1f pglUniform1f;
static PFNglUniform2f pglUniform2f;
static PFNglUniform3f pglUniform3f;
static PFNglUniform4f pglUniform4f;
static PFNglUniform1fv pglUniform1fv;
static PFNglUniform2fv pglUniform2fv;
static PFNglUniform3fv pglUniform3fv;
static PFNglGetUniformLocation pglGetUniformLocation;

// Macro to reduce boilerplate code
#define GLSL_SHARED_FOG_FUNCTION \
	"float fog(const float dist, const float density, const float globaldensity) {\n" \
		"const float LOG2 = -1.442695;\n" \
		"float d = density * dist;\n" \
		"return 1.0 - clamp(exp(d * globaldensity * LOG2), 0.0, 1.0);\n" \
	"}\n" \
	"float fog2(const float dist, const float density,  const float globaldensity) {\n" \
		"const float LOG2 = -1.442695;\n" \
		"float d = density * dist;\n" \
		"return 1.0 - clamp(exp2(d * d * globaldensity * LOG2), 0.0, 1.0);\n" \
	"}\n"

// Macro to reduce boilerplate code
#define GLSL_SHARED_FOG_MIX \
	"float fog_distance = gl_FragCoord.z / gl_FragCoord.w;\n" \
	"float fog_attenuation = fog(fog_distance, 0.0001 * ((256-lighting)/24), fog_density);\n" \
	"if (fog_mode == 2)\n" \
		"fog_attenuation = floor(fog_attenuation*10)/10;\n" \
	"vec4 fog_color = vec4(fade_color[0], fade_color[1], fade_color[2], 1.0);\n" \
	"vec4 mixed_color = color * mix_color;\n" \
	"vec4 final_color = mix(mixed_color, fog_color, fog_attenuation);\n" \
	"final_color[3] = mixed_color[3];\n"

// Macro to reduce boilerplate code
#define SHARED_FRAGMENT_SHADER \
	"uniform sampler2D tex;\n" \
	"uniform vec4 mix_color;\n" \
	"uniform vec4 fade_color;\n" \
	"uniform float lighting;\n" \
	"uniform int fog_mode;\n" \
	"uniform float fog_density;\n" \
	GLSL_SHARED_FOG_FUNCTION \
	"void main(void) {\n" \
		"vec4 color = texture2D(tex, gl_TexCoord[0].st);\n" \
		"if (fog_mode == 0)\n" \
			"gl_FragColor = color * mix_color;\n" \
		"else\n" \
		"{\n" \
			GLSL_SHARED_FOG_MIX \
			"gl_FragColor = final_color;\n" \
		"}\n" \
	"}\0"

static char *fragment_shaders[] = {
	// Default shader
	"uniform sampler2D tex;\n"
	"uniform vec4 mix_color;\n"
	"void main(void) {\n"
		"vec4 color = texture2D(tex, gl_TexCoord[0].st);\n"
		"vec4 mixed_color = color * mix_color;\n"
		"gl_FragColor = mixed_color;\n"
	"}\0",

	// Floor shader
	SHARED_FRAGMENT_SHADER,

	// Wall shader
	SHARED_FRAGMENT_SHADER,

	// Sprite shader
	SHARED_FRAGMENT_SHADER,

	// Water shader
	"uniform sampler2D tex;\n"
	"uniform vec4 mix_color;\n"
	"uniform vec4 fade_color;\n"
	"uniform float lighting;\n"
	"uniform int fog_mode;\n"
	"uniform int fog_density;\n"
	"uniform float leveltime;\n"

	GLSL_SHARED_FOG_FUNCTION

	"void main(void) {\n"
		"float texU = gl_TexCoord[0].s;\n"
		"float texV = gl_TexCoord[0].t;\n"
		"float wtofs = leveltime/16;\n"
		"float pi = 3.14159265358979323846;\n"
		"texU += cos(pi * 2.0 * (texV/4 + wtofs * 0.125)) * 0.3;\n"
		"texV += sin(pi * 2.0 * (texV/6 + wtofs * 0.125)) * 0.4;\n"
		"vec4 color = texture2D(tex, vec2(texU, texV));\n"

		GLSL_SHARED_FOG_MIX

		"gl_FragColor = final_color;\n"
	"}\0",

	// Sky shader
	"uniform sampler2D tex;\n"
	"uniform vec2 resolution;\n"
	"void main(void) {\n"
		"float texU = gl_TexCoord[0].s;\n"
		"float texV = gl_TexCoord[0].t;\n"
		//"float scale = abs(gl_FragCoord.x - (resolution.x/2.0));\n"
		//"scale /= resolution.x;\n"
		"gl_FragColor = texture2D(tex, vec2(texU, texV));\n"
	"}\0",
};

// Macro to reduce boilerplate code
#define BUFFER_OFFSET(i) ((void*)(i))

#endif	// USE_SHADERS

static GLuint gl_vertexarrayobject;
static GLuint gl_vertexbuffer;

void SetupGLFunc4(void)
{
	pglActiveTexture = GetGLFunc("glActiveTexture");
	pglMultiTexCoord2f = GetGLFunc("glMultiTexCoord2f");

	// 4.x
	pglDrawArrays = GetGLFunc("glDrawArrays");
	pglDrawElements = GetGLFunc("glDrawElements");
	pglVertexAttribPointer = GetGLFunc("glVertexAttribPointer");
	pglEnableClientState = GetGLFunc("glEnableClientState");
	pglDisableClientState = GetGLFunc("glDisableClientState");
	pglBindVertexArray = GetGLFunc("glBindVertexArray");
	pglGenVertexArrays = GetGLFunc("glGenVertexArrays");
	pglGenBuffers = GetGLFunc("glGenBuffers");
	pglBindBuffer = GetGLFunc("glBindBuffer");
	pglBufferData = GetGLFunc("glBufferData");
	pglBufferSubData = GetGLFunc("glBufferSubData");
	pglDeleteBuffers = GetGLFunc("glDeleteBuffers");
	pglDeleteVertexArrays = GetGLFunc("glDeleteVertexArrays");
	pglEnableVertexAttribArray = GetGLFunc("glEnableVertexAttribArray");
	pglDisableVertexAttribArray = GetGLFunc("glDisableVertexAttribArray");

	pglVertexPointer = GetGLFunc("glVertexPointer");
	pglNormalPointer = GetGLFunc("glNormalPointer");
	pglTexCoordPointer = GetGLFunc("glTexCoordPointer");

#ifdef USE_SHADERS
	pglCreateShader = GetGLFunc("glCreateShader");
	pglShaderSource = GetGLFunc("glShaderSource");
	pglCompileShader = GetGLFunc("glCompileShader");
	pglGetShaderiv = GetGLFunc("glGetShaderiv");
	pglDeleteShader = GetGLFunc("glDeleteShader");
	pglCreateProgram = GetGLFunc("glCreateProgram");
	pglAttachShader = GetGLFunc("glAttachShader");
	pglLinkProgram = GetGLFunc("glLinkProgram");
	pglGetProgramiv = GetGLFunc("glGetProgramiv");
	pglUseProgram = GetGLFunc("glUseProgram");
	pglUniform1i = GetGLFunc("glUniform1i");
	pglUniform1f = GetGLFunc("glUniform1f");
	pglUniform2f = GetGLFunc("glUniform2f");
	pglUniform3f = GetGLFunc("glUniform3f");
	pglUniform4f = GetGLFunc("glUniform4f");
	pglUniform1fv = GetGLFunc("glUniform1fv");
	pglUniform2fv = GetGLFunc("glUniform2fv");
	pglUniform3fv = GetGLFunc("glUniform3fv");
	pglGetUniformLocation = GetGLFunc("glGetUniformLocation");
#endif

	// GLU
	pgluBuild2DMipmaps = GetGLFunc("gluBuild2DMipmaps");
}

// jimita
EXPORT void HWRAPI(LoadShaders) (void)
{
#ifdef USE_SHADERS
	GLuint gl_fragShader;
	GLint i, result;

	for (i = 0; fragment_shaders[i]; i++)
	{
		GLchar* shader = fragment_shaders[i];
		if (i >= MAXSHADERS || i >= MAXSHADERPROGRAMS)
			break;

		gl_fragShader = gl_shaders[gl_totalshaders++] = pglCreateShader(GL_FRAGMENT_SHADER);
		if (!gl_fragShader)
			I_Error("Hardware driver: Error creating fragment shader %d", i);

		pglShaderSource(gl_fragShader, 1, &shader, NULL);
		pglCompileShader(gl_fragShader);

		// check for compile errors
		pglGetShaderiv(gl_fragShader, GL_COMPILE_STATUS, &result);
		if (result == GL_FALSE)
			I_Error("Hardware driver: Error compiling fragment shader %d", i);

		gl_shaderprograms[i] = pglCreateProgram();
		pglAttachShader(gl_shaderprograms[i], gl_fragShader);
		pglLinkProgram(gl_shaderprograms[i]);

		// check link status
		pglGetProgramiv(gl_shaderprograms[i], GL_LINK_STATUS, &result);
		if (result != GL_TRUE)
			I_Error("Hardware driver: Error linking shader program %d", i);
	}
#endif
}

EXPORT void HWRAPI(SetShader) (int shader)
{
#ifdef USE_SHADERS
	if (GLEXT_shaders)
	{
		gl_shadersenabled = true;
		gl_currentshaderprogram = shader;
	}
#endif
}

EXPORT void HWRAPI(UnSetShader) (void)
{
#ifdef USE_SHADERS
	gl_shadersenabled = false;
	gl_currentshaderprogram = 0;
#endif
}

EXPORT void HWRAPI(KillShaders) (void)
{
#ifdef USE_SHADERS
	GLint total_shaders = gl_totalshaders;
	GLint i;

	if (!total_shaders)
		return;

	for (i = 0; i < total_shaders; i++)
	{
		pglDeleteShader(gl_shaders[i]);
		gl_shaders[i] = 0;
		gl_totalshaders--;
	}
#endif
}

// -----------------+
// SetNoTexture     : Disable texture
// -----------------+
static void SetNoTexture(void)
{
	// Set small white texture.
	if (tex_downloaded != NOTEXTURE_NUM)
	{
		pglBindTexture(GL_TEXTURE_2D, NOTEXTURE_NUM);
		tex_downloaded = NOTEXTURE_NUM;
	}
}

static void GLPerspective(GLfloat fovy, GLfloat aspect)
{
	GLfloat m[4][4] =
	{
		{ 1.0f, 0.0f, 0.0f, 0.0f},
		{ 0.0f, 1.0f, 0.0f, 0.0f},
		{ 0.0f, 0.0f, 1.0f,-1.0f},
		{ 0.0f, 0.0f, 0.0f, 0.0f},
	};
	const GLfloat zNear = NEAR_CLIPPING_PLANE;
	const GLfloat zFar = FAR_CLIPPING_PLANE;
	const GLfloat radians = (GLfloat)(fovy / 2.0f * M_PIl / 180.0f);
	const GLfloat sine = sin(radians);
	const GLfloat deltaZ = zFar - zNear;
	GLfloat cotangent;

	if ((fabsf((float)deltaZ) < 1.0E-36f) || fpclassify(sine) == FP_ZERO || fpclassify(aspect) == FP_ZERO)
	{
		return;
	}
	cotangent = cosf(radians) / sine;

	m[0][0] = cotangent / aspect;
	m[1][1] = cotangent;
	m[2][2] = -(zFar + zNear) / deltaZ;
	m[3][2] = -2.0f * zNear * zFar / deltaZ;
	pglMultMatrixf(&m[0][0]);
}

// -----------------+
// SetModelView     :
// -----------------+
void SetModelView(GLint w, GLint h)
{
	//GL_DBG_Printf("SetModelView(): %dx%d\n", (int)w, (int)h);

	// The screen textures need to be flushed if the width or height change so that they be remade for the correct size
	if (screen_width != w || screen_height != h)
		FlushScreenTextures();

	screen_width = w;
	screen_height = h;

	pglViewport(0, 0, w, h);
#ifdef GL_ACCUM_BUFFER_BIT
	pglClear(GL_ACCUM_BUFFER_BIT);
#endif

	pglMatrixMode(GL_PROJECTION);
	pglLoadIdentity();

	pglMatrixMode(GL_MODELVIEW);
	pglLoadIdentity();

	GLPerspective(fov, ASPECT_RATIO);

	// added for new coronas' code (without depth buffer)
	pglGetIntegerv(GL_VIEWPORT, viewport);
	pglGetFloatv(GL_PROJECTION_MATRIX, projMatrix);
}


// -----------------+
// SetStates        : Set permanent states
// -----------------+
void SetStates(void)
{
	pglEnable(GL_TEXTURE_2D);      // two-dimensional texturing
	pglTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	pglAlphaFunc(GL_NOTEQUAL, 0.0f);
	pglEnable(GL_BLEND);           // enable color blending

	pglColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	pglEnable(GL_DEPTH_TEST);    // check the depth buffer
	pglDepthMask(GL_TRUE);             // enable writing to depth buffer
	pglClearDepth(1.0f);
	pglDepthRange(0.0f, 1.0f);
	pglDepthFunc(GL_LEQUAL);

	// this set CurrentPolyFlags to the actual configuration
	CurrentPolyFlags = 0xffffffff;
	SetBlend(0);

	tex_downloaded = (GLuint)-1;
	SetNoTexture();

	pglPolygonOffset(-1.0f, -1.0f);

	// bp : when no t&l :)
	pglLoadIdentity();
	pglScalef(1.0f, 1.0f, -1.0f);
	pglGetFloatv(GL_MODELVIEW_MATRIX, modelMatrix); // added for new coronas' code (without depth buffer)

	// jimita
#ifndef GL_RETAINED_MODE
	if (!GLEXT_legacy)
		I_Error("GPU does not support legacy specifications");
#endif

	pglDeleteVertexArrays(1, &gl_vertexarrayobject);
	pglDeleteBuffers(1, &gl_vertexbuffer);

	pglGenVertexArrays(1, &gl_vertexarrayobject);
	pglGenBuffers(1, &gl_vertexbuffer);

	pglBindVertexArray(gl_vertexarrayobject);
	pglBindBuffer(GL_ARRAY_BUFFER, gl_vertexbuffer);
}


// -----------------+
// Flush            : flush OpenGL textures
//                  : Clear list of downloaded mipmaps
// -----------------+
void Flush(void)
{
	//GL_DBG_Printf("HWR_Flush()\n");

	while (gr_cachehead)
	{
		// ceci n'est pas du tout necessaire vu que tu les a charger normalement et
		// donc il sont dans ta liste !
		pglDeleteTextures(1, (GLuint *)&gr_cachehead->downloaded);
		gr_cachehead->downloaded = 0;
		gr_cachehead = gr_cachehead->nextmipmap;
	}
	gr_cachetail = gr_cachehead = NULL; //Hurdler: well, gr_cachehead is already NULL
	NextTexAvail = FIRST_TEX_AVAIL;

	tex_downloaded = 0;
}


// -----------------+
// isExtAvailable   : Look if an OpenGL extension is available
// Returns          : true if extension available
// -----------------+
INT32 isExtAvailable(const char *extension, const GLubyte *start)
{
	GLubyte         *where, *terminator;

	if (!extension || !start) return 0;
	where = (GLubyte *) strchr(extension, ' ');
	if (where || *extension == '\0')
		return 0;

	for (;;)
	{
		where = (GLubyte *) strstr((const char *) start, extension);
		if (!where)
			break;
		terminator = where + strlen(extension);
		if (where == start || *(where - 1) == ' ')
			if (*terminator == ' ' || *terminator == '\0')
				return 1;
		start = terminator;
	}
	return 0;
}


// -----------------+
// Init             : Initialise the OpenGL interface API
// Returns          :
// -----------------+
EXPORT boolean HWRAPI(Init) (void)
{
	return LoadGL();
}


// -----------------+
// ClearMipMapCache : Flush OpenGL textures from memory
// -----------------+
EXPORT void HWRAPI(ClearMipMapCache) (void)
{
	Flush();
}


// -----------------+
// ReadRect         : Read a rectangle region of the truecolor framebuffer
//                  : store pixels as 16bit 565 RGB
// Returns          : 16bit 565 RGB pixel array stored in dst_data
// -----------------+
EXPORT void HWRAPI(ReadRect) (INT32 x, INT32 y, INT32 width, INT32 height,
                                INT32 dst_stride, UINT16 * dst_data)
{
	INT32 i;
	//GL_DBG_Printf("ReadRect()\n");
	if (dst_stride == width*3)
	{
		GLubyte*top = (GLvoid*)dst_data, *bottom = top + dst_stride * (height - 1);
		GLubyte *row = malloc(dst_stride);
		if (!row) return;
		pglPixelStorei(GL_PACK_ALIGNMENT, 1);
		pglReadPixels(x, y, width, height, GL_RGB, GL_UNSIGNED_BYTE, dst_data);
		pglPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		for(i = 0; i < height/2; i++)
		{
			memcpy(row, top, dst_stride);
			memcpy(top, bottom, dst_stride);
			memcpy(bottom, row, dst_stride);
			top += dst_stride;
			bottom -= dst_stride;
		}
		free(row);
	}
	else
	{
		INT32 j;
		GLubyte *image = malloc(width*height*3*sizeof (*image));
		if (!image) return;
		pglPixelStorei(GL_PACK_ALIGNMENT, 1);
		pglReadPixels(x, y, width, height, GL_RGB, GL_UNSIGNED_BYTE, image);
		pglPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		for (i = height-1; i >= 0; i--)
		{
			for (j = 0; j < width; j++)
			{
				dst_data[(height-1-i)*width+j] =
				(UINT16)(
				                 ((image[(i*width+j)*3]>>3)<<11) |
				                 ((image[(i*width+j)*3+1]>>2)<<5) |
				                 ((image[(i*width+j)*3+2]>>3)));
			}
		}
		free(image);
	}
}


// -----------------+
// GClipRect        : Defines the 2D hardware clipping window
// -----------------+
EXPORT void HWRAPI(GClipRect) (INT32 minx, INT32 miny, INT32 maxx, INT32 maxy, float nearclip)
{
	//GL_DBG_Printf("GClipRect(%d, %d, %d, %d)\n", minx, miny, maxx, maxy);

	pglViewport(minx, screen_height-maxy, maxx-minx, maxy-miny);
	NEAR_CLIPPING_PLANE = nearclip;

	//pglScissor(minx, screen_height-maxy, maxx-minx, maxy-miny);
	pglMatrixMode(GL_PROJECTION);
	pglLoadIdentity();
	GLPerspective(fov, ASPECT_RATIO);
	pglMatrixMode(GL_MODELVIEW);

	// added for new coronas' code (without depth buffer)
	pglGetIntegerv(GL_VIEWPORT, viewport);
	pglGetFloatv(GL_PROJECTION_MATRIX, projMatrix);
}


// -----------------+
// ClearBuffer      : Clear the color/alpha/depth buffer(s)
// -----------------+
EXPORT void HWRAPI(ClearBuffer) (FBOOLEAN ColorMask,
                                    FBOOLEAN DepthMask,
                                    FRGBAFloat * ClearColor)
{
	//GL_DBG_Printf("ClearBuffer(%d)\n", alpha);
	GLbitfield ClearMask = 0;

	if (ColorMask)
	{
		if (ClearColor)
			pglClearColor(ClearColor->red,
			              ClearColor->green,
			              ClearColor->blue,
			              ClearColor->alpha);
		ClearMask |= GL_COLOR_BUFFER_BIT;
	}
	if (DepthMask)
	{
		pglClearDepth(1.0f);     //Hurdler: all that are permanen states
		pglDepthRange(0.0f, 1.0f);
		pglDepthFunc(GL_LEQUAL);
		ClearMask |= GL_DEPTH_BUFFER_BIT;
	}

	SetBlend(DepthMask ? PF_Occlude | CurrentPolyFlags : CurrentPolyFlags&~PF_Occlude);

	pglClear(ClearMask);
}


// -----------------+
// HWRAPI Draw2DLine: Render a 2D line
// -----------------+
EXPORT void HWRAPI(Draw2DLine) (F2DCoord * v1,
                                   F2DCoord * v2,
                                   RGBA_t Color)
{
	GLRGBAFloat c;

	//GL_DBG_Printf("DrawLine() (%f %f %f) %d\n", v1->x, -v1->y, -v1->z, v1->argb);

	// BP: we should reflect the new state in our variable
	//SetBlend(PF_Modulated|PF_NoTexture);

	pglDisable(GL_TEXTURE_2D);

	c.red   = byte2float[Color.s.red];
	c.green = byte2float[Color.s.green];
	c.blue  = byte2float[Color.s.blue];
	c.alpha = byte2float[Color.s.alpha];

	pglColor4fv(&c.red);    // is in RGBA float format
	pglBegin(GL_LINES);
		pglVertex3f(v1->x, -v1->y, 1.0f);
		pglVertex3f(v2->x, -v2->y, 1.0f);
	pglEnd();

	pglEnable(GL_TEXTURE_2D);
}

static void Clamp2D(GLenum pname)
{
	pglTexParameteri(GL_TEXTURE_2D, pname, GL_CLAMP); // fallback clamp
#ifdef GL_CLAMP_TO_EDGE
	pglTexParameteri(GL_TEXTURE_2D, pname, GL_CLAMP_TO_EDGE);
#endif
}


// -----------------+
// SetBlend         : Set render mode
// -----------------+
// PF_Masked - we could use an ALPHA_TEST of GL_EQUAL, and alpha ref of 0,
//             is it faster when pixels are discarded ?
EXPORT void HWRAPI(SetBlend) (FBITFIELD PolyFlags)
{
	FBITFIELD Xor;
	Xor = CurrentPolyFlags^PolyFlags;
	if (Xor & (PF_Blending|PF_RemoveYWrap|PF_ForceWrapX|PF_ForceWrapY|PF_Occlude|PF_NoTexture|PF_Modulated|PF_NoDepthTest|PF_Decal|PF_Invisible|PF_NoAlphaTest))
	{
		if (Xor&(PF_Blending)) // if blending mode must be changed
		{
			switch (PolyFlags & PF_Blending) {
				case PF_Translucent & PF_Blending:
					pglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // alpha = level of transparency
					pglAlphaFunc(GL_NOTEQUAL, 0.0f);
					break;
				case PF_Masked & PF_Blending:
					// Hurdler: does that mean lighting is only made by alpha src?
					// it sounds ok, but not for polygonsmooth
					pglBlendFunc(GL_SRC_ALPHA, GL_ZERO);                // 0 alpha = holes in texture
					pglAlphaFunc(GL_GREATER, 0.5f);
					break;
				case PF_Additive & PF_Blending:
					pglBlendFunc(GL_SRC_ALPHA, GL_ONE);                 // src * alpha + dest
					pglAlphaFunc(GL_NOTEQUAL, 0.0f);
					break;
				case PF_Environment & PF_Blending:
					pglBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
					pglAlphaFunc(GL_NOTEQUAL, 0.0f);
					break;
				case PF_Substractive & PF_Blending:
					// good for shadow
					// not realy but what else ?
					pglBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
					pglAlphaFunc(GL_NOTEQUAL, 0.0f);
					break;
				case PF_Fog & PF_Fog:
					// Sryder: Fog
					// multiplies input colour by input alpha, and destination colour by input colour, then adds them
					pglBlendFunc(GL_SRC_ALPHA, GL_SRC_COLOR);
					pglAlphaFunc(GL_NOTEQUAL, 0.0f);
					break;
				default : // must be 0, otherwise it's an error
					// No blending
					pglBlendFunc(GL_ONE, GL_ZERO);   // the same as no blending
					pglAlphaFunc(GL_GREATER, 0.5f);
					break;
			}
		}
		if (Xor & PF_NoAlphaTest)
		{
			if (PolyFlags & PF_NoAlphaTest)
				pglDisable(GL_ALPHA_TEST);
			else
				pglEnable(GL_ALPHA_TEST);      // discard 0 alpha pixels (holes in texture)
		}

		if (Xor & PF_Decal)
		{
			if (PolyFlags & PF_Decal)
				pglEnable(GL_POLYGON_OFFSET_FILL);
			else
				pglDisable(GL_POLYGON_OFFSET_FILL);
		}
		if (Xor&PF_NoDepthTest)
		{
			if (PolyFlags & PF_NoDepthTest)
				pglDepthFunc(GL_ALWAYS); //pglDisable(GL_DEPTH_TEST);
			else
				pglDepthFunc(GL_LEQUAL); //pglEnable(GL_DEPTH_TEST);
		}

		if (Xor&PF_RemoveYWrap)
		{
			if (PolyFlags & PF_RemoveYWrap)
				Clamp2D(GL_TEXTURE_WRAP_T);
		}

		if (Xor&PF_ForceWrapX)
		{
			if (PolyFlags & PF_ForceWrapX)
				pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		}

		if (Xor&PF_ForceWrapY)
		{
			if (PolyFlags & PF_ForceWrapY)
				pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}

		if (Xor&PF_Modulated)
		{
#if defined (__unix__) || defined (UNIXCOMMON)
			if (oglflags & GLF_NOTEXENV)
			{
				if (!(PolyFlags & PF_Modulated))
					pglColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			}
			else
#endif

			// mix texture colour with Surface->PolyColor
			if (PolyFlags & PF_Modulated)
				pglTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			// colour from texture is unchanged before blending
			else
				pglTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		}

		if (Xor & PF_Occlude) // depth test but (no) depth write
		{
			if (PolyFlags&PF_Occlude)
				pglDepthMask(1);
			else
				pglDepthMask(0);
		}
		////Hurdler: not used if we don't define POLYSKY
		if (Xor & PF_Invisible)
		{
			if (PolyFlags&PF_Invisible)
				pglBlendFunc(GL_ZERO, GL_ONE);         // transparent blending
			else
			{   // big hack: (TODO: manage that better)
				// we test only for PF_Masked because PF_Invisible is only used
				// (for now) with it (yeah, that's crappy, sorry)
				if ((PolyFlags&PF_Blending)==PF_Masked)
					pglBlendFunc(GL_SRC_ALPHA, GL_ZERO);
			}
		}
		if (PolyFlags & PF_NoTexture)
			SetNoTexture();
	}
	CurrentPolyFlags = PolyFlags;
}


// -----------------+
// SetTexture       : The mipmap becomes the current texture source
// -----------------+
EXPORT void HWRAPI(SetTexture) (FTextureInfo *pTexInfo)
{
	if (!pTexInfo)
	{
		SetNoTexture();
		return;
	}
	else if (pTexInfo->downloaded)
	{
		if (pTexInfo->downloaded != tex_downloaded)
		{
			pglBindTexture(GL_TEXTURE_2D, pTexInfo->downloaded);
			tex_downloaded = pTexInfo->downloaded;
		}
	}
	else
	{
		// Download a mipmap
		static RGBA_t   tex[2048*2048];
		const GLvoid   *ptex = tex;
		INT32             w, h;

		//GL_DBG_Printf("DownloadMipmap %d\n", NextTexAvail, pTexInfo->grInfo.data);

		w = pTexInfo->width;
		h = pTexInfo->height;

#ifdef USE_PALETTED_TEXTURE
		if (glColorTableEXT &&
			(pTexInfo->grInfo.format == GR_TEXFMT_P_8) &&
			!(pTexInfo->flags & TF_CHROMAKEYED))
		{
			// do nothing here.
			// Not a problem with MiniGL since we don't use paletted texture
		}
		else
#endif

		if ((pTexInfo->grInfo.format == GR_TEXFMT_P_8) ||
			(pTexInfo->grInfo.format == GR_TEXFMT_AP_88))
		{
			const GLubyte *pImgData = (const GLubyte *)pTexInfo->grInfo.data;
			INT32 i, j;

			for (j = 0; j < h; j++)
			{
				for (i = 0; i < w; i++)
				{
					if ((*pImgData == HWR_PATCHES_CHROMAKEY_COLORINDEX) &&
					    (pTexInfo->flags & TF_CHROMAKEYED))
					{
						tex[w*j+i].s.red   = 0;
						tex[w*j+i].s.green = 0;
						tex[w*j+i].s.blue  = 0;
						tex[w*j+i].s.alpha = 0;
						pTexInfo->flags |= TF_TRANSPARENT; // there is a hole in it
					}
					else
					{
						tex[w*j+i].s.red   = myPaletteData[*pImgData].s.red;
						tex[w*j+i].s.green = myPaletteData[*pImgData].s.green;
						tex[w*j+i].s.blue  = myPaletteData[*pImgData].s.blue;
						tex[w*j+i].s.alpha = myPaletteData[*pImgData].s.alpha;
					}

					pImgData++;

					if (pTexInfo->grInfo.format == GR_TEXFMT_AP_88)
					{
						if (!(pTexInfo->flags & TF_CHROMAKEYED))
							tex[w*j+i].s.alpha = *pImgData;
						pImgData++;
					}

				}
			}
		}
		else if (pTexInfo->grInfo.format == GR_RGBA)
		{
			// corona test : passed as ARGB 8888, which is not in glide formats
			// Hurdler: not used for coronas anymore, just for dynamic lighting
			ptex = pTexInfo->grInfo.data;
		}
		else if (pTexInfo->grInfo.format == GR_TEXFMT_ALPHA_INTENSITY_88)
		{
			const GLubyte *pImgData = (const GLubyte *)pTexInfo->grInfo.data;
			INT32 i, j;

			for (j = 0; j < h; j++)
			{
				for (i = 0; i < w; i++)
				{
					tex[w*j+i].s.red   = *pImgData;
					tex[w*j+i].s.green = *pImgData;
					tex[w*j+i].s.blue  = *pImgData;
					pImgData++;
					tex[w*j+i].s.alpha = *pImgData;
					pImgData++;
				}
			}
		}
		else if (pTexInfo->grInfo.format == GR_TEXFMT_ALPHA_8) // Used for fade masks
		{
			const GLubyte *pImgData = (const GLubyte *)pTexInfo->grInfo.data;
			INT32 i, j;

			for (j = 0; j < h; j++)
			{
				for (i = 0; i < w; i++)
				{
					tex[w*j+i].s.red   = 255; // 255 because the fade mask is modulated with the screen texture, so alpha affects it while the colours don't
					tex[w*j+i].s.green = 255;
					tex[w*j+i].s.blue  = 255;
					tex[w*j+i].s.alpha = *pImgData;
					pImgData++;
				}
			}
		}
		else
			GL_DBG_Printf("SetTexture(bad format) %ld\n", pTexInfo->grInfo.format);

		pTexInfo->downloaded = NextTexAvail++;
		tex_downloaded = pTexInfo->downloaded;
		pglBindTexture(GL_TEXTURE_2D, pTexInfo->downloaded);

		// disable texture filtering on any texture that has holes so there's no dumb borders or blending issues
		if (pTexInfo->flags & TF_TRANSPARENT)
		{
			pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}
		else
		{
			pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
			pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
		}

#ifdef USE_PALETTED_TEXTURE
			//Hurdler: not really supported and not tested recently
		if (glColorTableEXT &&
			(pTexInfo->grInfo.format == GR_TEXFMT_P_8) &&
			!(pTexInfo->flags & TF_CHROMAKEYED))
		{
			glColorTableEXT(GL_TEXTURE_2D, GL_RGB8, 256, GL_RGB, GL_UNSIGNED_BYTE, palette_tex);
			pglTexImage2D(GL_TEXTURE_2D, 0, GL_COLOR_INDEX8_EXT, w, h, 0, GL_COLOR_INDEX, GL_UNSIGNED_BYTE, pTexInfo->grInfo.data);
		}
		else
#endif
		if (pTexInfo->grInfo.format == GR_TEXFMT_ALPHA_INTENSITY_88)
		{
			//pglTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, ptex);
			if (MipMap)
			{
				pgluBuild2DMipmaps(GL_TEXTURE_2D, GL_LUMINANCE_ALPHA, w, h, GL_RGBA, GL_UNSIGNED_BYTE, ptex);
#ifdef GL_TEXTURE_MIN_LOD
				pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, 0);
#endif
#ifdef GL_TEXTURE_MAX_LOD
				if (pTexInfo->flags & TF_TRANSPARENT)
					pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 0); // No mippmaps on transparent stuff
				else
					pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 4);
#endif
				//pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_LINEAR_MIPMAP_LINEAR);
			}
			else
				pglTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, ptex);
		}
		else if (pTexInfo->grInfo.format == GR_TEXFMT_ALPHA_8)
		{
			//pglTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, ptex);
			if (MipMap)
			{
				pgluBuild2DMipmaps(GL_TEXTURE_2D, GL_ALPHA, w, h, GL_RGBA, GL_UNSIGNED_BYTE, ptex);
#ifdef GL_TEXTURE_MIN_LOD
				pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, 0);
#endif
#ifdef GL_TEXTURE_MAX_LOD
				if (pTexInfo->flags & TF_TRANSPARENT)
					pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 0); // No mippmaps on transparent stuff
				else
					pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 4);
#endif
				//pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_LINEAR_MIPMAP_LINEAR);
			}
			else
				pglTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, ptex);
		}
		else
		{
			if (MipMap)
			{
				pgluBuild2DMipmaps(GL_TEXTURE_2D, textureformatGL, w, h, GL_RGBA, GL_UNSIGNED_BYTE, ptex);
				// Control the mipmap level of detail
#ifdef GL_TEXTURE_MIN_LOD
				pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, 0); // the lower the number, the higer the detail
#endif
#ifdef GL_TEXTURE_MAX_LOD
				if (pTexInfo->flags & TF_TRANSPARENT)
					pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 0); // No mippmaps on transparent stuff
				else
					pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 5);
#endif
			}
			else
				pglTexImage2D(GL_TEXTURE_2D, 0, textureformatGL, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, ptex);
		}

		if (pTexInfo->flags & TF_WRAPX)
			pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		else
			Clamp2D(GL_TEXTURE_WRAP_S);

		if (pTexInfo->flags & TF_WRAPY)
			pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		else
			Clamp2D(GL_TEXTURE_WRAP_T);

		if (maximumAnisotropy)
			pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropic_filter);

		pTexInfo->nextmipmap = NULL;
		if (gr_cachetail)
		{ // insertion en fin de liste
			gr_cachetail->nextmipmap = pTexInfo;
			gr_cachetail = pTexInfo;
		}
		else // initialisation de la liste
			gr_cachetail = gr_cachehead =  pTexInfo;
	}
}

// jimita

static void load_shaders(FSurfaceInfo *Surface, GLRGBAFloat *mix, GLRGBAFloat *fade)
{
#ifdef USE_SHADERS
	if (gl_shadersenabled)
	{
		if (gl_shaderprograms[gl_currentshaderprogram])
		{
			pglUseProgram(gl_shaderprograms[gl_currentshaderprogram]);

			//
			// set uniforms
			//
			GLint UNIFORM_fog_mode 		= pglGetUniformLocation(gl_shaderprograms[gl_currentshaderprogram], "fog_mode");
			GLint UNIFORM_fog_density 	= pglGetUniformLocation(gl_shaderprograms[gl_currentshaderprogram], "fog_density");

			GLint UNIFORM_mix_color 	= pglGetUniformLocation(gl_shaderprograms[gl_currentshaderprogram], "mix_color");
			GLint UNIFORM_fade_color 	= pglGetUniformLocation(gl_shaderprograms[gl_currentshaderprogram], "fade_color");
			GLint UNIFORM_lighting 		= pglGetUniformLocation(gl_shaderprograms[gl_currentshaderprogram], "lighting");

			GLint UNIFORM_resolution 	= pglGetUniformLocation(gl_shaderprograms[gl_currentshaderprogram], "resolution");
			GLint UNIFORM_leveltime 	= pglGetUniformLocation(gl_shaderprograms[gl_currentshaderprogram], "leveltime");

			#define UNIFORM_1(uniform, a, function) \
				if (uniform != -1) \
					function (uniform, a);

			#define UNIFORM_2(uniform, a, b, function) \
				if (uniform != -1) \
					function (uniform, a, b);

			#define UNIFORM_3(uniform, a, b, c, function) \
				if (uniform != -1) \
					function (uniform, a, b, c);

			#define UNIFORM_4(uniform, a, b, c, d, function) \
				if (uniform != -1) \
					function (uniform, a, b, c, d);

			// glstate
			UNIFORM_1(UNIFORM_fog_mode, 		glstate_fog_mode, 									pglUniform1i);
			UNIFORM_1(UNIFORM_fog_density, 		glstate_fog_density, 								pglUniform1f);

			// polygon
			UNIFORM_4(UNIFORM_mix_color, 		mix->red, mix->green, mix->blue, mix->alpha,		pglUniform4f);
			UNIFORM_4(UNIFORM_fade_color, 		fade->red, fade->green, fade->blue, fade->alpha,	pglUniform4f);
			UNIFORM_1(UNIFORM_lighting, 		Surface->LightInfo.light_level,						pglUniform1f);

			UNIFORM_2(UNIFORM_resolution, 		screen_width, screen_height,						pglUniform2f);
			UNIFORM_1(UNIFORM_leveltime, 		(float)gl_leveltime,								pglUniform1f);

			#undef UNIFORM_1
			#undef UNIFORM_2
			#undef UNIFORM_3
			#undef UNIFORM_4
		}
		else
			pglUseProgram(0);
	}
#endif
}

// -----------------+
// DrawPolygon      : Render a polygon, set the texture, set render mode
// -----------------+
EXPORT void HWRAPI(DrawPolygon) (FSurfaceInfo *pSurf, FOutVector *pOutVerts, FUINT iNumPts, FBITFIELD PolyFlags)
{
	static GLRGBAFloat mix = {0,0,0,0};
	static GLRGBAFloat fade = {0,0,0,0};
#ifndef GL_RETAINED_MODE
	FUINT i;
#endif

	SetBlend(PolyFlags);    //TODO: inline (#pragma..)

	// PolyColor
	if (pSurf)
	{
		// If Modulated, mix the surface colour to the texture
		if (CurrentPolyFlags & PF_Modulated)
		{
			// Mix color
			mix.red    = byte2float[pSurf->PolyColor.s.red];
			mix.green  = byte2float[pSurf->PolyColor.s.green];
			mix.blue   = byte2float[pSurf->PolyColor.s.blue];
			mix.alpha  = byte2float[pSurf->PolyColor.s.alpha];

			pglColor4fv(&mix.red);
		}

		// Fade color
		fade.red   = byte2float[pSurf->FadeColor.s.red];
		fade.green = byte2float[pSurf->FadeColor.s.green];
		fade.blue  = byte2float[pSurf->FadeColor.s.blue];
		fade.alpha = byte2float[pSurf->FadeColor.s.alpha];
	}

	// jimita
	load_shaders(pSurf, &mix, &fade);

#ifdef GL_RETAINED_MODE
	pglBufferData(GL_ARRAY_BUFFER, sizeof(FOutVector)*iNumPts, &pOutVerts[0].x, GL_STATIC_DRAW);

	pglEnableClientState(GL_VERTEX_ARRAY);
	pglEnableClientState(GL_TEXTURE_COORD_ARRAY);

	pglVertexPointer(3, GL_FLOAT, sizeof(FOutVector), BUFFER_OFFSET(0));
	pglTexCoordPointer(2, GL_FLOAT, sizeof(FOutVector), BUFFER_OFFSET(sizeof(FLOAT)*3));
	pglDrawArrays(GL_TRIANGLE_FAN, 0, iNumPts);
#else
	pglBegin(GL_TRIANGLE_FAN);
	for (i = 0; i < iNumPts; i++)
	{
		pglTexCoord2f(pOutVerts[i].sow, pOutVerts[i].tow);
		pglVertex3f(pOutVerts[i].x, pOutVerts[i].y, pOutVerts[i].z);
	}
	pglEnd();
#endif

	if (PolyFlags & PF_RemoveYWrap)
		pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	if (PolyFlags & PF_ForceWrapX)
		Clamp2D(GL_TEXTURE_WRAP_S);

	if (PolyFlags & PF_ForceWrapY)
		Clamp2D(GL_TEXTURE_WRAP_T);

#ifdef GL_RETAINED_MODE
	pglDisableClientState(GL_VERTEX_ARRAY);
	pglDisableClientState(GL_TEXTURE_COORD_ARRAY);
#endif

#ifdef USE_SHADERS
	pglUseProgram(0);
#endif
}

// ==========================================================================
//
// ==========================================================================
EXPORT void HWRAPI(SetSpecialState) (hwdspecialstate_t IdState, INT32 Value)
{
	switch (IdState)
	{
		case HWD_SET_FOG_MODE:
			glstate_fog_mode = Value;
			break;

		case HWD_SET_FOG_DENSITY:
			glstate_fog_density = FIXED_TO_FLOAT(Value);
			break;

		case HWD_SET_TEXTUREFILTERMODE:
			switch (Value)
			{
				case HWD_SET_TEXTUREFILTER_TRILINEAR:
					min_filter = GL_LINEAR_MIPMAP_LINEAR;
					mag_filter = GL_LINEAR;
					MipMap = GL_TRUE;
					break;
				case HWD_SET_TEXTUREFILTER_BILINEAR:
					min_filter = mag_filter = GL_LINEAR;
					MipMap = GL_FALSE;
					break;
				case HWD_SET_TEXTUREFILTER_POINTSAMPLED:
					min_filter = mag_filter = GL_NEAREST;
					MipMap = GL_FALSE;
					break;
				case HWD_SET_TEXTUREFILTER_MIXED1:
					min_filter = GL_NEAREST;
					mag_filter = GL_LINEAR;
					MipMap = GL_FALSE;
					break;
				case HWD_SET_TEXTUREFILTER_MIXED2:
					min_filter = GL_LINEAR;
					mag_filter = GL_NEAREST;
					MipMap = GL_FALSE;
					break;
				case HWD_SET_TEXTUREFILTER_MIXED3:
					min_filter = GL_LINEAR_MIPMAP_LINEAR;
					mag_filter = GL_NEAREST;
					MipMap = GL_TRUE;
					break;
				default:
					mag_filter = GL_LINEAR;
					min_filter = GL_NEAREST;
			}
			if (!pgluBuild2DMipmaps)
			{
				MipMap = GL_FALSE;
				min_filter = GL_LINEAR;
			}
			Flush(); //??? if we want to change filter mode by texture, remove this
			break;

		case HWD_SET_TEXTUREANISOTROPICMODE:
			anisotropic_filter = min(Value,maximumAnisotropy);
			if (maximumAnisotropy)
				Flush(); //??? if we want to change filter mode by texture, remove this
			break;

		default:
			break;
	}
}

static void DrawMD2Ex(INT32 *gl_cmd_buffer, md2_frame_t *frame, INT32 duration, INT32 tics, md2_frame_t *nextframe, FTransform *pos, float scale, UINT8 flipped, FSurfaceInfo *Surface)
{
	INT32 val, count, pindex;
	GLfloat s, t;

	float pol = 0.0f;
	float scalex = scale, scaley = scale, scalez = scale;

	static GLRGBAFloat mix = {0,0,0,0};
	static GLRGBAFloat fade = {0,0,0,0};

	if (duration != 0 && duration != -1 && tics != -1) // don't interpolate if instantaneous or infinite in length
	{
		UINT32 newtime = (duration - tics); // + 1;

		pol = (newtime)/(float)duration;

		if (pol > 1.0f)
			pol = 1.0f;

		if (pol < 0.0f)
			pol = 0.0f;
	}

	mix.red    = byte2float[Surface->PolyColor.s.red];
	mix.green  = byte2float[Surface->PolyColor.s.green];
	mix.blue   = byte2float[Surface->PolyColor.s.blue];
	mix.alpha  = byte2float[Surface->PolyColor.s.alpha];

	if (mix.alpha < 1)
		SetBlend(PF_Translucent|PF_Modulated);
	else
		SetBlend(PF_Masked|PF_Modulated|PF_Occlude);

	fade.red   = byte2float[Surface->FadeColor.s.red];
	fade.green = byte2float[Surface->FadeColor.s.green];
	fade.blue  = byte2float[Surface->FadeColor.s.blue];
	fade.alpha = byte2float[Surface->FadeColor.s.alpha];

	pglEnable(GL_CULL_FACE);

	// pos->flip is if the screen is flipped too
	if (flipped != pos->flip) // If either are active, but not both, invert the model's culling
		pglCullFace(GL_FRONT);
	else
		pglCullFace(GL_BACK);

	pglPushMatrix();
	pglTranslatef(pos->x, pos->z, pos->y);
	if (flipped)
		scaley = -scaley;
	pglRotatef(pos->angley, 0.0f, -1.0f, 0.0f);
	pglRotatef(pos->anglex, -1.0f, 0.0f, 0.0f);

	// jimita
	load_shaders(Surface, &mix, &fade);

	pglVertexPointer(3, GL_FLOAT, sizeof(FOutVectorMD2), BUFFER_OFFSET(0));
	pglNormalPointer(GL_FLOAT, sizeof(FOutVectorMD2), BUFFER_OFFSET(sizeof(float)*3));
	pglTexCoordPointer(2, GL_FLOAT, sizeof(FOutVectorMD2), BUFFER_OFFSET(sizeof(float)*6));

	val = *gl_cmd_buffer++;

	while (val != 0)
	{
#ifdef GL_RETAINED_MODE
		FOutVectorMD2 *polydata;
		int polyindex = 0;
#endif
		int drawarraytype;

		if (val < 0)
		{
			drawarraytype = GL_TRIANGLE_FAN;
			count = -val;
		}
		else
		{
			drawarraytype = GL_TRIANGLE_STRIP;
			count = val;
		}

#ifndef GL_RETAINED_MODE
		pglBegin(drawarraytype);
#else
		polydata = malloc(sizeof(FOutVectorMD2) * count);
#endif

		while (count--)
		{
			s = *(float *) gl_cmd_buffer++;
			t = *(float *) gl_cmd_buffer++;
			pindex = *gl_cmd_buffer++;

#ifdef GL_RETAINED_MODE
			polydata[polyindex].s = s;
			polydata[polyindex].t = t;
#else
			pglTexCoord2f(s, t);
#endif

			if (!nextframe || fpclassify(pol) == FP_ZERO)
			{
#ifdef GL_RETAINED_MODE
				polydata[polyindex].vx = frame->vertices[pindex].vertex[0]*scalex/2.0f;
				polydata[polyindex].vy = frame->vertices[pindex].vertex[1]*scaley/2.0f;
				polydata[polyindex].vz = frame->vertices[pindex].vertex[2]*scalez/2.0f;

				polydata[polyindex].nx = frame->vertices[pindex].normal[0];
				polydata[polyindex].ny = frame->vertices[pindex].normal[1];
				polydata[polyindex].nz = frame->vertices[pindex].normal[2];

				polyindex++;
#else

				pglNormal3f(frame->vertices[pindex].normal[0],
				            frame->vertices[pindex].normal[1],
				            frame->vertices[pindex].normal[2]);

				pglVertex3f(frame->vertices[pindex].vertex[0]*scalex/2.0f,
				            frame->vertices[pindex].vertex[1]*scaley/2.0f,
				            frame->vertices[pindex].vertex[2]*scalez/2.0f);
#endif
			}
			else
			{
				// Interpolate
				float px1 = frame->vertices[pindex].vertex[0]*scalex/2.0f;
				float px2 = nextframe->vertices[pindex].vertex[0]*scalex/2.0f;
				float py1 = frame->vertices[pindex].vertex[1]*scaley/2.0f;
				float py2 = nextframe->vertices[pindex].vertex[1]*scaley/2.0f;
				float pz1 = frame->vertices[pindex].vertex[2]*scalez/2.0f;
				float pz2 = nextframe->vertices[pindex].vertex[2]*scalez/2.0f;
				float nx1 = frame->vertices[pindex].normal[0];
				float nx2 = nextframe->vertices[pindex].normal[0];
				float ny1 = frame->vertices[pindex].normal[1];
				float ny2 = nextframe->vertices[pindex].normal[1];
				float nz1 = frame->vertices[pindex].normal[2];
				float nz2 = nextframe->vertices[pindex].normal[2];

#ifdef GL_RETAINED_MODE
				polydata[polyindex].vx = (px1 + pol * (px2 - px1));
				polydata[polyindex].vy = (py1 + pol * (py2 - py1));
				polydata[polyindex].vz = (pz1 + pol * (pz2 - pz1));

				polydata[polyindex].nx = (nx1 + pol * (nx2 - nx1));
				polydata[polyindex].ny = (ny1 + pol * (ny2 - ny1));
				polydata[polyindex].nz = (nz1 + pol * (nz2 - nz1));

				polyindex++;
#else
				pglNormal3f((nx1 + pol * (nx2 - nx1)),
				            (ny1 + pol * (ny2 - ny1)),
				            (nz1 + pol * (nz2 - nz1)));
				pglVertex3f((px1 + pol * (px2 - px1)),
				            (py1 + pol * (py2 - py1)),
				            (pz1 + pol * (pz2 - pz1)));
#endif
			}
		}

#ifdef GL_RETAINED_MODE
		pglEnableClientState(GL_VERTEX_ARRAY);
		pglEnableClientState(GL_NORMAL_ARRAY);
		pglEnableClientState(GL_TEXTURE_COORD_ARRAY);

		pglBufferData(GL_ARRAY_BUFFER, sizeof(FOutVectorMD2)*polyindex, &polydata[0].vx, GL_DYNAMIC_DRAW);

		pglDrawArrays(drawarraytype, 0, polyindex);

		free(polydata);
#else
		pglEnd();
#endif

		val = *gl_cmd_buffer++;
	}

	pglPopMatrix();
	pglDisable(GL_CULL_FACE);

	pglDisableClientState(GL_VERTEX_ARRAY);
	pglDisableClientState(GL_NORMAL_ARRAY);
	pglDisableClientState(GL_TEXTURE_COORD_ARRAY);

#ifdef USE_SHADERS
	pglUseProgram(0);
#endif
}

// -----------------+
// HWRAPI DrawMD2   : Draw an MD2 model with glcommands
// -----------------+
EXPORT void HWRAPI(DrawMD2) (
							INT32 *gl_cmd_buffer, md2_frame_t *frame,
							INT32 duration, INT32 tics, md2_frame_t *nextframe,
							FTransform *pos, float scale, UINT8 flipped,
							FSurfaceInfo *Surface)		// jimita 17032019
{
	DrawMD2Ex(gl_cmd_buffer, frame, duration, tics, nextframe, pos, scale, flipped, Surface);
}

// -----------------+
// SetTransform     :
// -----------------+
EXPORT void HWRAPI(SetTransform) (FTransform *stransform)
{
	static INT32 special_splitscreen;
	pglLoadIdentity();
	if (stransform)
	{
		boolean fovx90;
		// keep a trace of the transformation for md2
		memcpy(&md2_transform, stransform, sizeof (md2_transform));

		if (stransform->flip)
			pglScalef(stransform->scalex, -stransform->scaley, -stransform->scalez);
		else
			pglScalef(stransform->scalex, stransform->scaley, -stransform->scalez);

		pglRotatef(stransform->anglex, 1.0f, 0.0f, 0.0f);
		pglRotatef(stransform->angley+270.0f, 0.0f, 1.0f, 0.0f);
		pglTranslatef(-stransform->x, -stransform->z, -stransform->y);

		pglMatrixMode(GL_PROJECTION);
		pglLoadIdentity();
		fovx90 = stransform->fovxangle > 0.0f && fabsf(stransform->fovxangle - 90.0f) < 0.5f;
		special_splitscreen = (stransform->splitscreen && fovx90);
		if (special_splitscreen)
			GLPerspective(53.13f, 2*ASPECT_RATIO);  // 53.13 = 2*atan(0.5)
		else
			GLPerspective(stransform->fovxangle, ASPECT_RATIO);
		pglGetFloatv(GL_PROJECTION_MATRIX, projMatrix); // added for new coronas' code (without depth buffer)
		pglMatrixMode(GL_MODELVIEW);
	}
	else
	{
		pglScalef(1.0f, 1.0f, -1.0f);

		pglMatrixMode(GL_PROJECTION);
		pglLoadIdentity();
		if (special_splitscreen)
			GLPerspective(53.13f, 2*ASPECT_RATIO);  // 53.13 = 2*atan(0.5)
		else
			//Hurdler: is "fov" correct?
			GLPerspective(fov, ASPECT_RATIO);
		pglGetFloatv(GL_PROJECTION_MATRIX, projMatrix); // added for new coronas' code (without depth buffer)
		pglMatrixMode(GL_MODELVIEW);
	}

	pglGetFloatv(GL_MODELVIEW_MATRIX, modelMatrix); // added for new coronas' code (without depth buffer)
}

EXPORT INT32  HWRAPI(GetTextureUsed) (void)
{
	FTextureInfo*   tmp = gr_cachehead;
	INT32             res = 0;

	while (tmp)
	{
		res += tmp->height*tmp->width*(screen_depth/8);
		tmp = tmp->nextmipmap;
	}
	return res;
}

EXPORT INT32  HWRAPI(GetRenderVersion) (void)
{
	return VERSION;
}

EXPORT void HWRAPI(PostImgRedraw) (float points[SCREENVERTS][SCREENVERTS][2])
{
	INT32 x, y;
	float float_x, float_y, float_nextx, float_nexty;
	float xfix, yfix;
	INT32 texsize = 2048;

	// Use a power of two texture, dammit
	if(screen_width <= 1024)
		texsize = 1024;
	if(screen_width <= 512)
		texsize = 512;

	// X/Y stretch fix for all resolutions(!)
	xfix = (float)(texsize)/((float)((screen_width)/(float)(SCREENVERTS-1)));
	yfix = (float)(texsize)/((float)((screen_height)/(float)(SCREENVERTS-1)));

	pglDisable(GL_DEPTH_TEST);
	pglDisable(GL_BLEND);
	pglBegin(GL_QUADS);

		// Draw a black square behind the screen texture,
		// so nothing shows through the edges
		pglColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		pglVertex3f(-16.0f, -16.0f, 6.0f);
		pglVertex3f(-16.0f, 16.0f, 6.0f);
		pglVertex3f(16.0f, 16.0f, 6.0f);
		pglVertex3f(16.0f, -16.0f, 6.0f);

		for(x=0;x<SCREENVERTS-1;x++)
		{
			for(y=0;y<SCREENVERTS-1;y++)
			{
				// Used for texture coordinates
				// Annoying magic numbers to scale the square texture to
				// a non-square screen..
				float_x = (float)(x/(xfix));
				float_y = (float)(y/(yfix));
				float_nextx = (float)(x+1)/(xfix);
				float_nexty = (float)(y+1)/(yfix);

				// Attach the squares together.
				pglTexCoord2f( float_x, float_y);
				pglVertex3f(points[x][y][0], points[x][y][1], 4.4f);

				pglTexCoord2f( float_x, float_nexty);
				pglVertex3f(points[x][y+1][0], points[x][y+1][1], 4.4f);

				pglTexCoord2f( float_nextx, float_nexty);
				pglVertex3f(points[x+1][y+1][0], points[x+1][y+1][1], 4.4f);

				pglTexCoord2f( float_nextx, float_y);
				pglVertex3f(points[x+1][y][0], points[x+1][y][1], 4.4f);
			}
		}
	pglEnd();
	pglEnable(GL_DEPTH_TEST);
	pglEnable(GL_BLEND);
}

// Sryder:	This needs to be called whenever the screen changes resolution in order to reset the screen textures to use
//			a new size
EXPORT void HWRAPI(FlushScreenTextures) (void)
{
	pglDeleteTextures(1, &screentexture);
	pglDeleteTextures(1, &startScreenWipe);
	pglDeleteTextures(1, &endScreenWipe);
	pglDeleteTextures(1, &finalScreenTexture);
	screentexture = 0;
	startScreenWipe = 0;
	endScreenWipe = 0;
	finalScreenTexture = 0;
}

// Create Screen to fade from
EXPORT void HWRAPI(StartScreenWipe) (void)
{
	INT32 texsize = 2048;
	boolean firstTime = (startScreenWipe == 0);

	// Use a power of two texture, dammit
	if(screen_width <= 512)
		texsize = 512;
	else if(screen_width <= 1024)
		texsize = 1024;

	// Create screen texture
	if (firstTime)
		startScreenWipe = SCRTEX_STARTSCREENWIPE;
	pglBindTexture(GL_TEXTURE_2D, startScreenWipe);

	if (firstTime)
	{
		pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		Clamp2D(GL_TEXTURE_WRAP_S);
		Clamp2D(GL_TEXTURE_WRAP_T);
		pglCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, texsize, texsize, 0);
	}
	else
		pglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, texsize, texsize);

	tex_downloaded = startScreenWipe;
}

// Create Screen to fade to
EXPORT void HWRAPI(EndScreenWipe)(void)
{
	INT32 texsize = 2048;
	boolean firstTime = (endScreenWipe == 0);

	// Use a power of two texture, dammit
	if(screen_width <= 512)
		texsize = 512;
	else if(screen_width <= 1024)
		texsize = 1024;

	// Create screen texture
	if (firstTime)
		endScreenWipe = SCRTEX_ENDSCREENWIPE;
	pglBindTexture(GL_TEXTURE_2D, endScreenWipe);

	if (firstTime)
	{
		pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		Clamp2D(GL_TEXTURE_WRAP_S);
		Clamp2D(GL_TEXTURE_WRAP_T);
		pglCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, texsize, texsize, 0);
	}
	else
		pglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, texsize, texsize);

	tex_downloaded = endScreenWipe;
}

// Draw the last scene under the intermission
EXPORT void HWRAPI(DrawIntermissionBG)(void)
{
	float xfix, yfix;
	INT32 texsize = 2048;

	if(screen_width <= 1024)
		texsize = 1024;
	if(screen_width <= 512)
		texsize = 512;

	xfix = 1/((float)(texsize)/((float)((screen_width))));
	yfix = 1/((float)(texsize)/((float)((screen_height))));

	pglClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	pglBindTexture(GL_TEXTURE_2D, screentexture);
	pglBegin(GL_QUADS);

		pglColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		// Bottom left
		pglTexCoord2f(0.0f, 0.0f);
		pglVertex3f(-1.0f, -1.0f, 1.0f);

		// Top left
		pglTexCoord2f(0.0f, yfix);
		pglVertex3f(-1.0f, 1.0f, 1.0f);

		// Top right
		pglTexCoord2f(xfix, yfix);
		pglVertex3f(1.0f, 1.0f, 1.0f);

		// Bottom right
		pglTexCoord2f(xfix, 0.0f);
		pglVertex3f(1.0f, -1.0f, 1.0f);

	pglEnd();

	tex_downloaded = screentexture;
}

// Do screen fades!
EXPORT void HWRAPI(DoScreenWipe)(void)
{
	INT32 texsize = 2048;
	float xfix, yfix;

	// Use a power of two texture, dammit
	if(screen_width <= 1024)
		texsize = 1024;
	if(screen_width <= 512)
		texsize = 512;

	xfix = 1/((float)(texsize)/((float)((screen_width))));
	yfix = 1/((float)(texsize)/((float)((screen_height))));

	pglClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	SetBlend(PF_Modulated|PF_NoDepthTest);

	// Draw the original screen
	pglBindTexture(GL_TEXTURE_2D, startScreenWipe);
	pglBegin(GL_QUADS);
		pglColor4f(1.0f, 1.0f, 1.0f, 1.0f);

		// Bottom left
		pglTexCoord2f(0.0f, 0.0f);
		pglVertex3f(-1.0f, -1.0f, 1.0f);

		// Top left
		pglTexCoord2f(0.0f, yfix);
		pglVertex3f(-1.0f, 1.0f, 1.0f);

		// Top right
		pglTexCoord2f(xfix, yfix);
		pglVertex3f(1.0f, 1.0f, 1.0f);

		// Bottom right
		pglTexCoord2f(xfix, 0.0f);
		pglVertex3f(1.0f, -1.0f, 1.0f);

	pglEnd();

	SetBlend(PF_Modulated|PF_Translucent|PF_NoDepthTest);

	// Draw the end screen that fades in
	pglActiveTexture(GL_TEXTURE0);
	pglEnable(GL_TEXTURE_2D);
	pglBindTexture(GL_TEXTURE_2D, endScreenWipe);
	pglTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	pglActiveTexture(GL_TEXTURE1);
	pglEnable(GL_TEXTURE_2D);
	pglBindTexture(GL_TEXTURE_2D, tex_downloaded);

	pglTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	pglBegin(GL_QUADS);
		pglColor4f(1.0f, 1.0f, 1.0f, 1.0f);

		// Bottom left
		pglMultiTexCoord2f(GL_TEXTURE0, 0.0f, 0.0f);
		pglMultiTexCoord2f(GL_TEXTURE1, 0.0f, 1.0f);
		pglVertex3f(-1.0f, -1.0f, 1.0f);

		// Top left
		pglMultiTexCoord2f(GL_TEXTURE0, 0.0f, yfix);
		pglMultiTexCoord2f(GL_TEXTURE1, 0.0f, 0.0f);
		pglVertex3f(-1.0f, 1.0f, 1.0f);

		// Top right
		pglMultiTexCoord2f(GL_TEXTURE0, xfix, yfix);
		pglMultiTexCoord2f(GL_TEXTURE1, 1.0f, 0.0f);
		pglVertex3f(1.0f, 1.0f, 1.0f);

		// Bottom right
		pglMultiTexCoord2f(GL_TEXTURE0, xfix, 0.0f);
		pglMultiTexCoord2f(GL_TEXTURE1, 1.0f, 1.0f);
		pglVertex3f(1.0f, -1.0f, 1.0f);
	pglEnd();

	pglDisable(GL_TEXTURE_2D); // disable the texture in the 2nd texture unit
	pglActiveTexture(GL_TEXTURE0);
	tex_downloaded = endScreenWipe;
}

// Create a texture from the screen.
EXPORT void HWRAPI(MakeScreenTexture) (void)
{
	INT32 texsize = 2048;
	boolean firstTime = (screentexture == 0);

	// Use a power of two texture, dammit
	if(screen_width <= 512)
		texsize = 512;
	else if(screen_width <= 1024)
		texsize = 1024;

	// Create screen texture
	if (firstTime)
		screentexture = SCRTEX_SCREENTEXTURE;
	pglBindTexture(GL_TEXTURE_2D, screentexture);

	if (firstTime)
	{
		pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		Clamp2D(GL_TEXTURE_WRAP_S);
		Clamp2D(GL_TEXTURE_WRAP_T);
		pglCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, texsize, texsize, 0);
	}
	else
		pglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, texsize, texsize);

	tex_downloaded = screentexture;
}

EXPORT void HWRAPI(MakeScreenFinalTexture) (void)
{
	INT32 texsize = 2048;
	boolean firstTime = (finalScreenTexture == 0);

	// Use a power of two texture, dammit
	if(screen_width <= 512)
		texsize = 512;
	else if(screen_width <= 1024)
		texsize = 1024;

	// Create screen texture
	if (firstTime)
		finalScreenTexture = SCRTEX_FINALSCREENTEXTURE;
	pglBindTexture(GL_TEXTURE_2D, finalScreenTexture);

	if (firstTime)
	{
		pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		Clamp2D(GL_TEXTURE_WRAP_S);
		Clamp2D(GL_TEXTURE_WRAP_T);
		pglCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, texsize, texsize, 0);
	}
	else
		pglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, texsize, texsize);

	tex_downloaded = finalScreenTexture;
}

EXPORT void HWRAPI(DrawScreenFinalTexture)(int width, int height)
{
	float xfix, yfix;
	float origaspect, newaspect;
	float xoff = 1, yoff = 1; // xoffset and yoffset for the polygon to have black bars around the screen
	FRGBAFloat clearColour;
	INT32 texsize = 2048;

	if(screen_width <= 1024)
		texsize = 1024;
	if(screen_width <= 512)
		texsize = 512;

	xfix = 1/((float)(texsize)/((float)((screen_width))));
	yfix = 1/((float)(texsize)/((float)((screen_height))));

	origaspect = (float)screen_width / screen_height;
	newaspect = (float)width / height;
	if (origaspect < newaspect)
	{
		xoff = origaspect / newaspect;
		yoff = 1;
	}
	else if (origaspect > newaspect)
	{
		xoff = 1;
		yoff = newaspect / origaspect;
	}

	pglViewport(0, 0, width, height);

	clearColour.red = clearColour.green = clearColour.blue = 0;
	clearColour.alpha = 1;
	ClearBuffer(true, false, &clearColour);
	pglBindTexture(GL_TEXTURE_2D, finalScreenTexture);
	pglBegin(GL_QUADS);

		pglColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		// Bottom left
		pglTexCoord2f(0.0f, 0.0f);
		pglVertex3f(-xoff, -yoff, 1.0f);

		// Top left
		pglTexCoord2f(0.0f, yfix);
		pglVertex3f(-xoff, yoff, 1.0f);

		// Top right
		pglTexCoord2f(xfix, yfix);
		pglVertex3f(xoff, yoff, 1.0f);

		// Bottom right
		pglTexCoord2f(xfix, 0.0f);
		pglVertex3f(xoff, -yoff, 1.0f);

	pglEnd();

	tex_downloaded = finalScreenTexture;
}

#endif //HWRENDER

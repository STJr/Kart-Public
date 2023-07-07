// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 1999-2018 by Sonic Team Junior.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  r_main.c
/// \brief Rendering main loop and setup functions,
///        utility functions (BSP, geometry, trigonometry).
///        See tables.c, too.

#include "am_map.h"
#include "d_main.h"
#include "doomdef.h"
#include "doomstat.h" // MAXSPLITSCREENPLAYERS
#include "g_game.h"
#include "g_input.h"
#include "i_video.h"
#include "keys.h"
#include "m_menu.h"
#include "m_random.h" // quake camera shake
#include "p_local.h"
#include "p_spec.h" // skyboxmo
#include "r_fps.h"  // Frame interpolation/uncapped
#include "r_local.h"
#include "r_sky.h"
#include "r_splats.h" // faB(21jan): testing
#include "st_stuff.h"
#include "v_video.h"
#include "z_zone.h"

#include "hardware/hw_main.h"

// profile stuff ---------------------------------------------------------
//#define TIMING
#ifdef TIMING
#include "p5prof.h"
INT64 mycount;
INT64 mytotal = 0;
// unsigned long  nombre = 100000;
#endif
// profile stuff ---------------------------------------------------------

// Fineangles in the SCREENWIDTH wide window.
#define FIELDOFVIEW 2048

// increment every time a check is made
size_t validcount = 1;

INT32 centerx, centery;

fixed_t centerxfrac, centeryfrac;
fixed_t projection;
fixed_t projectiony; // aspect ratio
fixed_t fovtan;      // field of view

// just for profiling purposes
size_t framecount;

size_t loopcount;

fixed_t viewx, viewy, viewz;
angle_t viewangle, aimingangle;
UINT8 viewssnum;
fixed_t viewcos, viewsin;
boolean skyVisible;
boolean
    skyVisiblePerPlayer[MAXSPLITSCREENPLAYERS]; // saved values of skyVisible
                                                // for each splitscreen player
sector_t *viewsector;
player_t *viewplayer;

// PORTALS!
// You can thank and/or curse JTE for these.
UINT8 portalrender;
sector_t *portalcullsector;
typedef struct portal_pair {
  INT32 line1;
  INT32 line2;
  UINT8 pass;
  struct portal_pair *next;

  fixed_t viewx;
  fixed_t viewy;
  fixed_t viewz;
  angle_t viewangle;

  INT32 start;
  INT32 end;
  INT16 *ceilingclip;
  INT16 *floorclip;
  fixed_t *frontscale;
} portal_pair;
portal_pair *portal_base, *portal_cap;
line_t *portalclipline;
INT32 portalclipstart, portalclipend;

fixed_t rendertimefrac;
fixed_t renderdeltatics;
boolean renderisnewtic;

//
// precalculated math tables
//
angle_t clipangle;
angle_t doubleclipangle;

// The viewangletox[viewangle + FINEANGLES/4] lookup
// maps the visible view angles to screen X coordinates,
// flattening the arc to a flat projection plane.
// There will be many angles mapped to the same X.
INT32 viewangletox[FINEANGLES / 2];

// The xtoviewangleangle[] table maps a screen pixel
// to the lowest viewangle that maps back to x ranges
// from clipangle to -clipangle.
angle_t xtoviewangle[MAXVIDWIDTH + 1];

lighttable_t *scalelight[LIGHTLEVELS][MAXLIGHTSCALE];
lighttable_t *scalelightfixed[MAXLIGHTSCALE];
lighttable_t *zlight[LIGHTLEVELS][MAXLIGHTZ];

// Hack to support extra boom colormaps.
size_t num_extra_colormaps;
extracolormap_t extra_colormaps[MAXCOLORMAPS];

static CV_PossibleValue_t drawdist_cons_t[] = {
    /*{256, "256"},*/ {512, "512"},
    {768, "768"},
    {1024, "1024"},
    {1536, "1536"},
    {2048, "2048"},
    {3072, "3072"},
    {4096, "4096"},
    {6144, "6144"},
    {8192, "8192"},
    {0, "Infinite"},
    {0, NULL}};

// static CV_PossibleValue_t precipdensity_cons_t[] = {{0, "None"}, {1,
// "Light"}, {2, "Moderate"}, {4, "Heavy"}, {6, "Thick"}, {8, "V.Thick"}, {0,
// NULL}};

static CV_PossibleValue_t drawdist_precip_cons_t[] = {
    {256, "256"},   {512, "512"},   {768, "768"}, {1024, "1024"},
    {1536, "1536"}, {2048, "2048"}, {0, "None"},  {0, NULL}};

static CV_PossibleValue_t fov_cons_t[] = {
    {5 * FRACUNIT, "MIN"}, {178 * FRACUNIT, "MAX"}, {0, NULL}};

// static CV_PossibleValue_t precipdensity_cons_t[] = {{0, "None"}, {1,
// "Light"}, {2, "Moderate"}, {4, "Heavy"}, {6, "Thick"}, {8, "V.Thick"}, {0,
// NULL}};
static CV_PossibleValue_t translucenthud_cons_t[] = {
    {0, "MIN"}, {10, "MAX"}, {0, NULL}};
static CV_PossibleValue_t maxportals_cons_t[] = {
    {0, "MIN"},
    {12, "MAX"},
    {0, NULL}}; // lmao rendering 32 portals, you're a card
static CV_PossibleValue_t homremoval_cons_t[] = {
    {0, "No"}, {1, "Yes"}, {2, "Flash"}, {0, NULL}};

static void Fov_OnChange(void);
static void ChaseCam_OnChange(void);
static void ChaseCam2_OnChange(void);
static void ChaseCam3_OnChange(void);
static void ChaseCam4_OnChange(void);
static void FlipCam_OnChange(void);
static void FlipCam2_OnChange(void);
static void FlipCam3_OnChange(void);
static void FlipCam4_OnChange(void);
void SendWeaponPref(void);
void SendWeaponPref2(void);
void SendWeaponPref3(void);
void SendWeaponPref4(void);

consvar_t cv_tailspickup = {"tailspickup",
                            "On",
                            CV_NETVAR | CV_NOSHOWHELP,
                            CV_OnOff,
                            NULL,
                            0,
                            NULL,
                            NULL,
                            0,
                            0,
                            NULL};
consvar_t cv_chasecam = {"chasecam", "On", CV_CALL, CV_OnOff, ChaseCam_OnChange,
                         0,          NULL, NULL,    0,        0,
                         NULL};
consvar_t cv_chasecam2 = {
    "chasecam2", "On", CV_CALL, CV_OnOff, ChaseCam2_OnChange, 0, NULL,
    NULL,        0,    0,       NULL};
consvar_t cv_chasecam3 = {
    "chasecam3", "On", CV_CALL, CV_OnOff, ChaseCam3_OnChange, 0, NULL,
    NULL,        0,    0,       NULL};
consvar_t cv_chasecam4 = {
    "chasecam4", "On", CV_CALL, CV_OnOff, ChaseCam4_OnChange, 0, NULL,
    NULL,        0,    0,       NULL};
consvar_t cv_flipcam = {"flipcam",
                        "No",
                        CV_SAVE | CV_CALL | CV_NOINIT,
                        CV_YesNo,
                        FlipCam_OnChange,
                        0,
                        NULL,
                        NULL,
                        0,
                        0,
                        NULL};
consvar_t cv_flipcam2 = {"flipcam2",
                         "No",
                         CV_SAVE | CV_CALL | CV_NOINIT,
                         CV_YesNo,
                         FlipCam2_OnChange,
                         0,
                         NULL,
                         NULL,
                         0,
                         0,
                         NULL};
consvar_t cv_flipcam3 = {"flipcam3",
                         "No",
                         CV_SAVE | CV_CALL | CV_NOINIT,
                         CV_YesNo,
                         FlipCam3_OnChange,
                         0,
                         NULL,
                         NULL,
                         0,
                         0,
                         NULL};
consvar_t cv_flipcam4 = {"flipcam4",
                         "No",
                         CV_SAVE | CV_CALL | CV_NOINIT,
                         CV_YesNo,
                         FlipCam4_OnChange,
                         0,
                         NULL,
                         NULL,
                         0,
                         0,
                         NULL};

consvar_t cv_shadow = {"shadow", "Off", CV_SAVE, CV_OnOff, NULL, 0,
                       NULL,     NULL,  0,       0,        NULL};
consvar_t cv_shadowoffs = {
    "offsetshadows", "Off", CV_SAVE, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_skybox = {"skybox", "On", CV_SAVE, CV_OnOff, NULL, 0,
                       NULL,     NULL, 0,       0,        NULL};
consvar_t cv_soniccd = {"soniccd", "Off", CV_NETVAR | CV_NOSHOWHELP,
                        CV_OnOff,  NULL,  0,
                        NULL,      NULL,  0,
                        0,         NULL};
consvar_t cv_allowmlook = {"allowmlook", "Yes", CV_NETVAR, CV_YesNo, NULL, 0,
                           NULL,         NULL,  0,         0,        NULL};
consvar_t cv_showhud = {"showhud", "Yes", CV_CALL, CV_YesNo, R_SetViewSize, 0,
                        NULL,      NULL,  0,       0,        NULL};
consvar_t cv_translucenthud = {"translucenthud",
                               "10",
                               CV_SAVE,
                               translucenthud_cons_t,
                               NULL,
                               0,
                               NULL,
                               NULL,
                               0,
                               0,
                               NULL};

consvar_t cv_translucency = {"translucency", "On", CV_SAVE, CV_OnOff, NULL, 0,
                             NULL,           NULL, 0,       0,        NULL};
consvar_t cv_drawdist = {"drawdist", "Infinite", CV_SAVE, drawdist_cons_t,
                         NULL,       0,          NULL,    NULL,
                         0,          0,          NULL};
// consvar_t cv_drawdist_nights = {"drawdist_nights", "2048", CV_SAVE,
// drawdist_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_drawdist_precip = {"drawdist_precip",
                                "1024",
                                CV_SAVE,
                                drawdist_precip_cons_t,
                                NULL,
                                0,
                                NULL,
                                NULL,
                                0,
                                0,
                                NULL};
// consvar_t cv_precipdensity = {"precipdensity", "Moderate", CV_SAVE,
// precipdensity_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};

// cap fov, fov too high tears software apart.
consvar_t cv_fov = {"fov",      "90",         CV_FLOAT | CV_CALL | CV_SAVE,
                    fov_cons_t, Fov_OnChange, 0,
                    NULL,       NULL,         0,
                    0,          NULL};

// Okay, whoever said homremoval causes a performance hit should be shot.
consvar_t cv_homremoval = {
    "homremoval", "Yes", CV_SAVE, homremoval_cons_t, NULL, 0, NULL, NULL, 0, 0,
    NULL};

consvar_t cv_maxportals = {
    "maxportals", "2", CV_SAVE, maxportals_cons_t, NULL, 0, NULL, NULL, 0, 0,
    NULL};

void SplitScreen_OnChange(void) {
  UINT8 i;

  // recompute screen size
  R_ExecuteSetViewSize();

  if (!demo.playback && !botingame) {
    for (i = 1; i < MAXSPLITSCREENPLAYERS; i++) {
      if (i > splitscreen)
        CL_RemoveSplitscreenPlayer(displayplayers[i]);
      else
        CL_AddSplitscreenPlayer();
    }

    if (server && !netgame)
      multiplayer = splitscreen;
  } else {
    for (i = 1; i < MAXSPLITSCREENPLAYERS; i++)
      displayplayers[i] = consoleplayer;

    for (i = 0; i < MAXPLAYERS; i++) {
      if (playeringame[i] && i != consoleplayer) {
        UINT8 j;
        for (j = 1; j < MAXSPLITSCREENPLAYERS; j++) {
          if (displayplayers[j] == consoleplayer) {
            displayplayers[j] = i;
            break;
          }
        }

        if (j == MAXSPLITSCREENPLAYERS)
          break;
      }
    }
  }
}
static void Fov_OnChange(void) {
  // Shouldn't be needed with render parity?
  // if ((netgame || multiplayer) && !cv_debug && cv_fov.value != 90*FRACUNIT)
  //	CV_Set(&cv_fov, cv_fov.defaultvalue);

  R_SetViewSize();
}

static void ChaseCam_OnChange(void) {
  /*if (!cv_chasecam.value || !cv_useranalog.value)
          CV_SetValue(&cv_analog, 0);
  else
          CV_SetValue(&cv_analog, 1);*/
}

static void ChaseCam2_OnChange(void) {
  if (botingame)
    return;
  /*if (!cv_chasecam2.value || !cv_useranalog2.value)
          CV_SetValue(&cv_analog2, 0);
  else
          CV_SetValue(&cv_analog2, 1);*/
}

static void ChaseCam3_OnChange(void) {
  /*if (!cv_chasecam3.value || !cv_useranalog3.value)
          CV_SetValue(&cv_analog3, 0);
  else
          CV_SetValue(&cv_analog3, 1);*/
}

static void ChaseCam4_OnChange(void) {
  /*if (!cv_chasecam4.value || !cv_useranalog4.value)
          CV_SetValue(&cv_analog4, 0);
  else
          CV_SetValue(&cv_analog4, 1);*/
}

static void FlipCam_OnChange(void) { SendWeaponPref(); }

static void FlipCam2_OnChange(void) { SendWeaponPref2(); }

static void FlipCam3_OnChange(void) { SendWeaponPref3(); }

static void FlipCam4_OnChange(void) { SendWeaponPref4(); }

//
// R_PointOnSide
// Traverse BSP (sub) tree,
// check point against partition plane.
// Returns side 0 (front) or 1 (back).
//
// killough 5/2/98: reformatted
//
INT32
R_PointOnSide(fixed_t x, fixed_t y, node_t *node) {
  if (!node->dx)
    return x <= node->x ? node->dy > 0 : node->dy < 0;

  if (!node->dy)
    return y <= node->y ? node->dx < 0 : node->dx > 0;

  x -= node->x;
  y -= node->y;

  // Try to quickly decide by looking at sign bits.
  if ((node->dy ^ node->dx ^ x ^ y) < 0)
    return (node->dy ^ x) < 0; // (left is negative)
  return FixedMul(y, node->dx >> FRACBITS) >= FixedMul(node->dy >> FRACBITS, x);
}

// killough 5/2/98: reformatted
INT32
R_PointOnSegSide(fixed_t x, fixed_t y, seg_t *line) {
  fixed_t lx = line->v1->x;
  fixed_t ly = line->v1->y;
  fixed_t ldx = line->v2->x - lx;
  fixed_t ldy = line->v2->y - ly;

  if (!ldx)
    return x <= lx ? ldy > 0 : ldy < 0;

  if (!ldy)
    return y <= ly ? ldx < 0 : ldx > 0;

  x -= lx;
  y -= ly;

  // Try to quickly decide by looking at sign bits.
  if ((ldy ^ ldx ^ x ^ y) < 0)
    return (ldy ^ x) < 0; // (left is negative)
  return FixedMul(y, ldx >> FRACBITS) >= FixedMul(ldy >> FRACBITS, x);
}

//
// R_PointToAngle
// To get a global angle from cartesian coordinates,
//  the coordinates are flipped until they are in
//  the first octant of the coordinate system, then
//  the y (<=x) is scaled and divided by x to get a
//  tangent (slope) value which is looked up in the
//  tantoangle[] table. The +1 size of tantoangle[]
//  is to handle the case when x==y without additional
//  checking.
//
// killough 5/2/98: reformatted, cleaned up

angle_t R_PointToAngle(fixed_t x, fixed_t y) {
  return (y -= viewy, (x -= viewx) || y)
             ? x >= 0 ? y >= 0 ? (x > y) ? tantoangle[SlopeDiv(y, x)]
                                         : // octant 0
                                     ANGLE_90 - tantoangle[SlopeDiv(x, y)]
                               : // octant 1
                            x > (y = -y) ? 0 - tantoangle[SlopeDiv(y, x)]
                                         : // octant 8
                            ANGLE_270 + tantoangle[SlopeDiv(x, y)]
                      : // octant 7
                   y >= 0 ? (x = -x) > y
                                ? ANGLE_180 - tantoangle[SlopeDiv(y, x)]
                                : // octant 3
                                ANGLE_90 + tantoangle[SlopeDiv(x, y)]
                          : // octant 2
                   (x = -x) > (y = -y) ? ANGLE_180 + tantoangle[SlopeDiv(y, x)]
                                       : // octant 4
                   ANGLE_270 - tantoangle[SlopeDiv(x, y)]
             : // octant 5
             0;
}

angle_t R_PointToAngle2(fixed_t pviewx, fixed_t pviewy, fixed_t x, fixed_t y) {
  return (y -= pviewy, (x -= pviewx) || y)
             ? x >= 0 ? y >= 0 ? (x > y) ? tantoangle[SlopeDiv(y, x)]
                                         : // octant 0
                                     ANGLE_90 - tantoangle[SlopeDiv(x, y)]
                               : // octant 1
                            x > (y = -y) ? 0 - tantoangle[SlopeDiv(y, x)]
                                         : // octant 8
                            ANGLE_270 + tantoangle[SlopeDiv(x, y)]
                      : // octant 7
                   y >= 0 ? (x = -x) > y
                                ? ANGLE_180 - tantoangle[SlopeDiv(y, x)]
                                : // octant 3
                                ANGLE_90 + tantoangle[SlopeDiv(x, y)]
                          : // octant 2
                   (x = -x) > (y = -y) ? ANGLE_180 + tantoangle[SlopeDiv(y, x)]
                                       : // octant 4
                   ANGLE_270 - tantoangle[SlopeDiv(x, y)]
             : // octant 5
             0;
}

fixed_t R_PointToDist2(fixed_t px2, fixed_t py2, fixed_t px1, fixed_t py1) {
  return FixedHypot(px1 - px2, py1 - py2);
}

// Little extra utility. Works in the same way as R_PointToAngle2
fixed_t R_PointToDist(fixed_t x, fixed_t y) {
  return R_PointToDist2(viewx, viewy, x, y);
}

angle_t R_PointToAngleEx(INT32 x2, INT32 y2, INT32 x1, INT32 y1) {
  INT64 dx = x1 - x2;
  INT64 dy = y1 - y2;
  if (dx < INT32_MIN || dx > INT32_MAX || dy < INT32_MIN || dy > INT32_MAX) {
    x1 = (int)(dx / 2 + x2);
    y1 = (int)(dy / 2 + y2);
  }
  return (y1 -= y2, (x1 -= x2) || y1)
             ? x1 >= 0 ? y1 >= 0 ? (x1 > y1) ? tantoangle[SlopeDivEx(y1, x1)]
                                             : // octant 0
                                       ANGLE_90 - tantoangle[SlopeDivEx(x1, y1)]
                                 : // octant 1
                             x1 > (y1 = -y1)
                             ? 0 - tantoangle[SlopeDivEx(y1, x1)]
                             : // octant 8
                             ANGLE_270 + tantoangle[SlopeDivEx(x1, y1)]
                       : // octant 7
                   y1 >= 0 ? (x1 = -x1) > y1
                                 ? ANGLE_180 - tantoangle[SlopeDivEx(y1, x1)]
                                 : // octant 3
                                 ANGLE_90 + tantoangle[SlopeDivEx(x1, y1)]
                           : // octant 2
                   (x1 = -x1) > (y1 = -y1)
                   ? ANGLE_180 + tantoangle[SlopeDivEx(y1, x1)]
                   : // octant 4
                   ANGLE_270 - tantoangle[SlopeDivEx(x1, y1)]
             : // octant 5
             0;
}

//
// R_ScaleFromGlobalAngle
// Returns the texture mapping scale for the current line (horizontal span)
//  at the given angle.
// rw_distance must be calculated first.
//
// killough 5/2/98: reformatted, cleaned up
//
// note: THIS IS USED ONLY FOR WALLS!
fixed_t R_ScaleFromGlobalAngle(angle_t visangle) {
  angle_t anglea = ANGLE_90 + (visangle - viewangle);
  angle_t angleb = ANGLE_90 + (visangle - rw_normalangle);
  fixed_t den = FixedMul(rw_distance, FINESINE(anglea >> ANGLETOFINESHIFT));
  // proff 11/06/98: Changed for high-res
  fixed_t num = FixedMul(projectiony, FINESINE(angleb >> ANGLETOFINESHIFT));

  if (den > num >> 16) {
    num = FixedDiv(num, den);
    if (num > 64 * FRACUNIT)
      return 64 * FRACUNIT;
    if (num < 256)
      return 256;
    return num;
  }
  return 64 * FRACUNIT;
}

//
// R_DoCulling
// Checks viewz and top/bottom heights of an item against culling planes
// Returns true if the item is to be culled, i.e it shouldn't be drawn!
// if ML_NOCLIMB is set, the camera view is required to be in the same area for
// culling to occur
boolean R_DoCulling(line_t *cullheight, line_t *viewcullheight, fixed_t vz,
                    fixed_t bottomh, fixed_t toph) {
  fixed_t cullplane;

  if (!cullheight)
    return false;

  cullplane = cullheight->frontsector->floorheight;
  if (cullheight->flags & ML_NOCLIMB) // Group culling
  {
    if (!viewcullheight)
      return false;

    // Make sure this is part of the same group
    if (viewcullheight->frontsector == cullheight->frontsector) {
      // OK, we can cull
      if (vz > cullplane && toph < cullplane) // Cull if below plane
        return true;

      if (bottomh > cullplane && vz <= cullplane) // Cull if above plane
        return true;
    }
  } else // Quick culling
  {
    if (vz > cullplane && toph < cullplane) // Cull if below plane
      return true;

    if (bottomh > cullplane && vz <= cullplane) // Cull if above plane
      return true;
  }

  return false;
}

//
// R_InitTextureMapping
//
static void R_InitTextureMapping(void) {
  INT32 i;
  INT32 x;
  INT32 t;
  fixed_t focallength;

  // Use tangent table to generate viewangletox:
  //  viewangletox will give the next greatest x
  //  after the view angle.
  //
  // Calc focallength
  //  so FIELDOFVIEW angles covers SCREENWIDTH.
  focallength =
      FixedDiv(projection,
               FINETANGENT(FINEANGLES / 4 + /*cv_fov.value*/ FIELDOFVIEW / 2));

  focallengthf = FIXED_TO_FLOAT(focallength);

  for (i = 0; i < FINEANGLES / 2; i++) {
    if (FINETANGENT(i) > fovtan * 2)
      t = -1;
    else if (FINETANGENT(i) < -fovtan * 2)
      t = viewwidth + 1;
    else {
      t = FixedMul(FINETANGENT(i), focallength);
      t = (centerxfrac - t + FRACUNIT - 1) >> FRACBITS;

      if (t < -1)
        t = -1;
      else if (t > viewwidth + 1)
        t = viewwidth + 1;
    }
    viewangletox[i] = t;
  }

  // Scan viewangletox[] to generate xtoviewangle[]:
  //  xtoviewangle will give the smallest view angle
  //  that maps to x.
  for (x = 0; x <= viewwidth; x++) {
    i = 0;
    while (viewangletox[i] > x)
      i++;
    xtoviewangle[x] = (i << ANGLETOFINESHIFT) - ANGLE_90;
  }

  // Take out the fencepost cases from viewangletox.
  for (i = 0; i < FINEANGLES / 2; i++) {
    if (viewangletox[i] == -1)
      viewangletox[i] = 0;
    else if (viewangletox[i] == viewwidth + 1)
      viewangletox[i] = viewwidth;
  }

  clipangle = xtoviewangle[0];
  doubleclipangle = clipangle * 2;
}

//
// R_InitLightTables
// Only inits the zlight table,
//  because the scalelight table changes with view size.
//
#define DISTMAP 2

static inline void R_InitLightTables(void) {
  INT32 i;
  INT32 j;
  INT32 level;
  INT32 startmapl;
  INT32 scale;

  // Calculate the light levels to use
  //  for each level / distance combination.
  for (i = 0; i < LIGHTLEVELS; i++) {
    startmapl = ((LIGHTLEVELS - 1 - i) * 2) * NUMCOLORMAPS / LIGHTLEVELS;
    for (j = 0; j < MAXLIGHTZ; j++) {
      // added : 02-02-98 : use BASEVIDWIDTH, vid.width is not set already,
      //  and it seems it needs to be calculated only once.
      scale = FixedDiv((BASEVIDWIDTH / 2 * FRACUNIT), (j + 1) << LIGHTZSHIFT);
      scale >>= LIGHTSCALESHIFT;
      level = startmapl - scale / DISTMAP;

      if (level < 0)
        level = 0;

      if (level >= NUMCOLORMAPS)
        level = NUMCOLORMAPS - 1;

      zlight[i][j] = colormaps + level * 256;
    }
  }
}

//
// R_SetViewSize
// Do not really change anything here,
// because it might be in the middle of a refresh.
// The change will take effect next refresh.
//
boolean setsizeneeded;

void R_SetViewSize(void) { setsizeneeded = true; }

//
// R_ExecuteSetViewSize
//
void R_ExecuteSetViewSize(void) {
  fixed_t dy;
  INT32 i;
  INT32 j;
  INT32 level;
  INT32 startmapl;
  angle_t fov;

  setsizeneeded = false;

  if (rendermode == render_none)
    return;

  // status bar overlay
  st_overlay = cv_showhud.value;

  scaledviewwidth = vid.width;
  viewheight = vid.height;

  if (splitscreen)
    viewheight >>= 1;

  viewwidth = scaledviewwidth;

  if (splitscreen > 1) {
    viewwidth >>= 1;
    scaledviewwidth >>= 1;
  }

  centerx = viewwidth / 2;
  centery = viewheight / 2;
  centerxfrac = centerx << FRACBITS;
  centeryfrac = centery << FRACBITS;

  fov = FixedAngle(cv_fov.value / 2) + ANGLE_90;
  fovtan = FINETANGENT(fov >> ANGLETOFINESHIFT);
  if (splitscreen == 1) // Splitscreen FOV should be adjusted to maintain
                        // expected vertical view
    fovtan = 17 * fovtan / 10;

  projection = projectiony = FixedDiv(centerxfrac, fovtan);

  R_InitViewBuffer(scaledviewwidth, viewheight);

  R_InitTextureMapping();

  // thing clipping
  for (i = 0; i < viewwidth; i++)
    screenheightarray[i] = (INT16)viewheight;

  // setup sky scaling
  R_SetSkyScale();

  // planes
  if (rendermode == render_soft) {
    // this is only used for planes rendering in software mode
    j = viewheight * 16;
    for (i = 0; i < j; i++) {
      dy = ((i - viewheight * 8) << FRACBITS) + FRACUNIT / 2;
      dy = FixedMul(abs(dy), fovtan);
      yslopetab[i] = FixedDiv(centerx * FRACUNIT, dy);
    }
  }

  memset(scalelight, 0xFF, sizeof(scalelight));

  // Calculate the light levels to use for each level/scale combination.
  for (i = 0; i < LIGHTLEVELS; i++) {
    startmapl = ((LIGHTLEVELS - 1 - i) * 2) * NUMCOLORMAPS / LIGHTLEVELS;
    for (j = 0; j < MAXLIGHTSCALE; j++) {
      level = startmapl - j * vid.width / (viewwidth) / DISTMAP;

      if (level < 0)
        level = 0;

      if (level >= NUMCOLORMAPS)
        level = NUMCOLORMAPS - 1;

      scalelight[i][j] = colormaps + level * 256;
    }
  }

  // continue to do the software setviewsize as long as we use the reference
  // software view
  if (rendermode != render_soft)
    HWR_SetViewSize();

  am_recalc = true;
}

//
// R_Init
//

void R_Init(void) {
  // screensize independent
  // I_OutputMsg("\nR_InitData");
  R_InitData();

  // I_OutputMsg("\nR_InitViewBorder");
  R_InitViewBorder();
  R_SetViewSize(); // setsizeneeded is set true

  // I_OutputMsg("\nR_InitPlanes");
  R_InitPlanes();

  // this is now done by SCR_Recalc() at the first mode set
  // I_OutputMsg("\nR_InitLightTables");
  R_InitLightTables();

  // I_OutputMsg("\nR_InitTranslationTables\n");
  R_InitTranslationTables();

  R_InitDrawNodes();

  framecount = 0;
}

//
// R_PointInSubsector
//
subsector_t *R_PointInSubsector(fixed_t x, fixed_t y) {
  size_t nodenum = numnodes - 1;

  while (!(nodenum & NF_SUBSECTOR))
    nodenum = nodes[nodenum].children[R_PointOnSide(x, y, nodes + nodenum)];

  return &subsectors[nodenum & ~NF_SUBSECTOR];
}

//
// R_IsPointInSubsector, same as above but returns 0 if not in subsector
//
subsector_t *R_IsPointInSubsector(fixed_t x, fixed_t y) {
  node_t *node;
  INT32 side, i;
  size_t nodenum;
  subsector_t *ret;

  // single subsector is a special case
  if (numnodes == 0)
    return subsectors;

  nodenum = numnodes - 1;

  while (!(nodenum & NF_SUBSECTOR)) {
    node = &nodes[nodenum];
    side = R_PointOnSide(x, y, node);
    nodenum = node->children[side];
  }

  ret = &subsectors[nodenum & ~NF_SUBSECTOR];
  for (i = 0; i < ret->numlines; i++)
    // if (R_PointOnSegSide(x, y, &segs[ret->firstline + i])) -- breaks in ogl
    // because polyvertex_t cast over vertex pointers
    if (P_PointOnLineSide(x, y, segs[ret->firstline + i].linedef) !=
        segs[ret->firstline + i].side)
      return 0;

  return ret;
}

//
// R_SetupFrame
//

static mobj_t *viewmobj;

void R_SkyboxFrame(player_t *player) {
  camera_t *thiscam = &camera[0];
  UINT8 i;

  if (splitscreen) {
    for (i = 0; i <= splitscreen; i++) {
      if (player == &players[displayplayers[i]]) {
        thiscam = &camera[i];
        R_SetViewContext(VIEWCONTEXT_SKY1 + i);
        break;
      }
    }
  } else {
    R_SetViewContext(VIEWCONTEXT_SKY1);
  }

  // cut-away view stuff
  newview->sky = true;
  viewmobj = skyboxmo[0];
#ifdef PARANOIA
  if (!viewmobj) {
    const size_t playeri = (size_t)(player - players);
    I_Error("R_SkyboxFrame: viewmobj null (player %s)", sizeu1(playeri));
  }
#endif
  if (player->awayviewtics) {
    newview->aim = player->awayviewaiming;
    newview->angle = player->awayviewmobj->angle;
  } else if (thiscam->chase) {
    newview->aim = thiscam->aiming;
    newview->angle = thiscam->angle;
  } else {
    newview->aim = player->aiming;
    newview->angle = player->mo->angle;
    if (!demo.playback && player->playerstate != PST_DEAD) {
      if (player == &players[consoleplayer]) {
        newview->angle = localangle[0]; // WARNING: camera uses this
        newview->aim = localaiming[0];
      } else if (splitscreen) {
        for (i = 1; i <= splitscreen; i++) {
          if (player == &players[displayplayers[i]]) {
            newview->angle = localangle[i];
            newview->aim = localaiming[i];
            break;
          }
        }
      }
    }
  }
  newview->angle += viewmobj->angle;

  newview->player = player;

  newview->x = viewmobj->x;
  newview->y = viewmobj->y;
  newview->z = 0;
  if (viewmobj->spawnpoint)
    newview->z = ((fixed_t)viewmobj->spawnpoint->angle) << FRACBITS;

  newview->x += quake.x;
  newview->y += quake.y;
  newview->z += quake.z;

  if (mapheaderinfo[gamemap - 1]) {
    mapheader_t *mh = mapheaderinfo[gamemap - 1];
    if (player->awayviewtics) {
      if (skyboxmo[1]) {
        fixed_t x = 0, y = 0;
        if (mh->skybox_scalex > 0)
          x = (player->awayviewmobj->x - skyboxmo[1]->x) / mh->skybox_scalex;
        else if (mh->skybox_scalex < 0)
          x = (player->awayviewmobj->x - skyboxmo[1]->x) * -mh->skybox_scalex;

        if (mh->skybox_scaley > 0)
          y = (player->awayviewmobj->y - skyboxmo[1]->y) / mh->skybox_scaley;
        else if (mh->skybox_scaley < 0)
          y = (player->awayviewmobj->y - skyboxmo[1]->y) * -mh->skybox_scaley;

        if (viewmobj->angle == 0) {
          newview->x += x;
          newview->y += y;
        } else if (viewmobj->angle == ANGLE_90) {
          newview->x -= y;
          newview->y += x;
        } else if (viewmobj->angle == ANGLE_180) {
          newview->x -= x;
          newview->y -= y;
        } else if (viewmobj->angle == ANGLE_270) {
          newview->x += y;
          newview->y -= x;
        } else {
          angle_t ang = viewmobj->angle >> ANGLETOFINESHIFT;
          newview->x +=
              FixedMul(x, FINECOSINE(ang)) - FixedMul(y, FINESINE(ang));
          newview->y +=
              FixedMul(x, FINESINE(ang)) + FixedMul(y, FINECOSINE(ang));
        }
      }
      if (mh->skybox_scalez > 0)
        newview->z +=
            (player->awayviewmobj->z + 20 * FRACUNIT) / mh->skybox_scalez;
      else if (mh->skybox_scalez < 0)
        newview->z +=
            (player->awayviewmobj->z + 20 * FRACUNIT) * -mh->skybox_scalez;
    } else if (thiscam->chase) {
      if (skyboxmo[1]) {
        fixed_t x = 0, y = 0;
        if (mh->skybox_scalex > 0)
          x = (thiscam->x - skyboxmo[1]->x) / mh->skybox_scalex;
        else if (mh->skybox_scalex < 0)
          x = (thiscam->x - skyboxmo[1]->x) * -mh->skybox_scalex;

        if (mh->skybox_scaley > 0)
          y = (thiscam->y - skyboxmo[1]->y) / mh->skybox_scaley;
        else if (mh->skybox_scaley < 0)
          y = (thiscam->y - skyboxmo[1]->y) * -mh->skybox_scaley;

        if (viewmobj->angle == 0) {
          newview->x += x;
          newview->y += y;
        } else if (viewmobj->angle == ANGLE_90) {
          newview->x -= y;
          newview->y += x;
        } else if (viewmobj->angle == ANGLE_180) {
          newview->x -= x;
          newview->y -= y;
        } else if (viewmobj->angle == ANGLE_270) {
          newview->x += y;
          newview->y -= x;
        } else {
          angle_t ang = viewmobj->angle >> ANGLETOFINESHIFT;
          newview->x +=
              FixedMul(x, FINECOSINE(ang)) - FixedMul(y, FINESINE(ang));
          newview->y +=
              FixedMul(x, FINESINE(ang)) + FixedMul(y, FINECOSINE(ang));
        }
      }
      if (mh->skybox_scalez > 0)
        newview->z += (thiscam->z + (thiscam->height >> 1)) / mh->skybox_scalez;
      else if (mh->skybox_scalez < 0)
        newview->z +=
            (thiscam->z + (thiscam->height >> 1)) * -mh->skybox_scalez;
    } else {
      if (skyboxmo[1]) {
        fixed_t x = 0, y = 0;
        if (mh->skybox_scalex > 0)
          x = (player->mo->x - skyboxmo[1]->x) / mh->skybox_scalex;
        else if (mh->skybox_scalex < 0)
          x = (player->mo->x - skyboxmo[1]->x) * -mh->skybox_scalex;
        if (mh->skybox_scaley > 0)
          y = (player->mo->y - skyboxmo[1]->y) / mh->skybox_scaley;
        else if (mh->skybox_scaley < 0)
          y = (player->mo->y - skyboxmo[1]->y) * -mh->skybox_scaley;

        if (viewmobj->angle == 0) {
          newview->x += x;
          newview->y += y;
        } else if (viewmobj->angle == ANGLE_90) {
          newview->x -= y;
          newview->y += x;
        } else if (viewmobj->angle == ANGLE_180) {
          newview->x -= x;
          newview->y -= y;
        } else if (viewmobj->angle == ANGLE_270) {
          newview->x += y;
          newview->y -= x;
        } else {
          angle_t ang = viewmobj->angle >> ANGLETOFINESHIFT;
          newview->x +=
              FixedMul(x, FINECOSINE(ang)) - FixedMul(y, FINESINE(ang));
          newview->y +=
              FixedMul(x, FINESINE(ang)) + FixedMul(y, FINECOSINE(ang));
        }
      }
      if (mh->skybox_scalez > 0)
        newview->z += player->viewz / mh->skybox_scalez;
      else if (mh->skybox_scalez < 0)
        newview->z += player->viewz * -mh->skybox_scalez;
    }
  }

  if (viewmobj->subsector)
    newview->sector = viewmobj->subsector->sector;
  else
    newview->sector = R_PointInSubsector(newview->x, newview->y)->sector;

  // newview->sin = FINESINE(viewangle>>ANGLETOFINESHIFT);
  // newview->cos = FINECOSINE(viewangle>>ANGLETOFINESHIFT);

  R_InterpolateView(R_UsingFrameInterpolation() ? rendertimefrac : FRACUNIT);
}

void R_SetupFrame(player_t *player, boolean skybox) {
  camera_t *thiscam;
  boolean chasecam = false;

  if (splitscreen > 2 && player == &players[displayplayers[3]]) {
    thiscam = &camera[3];
    chasecam = (cv_chasecam4.value != 0);
    R_SetViewContext(VIEWCONTEXT_PLAYER4);
    if (thiscam->reset) {
      R_ResetViewInterpolation(4);
      thiscam->reset = false;
    }
  } else if (splitscreen > 1 && player == &players[displayplayers[2]]) {
    thiscam = &camera[2];
    chasecam = (cv_chasecam3.value != 0);
    R_SetViewContext(VIEWCONTEXT_PLAYER3);
    if (thiscam->reset) {
      R_ResetViewInterpolation(3);
      thiscam->reset = false;
    }
  } else if (splitscreen && player == &players[displayplayers[1]]) {
    thiscam = &camera[1];
    chasecam = (cv_chasecam2.value != 0);
    R_SetViewContext(VIEWCONTEXT_PLAYER2);
    if (thiscam->reset) {
      R_ResetViewInterpolation(2);
      thiscam->reset = false;
    }
  } else {
    thiscam = &camera[0];
    chasecam = (cv_chasecam.value != 0);
    R_SetViewContext(VIEWCONTEXT_PLAYER1);
    if (thiscam->reset) {
      R_ResetViewInterpolation(1);
      thiscam->reset = false;
    }
  }

  if (player->spectator) // no spectator chasecam
    chasecam = false;    // force chasecam off
  else if (player->playerstate == PST_DEAD || player->exiting)
    chasecam = true; // force chasecam on

  if (chasecam && !thiscam->chase) {
    P_ResetCamera(player, thiscam);
    thiscam->chase = true;
  } else if (!chasecam)
    thiscam->chase = false;

  newview->sky = !skybox;
  if (player->awayviewtics) {
    // cut-away view stuff
    viewmobj = player->awayviewmobj; // should be a MT_ALTVIEWMAN
    I_Assert(viewmobj != NULL);
    newview->z = viewmobj->z + 20 * FRACUNIT;
    newview->aim = player->awayviewaiming;
    newview->angle = viewmobj->angle;
  } else if (!player->spectator && chasecam)
  // use outside cam view
  {
    viewmobj = NULL;
    newview->z = thiscam->z + (thiscam->height >> 1);
    newview->aim = thiscam->aiming;
    newview->angle = thiscam->angle;
  } else
  // use the player's eyes view
  {
    newview->z = player->viewz;

    viewmobj = player->mo;
    I_Assert(viewmobj != NULL);

    newview->aim = player->aiming;
    newview->angle = viewmobj->angle;

    if (!demo.playback && player->playerstate != PST_DEAD) {
      if (player == &players[consoleplayer]) {
        newview->angle = localangle[0]; // WARNING: camera uses this
        newview->aim = localaiming[0];
      } else if (splitscreen) {
        UINT8 i;
        for (i = 1; i <= splitscreen; i++) {
          if (player == &players[displayplayers[i]]) {
            newview->angle = localangle[i];
            newview->aim = localaiming[i];
            break;
          }
        }
      }
    }
  }
  newview->z += quake.z;

  newview->player = player;

  if (chasecam && !player->awayviewtics && !player->spectator) {
    newview->x = thiscam->x;
    newview->y = thiscam->y;
    newview->x += quake.x;
    newview->y += quake.y;

    if (thiscam->subsector && thiscam->subsector->sector)
      newview->sector = thiscam->subsector->sector;
    else
      newview->sector = R_PointInSubsector(newview->x, newview->y)->sector;
  } else {
    newview->x = viewmobj->x;
    newview->y = viewmobj->y;
    newview->x += quake.x;
    newview->y += quake.y;

    if (viewmobj->subsector && thiscam->subsector->sector)
      newview->sector = viewmobj->subsector->sector;
    else
      newview->sector = R_PointInSubsector(newview->x, newview->y)->sector;
  }

  // newview->sin = FINESINE(viewangle>>ANGLETOFINESHIFT);
  // newview->cos = FINECOSINE(viewangle>>ANGLETOFINESHIFT);

  R_InterpolateView(R_UsingFrameInterpolation() ? rendertimefrac : FRACUNIT);
}

#define ANGLED_PORTALS

static void R_PortalFrame(line_t *start, line_t *dest, portal_pair *portal) {
  vertex_t dest_c, start_c;
#ifdef ANGLED_PORTALS
  // delta angle
  angle_t dangle = R_PointToAngle2(0, 0, dest->dx, dest->dy) -
                   R_PointToAngle2(start->dx, start->dy, 0, 0);
#endif

  // R_SetupFrame(player, false);
  viewx = portal->viewx;
  viewy = portal->viewy;
  viewz = portal->viewz;

  viewangle = portal->viewangle;
  // newview->sin = FINESINE(newview->angle>>ANGLETOFINESHIFT);
  // newview->cos = FINECOSINE(newview->angle>>ANGLETOFINESHIFT);

  portalcullsector = dest->frontsector;
  viewsector = dest->frontsector;
  portalclipline = dest;
  portalclipstart = portal->start;
  portalclipend = portal->end;

  // Offset the portal view by the linedef centers

  // looking glass center
  start_c.x = (start->v1->x + start->v2->x) / 2;
  start_c.y = (start->v1->y + start->v2->y) / 2;

  // other side center
  dest_c.x = (dest->v1->x + dest->v2->x) / 2;
  dest_c.y = (dest->v1->y + dest->v2->y) / 2;

  // Heights!
  viewz += dest->frontsector->floorheight - start->frontsector->floorheight;

  // calculate the difference in position and rotation!
#ifdef ANGLED_PORTALS
  if (dangle == 0)
#endif
  { // the entrance goes straight opposite the exit, so we just need to mess
    // with the offset.
    viewx += dest_c.x - start_c.x;
    viewy += dest_c.y - start_c.y;
    return;
  }

#ifdef ANGLED_PORTALS
  viewangle += dangle;
  // newview->sin = FINESINE(newview->angle>>ANGLETOFINESHIFT);
  // newview->cos = FINECOSINE(newview->angle>>ANGLETOFINESHIFT);
  // CONS_Printf("dangle == %u\n", AngleFixed(dangle)>>FRACBITS);

  // ????
  {
    fixed_t disttopoint;
    angle_t angtopoint;

    disttopoint = R_PointToDist2(start_c.x, start_c.y, newview->x, newview->y);
    angtopoint = R_PointToAngle2(start_c.x, start_c.y, newview->x, newview->y);
    angtopoint += dangle;

    viewx = dest_c.x +
            FixedMul(FINECOSINE(angtopoint >> ANGLETOFINESHIFT), disttopoint);
    viewy = dest_c.y +
            FixedMul(FINESINE(angtopoint >> ANGLETOFINESHIFT), disttopoint);
  }
#endif
}

void R_AddPortal(INT32 line1, INT32 line2, INT32 x1, INT32 x2) {
  portal_pair *portal = Z_Malloc(sizeof(portal_pair), PU_LEVEL, NULL);
  INT16 *ceilingclipsave = Z_Malloc(sizeof(INT16) * (x2 - x1), PU_LEVEL, NULL);
  INT16 *floorclipsave = Z_Malloc(sizeof(INT16) * (x2 - x1), PU_LEVEL, NULL);
  fixed_t *frontscalesave =
      Z_Malloc(sizeof(fixed_t) * (x2 - x1), PU_LEVEL, NULL);

  portal->line1 = line1;
  portal->line2 = line2;
  portal->pass = portalrender + 1;
  portal->next = NULL;

  R_PortalStoreClipValues(x1, x2, ceilingclipsave, floorclipsave,
                          frontscalesave);

  portal->ceilingclip = ceilingclipsave;
  portal->floorclip = floorclipsave;
  portal->frontscale = frontscalesave;

  portal->start = x1;
  portal->end = x2;

  portalline = true; // this tells R_StoreWallRange that curline is a portal seg

  portal->viewx = viewx;
  portal->viewy = viewy;
  portal->viewz = viewz;
  portal->viewangle = viewangle;

  if (!portal_base) {
    portal_base = portal;
    portal_cap = portal;
  } else {
    portal_cap->next = portal;
    portal_cap = portal;
  }
}

// ================
// R_RenderView
// ================

//                     FAB NOTE FOR WIN32 PORT !! I'm not finished already,
// but I suspect network may have problems with the video buffer being locked
// for all duration of rendering, and being released only once at the end..
// I mean, there is a win16lock() or something that lasts all the rendering,
// so maybe we should release screen lock before each netupdate below..?

void R_RenderPlayerView(player_t *player) {
  portal_pair *portal;
  const boolean skybox = (skyboxmo[0] && cv_skybox.value);
  UINT8 i;

  // if this is display player 1
  if (cv_homremoval.value && player == &players[displayplayers[0]]) {
    if (cv_homremoval.value == 1)
      V_DrawFill(0, 0, BASEVIDWIDTH, BASEVIDHEIGHT, 31); // No HOM effect!
    else //'development' HOM removal -- makes it blindingly obvious if HOM is
         // spotted.
      V_DrawFill(0, 0, BASEVIDWIDTH, BASEVIDHEIGHT, 128 + (timeinmap & 15));
  }
  // Draw over the fourth screen so you don't have to stare at a HOM :V
  else if (splitscreen == 2 && player == &players[displayplayers[2]])
#if 1
  {
    // V_DrawPatchFill, but for the fourth screen only
    patch_t *pat = W_CachePatchName("SRB2BACK", PU_CACHE);
    INT32 dupz = (vid.dupx < vid.dupy ? vid.dupx : vid.dupy);
    INT32 x, y, pw = SHORT(pat->width) * dupz, ph = SHORT(pat->height) * dupz;

    for (x = vid.width >> 1; x < vid.width; x += pw) {
      for (y = vid.height >> 1; y < vid.height; y += ph)
        V_DrawScaledPatch(x, y, V_NOSCALESTART, pat);
    }
  }
#else
    V_DrawFill(viewwidth, viewheight, viewwidth, viewheight,
               31 | V_NOSCALESTART);
#endif

  // load previous saved value of skyVisible for the player
  for (i = 0; i <= splitscreen; i++) {
    if (player == &players[displayplayers[i]]) {
      skyVisible = skyVisiblePerPlayer[i];
      break;
    }
  }

  portalrender = 0;
  portal_base = portal_cap = NULL;

  if (skybox && skyVisible) {
    R_SkyboxFrame(player);

    R_ClearClipSegs();
    R_ClearDrawSegs();
    R_ClearPlanes();
    R_ClearSprites();
#ifdef FLOORSPLATS
    R_ClearVisibleFloorSplats();
#endif

    R_RenderBSPNode((INT32)numnodes - 1);
    R_ClipSprites();
    R_DrawPlanes();
#ifdef FLOORSPLATS
    R_DrawVisibleFloorSplats();
#endif
    R_DrawMasked();
  }

  R_SetupFrame(player, skybox);
  skyVisible = false;
  framecount++;
  validcount++;

  // Clear buffers.
  R_ClearClipSegs();
  R_ClearDrawSegs();
  R_ClearPlanes();
  R_ClearSprites();
#ifdef FLOORSPLATS
  R_ClearVisibleFloorSplats();
#endif

  // check for new console commands.
  NetUpdate();

  // The head node is the last node output.

// profile stuff ---------------------------------------------------------
#ifdef TIMING
  mytotal = 0;
  ProfZeroTimer();
#endif
  R_RenderBSPNode((INT32)numnodes - 1);
  R_ClipSprites();
#ifdef TIMING
  RDMSR(0x10, &mycount);
  mytotal += mycount; // 64bit add

  CONS_Debug(DBG_RENDER, "RenderBSPNode: 0x%d %d\n", *((INT32 *)&mytotal + 1),
             (INT32)mytotal);
#endif
  // profile stuff ---------------------------------------------------------

  // PORTAL RENDERING
  for (portal = portal_base; portal; portal = portal_base) {
    // render the portal
    CONS_Debug(DBG_RENDER, "Rendering portal from line %d to %d\n",
               portal->line1, portal->line2);
    portalrender = portal->pass;

    R_PortalFrame(&lines[portal->line1], &lines[portal->line2], portal);

    R_PortalClearClipSegs(portal->start, portal->end);

    R_PortalRestoreClipValues(portal->start, portal->end, portal->ceilingclip,
                              portal->floorclip, portal->frontscale);

    validcount++;

    R_RenderBSPNode((INT32)numnodes - 1);
    R_ClipSprites();
    // R_DrawPlanes();
    // R_DrawMasked();

    // okay done. free it.
    portalcullsector = NULL; // Just in case...
    portal_base = portal->next;
    Z_Free(portal->ceilingclip);
    Z_Free(portal->floorclip);
    Z_Free(portal->frontscale);
    Z_Free(portal);
  }
  // END PORTAL RENDERING

  R_DrawPlanes();
#ifdef FLOORSPLATS
  R_DrawVisibleFloorSplats();
#endif
  // draw mid texture and sprite
  // And now 3D floors/sides!
  R_DrawMasked();

  // Check for new console commands.
  NetUpdate();

  // save value to skyVisiblePerPlayer
  // this is so that P1 can't affect whether P2 can see a skybox or not, or vice
  // versa
  for (i = 0; i <= splitscreen; i++) {
    if (player == &players[displayplayers[i]]) {
      skyVisiblePerPlayer[i] = skyVisible;
      break;
    }
  }
}

// =========================================================================
//                    ENGINE COMMANDS & VARS
// =========================================================================

void R_RegisterEngineStuff(void) {
  CV_RegisterVar(&cv_gravity);
  CV_RegisterVar(&cv_tailspickup);
  CV_RegisterVar(&cv_soniccd);
  CV_RegisterVar(&cv_allowmlook);
  CV_RegisterVar(&cv_homremoval);
  CV_RegisterVar(&cv_flipcam);
  CV_RegisterVar(&cv_flipcam2);
  CV_RegisterVar(&cv_flipcam3);
  CV_RegisterVar(&cv_flipcam4);

  // Enough for dedicated server
  if (dedicated)
    return;

  // CV_RegisterVar(&cv_precipdensity);
  CV_RegisterVar(&cv_translucency);
  CV_RegisterVar(&cv_drawdist);
  // CV_RegisterVar(&cv_drawdist_nights);
  CV_RegisterVar(&cv_drawdist_precip);
  CV_RegisterVar(&cv_fov);

  CV_RegisterVar(&cv_chasecam);
  CV_RegisterVar(&cv_chasecam2);
  CV_RegisterVar(&cv_chasecam3);
  CV_RegisterVar(&cv_chasecam4);
  CV_RegisterVar(&cv_shadow);
  CV_RegisterVar(&cv_shadowoffs);
  CV_RegisterVar(&cv_skybox);

  CV_RegisterVar(&cv_cam_dist);
  CV_RegisterVar(&cv_cam_still);
  CV_RegisterVar(&cv_cam_height);
  CV_RegisterVar(&cv_cam_speed);
  CV_RegisterVar(&cv_cam_rotate);
  CV_RegisterVar(&cv_cam_rotspeed);

  CV_RegisterVar(&cv_cam2_dist);
  CV_RegisterVar(&cv_cam2_still);
  CV_RegisterVar(&cv_cam2_height);
  CV_RegisterVar(&cv_cam2_speed);
  CV_RegisterVar(&cv_cam2_rotate);
  CV_RegisterVar(&cv_cam2_rotspeed);

  CV_RegisterVar(&cv_cam3_dist);
  CV_RegisterVar(&cv_cam3_still);
  CV_RegisterVar(&cv_cam3_height);
  CV_RegisterVar(&cv_cam3_speed);
  CV_RegisterVar(&cv_cam3_rotate);
  CV_RegisterVar(&cv_cam3_rotspeed);

  CV_RegisterVar(&cv_cam4_dist);
  CV_RegisterVar(&cv_cam4_still);
  CV_RegisterVar(&cv_cam4_height);
  CV_RegisterVar(&cv_cam4_speed);
  CV_RegisterVar(&cv_cam4_rotate);
  CV_RegisterVar(&cv_cam4_rotspeed);

  CV_RegisterVar(&cv_showhud);
  CV_RegisterVar(&cv_translucenthud);

  CV_RegisterVar(&cv_maxportals);

  // Default viewheight is changeable,
  // initialized to standard viewheight
  // CV_RegisterVar(&cv_viewheight);

  // GL-specific Commands
  CV_RegisterVar(&cv_grgammablue);
  CV_RegisterVar(&cv_grgammagreen);
  CV_RegisterVar(&cv_grgammared);
  CV_RegisterVar(&cv_grfovchange);
#ifdef ALAM_LIGHTING
  CV_RegisterVar(&cv_grstaticlighting);
  CV_RegisterVar(&cv_grdynamiclighting);
  CV_RegisterVar(&cv_grcoronas);
  CV_RegisterVar(&cv_grcoronasize);
#endif
  CV_RegisterVar(&cv_grmdls);
  CV_RegisterVar(&cv_grfallbackplayermodel);
  CV_RegisterVar(&cv_grspritebillboarding);
  CV_RegisterVar(&cv_grfakecontrast);
  CV_RegisterVar(&cv_grshearing);
  CV_RegisterVar(&cv_grshaders);

  if (rendermode != render_soft && rendermode != render_none)
    HWR_AddCommands();

  // Frame interpolation/uncapped
  CV_RegisterVar(&cv_fpscap);
}

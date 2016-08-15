// SONIC ROBO BLAST 2 KART ~ ZarroTsu
//-----------------------------------------------------------------------------
/// \file  k_kart.h
/// \brief SRB2kart stuff.

#ifndef __K_KART__
#define __K_KART__

#include "doomdef.h"
#include "d_player.h" // Need for player_t

extern const char *KartColor_Names[MAXSKINCOLORS];
void K_GenerateKartColormap(UINT8 *dest_colormap, INT32 skinnum, UINT8 color);
UINT8 K_GetKartColorByName(const char *name);

void K_RegisterKartStuff(void);

void K_KartPlayerThink(player_t *player, ticcmd_t *cmd);
boolean P_SpinPlayerMobj(mobj_t *target, mobj_t *source);
boolean P_SquishPlayerMobj(mobj_t *target, mobj_t *source);
boolean P_ExplodePlayerMobj(mobj_t *target, mobj_t *source);

void K_LoadKartHUDGraphics(void);
void K_drawKartHUD(void);

// =========================================================================
#endif  // __K_KART__

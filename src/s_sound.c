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
/// \file  s_sound.c
/// \brief System-independent sound and music routines

#ifdef MUSSERV
#include <sys/msg.h>
struct musmsg {
  long msg_type;
  char msg_text[12];
};
extern INT32 msg_id;
#endif

#include "command.h"
#include "d_main.h"
#include "doomdef.h"
#include "doomstat.h"
#include "g_game.h"
#include "i_sound.h"
#include "i_system.h"
#include "m_argv.h"
#include "m_misc.h"   // for tunes command
#include "p_local.h"  // camera info
#include "r_main.h"   // R_PointToAngle2() used to calc stereo sep.
#include "r_sky.h"    // skyflatnum
#include "r_things.h" // for skins
#include "s_sound.h"
#include "w_wad.h"
#include "z_zone.h"

#include "lua_hook.h" // MusicChange hook

#ifdef HW3SOUND
// 3D Sound Interface
#include "hardware/hw3sound.h"
#else
static boolean S_AdjustSoundParams(const mobj_t *listener, const mobj_t *source,
                                   INT32 *vol, INT32 *sep, INT32 *pitch,
                                   sfxinfo_t *sfxinfo);
#endif

CV_PossibleValue_t soundvolume_cons_t[] = {{0, "MIN"}, {31, "MAX"}, {0, NULL}};
static void SetChannelsNum(void);
static void Command_Tunes_f(void);
static void Command_RestartAudio_f(void);

// Sound system toggles
#ifndef NO_MIDI
static void GameMIDIMusic_OnChange(void);
#endif
static void GameSounds_OnChange(void);
static void GameDigiMusic_OnChange(void);

static void PlayMusicIfUnfocused_OnChange(void);
static void PlaySoundIfUnfocused_OnChange(void);

// commands for music and sound servers
#ifdef MUSSERV
consvar_t musserver_cmd = {"musserver_cmd",
                           "musserver",
                           CV_SAVE,
                           NULL,
                           NULL,
                           0,
                           NULL,
                           NULL,
                           0,
                           0,
                           NULL};
consvar_t musserver_arg = {"musserver_arg",
                           "-t 20 -f -u 0 -i music.dta",
                           CV_SAVE,
                           NULL,
                           NULL,
                           0,
                           NULL,
                           NULL,
                           0,
                           0,
                           NULL};
#endif
#ifdef SNDSERV
consvar_t sndserver_cmd = {
    "sndserver_cmd", "llsndserv", CV_SAVE, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t sndserver_arg = {
    "sndserver_arg", "-quiet", CV_SAVE, NULL, 0, NULL, NULL, 0, 0, NULL};
#endif

#if defined(_WIN32_WCE) || defined(DC) || defined(GP2X)
consvar_t cv_samplerate = {
    "samplerate", "11025", 0, CV_Unsigned, NULL, 11025,
    NULL,         NULL,    0, 0,           NULL}; // Alam: For easy hacking?
#else
consvar_t cv_samplerate = {
    "samplerate", "22050", 0, CV_Unsigned, NULL, 22050,
    NULL,         NULL,    0, 0,           NULL}; // Alam: For easy hacking?
#endif

// stereo reverse
consvar_t stereoreverse = {
    "stereoreverse", "Off", CV_SAVE, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL};

// if true, all sounds are loaded at game startup
static consvar_t precachesound = {
    "precachesound", "Off", CV_SAVE, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL};

// actual general (maximum) sound & music volume, saved into the config
consvar_t cv_soundvolume = {
    "soundvolume", "18", CV_SAVE, soundvolume_cons_t, NULL, 0, NULL, NULL, 0, 0,
    NULL};
consvar_t cv_digmusicvolume = {"digmusicvolume",
                               "18",
                               CV_SAVE,
                               soundvolume_cons_t,
                               NULL,
                               0,
                               NULL,
                               NULL,
                               0,
                               0,
                               NULL};
#ifndef NO_MIDI
consvar_t cv_midimusicvolume = {"midimusicvolume",
                                "18",
                                CV_SAVE,
                                soundvolume_cons_t,
                                NULL,
                                0,
                                NULL,
                                NULL,
                                0,
                                0,
                                NULL};
#endif
// number of channels available
#if defined(_WIN32_WCE) || defined(DC) || defined(PSP) || defined(GP2X)
consvar_t cv_numChannels = {"snd_channels",
                            "8",
                            CV_SAVE | CV_CALL,
                            CV_Unsigned,
                            SetChannelsNum,
                            0,
                            NULL,
                            NULL,
                            0,
                            0,
                            NULL};
#else
consvar_t cv_numChannels = {"snd_channels",
                            "64",
                            CV_SAVE | CV_CALL,
                            CV_Unsigned,
                            SetChannelsNum,
                            0,
                            NULL,
                            NULL,
                            0,
                            0,
                            NULL};
#endif

consvar_t surround = {"surround", "Off", CV_SAVE, CV_OnOff, NULL, 0,
                      NULL,       NULL,  0,       0,        NULL};
// consvar_t cv_resetmusic = {"resetmusic", "No", CV_SAVE|CV_NOSHOWHELP,
// CV_YesNo, NULL, 0, NULL, NULL, 0, 0, NULL};

// Sound system toggles, saved into the config
consvar_t cv_gamedigimusic = {"digimusic",
                              "On",
                              CV_SAVE | CV_CALL | CV_NOINIT,
                              CV_OnOff,
                              GameDigiMusic_OnChange,
                              0,
                              NULL,
                              NULL,
                              0,
                              0,
                              NULL};
#ifndef NO_MIDI
consvar_t cv_gamemidimusic = {"midimusic",
                              "On",
                              CV_SAVE | CV_CALL | CV_NOINIT,
                              CV_OnOff,
                              GameMIDIMusic_OnChange,
                              0,
                              NULL,
                              NULL,
                              0,
                              0,
                              NULL};
#endif
consvar_t cv_gamesounds = {"sounds",
                           "On",
                           CV_SAVE | CV_CALL | CV_NOINIT,
                           CV_OnOff,
                           GameSounds_OnChange,
                           0,
                           NULL,
                           NULL,
                           0,
                           0,
                           NULL};

consvar_t cv_playmusicifunfocused = {"playmusicifunfocused",
                                     "No",
                                     CV_SAVE | CV_CALL | CV_NOINIT,
                                     CV_YesNo,
                                     PlayMusicIfUnfocused_OnChange,
                                     0,
                                     NULL,
                                     NULL,
                                     0,
                                     0,
                                     NULL};
consvar_t cv_playsoundifunfocused = {"playsoundsifunfocused",
                                     "No",
                                     CV_SAVE | CV_CALL | CV_NOINIT,
                                     CV_YesNo,
                                     PlaySoundIfUnfocused_OnChange,
                                     0,
                                     NULL,
                                     NULL,
                                     0,
                                     0,
                                     NULL};

#define S_MAX_VOLUME 127

// when to clip out sounds
// Does not fit the large outdoor areas.
// added 2-2-98 in 8 bit volume control (before (1200*0x10000))
#define S_CLIPPING_DIST (1536 * 0x10000)

// Distance to origin when sounds should be maxed out.
// This should relate to movement clipping resolution
// (see BLOCKMAP handling).
// Originally: (200*0x10000).
// added 2-2-98 in 8 bit volume control (before (160*0x10000))
#define S_CLOSE_DIST (160 * 0x10000)

// added 2-2-98 in 8 bit volume control (before remove the +4)
#define S_ATTENUATOR ((S_CLIPPING_DIST - S_CLOSE_DIST) >> (FRACBITS + 4))

// Adjustable by menu.
#define NORM_VOLUME snd_MaxVolume

#define NORM_PITCH 128
#define NORM_PRIORITY 64
#define NORM_SEP 128

#define S_PITCH_PERTURB 1
#define S_STEREO_SWING (96 * 0x10000)

#ifdef SURROUND
#define SURROUND_SEP -128
#endif

// percent attenuation from front to back
#define S_IFRACVOL 30

typedef struct {
  // sound information (if null, channel avail.)
  sfxinfo_t *sfxinfo;

  // origin of sound
  const void *origin;

  // initial volume of sound, which is applied after distance and direction
  INT32 volume;

  // handle of the sound being played
  INT32 handle;

} channel_t;

// the set of channels available
static channel_t *channels = NULL;
static INT32 numofchannels = 0;

//
// Internals.
//
static void S_StopChannel(INT32 cnum);

//
// S_getChannel
//
// If none available, return -1. Otherwise channel #.
//
static INT32 S_getChannel(const void *origin, sfxinfo_t *sfxinfo) {
  // channel number to use
  INT32 cnum;

  // Find an open channel
  for (cnum = 0; cnum < numofchannels; cnum++) {
    if (!channels[cnum].sfxinfo)
      break;

    // Now checks if same sound is being played, rather
    // than just one sound per mobj
    else if (sfxinfo == channels[cnum].sfxinfo &&
             (sfxinfo->pitch & SF_NOMULTIPLESOUND)) {
      return -1;
    } else if (sfxinfo == channels[cnum].sfxinfo &&
               sfxinfo->singularity == true) {
      S_StopChannel(cnum);
      break;
    } else if (origin && channels[cnum].origin == origin &&
               channels[cnum].sfxinfo == sfxinfo) {
      if (sfxinfo->pitch & SF_NOINTERRUPT)
        return -1;
      else
        S_StopChannel(cnum);
      break;
    } else if (origin && channels[cnum].origin == origin &&
               channels[cnum].sfxinfo->name != sfxinfo->name &&
               (channels[cnum].sfxinfo->pitch & SF_TOTALLYSINGLE) &&
               (sfxinfo->pitch & SF_TOTALLYSINGLE)) {
      S_StopChannel(cnum);
      break;
    }
  }

  // None available
  if (cnum == numofchannels) {
    // Look for lower priority
    for (cnum = 0; cnum < numofchannels; cnum++)
      if (channels[cnum].sfxinfo->priority <= sfxinfo->priority)
        break;

    if (cnum == numofchannels) {
      // No lower priority. Sorry, Charlie.
      return -1;
    } else {
      // Otherwise, kick out lower priority.
      S_StopChannel(cnum);
    }
  }

  return cnum;
}

void S_RegisterSoundStuff(void) {
  if (dedicated) {
    sound_disabled = true;
    return;
  }

  CV_RegisterVar(&stereoreverse);
  CV_RegisterVar(&precachesound);

#ifdef SNDSERV
  CV_RegisterVar(&sndserver_cmd);
  CV_RegisterVar(&sndserver_arg);
#endif
#ifdef MUSSERV
  CV_RegisterVar(&musserver_cmd);
  CV_RegisterVar(&musserver_arg);
#endif
  CV_RegisterVar(&surround);
  CV_RegisterVar(&cv_samplerate);
  // CV_RegisterVar(&cv_resetmusic);
  CV_RegisterVar(&cv_gamesounds);
  CV_RegisterVar(&cv_gamedigimusic);
#ifndef NO_MIDI
  CV_RegisterVar(&cv_gamemidimusic);
#endif

  CV_RegisterVar(&cv_playmusicifunfocused);
  CV_RegisterVar(&cv_playsoundifunfocused);

  COM_AddCommand("tunes", Command_Tunes_f);
  COM_AddCommand("restartaudio", Command_RestartAudio_f);
}

static void SetChannelsNum(void) {
  INT32 i;

  // Allocating the internal channels for mixing
  // (the maximum number of sounds rendered
  // simultaneously) within zone memory.
  if (channels)
    S_StopSounds();

  Z_Free(channels);
  channels = NULL;

  if (cv_numChannels.value ==
      999999999) // Alam_GBC: OH MY ROD!(ROD rimmiced with GOD!)
    CV_StealthSet(&cv_numChannels, cv_numChannels.defaultvalue);

#ifdef HW3SOUND
  if (hws_mode != HWS_DEFAULT_MODE) {
    HW3S_SetSourcesNum();
    return;
  }
#endif
  if (cv_numChannels.value)
    channels = (channel_t *)Z_Malloc(cv_numChannels.value * sizeof(channel_t),
                                     PU_STATIC, NULL);
  numofchannels = cv_numChannels.value;

  // Free all channels for use
  for (i = 0; i < numofchannels; i++)
    channels[i].sfxinfo = 0;
}

// Retrieve the lump number of sfx
//
lumpnum_t S_GetSfxLumpNum(sfxinfo_t *sfx) {
  char namebuf[9];
  lumpnum_t sfxlump;

  sprintf(namebuf, "ds%s", sfx->name);

  sfxlump = W_CheckNumForName(namebuf);
  if (sfxlump != LUMPERROR)
    return sfxlump;

  strlcpy(namebuf, sfx->name, sizeof namebuf);

  sfxlump = W_CheckNumForName(namebuf);
  if (sfxlump != LUMPERROR)
    return sfxlump;

  return W_GetNumForName("dsthok");
}

// Stop all sounds, load level info, THEN start sounds.
void S_StopSounds(void) {
  INT32 cnum;

#ifdef HW3SOUND
  if (hws_mode != HWS_DEFAULT_MODE) {
    HW3S_StopSounds();
    return;
  }
#endif

  // kill all playing sounds at start of level
  for (cnum = 0; cnum < numofchannels; cnum++)
    if (channels[cnum].sfxinfo)
      S_StopChannel(cnum);
}

void S_StopSoundByID(void *origin, sfxenum_t sfx_id) {
  INT32 cnum;

  // Sounds without origin can have multiple sources, they shouldn't
  // be stopped by new sounds.
  if (!origin)
    return;
#ifdef HW3SOUND
  if (hws_mode != HWS_DEFAULT_MODE) {
    HW3S_StopSoundByID(origin, sfx_id);
    return;
  }
#endif
  for (cnum = 0; cnum < numofchannels; cnum++) {
    if (channels[cnum].sfxinfo == &S_sfx[sfx_id] &&
        channels[cnum].origin == origin) {
      S_StopChannel(cnum);
    }
  }
}

void S_StopSoundByNum(sfxenum_t sfxnum) {
  INT32 cnum;

#ifdef HW3SOUND
  if (hws_mode != HWS_DEFAULT_MODE) {
    HW3S_StopSoundByNum(sfxnum);
    return;
  }
#endif
  for (cnum = 0; cnum < numofchannels; cnum++) {
    if (channels[cnum].sfxinfo == &S_sfx[sfxnum]) {
      S_StopChannel(cnum);
    }
  }
}

static INT32 S_ScaleVolumeWithSplitscreen(INT32 volume) {
  fixed_t root = INT32_MAX;

  if (splitscreen == 0) {
    return volume;
  }

  root = FixedSqrt((splitscreen + 1) * (FRACUNIT / 3));

  return FixedDiv(volume * FRACUNIT, root) / FRACUNIT;
}

void S_StartSoundAtVolume(const void *origin_p, sfxenum_t sfx_id,
                          INT32 volume) {
  const mobj_t *origin = (const mobj_t *)origin_p;
  const boolean reverse = (stereoreverse.value ^ encoremode);
  const INT32 initial_volume =
      (origin ? S_ScaleVolumeWithSplitscreen(volume) : volume);

  sfxinfo_t *sfx;
  INT32 sep, pitch, priority, cnum;
  boolean anyListeners = false;
  boolean itsUs = false;
  INT32 i;

  listener_t listener[MAXSPLITSCREENPLAYERS];
  mobj_t *listenmobj[MAXSPLITSCREENPLAYERS];

  if (sound_disabled || !sound_started)
    return;

  // Don't want a sound? Okay then...
  if (sfx_id == sfx_None)
    return;

  for (i = 0; i <= splitscreen; i++) {
    player_t *player = &players[displayplayers[i]];

    memset(&listener[i], 0, sizeof(listener[i]));
    listenmobj[i] = NULL;

    if (i == 0 && democam.soundmobj) {
      listenmobj[i] = democam.soundmobj;
    } else if (player->awayviewtics) {
      listenmobj[i] = player->awayviewmobj;
    } else {
      listenmobj[i] = player->mo;
    }

    if (origin && origin == listenmobj[i]) {
      itsUs = true;
    }
  }

#ifdef HW3SOUND
  if (hws_mode != HWS_DEFAULT_MODE) {
    HW3S_StartSound(origin, sfx_id);
    return;
  };
#endif

  for (i = 0; i <= splitscreen; i++) {
    player_t *player = &players[displayplayers[i]];

    if (camera[i].chase && !player->awayviewtics) {
      listener[i].x = camera[i].x;
      listener[i].y = camera[i].y;
      listener[i].z = camera[i].z;
      listener[i].angle = camera[i].angle;
      anyListeners = true;
    } else if (listenmobj[i]) {
      listener[i].x = listenmobj[i]->x;
      listener[i].y = listenmobj[i]->y;
      listener[i].z = listenmobj[i]->z;
      listener[i].angle = listenmobj[i]->angle;
      anyListeners = true;
    }
  }

  if (origin && anyListeners == false) {
    // If a mobj is trying to make a noise, and no one is around to hear it,
    // does it make a sound?
    return;
  }

  // check for bogus sound #
  I_Assert(sfx_id >= 1);
  I_Assert(sfx_id < NUMSFX);

  sfx = &S_sfx[sfx_id];

  if (sfx->skinsound != -1 && origin && origin->skin) {
    // redirect player sound to the sound in the skin table
    sfx_id = ((skin_t *)origin->skin)->soundsid[sfx->skinsound];
    sfx = &S_sfx[sfx_id];
  }

  // Initialize sound parameters
  pitch = NORM_PITCH;
  priority = NORM_PRIORITY;
  sep = NORM_SEP;

  i = 0; // sensible default

  {
    // Check to see if it is audible, and if not, modify the params
    if (origin && !itsUs) {
      boolean audible = false;
      if (splitscreen > 0) {
        fixed_t recdist = INT32_MAX;
        UINT8 j = 0;

        for (; j <= splitscreen; j++) {
          fixed_t thisdist = INT32_MAX;

          if (!listenmobj[j]) {
            continue;
          }

          thisdist = P_AproxDistance(listener[j].x - origin->x,
                                     listener[j].y - origin->y);

          if (thisdist >= recdist) {
            continue;
          }

          recdist = thisdist;
          i = j;
        }
      }

      if (listenmobj[i]) {
        audible = S_AdjustSoundParams(listenmobj[i], origin, &volume, &sep,
                                      &pitch, sfx);
      }

      if (!audible) {
        return;
      }
    }

    // This is supposed to handle the loading/caching.
    // For some odd reason, the caching is done nearly
    // each time the sound is needed?

    // cache data if necessary
    // NOTE: set sfx->data NULL sfx->lump -1 to force a reload
    if (!sfx->data) {
      sfx->data = I_GetSfx(sfx);
    }

    // increase the usefulness
    if (sfx->usefulness++ < 0) {
      sfx->usefulness = -1;
    }

    // Avoid channel reverse if surround
    if (reverse
#ifdef SURROUND
        && sep != SURROUND_SEP
#endif
    ) {
      sep = (~sep) & 255;
    }

    // At this point it is determined that a sound can and should be played, so
    // find a free channel to play it on
    cnum = S_getChannel(origin, sfx);

    if (cnum < 0) {
      return; // If there's no free channels, there won't be any for anymore
              // players either
    }

    // Now that we know we are going to play a sound, fill out this info
    channels[cnum].sfxinfo = sfx;
    channels[cnum].origin = origin;
    channels[cnum].volume = initial_volume;
    channels[cnum].handle =
        I_StartSound(sfx_id, volume, sep, pitch, priority, cnum);
  }
}

void S_StartSound(const void *origin, sfxenum_t sfx_id) {
  if (sound_disabled)
    return;

  if (mariomode) // Sounds change in Mario mode!
  {
    switch (sfx_id) {
      //			case sfx_altow1:
      //			case sfx_altow2:
      //			case sfx_altow3:
      //			case sfx_altow4:
      //				sfx_id = sfx_mario8;
      //				break;
    case sfx_thok:
      sfx_id = sfx_mario7;
      break;
    case sfx_pop:
      sfx_id = sfx_mario5;
      break;
    case sfx_jump:
      sfx_id = sfx_mario6;
      break;
    case sfx_shield:
      sfx_id = sfx_mario3;
      break;
    case sfx_itemup:
      sfx_id = sfx_mario4;
      break;
      //			case sfx_tink:
      //				sfx_id = sfx_mario1;
      //				break;
      //			case sfx_cgot:
      //				sfx_id = sfx_mario9;
      //				break;
      //			case sfx_lose:
      //				sfx_id = sfx_mario2;
      //				break;
    default:
      break;
    }
  }
  if (maptol & TOL_XMAS) // Some sounds change for xmas
  {
    switch (sfx_id) {
    case sfx_ideya:
    case sfx_nbmper:
    case sfx_ncitem:
    case sfx_ngdone:
      ++sfx_id;
    default:
      break;
    }
  }

  // the volume is handled 8 bits
#ifdef HW3SOUND
  if (hws_mode != HWS_DEFAULT_MODE)
    HW3S_StartSound(origin, sfx_id);
  else
#endif
    S_StartSoundAtVolume(origin, sfx_id, 255);
}

void S_StopSound(void *origin) {
  INT32 cnum;

  // Sounds without origin can have multiple sources, they shouldn't
  // be stopped by new sounds.
  if (!origin)
    return;

#ifdef HW3SOUND
  if (hws_mode != HWS_DEFAULT_MODE) {
    HW3S_StopSound(origin);
    return;
  }
#endif
  for (cnum = 0; cnum < numofchannels; cnum++) {
    if (channels[cnum].sfxinfo && channels[cnum].origin == origin) {
      S_StopChannel(cnum);
    }
  }
}

//
// Updates music & sounds
//
static INT32 actualsfxvolume; // check for change through console
static INT32 actualdigmusicvolume;
#ifndef NO_MIDI
static INT32 actualmidimusicvolume;
#endif

void S_UpdateSounds(void) {
  INT32 cnum, volume, sep, pitch;
  boolean audible = false;
  channel_t *c;
  INT32 i;

  listener_t listener[MAXSPLITSCREENPLAYERS];
  mobj_t *listenmobj[MAXSPLITSCREENPLAYERS];

  // Update sound/music volumes, if changed manually at console
  if (actualsfxvolume != cv_soundvolume.value)
    S_SetSfxVolume(cv_soundvolume.value);
  if (actualdigmusicvolume != cv_digmusicvolume.value)
    S_SetDigMusicVolume(cv_digmusicvolume.value);
#ifndef NO_MIDI
  if (actualmidimusicvolume != cv_midimusicvolume.value)
    S_SetMIDIMusicVolume(cv_midimusicvolume.value);
#endif

  // We're done now, if we're not in a level.
  if (gamestate != GS_LEVEL) {
#ifndef NOMUMBLE
    // Stop Mumble cutting out. I'm sick of it.
    I_UpdateMumble(NULL, listener[0]);
#endif

    // Stop cutting FMOD out. WE'RE sick of it.
    I_UpdateSound();
    return;
  }

  if (dedicated || sound_disabled)
    return;

  for (i = 0; i <= splitscreen; i++) {
    player_t *player = &players[displayplayers[i]];

    memset(&listener[i], 0, sizeof(listener[i]));
    listenmobj[i] = NULL;

    if (i == 0 && democam.soundmobj) {
      listenmobj[i] = democam.soundmobj;
      continue;
    }

    if (player->awayviewtics) {
      listenmobj[i] = player->awayviewmobj;
    } else {
      listenmobj[i] = player->mo;
    }
  }

#ifndef NOMUMBLE
  I_UpdateMumble(players[consoleplayer].mo, listener[0]);
#endif

#ifdef HW3SOUND
  if (hws_mode != HWS_DEFAULT_MODE) {
    HW3S_UpdateSources();
    I_UpdateSound();
    return;
  }
#endif

  for (i = 0; i <= splitscreen; i++) {
    player_t *player = &players[displayplayers[i]];

    if (camera[i].chase && !player->awayviewtics) {
      listener[i].x = camera[i].x;
      listener[i].y = camera[i].y;
      listener[i].z = camera[i].z;
      listener[i].angle = camera[i].angle;
    } else if (listenmobj[i]) {
      listener[i].x = listenmobj[i]->x;
      listener[i].y = listenmobj[i]->y;
      listener[i].z = listenmobj[i]->z;
      listener[i].angle = listenmobj[i]->angle;
    }
  }

  for (cnum = 0; cnum < numofchannels; cnum++) {
    c = &channels[cnum];

    if (c->sfxinfo) {
      if (I_SoundIsPlaying(c->handle)) {
        // initialize parameters
        volume = c->volume; // 8 bits internal volume precision
        pitch = NORM_PITCH;
        sep = NORM_SEP;

        // check non-local sounds for distance clipping
        //  or modify their params
        if (c->origin) {
          boolean itsUs = false;

          for (i = splitscreen; i >= 0; i--) {
            if (c->origin != listenmobj[i])
              continue;

            itsUs = true;
          }

          if (itsUs == false) {
            const mobj_t *origin = c->origin;

            i = 0;

            if (splitscreen > 0) {
              fixed_t recdist = INT32_MAX;
              UINT8 j = 0;

              for (; j <= splitscreen; j++) {
                fixed_t thisdist = INT32_MAX;

                if (!listenmobj[j]) {
                  continue;
                }

                thisdist = P_AproxDistance(listener[j].x - origin->x,
                                           listener[j].y - origin->y);

                if (thisdist >= recdist) {
                  continue;
                }

                recdist = thisdist;
                i = j;
              }
            }

            if (listenmobj[i]) {
              audible = S_AdjustSoundParams(listenmobj[i], c->origin, &volume,
                                            &sep, &pitch, c->sfxinfo);
            }

            if (audible)
              I_UpdateSoundParams(c->handle, volume, sep, pitch);
            else
              S_StopChannel(cnum);
          }
        }
      } else {
        // if channel is allocated but sound has stopped, free it
        S_StopChannel(cnum);
      }
    }
  }

  I_UpdateSound();
}

void S_SetSfxVolume(INT32 volume) {
  if (volume < 0 || volume > 31)
    CONS_Alert(CONS_WARNING, "sfxvolume should be between 0-31\n");

  CV_SetValue(&cv_soundvolume, volume & 0x1F);
  actualsfxvolume = cv_soundvolume.value; // check for change of var

#ifdef HW3SOUND
  hws_mode == HWS_DEFAULT_MODE ? I_SetSfxVolume(volume & 0x1F)
                               : HW3S_SetSfxVolume(volume & 0x1F);
#else
  // now hardware volume
  I_SetSfxVolume(volume & 0x1F);
#endif
}

void S_ClearSfx(void) {
#ifndef DJGPPDOS
  size_t i;
  for (i = 1; i < NUMSFX; i++)
    I_FreeSfx(S_sfx + i);
#endif
}

static void S_StopChannel(INT32 cnum) {
  INT32 i;
  channel_t *c = &channels[cnum];

  if (c->sfxinfo) {
    // stop the sound playing
    if (I_SoundIsPlaying(c->handle))
      I_StopSound(c->handle);

    // check to see
    //  if other channels are playing the sound
    for (i = 0; i < numofchannels; i++)
      if (cnum != i && c->sfxinfo == channels[i].sfxinfo)
        break;

    // degrade usefulness of sound data
    c->sfxinfo->usefulness--;

    c->sfxinfo = 0;
  }
}

//
// S_CalculateSoundDistance
//
// Calculates the distance between two points for a sound.
// Clips the distance to prevent overflow.
//
fixed_t S_CalculateSoundDistance(fixed_t sx1, fixed_t sy1, fixed_t sz1,
                                 fixed_t sx2, fixed_t sy2, fixed_t sz2) {
  fixed_t approx_dist, adx, ady;

  // calculate the distance to sound origin and clip it if necessary
  adx = abs((sx1 >> FRACBITS) - (sx2 >> FRACBITS));
  ady = abs((sy1 >> FRACBITS) - (sy2 >> FRACBITS));

  // From _GG1_ p.428. Approx. euclidian distance fast.
  // Take Z into account
  adx = adx + ady - ((adx < ady ? adx : ady) >> 1);
  ady = abs((sz1 >> FRACBITS) - (sz2 >> FRACBITS));
  approx_dist = adx + ady - ((adx < ady ? adx : ady) >> 1);

  if (approx_dist >= FRACUNIT / 2)
    approx_dist = FRACUNIT / 2 - 1;

  approx_dist <<= FRACBITS;

  return FixedDiv(approx_dist, mapobjectscale); // approx_dist
}

//
// Changes volume, stereo-separation, and pitch variables
// from the norm of a sound effect to be played.
// If the sound is not audible, returns a 0.
// Otherwise, modifies parameters and returns 1.
//
boolean S_AdjustSoundParams(const mobj_t *listener, const mobj_t *source,
                            INT32 *vol, INT32 *sep, INT32 *pitch,
                            sfxinfo_t *sfxinfo) {
  const boolean reverse = (stereoreverse.value ^ encoremode);

  fixed_t approx_dist;

  listener_t listensource;
  INT32 i;

  (void)pitch;

  if (!listener)
    return false;

  // Init listensource with default listener
  listensource.x = listener->x;
  listensource.y = listener->y;
  listensource.z = listener->z;
  listensource.angle = listener->angle;

  for (i = 0; i <= splitscreen; i++) {
    // If listener is a chasecam player, use the camera instead
    if (listener == players[displayplayers[i]].mo && camera[i].chase) {
      listensource.x = camera[i].x;
      listensource.y = camera[i].y;
      listensource.z = camera[i].z;
      listensource.angle = camera[i].angle;
      break;
    }
  }

  if (sfxinfo->pitch & SF_OUTSIDESOUND) // Rain special case
  {
    fixed_t x, y, yl, yh, xl, xh, newdist;

    if (R_PointInSubsector(listensource.x, listensource.y)
            ->sector->ceilingpic == skyflatnum)
      approx_dist = 0;
    else {
      // Essentially check in a 1024 unit radius of the player for an outdoor
      // area.
      yl = listensource.y - 1024 * FRACUNIT;
      yh = listensource.y + 1024 * FRACUNIT;
      xl = listensource.x - 1024 * FRACUNIT;
      xh = listensource.x + 1024 * FRACUNIT;
      approx_dist = 1024 * FRACUNIT;
      for (y = yl; y <= yh; y += FRACUNIT * 64)
        for (x = xl; x <= xh; x += FRACUNIT * 64) {
          if (R_PointInSubsector(x, y)->sector->ceilingpic == skyflatnum) {
            // Found the outdoors!
            newdist = S_CalculateSoundDistance(listensource.x, listensource.y,
                                               0, x, y, 0);
            if (newdist < approx_dist) {
              approx_dist = newdist;
            }
          }
        }
    }
  } else {
    approx_dist =
        S_CalculateSoundDistance(listensource.x, listensource.y, listensource.z,
                                 source->x, source->y, source->z);
  }

  // Ring loss, deaths, etc, should all be heard louder.
  if (sfxinfo->pitch & SF_X8AWAYSOUND)
    approx_dist = FixedDiv(approx_dist, 8 * FRACUNIT);

  // Combine 8XAWAYSOUND with 4XAWAYSOUND and get.... 32XAWAYSOUND?
  if (sfxinfo->pitch & SF_X4AWAYSOUND)
    approx_dist = FixedDiv(approx_dist, 4 * FRACUNIT);

  if (sfxinfo->pitch & SF_X2AWAYSOUND)
    approx_dist = FixedDiv(approx_dist, 2 * FRACUNIT);

  if (approx_dist > S_CLIPPING_DIST)
    return false;

  if (source->x == listensource.x && source->y == listensource.y) {
    *sep = NORM_SEP;
  } else {
    // angle of source to listener
    angle_t angle =
        R_PointToAngle2(listensource.x, listensource.y, source->x, source->y);

    if (angle > listensource.angle)
      angle = angle - listensource.angle;
    else
      angle = angle + InvAngle(listensource.angle);

    if (reverse)
      angle = InvAngle(angle);

#ifdef SURROUND
    // Produce a surround sound for angle from 105 till 255
    if (surround.value == 1 && (angle > ANG105 && angle < ANG255))
      *sep = SURROUND_SEP;
    else
#endif
    {
      angle >>= ANGLETOFINESHIFT;

      // stereo separation
      *sep = 128 - (FixedMul(S_STEREO_SWING, FINESINE(angle)) >> FRACBITS);
    }
  }

  // volume calculation
  /* not sure if it should be > (no =), but this matches the old behavior */
  if (approx_dist >= S_CLOSE_DIST) {
    // distance effect
    INT32 n = (15 * ((S_CLIPPING_DIST - approx_dist) >> FRACBITS));
    *vol = FixedMul(*vol * FRACUNIT / 255, n) / S_ATTENUATOR;
  }

  return (*vol > 0);
}

// Searches through the channels and checks if a sound is playing
// on the given origin.
INT32
S_OriginPlaying(void *origin) {
  INT32 cnum;
  if (!origin)
    return false;

#ifdef HW3SOUND
  if (hws_mode != HWS_DEFAULT_MODE)
    return HW3S_OriginPlaying(origin);
#endif

  for (cnum = 0; cnum < numofchannels; cnum++)
    if (channels[cnum].origin == origin)
      return 1;
  return 0;
}

// Searches through the channels and checks if a given id
// is playing anywhere.
INT32
S_IdPlaying(sfxenum_t id) {
  INT32 cnum;

#ifdef HW3SOUND
  if (hws_mode != HWS_DEFAULT_MODE)
    return HW3S_IdPlaying(id);
#endif

  for (cnum = 0; cnum < numofchannels; cnum++)
    if ((size_t)(channels[cnum].sfxinfo - S_sfx) == (size_t)id)
      return 1;
  return 0;
}

// Searches through the channels and checks for
// origin x playing sound id y.
INT32
S_SoundPlaying(void *origin, sfxenum_t id) {
  INT32 cnum;
  if (!origin)
    return 0;

#ifdef HW3SOUND
  if (hws_mode != HWS_DEFAULT_MODE)
    return HW3S_SoundPlaying(origin, id);
#endif

  for (cnum = 0; cnum < numofchannels; cnum++) {
    if (channels[cnum].origin == origin &&
        (size_t)(channels[cnum].sfxinfo - S_sfx) == (size_t)id)
      return 1;
  }
  return 0;
}

//
// S_StartSoundName
// Starts a sound using the given name.
#define MAXNEWSOUNDS 10
static sfxenum_t newsounds[MAXNEWSOUNDS] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void S_StartSoundName(void *mo, const char *soundname) {
  INT32 i, soundnum = 0;
  // Search existing sounds...
  for (i = sfx_None + 1; i < NUMSFX; i++) {
    if (!S_sfx[i].name)
      continue;
    if (!stricmp(S_sfx[i].name, soundname)) {
      soundnum = i;
      break;
    }
  }

  if (!soundnum) {
    for (i = 0; i < MAXNEWSOUNDS; i++) {
      if (newsounds[i] == 0)
        break;
      if (!S_IdPlaying(newsounds[i])) {
        S_RemoveSoundFx(newsounds[i]);
        break;
      }
    }

    if (i == MAXNEWSOUNDS) {
      CONS_Debug(DBG_GAMELOGIC, "Cannot load another extra sound!\n");
      return;
    }

    soundnum = S_AddSoundFx(soundname, false, 0, false);
    newsounds[i] = soundnum;
  }

  S_StartSound(mo, soundnum);
}

//
// Initializes sound stuff, including volume
// Sets channels, SFX volume,
//  allocates channel buffer, sets S_sfx lookup.
//
void S_InitSfxChannels(INT32 sfxVolume) {
  INT32 i;

  if (dedicated)
    return;

  S_SetSfxVolume(sfxVolume);

  SetChannelsNum();

  // Note that sounds have not been cached (yet).
  for (i = 1; i < NUMSFX; i++) {
    S_sfx[i].usefulness = -1; // for I_GetSfx()
    S_sfx[i].lumpnum = LUMPERROR;
  }

  // precache sounds if requested by cmdline, or precachesound var true
  if (!sound_disabled &&
      (M_CheckParm("-precachesound") || precachesound.value)) {
    // Initialize external data (all sounds) at start, keep static.
    CONS_Printf(M_GetText("Loading sounds... "));

    for (i = 1; i < NUMSFX; i++)
      if (S_sfx[i].name)
        S_sfx[i].data = I_GetSfx(&S_sfx[i]);

    CONS_Printf(M_GetText(" pre-cached all sound data\n"));
  }
}

/// ------------------------
/// Music
/// ------------------------

#ifdef MUSICSLOT_COMPATIBILITY
const char *compat_special_music_slots[16] = {
    "titles", // 1036  title screen
    "read_m", // 1037  intro
    "lclear", // 1038  level clear
    "invinc", // 1039  invincibility
    "shoes",  // 1040  super sneakers
    "minvnc", // 1041  Mario invincibility
    "drown",  // 1042  drowning
    "gmover", // 1043  game over
    "xtlife", // 1044  extra life
    "contsc", // 1045  continue screen
    "supers", // 1046  Super Sonic
    "chrsel", // 1047  character select
    "credit", // 1048  credits
    "racent", // 1049  Race Results
    "stjr",   // 1050  Sonic Team Jr. Presents
    ""};
#endif

static char music_name[7]; // up to 6-character name
static void *music_data;
static UINT16 music_flags;
static boolean music_looping;

static char queue_name[7];
static UINT16 queue_flags;
static boolean queue_looping;
static UINT32 queue_position;
static UINT32 queue_fadeinms;

/// ------------------------
/// Music Definitions
/// ------------------------

musicdef_t *musicdefstart = NULL;   // First music definition
struct cursongcredit cursongcredit; // Currently displayed song credit info

//
// search for music definition in wad
//
static UINT16 W_CheckForMusicDefInPwad(UINT16 wadid) {
  UINT16 i;
  lumpinfo_t *lump_p;

  lump_p = wadfiles[wadid]->lumpinfo;
  for (i = 0; i < wadfiles[wadid]->numlumps; i++, lump_p++)
    if (memcmp(lump_p->name, "MUSICDEF", 8) == 0)
      return i;

  return INT16_MAX; // not found
}

void S_LoadMusicDefs(UINT16 wadnum) {
  UINT16 lump;
  char *buf;
  char *buf2;
  char *stoken;
  char *value;
  size_t size;
  musicdef_t *def, *prev;
  UINT16 line = 1; // for better error msgs

  lump = W_CheckForMusicDefInPwad(wadnum);
  if (lump == INT16_MAX)
    return;

  buf = W_CacheLumpNumPwad(wadnum, lump, PU_CACHE);
  size = W_LumpLengthPwad(wadnum, lump);

  // for strtok
  buf2 = malloc(size + 1);
  if (!buf2)
    I_Error("S_LoadMusicDefs: No more free memory\n");
  M_Memcpy(buf2, buf, size);
  buf2[size] = '\0';

  def = prev = NULL;

  stoken = strtok(buf2, "\r\n ");
  // Find music def
  while (stoken) {
    /*if ((stoken[0] == '/' && stoken[1] == '/')
            || (stoken[0] == '#')) // skip comments
    {
            stoken = strtok(NULL, "\r\n"); // skip end of line
            if (def)
                    stoken = strtok(NULL, "\r\n= ");
            else
                    stoken = strtok(NULL, "\r\n ");
            line++;
    }
    else*/
    if (!stricmp(stoken, "lump")) {
      value = strtok(NULL, "\r\n ");

      if (!value) {
        CONS_Alert(CONS_WARNING,
                   "MUSICDEF: Lump '%s' is missing name. (file %s, line %d)\n",
                   stoken, wadfiles[wadnum]->filename, line);
        stoken = strtok(NULL, "\r\n"); // skip end of line
        goto skip_lump;
      }

      // No existing musicdefs
      if (!musicdefstart) {
        musicdefstart = Z_Calloc(sizeof(musicdef_t), PU_STATIC, NULL);
        STRBUFCPY(musicdefstart->name, value);
        strlwr(musicdefstart->name);
        def = musicdefstart;
        // CONS_Printf("S_LoadMusicDefs: Initialized musicdef w/ song '%s'\n",
        // def->name);
      } else {
        def = musicdefstart;

        // Search if this is a replacement
        // CONS_Printf("S_LoadMusicDefs: Searching for song replacement...\n");
        while (def) {
          if (!stricmp(def->name, value)) {
            // CONS_Printf("S_LoadMusicDefs: Found song replacement '%s'\n",
            // def->name);
            break;
          }

          prev = def;
          def = def->next;
        }

        // Nothing found, add to the end.
        if (!def) {
          def = Z_Calloc(sizeof(musicdef_t), PU_STATIC, NULL);
          STRBUFCPY(def->name, value);
          strlwr(def->name);
          if (prev != NULL)
            prev->next = def;
          // CONS_Printf("S_LoadMusicDefs: Added song '%s'\n", def->name);
        }
      }

    skip_lump:
      stoken = strtok(NULL, "\r\n ");
      line++;
    } else {
      value = strtok(NULL, "\r\n= ");

      if (!value) {
        CONS_Alert(
            CONS_WARNING,
            "MUSICDEF: Field '%s' is missing value. (file %s, line %d)\n",
            stoken, wadfiles[wadnum]->filename, line);
        stoken = strtok(NULL, "\r\n"); // skip end of line
        goto skip_field;
      }

      if (!def) {
        CONS_Alert(CONS_ERROR,
                   "MUSICDEF: No music definition before field '%s'. (file %s, "
                   "line %d)\n",
                   stoken, wadfiles[wadnum]->filename, line);
        free(buf2);
        return;
      }

      if (!stricmp(stoken, "usage")) {
#if 0 // Ignore for now
				STRBUFCPY(def->usage, value);
				for (value = def->usage; *value; value++)
					if (*value == '_') *value = ' '; // turn _ into spaces.
				//CONS_Printf("S_LoadMusicDefs: Set usage to '%s'\n", def->usage);
#endif
      } else if (!stricmp(stoken, "source")) {
        STRBUFCPY(def->source, value);
        for (value = def->source; *value; value++)
          if (*value == '_')
            *value = ' '; // turn _ into spaces.
        // CONS_Printf("S_LoadMusicDefs: Set source to '%s'\n", def->source);
      } else {
        CONS_Alert(CONS_WARNING,
                   "MUSICDEF: Invalid field '%s'. (file %s, line %d)\n", stoken,
                   wadfiles[wadnum]->filename, line);
      }

    skip_field:
      stoken = strtok(NULL, "\r\n= ");
      line++;
    }
  }

  free(buf2);
  return;
}

//
// S_InitMusicDefs
//
// Simply load music defs in all wads.
//
void S_InitMusicDefs(void) {
  UINT16 i;
  for (i = 0; i < numwadfiles; i++)
    S_LoadMusicDefs(i);
}

//
// S_ShowMusicCredit
//
// Display current song's credit on screen
//
void S_ShowMusicCredit(void) {
  musicdef_t *def = musicdefstart;

  if (!cv_songcredits.value || demo.rewinding)
    return;

  if (!def) // No definitions
    return;

  while (def) {
    if (!stricmp(def->name, music_name)) {
      cursongcredit.def = def;
      cursongcredit.anim = 5 * TICRATE;
      cursongcredit.x = 0;
      cursongcredit.trans = NUMTRANSMAPS;
      return;
    } else
      def = def->next;
  }
}

/// ------------------------
/// Music Status
/// ------------------------

boolean S_DigMusicDisabled(void) { return digital_disabled; }

boolean S_MIDIMusicDisabled(void) {
  return midi_disabled; // SRB2Kart: defined as "true" w/ NO_MIDI
}

boolean S_MusicDisabled(void) { return (midi_disabled && digital_disabled); }

boolean S_MusicPlaying(void) { return I_SongPlaying(); }

boolean S_MusicPaused(void) { return I_SongPaused(); }

musictype_t S_MusicType(void) { return I_SongType(); }

const char *S_MusicName(void) { return music_name; }

boolean S_MusicInfo(char *mname, UINT16 *mflags, boolean *looping) {
  if (!I_SongPlaying())
    return false;

  strncpy(mname, music_name, 7);
  mname[6] = 0;
  *mflags = music_flags;
  *looping = music_looping;

  return (boolean)mname[0];
}

boolean S_MusicExists(const char *mname, boolean checkMIDI, boolean checkDigi) {
  return (
      (checkDigi ? W_CheckNumForName(va("O_%s", mname)) != LUMPERROR : false) ||
      (checkMIDI ? W_CheckNumForName(va("D_%s", mname)) != LUMPERROR : false));
}

/// ------------------------
/// Music Effects
/// ------------------------

boolean S_SpeedMusic(float speed) { return I_SetSongSpeed(speed); }

/// ------------------------
/// Music Seeking
/// ------------------------

UINT32
S_GetMusicLength(void) { return I_GetSongLength(); }

boolean S_SetMusicLoopPoint(UINT32 looppoint) {
  return I_SetSongLoopPoint(looppoint);
}

UINT32
S_GetMusicLoopPoint(void) { return I_GetSongLoopPoint(); }

boolean S_SetMusicPosition(UINT32 position) {
  return I_SetSongPosition(position);
}

UINT32
S_GetMusicPosition(void) { return I_GetSongPosition(); }

/// ------------------------
/// Music Playback
/// ------------------------

static boolean S_LoadMusic(const char *mname) {
  lumpnum_t mlumpnum;
  void *mdata;

  if (S_MusicDisabled())
    return false;

  if (!S_DigMusicDisabled() && S_DigExists(mname))
    mlumpnum = W_GetNumForName(va("o_%s", mname));
  else if (!S_MIDIMusicDisabled() && S_MIDIExists(mname))
    mlumpnum = W_GetNumForName(va("d_%s", mname));
  else if (S_DigMusicDisabled() && S_DigExists(mname)) {
    CONS_Alert(CONS_NOTICE, "Digital music is disabled!\n");
    return false;
  } else if (S_MIDIMusicDisabled() && S_MIDIExists(mname)) {
#ifdef NO_MIDI
    CONS_Alert(CONS_ERROR,
               "A MIDI music lump %.6s was found,\nbut SRB2Kart does not "
               "support MIDI output.\nWe apologise for the inconvenience.\n",
               mname);
#else
    CONS_Alert(CONS_NOTICE, "MIDI music is disabled!\n");
#endif
    return false;
  } else {
    CONS_Alert(CONS_ERROR, M_GetText("Music lump %.6s not found!\n"), mname);
    return false;
  }

  // load & register it
  mdata = W_CacheLumpNum(mlumpnum, PU_MUSIC);

#ifdef MUSSERV
  if (msg_id != -1) {
    struct musmsg msg_buffer;

    msg_buffer.msg_type = 6;
    memset(msg_buffer.msg_text, 0, sizeof(msg_buffer.msg_text));
    sprintf(msg_buffer.msg_text, "d_%s", mname);
    msgsnd(msg_id, (struct msgbuf *)&msg_buffer, sizeof(msg_buffer.msg_text),
           IPC_NOWAIT);
  }
#endif

  if (I_LoadSong(mdata, W_LumpLength(mlumpnum))) {
    strncpy(music_name, mname, 7);
    music_name[6] = 0;
    music_data = mdata;
    return true;
  } else
    return false;
}

static void S_UnloadMusic(void) {
  I_UnloadSong();

  music_data = NULL;

  music_name[0] = 0;
  music_flags = 0;
  music_looping = false;
}

static boolean S_PlayMusic(boolean looping, UINT32 fadeinms) {
  if (S_MusicDisabled())
    return false;

  if ((!fadeinms && !I_PlaySong(looping)) ||
      (fadeinms && !I_FadeInPlaySong(fadeinms, looping))) {
    S_UnloadMusic();
    return false;
  }

  S_InitMusicVolume(); // switch between digi and sequence volume

  if (window_notinfocus && !cv_playmusicifunfocused.value)
    I_SetMusicVolume(0);

  return true;
}

static void S_QueueMusic(const char *mmusic, UINT16 mflags, boolean looping,
                         UINT32 position, UINT32 fadeinms) {
  strncpy(queue_name, mmusic, 7);
  queue_flags = mflags;
  queue_looping = looping;
  queue_position = position;
  queue_fadeinms = fadeinms;
}

static void S_ClearQueue(void) {
  queue_name[0] = queue_flags = queue_looping = queue_position =
      queue_fadeinms = 0;
}

static void S_ChangeMusicToQueue(void) {
  S_ChangeMusicEx(queue_name, queue_flags, queue_looping, queue_position, 0,
                  queue_fadeinms);
  S_ClearQueue();
}

void S_ChangeMusicEx(const char *mmusic, UINT16 mflags, boolean looping,
                     UINT32 position, UINT32 prefadems, UINT32 fadeinms) {
  char newmusic[7];

#if defined(DC) || defined(_WIN32_WCE) || defined(PSP) || defined(GP2X)
  S_ClearSfx();
#endif

  if (S_MusicDisabled() ||
      demo.rewinding // Don't mess with music while rewinding!
      || demo.title) // SRB2Kart: Demos don't interrupt title screen music
    return;

  strncpy(newmusic, mmusic, 7);
  if (LUAh_MusicChange(music_name, newmusic, &mflags, &looping, &position,
                       &prefadems, &fadeinms))
    return;
  newmusic[6] = 0;

  // No Music (empty string)
  if (newmusic[0] == 0) {
    if (prefadems)
      I_FadeSong(0, prefadems, &S_StopMusic);
    else
      S_StopMusic();
    return;
  }

  if (prefadems && S_MusicPlaying()) // queue music change for after fade //
                                     // allow even if the music is the same
  {
    CONS_Debug(DBG_DETAILED, "Now fading out song %s\n", music_name);
    S_QueueMusic(newmusic, mflags, looping, position, fadeinms);
    I_FadeSong(0, prefadems, S_ChangeMusicToQueue);
    return;
  } else if (strnicmp(music_name, newmusic, 6) || (mflags & MUSIC_FORCERESET)) {
    CONS_Debug(DBG_DETAILED, "Now playing song %s\n", newmusic);

    S_StopMusic();

    if (!S_LoadMusic(newmusic)) {
      CONS_Alert(CONS_ERROR, "Music %.6s could not be loaded!\n", newmusic);
      return;
    }

    music_flags = mflags;
    music_looping = looping;

    if (!S_PlayMusic(looping, fadeinms)) {
      CONS_Alert(CONS_ERROR, "Music %.6s could not be played!\n", newmusic);
      return;
    }

    if (position)
      I_SetSongPosition(position);

    I_SetSongTrack(mflags & MUSIC_TRACKMASK);
  } else if (fadeinms) // let fades happen with same music
  {
    I_SetSongPosition(position);
    I_FadeSong(100, fadeinms, NULL);
  } else // reset volume to 100 with same music
  {
    I_StopFadingSong();
    I_FadeSong(100, 500, NULL);
  }
}

void S_StopMusic(void) {
  if (!I_SongPlaying() ||
      demo.rewinding // Don't mess with music while rewinding!
      || demo.title) // SRB2Kart: Demos don't interrupt title screen music
    return;

  if (I_SongPaused())
    I_ResumeSong();

  S_SpeedMusic(1.0f);
  I_StopSong();
  S_UnloadMusic(); // for now, stopping also means you unload the song
}

//
// Stop and resume music, during game PAUSE.
//
void S_PauseAudio(void) {
  if (I_SongPlaying() && !I_SongPaused())
    I_PauseSong();

  // pause cd music
  I_PauseCD();
}

void S_ResumeAudio(void) {
  if (I_SongPlaying() && I_SongPaused())
    I_ResumeSong();

  // resume cd music
  I_ResumeCD();
}

void S_DisableSound(void) {
  if (sound_started && !sound_disabled) {
    sound_disabled = true;
    S_StopSounds();
  }
}

void S_EnableSound(void) {
  if (sound_started && sound_disabled) {
    sound_disabled = false;
    S_InitSfxChannels(cv_soundvolume.value);
  }
}

void S_SetMusicVolume(INT32 digvolume, INT32 seqvolume) {
  if (digvolume < 0)
    digvolume = cv_digmusicvolume.value;

#ifdef NO_MIDI
  (void)seqvolume;
#else
  if (seqvolume < 0)
    seqvolume = cv_midimusicvolume.value;
#endif

  if (digvolume < 0 || digvolume > 31)
    CONS_Alert(CONS_WARNING, "digmusicvolume should be between 0-31\n");
  CV_SetValue(&cv_digmusicvolume, digvolume & 31);
  actualdigmusicvolume = cv_digmusicvolume.value; // check for change of var

#ifndef NO_MIDI
  if (seqvolume < 0 || seqvolume > 31)
    CONS_Alert(CONS_WARNING, "midimusicvolume should be between 0-31\n");
  CV_SetValue(&cv_midimusicvolume, seqvolume & 31);
  actualmidimusicvolume = cv_midimusicvolume.value; // check for change of var
#endif

#ifdef DJGPPDOS
  digvolume = 31;
#ifndef NO_MIDI
  seqvolume = 31;
#endif
#endif

  switch (I_SongType()) {
#ifndef NO_MIDI
  case MU_MID:
    // case MU_MOD:
    // case MU_GME:
    I_SetMusicVolume(seqvolume & 31);
    break;
#endif
  default:
    I_SetMusicVolume(digvolume & 31);
    break;
  }
}

/// ------------------------
/// Music Fading
/// ------------------------

void S_SetInternalMusicVolume(INT32 volume) {
  I_SetInternalMusicVolume(min(max(volume, 0), 100));
}

void S_StopFadingMusic(void) { I_StopFadingSong(); }

boolean S_FadeMusicFromVolume(UINT8 target_volume, INT16 source_volume,
                              UINT32 ms) {
  if (source_volume < 0)
    return I_FadeSong(target_volume, ms, NULL);
  else
    return I_FadeSongFromVolume(target_volume, source_volume, ms, NULL);
}

boolean S_FadeOutStopMusic(UINT32 ms) {
  return I_FadeSong(0, ms, &S_StopMusic);
}

/// ------------------------
/// Init & Others
/// ------------------------

//
// Per level startup code.
// Kills playing sounds at start of level,
//  determines music if any, changes music.
//
void S_Start(void) {
  if (mapmusflags & MUSIC_RELOADRESET) {
    strncpy(mapmusname, mapheaderinfo[gamemap - 1]->musname, 7);
    mapmusname[6] = 0;
    mapmusflags = (mapheaderinfo[gamemap - 1]->mustrack & MUSIC_TRACKMASK);
    mapmusposition = mapheaderinfo[gamemap - 1]->muspos;
  }

  // if (cv_resetmusic.value) // Starting ambience should always be restarted
  S_StopMusic();

  if (leveltime < (starttime + (TICRATE / 2))) // SRB2Kart
    S_ChangeMusicEx((encoremode ? "estart" : "kstart"), 0, false,
                    mapmusposition, 0, 0);
  else
    S_ChangeMusicEx(mapmusname, mapmusflags, true, mapmusposition, 0, 0);
}

static void Command_Tunes_f(void) {
  const char *tunearg;
  UINT16 tunenum, track = 0;
  UINT32 position = 0;
  const size_t argc = COM_Argc();

  if (argc < 2) // tunes slot ...
  {
    CONS_Printf("tunes <name/num> [track] [speed] [position] / <-show> / "
                "<-default> / <-none>:\n");
    CONS_Printf(M_GetText("Play an arbitrary music lump. If a map number is "
                          "used, 'MAP##M' is played.\n"));
    CONS_Printf(M_GetText("If the format supports multiple songs, you can "
                          "specify which one to play.\n\n"));
    CONS_Printf(M_GetText(
        "* With \"-show\", shows the currently playing tune and track.\n"));
    CONS_Printf(M_GetText(
        "* With \"-default\", returns to the default music for the map.\n"));
    CONS_Printf(
        M_GetText("* With \"-none\", any music playing will be stopped.\n"));
    return;
  }

  tunearg = COM_Argv(1);
  tunenum = (UINT16)atoi(tunearg);
  track = 0;

  if (!strcasecmp(tunearg, "-show")) {
    CONS_Printf(M_GetText("The current tune is: %s [track %d]\n"), mapmusname,
                (mapmusflags & MUSIC_TRACKMASK));
    return;
  }
  if (!strcasecmp(tunearg, "-none")) {
    S_StopMusic();
    return;
  } else if (!strcasecmp(tunearg, "-default")) {
    tunearg = mapheaderinfo[gamemap - 1]->musname;
    track = mapheaderinfo[gamemap - 1]->mustrack;
  } else if (!tunearg[2] && toupper(tunearg[0]) >= 'A' &&
             toupper(tunearg[0]) <= 'Z')
    tunenum = (UINT16)M_MapNumber(tunearg[0], tunearg[1]);

  if (tunenum && tunenum >= 1036) {
    CONS_Alert(CONS_NOTICE, M_GetText("Valid music slots are 1 to 1035.\n"));
    return;
  }
  if (!tunenum &&
      strlen(tunearg) >
          6) // This is automatic -- just show the error just in case
    CONS_Alert(
        CONS_NOTICE,
        M_GetText("Music name too long - truncated to six characters.\n"));

  if (argc > 2)
    track = (UINT16)atoi(COM_Argv(2)) - 1;

  if (tunenum)
    snprintf(mapmusname, 7, "%sM", G_BuildMapName(tunenum));
  else
    strncpy(mapmusname, tunearg, 7);

  if (argc > 4)
    position = (UINT32)atoi(COM_Argv(4));

  mapmusname[6] = 0;
  mapmusflags = (track & MUSIC_TRACKMASK);
  mapmusposition = position;

  S_ChangeMusicEx(mapmusname, mapmusflags, true, mapmusposition, 0, 0);

  if (argc > 3) {
    float speed = (float)atof(COM_Argv(3));
    if (speed > 0.0f)
      S_SpeedMusic(speed);
  }
}

static void Command_RestartAudio_f(void) {
  if (dedicated) // No point in doing anything if game is a dedicated server.
    return;

  S_StopMusic();
  S_StopSounds();
  I_ShutdownMusic();
  I_ShutdownSound();
  I_StartupSound();
  I_InitMusic();

  // These must be called or no sound and music until manually set.

  I_SetSfxVolume(cv_soundvolume.value);
#ifdef NO_MIDI
  S_SetMusicVolume(cv_digmusicvolume.value, -1);
#else
  S_SetMusicVolume(cv_digmusicvolume.value, cv_midimusicvolume.value);
#endif

  S_StartSound(NULL, sfx_strpst);

  if (Playing()) // Gotta make sure the player is in a level
    P_RestoreMusic(&players[consoleplayer]);
  else
    S_ChangeMusicInternal("titles", looptitle);
}

void GameSounds_OnChange(void) {
  if (M_CheckParm("-nosound") || M_CheckParm("-noaudio"))
    return;

  if (sound_disabled) {
    if (!(cv_playsoundifunfocused.value && window_notinfocus))
      S_EnableSound();
  } else
    S_DisableSound();
}

void GameDigiMusic_OnChange(void) {
  if (M_CheckParm("-nomusic") || M_CheckParm("-noaudio"))
    return;
  else if (M_CheckParm("-nodigmusic"))
    return;

  if (digital_disabled) {
    digital_disabled = false;
    I_InitMusic();
    S_StopMusic();
    if (Playing())
      P_RestoreMusic(&players[consoleplayer]);
    else
      S_ChangeMusicInternal("titles", looptitle);
  } else {
    digital_disabled = true;
    if (S_MusicType() != MU_MID) {
      if (midi_disabled)
        S_StopMusic();
      else {
        char mmusic[7];
        UINT16 mflags;
        boolean looping;

        if (S_MusicInfo(mmusic, &mflags, &looping) && S_MIDIExists(mmusic)) {
          S_StopMusic();
          S_ChangeMusic(mmusic, mflags, looping);
        } else
          S_StopMusic();
      }
    }
  }
}

#ifndef NO_MIDI
void GameMIDIMusic_OnChange(void) {
  if (M_CheckParm("-nomusic") || M_CheckParm("-noaudio"))
    return;
  else if (M_CheckParm("-nomidimusic"))
    return;

  if (midi_disabled) {
    midi_disabled = false;
    I_InitMusic();
    if (Playing())
      P_RestoreMusic(&players[consoleplayer]);
    else
      S_ChangeMusicInternal("titles", looptitle);
  } else {
    midi_disabled = true;
    if (S_MusicType() == MU_MID) {
      if (digital_disabled)
        S_StopMusic();
      else {
        char mmusic[7];
        UINT16 mflags;
        boolean looping;

        if (S_MusicInfo(mmusic, &mflags, &looping) && S_DigExists(mmusic)) {
          S_StopMusic();
          S_ChangeMusic(mmusic, mflags, looping);
        } else
          S_StopMusic();
      }
    }
  }
}
#endif

static void PlayMusicIfUnfocused_OnChange(void) {
  if (window_notinfocus) {
    if (cv_playmusicifunfocused.value)
      I_SetMusicVolume(0);
    else
      S_InitMusicVolume();
  }
}

static void PlaySoundIfUnfocused_OnChange(void) {
  if (!cv_gamesounds.value)
    return;

  if (window_notinfocus) {
    if (cv_playsoundifunfocused.value)
      S_DisableSound();
    else
      S_EnableSound();
  }
}

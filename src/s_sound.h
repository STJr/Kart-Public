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
/// \file  s_sound.h
/// \brief The not so system specific sound interface

#ifndef __S_SOUND__
#define __S_SOUND__

#include "i_sound.h" // musictype_t
#include "sounds.h"
#include "m_fixed.h"
#include "command.h"
#include "tables.h" // angle_t

// mask used to indicate sound origin is player item pickup
#define PICKUP_SOUND 0x8000

extern consvar_t stereoreverse;
extern consvar_t cv_soundvolume, cv_digmusicvolume;//, cv_midimusicvolume;
extern consvar_t cv_numChannels;
extern consvar_t surround;
//extern consvar_t cv_resetmusic;
extern consvar_t cv_gamedigimusic;
#ifndef NO_MIDI
extern consvar_t cv_gamemidimusic;
#endif
extern consvar_t cv_gamesounds;
extern consvar_t cv_playmusicifunfocused;
extern consvar_t cv_playsoundifunfocused;

#ifdef SNDSERV
extern consvar_t sndserver_cmd, sndserver_arg;
#endif
#ifdef MUSSERV
extern consvar_t musserver_cmd, musserver_arg;
#endif

extern CV_PossibleValue_t soundvolume_cons_t[];
//part of i_cdmus.c
extern consvar_t cd_volume, cdUpdate;

#if defined (macintosh) && !defined (HAVE_SDL)
typedef enum
{
	music_normal,
	playlist_random,
	playlist_normal
} playmode_t;

extern consvar_t play_mode;
#endif

typedef enum
{
	SF_TOTALLYSINGLE =  1, // Only play one of these sounds at a time...GLOBALLY
	SF_NOMULTIPLESOUND =  2, // Like SF_NOINTERRUPT, but doesnt care what the origin is
	SF_OUTSIDESOUND  =  4, // Volume is adjusted depending on how far away you are from 'outside'
	SF_X4AWAYSOUND   =  8, // Hear it from 4x the distance away
	SF_X8AWAYSOUND   = 16, // Hear it from 8x the distance away
	SF_NOINTERRUPT   = 32, // Only play this sound if it isn't already playing on the origin
	SF_X2AWAYSOUND   = 64, // Hear it from 2x the distance away
} soundflags_t;

typedef struct {
	fixed_t x, y, z;
	angle_t angle;
} listener_t;

// register sound vars and commands at game startup
void S_RegisterSoundStuff(void);

//
// Initializes sound stuff, including volume
// Sets channels, SFX, allocates channel buffer, sets S_sfx lookup.
//
void S_InitSfxChannels(INT32 sfxVolume);

//
// Per level startup code.
// Kills playing sounds at start of level, determines music if any, changes music.
//
void S_StopSounds(void);
void S_ClearSfx(void);
void S_Start(void);

//
// Basically a W_GetNumForName that adds "ds" at the beginning of the string. Returns a lumpnum.
//
lumpnum_t S_GetSfxLumpNum(sfxinfo_t *sfx);

//
// Start sound for thing at <origin> using <sound_id> from sounds.h
//
void S_StartSound(const void *origin, sfxenum_t sound_id);

// Will start a sound at a given volume.
void S_StartSoundAtVolume(const void *origin, sfxenum_t sound_id, INT32 volume);

// Stop sound for thing at <origin>
void S_StopSound(void *origin);

//
// Music Status
//

boolean S_DigMusicDisabled(void);
boolean S_MIDIMusicDisabled(void);
boolean S_MusicDisabled(void);
boolean S_MusicPlaying(void);
boolean S_MusicPaused(void);
musictype_t S_MusicType(void);
const char *S_MusicName(void);
boolean S_MusicInfo(char *mname, UINT16 *mflags, boolean *looping);
boolean S_MusicExists(const char *mname, boolean checkMIDI, boolean checkDigi);
#define S_DigExists(a) S_MusicExists(a, false, true)
#define S_MIDIExists(a) S_MusicExists(a, true, false)

//
// Music Effects
//

// Set Speed of Music
boolean S_SpeedMusic(float speed);

// Music credits
typedef struct musicdef_s
{
	char name[7];
	//char usage[256];
	char source[256];
	struct musicdef_s *next;
} musicdef_t;

extern struct cursongcredit
{
	musicdef_t *def;
	UINT16 anim;
	fixed_t x;
	UINT8 trans;
} cursongcredit;

extern musicdef_t *musicdefstart;

void S_LoadMusicDefs(UINT16 wadnum);
void S_InitMusicDefs(void);
void S_ShowMusicCredit(void);

//
// Music Seeking
//

// Get Length of Music
UINT32 S_GetMusicLength(void);

// Set LoopPoint of Music
boolean S_SetMusicLoopPoint(UINT32 looppoint);

// Get LoopPoint of Music
UINT32 S_GetMusicLoopPoint(void);

// Set Position of Music
boolean S_SetMusicPosition(UINT32 position);

// Get Position of Music
UINT32 S_GetMusicPosition(void);

//
// Music Playback
//

// Start music track, arbitrary, given its name, and set whether looping
// note: music flags 12 bits for tracknum (gme, other formats with more than one track)
//       13-15 aren't used yet
//       and the last bit we ignore (internal game flag for resetting music on reload)
void S_ChangeMusicEx(const char *mmusic, UINT16 mflags, boolean looping, UINT32 position, UINT32 prefadems, UINT32 fadeinms);
#define S_ChangeMusicInternal(a,b) S_ChangeMusicEx(a,0,b,0,0,0)
#define S_ChangeMusic(a,b,c) S_ChangeMusicEx(a,b,c,0,0,0)

// Stops the music.
void S_StopMusic(void);

// Stop and resume music, during game PAUSE.
void S_PauseAudio(void);
void S_ResumeAudio(void);

// Enable and disable sound effects
void S_EnableSound(void);
void S_DisableSound(void);

//
// Music Fading
//

void S_SetInternalMusicVolume(INT32 volume);
void S_StopFadingMusic(void);
boolean S_FadeMusicFromVolume(UINT8 target_volume, INT16 source_volume, UINT32 ms);
#define S_FadeMusic(a, b) S_FadeMusicFromVolume(a, -1, b)
#define S_FadeInChangeMusic(a,b,c,d) S_ChangeMusicEx(a,b,c,0,0,d)
boolean S_FadeOutStopMusic(UINT32 ms);

//
// Updates music & sounds
//
void S_UpdateSounds(void);

FUNCMATH fixed_t S_CalculateSoundDistance(fixed_t px1, fixed_t py1, fixed_t pz1, fixed_t px2, fixed_t py2, fixed_t pz2);

void S_SetSfxVolume(INT32 volume);
void S_SetMusicVolume(INT32 digvolume, INT32 seqvolume);
#define S_SetDigMusicVolume(a) S_SetMusicVolume(a,-1)
#define S_SetMIDIMusicVolume(a) S_SetMusicVolume(-1,a)
#define S_InitMusicVolume() S_SetMusicVolume(-1,-1)

INT32 S_OriginPlaying(void *origin);
INT32 S_IdPlaying(sfxenum_t id);
INT32 S_SoundPlaying(void *origin, sfxenum_t id);

void S_StartSoundName(void *mo, const  char *soundname);

void S_StopSoundByID(void *origin, sfxenum_t sfx_id);
void S_StopSoundByNum(sfxenum_t sfxnum);

#ifndef HW3SOUND
#define S_StartAttackSound S_StartSound
#define S_StartScreamSound S_StartSound
#endif

#ifdef MUSICSLOT_COMPATIBILITY
// For compatibility with code/scripts relying on older versions
// This is a list of all the "special" slot names and their associated numbers
extern const char *compat_special_music_slots[16];
#endif

#endif

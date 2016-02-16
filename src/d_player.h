// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 1999-2014 by Sonic Team Junior.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  d_player.h
/// \brief player data structures

#ifndef __D_PLAYER__
#define __D_PLAYER__

// The player data structure depends on a number
// of other structs: items (internal inventory),
// animation states (closely tied to the sprites
// used to represent them, unfortunately).
#include "p_pspr.h"

// In addition, the player is just a special
// case of the generic moving object/actor.
#include "p_mobj.h"

// Finally, for odd reasons, the player input
// is buffered within the player data struct,
// as commands per game tick.
#include "d_ticcmd.h"

// Extra abilities/settings for skins (combinable stuff)
typedef enum
{
	SF_SUPER            = 1, // Can turn super in singleplayer/co-op mode.
	SF_SUPERANIMS       = 1<<1, // If super, use the super sonic animations
	SF_SUPERSPIN        = 1<<2, // Should spin frames be played while super?
	SF_HIRES            = 1<<3, // Draw the sprite 2x as small?
	SF_NOSKID           = 1<<4, // No skid particles etc
	SF_NOSPEEDADJUST    = 1<<5, // Skin-specific version of disablespeedadjust
	SF_RUNONWATER       = 1<<6, // Run on top of water FOFs?
} skinflags_t;

//Primary and secondary skin abilities
typedef enum
{
	CA_NONE=0,
	CA_THOK,
	CA_FLY,
	CA_GLIDEANDCLIMB,
	CA_HOMINGTHOK,
	CA_SWIM,
	CA_DOUBLEJUMP,
	CA_FLOAT,
	CA_SLOWFALL,
	CA_TELEKINESIS,
	CA_FALLSWITCH,
	CA_JUMPBOOST,
	CA_AIRDRILL,
	CA_JUMPTHOK
} charability_t;

//Secondary skin abilities
typedef enum
{
	CA2_NONE=0,
	CA2_SPINDASH,
	CA2_MULTIABILITY
} charability2_t;

//
// Player states.
//
typedef enum
{
	// Playing or camping.
	PST_LIVE,
	// Dead on the ground, view follows killer.
	PST_DEAD,
	// Ready to restart/respawn???
	PST_REBORN
} playerstate_t;

//
// Player internal flags
//
typedef enum
{
	// Flip camera angle with gravity flip prefrence.
	PF_FLIPCAM = 1,

	// Cheats
	PF_GODMODE = 1<<1,
	PF_NOCLIP  = 1<<2,
	PF_INVIS   = 1<<3,

	// True if button down last tic.
	PF_ATTACKDOWN = 1<<4,
	PF_USEDOWN    = 1<<5,
	PF_JUMPDOWN   = 1<<6,
	PF_WPNDOWN    = 1<<7,

	// Unmoving states
	PF_STASIS     = 1<<8, // Player is not allowed to move
	PF_JUMPSTASIS = 1<<9, // and that includes jumping.
	PF_FULLSTASIS = PF_STASIS|PF_JUMPSTASIS,

	// Did you get a time-over?
	PF_TIMEOVER = 1<<10,

	// Ready for Super?
	PF_SUPERREADY = 1<<11,

	// Character action status
	PF_JUMPED    = 1<<12,
	PF_SPINNING  = 1<<13,
	PF_STARTDASH = 1<<14,
	PF_THOKKED   = 1<<15,

	// Are you gliding?
	PF_GLIDING   = 1<<16,

	// Tails pickup!
	PF_CARRIED   = 1<<17,

	// Sliding (usually in water) like Labyrinth/Oil Ocean
	PF_SLIDING   = 1<<18,

	// Hanging on a rope
	PF_ROPEHANG = 1<<19,

	// Hanging on an item of some kind - zipline, chain, etc. (->tracer)
	PF_ITEMHANG = 1<<20,

	// On the mace chain spinning around (->tracer)
	PF_MACESPIN = 1<<21,

	/*** NIGHTS STUFF ***/
	// Is the player in NiGHTS mode?
	PF_NIGHTSMODE        = 1<<22,
	PF_TRANSFERTOCLOSEST = 1<<23,

	// Spill rings after falling
	PF_NIGHTSFALL        = 1<<24,
	PF_DRILLING          = 1<<25,
	PF_SKIDDOWN          = 1<<26,

	/*** TAG STUFF ***/
	PF_TAGGED            = 1<<27, // Player has been tagged and awaits the next round in hide and seek.
	PF_TAGIT             = 1<<28, // The player is it! For Tag Mode

	/*** misc ***/
	PF_FORCESTRAFE       = 1<<29, // Turning inputs are translated into strafing inputs
	PF_ANALOGMODE        = 1<<30, // Analog mode?

	// free: 1<<30 and 1<<31
} pflags_t;

typedef enum
{
	// Are animation frames playing?
	PA_ETC=0,
	PA_IDLE,
	PA_WALK,
	PA_RUN,
	PA_ROLL,
	PA_FALL,
	PA_ABILITY
} panim_t;

typedef enum
{
	SH_NONE = 0,
	// Standard shields
	SH_JUMP,
	SH_ATTRACT,
	SH_ELEMENTAL,
	SH_BOMB,
	// Stupid useless unimplimented Sonic 3 shields
	SH_BUBBLEWRAP,
	SH_THUNDERCOIN,
	SH_FLAMEAURA,
	// Pity shield: the world's most basic shield ever, given to players who suck at Match
	SH_PITY,
	// The fireflower is special, it combines with other shields.
	SH_FIREFLOWER = 0x100,
	// The force shield uses the lower 8 bits to count how many hits are left.
	SH_FORCE = 0x200,

	SH_STACK = SH_FIREFLOWER,
	SH_NOSTACK = ~SH_STACK
} shieldtype_t;

// Player powers. (don't edit this comment)
typedef enum
{
	pw_invulnerability,
	pw_sneakers,
	pw_flashing,
	pw_shield,
	pw_tailsfly, // tails flying
	pw_underwater, // underwater timer
	pw_spacetime, // In space, no one can hear you spin!
	pw_extralife, // Extra Life timer

	pw_super, // Are you super?
	pw_gravityboots, // gravity boots

	// Weapon ammunition
	pw_infinityring,
	pw_automaticring,
	pw_bouncering,
	pw_scatterring,
	pw_grenadering,
	pw_explosionring,
	pw_railring,

	// Power Stones
	pw_emeralds, // stored like global 'emeralds' variable

	// NiGHTS powerups
	pw_nights_superloop,
	pw_nights_helper,
	pw_nights_linkfreeze,

	//for linedef exec 427
	pw_nocontrol,
	pw_ingoop, // In goop

	// /////////////// //
	// SRB2kart Powers //		// SRB2kart 16/02/15  (TODO: Remove Riders powers we don't need.)
	// /////////////// //
    
    // Riders
	pw_boost,
	pw_brake,
	pw_exrun,			// Are you on your gear?
	pw_exspeed,			// Extreme Gear speed
	pw_airtank,			// Air points for your gear
	pw_airstartpoint,	// More points = Less airtank points
	pw_bustable,
	pw_ramp,
	pw_rampspeed,
	pw_rampheight,
	pw_rampspeed2,
	pw_rampheight2,
	pw_rampangle,
	pw_rampscore,
	pw_jmpcharge,		// Hold the jump button to charge it first

	// Race
	pw_drift,			// Drifting Left or Right, plus a bigger counter = sharper turn
	pw_driftcharge,		// Charge your drift so you can release a burst of speed
	pw_jmp,				// In Mario Kart, letting go of the jump button stops the drift
	pw_positiondelay,	// Similar to P_PlayTauntSound's delay
	pw_oldposition,

	// Kart
	pw_kartitem,
	pw_itemslot,		// If you have X item, and kartitem chose X too, save it.

	pw_star,			// 1 = Star in inventory

	pw_mushroom,		// Whether you are currently shroom boosting

	pw_shroom,			// 1 = 1 Mushroom in inventory, 2 = 2 Mushrooms in inventory, 
						// 4 = 3 Mushrooms in inventory, 8 = Gold Shroom in inventory
	pw_goldshroom,		// Timer for gold shroom usage

	pw_megamushroom,	// 1 = mega shroom in inventory

	// Moved here because I don't want to break maps
	pw_bananacam,		// Player turning timer

	pw_thunder,			// 1 = lightning in inventory

	pw_blueshell,		// 1 = Blue Shell in inventory

	pw_shell,			// 1 = shell being held, 2 = shell in inventory
	pw_tripleshell,		// 1 = 1 shell circling, 2 = 2 shells circling, 
						// 4 = 3 shells circling, 8 = triple shells in inventory

	pw_redshell,		// 1 = redshell being held, 2 = redshell in inventory
	pw_tripleredshell,	// 1 = 1 red shell circling, 2 = 2 red shells circling, 
						// 4 = 3 red shells circling, 8 = triple red shells in inventory

	pw_banana,			// 1 = banana being held, 2 = banana in inventory
	pw_triplebanana,	// 1 = 1 banana following, 2 = 2 bananas following, 
						// 4 = 3 bananas following, 8 = triple bananas in inventory

	pw_fakeitem,		// 1 = fake being held, 2 = fake in inventory

	pw_bomb,			// 1 = bomb being held, 2 = bomb in inventory

	pw_shrink,			// > 0  = small, < 0 = big
	pw_squished,		// Squashed timer
	pw_boostcharge,		// Charge your engine at the start of the race for a boost
	pw_fishing,			// Set up a timer that forces you to be in the air while respawning
	
	// Misc
	pw_psychic,			// Silver's floating power

	pw_waypoint,		// Waypoints. 'nuff said.
	pw_introcam,		// Early joiners need this, or a new joiner's camera will randomly rotate
	pw_sounds,			// We use this to avoid sounds being played every tic

	pw_boo,				// The initial boo, when used we decide who to swap with and 
                        // if we will actually swap, 1 = have boo in inventory
	pw_bootake,			// You are stealing an item from another player with boo, this is your timer
	pw_boostolen,		// you are the player being stolen from, this is your timer

	pw_greenboo,
	pw_kitchensink,

	pw_pcd,				// Past Checkpoint Distance
	pw_ncd,				// Next Chekpoint Distance
    
    
	NUMPOWERS
} powertype_t;

#define WEP_AUTO    1
#define WEP_BOUNCE  2
#define WEP_SCATTER 3
#define WEP_GRENADE 4
#define WEP_EXPLODE 5
#define WEP_RAIL    6
#define NUM_WEAPONS 7

typedef enum
{
	RW_AUTO    =  1,
	RW_BOUNCE  =  2,
	RW_SCATTER =  4,
	RW_GRENADE =  8,
	RW_EXPLODE = 16,
	RW_RAIL    = 32
} ringweapons_t;

// ========================================================================
//                          PLAYER STRUCTURE
// ========================================================================
typedef struct player_s
{
	mobj_t *mo;

	// Caveat: ticcmd_t is ATTRPACK! Be careful what precedes it.
	ticcmd_t cmd;

	playerstate_t playerstate;

	// Determine POV, including viewpoint bobbing during movement.
	// Focal origin above r.z
	fixed_t viewz;
	// Base height above floor for viewz.
	fixed_t viewheight;
	// Bob/squat speed.
	fixed_t deltaviewheight;
	// bounded/scaled total momentum.
	fixed_t bob;

	// Mouse aiming, where the guy is looking at!
	// It is updated with cmd->aiming.
	angle_t aiming;

	// This is only used between levels,
	// mo->health is used during levels.
	/// \todo Remove this.  We don't need a second health definition for players.
	INT32 health;

	SINT8 pity; // i pity the fool.
	INT32 currentweapon; // current weapon selected.
	INT32 ringweapons; // weapons currently obtained.

	// Power ups. invinc and invis are tic counters.
	UINT16 powers[NUMPOWERS];

	// Bit flags.
	// See pflags_t, above.
	pflags_t pflags;

	// playing animation.
	panim_t panim;

	// For screen flashing (bright).
	UINT16 flashcount;
	UINT16 flashpal;

	// Player skin colorshift, 0-15 for which color to draw player.
	UINT8 skincolor;

	INT32 skin;

	UINT32 score; // player score
	fixed_t dashspeed; // dashing speed
	INT32 dashtime; // tics dashing, used for rev sound

	fixed_t normalspeed; // Normal ground
	fixed_t runspeed; // Speed you break into the run animation
	UINT8 thrustfactor; // Thrust = thrustfactor * acceleration
	UINT8 accelstart; // Starting acceleration if speed = 0.
	UINT8 acceleration; // Acceleration

	// See charability_t and charability2_t for more information.
	UINT8 charability; // Ability definition
	UINT8 charability2; // Secondary ability definition

	UINT32 charflags; // Extra abilities/settings for skins (combinable stuff)
	                 // See SF_ flags

	mobjtype_t thokitem; // Object # to spawn for the thok
	mobjtype_t spinitem; // Object # to spawn for spindash/spinning
	mobjtype_t revitem; // Object # to spawn for spindash/spinning

	fixed_t actionspd; // Speed of thok/glide/fly
	fixed_t mindash; // Minimum spindash speed
	fixed_t maxdash; // Maximum spindash speed

	fixed_t jumpfactor; // How high can the player jump?

	SINT8 lives;
	SINT8 continues; // continues that player has acquired

	SINT8 xtralife; // Ring Extra Life counter
	UINT8 gotcontinue; // Got continue from this stage?

	fixed_t speed; // Player's speed (distance formula of MOMX and MOMY values)
	UINT8 jumping; // Jump counter
	UINT8 secondjump;

	UINT8 fly1; // Tails flying
	UINT8 scoreadd; // Used for multiple enemy attack bonus
	tic_t glidetime; // Glide counter for thrust
	UINT8 climbing; // Climbing on the wall
	INT32 deadtimer; // End game if game over lasts too long
	tic_t exiting; // Exitlevel timer

	UINT8 homing; // Are you homing?

	tic_t skidtime; // Skid timer

	////////////////////////////
	// Conveyor Belt Movement //
	////////////////////////////
	fixed_t cmomx; // Conveyor momx
	fixed_t cmomy; // Conveyor momy
	fixed_t rmomx; // "Real" momx (momx - cmomx)
	fixed_t rmomy; // "Real" momy (momy - cmomy)

	/////////////////////
	// Race Mode Stuff //
	/////////////////////
	INT16 numboxes; // Number of item boxes obtained for Race Mode
	INT16 totalring; // Total number of rings obtained for Race Mode
	tic_t realtime; // integer replacement for leveltime
	UINT8 laps; // Number of laps (optional)

    tic_t checkpointtimes[256]; // Checkpoints for race mode, etc       // SRB2kart 16/02/15
	INT32 playerahead; // is someone ahead of me or not?                // "

	////////////////////
	// CTF Mode Stuff //
	////////////////////
	INT32 ctfteam; // 0 == Spectator, 1 == Red, 2 == Blue
	UINT16 gotflag; // 1 == Red, 2 == Blue Do you have the flag?

	INT32 weapondelay; // Delay (if any) to fire the weapon again
	INT32 tossdelay;   // Delay (if any) to toss a flag/emeralds again

	// Starpost information
	INT16 starpostx;
	INT16 starposty;
	INT16 starpostz;
	INT32 starpostnum; // The number of the last starpost you hit
	tic_t starposttime; // Your time when you hit the starpost
	angle_t starpostangle; // Angle that the starpost is facing - you respawn facing this way

	/////////////////
	// NiGHTS Stuff//
	/////////////////
	angle_t angle_pos;
	angle_t old_angle_pos;

	mobj_t *axis1;
	mobj_t *axis2;
	tic_t bumpertime; // Currently being bounced by MT_NIGHTSBUMPER
	INT32 flyangle;
	tic_t drilltimer;
	INT32 linkcount;
	tic_t linktimer;
	INT32 anotherflyangle;
	tic_t nightstime; // How long you can fly as NiGHTS.
	INT32 drillmeter;
	UINT8 drilldelay;
	boolean bonustime; // Capsule destroyed, now it's bonus time!
	mobj_t *capsule; // Go inside the capsule
	UINT8 mare; // Current mare

	// Statistical purposes.
	tic_t marebegunat; // Leveltime when mare begun
	tic_t startedtime; // Time which you started this mare with.
	tic_t finishedtime; // Time it took you to finish the mare (used for display)
	INT16 finishedrings; // The rings you had left upon finishing the mare
	UINT32 marescore; // score for this nights stage
	UINT32 lastmarescore; // score for the last mare
	UINT8 lastmare; // previous mare
	INT32 maxlink; // maximum link obtained
	UINT8 texttimer; // nights_texttime should not be local
	UINT8 textvar; // which line of NiGHTS text to show -- let's not use cheap hacks

	INT16 lastsidehit, lastlinehit;

	tic_t losstime;
	UINT8 timeshit; // That's TIMES HIT, not TIME SHIT, you doofus!

	INT32 onconveyor; // You are on a conveyor belt if nonzero

	mobj_t *awayviewmobj;
	INT32 awayviewtics;
	angle_t awayviewaiming; // Used for cut-away view

	boolean spectator;
	UINT8 bot;

	tic_t jointime; // Timer when player joins game to change skin/color
#ifdef HWRENDER
	fixed_t fovadd; // adjust FOV for hw rendering
#endif

	INT32 chargei;      // SRB2kart 16/02/15  ++ All lines below until end of enum

	INT32 starpostwp;
	INT32 position;
	INT32 heldDir;      // Held dir of controls; 1 = forward, 0 = none, -1 = backward
	INT32 turning;
	INT32 newfloorz;
	INT32 lakitu;

	INT32 spinout;      // Seperate timer from pw_bananacam so that you can't get stuck spinning
	UINT8 spinouttype;  // The type of spinout you are currently suffering, 0 = slipping forwards, 1 = completely still

	UINT8 boosting;     // if you're currently shroomboosting

} player_t;

#endif

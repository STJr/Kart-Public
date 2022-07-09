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
	SF_HIRES = 1, // Draw the sprite 2x as small?
} skinflags_t;

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

	// SRB2Kart: Spectator that wants to join
	PF_WANTSTOJOIN = 1<<11,

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

	NUMPOWERS
} powertype_t;

typedef enum
{
	KITEM_SAD = -1,
	KITEM_NONE = 0,
	KITEM_SNEAKER,
	KITEM_ROCKETSNEAKER,
	KITEM_INVINCIBILITY,
	KITEM_BANANA,
	KITEM_EGGMAN,
	KITEM_ORBINAUT,
	KITEM_JAWZ,
	KITEM_MINE,
	KITEM_BALLHOG,
	KITEM_SPB,
	KITEM_GROW,
	KITEM_SHRINK,
	KITEM_THUNDERSHIELD,
	KITEM_HYUDORO,
	KITEM_POGOSPRING,
	KITEM_KITCHENSINK,

	NUMKARTITEMS,

	// Additional roulette numbers, only used for K_KartGetItemResult
	KRITEM_TRIPLESNEAKER = NUMKARTITEMS,
	KRITEM_TRIPLEBANANA,
	KRITEM_TENFOLDBANANA,
	KRITEM_TRIPLEORBINAUT,
	KRITEM_QUADORBINAUT,
	KRITEM_DUALJAWZ,

	NUMKARTRESULTS
} kartitems_t;

//{ SRB2kart - kartstuff
typedef enum
{
	// Basic gameplay things
	k_position,			// Used for Kart positions, mostly for deterministic stuff
	k_oldposition,		// Used for taunting when you pass someone
	k_positiondelay,	// Used for position number, so it can grow when passing/being passed
	k_prevcheck,		// Previous checkpoint distance; for p_user.c (was "pw_pcd")
	k_nextcheck,		// Next checkpoint distance; for p_user.c (was "pw_ncd")
	k_waypoint,			// Waypoints.
	k_starpostwp,		// Temporarily stores player waypoint for... some reason. Used when respawning and finishing.
	k_starpostflip,		// the last starpost we hit requires flipping?
	k_respawn,			// Timer for the DEZ laser respawn effect
	k_dropdash,			// Charge up for respawn Drop Dash

	k_throwdir, 		// Held dir of controls; 1 = forward, 0 = none, -1 = backward (was "player->heldDir")
	k_lapanimation,		// Used to show the lap start wing logo animation
	k_laphand,			// Lap hand gfx to use; 0 = none, 1 = :ok_hand:, 2 = :thumbs_up:, 3 = :thumps_down:
	k_cardanimation,	// Used to determine the position of some full-screen Battle Mode graphics
	k_voices,			// Used to stop the player saying more voices than it should
	k_tauntvoices,		// Used to specifically stop taunt voice spam
	k_instashield,		// Instashield no-damage animation timer
	k_enginesnd,		// Engine sound number you're on.

	k_floorboost,		// Prevents Sneaker sounds for a breif duration when triggered by a floor panel
	k_spinouttype,		// Determines whether to thrust forward or not while spinning out; 0 = move forwards, 1 = stay still

	k_drift,			// Drifting Left or Right, plus a bigger counter = sharper turn
	k_driftend,			// Drift has ended, used to adjust character angle after drift
	k_driftcharge,		// Charge your drift so you can release a burst of speed
	k_driftboost,		// Boost you get from drifting
	k_boostcharge,		// Charge-up for boosting at the start of the race
	k_startboost,		// Boost you get from start of race or respawn drop dash
	k_jmp,				// In Mario Kart, letting go of the jump button stops the drift
	k_offroad,			// In Super Mario Kart, going offroad has lee-way of about 1 second before you start losing speed
	k_pogospring,		// Pogo spring bounce effect
	k_brakestop,		// Wait until you've made a complete stop for a few tics before letting brake go in reverse.
	k_waterskip,		// Water skipping counter
	k_dashpadcooldown,	// Separate the vanilla SA-style dash pads from using pw_flashing
	k_boostpower,		// Base boost value, for offroad
	k_speedboost,		// Boost value smoothing for max speed
	k_accelboost,		// Boost value smoothing for acceleration
	k_boostangle,		// angle set when not spun out OR boosted to determine what direction you should keep going at if you're spun out and boosted.
	k_boostcam,			// Camera push forward on boost
	k_destboostcam,		// Ditto
	k_timeovercam,		// Camera timer for leaving behind or not
	k_aizdriftstrat,	// Let go of your drift while boosting? Helper for the SICK STRATZ you have just unlocked
	k_brakedrift,		// Helper for brake-drift spark spawning

	k_itemroulette,		// Used for the roulette when deciding what item to give you (was "pw_kartitem")
	k_roulettetype,		// Used for the roulette, for deciding type (currently only used for Battle, to give you better items from Karma items)

	// Item held stuff
	k_itemtype,		// KITEM_ constant for item number
	k_itemamount,	// Amount of said item
	k_itemheld,		// Are you holding an item?

	// Some items use timers for their duration or effects
	//k_thunderanim,			// Duration of Thunder Shield's use animation
	k_curshield,			// 0 = no shield, 1 = thunder shield
	k_hyudorotimer,			// Duration of the Hyudoro offroad effect itself
	k_stealingtimer,		// You are stealing an item, this is your timer
	k_stolentimer,			// You are being stolen from, this is your timer
	k_sneakertimer,			// Duration of the Sneaker Boost itself
	k_growshrinktimer,		// > 0 = Big, < 0 = small
	k_squishedtimer,		// Squished frame timer
	k_rocketsneakertimer,	// Rocket Sneaker duration timer
	k_invincibilitytimer,	// Invincibility timer
	k_eggmanheld,			// Eggman monitor held, separate from k_itemheld so it doesn't stop you from getting items
	k_eggmanexplode,		// Fake item recieved, explode in a few seconds
	k_eggmanblame,			// Fake item recieved, who set this fake
	k_lastjawztarget,		// Last person you target with jawz, for playing the target switch sfx
	k_bananadrag,			// After a second of holding a banana behind you, you start to slow down
	k_spinouttimer,			// Spin-out from a banana peel or oil slick (was "pw_bananacam")
	k_wipeoutslow,			// Timer before you slowdown when getting wiped out
	k_justbumped,			// Prevent players from endlessly bumping into each other
	k_comebacktimer,		// Battle mode, how long before you become a bomb after death
	k_sadtimer,				// How long you've been sad

	// Battle Mode vars
	k_bumper,			// Number of bumpers left
	k_comebackpoints,	// Number of times you've bombed or gave an item to someone; once it's 3 it gets set back to 0 and you're given a bumper
	k_comebackmode, 	// 0 = bomb, 1 = item
	k_wanted, 			// Timer for determining WANTED status, lowers when hitting people, prevents the game turning into Camp Lazlo
	k_yougotem, 		// "You Got Em" gfx when hitting someone as a karma player via a method that gets you back in the game instantly

	// v1.0.2+ vars
	k_itemblink,		// Item flashing after roulette, prevents Hyudoro stealing AND serves as a mashing indicator
	k_itemblinkmode,	// Type of flashing: 0 = white (normal), 1 = red (mashing), 2 = rainbow (enhanced items)
	k_getsparks,		// Disable drift sparks at low speed, JUST enough to give acceleration the actual headstart above speed
	k_jawztargetdelay,	// Delay for Jawz target switching, to make it less twitchy
	k_spectatewait,		// How long have you been waiting as a spectator
	k_growcancel,		// Hold the item button down to cancel Grow

	NUMKARTSTUFF
} kartstufftype_t;
//}

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
	//fixed_t deltaviewheight;
	// bounded/scaled total momentum.
	//fixed_t bob;

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

	// SRB2kart stuff
	INT32 kartstuff[NUMKARTSTUFF];
	angle_t frameangle; // for the player add the ability to have the sprite only face other angles
	angle_t old_frameangle, old_frameangle2;
	INT16 lturn_max[MAXPREDICTTICS]; // What's the expected turn value for full-left for a number of frames back (to account for netgame latency)?
	INT16 rturn_max[MAXPREDICTTICS]; // Ditto but for full-right

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

	// SRB2kart
	UINT8 kartspeed; // Kart speed stat between 1 and 9
	UINT8 kartweight; // Kart weight stat between 1 and 9
	//

	UINT32 charflags; // Extra abilities/settings for skins (combinable stuff)
	                 // See SF_ flags
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
	UINT32 marescore; // SRB2Kart: Battle score
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
	tic_t spectatorreentry;

	tic_t grieftime;
	UINT8 griefstrikes;

	UINT8 splitscreenindex;
#ifdef HWRENDER
	fixed_t fovadd; // adjust FOV for hw rendering
#endif
} player_t;

#endif

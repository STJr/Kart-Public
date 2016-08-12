// SONIC ROBO BLAST 2 KART ~ ZarroTsu
//-----------------------------------------------------------------------------
/// \file  k_kart.h
/// \brief SRB2kart stuff.

#ifndef __K_KART__
#define __K_KART__

extern const char *KartColor_Names[MAXSKINCOLORS];
void K_GenerateKartColormap(UINT8 *dest_colormap, INT32 skinnum, UINT8 color);
UINT8 K_GetKartColorByName(const char *name);

//{ SRB2kart Player structure - kartstuff

typedef enum
{
	// Basic gameplay things
	k_position,			// Used for Kart positions, mostly for deterministic stuff
	k_playerahead,		// Is someone ahead of me or not?
	k_oldposition,		// Used for taunting when you pass someone
	k_prevcheck,		// Previous checkpoint distance; for p_user.c (was "pw_pcd")
	k_nextcheck,		// Next checkpoint distance; for p_user.c (was "pw_ncd")
	k_waypoint,			// Waypoints.
	k_starpostwp,		// Temporarily stores player waypoint for... some reason. Used when respawning and finishing.
	
	k_throwdir, 		// Held dir of controls; 1 = forward, 0 = none, -1 = backward (was "player->heldDir")
	k_turndir,			// Turn direction for drifting; -1 = Left, 1 = Right, 0 = none
	k_sounds,			// Used this to avoid sounds being played every tic
	
	k_boosting,			// Determines if you're currently shroom-boosting to change how drifting works
	k_spinout,			// Separate confirmation to prevent endless wipeout loops
	k_spinouttype,		// Determines whether to thrust forward or not while spinning out; 0 = move forwards, 1 = stay still
	
	k_drift,			// Drifting Left or Right, plus a bigger counter = sharper turn
	k_driftcharge,		// Charge your drift so you can release a burst of speed
	k_jmp,				// In Mario Kart, letting go of the jump button stops the drift
	k_lakitu,			// > 0 = Lakitu fishing, < 0 = Lakitu lap counter (was "player->airtime") // NOTE: Check for ->lakitu, replace with this
	
	k_itemroulette,		// Used for the roulette when deciding what item to give you (was "pw_kartitem")
	k_itemslot,			// If you have X item, and kartitem chose X too, save it
	k_itemclose,		// Used to animate the item window closing (was "pw_psychic")

	// Some items use timers for their duration or effects
	k_magnettimer,		// Duration of Magnet's item-break and item box pull
	k_bootaketimer,		// You are stealing an item, this is your timer
	k_boostolentimer,	// You are being stolen from, this is your timer
	k_mushroomtimer,	// Duration of the Mushroom Boost itself
	k_growshrinktimer,	// > 0 = Big, < 0 = small
	k_squishedtimer,	// Squished frame timer
	k_goldshroomtimer,	// Gold Mushroom duration timer
	k_startimer,		// Invincibility timer
	k_spinouttimer,		// Wipe-out from a banana peel or oil slick (was "pw_bananacam")
	k_laserwisptimer,	// The duration and relative angle of the laser
	k_fireflowertimer,	// Duration of Fire Flower

	// Each item needs its own power slot, for the HUD and held use
	k_magnet,			// 0x1 = Magnet in inventory
	k_boo,				// 0x1 = Boo in inventory
	k_mushroom,			// 0x1 = 1 Mushroom in inventory, 0x2 = 2 Mushrooms in inventory
						// 0x4 = 3 Mushrooms in inventory
	k_megashroom,		// 0x1 = Mega Mushroom in inventory
	k_goldshroom,		// 0x1 = Gold Mushroom in inventory
	k_star,				// 0x1 = Star in inventory
	k_triplebanana,		// 0x1 = 1 Banana following, 0x2 = 2 Bananas following
						// 0x4 = 3 Bananas following, 0x8 = Triple Banana in inventory
	k_fakeitem,			// 0x1 = Fake Item being held, 0x2 = Fake Item in inventory
	k_banana,			// 0x1 = Banana being held, 0x2 = Banana in inventory
	k_greenshell,		// 0x1 = Green Shell being held, 0x2 = Green Shell in inventory
	k_redshell,			// 0x1 = Red Shell being held, 0x2 = Red Shell in inventory
	k_laserwisp,		// 0x1 = Laser Wisp in inventory
	k_triplegreenshell,	// 0x1 = 1 Green Shell orbiting, 0x2 = 2 Green Shells orbiting
						// 0x4 = 3 Green Shells orbiting, 0x8 = Triple Green Shell in inventory
	k_bobomb,			// 0x1 = Bob-omb being held, 0x2 = Bob-omb in inventory
	k_blueshell,		// 0x1 = Blue Shell in inventory
	k_jaws,				// 0x1 = 1 Jaws orbiting, 0x2 = 2 Jaws orbiting, 
						// 0x4 = 2x Jaws in inventory
	k_fireflower,		// 0x1 = Fire Flower in inventory
	k_tripleredshell,	// 0x1 = 1 Red Shell orbiting, 0x2 = 2 Red Shells orbiting
						// 0x4 = 3 Red Shells orbiting, 0x8 = Triple Red Shell in inventory
	k_lightning,		// 0x1 = Lightning in inventory

	NUMKARTSTUFF
} kartstufftype_t;

//}

// =========================================================================
#endif  // __K_KART__

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
/// \file  info.h
/// \brief Thing frame/state LUT

#ifndef __INFO__
#define __INFO__

// Needed for action function pointer handling.
#include "d_think.h"
#include "sounds.h"
#include "m_fixed.h"

// dehacked.c now has lists for the more named enums! PLEASE keep them up to date!
// For great modding!!

// IMPORTANT NOTE: If you add/remove from this list of action
// functions, don't forget to update them in dehacked.c!
void A_Explode();
void A_Pain();
void A_Fall();
void A_MonitorPop();
void A_Look();
void A_Chase();
void A_FaceStabChase();
void A_FaceTarget();
void A_FaceTracer();
void A_Scream();
void A_BossDeath();
void A_CustomPower(); // Use this for a custom power
void A_GiveWeapon(); // Gives the player weapon(s)
void A_JumpShield(); // Obtained Jump Shield
void A_RingShield(); // Obtained Ring Shield
void A_RingBox(); // Obtained Ring Box Tails
void A_Invincibility(); // Obtained Invincibility Box
void A_SuperSneakers(); // Obtained Super Sneakers Box
void A_BunnyHop(); // have bunny hop tails
void A_BubbleSpawn(); // Randomly spawn bubbles
void A_FanBubbleSpawn();
void A_BubbleRise(); // Bubbles float to surface
void A_BubbleCheck(); // Don't draw if not underwater
void A_AwardScore();
void A_ExtraLife(); // Extra Life
void A_BombShield(); // Obtained Bomb Shield
void A_WaterShield(); // Obtained Water Shield
void A_ForceShield(); // Obtained Force Shield
void A_PityShield(); // Obtained Pity Shield. We're... sorry.
void A_GravityBox();
void A_ScoreRise(); // Rise the score logo
void A_ParticleSpawn();
void A_AttractChase(); // Ring Chase
void A_DropMine(); // Drop Mine from Skim or Jetty-Syn Bomber
void A_FishJump(); // Fish Jump
void A_ThrownRing(); // Sparkle trail for red ring
void A_GrenadeRing(); // SRB2kart
void A_SetSolidSteam();
void A_UnsetSolidSteam();
void A_SignPlayer();
void A_OverlayThink();
void A_JetChase();
void A_JetbThink(); // Jetty-Syn Bomber Thinker
void A_JetgThink(); // Jetty-Syn Gunner Thinker
void A_JetgShoot(); // Jetty-Syn Shoot Function
void A_ShootBullet(); // JetgShoot without reactiontime setting
void A_MinusDigging();
void A_MinusPopup();
void A_MinusCheck();
void A_ChickenCheck();
void A_MouseThink(); // Mouse Thinker
void A_DetonChase(); // Deton Chaser
void A_CapeChase(); // Fake little Super Sonic cape
void A_RotateSpikeBall(); // Spike ball rotation
void A_SlingAppear();
void A_MaceRotate();
void A_UnidusBall();
void A_RockSpawn();
void A_SetFuse();
void A_CrawlaCommanderThink(); // Crawla Commander
void A_SmokeTrailer();
void A_RingExplode();
void A_OldRingExplode();
void A_MixUp();
void A_RecyclePowers();
void A_BossScream();
void A_Boss2TakeDamage();
void A_GoopSplat();
void A_Boss2PogoSFX();
void A_Boss2PogoTarget();
void A_EggmanBox();
void A_TurretFire();
void A_SuperTurretFire();
void A_TurretStop();
void A_JetJawRoam();
void A_JetJawChomp();
void A_PointyThink();
void A_CheckBuddy();
void A_HoodThink();
void A_ArrowCheck();
void A_SnailerThink();
void A_SharpChase();
void A_SharpSpin();
void A_VultureVtol();
void A_VultureCheck();
void A_SkimChase();
void A_SkullAttack();
void A_LobShot();
void A_FireShot();
void A_SuperFireShot();
void A_BossFireShot();
void A_Boss7FireMissiles();
void A_Boss1Laser();
void A_FocusTarget();
void A_Boss4Reverse();
void A_Boss4SpeedUp();
void A_Boss4Raise();
void A_SparkFollow();
void A_BuzzFly();
void A_GuardChase();
void A_EggShield();
void A_SetReactionTime();
void A_Boss1Spikeballs();
void A_Boss3TakeDamage();
void A_Boss3Path();
void A_LinedefExecute();
void A_PlaySeeSound();
void A_PlayAttackSound();
void A_PlayActiveSound();
void A_1upThinker();
void A_BossZoom(); //Unused
void A_Boss1Chase();
void A_Boss2Chase();
void A_Boss2Pogo();
void A_Boss7Chase();
void A_BossJetFume();
void A_SpawnObjectAbsolute();
void A_SpawnObjectRelative();
void A_ChangeAngleRelative();
void A_ChangeAngleAbsolute();
void A_PlaySound();
void A_FindTarget();
void A_FindTracer();
void A_SetTics();
void A_SetRandomTics();
void A_ChangeColorRelative();
void A_ChangeColorAbsolute();
void A_MoveRelative();
void A_MoveAbsolute();
void A_Thrust();
void A_ZThrust();
void A_SetTargetsTarget();
void A_SetObjectFlags();
void A_SetObjectFlags2();
void A_RandomState();
void A_RandomStateRange();
void A_DualAction();
void A_RemoteAction();
void A_ToggleFlameJet();
void A_ItemPop(); // SRB2kart
void A_JawzChase(); // SRB2kart
void A_JawzExplode(); // SRB2kart
void A_SPBChase(); // SRB2kart
void A_MineExplode(); // SRB2kart
void A_BallhogExplode(); // SRB2kart
void A_LightningFollowPlayer();	// SRB2kart: Lightning shield effect player chasing
void A_FZBoomFlash(); // SRB2kart
void A_FZBoomSmoke(); // SRB2kart
void A_RandomShadowFrame();	//SRB2kart: Shadow spawner frame randomizer
void A_RoamingShadowThinker();	// SRB2kart: Roaming Shadow moving + attacking players.
void A_MayonakaArrow();	//SRB2kart: midnight channel arrow sign
void A_ReaperThinker();	//SRB2kart: mementos reaper
void A_MementosTPParticles();	//SRB2kart: mementos teleporter particles. Man that's a lot of actions for my shite.
void A_FlameParticle(); // SRB2kart
void A_OrbitNights();
void A_GhostMe();
void A_SetObjectState();
void A_SetObjectTypeState();
void A_KnockBack();
void A_PushAway();
void A_RingDrain();
void A_SplitShot();
void A_MissileSplit();
void A_MultiShot();
void A_InstaLoop();
void A_Custom3DRotate();
void A_SearchForPlayers();
void A_CheckRandom();
void A_CheckTargetRings();
void A_CheckRings();
void A_CheckTotalRings();
void A_CheckHealth();
void A_CheckRange();
void A_CheckHeight();
void A_CheckTrueRange();
void A_CheckThingCount();
void A_CheckAmbush();
void A_CheckCustomValue();
void A_CheckCusValMemo();
void A_SetCustomValue();
void A_UseCusValMemo();
void A_RelayCustomValue();
void A_CusValAction();
void A_ForceStop();
void A_ForceWin();
void A_SpikeRetract();
void A_InfoState();
void A_Repeat();
void A_SetScale();
void A_RemoteDamage();
void A_HomingChase();
void A_TrapShot();
void A_VileTarget();
void A_VileAttack();
void A_VileFire();
void A_BrakChase();
void A_BrakFireShot();
void A_BrakLobShot();
void A_NapalmScatter();
void A_SpawnFreshCopy();

// ratio of states to sprites to mobj types is roughly 6 : 1 : 1
#define NUMMOBJFREESLOTS 512
#define NUMSPRITEFREESLOTS NUMMOBJFREESLOTS
#define NUMSTATEFREESLOTS (NUMMOBJFREESLOTS*8)

// Hey, moron! If you change this table, don't forget about sprnames in info.c and the sprite lights in hw_light.c!
typedef enum sprite
{
	SPR_NULL, // invisible object
	SPR_UNKN,

	SPR_THOK, // Thok! mobj
	SPR_PLAY,

	// Enemies
	SPR_POSS,
	SPR_SPOS,
	SPR_FISH, // Greenflower Fish
	SPR_BUZZ, // Buzz (Gold)
	SPR_RBUZ, // Buzz (Red)
	SPR_JETB, // Jetty-Syn Bomber
	SPR_JETW, // Jetty-Syn Water Bomber
	SPR_JETG, // Jetty-Syn Gunner
	SPR_CCOM, // Crawla Commander
	SPR_DETN, // Deton
	SPR_SKIM, // Skim mine dropper
	SPR_TRET,
	SPR_TURR, // Pop-Up Turret
	SPR_SHRP, // Sharp
	SPR_JJAW, // Jet Jaw
	SPR_SNLR, // Snailer
	SPR_VLTR, // Vulture
	SPR_PNTY, // Pointy
	SPR_ARCH, // Robo-Hood
	SPR_CBFS, // CastleBot FaceStabber (Egg Knight?)
	SPR_SPSH, // Egg Guard
	SPR_ESHI, // Egg Shield for Egg Guard
	SPR_GSNP, // Green Snapper
	SPR_MNUS, // Minus
	SPR_SSHL, // Spring Shell
	SPR_UNID, // Unidus
	SPR_BBUZ, // AquaBuzz, for Azure Temple

	// Generic Boss Items
	SPR_JETF, // Boss jet fumes

	// Boss 1 (Greenflower)
	SPR_EGGM,

	// Boss 2 (Techno Hill)
	SPR_EGGN, // Boss 2
	SPR_TNKA, // Boss 2 Tank 1
	SPR_TNKB, // Boss 2 Tank 2
	SPR_SPNK, // Boss 2 Spigot
	SPR_GOOP, // Boss 2 Goop

	// Boss 3 (Deep Sea)
	SPR_EGGO, // Boss 3
	SPR_PRPL, // Boss 3 Propeller
	SPR_FAKE, // Boss 3 Fakemobile

	// Boss 4 (Castle Eggman)
	SPR_EGGP,
	SPR_EFIR, // Boss 4 jet flame

	// Boss 5 (Arid Canyon)
	SPR_EGGQ,

	// Boss 6 (Red Volcano)
	SPR_EGGR,

	// Boss 7 (Dark City)
	SPR_BRAK,
	SPR_BGOO, // Goop
	SPR_BMSL,

	// Boss 8 (Egg Rock)
	SPR_EGGT,

	// Cy-Brak-Demon; uses SPR_BRAK as well, but has some extras
	SPR_RCKT, // Rockets!
	SPR_ELEC, // Electricity!
	SPR_TARG, // Targeting reticules!
	SPR_NPLM, // Big napalm bombs!
	SPR_MNPL, // Mini napalm bombs!

	// Metal Sonic
	SPR_METL,
	SPR_MSCF,
	SPR_MSCB,

	// Collectible Items
	SPR_RING,
	SPR_TRNG, // Team Rings
	SPR_EMMY, // emerald test
	SPR_TOKE, // Special Stage Token
	SPR_RFLG, // Red CTF Flag
	SPR_BFLG, // Blue CTF Flag
	SPR_NWNG, // NiGHTS Wing collectable item.
	SPR_EMBM, // Emblem
	SPR_CEMG, // Chaos Emeralds
	SPR_EMER, // Emerald Hunt

	// Interactive Objects
	SPR_FANS,
	SPR_BUBL, // water bubble source
	SPR_SIGN, // Level end sign
	SPR_STEM, // Steam riser
	SPR_SPIK, // Spike Ball
	SPR_SFLM, // Spin fire
	SPR_USPK, // Floor spike
	SPR_STPT, // Starpost
	SPR_BMNE, // Big floating mine

	// Monitor Boxes
	SPR_SRBX,
	SPR_RRBX,
	SPR_BRBX,
	SPR_SHTV,
	SPR_PINV,
	SPR_YLTV,
	SPR_BLTV, // Force shield
	SPR_BKTV, // Bomb shield TV
	SPR_WHTV, // Jump shield TV
	SPR_GRTV, // Pity shield TV
	SPR_ELTV, // Elemental shield TV
	SPR_EGGB, // Eggman box
	SPR_MIXU, // Player mixing monitor
	SPR_RECY, // Recycler (power mixing) monitor
	SPR_QUES, // Random monitor
	SPR_GBTV, // Gravity boots TV
	SPR_PRUP, // 1up
	SPR_PTTV, // Score TVs

	// Monitor Miscellany
	SPR_MTEX, // Exploding monitor

	// Projectiles
	SPR_MISL,
	SPR_TORP, // Torpedo
	SPR_ENRG, // Energy ball
	SPR_MINE, // Skim mine
	SPR_JBUL, // Jetty-Syn Bullet
	SPR_TRLS,
	SPR_CBLL, // Cannonball
	SPR_AROW, // Arrow
	SPR_CFIR, // Colored fire of various sorts

	// Greenflower Scenery
	SPR_FWR1,
	SPR_FWR2, // GFZ Sunflower
	SPR_FWR3, // GFZ budding flower
	SPR_FWR4,
	SPR_BUS1, // GFZ Bush w/ berries
	SPR_BUS2, // GFZ Bush w/o berries

	// Techno Hill Scenery
	SPR_THZP, // Techno Hill Zone Plant
	SPR_ALRM, // THZ2 Alarm

	// Deep Sea Scenery
	SPR_GARG, // Deep Sea Gargoyle
	SPR_SEWE, // Deep Sea Seaweed
	SPR_DRIP, // Dripping water
	SPR_CRL1, // Coral 1
	SPR_CRL2, // Coral 2
	SPR_CRL3, // Coral 3
	SPR_BCRY, // Blue Crystal

	// Castle Eggman Scenery
	SPR_CHAN, // CEZ Chain
	SPR_FLAM, // Flame
	SPR_ESTA, // Eggman esta una estatua!
	SPR_SMCH, // Small Mace Chain
	SPR_BMCH, // Big Mace Chain
	SPR_SMCE, // Small Mace
	SPR_BMCE, // Big Mace

	// Arid Canyon Scenery
	SPR_BTBL, // Big tumbleweed
	SPR_STBL, // Small tumbleweed
	SPR_CACT, // Cacti sprites

	// Red Volcano Scenery
	SPR_FLME, // Flame jet
	SPR_DFLM, // Blade's flame

	// Dark City Scenery

	// Egg Rock Scenery

	// Christmas Scenery
	SPR_XMS1,
	SPR_XMS2,
	SPR_XMS3,

	// Botanic Serenity Scenery
	SPR_BSZ1, // Tall flowers
	SPR_BSZ2, // Medium flowers
	SPR_BSZ3, // Small flowers
	SPR_BSZ4, // Tulip
	SPR_BSZ5, // Cluster of Tulips
	SPR_BSZ6, // Bush
	SPR_BSZ7, // Vine
	SPR_BSZ8, // Misc things

	// Misc Scenery
	SPR_STLG, // Stalagmites
	SPR_DBAL, // Disco
	SPR_RCRY, // ATZ Red Crystal (Target)

	// Powerup Indicators
	SPR_ARMA, // Armageddon Shield Orb
	SPR_ARMF, // Armageddon Shield Ring, Front
	SPR_ARMB, // Armageddon Shield Ring, Back
	SPR_WIND, // Whirlwind Shield Orb
	SPR_MAGN, // Attract Shield Orb
	SPR_ELEM, // Elemental Shield Orb and Fire
	SPR_FORC, // Force Shield Orb
	SPR_PITY, // Pity Shield Orb
	SPR_IVSP, // invincibility sparkles
	SPR_SSPK, // Super Sonic Spark

	SPR_GOAL, // Special Stage goal (here because lol NiGHTS)

	// Freed Animals
	SPR_BIRD, // Birdie freed!
	SPR_BUNY, // Bunny freed!
	SPR_MOUS, // Mouse
	SPR_CHIC, // Chicken
	SPR_COWZ, // Cow
	SPR_RBRD, // Red Birdie in Bubble

	// Springs
	SPR_SPRY, // yellow spring
	SPR_SPRR, // red spring
	SPR_SPRB, // Blue springs
	SPR_YSPR, // Yellow Diagonal Spring
	SPR_RSPR, // Red Diagonal Spring

	// Environmental Effects
	SPR_RAIN, // Rain
	SPR_SNO1, // Snowflake
	SPR_SPLH, // Water Splish
	SPR_SPLA, // Water Splash
	SPR_SMOK,
	SPR_BUBP, // Small bubble
	SPR_BUBO, // Medium bubble
	SPR_BUBN, // Large bubble
	SPR_BUBM, // Extra Large (would you like fries with that?) bubble
	SPR_POPP, // Extra Large bubble goes POP!
	SPR_TFOG, // Teleport Fog
	SPR_SEED, // Sonic CD flower seed
	SPR_PRTL, // Particle (for fans, etc.)

	// Game Indicators
	SPR_SCOR, // Score logo
	SPR_DRWN, // Drowning Timer
	SPR_TTAG, // Tag Sign
	SPR_GFLG, // Got Flag sign

	// Ring Weapons
	SPR_RRNG, // Red Ring
	SPR_RNGB, // Bounce Ring
	SPR_RNGR, // Rail Ring
	SPR_RNGI, // Infinity Ring
	SPR_RNGA, // Automatic Ring
	SPR_RNGE, // Explosion Ring
	SPR_RNGS, // Scatter Ring
	SPR_RNGG, // Grenade Ring

	SPR_PIKB, // Bounce Ring Pickup
	SPR_PIKR, // Rail Ring Pickup
	SPR_PIKA, // Automatic Ring Pickup
	SPR_PIKE, // Explosion Ring Pickup
	SPR_PIKS, // Scatter Ring Pickup
	SPR_PIKG, // Grenade Ring Pickup

	SPR_TAUT, // Thrown Automatic Ring
	SPR_TGRE, // Thrown Grenade Ring
	SPR_TSCR, // Thrown Scatter Ring

	// Mario-specific stuff
	SPR_COIN,
	SPR_CPRK,
	SPR_GOOM,
	SPR_BGOM,
	SPR_FFWR,
	SPR_FBLL,
	SPR_SHLL,
	SPR_PUMA,
	SPR_HAMM,
	SPR_KOOP,
	SPR_BFLM,
	SPR_MAXE,
	SPR_MUS1,
	SPR_MUS2,
	SPR_TOAD,

	// NiGHTS Stuff
	SPR_NDRN, // NiGHTS drone
	SPR_SUPE, // NiGHTS character flying
	SPR_SUPZ, // NiGHTS hurt
	SPR_NDRL, // NiGHTS character drilling
	SPR_NSPK, // NiGHTS sparkle
	SPR_NBMP, // NiGHTS Bumper
	SPR_HOOP, // NiGHTS hoop sprite
	SPR_NSCR, // NiGHTS score sprite
	SPR_NPRU, // Nights Powerups
	SPR_CAPS, // Capsule thingy for NiGHTS
	SPR_SUPT, // Super Sonic Transformation (NiGHTS)

	// Debris
	SPR_SPRK, // spark
	SPR_BOM1, // Robot Explosion
	SPR_BOM2, // Boss Explosion 1
	SPR_BOM3, // Boss Explosion 2
	SPR_BOM4, // Underwater Explosion

	// Crumbly rocks
	SPR_ROIA,
	SPR_ROIB,
	SPR_ROIC,
	SPR_ROID,
	SPR_ROIE,
	SPR_ROIF,
	SPR_ROIG,
	SPR_ROIH,
	SPR_ROII,
	SPR_ROIJ,
	SPR_ROIK,
	SPR_ROIL,
	SPR_ROIM,
	SPR_ROIN,
	SPR_ROIO,
	SPR_ROIP,

	// Blue Spheres
	SPR_BBAL,

	// Gravity Well Objects
	SPR_GWLG,
	SPR_GWLR,

	// SRB1 Sprites
	SPR_SRBA,
	SPR_SRBB,
	SPR_SRBC,
	SPR_SRBD,
	SPR_SRBE,
	SPR_SRBF,
	SPR_SRBG,
	SPR_SRBH,
	SPR_SRBI,
	SPR_SRBJ,
	SPR_SRBK,
	SPR_SRBL,
	SPR_SRBM,
	SPR_SRBN,
	SPR_SRBO,

	// Springs
	SPR_SPRG, // Gray Spring
	SPR_BSPR, // Blue Diagonal Spring

	SPR_RNDM, // Random Item Box
	SPR_RPOP, // Random Item Box Pop
	SPR_SGNS, // Signpost sparkle
	SPR_FAST, // Speed boost trail
	SPR_DSHR, // Speed boost dust release
	SPR_BOST, // Sneaker booster flame
	SPR_BOSM, // Sneaker booster smoke
	SPR_KFRE, // Sneaker fire trail
	SPR_KINV, // Invincibility sparkle trail
	SPR_KINF, // Invincibility flash
	SPR_WIPD, // Wipeout dust trail
	SPR_DRIF, // Drift Sparks
	SPR_BDRF, // Brake drift sparks
	SPR_DUST, // Drift Dust

	// Kart Items
	SPR_RSHE, // Rocket sneaker
	SPR_FITM, // Eggman Monitor
	SPR_BANA, // Banana Peel
	SPR_ORBN, // Orbinaut
	SPR_JAWZ, // Jawz
	SPR_SSMN, // SS Mine
	SPR_KRBM, // SS Mine BOOM
	SPR_BHOG, // Ballhog
	SPR_BHBM, // Ballhog BOOM
	SPR_SPBM, // Self-Propelled Bomb
	SPR_THNS, // Thunder Shield
	SPR_SINK, // Kitchen Sink
	SPR_SITR, // Kitchen Sink Trail
	SPR_KBLN, // Battle Mode Bumper

	SPR_DEZL, // DEZ Laser respawn

	// Additional Kart Objects
	SPR_POKE, // Pokey
	SPR_AUDI, // Audience members
	SPR_DECO, // Old 1.0 Kart Decoratives + New misc ones
	SPR_DOOD, // All the old D00Dkart objects
	SPR_SNES, // Sprites for SNES remake maps
	SPR_GBAS, // Sprites for GBA remake maps
	SPR_SPRS, // Sapphire Coast Spring Shell
	SPR_BUZB, // Sapphire Coast Buzz Mk3
	SPR_CHOM, // Sapphire Coast Chomper
	SPR_SACO, // Sapphire Coast Fauna
	SPR_CRAB, // Crystal Abyss mobs
	SPR_SHAD, // TD shadows
	SPR_BRNG, // Chaotix Big Ring

	SPR_BUMP, // Player/shell bump
	SPR_FLEN, // Shell hit graphics stuff
	SPR_CLAS, // items clash
	SPR_PSHW, // thrown indicator
	SPR_ISTA, // instashield layer A
	SPR_ISTB, // instashield layer B

	SPR_ARRO, // player arrows
	SPR_ITEM,
	SPR_ITMO,
	SPR_ITMI,
	SPR_ITMN,
	SPR_WANT,

	SPR_PBOM, // player bomb

	SPR_HIT1, // battle points
	SPR_HIT2, // battle points
	SPR_HIT3, // battle points

	SPR_RETI, // player reticule

	SPR_AIDU,

	SPR_KSPK, // Spark radius for the lightning shield
	SPR_LZI1, // Lightning that falls on the player for lightning shield
	SPR_LZI2, // ditto
	SPR_KLIT, // You have a twisted mind. But this actually is for the diagonal lightning.

	SPR_FZSM, // F-Zero NO CONTEST explosion
	SPR_FZBM,
	SPR_FPRT,

	// Various plants
	SPR_SBUS,

	SPR_MARB, // Marble Zone sprites
	SPR_FUFO, // CD Special Stage UFO (don't ask me why it begins with an F)

	SPR_RUST, // Rusty Rig sprites

	SPR_BLON, // D2 Balloon Panic

	SPR_VAPE, // Volcanic Valley

	// Hill Top Zone
	SPR_HTZA,
	SPR_HTZB,

	// Ports of gardens
	SPR_SGVA,
	SPR_SGVB,
	SPR_SGVC,
	SPR_PGTR,
	SPR_PGF1,
	SPR_PGF2,
	SPR_PGF3,
	SPR_PGBH,
	SPR_DPLR,

	// Midnight Channel stuff:
	SPR_SPTL,	// Spotlight
	SPR_ENM1,	// Shadows (Roaming and static)
	SPR_GARU,	// Wind attack roaming shadows use.
	SPR_MARR,	// Mayonaka Arrow

	//Mementos stuff:
	SPR_REAP,

	SPR_JITB,	// Jack In The Box

	// Color Drive stuff:
	SPR_CDMO,
	SPR_CDBU,

	// Daytona Speedway
	SPR_PINE,

	// Egg Zeppelin
	SPR_PPLR,

	// Desert Palace
	SPR_DPPT,

	// Aurora Atoll
	SPR_AATR,
	SPR_COCO,

	// Barren Badlands
	SPR_BDST,
	SPR_FROG,
	SPR_CBRA,
	SPR_HOLE,
	SPR_BBRA,

	// Eerie Grove
	SPR_EGFG,

	// SMK ports
	SPR_SMKP,
	SPR_MTYM,
	SPR_THWP,
	SPR_SNOB,
	SPR_ICEB,

	// Ezo's maps - many single-use sprites!
	SPR_CNDL,
	SPR_DOCH,
	SPR_DUCK,
	SPR_GTRE,
	SPR_CHES,
	SPR_CHIM,
	SPR_DRGN,
	SPR_LZMN,
	SPR_PGSS,
	SPR_ZTCH,
	SPR_MKMA,
	SPR_MKMP,
	SPR_RTCH,
	SPR_BOWL,
	SPR_BOWH,
	SPR_BRRL,
	SPR_BRRR,
	SPR_HRSE,
	SPR_TOAH,
	SPR_BFRT,
	SPR_OFRT,
	SPR_RFRT,
	SPR_PFRT,
	SPR_ASPK,
	SPR_HBST,
	SPR_HBSO,
	SPR_HBSF,
	SPR_WBLZ,
	SPR_WBLN,

	SPR_FWRK,

	// Xmas-specific sprites that don't fit aboxe
	SPR_XMS4,
	SPR_XMS5,

	// First person view sprites; this is a sprite so that it can be replaced by a specialized MD2 draw later
	SPR_VIEW,

	SPR_FIRSTFREESLOT,
	SPR_LASTFREESLOT = SPR_FIRSTFREESLOT + NUMSPRITEFREESLOTS - 1,
	NUMSPRITES
} spritenum_t;

typedef enum state
{
	S_NULL,
	S_UNKNOWN,
	S_INVISIBLE, // state for invisible sprite

	S_SPAWNSTATE,
	S_SEESTATE,
	S_MELEESTATE,
	S_MISSILESTATE,
	S_DEATHSTATE,
	S_XDEATHSTATE,
	S_RAISESTATE,

	// Thok
	S_THOK,

	// SRB2kart Frames
	S_KART_STND1,
	S_KART_STND2,
	S_KART_STND1_L,
	S_KART_STND2_L,
	S_KART_STND1_R,
	S_KART_STND2_R,
	S_KART_WALK1,
	S_KART_WALK2,
	S_KART_WALK1_L,
	S_KART_WALK2_L,
	S_KART_WALK1_R,
	S_KART_WALK2_R,
	S_KART_RUN1,
	S_KART_RUN2,
	S_KART_RUN1_L,
	S_KART_RUN2_L,
	S_KART_RUN1_R,
	S_KART_RUN2_R,
	S_KART_DRIFT1_L,
	S_KART_DRIFT2_L,
	S_KART_DRIFT1_R,
	S_KART_DRIFT2_R,
	S_KART_SPIN,
	S_KART_PAIN,
	S_KART_SQUISH,
	/*
	S_PLAY_STND,
	S_PLAY_TAP1,
	S_PLAY_TAP2,
	S_PLAY_RUN1,
	S_PLAY_RUN2,
	S_PLAY_RUN3,
	S_PLAY_RUN4,
	S_PLAY_RUN5,
	S_PLAY_RUN6,
	S_PLAY_RUN7,
	S_PLAY_RUN8,
	S_PLAY_SPD1,
	S_PLAY_SPD2,
	S_PLAY_SPD3,
	S_PLAY_SPD4,
	S_PLAY_ATK1,
	S_PLAY_ATK2,
	S_PLAY_ATK3,
	S_PLAY_ATK4,
	S_PLAY_SPRING,
	S_PLAY_FALL1,
	S_PLAY_FALL2,
	S_PLAY_ABL1,
	S_PLAY_ABL2,
	S_PLAY_SPC1,
	S_PLAY_SPC2,
	S_PLAY_SPC3,
	S_PLAY_SPC4,
	S_PLAY_CLIMB1,
	S_PLAY_CLIMB2,
	S_PLAY_CLIMB3,
	S_PLAY_CLIMB4,
	S_PLAY_CLIMB5,
	S_PLAY_GASP,
	S_PLAY_PAIN,
	S_PLAY_DIE,
	S_PLAY_TEETER1,
	S_PLAY_TEETER2,
	S_PLAY_CARRY,
	S_PLAY_SUPERSTAND,
	S_PLAY_SUPERWALK1,
	S_PLAY_SUPERWALK2,
	S_PLAY_SUPERFLY1,
	S_PLAY_SUPERFLY2,
	S_PLAY_SUPERTEETER,
	S_PLAY_SUPERHIT,
	S_PLAY_SUPERTRANS1,
	S_PLAY_SUPERTRANS2,
	S_PLAY_SUPERTRANS3,
	S_PLAY_SUPERTRANS4,
	S_PLAY_SUPERTRANS5,
	S_PLAY_SUPERTRANS6,
	S_PLAY_SUPERTRANS7,
	S_PLAY_SUPERTRANS8,
	S_PLAY_SUPERTRANS9, // This has special significance in the code. If you add more frames, search for it and make the appropriate changes.
	*/

	// technically the player goes here but it's an infinite tic state
	S_OBJPLACE_DUMMY,

	// 1-Up Box Sprites overlay (uses player sprite)
	S_PLAY_BOX1,
	S_PLAY_BOX2,
	S_PLAY_ICON1,
	S_PLAY_ICON2,
	S_PLAY_ICON3,

	// Level end sign overlay (uses player sprite)
	S_PLAY_SIGN,

	// Blue Crawla
	S_POSS_STND,
	S_POSS_RUN1,
	S_POSS_RUN2,
	S_POSS_RUN3,
	S_POSS_RUN4,
	S_POSS_RUN5,
	S_POSS_RUN6,

	// Red Crawla
	S_SPOS_STND,
	S_SPOS_RUN1,
	S_SPOS_RUN2,
	S_SPOS_RUN3,
	S_SPOS_RUN4,
	S_SPOS_RUN5,
	S_SPOS_RUN6,

	// Greenflower Fish
	S_FISH1,
	S_FISH2,
	S_FISH3,
	S_FISH4,

	// Buzz (Gold)
	S_BUZZLOOK1,
	S_BUZZLOOK2,
	S_BUZZFLY1,
	S_BUZZFLY2,

	// Buzz (Red)
	S_RBUZZLOOK1,
	S_RBUZZLOOK2,
	S_RBUZZFLY1,
	S_RBUZZFLY2,

	// AquaBuzz
	S_BBUZZFLY1,
	S_BBUZZFLY2,

	// Jetty-Syn Bomber
	S_JETBLOOK1,
	S_JETBLOOK2,
	S_JETBZOOM1,
	S_JETBZOOM2,

	// Jetty-Syn Gunner
	S_JETGLOOK1,
	S_JETGLOOK2,
	S_JETGZOOM1,
	S_JETGZOOM2,
	S_JETGSHOOT1,
	S_JETGSHOOT2,

	// Crawla Commander
	S_CCOMMAND1,
	S_CCOMMAND2,
	S_CCOMMAND3,
	S_CCOMMAND4,

	// Deton
	S_DETON1,
	S_DETON2,
	S_DETON3,
	S_DETON4,
	S_DETON5,
	S_DETON6,
	S_DETON7,
	S_DETON8,
	S_DETON9,
	S_DETON10,
	S_DETON11,
	S_DETON12,
	S_DETON13,
	S_DETON14,
	S_DETON15,
	S_DETON16,

	// Skim Mine Dropper
	S_SKIM1,
	S_SKIM2,
	S_SKIM3,
	S_SKIM4,

	// THZ Turret
	S_TURRET,
	S_TURRETFIRE,
	S_TURRETSHOCK1,
	S_TURRETSHOCK2,
	S_TURRETSHOCK3,
	S_TURRETSHOCK4,
	S_TURRETSHOCK5,
	S_TURRETSHOCK6,
	S_TURRETSHOCK7,
	S_TURRETSHOCK8,
	S_TURRETSHOCK9,

	// Popup Turret
	S_TURRETLOOK,
	S_TURRETSEE,
	S_TURRETPOPUP1,
	S_TURRETPOPUP2,
	S_TURRETPOPUP3,
	S_TURRETPOPUP4,
	S_TURRETPOPUP5,
	S_TURRETPOPUP6,
	S_TURRETPOPUP7,
	S_TURRETPOPUP8,
	S_TURRETSHOOT,
	S_TURRETPOPDOWN1,
	S_TURRETPOPDOWN2,
	S_TURRETPOPDOWN3,
	S_TURRETPOPDOWN4,
	S_TURRETPOPDOWN5,
	S_TURRETPOPDOWN6,
	S_TURRETPOPDOWN7,
	S_TURRETPOPDOWN8,

	// Sharp
	S_SHARP_ROAM1,
	S_SHARP_ROAM2,
	S_SHARP_AIM1,
	S_SHARP_AIM2,
	S_SHARP_AIM3,
	S_SHARP_AIM4,
	S_SHARP_SPIN,

	// Jet Jaw
	S_JETJAW_ROAM1,
	S_JETJAW_ROAM2,
	S_JETJAW_ROAM3,
	S_JETJAW_ROAM4,
	S_JETJAW_ROAM5,
	S_JETJAW_ROAM6,
	S_JETJAW_ROAM7,
	S_JETJAW_ROAM8,
	S_JETJAW_CHOMP1,
	S_JETJAW_CHOMP2,
	S_JETJAW_CHOMP3,
	S_JETJAW_CHOMP4,
	S_JETJAW_CHOMP5,
	S_JETJAW_CHOMP6,
	S_JETJAW_CHOMP7,
	S_JETJAW_CHOMP8,
	S_JETJAW_CHOMP9,
	S_JETJAW_CHOMP10,
	S_JETJAW_CHOMP11,
	S_JETJAW_CHOMP12,
	S_JETJAW_CHOMP13,
	S_JETJAW_CHOMP14,
	S_JETJAW_CHOMP15,
	S_JETJAW_CHOMP16,

	// Snailer
	S_SNAILER1,

	// Vulture
	S_VULTURE_STND,
	S_VULTURE_VTOL1,
	S_VULTURE_VTOL2,
	S_VULTURE_VTOL3,
	S_VULTURE_VTOL4,
	S_VULTURE_ZOOM1,
	S_VULTURE_ZOOM2,
	S_VULTURE_ZOOM3,
	S_VULTURE_ZOOM4,
	S_VULTURE_ZOOM5,

	// Pointy
	S_POINTY1,
	S_POINTYBALL1,

	// Robo-Hood
	S_ROBOHOOD_LOOK,
	S_ROBOHOOD_STND,
	S_ROBOHOOD_SHOOT,
	S_ROBOHOOD_JUMP,
	S_ROBOHOOD_JUMP2,
	S_ROBOHOOD_FALL,

	// CastleBot FaceStabber
	S_FACESTABBER_STND1,
	S_FACESTABBER_STND2,
	S_FACESTABBER_STND3,
	S_FACESTABBER_STND4,
	S_FACESTABBER_STND5,
	S_FACESTABBER_STND6,
	S_FACESTABBER_CHARGE1,
	S_FACESTABBER_CHARGE2,
	S_FACESTABBER_CHARGE3,
	S_FACESTABBER_CHARGE4,

	// Egg Guard
	S_EGGGUARD_STND,
	S_EGGGUARD_WALK1,
	S_EGGGUARD_WALK2,
	S_EGGGUARD_WALK3,
	S_EGGGUARD_WALK4,
	S_EGGGUARD_MAD1,
	S_EGGGUARD_MAD2,
	S_EGGGUARD_MAD3,
	S_EGGGUARD_RUN1,
	S_EGGGUARD_RUN2,
	S_EGGGUARD_RUN3,
	S_EGGGUARD_RUN4,

	// Egg Shield for Egg Guard
	S_EGGSHIELD,

	// Green Snapper
	S_GSNAPPER_STND,
	S_GSNAPPER1,
	S_GSNAPPER2,
	S_GSNAPPER3,
	S_GSNAPPER4,

	// Minus
	S_MINUS_STND,
	S_MINUS_DIGGING,
	S_MINUS_POPUP,
	S_MINUS_UPWARD1,
	S_MINUS_UPWARD2,
	S_MINUS_UPWARD3,
	S_MINUS_UPWARD4,
	S_MINUS_UPWARD5,
	S_MINUS_UPWARD6,
	S_MINUS_UPWARD7,
	S_MINUS_UPWARD8,
	S_MINUS_DOWNWARD1,
	S_MINUS_DOWNWARD2,
	S_MINUS_DOWNWARD3,
	S_MINUS_DOWNWARD4,
	S_MINUS_DOWNWARD5,
	S_MINUS_DOWNWARD6,
	S_MINUS_DOWNWARD7,
	S_MINUS_DOWNWARD8,

	// Spring Shell
	S_SSHELL_STND,
	S_SSHELL_RUN1,
	S_SSHELL_RUN2,
	S_SSHELL_RUN3,
	S_SSHELL_RUN4,
	S_SSHELL_SPRING1,
	S_SSHELL_SPRING2,
	S_SSHELL_SPRING3,
	S_SSHELL_SPRING4,

	// Spring Shell (yellow)
	S_YSHELL_STND,
	S_YSHELL_RUN1,
	S_YSHELL_RUN2,
	S_YSHELL_RUN3,
	S_YSHELL_RUN4,
	S_YSHELL_SPRING1,
	S_YSHELL_SPRING2,
	S_YSHELL_SPRING3,
	S_YSHELL_SPRING4,

	// Unidus
	S_UNIDUS_STND,
	S_UNIDUS_RUN,
	S_UNIDUS_BALL,

	// Boss Explosion
	S_BPLD1,
	S_BPLD2,
	S_BPLD3,
	S_BPLD4,
	S_BPLD5,
	S_BPLD6,
	S_BPLD7,

	// S3&K Boss Explosion
	S_SONIC3KBOSSEXPLOSION1,
	S_SONIC3KBOSSEXPLOSION2,
	S_SONIC3KBOSSEXPLOSION3,
	S_SONIC3KBOSSEXPLOSION4,
	S_SONIC3KBOSSEXPLOSION5,
	S_SONIC3KBOSSEXPLOSION6,

	S_JETFUME1,
	S_JETFUME2,

	// Boss 1
	S_EGGMOBILE_STND,
	S_EGGMOBILE_LATK1,
	S_EGGMOBILE_LATK2,
	S_EGGMOBILE_LATK3,
	S_EGGMOBILE_LATK4,
	S_EGGMOBILE_LATK5,
	S_EGGMOBILE_LATK6,
	S_EGGMOBILE_LATK7,
	S_EGGMOBILE_LATK8,
	S_EGGMOBILE_LATK9,
	S_EGGMOBILE_LATK10,
	S_EGGMOBILE_RATK1,
	S_EGGMOBILE_RATK2,
	S_EGGMOBILE_RATK3,
	S_EGGMOBILE_RATK4,
	S_EGGMOBILE_RATK5,
	S_EGGMOBILE_RATK6,
	S_EGGMOBILE_RATK7,
	S_EGGMOBILE_RATK8,
	S_EGGMOBILE_RATK9,
	S_EGGMOBILE_RATK10,
	S_EGGMOBILE_PANIC1,
	S_EGGMOBILE_PANIC2,
	S_EGGMOBILE_PANIC3,
	S_EGGMOBILE_PANIC4,
	S_EGGMOBILE_PANIC5,
	S_EGGMOBILE_PANIC6,
	S_EGGMOBILE_PANIC7,
	S_EGGMOBILE_PANIC8,
	S_EGGMOBILE_PANIC9,
	S_EGGMOBILE_PANIC10,
	S_EGGMOBILE_PAIN,
	S_EGGMOBILE_PAIN2,
	S_EGGMOBILE_DIE1,
	S_EGGMOBILE_DIE2,
	S_EGGMOBILE_DIE3,
	S_EGGMOBILE_DIE4,
	S_EGGMOBILE_DIE5,
	S_EGGMOBILE_DIE6,
	S_EGGMOBILE_DIE7,
	S_EGGMOBILE_DIE8,
	S_EGGMOBILE_DIE9,
	S_EGGMOBILE_DIE10,
	S_EGGMOBILE_DIE11,
	S_EGGMOBILE_DIE12,
	S_EGGMOBILE_DIE13,
	S_EGGMOBILE_DIE14,
	S_EGGMOBILE_FLEE1,
	S_EGGMOBILE_FLEE2,
	S_EGGMOBILE_BALL,
	S_EGGMOBILE_TARGET,

	// Boss 2
	S_EGGMOBILE2_STND,
	S_EGGMOBILE2_POGO1,
	S_EGGMOBILE2_POGO2,
	S_EGGMOBILE2_POGO3,
	S_EGGMOBILE2_POGO4,
	S_EGGMOBILE2_POGO5,
	S_EGGMOBILE2_POGO6,
	S_EGGMOBILE2_POGO7,
	S_EGGMOBILE2_PAIN,
	S_EGGMOBILE2_PAIN2,
	S_EGGMOBILE2_DIE1,
	S_EGGMOBILE2_DIE2,
	S_EGGMOBILE2_DIE3,
	S_EGGMOBILE2_DIE4,
	S_EGGMOBILE2_DIE5,
	S_EGGMOBILE2_DIE6,
	S_EGGMOBILE2_DIE7,
	S_EGGMOBILE2_DIE8,
	S_EGGMOBILE2_DIE9,
	S_EGGMOBILE2_DIE10,
	S_EGGMOBILE2_DIE11,
	S_EGGMOBILE2_DIE12,
	S_EGGMOBILE2_DIE13,
	S_EGGMOBILE2_DIE14,
	S_EGGMOBILE2_FLEE1,
	S_EGGMOBILE2_FLEE2,

	S_BOSSTANK1,
	S_BOSSTANK2,
	S_BOSSSPIGOT,

	// Boss 2 Goop
	S_GOOP1,
	S_GOOP2,
	S_GOOP3,

	// Boss 3
	S_EGGMOBILE3_STND,
	S_EGGMOBILE3_ATK1,
	S_EGGMOBILE3_ATK2,
	S_EGGMOBILE3_ATK3A,
	S_EGGMOBILE3_ATK3B,
	S_EGGMOBILE3_ATK3C,
	S_EGGMOBILE3_ATK3D,
	S_EGGMOBILE3_ATK4,
	S_EGGMOBILE3_ATK5,
	S_EGGMOBILE3_LAUGH1,
	S_EGGMOBILE3_LAUGH2,
	S_EGGMOBILE3_LAUGH3,
	S_EGGMOBILE3_LAUGH4,
	S_EGGMOBILE3_LAUGH5,
	S_EGGMOBILE3_LAUGH6,
	S_EGGMOBILE3_LAUGH7,
	S_EGGMOBILE3_LAUGH8,
	S_EGGMOBILE3_LAUGH9,
	S_EGGMOBILE3_LAUGH10,
	S_EGGMOBILE3_LAUGH11,
	S_EGGMOBILE3_LAUGH12,
	S_EGGMOBILE3_LAUGH13,
	S_EGGMOBILE3_LAUGH14,
	S_EGGMOBILE3_LAUGH15,
	S_EGGMOBILE3_LAUGH16,
	S_EGGMOBILE3_LAUGH17,
	S_EGGMOBILE3_LAUGH18,
	S_EGGMOBILE3_LAUGH19,
	S_EGGMOBILE3_LAUGH20,
	S_EGGMOBILE3_PAIN,
	S_EGGMOBILE3_PAIN2,
	S_EGGMOBILE3_DIE1,
	S_EGGMOBILE3_DIE2,
	S_EGGMOBILE3_DIE3,
	S_EGGMOBILE3_DIE4,
	S_EGGMOBILE3_DIE5,
	S_EGGMOBILE3_DIE6,
	S_EGGMOBILE3_DIE7,
	S_EGGMOBILE3_DIE8,
	S_EGGMOBILE3_DIE9,
	S_EGGMOBILE3_DIE10,
	S_EGGMOBILE3_DIE11,
	S_EGGMOBILE3_DIE12,
	S_EGGMOBILE3_DIE13,
	S_EGGMOBILE3_DIE14,
	S_EGGMOBILE3_FLEE1,
	S_EGGMOBILE3_FLEE2,

	// Boss 3 Propeller
	S_PROPELLER1,
	S_PROPELLER2,
	S_PROPELLER3,
	S_PROPELLER4,
	S_PROPELLER5,
	S_PROPELLER6,
	S_PROPELLER7,

	// Boss 3 Pinch
	S_FAKEMOBILE_INIT,
	S_FAKEMOBILE,
	S_FAKEMOBILE_ATK1,
	S_FAKEMOBILE_ATK2,
	S_FAKEMOBILE_ATK3A,
	S_FAKEMOBILE_ATK3B,
	S_FAKEMOBILE_ATK3C,
	S_FAKEMOBILE_ATK3D,
	S_FAKEMOBILE_ATK4,
	S_FAKEMOBILE_ATK5,

	// Boss 4
	S_EGGMOBILE4_STND,
	S_EGGMOBILE4_LATK1,
	S_EGGMOBILE4_LATK2,
	S_EGGMOBILE4_LATK3,
	S_EGGMOBILE4_LATK4,
	S_EGGMOBILE4_LATK5,
	S_EGGMOBILE4_LATK6,
	S_EGGMOBILE4_RATK1,
	S_EGGMOBILE4_RATK2,
	S_EGGMOBILE4_RATK3,
	S_EGGMOBILE4_RATK4,
	S_EGGMOBILE4_RATK5,
	S_EGGMOBILE4_RATK6,
	S_EGGMOBILE4_RAISE1,
	S_EGGMOBILE4_RAISE2,
	S_EGGMOBILE4_RAISE3,
	S_EGGMOBILE4_RAISE4,
	S_EGGMOBILE4_RAISE5,
	S_EGGMOBILE4_RAISE6,
	S_EGGMOBILE4_RAISE7,
	S_EGGMOBILE4_RAISE8,
	S_EGGMOBILE4_RAISE9,
	S_EGGMOBILE4_RAISE10,
	S_EGGMOBILE4_PAIN,
	S_EGGMOBILE4_DIE1,
	S_EGGMOBILE4_DIE2,
	S_EGGMOBILE4_DIE3,
	S_EGGMOBILE4_DIE4,
	S_EGGMOBILE4_DIE5,
	S_EGGMOBILE4_DIE6,
	S_EGGMOBILE4_DIE7,
	S_EGGMOBILE4_DIE8,
	S_EGGMOBILE4_DIE9,
	S_EGGMOBILE4_DIE10,
	S_EGGMOBILE4_DIE11,
	S_EGGMOBILE4_DIE12,
	S_EGGMOBILE4_DIE13,
	S_EGGMOBILE4_DIE14,
	S_EGGMOBILE4_FLEE1,
	S_EGGMOBILE4_FLEE2,
	S_EGGMOBILE4_MACE,

	// Boss 4 jet flame
	S_JETFLAME1,
	S_JETFLAME2,

	// Black Eggman (Boss 7)
	S_BLACKEGG_STND,
	S_BLACKEGG_STND2,
	S_BLACKEGG_WALK1,
	S_BLACKEGG_WALK2,
	S_BLACKEGG_WALK3,
	S_BLACKEGG_WALK4,
	S_BLACKEGG_WALK5,
	S_BLACKEGG_WALK6,
	S_BLACKEGG_SHOOT1,
	S_BLACKEGG_SHOOT2,
	S_BLACKEGG_PAIN1,
	S_BLACKEGG_PAIN2,
	S_BLACKEGG_PAIN3,
	S_BLACKEGG_PAIN4,
	S_BLACKEGG_PAIN5,
	S_BLACKEGG_PAIN6,
	S_BLACKEGG_PAIN7,
	S_BLACKEGG_PAIN8,
	S_BLACKEGG_PAIN9,
	S_BLACKEGG_PAIN10,
	S_BLACKEGG_PAIN11,
	S_BLACKEGG_PAIN12,
	S_BLACKEGG_PAIN13,
	S_BLACKEGG_PAIN14,
	S_BLACKEGG_PAIN15,
	S_BLACKEGG_PAIN16,
	S_BLACKEGG_PAIN17,
	S_BLACKEGG_PAIN18,
	S_BLACKEGG_PAIN19,
	S_BLACKEGG_PAIN20,
	S_BLACKEGG_PAIN21,
	S_BLACKEGG_PAIN22,
	S_BLACKEGG_PAIN23,
	S_BLACKEGG_PAIN24,
	S_BLACKEGG_PAIN25,
	S_BLACKEGG_PAIN26,
	S_BLACKEGG_PAIN27,
	S_BLACKEGG_PAIN28,
	S_BLACKEGG_PAIN29,
	S_BLACKEGG_PAIN30,
	S_BLACKEGG_PAIN31,
	S_BLACKEGG_PAIN32,
	S_BLACKEGG_PAIN33,
	S_BLACKEGG_PAIN34,
	S_BLACKEGG_PAIN35,
	S_BLACKEGG_HITFACE1,
	S_BLACKEGG_HITFACE2,
	S_BLACKEGG_HITFACE3,
	S_BLACKEGG_HITFACE4,
	S_BLACKEGG_DIE1,
	S_BLACKEGG_DIE2,
	S_BLACKEGG_DIE3,
	S_BLACKEGG_DIE4,
	S_BLACKEGG_DIE5,
	S_BLACKEGG_MISSILE1,
	S_BLACKEGG_MISSILE2,
	S_BLACKEGG_MISSILE3,
	S_BLACKEGG_GOOP,
	S_BLACKEGG_JUMP1,
	S_BLACKEGG_JUMP2,
	S_BLACKEGG_DESTROYPLAT1,
	S_BLACKEGG_DESTROYPLAT2,
	S_BLACKEGG_DESTROYPLAT3,

	S_BLACKEGG_HELPER, // Collision helper

	S_BLACKEGG_GOOP1,
	S_BLACKEGG_GOOP2,
	S_BLACKEGG_GOOP3,
	S_BLACKEGG_GOOP4,
	S_BLACKEGG_GOOP5,
	S_BLACKEGG_GOOP6,
	S_BLACKEGG_GOOP7,

	S_BLACKEGG_MISSILE,

	// New Very-Last-Minute 2.1 Brak Eggman (Cy-Brak-demon)
	S_CYBRAKDEMON_IDLE,
	S_CYBRAKDEMON_WALK1,
	S_CYBRAKDEMON_WALK2,
	S_CYBRAKDEMON_WALK3,
	S_CYBRAKDEMON_WALK4,
	S_CYBRAKDEMON_WALK5,
	S_CYBRAKDEMON_WALK6,
	S_CYBRAKDEMON_CHOOSE_ATTACK1,
	S_CYBRAKDEMON_MISSILE_ATTACK1, // Aim
	S_CYBRAKDEMON_MISSILE_ATTACK2, // Fire
	S_CYBRAKDEMON_MISSILE_ATTACK3, // Aim
	S_CYBRAKDEMON_MISSILE_ATTACK4, // Fire
	S_CYBRAKDEMON_MISSILE_ATTACK5, // Aim
	S_CYBRAKDEMON_MISSILE_ATTACK6, // Fire
	S_CYBRAKDEMON_FLAME_ATTACK1, // Reset
	S_CYBRAKDEMON_FLAME_ATTACK2, // Aim
	S_CYBRAKDEMON_FLAME_ATTACK3, // Fire
	S_CYBRAKDEMON_FLAME_ATTACK4, // Loop
	S_CYBRAKDEMON_CHOOSE_ATTACK2,
	S_CYBRAKDEMON_VILE_ATTACK1,
	S_CYBRAKDEMON_VILE_ATTACK2,
	S_CYBRAKDEMON_VILE_ATTACK3,
	S_CYBRAKDEMON_VILE_ATTACK4,
	S_CYBRAKDEMON_VILE_ATTACK5,
	S_CYBRAKDEMON_VILE_ATTACK6,
	S_CYBRAKDEMON_NAPALM_ATTACK1,
	S_CYBRAKDEMON_NAPALM_ATTACK2,
	S_CYBRAKDEMON_NAPALM_ATTACK3,
	S_CYBRAKDEMON_FINISH_ATTACK1, // If just attacked, remove MF2_FRET w/out going back to spawnstate
	S_CYBRAKDEMON_FINISH_ATTACK2, // Force a delay between attacks so you don't get bombarded with them back-to-back
	S_CYBRAKDEMON_PAIN1,
	S_CYBRAKDEMON_PAIN2,
	S_CYBRAKDEMON_PAIN3,
	S_CYBRAKDEMON_DIE1,
	S_CYBRAKDEMON_DIE2,
	S_CYBRAKDEMON_DIE3,
	S_CYBRAKDEMON_DIE4,
	S_CYBRAKDEMON_DIE5,
	S_CYBRAKDEMON_DIE6,
	S_CYBRAKDEMON_DIE7,
	S_CYBRAKDEMON_DIE8,
	S_CYBRAKDEMON_DEINVINCIBLERIZE,
	S_CYBRAKDEMON_INVINCIBLERIZE,

	S_CYBRAKDEMONMISSILE,
	S_CYBRAKDEMONMISSILE_EXPLODE1,
	S_CYBRAKDEMONMISSILE_EXPLODE2,
	S_CYBRAKDEMONMISSILE_EXPLODE3,

	S_CYBRAKDEMONFLAMESHOT_FLY1,
	S_CYBRAKDEMONFLAMESHOT_FLY2,
	S_CYBRAKDEMONFLAMESHOT_FLY3,
	S_CYBRAKDEMONFLAMESHOT_DIE,

	S_CYBRAKDEMONFLAMEREST,

	S_CYBRAKDEMONELECTRICBARRIER_INIT1,
	S_CYBRAKDEMONELECTRICBARRIER_INIT2,
	S_CYBRAKDEMONELECTRICBARRIER_PLAYSOUND,
	S_CYBRAKDEMONELECTRICBARRIER1,
	S_CYBRAKDEMONELECTRICBARRIER2,
	S_CYBRAKDEMONELECTRICBARRIER3,
	S_CYBRAKDEMONELECTRICBARRIER4,
	S_CYBRAKDEMONELECTRICBARRIER5,
	S_CYBRAKDEMONELECTRICBARRIER6,
	S_CYBRAKDEMONELECTRICBARRIER7,
	S_CYBRAKDEMONELECTRICBARRIER8,
	S_CYBRAKDEMONELECTRICBARRIER9,
	S_CYBRAKDEMONELECTRICBARRIER10,
	S_CYBRAKDEMONELECTRICBARRIER11,
	S_CYBRAKDEMONELECTRICBARRIER12,
	S_CYBRAKDEMONELECTRICBARRIER13,
	S_CYBRAKDEMONELECTRICBARRIER14,
	S_CYBRAKDEMONELECTRICBARRIER15,
	S_CYBRAKDEMONELECTRICBARRIER16,
	S_CYBRAKDEMONELECTRICBARRIER17,
	S_CYBRAKDEMONELECTRICBARRIER18,
	S_CYBRAKDEMONELECTRICBARRIER19,
	S_CYBRAKDEMONELECTRICBARRIER20,
	S_CYBRAKDEMONELECTRICBARRIER21,
	S_CYBRAKDEMONELECTRICBARRIER22,
	S_CYBRAKDEMONELECTRICBARRIER23,
	S_CYBRAKDEMONELECTRICBARRIER24,
	S_CYBRAKDEMONELECTRICBARRIER_DIE1,
	S_CYBRAKDEMONELECTRICBARRIER_DIE2,
	S_CYBRAKDEMONELECTRICBARRIER_DIE3,
	S_CYBRAKDEMONELECTRICBARRIER_SPARK_RANDOMCHECK,
	S_CYBRAKDEMONELECTRICBARRIER_SPARK_RANDOMSUCCESS,
	S_CYBRAKDEMONELECTRICBARRIER_SPARK_RANDOMCHOOSE,
	S_CYBRAKDEMONELECTRICBARRIER_SPARK_RANDOM1,
	S_CYBRAKDEMONELECTRICBARRIER_SPARK_RANDOM2,
	S_CYBRAKDEMONELECTRICBARRIER_SPARK_RANDOM3,
	S_CYBRAKDEMONELECTRICBARRIER_SPARK_RANDOM4,
	S_CYBRAKDEMONELECTRICBARRIER_SPARK_RANDOM5,
	S_CYBRAKDEMONELECTRICBARRIER_SPARK_RANDOM6,
	S_CYBRAKDEMONELECTRICBARRIER_SPARK_RANDOM7,
	S_CYBRAKDEMONELECTRICBARRIER_SPARK_RANDOM8,
	S_CYBRAKDEMONELECTRICBARRIER_SPARK_RANDOM9,
	S_CYBRAKDEMONELECTRICBARRIER_SPARK_RANDOM10,
	S_CYBRAKDEMONELECTRICBARRIER_SPARK_RANDOM11,
	S_CYBRAKDEMONELECTRICBARRIER_SPARK_RANDOM12,
	S_CYBRAKDEMONELECTRICBARRIER_SPARK_RANDOMFAIL,
	S_CYBRAKDEMONELECTRICBARRIER_SPARK_RANDOMLOOP,
	S_CYBRAKDEMONELECTRICBARRIER_REVIVE1,
	S_CYBRAKDEMONELECTRICBARRIER_REVIVE2,
	S_CYBRAKDEMONELECTRICBARRIER_REVIVE3,

	S_CYBRAKDEMONTARGETRETICULE1,
	S_CYBRAKDEMONTARGETRETICULE2,
	S_CYBRAKDEMONTARGETRETICULE3,
	S_CYBRAKDEMONTARGETRETICULE4,
	S_CYBRAKDEMONTARGETRETICULE5,
	S_CYBRAKDEMONTARGETRETICULE6,
	S_CYBRAKDEMONTARGETRETICULE7,
	S_CYBRAKDEMONTARGETRETICULE8,
	S_CYBRAKDEMONTARGETRETICULE9,
	S_CYBRAKDEMONTARGETRETICULE10,
	S_CYBRAKDEMONTARGETRETICULE11,
	S_CYBRAKDEMONTARGETRETICULE12,
	S_CYBRAKDEMONTARGETRETICULE13,
	S_CYBRAKDEMONTARGETRETICULE14,

	S_CYBRAKDEMONTARGETDOT,

	S_CYBRAKDEMONNAPALMBOMBLARGE_FLY1,
	S_CYBRAKDEMONNAPALMBOMBLARGE_FLY2,
	S_CYBRAKDEMONNAPALMBOMBLARGE_FLY3,
	S_CYBRAKDEMONNAPALMBOMBLARGE_FLY4,
	S_CYBRAKDEMONNAPALMBOMBLARGE_DIE1, // Explode
	S_CYBRAKDEMONNAPALMBOMBLARGE_DIE2, // Outer ring
	S_CYBRAKDEMONNAPALMBOMBLARGE_DIE3, // Center
	S_CYBRAKDEMONNAPALMBOMBLARGE_DIE4, // Sound

	S_CYBRAKDEMONNAPALMBOMBSMALL,
	S_CYBRAKDEMONNAPALMBOMBSMALL_DIE1, // Explode
	S_CYBRAKDEMONNAPALMBOMBSMALL_DIE2, // Outer ring
	S_CYBRAKDEMONNAPALMBOMBSMALL_DIE3, // Inner ring
	S_CYBRAKDEMONNAPALMBOMBSMALL_DIE4, // Center
	S_CYBRAKDEMONNAPALMBOMBSMALL_DIE5, // Sound

	S_CYBRAKDEMONNAPALMFLAME_FLY1,
	S_CYBRAKDEMONNAPALMFLAME_FLY2,
	S_CYBRAKDEMONNAPALMFLAME_FLY3,
	S_CYBRAKDEMONNAPALMFLAME_FLY4,
	S_CYBRAKDEMONNAPALMFLAME_FLY5,
	S_CYBRAKDEMONNAPALMFLAME_FLY6,
	S_CYBRAKDEMONNAPALMFLAME_DIE,

	S_CYBRAKDEMONVILEEXPLOSION1,
	S_CYBRAKDEMONVILEEXPLOSION2,
	S_CYBRAKDEMONVILEEXPLOSION3,

	// Metal Sonic (Race)
	// S_PLAY_STND
	S_METALSONIC_STAND,
	// S_PLAY_TAP1
	S_METALSONIC_WAIT1,
	S_METALSONIC_WAIT2,
	// S_PLAY_RUN1
	S_METALSONIC_WALK1,
	S_METALSONIC_WALK2,
	S_METALSONIC_WALK3,
	S_METALSONIC_WALK4,
	S_METALSONIC_WALK5,
	S_METALSONIC_WALK6,
	S_METALSONIC_WALK7,
	S_METALSONIC_WALK8,
	// S_PLAY_SPD1
	S_METALSONIC_RUN1,
	S_METALSONIC_RUN2,
	S_METALSONIC_RUN3,
	S_METALSONIC_RUN4,
	// Metal Sonic (Battle)
	S_METALSONIC_FLOAT,
	S_METALSONIC_VECTOR,
	S_METALSONIC_STUN,
	S_METALSONIC_BLOCK,
	S_METALSONIC_RAISE,
	S_METALSONIC_GATHER,
	S_METALSONIC_DASH,
	S_METALSONIC_BOUNCE,
	S_METALSONIC_SHOOT,
	S_METALSONIC_PAIN,
	S_METALSONIC_DEATH,
	S_METALSONIC_FLEE1,
	S_METALSONIC_FLEE2,
	S_METALSONIC_FLEE3,
	S_METALSONIC_FLEE4,

	S_MSSHIELD_F1,
	S_MSSHIELD_F2,
	S_MSSHIELD_F3,
	S_MSSHIELD_F4,
	S_MSSHIELD_F5,
	S_MSSHIELD_F6,
	S_MSSHIELD_F7,
	S_MSSHIELD_F8,
	S_MSSHIELD_F9,
	S_MSSHIELD_F10,
	S_MSSHIELD_F11,
	S_MSSHIELD_F12,

	// Ring
	S_RING,

	// Blue Sphere for special stages
	S_BLUEBALL,
	S_BLUEBALLSPARK,

	// Gravity Wells for special stages
	S_GRAVWELLGREEN,
	S_GRAVWELLGREEN2,
	S_GRAVWELLGREEN3,

	S_GRAVWELLRED,
	S_GRAVWELLRED2,
	S_GRAVWELLRED3,

	// Individual Team Rings
	S_TEAMRING,

	// Special Stage Token
	S_EMMY,

	// Special Stage Token
	S_TOKEN,
	S_MOVINGTOKEN,

	// CTF Flags
	S_REDFLAG,
	S_BLUEFLAG,

	// Emblem
	S_EMBLEM1,
	S_EMBLEM2,
	S_EMBLEM3,
	S_EMBLEM4,
	S_EMBLEM5,
	S_EMBLEM6,
	S_EMBLEM7,
	S_EMBLEM8,
	S_EMBLEM9,
	S_EMBLEM10,
	S_EMBLEM11,
	S_EMBLEM12,
	S_EMBLEM13,
	S_EMBLEM14,
	S_EMBLEM15,
	S_EMBLEM16,
	S_EMBLEM17,
	S_EMBLEM18,
	S_EMBLEM19,
	S_EMBLEM20,
	S_EMBLEM21,
	S_EMBLEM22,
	S_EMBLEM23,
	S_EMBLEM24,
	S_EMBLEM25,
	S_EMBLEM26,

	// Chaos Emeralds
	S_CEMG1,
	S_CEMG2,
	S_CEMG3,
	S_CEMG4,
	S_CEMG5,
	S_CEMG6,
	S_CEMG7,

	// Emeralds (for hunt)
	S_EMER1,

	S_FAN,
	S_FAN2,
	S_FAN3,
	S_FAN4,
	S_FAN5,

	// Bubble Source
	S_BUBBLES1,
	S_BUBBLES2,

	// Level End Sign
	S_SIGN1,
	S_SIGN2,
	S_SIGN3,
	S_SIGN4,
	S_SIGN5,
	S_SIGN6,
	S_SIGN7,
	S_SIGN8,
	S_SIGN9,
	S_SIGN10,
	S_SIGN11,
	S_SIGN12,
	S_SIGN13,
	S_SIGN14,
	S_SIGN15,
	S_SIGN16,
	S_SIGN17,
	S_SIGN18,
	S_SIGN19,
	S_SIGN20,
	S_SIGN_END,

	// Steam Riser
	S_STEAM1,
	S_STEAM2,
	S_STEAM3,
	S_STEAM4,
	S_STEAM5,
	S_STEAM6,
	S_STEAM7,
	S_STEAM8,

	// Spike Ball
	S_SPIKEBALL1,
	S_SPIKEBALL2,
	S_SPIKEBALL3,
	S_SPIKEBALL4,
	S_SPIKEBALL5,
	S_SPIKEBALL6,
	S_SPIKEBALL7,
	S_SPIKEBALL8,

	// Fire Shield's Spawn
	S_SPINFIRE1,
	S_SPINFIRE2,
	S_SPINFIRE3,
	S_SPINFIRE4,
	S_SPINFIRE5,
	S_SPINFIRE6,

	// Spikes
	S_SPIKE1,
	S_SPIKE2,
	S_SPIKE3,
	S_SPIKE4,
	S_SPIKE5,
	S_SPIKE6,
	S_SPIKED1,
	S_SPIKED2,

	// Starpost
	S_STARPOST_IDLE,
	S_STARPOST_FLASH,
	S_STARPOST_SPIN,

	// Big floating mine
	S_BIGMINE1,
	S_BIGMINE2,
	S_BIGMINE3,
	S_BIGMINE4,
	S_BIGMINE5,
	S_BIGMINE6,
	S_BIGMINE7,
	S_BIGMINE8,

	// Cannon Launcher
	S_CANNONLAUNCHER1,
	S_CANNONLAUNCHER2,
	S_CANNONLAUNCHER3,

	// Super Ring Box
	S_SUPERRINGBOX,
	S_SUPERRINGBOX1,
	S_SUPERRINGBOX2,
	S_SUPERRINGBOX3,
	S_SUPERRINGBOX4,
	S_SUPERRINGBOX5,
	S_SUPERRINGBOX6,

	// Red Team Ring Box
	S_REDRINGBOX,
	S_REDRINGBOX1,

	// Blue Team Ring Box
	S_BLUERINGBOX,
	S_BLUERINGBOX1,

	// Super Sneakers Box
	S_SHTV,
	S_SHTV1,
	S_SHTV2,
	S_SHTV3,
	S_SHTV4,
	S_SHTV5,
	S_SHTV6,

	// Invincibility Box
	S_PINV,
	S_PINV1,
	S_PINV2,
	S_PINV3,
	S_PINV4,
	S_PINV5,
	S_PINV6,

	// 1up Box
	S_PRUP,
	S_PRUP1,
	S_PRUP2,
	S_PRUP3,
	S_PRUP4,
	S_PRUP5,
	S_PRUP6,

	// Ring Shield Box
	S_YLTV,
	S_YLTV1,
	S_YLTV2,
	S_YLTV3,
	S_YLTV4,
	S_YLTV5,
	S_YLTV6,

	// Force Shield Box
	S_BLTV1,
	S_BLTV2,
	S_BLTV3,
	S_BLTV4,
	S_BLTV5,
	S_BLTV6,
	S_BLTV7,

	// Bomb Shield Box
	S_BKTV1,
	S_BKTV2,
	S_BKTV3,
	S_BKTV4,
	S_BKTV5,
	S_BKTV6,
	S_BKTV7,

	// Jump Shield Box
	S_WHTV1,
	S_WHTV2,
	S_WHTV3,
	S_WHTV4,
	S_WHTV5,
	S_WHTV6,
	S_WHTV7,

	// Water Shield Box
	S_GRTV,
	S_GRTV1,
	S_GRTV2,
	S_GRTV3,
	S_GRTV4,
	S_GRTV5,
	S_GRTV6,

	// Pity Shield Box
	S_PITV1,
	S_PITV2,
	S_PITV3,
	S_PITV4,
	S_PITV5,
	S_PITV6,
	S_PITV7,

	// Eggman Box
	S_EGGTV1,
	S_EGGTV2,
	S_EGGTV3,
	S_EGGTV4,
	S_EGGTV5,
	S_EGGTV6,
	S_EGGTV7,

	// Teleport Box
	S_MIXUPBOX1,
	S_MIXUPBOX2,
	S_MIXUPBOX3,
	S_MIXUPBOX4,
	S_MIXUPBOX5,
	S_MIXUPBOX6,
	S_MIXUPBOX7,

	// Recycler Box
	S_RECYCLETV1,
	S_RECYCLETV2,
	S_RECYCLETV3,
	S_RECYCLETV4,
	S_RECYCLETV5,
	S_RECYCLETV6,
	S_RECYCLETV7,

	// Question Box
	S_RANDOMBOX1,
	S_RANDOMBOX2,
	S_RANDOMBOX3,

	// Gravity Boots Box
	S_GBTV1,
	S_GBTV2,
	S_GBTV3,
	S_GBTV4,
	S_GBTV5,
	S_GBTV6,
	S_GBTV7,

	// Score boxes
	S_SCORETVA1,
	S_SCORETVA2,
	S_SCORETVA3,
	S_SCORETVA4,
	S_SCORETVA5,
	S_SCORETVA6,
	S_SCORETVA7,
	S_SCORETVB1,
	S_SCORETVB2,
	S_SCORETVB3,
	S_SCORETVB4,
	S_SCORETVB5,
	S_SCORETVB6,
	S_SCORETVB7,

	// Monitor Explosion
	S_MONITOREXPLOSION1,
	S_MONITOREXPLOSION2,

	S_REDMONITOREXPLOSION1,
	S_REDMONITOREXPLOSION2,

	S_BLUEMONITOREXPLOSION1,
	S_BLUEMONITOREXPLOSION2,

	S_ROCKET,

	S_LASER,

	S_TORPEDO,

	S_ENERGYBALL1,
	S_ENERGYBALL2,

	// Skim Mine, also used by Jetty-Syn bomber
	S_MINE1,
	S_MINE_BOOM1,
	S_MINE_BOOM2,
	S_MINE_BOOM3,
	S_MINE_BOOM4,

	// Jetty-Syn Bullet
	S_JETBULLET1,
	S_JETBULLET2,

	S_TURRETLASER,
	S_TURRETLASEREXPLODE1,
	S_TURRETLASEREXPLODE2,

	// Cannonball
	S_CANNONBALL1,

	// Arrow
	S_ARROW,
	S_ARROWUP,
	S_ARROWDOWN,

	// Trapgoyle Demon fire
	S_DEMONFIRE1,
	S_DEMONFIRE2,
	S_DEMONFIRE3,
	S_DEMONFIRE4,
	S_DEMONFIRE5,
	S_DEMONFIRE6,

	S_GFZFLOWERA,
	S_GFZFLOWERA2,

	S_GFZFLOWERB1,
	S_GFZFLOWERB2,

	S_GFZFLOWERC1,

	S_BERRYBUSH,
	S_BUSH,

	// THZ Plant
	S_THZPLANT1,
	S_THZPLANT2,
	S_THZPLANT3,
	S_THZPLANT4,

	// THZ Alarm
	S_ALARM1,

	// Deep Sea Gargoyle
	S_GARGOYLE,

	// DSZ Seaweed
	S_SEAWEED1,
	S_SEAWEED2,
	S_SEAWEED3,
	S_SEAWEED4,
	S_SEAWEED5,
	S_SEAWEED6,

	// Dripping Water
	S_DRIPA1,
	S_DRIPA2,
	S_DRIPA3,
	S_DRIPA4,
	S_DRIPB1,
	S_DRIPC1,
	S_DRIPC2,

	// Coral 1
	S_CORAL1,

	// Coral 2
	S_CORAL2,

	// Coral 3
	S_CORAL3,

	// Blue Crystal
	S_BLUECRYSTAL1,

	// CEZ Chain
	S_CEZCHAIN,

	// Flame
	S_FLAME1,
	S_FLAME2,
	S_FLAME3,
	S_FLAME4,

	// Eggman Statue
	S_EGGSTATUE1,

	// CEZ hidden sling
	S_SLING1,
	S_SLING2,

	// CEZ Small Mace Chain
	S_SMALLMACECHAIN,

	// CEZ Big Mace Chain
	S_BIGMACECHAIN,

	// CEZ Small Mace
	S_SMALLMACE,

	// CEZ Big Mace
	S_BIGMACE,

	S_CEZFLOWER1,

	// Big Tumbleweed
	S_BIGTUMBLEWEED,
	S_BIGTUMBLEWEED_ROLL1,
	S_BIGTUMBLEWEED_ROLL2,
	S_BIGTUMBLEWEED_ROLL3,
	S_BIGTUMBLEWEED_ROLL4,
	S_BIGTUMBLEWEED_ROLL5,
	S_BIGTUMBLEWEED_ROLL6,
	S_BIGTUMBLEWEED_ROLL7,
	S_BIGTUMBLEWEED_ROLL8,

	// Little Tumbleweed
	S_LITTLETUMBLEWEED,
	S_LITTLETUMBLEWEED_ROLL1,
	S_LITTLETUMBLEWEED_ROLL2,
	S_LITTLETUMBLEWEED_ROLL3,
	S_LITTLETUMBLEWEED_ROLL4,
	S_LITTLETUMBLEWEED_ROLL5,
	S_LITTLETUMBLEWEED_ROLL6,
	S_LITTLETUMBLEWEED_ROLL7,
	S_LITTLETUMBLEWEED_ROLL8,

	// Cacti Sprites
	S_CACTI1,
	S_CACTI2,
	S_CACTI3,
	S_CACTI4,

	// Flame jet
	S_FLAMEJETSTND,
	S_FLAMEJETSTART,
	S_FLAMEJETSTOP,
	S_FLAMEJETFLAME1,
	S_FLAMEJETFLAME2,
	S_FLAMEJETFLAME3,

	// Spinning flame jets
	S_FJSPINAXISA1, // Counter-clockwise
	S_FJSPINAXISA2,
	S_FJSPINAXISA3,
	S_FJSPINAXISA4,
	S_FJSPINAXISA5,
	S_FJSPINAXISA6,
	S_FJSPINAXISA7,
	S_FJSPINAXISA8,
	S_FJSPINAXISA9,
	S_FJSPINHELPERA1,
	S_FJSPINHELPERA2,
	S_FJSPINHELPERA3,
	S_FJSPINAXISB1, // Clockwise
	S_FJSPINAXISB2,
	S_FJSPINAXISB3,
	S_FJSPINAXISB4,
	S_FJSPINAXISB5,
	S_FJSPINAXISB6,
	S_FJSPINAXISB7,
	S_FJSPINAXISB8,
	S_FJSPINAXISB9,
	S_FJSPINHELPERB1,
	S_FJSPINHELPERB2,
	S_FJSPINHELPERB3,

	// Blade's flame
	S_FLAMEJETFLAMEB1,
	S_FLAMEJETFLAMEB2,
	S_FLAMEJETFLAMEB3,
	S_FLAMEJETFLAMEB4,
	S_FLAMEJETFLAMEB5,
	S_FLAMEJETFLAMEB6,

	// Trapgoyles
	S_TRAPGOYLE,
	S_TRAPGOYLE_CHECK,
	S_TRAPGOYLE_FIRE1,
	S_TRAPGOYLE_FIRE2,
	S_TRAPGOYLE_FIRE3,
	S_TRAPGOYLEUP,
	S_TRAPGOYLEUP_CHECK,
	S_TRAPGOYLEUP_FIRE1,
	S_TRAPGOYLEUP_FIRE2,
	S_TRAPGOYLEUP_FIRE3,
	S_TRAPGOYLEDOWN,
	S_TRAPGOYLEDOWN_CHECK,
	S_TRAPGOYLEDOWN_FIRE1,
	S_TRAPGOYLEDOWN_FIRE2,
	S_TRAPGOYLEDOWN_FIRE3,
	S_TRAPGOYLELONG,
	S_TRAPGOYLELONG_CHECK,
	S_TRAPGOYLELONG_FIRE1,
	S_TRAPGOYLELONG_FIRE2,
	S_TRAPGOYLELONG_FIRE3,
	S_TRAPGOYLELONG_FIRE4,
	S_TRAPGOYLELONG_FIRE5,

	// ATZ's Red Crystal/Target
	S_TARGET_IDLE,
	S_TARGET_HIT1,
	S_TARGET_HIT2,
	S_TARGET_RESPAWN,
	S_TARGET_ALLDONE,

	// Stalagmites
	S_STG0,
	S_STG1,
	S_STG2,
	S_STG3,
	S_STG4,
	S_STG5,
	S_STG6,
	S_STG7,
	S_STG8,
	S_STG9,

	// Xmas-specific stuff
	S_XMASPOLE,
	S_CANDYCANE,
	S_SNOWMAN,
	S_SNOWMANHAT,
	S_LAMPPOST1,
	S_LAMPPOST2,
	S_HANGSTAR,

	// Botanic Serenity's loads of scenery states
	S_BSZTALLFLOWER_RED,
	S_BSZTALLFLOWER_PURPLE,
	S_BSZTALLFLOWER_BLUE,
	S_BSZTALLFLOWER_CYAN,
	S_BSZTALLFLOWER_YELLOW,
	S_BSZTALLFLOWER_ORANGE,
	S_BSZFLOWER_RED,
	S_BSZFLOWER_PURPLE,
	S_BSZFLOWER_BLUE,
	S_BSZFLOWER_CYAN,
	S_BSZFLOWER_YELLOW,
	S_BSZFLOWER_ORANGE,
	S_BSZSHORTFLOWER_RED,
	S_BSZSHORTFLOWER_PURPLE,
	S_BSZSHORTFLOWER_BLUE,
	S_BSZSHORTFLOWER_CYAN,
	S_BSZSHORTFLOWER_YELLOW,
	S_BSZSHORTFLOWER_ORANGE,
	S_BSZTULIP_RED,
	S_BSZTULIP_PURPLE,
	S_BSZTULIP_BLUE,
	S_BSZTULIP_CYAN,
	S_BSZTULIP_YELLOW,
	S_BSZTULIP_ORANGE,
	S_BSZCLUSTER_RED,
	S_BSZCLUSTER_PURPLE,
	S_BSZCLUSTER_BLUE,
	S_BSZCLUSTER_CYAN,
	S_BSZCLUSTER_YELLOW,
	S_BSZCLUSTER_ORANGE,
	S_BSZBUSH_RED,
	S_BSZBUSH_PURPLE,
	S_BSZBUSH_BLUE,
	S_BSZBUSH_CYAN,
	S_BSZBUSH_YELLOW,
	S_BSZBUSH_ORANGE,
	S_BSZVINE_RED,
	S_BSZVINE_PURPLE,
	S_BSZVINE_BLUE,
	S_BSZVINE_CYAN,
	S_BSZVINE_YELLOW,
	S_BSZVINE_ORANGE,
	S_BSZSHRUB,
	S_BSZCLOVER,
	S_BSZFISH,
	S_BSZSUNFLOWER,

	S_DBALL1,
	S_DBALL2,
	S_DBALL3,
	S_DBALL4,
	S_DBALL5,
	S_DBALL6,
	S_EGGSTATUE2,

	// Shield Orb
	S_ARMA1,
	S_ARMA2,
	S_ARMA3,
	S_ARMA4,
	S_ARMA5,
	S_ARMA6,
	S_ARMA7,
	S_ARMA8,
	S_ARMA9,
	S_ARMA10,
	S_ARMA11,
	S_ARMA12,
	S_ARMA13,
	S_ARMA14,
	S_ARMA15,
	S_ARMA16,

	S_ARMF1,
	S_ARMF2,
	S_ARMF3,
	S_ARMF4,
	S_ARMF5,
	S_ARMF6,
	S_ARMF7,
	S_ARMF8,
	S_ARMF9,
	S_ARMF10,
	S_ARMF11,
	S_ARMF12,
	S_ARMF13,
	S_ARMF14,
	S_ARMF15,
	S_ARMF16,

	S_ARMB1,
	S_ARMB2,
	S_ARMB3,
	S_ARMB4,
	S_ARMB5,
	S_ARMB6,
	S_ARMB7,
	S_ARMB8,
	S_ARMB9,
	S_ARMB10,
	S_ARMB11,
	S_ARMB12,
	S_ARMB13,
	S_ARMB14,
	S_ARMB15,
	S_ARMB16,

	S_WIND1,
	S_WIND2,
	S_WIND3,
	S_WIND4,
	S_WIND5,
	S_WIND6,
	S_WIND7,
	S_WIND8,

	S_MAGN1,
	S_MAGN2,
	S_MAGN3,
	S_MAGN4,
	S_MAGN5,
	S_MAGN6,
	S_MAGN7,
	S_MAGN8,
	S_MAGN9,
	S_MAGN10,
	S_MAGN11,
	S_MAGN12,

	S_FORC1,
	S_FORC2,
	S_FORC3,
	S_FORC4,
	S_FORC5,
	S_FORC6,
	S_FORC7,
	S_FORC8,
	S_FORC9,
	S_FORC10,

	S_FORC11,
	S_FORC12,
	S_FORC13,
	S_FORC14,
	S_FORC15,
	S_FORC16,
	S_FORC17,
	S_FORC18,
	S_FORC19,
	S_FORC20,

	S_ELEM1,
	S_ELEM2,
	S_ELEM3,
	S_ELEM4,
	S_ELEM5,
	S_ELEM6,
	S_ELEM7,
	S_ELEM8,
	S_ELEM9,
	S_ELEM10,
	S_ELEM11,
	S_ELEM12,

	S_ELEMF1,
	S_ELEMF2,
	S_ELEMF3,
	S_ELEMF4,
	S_ELEMF5,
	S_ELEMF6,
	S_ELEMF7,
	S_ELEMF8,

	S_PITY1,
	S_PITY2,
	S_PITY3,
	S_PITY4,
	S_PITY5,
	S_PITY6,
	S_PITY7,
	S_PITY8,
	S_PITY9,
	S_PITY10,

	// Invincibility Sparkles
	S_IVSP,

	// Super Sonic Spark
	S_SSPK1,
	S_SSPK2,
	S_SSPK3,
	S_SSPK4,
	S_SSPK5,

	// Freed Birdie
	S_BIRD1,
	S_BIRD2,
	S_BIRD3,

	// Freed Bunny
	S_BUNNY1,
	S_BUNNY2,
	S_BUNNY3,
	S_BUNNY4,
	S_BUNNY5,
	S_BUNNY6,
	S_BUNNY7,
	S_BUNNY8,
	S_BUNNY9,
	S_BUNNY10,

	// Freed Mouse
	S_MOUSE1,
	S_MOUSE2,

	// Freed Chicken
	S_CHICKEN1,
	S_CHICKENHOP,
	S_CHICKENFLY1,
	S_CHICKENFLY2,

	// Freed Cow
	S_COW1,
	S_COW2,
	S_COW3,
	S_COW4,

	// Red Birdie in Bubble
	S_RBIRD1,
	S_RBIRD2,
	S_RBIRD3,

	S_YELLOWSPRING,
	S_YELLOWSPRING2,
	S_YELLOWSPRING3,
	S_YELLOWSPRING4,
	S_YELLOWSPRING5,

	S_REDSPRING,
	S_REDSPRING2,
	S_REDSPRING3,
	S_REDSPRING4,
	S_REDSPRING5,

	// Blue Springs
	S_BLUESPRING,
	S_BLUESPRING2,
	S_BLUESPRING3,
	S_BLUESPRING4,
	S_BLUESPRING5,

	// Yellow Diagonal Spring
	S_YDIAG1,
	S_YDIAG2,
	S_YDIAG3,
	S_YDIAG4,
	S_YDIAG5,
	S_YDIAG6,
	S_YDIAG7,
	S_YDIAG8,

	// Red Diagonal Spring
	S_RDIAG1,
	S_RDIAG2,
	S_RDIAG3,
	S_RDIAG4,
	S_RDIAG5,
	S_RDIAG6,
	S_RDIAG7,
	S_RDIAG8,

	// Rain
	S_RAIN1,
	S_RAINRETURN,

	// Snowflake
	S_SNOW1,
	S_SNOW2,
	S_SNOW3,

	// Water Splish
	S_SPLISH1,
	S_SPLISH2,
	S_SPLISH3,
	S_SPLISH4,
	S_SPLISH5,
	S_SPLISH6,
	S_SPLISH7,
	S_SPLISH8,
	S_SPLISH9,

	// added water splash
	S_SPLASH1,
	S_SPLASH2,
	S_SPLASH3,

	// lava/slime damage burn smoke
	S_SMOKE1,
	S_SMOKE2,
	S_SMOKE3,
	S_SMOKE4,
	S_SMOKE5,

	// Bubbles
	S_SMALLBUBBLE,
	S_SMALLBUBBLE1,
	S_MEDIUMBUBBLE,
	S_MEDIUMBUBBLE1,
	S_LARGEBUBBLE,
	S_EXTRALARGEBUBBLE, // breathable

	S_POP1, // Extra Large bubble goes POP!

	S_FOG1,
	S_FOG2,
	S_FOG3,
	S_FOG4,
	S_FOG5,
	S_FOG6,
	S_FOG7,
	S_FOG8,
	S_FOG9,
	S_FOG10,
	S_FOG11,
	S_FOG12,
	S_FOG13,
	S_FOG14,

	S_SEED,

	S_PARTICLE,
	S_PARTICLEGEN,

	// Score Logos
	S_SCRA, // 100
	S_SCRB, // 200
	S_SCRC, // 500
	S_SCRD, // 1000
	S_SCRE, // 10000
	S_SCRF, // 400 (mario)
	S_SCRG, // 800 (mario)
	S_SCRH, // 2000 (mario)
	S_SCRI, // 4000 (mario)
	S_SCRJ, // 8000 (mario)
	S_SCRK, // 1UP (mario)

	// Drowning Timer Numbers
	S_ZERO1,
	S_ONE1,
	S_TWO1,
	S_THREE1,
	S_FOUR1,
	S_FIVE1,

	// Tag Sign
	S_TTAG1,

	// Got Flag Sign
	S_GOTFLAG1,
	S_GOTFLAG2,
	S_GOTFLAG3,
	S_GOTFLAG4,

	// Red Ring
	S_RRNG1,
	S_RRNG2,
	S_RRNG3,
	S_RRNG4,
	S_RRNG5,
	S_RRNG6,
	S_RRNG7,

	// Weapon Ring Ammo
	S_BOUNCERINGAMMO,
	S_RAILRINGAMMO,
	S_INFINITYRINGAMMO,
	S_AUTOMATICRINGAMMO,
	S_EXPLOSIONRINGAMMO,
	S_SCATTERRINGAMMO,
	S_GRENADERINGAMMO,

	// Weapon pickup
	S_BOUNCEPICKUP,
	S_BOUNCEPICKUPFADE1,
	S_BOUNCEPICKUPFADE2,
	S_BOUNCEPICKUPFADE3,
	S_BOUNCEPICKUPFADE4,
	S_BOUNCEPICKUPFADE5,
	S_BOUNCEPICKUPFADE6,
	S_BOUNCEPICKUPFADE7,
	S_BOUNCEPICKUPFADE8,

	S_RAILPICKUP,
	S_RAILPICKUPFADE1,
	S_RAILPICKUPFADE2,
	S_RAILPICKUPFADE3,
	S_RAILPICKUPFADE4,
	S_RAILPICKUPFADE5,
	S_RAILPICKUPFADE6,
	S_RAILPICKUPFADE7,
	S_RAILPICKUPFADE8,

	S_AUTOPICKUP,
	S_AUTOPICKUPFADE1,
	S_AUTOPICKUPFADE2,
	S_AUTOPICKUPFADE3,
	S_AUTOPICKUPFADE4,
	S_AUTOPICKUPFADE5,
	S_AUTOPICKUPFADE6,
	S_AUTOPICKUPFADE7,
	S_AUTOPICKUPFADE8,

	S_EXPLODEPICKUP,
	S_EXPLODEPICKUPFADE1,
	S_EXPLODEPICKUPFADE2,
	S_EXPLODEPICKUPFADE3,
	S_EXPLODEPICKUPFADE4,
	S_EXPLODEPICKUPFADE5,
	S_EXPLODEPICKUPFADE6,
	S_EXPLODEPICKUPFADE7,
	S_EXPLODEPICKUPFADE8,

	S_SCATTERPICKUP,
	S_SCATTERPICKUPFADE1,
	S_SCATTERPICKUPFADE2,
	S_SCATTERPICKUPFADE3,
	S_SCATTERPICKUPFADE4,
	S_SCATTERPICKUPFADE5,
	S_SCATTERPICKUPFADE6,
	S_SCATTERPICKUPFADE7,
	S_SCATTERPICKUPFADE8,

	S_GRENADEPICKUP,
	S_GRENADEPICKUPFADE1,
	S_GRENADEPICKUPFADE2,
	S_GRENADEPICKUPFADE3,
	S_GRENADEPICKUPFADE4,
	S_GRENADEPICKUPFADE5,
	S_GRENADEPICKUPFADE6,
	S_GRENADEPICKUPFADE7,
	S_GRENADEPICKUPFADE8,

	// Thrown Weapon Rings
	S_THROWNBOUNCE1,
	S_THROWNBOUNCE2,
	S_THROWNBOUNCE3,
	S_THROWNBOUNCE4,
	S_THROWNBOUNCE5,
	S_THROWNBOUNCE6,
	S_THROWNBOUNCE7,
	S_THROWNINFINITY1,
	S_THROWNINFINITY2,
	S_THROWNINFINITY3,
	S_THROWNINFINITY4,
	S_THROWNINFINITY5,
	S_THROWNINFINITY6,
	S_THROWNINFINITY7,
	S_THROWNAUTOMATIC1,
	S_THROWNAUTOMATIC2,
	S_THROWNAUTOMATIC3,
	S_THROWNAUTOMATIC4,
	S_THROWNAUTOMATIC5,
	S_THROWNAUTOMATIC6,
	S_THROWNAUTOMATIC7,
	S_THROWNEXPLOSION1,
	S_THROWNEXPLOSION2,
	S_THROWNEXPLOSION3,
	S_THROWNEXPLOSION4,
	S_THROWNEXPLOSION5,
	S_THROWNEXPLOSION6,
	S_THROWNEXPLOSION7,
	S_THROWNGRENADE1,
	S_THROWNGRENADE2,
	S_THROWNGRENADE3,
	S_THROWNGRENADE4,
	S_THROWNGRENADE5,
	S_THROWNGRENADE6,
	S_THROWNGRENADE7,
	S_THROWNGRENADE8,
	S_THROWNGRENADE9,
	S_THROWNGRENADE10,
	S_THROWNGRENADE11,
	S_THROWNGRENADE12,
	S_THROWNGRENADE13,
	S_THROWNGRENADE14,
	S_THROWNGRENADE15,
	S_THROWNGRENADE16,
	S_THROWNGRENADE17,
	S_THROWNGRENADE18,
	S_THROWNSCATTER,

	S_RINGEXPLODE,

	S_COIN1,
	S_COIN2,
	S_COIN3,
	S_COINSPARKLE1,
	S_COINSPARKLE2,
	S_COINSPARKLE3,
	S_COINSPARKLE4,
	S_GOOMBA1,
	S_GOOMBA1B,
	S_GOOMBA2,
	S_GOOMBA3,
	S_GOOMBA4,
	S_GOOMBA5,
	S_GOOMBA6,
	S_GOOMBA7,
	S_GOOMBA8,
	S_GOOMBA9,
	S_GOOMBA_DEAD,
	S_BLUEGOOMBA1,
	S_BLUEGOOMBA1B,
	S_BLUEGOOMBA2,
	S_BLUEGOOMBA3,
	S_BLUEGOOMBA4,
	S_BLUEGOOMBA5,
	S_BLUEGOOMBA6,
	S_BLUEGOOMBA7,
	S_BLUEGOOMBA8,
	S_BLUEGOOMBA9,
	S_BLUEGOOMBA_DEAD,

	// Mario-specific stuff
	S_FIREFLOWER1,
	S_FIREFLOWER2,
	S_FIREFLOWER3,
	S_FIREFLOWER4,
	S_FIREBALL1,
	S_FIREBALL2,
	S_FIREBALL3,
	S_FIREBALL4,
	S_FIREBALLEXP1,
	S_FIREBALLEXP2,
	S_FIREBALLEXP3,
	S_SHELL,
	S_SHELL1,
	S_SHELL2,
	S_SHELL3,
	S_SHELL4,
	S_PUMA1,
	S_PUMA2,
	S_PUMA3,
	S_PUMA4,
	S_PUMA5,
	S_PUMA6,
	S_HAMMER1,
	S_HAMMER2,
	S_HAMMER3,
	S_HAMMER4,
	S_KOOPA1,
	S_KOOPA2,
	S_KOOPAFLAME1,
	S_KOOPAFLAME2,
	S_KOOPAFLAME3,
	S_AXE1,
	S_AXE2,
	S_AXE3,
	S_MARIOBUSH1,
	S_MARIOBUSH2,
	S_TOAD,

	// Nights-specific stuff
	S_NIGHTSDRONE1,
	S_NIGHTSDRONE2,
	S_NIGHTSDRONE_SPARKLING1,
	S_NIGHTSDRONE_SPARKLING2,
	S_NIGHTSDRONE_SPARKLING3,
	S_NIGHTSDRONE_SPARKLING4,
	S_NIGHTSDRONE_SPARKLING5,
	S_NIGHTSDRONE_SPARKLING6,
	S_NIGHTSDRONE_SPARKLING7,
	S_NIGHTSDRONE_SPARKLING8,
	S_NIGHTSDRONE_SPARKLING9,
	S_NIGHTSDRONE_SPARKLING10,
	S_NIGHTSDRONE_SPARKLING11,
	S_NIGHTSDRONE_SPARKLING12,
	S_NIGHTSDRONE_SPARKLING13,
	S_NIGHTSDRONE_SPARKLING14,
	S_NIGHTSDRONE_SPARKLING15,
	S_NIGHTSDRONE_SPARKLING16,
	S_NIGHTSGOAL1,
	S_NIGHTSGOAL2,
	S_NIGHTSGOAL3,
	S_NIGHTSGOAL4,

	S_NIGHTSFLY1A,
	S_NIGHTSFLY1B,
	S_NIGHTSDRILL1A,
	S_NIGHTSDRILL1B,
	S_NIGHTSDRILL1C,
	S_NIGHTSDRILL1D,
	S_NIGHTSFLY2A,
	S_NIGHTSFLY2B,
	S_NIGHTSDRILL2A,
	S_NIGHTSDRILL2B,
	S_NIGHTSDRILL2C,
	S_NIGHTSDRILL2D,
	S_NIGHTSFLY3A,
	S_NIGHTSFLY3B,
	S_NIGHTSDRILL3A,
	S_NIGHTSDRILL3B,
	S_NIGHTSDRILL3C,
	S_NIGHTSDRILL3D,
	S_NIGHTSFLY4A,
	S_NIGHTSFLY4B,
	S_NIGHTSDRILL4A,
	S_NIGHTSDRILL4B,
	S_NIGHTSDRILL4C,
	S_NIGHTSDRILL4D,
	S_NIGHTSFLY5A,
	S_NIGHTSFLY5B,
	S_NIGHTSDRILL5A,
	S_NIGHTSDRILL5B,
	S_NIGHTSDRILL5C,
	S_NIGHTSDRILL5D,
	S_NIGHTSFLY6A,
	S_NIGHTSFLY6B,
	S_NIGHTSDRILL6A,
	S_NIGHTSDRILL6B,
	S_NIGHTSDRILL6C,
	S_NIGHTSDRILL6D,
	S_NIGHTSFLY7A,
	S_NIGHTSFLY7B,
	S_NIGHTSDRILL7A,
	S_NIGHTSDRILL7B,
	S_NIGHTSDRILL7C,
	S_NIGHTSDRILL7D,
	S_NIGHTSFLY8A,
	S_NIGHTSFLY8B,
	S_NIGHTSDRILL8A,
	S_NIGHTSDRILL8B,
	S_NIGHTSDRILL8C,
	S_NIGHTSDRILL8D,
	S_NIGHTSFLY9A,
	S_NIGHTSFLY9B,
	S_NIGHTSDRILL9A,
	S_NIGHTSDRILL9B,
	S_NIGHTSDRILL9C,
	S_NIGHTSDRILL9D,
	S_NIGHTSHURT1,
	S_NIGHTSHURT2,
	S_NIGHTSHURT3,
	S_NIGHTSHURT4,
	S_NIGHTSHURT5,
	S_NIGHTSHURT6,
	S_NIGHTSHURT7,
	S_NIGHTSHURT8,
	S_NIGHTSHURT9,
	S_NIGHTSHURT10,
	S_NIGHTSHURT11,
	S_NIGHTSHURT12,
	S_NIGHTSHURT13,
	S_NIGHTSHURT14,
	S_NIGHTSHURT15,
	S_NIGHTSHURT16,
	S_NIGHTSHURT17,
	S_NIGHTSHURT18,
	S_NIGHTSHURT19,
	S_NIGHTSHURT20,
	S_NIGHTSHURT21,
	S_NIGHTSHURT22,
	S_NIGHTSHURT23,
	S_NIGHTSHURT24,
	S_NIGHTSHURT25,
	S_NIGHTSHURT26,
	S_NIGHTSHURT27,
	S_NIGHTSHURT28,
	S_NIGHTSHURT29,
	S_NIGHTSHURT30,
	S_NIGHTSHURT31,
	S_NIGHTSHURT32,

	S_NIGHTSPARKLE1,
	S_NIGHTSPARKLE2,
	S_NIGHTSPARKLE3,
	S_NIGHTSPARKLE4,
	S_NIGHTSPARKLESUPER1,
	S_NIGHTSPARKLESUPER2,
	S_NIGHTSPARKLESUPER3,
	S_NIGHTSPARKLESUPER4,
	S_NIGHTSLOOPHELPER,

	// NiGHTS bumper
	S_NIGHTSBUMPER1,
	S_NIGHTSBUMPER2,
	S_NIGHTSBUMPER3,
	S_NIGHTSBUMPER4,
	S_NIGHTSBUMPER5,
	S_NIGHTSBUMPER6,
	S_NIGHTSBUMPER7,
	S_NIGHTSBUMPER8,
	S_NIGHTSBUMPER9,
	S_NIGHTSBUMPER10,
	S_NIGHTSBUMPER11,
	S_NIGHTSBUMPER12,

	S_HOOP,
	S_HOOP_XMASA,
	S_HOOP_XMASB,

	S_NIGHTSCORE10,
	S_NIGHTSCORE20,
	S_NIGHTSCORE30,
	S_NIGHTSCORE40,
	S_NIGHTSCORE50,
	S_NIGHTSCORE60,
	S_NIGHTSCORE70,
	S_NIGHTSCORE80,
	S_NIGHTSCORE90,
	S_NIGHTSCORE100,
	S_NIGHTSCORE10_2,
	S_NIGHTSCORE20_2,
	S_NIGHTSCORE30_2,
	S_NIGHTSCORE40_2,
	S_NIGHTSCORE50_2,
	S_NIGHTSCORE60_2,
	S_NIGHTSCORE70_2,
	S_NIGHTSCORE80_2,
	S_NIGHTSCORE90_2,
	S_NIGHTSCORE100_2,

	S_NIGHTSWING,
	S_NIGHTSWING_XMAS,

	// NiGHTS Paraloop Powerups
	S_NIGHTSPOWERUP1,
	S_NIGHTSPOWERUP2,
	S_NIGHTSPOWERUP3,
	S_NIGHTSPOWERUP4,
	S_NIGHTSPOWERUP5,
	S_NIGHTSPOWERUP6,
	S_NIGHTSPOWERUP7,
	S_NIGHTSPOWERUP8,
	S_NIGHTSPOWERUP9,
	S_NIGHTSPOWERUP10,
	S_EGGCAPSULE,

	// Orbiting Chaos Emeralds
	S_ORBITEM1,
	S_ORBITEM2,
	S_ORBITEM3,
	S_ORBITEM4,
	S_ORBITEM5,
	S_ORBITEM6,
	S_ORBITEM7,
	S_ORBITEM8,
	S_ORBITEM9,
	S_ORBITEM10,
	S_ORBITEM11,
	S_ORBITEM12,
	S_ORBITEM13,
	S_ORBITEM14,
	S_ORBITEM15,
	S_ORBITEM16,

	// "Flicky" helper
	S_NIGHTOPIANHELPER1,
	S_NIGHTOPIANHELPER2,
	S_NIGHTOPIANHELPER3,
	S_NIGHTOPIANHELPER4,
	S_NIGHTOPIANHELPER5,
	S_NIGHTOPIANHELPER6,
	S_NIGHTOPIANHELPER7,
	S_NIGHTOPIANHELPER8,

	S_CRUMBLE1,
	S_CRUMBLE2,

	S_SUPERTRANS1,
	S_SUPERTRANS2,
	S_SUPERTRANS3,
	S_SUPERTRANS4,
	S_SUPERTRANS5,
	S_SUPERTRANS6,
	S_SUPERTRANS7,
	S_SUPERTRANS8,
	S_SUPERTRANS9,

	// Spark
	S_SPRK1,
	S_SPRK2,
	S_SPRK3,
	S_SPRK4,
	S_SPRK5,
	S_SPRK6,
	S_SPRK7,
	S_SPRK8,
	S_SPRK9,
	S_SPRK10,
	S_SPRK11,
	S_SPRK12,
	S_SPRK13,
	S_SPRK14,
	S_SPRK15,
	S_SPRK16,

	// Robot Explosion
	S_XPLD1,
	S_XPLD2,
	S_XPLD3,
	S_XPLD4,

	// Underwater Explosion
	S_WPLD1,
	S_WPLD2,
	S_WPLD3,
	S_WPLD4,
	S_WPLD5,
	S_WPLD6,

	S_ROCKSPAWN,

	S_ROCKCRUMBLEA,
	S_ROCKCRUMBLEB,
	S_ROCKCRUMBLEC,
	S_ROCKCRUMBLED,
	S_ROCKCRUMBLEE,
	S_ROCKCRUMBLEF,
	S_ROCKCRUMBLEG,
	S_ROCKCRUMBLEH,
	S_ROCKCRUMBLEI,
	S_ROCKCRUMBLEJ,
	S_ROCKCRUMBLEK,
	S_ROCKCRUMBLEL,
	S_ROCKCRUMBLEM,
	S_ROCKCRUMBLEN,
	S_ROCKCRUMBLEO,
	S_ROCKCRUMBLEP,

	S_SRB1_CRAWLA1,
	S_SRB1_CRAWLA2,
	S_SRB1_CRAWLA3,
	S_SRB1_CRAWLA4,

	S_SRB1_BAT1,
	S_SRB1_BAT2,
	S_SRB1_BAT3,
	S_SRB1_BAT4,

	S_SRB1_ROBOFISH1,
	S_SRB1_ROBOFISH2,
	S_SRB1_ROBOFISH3,

	S_SRB1_VOLCANOGUY1,
	S_SRB1_VOLCANOGUY2,

	S_SRB1_HOPPY1,
	S_SRB1_HOPPY2,

	S_SRB1_HOPPYWATER1,
	S_SRB1_HOPPYWATER2,

	S_SRB1_HOPPYSKYLAB1,

	S_SRB1_MMZFLYING1,
	S_SRB1_MMZFLYING2,
	S_SRB1_MMZFLYING3,
	S_SRB1_MMZFLYING4,
	S_SRB1_MMZFLYING5,

	S_SRB1_UFO1,
	S_SRB1_UFO2,

	S_SRB1_GRAYBOT1,
	S_SRB1_GRAYBOT2,
	S_SRB1_GRAYBOT3,
	S_SRB1_GRAYBOT4,
	S_SRB1_GRAYBOT5,
	S_SRB1_GRAYBOT6,

	S_SRB1_ROBOTOPOLIS1,
	S_SRB1_ROBOTOPOLIS2,

	S_SRB1_RBZBUZZ1,
	S_SRB1_RBZBUZZ2,

	S_SRB1_RBZSPIKES1,
	S_SRB1_RBZSPIKES2,

	S_SRB1_METALSONIC1,
	S_SRB1_METALSONIC2,
	S_SRB1_METALSONIC3,

	S_SRB1_GOLDBOT1,
	S_SRB1_GOLDBOT2,
	S_SRB1_GOLDBOT3,
	S_SRB1_GOLDBOT4,
	S_SRB1_GOLDBOT5,
	S_SRB1_GOLDBOT6,

	S_SRB1_GENREX1,
	S_SRB1_GENREX2,

	// Gray Springs
	S_GRAYSPRING,
	S_GRAYSPRING2,
	S_GRAYSPRING3,
	S_GRAYSPRING4,
	S_GRAYSPRING5,

	// Invis-spring - this is used just for the sproing sound.
	S_INVISSPRING,

	// Blue Diagonal Spring
	S_BDIAG1,
	S_BDIAG2,
	S_BDIAG3,
	S_BDIAG4,
	S_BDIAG5,
	S_BDIAG6,
	S_BDIAG7,
	S_BDIAG8,

	//{ Random Item Box
	S_RANDOMITEM1,
	S_RANDOMITEM2,
	S_RANDOMITEM3,
	S_RANDOMITEM4,
	S_RANDOMITEM5,
	S_RANDOMITEM6,
	S_RANDOMITEM7,
	S_RANDOMITEM8,
	S_RANDOMITEM9,
	S_RANDOMITEM10,
	S_RANDOMITEM11,
	S_RANDOMITEM12,
	S_RANDOMITEM13,
	S_RANDOMITEM14,
	S_RANDOMITEM15,
	S_RANDOMITEM16,
	S_RANDOMITEM17,
	S_RANDOMITEM18,
	S_RANDOMITEM19,
	S_RANDOMITEM20,
	S_RANDOMITEM21,
	S_RANDOMITEM22,
	S_RANDOMITEM23,
	S_RANDOMITEM24,
	S_DEADRANDOMITEM,

	// Random Item Pop
	S_RANDOMITEMPOP1,
	S_RANDOMITEMPOP2,
	S_RANDOMITEMPOP3,
	S_RANDOMITEMPOP4,
	//}

	S_ITEMICON,

	// Signpost sparkles
	S_SIGNSPARK1,
	S_SIGNSPARK2,
	S_SIGNSPARK3,
	S_SIGNSPARK4,
	S_SIGNSPARK5,
	S_SIGNSPARK6,
	S_SIGNSPARK7,
	S_SIGNSPARK8,
	S_SIGNSPARK9,
	S_SIGNSPARK10,
	S_SIGNSPARK11,

	// Drift Sparks
	S_DRIFTSPARK_A1,
	S_DRIFTSPARK_A2,
	S_DRIFTSPARK_A3,
	S_DRIFTSPARK_B1,
	S_DRIFTSPARK_C1,
	S_DRIFTSPARK_C2,

	// Brake drift sparks
	S_BRAKEDRIFT,

	// Drift Smoke
	S_DRIFTDUST1,
	S_DRIFTDUST2,
	S_DRIFTDUST3,
	S_DRIFTDUST4,

	// Fast lines
	S_FASTLINE1,
	S_FASTLINE2,
	S_FASTLINE3,
	S_FASTLINE4,
	S_FASTLINE5,

	// Fast dust release
	S_FASTDUST1,
	S_FASTDUST2,
	S_FASTDUST3,
	S_FASTDUST4,
	S_FASTDUST5,
	S_FASTDUST6,
	S_FASTDUST7,

	// Magnet Burst

	// Sneaker boost effect
	S_BOOSTFLAME,
	S_BOOSTSMOKESPAWNER,
	S_BOOSTSMOKE1,
	S_BOOSTSMOKE2,
	S_BOOSTSMOKE3,
	S_BOOSTSMOKE4,
	S_BOOSTSMOKE5,
	S_BOOSTSMOKE6,

	// Sneaker Fire Trail
	S_KARTFIRE1,
	S_KARTFIRE2,
	S_KARTFIRE3,
	S_KARTFIRE4,
	S_KARTFIRE5,
	S_KARTFIRE6,
	S_KARTFIRE7,
	S_KARTFIRE8,

	// Angel Island Drift Strat Dust (what a mouthful!)
	S_KARTAIZDRIFTSTRAT,

	// Invincibility Sparks
	S_KARTINVULN_SMALL1,
	S_KARTINVULN_SMALL2,
	S_KARTINVULN_SMALL3,
	S_KARTINVULN_SMALL4,
	S_KARTINVULN_SMALL5,

	S_KARTINVULN_LARGE1,
	S_KARTINVULN_LARGE2,
	S_KARTINVULN_LARGE3,
	S_KARTINVULN_LARGE4,
	S_KARTINVULN_LARGE5,

	// Invincibility flash
	S_INVULNFLASH1,
	S_INVULNFLASH2,
	S_INVULNFLASH3,
	S_INVULNFLASH4,

	// Wipeout dust trail
	S_WIPEOUTTRAIL1,
	S_WIPEOUTTRAIL2,
	S_WIPEOUTTRAIL3,
	S_WIPEOUTTRAIL4,
	S_WIPEOUTTRAIL5,

	// Rocket sneaker
	S_ROCKETSNEAKER_L,
	S_ROCKETSNEAKER_R,
	S_ROCKETSNEAKER_LVIBRATE,
	S_ROCKETSNEAKER_RVIBRATE,

	//{ Eggman Monitor
	S_EGGMANITEM1,
	S_EGGMANITEM2,
	S_EGGMANITEM3,
	S_EGGMANITEM4,
	S_EGGMANITEM5,
	S_EGGMANITEM6,
	S_EGGMANITEM7,
	S_EGGMANITEM8,
	S_EGGMANITEM9,
	S_EGGMANITEM10,
	S_EGGMANITEM11,
	S_EGGMANITEM12,
	S_EGGMANITEM13,
	S_EGGMANITEM14,
	S_EGGMANITEM15,
	S_EGGMANITEM16,
	S_EGGMANITEM17,
	S_EGGMANITEM18,
	S_EGGMANITEM19,
	S_EGGMANITEM20,
	S_EGGMANITEM21,
	S_EGGMANITEM22,
	S_EGGMANITEM23,
	S_EGGMANITEM24,
	S_EGGMANITEM_DEAD,
	//}

	// Banana
	S_BANANA,
	S_BANANA_DEAD,

	//{ Orbinaut
	S_ORBINAUT1,
	S_ORBINAUT2,
	S_ORBINAUT3,
	S_ORBINAUT4,
	S_ORBINAUT5,
	S_ORBINAUT6,
	S_ORBINAUT_DEAD,
	S_ORBINAUT_SHIELD1,
	S_ORBINAUT_SHIELD2,
	S_ORBINAUT_SHIELD3,
	S_ORBINAUT_SHIELD4,
	S_ORBINAUT_SHIELD5,
	S_ORBINAUT_SHIELD6,
	S_ORBINAUT_SHIELDDEAD,
	//}
	//{ Jawz
	S_JAWZ1,
	S_JAWZ2,
	S_JAWZ3,
	S_JAWZ4,
	S_JAWZ5,
	S_JAWZ6,
	S_JAWZ7,
	S_JAWZ8,
	S_JAWZ_DUD1,
	S_JAWZ_DUD2,
	S_JAWZ_DUD3,
	S_JAWZ_DUD4,
	S_JAWZ_DUD5,
	S_JAWZ_DUD6,
	S_JAWZ_DUD7,
	S_JAWZ_DUD8,
	S_JAWZ_SHIELD1,
	S_JAWZ_SHIELD2,
	S_JAWZ_SHIELD3,
	S_JAWZ_SHIELD4,
	S_JAWZ_SHIELD5,
	S_JAWZ_SHIELD6,
	S_JAWZ_SHIELD7,
	S_JAWZ_SHIELD8,
	S_JAWZ_DEAD1,
	S_JAWZ_DEAD2,
	//}

	S_PLAYERRETICULE, // Player reticule

	// Special Stage Mine
	S_SSMINE1,
	S_SSMINE2,
	S_SSMINE3,
	S_SSMINE4,
	S_SSMINE_SHIELD1,
	S_SSMINE_SHIELD2,
	S_SSMINE_AIR1,
	S_SSMINE_AIR2,
	S_SSMINE_DEPLOY1,
	S_SSMINE_DEPLOY2,
	S_SSMINE_DEPLOY3,
	S_SSMINE_DEPLOY4,
	S_SSMINE_DEPLOY5,
	S_SSMINE_DEPLOY6,
	S_SSMINE_DEPLOY7,
	S_SSMINE_DEPLOY8,
	S_SSMINE_DEPLOY9,
	S_SSMINE_DEPLOY10,
	S_SSMINE_DEPLOY11,
	S_SSMINE_DEPLOY12,
	S_SSMINE_DEPLOY13,
	S_SSMINE_EXPLODE,
	S_MINEEXPLOSION1,
	S_MINEEXPLOSION2,

	// New explosion
	S_QUICKBOOM1,
	S_QUICKBOOM2,
	S_QUICKBOOM3,
	S_QUICKBOOM4,
	S_QUICKBOOM5,
	S_QUICKBOOM6,
	S_QUICKBOOM7,
	S_QUICKBOOM8,
	S_QUICKBOOM9,
	S_QUICKBOOM10,

	S_SLOWBOOM1,
	S_SLOWBOOM2,
	S_SLOWBOOM3,
	S_SLOWBOOM4,
	S_SLOWBOOM5,
	S_SLOWBOOM6,
	S_SLOWBOOM7,
	S_SLOWBOOM8,
	S_SLOWBOOM9,
	S_SLOWBOOM10,

	// Ballhog
	S_BALLHOG1,
	S_BALLHOG2,
	S_BALLHOG3,
	S_BALLHOG4,
	S_BALLHOG5,
	S_BALLHOG6,
	S_BALLHOG7,
	S_BALLHOG8,
	S_BALLHOG_DEAD,
	S_BALLHOGBOOM1,
	S_BALLHOGBOOM2,
	S_BALLHOGBOOM3,
	S_BALLHOGBOOM4,
	S_BALLHOGBOOM5,
	S_BALLHOGBOOM6,
	S_BALLHOGBOOM7,
	S_BALLHOGBOOM8,
	S_BALLHOGBOOM9,
	S_BALLHOGBOOM10,
	S_BALLHOGBOOM11,
	S_BALLHOGBOOM12,
	S_BALLHOGBOOM13,
	S_BALLHOGBOOM14,
	S_BALLHOGBOOM15,
	S_BALLHOGBOOM16,

	// Self-Propelled Bomb
	S_SPB1,
	S_SPB2,
	S_SPB3,
	S_SPB4,
	S_SPB5,
	S_SPB6,
	S_SPB7,
	S_SPB8,
	S_SPB9,
	S_SPB10,
	S_SPB11,
	S_SPB12,
	S_SPB13,
	S_SPB14,
	S_SPB15,
	S_SPB16,
	S_SPB17,
	S_SPB18,
	S_SPB19,
	S_SPB20,
	S_SPB_DEAD,

	// Thunder Shield
	S_THUNDERSHIELD1,
	S_THUNDERSHIELD2,
	S_THUNDERSHIELD3,
	S_THUNDERSHIELD4,
	S_THUNDERSHIELD5,
	S_THUNDERSHIELD6,
	S_THUNDERSHIELD7,
	S_THUNDERSHIELD8,
	S_THUNDERSHIELD9,
	S_THUNDERSHIELD10,
	S_THUNDERSHIELD11,
	S_THUNDERSHIELD12,
	S_THUNDERSHIELD13,
	S_THUNDERSHIELD14,
	S_THUNDERSHIELD15,
	S_THUNDERSHIELD16,
	S_THUNDERSHIELD17,
	S_THUNDERSHIELD18,
	S_THUNDERSHIELD19,
	S_THUNDERSHIELD20,
	S_THUNDERSHIELD21,
	S_THUNDERSHIELD22,
	S_THUNDERSHIELD23,
	S_THUNDERSHIELD24,

	// The legend
	S_SINK,
	S_SINK_SHIELD,
	S_SINKTRAIL1,
	S_SINKTRAIL2,
	S_SINKTRAIL3,

	// Battle Mode bumpers
	S_BATTLEBUMPER1,
	S_BATTLEBUMPER2,
	S_BATTLEBUMPER3,

	// DEZ Laser respawn
	S_DEZLASER,

	// Audience Members
	S_RANDOMAUDIENCE,
	S_AUDIENCE_CHAO_CHEER1,
	S_AUDIENCE_CHAO_CHEER2,
	S_AUDIENCE_CHAO_WIN1,
	S_AUDIENCE_CHAO_WIN2,
	S_AUDIENCE_CHAO_LOSE,

	// 1.0 Kart Decoratives
	S_FLAYM1,
	S_FLAYM2,
	S_FLAYM3,
	S_FLAYM4,
	S_DEVIL,
	S_ANGEL,
	S_PALMTREE,
	S_FLAG,
	S_HEDGEHOG, // (Rimshot)
	S_BUSH1,
	S_TWEE,
	S_HYDRANT,

	// New Misc Decorations
	S_BIGPUMA1,
	S_BIGPUMA2,
	S_BIGPUMA3,
	S_BIGPUMA4,
	S_BIGPUMA5,
	S_BIGPUMA6,
	S_APPLE1,
	S_APPLE2,
	S_APPLE3,
	S_APPLE4,
	S_APPLE5,
	S_APPLE6,
	S_APPLE7,
	S_APPLE8,

	// D00Dkart - Fall Flowers
	S_DOOD_FLOWER1,
	S_DOOD_FLOWER2,
	S_DOOD_FLOWER3,
	S_DOOD_FLOWER4,
	S_DOOD_FLOWER5,
	S_DOOD_FLOWER6,

	// D00Dkart - Super Circuit Box
	S_DOOD_BOX1,
	S_DOOD_BOX2,
	S_DOOD_BOX3,
	S_DOOD_BOX4,
	S_DOOD_BOX5,

	// D00Dkart - Diddy Kong Racing Bumper
	S_DOOD_BALLOON,

	// Chaotix Big Ring
	S_BIGRING01,
	S_BIGRING02,
	S_BIGRING03,
	S_BIGRING04,
	S_BIGRING05,
	S_BIGRING06,
	S_BIGRING07,
	S_BIGRING08,
	S_BIGRING09,
	S_BIGRING10,
	S_BIGRING11,
	S_BIGRING12,

	// SNES Objects
	S_SNES_DONUTBUSH1,
	S_SNES_DONUTBUSH2,
	S_SNES_DONUTBUSH3,

	// GBA Objects
	S_GBA_BOO1,
	S_GBA_BOO2,
	S_GBA_BOO3,
	S_GBA_BOO4,

	// Sapphire Coast Mobs
	S_BUZZBOMBER_LOOK1,
	S_BUZZBOMBER_LOOK2,
	S_BUZZBOMBER_FLY1,
	S_BUZZBOMBER_FLY2,
	S_BUZZBOMBER_FLY3,
	S_BUZZBOMBER_FLY4,

	S_CHOMPER_SPAWN,
	S_CHOMPER_HOP1,
	S_CHOMPER_HOP2,
	S_CHOMPER_TURNAROUND,

	S_PALMTREE2,
	S_PURPLEFLOWER1,
	S_PURPLEFLOWER2,
	S_YELLOWFLOWER1,
	S_YELLOWFLOWER2,
	S_PLANT2,
	S_PLANT3,
	S_PLANT4,

	// Crystal Abyss Mobs
	S_SKULL,
	S_PHANTREE,
	S_FLYINGGARG1,
	S_FLYINGGARG2,
	S_FLYINGGARG3,
	S_FLYINGGARG4,
	S_FLYINGGARG5,
	S_FLYINGGARG6,
	S_FLYINGGARG7,
	S_FLYINGGARG8,
	S_LAMPPOST,
	S_MOSSYTREE,

	S_SHADOW,
	S_WHITESHADOW,

	S_BUMP1,
	S_BUMP2,
	S_BUMP3,

	S_FLINGENERGY1,
	S_FLINGENERGY2,
	S_FLINGENERGY3,

	S_CLASH1,
	S_CLASH2,
	S_CLASH3,
	S_CLASH4,
	S_CLASH5,
	S_CLASH6,

	S_FIREDITEM1,
	S_FIREDITEM2,
	S_FIREDITEM3,
	S_FIREDITEM4,

	S_INSTASHIELDA1, // No damage instashield effect
	S_INSTASHIELDA2,
	S_INSTASHIELDA3,
	S_INSTASHIELDA4,
	S_INSTASHIELDA5,
	S_INSTASHIELDA6,
	S_INSTASHIELDA7,
	S_INSTASHIELDB1,
	S_INSTASHIELDB2,
	S_INSTASHIELDB3,
	S_INSTASHIELDB4,
	S_INSTASHIELDB5,
	S_INSTASHIELDB6,
	S_INSTASHIELDB7,

	S_PLAYERARROW, // Above player arrow
	S_PLAYERARROW_BOX,
	S_PLAYERARROW_ITEM,
	S_PLAYERARROW_NUMBER,
	S_PLAYERARROW_X,
	S_PLAYERARROW_WANTED1,
	S_PLAYERARROW_WANTED2,
	S_PLAYERARROW_WANTED3,
	S_PLAYERARROW_WANTED4,
	S_PLAYERARROW_WANTED5,
	S_PLAYERARROW_WANTED6,
	S_PLAYERARROW_WANTED7,

	S_PLAYERBOMB1, // Karma player overlays
	S_PLAYERBOMB2,
	S_PLAYERBOMB3,
	S_PLAYERBOMB4,
	S_PLAYERBOMB5,
	S_PLAYERBOMB6,
	S_PLAYERBOMB7,
	S_PLAYERBOMB8,
	S_PLAYERBOMB9,
	S_PLAYERBOMB10,
	S_PLAYERBOMB11,
	S_PLAYERBOMB12,
	S_PLAYERBOMB13,
	S_PLAYERBOMB14,
	S_PLAYERBOMB15,
	S_PLAYERBOMB16,
	S_PLAYERBOMB17,
	S_PLAYERBOMB18,
	S_PLAYERBOMB19,
	S_PLAYERBOMB20,
	S_PLAYERITEM,
	S_PLAYERFAKE,

	S_KARMAWHEEL,

	S_BATTLEPOINT1A, // Battle point indicators
	S_BATTLEPOINT1B,
	S_BATTLEPOINT1C,
	S_BATTLEPOINT1D,
	S_BATTLEPOINT1E,
	S_BATTLEPOINT1F,
	S_BATTLEPOINT1G,
	S_BATTLEPOINT1H,
	S_BATTLEPOINT1I,

	S_BATTLEPOINT2A,
	S_BATTLEPOINT2B,
	S_BATTLEPOINT2C,
	S_BATTLEPOINT2D,
	S_BATTLEPOINT2E,
	S_BATTLEPOINT2F,
	S_BATTLEPOINT2G,
	S_BATTLEPOINT2H,
	S_BATTLEPOINT2I,

	S_BATTLEPOINT3A,
	S_BATTLEPOINT3B,
	S_BATTLEPOINT3C,
	S_BATTLEPOINT3D,
	S_BATTLEPOINT3E,
	S_BATTLEPOINT3F,
	S_BATTLEPOINT3G,
	S_BATTLEPOINT3H,
	S_BATTLEPOINT3I,

	// Thunder shield use stuff;
	S_KSPARK1,	// Sparkling Radius
	S_KSPARK2,
	S_KSPARK3,
	S_KSPARK4,
	S_KSPARK5,
	S_KSPARK6,
	S_KSPARK7,
	S_KSPARK8,
	S_KSPARK9,
	S_KSPARK10,
	S_KSPARK11,
	S_KSPARK12,
	S_KSPARK13,	// ... that's an awful lot.

	S_LZIO11,	// Straight lightning bolt
	S_LZIO12,
	S_LZIO13,
	S_LZIO14,
	S_LZIO15,
	S_LZIO16,
	S_LZIO17,
	S_LZIO18,
	S_LZIO19,

	S_LZIO21,	// Straight lightning bolt (flipped)
	S_LZIO22,
	S_LZIO23,
	S_LZIO24,
	S_LZIO25,
	S_LZIO26,
	S_LZIO27,
	S_LZIO28,
	S_LZIO29,

	S_KLIT1,	// Diagonal lightning. No, it not being straight doesn't make it gay.
	S_KLIT2,
	S_KLIT3,
	S_KLIT4,
	S_KLIT5,
	S_KLIT6,
	S_KLIT7,
	S_KLIT8,
	S_KLIT9,
	S_KLIT10,
	S_KLIT11,
	S_KLIT12,

	S_FZEROSMOKE1, // F-Zero NO CONTEST explosion
	S_FZEROSMOKE2,
	S_FZEROSMOKE3,
	S_FZEROSMOKE4,
	S_FZEROSMOKE5,

	S_FZEROBOOM1,
	S_FZEROBOOM2,
	S_FZEROBOOM3,
	S_FZEROBOOM4,
	S_FZEROBOOM5,
	S_FZEROBOOM6,
	S_FZEROBOOM7,
	S_FZEROBOOM8,
	S_FZEROBOOM9,
	S_FZEROBOOM10,
	S_FZEROBOOM11,
	S_FZEROBOOM12,

	S_FZSLOWSMOKE1,
	S_FZSLOWSMOKE2,
	S_FZSLOWSMOKE3,
	S_FZSLOWSMOKE4,
	S_FZSLOWSMOKE5,

	// Various plants
	S_SONICBUSH,

	// Marble Zone
	S_FLAMEPARTICLE,
	S_MARBLETORCH,
	S_MARBLELIGHT,
	S_MARBLEBURNER,

	// CD Special Stage
	S_CDUFO,
	S_CDUFO_DIE,

	// Rusty Rig
	S_RUSTYLAMP_ORANGE,
	S_RUSTYCHAIN,

	// D2 Balloon Panic
	S_BALLOON,
	S_BALLOONPOP1,
	S_BALLOONPOP2,
	S_BALLOONPOP3,

	// Smokin' & Vapin' (Don't try this at home, kids!)
	S_PETSMOKE0,
	S_PETSMOKE1,
	S_PETSMOKE2,
	S_PETSMOKE3,
	S_PETSMOKE4,
	S_PETSMOKE5,
	S_VVVAPING0,
	S_VVVAPING1,
	S_VVVAPING2,
	S_VVVAPING3,
	S_VVVAPING4,
	S_VVVAPING5,
	S_VVVAPE,

	// Hill Top Zone
	S_HTZTREE,
	S_HTZBUSH,

	// Ports of gardens
	S_SGVINE1,
	S_SGVINE2,
	S_SGVINE3,
	S_PGTREE,
	S_PGFLOWER1,
	S_PGFLOWER2,
	S_PGFLOWER3,
	S_PGBUSH,
	S_DHPILLAR,

	// Midnight Channel stuff:
	S_SPOTLIGHT,	// Spotlight decoration
	S_RANDOMSHADOW,	// Random Shadow. They're static and don't do nothing.
	S_GARU1,
	S_GARU2,
	S_GARU3,
	S_TGARU0,
	S_TGARU1,
	S_TGARU2,
	S_TGARU3,	// Wind attack used by Roaming Shadows on Players.
	S_ROAMINGSHADOW,	// Roaming Shadow (the one that uses above's wind attack or smth)
	S_MAYONAKAARROW,	// Arrow sign

	// Mementos stuff:
	S_REAPER_INVIS,		// Reaper waiting for spawning
	S_REAPER,			// Reaper main frame where its thinker is handled
	S_MEMENTOSTP,		// Mementos teleporter state. (Used for spawning particles)

	// JackInTheBox
	S_JITB1,
	S_JITB2,
	S_JITB3,
	S_JITB4,
	S_JITB5,
	S_JITB6,

	// Color Drive
	S_CDMOONSP,
	S_CDBUSHSP,
	S_CDTREEASP,
	S_CDTREEBSP,

	// Daytona Speedway
	S_PINETREE,
	S_PINETREE_SIDE,

	// Egg Zeppelin
	S_EZZPROPELLER,
	S_EZZPROPELLER_BLADE,

	// Desert Palace
	S_DP_PALMTREE,

	// Aurora Atoll
	S_AAZTREE_SEG,
	S_AAZTREE_COCONUT,
	S_AAZTREE_LEAF,

	// Barren Badlands
	S_BBZDUST1, // Dust
	S_BBZDUST2,
	S_BBZDUST3,
	S_BBZDUST4,
	S_FROGGER, // Frog badniks
	S_FROGGER_ATTACK,
	S_FROGGER_JUMP,
	S_FROGTONGUE,
	S_FROGTONGUE_JOINT,
	S_ROBRA, // Black cobra badniks
	S_ROBRA_HEAD,
	S_ROBRA_JOINT,
	S_ROBRASHELL_INSIDE,
	S_ROBRASHELL_OUTSIDE,
	S_BLUEROBRA, // Blue cobra badniks
	S_BLUEROBRA_HEAD,
	S_BLUEROBRA_JOINT,

	// Eerie Grove
	S_EERIEFOG1,
	S_EERIEFOG2,
	S_EERIEFOG3,
	S_EERIEFOG4,
	S_EERIEFOG5,

	// SMK ports
	S_SMK_PIPE1, // Generic pipes
	S_SMK_PIPE2,
	S_SMK_MOLE, // Donut Plains Monty Moles
	S_SMK_THWOMP, // Bowser Castle Thwomps
	S_SMK_SNOWBALL, // Vanilla Lake snowballs
	S_SMK_ICEBLOCK, // Vanilla Lake breakable ice blocks
	S_SMK_ICEBLOCK2,
	S_SMK_ICEBLOCK_DEBRIS,
	S_SMK_ICEBLOCK_DEBRIS2,

	// Ezo's maps
	S_BLUEFIRE1,
	S_BLUEFIRE2,
	S_BLUEFIRE3,
	S_BLUEFIRE4,

	S_GREENFIRE1,
	S_GREENFIRE2,
	S_GREENFIRE3,
	S_GREENFIRE4,

	S_REGALCHEST,
	S_CHIMERASTATUE,
	S_DRAGONSTATUE,
	S_LIZARDMANSTATUE,
	S_PEGASUSSTATUE,

	S_ZELDAFIRE1,
	S_ZELDAFIRE2,
	S_ZELDAFIRE3,
	S_ZELDAFIRE4,

	S_GANBARETHING,
	S_GANBAREDUCK,
	S_GANBARETREE,

	S_MONOIDLE,
	S_MONOCHASE1,
	S_MONOCHASE2,
	S_MONOCHASE3,
	S_MONOCHASE4,
	S_MONOPAIN,

	S_REDZELDAFIRE1,
	S_REDZELDAFIRE2,
	S_REDZELDAFIRE3,
	S_REDZELDAFIRE4,

	S_BOWLINGPIN,
	S_BOWLINGHIT1,
	S_BOWLINGHIT2,
	S_BOWLINGHIT3,
	S_BOWLINGHIT4,

	S_ARIDTOAD,
	S_TOADHIT1,
	S_TOADHIT2,
	S_TOADHIT3,
	S_TOADHIT4,

	S_EBARRELIDLE,
	S_EBARREL1,
	S_EBARREL2,
	S_EBARREL3,
	S_EBARREL4,
	S_EBARREL5,
	S_EBARREL6,
	S_EBARREL7,
	S_EBARREL8,
	S_EBARREL9,
	S_EBARREL10,
	S_EBARREL11,
	S_EBARREL12,
	S_EBARREL13,
	S_EBARREL14,
	S_EBARREL15,
	S_EBARREL16,
	S_EBARREL17,
	S_EBARREL18,

	S_MERRYHORSE,

	S_BLUEFRUIT,
	S_ORANGEFRUIT,
	S_REDFRUIT,
	S_PINKFRUIT,

	S_ADVENTURESPIKEA1,
	S_ADVENTURESPIKEA2,
	S_ADVENTURESPIKEB1,
	S_ADVENTURESPIKEB2,
	S_ADVENTURESPIKEC1,
	S_ADVENTURESPIKEC2,

	S_BOOSTPROMPT1,
	S_BOOSTPROMPT2,

	S_BOOSTOFF1,
	S_BOOSTOFF2,

	S_BOOSTON1,
	S_BOOSTON2,

	S_LIZARDMAN,
	S_LIONMAN,

	S_KARMAFIREWORK1,
	S_KARMAFIREWORK2,
	S_KARMAFIREWORK3,
	S_KARMAFIREWORK4,
	S_KARMAFIREWORKTRAIL,

	S_OPAQUESMOKE1,
	S_OPAQUESMOKE2,
	S_OPAQUESMOKE3,
	S_OPAQUESMOKE4,
	S_OPAQUESMOKE5,

#ifdef SEENAMES
	S_NAMECHECK,
#endif

	S_FIRSTFREESLOT,
	S_LASTFREESLOT = S_FIRSTFREESLOT + NUMSTATEFREESLOTS - 1,
	NUMSTATES
} statenum_t;

typedef struct
{
	spritenum_t sprite;
	UINT32 frame; // we use the upper 16 bits for translucency and other shade effects
	INT32 tics;
	actionf_t action;
	INT32 var1;
	INT32 var2;
	statenum_t nextstate;
} state_t;

extern state_t states[NUMSTATES];
extern char sprnames[NUMSPRITES + 1][5];
extern state_t *astate;

typedef enum mobj_type
{
	MT_NULL,
	MT_UNKNOWN,

	MT_THOK, // Thok! mobj
	MT_PLAYER,

	// Enemies
	MT_BLUECRAWLA,
	MT_REDCRAWLA,
	MT_GFZFISH, // Greenflower Fish
	MT_GOLDBUZZ,
	MT_REDBUZZ,
	MT_AQUABUZZ, // AquaBuzz for ATZ
	MT_JETTBOMBER, // Jetty-Syn Bomber
	MT_JETTGUNNER, // Jetty-Syn Gunner
	MT_CRAWLACOMMANDER, // Crawla Commander
	MT_DETON, // Deton
	MT_SKIM, // Skim mine dropper
	MT_TURRET,
	MT_POPUPTURRET,
	MT_SHARP, // Sharp
	MT_JETJAW, // Jet Jaw
	MT_SNAILER, // Snailer
	MT_VULTURE, // Vulture
	MT_POINTY, // Pointy
	MT_POINTYBALL, // Pointy Ball
	MT_ROBOHOOD, // Robo-Hood
	MT_FACESTABBER, // CastleBot FaceStabber
	MT_EGGGUARD, // Egg Guard
	MT_EGGSHIELD, // Egg Shield for Egg Guard
	MT_GSNAPPER, // Green Snapper
	MT_MINUS, // Minus
	MT_SPRINGSHELL, // Spring Shell
	MT_YELLOWSHELL, // Spring Shell (yellow)
	MT_UNIDUS, // Unidus
	MT_UNIBALL, // Unidus Ball

	// Generic Boss Items
	MT_BOSSEXPLODE,
	MT_SONIC3KBOSSEXPLODE,
	MT_BOSSFLYPOINT,
	MT_EGGTRAP,
	MT_BOSS3WAYPOINT,
	MT_BOSS9GATHERPOINT,

	// Boss 1
	MT_EGGMOBILE,
	MT_JETFUME1,
	MT_EGGMOBILE_BALL,
	MT_EGGMOBILE_TARGET,
	MT_EGGMOBILE_FIRE,

	// Boss 2
	MT_EGGMOBILE2,
	MT_EGGMOBILE2_POGO,
	MT_BOSSTANK1,
	MT_BOSSTANK2,
	MT_BOSSSPIGOT,
	MT_GOOP,

	// Boss 3
	MT_EGGMOBILE3,
	MT_PROPELLER,
	MT_FAKEMOBILE,

	// Boss 4
	MT_EGGMOBILE4,
	MT_EGGMOBILE4_MACE,
	MT_JETFLAME,

	// Black Eggman (Boss 7)
	MT_BLACKEGGMAN,
	MT_BLACKEGGMAN_HELPER,
	MT_BLACKEGGMAN_GOOPFIRE,
	MT_BLACKEGGMAN_MISSILE,

	// New Very-Last-Minute 2.1 Brak Eggman (Cy-Brak-demon)
	MT_CYBRAKDEMON,
	MT_CYBRAKDEMON_ELECTRIC_BARRIER,
	MT_CYBRAKDEMON_MISSILE,
	MT_CYBRAKDEMON_FLAMESHOT,
	MT_CYBRAKDEMON_FLAMEREST,
	MT_CYBRAKDEMON_TARGET_RETICULE,
	MT_CYBRAKDEMON_TARGET_DOT,
	MT_CYBRAKDEMON_NAPALM_BOMB_LARGE,
	MT_CYBRAKDEMON_NAPALM_BOMB_SMALL,
	MT_CYBRAKDEMON_NAPALM_FLAMES,
	MT_CYBRAKDEMON_VILE_EXPLOSION,

	// Metal Sonic (Boss 9)
	MT_METALSONIC_RACE,
	MT_METALSONIC_BATTLE,
	MT_MSSHIELD_FRONT,
	MT_MSGATHER,

	// Collectible Items
	MT_RING,
	MT_FLINGRING, // Lost ring
	MT_BLUEBALL,  // Blue sphere replacement for special stages
	MT_REDTEAMRING,  //Rings collectable by red team.
	MT_BLUETEAMRING, //Rings collectable by blue team.
	MT_EMMY, // emerald token for special stage
	MT_TOKEN, // Special Stage Token (uncollectible part)
	MT_REDFLAG, // Red CTF Flag
	MT_BLUEFLAG, // Blue CTF Flag
	MT_EMBLEM,
	MT_EMERALD1,
	MT_EMERALD2,
	MT_EMERALD3,
	MT_EMERALD4,
	MT_EMERALD5,
	MT_EMERALD6,
	MT_EMERALD7,
	MT_EMERHUNT, // Emerald Hunt
	MT_EMERALDSPAWN, // Emerald spawner w/ delay
	MT_FLINGEMERALD, // Lost emerald

	// Springs and others
	MT_FAN,
	MT_STEAM, // Steam riser
	MT_BLUESPRING,
	MT_YELLOWSPRING,
	MT_REDSPRING,
	MT_YELLOWDIAG, // Yellow Diagonal Spring
	MT_REDDIAG, // Red Diagonal Spring

	// Interactive Objects
	MT_BUBBLES, // Bubble source
	MT_SIGN, // Level end sign
	MT_SPIKEBALL, // Spike Ball
	MT_SPECIALSPIKEBALL,
	MT_SPINFIRE,
	MT_SPIKE,
	MT_STARPOST,
	MT_BIGMINE,
	MT_BIGAIRMINE,
	MT_CANNONLAUNCHER,

	// Monitor Boxes
	MT_SUPERRINGBOX,
	MT_REDRINGBOX,
	MT_BLUERINGBOX,
	MT_SNEAKERTV,
	MT_INV,
	MT_PRUP, // 1up Box
	MT_YELLOWTV,
	MT_BLUETV,
	MT_BLACKTV, // Bomb shield TV
	MT_WHITETV, // Jump shield TV
	MT_GREENTV,
	MT_PITYTV, // Pity Shield TV
	MT_EGGMANBOX,
	MT_MIXUPBOX,
	MT_RECYCLETV,
	MT_RECYCLEICO,
	MT_QUESTIONBOX,
	MT_GRAVITYBOX,
	MT_SCORETVSMALL,
	MT_SCORETVLARGE,

	// Monitor miscellany
	MT_MONITOREXPLOSION,
	MT_REDMONITOREXPLOSION,
	MT_BLUEMONITOREXPLOSION,
	MT_RINGICO,
	MT_SHOESICO,
	MT_INVCICO,
	MT_1UPICO,
	MT_YSHIELDICO,
	MT_BSHIELDICO,
	MT_KSHIELDICO,
	MT_WSHIELDICO,
	MT_GSHIELDICO,
	MT_PITYSHIELDICO,
	MT_EGGMANICO,
	MT_MIXUPICO,
	MT_GRAVITYICO,
	MT_SCOREICOSMALL,
	MT_SCOREICOLARGE,

	// Projectiles
	MT_ROCKET,
	MT_LASER,
	MT_TORPEDO,
	MT_TORPEDO2, // silent
	MT_ENERGYBALL,
	MT_MINE, // Skim/Jetty-Syn mine
	MT_JETTBULLET, // Jetty-Syn Bullet
	MT_TURRETLASER,
	MT_CANNONBALL, // Cannonball
	MT_CANNONBALLDECOR, // Decorative/still cannonball
	MT_ARROW, // Arrow
	MT_DEMONFIRE, // Trapgoyle fire

	// Greenflower Scenery
	MT_GFZFLOWER1,
	MT_GFZFLOWER2,
	MT_GFZFLOWER3,
	MT_BERRYBUSH,
	MT_BUSH,

	// Techno Hill Scenery
	MT_THZPLANT, // THZ Plant
	MT_ALARM,

	// Deep Sea Scenery
	MT_GARGOYLE, // Deep Sea Gargoyle
	MT_SEAWEED, // DSZ Seaweed
	MT_WATERDRIP, // Dripping Water source
	MT_WATERDROP, // Water drop from dripping water
	MT_CORAL1, // Coral 1
	MT_CORAL2, // Coral 2
	MT_CORAL3, // Coral 3
	MT_BLUECRYSTAL, // Blue Crystal

	// Castle Eggman Scenery
	MT_CHAIN, // CEZ Chain
	MT_FLAME, // Flame
	MT_EGGSTATUE, // Eggman Statue
	MT_MACEPOINT, // Mace rotation point
	MT_SWINGMACEPOINT, // Mace swinging point
	MT_HANGMACEPOINT, // Hangable mace chain
	MT_SPINMACEPOINT, // Spin/Controllable mace chain
	MT_HIDDEN_SLING, // Spin mace chain (activatable)
	MT_SMALLMACECHAIN, // Small Mace Chain
	MT_BIGMACECHAIN, // Big Mace Chain
	MT_SMALLMACE, // Small Mace
	MT_BIGMACE, // Big Mace
	MT_CEZFLOWER,

	// Arid Canyon Scenery
	MT_BIGTUMBLEWEED,
	MT_LITTLETUMBLEWEED,
	MT_CACTI1,
	MT_CACTI2,
	MT_CACTI3,
	MT_CACTI4,

	// Red Volcano Scenery
	MT_FLAMEJET,
	MT_VERTICALFLAMEJET,
	MT_FLAMEJETFLAME,

	MT_FJSPINAXISA, // Counter-clockwise
	MT_FJSPINHELPERA,
	MT_FJSPINAXISB, // Clockwise
	MT_FJSPINHELPERB,

	MT_FLAMEJETFLAMEB, // Blade's flame

	// Dark City Scenery

	// Egg Rock Scenery

	// Azure Temple Scenery
	MT_TRAPGOYLE,
	MT_TRAPGOYLEUP,
	MT_TRAPGOYLEDOWN,
	MT_TRAPGOYLELONG,
	MT_TARGET, // AKA Red Crystal

	// Stalagmites
	MT_STALAGMITE0,
	MT_STALAGMITE1,
	MT_STALAGMITE2,
	MT_STALAGMITE3,
	MT_STALAGMITE4,
	MT_STALAGMITE5,
	MT_STALAGMITE6,
	MT_STALAGMITE7,
	MT_STALAGMITE8,
	MT_STALAGMITE9,

	// Christmas Scenery
	MT_XMASPOLE,
	MT_CANDYCANE,
	MT_SNOWMAN,
	MT_SNOWMANHAT,
	MT_LAMPPOST1,
	MT_LAMPPOST2,
	MT_HANGSTAR,

	// Botanic Serenity scenery
	MT_BSZTALLFLOWER_RED,
	MT_BSZTALLFLOWER_PURPLE,
	MT_BSZTALLFLOWER_BLUE,
	MT_BSZTALLFLOWER_CYAN,
	MT_BSZTALLFLOWER_YELLOW,
	MT_BSZTALLFLOWER_ORANGE,
	MT_BSZFLOWER_RED,
	MT_BSZFLOWER_PURPLE,
	MT_BSZFLOWER_BLUE,
	MT_BSZFLOWER_CYAN,
	MT_BSZFLOWER_YELLOW,
	MT_BSZFLOWER_ORANGE,
	MT_BSZSHORTFLOWER_RED,
	MT_BSZSHORTFLOWER_PURPLE,
	MT_BSZSHORTFLOWER_BLUE,
	MT_BSZSHORTFLOWER_CYAN,
	MT_BSZSHORTFLOWER_YELLOW,
	MT_BSZSHORTFLOWER_ORANGE,
	MT_BSZTULIP_RED,
	MT_BSZTULIP_PURPLE,
	MT_BSZTULIP_BLUE,
	MT_BSZTULIP_CYAN,
	MT_BSZTULIP_YELLOW,
	MT_BSZTULIP_ORANGE,
	MT_BSZCLUSTER_RED,
	MT_BSZCLUSTER_PURPLE,
	MT_BSZCLUSTER_BLUE,
	MT_BSZCLUSTER_CYAN,
	MT_BSZCLUSTER_YELLOW,
	MT_BSZCLUSTER_ORANGE,
	MT_BSZBUSH_RED,
	MT_BSZBUSH_PURPLE,
	MT_BSZBUSH_BLUE,
	MT_BSZBUSH_CYAN,
	MT_BSZBUSH_YELLOW,
	MT_BSZBUSH_ORANGE,
	MT_BSZVINE_RED,
	MT_BSZVINE_PURPLE,
	MT_BSZVINE_BLUE,
	MT_BSZVINE_CYAN,
	MT_BSZVINE_YELLOW,
	MT_BSZVINE_ORANGE,
	MT_BSZSHRUB,
	MT_BSZCLOVER,
	MT_BSZFISH,
	MT_BSZSUNFLOWER,

	// Misc scenery
	MT_DBALL,
	MT_EGGSTATUE2,

	// Powerup Indicators
	MT_GREENORB, // Elemental shield mobj
	MT_YELLOWORB, // Attract shield mobj
	MT_BLUEORB, // Force shield mobj
	MT_BLACKORB, // Armageddon shield mobj
	MT_WHITEORB, // Whirlwind shield mobj
	MT_PITYORB, // Pity shield mobj
	MT_IVSP, // invincibility sparkles
	MT_SUPERSPARK, // Super Sonic Spark

	// Freed Animals
	MT_BIRD, // Birdie freed!
	MT_BUNNY, // Bunny freed!
	MT_MOUSE, // Mouse
	MT_CHICKEN, // Chicken
	MT_COW, // Cow
	MT_REDBIRD, // Red Birdie in Bubble

	// Environmental Effects
	MT_RAIN, // Rain
	MT_SNOWFLAKE, // Snowflake
	MT_SPLISH, // Water splish!
	MT_SMOKE,
	MT_SMALLBUBBLE, // small bubble
	MT_MEDIUMBUBBLE, // medium bubble
	MT_EXTRALARGEBUBBLE, // extra large bubble
	MT_TFOG,
	MT_SEED,
	MT_PARTICLE,
	MT_PARTICLEGEN, // For fans, etc.

	// Game Indicators
	MT_SCORE, // score logo
	MT_DROWNNUMBERS, // Drowning Timer
	MT_GOTEMERALD, // Chaos Emerald (intangible)
	MT_TAG, // Tag Sign
	MT_GOTFLAG, // Got Flag sign
	MT_GOTFLAG2, // Got Flag sign

	// Ambient Sounds
	MT_AWATERA, // Ambient Water Sound 1
	MT_AWATERB, // Ambient Water Sound 2
	MT_AWATERC, // Ambient Water Sound 3
	MT_AWATERD, // Ambient Water Sound 4
	MT_AWATERE, // Ambient Water Sound 5
	MT_AWATERF, // Ambient Water Sound 6
	MT_AWATERG, // Ambient Water Sound 7
	MT_AWATERH, // Ambient Water Sound 8
	MT_RANDOMAMBIENT,
	MT_RANDOMAMBIENT2,

	// Ring Weapons
	MT_REDRING,
	MT_BOUNCERING,
	MT_RAILRING,
	MT_INFINITYRING,
	MT_AUTOMATICRING,
	MT_EXPLOSIONRING,
	MT_SCATTERRING,
	MT_GRENADERING,

	MT_BOUNCEPICKUP,
	MT_RAILPICKUP,
	MT_AUTOPICKUP,
	MT_EXPLODEPICKUP,
	MT_SCATTERPICKUP,
	MT_GRENADEPICKUP,

	MT_THROWNBOUNCE,
	MT_THROWNINFINITY,
	MT_THROWNAUTOMATIC,
	MT_THROWNSCATTER,
	MT_THROWNEXPLOSION,
	MT_THROWNGRENADE,

	// Mario-specific stuff
	MT_COIN,
	MT_FLINGCOIN,
	MT_GOOMBA,
	MT_BLUEGOOMBA,
	MT_FIREFLOWER,
	MT_FIREBALL,
	MT_SHELL,
	MT_PUMA,
	MT_HAMMER,
	MT_KOOPA,
	MT_KOOPAFLAME,
	MT_AXE,
	MT_MARIOBUSH1,
	MT_MARIOBUSH2,
	MT_TOAD,

	// NiGHTS Stuff
	MT_AXIS,
	MT_AXISTRANSFER,
	MT_AXISTRANSFERLINE,
	MT_NIGHTSDRONE,
	MT_NIGHTSGOAL,
	MT_NIGHTSCHAR,
	MT_NIGHTSPARKLE,
	MT_NIGHTSLOOPHELPER,
	MT_NIGHTSBUMPER, // NiGHTS Bumper
	MT_HOOP,
	MT_HOOPCOLLIDE, // Collision detection for NiGHTS hoops
	MT_HOOPCENTER, // Center of a hoop
	MT_NIGHTSCORE,
	MT_NIGHTSWING,
	MT_NIGHTSSUPERLOOP,
	MT_NIGHTSDRILLREFILL,
	MT_NIGHTSHELPER,
	MT_NIGHTSEXTRATIME,
	MT_NIGHTSLINKFREEZE,
	MT_EGGCAPSULE,
	MT_NIGHTOPIANHELPER, // the actual helper object that orbits you

	// Utility Objects
	MT_TELEPORTMAN,
	MT_ALTVIEWMAN,
	MT_CRUMBLEOBJ, // Sound generator for crumbling platform
	MT_TUBEWAYPOINT,
	MT_PUSH,
	MT_PULL,
	MT_GHOST,
	MT_OVERLAY,
	MT_POLYANCHOR,
	MT_POLYSPAWN,
	MT_POLYSPAWNCRUSH,

	// Skybox objects
	MT_SKYBOX,

	// Debris
	MT_SPARK, //spark
	MT_EXPLODE, // Robot Explosion
	MT_UWEXPLODE, // Underwater Explosion
	MT_ROCKSPAWNER,
	MT_FALLINGROCK,
	MT_ROCKCRUMBLE1,
	MT_ROCKCRUMBLE2,
	MT_ROCKCRUMBLE3,
	MT_ROCKCRUMBLE4,
	MT_ROCKCRUMBLE5,
	MT_ROCKCRUMBLE6,
	MT_ROCKCRUMBLE7,
	MT_ROCKCRUMBLE8,
	MT_ROCKCRUMBLE9,
	MT_ROCKCRUMBLE10,
	MT_ROCKCRUMBLE11,
	MT_ROCKCRUMBLE12,
	MT_ROCKCRUMBLE13,
	MT_ROCKCRUMBLE14,
	MT_ROCKCRUMBLE15,
	MT_ROCKCRUMBLE16,

	MT_SRB1_CRAWLA,
	MT_SRB1_BAT,
	MT_SRB1_ROBOFISH,
	MT_SRB1_VOLCANOGUY,
	MT_SRB1_HOPPY,
	MT_SRB1_HOPPYWATER,
	MT_SRB1_HOPPYSKYLAB,
	MT_SRB1_MMZFLYING,
	MT_SRB1_UFO,
	MT_SRB1_GRAYBOT,
	MT_SRB1_ROBOTOPOLIS,
	MT_SRB1_RBZBUZZ,
	MT_SRB1_RBZSPIKES,
	MT_SRB1_METALSONIC,
	MT_SRB1_GOLDBOT,
	MT_SRB1_GENREX,

	// SRB2kart
	MT_GRAYSPRING,
	MT_INVISSPRING,
	MT_BLUEDIAG,
	MT_RANDOMITEM,
	MT_RANDOMITEMPOP,
	MT_FLOATINGITEM,

	MT_SIGNSPARKLE,

	MT_FASTLINE,
	MT_FASTDUST,
	MT_BOOSTFLAME,
	MT_BOOSTSMOKE,
	MT_SNEAKERTRAIL,
	MT_AIZDRIFTSTRAT,
	MT_SPARKLETRAIL,
	MT_INVULNFLASH,
	MT_WIPEOUTTRAIL,
	MT_DRIFTSPARK,
	MT_BRAKEDRIFT,
	MT_DRIFTDUST,

	MT_ROCKETSNEAKER,

	MT_EGGMANITEM, // Eggman items
	MT_EGGMANITEM_SHIELD,

	MT_BANANA, // Banana Stuff
	MT_BANANA_SHIELD,

	MT_ORBINAUT, // Orbinaut stuff
	MT_ORBINAUT_SHIELD,

	MT_JAWZ, // Jawz stuff
	MT_JAWZ_DUD,
	MT_JAWZ_SHIELD,

	MT_PLAYERRETICULE, // Jawz reticule

	MT_SSMINE, // Mine stuff
	MT_SSMINE_SHIELD,
	MT_MINEEXPLOSION,
	MT_MINEEXPLOSIONSOUND,

	MT_SMOLDERING, // New explosion
	MT_BOOMEXPLODE,
	MT_BOOMPARTICLE,

	MT_BALLHOG, // Ballhog
	MT_BALLHOGBOOM,

	MT_SPB, // SPB stuff
	MT_SPBEXPLOSION,

	MT_THUNDERSHIELD, // Thunder Shield stuff

	MT_SINK, // Kitchen Sink Stuff
	MT_SINK_SHIELD,
	MT_SINKTRAIL,

	MT_BATTLEBUMPER, // Battle Mode bumpers

	MT_DEZLASER,

	MT_WAYPOINT,

	MT_RANDOMAUDIENCE,

	MT_FLAYM,
	MT_DEVIL,
	MT_ANGEL,
	MT_PALMTREE,
	MT_FLAG,
	MT_HEDGEHOG,
	MT_BUSH1,
	MT_TWEE,
	MT_HYDRANT,

	MT_BIGPUMA,
	MT_APPLE,

	MT_DOOD_FLOWER1,
	MT_DOOD_FLOWER2,
	MT_DOOD_FLOWER3,
	MT_DOOD_FLOWER4,
	MT_DOOD_BOX,
	MT_DOOD_BALLOON,
	MT_BIGRING,

	MT_SNES_DONUTBUSH1,
	MT_SNES_DONUTBUSH2,
	MT_SNES_DONUTBUSH3,

	MT_GBA_BOO,

	MT_BUZZBOMBER,
	MT_CHOMPER,
	MT_PALMTREE2,
	MT_PURPLEFLOWER1,
	MT_PURPLEFLOWER2,
	MT_YELLOWFLOWER1,
	MT_YELLOWFLOWER2,
	MT_PLANT2,
	MT_PLANT3,
	MT_PLANT4,

	MT_SKULL,
	MT_PHANTREE,
	MT_FLYINGGARG,
	MT_LAMPPOST,
	MT_MOSSYTREE,

	MT_SHADOW,

	MT_BUMP,

	MT_FLINGENERGY,

	MT_ITEMCLASH,

	MT_FIREDITEM,

	MT_INSTASHIELDA,
	MT_INSTASHIELDB,

	MT_PLAYERARROW,
	MT_PLAYERWANTED,

	MT_KARMAHITBOX,
	MT_KARMAWHEEL,

	MT_BATTLEPOINT,

	MT_FZEROBOOM,

	// Various plants
	MT_SONICBUSH,

	// Marble Zone
	MT_FLAMEPARTICLE,
	MT_MARBLETORCH,
	MT_MARBLELIGHT,
	MT_MARBLEBURNER,

	// CD Special Stage
	MT_CDUFO,

	// Rusty Rig
	MT_RUSTYLAMP_ORANGE,
	MT_RUSTYCHAIN,

	// D2 Balloon Panic
	MT_BALLOON,

	// Smokin' & Vapin' (Don't try this at home, kids!)
	MT_PETSMOKER,
	MT_PETSMOKE,
	MT_VVVAPE,

	// Hill Top Zone
	MT_HTZTREE,
	MT_HTZBUSH,

	// Ports of gardens
	MT_SGVINE1,
	MT_SGVINE2,
	MT_SGVINE3,
	MT_PGTREE,
	MT_PGFLOWER1,
	MT_PGFLOWER2,
	MT_PGFLOWER3,
	MT_PGBUSH,
	MT_DHPILLAR,

	// Midnight Channel stuff:
	MT_SPOTLIGHT,		// Spotlight Object
	MT_RANDOMSHADOW,	// Random static Shadows.
	MT_ROAMINGSHADOW,	// Roaming Shadows.
	MT_MAYONAKAARROW,	// Arrow static signs for Mayonaka

	// Mementos stuff
	MT_REAPERWAYPOINT,
	MT_REAPER,
	MT_MEMENTOSTP,
	MT_MEMENTOSPARTICLE,

	MT_JACKINTHEBOX,

	// Color Drive:
	MT_CDMOON,
	MT_CDBUSH,
	MT_CDTREEA,
	MT_CDTREEB,

	// Daytona Speedway
	MT_PINETREE,
	MT_PINETREE_SIDE,

	// Egg Zeppelin
	MT_EZZPROPELLER,
	MT_EZZPROPELLER_BLADE,

	// Desert Palace
	MT_DP_PALMTREE,

	// Aurora Atoll
	MT_AAZTREE_HELPER,
	MT_AAZTREE_SEG,
	MT_AAZTREE_COCONUT,
	MT_AAZTREE_LEAF,

	// Barren Badlands
	MT_BBZDUST,
	MT_FROGGER,
	MT_FROGTONGUE,
	MT_FROGTONGUE_JOINT,
	MT_ROBRA,
	MT_ROBRA_HEAD,
	MT_ROBRA_JOINT,
	MT_BLUEROBRA,
	MT_BLUEROBRA_HEAD,
	MT_BLUEROBRA_JOINT,

	// Eerie Grove
	MT_EERIEFOG,
	MT_EERIEFOGGEN,

	// SMK ports
	MT_SMK_PIPE,
	MT_SMK_MOLESPAWNER,
	MT_SMK_MOLE,
	MT_SMK_THWOMP,
	MT_SMK_SNOWBALL,
	MT_SMK_ICEBLOCK,
	MT_SMK_ICEBLOCK_SIDE,
	MT_SMK_ICEBLOCK_DEBRIS,

	// Ezo's maps
	MT_BLUEFIRE,
	MT_GREENFIRE,
	MT_REGALCHEST,
	MT_CHIMERASTATUE,
	MT_DRAGONSTATUE,
	MT_LIZARDMANSTATUE,
	MT_PEGASUSSTATUE,
	MT_ZELDAFIRE,
	MT_GANBARETHING,
	MT_GANBAREDUCK,
	MT_GANBARETREE,
	MT_MONOKUMA,
	MT_REDZELDAFIRE,
	MT_BOWLINGPIN,
	MT_MERRYAMBIENCE,
	MT_TWINKLECARTAMBIENCE,
	MT_EXPLODINGBARREL,
	MT_MERRYHORSE,
	MT_ARIDTOAD,
	MT_BLUEFRUIT,
	MT_ORANGEFRUIT,
	MT_REDFRUIT,
	MT_PINKFRUIT,
	MT_ADVENTURESPIKEA,
	MT_ADVENTURESPIKEB,
	MT_ADVENTURESPIKEC,
	MT_BOOSTPROMPT,
	MT_BOOSTOFF,
	MT_BOOSTON,
	MT_LIZARDMAN,
	MT_LIONMAN,

	MT_KARMAFIREWORK,

#ifdef SEENAMES
	MT_NAMECHECK,
#endif

	MT_FIRSTFREESLOT,
	MT_LASTFREESLOT = MT_FIRSTFREESLOT + NUMMOBJFREESLOTS - 1,
	NUMMOBJTYPES
} mobjtype_t;

typedef struct
{
	INT32 doomednum;
	statenum_t spawnstate;
	INT32 spawnhealth;
	statenum_t seestate;
	sfxenum_t seesound;
	INT32 reactiontime;
	sfxenum_t attacksound;
	statenum_t painstate;
	INT32 painchance;
	sfxenum_t painsound;
	statenum_t meleestate;
	statenum_t missilestate;
	statenum_t deathstate;
	statenum_t xdeathstate;
	sfxenum_t deathsound;
	fixed_t speed;
	fixed_t radius;
	fixed_t height;
	INT32 dispoffset;
	INT32 mass;
	INT32 damage;
	sfxenum_t activesound;
	UINT32 flags;
	statenum_t raisestate;
} mobjinfo_t;

extern mobjinfo_t mobjinfo[NUMMOBJTYPES];

void P_PatchInfoTables(void);

void P_BackupTables(void);

void P_ResetData(INT32 flags);

#endif

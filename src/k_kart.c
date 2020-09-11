// SONIC ROBO BLAST 2 KART ~ ZarroTsu
//-----------------------------------------------------------------------------
/// \file  k_kart.c
/// \brief SRB2kart general.
///        All of the SRB2kart-unique stuff.

#include "doomdef.h"
#include "hu_stuff.h"
#include "g_game.h"
#include "m_random.h"
#include "p_local.h"
#include "p_slopes.h"
#include "r_draw.h"
#include "r_local.h"
#include "s_sound.h"
#include "st_stuff.h"
#include "v_video.h"
#include "z_zone.h"
#include "m_misc.h"
#include "m_cond.h"
#include "k_kart.h"
#include "f_finale.h"
#include "lua_hud.h"	// For Lua hud checks
#include "lua_hook.h"	// For MobjDamage and ShouldDamage

// SOME IMPORTANT VARIABLES DEFINED IN DOOMDEF.H:
// gamespeed is cc (0 for easy, 1 for normal, 2 for hard)
// franticitems is Frantic Mode items, bool
// encoremode is Encore Mode (duh), bool
// comeback is Battle Mode's karma comeback, also bool
// battlewanted is an array of the WANTED player nums, -1 for no player in that slot
// indirectitemcooldown is timer before anyone's allowed another Shrink/SPB
// mapreset is set when enough players fill an empty server
// nospectategrief is the players in-game needed to eliminate the person in last


//{ SRB2kart Color Code

#define SKIN_RAMP_LENGTH 16
#define DEFAULT_STARTTRANSCOLOR 160
#define NUM_PALETTE_ENTRIES 256

// These should be within 14 characters to fit on the character select screen
const char *KartColor_Names[MAXSKINCOLORS] =
{
	"None",           // SKINCOLOR_NONE
	"White",          // SKINCOLOR_WHITE
	"Silver",         // SKINCOLOR_SILVER
	"Grey",           // SKINCOLOR_GREY
	"Nickel",         // SKINCOLOR_NICKEL
	"Black",          // SKINCOLOR_BLACK
	"Skunk",          // SKINCOLOR_SKUNK
	"Fairy",          // SKINCOLOR_FAIRY
	"Popcorn",        // SKINCOLOR_POPCORN
	"Artichoke",      // SKINCOLOR_ARTICHOKE
	"Pigeon",         // SKINCOLOR_PIGEON
	"Sepia",          // SKINCOLOR_SEPIA
	"Beige",          // SKINCOLOR_BEIGE
	"Walnut",         // SKINCOLOR_WALNUT
	"Brown",          // SKINCOLOR_BROWN
	"Leather",        // SKINCOLOR_LEATHER
	"Salmon",         // SKINCOLOR_SALMON
	"Pink",           // SKINCOLOR_PINK
	"Rose",           // SKINCOLOR_ROSE
	"Brick",          // SKINCOLOR_BRICK
	"Cinnamon",       // SKINCOLOR_CINNAMON
	"Ruby",           // SKINCOLOR_RUBY
	"Raspberry",      // SKINCOLOR_RASPBERRY
	"Cherry",         // SKINCOLOR_CHERRY
	"Red",            // SKINCOLOR_RED
	"Crimson",        // SKINCOLOR_CRIMSON
	"Maroon",         // SKINCOLOR_MAROON
	"Lemonade",       // SKINCOLOR_LEMONADE
	"Flame",          // SKINCOLOR_FLAME
	"Scarlet",        // SKINCOLOR_SCARLET
	"Ketchup",        // SKINCOLOR_KETCHUP
	"Dawn",           // SKINCOLOR_DAWN
	"Sunset",         // SKINCOLOR_SUNSET
	"Creamsicle",     // SKINCOLOR_CREAMSICLE
	"Orange",         // SKINCOLOR_ORANGE
	"Pumpkin",        // SKINCOLOR_PUMPKIN
	"Rosewood",       // SKINCOLOR_ROSEWOOD
	"Burgundy",       // SKINCOLOR_BURGUNDY
	"Tangerine",      // SKINCOLOR_TANGERINE
	"Peach",          // SKINCOLOR_PEACH
	"Caramel",        // SKINCOLOR_CARAMEL
	"Cream",          // SKINCOLOR_CREAM
	"Gold",           // SKINCOLOR_GOLD
	"Royal",          // SKINCOLOR_ROYAL
	"Bronze",         // SKINCOLOR_BRONZE
	"Copper",         // SKINCOLOR_COPPER
	"Quarry",         // SKINCOLOR_QUARRY
	"Yellow",         // SKINCOLOR_YELLOW
	"Mustard",        // SKINCOLOR_MUSTARD
	"Crocodile",      // SKINCOLOR_CROCODILE
	"Olive",          // SKINCOLOR_OLIVE
	"Vomit",          // SKINCOLOR_VOMIT
	"Garden",         // SKINCOLOR_GARDEN
	"Lime",           // SKINCOLOR_LIME
	"Handheld",       // SKINCOLOR_HANDHELD
	"Tea",            // SKINCOLOR_TEA
	"Pistachio",      // SKINCOLOR_PISTACHIO
	"Moss",           // SKINCOLOR_MOSS
	"Camouflage",     // SKINCOLOR_CAMOUFLAGE
	"Robo-Hood",      // SKINCOLOR_ROBOHOOD
	"Mint",           // SKINCOLOR_MINT
	"Green",          // SKINCOLOR_GREEN
	"Pinetree",       // SKINCOLOR_PINETREE
	"Emerald",        // SKINCOLOR_EMERALD
	"Swamp",          // SKINCOLOR_SWAMP
	"Dream",          // SKINCOLOR_DREAM
	"Plague",         // SKINCOLOR_PLAGUE
	"Algae",          // SKINCOLOR_ALGAE
	"Caribbean",      // SKINCOLOR_CARIBBEAN
	"Azure",          // SKINCOLOR_AZURE
	"Aqua",           // SKINCOLOR_AQUA
	"Teal",           // SKINCOLOR_TEAL
	"Cyan",           // SKINCOLOR_CYAN
	"Jawz",           // SKINCOLOR_JAWZ
	"Cerulean",       // SKINCOLOR_CERULEAN
	"Navy",           // SKINCOLOR_NAVY
	"Platinum",       // SKINCOLOR_PLATINUM
	"Slate",          // SKINCOLOR_SLATE
	"Steel",          // SKINCOLOR_STEEL
	"Thunder",        // SKINCOLOR_THUNDER
	"Rust",           // SKINCOLOR_RUST
	"Wristwatch",     // SKINCOLOR_WRISTWATCH
	"Jet",            // SKINCOLOR_JET
	"Sapphire",       // SKINCOLOR_SAPPHIRE
	"Periwinkle",     // SKINCOLOR_PERIWINKLE
	"Blue",           // SKINCOLOR_BLUE
	"Blueberry",      // SKINCOLOR_BLUEBERRY
	"Nova",           // SKINCOLOR_NOVA
	"Pastel",         // SKINCOLOR_PASTEL
	"Moonslam",       // SKINCOLOR_MOONSLAM
	"Ultraviolet",    // SKINCOLOR_ULTRAVIOLET
	"Dusk",           // SKINCOLOR_DUSK
	"Bubblegum",      // SKINCOLOR_BUBBLEGUM
	"Purple",         // SKINCOLOR_PURPLE
	"Fuchsia",        // SKINCOLOR_FUCHSIA
	"Toxic",          // SKINCOLOR_TOXIC
	"Mauve",          // SKINCOLOR_MAUVE
	"Lavender",       // SKINCOLOR_LAVENDER
	"Byzantium",      // SKINCOLOR_BYZANTIUM
	"Pomegranate",    // SKINCOLOR_POMEGRANATE
	"Lilac"           // SKINCOLOR_LILAC
};

// Color_Opposite replacement; frame setting has not been changed from 8 for most, should be done later
const UINT8 KartColor_Opposite[MAXSKINCOLORS*2] =
{
	SKINCOLOR_NONE,8,         // SKINCOLOR_NONE
	SKINCOLOR_BLACK,8,        // SKINCOLOR_WHITE
	SKINCOLOR_NICKEL,8,       // SKINCOLOR_SILVER
	SKINCOLOR_GREY,8,         // SKINCOLOR_GREY
	SKINCOLOR_SILVER,8,       // SKINCOLOR_NICKEL
	SKINCOLOR_WHITE,8,        // SKINCOLOR_BLACK
	SKINCOLOR_SKUNK,8,        // SKINCOLOR_SKUNK
	SKINCOLOR_ARTICHOKE,12,   // SKINCOLOR_FAIRY
	SKINCOLOR_PIGEON,12,      // SKINCOLOR_POPCORN
	SKINCOLOR_FAIRY,12,       // SKINCOLOR_ARTICHOKE
	SKINCOLOR_POPCORN,12,     // SKINCOLOR_PIGEON
	SKINCOLOR_LEATHER,6,      // SKINCOLOR_SEPIA
	SKINCOLOR_BROWN,2,        // SKINCOLOR_BEIGE
	SKINCOLOR_CAMOUFLAGE,8,   // SKINCOLOR_WALNUT
	SKINCOLOR_BEIGE,8,        // SKINCOLOR_BROWN
	SKINCOLOR_SEPIA,8,        // SKINCOLOR_LEATHER
	SKINCOLOR_TEA,8,          // SKINCOLOR_SALMON
	SKINCOLOR_PISTACHIO,8,    // SKINCOLOR_PINK
	SKINCOLOR_MOSS,8,         // SKINCOLOR_ROSE
	SKINCOLOR_RUST,8,         // SKINCOLOR_BRICK
	SKINCOLOR_WRISTWATCH,6,   // SKINCOLOR_CINNAMON
	SKINCOLOR_SAPPHIRE,8,     // SKINCOLOR_RUBY
	SKINCOLOR_MINT,8,         // SKINCOLOR_RASPBERRY
	SKINCOLOR_HANDHELD,10,    // SKINCOLOR_CHERRY
	SKINCOLOR_GREEN,6,        // SKINCOLOR_RED
	SKINCOLOR_PINETREE,6,     // SKINCOLOR_CRIMSON
	SKINCOLOR_TOXIC,8,        // SKINCOLOR_MAROON
	SKINCOLOR_THUNDER,8,      // SKINCOLOR_LEMONADE
	SKINCOLOR_CARIBBEAN,10,   // SKINCOLOR_FLAME
	SKINCOLOR_ALGAE,10,       // SKINCOLOR_SCARLET
	SKINCOLOR_MUSTARD,10,     // SKINCOLOR_KETCHUP
	SKINCOLOR_DUSK,8,         // SKINCOLOR_DAWN
	SKINCOLOR_MOONSLAM,8,     // SKINCOLOR_SUNSET
	SKINCOLOR_PERIWINKLE,8,   // SKINCOLOR_CREAMSICLE
	SKINCOLOR_BLUE,8,         // SKINCOLOR_ORANGE
	SKINCOLOR_BLUEBERRY,8,    // SKINCOLOR_PUMPKIN
	SKINCOLOR_NAVY,6,         // SKINCOLOR_ROSEWOOD
	SKINCOLOR_JET,8,          // SKINCOLOR_BURGUNDY
	SKINCOLOR_LIME,8,         // SKINCOLOR_TANGERINE
	SKINCOLOR_CYAN,8,         // SKINCOLOR_PEACH
	SKINCOLOR_CERULEAN,8,     // SKINCOLOR_CARAMEL
	SKINCOLOR_COPPER,10,      // SKINCOLOR_CREAM
	SKINCOLOR_SLATE,8,        // SKINCOLOR_GOLD
	SKINCOLOR_PLATINUM,6,     // SKINCOLOR_ROYAL
	SKINCOLOR_STEEL,8,        // SKINCOLOR_BRONZE
	SKINCOLOR_CREAM,6,        // SKINCOLOR_COPPER
	SKINCOLOR_AZURE,8,        // SKINCOLOR_QUARRY
	SKINCOLOR_AQUA,8,         // SKINCOLOR_YELLOW
	SKINCOLOR_KETCHUP,8,      // SKINCOLOR_MUSTARD
	SKINCOLOR_BUBBLEGUM,8,    // SKINCOLOR_CROCODILE
	SKINCOLOR_TEAL,8,         // SKINCOLOR_OLIVE
	SKINCOLOR_ROBOHOOD,8,     // SKINCOLOR_VOMIT
	SKINCOLOR_LAVENDER,6,     // SKINCOLOR_GARDEN
	SKINCOLOR_TANGERINE,8,    // SKINCOLOR_LIME
	SKINCOLOR_CHERRY,8,       // SKINCOLOR_HANDHELD
	SKINCOLOR_SALMON,8,       // SKINCOLOR_TEA
	SKINCOLOR_PINK,6,         // SKINCOLOR_PISTACHIO
	SKINCOLOR_ROSE,8,         // SKINCOLOR_MOSS
	SKINCOLOR_WALNUT,8,       // SKINCOLOR_CAMOUFLAGE
	SKINCOLOR_VOMIT,8,        // SKINCOLOR_ROBOHOOD
	SKINCOLOR_RASPBERRY,8,    // SKINCOLOR_MINT
	SKINCOLOR_RED,8,          // SKINCOLOR_GREEN
	SKINCOLOR_CRIMSON,8,      // SKINCOLOR_PINETREE
	SKINCOLOR_PURPLE,8,       // SKINCOLOR_EMERALD
	SKINCOLOR_BYZANTIUM,8,    // SKINCOLOR_SWAMP
	SKINCOLOR_POMEGRANATE,8,  // SKINCOLOR_DREAM
	SKINCOLOR_NOVA,8,         // SKINCOLOR_PLAGUE
	SKINCOLOR_SCARLET,10,     // SKINCOLOR_ALGAE
	SKINCOLOR_FLAME,8,        // SKINCOLOR_CARIBBEAN
	SKINCOLOR_QUARRY,8,       // SKINCOLOR_AZURE
	SKINCOLOR_YELLOW,8,       // SKINCOLOR_AQUA
	SKINCOLOR_OLIVE,8,        // SKINCOLOR_TEAL
	SKINCOLOR_PEACH,8,        // SKINCOLOR_CYAN
	SKINCOLOR_LILAC,10,       // SKINCOLOR_JAWZ
	SKINCOLOR_CARAMEL,8,      // SKINCOLOR_CERULEAN
	SKINCOLOR_ROSEWOOD,8,     // SKINCOLOR_NAVY
	SKINCOLOR_ROYAL,8,        // SKINCOLOR_PLATINUM
	SKINCOLOR_GOLD,10,        // SKINCOLOR_SLATE
	SKINCOLOR_BRONZE,10,      // SKINCOLOR_STEEL
	SKINCOLOR_LEMONADE,8,     // SKINCOLOR_THUNDER
	SKINCOLOR_BRICK,10,       // SKINCOLOR_RUST
	SKINCOLOR_CINNAMON,8,     // SKINCOLOR_WRISTWATCH
	SKINCOLOR_BURGUNDY,8,     // SKINCOLOR_JET
	SKINCOLOR_RUBY,6,         // SKINCOLOR_SAPPHIRE
	SKINCOLOR_CREAMSICLE,8,   // SKINCOLOR_PERIWINKLE
	SKINCOLOR_ORANGE,8,       // SKINCOLOR_BLUE
	SKINCOLOR_PUMPKIN,8,      // SKINCOLOR_BLUEBERRY
	SKINCOLOR_PLAGUE,10,      // SKINCOLOR_NOVA
	SKINCOLOR_FUCHSIA,11,     // SKINCOLOR_PASTEL
	SKINCOLOR_SUNSET,10,      // SKINCOLOR_MOONSLAM
	SKINCOLOR_MAUVE,10,       // SKINCOLOR_ULTRAVIOLET
	SKINCOLOR_DAWN,6,         // SKINCOLOR_DUSK
	SKINCOLOR_CROCODILE,8,    // SKINCOLOR_BUBBLEGUM
	SKINCOLOR_EMERALD,8,      // SKINCOLOR_PURPLE
	SKINCOLOR_PASTEL,11,      // SKINCOLOR_FUCHSIA
	SKINCOLOR_MAROON,8,       // SKINCOLOR_TOXIC
	SKINCOLOR_ULTRAVIOLET,8,  // SKINCOLOR_MAUVE
	SKINCOLOR_GARDEN,6,       // SKINCOLOR_LAVENDER
	SKINCOLOR_SWAMP,8,        // SKINCOLOR_BYZANTIUM
	SKINCOLOR_DREAM,8,        // SKINCOLOR_POMEGRANATE
	SKINCOLOR_JAWZ,6          // SKINCOLOR_LILAC
};

UINT8 colortranslations[MAXTRANSLATIONS][16] = {
	{  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0}, // SKINCOLOR_NONE
	{120, 120, 120, 120,   0,   2,   5,   8,   9,  11,  14,  17,  20,  22,  25,  28}, // SKINCOLOR_WHITE
	{  0,   1,   2,   3,   5,   7,   9,  12,  13,  15,  18,  20,  23,  25,  27,  30}, // SKINCOLOR_SILVER
	{  1,   3,   5,   7,   9,  11,  13,  15,  17,  19,  21,  23,  25,  27,  29,  31}, // SKINCOLOR_GREY
	{  3,   5,   8,  11,  15,  17,  19,  21,  23,  24,  25,  26,  27,  29,  30,  31}, // SKINCOLOR_NICKEL
	{  4,   7,  11,  15,  20,  22,  24,  27,  28,  28,  28,  29,  29,  30,  30,  31}, // SKINCOLOR_BLACK
	{120, 120,   0,   2,   4,  10,  16,  22,  23,  24,  25,  26,  27,  28,  29,  31}, // SKINCOLOR_SKUNK
	{120, 120, 121, 121, 122, 123,  10,  14,  16,  18,  20,  22,  24,  26,  28,  31}, // SKINCOLOR_FAIRY
	{120,  96,  97,  98,  99,  71,  32,  11,  13,  16,  18,  21,  23,  26,  28,  31}, // SKINCOLOR_POPCORN
	{ 97, 176, 177, 162, 163, 179,  12,  14,  16,  18,  20,  22,  24,  26,  28,  31}, // SKINCOLOR_ARTICHOKE
	{  0, 208, 209, 211, 226, 202,  14,  15,  17,  19,  21,  23,  25,  27,  29,  31}, // SKINCOLOR_PIGEON
	{  0,   1,   3,   5,   7,   9,  34,  36,  38,  40,  42,  44,  60,  61,  62,  63}, // SKINCOLOR_SEPIA
	{120,  65,  67,  69,  32,  34,  36,  38,  40,  42,  44,  45,  46,  47,  62,  63}, // SKINCOLOR_BEIGE
	{  3,   6,  32,  33,  35,  37,  51,  52,  54,  55,  57,  58,  60,  61,  63,  30}, // SKINCOLOR_WALNUT
	{ 67,  70,  73,  76,  48,  49,  51,  53,  54,  56,  58,  59,  61,  63,  29,  30}, // SKINCOLOR_BROWN
	{ 72,  76,  48,  51,  53,  55,  57,  59,  61,  63,  28,  28,  29,  29,  30,  31}, // SKINCOLOR_LEATHER
	{120, 120, 120, 121, 121, 122, 123, 124, 126, 127, 129, 131, 133, 135, 137, 139}, // SKINCOLOR_SALMON
	{120, 121, 121, 122, 144, 145, 146, 147, 148, 149, 150, 151, 134, 136, 138, 140}, // SKINCOLOR_PINK
	{144, 145, 146, 147, 148, 149, 150, 151, 134, 135, 136, 137, 138, 139, 140, 141}, // SKINCOLOR_ROSE
	{ 64,  67,  70,  73, 146, 147, 148, 150, 118, 118, 119, 119, 156, 159, 141, 143}, // SKINCOLOR_BRICK
	{ 68,  75,  48,  50,  52,  94, 152, 136, 137, 138, 139, 140, 141, 142, 143,  31}, // SKINCOLOR_CINNAMON
	{120, 121, 144, 145, 147, 149, 132, 133, 134, 136, 198, 198, 199, 255,  30,  31}, // SKINCOLOR_RUBY
	{120, 121, 122, 123, 124, 125, 126, 127, 128, 130, 131, 134, 136, 137, 139, 140}, // SKINCOLOR_RASPBERRY
	{120,  65,  67,  69,  71, 124, 125, 127, 132, 133, 135, 136, 138, 139, 140, 141}, // SKINCOLOR_CHERRY
	{122, 123, 124, 126, 129, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142}, // SKINCOLOR_RED
	{123, 125, 128, 131, 133, 135, 136, 138, 140, 140, 141, 141, 142, 142, 143,  31}, // SKINCOLOR_CRIMSON
	{123, 124, 126, 128, 132, 135, 137,  27,  28,  28,  28,  29,  29,  30,  30,  31}, // SKINCOLOR_MAROON
	{120,  96,  97,  98,  99,  65, 122, 144, 123, 124, 147, 149, 151, 153, 156, 159}, // SKINCOLOR_LEMONADE
	{120,  97, 112, 113, 113,  85,  87, 126, 149, 150, 151, 252, 253, 254, 255,  29}, // SKINCOLOR_FLAME
	{ 99, 113, 113,  84,  85,  87, 126, 128, 130, 196, 197, 198, 199, 240, 243, 246}, // SKINCOLOR_SCARLET
	{103, 113, 113,  84,  85,  88, 127, 130, 131, 133, 134, 136, 138, 139, 141, 143}, // SKINCOLOR_KETCHUP
	{120, 121, 122, 123, 124, 147, 148,  91,  93,  95, 152, 154, 156, 159, 141, 143}, // SKINCOLOR_DAWN
	{ 98, 112, 113,  84,  85,  87,  89, 149, 150, 251, 251, 205, 206, 207,  29,  31}, // SKINCOLOR_SUNSET
	{120, 120,  80,  80,  81,  82,  83,  83,  84,  85,  86,  88,  89,  91,  93,  95}, // SKINCOLOR_CREAMSICLE
	{ 80,  81,  82,  83,  84,  85,  86,  88,  89,  91,  94,  95, 154, 156, 158, 159}, // SKINCOLOR_ORANGE
	{ 82,  83,  84,  85,  87,  89,  90,  92,  94, 152, 153, 155, 157, 159, 141, 142}, // SKINCOLOR_PUMPKIN
	{ 83,  85,  88,  90,  92,  94, 152, 153, 154, 156, 157, 159, 140, 141, 142, 143}, // SKINCOLOR_ROSEWOOD
	{ 84,  86,  89,  91, 152, 154, 155, 157, 158, 159, 140, 141, 142, 143,  31,  31}, // SKINCOLOR_BURGUNDY
	{ 98,  98, 112, 112, 113, 113,  84,  85,  87,  89,  91,  93,  95, 153, 156, 159}, // SKINCOLOR_TANGERINE
	{120,  80,  66,  70,  72,  76, 148, 149, 150, 151, 153, 154, 156,  61,  62,  63}, // SKINCOLOR_PEACH
	{ 64,  66,  68,  70,  72,  74,  76,  78,  48,  50,  52,  54,  56,  58,  60,  62}, // SKINCOLOR_CARAMEL
	{120,  96,  96,  97,  98,  82,  84,  77,  50,  54,  57,  59,  61,  63,  29,  31}, // SKINCOLOR_CREAM
	{ 96,  97,  98, 112, 113, 114, 115, 116, 117, 151, 118, 119, 157, 159, 140, 143}, // SKINCOLOR_GOLD
	{ 97, 112, 113, 113, 114,  78,  53, 252, 252, 253, 253, 254, 255,  29,  30,  31}, // SKINCOLOR_ROYAL
	{112, 113, 114, 115, 116, 117, 118, 119, 156, 157, 158, 159, 141, 141, 142, 143}, // SKINCOLOR_BRONZE
	{120,  99, 113, 114, 116, 117, 119,  61,  63,  28,  28,  29,  29,  30,  30,  31}, // SKINCOLOR_COPPER
	{ 96,  97,  98,  99, 104, 105, 106, 107, 117, 152, 154, 156, 159, 141, 142, 143}, // SKINCOLOR_QUARRY
	{ 96,  97,  98, 100, 101, 102, 104, 113, 114, 115, 116, 117, 118, 119, 156, 159}, // SKINCOLOR_YELLOW
	{ 96,  98,  99, 112, 113, 114, 114, 106, 106, 107, 107, 108, 108, 109, 110, 111}, // SKINCOLOR_MUSTARD
	{120,  96,  97,  98, 176, 113, 114, 106, 115, 107, 108, 109, 110, 174, 175,  31}, // SKINCOLOR_CROCODILE
	{ 98, 101, 104, 105, 106, 115, 107, 108, 182, 109, 183, 110, 174, 111,  30,  31}, // SKINCOLOR_OLIVE
	{  0, 121, 122, 144,  71,  84, 114, 115, 107, 108, 109, 183, 223, 207,  30, 246}, // SKINCOLOR_VOMIT
	{ 98,  99, 112, 101, 113, 114, 106, 179, 180, 180, 181, 182, 183, 173, 174, 175}, // SKINCOLOR_GARDEN
	{120,  96,  97,  98,  99, 176, 177, 163, 164, 166, 168, 170, 223, 207, 243,  31}, // SKINCOLOR_LIME
	{ 98, 104, 105, 105, 106, 167, 168, 169, 170, 171, 172, 173, 174, 175,  30,  31}, // SKINCOLOR_HANDHELD
	{120, 120, 176, 176, 176, 177, 177, 178, 178, 179, 179, 180, 180, 181, 182, 183}, // SKINCOLOR_TEA
	{120, 120, 176, 176, 177, 177, 178, 179, 165, 166, 167, 168, 169, 170, 171, 172}, // SKINCOLOR_PISTACHIO
	{178, 178, 178, 179, 179, 180, 181, 182, 183, 172, 172, 173, 173, 174, 174, 175}, // SKINCOLOR_MOSS
	{ 64,  66,  69,  32,  34,  37,  40, 182, 171, 172, 172, 173, 173, 174, 174, 175}, // SKINCOLOR_CAMOUFLAGE
	{120, 176, 160, 165, 167, 168, 169, 182, 182, 171,  60,  61,  63,  29,  30,  31}, // SKINCOLOR_ROBOHOOD
	{120, 176, 176, 176, 177, 163, 164, 165, 167, 221, 221, 222, 223, 207, 207,  31}, // SKINCOLOR_MINT
	{160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175}, // SKINCOLOR_GREEN
	{161, 163, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,  30,  30,  31}, // SKINCOLOR_PINETREE
	{160, 184, 184, 185, 185, 186, 186, 187, 187, 188, 188, 189, 189, 190, 191, 175}, // SKINCOLOR_EMERALD
	{160, 184, 185, 186, 187, 188, 189, 190, 191, 191,  29,  29,  30,  30,  31,  31}, // SKINCOLOR_SWAMP
	{120, 120,  80,  80,  81, 177, 162, 164, 228, 228, 204, 204, 205, 205, 206, 207}, // SKINCOLOR_DREAM
	{ 97, 176, 160, 184, 185, 186, 187, 229, 229, 205, 206, 207,  28,  29,  30,  31}, // SKINCOLOR_PLAGUE
	{208, 209, 210, 211, 213, 220, 216, 167, 168, 188, 188, 189, 190, 191,  30,  31}, // SKINCOLOR_ALGAE
	{120, 176, 177, 160, 185, 220, 216, 217, 229, 229, 204, 205, 206, 254, 255,  31}, // SKINCOLOR_CARIBBEAN
	{120,  96,  97,  98, 177, 220, 216, 217, 218, 204, 252, 253, 254, 255,  30,  31}, // SKINCOLOR_AZURE
	{120, 208, 208, 210, 212, 214, 220, 220, 220, 221, 221, 222, 222, 223, 223, 191}, // SKINCOLOR_AQUA
	{210, 213, 220, 220, 220, 216, 216, 221, 221, 221, 222, 222, 223, 223, 191,  31}, // SKINCOLOR_TEAL
	{120, 120, 208, 208, 209, 210, 211, 212, 213, 215, 216, 217, 218, 219, 222, 223}, // SKINCOLOR_CYAN
	{120, 120, 208, 209, 210, 226, 215, 216, 217, 229, 229, 205, 205, 206, 207,  31}, // SKINCOLOR_JAWZ
	{208, 209, 211, 213, 215, 216, 216, 217, 217, 218, 218, 219, 205, 206, 207, 207}, // SKINCOLOR_CERULEAN
	{211, 212, 213, 215, 216, 218, 219, 205, 206, 206, 207, 207,  28,  29,  30,  31}, // SKINCOLOR_NAVY
	{120,   0,   0, 200, 200, 201,  11,  14,  17, 218, 222, 223, 238, 240, 243, 246}, // SKINCOLOR_PLATINUM
	{120, 120, 200, 200, 200, 201, 201, 201, 202, 202, 202, 203, 204, 205, 206, 207}, // SKINCOLOR_SLATE
	{120, 200, 200, 201, 201, 202, 202, 203, 203, 204, 204, 205, 205, 206, 207,  31}, // SKINCOLOR_STEEL
	{ 96,  97,  98, 112, 113, 114,  11, 203, 204, 205, 205, 237, 239, 241, 243, 246}, // SKINCOLOR_THUNDER
	{ 64,  66,  68,  70,  32,  34,  36, 203, 204, 205,  24,  25,  26,  28,  29,  31}, // SKINCOLOR_RUST
	{ 81,  72,  76,  48,  51,  55, 252, 205, 205, 206, 240, 241, 242, 243, 244, 246}, // SKINCOLOR_WRISTWATCH
	{225, 226, 227, 228, 229, 205, 205, 206, 207, 207,  28,  28,  29,  29,  30,  31}, // SKINCOLOR_JET
	{208, 209, 211, 213, 215, 217, 229, 230, 232, 234, 236, 238, 240, 242, 244, 246}, // SKINCOLOR_SAPPHIRE
	{120, 120, 224, 225, 226, 202, 227, 228, 229, 230, 231, 233, 235, 237, 239, 241}, // SKINCOLOR_PERIWINKLE
	{224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 235, 236, 238, 242, 244, 246}, // SKINCOLOR_BLUE
	{226, 228, 229, 230, 232, 233, 235, 237, 239, 240, 242, 244, 246,  31,  31,  31}, // SKINCOLOR_BLUEBERRY
	{120, 112,  82,  83,  84, 124, 248, 228, 228, 204, 205, 206, 207,  29,  30,  31}, // SKINCOLOR_NOVA
	{120, 208, 209, 210, 211, 226, 202, 249, 194, 195, 196, 197, 198, 199, 255,  30}, // SKINCOLOR_PASTEL
	{120, 224, 201, 226, 202, 249, 250, 196, 197, 198, 199, 140, 141, 142, 143,  31}, // SKINCOLOR_MOONSLAM
	{120,  64,  81, 122, 192, 249, 203, 221, 221, 219, 219, 223, 223, 191, 191,  31}, // SKINCOLOR_ULTRAVIOLET
	{121, 145, 192, 249, 250, 251, 204, 204, 205, 205, 206, 206, 207,  29,  30,  31}, // SKINCOLOR_DUSK
	{120,  96,  64, 121,  67, 144, 123, 192, 193, 194, 195, 196, 197, 198, 199,  30}, // SKINCOLOR_BUBBLEGUM
	{121, 145, 192, 192, 193, 194, 195, 196, 196, 197, 197, 198, 198, 199,  30,  31}, // SKINCOLOR_PURPLE
	{120, 122, 124, 125, 126, 150, 196, 197, 198, 198, 199, 199, 240, 242, 244, 246}, // SKINCOLOR_FUCHSIA
	{120, 120, 176, 176, 177,   6,   8,  10, 249, 250, 196, 197, 198, 199, 143,  31}, // SKINCOLOR_TOXIC
	{ 96,  97,  98, 112, 113,  73, 146, 248, 249, 251, 205, 205, 206, 207,  29,  31}, // SKINCOLOR_MAUVE
	{121, 145, 192, 248, 249, 250, 251, 252, 252, 253, 253, 254, 254, 255,  30,  31}, // SKINCOLOR_LAVENDER
	{201, 248, 249, 250, 251, 252, 253, 254, 255, 255,  29,  29,  30,  30,  31,  31}, // SKINCOLOR_BYZANTIUM
	{144, 145, 146, 147, 148, 149, 150, 251, 251, 252, 252, 253, 254, 255,  29,  30}, // SKINCOLOR_POMEGRANATE
	{120, 120, 120, 121, 121, 122, 122, 123, 192, 248, 249, 250, 251, 252, 253, 254}, // SKINCOLOR_LILAC

	{120, 120, 120, 120, 120, 120, 120, 120, 120, 120,  96, 100, 104, 113, 116, 119}, // SKINCOLOR_SUPER1
	{120, 120, 120, 120, 120, 120, 120, 120,  96,  98, 101, 104, 113, 115, 117, 119}, // SKINCOLOR_SUPER2
	{120, 120, 120, 120, 120, 120,  96,  98, 100, 102, 104, 113, 114, 116, 117, 119}, // SKINCOLOR_SUPER3
	{120, 120, 120, 120,  96,  97,  99, 100, 102, 104, 113, 114, 115, 116, 117, 119}, // SKINCOLOR_SUPER4
	{120, 120,  96, 120, 120, 120, 120, 120, 104, 113, 114, 115, 116, 117, 118, 119}, // SKINCOLOR_SUPER5
	{120, 120, 120, 120, 120, 120, 120, 120, 120, 120,  80,  82,  85, 115, 117, 119}, // SKINCOLOR_TSUPER1
	{120, 120, 120, 120, 120, 120, 120, 120,  80,  81,  83,  85, 115, 116, 117, 119}, // SKINCOLOR_TSUPER2
	{120, 120, 120, 120, 120, 120,  80,  81,  82,  83,  85, 115, 116, 117, 118, 119}, // SKINCOLOR_TSUPER3
	{120, 120, 120, 120,  80,  81,  82,  83,  84,  85, 115, 115, 116, 117, 118, 119}, // SKINCOLOR_TSUPER4
	{120, 120,  80,  80,  81,  82,  83,  84,  85, 115, 115, 116, 117, 117, 118, 119}, // SKINCOLOR_TSUPER5
	{120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 121, 123, 125, 127, 129, 132}, // SKINCOLOR_KSUPER1
	{120, 120, 120, 120, 120, 120, 120, 120, 121, 122, 124, 125, 127, 128, 130, 132}, // SKINCOLOR_KSUPER2
	{120, 120, 120, 120, 120, 120, 121, 122, 123, 124, 125, 127, 128, 129, 130, 132}, // SKINCOLOR_KSUPER3
	{120, 120, 120, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132}, // SKINCOLOR_KSUPER4
	{120, 120, 121, 121, 122, 123, 124, 125, 126, 126, 127, 128, 129, 130, 131, 132}, // SKINCOLOR_KSUPER5
	{120, 120, 120, 120, 120, 120, 120, 120, 120, 120,   0, 122, 124, 248, 251, 255}, // SKINCOLOR_PSUPER1
	{120, 120, 120, 120, 120, 120, 120, 120,   0, 121, 122, 124, 248, 250, 252, 255}, // SKINCOLOR_PSUPER2
	{120, 120, 120, 120, 120, 120,   0, 121, 122, 123, 124, 248, 249, 251, 253, 255}, // SKINCOLOR_PSUPER3
	{120, 120, 120, 120,   0, 121, 122, 123, 124, 248, 249, 250, 251, 252, 253, 255}, // SKINCOLOR_PSUPER4
	{120, 120,   0, 121, 122, 123, 124, 248, 248, 249, 250, 251, 252, 253, 254, 255}, // SKINCOLOR_PSUPER5
	{120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 224, 225, 227, 228, 230, 232}, // SKINCOLOR_BSUPER1
	{120, 120, 120, 120, 120, 120, 120, 120, 224, 225, 226, 227, 228, 229, 230, 232}, // SKINCOLOR_BSUPER2
	{120, 120, 120, 120, 120, 120, 224, 224, 225, 226, 227, 228, 229, 230, 231, 232}, // SKINCOLOR_BSUPER3
	{120, 120, 120, 120, 224, 224, 225, 226, 226, 227, 228, 229, 229, 230, 231, 232}, // SKINCOLOR_BSUPER4
	{120, 120, 224, 224, 225, 225, 226, 227, 227, 228, 228, 229, 230, 230, 231, 232}, // SKINCOLOR_BSUPER5
	{120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 208, 210, 212, 215, 220, 222}, // SKINCOLOR_ASUPER1
	{120, 120, 120, 120, 120, 120, 120, 120, 208, 209, 211, 213, 215, 220, 221, 223}, // SKINCOLOR_ASUPER2
	{120, 120, 120, 120, 120, 120, 208, 209, 210, 211, 212, 213, 215, 220, 221, 223}, // SKINCOLOR_ASUPER3
	{120, 120, 120, 120, 208, 209, 210, 211, 212, 213, 214, 215, 220, 221, 222, 223}, // SKINCOLOR_ASUPER4
	{120, 120, 208, 208, 209, 210, 211, 211, 212, 213, 214, 215, 220, 221, 222, 223}, // SKINCOLOR_ASUPER5
	{120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 176, 160, 163, 167, 171, 175}, // SKINCOLOR_GSUPER1
	{120, 120, 120, 120, 120, 120, 120, 120, 176, 176, 160, 163, 166, 169, 172, 175}, // SKINCOLOR_GSUPER2
	{120, 120, 120, 120, 120, 120, 176, 176, 160, 162, 164, 166, 168, 170, 172, 175}, // SKINCOLOR_GSUPER3
	{120, 120, 120, 120, 176, 176, 176, 160, 161, 163, 165, 167, 169, 171, 173, 175}, // SKINCOLOR_GSUPER4
	{120, 120, 176, 176, 176, 160, 161, 163, 164, 166, 167, 169, 170, 172, 173, 175}, // SKINCOLOR_GSUPER5
	{120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120}, // SKINCOLOR_WSUPER1
	{120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120,   0,   4,   9}, // SKINCOLOR_WSUPER2
	{120, 120, 120, 120, 120, 120, 120, 120, 120, 120,   0,   2,   4,   6,   8,  11}, // SKINCOLOR_WSUPER3
	{120, 120, 120, 120, 120, 120, 120,   0,   1,   3,   4,   6,   8,   9,  11,  13}, // SKINCOLOR_WSUPER4
	{120, 120, 120, 120,   0,   1,   2,   4,   5,   6,   8,   9,  10,  12,  13,  15}, // SKINCOLOR_WSUPER5
	{120, 120, 120, 120, 120, 120, 120, 120, 120, 120,  96,  98,  99,  81,  73,  79}, // SKINCOLOR_CSUPER1
	{120, 120, 120, 120, 120, 120, 120, 120,  96,  97,  98,  81,  81,  71,  75,  79}, // SKINCOLOR_CSUPER2
	{120, 120, 120, 120, 120, 120,  96,  97,  98,  99,  81,  81,  70,  73,  76,  79}, // SKINCOLOR_CSUPER3
	{120, 120, 120, 120,  96,  96,  97,  98,  99,  81,  81,  70,  72,  74,  76,  79}, // SKINCOLOR_CSUPER4
	{120, 120,  96,  96,  97,  98,  98,  99,  81,  81,  69,  71,  73,  75,  77,  79}, // SKINCOLOR_CSUPER5
};

// Define for getting accurate color brightness readings according to how the human eye sees them.
// https://en.wikipedia.org/wiki/Relative_luminance
// 0.2126 to red
// 0.7152 to green
// 0.0722 to blue
// (See this same define in hw_md2.c!)
#define SETBRIGHTNESS(brightness,r,g,b) \
	brightness = (UINT8)(((1063*(UINT16)(r))/5000) + ((3576*(UINT16)(g))/5000) + ((361*(UINT16)(b))/5000))

/** \brief	Generates the rainbow colourmaps that are used when a player has the invincibility power

	\param	dest_colormap	colormap to populate
	\param	skincolor		translation color
*/
void K_RainbowColormap(UINT8 *dest_colormap, UINT8 skincolor)
{
	INT32 i;
	RGBA_t color;
	UINT8 brightness;
	INT32 j;
	UINT8 colorbrightnesses[16];
	UINT16 brightdif;
	INT32 temp;

	// first generate the brightness of all the colours of that skincolour
	for (i = 0; i < 16; i++)
	{
		color = V_GetColor(colortranslations[skincolor][i]);
		SETBRIGHTNESS(colorbrightnesses[i], color.s.red, color.s.green, color.s.blue);
	}

	// next, for every colour in the palette, choose the transcolor that has the closest brightness
	for (i = 0; i < NUM_PALETTE_ENTRIES; i++)
	{
		if (i == 0 || i == 31 || i == 120) // pure black and pure white don't change
		{
			dest_colormap[i] = (UINT8)i;
			continue;
		}
		color = V_GetColor(i);
		SETBRIGHTNESS(brightness, color.s.red, color.s.green, color.s.blue);
		brightdif = 256;
		for (j = 0; j < 16; j++)
		{
			temp = abs((INT16)brightness - (INT16)colorbrightnesses[j]);
			if (temp < brightdif)
			{
				brightdif = (UINT16)temp;
				dest_colormap[i] = colortranslations[skincolor][j];
			}
		}
	}
}

#undef SETBRIGHTNESS

/**	\brief	Generates a translation colormap for Kart, to replace R_GenerateTranslationColormap in r_draw.c

	\param	dest_colormap	colormap to populate
	\param	skinnum			number of skin, TC_DEFAULT or TC_BOSS
	\param	color			translation color

	\return	void
*/
void K_GenerateKartColormap(UINT8 *dest_colormap, INT32 skinnum, UINT8 color)
{
	INT32 i;
	INT32 starttranscolor;

	// Handle a couple of simple special cases
	if (skinnum == TC_BOSS
		|| skinnum == TC_ALLWHITE
		|| skinnum == TC_METALSONIC
		|| skinnum == TC_BLINK
		|| color == SKINCOLOR_NONE)
	{
		for (i = 0; i < NUM_PALETTE_ENTRIES; i++)
		{
			if (skinnum == TC_ALLWHITE)
				dest_colormap[i] = 0;
			else if (skinnum == TC_BLINK)
				dest_colormap[i] = colortranslations[color][3];
			else
				dest_colormap[i] = (UINT8)i;
		}

		// White!
		if (skinnum == TC_BOSS)
			dest_colormap[31] = 0;
		else if (skinnum == TC_METALSONIC)
			dest_colormap[239] = 0;

		return;
	}
	else if (skinnum == TC_RAINBOW)
	{
		K_RainbowColormap(dest_colormap, color);
		return;
	}

	starttranscolor = (skinnum != TC_DEFAULT) ? skins[skinnum].starttranscolor : DEFAULT_STARTTRANSCOLOR;

	// Fill in the entries of the palette that are fixed
	for (i = 0; i < starttranscolor; i++)
		dest_colormap[i] = (UINT8)i;

	for (i = (UINT8)(starttranscolor + 16); i < NUM_PALETTE_ENTRIES; i++)
		dest_colormap[i] = (UINT8)i;

	// Build the translated ramp
	for (i = 0; i < SKIN_RAMP_LENGTH; i++)
	{
		// Sryder 2017-10-26: What was here before was most definitely not particularly readable, check above for new color translation table
		dest_colormap[starttranscolor + i] = colortranslations[color][i];
	}
}

/**	\brief	Pulls kart color by name, to replace R_GetColorByName in r_draw.c

	\param	name	color name

	\return	0
*/
UINT8 K_GetKartColorByName(const char *name)
{
	UINT8 color = (UINT8)atoi(name);
	if (color > 0 && color < MAXSKINCOLORS)
		return color;
	for (color = 1; color < MAXSKINCOLORS; color++)
		if (!stricmp(KartColor_Names[color], name))
			return color;
	return 0;
}

//}

//{ SRB2kart Net Variables

void K_RegisterKartStuff(void)
{
	CV_RegisterVar(&cv_sneaker);
	CV_RegisterVar(&cv_rocketsneaker);
	CV_RegisterVar(&cv_invincibility);
	CV_RegisterVar(&cv_banana);
	CV_RegisterVar(&cv_eggmanmonitor);
	CV_RegisterVar(&cv_orbinaut);
	CV_RegisterVar(&cv_jawz);
	CV_RegisterVar(&cv_mine);
	CV_RegisterVar(&cv_ballhog);
	CV_RegisterVar(&cv_selfpropelledbomb);
	CV_RegisterVar(&cv_grow);
	CV_RegisterVar(&cv_shrink);
	CV_RegisterVar(&cv_thundershield);
	CV_RegisterVar(&cv_hyudoro);
	CV_RegisterVar(&cv_pogospring);
	CV_RegisterVar(&cv_kitchensink);

	CV_RegisterVar(&cv_triplesneaker);
	CV_RegisterVar(&cv_triplebanana);
	CV_RegisterVar(&cv_decabanana);
	CV_RegisterVar(&cv_tripleorbinaut);
	CV_RegisterVar(&cv_quadorbinaut);
	CV_RegisterVar(&cv_dualjawz);

	CV_RegisterVar(&cv_kartminimap);
	CV_RegisterVar(&cv_kartcheck);
	CV_RegisterVar(&cv_kartinvinsfx);
	CV_RegisterVar(&cv_kartspeed);
	CV_RegisterVar(&cv_kartbumpers);
	CV_RegisterVar(&cv_kartfrantic);
	CV_RegisterVar(&cv_kartcomeback);
	CV_RegisterVar(&cv_kartencore);
	CV_RegisterVar(&cv_kartvoterulechanges);
	CV_RegisterVar(&cv_kartspeedometer);
	CV_RegisterVar(&cv_kartvoices);
	CV_RegisterVar(&cv_karteliminatelast);
	CV_RegisterVar(&cv_votetime);

	CV_RegisterVar(&cv_kartdebugitem);
	CV_RegisterVar(&cv_kartdebugamount);
	CV_RegisterVar(&cv_kartdebugshrink);
	CV_RegisterVar(&cv_kartdebugdistribution);
	CV_RegisterVar(&cv_kartdebughuddrop);

	CV_RegisterVar(&cv_kartdebugcheckpoint);
	CV_RegisterVar(&cv_kartdebugnodes);
	CV_RegisterVar(&cv_kartdebugcolorize);
}

//}

boolean K_IsPlayerLosing(player_t *player)
{
	INT32 winningpos = 1;
	UINT8 i, pcount = 0;

	if (player->kartstuff[k_position] == 1)
		return false;

	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (!playeringame[i] || players[i].spectator)
			continue;
		if (players[i].kartstuff[k_position] > pcount)
			pcount = players[i].kartstuff[k_position];
	}

	if (pcount <= 1)
		return false;

	winningpos = pcount/2;
	if (pcount % 2) // any remainder?
		winningpos++;

	return (player->kartstuff[k_position] > winningpos);
}

boolean K_IsPlayerWanted(player_t *player)
{
	UINT8 i;
	if (!(G_BattleGametype()))
		return false;
	for (i = 0; i < 4; i++)
	{
		if (battlewanted[i] == -1)
			break;
		if (player == &players[battlewanted[i]])
			return true;
	}
	return false;
}

//{ SRB2kart Roulette Code - Position Based

#define NUMKARTODDS 	80

// Less ugly 2D arrays
static INT32 K_KartItemOddsRace[NUMKARTRESULTS][10] =
{
				//P-Odds	 0  1  2  3  4  5  6  7  8  9
			   /*Sneaker*/ {20, 0, 0, 4, 6, 7, 0, 0, 0, 0 }, // Sneaker
		/*Rocket Sneaker*/ { 0, 0, 0, 0, 0, 1, 4, 5, 3, 0 }, // Rocket Sneaker
		 /*Invincibility*/ { 0, 0, 0, 0, 0, 1, 4, 6,10, 0 }, // Invincibility
				/*Banana*/ { 0, 9, 4, 2, 1, 0, 0, 0, 0, 0 }, // Banana
		/*Eggman Monitor*/ { 0, 3, 2, 1, 0, 0, 0, 0, 0, 0 }, // Eggman Monitor
			  /*Orbinaut*/ { 0, 7, 6, 4, 2, 0, 0, 0, 0, 0 }, // Orbinaut
				  /*Jawz*/ { 0, 0, 3, 2, 1, 1, 0, 0, 0, 0 }, // Jawz
				  /*Mine*/ { 0, 0, 2, 2, 1, 0, 0, 0, 0, 0 }, // Mine
			   /*Ballhog*/ { 0, 0, 0, 2, 1, 0, 0, 0, 0, 0 }, // Ballhog
   /*Self-Propelled Bomb*/ { 0, 0, 1, 2, 3, 4, 2, 2, 0,20 }, // Self-Propelled Bomb
				  /*Grow*/ { 0, 0, 0, 0, 0, 0, 2, 5, 7, 0 }, // Grow
				/*Shrink*/ { 0, 0, 0, 0, 0, 0, 0, 2, 0, 0 }, // Shrink
		/*Thunder Shield*/ { 0, 1, 2, 0, 0, 0, 0, 0, 0, 0 }, // Thunder Shield
			   /*Hyudoro*/ { 0, 0, 0, 0, 1, 2, 1, 0, 0, 0 }, // Hyudoro
		   /*Pogo Spring*/ { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // Pogo Spring
		  /*Kitchen Sink*/ { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // Kitchen Sink
			/*Sneaker x3*/ { 0, 0, 0, 0, 3, 7, 9, 2, 0, 0 }, // Sneaker x3
			 /*Banana x3*/ { 0, 0, 1, 1, 0, 0, 0, 0, 0, 0 }, // Banana x3
			/*Banana x10*/ { 0, 0, 0, 0, 1, 0, 0, 0, 0, 0 }, // Banana x10
		   /*Orbinaut x3*/ { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0 }, // Orbinaut x3
		   /*Orbinaut x4*/ { 0, 0, 0, 0, 1, 1, 0, 0, 0, 0 }, // Orbinaut x4
			   /*Jawz x2*/ { 0, 0, 0, 1, 2, 0, 0, 0, 0, 0 }  // Jawz x2
};

static INT32 K_KartItemOddsBattle[NUMKARTRESULTS][2] =
{
				//P-Odds	 0  1
			   /*Sneaker*/ { 2, 1 }, // Sneaker
		/*Rocket Sneaker*/ { 0, 0 }, // Rocket Sneaker
		 /*Invincibility*/ { 2, 1 }, // Invincibility
				/*Banana*/ { 1, 0 }, // Banana
		/*Eggman Monitor*/ { 1, 0 }, // Eggman Monitor
			  /*Orbinaut*/ { 8, 0 }, // Orbinaut
				  /*Jawz*/ { 8, 1 }, // Jawz
				  /*Mine*/ { 4, 1 }, // Mine
			   /*Ballhog*/ { 2, 1 }, // Ballhog
   /*Self-Propelled Bomb*/ { 0, 0 }, // Self-Propelled Bomb
				  /*Grow*/ { 2, 1 }, // Grow
				/*Shrink*/ { 0, 0 }, // Shrink
		/*Thunder Shield*/ { 0, 0 }, // Thunder Shield
			   /*Hyudoro*/ { 2, 0 }, // Hyudoro
		   /*Pogo Spring*/ { 2, 0 }, // Pogo Spring
		  /*Kitchen Sink*/ { 0, 0 }, // Kitchen Sink
			/*Sneaker x3*/ { 0, 1 }, // Sneaker x3
			 /*Banana x3*/ { 1, 0 }, // Banana x3
			/*Banana x10*/ { 0, 1 }, // Banana x10
		   /*Orbinaut x3*/ { 2, 0 }, // Orbinaut x3
		   /*Orbinaut x4*/ { 1, 1 }, // Orbinaut x4
			   /*Jawz x2*/ { 2, 1 }  // Jawz x2
};

/**	\brief	Item Roulette for Kart

	\param	player		player
	\param	getitem		what item we're looking for

	\return	void
*/
static void K_KartGetItemResult(player_t *player, SINT8 getitem)
{
	if (getitem == KITEM_SPB || getitem == KITEM_SHRINK) // Indirect items
		indirectitemcooldown = 20*TICRATE;
	if (getitem == KITEM_HYUDORO) // Hyudoro cooldown
		hyubgone = 5*TICRATE;

	switch (getitem)
	{
		// Special roulettes first, then the generic ones are handled by default
		case KRITEM_TRIPLESNEAKER: // Sneaker x3
			player->kartstuff[k_itemtype] = KITEM_SNEAKER;
			player->kartstuff[k_itemamount] = 3;
			break;
		case KRITEM_TRIPLEBANANA: // Banana x3
			player->kartstuff[k_itemtype] = KITEM_BANANA;
			player->kartstuff[k_itemamount] = 3;
			break;
		case KRITEM_TENFOLDBANANA: // Banana x10
			player->kartstuff[k_itemtype] = KITEM_BANANA;
			player->kartstuff[k_itemamount] = 10;
			break;
		case KRITEM_TRIPLEORBINAUT: // Orbinaut x3
			player->kartstuff[k_itemtype] = KITEM_ORBINAUT;
			player->kartstuff[k_itemamount] = 3;
			break;
		case KRITEM_QUADORBINAUT: // Orbinaut x4
			player->kartstuff[k_itemtype] = KITEM_ORBINAUT;
			player->kartstuff[k_itemamount] = 4;
			break;
		case KRITEM_DUALJAWZ: // Jawz x2
			player->kartstuff[k_itemtype] = KITEM_JAWZ;
			player->kartstuff[k_itemamount] = 2;
			break;
		default:
			if (getitem <= 0 || getitem >= NUMKARTRESULTS) // Sad (Fallback)
			{
				if (getitem != 0)
					CONS_Printf("ERROR: P_KartGetItemResult - Item roulette gave bad item (%d) :(\n", getitem);
				player->kartstuff[k_itemtype] = KITEM_SAD;
			}
			else
				player->kartstuff[k_itemtype] = getitem;
			player->kartstuff[k_itemamount] = 1;
			break;
	}
}

/**	\brief	Item Roulette for Kart

	\param	player	player object passed from P_KartPlayerThink

	\return	void
*/

static INT32 K_KartGetItemOdds(UINT8 pos, SINT8 item, fixed_t mashed, boolean spbrush)
{
	const INT32 distvar = (64*14);
	INT32 newodds;
	INT32 i;
	UINT8 pingame = 0, pexiting = 0;
	boolean thunderisout = false;
	SINT8 first = -1, second = -1;
	INT32 secondist = 0;
	boolean itemenabled[NUMKARTRESULTS-1] = {
		cv_sneaker.value,
		cv_rocketsneaker.value,
		cv_invincibility.value,
		cv_banana.value,
		cv_eggmanmonitor.value,
		cv_orbinaut.value,
		cv_jawz.value,
		cv_mine.value,
		cv_ballhog.value,
		cv_selfpropelledbomb.value,
		cv_grow.value,
		cv_shrink.value,
		cv_thundershield.value,
		cv_hyudoro.value,
		cv_pogospring.value,
		cv_kitchensink.value,
		cv_triplesneaker.value,
		cv_triplebanana.value,
		cv_decabanana.value,
		cv_tripleorbinaut.value,
		cv_quadorbinaut.value,
		cv_dualjawz.value
	};

	I_Assert(item > KITEM_NONE); // too many off by one scenarioes.

	if (!itemenabled[item-1] && !modeattacking)
		return 0;

	if (G_BattleGametype())
		newodds = K_KartItemOddsBattle[item-1][pos];
	else
		newodds = K_KartItemOddsRace[item-1][pos];

	// Base multiplication to ALL item odds to simulate fractional precision
	newodds *= 4;

	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (!playeringame[i] || players[i].spectator)
			continue;

		if (!G_BattleGametype() || players[i].kartstuff[k_bumper])
			pingame++;

		if (players[i].exiting)
			pexiting++;

		if (players[i].mo)
		{
			if (players[i].kartstuff[k_itemtype] == KITEM_THUNDERSHIELD)
				thunderisout = true;

			if (!G_BattleGametype())
			{
				if (players[i].kartstuff[k_position] == 1 && first == -1)
					first = i;
				if (players[i].kartstuff[k_position] == 2 && second == -1)
					second = i;
			}
		}
	}

	if (first != -1 && second != -1) // calculate 2nd's distance from 1st, for SPB
	{
		secondist = P_AproxDistance(P_AproxDistance(players[first].mo->x - players[second].mo->x,
													players[first].mo->y - players[second].mo->y),
													players[first].mo->z - players[second].mo->z) / mapobjectscale;
		if (franticitems)
			secondist = (15 * secondist) / 14;
		secondist = ((28 + (8-pingame)) * secondist) / 28;
	}

	// POWERITEMODDS handles all of the "frantic item" related functionality, for all of our powerful items.
	// First, it multiplies it by 2 if franticitems is true; easy-peasy.
	// Next, it multiplies it again if it's in SPB mode and 2nd needs to apply pressure to 1st.
	// Then, it multiplies it further if the player count isn't equal to 8.
	// This is done to make low player count races more interesting and high player count rates more fair.
	// (2P normal would be about halfway between 8P normal and 8P frantic.)
	// (This scaling is not done for SPB Rush, so that catchup strength is not weakened.)
	// Lastly, it *divides* it by your mashed value, which was determined in K_KartItemRoulette, for lesser items needed in a pinch.

#define PLAYERSCALING (8 - (spbrush ? 2 : pingame))

#define POWERITEMODDS(odds) {\
	if (franticitems) \
		odds <<= 1; \
	odds = FixedMul(odds<<FRACBITS, FRACUNIT + ((PLAYERSCALING << FRACBITS) / 25)) >> FRACBITS; \
	if (mashed > 0) \
		odds = FixedDiv(odds<<FRACBITS, FRACUNIT + mashed) >> FRACBITS; \
}

#define COOLDOWNONSTART (leveltime < (30*TICRATE)+starttime)

	switch (item)
	{
		case KITEM_ROCKETSNEAKER:
		case KITEM_JAWZ:
		case KITEM_BALLHOG:
		case KRITEM_TRIPLESNEAKER:
		case KRITEM_TRIPLEBANANA:
		case KRITEM_TENFOLDBANANA:
		case KRITEM_TRIPLEORBINAUT:
		case KRITEM_QUADORBINAUT:
		case KRITEM_DUALJAWZ:
			POWERITEMODDS(newodds);
			break;
		case KITEM_INVINCIBILITY:
		case KITEM_MINE:
		case KITEM_GROW:
			if (COOLDOWNONSTART)
				newodds = 0;
			else
				POWERITEMODDS(newodds);
			break;
		case KITEM_SPB:
			if (((indirectitemcooldown > 0) || (pexiting > 0) || (secondist/distvar < 3))
				&& (pos != 9)) // Force SPB
				newodds = 0;
			else
				newodds *= min((secondist/distvar)-4, 3); // POWERITEMODDS(newodds);
			break;
		case KITEM_SHRINK:
			if ((indirectitemcooldown > 0) || (pingame-1 <= pexiting) || COOLDOWNONSTART)
				newodds = 0;
			else
				POWERITEMODDS(newodds);
			break;
		case KITEM_THUNDERSHIELD:
			if (thunderisout)
				newodds = 0;
			else
				POWERITEMODDS(newodds);
			break;
		case KITEM_HYUDORO:
			if ((hyubgone > 0) || COOLDOWNONSTART)
				newodds = 0;
			break;
		default:
			break;
	}

#undef POWERITEMODDS

	return newodds;
}

//{ SRB2kart Roulette Code - Distance Based, no waypoints

static INT32 K_FindUseodds(player_t *player, fixed_t mashed, INT32 pingame, INT32 bestbumper, boolean spbrush, boolean dontforcespb)
{
	const INT32 distvar = (64*14);
	INT32 i;
	INT32 pdis = 0, useodds = 0;
	UINT8 disttable[14];
	UINT8 distlen = 0;
	boolean oddsvalid[10];

	// Unused now, oops :V
	(void)bestbumper;

	for (i = 0; i < 10; i++)
	{
		INT32 j;
		boolean available = false;

		if (G_BattleGametype() && i > 1)
		{
			oddsvalid[i] = false;
			break;
		}

		for (j = 1; j < NUMKARTRESULTS; j++)
		{
			if (K_KartGetItemOdds(i, j, mashed, spbrush) > 0)
			{
				available = true;
				break;
			}
		}

		oddsvalid[i] = available;
	}

	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (playeringame[i] && !players[i].spectator && players[i].mo
			&& players[i].kartstuff[k_position] < player->kartstuff[k_position])
			pdis += P_AproxDistance(P_AproxDistance(players[i].mo->x - player->mo->x,
													players[i].mo->y - player->mo->y),
													players[i].mo->z - player->mo->z) / mapobjectscale
													* (pingame - players[i].kartstuff[k_position])
													/ max(1, ((pingame - 1) * (pingame + 1) / 3));
	}

#define SETUPDISTTABLE(odds, num) \
	for (i = num; i; --i) disttable[distlen++] = odds

	if (G_BattleGametype()) // Battle Mode
	{
		if (player->kartstuff[k_roulettetype] == 1 && oddsvalid[1] == true)
		{
			// 1 is the extreme odds of player-controlled "Karma" items
			useodds = 1;
		}
		else
		{
			useodds = 0;

			if (oddsvalid[0] == false && oddsvalid[1] == true)
			{
				// try to use karma odds as a fallback
				useodds = 1;
			}
		}
	}
	else
	{
		if (oddsvalid[1]) SETUPDISTTABLE(1,1);
		if (oddsvalid[2]) SETUPDISTTABLE(2,1);
		if (oddsvalid[3]) SETUPDISTTABLE(3,1);
		if (oddsvalid[4]) SETUPDISTTABLE(4,2);
		if (oddsvalid[5]) SETUPDISTTABLE(5,2);
		if (oddsvalid[6]) SETUPDISTTABLE(6,3);
		if (oddsvalid[7]) SETUPDISTTABLE(7,3);
		if (oddsvalid[8]) SETUPDISTTABLE(8,1);

		if (franticitems) // Frantic items make the distances between everyone artifically higher, for crazier items
			pdis = (15 * pdis) / 14;

		if (spbrush) // SPB Rush Mode: It's 2nd place's job to catch-up items and make 1st place's job hell
			pdis = (3 * pdis) / 2;

		pdis = ((28 + (8-pingame)) * pdis) / 28;

		if (pingame == 1 && oddsvalid[0])					// Record Attack, or just alone
			useodds = 0;
		else if (pdis <= 0)									// (64*14) *  0 =     0
			useodds = disttable[0];
		else if (player->kartstuff[k_position] == 2 && pdis > (distvar*6)
			&& spbplace == -1 && !indirectitemcooldown && !dontforcespb
			&& oddsvalid[9])								// Force SPB in 2nd
			useodds = 9;
		else if (pdis > distvar * ((12 * distlen) / 14))	// (64*14) * 12 = 10752
			useodds = disttable[distlen-1];
		else
		{
			for (i = 1; i < 13; i++)
			{
				if (pdis <= distvar * ((i * distlen) / 14))
				{
					useodds = disttable[((i * distlen) / 14)];
					break;
				}
			}
		}
	}

#undef SETUPDISTTABLE

	//CONS_Printf("Got useodds %d. (position: %d, distance: %d)\n", useodds, player->kartstuff[k_position], pdis);

	return useodds;
}

static void K_KartItemRoulette(player_t *player, ticcmd_t *cmd)
{
	INT32 i;
	UINT8 pingame = 0;
	UINT8 roulettestop;
	INT32 useodds = 0;
	INT32 spawnchance[NUMKARTRESULTS];
	INT32 totalspawnchance = 0;
	INT32 bestbumper = 0;
	fixed_t mashed = 0;
	boolean dontforcespb = false;
	boolean spbrush = false;

	// This makes the roulette cycle through items - if this is 0, you shouldn't be here.
	if (player->kartstuff[k_itemroulette])
		player->kartstuff[k_itemroulette]++;
	else
		return;

	// Gotta check how many players are active at this moment.
	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (!playeringame[i] || players[i].spectator)
			continue;
		pingame++;
		if (players[i].exiting)
			dontforcespb = true;
		if (players[i].kartstuff[k_bumper] > bestbumper)
			bestbumper = players[i].kartstuff[k_bumper];
	}

	// No forced SPB in 1v1s, it has to be randomly rolled
	if (pingame <= 2)
		dontforcespb = true;

	// This makes the roulette produce the random noises.
	if ((player->kartstuff[k_itemroulette] % 3) == 1 && P_IsDisplayPlayer(player) && !demo.freecam)
	{
#define PLAYROULETTESND S_StartSound(NULL, sfx_itrol1 + ((player->kartstuff[k_itemroulette] / 3) % 8))
		for (i = 0; i <= splitscreen; i++)
		{
			if (player == &players[displayplayers[i]] && players[displayplayers[i]].kartstuff[k_itemroulette])
				PLAYROULETTESND;
		}
#undef PLAYROULETTESND
	}

	roulettestop = TICRATE + (3*(pingame - player->kartstuff[k_position]));

	// If the roulette finishes or the player presses BT_ATTACK, stop the roulette and calculate the item.
	// I'm returning via the exact opposite, however, to forgo having another bracket embed. Same result either way, I think.
	// Finally, if you get past this check, now you can actually start calculating what item you get.
	if ((cmd->buttons & BT_ATTACK) && !(player->kartstuff[k_eggmanheld] || player->kartstuff[k_itemheld]) && player->kartstuff[k_itemroulette] >= roulettestop && !modeattacking)
	{
		// Mashing reduces your chances for the good items
		mashed = FixedDiv((player->kartstuff[k_itemroulette])*FRACUNIT, ((TICRATE*3)+roulettestop)*FRACUNIT) - FRACUNIT;
	}
	else if (!(player->kartstuff[k_itemroulette] >= (TICRATE*3)))
		return;

	if (cmd->buttons & BT_ATTACK)
		player->pflags |= PF_ATTACKDOWN;

	if (player->kartstuff[k_roulettetype] == 2) // Fake items
	{
		player->kartstuff[k_eggmanexplode] = 4*TICRATE;
		//player->kartstuff[k_itemblink] = TICRATE;
		//player->kartstuff[k_itemblinkmode] = 1;
		player->kartstuff[k_itemroulette] = 0;
		player->kartstuff[k_roulettetype] = 0;
		if (P_IsDisplayPlayer(player) && !demo.freecam)
			S_StartSound(NULL, sfx_itrole);
		return;
	}

	if (cv_kartdebugitem.value != 0 && !modeattacking)
	{
		K_KartGetItemResult(player, cv_kartdebugitem.value);
		player->kartstuff[k_itemamount] = cv_kartdebugamount.value;
		player->kartstuff[k_itemblink] = TICRATE;
		player->kartstuff[k_itemblinkmode] = 2;
		player->kartstuff[k_itemroulette] = 0;
		player->kartstuff[k_roulettetype] = 0;
		if (P_IsDisplayPlayer(player) && !demo.freecam)
			S_StartSound(NULL, sfx_dbgsal);
		return;
	}

	if (G_RaceGametype())
		spbrush = (spbplace != -1 && player->kartstuff[k_position] == spbplace+1);

	// Initializes existing spawnchance values
	for (i = 0; i < NUMKARTRESULTS; i++)
		spawnchance[i] = 0;

	// Split into another function for a debug function below
	useodds = K_FindUseodds(player, mashed, pingame, bestbumper, spbrush, dontforcespb);

	for (i = 1; i < NUMKARTRESULTS; i++)
		spawnchance[i] = (totalspawnchance += K_KartGetItemOdds(useodds, i, mashed, spbrush));

	// Award the player whatever power is rolled
	if (totalspawnchance > 0)
	{
		totalspawnchance = P_RandomKey(totalspawnchance);
		for (i = 0; i < NUMKARTRESULTS && spawnchance[i] <= totalspawnchance; i++);

		K_KartGetItemResult(player, i);
	}
	else
	{
		player->kartstuff[k_itemtype] = KITEM_SAD;
		player->kartstuff[k_itemamount] = 1;
	}

	if (P_IsDisplayPlayer(player) && !demo.freecam)
		S_StartSound(NULL, ((player->kartstuff[k_roulettetype] == 1) ? sfx_itrolk : (mashed ? sfx_itrolm : sfx_itrolf)));

	player->kartstuff[k_itemblink] = TICRATE;
	player->kartstuff[k_itemblinkmode] = ((player->kartstuff[k_roulettetype] == 1) ? 2 : (mashed ? 1 : 0));

	player->kartstuff[k_itemroulette] = 0; // Since we're done, clear the roulette number
	player->kartstuff[k_roulettetype] = 0; // This too
}

//}

//{ SRB2kart p_user.c Stuff

static fixed_t K_GetMobjWeight(mobj_t *mobj, mobj_t *against)
{
	fixed_t weight = 5<<FRACBITS;

	switch (mobj->type)
	{
		case MT_PLAYER:
			if (!mobj->player)
				break;
			if (against->player && !against->player->kartstuff[k_spinouttimer] && mobj->player->kartstuff[k_spinouttimer])
				weight = 0; // Do not bump
			else
			{
				weight = (mobj->player->kartweight)<<FRACBITS;
				if (mobj->player->speed > K_GetKartSpeed(mobj->player, false))
					weight += (mobj->player->speed - K_GetKartSpeed(mobj->player, false))/8;
			}
			break;
		case MT_FALLINGROCK:
			if (against->player)
			{
				if (against->player->kartstuff[k_invincibilitytimer]
					|| against->player->kartstuff[k_growshrinktimer] > 0)
					weight = 0;
				else
					weight = (against->player->kartweight)<<FRACBITS;
			}
			break;
		case MT_ORBINAUT:
		case MT_ORBINAUT_SHIELD:
			if (against->player)
				weight = (against->player->kartweight)<<FRACBITS;
			break;
		case MT_JAWZ:
		case MT_JAWZ_DUD:
		case MT_JAWZ_SHIELD:
			if (against->player)
				weight = (against->player->kartweight+3)<<FRACBITS;
			else
				weight = 8<<FRACBITS;
			break;
		default:
			break;
	}

	return weight;
}

void K_KartBouncing(mobj_t *mobj1, mobj_t *mobj2, boolean bounce, boolean solid)
{
	mobj_t *fx;
	fixed_t momdifx, momdify;
	fixed_t distx, disty;
	fixed_t dot, force;
	fixed_t mass1, mass2;

	if (!mobj1 || !mobj2)
		return;

	// Don't bump when you're being reborn
	if ((mobj1->player && mobj1->player->playerstate != PST_LIVE)
		|| (mobj2->player && mobj2->player->playerstate != PST_LIVE))
		return;

	if ((mobj1->player && mobj1->player->kartstuff[k_respawn])
		|| (mobj2->player && mobj2->player->kartstuff[k_respawn]))
		return;

	{ // Don't bump if you're flashing
		INT32 flash;

		flash = K_GetKartFlashing(mobj1->player);
		if (mobj1->player && mobj1->player->powers[pw_flashing] > 0 && mobj1->player->powers[pw_flashing] < flash)
		{
			if (mobj1->player->powers[pw_flashing] < flash-1)
				mobj1->player->powers[pw_flashing]++;
			return;
		}

		flash = K_GetKartFlashing(mobj2->player);
		if (mobj2->player && mobj2->player->powers[pw_flashing] > 0 && mobj2->player->powers[pw_flashing] < flash)
		{
			if (mobj2->player->powers[pw_flashing] < flash-1)
				mobj2->player->powers[pw_flashing]++;
			return;
		}
	}

	// Don't bump if you've recently bumped
	if (mobj1->player && mobj1->player->kartstuff[k_justbumped])
	{
		mobj1->player->kartstuff[k_justbumped] = bumptime;
		return;
	}

	if (mobj2->player && mobj2->player->kartstuff[k_justbumped])
	{
		mobj2->player->kartstuff[k_justbumped] = bumptime;
		return;
	}

	mass1 = K_GetMobjWeight(mobj1, mobj2);

	if (solid == true && mass1 > 0)
		mass2 = mass1;
	else
		mass2 = K_GetMobjWeight(mobj2, mobj1);

	momdifx = mobj1->momx - mobj2->momx;
	momdify = mobj1->momy - mobj2->momy;

	// Adds the OTHER player's momentum times a bunch, for the best chance of getting the correct direction
	distx = (mobj1->x + mobj2->momx*3) - (mobj2->x + mobj1->momx*3);
	disty = (mobj1->y + mobj2->momy*3) - (mobj2->y + mobj1->momy*3);

	if (distx == 0 && disty == 0)
		// if there's no distance between the 2, they're directly on top of each other, don't run this
		return;

	{ // Normalize distance to the sum of the two objects' radii, since in a perfect world that would be the distance at the point of collision...
		fixed_t dist = P_AproxDistance(distx, disty);
		fixed_t nx = FixedDiv(distx, dist);
		fixed_t ny = FixedDiv(disty, dist);

		dist = dist ? dist : 1;
		distx = FixedMul(mobj1->radius+mobj2->radius, nx);
		disty = FixedMul(mobj1->radius+mobj2->radius, ny);

		if (momdifx == 0 && momdify == 0)
		{
			// If there's no momentum difference, they're moving at exactly the same rate. Pretend they moved into each other.
			momdifx = -nx;
			momdify = -ny;
		}
	}

	// if the speed difference is less than this let's assume they're going proportionately faster from each other
	if (P_AproxDistance(momdifx, momdify) < (25*mapobjectscale))
	{
		fixed_t momdiflength = P_AproxDistance(momdifx, momdify);
		fixed_t normalisedx = FixedDiv(momdifx, momdiflength);
		fixed_t normalisedy = FixedDiv(momdify, momdiflength);
		momdifx = FixedMul((25*mapobjectscale), normalisedx);
		momdify = FixedMul((25*mapobjectscale), normalisedy);
	}

	dot = FixedMul(momdifx, distx) + FixedMul(momdify, disty);

	if (dot >= 0)
	{
		// They're moving away from each other
		return;
	}

	force = FixedDiv(dot, FixedMul(distx, distx)+FixedMul(disty, disty));

	if (bounce == true && mass2 > 0) // Perform a Goomba Bounce.
		mobj1->momz = -mobj1->momz;
	else
	{
		fixed_t newz = mobj1->momz;
		if (mass2 > 0)
			mobj1->momz = mobj2->momz;
		if (mass1 > 0 && solid == false)
			mobj2->momz = newz;
	}

	if (mass2 > 0)
	{
		mobj1->momx = mobj1->momx - FixedMul(FixedMul(FixedDiv(2*mass2, mass1 + mass2), force), distx);
		mobj1->momy = mobj1->momy - FixedMul(FixedMul(FixedDiv(2*mass2, mass1 + mass2), force), disty);
	}

	if (mass1 > 0 && solid == false)
	{
		mobj2->momx = mobj2->momx - FixedMul(FixedMul(FixedDiv(2*mass1, mass1 + mass2), force), -distx);
		mobj2->momy = mobj2->momy - FixedMul(FixedMul(FixedDiv(2*mass1, mass1 + mass2), force), -disty);
	}

	// Do the bump fx when we've CONFIRMED we can bump.
	S_StartSound(mobj1, sfx_s3k49);

	fx = P_SpawnMobj(mobj1->x/2 + mobj2->x/2, mobj1->y/2 + mobj2->y/2, mobj1->z/2 + mobj2->z/2, MT_BUMP);
	if (mobj1->eflags & MFE_VERTICALFLIP)
		fx->eflags |= MFE_VERTICALFLIP;
	else
		fx->eflags &= ~MFE_VERTICALFLIP;
	P_SetScale(fx, mobj1->scale);

	// Because this is done during collision now, rmomx and rmomy need to be recalculated
	// so that friction doesn't immediately decide to stop the player if they're at a standstill
	// Also set justbumped here
	if (mobj1->player)
	{
		mobj1->player->rmomx = mobj1->momx - mobj1->player->cmomx;
		mobj1->player->rmomy = mobj1->momy - mobj1->player->cmomy;
		mobj1->player->kartstuff[k_justbumped] = bumptime;
		if (mobj1->player->kartstuff[k_spinouttimer])
		{
			mobj1->player->kartstuff[k_wipeoutslow] = wipeoutslowtime+1;
			mobj1->player->kartstuff[k_spinouttimer] = max(wipeoutslowtime+1, mobj1->player->kartstuff[k_spinouttimer]);
		}
	}

	if (mobj2->player)
	{
		mobj2->player->rmomx = mobj2->momx - mobj2->player->cmomx;
		mobj2->player->rmomy = mobj2->momy - mobj2->player->cmomy;
		mobj2->player->kartstuff[k_justbumped] = bumptime;
		if (mobj2->player->kartstuff[k_spinouttimer])
		{
			mobj2->player->kartstuff[k_wipeoutslow] = wipeoutslowtime+1;
			mobj2->player->kartstuff[k_spinouttimer] = max(wipeoutslowtime+1, mobj2->player->kartstuff[k_spinouttimer]);
		}
	}
}

/**	\brief	Checks that the player is on an offroad subsector for realsies. Also accounts for line riding to prevent cheese.

	\param	mo	player mobj object

	\return	boolean
*/
static UINT8 K_CheckOffroadCollide(mobj_t *mo)
{
	// Check for sectors in touching_sectorlist
	UINT8 i;			// special type iter
	msecnode_t *node;	// touching_sectorlist iter
	sector_t *s;		// main sector shortcut
	sector_t *s2;		// FOF sector shortcut
	ffloor_t *rover;	// FOF

	fixed_t flr;
	fixed_t cel;	// floor & ceiling for height checks to make sure we're touching the offroad sector.

	I_Assert(mo != NULL);
	I_Assert(!P_MobjWasRemoved(mo));

	for (node = mo->touching_sectorlist; node; node = node->m_sectorlist_next)
	{
		if (!node->m_sector)
			break;	// shouldn't happen.

		s = node->m_sector;
		// 1: Check for the main sector, make sure we're on the floor of that sector and see if we can apply offroad.
		// Make arbitrary Z checks because we want to check for 1 sector in particular, we don't want to affect the player if the offroad sector is way below them and they're lineriding a normal sector above.

		flr = P_MobjFloorZ(mo, s, s, mo->x, mo->y, NULL, false, true);
		cel = P_MobjCeilingZ(mo, s, s, mo->x, mo->y, NULL, true, true);	// get Z coords of both floors and ceilings for this sector (this accounts for slopes properly.)
		// NOTE: we don't use P_GetZAt with our x/y directly because the mobj won't have the same height because of its hitbox on the slope. Complex garbage but tldr it doesn't work.

		if ( ((s->flags & SF_FLIPSPECIAL_FLOOR) && mo->z == flr)	// floor check
			|| ((mo->eflags & MFE_VERTICALFLIP && (s->flags & SF_FLIPSPECIAL_CEILING) && (mo->z + mo->height) == cel)) )	// ceiling check.

			for (i = 2; i < 5; i++)	// check for sector special

				if (GETSECSPECIAL(s->special, 1) == i)
					return i-1;	// return offroad type

		// 2: If we're here, we haven't found anything. So let's try looking for FOFs in the sectors using the same logic.
		for (rover = s->ffloors; rover; rover = rover->next)
		{
			if (!(rover->flags & FF_EXISTS))	// This FOF doesn't exist anymore.
				continue;

			s2 = &sectors[rover->secnum];	// makes things easier for us

			flr = P_GetFOFBottomZ(mo, s, rover, mo->x, mo->y, NULL);
			cel = P_GetFOFTopZ(mo, s, rover, mo->x, mo->y, NULL);	// Z coords for fof top/bottom.

			// we will do essentially the same checks as above instead of bothering with top/bottom height of the FOF.
			// Reminder that an FOF's floor is its bottom, silly!
			if ( ((s2->flags & SF_FLIPSPECIAL_FLOOR) && mo->z == cel)	// "floor" check
				|| ((s2->flags & SF_FLIPSPECIAL_CEILING) && (mo->z + mo->height) == flr) )	// "ceiling" check.

				for (i = 2; i < 5; i++)	// check for sector special

					if (GETSECSPECIAL(s2->special, 1) == i)
						return i-1;	// return offroad type

		}
	}
	return 0;	// couldn't find any offroad
}

/**	\brief	Updates the Player's offroad value once per frame

	\param	player	player object passed from K_KartPlayerThink

	\return	void
*/
static void K_UpdateOffroad(player_t *player)
{
	fixed_t offroad;
	UINT8 offroadstrength = K_CheckOffroadCollide(player->mo);

	// If you are in offroad, a timer starts.
	if (offroadstrength)
	{
		if (/*K_CheckOffroadCollide(player->mo) &&*/ player->kartstuff[k_offroad] == 0)	// With the way offroad is detected now that first check is no longer necessary. -Lat'
			player->kartstuff[k_offroad] = (TICRATE/2);

		if (player->kartstuff[k_offroad] > 0)
		{
			offroad = (offroadstrength << FRACBITS) / (TICRATE/2);

			//if (player->kartstuff[k_growshrinktimer] > 1) // grow slows down half as fast
			//	offroad /= 2;

			player->kartstuff[k_offroad] += offroad;
		}

		if (player->kartstuff[k_offroad] > (offroadstrength << FRACBITS))
			player->kartstuff[k_offroad] = (offroadstrength << FRACBITS);
	}
	else
		player->kartstuff[k_offroad] = 0;
}

// Adds gravity flipping to an object relative to its master and shifts the z coordinate accordingly.
void K_FlipFromObject(mobj_t *mo, mobj_t *master)
{
	mo->eflags = (mo->eflags & ~MFE_VERTICALFLIP)|(master->eflags & MFE_VERTICALFLIP);
	mo->flags2 = (mo->flags2 & ~MF2_OBJECTFLIP)|(master->flags2 & MF2_OBJECTFLIP);

	if (mo->eflags & MFE_VERTICALFLIP)
		mo->z += master->height - FixedMul(master->scale, mo->height);
}

// These have to go earlier than its sisters because of K_RespawnChecker...
void K_MatchGenericExtraFlags(mobj_t *mo, mobj_t *master)
{
	// flipping
	// handle z shifting from there too. This is here since there's no reason not to flip us if needed when we do this anyway;
	K_FlipFromObject(mo, master);

	// visibility (usually for hyudoro)
	mo->flags2 = (mo->flags2 & ~MF2_DONTDRAW)|(master->flags2 & MF2_DONTDRAW);
	mo->eflags = (mo->eflags & ~MFE_DRAWONLYFORP1)|(master->eflags & MFE_DRAWONLYFORP1);
	mo->eflags = (mo->eflags & ~MFE_DRAWONLYFORP2)|(master->eflags & MFE_DRAWONLYFORP2);
	mo->eflags = (mo->eflags & ~MFE_DRAWONLYFORP3)|(master->eflags & MFE_DRAWONLYFORP3);
	mo->eflags = (mo->eflags & ~MFE_DRAWONLYFORP4)|(master->eflags & MFE_DRAWONLYFORP4);
}

static void K_SpawnDashDustRelease(player_t *player)
{
	fixed_t newx;
	fixed_t newy;
	mobj_t *dust;
	angle_t travelangle;
	INT32 i;

	I_Assert(player != NULL);
	I_Assert(player->mo != NULL);
	I_Assert(!P_MobjWasRemoved(player->mo));

	if (!P_IsObjectOnGround(player->mo))
		return;

	if (!player->speed && !player->kartstuff[k_startboost])
		return;

	travelangle = player->mo->angle;

	if (player->kartstuff[k_drift] || player->kartstuff[k_driftend])
		travelangle -= (ANGLE_45/5)*player->kartstuff[k_drift];

	for (i = 0; i < 2; i++)
	{
		newx = player->mo->x + P_ReturnThrustX(player->mo, travelangle + ((i&1) ? -1 : 1)*ANGLE_90, FixedMul(48*FRACUNIT, player->mo->scale));
		newy = player->mo->y + P_ReturnThrustY(player->mo, travelangle + ((i&1) ? -1 : 1)*ANGLE_90, FixedMul(48*FRACUNIT, player->mo->scale));
		dust = P_SpawnMobj(newx, newy, player->mo->z, MT_FASTDUST);

		P_SetTarget(&dust->target, player->mo);
		dust->angle = travelangle - ((i&1) ? -1 : 1)*ANGLE_45;
		dust->destscale = player->mo->scale;
		P_SetScale(dust, player->mo->scale);

		dust->momx = 3*player->mo->momx/5;
		dust->momy = 3*player->mo->momy/5;
		//dust->momz = 3*player->mo->momz/5;

		K_MatchGenericExtraFlags(dust, player->mo);
	}
}

static void K_SpawnBrakeDriftSparks(player_t *player) // Be sure to update the mobj thinker case too!
{
	mobj_t *sparks;

	I_Assert(player != NULL);
	I_Assert(player->mo != NULL);
	I_Assert(!P_MobjWasRemoved(player->mo));

	// Position & etc are handled in its thinker, and its spawned invisible.
	// This avoids needing to dupe code if we don't need it.
	sparks = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_BRAKEDRIFT);
	P_SetTarget(&sparks->target, player->mo);
	P_SetScale(sparks, (sparks->destscale = player->mo->scale));
	K_MatchGenericExtraFlags(sparks, player->mo);
	sparks->flags2 |= MF2_DONTDRAW;
}

/**	\brief	Calculates the respawn timer and drop-boosting

	\param	player	player object passed from K_KartPlayerThink

	\return	void
*/
void K_RespawnChecker(player_t *player)
{
	ticcmd_t *cmd = &player->cmd;

	if (player->spectator)
		return;

	if (player->kartstuff[k_respawn] > 1)
	{
		player->kartstuff[k_respawn]--;
		player->mo->momz = 0;
		player->powers[pw_flashing] = 2;
		player->powers[pw_nocontrol] = 2;
		if (leveltime % 8 == 0)
		{
			INT32 i;
			if (!mapreset)
				S_StartSound(player->mo, sfx_s3kcas);

			for (i = 0; i < 8; i++)
			{
				mobj_t *mo;
				angle_t newangle;
				fixed_t newx, newy, newz;

				newangle = FixedAngle(((360/8)*i)*FRACUNIT);
				newx = player->mo->x + P_ReturnThrustX(player->mo, newangle, 31<<FRACBITS); // does NOT use scale, since this effect doesn't scale properly
				newy = player->mo->y + P_ReturnThrustY(player->mo, newangle, 31<<FRACBITS);
				if (player->mo->eflags & MFE_VERTICALFLIP)
					newz = player->mo->z + player->mo->height;
				else
					newz = player->mo->z;

				mo = P_SpawnMobj(newx, newy, newz, MT_DEZLASER);
				if (mo)
				{
					if (player->mo->eflags & MFE_VERTICALFLIP)
						mo->eflags |= MFE_VERTICALFLIP;
					P_SetTarget(&mo->target, player->mo);
					mo->angle = newangle+ANGLE_90;
					mo->momz = (8<<FRACBITS) * P_MobjFlip(player->mo);
					P_SetScale(mo, (mo->destscale = FRACUNIT));
				}
			}
		}
	}
	else if (player->kartstuff[k_respawn] == 1)
	{
		if (player->kartstuff[k_growshrinktimer] < 0)
		{
			player->mo->scalespeed = mapobjectscale/TICRATE;
			player->mo->destscale = (6*mapobjectscale)/8;
			if (cv_kartdebugshrink.value && !modeattacking && !player->bot)
				player->mo->destscale = (6*player->mo->destscale)/8;
		}

		if (!P_IsObjectOnGround(player->mo) && !mapreset)
		{
			player->powers[pw_flashing] = K_GetKartFlashing(player);

			// Sal: The old behavior was stupid and prone to accidental usage.
			// Let's rip off Mania instead, and turn this into a Drop Dash!

			if (cmd->buttons & BT_ACCELERATE)
				player->kartstuff[k_dropdash]++;
			else
				player->kartstuff[k_dropdash] = 0;

			if (player->kartstuff[k_dropdash] == TICRATE/4)
				S_StartSound(player->mo, sfx_ddash);

			if ((player->kartstuff[k_dropdash] >= TICRATE/4)
				&& (player->kartstuff[k_dropdash] & 1))
				player->mo->colorized = true;
			else
				player->mo->colorized = false;
		}
		else
		{
			if ((cmd->buttons & BT_ACCELERATE) && (player->kartstuff[k_dropdash] >= TICRATE/4))
			{
				S_StartSound(player->mo, sfx_s23c);
				player->kartstuff[k_startboost] = 50;
				K_SpawnDashDustRelease(player);
			}
			player->mo->colorized = false;
			player->kartstuff[k_dropdash] = 0;
			player->kartstuff[k_respawn] = 0;
		}
	}
}

/**	\brief Handles the state changing for moving players, moved here to eliminate duplicate code

	\param	player	player data

	\return	void
*/
void K_KartMoveAnimation(player_t *player)
{
	ticcmd_t *cmd = &player->cmd;
	// Standing frames - S_KART_STND1   S_KART_STND1_L   S_KART_STND1_R
	if (player->speed == 0)
	{
		if (cmd->driftturn < 0 && !(player->mo->state >= &states[S_KART_STND1_R] && player->mo->state <= &states[S_KART_STND2_R]))
			P_SetPlayerMobjState(player->mo, S_KART_STND1_R);
		else if (cmd->driftturn > 0 && !(player->mo->state >= &states[S_KART_STND1_L] && player->mo->state <= &states[S_KART_STND2_L]))
			P_SetPlayerMobjState(player->mo, S_KART_STND1_L);
		else if (cmd->driftturn == 0 && !(player->mo->state >= &states[S_KART_STND1] && player->mo->state <= &states[S_KART_STND2]))
			P_SetPlayerMobjState(player->mo, S_KART_STND1);
	}
	// Drifting Left - S_KART_DRIFT1_L
	else if (player->kartstuff[k_drift] > 0 && P_IsObjectOnGround(player->mo))
	{
		if (!(player->mo->state >= &states[S_KART_DRIFT1_L] && player->mo->state <= &states[S_KART_DRIFT2_L]))
			P_SetPlayerMobjState(player->mo, S_KART_DRIFT1_L);
	}
	// Drifting Right - S_KART_DRIFT1_R
	else if (player->kartstuff[k_drift] < 0 && P_IsObjectOnGround(player->mo))
	{
		if (!(player->mo->state >= &states[S_KART_DRIFT1_R] && player->mo->state <= &states[S_KART_DRIFT2_R]))
			P_SetPlayerMobjState(player->mo, S_KART_DRIFT1_R);
	}
	// Run frames - S_KART_RUN1   S_KART_RUN1_L   S_KART_RUN1_R
	else if (player->speed > (20*player->mo->scale))
	{
		if (cmd->driftturn < 0 && !(player->mo->state >= &states[S_KART_RUN1_R] && player->mo->state <= &states[S_KART_RUN2_R]))
			P_SetPlayerMobjState(player->mo, S_KART_RUN1_R);
		else if (cmd->driftturn > 0 && !(player->mo->state >= &states[S_KART_RUN1_L] && player->mo->state <= &states[S_KART_RUN2_L]))
			P_SetPlayerMobjState(player->mo, S_KART_RUN1_L);
		else if (cmd->driftturn == 0 && !(player->mo->state >= &states[S_KART_RUN1] && player->mo->state <= &states[S_KART_RUN2]))
			P_SetPlayerMobjState(player->mo, S_KART_RUN1);
	}
	// Walk frames - S_KART_WALK1   S_KART_WALK1_L   S_KART_WALK1_R
	else if (player->speed <= (20*player->mo->scale))
	{
		if (cmd->driftturn < 0 && !(player->mo->state >= &states[S_KART_WALK1_R] && player->mo->state <= &states[S_KART_WALK2_R]))
			P_SetPlayerMobjState(player->mo, S_KART_WALK1_R);
		else if (cmd->driftturn > 0 && !(player->mo->state >= &states[S_KART_WALK1_L] && player->mo->state <= &states[S_KART_WALK2_L]))
			P_SetPlayerMobjState(player->mo, S_KART_WALK1_L);
		else if (cmd->driftturn == 0 && !(player->mo->state >= &states[S_KART_WALK1] && player->mo->state <= &states[S_KART_WALK2]))
			P_SetPlayerMobjState(player->mo, S_KART_WALK1);
	}
}

static void K_TauntVoiceTimers(player_t *player)
{
	if (!player)
		return;

	player->kartstuff[k_tauntvoices] = 6*TICRATE;
	player->kartstuff[k_voices] = 4*TICRATE;
}

static void K_RegularVoiceTimers(player_t *player)
{
	if (!player)
		return;

	player->kartstuff[k_voices] = 4*TICRATE;

	if (player->kartstuff[k_tauntvoices] < 4*TICRATE)
		player->kartstuff[k_tauntvoices] = 4*TICRATE;
}

void K_PlayAttackTaunt(mobj_t *source)
{
	sfxenum_t pick = P_RandomKey(2); // Gotta roll the RNG every time this is called for sync reasons
	boolean tasteful = (!source->player || !source->player->kartstuff[k_tauntvoices]);

	if (cv_kartvoices.value && (tasteful || cv_kartvoices.value == 2))
		S_StartSound(source, sfx_kattk1+pick);

	if (!tasteful)
		return;

	K_TauntVoiceTimers(source->player);
}

void K_PlayBoostTaunt(mobj_t *source)
{
	sfxenum_t pick = P_RandomKey(2); // Gotta roll the RNG every time this is called for sync reasons
	boolean tasteful = (!source->player || !source->player->kartstuff[k_tauntvoices]);

	if (cv_kartvoices.value && (tasteful || cv_kartvoices.value == 2))
		S_StartSound(source, sfx_kbost1+pick);

	if (!tasteful)
		return;

	K_TauntVoiceTimers(source->player);
}

void K_PlayOvertakeSound(mobj_t *source)
{
	boolean tasteful = (!source->player || !source->player->kartstuff[k_voices]);

	if (!G_RaceGametype()) // Only in race
		return;

	// 4 seconds from before race begins, 10 seconds afterwards
	if (leveltime < starttime+(10*TICRATE))
		return;

	if (cv_kartvoices.value && (tasteful || cv_kartvoices.value == 2))
		S_StartSound(source, sfx_kslow);

	if (!tasteful)
		return;

	K_RegularVoiceTimers(source->player);
}

void K_PlayHitEmSound(mobj_t *source)
{
	if (cv_kartvoices.value)
		S_StartSound(source, sfx_khitem);
	else
		S_StartSound(source, sfx_s1c9); // The only lost gameplay functionality with voices disabled

	K_RegularVoiceTimers(source->player);
}

void K_PlayPowerGloatSound(mobj_t *source)
{
	if (cv_kartvoices.value)
		S_StartSound(source, sfx_kgloat);

	K_RegularVoiceTimers(source->player);
}

void K_MomentumToFacing(player_t *player)
{
	angle_t dangle = player->mo->angle - R_PointToAngle2(0, 0, player->mo->momx, player->mo->momy);

	if (dangle > ANGLE_180)
		dangle = InvAngle(dangle);

	// If you aren't on the ground or are moving in too different of a direction don't do this
	if (player->mo->eflags & MFE_JUSTHITFLOOR)
		; // Just hit floor ALWAYS redirects
	else if (!P_IsObjectOnGround(player->mo) || dangle > ANGLE_90)
		return;

	P_Thrust(player->mo, player->mo->angle, player->speed - FixedMul(player->speed, player->mo->friction));
	player->mo->momx = FixedMul(player->mo->momx - player->cmomx, player->mo->friction) + player->cmomx;
	player->mo->momy = FixedMul(player->mo->momy - player->cmomy, player->mo->friction) + player->cmomy;
}

// sets k_boostpower, k_speedboost, and k_accelboost to whatever we need it to be
static void K_GetKartBoostPower(player_t *player)
{
	fixed_t boostpower = FRACUNIT;
	fixed_t speedboost = 0, accelboost = 0;

	if (player->kartstuff[k_spinouttimer] && player->kartstuff[k_wipeoutslow] == 1) // Slow down after you've been bumped
	{
		player->kartstuff[k_boostpower] = player->kartstuff[k_speedboost] = player->kartstuff[k_accelboost] = 0;
		return;
	}

	// Offroad is separate, it's difficult to factor it in with a variable value anyway.
	if (!(player->kartstuff[k_invincibilitytimer] || player->kartstuff[k_hyudorotimer] || player->kartstuff[k_sneakertimer])
		&& player->kartstuff[k_offroad] >= 0)
		boostpower = FixedDiv(boostpower, player->kartstuff[k_offroad] + FRACUNIT);

	if (player->kartstuff[k_bananadrag] > TICRATE)
		boostpower = (4*boostpower)/5;

	// Banana drag/offroad dust
	if (boostpower < FRACUNIT
		&& player->mo && P_IsObjectOnGround(player->mo)
		&& player->speed > 0
		&& !player->spectator)
	{
		K_SpawnWipeoutTrail(player->mo, true);
		if (leveltime % 6 == 0)
			S_StartSound(player->mo, sfx_cdfm70);
	}

	if (player->kartstuff[k_sneakertimer]) // Sneaker
	{
		switch (gamespeed)
		{
			case 0:
				speedboost = max(speedboost, 53740+768);
				break;
			case 2:
				speedboost = max(speedboost, 17294+768);
				break;
			default:
				speedboost = max(speedboost, 32768);
				break;
		}
		accelboost = max(accelboost, 8*FRACUNIT); // + 800%
	}

	if (player->kartstuff[k_invincibilitytimer]) // Invincibility
	{
		speedboost = max(speedboost, 3*FRACUNIT/8); // + 37.5%
		accelboost = max(accelboost, 3*FRACUNIT); // + 300%
	}

	if (player->kartstuff[k_growshrinktimer] > 0) // Grow
	{
		speedboost = max(speedboost, FRACUNIT/5); // + 20%
	}

	if (player->kartstuff[k_driftboost]) // Drift Boost
	{
		speedboost = max(speedboost, FRACUNIT/4); // + 25%
		accelboost = max(accelboost, 4*FRACUNIT); // + 400%
	}

	if (player->kartstuff[k_startboost]) // Startup Boost
	{
		speedboost = max(speedboost, FRACUNIT/4); // + 25%
		accelboost = max(accelboost, 6*FRACUNIT); // + 300%
	}

	// don't average them anymore, this would make a small boost and a high boost less useful
	// just take the highest we want instead

	player->kartstuff[k_boostpower] = boostpower;

	// value smoothing
	if (speedboost > player->kartstuff[k_speedboost])
		player->kartstuff[k_speedboost] = speedboost;
	else
		player->kartstuff[k_speedboost] += (speedboost - player->kartstuff[k_speedboost])/(TICRATE/2);

	player->kartstuff[k_accelboost] = accelboost;
}

fixed_t K_GetKartSpeed(player_t *player, boolean doboostpower)
{
	fixed_t k_speed = 150;
	fixed_t g_cc = FRACUNIT;
	fixed_t xspd = 3072;		// 4.6875 aka 3/64
	UINT8 kartspeed = player->kartspeed;
	fixed_t finalspeed;

	if (doboostpower && !player->kartstuff[k_pogospring] && !P_IsObjectOnGround(player->mo))
		return (75*mapobjectscale); // air speed cap

	switch (gamespeed)
	{
		case 0:
			g_cc = 53248 + xspd; //  50cc =  81.25 + 4.69 =  85.94%
			break;
		case 2:
			g_cc = 77824 + xspd; // 150cc = 118.75 + 4.69 = 123.44%
			break;
		default:
			g_cc = 65536 + xspd; // 100cc = 100.00 + 4.69 = 104.69%
			break;
	}

	if (G_BattleGametype() && player->kartstuff[k_bumper] <= 0)
		kartspeed = 1;

	k_speed += kartspeed*3; // 153 - 177

	finalspeed = FixedMul(FixedMul(k_speed<<14, g_cc), player->mo->scale);

	if (doboostpower)
		return FixedMul(finalspeed, player->kartstuff[k_boostpower]+player->kartstuff[k_speedboost]);
	return finalspeed;
}

fixed_t K_GetKartAccel(player_t *player)
{
	fixed_t k_accel = 32; // 36;
	UINT8 kartspeed = player->kartspeed;

	if (G_BattleGametype() && player->kartstuff[k_bumper] <= 0)
		kartspeed = 1;

	//k_accel += 3 * (9 - kartspeed); // 36 - 60
	k_accel += 4 * (9 - kartspeed); // 32 - 64

	return FixedMul(k_accel, FRACUNIT+player->kartstuff[k_accelboost]);
}

UINT16 K_GetKartFlashing(player_t *player)
{
	UINT16 tics = flashingtics;

	if (!player)
		return tics;

	if (G_BattleGametype())
		tics *= 2;

	tics += (flashingtics/8) * (player->kartspeed);

	return tics;
}

fixed_t K_3dKartMovement(player_t *player, boolean onground, fixed_t forwardmove)
{
	fixed_t accelmax = 4000;
	fixed_t newspeed, oldspeed, finalspeed;
	fixed_t p_speed = K_GetKartSpeed(player, true);
	fixed_t p_accel = K_GetKartAccel(player);

	if (!onground) return 0; // If the player isn't on the ground, there is no change in speed

	// ACCELCODE!!!1!11!
	oldspeed = R_PointToDist2(0, 0, player->rmomx, player->rmomy); // FixedMul(P_AproxDistance(player->rmomx, player->rmomy), player->mo->scale);
	newspeed = FixedDiv(FixedDiv(FixedMul(oldspeed, accelmax - p_accel) + FixedMul(p_speed, p_accel), accelmax), ORIG_FRICTION);

	if (player->kartstuff[k_pogospring]) // Pogo Spring minimum/maximum thrust
	{
		const fixed_t hscale = mapobjectscale /*+ (mapobjectscale - player->mo->scale)*/;
		const fixed_t minspeed = 24*hscale;
		const fixed_t maxspeed = 28*hscale;

		if (newspeed > maxspeed && player->kartstuff[k_pogospring] == 2)
			newspeed = maxspeed;
		if (newspeed < minspeed)
			newspeed = minspeed;
	}

	finalspeed = newspeed - oldspeed;

	// forwardmove is:
	//  50 while accelerating,
	//  25 while clutching,
	//   0 with no gas, and
	// -25 when only braking.

	finalspeed *= forwardmove/25;
	finalspeed /= 2;

	if (forwardmove < 0 && finalspeed > FRACUNIT*2)
		return finalspeed/2;
	else if (forwardmove < 0)
		return -FRACUNIT/2;

	if (finalspeed < 0)
		finalspeed = 0;

	return finalspeed;
}

void K_DoInstashield(player_t *player)
{
	mobj_t *layera;
	mobj_t *layerb;

	if (player->kartstuff[k_instashield] > 0)
		return;

	player->kartstuff[k_instashield] = 15; // length of instashield animation
	S_StartSound(player->mo, sfx_cdpcm9);

	layera = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_INSTASHIELDA);
	P_SetTarget(&layera->target, player->mo);

	layerb = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_INSTASHIELDB);
	P_SetTarget(&layerb->target, player->mo);
}

void K_SpawnBattlePoints(player_t *source, player_t *victim, UINT8 amount)
{
	statenum_t st;
	mobj_t *pt;

	if (!source || !source->mo)
		return;

	if (amount == 1)
		st = S_BATTLEPOINT1A;
	else if (amount == 2)
		st = S_BATTLEPOINT2A;
	else if (amount == 3)
		st = S_BATTLEPOINT3A;
	else
		return; // NO STATE!

	pt = P_SpawnMobj(source->mo->x, source->mo->y, source->mo->z, MT_BATTLEPOINT);
	P_SetTarget(&pt->target, source->mo);
	P_SetMobjState(pt, st);
	if (victim && victim->skincolor)
		pt->color = victim->skincolor;
	else
		pt->color = source->skincolor;
}

void K_SpinPlayer(player_t *player, mobj_t *source, INT32 type, mobj_t *inflictor, boolean trapitem)
{
	UINT8 scoremultiply = 1;
	// PS: Inflictor is unused for all purposes here and is actually only ever relevant to Lua. It may be nil too.
#ifdef HAVE_BLUA
	boolean force = false;	// Used to check if Lua ShouldSpin should get us damaged reguardless of flashtics or heck knows what.
	UINT8 shouldForce = LUAh_ShouldSpin(player, inflictor, source);
	if (P_MobjWasRemoved(player->mo))
		return; // mobj was removed (in theory that shouldn't happen)
	if (shouldForce == 1)
		force = true;
	else if (shouldForce == 2)
		return;
#else
	static const boolean force = false;
	(void)inflictor;	// in case some weirdo doesn't want Lua.
#endif


	if (!trapitem && G_BattleGametype())
	{
		if (K_IsPlayerWanted(player))
			scoremultiply = 3;
		else if (player->kartstuff[k_bumper] == 1)
			scoremultiply = 2;
	}

	if (player->health <= 0)
		return;

	if (player->powers[pw_flashing] > 0 || player->kartstuff[k_squishedtimer] > 0 || player->kartstuff[k_spinouttimer] > 0
		|| player->kartstuff[k_invincibilitytimer] > 0 || player->kartstuff[k_growshrinktimer] > 0 || player->kartstuff[k_hyudorotimer] > 0
		|| (G_BattleGametype() && ((player->kartstuff[k_bumper] <= 0 && player->kartstuff[k_comebacktimer]) || player->kartstuff[k_comebackmode] == 1)))
	{
		if (!force)	// if shoulddamage force, we go THROUGH that.
		{
			K_DoInstashield(player);
			return;
		}
	}

#ifdef HAVE_BLUA
	if (LUAh_PlayerSpin(player, inflictor, source))	// Let Lua do its thing or overwrite if it wants to. Make sure to let any possible instashield happen because we didn't get "damaged" in this case.
		return;
#endif

	if (source && source != player->mo && source->player)
		K_PlayHitEmSound(source);

	//player->kartstuff[k_sneakertimer] = 0;
	player->kartstuff[k_driftboost] = 0;

	player->kartstuff[k_drift] = 0;
	player->kartstuff[k_driftcharge] = 0;
	player->kartstuff[k_pogospring] = 0;

	if (G_BattleGametype())
	{
		if (source && source->player && player != source->player)
		{
			P_AddPlayerScore(source->player, scoremultiply);
			K_SpawnBattlePoints(source->player, player, scoremultiply);
			if (!trapitem)
			{
				source->player->kartstuff[k_wanted] -= wantedreduce;
				player->kartstuff[k_wanted] -= (wantedreduce/2);
			}
		}

		if (player->kartstuff[k_bumper] > 0)
		{
			if (player->kartstuff[k_bumper] == 1)
			{
				mobj_t *karmahitbox = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_KARMAHITBOX); // Player hitbox is too small!!
				P_SetTarget(&karmahitbox->target, player->mo);
				karmahitbox->destscale = player->mo->scale;
				P_SetScale(karmahitbox, player->mo->scale);
				CONS_Printf(M_GetText("%s lost all of their bumpers!\n"), player_names[player-players]);
			}
			player->kartstuff[k_bumper]--;
			if (K_IsPlayerWanted(player))
				K_CalculateBattleWanted();
		}

		if (!player->kartstuff[k_bumper])
		{
			player->kartstuff[k_comebacktimer] = comebacktime;
			if (player->kartstuff[k_comebackmode] == 2)
			{
				mobj_t *poof = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_EXPLODE);
				S_StartSound(poof, mobjinfo[MT_KARMAHITBOX].seesound);
				player->kartstuff[k_comebackmode] = 0;
			}
		}

		K_CheckBumpers();
	}

	player->kartstuff[k_spinouttype] = type;

	if (player->kartstuff[k_spinouttype] <= 0) // type 0 is spinout, type 1 is wipeout
	{
		// At spinout, player speed is increased to 1/4 their regular speed, moving them forward
		if (player->speed < K_GetKartSpeed(player, true)/4)
			P_InstaThrust(player->mo, player->mo->angle, FixedMul(K_GetKartSpeed(player, true)/4, player->mo->scale));
		S_StartSound(player->mo, sfx_slip);
	}

	player->kartstuff[k_spinouttimer] = (3*TICRATE/2)+2;
	player->powers[pw_flashing] = K_GetKartFlashing(player);

	if (player->mo->state != &states[S_KART_SPIN])
		P_SetPlayerMobjState(player->mo, S_KART_SPIN);

	player->kartstuff[k_instashield] = 15;
	if (cv_kartdebughuddrop.value && !modeattacking)
		K_DropItems(player);
	else
		K_DropHnextList(player);
	return;
}

static void K_RemoveGrowShrink(player_t *player)
{
	if (player->mo && !P_MobjWasRemoved(player->mo))
	{
		if (player->kartstuff[k_growshrinktimer] > 0) // Play Shrink noise
			S_StartSound(player->mo, sfx_kc59);
		else if (player->kartstuff[k_growshrinktimer] < 0) // Play Grow noise
			S_StartSound(player->mo, sfx_kc5a);

		if (player->kartstuff[k_invincibilitytimer] == 0)
			player->mo->color = player->skincolor;

		player->mo->scalespeed = mapobjectscale/TICRATE;
		player->mo->destscale = mapobjectscale;
		if (cv_kartdebugshrink.value && !modeattacking && !player->bot)
			player->mo->destscale = (6*player->mo->destscale)/8;
	}

	player->kartstuff[k_growshrinktimer] = 0;
	player->kartstuff[k_growcancel] = -1;

	P_RestoreMusic(player);
}

void K_SquishPlayer(player_t *player, mobj_t *source, mobj_t *inflictor)
{
	UINT8 scoremultiply = 1;
	// PS: Inflictor is unused for all purposes here and is actually only ever relevant to Lua. It may be nil too.
#ifdef HAVE_BLUA
	boolean force = false;	// Used to check if Lua ShouldSquish should get us damaged reguardless of flashtics or heck knows what.
	UINT8 shouldForce = LUAh_ShouldSquish(player, inflictor, source);
	if (P_MobjWasRemoved(player->mo))
		return; // mobj was removed (in theory that shouldn't happen)
	if (shouldForce == 1)
		force = true;
	else if (shouldForce == 2)
		return;
#else
	static const boolean force = false;
	(void)inflictor;	// Please stop forgetting to put inflictor in yer functions thank -Lat'
#endif

	if (G_BattleGametype())
	{
		if (K_IsPlayerWanted(player))
			scoremultiply = 3;
		else if (player->kartstuff[k_bumper] == 1)
			scoremultiply = 2;
	}

	if (player->health <= 0)
		return;

	if (player->powers[pw_flashing] > 0 || player->kartstuff[k_squishedtimer] > 0 || player->kartstuff[k_invincibilitytimer] > 0
		|| player->kartstuff[k_growshrinktimer] > 0 || player->kartstuff[k_hyudorotimer] > 0
		|| (G_BattleGametype() && ((player->kartstuff[k_bumper] <= 0 && player->kartstuff[k_comebacktimer]) || player->kartstuff[k_comebackmode] == 1)))
	{
		if (!force)	// You know the drill by now.
		{
			K_DoInstashield(player);
			return;
		}
	}

#ifdef HAVE_BLUA
	if (LUAh_PlayerSquish(player, inflictor, source))	// Let Lua do its thing or overwrite if it wants to. Make sure to let any possible instashield happen because we didn't get "damaged" in this case.
		return;
#endif

	player->kartstuff[k_sneakertimer] = 0;
	player->kartstuff[k_driftboost] = 0;

	player->kartstuff[k_drift] = 0;
	player->kartstuff[k_driftcharge] = 0;
	player->kartstuff[k_pogospring] = 0;

	if (G_BattleGametype())
	{
		if (source && source->player && player != source->player)
		{
			P_AddPlayerScore(source->player, scoremultiply);
			K_SpawnBattlePoints(source->player, player, scoremultiply);
			source->player->kartstuff[k_wanted] -= wantedreduce;
			player->kartstuff[k_wanted] -= (wantedreduce/2);
		}

		if (player->kartstuff[k_bumper] > 0)
		{
			if (player->kartstuff[k_bumper] == 1)
			{
				mobj_t *karmahitbox = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_KARMAHITBOX); // Player hitbox is too small!!
				P_SetTarget(&karmahitbox->target, player->mo);
				karmahitbox->destscale = player->mo->scale;
				P_SetScale(karmahitbox, player->mo->scale);
				CONS_Printf(M_GetText("%s lost all of their bumpers!\n"), player_names[player-players]);
			}
			player->kartstuff[k_bumper]--;
			if (K_IsPlayerWanted(player))
				K_CalculateBattleWanted();
		}

		if (!player->kartstuff[k_bumper])
		{
			player->kartstuff[k_comebacktimer] = comebacktime;
			if (player->kartstuff[k_comebackmode] == 2)
			{
				mobj_t *poof = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_EXPLODE);
				S_StartSound(poof, mobjinfo[MT_KARMAHITBOX].seesound);
				player->kartstuff[k_comebackmode] = 0;
			}
		}

		K_CheckBumpers();
	}

	player->kartstuff[k_squishedtimer] = TICRATE;

	// Reduce Shrink timer
	if (player->kartstuff[k_growshrinktimer] < 0)
	{
		player->kartstuff[k_growshrinktimer] += TICRATE;
		if (player->kartstuff[k_growshrinktimer] >= 0)
			K_RemoveGrowShrink(player);
	}

	player->powers[pw_flashing] = K_GetKartFlashing(player);

	player->mo->flags |= MF_NOCLIP;

	if (player->mo->state != &states[S_KART_SQUISH]) // Squash
		P_SetPlayerMobjState(player->mo, S_KART_SQUISH);

	P_PlayRinglossSound(player->mo);

	player->kartstuff[k_instashield] = 15;
	if (cv_kartdebughuddrop.value && !modeattacking)
		K_DropItems(player);
	else
		K_DropHnextList(player);
	return;
}

void K_ExplodePlayer(player_t *player, mobj_t *source, mobj_t *inflictor) // A bit of a hack, we just throw the player up higher here and extend their spinout timer
{
	UINT8 scoremultiply = 1;
#ifdef HAVE_BLUA
	boolean force = false;	// Used to check if Lua ShouldExplode should get us damaged reguardless of flashtics or heck knows what.
	UINT8 shouldForce = LUAh_ShouldExplode(player, inflictor, source);

	if (P_MobjWasRemoved(player->mo))
		return; // mobj was removed (in theory that shouldn't happen)
	if (shouldForce == 1)
		force = true;
	else if (shouldForce == 2)
		return;

#else
	static const boolean force = false;
#endif

	if (G_BattleGametype())
	{
		if (K_IsPlayerWanted(player))
			scoremultiply = 3;
		else if (player->kartstuff[k_bumper] == 1)
			scoremultiply = 2;
	}

	if (player->health <= 0)
		return;

	if (/*player->powers[pw_flashing] > 0 || player->kartstuff[k_squishedtimer] > 0 || player->kartstuff[k_spinouttimer] > 0 // Explosions should combo, because of SPB and Eggman
		||*/player->kartstuff[k_invincibilitytimer] > 0 || player->kartstuff[k_growshrinktimer] > 0 || player->kartstuff[k_hyudorotimer] > 0
		|| (G_BattleGametype() && ((player->kartstuff[k_bumper] <= 0 && player->kartstuff[k_comebacktimer]) || player->kartstuff[k_comebackmode] == 1)))
	{
		if (!force)	// ShouldDamage can bypass that, again.
		{
			K_DoInstashield(player);
			return;
		}
	}

#ifdef HAVE_BLUA
	if (LUAh_PlayerExplode(player, inflictor, source))	// Same thing. Also make sure to let Instashield happen blah blah
		return;
#endif

	if (source && source != player->mo && source->player)
		K_PlayHitEmSound(source);

	player->mo->momz = 18*mapobjectscale*P_MobjFlip(player->mo);	// please stop forgetting mobjflip checks!!!!
	player->mo->momx = player->mo->momy = 0;

	player->kartstuff[k_sneakertimer] = 0;
	player->kartstuff[k_driftboost] = 0;

	player->kartstuff[k_drift] = 0;
	player->kartstuff[k_driftcharge] = 0;
	player->kartstuff[k_pogospring] = 0;

	// This is the only part that SHOULDN'T combo :VVVVV
	if (G_BattleGametype() && !(player->powers[pw_flashing] > 0 || player->kartstuff[k_squishedtimer] > 0 || player->kartstuff[k_spinouttimer] > 0))
	{
		if (source && source->player && player != source->player)
		{
			P_AddPlayerScore(source->player, scoremultiply);
			K_SpawnBattlePoints(source->player, player, scoremultiply);
			source->player->kartstuff[k_wanted] -= wantedreduce;
			player->kartstuff[k_wanted] -= (wantedreduce/2);
		}

		if (player->kartstuff[k_bumper] > 0)
		{
			if (player->kartstuff[k_bumper] == 1)
			{
				mobj_t *karmahitbox = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_KARMAHITBOX); // Player hitbox is too small!!
				P_SetTarget(&karmahitbox->target, player->mo);
				karmahitbox->destscale = player->mo->scale;
				P_SetScale(karmahitbox, player->mo->scale);
				CONS_Printf(M_GetText("%s lost all of their bumpers!\n"), player_names[player-players]);
			}
			player->kartstuff[k_bumper]--;
			if (K_IsPlayerWanted(player))
				K_CalculateBattleWanted();
		}

		if (!player->kartstuff[k_bumper])
		{
			player->kartstuff[k_comebacktimer] = comebacktime;
			if (player->kartstuff[k_comebackmode] == 2)
			{
				mobj_t *poof = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_EXPLODE);
				S_StartSound(poof, mobjinfo[MT_KARMAHITBOX].seesound);
				player->kartstuff[k_comebackmode] = 0;
			}
		}

		K_CheckBumpers();
	}

	player->kartstuff[k_spinouttype] = 1;
	player->kartstuff[k_spinouttimer] = (3*TICRATE/2)+2;

	player->powers[pw_flashing] = K_GetKartFlashing(player);

	if (inflictor && inflictor->type == MT_SPBEXPLOSION && inflictor->extravalue1)
	{
		player->kartstuff[k_spinouttimer] = ((5*player->kartstuff[k_spinouttimer])/2)+1;
		player->mo->momz *= 2;
	}

	if (player->mo->eflags & MFE_UNDERWATER)
		player->mo->momz = (117 * player->mo->momz) / 200;

	if (player->mo->state != &states[S_KART_SPIN])
		P_SetPlayerMobjState(player->mo, S_KART_SPIN);

	P_PlayRinglossSound(player->mo);

	if (P_IsLocalPlayer(player))
	{
		quake.intensity = 64*FRACUNIT;
		quake.time = 5;
	}

	player->kartstuff[k_instashield] = 15;
	K_DropItems(player);

	return;
}

void K_StealBumper(player_t *player, player_t *victim, boolean force)
{
	INT32 newbumper;
	angle_t newangle, diff;
	fixed_t newx, newy;
	mobj_t *newmo;

	if (!G_BattleGametype())
		return;

	if (player->health <= 0 || victim->health <= 0)
		return;

	if (!force)
	{
		if (victim->kartstuff[k_bumper] <= 0) // || player->kartstuff[k_bumper] >= cv_kartbumpers.value+2
			return;

		if (player->kartstuff[k_squishedtimer] > 0 || player->kartstuff[k_spinouttimer] > 0)
			return;

		if (victim->powers[pw_flashing] > 0 || victim->kartstuff[k_squishedtimer] > 0 || victim->kartstuff[k_spinouttimer] > 0
			|| victim->kartstuff[k_invincibilitytimer] > 0 || victim->kartstuff[k_growshrinktimer] > 0 || victim->kartstuff[k_hyudorotimer] > 0)
		{
			K_DoInstashield(victim);
			return;
		}
	}

	if (netgame && player->kartstuff[k_bumper] <= 0)
		CONS_Printf(M_GetText("%s is back in the game!\n"), player_names[player-players]);

	newbumper = player->kartstuff[k_bumper];
	if (newbumper <= 1)
		diff = 0;
	else
		diff = FixedAngle(360*FRACUNIT/newbumper);

	newangle = player->mo->angle;
	newx = player->mo->x + P_ReturnThrustX(player->mo, newangle + ANGLE_180, 64*FRACUNIT);
	newy = player->mo->y + P_ReturnThrustY(player->mo, newangle + ANGLE_180, 64*FRACUNIT);

	newmo = P_SpawnMobj(newx, newy, player->mo->z, MT_BATTLEBUMPER);
	newmo->threshold = newbumper;
	P_SetTarget(&newmo->tracer, victim->mo);
	P_SetTarget(&newmo->target, player->mo);
	newmo->angle = (diff * (newbumper-1));
	newmo->color = victim->skincolor;

	if (newbumper+1 < 2)
		P_SetMobjState(newmo, S_BATTLEBUMPER3);
	else if (newbumper+1 < 3)
		P_SetMobjState(newmo, S_BATTLEBUMPER2);
	else
		P_SetMobjState(newmo, S_BATTLEBUMPER1);

	S_StartSound(player->mo, sfx_3db06);

	player->kartstuff[k_bumper]++;
	player->kartstuff[k_comebackpoints] = 0;
	player->powers[pw_flashing] = K_GetKartFlashing(player);
	player->kartstuff[k_comebacktimer] = comebacktime;

	/*victim->powers[pw_flashing] = K_GetKartFlashing(victim);
	victim->kartstuff[k_comebacktimer] = comebacktime;*/

	victim->kartstuff[k_instashield] = 15;
	if (cv_kartdebughuddrop.value && !modeattacking)
		K_DropItems(victim);
	else
		K_DropHnextList(victim);
	return;
}

// source is the mobj that originally threw the bomb that exploded etc.
// Spawns the sphere around the explosion that handles spinout
void K_SpawnKartExplosion(fixed_t x, fixed_t y, fixed_t z, fixed_t radius, INT32 number, mobjtype_t type, angle_t rotangle, boolean spawncenter, boolean ghostit, mobj_t *source)
{
	mobj_t *mobj;
	mobj_t *ghost = NULL;
	INT32 i;
	TVector v;
	TVector *res;
	fixed_t finalx, finaly, finalz, dist;
	//mobj_t hoopcenter;
	angle_t degrees, fa, closestangle;
	fixed_t mobjx, mobjy, mobjz;

	//hoopcenter.x = x;
	//hoopcenter.y = y;
	//hoopcenter.z = z;

	//hoopcenter.z = z - mobjinfo[type].height/2;

	degrees = FINEANGLES/number;

	closestangle = 0;

	// Create the hoop!
	for (i = 0; i < number; i++)
	{
		fa = (i*degrees);
		v[0] = FixedMul(FINECOSINE(fa),radius);
		v[1] = 0;
		v[2] = FixedMul(FINESINE(fa),radius);
		v[3] = FRACUNIT;

		res = VectorMatrixMultiply(v, *RotateXMatrix(rotangle));
		M_Memcpy(&v, res, sizeof (v));
		res = VectorMatrixMultiply(v, *RotateZMatrix(closestangle));
		M_Memcpy(&v, res, sizeof (v));

		finalx = x + v[0];
		finaly = y + v[1];
		finalz = z + v[2];

		mobj = P_SpawnMobj(finalx, finaly, finalz, type);

		mobj->z -= mobj->height>>1;

		// change angle
		mobj->angle = R_PointToAngle2(mobj->x, mobj->y, x, y);

		// change slope
		dist = P_AproxDistance(P_AproxDistance(x - mobj->x, y - mobj->y), z - mobj->z);

		if (dist < 1)
			dist = 1;

		mobjx = mobj->x;
		mobjy = mobj->y;
		mobjz = mobj->z;

		if (ghostit)
		{
			ghost = P_SpawnGhostMobj(mobj);
			P_SetMobjState(mobj, S_NULL);
			mobj = ghost;
		}

		if (spawncenter)
		{
			mobj->x = x;
			mobj->y = y;
			mobj->z = z;
		}

		mobj->momx = FixedMul(FixedDiv(mobjx - x, dist), FixedDiv(dist, 6*FRACUNIT));
		mobj->momy = FixedMul(FixedDiv(mobjy - y, dist), FixedDiv(dist, 6*FRACUNIT));
		mobj->momz = FixedMul(FixedDiv(mobjz - z, dist), FixedDiv(dist, 6*FRACUNIT));

		if (source && !P_MobjWasRemoved(source))
			P_SetTarget(&mobj->target, source);
	}
}

// Spawns the purely visual explosion
void K_SpawnMineExplosion(mobj_t *source, UINT8 color)
{
	mobj_t *dust;
	mobj_t *truc;
	INT32 speed, speed2;

	INT32 i, radius, height;
	mobj_t *smoldering = P_SpawnMobj(source->x, source->y, source->z, MT_SMOLDERING);
	K_MatchGenericExtraFlags(smoldering, source);

	smoldering->tics = TICRATE*3;
	radius = source->radius>>FRACBITS;
	height = source->height>>FRACBITS;

	if (!color)
		color = SKINCOLOR_KETCHUP;

	for (i = 0; i < 32; i++)
	{
		dust = P_SpawnMobj(source->x, source->y, source->z, MT_SMOKE);
		P_SetMobjState(dust, S_OPAQUESMOKE1);
		dust->angle = (ANGLE_180/16) * i;
		P_SetScale(dust, source->scale);
		dust->destscale = source->scale*10;
		dust->scalespeed = source->scale/12;
		P_InstaThrust(dust, dust->angle, FixedMul(20*FRACUNIT, source->scale));

		truc = P_SpawnMobj(source->x + P_RandomRange(-radius, radius)*FRACUNIT,
			source->y + P_RandomRange(-radius, radius)*FRACUNIT,
			source->z + P_RandomRange(0, height)*FRACUNIT, MT_BOOMEXPLODE);
		K_MatchGenericExtraFlags(truc, source);
		P_SetScale(truc, source->scale);
		truc->destscale = source->scale*6;
		truc->scalespeed = source->scale/12;
		speed = FixedMul(10*FRACUNIT, source->scale)>>FRACBITS;
		truc->momx = P_RandomRange(-speed, speed)*FRACUNIT;
		truc->momy = P_RandomRange(-speed, speed)*FRACUNIT;
		speed = FixedMul(20*FRACUNIT, source->scale)>>FRACBITS;
		truc->momz = P_RandomRange(-speed, speed)*FRACUNIT*P_MobjFlip(truc);
		if (truc->eflags & MFE_UNDERWATER)
			truc->momz = (117 * truc->momz) / 200;
		truc->color = color;
	}

	for (i = 0; i < 16; i++)
	{
		dust = P_SpawnMobj(source->x + P_RandomRange(-radius, radius)*FRACUNIT,
			source->y + P_RandomRange(-radius, radius)*FRACUNIT,
			source->z + P_RandomRange(0, height)*FRACUNIT, MT_SMOKE);
		P_SetMobjState(dust, S_OPAQUESMOKE1);
		P_SetScale(dust, source->scale);
		dust->destscale = source->scale*10;
		dust->scalespeed = source->scale/12;
		dust->tics = 30;
		dust->momz = P_RandomRange(FixedMul(3*FRACUNIT, source->scale)>>FRACBITS, FixedMul(7*FRACUNIT, source->scale)>>FRACBITS)*FRACUNIT;

		truc = P_SpawnMobj(source->x + P_RandomRange(-radius, radius)*FRACUNIT,
			source->y + P_RandomRange(-radius, radius)*FRACUNIT,
			source->z + P_RandomRange(0, height)*FRACUNIT, MT_BOOMPARTICLE);
		K_MatchGenericExtraFlags(truc, source);
		P_SetScale(truc, source->scale);
		truc->destscale = source->scale*5;
		truc->scalespeed = source->scale/12;
		speed = FixedMul(20*FRACUNIT, source->scale)>>FRACBITS;
		truc->momx = P_RandomRange(-speed, speed)*FRACUNIT;
		truc->momy = P_RandomRange(-speed, speed)*FRACUNIT;
		speed = FixedMul(15*FRACUNIT, source->scale)>>FRACBITS;
		speed2 = FixedMul(45*FRACUNIT, source->scale)>>FRACBITS;
		truc->momz = P_RandomRange(speed, speed2)*FRACUNIT*P_MobjFlip(truc);
		if (P_RandomChance(FRACUNIT/2))
			truc->momz = -truc->momz;
		if (truc->eflags & MFE_UNDERWATER)
			truc->momz = (117 * truc->momz) / 200;
		truc->tics = TICRATE*2;
		truc->color = color;
	}
}

static mobj_t *K_SpawnKartMissile(mobj_t *source, mobjtype_t type, angle_t an, INT32 flags2, fixed_t speed)
{
	mobj_t *th;
	fixed_t x, y, z;
	fixed_t finalspeed = speed;
	mobj_t *throwmo;

	if (source->player && source->player->speed > K_GetKartSpeed(source->player, false))
	{
		angle_t input = source->angle - an;
		boolean invert = (input > ANGLE_180);
		if (invert)
			input = InvAngle(input);

		finalspeed = max(speed, FixedMul(speed, FixedMul(
			FixedDiv(source->player->speed, K_GetKartSpeed(source->player, false)), // Multiply speed to be proportional to your own, boosted maxspeed.
			(((180<<FRACBITS) - AngleFixed(input)) / 180) // multiply speed based on angle diff... i.e: don't do this for firing backward :V
			)));
	}

	x = source->x + source->momx + FixedMul(finalspeed, FINECOSINE(an>>ANGLETOFINESHIFT));
	y = source->y + source->momy + FixedMul(finalspeed, FINESINE(an>>ANGLETOFINESHIFT));
	z = source->z; // spawn on the ground please

	if (P_MobjFlip(source) < 0)
	{
		z = source->z+source->height - mobjinfo[type].height;
	}

	th = P_SpawnMobj(x, y, z, type);

	th->flags2 |= flags2;

	th->threshold = 10;

	if (th->info->seesound)
		S_StartSound(source, th->info->seesound);

	P_SetTarget(&th->target, source);

	if (P_IsObjectOnGround(source))
	{
		// floorz and ceilingz aren't properly set to account for FOFs and Polyobjects on spawn
		// This should set it for FOFs
		P_TeleportMove(th, th->x, th->y, th->z);
		// spawn on the ground if the player is on the ground
		if (P_MobjFlip(source) < 0)
		{
			th->z = th->ceilingz - th->height;
			th->eflags |= MFE_VERTICALFLIP;
		}
		else
			th->z = th->floorz;
	}

	th->angle = an;
	th->momx = FixedMul(finalspeed, FINECOSINE(an>>ANGLETOFINESHIFT));
	th->momy = FixedMul(finalspeed, FINESINE(an>>ANGLETOFINESHIFT));

	switch (type)
	{
		case MT_ORBINAUT:
			if (source && source->player)
				th->color = source->player->skincolor;
			else
				th->color = SKINCOLOR_GREY;
			th->movefactor = finalspeed;
			break;
		case MT_JAWZ:
			if (source && source->player)
			{
				INT32 lasttarg = source->player->kartstuff[k_lastjawztarget];
				th->cvmem = source->player->skincolor;
				if ((lasttarg >= 0 && lasttarg < MAXPLAYERS)
					&& playeringame[lasttarg]
					&& !players[lasttarg].spectator
					&& players[lasttarg].mo)
				{
					P_SetTarget(&th->tracer, players[lasttarg].mo);
				}
			}
			else
				th->cvmem = SKINCOLOR_KETCHUP;
			/* FALLTHRU */
		case MT_JAWZ_DUD:
			S_StartSound(th, th->info->activesound);
			/* FALLTHRU */
		case MT_SPB:
			th->movefactor = finalspeed;
			break;
		default:
			break;
	}

	x = x + P_ReturnThrustX(source, an, source->radius + th->radius);
	y = y + P_ReturnThrustY(source, an, source->radius + th->radius);
	throwmo = P_SpawnMobj(x, y, z, MT_FIREDITEM);
	throwmo->movecount = 1;
	throwmo->movedir = source->angle - an;
	P_SetTarget(&throwmo->target, source);

	return NULL;
}

static void K_SpawnDriftSparks(player_t *player)
{
	fixed_t newx;
	fixed_t newy;
	mobj_t *spark;
	angle_t travelangle;
	INT32 i;

	I_Assert(player != NULL);
	I_Assert(player->mo != NULL);
	I_Assert(!P_MobjWasRemoved(player->mo));

	if (leveltime % 2 == 1)
		return;

	if (!P_IsObjectOnGround(player->mo))
		return;

	if (!player->kartstuff[k_drift] || player->kartstuff[k_driftcharge] < K_GetKartDriftSparkValue(player))
		return;

	travelangle = player->mo->angle-(ANGLE_45/5)*player->kartstuff[k_drift];

	for (i = 0; i < 2; i++)
	{
		newx = player->mo->x + P_ReturnThrustX(player->mo, travelangle + ((i&1) ? -1 : 1)*ANGLE_135, FixedMul(32*FRACUNIT, player->mo->scale));
		newy = player->mo->y + P_ReturnThrustY(player->mo, travelangle + ((i&1) ? -1 : 1)*ANGLE_135, FixedMul(32*FRACUNIT, player->mo->scale));
		spark = P_SpawnMobj(newx, newy, player->mo->z, MT_DRIFTSPARK);

		P_SetTarget(&spark->target, player->mo);
		spark->angle = travelangle-(ANGLE_45/5)*player->kartstuff[k_drift];
		spark->destscale = player->mo->scale;
		P_SetScale(spark, player->mo->scale);

		spark->momx = player->mo->momx/2;
		spark->momy = player->mo->momy/2;
		//spark->momz = player->mo->momz/2;

		if (player->kartstuff[k_driftcharge] >= K_GetKartDriftSparkValue(player)*4)
		{
			spark->color = (UINT8)(1 + (leveltime % (MAXSKINCOLORS-1)));
		}
		else if (player->kartstuff[k_driftcharge] >= K_GetKartDriftSparkValue(player)*2)
		{
			if (player->kartstuff[k_driftcharge] <= (K_GetKartDriftSparkValue(player)*2)+(24*3))
				spark->color = SKINCOLOR_RASPBERRY; // transition
			else
				spark->color = SKINCOLOR_KETCHUP;
		}
		else
		{
			spark->color = SKINCOLOR_SAPPHIRE;
		}

		if ((player->kartstuff[k_drift] > 0 && player->cmd.driftturn > 0) // Inward drifts
			|| (player->kartstuff[k_drift] < 0 && player->cmd.driftturn < 0))
		{
			if ((player->kartstuff[k_drift] < 0 && (i & 1))
				|| (player->kartstuff[k_drift] > 0 && !(i & 1)))
				P_SetMobjState(spark, S_DRIFTSPARK_A1);
			else if ((player->kartstuff[k_drift] < 0 && !(i & 1))
				|| (player->kartstuff[k_drift] > 0 && (i & 1)))
				P_SetMobjState(spark, S_DRIFTSPARK_C1);
		}
		else if ((player->kartstuff[k_drift] > 0 && player->cmd.driftturn < 0) // Outward drifts
			|| (player->kartstuff[k_drift] < 0 && player->cmd.driftturn > 0))
		{
			if ((player->kartstuff[k_drift] < 0 && (i & 1))
				|| (player->kartstuff[k_drift] > 0 && !(i & 1)))
				P_SetMobjState(spark, S_DRIFTSPARK_C1);
			else if ((player->kartstuff[k_drift] < 0 && !(i & 1))
				|| (player->kartstuff[k_drift] > 0 && (i & 1)))
				P_SetMobjState(spark, S_DRIFTSPARK_A1);
		}

		K_MatchGenericExtraFlags(spark, player->mo);
	}
}

static void K_SpawnAIZDust(player_t *player)
{
	fixed_t newx;
	fixed_t newy;
	mobj_t *spark;
	angle_t travelangle;

	I_Assert(player != NULL);
	I_Assert(player->mo != NULL);
	I_Assert(!P_MobjWasRemoved(player->mo));

	if (leveltime % 2 == 1)
		return;

	if (!P_IsObjectOnGround(player->mo))
		return;

	travelangle = R_PointToAngle2(0, 0, player->mo->momx, player->mo->momy);
	//S_StartSound(player->mo, sfx_s3k47);

	{
		newx = player->mo->x + P_ReturnThrustX(player->mo, travelangle - (player->kartstuff[k_aizdriftstrat]*ANGLE_45), FixedMul(24*FRACUNIT, player->mo->scale));
		newy = player->mo->y + P_ReturnThrustY(player->mo, travelangle - (player->kartstuff[k_aizdriftstrat]*ANGLE_45), FixedMul(24*FRACUNIT, player->mo->scale));
		spark = P_SpawnMobj(newx, newy, player->mo->z, MT_AIZDRIFTSTRAT);

		spark->angle = travelangle+(player->kartstuff[k_aizdriftstrat]*ANGLE_90);
		P_SetScale(spark, (spark->destscale = (3*player->mo->scale)>>2));

		spark->momx = (6*player->mo->momx)/5;
		spark->momy = (6*player->mo->momy)/5;
		//spark->momz = player->mo->momz/2;

		K_MatchGenericExtraFlags(spark, player->mo);
	}
}

void K_SpawnBoostTrail(player_t *player)
{
	fixed_t newx;
	fixed_t newy;
	fixed_t ground;
	mobj_t *flame;
	angle_t travelangle;
	INT32 i;

	I_Assert(player != NULL);
	I_Assert(player->mo != NULL);
	I_Assert(!P_MobjWasRemoved(player->mo));

	if (!P_IsObjectOnGround(player->mo)
		|| player->kartstuff[k_hyudorotimer] != 0
		|| (G_BattleGametype() && player->kartstuff[k_bumper] <= 0 && player->kartstuff[k_comebacktimer]))
		return;

	if (player->mo->eflags & MFE_VERTICALFLIP)
		ground = player->mo->ceilingz - FixedMul(mobjinfo[MT_SNEAKERTRAIL].height, player->mo->scale);
	else
		ground = player->mo->floorz;

	if (player->kartstuff[k_drift] != 0)
		travelangle = player->mo->angle;
	else
		travelangle = R_PointToAngle2(0, 0, player->rmomx, player->rmomy);

	for (i = 0; i < 2; i++)
	{
		newx = player->mo->x + P_ReturnThrustX(player->mo, travelangle + ((i&1) ? -1 : 1)*ANGLE_135, FixedMul(24*FRACUNIT, player->mo->scale));
		newy = player->mo->y + P_ReturnThrustY(player->mo, travelangle + ((i&1) ? -1 : 1)*ANGLE_135, FixedMul(24*FRACUNIT, player->mo->scale));
		if (player->mo->standingslope)
		{
			ground = P_GetZAt(player->mo->standingslope, newx, newy);
			if (player->mo->eflags & MFE_VERTICALFLIP)
				ground -= FixedMul(mobjinfo[MT_SNEAKERTRAIL].height, player->mo->scale);
		}
		flame = P_SpawnMobj(newx, newy, ground, MT_SNEAKERTRAIL);

		P_SetTarget(&flame->target, player->mo);
		flame->angle = travelangle;
		flame->fuse = TICRATE*2;
		flame->destscale = player->mo->scale;
		P_SetScale(flame, player->mo->scale);
		// not K_MatchGenericExtraFlags so that a stolen sneaker can be seen
		K_FlipFromObject(flame, player->mo);

		flame->momx = 8;
		P_XYMovement(flame);
		if (P_MobjWasRemoved(flame))
			continue;

		if (player->mo->eflags & MFE_VERTICALFLIP)
		{
			if (flame->z + flame->height < flame->ceilingz)
				P_RemoveMobj(flame);
		}
		else if (flame->z > flame->floorz)
			P_RemoveMobj(flame);
	}
}

void K_SpawnSparkleTrail(mobj_t *mo)
{
	const INT32 rad = (mo->radius*2)>>FRACBITS;
	mobj_t *sparkle;
	INT32 i;

	I_Assert(mo != NULL);
	I_Assert(!P_MobjWasRemoved(mo));

	for (i = 0; i < 3; i++)
	{
		fixed_t newx = mo->x + mo->momx + (P_RandomRange(-rad, rad)<<FRACBITS);
		fixed_t newy = mo->y + mo->momy + (P_RandomRange(-rad, rad)<<FRACBITS);
		fixed_t newz = mo->z + mo->momz + (P_RandomRange(0, mo->height>>FRACBITS)<<FRACBITS);

		sparkle = P_SpawnMobj(newx, newy, newz, MT_SPARKLETRAIL);
		K_FlipFromObject(sparkle, mo);

		//if (i == 0)
			//P_SetMobjState(sparkle, S_KARTINVULN_LARGE1);

		P_SetTarget(&sparkle->target, mo);
		sparkle->destscale = mo->destscale;
		P_SetScale(sparkle, mo->scale);
		sparkle->color = mo->color;
		//sparkle->colorized = mo->colorized;
	}

	P_SetMobjState(sparkle, S_KARTINVULN_LARGE1);
}

void K_SpawnWipeoutTrail(mobj_t *mo, boolean translucent)
{
	mobj_t *dust;
	angle_t aoff;

	I_Assert(mo != NULL);
	I_Assert(!P_MobjWasRemoved(mo));

	if (mo->player)
		aoff = (mo->player->frameangle + ANGLE_180);
	else
		aoff = (mo->angle + ANGLE_180);

	if ((leveltime / 2) & 1)
		aoff -= ANGLE_45;
	else
		aoff += ANGLE_45;

	dust = P_SpawnMobj(mo->x + FixedMul(24*mo->scale, FINECOSINE(aoff>>ANGLETOFINESHIFT)) + (P_RandomRange(-8,8) << FRACBITS),
		mo->y + FixedMul(24*mo->scale, FINESINE(aoff>>ANGLETOFINESHIFT)) + (P_RandomRange(-8,8) << FRACBITS),
		mo->z, MT_WIPEOUTTRAIL);

	P_SetTarget(&dust->target, mo);
	dust->angle = R_PointToAngle2(0,0,mo->momx,mo->momy);
	dust->destscale = mo->scale;
	P_SetScale(dust, mo->scale);
	K_FlipFromObject(dust, mo);

	if (translucent) // offroad effect
	{
		dust->momx = mo->momx/2;
		dust->momy = mo->momy/2;
		dust->momz = mo->momz/2;
	}

	if (translucent)
		dust->flags2 |= MF2_SHADOW;
}

//	K_DriftDustHandling
//	Parameters:
//		spawner: The map object that is spawning the drift dust
//	Description: Spawns the drift dust for objects, players use rmomx/y, other objects use regular momx/y.
//		Also plays the drift sound.
//		Other objects should be angled towards where they're trying to go so they don't randomly spawn dust
//		Do note that most of the function won't run in odd intervals of frames
void K_DriftDustHandling(mobj_t *spawner)
{
	angle_t anglediff;
	const INT16 spawnrange = spawner->radius>>FRACBITS;

	if (!P_IsObjectOnGround(spawner) || leveltime % 2 != 0)
		return;

	if (spawner->player)
	{
		if (spawner->player->pflags & PF_SKIDDOWN)
		{
			anglediff = abs((signed)(spawner->angle - spawner->player->frameangle));
			if (leveltime % 6 == 0)
				S_StartSound(spawner, sfx_screec); // repeated here because it doesn't always happen to be within the range when this is the case
		}
		else
		{
			angle_t playerangle = spawner->angle;

			if (spawner->player->speed < 5<<FRACBITS)
				return;

			if (spawner->player->cmd.forwardmove < 0)
				playerangle += ANGLE_180;

			anglediff = abs((signed)(playerangle - R_PointToAngle2(0, 0, spawner->player->rmomx, spawner->player->rmomy)));
		}
	}
	else
	{
		if (P_AproxDistance(spawner->momx, spawner->momy) < 5<<FRACBITS)
			return;

		anglediff = abs((signed)(spawner->angle - R_PointToAngle2(0, 0, spawner->momx, spawner->momy)));
	}

	if (anglediff > ANGLE_180)
		anglediff = InvAngle(anglediff);

	if (anglediff > ANG10*4) // Trying to turn further than 40 degrees
	{
		fixed_t spawnx = P_RandomRange(-spawnrange, spawnrange)<<FRACBITS;
		fixed_t spawny = P_RandomRange(-spawnrange, spawnrange)<<FRACBITS;
		INT32 speedrange = 2;
		mobj_t *dust = P_SpawnMobj(spawner->x + spawnx, spawner->y + spawny, spawner->z, MT_DRIFTDUST);
		dust->momx = FixedMul(spawner->momx + (P_RandomRange(-speedrange, speedrange)<<FRACBITS), 3*(spawner->scale)/4);
		dust->momy = FixedMul(spawner->momy + (P_RandomRange(-speedrange, speedrange)<<FRACBITS), 3*(spawner->scale)/4);
		dust->momz = P_MobjFlip(spawner) * (P_RandomRange(1, 4) * (spawner->scale));
		P_SetScale(dust, spawner->scale/2);
		dust->destscale = spawner->scale * 3;
		dust->scalespeed = spawner->scale/12;

		if (leveltime % 6 == 0)
			S_StartSound(spawner, sfx_screec);

		K_MatchGenericExtraFlags(dust, spawner);
	}
}

static mobj_t *K_FindLastTrailMobj(player_t *player)
{
	mobj_t *trail;

	if (!player || !(trail = player->mo) || !player->mo->hnext || !player->mo->hnext->health)
		return NULL;

	while (trail->hnext && !P_MobjWasRemoved(trail->hnext) && trail->hnext->health)
	{
		trail = trail->hnext;
	}

	return trail;
}

static mobj_t *K_ThrowKartItem(player_t *player, boolean missile, mobjtype_t mapthing, INT32 defaultDir, INT32 altthrow)
{
	mobj_t *mo;
	INT32 dir;
	fixed_t PROJSPEED;
	angle_t newangle;
	fixed_t newx, newy, newz;
	mobj_t *throwmo;

	if (!player)
		return NULL;

	// Figure out projectile speed by game speed
	if (missile && mapthing != MT_BALLHOG) // Trying to keep compatability...
	{
		PROJSPEED = mobjinfo[mapthing].speed;
		if (gamespeed == 0)
			PROJSPEED = FixedMul(PROJSPEED, FRACUNIT-FRACUNIT/4);
		else if (gamespeed == 2)
			PROJSPEED = FixedMul(PROJSPEED, FRACUNIT+FRACUNIT/4);
		PROJSPEED = FixedMul(PROJSPEED, mapobjectscale);
	}
	else
	{
		switch (gamespeed)
		{
			case 0:
				PROJSPEED = 68*mapobjectscale; // Avg Speed is 34
				break;
			case 2:
				PROJSPEED = 96*mapobjectscale; // Avg Speed is 48
				break;
			default:
				PROJSPEED = 82*mapobjectscale; // Avg Speed is 41
				break;
		}
	}

	if (altthrow)
	{
		if (altthrow == 2) // Kitchen sink throwing
		{
#if 0
			if (player->kartstuff[k_throwdir] == 1)
				dir = 3;
			else if (player->kartstuff[k_throwdir] == -1)
				dir = 1;
			else
				dir = 2;
#else
			if (player->kartstuff[k_throwdir] == 1)
				dir = 2;
			else
				dir = 1;
#endif
		}
		else
		{
			if (player->kartstuff[k_throwdir] == 1)
				dir = 2;
			else if (player->kartstuff[k_throwdir] == -1)
				dir = -1;
			else
				dir = 1;
		}
	}
	else
	{
		if (player->kartstuff[k_throwdir] != 0)
			dir = player->kartstuff[k_throwdir];
		else
			dir = defaultDir;
	}

	if (missile) // Shootables
	{
		if (mapthing == MT_BALLHOG) // Messy
		{
			if (dir == -1)
			{
				// Shoot backward
				mo = K_SpawnKartMissile(player->mo, mapthing, player->mo->angle + ANGLE_180 - 0x06000000, 0, PROJSPEED/16);
				K_SpawnKartMissile(player->mo, mapthing, player->mo->angle + ANGLE_180 - 0x03000000, 0, PROJSPEED/16);
				K_SpawnKartMissile(player->mo, mapthing, player->mo->angle + ANGLE_180, 0, PROJSPEED/16);
				K_SpawnKartMissile(player->mo, mapthing, player->mo->angle + ANGLE_180 + 0x03000000, 0, PROJSPEED/16);
				K_SpawnKartMissile(player->mo, mapthing, player->mo->angle + ANGLE_180 + 0x06000000, 0, PROJSPEED/16);
			}
			else
			{
				// Shoot forward
				mo = K_SpawnKartMissile(player->mo, mapthing, player->mo->angle - 0x06000000, 0, PROJSPEED);
				K_SpawnKartMissile(player->mo, mapthing, player->mo->angle - 0x03000000, 0, PROJSPEED);
				K_SpawnKartMissile(player->mo, mapthing, player->mo->angle, 0, PROJSPEED);
				K_SpawnKartMissile(player->mo, mapthing, player->mo->angle + 0x03000000, 0, PROJSPEED);
				K_SpawnKartMissile(player->mo, mapthing, player->mo->angle + 0x06000000, 0, PROJSPEED);
			}
		}
		else
		{
			if (dir == -1 && mapthing != MT_SPB)
			{
				// Shoot backward
				mo = K_SpawnKartMissile(player->mo, mapthing, player->mo->angle + ANGLE_180, 0, PROJSPEED/8);
			}
			else
			{
				// Shoot forward
				mo = K_SpawnKartMissile(player->mo, mapthing, player->mo->angle, 0, PROJSPEED);
			}
		}
	}
	else
	{
		player->kartstuff[k_bananadrag] = 0; // RESET timer, for multiple bananas

		if (dir > 0)
		{
			// Shoot forward
			mo = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z + player->mo->height/2, mapthing);
			//K_FlipFromObject(mo, player->mo);
			// These are really weird so let's make it a very specific case to make SURE it works...
			if (player->mo->eflags & MFE_VERTICALFLIP)
			{
				mo->z -= player->mo->height;
				mo->flags2 |= MF2_OBJECTFLIP;
				mo->eflags |= MFE_VERTICALFLIP;
			}

			mo->threshold = 10;
			P_SetTarget(&mo->target, player->mo);

			S_StartSound(player->mo, mo->info->seesound);

			if (mo)
			{
				angle_t fa = player->mo->angle>>ANGLETOFINESHIFT;
				fixed_t HEIGHT = (20 + (dir*10))*mapobjectscale + (player->mo->momz*P_MobjFlip(player->mo));

				P_SetObjectMomZ(mo, HEIGHT, false);
				mo->momx = player->mo->momx + FixedMul(FINECOSINE(fa), PROJSPEED*dir);
				mo->momy = player->mo->momy + FixedMul(FINESINE(fa), PROJSPEED*dir);

				mo->extravalue2 = dir;

				if (mo->eflags & MFE_UNDERWATER)
					mo->momz = (117 * mo->momz) / 200;
			}

			// this is the small graphic effect that plops in you when you throw an item:
			throwmo = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z + player->mo->height/2, MT_FIREDITEM);
			P_SetTarget(&throwmo->target, player->mo);
			// Ditto:
			if (player->mo->eflags & MFE_VERTICALFLIP)
			{
				throwmo->z -= player->mo->height;
				throwmo->flags2 |= MF2_OBJECTFLIP;
				throwmo->eflags |= MFE_VERTICALFLIP;
			}

			throwmo->movecount = 0; // above player
		}
		else
		{
			mobj_t *lasttrail = K_FindLastTrailMobj(player);

			if (lasttrail)
			{
				newx = lasttrail->x;
				newy = lasttrail->y;
				newz = lasttrail->z;
			}
			else
			{
				// Drop it directly behind you.
				fixed_t dropradius = FixedHypot(player->mo->radius, player->mo->radius) + FixedHypot(mobjinfo[mapthing].radius, mobjinfo[mapthing].radius);

				newangle = player->mo->angle;

				newx = player->mo->x + P_ReturnThrustX(player->mo, newangle + ANGLE_180, dropradius);
				newy = player->mo->y + P_ReturnThrustY(player->mo, newangle + ANGLE_180, dropradius);
				newz = player->mo->z;
			}

			mo = P_SpawnMobj(newx, newy, newz, mapthing); // this will never return null because collision isn't processed here
			K_FlipFromObject(mo, player->mo);

			mo->threshold = 10;
			P_SetTarget(&mo->target, player->mo);

			if (P_IsObjectOnGround(player->mo))
			{
				// floorz and ceilingz aren't properly set to account for FOFs and Polyobjects on spawn
				// This should set it for FOFs
				P_TeleportMove(mo, mo->x, mo->y, mo->z); // however, THIS can fuck up your day. just absolutely ruin you.
				if (P_MobjWasRemoved(mo))
					return NULL;

				if (P_MobjFlip(mo) > 0)
				{
					if (mo->floorz > mo->target->z - mo->height)
					{
						mo->z = mo->floorz;
					}
				}
				else
				{
					if (mo->ceilingz < mo->target->z + mo->target->height + mo->height)
					{
						mo->z = mo->ceilingz - mo->height;
					}
				}
			}

			if (player->mo->eflags & MFE_VERTICALFLIP)
				mo->eflags |= MFE_VERTICALFLIP;

			if (mapthing == MT_SSMINE)
				mo->extravalue1 = 49; // Pads the start-up length from 21 frames to a full 2 seconds
		}
	}

	return mo;
}

void K_PuntMine(mobj_t *thismine, mobj_t *punter)
{
	angle_t fa = R_PointToAngle2(0, 0, punter->momx, punter->momy) >> ANGLETOFINESHIFT;
	fixed_t z = 30*mapobjectscale + punter->momz;
	fixed_t spd;
	mobj_t *mine;

	if (!thismine || P_MobjWasRemoved(thismine))
		return;

	//This guarantees you hit a mine being dragged
	if (thismine->type == MT_SSMINE_SHIELD) // Create a new mine, and clean up the old one
	{
		mine = P_SpawnMobj(thismine->x, thismine->y, thismine->z, MT_SSMINE);
		P_SetTarget(&mine->target, thismine->target);
		mine->angle = thismine->angle;
		mine->flags2 = thismine->flags2;
		mine->floorz = thismine->floorz;
		mine->ceilingz = thismine->ceilingz;

		//Since we aren't using P_KillMobj, we need to clean up the hnext reference
		{
			P_SetTarget(&thismine->target->hnext, NULL); //target is the player who owns the mine
			thismine->target->player->kartstuff[k_bananadrag] = 0;
			thismine->target->player->kartstuff[k_itemheld] = 0;

			if (--thismine->target->player->kartstuff[k_itemamount] <= 0)
				thismine->target->player->kartstuff[k_itemtype] = KITEM_NONE;
		}

		P_RemoveMobj(thismine);

	}
	else
		mine = thismine;

	if (!mine || P_MobjWasRemoved(mine))
		return;

	switch (gamespeed)
	{
		case 0:
			spd = 68*mapobjectscale; // Avg Speed is 34
			break;
		case 2:
			spd = 96*mapobjectscale; // Avg Speed is 48
			break;
		default:
			spd = 82*mapobjectscale; // Avg Speed is 41
			break;
	}

	mine->flags |= MF_NOCLIPTHING;

	P_SetMobjState(mine, S_SSMINE_AIR1);
	mine->threshold = 10;
	mine->extravalue1 = 0;
	mine->reactiontime = mine->info->reactiontime;

	mine->momx = punter->momx + FixedMul(FINECOSINE(fa), spd);
	mine->momy = punter->momy + FixedMul(FINESINE(fa), spd);
	mine->momz = P_MobjFlip(mine) * z;

	mine->flags &= ~MF_NOCLIPTHING;
}

#define THUNDERRADIUS 320

static void K_DoThunderShield(player_t *player)
{
	mobj_t *mo;
	int i = 0;
	fixed_t sx;
	fixed_t sy;
	angle_t an;

	S_StartSound(player->mo, sfx_zio3);
	//player->kartstuff[k_thunderanim] = 35;
	P_NukeEnemies(player->mo, player->mo, RING_DIST/4);

	// spawn vertical bolt
	mo = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_THOK);
	P_SetTarget(&mo->target, player->mo);
	P_SetMobjState(mo, S_LZIO11);
	mo->color = SKINCOLOR_TEAL;
	mo->scale = player->mo->scale*3 + (player->mo->scale/2);

	mo = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_THOK);
	P_SetTarget(&mo->target, player->mo);
	P_SetMobjState(mo, S_LZIO21);
	mo->color = SKINCOLOR_CYAN;
	mo->scale = player->mo->scale*3 + (player->mo->scale/2);

	// spawn horizontal bolts;
	for (i=0; i<7; i++)
	{
		mo = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_THOK);
		mo->angle = P_RandomRange(0, 359)*ANG1;
		mo->fuse = P_RandomRange(20, 50);
		P_SetTarget(&mo->target, player->mo);
		P_SetMobjState(mo, S_KLIT1);
	}

	// spawn the radius thing:
	an = ANGLE_22h;
	for (i=0; i<15; i++)
	{
		sx = player->mo->x + FixedMul((player->mo->scale*THUNDERRADIUS), FINECOSINE((an*i)>>ANGLETOFINESHIFT));
		sy = player->mo->y + FixedMul((player->mo->scale*THUNDERRADIUS), FINESINE((an*i)>>ANGLETOFINESHIFT));
		mo = P_SpawnMobj(sx, sy, player->mo->z, MT_THOK);
		mo-> angle = an*i;
		mo->extravalue1 = THUNDERRADIUS;	// Used to know whether we should teleport by radius or something.
		mo->scale = player->mo->scale*3;
		P_SetTarget(&mo->target, player->mo);
		P_SetMobjState(mo, S_KSPARK1);
	}
}

#undef THUNDERRADIUS

static void K_DoHyudoroSteal(player_t *player)
{
	INT32 i, numplayers = 0;
	INT32 playerswappable[MAXPLAYERS];
	INT32 stealplayer = -1; // The player that's getting stolen from
	INT32 prandom = 0;
	boolean sink = P_RandomChance(FRACUNIT/64);

	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (playeringame[i] && players[i].mo && players[i].mo->health > 0 && players[i].playerstate == PST_LIVE
			&& player != &players[i] && !players[i].exiting && !players[i].spectator // Player in-game

			// Can steal from this player
			&& (G_RaceGametype() //&& players[i].kartstuff[k_position] < player->kartstuff[k_position])
			|| (G_BattleGametype() && players[i].kartstuff[k_bumper] > 0))

			// Has an item
			&& (players[i].kartstuff[k_itemtype]
			&& players[i].kartstuff[k_itemamount]
			&& !players[i].kartstuff[k_itemheld]
			&& !players[i].kartstuff[k_itemblink]))
		{
			playerswappable[numplayers] = i;
			numplayers++;
		}
	}

	prandom = P_RandomFixed();
	S_StartSound(player->mo, sfx_s3k92);

	if (sink && numplayers > 0 && cv_kitchensink.value) // BEHOLD THE KITCHEN SINK
	{
		player->kartstuff[k_hyudorotimer] = hyudorotime;
		player->kartstuff[k_stealingtimer] = stealtime;

		player->kartstuff[k_itemtype] = KITEM_KITCHENSINK;
		player->kartstuff[k_itemamount] = 1;
		player->kartstuff[k_itemheld] = 0;
		return;
	}
	else if ((G_RaceGametype() && player->kartstuff[k_position] == 1) || numplayers == 0) // No-one can be stolen from? Oh well...
	{
		player->kartstuff[k_hyudorotimer] = hyudorotime;
		player->kartstuff[k_stealingtimer] = stealtime;
		return;
	}
	else if (numplayers == 1) // With just 2 players, we just need to set the other player to be the one to steal from
	{
		stealplayer = playerswappable[numplayers-1];
	}
	else if (numplayers > 1) // We need to choose between the available candidates for the 2nd player
	{
		stealplayer = playerswappable[prandom%(numplayers-1)];
	}

	if (stealplayer > -1) // Now here's where we do the stealing, has to be done here because we still know the player we're stealing from
	{
		player->kartstuff[k_hyudorotimer] = hyudorotime;
		player->kartstuff[k_stealingtimer] = stealtime;
		players[stealplayer].kartstuff[k_stolentimer] = stealtime;

		player->kartstuff[k_itemtype] = players[stealplayer].kartstuff[k_itemtype];
		player->kartstuff[k_itemamount] = players[stealplayer].kartstuff[k_itemamount];
		player->kartstuff[k_itemheld] = 0;

		players[stealplayer].kartstuff[k_itemtype] = KITEM_NONE;
		players[stealplayer].kartstuff[k_itemamount] = 0;
		players[stealplayer].kartstuff[k_itemheld] = 0;

		if (P_IsLocalPlayer(&players[stealplayer]) && !splitscreen)
			S_StartSound(NULL, sfx_s3k92);
	}
}

void K_DoSneaker(player_t *player, INT32 type)
{
	fixed_t intendedboost;

	switch (gamespeed)
	{
		case 0:
			intendedboost = 53740+768;
			break;
		case 2:
			intendedboost = 17294+768;
			break;
		default:
			intendedboost = 32768;
			break;
	}

	if (!player->kartstuff[k_floorboost] || player->kartstuff[k_floorboost] == 3)
	{
		S_StartSound(player->mo, sfx_cdfm01);
		K_SpawnDashDustRelease(player);
		if (intendedboost > player->kartstuff[k_speedboost])
			player->kartstuff[k_destboostcam] = FixedMul(FRACUNIT, FixedDiv((intendedboost - player->kartstuff[k_speedboost]), intendedboost));
	}

	if (!player->kartstuff[k_sneakertimer])
	{
		if (type == 2)
		{
			if (player->mo->hnext)
			{
				mobj_t *cur = player->mo->hnext;
				while (cur && !P_MobjWasRemoved(cur))
				{
					if (!cur->tracer)
					{
						mobj_t *overlay = P_SpawnMobj(cur->x, cur->y, cur->z, MT_BOOSTFLAME);
						P_SetTarget(&overlay->target, cur);
						P_SetTarget(&cur->tracer, overlay);
						P_SetScale(overlay, (overlay->destscale = 3*cur->scale/4));
						K_FlipFromObject(overlay, cur);
					}
					cur = cur->hnext;
				}
			}
		}
		else
		{
			mobj_t *overlay = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_BOOSTFLAME);
			P_SetTarget(&overlay->target, player->mo);
			P_SetScale(overlay, (overlay->destscale = player->mo->scale));
			K_FlipFromObject(overlay, player->mo);
		}
	}

	player->kartstuff[k_sneakertimer] = sneakertime;

	// set angle for spun out players:
	player->kartstuff[k_boostangle] = (INT32)player->mo->angle;

	if (type != 0)
	{
		player->pflags |= PF_ATTACKDOWN;
		K_PlayBoostTaunt(player->mo);
	}
}

static void K_DoShrink(player_t *user)
{
	INT32 i;

	S_StartSound(user->mo, sfx_kc46); // Sound the BANG!
	user->pflags |= PF_ATTACKDOWN;

	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (!playeringame[i] || players[i].spectator || !players[i].mo)
			continue;
		if (&players[i] == user)
			continue;
		if (players[i].kartstuff[k_position] < user->kartstuff[k_position])
		{
			//P_FlashPal(&players[i], PAL_NUKE, 10);

			// Grow should get taken away.
			if (players[i].kartstuff[k_growshrinktimer] > 0)
				K_RemoveGrowShrink(&players[i]);
			// Don't hit while invulnerable!
			else if (!players[i].kartstuff[k_invincibilitytimer]
				&& players[i].kartstuff[k_growshrinktimer] <= 0
				&& !players[i].kartstuff[k_hyudorotimer])
			{
				// Start shrinking!
				K_DropItems(&players[i]);
				players[i].kartstuff[k_growshrinktimer] = -(20*TICRATE);

				if (players[i].mo && !P_MobjWasRemoved(players[i].mo))
				{
					players[i].mo->scalespeed = mapobjectscale/TICRATE;
					players[i].mo->destscale = (6*mapobjectscale)/8;
					if (cv_kartdebugshrink.value && !modeattacking && !players[i].bot)
						players[i].mo->destscale = (6*players[i].mo->destscale)/8;
					S_StartSound(players[i].mo, sfx_kc59);
				}
			}
		}
	}
}


void K_DoPogoSpring(mobj_t *mo, fixed_t vertispeed, UINT8 sound)
{
	const fixed_t vscale = mapobjectscale + (mo->scale - mapobjectscale);

	if (mo->player && mo->player->spectator)
		return;

	if (mo->eflags & MFE_SPRUNG)
		return;

	mo->standingslope = NULL;

	mo->eflags |= MFE_SPRUNG;

	if (mo->eflags & MFE_VERTICALFLIP)
		vertispeed *= -1;

	if (vertispeed == 0)
	{
		fixed_t thrust;

		if (mo->player)
		{
			thrust = 3*mo->player->speed/2;
			if (thrust < 48<<FRACBITS)
				thrust = 48<<FRACBITS;
			if (thrust > 72<<FRACBITS)
				thrust = 72<<FRACBITS;
			if (mo->player->kartstuff[k_pogospring] != 2)
			{
				if (mo->player->kartstuff[k_sneakertimer])
					thrust = FixedMul(thrust, 5*FRACUNIT/4);
				else if (mo->player->kartstuff[k_invincibilitytimer])
					thrust = FixedMul(thrust, 9*FRACUNIT/8);
			}
		}
		else
		{
			thrust = FixedDiv(3*P_AproxDistance(mo->momx, mo->momy)/2, 5*FRACUNIT/2);
			if (thrust < 16<<FRACBITS)
				thrust = 16<<FRACBITS;
			if (thrust > 32<<FRACBITS)
				thrust = 32<<FRACBITS;
		}

		mo->momz = P_MobjFlip(mo)*FixedMul(FINESINE(ANGLE_22h>>ANGLETOFINESHIFT), FixedMul(thrust, vscale));
	}
	else
		mo->momz = FixedMul(vertispeed, vscale);

	if (mo->eflags & MFE_UNDERWATER)
		mo->momz = (117 * mo->momz) / 200;

	if (sound)
		S_StartSound(mo, (sound == 1 ? sfx_kc2f : sfx_kpogos));
}

void K_KillBananaChain(mobj_t *banana, mobj_t *inflictor, mobj_t *source)
{
	mobj_t *cachenext;

killnext:
	cachenext = banana->hnext;

	if (banana->health)
	{
		if (banana->eflags & MFE_VERTICALFLIP)
			banana->z -= banana->height;
		else
			banana->z += banana->height;

		S_StartSound(banana, banana->info->deathsound);
		P_KillMobj(banana, inflictor, source);

		P_SetObjectMomZ(banana, 8*FRACUNIT, false);
		if (inflictor)
			P_InstaThrust(banana, R_PointToAngle2(inflictor->x, inflictor->y, banana->x, banana->y)+ANGLE_90, 16*FRACUNIT);
	}

	if ((banana = cachenext))
		goto killnext;
}

// Just for firing/dropping items.
void K_UpdateHnextList(player_t *player, boolean clean)
{
	mobj_t *work = player->mo, *nextwork;

	if (!work)
		return;

	nextwork = work->hnext;

	while ((work = nextwork) && !P_MobjWasRemoved(work))
	{
		nextwork = work->hnext;

		if (!clean && (!work->movedir || work->movedir <= (UINT16)player->kartstuff[k_itemamount]))
			continue;

		P_RemoveMobj(work);
	}
}

// For getting hit!
void K_DropHnextList(player_t *player)
{
	mobj_t *work = player->mo, *nextwork, *dropwork;
	INT32 flip;
	mobjtype_t type;
	boolean orbit, ponground, dropall = true;

	if (!work || P_MobjWasRemoved(work))
		return;

	flip = P_MobjFlip(player->mo);
	ponground = P_IsObjectOnGround(player->mo);

	if (player->kartstuff[k_itemtype] == KITEM_THUNDERSHIELD && player->kartstuff[k_itemamount])
	{
		K_DoThunderShield(player);
		player->kartstuff[k_itemamount] = 0;
		player->kartstuff[k_itemtype] = KITEM_NONE;
		player->kartstuff[k_curshield] = 0;
	}

	nextwork = work->hnext;

	while ((work = nextwork) && !P_MobjWasRemoved(work))
	{
		nextwork = work->hnext;

		switch (work->type)
		{
			// Kart orbit items
			case MT_ORBINAUT_SHIELD:
				orbit = true;
				type = MT_ORBINAUT;
				break;
			case MT_JAWZ_SHIELD:
				orbit = true;
				type = MT_JAWZ_DUD;
				break;
			// Kart trailing items
			case MT_BANANA_SHIELD:
				orbit = false;
				type = MT_BANANA;
				break;
			case MT_SSMINE_SHIELD:
				orbit = false;
				dropall = false;
				type = MT_SSMINE;
				break;
			case MT_EGGMANITEM_SHIELD:
				orbit = false;
				type = MT_EGGMANITEM;
				break;
			// intentionally do nothing
			case MT_SINK_SHIELD:
			case MT_ROCKETSNEAKER:
				return;
			default:
				continue;
		}

		dropwork = P_SpawnMobj(work->x, work->y, work->z, type);
		P_SetTarget(&dropwork->target, player->mo);
		dropwork->angle = work->angle;
		dropwork->flags2 = work->flags2;
		dropwork->flags |= MF_NOCLIPTHING;
		dropwork->floorz = work->floorz;
		dropwork->ceilingz = work->ceilingz;

		if (ponground)
		{
			// floorz and ceilingz aren't properly set to account for FOFs and Polyobjects on spawn
			// This should set it for FOFs
			//P_TeleportMove(dropwork, dropwork->x, dropwork->y, dropwork->z); -- handled better by above floorz/ceilingz passing

			if (flip == 1)
			{
				if (dropwork->floorz > dropwork->target->z - dropwork->height)
				{
					dropwork->z = dropwork->floorz;
				}
			}
			else
			{
				if (dropwork->ceilingz < dropwork->target->z + dropwork->target->height + dropwork->height)
				{
					dropwork->z = dropwork->ceilingz - dropwork->height;
				}
			}
		}

		if (orbit) // splay out
		{
			dropwork->flags2 |= MF2_AMBUSH;
			dropwork->z += flip;
			dropwork->momx = player->mo->momx>>1;
			dropwork->momy = player->mo->momy>>1;
			dropwork->momz = 3*flip*mapobjectscale;
			if (dropwork->eflags & MFE_UNDERWATER)
				dropwork->momz = (117 * dropwork->momz) / 200;
			P_Thrust(dropwork, work->angle - ANGLE_90, 6*mapobjectscale);
			dropwork->movecount = 2;
			dropwork->movedir = work->angle - ANGLE_90;
			P_SetMobjState(dropwork, dropwork->info->deathstate);
			dropwork->tics = -1;
			if (type == MT_JAWZ_DUD)
				dropwork->z += 20*flip*dropwork->scale;
			else
			{
				dropwork->color = work->color;
				dropwork->angle -= ANGLE_90;
			}
		}
		else // plop on the ground
		{
			dropwork->flags &= ~MF_NOCLIPTHING;
			dropwork->threshold = 10;
		}

		P_RemoveMobj(work);
	}

	{
		// we need this here too because this is done in afterthink - pointers are cleaned up at the START of each tic...
		P_SetTarget(&player->mo->hnext, NULL);
		player->kartstuff[k_bananadrag] = 0;
		if (player->kartstuff[k_eggmanheld])
			player->kartstuff[k_eggmanheld] = 0;
		else if (player->kartstuff[k_itemheld]
			&& (dropall || (--player->kartstuff[k_itemamount] <= 0)))
		{
			player->kartstuff[k_itemamount] = player->kartstuff[k_itemheld] = 0;
			player->kartstuff[k_itemtype] = KITEM_NONE;
		}
	}
}

// For getting EXTRA hit!
void K_DropItems(player_t *player)
{
	boolean thunderhack = (player->kartstuff[k_curshield] && player->kartstuff[k_itemtype] == KITEM_THUNDERSHIELD);

	if (thunderhack)
		player->kartstuff[k_itemtype] = KITEM_NONE;

	K_DropHnextList(player);

	if (player->mo && !P_MobjWasRemoved(player->mo) && player->kartstuff[k_itemamount])
	{
		mobj_t *drop = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z + player->mo->height/2, MT_FLOATINGITEM);
		P_SetScale(drop, drop->scale>>4);
		drop->destscale = (3*drop->destscale)/2;;

		drop->angle = player->mo->angle + ANGLE_90;
		P_Thrust(drop,
			FixedAngle(P_RandomFixed()*180) + player->mo->angle + ANGLE_90,
			16*mapobjectscale);
		drop->momz = P_MobjFlip(player->mo)*3*mapobjectscale;
		if (drop->eflags & MFE_UNDERWATER)
			drop->momz = (117 * drop->momz) / 200;

		drop->threshold = (thunderhack ? KITEM_THUNDERSHIELD : player->kartstuff[k_itemtype]);
		drop->movecount = player->kartstuff[k_itemamount];

		drop->flags |= MF_NOCLIPTHING;
	}

	K_StripItems(player);
}

void K_DropRocketSneaker(player_t *player)
{
	mobj_t *shoe = player->mo;
	fixed_t flingangle;
	boolean leftshoe = true; //left shoe is first

	if (!(player->mo && !P_MobjWasRemoved(player->mo) && player->mo->hnext && !P_MobjWasRemoved(player->mo->hnext)))
		return;

	while ((shoe = shoe->hnext) && !P_MobjWasRemoved(shoe))
	{
		if (shoe->type != MT_ROCKETSNEAKER)
			return; //woah, not a rocketsneaker, bail! safeguard in case this gets used when you're holding non-rocketsneakers

		shoe->flags2 &= ~MF2_DONTDRAW;
		shoe->flags &= ~MF_NOGRAVITY;
		shoe->angle += ANGLE_45;

		if (shoe->eflags & MFE_VERTICALFLIP)
			shoe->z -= shoe->height;
		else
			shoe->z += shoe->height;

		//left shoe goes off tot eh left, right shoe off to the right
		if (leftshoe)
			flingangle = -(ANG60);
		else
			flingangle = ANG60;

		S_StartSound(shoe, shoe->info->deathsound);
		P_SetObjectMomZ(shoe, 8*FRACUNIT, false);
		P_InstaThrust(shoe, R_PointToAngle2(shoe->target->x, shoe->target->y, shoe->x, shoe->y)+flingangle, 16*FRACUNIT);
		shoe->momx += shoe->target->momx;
		shoe->momy += shoe->target->momy;
		shoe->momz += shoe->target->momz;
		shoe->extravalue2 = 1;

		leftshoe = false;
	}
	P_SetTarget(&player->mo->hnext, NULL);
	player->kartstuff[k_rocketsneakertimer] = 0;
}

void K_DropKitchenSink(player_t *player)
{
	if (!(player->mo && !P_MobjWasRemoved(player->mo) && player->mo->hnext && !P_MobjWasRemoved(player->mo->hnext)))
		return;

	if (player->mo->hnext->type != MT_SINK_SHIELD)
		return; //so we can just call this function regardless of what is being held

	P_KillMobj(player->mo->hnext, NULL, NULL);

	P_SetTarget(&player->mo->hnext, NULL);
}

// When an item in the hnext chain dies.
void K_RepairOrbitChain(mobj_t *orbit)
{
	mobj_t *cachenext = orbit->hnext;

	// First, repair the chain
	if (orbit->hnext && orbit->hnext->health && !P_MobjWasRemoved(orbit->hnext))
	{
		P_SetTarget(&orbit->hnext->hprev, orbit->hprev);
		P_SetTarget(&orbit->hnext, NULL);
	}

	if (orbit->hprev && orbit->hprev->health && !P_MobjWasRemoved(orbit->hprev))
	{
		P_SetTarget(&orbit->hprev->hnext, cachenext);
		P_SetTarget(&orbit->hprev, NULL);
	}

	// Then recount to make sure item amount is correct
	if (orbit->target && orbit->target->player)
	{
		INT32 num = 0;

		mobj_t *cur = orbit->target->hnext;
		mobj_t *prev = NULL;

		while (cur && !P_MobjWasRemoved(cur))
		{
			prev = cur;
			cur = cur->hnext;
			if (++num > orbit->target->player->kartstuff[k_itemamount])
				P_RemoveMobj(prev);
			else
				prev->movedir = num;
		}

		if (orbit->target->player->kartstuff[k_itemamount] != num)
			orbit->target->player->kartstuff[k_itemamount] = num;
	}
}

// Move the hnext chain!
static void K_MoveHeldObjects(player_t *player)
{
	if (!player->mo)
		return;

	if (!player->mo->hnext)
	{
		player->kartstuff[k_bananadrag] = 0;
		if (player->kartstuff[k_eggmanheld])
			player->kartstuff[k_eggmanheld] = 0;
		else if (player->kartstuff[k_itemheld])
		{
			player->kartstuff[k_itemamount] = player->kartstuff[k_itemheld] = 0;
			player->kartstuff[k_itemtype] = KITEM_NONE;
		}
		return;
	}

	if (P_MobjWasRemoved(player->mo->hnext))
	{
		// we need this here too because this is done in afterthink - pointers are cleaned up at the START of each tic...
		P_SetTarget(&player->mo->hnext, NULL);
		player->kartstuff[k_bananadrag] = 0;
		if (player->kartstuff[k_eggmanheld])
			player->kartstuff[k_eggmanheld] = 0;
		else if (player->kartstuff[k_itemheld])
		{
			player->kartstuff[k_itemamount] = player->kartstuff[k_itemheld] = 0;
			player->kartstuff[k_itemtype] = KITEM_NONE;
		}
		return;
	}

	switch (player->mo->hnext->type)
	{
		case MT_ORBINAUT_SHIELD: // Kart orbit items
		case MT_JAWZ_SHIELD:
			{
				mobj_t *cur = player->mo->hnext;
				fixed_t speed = ((8 - min(4, player->kartstuff[k_itemamount])) * cur->info->speed) / 7;

				player->kartstuff[k_bananadrag] = 0; // Just to make sure

				while (cur && !P_MobjWasRemoved(cur))
				{
					const fixed_t radius = FixedHypot(player->mo->radius, player->mo->radius) + FixedHypot(cur->radius, cur->radius); // mobj's distance from its Target, or Radius.
					fixed_t z;

					if (!cur->health)
					{
						cur = cur->hnext;
						continue;
					}

					cur->color = player->skincolor;

					cur->angle -= ANGLE_90;
					cur->angle += FixedAngle(speed);

					if (cur->extravalue1 < radius)
						cur->extravalue1 += P_AproxDistance(cur->extravalue1, radius) / 12;
					if (cur->extravalue1 > radius)
						cur->extravalue1 = radius;

					// If the player is on the ceiling, then flip your items as well.
					if (player && player->mo->eflags & MFE_VERTICALFLIP)
						cur->eflags |= MFE_VERTICALFLIP;
					else
						cur->eflags &= ~MFE_VERTICALFLIP;

					// Shrink your items if the player shrunk too.
					P_SetScale(cur, (cur->destscale = FixedMul(FixedDiv(cur->extravalue1, radius), player->mo->scale)));

					if (P_MobjFlip(cur) > 0)
						z = player->mo->z;
					else
						z = player->mo->z + player->mo->height - cur->height;

					cur->flags |= MF_NOCLIPTHING; // temporarily make them noclip other objects so they can't hit anyone while in the player
					P_TeleportMove(cur, player->mo->x, player->mo->y, z);
					cur->momx = FixedMul(FINECOSINE(cur->angle>>ANGLETOFINESHIFT), cur->extravalue1);
					cur->momy = FixedMul(FINESINE(cur->angle>>ANGLETOFINESHIFT), cur->extravalue1);
					cur->flags &= ~MF_NOCLIPTHING;
					if (!P_TryMove(cur, player->mo->x + cur->momx, player->mo->y + cur->momy, true))
						P_SlideMove(cur, true);
					if (P_IsObjectOnGround(player->mo))
					{
						if (P_MobjFlip(cur) > 0)
						{
							if (cur->floorz > player->mo->z - cur->height)
								z = cur->floorz;
						}
						else
						{
							if (cur->ceilingz < player->mo->z + player->mo->height + cur->height)
								z = cur->ceilingz - cur->height;
						}
					}

					// Center it during the scale up animation
					z += (FixedMul(mobjinfo[cur->type].height, player->mo->scale - cur->scale)>>1) * P_MobjFlip(cur);

					cur->z = z;
					cur->momx = cur->momy = 0;
					cur->angle += ANGLE_90;

					cur = cur->hnext;
				}
			}
			break;
		case MT_BANANA_SHIELD: // Kart trailing items
		case MT_SSMINE_SHIELD:
		case MT_EGGMANITEM_SHIELD:
		case MT_SINK_SHIELD:
			{
				mobj_t *cur = player->mo->hnext;
				mobj_t *targ = player->mo;

				if (P_IsObjectOnGround(player->mo) && player->speed > 0)
					player->kartstuff[k_bananadrag]++;

				while (cur && !P_MobjWasRemoved(cur))
				{
					const fixed_t radius = FixedHypot(targ->radius, targ->radius) + FixedHypot(cur->radius, cur->radius);
					angle_t ang;
					fixed_t targx, targy, targz;
					fixed_t speed, dist;

					cur->flags &= ~MF_NOCLIPTHING;

					if (!cur->health)
					{
						cur = cur->hnext;
						continue;
					}

					if (cur->extravalue1 < radius)
						cur->extravalue1 += FixedMul(P_AproxDistance(cur->extravalue1, radius), FRACUNIT/12);
					if (cur->extravalue1 > radius)
						cur->extravalue1 = radius;

					if (cur != player->mo->hnext)
					{
						targ = cur->hprev;
						dist = cur->extravalue1/4;
					}
					else
						dist = cur->extravalue1/2;

					if (!targ || P_MobjWasRemoved(targ))
						continue;

					// Shrink your items if the player shrunk too.
					P_SetScale(cur, (cur->destscale = FixedMul(FixedDiv(cur->extravalue1, radius), player->mo->scale)));

					ang = targ->angle;
					targx = targ->x + P_ReturnThrustX(cur, ang + ANGLE_180, dist);
					targy = targ->y + P_ReturnThrustY(cur, ang + ANGLE_180, dist);
					targz = targ->z;

					speed = FixedMul(R_PointToDist2(cur->x, cur->y, targx, targy), 3*FRACUNIT/4);
					if (P_IsObjectOnGround(targ))
						targz = cur->floorz;

					cur->angle = R_PointToAngle2(cur->x, cur->y, targx, targy);

					/*if (P_IsObjectOnGround(player->mo) && player->speed > 0 && player->kartstuff[k_bananadrag] > TICRATE
						&& P_RandomChance(min(FRACUNIT/2, FixedDiv(player->speed, K_GetKartSpeed(player, false))/2)))
					{
						if (leveltime & 1)
							targz += 8*(2*FRACUNIT)/7;
						else
							targz -= 8*(2*FRACUNIT)/7;
					}*/

					if (speed > dist)
						P_InstaThrust(cur, cur->angle, speed-dist);

					P_SetObjectMomZ(cur, FixedMul(targz - cur->z, 7*FRACUNIT/8) - gravity, false);

					if (R_PointToDist2(cur->x, cur->y, targx, targy) > 768*FRACUNIT)
						P_TeleportMove(cur, targx, targy, cur->z);

					cur = cur->hnext;
				}
			}
			break;
		case MT_ROCKETSNEAKER: // Special rocket sneaker stuff
			{
				mobj_t *cur = player->mo->hnext;
				INT32 num = 0;

				while (cur && !P_MobjWasRemoved(cur))
				{
					const fixed_t radius = FixedHypot(player->mo->radius, player->mo->radius) + FixedHypot(cur->radius, cur->radius);
					boolean vibrate = ((leveltime & 1) && !cur->tracer);
					angle_t angoffset;
					fixed_t targx, targy, targz;

					cur->flags &= ~MF_NOCLIPTHING;

					if (player->kartstuff[k_rocketsneakertimer] <= TICRATE && (leveltime & 1))
						cur->flags2 |= MF2_DONTDRAW;
					else
						cur->flags2 &= ~MF2_DONTDRAW;

					if (num & 1)
						P_SetMobjStateNF(cur, (vibrate ? S_ROCKETSNEAKER_LVIBRATE : S_ROCKETSNEAKER_L));
					else
						P_SetMobjStateNF(cur, (vibrate ? S_ROCKETSNEAKER_RVIBRATE : S_ROCKETSNEAKER_R));

					if (!player->kartstuff[k_rocketsneakertimer] || cur->extravalue2 || !cur->health)
					{
						num = (num+1) % 2;
						cur = cur->hnext;
						continue;
					}

					if (cur->extravalue1 < radius)
						cur->extravalue1 += FixedMul(P_AproxDistance(cur->extravalue1, radius), FRACUNIT/12);
					if (cur->extravalue1 > radius)
						cur->extravalue1 = radius;

					// Shrink your items if the player shrunk too.
					P_SetScale(cur, (cur->destscale = FixedMul(FixedDiv(cur->extravalue1, radius), player->mo->scale)));

#if 1
					{
						angle_t input = player->frameangle - cur->angle;
						boolean invert = (input > ANGLE_180);
						if (invert)
							input = InvAngle(input);

						input = FixedAngle(AngleFixed(input)/4);
						if (invert)
							input = InvAngle(input);

						cur->angle = cur->angle + input;
					}
#else
					cur->angle = player->frameangle;
#endif

					angoffset = ANGLE_90 + (ANGLE_180 * num);

					targx = player->mo->x + P_ReturnThrustX(cur, cur->angle + angoffset, cur->extravalue1);
					targy = player->mo->y + P_ReturnThrustY(cur, cur->angle + angoffset, cur->extravalue1);

					{ // bobbing, copy pasted from my kimokawaiii entry
						const fixed_t pi = (22<<FRACBITS) / 7; // loose approximation, this doesn't need to be incredibly precise
						fixed_t sine = FixedMul(player->mo->scale, 8 * FINESINE((((2*pi*(4*TICRATE)) * leveltime)>>ANGLETOFINESHIFT) & FINEMASK));
						targz = (player->mo->z + (player->mo->height/2)) + sine;
						if (player->mo->eflags & MFE_VERTICALFLIP)
							targz += (player->mo->height/2 - 32*player->mo->scale)*6;

					}

					if (cur->tracer)
					{
						fixed_t diffx, diffy, diffz;

						diffx = targx - cur->x;
						diffy = targy - cur->y;
						diffz = targz - cur->z;

						P_TeleportMove(cur->tracer, cur->tracer->x + diffx + P_ReturnThrustX(cur, cur->angle + angoffset, 6*cur->scale),
							cur->tracer->y + diffy + P_ReturnThrustY(cur, cur->angle + angoffset, 6*cur->scale), cur->tracer->z + diffz);
						P_SetScale(cur->tracer, (cur->tracer->destscale = 3*cur->scale/4));
					}

					P_TeleportMove(cur, targx, targy, targz);
					K_FlipFromObject(cur, player->mo);	// Update graviflip in real time thanks.
					num = (num+1) % 2;
					cur = cur->hnext;
				}
			}
			break;
		default:
			break;
	}
}

player_t *K_FindJawzTarget(mobj_t *actor, player_t *source)
{
	fixed_t best = -1;
	player_t *wtarg = NULL;
	INT32 i;

	for (i = 0; i < MAXPLAYERS; i++)
	{
		angle_t thisang;
		player_t *player;

		if (!playeringame[i])
			continue;

		player = &players[i];

		if (player->spectator)
			continue; // spectator

		if (!player->mo)
			continue;

		if (player->mo->health <= 0)
			continue; // dead

		// Don't target yourself, stupid.
		if (player == source)
			continue;

		// Don't home in on teammates.
		if (G_GametypeHasTeams() && source->ctfteam == player->ctfteam)
			continue;

		// Invisible, don't bother
		if (player->kartstuff[k_hyudorotimer])
			continue;

		// Find the angle, see who's got the best.
		thisang = actor->angle - R_PointToAngle2(actor->x, actor->y, player->mo->x, player->mo->y);
		if (thisang > ANGLE_180)
			thisang = InvAngle(thisang);

		// Jawz only go after the person directly ahead of you in race... sort of literally now!
		if (G_RaceGametype())
		{
			// Don't go for people who are behind you
			if (thisang > ANGLE_67h)
				continue;
			// Don't pay attention to people who aren't above your position
			if (player->kartstuff[k_position] >= source->kartstuff[k_position])
				continue;
			if ((best == -1) || (player->kartstuff[k_position] > best))
			{
				wtarg = player;
				best = player->kartstuff[k_position];
			}
		}
		else
		{
			fixed_t thisdist;
			fixed_t thisavg;

			// Don't go for people who are behind you
			if (thisang > ANGLE_45)
				continue;

			// Don't pay attention to dead players
			if (player->kartstuff[k_bumper] <= 0)
				continue;

			// Z pos too high/low
			if (abs(player->mo->z - (actor->z + actor->momz)) > RING_DIST/8)
				continue;

			thisdist = P_AproxDistance(player->mo->x - (actor->x + actor->momx), player->mo->y - (actor->y + actor->momy));

			if (thisdist > 2*RING_DIST) // Don't go for people who are too far away
				continue;

			thisavg = (AngleFixed(thisang) + thisdist) / 2;

			//CONS_Printf("got avg %d from player # %d\n", thisavg>>FRACBITS, i);

			if ((best == -1) || (thisavg < best))
			{
				wtarg = player;
				best = thisavg;
			}
		}
	}

	return wtarg;
}

// Engine Sounds.
static void K_UpdateEngineSounds(player_t *player, ticcmd_t *cmd)
{
	const INT32 numsnds = 13;
	INT32 class, s, w; // engine class number
	UINT8 volume = 255;
	fixed_t volumedampen = 0;
	INT32 targetsnd = 0;
	INT32 i;

	s = (player->kartspeed-1)/3;
	w = (player->kartweight-1)/3;

#define LOCKSTAT(stat) \
	if (stat < 0) { stat = 0; } \
	if (stat > 2) { stat = 2; }
	LOCKSTAT(s);
	LOCKSTAT(w);
#undef LOCKSTAT

	class = s+(3*w);

	// Silence the engines
	if (leveltime < 8 || player->spectator || player->exiting)
	{
		player->kartstuff[k_enginesnd] = 0; // Reset sound number
		return;
	}

#if 0
	if ((leveltime % 8) != ((player-players) % 8)) // Per-player offset, to make engines sound distinct!
#else
	if (leveltime % 8) // .25 seconds of wait time between engine sounds
#endif
		return;

	if ((leveltime >= starttime-(2*TICRATE) && leveltime <= starttime) || (player->kartstuff[k_respawn] == 1)) // Startup boosts
		targetsnd = ((cmd->buttons & BT_ACCELERATE) ? 12 : 0);
	else
		targetsnd = (((6*cmd->forwardmove)/25) + ((player->speed / mapobjectscale)/5))/2;

	if (targetsnd < 0)
		targetsnd = 0;
	if (targetsnd > 12)
		targetsnd = 12;

	if (player->kartstuff[k_enginesnd] < targetsnd)
		player->kartstuff[k_enginesnd]++;
	if (player->kartstuff[k_enginesnd] > targetsnd)
		player->kartstuff[k_enginesnd]--;

	if (player->kartstuff[k_enginesnd] < 0)
		player->kartstuff[k_enginesnd] = 0;
	if (player->kartstuff[k_enginesnd] > 12)
		player->kartstuff[k_enginesnd] = 12;

	for (i = 0; i < MAXPLAYERS; i++)
	{
		UINT8 thisvol = 0;
		fixed_t dist;

		if (!playeringame[i] || !players[i].mo || players[i].spectator || players[i].exiting)
			continue;

		if (P_IsDisplayPlayer(&players[i]))
		{
			volumedampen += FRACUNIT; // We already know what this is gonna be, let's not waste our time.
			continue;
		}

		dist = P_AproxDistance(P_AproxDistance(player->mo->x-players[i].mo->x,
			player->mo->y-players[i].mo->y), player->mo->z-players[i].mo->z) / 2;

		dist = FixedDiv(dist, mapobjectscale);

		if (dist > 1536<<FRACBITS)
			continue;
		else if (dist < 160<<FRACBITS) // engine sounds' approx. range
			thisvol = 255;
		else
			thisvol = (15 * (((160<<FRACBITS) - dist)>>FRACBITS)) / (((1536<<FRACBITS)-(160<<FRACBITS))>>(FRACBITS+4));

		if (thisvol == 0)
			continue;

		volumedampen += (thisvol * 257); // 255 * 257 = FRACUNIT
	}

	if (volumedampen > FRACUNIT)
		volume = FixedDiv(volume<<FRACBITS, volumedampen)>>FRACBITS;

	if (volume <= 0) // Might as well
		return;

	S_StartSoundAtVolume(player->mo, (sfx_krta00 + player->kartstuff[k_enginesnd]) + (class*numsnds), volume);
}

static void K_UpdateInvincibilitySounds(player_t *player)
{
	INT32 sfxnum = sfx_None;

	if (player->mo->health > 0 && !P_IsLocalPlayer(player))
	{
		if (cv_kartinvinsfx.value)
		{
			if (player->kartstuff[k_growshrinktimer] > 0) // Prioritize Grow
				sfxnum = sfx_alarmg;
			else if (player->kartstuff[k_invincibilitytimer] > 0)
				sfxnum = sfx_alarmi;
		}
		else
		{
			if (player->kartstuff[k_growshrinktimer] > 0)
				sfxnum = sfx_kgrow;
			else if (player->kartstuff[k_invincibilitytimer] > 0)
				sfxnum = sfx_kinvnc;
		}
	}

	if (sfxnum != sfx_None && !S_SoundPlaying(player->mo, sfxnum))
		S_StartSound(player->mo, sfxnum);

#define STOPTHIS(this) \
	if (sfxnum != this && S_SoundPlaying(player->mo, this)) \
		S_StopSoundByID(player->mo, this);
	STOPTHIS(sfx_alarmi);
	STOPTHIS(sfx_alarmg);
	STOPTHIS(sfx_kinvnc);
	STOPTHIS(sfx_kgrow);
#undef STOPTHIS
}

void K_KartPlayerHUDUpdate(player_t *player)
{
	if (player->kartstuff[k_lapanimation])
		player->kartstuff[k_lapanimation]--;

	if (player->kartstuff[k_yougotem])
		player->kartstuff[k_yougotem]--;

	if (G_BattleGametype() && (player->exiting || player->kartstuff[k_comebacktimer]))
	{
		if (player->exiting)
		{
			if (player->exiting < 6*TICRATE)
				player->kartstuff[k_cardanimation] += ((164-player->kartstuff[k_cardanimation])/8)+1;
			else if (player->exiting == 6*TICRATE)
				player->kartstuff[k_cardanimation] = 0;
			else if (player->kartstuff[k_cardanimation] < 2*TICRATE)
				player->kartstuff[k_cardanimation]++;
		}
		else
		{
			if (player->kartstuff[k_comebacktimer] < 6*TICRATE)
				player->kartstuff[k_cardanimation] -= ((164-player->kartstuff[k_cardanimation])/8)+1;
			else if (player->kartstuff[k_comebacktimer] < 9*TICRATE)
				player->kartstuff[k_cardanimation] += ((164-player->kartstuff[k_cardanimation])/8)+1;
		}

		if (player->kartstuff[k_cardanimation] > 164)
			player->kartstuff[k_cardanimation] = 164;
		if (player->kartstuff[k_cardanimation] < 0)
			player->kartstuff[k_cardanimation] = 0;
	}
	else if (G_RaceGametype() && player->exiting)
	{
		if (player->kartstuff[k_cardanimation] < 2*TICRATE)
			player->kartstuff[k_cardanimation]++;
	}
	else
		player->kartstuff[k_cardanimation] = 0;
}

/**	\brief	Decreases various kart timers and powers per frame. Called in P_PlayerThink in p_user.c

	\param	player	player object passed from P_PlayerThink
	\param	cmd		control input from player

	\return	void
*/
void K_KartPlayerThink(player_t *player, ticcmd_t *cmd)
{
	K_UpdateOffroad(player);
	K_UpdateEngineSounds(player, cmd); // Thanks, VAda!

	// update boost angle if not spun out
	if (!player->kartstuff[k_spinouttimer] && !player->kartstuff[k_wipeoutslow])
		player->kartstuff[k_boostangle] = (INT32)player->mo->angle;

	K_GetKartBoostPower(player);

	// Speed lines
	if ((player->kartstuff[k_sneakertimer] || player->kartstuff[k_driftboost] || player->kartstuff[k_startboost]) && player->speed > 0)
	{
		mobj_t *fast = P_SpawnMobj(player->mo->x + (P_RandomRange(-36,36) * player->mo->scale),
			player->mo->y + (P_RandomRange(-36,36) * player->mo->scale),
			player->mo->z + (player->mo->height/2) + (P_RandomRange(-20,20) * player->mo->scale),
			MT_FASTLINE);
		fast->angle = R_PointToAngle2(0, 0, player->mo->momx, player->mo->momy);
		fast->momx = 3*player->mo->momx/4;
		fast->momy = 3*player->mo->momy/4;
		fast->momz = 3*player->mo->momz/4;
		P_SetTarget(&fast->target, player->mo); // easier lua access
		K_MatchGenericExtraFlags(fast, player->mo);
	}

	if (player->playerstate == PST_DEAD || player->kartstuff[k_respawn] > 1) // Ensure these are set correctly here
	{
		player->mo->colorized = false;
		player->mo->color = player->skincolor;
	}
	else if (player->kartstuff[k_eggmanexplode]) // You're gonna diiiiie
	{
		const INT32 flashtime = 4<<(player->kartstuff[k_eggmanexplode]/TICRATE);
		if (player->kartstuff[k_eggmanexplode] == 1 || (player->kartstuff[k_eggmanexplode] % (flashtime/2) != 0))
		{
			player->mo->colorized = false;
			player->mo->color = player->skincolor;
		}
		else if (player->kartstuff[k_eggmanexplode] % flashtime == 0)
		{
			player->mo->colorized = true;
			player->mo->color = SKINCOLOR_BLACK;
		}
		else
		{
			player->mo->colorized = true;
			player->mo->color = SKINCOLOR_CRIMSON;
		}
	}
	else if (player->kartstuff[k_invincibilitytimer]) // setting players to use the star colormap and spawning afterimages
	{
		mobj_t *ghost;
		player->mo->colorized = true;
		ghost = P_SpawnGhostMobj(player->mo);
		ghost->fuse = 4;
		ghost->frame |= FF_FULLBRIGHT;
	}
	else if (player->kartstuff[k_growshrinktimer]) // Ditto, for grow/shrink
	{
		if (player->kartstuff[k_growshrinktimer] % 5 == 0)
		{
			player->mo->colorized = true;
			player->mo->color = (player->kartstuff[k_growshrinktimer] < 0 ? SKINCOLOR_CREAMSICLE : SKINCOLOR_PERIWINKLE);
		}
		else
		{
			player->mo->colorized = false;
			player->mo->color = player->skincolor;
		}
	}
	else
	{
		player->mo->colorized = false;
	}

	if (player->kartstuff[k_dashpadcooldown]) // Twinkle Circuit inspired afterimages
	{
		mobj_t *ghost;
		ghost = P_SpawnGhostMobj(player->mo);
		ghost->fuse = player->kartstuff[k_dashpadcooldown]+1;
		ghost->momx = player->mo->momx / (player->kartstuff[k_dashpadcooldown]+1);
		ghost->momy = player->mo->momy / (player->kartstuff[k_dashpadcooldown]+1);
		ghost->momz = player->mo->momz / (player->kartstuff[k_dashpadcooldown]+1);
		player->kartstuff[k_dashpadcooldown]--;
	}

	// DKR style camera for boosting
	if (player->kartstuff[k_boostcam] != 0 || player->kartstuff[k_destboostcam] != 0)
	{
		if (player->kartstuff[k_boostcam] < player->kartstuff[k_destboostcam]
			&& player->kartstuff[k_destboostcam] != 0)
		{
			player->kartstuff[k_boostcam] += FRACUNIT/(TICRATE/4);
			if (player->kartstuff[k_boostcam] >= player->kartstuff[k_destboostcam])
				player->kartstuff[k_destboostcam] = 0;
		}
		else
		{
			player->kartstuff[k_boostcam] -= FRACUNIT/TICRATE;
			if (player->kartstuff[k_boostcam] < player->kartstuff[k_destboostcam])
				player->kartstuff[k_boostcam] = player->kartstuff[k_destboostcam] = 0;
		}
		//CONS_Printf("cam: %d, dest: %d\n", player->kartstuff[k_boostcam], player->kartstuff[k_destboostcam]);
	}

	player->kartstuff[k_timeovercam] = 0;

	// Make ABSOLUTELY SURE that your flashing tics don't get set WHILE you're still in hit animations.
	if (player->kartstuff[k_spinouttimer] != 0
		|| player->kartstuff[k_wipeoutslow] != 0
		|| player->kartstuff[k_squishedtimer] != 0)
	{
		player->powers[pw_flashing] = K_GetKartFlashing(player);
	}
	else if (player->powers[pw_flashing] >= K_GetKartFlashing(player))
	{
		player->powers[pw_flashing]--;
	}

	if (player->kartstuff[k_spinouttimer])
	{
		if ((P_IsObjectOnGround(player->mo) || player->kartstuff[k_spinouttype] == 1)
			&& (player->kartstuff[k_sneakertimer] == 0))
		{
			player->kartstuff[k_spinouttimer]--;
			if (player->kartstuff[k_wipeoutslow] > 1)
				player->kartstuff[k_wipeoutslow]--;
			if (player->kartstuff[k_spinouttimer] == 0)
				player->kartstuff[k_spinouttype] = 0; // Reset type
		}
	}
	else
	{
		if (player->kartstuff[k_wipeoutslow] == 1)
			player->mo->friction = ORIG_FRICTION;
		player->kartstuff[k_wipeoutslow] = 0;
		if (!comeback)
			player->kartstuff[k_comebacktimer] = comebacktime;
		else if (player->kartstuff[k_comebacktimer])
		{
			player->kartstuff[k_comebacktimer]--;
			if (P_IsLocalPlayer(player) && player->kartstuff[k_bumper] <= 0 && player->kartstuff[k_comebacktimer] <= 0)
				comebackshowninfo = true; // client has already seen the message
		}
	}

	/*if (player->kartstuff[k_thunderanim])
		player->kartstuff[k_thunderanim]--;*/

	if (player->kartstuff[k_sneakertimer])
	{
		player->kartstuff[k_sneakertimer]--;
		if (player->kartstuff[k_wipeoutslow] > 0 && player->kartstuff[k_wipeoutslow] < wipeoutslowtime+1)
			player->kartstuff[k_wipeoutslow] = wipeoutslowtime+1;
	}

	if (player->kartstuff[k_floorboost])
		player->kartstuff[k_floorboost]--;

	if (player->kartstuff[k_driftboost])
		player->kartstuff[k_driftboost]--;

	if (player->kartstuff[k_startboost])
		player->kartstuff[k_startboost]--;

	if (player->kartstuff[k_invincibilitytimer])
		player->kartstuff[k_invincibilitytimer]--;

	if (!player->kartstuff[k_respawn] && player->kartstuff[k_growshrinktimer] != 0)
	{
		if (player->kartstuff[k_growshrinktimer] > 0)
			player->kartstuff[k_growshrinktimer]--;
		if (player->kartstuff[k_growshrinktimer] < 0)
			player->kartstuff[k_growshrinktimer]++;

		// Back to normal
		if (player->kartstuff[k_growshrinktimer] == 0)
			K_RemoveGrowShrink(player);
	}

	if (player->kartstuff[k_stealingtimer] == 0 && player->kartstuff[k_stolentimer] == 0
		&& player->kartstuff[k_rocketsneakertimer])
		player->kartstuff[k_rocketsneakertimer]--;

	if (player->kartstuff[k_hyudorotimer])
		player->kartstuff[k_hyudorotimer]--;

	if (player->kartstuff[k_sadtimer])
		player->kartstuff[k_sadtimer]--;

	if (player->kartstuff[k_stealingtimer])
		player->kartstuff[k_stealingtimer]--;

	if (player->kartstuff[k_stolentimer])
		player->kartstuff[k_stolentimer]--;

	if (player->kartstuff[k_squishedtimer])
		player->kartstuff[k_squishedtimer]--;

	if (player->kartstuff[k_justbumped])
		player->kartstuff[k_justbumped]--;

	// This doesn't go in HUD update because it has potential gameplay ramifications
	if (player->kartstuff[k_itemblink] && player->kartstuff[k_itemblink]-- <= 0)
	{
		player->kartstuff[k_itemblinkmode] = 0;
		player->kartstuff[k_itemblink] = 0;
	}

	K_KartPlayerHUDUpdate(player);

	if (player->kartstuff[k_voices])
		player->kartstuff[k_voices]--;

	if (player->kartstuff[k_tauntvoices])
		player->kartstuff[k_tauntvoices]--;

	if (G_BattleGametype() && player->kartstuff[k_bumper] > 0)
		player->kartstuff[k_wanted]++;

	if (P_IsObjectOnGround(player->mo))
		player->kartstuff[k_waterskip] = 0;

	if (player->kartstuff[k_instashield])
		player->kartstuff[k_instashield]--;

	if (player->kartstuff[k_eggmanexplode])
	{
		if (player->spectator || (G_BattleGametype() && !player->kartstuff[k_bumper]))
			player->kartstuff[k_eggmanexplode] = 0;
		else
		{
			player->kartstuff[k_eggmanexplode]--;
			if (player->kartstuff[k_eggmanexplode] <= 0)
			{
				mobj_t *eggsexplode;
				//player->powers[pw_flashing] = 0;
				eggsexplode = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_SPBEXPLOSION);
				if (player->kartstuff[k_eggmanblame] >= 0
				&& player->kartstuff[k_eggmanblame] < MAXPLAYERS
				&& playeringame[player->kartstuff[k_eggmanblame]]
				&& !players[player->kartstuff[k_eggmanblame]].spectator
				&& players[player->kartstuff[k_eggmanblame]].mo)
					P_SetTarget(&eggsexplode->target, players[player->kartstuff[k_eggmanblame]].mo);
			}
		}
	}

	// ???
	/*
	if (player->kartstuff[k_jmp] > 1 && onground)
	{
		S_StartSound(player->mo, sfx_spring);
		P_DoJump(player, false);
		player->mo->momz *= player->kartstuff[k_jmp];
		player->kartstuff[k_jmp] = 0;
	}
	*/

	if (player->kartstuff[k_comebacktimer])
		player->kartstuff[k_comebackmode] = 0;

	if (P_IsObjectOnGround(player->mo) && player->kartstuff[k_pogospring])
	{
		if (P_MobjFlip(player->mo)*player->mo->momz <= 0)
			player->kartstuff[k_pogospring] = 0;
	}

	if (cmd->buttons & BT_DRIFT)
		player->kartstuff[k_jmp] = 1;
	else
		player->kartstuff[k_jmp] = 0;

	// Respawn Checker
	if (player->kartstuff[k_respawn])
		K_RespawnChecker(player);

	// Roulette Code
	K_KartItemRoulette(player, cmd);

	// Handle invincibility sfx
	K_UpdateInvincibilitySounds(player); // Also thanks, VAda!

	// Plays the music after the starting countdown.
	if (P_IsLocalPlayer(player) && leveltime == (starttime + (TICRATE/2)))
	{
		S_ChangeMusic(mapmusname, mapmusflags, true);
		S_ShowMusicCredit();
	}
}

void K_KartPlayerAfterThink(player_t *player)
{
	if (player->kartstuff[k_curshield]
		|| player->kartstuff[k_invincibilitytimer]
		|| (player->kartstuff[k_growshrinktimer] != 0 && player->kartstuff[k_growshrinktimer] % 5 == 4)) // 4 instead of 0 because this is afterthink!
	{
		player->mo->frame |= FF_FULLBRIGHT;
	}
	else
	{
		if (!(player->mo->state->frame & FF_FULLBRIGHT))
			player->mo->frame &= ~FF_FULLBRIGHT;
	}

	// Move held objects (Bananas, Orbinaut, etc)
	K_MoveHeldObjects(player);

	// Jawz reticule (seeking)
	if (player->kartstuff[k_itemtype] == KITEM_JAWZ && player->kartstuff[k_itemheld])
	{
		INT32 lasttarg = player->kartstuff[k_lastjawztarget];
		player_t *targ;
		mobj_t *ret;

		if (player->kartstuff[k_jawztargetdelay] && playeringame[lasttarg] && !players[lasttarg].spectator)
		{
			targ = &players[lasttarg];
			player->kartstuff[k_jawztargetdelay]--;
		}
		else
			targ = K_FindJawzTarget(player->mo, player);

		if (!targ || !targ->mo || P_MobjWasRemoved(targ->mo))
		{
			player->kartstuff[k_lastjawztarget] = -1;
			player->kartstuff[k_jawztargetdelay] = 0;
			return;
		}

		ret = P_SpawnMobj(targ->mo->x, targ->mo->y, targ->mo->z, MT_PLAYERRETICULE);
		P_SetTarget(&ret->target, targ->mo);
		ret->frame |= ((leveltime % 10) / 2);
		ret->tics = 1;
		ret->color = player->skincolor;

		if (targ-players != lasttarg)
		{
			if (P_IsLocalPlayer(player) || P_IsLocalPlayer(targ))
				S_StartSound(NULL, sfx_s3k89);
			else
				S_StartSound(targ->mo, sfx_s3k89);

			player->kartstuff[k_lastjawztarget] = targ-players;
			player->kartstuff[k_jawztargetdelay] = 5;
		}
	}
	else
	{
		player->kartstuff[k_lastjawztarget] = -1;
		player->kartstuff[k_jawztargetdelay] = 0;
	}
}

// Returns false if this player being placed here causes them to collide with any other player
// Used in g_game.c for match etc. respawning
// This does not check along the z because the z is not correctly set for the spawnee at this point
boolean K_CheckPlayersRespawnColliding(INT32 playernum, fixed_t x, fixed_t y)
{
	INT32 i;
	fixed_t p1radius = players[playernum].mo->radius;
	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (playernum == i || !playeringame[i] || players[i].spectator || !players[i].mo || players[i].mo->health <= 0
			|| players[i].playerstate != PST_LIVE || (players[i].mo->flags & MF_NOCLIP) || (players[i].mo->flags & MF_NOCLIPTHING))
			continue;

		if (abs(x - players[i].mo->x) < (p1radius + players[i].mo->radius)
			&& abs(y - players[i].mo->y) < (p1radius + players[i].mo->radius))
		{
			return false;
		}
	}
	return true;
}

// countersteer is how strong the controls are telling us we are turning
// turndir is the direction the controls are telling us to turn, -1 if turning right and 1 if turning left
static INT16 K_GetKartDriftValue(player_t *player, fixed_t countersteer)
{
	INT16 basedrift, driftangle;
	fixed_t driftweight = player->kartweight*14; // 12

	// If they aren't drifting or on the ground this doesn't apply
	if (player->kartstuff[k_drift] == 0 || !P_IsObjectOnGround(player->mo))
		return 0;

	if (player->kartstuff[k_driftend] != 0)
	{
		return -266*player->kartstuff[k_drift]; // Drift has ended and we are tweaking their angle back a bit
	}

	//basedrift = 90*player->kartstuff[k_drift]; // 450
	//basedrift = 93*player->kartstuff[k_drift] - driftweight*3*player->kartstuff[k_drift]/10; // 447 - 303
	basedrift = 83*player->kartstuff[k_drift] - (driftweight - 14)*player->kartstuff[k_drift]/5; // 415 - 303
	driftangle = abs((252 - driftweight)*player->kartstuff[k_drift]/5);

	return basedrift + FixedMul(driftangle, countersteer);
}

INT16 K_GetKartTurnValue(player_t *player, INT16 turnvalue)
{
	fixed_t p_maxspeed = FixedMul(K_GetKartSpeed(player, false), 3*FRACUNIT);
	fixed_t adjustangle = FixedDiv((p_maxspeed>>16) - (player->speed>>16), (p_maxspeed>>16) + player->kartweight);

	if (player->spectator)
		return turnvalue;

	if (player->kartstuff[k_drift] != 0 && P_IsObjectOnGround(player->mo))
	{
		// If we're drifting we have a completely different turning value
		if (player->kartstuff[k_driftend] == 0)
		{
			// 800 is the max set in g_game.c with angleturn
			fixed_t countersteer = FixedDiv(turnvalue*FRACUNIT, 800*FRACUNIT);
			turnvalue = K_GetKartDriftValue(player, countersteer);
		}
		else
			turnvalue = (INT16)(turnvalue + K_GetKartDriftValue(player, FRACUNIT));

		return turnvalue;
	}

	turnvalue = FixedMul(turnvalue, adjustangle); // Weight has a small effect on turning

	if (player->kartstuff[k_invincibilitytimer] || player->kartstuff[k_sneakertimer] || player->kartstuff[k_growshrinktimer] > 0)
		turnvalue = FixedMul(turnvalue, FixedDiv(5*FRACUNIT, 4*FRACUNIT));

	return turnvalue;
}

INT32 K_GetKartDriftSparkValue(player_t *player)
{
	UINT8 kartspeed = (G_BattleGametype() && player->kartstuff[k_bumper] <= 0)
		? 1
		: player->kartspeed;
	return (26*4 + kartspeed*2 + (9 - player->kartweight))*8;
}

static void K_KartDrift(player_t *player, boolean onground)
{
	fixed_t minspeed = (10 * player->mo->scale);
	INT32 dsone = K_GetKartDriftSparkValue(player);
	INT32 dstwo = dsone*2;
	INT32 dsthree = dstwo*2;

	// Drifting is actually straffing + automatic turning.
	// Holding the Jump button will enable drifting.

	// Drift Release (Moved here so you can't "chain" drifts)
	if ((player->kartstuff[k_drift] != -5 && player->kartstuff[k_drift] != 5)
		// || (player->kartstuff[k_drift] >= 1 && player->kartstuff[k_turndir] != 1) || (player->kartstuff[k_drift] <= -1 && player->kartstuff[k_turndir] != -1))
		&& player->kartstuff[k_driftcharge] < dsone
		&& onground)
	{
		player->kartstuff[k_driftcharge] = 0;
	}
	else if ((player->kartstuff[k_drift] != -5 && player->kartstuff[k_drift] != 5)
		// || (player->kartstuff[k_drift] >= 1 && player->kartstuff[k_turndir] != 1) || (player->kartstuff[k_drift] <= -1 && player->kartstuff[k_turndir] != -1))
		&& (player->kartstuff[k_driftcharge] >= dsone && player->kartstuff[k_driftcharge] < dstwo)
		&& onground)
	{
		if (player->kartstuff[k_driftboost] < 20)
			player->kartstuff[k_driftboost] = 20;
		S_StartSound(player->mo, sfx_s23c);
		//K_SpawnDashDustRelease(player);
		player->kartstuff[k_driftcharge] = 0;
	}
	else if ((player->kartstuff[k_drift] != -5 && player->kartstuff[k_drift] != 5)
		// || (player->kartstuff[k_drift] >= 1 && player->kartstuff[k_turndir] != 1) || (player->kartstuff[k_drift] <= -1 && player->kartstuff[k_turndir] != -1))
		&& player->kartstuff[k_driftcharge] < dsthree
		&& onground)
	{
		if (player->kartstuff[k_driftboost] < 50)
			player->kartstuff[k_driftboost] = 50;
		S_StartSound(player->mo, sfx_s23c);
		//K_SpawnDashDustRelease(player);
		player->kartstuff[k_driftcharge] = 0;
	}
	else if ((player->kartstuff[k_drift] != -5 && player->kartstuff[k_drift] != 5)
		// || (player->kartstuff[k_drift] >= 1 && player->kartstuff[k_turndir] != 1) || (player->kartstuff[k_drift] <= -1 && player->kartstuff[k_turndir] != -1))
		&& player->kartstuff[k_driftcharge] >= dsthree
		&& onground)
	{
		if (player->kartstuff[k_driftboost] < 125)
			player->kartstuff[k_driftboost] = 125;
		S_StartSound(player->mo, sfx_s23c);
		//K_SpawnDashDustRelease(player);
		player->kartstuff[k_driftcharge] = 0;
	}

	// Drifting: left or right?
	if ((player->cmd.driftturn > 0) && player->speed > minspeed && player->kartstuff[k_jmp] == 1
		&& (player->kartstuff[k_drift] == 0 || player->kartstuff[k_driftend] == 1)) // && player->kartstuff[k_drift] != 1)
	{
		// Starting left drift
		player->kartstuff[k_drift] = 1;
		player->kartstuff[k_driftend] = player->kartstuff[k_driftcharge] = 0;
	}
	else if ((player->cmd.driftturn < 0) && player->speed > minspeed && player->kartstuff[k_jmp] == 1
		&& (player->kartstuff[k_drift] == 0 || player->kartstuff[k_driftend] == 1)) // && player->kartstuff[k_drift] != -1)
	{
		// Starting right drift
		player->kartstuff[k_drift] = -1;
		player->kartstuff[k_driftend] = player->kartstuff[k_driftcharge] = 0;
	}
	else if (player->kartstuff[k_jmp] == 0) // || player->kartstuff[k_turndir] == 0)
	{
		// drift is not being performed so if we're just finishing set driftend and decrement counters
		if (player->kartstuff[k_drift] > 0)
		{
			player->kartstuff[k_drift]--;
			player->kartstuff[k_driftend] = 1;
		}
		else if (player->kartstuff[k_drift] < 0)
		{
			player->kartstuff[k_drift]++;
			player->kartstuff[k_driftend] = 1;
		}
		else
			player->kartstuff[k_driftend] = 0;
	}


	// Incease/decrease the drift value to continue drifting in that direction
	if (player->kartstuff[k_spinouttimer] == 0 && player->kartstuff[k_jmp] == 1 && onground && player->kartstuff[k_drift] != 0)
	{
		fixed_t driftadditive = 24;

		if (player->kartstuff[k_drift] >= 1) // Drifting to the left
		{
			player->kartstuff[k_drift]++;
			if (player->kartstuff[k_drift] > 5)
				player->kartstuff[k_drift] = 5;

			if (player->cmd.driftturn > 0) // Inward
				driftadditive += abs(player->cmd.driftturn)/100;
			if (player->cmd.driftturn < 0) // Outward
				driftadditive -= abs(player->cmd.driftturn)/75;
		}
		else if (player->kartstuff[k_drift] <= -1) // Drifting to the right
		{
			player->kartstuff[k_drift]--;
			if (player->kartstuff[k_drift] < -5)
				player->kartstuff[k_drift] = -5;

			if (player->cmd.driftturn < 0) // Inward
				driftadditive += abs(player->cmd.driftturn)/100;
			if (player->cmd.driftturn > 0) // Outward
				driftadditive -= abs(player->cmd.driftturn)/75;
		}

		// Disable drift-sparks until you're going fast enough
		if (player->kartstuff[k_getsparks] == 0 || (player->kartstuff[k_offroad] && !player->kartstuff[k_invincibilitytimer] && !player->kartstuff[k_hyudorotimer] && !player->kartstuff[k_sneakertimer]))
			driftadditive = 0;
		if (player->speed > minspeed*2)
			player->kartstuff[k_getsparks] = 1;

		// This spawns the drift sparks
		if (player->kartstuff[k_driftcharge] + driftadditive >= dsone)
			K_SpawnDriftSparks(player);

		// Sound whenever you get a different tier of sparks
		if (P_IsLocalPlayer(player) // UGHGHGH...
			&& ((player->kartstuff[k_driftcharge] < dsone && player->kartstuff[k_driftcharge]+driftadditive >= dsone)
			|| (player->kartstuff[k_driftcharge] < dstwo && player->kartstuff[k_driftcharge]+driftadditive >= dstwo)
			|| (player->kartstuff[k_driftcharge] < dsthree && player->kartstuff[k_driftcharge]+driftadditive >= dsthree)))
		{
			//S_StartSound(player->mo, sfx_s3ka2);
			S_StartSoundAtVolume(player->mo, sfx_s3ka2, 192); // Ugh...
		}

		player->kartstuff[k_driftcharge] += driftadditive;
		player->kartstuff[k_driftend] = 0;
	}

	// Stop drifting
	if (player->kartstuff[k_spinouttimer] > 0 || player->speed < minspeed)
	{
		player->kartstuff[k_drift] = player->kartstuff[k_driftcharge] = 0;
		player->kartstuff[k_aizdriftstrat] = player->kartstuff[k_brakedrift] = 0;
		player->kartstuff[k_getsparks] = 0;
	}

	if ((!player->kartstuff[k_sneakertimer])
	|| (!player->cmd.driftturn)
	|| (!player->kartstuff[k_aizdriftstrat])
	|| (player->cmd.driftturn > 0) != (player->kartstuff[k_aizdriftstrat] > 0))
	{
		if (!player->kartstuff[k_drift])
			player->kartstuff[k_aizdriftstrat] = 0;
		else
			player->kartstuff[k_aizdriftstrat] = ((player->kartstuff[k_drift] > 0) ? 1 : -1);
	}
	else if (player->kartstuff[k_aizdriftstrat] && !player->kartstuff[k_drift])
		K_SpawnAIZDust(player);

	if (player->kartstuff[k_drift]
		&& ((player->cmd.buttons & BT_BRAKE)
		|| !(player->cmd.buttons & BT_ACCELERATE))
		&& P_IsObjectOnGround(player->mo))
	{
		if (!player->kartstuff[k_brakedrift])
			K_SpawnBrakeDriftSparks(player);
		player->kartstuff[k_brakedrift] = 1;
	}
	else
		player->kartstuff[k_brakedrift] = 0;
}
//
// K_KartUpdatePosition
//
void K_KartUpdatePosition(player_t *player)
{
	fixed_t position = 1;
	fixed_t oldposition = player->kartstuff[k_position];
	fixed_t i, ppcd, pncd, ipcd, incd;
	fixed_t pmo, imo;
	mobj_t *mo;

	if (player->spectator || !player->mo)
		return;

	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (!playeringame[i] || players[i].spectator || !players[i].mo)
			continue;

		if (G_RaceGametype())
		{
			if ((((players[i].starpostnum) + (numstarposts + 1) * players[i].laps) >
				((player->starpostnum) + (numstarposts + 1) * player->laps)))
				position++;
			else if (((players[i].starpostnum) + (numstarposts+1)*players[i].laps) ==
				((player->starpostnum) + (numstarposts+1)*player->laps))
			{
				ppcd = pncd = ipcd = incd = 0;

				player->kartstuff[k_prevcheck] = players[i].kartstuff[k_prevcheck] = 0;
				player->kartstuff[k_nextcheck] = players[i].kartstuff[k_nextcheck] = 0;

				// This checks every thing on the map, and looks for MT_BOSS3WAYPOINT (the thing we're using for checkpoint wp's, for now)
				for (mo = waypointcap; mo != NULL; mo = mo->tracer)
				{
					pmo = P_AproxDistance(P_AproxDistance(	mo->x - player->mo->x,
															mo->y - player->mo->y),
															mo->z - player->mo->z) / FRACUNIT;
					imo = P_AproxDistance(P_AproxDistance(	mo->x - players[i].mo->x,
															mo->y - players[i].mo->y),
															mo->z - players[i].mo->z) / FRACUNIT;

					if (mo->health == player->starpostnum && (!mo->movecount || mo->movecount == player->laps+1))
					{
						player->kartstuff[k_prevcheck] += pmo;
						ppcd++;
					}
					if (mo->health == (player->starpostnum + 1) && (!mo->movecount || mo->movecount == player->laps+1))
					{
						player->kartstuff[k_nextcheck] += pmo;
						pncd++;
					}
					if (mo->health == players[i].starpostnum && (!mo->movecount || mo->movecount == players[i].laps+1))
					{
						players[i].kartstuff[k_prevcheck] += imo;
						ipcd++;
					}
					if (mo->health == (players[i].starpostnum + 1) && (!mo->movecount || mo->movecount == players[i].laps+1))
					{
						players[i].kartstuff[k_nextcheck] += imo;
						incd++;
					}
				}

				if (ppcd > 1) player->kartstuff[k_prevcheck] /= ppcd;
				if (pncd > 1) player->kartstuff[k_nextcheck] /= pncd;
				if (ipcd > 1) players[i].kartstuff[k_prevcheck] /= ipcd;
				if (incd > 1) players[i].kartstuff[k_nextcheck] /= incd;

				if ((players[i].kartstuff[k_nextcheck] > 0 || player->kartstuff[k_nextcheck] > 0) && !player->exiting)
				{
					if ((players[i].kartstuff[k_nextcheck] - players[i].kartstuff[k_prevcheck]) <
						(player->kartstuff[k_nextcheck] - player->kartstuff[k_prevcheck]))
						position++;
				}
				else if (!player->exiting)
				{
					if (players[i].kartstuff[k_prevcheck] > player->kartstuff[k_prevcheck])
						position++;
				}
				else
				{
					if (players[i].starposttime < player->starposttime)
						position++;
				}
			}
		}
		else if (G_BattleGametype())
		{
			if (player->exiting) // End of match standings
			{
				if (players[i].marescore > player->marescore) // Only score matters
					position++;
			}
			else
			{
				if (players[i].kartstuff[k_bumper] == player->kartstuff[k_bumper] && players[i].marescore > player->marescore)
					position++;
				else if (players[i].kartstuff[k_bumper] > player->kartstuff[k_bumper])
					position++;
			}
		}
	}

	if (leveltime < starttime || oldposition == 0)
		oldposition = position;

	if (oldposition != position) // Changed places?
		player->kartstuff[k_positiondelay] = 10; // Position number growth

	player->kartstuff[k_position] = position;
}

//
// K_StripItems
//
void K_StripItems(player_t *player)
{
	K_DropRocketSneaker(player);
	K_DropKitchenSink(player);
	player->kartstuff[k_itemtype] = KITEM_NONE;
	player->kartstuff[k_itemamount] = 0;
	player->kartstuff[k_itemheld] = 0;

	if (!player->kartstuff[k_itemroulette] || player->kartstuff[k_roulettetype] != 2)
	{
		player->kartstuff[k_itemroulette] = 0;
		player->kartstuff[k_roulettetype] = 0;
	}
	player->kartstuff[k_eggmanheld] = 0;

	player->kartstuff[k_hyudorotimer] = 0;
	player->kartstuff[k_stealingtimer] = 0;
	player->kartstuff[k_stolentimer] = 0;

	player->kartstuff[k_curshield] = 0;
	//player->kartstuff[k_thunderanim] = 0;
	player->kartstuff[k_bananadrag] = 0;

	player->kartstuff[k_sadtimer] = 0;

	K_UpdateHnextList(player, true);
}

void K_StripOther(player_t *player)
{
	player->kartstuff[k_itemroulette] = 0;
	player->kartstuff[k_roulettetype] = 0;

	player->kartstuff[k_invincibilitytimer] = 0;
	K_RemoveGrowShrink(player);

	if (player->kartstuff[k_eggmanexplode])
	{
		player->kartstuff[k_eggmanexplode] = 0;
		player->kartstuff[k_eggmanblame] = -1;
	}
}

//
// K_MoveKartPlayer
//
void K_MoveKartPlayer(player_t *player, boolean onground)
{
	ticcmd_t *cmd = &player->cmd;
	boolean ATTACK_IS_DOWN = ((cmd->buttons & BT_ATTACK) && !(player->pflags & PF_ATTACKDOWN));
	boolean HOLDING_ITEM = (player->kartstuff[k_itemheld] || player->kartstuff[k_eggmanheld]);
	boolean NO_HYUDORO = (player->kartstuff[k_stolentimer] == 0 && player->kartstuff[k_stealingtimer] == 0);

	K_KartUpdatePosition(player);

	if (!player->exiting)
	{
		if (player->kartstuff[k_oldposition] < player->kartstuff[k_position]) // But first, if you lost a place,
		{
			player->kartstuff[k_oldposition] = player->kartstuff[k_position]; // then the other player taunts.
			K_RegularVoiceTimers(player); // and you can't for a bit
		}
		else if (player->kartstuff[k_oldposition] > player->kartstuff[k_position]) // Otherwise,
		{
			K_PlayOvertakeSound(player->mo); // Say "YOU'RE TOO SLOW!"
			player->kartstuff[k_oldposition] = player->kartstuff[k_position]; // Restore the old position,
		}
	}

	if (player->kartstuff[k_positiondelay])
		player->kartstuff[k_positiondelay]--;

	if ((player->pflags & PF_ATTACKDOWN) && !(cmd->buttons & BT_ATTACK))
		player->pflags &= ~PF_ATTACKDOWN;
	else if (cmd->buttons & BT_ATTACK)
		player->pflags |= PF_ATTACKDOWN;

	if (player && player->mo && player->mo->health > 0 && !player->spectator && !(player->exiting || mapreset)
		&& player->kartstuff[k_spinouttimer] == 0 && player->kartstuff[k_squishedtimer] == 0 && player->kartstuff[k_respawn] == 0)
	{
		// First, the really specific, finicky items that function without the item being directly in your item slot.
		// Karma item dropping
		if (ATTACK_IS_DOWN && player->kartstuff[k_comebackmode] && !player->kartstuff[k_comebacktimer])
		{
			mobj_t *newitem;

			if (player->kartstuff[k_comebackmode] == 1)
			{
				newitem = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_RANDOMITEM);
				newitem->threshold = 69; // selected "randomly".
			}
			else
			{
				newitem = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_EGGMANITEM);
				if (player->kartstuff[k_eggmanblame] >= 0
				&& player->kartstuff[k_eggmanblame] < MAXPLAYERS
				&& playeringame[player->kartstuff[k_eggmanblame]]
				&& !players[player->kartstuff[k_eggmanblame]].spectator
				&& players[player->kartstuff[k_eggmanblame]].mo)
					P_SetTarget(&newitem->target, players[player->kartstuff[k_eggmanblame]].mo);
				player->kartstuff[k_eggmanblame] = -1;
			}

			newitem->flags2 = (player->mo->flags2 & MF2_OBJECTFLIP);
			newitem->fuse = 15*TICRATE; // selected randomly.

			player->kartstuff[k_comebackmode] = 0;
			player->kartstuff[k_comebacktimer] = comebacktime;
			S_StartSound(player->mo, sfx_s254);
		}
		// Eggman Monitor exploding
		else if (player->kartstuff[k_eggmanexplode])
		{
			if (ATTACK_IS_DOWN && player->kartstuff[k_eggmanexplode] <= 3*TICRATE && player->kartstuff[k_eggmanexplode] > 1)
				player->kartstuff[k_eggmanexplode] = 1;
		}
		// Eggman Monitor throwing
		else if (ATTACK_IS_DOWN && player->kartstuff[k_eggmanheld])
		{
			K_ThrowKartItem(player, false, MT_EGGMANITEM, -1, 0);
			K_PlayAttackTaunt(player->mo);
			player->kartstuff[k_eggmanheld] = 0;
			K_UpdateHnextList(player, true);
		}
		// Rocket Sneaker
		else if (ATTACK_IS_DOWN && !HOLDING_ITEM && onground && NO_HYUDORO
			&& player->kartstuff[k_rocketsneakertimer] > 1)
		{
			K_DoSneaker(player, 2);
			K_PlayBoostTaunt(player->mo);
			player->kartstuff[k_rocketsneakertimer] -= 2*TICRATE;
			if (player->kartstuff[k_rocketsneakertimer] < 1)
				player->kartstuff[k_rocketsneakertimer] = 1;
		}
		// Grow Canceling
		else if (player->kartstuff[k_growshrinktimer] > 0)
		{
			if (player->kartstuff[k_growcancel] >= 0)
			{
				if (cmd->buttons & BT_ATTACK)
				{
					player->kartstuff[k_growcancel]++;
					if (player->kartstuff[k_growcancel] > 26)
						K_RemoveGrowShrink(player);
				}
				else
					player->kartstuff[k_growcancel] = 0;
			}
			else
			{
				if ((cmd->buttons & BT_ATTACK) || (player->pflags & PF_ATTACKDOWN))
					player->kartstuff[k_growcancel] = -1;
				else
					player->kartstuff[k_growcancel] = 0;
			}
		}
		else if (player->kartstuff[k_itemamount] <= 0)
		{
			player->kartstuff[k_itemamount] = player->kartstuff[k_itemheld] = 0;
		}
		else
		{
			switch (player->kartstuff[k_itemtype])
			{
				case KITEM_SNEAKER:
					if (ATTACK_IS_DOWN && !HOLDING_ITEM && onground && NO_HYUDORO)
					{
						K_DoSneaker(player, 1);
						K_PlayBoostTaunt(player->mo);
						player->kartstuff[k_itemamount]--;
					}
					break;
				case KITEM_ROCKETSNEAKER:
					if (ATTACK_IS_DOWN && !HOLDING_ITEM && onground && NO_HYUDORO
						&& player->kartstuff[k_rocketsneakertimer] == 0)
					{
						INT32 moloop;
						mobj_t *mo = NULL;
						mobj_t *prev = player->mo;

						K_PlayBoostTaunt(player->mo);
						//player->kartstuff[k_itemheld] = 1;
						S_StartSound(player->mo, sfx_s3k3a);

						//K_DoSneaker(player, 2);

						player->kartstuff[k_rocketsneakertimer] = (itemtime*3);
						player->kartstuff[k_itemamount]--;
						K_UpdateHnextList(player, true);

						for (moloop = 0; moloop < 2; moloop++)
						{
							mo = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_ROCKETSNEAKER);
							K_MatchGenericExtraFlags(mo, player->mo);
							mo->flags |= MF_NOCLIPTHING;
							mo->angle = player->mo->angle;
							mo->threshold = 10;
							mo->movecount = moloop%2;
							mo->movedir = mo->lastlook = moloop+1;
							P_SetTarget(&mo->target, player->mo);
							P_SetTarget(&mo->hprev, prev);
							P_SetTarget(&prev->hnext, mo);
							prev = mo;
						}
					}
					break;
				case KITEM_INVINCIBILITY:
					if (ATTACK_IS_DOWN && !HOLDING_ITEM && NO_HYUDORO) // Doesn't hold your item slot hostage normally, so you're free to waste it if you have multiple
					{
						if (!player->kartstuff[k_invincibilitytimer])
						{
							mobj_t *overlay = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_INVULNFLASH);
							P_SetTarget(&overlay->target, player->mo);
							overlay->destscale = player->mo->scale;
							P_SetScale(overlay, player->mo->scale);
						}
						player->kartstuff[k_invincibilitytimer] = itemtime+(2*TICRATE); // 10 seconds
						P_RestoreMusic(player);
						if (!P_IsLocalPlayer(player))
							S_StartSound(player->mo, (cv_kartinvinsfx.value ? sfx_alarmi : sfx_kinvnc));
						K_PlayPowerGloatSound(player->mo);
						player->kartstuff[k_itemamount]--;
					}
					break;
				case KITEM_BANANA:
					if (ATTACK_IS_DOWN && !HOLDING_ITEM && NO_HYUDORO)
					{
						INT32 moloop;
						mobj_t *mo;
						mobj_t *prev = player->mo;

						//K_PlayAttackTaunt(player->mo);
						player->kartstuff[k_itemheld] = 1;
						S_StartSound(player->mo, sfx_s254);

						for (moloop = 0; moloop < player->kartstuff[k_itemamount]; moloop++)
						{
							mo = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_BANANA_SHIELD);
							if (!mo)
							{
								player->kartstuff[k_itemamount] = moloop;
								break;
							}
							mo->flags |= MF_NOCLIPTHING;
							mo->threshold = 10;
							mo->movecount = player->kartstuff[k_itemamount];
							mo->movedir = moloop+1;
							P_SetTarget(&mo->target, player->mo);
							P_SetTarget(&mo->hprev, prev);
							P_SetTarget(&prev->hnext, mo);
							prev = mo;
						}
					}
					else if (ATTACK_IS_DOWN && player->kartstuff[k_itemheld]) // Banana x3 thrown
					{
						K_ThrowKartItem(player, false, MT_BANANA, -1, 0);
						K_PlayAttackTaunt(player->mo);
						player->kartstuff[k_itemamount]--;
						K_UpdateHnextList(player, false);
					}
					break;
				case KITEM_EGGMAN:
					if (ATTACK_IS_DOWN && !HOLDING_ITEM && NO_HYUDORO)
					{
						mobj_t *mo;
						player->kartstuff[k_itemamount]--;
						player->kartstuff[k_eggmanheld] = 1;
						S_StartSound(player->mo, sfx_s254);
						mo = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_EGGMANITEM_SHIELD);
						if (mo)
						{
							mo->flags |= MF_NOCLIPTHING;
							mo->threshold = 10;
							mo->movecount = 1;
							mo->movedir = 1;
							P_SetTarget(&mo->target, player->mo);
							P_SetTarget(&player->mo->hnext, mo);
						}
					}
					break;
				case KITEM_ORBINAUT:
					if (ATTACK_IS_DOWN && !HOLDING_ITEM && NO_HYUDORO)
					{
						angle_t newangle;
						INT32 moloop;
						mobj_t *mo = NULL;
						mobj_t *prev = player->mo;

						//K_PlayAttackTaunt(player->mo);
						player->kartstuff[k_itemheld] = 1;
						S_StartSound(player->mo, sfx_s3k3a);

						for (moloop = 0; moloop < player->kartstuff[k_itemamount]; moloop++)
						{
							newangle = (player->mo->angle + ANGLE_157h) + FixedAngle(((360 / player->kartstuff[k_itemamount]) * moloop) << FRACBITS) + ANGLE_90;
							mo = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_ORBINAUT_SHIELD);
							if (!mo)
							{
								player->kartstuff[k_itemamount] = moloop;
								break;
							}
							mo->flags |= MF_NOCLIPTHING;
							mo->angle = newangle;
							mo->threshold = 10;
							mo->movecount = player->kartstuff[k_itemamount];
							mo->movedir = mo->lastlook = moloop+1;
							mo->color = player->skincolor;
							P_SetTarget(&mo->target, player->mo);
							P_SetTarget(&mo->hprev, prev);
							P_SetTarget(&prev->hnext, mo);
							prev = mo;
						}
					}
					else if (ATTACK_IS_DOWN && player->kartstuff[k_itemheld]) // Orbinaut x3 thrown
					{
						K_ThrowKartItem(player, true, MT_ORBINAUT, 1, 0);
						K_PlayAttackTaunt(player->mo);
						player->kartstuff[k_itemamount]--;
						K_UpdateHnextList(player, false);
					}
					break;
				case KITEM_JAWZ:
					if (ATTACK_IS_DOWN && !HOLDING_ITEM && NO_HYUDORO)
					{
						angle_t newangle;
						INT32 moloop;
						mobj_t *mo = NULL;
						mobj_t *prev = player->mo;

						//K_PlayAttackTaunt(player->mo);
						player->kartstuff[k_itemheld] = 1;
						S_StartSound(player->mo, sfx_s3k3a);

						for (moloop = 0; moloop < player->kartstuff[k_itemamount]; moloop++)
						{
							newangle = (player->mo->angle + ANGLE_157h) + FixedAngle(((360 / player->kartstuff[k_itemamount]) * moloop) << FRACBITS) + ANGLE_90;
							mo = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_JAWZ_SHIELD);
							if (!mo)
							{
								player->kartstuff[k_itemamount] = moloop;
								break;
							}
							mo->flags |= MF_NOCLIPTHING;
							mo->angle = newangle;
							mo->threshold = 10;
							mo->movecount = player->kartstuff[k_itemamount];
							mo->movedir = mo->lastlook = moloop+1;
							P_SetTarget(&mo->target, player->mo);
							P_SetTarget(&mo->hprev, prev);
							P_SetTarget(&prev->hnext, mo);
							prev = mo;
						}
					}
					else if (ATTACK_IS_DOWN && HOLDING_ITEM && player->kartstuff[k_itemheld]) // Jawz thrown
					{
						if (player->kartstuff[k_throwdir] == 1 || player->kartstuff[k_throwdir] == 0)
							K_ThrowKartItem(player, true, MT_JAWZ, 1, 0);
						else if (player->kartstuff[k_throwdir] == -1) // Throwing backward gives you a dud that doesn't home in
							K_ThrowKartItem(player, true, MT_JAWZ_DUD, -1, 0);
						K_PlayAttackTaunt(player->mo);
						player->kartstuff[k_itemamount]--;
						K_UpdateHnextList(player, false);
					}
					break;
				case KITEM_MINE:
					if (ATTACK_IS_DOWN && !HOLDING_ITEM && NO_HYUDORO)
					{
						mobj_t *mo;
						player->kartstuff[k_itemheld] = 1;
						S_StartSound(player->mo, sfx_s254);
						mo = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_SSMINE_SHIELD);
						if (mo)
						{
							mo->flags |= MF_NOCLIPTHING;
							mo->threshold = 10;
							mo->movecount = 1;
							mo->movedir = 1;
							P_SetTarget(&mo->target, player->mo);
							P_SetTarget(&player->mo->hnext, mo);
						}
					}
					else if (ATTACK_IS_DOWN && player->kartstuff[k_itemheld])
					{
						K_ThrowKartItem(player, false, MT_SSMINE, 1, 1);
						K_PlayAttackTaunt(player->mo);
						player->kartstuff[k_itemamount]--;
						player->kartstuff[k_itemheld] = 0;
						K_UpdateHnextList(player, true);
					}
					break;
				case KITEM_BALLHOG:
					if (ATTACK_IS_DOWN && !HOLDING_ITEM && NO_HYUDORO)
					{
						player->kartstuff[k_itemamount]--;
						K_ThrowKartItem(player, true, MT_BALLHOG, 1, 0);
						K_PlayAttackTaunt(player->mo);
					}
					break;
				case KITEM_SPB:
					if (ATTACK_IS_DOWN && !HOLDING_ITEM && NO_HYUDORO)
					{
						player->kartstuff[k_itemamount]--;
						K_ThrowKartItem(player, true, MT_SPB, 1, 0);
						K_PlayAttackTaunt(player->mo);
					}
					break;
				case KITEM_GROW:
					if (ATTACK_IS_DOWN && !HOLDING_ITEM && NO_HYUDORO
						&& player->kartstuff[k_growshrinktimer] <= 0) // Grow holds the item box hostage
					{
						if (player->kartstuff[k_growshrinktimer] < 0) // If you're shrunk, then "grow" will just make you normal again.
							K_RemoveGrowShrink(player);
						else
						{
							K_PlayPowerGloatSound(player->mo);
							player->mo->scalespeed = mapobjectscale/TICRATE;
							player->mo->destscale = (3*mapobjectscale)/2;
							if (cv_kartdebugshrink.value && !modeattacking && !player->bot)
								player->mo->destscale = (6*player->mo->destscale)/8;
							player->kartstuff[k_growshrinktimer] = itemtime+(4*TICRATE); // 12 seconds
							P_RestoreMusic(player);
							if (!P_IsLocalPlayer(player))
								S_StartSound(player->mo, (cv_kartinvinsfx.value ? sfx_alarmg : sfx_kgrow));
							S_StartSound(player->mo, sfx_kc5a);
						}
						player->kartstuff[k_itemamount]--;
					}
					break;
				case KITEM_SHRINK:
					if (ATTACK_IS_DOWN && !HOLDING_ITEM && NO_HYUDORO)
					{
						K_DoShrink(player);
						player->kartstuff[k_itemamount]--;
						K_PlayPowerGloatSound(player->mo);
					}
					break;
				case KITEM_THUNDERSHIELD:
					if (player->kartstuff[k_curshield] != 1)
					{
						mobj_t *shield = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_THUNDERSHIELD);
						P_SetScale(shield, (shield->destscale = (5*shield->destscale)>>2));
						P_SetTarget(&shield->target, player->mo);
						S_StartSound(shield, sfx_s3k41);
						player->kartstuff[k_curshield] = 1;
					}
					if (ATTACK_IS_DOWN && !HOLDING_ITEM && NO_HYUDORO)
					{
						K_DoThunderShield(player);
						player->kartstuff[k_itemamount]--;
						K_PlayAttackTaunt(player->mo);
					}
					break;
				case KITEM_HYUDORO:
					if (ATTACK_IS_DOWN && !HOLDING_ITEM && NO_HYUDORO)
					{
						player->kartstuff[k_itemamount]--;
						K_DoHyudoroSteal(player); // yes. yes they do.
					}
					break;
				case KITEM_POGOSPRING:
					if (ATTACK_IS_DOWN && !HOLDING_ITEM && onground && NO_HYUDORO
						&& !player->kartstuff[k_pogospring])
					{
						K_PlayBoostTaunt(player->mo);
						K_DoPogoSpring(player->mo, 32<<FRACBITS, 2);
						player->kartstuff[k_pogospring] = 1;
						player->kartstuff[k_itemamount]--;
					}
					break;
				case KITEM_KITCHENSINK:
					if (ATTACK_IS_DOWN && !HOLDING_ITEM && NO_HYUDORO)
					{
						mobj_t *mo;
						player->kartstuff[k_itemheld] = 1;
						S_StartSound(player->mo, sfx_s254);
						mo = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_SINK_SHIELD);
						if (mo)
						{
							mo->flags |= MF_NOCLIPTHING;
							mo->threshold = 10;
							mo->movecount = 1;
							mo->movedir = 1;
							P_SetTarget(&mo->target, player->mo);
							P_SetTarget(&player->mo->hnext, mo);
						}
					}
					else if (ATTACK_IS_DOWN && HOLDING_ITEM && player->kartstuff[k_itemheld]) // Sink thrown
					{
						K_ThrowKartItem(player, false, MT_SINK, 1, 2);
						K_PlayAttackTaunt(player->mo);
						player->kartstuff[k_itemamount]--;
						player->kartstuff[k_itemheld] = 0;
						K_UpdateHnextList(player, true);
					}
					break;
				case KITEM_SAD:
					if (ATTACK_IS_DOWN && !HOLDING_ITEM && NO_HYUDORO
						&& !player->kartstuff[k_sadtimer])
					{
						player->kartstuff[k_sadtimer] = stealtime;
						player->kartstuff[k_itemamount]--;
					}
					break;
				default:
					break;
			}
		}

		// No more!
		if (!player->kartstuff[k_itemamount])
		{
			player->kartstuff[k_itemheld] = 0;
			player->kartstuff[k_itemtype] = KITEM_NONE;
		}

		if (player->kartstuff[k_itemtype] != KITEM_THUNDERSHIELD)
			player->kartstuff[k_curshield] = 0;

		if (player->kartstuff[k_growshrinktimer] <= 0)
			player->kartstuff[k_growcancel] = -1;

		if (player->kartstuff[k_itemtype] == KITEM_SPB
			|| player->kartstuff[k_itemtype] == KITEM_SHRINK
			|| player->kartstuff[k_growshrinktimer] < 0)
			indirectitemcooldown = 20*TICRATE;

		if (player->kartstuff[k_hyudorotimer] > 0)
		{
			if (splitscreen)
			{
				if (leveltime & 1)
					player->mo->flags2 |= MF2_DONTDRAW;
				else
					player->mo->flags2 &= ~MF2_DONTDRAW;

				if (player->kartstuff[k_hyudorotimer] >= (1*TICRATE/2) && player->kartstuff[k_hyudorotimer] <= hyudorotime-(1*TICRATE/2))
				{
					if (player == &players[displayplayers[1]])
						player->mo->eflags |= MFE_DRAWONLYFORP2;
					else if (player == &players[displayplayers[2]] && splitscreen > 1)
						player->mo->eflags |= MFE_DRAWONLYFORP3;
					else if (player == &players[displayplayers[3]] && splitscreen > 2)
						player->mo->eflags |= MFE_DRAWONLYFORP4;
					else if (player == &players[displayplayers[0]])
						player->mo->eflags |= MFE_DRAWONLYFORP1;
					else
						player->mo->flags2 |= MF2_DONTDRAW;
				}
				else
					player->mo->eflags &= ~(MFE_DRAWONLYFORP1|MFE_DRAWONLYFORP2|MFE_DRAWONLYFORP3|MFE_DRAWONLYFORP4);
			}
			else
			{
				if (P_IsDisplayPlayer(player)
					|| (!P_IsDisplayPlayer(player) && (player->kartstuff[k_hyudorotimer] < (1*TICRATE/2) || player->kartstuff[k_hyudorotimer] > hyudorotime-(1*TICRATE/2))))
				{
					if (leveltime & 1)
						player->mo->flags2 |= MF2_DONTDRAW;
					else
						player->mo->flags2 &= ~MF2_DONTDRAW;
				}
				else
					player->mo->flags2 |= MF2_DONTDRAW;
			}

			player->powers[pw_flashing] = player->kartstuff[k_hyudorotimer]; // We'll do this for now, let's people know about the invisible people through subtle hints
		}
		else if (player->kartstuff[k_hyudorotimer] == 0)
		{
			player->mo->flags2 &= ~MF2_DONTDRAW;
			player->mo->eflags &= ~(MFE_DRAWONLYFORP1|MFE_DRAWONLYFORP2|MFE_DRAWONLYFORP3|MFE_DRAWONLYFORP4);
		}

		if (G_BattleGametype() && player->kartstuff[k_bumper] <= 0) // dead in match? you da bomb
		{
			K_DropItems(player); //K_StripItems(player);
			K_StripOther(player);
			player->mo->flags2 |= MF2_SHADOW;
			player->powers[pw_flashing] = player->kartstuff[k_comebacktimer];
		}
		else if (G_RaceGametype() || player->kartstuff[k_bumper] > 0)
		{
			player->mo->flags2 &= ~MF2_SHADOW;
		}
	}

	if (onground)
	{
		// Friction
		if (!player->kartstuff[k_offroad])
		{
			if (player->speed > 0 && cmd->forwardmove == 0 && player->mo->friction == 59392)
				player->mo->friction += 4608;
		}

		if (player->speed > 0 && cmd->forwardmove < 0)	// change friction while braking no matter what, otherwise it's not any more effective than just letting go off accel
			player->mo->friction -= 2048;

		// Karma ice physics
		if (G_BattleGametype() && player->kartstuff[k_bumper] <= 0)
		{
			player->mo->friction += 1228;

			if (player->mo->friction > FRACUNIT)
				player->mo->friction = FRACUNIT;
			if (player->mo->friction < 0)
				player->mo->friction = 0;

			player->mo->movefactor = FixedDiv(ORIG_FRICTION, player->mo->friction);

			if (player->mo->movefactor < FRACUNIT)
				player->mo->movefactor = 19*player->mo->movefactor - 18*FRACUNIT;
			else
				player->mo->movefactor = FRACUNIT; //player->mo->movefactor = ((player->mo->friction - 0xDB34)*(0xA))/0x80;

			if (player->mo->movefactor < 32)
				player->mo->movefactor = 32;
		}

		// Wipeout slowdown
		if (player->kartstuff[k_spinouttimer] && player->kartstuff[k_wipeoutslow])
		{
			if (player->kartstuff[k_offroad])
				player->mo->friction -= 4912;
			if (player->kartstuff[k_wipeoutslow] == 1)
				player->mo->friction -= 9824;
		}
	}

	K_KartDrift(player, onground);

	// Quick Turning
	// You can't turn your kart when you're not moving.
	// So now it's time to burn some rubber!
	if (player->speed < 2 && leveltime > starttime && cmd->buttons & BT_ACCELERATE && cmd->buttons & BT_BRAKE && cmd->driftturn != 0)
	{
		if (leveltime % 8 == 0)
			S_StartSound(player->mo, sfx_s224);
	}

	// Squishing
	// If a Grow player or a sector crushes you, get flattened instead of being killed.

	if (player->kartstuff[k_squishedtimer] <= 0)
	{
		player->mo->flags &= ~MF_NOCLIP;
	}
	else
	{
		player->mo->flags |= MF_NOCLIP;
		player->mo->momx = 0;
		player->mo->momy = 0;
	}

	// Play the starting countdown sounds
	if (player == &players[displayplayers[0]]) // Don't play louder in splitscreen
	{
		if ((leveltime == starttime-(3*TICRATE)) || (leveltime == starttime-(2*TICRATE)) || (leveltime == starttime-TICRATE))
			S_StartSound(NULL, sfx_s3ka7);
		if (leveltime == starttime)
		{
			S_StartSound(NULL, sfx_s3kad);
			S_StopMusic(); // The GO! sound stops the level start ambience
		}
	}

	// Start charging once you're given the opportunity.
	if (leveltime >= starttime-(2*TICRATE) && leveltime <= starttime)
	{
		if (cmd->buttons & BT_ACCELERATE)
		{
			if (player->kartstuff[k_boostcharge] == 0)
				player->kartstuff[k_boostcharge] = cmd->latency;

			player->kartstuff[k_boostcharge]++;
		}
		else
			player->kartstuff[k_boostcharge] = 0;
	}

	// Increase your size while charging your engine.
	if (leveltime < starttime+10)
	{
		player->mo->scalespeed = mapobjectscale/12;
		player->mo->destscale = mapobjectscale + (player->kartstuff[k_boostcharge]*131);
		if (cv_kartdebugshrink.value && !modeattacking && !player->bot)
			player->mo->destscale = (6*player->mo->destscale)/8;
	}

	// Determine the outcome of your charge.
	if (leveltime > starttime && player->kartstuff[k_boostcharge])
	{
		// Not even trying?
		if (player->kartstuff[k_boostcharge] < 35)
		{
			if (player->kartstuff[k_boostcharge] > 17)
				S_StartSound(player->mo, sfx_cdfm00); // chosen instead of a conventional skid because it's more engine-like
		}
		// Get an instant boost!
		else if (player->kartstuff[k_boostcharge] <= 50)
		{
			player->kartstuff[k_startboost] = (50-player->kartstuff[k_boostcharge])+20;

			if (player->kartstuff[k_boostcharge] <= 36)
			{
				player->kartstuff[k_startboost] = 0;
				K_DoSneaker(player, 0);
				player->kartstuff[k_sneakertimer] = 70; // PERFECT BOOST!!

				if (!player->kartstuff[k_floorboost] || player->kartstuff[k_floorboost] == 3) // Let everyone hear this one
					S_StartSound(player->mo, sfx_s25f);
			}
			else
			{
				K_SpawnDashDustRelease(player); // already handled for perfect boosts by K_DoSneaker
				if ((!player->kartstuff[k_floorboost] || player->kartstuff[k_floorboost] == 3) && P_IsLocalPlayer(player))
				{
					if (player->kartstuff[k_boostcharge] <= 40)
						S_StartSound(player->mo, sfx_cdfm01); // You were almost there!
					else
						S_StartSound(player->mo, sfx_s23c); // Nope, better luck next time.
				}
			}
		}
		// You overcharged your engine? Those things are expensive!!!
		else if (player->kartstuff[k_boostcharge] > 50)
		{
			player->powers[pw_nocontrol] = 40;
			//S_StartSound(player->mo, sfx_kc34);
			S_StartSound(player->mo, sfx_s3k83);
			player->pflags |= PF_SKIDDOWN; // cheeky pflag reuse
		}

		player->kartstuff[k_boostcharge] = 0;
	}
}

void K_CalculateBattleWanted(void)
{
	UINT8 numingame = 0, numplaying = 0, numwanted = 0;
	SINT8 bestbumperplayer = -1, bestbumper = -1;
	SINT8 camppos[MAXPLAYERS]; // who is the biggest camper
	UINT8 ties = 0, nextcamppos = 0;
	boolean setbumper = false;
	UINT8 i, j;

	if (!G_BattleGametype())
	{
		for (i = 0; i < 4; i++)
			battlewanted[i] = -1;
		return;
	}

	wantedcalcdelay = wantedfrequency;

	for (i = 0; i < MAXPLAYERS; i++)
		camppos[i] = -1; // initialize

	for (i = 0; i < MAXPLAYERS; i++)
	{
		UINT8 position = 1;

		if (!playeringame[i] || players[i].spectator) // Not playing
			continue;

		if (players[i].exiting) // We're done, don't calculate.
			return;

		numplaying++;

		if (players[i].kartstuff[k_bumper] <= 0) // Not alive, so don't do anything else
			continue;

		numingame++;

		if (bestbumper == -1 || players[i].kartstuff[k_bumper] > bestbumper)
		{
			bestbumper = players[i].kartstuff[k_bumper];
			bestbumperplayer = i;
		}
		else if (players[i].kartstuff[k_bumper] == bestbumper)
			bestbumperplayer = -1; // Tie, no one has best bumper.

		for (j = 0; j < MAXPLAYERS; j++)
		{
			if (!playeringame[j] || players[j].spectator)
				continue;
			if (players[j].kartstuff[k_bumper] <= 0)
				continue;
			if (j == i)
				continue;
			if (players[j].kartstuff[k_wanted] == players[i].kartstuff[k_wanted] && players[j].marescore > players[i].marescore)
				position++;
			else if (players[j].kartstuff[k_wanted] > players[i].kartstuff[k_wanted])
				position++;
		}

		position--; // Make zero based

		while (camppos[position] != -1) // Port priority!
			position++;

		camppos[position] = i;
	}

	if (numplaying <= 2 || (numingame <= 2 && bestbumper == 1)) // In 1v1s then there's no need for WANTED. In bigger netgames, don't show anyone as WANTED when they're equally matched.
		numwanted = 0;
	else
		numwanted = min(4, 1 + ((numingame-2) / 4));

	for (i = 0; i < 4; i++)
	{
		if (i+1 > numwanted) // Not enough players for this slot to be wanted!
			battlewanted[i] = -1;
		else if (bestbumperplayer != -1 && !setbumper) // If there's a player who has an untied bumper lead over everyone else, they are the first to be wanted.
		{
			battlewanted[i] = bestbumperplayer;
			setbumper = true; // Don't set twice
		}
		else
		{
			// Don't accidentally set the same player, if the bestbumperplayer is also a huge camper.
			while (bestbumperplayer != -1 && camppos[nextcamppos] != -1
				&& bestbumperplayer == camppos[nextcamppos])
				nextcamppos++;

			// Do not add *any* more people if there's too many times that are tied with others.
			// This could theoretically happen very easily if people don't hit each other for a while after the start of a match.
			// (I will be sincerely impressed if more than 2 people tie after people start hitting each other though)

			if (camppos[nextcamppos] == -1 // Out of entries
				|| ties >= (numwanted-i)) // Already counted ties
			{
				battlewanted[i] = -1;
				continue;
			}

			if (ties < (numwanted-i))
			{
				ties = 0; // Reset
				for (j = 0; j < 2; j++)
				{
					if (camppos[nextcamppos+(j+1)] == -1) // Nothing beyond, cancel
						break;
					if (players[camppos[nextcamppos]].kartstuff[k_wanted] == players[camppos[nextcamppos+(j+1)]].kartstuff[k_wanted])
						ties++;
				}
			}

			if (ties < (numwanted-i)) // Is it still low enough after counting?
			{
				battlewanted[i] = camppos[nextcamppos];
				nextcamppos++;
			}
			else
				battlewanted[i] = -1;
		}
	}
}

void K_CheckBumpers(void)
{
	UINT8 i;
	UINT8 numingame = 0;
	SINT8 winnernum = -1;
	INT32 winnerscoreadd = 0;

	if (!multiplayer)
		return;

	if (!G_BattleGametype())
		return;

	if (gameaction == ga_completed)
		return;

	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (!playeringame[i] || players[i].spectator) // not even in-game
			continue;

		if (players[i].exiting) // we're already exiting! stop!
			return;

		numingame++;
		winnerscoreadd += players[i].marescore;

		if (players[i].kartstuff[k_bumper] <= 0) // if you don't have any bumpers, you're probably not a winner
			continue;
		else if (winnernum > -1) // TWO winners? that's dumb :V
			return;

		winnernum = i;
		winnerscoreadd -= players[i].marescore;
	}

	if (numingame <= 1)
		return;

	if (winnernum > -1 && playeringame[winnernum])
	{
		players[winnernum].marescore += winnerscoreadd;
		CONS_Printf(M_GetText("%s recieved %d point%s for winning!\n"), player_names[winnernum], winnerscoreadd, (winnerscoreadd == 1 ? "" : "s"));
	}

	for (i = 0; i < MAXPLAYERS; i++) // This can't go in the earlier loop because winning adds points
		K_KartUpdatePosition(&players[i]);

	for (i = 0; i < MAXPLAYERS; i++) // and it can't be merged with this loop because it needs to be all updated before exiting... multi-loops suck...
		P_DoPlayerExit(&players[i]);
}

void K_CheckSpectateStatus(void)
{
	UINT8 respawnlist[MAXPLAYERS];
	UINT8 i, j, numingame = 0, numjoiners = 0;

	// Maintain spectate wait timer
	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (!playeringame[i])
			continue;
		if (players[i].spectator && (players[i].pflags & PF_WANTSTOJOIN))
			players[i].kartstuff[k_spectatewait]++;
		else
			players[i].kartstuff[k_spectatewait] = 0;
	}

	// No one's allowed to join
	if (!cv_allowteamchange.value)
		return;

	// Get the number of players in game, and the players to be de-spectated.
	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (!playeringame[i])
			continue;

		if (!players[i].spectator)
		{
			numingame++;
			if (cv_ingamecap.value && numingame >= cv_ingamecap.value) // DON'T allow if you've hit the in-game player cap
				return;
			if (gamestate != GS_LEVEL) // Allow if you're not in a level
				continue;
			if (players[i].exiting) // DON'T allow if anyone's exiting
				return;
			if (numingame < 2 || leveltime < starttime || mapreset) // Allow if the match hasn't started yet
				continue;
			if (leveltime > (starttime + 20*TICRATE)) // DON'T allow if the match is 20 seconds in
				return;
			if (G_RaceGametype() && players[i].laps) // DON'T allow if the race is at 2 laps
				return;
			continue;
		}
		else if (!(players[i].pflags & PF_WANTSTOJOIN))
			continue;

		respawnlist[numjoiners++] = i;
	}

	// literally zero point in going any further if nobody is joining
	if (!numjoiners)
		return;

	// Organize by spectate wait timer
	if (cv_ingamecap.value)
	{
		UINT8 oldrespawnlist[MAXPLAYERS];
		memcpy(oldrespawnlist, respawnlist, numjoiners);
		for (i = 0; i < numjoiners; i++)
		{
			UINT8 pos = 0;
			INT32 ispecwait = players[oldrespawnlist[i]].kartstuff[k_spectatewait];

			for (j = 0; j < numjoiners; j++)
			{
				INT32 jspecwait = players[oldrespawnlist[j]].kartstuff[k_spectatewait];
				if (j == i)
					continue;
				if (jspecwait > ispecwait)
					pos++;
				else if (jspecwait == ispecwait && j < i)
					pos++;
			}

			respawnlist[pos] = oldrespawnlist[i];
		}
	}

	// Finally, we can de-spectate everyone!
	for (i = 0; i < numjoiners; i++)
	{
		if (cv_ingamecap.value && numingame+i >= cv_ingamecap.value) // Hit the in-game player cap while adding people?
			break;
		P_SpectatorJoinGame(&players[respawnlist[i]]);
	}

	// Reset the match if you're in an empty server
	if (!mapreset && gamestate == GS_LEVEL && leveltime >= starttime && (numingame < 2 && numingame+i >= 2)) // use previous i value
	{
		S_ChangeMusicInternal("chalng", false); // COME ON
		mapreset = 3*TICRATE; // Even though only the server uses this for game logic, set for everyone for HUD
	}
}

//}

//{ SRB2kart HUD Code

#define NUMPOSNUMS 10
#define NUMPOSFRAMES 7 // White, three blues, three reds
#define NUMWINFRAMES 6 // Red, yellow, green, cyan, blue, purple

//{ 	Patch Definitions
static patch_t *kp_nodraw;

static patch_t *kp_timesticker;
static patch_t *kp_timestickerwide;
static patch_t *kp_lapsticker;
static patch_t *kp_lapstickerwide;
static patch_t *kp_lapstickernarrow;
static patch_t *kp_splitlapflag;
static patch_t *kp_bumpersticker;
static patch_t *kp_bumperstickerwide;
static patch_t *kp_karmasticker;
static patch_t *kp_splitkarmabomb;
static patch_t *kp_timeoutsticker;

static patch_t *kp_startcountdown[16];
static patch_t *kp_racefinish[6];

static patch_t *kp_positionnum[NUMPOSNUMS][NUMPOSFRAMES];
static patch_t *kp_winnernum[NUMPOSFRAMES];

static patch_t *kp_facenum[MAXPLAYERS+1];
static patch_t *kp_facehighlight[8];

static patch_t *kp_rankbumper;
static patch_t *kp_tinybumper[2];
static patch_t *kp_ranknobumpers;

static patch_t *kp_battlewin;
static patch_t *kp_battlecool;
static patch_t *kp_battlelose;
static patch_t *kp_battlewait;
static patch_t *kp_battleinfo;
static patch_t *kp_wanted;
static patch_t *kp_wantedsplit;
static patch_t *kp_wantedreticle;

static patch_t *kp_itembg[4];
static patch_t *kp_itemtimer[2];
static patch_t *kp_itemmulsticker[2];
static patch_t *kp_itemx;

static patch_t *kp_sneaker[2];
static patch_t *kp_rocketsneaker[2];
static patch_t *kp_invincibility[13];
static patch_t *kp_banana[2];
static patch_t *kp_eggman[2];
static patch_t *kp_orbinaut[5];
static patch_t *kp_jawz[2];
static patch_t *kp_mine[2];
static patch_t *kp_ballhog[2];
static patch_t *kp_selfpropelledbomb[2];
static patch_t *kp_grow[2];
static patch_t *kp_shrink[2];
static patch_t *kp_thundershield[2];
static patch_t *kp_hyudoro[2];
static patch_t *kp_pogospring[2];
static patch_t *kp_kitchensink[2];
static patch_t *kp_sadface[2];

static patch_t *kp_check[6];

static patch_t *kp_eggnum[4];

static patch_t *kp_fpview[3];
static patch_t *kp_inputwheel[5];

static patch_t *kp_challenger[25];

static patch_t *kp_lapanim_lap[7];
static patch_t *kp_lapanim_final[11];
static patch_t *kp_lapanim_number[10][3];
static patch_t *kp_lapanim_emblem[2];
static patch_t *kp_lapanim_hand[3];

static patch_t *kp_yougotem;

void K_LoadKartHUDGraphics(void)
{
	INT32 i, j;
	char buffer[9];

	// Null Stuff
	kp_nodraw = 				W_CachePatchName("K_TRNULL", PU_HUDGFX);

	// Stickers
	kp_timesticker = 			W_CachePatchName("K_STTIME", PU_HUDGFX);
	kp_timestickerwide = 		W_CachePatchName("K_STTIMW", PU_HUDGFX);
	kp_lapsticker = 			W_CachePatchName("K_STLAPS", PU_HUDGFX);
	kp_lapstickerwide = 		W_CachePatchName("K_STLAPW", PU_HUDGFX);
	kp_lapstickernarrow = 		W_CachePatchName("K_STLAPN", PU_HUDGFX);
	kp_splitlapflag = 			W_CachePatchName("K_SPTLAP", PU_HUDGFX);
	kp_bumpersticker = 			W_CachePatchName("K_STBALN", PU_HUDGFX);
	kp_bumperstickerwide = 		W_CachePatchName("K_STBALW", PU_HUDGFX);
	kp_karmasticker = 			W_CachePatchName("K_STKARM", PU_HUDGFX);
	kp_splitkarmabomb = 		W_CachePatchName("K_SPTKRM", PU_HUDGFX);
	kp_timeoutsticker = 		W_CachePatchName("K_STTOUT", PU_HUDGFX);

	// Starting countdown
	kp_startcountdown[0] = 		W_CachePatchName("K_CNT3A", PU_HUDGFX);
	kp_startcountdown[1] = 		W_CachePatchName("K_CNT2A", PU_HUDGFX);
	kp_startcountdown[2] = 		W_CachePatchName("K_CNT1A", PU_HUDGFX);
	kp_startcountdown[3] = 		W_CachePatchName("K_CNTGOA", PU_HUDGFX);
	kp_startcountdown[4] = 		W_CachePatchName("K_CNT3B", PU_HUDGFX);
	kp_startcountdown[5] = 		W_CachePatchName("K_CNT2B", PU_HUDGFX);
	kp_startcountdown[6] = 		W_CachePatchName("K_CNT1B", PU_HUDGFX);
	kp_startcountdown[7] = 		W_CachePatchName("K_CNTGOB", PU_HUDGFX);
	// Splitscreen
	kp_startcountdown[8] = 		W_CachePatchName("K_SMC3A", PU_HUDGFX);
	kp_startcountdown[9] = 		W_CachePatchName("K_SMC2A", PU_HUDGFX);
	kp_startcountdown[10] = 	W_CachePatchName("K_SMC1A", PU_HUDGFX);
	kp_startcountdown[11] = 	W_CachePatchName("K_SMCGOA", PU_HUDGFX);
	kp_startcountdown[12] = 	W_CachePatchName("K_SMC3B", PU_HUDGFX);
	kp_startcountdown[13] = 	W_CachePatchName("K_SMC2B", PU_HUDGFX);
	kp_startcountdown[14] = 	W_CachePatchName("K_SMC1B", PU_HUDGFX);
	kp_startcountdown[15] = 	W_CachePatchName("K_SMCGOB", PU_HUDGFX);

	// Finish
	kp_racefinish[0] = 			W_CachePatchName("K_FINA", PU_HUDGFX);
	kp_racefinish[1] = 			W_CachePatchName("K_FINB", PU_HUDGFX);
	// Splitscreen
	kp_racefinish[2] = 			W_CachePatchName("K_SMFINA", PU_HUDGFX);
	kp_racefinish[3] = 			W_CachePatchName("K_SMFINB", PU_HUDGFX);
	// 2P splitscreen
	kp_racefinish[4] = 			W_CachePatchName("K_2PFINA", PU_HUDGFX);
	kp_racefinish[5] = 			W_CachePatchName("K_2PFINB", PU_HUDGFX);

	// Position numbers
	sprintf(buffer, "K_POSNxx");
	for (i = 0; i < NUMPOSNUMS; i++)
	{
		buffer[6] = '0'+i;
		for (j = 0; j < NUMPOSFRAMES; j++)
		{
			//sprintf(buffer, "K_POSN%d%d", i, j);
			buffer[7] = '0'+j;
			kp_positionnum[i][j] = (patch_t *) W_CachePatchName(buffer, PU_HUDGFX);
		}
	}

	sprintf(buffer, "K_POSNWx");
	for (i = 0; i < NUMWINFRAMES; i++)
	{
		buffer[7] = '0'+i;
		kp_winnernum[i] = (patch_t *) W_CachePatchName(buffer, PU_HUDGFX);
	}

	sprintf(buffer, "OPPRNKxx");
	for (i = 0; i <= MAXPLAYERS; i++)
	{
		buffer[6] = '0'+(i/10);
		buffer[7] = '0'+(i%10);
		kp_facenum[i] = (patch_t *) W_CachePatchName(buffer, PU_HUDGFX);
	}

	sprintf(buffer, "K_CHILIx");
	for (i = 0; i < 8; i++)
	{
		buffer[7] = '0'+(i+1);
		kp_facehighlight[i] = (patch_t *) W_CachePatchName(buffer, PU_HUDGFX);
	}

	// Extra ranking icons
	kp_rankbumper =				W_CachePatchName("K_BLNICO", PU_HUDGFX);
	kp_tinybumper[0] =			W_CachePatchName("K_BLNA", PU_HUDGFX);
	kp_tinybumper[1] =			W_CachePatchName("K_BLNB", PU_HUDGFX);
	kp_ranknobumpers =			W_CachePatchName("K_NOBLNS", PU_HUDGFX);

	// Battle graphics
	kp_battlewin = 				W_CachePatchName("K_BWIN", PU_HUDGFX);
	kp_battlecool = 			W_CachePatchName("K_BCOOL", PU_HUDGFX);
	kp_battlelose = 			W_CachePatchName("K_BLOSE", PU_HUDGFX);
	kp_battlewait = 			W_CachePatchName("K_BWAIT", PU_HUDGFX);
	kp_battleinfo = 			W_CachePatchName("K_BINFO", PU_HUDGFX);
	kp_wanted = 				W_CachePatchName("K_WANTED", PU_HUDGFX);
	kp_wantedsplit = 			W_CachePatchName("4PWANTED", PU_HUDGFX);
	kp_wantedreticle =			W_CachePatchName("MMAPWANT", PU_HUDGFX);

	// Kart Item Windows
	kp_itembg[0] = 				W_CachePatchName("K_ITBG", PU_HUDGFX);
	kp_itembg[1] = 				W_CachePatchName("K_ITBGD", PU_HUDGFX);
	kp_itemtimer[0] = 			W_CachePatchName("K_ITIMER", PU_HUDGFX);
	kp_itemmulsticker[0] = 		W_CachePatchName("K_ITMUL", PU_HUDGFX);
	kp_itemx = 					W_CachePatchName("K_ITX", PU_HUDGFX);

	kp_sneaker[0] =				W_CachePatchName("K_ITSHOE", PU_HUDGFX);
	kp_rocketsneaker[0] =		W_CachePatchName("K_ITRSHE", PU_HUDGFX);

	sprintf(buffer, "K_ITINVx");
	for (i = 0; i < 7; i++)
	{
		buffer[7] = '1'+i;
		kp_invincibility[i] = (patch_t *) W_CachePatchName(buffer, PU_HUDGFX);
	}
	kp_banana[0] =				W_CachePatchName("K_ITBANA", PU_HUDGFX);
	kp_eggman[0] =				W_CachePatchName("K_ITEGGM", PU_HUDGFX);
	sprintf(buffer, "K_ITORBx");
	for (i = 0; i < 4; i++)
	{
		buffer[7] = '1'+i;
		kp_orbinaut[i] = (patch_t *) W_CachePatchName(buffer, PU_HUDGFX);
	}
	kp_jawz[0] =				W_CachePatchName("K_ITJAWZ", PU_HUDGFX);
	kp_mine[0] =				W_CachePatchName("K_ITMINE", PU_HUDGFX);
	kp_ballhog[0] =				W_CachePatchName("K_ITBHOG", PU_HUDGFX);
	kp_selfpropelledbomb[0] =	W_CachePatchName("K_ITSPB", PU_HUDGFX);
	kp_grow[0] =				W_CachePatchName("K_ITGROW", PU_HUDGFX);
	kp_shrink[0] =				W_CachePatchName("K_ITSHRK", PU_HUDGFX);
	kp_thundershield[0] =		W_CachePatchName("K_ITTHNS", PU_HUDGFX);
	kp_hyudoro[0] = 			W_CachePatchName("K_ITHYUD", PU_HUDGFX);
	kp_pogospring[0] = 			W_CachePatchName("K_ITPOGO", PU_HUDGFX);
	kp_kitchensink[0] = 		W_CachePatchName("K_ITSINK", PU_HUDGFX);
	kp_sadface[0] = 			W_CachePatchName("K_ITSAD", PU_HUDGFX);

	// Splitscreen
	kp_itembg[2] = 				W_CachePatchName("K_ISBG", PU_HUDGFX);
	kp_itembg[3] = 				W_CachePatchName("K_ISBGD", PU_HUDGFX);
	kp_itemtimer[1] = 			W_CachePatchName("K_ISIMER", PU_HUDGFX);
	kp_itemmulsticker[1] = 		W_CachePatchName("K_ISMUL", PU_HUDGFX);

	kp_sneaker[1] =				W_CachePatchName("K_ISSHOE", PU_HUDGFX);
	kp_rocketsneaker[1] =		W_CachePatchName("K_ISRSHE", PU_HUDGFX);
	sprintf(buffer, "K_ISINVx");
	for (i = 0; i < 6; i++)
	{
		buffer[7] = '1'+i;
		kp_invincibility[i+7] = (patch_t *) W_CachePatchName(buffer, PU_HUDGFX);
	}
	kp_banana[1] =				W_CachePatchName("K_ISBANA", PU_HUDGFX);
	kp_eggman[1] =				W_CachePatchName("K_ISEGGM", PU_HUDGFX);
	kp_orbinaut[4] =			W_CachePatchName("K_ISORBN", PU_HUDGFX);
	kp_jawz[1] =				W_CachePatchName("K_ISJAWZ", PU_HUDGFX);
	kp_mine[1] =				W_CachePatchName("K_ISMINE", PU_HUDGFX);
	kp_ballhog[1] =				W_CachePatchName("K_ISBHOG", PU_HUDGFX);
	kp_selfpropelledbomb[1] =	W_CachePatchName("K_ISSPB", PU_HUDGFX);
	kp_grow[1] =				W_CachePatchName("K_ISGROW", PU_HUDGFX);
	kp_shrink[1] =				W_CachePatchName("K_ISSHRK", PU_HUDGFX);
	kp_thundershield[1] =		W_CachePatchName("K_ISTHNS", PU_HUDGFX);
	kp_hyudoro[1] = 			W_CachePatchName("K_ISHYUD", PU_HUDGFX);
	kp_pogospring[1] = 			W_CachePatchName("K_ISPOGO", PU_HUDGFX);
	kp_kitchensink[1] = 		W_CachePatchName("K_ISSINK", PU_HUDGFX);
	kp_sadface[1] = 			W_CachePatchName("K_ISSAD", PU_HUDGFX);

	// CHECK indicators
	sprintf(buffer, "K_CHECKx");
	for (i = 0; i < 6; i++)
	{
		buffer[7] = '1'+i;
		kp_check[i] = (patch_t *) W_CachePatchName(buffer, PU_HUDGFX);
	}

	// Eggman warning numbers
	sprintf(buffer, "K_EGGNx");
	for (i = 0; i < 4; i++)
	{
		buffer[6] = '0'+i;
		kp_eggnum[i] = (patch_t *) W_CachePatchName(buffer, PU_HUDGFX);
	}

	// First person mode
	kp_fpview[0] = 				W_CachePatchName("VIEWA0", PU_HUDGFX);
	kp_fpview[1] =				W_CachePatchName("VIEWB0D0", PU_HUDGFX);
	kp_fpview[2] = 				W_CachePatchName("VIEWC0E0", PU_HUDGFX);

	// Input UI Wheel
	sprintf(buffer, "K_WHEELx");
	for (i = 0; i < 5; i++)
	{
		buffer[7] = '0'+i;
		kp_inputwheel[i] = (patch_t *) W_CachePatchName(buffer, PU_HUDGFX);
	}

	// HERE COMES A NEW CHALLENGER
	sprintf(buffer, "K_CHALxx");
	for (i = 0; i < 25; i++)
	{
		buffer[6] = '0'+((i+1)/10);
		buffer[7] = '0'+((i+1)%10);
		kp_challenger[i] = (patch_t *) W_CachePatchName(buffer, PU_HUDGFX);
	}

	// Lap start animation
	sprintf(buffer, "K_LAP0x");
	for (i = 0; i < 7; i++)
	{
		buffer[6] = '0'+(i+1);
		kp_lapanim_lap[i] = (patch_t *) W_CachePatchName(buffer, PU_HUDGFX);
	}

	sprintf(buffer, "K_LAPFxx");
	for (i = 0; i < 11; i++)
	{
		buffer[6] = '0'+((i+1)/10);
		buffer[7] = '0'+((i+1)%10);
		kp_lapanim_final[i] = (patch_t *) W_CachePatchName(buffer, PU_HUDGFX);
	}

	sprintf(buffer, "K_LAPNxx");
	for (i = 0; i < 10; i++)
	{
		buffer[6] = '0'+i;
		for (j = 0; j < 3; j++)
		{
			buffer[7] = '0'+(j+1);
			kp_lapanim_number[i][j] = (patch_t *) W_CachePatchName(buffer, PU_HUDGFX);
		}
	}

	sprintf(buffer, "K_LAPE0x");
	for (i = 0; i < 2; i++)
	{
		buffer[7] = '0'+(i+1);
		kp_lapanim_emblem[i] = (patch_t *) W_CachePatchName(buffer, PU_HUDGFX);
	}

	sprintf(buffer, "K_LAPH0x");
	for (i = 0; i < 3; i++)
	{
		buffer[7] = '0'+(i+1);
		kp_lapanim_hand[i] = (patch_t *) W_CachePatchName(buffer, PU_HUDGFX);
	}

	kp_yougotem = (patch_t *) W_CachePatchName("YOUGOTEM", PU_HUDGFX);
}

// For the item toggle menu
const char *K_GetItemPatch(UINT8 item, boolean tiny)
{
	switch (item)
	{
		case KITEM_SNEAKER:
		case KRITEM_TRIPLESNEAKER:
			return (tiny ? "K_ISSHOE" : "K_ITSHOE");
		case KITEM_ROCKETSNEAKER:
			return (tiny ? "K_ISRSHE" : "K_ITRSHE");
		case KITEM_INVINCIBILITY:
			return (tiny ? "K_ISINV1" : "K_ITINV1");
		case KITEM_BANANA:
		case KRITEM_TRIPLEBANANA:
		case KRITEM_TENFOLDBANANA:
			return (tiny ? "K_ISBANA" : "K_ITBANA");
		case KITEM_EGGMAN:
			return (tiny ? "K_ISEGGM" : "K_ITEGGM");
		case KITEM_ORBINAUT:
			return (tiny ? "K_ISORBN" : "K_ITORB1");
		case KITEM_JAWZ:
		case KRITEM_DUALJAWZ:
			return (tiny ? "K_ISJAWZ" : "K_ITJAWZ");
		case KITEM_MINE:
			return (tiny ? "K_ISMINE" : "K_ITMINE");
		case KITEM_BALLHOG:
			return (tiny ? "K_ISBHOG" : "K_ITBHOG");
		case KITEM_SPB:
			return (tiny ? "K_ISSPB" : "K_ITSPB");
		case KITEM_GROW:
			return (tiny ? "K_ISGROW" : "K_ITGROW");
		case KITEM_SHRINK:
			return (tiny ? "K_ISSHRK" : "K_ITSHRK");
		case KITEM_THUNDERSHIELD:
			return (tiny ? "K_ISTHNS" : "K_ITTHNS");
		case KITEM_HYUDORO:
			return (tiny ? "K_ISHYUD" : "K_ITHYUD");
		case KITEM_POGOSPRING:
			return (tiny ? "K_ISPOGO" : "K_ITPOGO");
		case KITEM_KITCHENSINK:
			return (tiny ? "K_ISSINK" : "K_ITSINK");
		case KRITEM_TRIPLEORBINAUT:
			return (tiny ? "K_ISORBN" : "K_ITORB3");
		case KRITEM_QUADORBINAUT:
			return (tiny ? "K_ISORBN" : "K_ITORB4");
		default:
			return (tiny ? "K_ISSAD" : "K_ITSAD");
	}
}

//}

INT32 ITEM_X, ITEM_Y;	// Item Window
INT32 TIME_X, TIME_Y;	// Time Sticker
INT32 LAPS_X, LAPS_Y;	// Lap Sticker
INT32 SPDM_X, SPDM_Y;	// Speedometer
INT32 POSI_X, POSI_Y;	// Position Number
INT32 FACE_X, FACE_Y;	// Top-four Faces
INT32 STCD_X, STCD_Y;	// Starting countdown
INT32 CHEK_Y;			// CHECK graphic
INT32 MINI_X, MINI_Y;	// Minimap
INT32 WANT_X, WANT_Y;	// Battle WANTED poster

// This is for the P2 and P4 side of splitscreen. Then we'll flip P1's and P2's to the bottom with V_SPLITSCREEN.
INT32 ITEM2_X, ITEM2_Y;
INT32 LAPS2_X, LAPS2_Y;
INT32 POSI2_X, POSI2_Y;


static void K_initKartHUD(void)
{
	/*
		BASEVIDWIDTH  = 320
		BASEVIDHEIGHT = 200

		Item window graphic is 41 x 33

		Time Sticker graphic is 116 x 11
		Time Font is a solid block of (8 x [12) x 14], equal to 96 x 14
		Therefore, timestamp is 116 x 14 altogether

		Lap Sticker is 80 x 11
		Lap flag is 22 x 20
		Lap Font is a solid block of (3 x [12) x 14], equal to 36 x 14
		Therefore, lapstamp is 80 x 20 altogether

		Position numbers are 43 x 53

		Faces are 32 x 32
		Faces draw downscaled at 16 x 16
		Therefore, the allocated space for them is 16 x 67 altogether

		----

		ORIGINAL CZ64 SPLITSCREEN:

		Item window:
		if (!splitscreen) 	{ ICONX = 139; 				ICONY = 20; }
		else 				{ ICONX = BASEVIDWIDTH-315; ICONY = 60; }

		Time: 			   236, STRINGY(			   12)
		Lap:  BASEVIDWIDTH-304, STRINGY(BASEVIDHEIGHT-189)

	*/

	// Single Screen (defaults)
	// Item Window
	ITEM_X = 5;						//   5
	ITEM_Y = 5;						//   5
	// Level Timer
	TIME_X = BASEVIDWIDTH - 148;	// 172
	TIME_Y = 9;						//   9
	// Level Laps
	LAPS_X = 9;						//   9
	LAPS_Y = BASEVIDHEIGHT - 29;	// 171
	// Speedometer
	SPDM_X = 9;						//   9
	SPDM_Y = BASEVIDHEIGHT - 45;	// 155
	// Position Number
	POSI_X = BASEVIDWIDTH  - 9;		// 268
	POSI_Y = BASEVIDHEIGHT - 9;		// 138
	// Top-Four Faces
	FACE_X = 9;						//   9
	FACE_Y = 92;					//  92
	// Starting countdown
	STCD_X = BASEVIDWIDTH/2;		//   9
	STCD_Y = BASEVIDHEIGHT/2;		//  92
	// CHECK graphic
	CHEK_Y = BASEVIDHEIGHT;			// 200
	// Minimap
	MINI_X = BASEVIDWIDTH - 50;		// 270
	MINI_Y = (BASEVIDHEIGHT/2)-16; //  84
	// Battle WANTED poster
	WANT_X = BASEVIDWIDTH - 55;		// 270
	WANT_Y = BASEVIDHEIGHT- 71;		// 176

	if (splitscreen)	// Splitscreen
	{
		ITEM_X = 5;
		ITEM_Y = 3;

		LAPS_Y = (BASEVIDHEIGHT/2)-24;

		POSI_Y = (BASEVIDHEIGHT/2)- 2;

		STCD_Y = BASEVIDHEIGHT/4;

		MINI_Y = (BASEVIDHEIGHT/2);

		if (splitscreen > 1)	// 3P/4P Small Splitscreen
		{
			// 1P (top left)
			ITEM_X = -9;
			ITEM_Y = -8;

			LAPS_X = 3;
			LAPS_Y = (BASEVIDHEIGHT/2)-13;

			POSI_X = 24;
			POSI_Y = (BASEVIDHEIGHT/2)- 16;

			// 2P (top right)
			ITEM2_X = BASEVIDWIDTH-39;
			ITEM2_Y = -8;

			LAPS2_X = BASEVIDWIDTH-40;
			LAPS2_Y = (BASEVIDHEIGHT/2)-13;

			POSI2_X = BASEVIDWIDTH -4;
			POSI2_Y = (BASEVIDHEIGHT/2)- 16;

			// Reminder that 3P and 4P are just 1P and 2P splitscreen'd to the bottom.

			STCD_X = BASEVIDWIDTH/4;

			MINI_X = (3*BASEVIDWIDTH/4);
			MINI_Y = (3*BASEVIDHEIGHT/4);

			if (splitscreen > 2) // 4P-only
			{
				MINI_X = (BASEVIDWIDTH/2);
				MINI_Y = (BASEVIDHEIGHT/2);
			}
		}
	}

	if (timeinmap > 113)
		hudtrans = cv_translucenthud.value;
	else if (timeinmap > 105)
		hudtrans = ((((INT32)timeinmap) - 105)*cv_translucenthud.value)/(113-105);
	else
		hudtrans = 0;
}

INT32 K_calcSplitFlags(INT32 snapflags)
{
	INT32 splitflags = 0;

	if (splitscreen == 0)
		return snapflags;

	if (stplyr != &players[displayplayers[0]])
	{
		if (splitscreen == 1 && stplyr == &players[displayplayers[1]])
		{
			splitflags |= V_SPLITSCREEN;
		}
		else if (splitscreen > 1)
		{
			if (stplyr == &players[displayplayers[2]] || (splitscreen == 3 && stplyr == &players[displayplayers[3]]))
				splitflags |= V_SPLITSCREEN;
			if (stplyr == &players[displayplayers[1]] || (splitscreen == 3 && stplyr == &players[displayplayers[3]]))
				splitflags |= V_HORZSCREEN;
		}
	}

	if (splitflags & V_SPLITSCREEN)
		snapflags &= ~V_SNAPTOTOP;
	else
		snapflags &= ~V_SNAPTOBOTTOM;

	if (splitscreen > 1)
	{
		if (splitflags & V_HORZSCREEN)
			snapflags &= ~V_SNAPTOLEFT;
		else
			snapflags &= ~V_SNAPTORIGHT;
	}

	return (splitflags|snapflags);
}

static void K_drawKartItem(void)
{
	// ITEM_X = BASEVIDWIDTH-50;	// 270
	// ITEM_Y = 24;					//  24

	// Why write V_DrawScaledPatch calls over and over when they're all the same?
	// Set to 'no item' just in case.
	const UINT8 offset = ((splitscreen > 1) ? 1 : 0);
	patch_t *localpatch = kp_nodraw;
	patch_t *localbg = ((offset) ? kp_itembg[2] : kp_itembg[0]);
	patch_t *localinv = ((offset) ? kp_invincibility[((leveltime % (6*3)) / 3) + 7] : kp_invincibility[(leveltime % (7*3)) / 3]);
	INT32 fx = 0, fy = 0, fflags = 0;	// final coords for hud and flags...
	//INT32 splitflags = K_calcSplitFlags(V_SNAPTOTOP|V_SNAPTOLEFT);
	const INT32 numberdisplaymin = ((!offset && stplyr->kartstuff[k_itemtype] == KITEM_ORBINAUT) ? 5 : 2);
	INT32 itembar = 0;
	INT32 maxl = 0; // itembar's normal highest value
	const INT32 barlength = (splitscreen > 1 ? 12 : 26);
	UINT8 localcolor = SKINCOLOR_NONE;
	SINT8 colormode = TC_RAINBOW;
	UINT8 *colmap = NULL;
	boolean flipamount = false;	// Used for 3P/4P splitscreen to flip item amount stuff

	if (stplyr->kartstuff[k_itemroulette])
	{
		if (stplyr->skincolor)
			localcolor = stplyr->skincolor;

		switch((stplyr->kartstuff[k_itemroulette] % (14*3)) / 3)
		{
			// Each case is handled in threes, to give three frames of in-game time to see the item on the roulette
			case 0: // Sneaker
				localpatch = kp_sneaker[offset];
				//localcolor = SKINCOLOR_RASPBERRY;
				break;
			case 1: // Banana
				localpatch = kp_banana[offset];
				//localcolor = SKINCOLOR_YELLOW;
				break;
			case 2: // Orbinaut
				localpatch = kp_orbinaut[3+offset];
				//localcolor = SKINCOLOR_STEEL;
				break;
			case 3: // Mine
				localpatch = kp_mine[offset];
				//localcolor = SKINCOLOR_JET;
				break;
			case 4: // Grow
				localpatch = kp_grow[offset];
				//localcolor = SKINCOLOR_TEAL;
				break;
			case 5: // Hyudoro
				localpatch = kp_hyudoro[offset];
				//localcolor = SKINCOLOR_STEEL;
				break;
			case 6: // Rocket Sneaker
				localpatch = kp_rocketsneaker[offset];
				//localcolor = SKINCOLOR_TANGERINE;
				break;
			case 7: // Jawz
				localpatch = kp_jawz[offset];
				//localcolor = SKINCOLOR_JAWZ;
				break;
			case 8: // Self-Propelled Bomb
				localpatch = kp_selfpropelledbomb[offset];
				//localcolor = SKINCOLOR_JET;
				break;
			case 9: // Shrink
				localpatch = kp_shrink[offset];
				//localcolor = SKINCOLOR_ORANGE;
				break;
			case 10: // Invincibility
				localpatch = localinv;
				//localcolor = SKINCOLOR_GREY;
				break;
			case 11: // Eggman Monitor
				localpatch = kp_eggman[offset];
				//localcolor = SKINCOLOR_ROSE;
				break;
			case 12: // Ballhog
				localpatch = kp_ballhog[offset];
				//localcolor = SKINCOLOR_LILAC;
				break;
			case 13: // Thunder Shield
				localpatch = kp_thundershield[offset];
				//localcolor = SKINCOLOR_CYAN;
				break;
			/*case 14: // Pogo Spring
				localpatch = kp_pogospring[offset];
				localcolor = SKINCOLOR_TANGERINE;
				break;
			case 15: // Kitchen Sink
				localpatch = kp_kitchensink[offset];
				localcolor = SKINCOLOR_STEEL;
				break;*/
			default:
				break;
		}
	}
	else
	{
		// I'm doing this a little weird and drawing mostly in reverse order
		// The only actual reason is to make sneakers line up this way in the code below
		// This shouldn't have any actual baring over how it functions
		// Hyudoro is first, because we're drawing it on top of the player's current item
		if (stplyr->kartstuff[k_stolentimer] > 0)
		{
			if (leveltime & 2)
				localpatch = kp_hyudoro[offset];
			else
				localpatch = kp_nodraw;
		}
		else if ((stplyr->kartstuff[k_stealingtimer] > 0) && (leveltime & 2))
		{
			localpatch = kp_hyudoro[offset];
		}
		else if (stplyr->kartstuff[k_eggmanexplode] > 1)
		{
			if (leveltime & 1)
				localpatch = kp_eggman[offset];
			else
				localpatch = kp_nodraw;
		}
		else if (stplyr->kartstuff[k_rocketsneakertimer] > 1)
		{
			itembar = stplyr->kartstuff[k_rocketsneakertimer];
			maxl = (itemtime*3) - barlength;

			if (leveltime & 1)
				localpatch = kp_rocketsneaker[offset];
			else
				localpatch = kp_nodraw;
		}
		else if (stplyr->kartstuff[k_growshrinktimer] > 0)
		{
			if (stplyr->kartstuff[k_growcancel] > 0)
			{
				itembar = stplyr->kartstuff[k_growcancel];
				maxl = 26;
			}

			if (leveltime & 1)
				localpatch = kp_grow[offset];
			else
				localpatch = kp_nodraw;
		}
		else if (stplyr->kartstuff[k_sadtimer] > 0)
		{
			if (leveltime & 2)
				localpatch = kp_sadface[offset];
			else
				localpatch = kp_nodraw;
		}
		else
		{
			if (stplyr->kartstuff[k_itemamount] <= 0)
				return;

			switch(stplyr->kartstuff[k_itemtype])
			{
				case KITEM_SNEAKER:
					localpatch = kp_sneaker[offset];
					break;
				case KITEM_ROCKETSNEAKER:
					localpatch = kp_rocketsneaker[offset];
					break;
				case KITEM_INVINCIBILITY:
					localpatch = localinv;
					localbg = kp_itembg[offset+1];
					break;
				case KITEM_BANANA:
					localpatch = kp_banana[offset];
					break;
				case KITEM_EGGMAN:
					localpatch = kp_eggman[offset];
					break;
				case KITEM_ORBINAUT:
					localpatch = kp_orbinaut[(offset ? 4 : min(stplyr->kartstuff[k_itemamount]-1, 3))];
					break;
				case KITEM_JAWZ:
					localpatch = kp_jawz[offset];
					break;
				case KITEM_MINE:
					localpatch = kp_mine[offset];
					break;
				case KITEM_BALLHOG:
					localpatch = kp_ballhog[offset];
					break;
				case KITEM_SPB:
					localpatch = kp_selfpropelledbomb[offset];
					localbg = kp_itembg[offset+1];
					break;
				case KITEM_GROW:
					localpatch = kp_grow[offset];
					break;
				case KITEM_SHRINK:
					localpatch = kp_shrink[offset];
					break;
				case KITEM_THUNDERSHIELD:
					localpatch = kp_thundershield[offset];
					localbg = kp_itembg[offset+1];
					break;
				case KITEM_HYUDORO:
					localpatch = kp_hyudoro[offset];
					break;
				case KITEM_POGOSPRING:
					localpatch = kp_pogospring[offset];
					break;
				case KITEM_KITCHENSINK:
					localpatch = kp_kitchensink[offset];
					break;
				case KITEM_SAD:
					localpatch = kp_sadface[offset];
					break;
				default:
					return;
			}

			if (stplyr->kartstuff[k_itemheld] && !(leveltime & 1))
				localpatch = kp_nodraw;
		}

		if (stplyr->kartstuff[k_itemblink] && (leveltime & 1))
		{
			colormode = TC_BLINK;

			switch (stplyr->kartstuff[k_itemblinkmode])
			{
				case 2:
					localcolor = (UINT8)(1 + (leveltime % (MAXSKINCOLORS-1)));
					break;
				case 1:
					localcolor = SKINCOLOR_RED;
					break;
				default:
					localcolor = SKINCOLOR_WHITE;
					break;
			}
		}
	}

	// pain and suffering defined below
	if (splitscreen < 2) // don't change shit for THIS splitscreen.
	{
		fx = ITEM_X;
		fy = ITEM_Y;
		fflags = K_calcSplitFlags(V_SNAPTOTOP|V_SNAPTOLEFT);
	}
	else // now we're having a fun game.
	{
		if (stplyr == &players[displayplayers[0]] || stplyr == &players[displayplayers[2]]) // If we are P1 or P3...
		{
			fx = ITEM_X;
			fy = ITEM_Y;
			fflags = V_SNAPTOLEFT|((stplyr == &players[displayplayers[2]]) ? V_SPLITSCREEN : V_SNAPTOTOP); // flip P3 to the bottom.
		}
		else // else, that means we're P2 or P4.
		{
			fx = ITEM2_X;
			fy = ITEM2_Y;
			fflags = V_SNAPTORIGHT|((stplyr == &players[displayplayers[3]]) ? V_SPLITSCREEN : V_SNAPTOTOP); // flip P4 to the bottom
			flipamount = true;
		}
	}

	if (localcolor != SKINCOLOR_NONE)
		colmap = R_GetTranslationColormap(colormode, localcolor, GTC_CACHE);

	V_DrawScaledPatch(fx, fy, V_HUDTRANS|fflags, localbg);

	// Then, the numbers:
	if (stplyr->kartstuff[k_itemamount] >= numberdisplaymin && !stplyr->kartstuff[k_itemroulette])
	{
		V_DrawScaledPatch(fx + (flipamount ? 48 : 0), fy, V_HUDTRANS|fflags|(flipamount ? V_FLIP : 0), kp_itemmulsticker[offset]); // flip this graphic for p2 and p4 in split and shift it.
		V_DrawFixedPatch(fx<<FRACBITS, fy<<FRACBITS, FRACUNIT, V_HUDTRANS|fflags, localpatch, colmap);
		if (offset)
			if (flipamount) // reminder that this is for 3/4p's right end of the screen.
				V_DrawString(fx+2, fy+31, V_ALLOWLOWERCASE|V_HUDTRANS|fflags, va("x%d", stplyr->kartstuff[k_itemamount]));
			else
				V_DrawString(fx+24, fy+31, V_ALLOWLOWERCASE|V_HUDTRANS|fflags, va("x%d", stplyr->kartstuff[k_itemamount]));
		else
		{
			V_DrawScaledPatch(fy+28, fy+41, V_HUDTRANS|fflags, kp_itemx);
			V_DrawKartString(fx+38, fy+36, V_HUDTRANS|fflags, va("%d", stplyr->kartstuff[k_itemamount]));
		}
	}
	else
		V_DrawFixedPatch(fx<<FRACBITS, fy<<FRACBITS, FRACUNIT, V_HUDTRANS|fflags, localpatch, colmap);

	// Extensible meter, currently only used for rocket sneaker...
	if (itembar && hudtrans)
	{
		const INT32 fill = ((itembar*barlength)/maxl);
		const INT32 length = min(barlength, fill);
		const INT32 height = (offset ? 1 : 2);
		const INT32 x = (offset ? 17 : 11), y = (offset ? 27 : 35);

		V_DrawScaledPatch(fx+x, fy+y, V_HUDTRANS|fflags, kp_itemtimer[offset]);
		// The left dark "AA" edge
		V_DrawFill(fx+x+1, fy+y+1, (length == 2 ? 2 : 1), height, 12|fflags);
		// The bar itself
		if (length > 2)
		{
			V_DrawFill(fx+x+length, fy+y+1, 1, height, 12|fflags); // the right one
			if (height == 2)
				V_DrawFill(fx+x+2, fy+y+2, length-2, 1, 8|fflags); // the dulled underside
			V_DrawFill(fx+x+2, fy+y+1, length-2, 1, 120|fflags); // the shine
		}
	}

	// Quick Eggman numbers
	if (stplyr->kartstuff[k_eggmanexplode] > 1 /*&& stplyr->kartstuff[k_eggmanexplode] <= 3*TICRATE*/)
		V_DrawScaledPatch(fx+17, fy+13-offset, V_HUDTRANS|fflags, kp_eggnum[min(3, G_TicsToSeconds(stplyr->kartstuff[k_eggmanexplode]))]);

}

void K_drawKartTimestamp(tic_t drawtime, INT32 TX, INT32 TY, INT16 emblemmap, UINT8 mode)
{
	// TIME_X = BASEVIDWIDTH-124;	// 196
	// TIME_Y = 6;					//   6

	tic_t worktime;

	INT32 splitflags = 0;
	if (!mode)
	{
		splitflags = V_HUDTRANS|K_calcSplitFlags(V_SNAPTOTOP|V_SNAPTORIGHT);
		if (cv_timelimit.value && timelimitintics > 0)
		{
			if (drawtime >= timelimitintics)
				drawtime = 0;
			else
				drawtime = timelimitintics - drawtime;
		}
	}

	V_DrawScaledPatch(TX, TY, splitflags, ((mode == 2) ? kp_lapstickerwide : kp_timestickerwide));

	TX += 33;

	worktime = drawtime/(60*TICRATE);

	if (mode && !drawtime)
		V_DrawKartString(TX, TY+3, splitflags, va("--'--\"--"));
	else if (worktime < 100) // 99:99:99 only
	{
		// zero minute
		if (worktime < 10)
		{
			V_DrawKartString(TX, TY+3, splitflags, va("0"));
			// minutes time       0 __ __
			V_DrawKartString(TX+12, TY+3, splitflags, va("%d", worktime));
		}
		// minutes time       0 __ __
		else
			V_DrawKartString(TX, TY+3, splitflags, va("%d", worktime));

		// apostrophe location     _'__ __
		V_DrawKartString(TX+24, TY+3, splitflags, va("'"));

		worktime = (drawtime/TICRATE % 60);

		// zero second        _ 0_ __
		if (worktime < 10)
		{
			V_DrawKartString(TX+36, TY+3, splitflags, va("0"));
		// seconds time       _ _0 __
			V_DrawKartString(TX+48, TY+3, splitflags, va("%d", worktime));
		}
		// zero second        _ 00 __
		else
			V_DrawKartString(TX+36, TY+3, splitflags, va("%d", worktime));

		// quotation mark location    _ __"__
		V_DrawKartString(TX+60, TY+3, splitflags, va("\""));

		worktime = G_TicsToCentiseconds(drawtime);

		// zero tick          _ __ 0_
		if (worktime < 10)
		{
			V_DrawKartString(TX+72, TY+3, splitflags, va("0"));
		// tics               _ __ _0
			V_DrawKartString(TX+84, TY+3, splitflags, va("%d", worktime));
		}
		// zero tick          _ __ 00
		else
			V_DrawKartString(TX+72, TY+3, splitflags, va("%d", worktime));
	}
	else if ((drawtime/TICRATE) & 1)
		V_DrawKartString(TX, TY+3, splitflags, va("99'59\"99"));

	if (emblemmap && (modeattacking || (mode == 1)) && !demo.playback) // emblem time!
	{
		INT32 workx = TX + 96, worky = TY+18;
		SINT8 curemb = 0;
		patch_t *emblempic[3] = {NULL, NULL, NULL};
		UINT8 *emblemcol[3] = {NULL, NULL, NULL};

		emblem_t *emblem = M_GetLevelEmblems(emblemmap);
		while (emblem)
		{
			char targettext[9];

			switch (emblem->type)
			{
				case ET_TIME:
					{
						static boolean canplaysound = true;
						tic_t timetoreach = emblem->var;

						if (emblem->collected)
						{
							emblempic[curemb] = W_CachePatchName(M_GetEmblemPatch(emblem), PU_CACHE);
							emblemcol[curemb] = R_GetTranslationColormap(TC_DEFAULT, M_GetEmblemColor(emblem), GTC_CACHE);
							if (++curemb == 3)
								break;
							goto bademblem;
						}

						snprintf(targettext, 9, "%i'%02i\"%02i",
							G_TicsToMinutes(timetoreach, false),
							G_TicsToSeconds(timetoreach),
							G_TicsToCentiseconds(timetoreach));

						if (!mode)
						{
							if (stplyr->realtime > timetoreach)
							{
								splitflags = (splitflags &~ V_HUDTRANS)|V_HUDTRANSHALF;
								if (canplaysound)
								{
									S_StartSound(NULL, sfx_s3k72); //sfx_s26d); -- you STOLE fizzy lifting drinks
									canplaysound = false;
								}
							}
							else if (!canplaysound)
								canplaysound = true;
						}

						targettext[8] = 0;
					}
					break;
				default:
					goto bademblem;
			}

			V_DrawRightAlignedString(workx, worky, splitflags, targettext);
			workx -= 67;
			V_DrawSmallScaledPatch(workx + 4, worky, splitflags, W_CachePatchName("NEEDIT", PU_CACHE));

			break;

			bademblem:
			emblem = M_GetLevelEmblems(-1);
		}

		if (!mode)
			splitflags = (splitflags &~ V_HUDTRANSHALF)|V_HUDTRANS;
		while (curemb--)
		{
			workx -= 12;
			V_DrawSmallMappedPatch(workx + 4, worky, splitflags, emblempic[curemb], emblemcol[curemb]);
		}
	}
}

static void K_DrawKartPositionNum(INT32 num)
{
	// POSI_X = BASEVIDWIDTH - 51;	// 269
	// POSI_Y = BASEVIDHEIGHT- 64;	// 136

	boolean win = (stplyr->exiting && num == 1);
	//INT32 X = POSI_X;
	INT32 W = SHORT(kp_positionnum[0][0]->width);
	fixed_t scale = FRACUNIT;
	patch_t *localpatch = kp_positionnum[0][0];
	//INT32 splitflags = K_calcSplitFlags(V_SNAPTOBOTTOM|V_SNAPTORIGHT);
	INT32 fx = 0, fy = 0, fflags = 0;
	boolean flipdraw = false;	// flip the order we draw it in for MORE splitscreen bs. fun.
	boolean flipvdraw = false;	// used only for 2p splitscreen so overtaking doesn't make 1P's position fly off the screen.
	boolean overtake = false;

	if (stplyr->kartstuff[k_positiondelay] || stplyr->exiting)
	{
		scale *= 2;
		overtake = true;	// this is used for splitscreen stuff in conjunction with flipdraw.
	}
	if (splitscreen)
		scale /= 2;

	W = FixedMul(W<<FRACBITS, scale)>>FRACBITS;

	// pain and suffering defined below
	if (!splitscreen)
	{
		fx = POSI_X;
		fy = BASEVIDHEIGHT - 8;
		fflags = V_SNAPTOBOTTOM|V_SNAPTORIGHT;
	}
	else if (splitscreen == 1)	// for this splitscreen, we'll use case by case because it's a bit different.
	{
		fx = POSI_X;
		if (stplyr == &players[displayplayers[0]])	// for player 1: display this at the top right, above the minimap.
		{
			fy = 30;
			fflags = V_SNAPTOTOP|V_SNAPTORIGHT;
			if (overtake)
				flipvdraw = true;	// make sure overtaking doesn't explode us
		}
		else	// if we're not p1, that means we're p2. display this at the bottom right, below the minimap.
		{
			fy = BASEVIDHEIGHT - 8;
			fflags = V_SNAPTOBOTTOM|V_SNAPTORIGHT;
		}
	}
	else
	{
		if (stplyr == &players[displayplayers[0]] || stplyr == &players[displayplayers[2]])	// If we are P1 or P3...
		{
			fx = POSI_X;
			fy = POSI_Y;
			fflags = V_SNAPTOLEFT|((stplyr == &players[displayplayers[2]]) ? V_SPLITSCREEN|V_SNAPTOBOTTOM : 0);	// flip P3 to the bottom.
			flipdraw = true;
			if (num && num >= 10)
				fx += W;	// this seems dumb, but we need to do this in order for positions above 10 going off screen.
		}
		else // else, that means we're P2 or P4.
		{
			fx = POSI2_X;
			fy = POSI2_Y;
			fflags = V_SNAPTORIGHT|((stplyr == &players[displayplayers[3]]) ? V_SPLITSCREEN|V_SNAPTOBOTTOM : 0);	// flip P4 to the bottom
		}
	}

	// Special case for 0
	if (!num)
	{
		V_DrawFixedPatch(fx<<FRACBITS, fy<<FRACBITS, scale, V_HUDTRANSHALF|fflags, kp_positionnum[0][0], NULL);
		return;
	}

	I_Assert(num >= 0); // This function does not draw negative numbers

	// Draw the number
	while (num)
	{
		if (win) // 1st place winner? You get rainbows!!
			localpatch = kp_winnernum[(leveltime % (NUMWINFRAMES*3)) / 3];
		else if (stplyr->laps+1 >= cv_numlaps.value || stplyr->exiting) // Check for the final lap, or won
		{
			// Alternate frame every three frames
			switch (leveltime % 9)
			{
				case 1: case 2: case 3:
					if (K_IsPlayerLosing(stplyr))
						localpatch = kp_positionnum[num % 10][4];
					else
						localpatch = kp_positionnum[num % 10][1];
					break;
				case 4: case 5: case 6:
					if (K_IsPlayerLosing(stplyr))
						localpatch = kp_positionnum[num % 10][5];
					else
						localpatch = kp_positionnum[num % 10][2];
					break;
				case 7: case 8: case 9:
					if (K_IsPlayerLosing(stplyr))
						localpatch = kp_positionnum[num % 10][6];
					else
						localpatch = kp_positionnum[num % 10][3];
					break;
				default:
					localpatch = kp_positionnum[num % 10][0];
					break;
			}
		}
		else
			localpatch = kp_positionnum[num % 10][0];

		V_DrawFixedPatch((fx<<FRACBITS) + ((overtake && flipdraw) ? (SHORT(localpatch->width)*scale/2) : 0), (fy<<FRACBITS) + ((overtake && flipvdraw) ? (SHORT(localpatch->height)*scale/2) : 0), scale, V_HUDTRANSHALF|fflags, localpatch, NULL);
		// ^ if we overtake as p1 or p3 in splitscren, we shift it so that it doesn't go off screen.
		// ^ if we overtake as p1 in 2p splits, shift vertically so that this doesn't happen either.

		fx -= W;
		num /= 10;
	}
}

static boolean K_drawKartPositionFaces(void)
{
	// FACE_X = 15;				//  15
	// FACE_Y = 72;				//  72

	INT32 Y = FACE_Y+9; // +9 to offset where it's being drawn if there are more than one
	INT32 i, j, ranklines, strank = -1;
	boolean completed[MAXPLAYERS];
	INT32 rankplayer[MAXPLAYERS];
	INT32 bumperx, numplayersingame = 0;
	UINT8 *colormap;

	ranklines = 0;
	memset(completed, 0, sizeof (completed));
	memset(rankplayer, 0, sizeof (rankplayer));

	for (i = 0; i < MAXPLAYERS; i++)
	{
		rankplayer[i] = -1;

		if (!playeringame[i] || players[i].spectator || !players[i].mo)
			continue;

		numplayersingame++;
	}

	if (numplayersingame <= 1)
		return true;

#ifdef HAVE_BLUA
	if (!LUA_HudEnabled(hud_minirankings))
		return false;	// Don't proceed but still return true for free play above if HUD is disabled.
#endif

	for (j = 0; j < numplayersingame; j++)
	{
		UINT8 lowestposition = MAXPLAYERS+1;
		for (i = 0; i < MAXPLAYERS; i++)
		{
			if (completed[i] || !playeringame[i] || players[i].spectator || !players[i].mo)
				continue;

			if (players[i].kartstuff[k_position] >= lowestposition)
				continue;

			rankplayer[ranklines] = i;
			lowestposition = players[i].kartstuff[k_position];
		}

		i = rankplayer[ranklines];

		completed[i] = true;

		if (players+i == stplyr)
			strank = ranklines;

		//if (ranklines == 5)
			//break; // Only draw the top 5 players -- we do this a different way now...

		ranklines++;
	}

	if (ranklines < 5)
		Y -= (9*ranklines);
	else
		Y -= (9*5);

	if (G_BattleGametype() || strank <= 2) // too close to the top, or playing battle, or a spectator? would have had (strank == -1) called out, but already caught by (strank <= 2)
	{
		i = 0;
		if (ranklines > 5) // could be both...
			ranklines = 5;
	}
	else if (strank+3 > ranklines) // too close to the bottom?
	{
		i = ranklines - 5;
		if (i < 0)
			i = 0;
	}
	else
	{
		i = strank-2;
		ranklines = strank+3;
	}

	for (; i < ranklines; i++)
	{
		if (!playeringame[rankplayer[i]]) continue;
		if (players[rankplayer[i]].spectator) continue;
		if (!players[rankplayer[i]].mo) continue;

		bumperx = FACE_X+19;

		if (players[rankplayer[i]].mo->color)
		{
			colormap = R_GetTranslationColormap(players[rankplayer[i]].skin, players[rankplayer[i]].mo->color, GTC_CACHE);
			if (players[rankplayer[i]].mo->colorized)
				colormap = R_GetTranslationColormap(TC_RAINBOW, players[rankplayer[i]].mo->color, GTC_CACHE);
			else
				colormap = R_GetTranslationColormap(players[rankplayer[i]].skin, players[rankplayer[i]].mo->color, GTC_CACHE);

			V_DrawMappedPatch(FACE_X, Y, V_HUDTRANS|V_SNAPTOLEFT, facerankprefix[players[rankplayer[i]].skin], colormap);

#ifdef HAVE_BLUA
			if (LUA_HudEnabled(hud_battlebumpers))
			{
#endif
				if (G_BattleGametype() && players[rankplayer[i]].kartstuff[k_bumper] > 0)
				{
					V_DrawMappedPatch(bumperx-2, Y, V_HUDTRANS|V_SNAPTOLEFT, kp_tinybumper[0], colormap);
					for (j = 1; j < players[rankplayer[i]].kartstuff[k_bumper]; j++)
					{
						bumperx += 5;
						V_DrawMappedPatch(bumperx, Y, V_HUDTRANS|V_SNAPTOLEFT, kp_tinybumper[1], colormap);
					}
				}
#ifdef HAVE_BLUA
			}	// A new level of stupidity: checking if lua is enabled to close a bracket. :Fascinating:
#endif
		}

		if (i == strank)
			V_DrawScaledPatch(FACE_X, Y, V_HUDTRANS|V_SNAPTOLEFT, kp_facehighlight[(leveltime / 4) % 8]);

		if (G_BattleGametype() && players[rankplayer[i]].kartstuff[k_bumper] <= 0)
			V_DrawScaledPatch(FACE_X-4, Y-3, V_HUDTRANS|V_SNAPTOLEFT, kp_ranknobumpers);
		else
		{
			INT32 pos = players[rankplayer[i]].kartstuff[k_position];
			if (pos < 0 || pos > MAXPLAYERS)
				pos = 0;
			// Draws the little number over the face
			V_DrawScaledPatch(FACE_X-5, Y+10, V_HUDTRANS|V_SNAPTOLEFT, kp_facenum[pos]);
		}

		Y += 18;
	}

	return false;
}

//
// HU_DrawTabRankings -- moved here to take advantage of kart stuff!
//
void HU_DrawTabRankings(INT32 x, INT32 y, playersort_t *tab, INT32 scorelines, INT32 whiteplayer, INT32 hilicol)
{
	INT32 i, rightoffset = 240;
	const UINT8 *colormap;
	INT32 dupadjust = (vid.width/vid.dupx), duptweak = (dupadjust - BASEVIDWIDTH)/2;

	//this function is designed for 9 or less score lines only
	//I_Assert(scorelines <= 9); -- not today bitch, kart fixed it up

	V_DrawFill(1-duptweak, 26, dupadjust-2, 1, 0); // Draw a horizontal line because it looks nice!
	if (scorelines > 8)
	{
		V_DrawFill(160, 26, 1, 147, 0); // Draw a vertical line to separate the two sides.
		V_DrawFill(1-duptweak, 173, dupadjust-2, 1, 0); // And a horizontal line near the bottom.
		rightoffset = (BASEVIDWIDTH/2) - 4 - x;
	}

	for (i = 0; i < scorelines; i++)
	{
		char strtime[MAXPLAYERNAME+1];

		if (players[tab[i].num].spectator || !players[tab[i].num].mo)
			continue; //ignore them.

		if (netgame // don't draw it offline
		&& tab[i].num != serverplayer)
			HU_drawPing(x + ((i < 8) ? -17 : rightoffset + 11), y-4, playerpingtable[tab[i].num], 0);

		STRBUFCPY(strtime, tab[i].name);

		if (scorelines > 8)
			V_DrawThinString(x + 20, y, ((tab[i].num == whiteplayer) ? hilicol : 0)|V_ALLOWLOWERCASE|V_6WIDTHSPACE, strtime);
		else
			V_DrawString(x + 20, y, ((tab[i].num == whiteplayer) ? hilicol : 0)|V_ALLOWLOWERCASE, strtime);

		if (players[tab[i].num].mo->color)
		{
			colormap = R_GetTranslationColormap(players[tab[i].num].skin, players[tab[i].num].mo->color, GTC_CACHE);
			if (players[tab[i].num].mo->colorized)
				colormap = R_GetTranslationColormap(TC_RAINBOW, players[tab[i].num].mo->color, GTC_CACHE);
			else
				colormap = R_GetTranslationColormap(players[tab[i].num].skin, players[tab[i].num].mo->color, GTC_CACHE);

			V_DrawMappedPatch(x, y-4, 0, facerankprefix[players[tab[i].num].skin], colormap);
			/*if (G_BattleGametype() && players[tab[i].num].kartstuff[k_bumper] > 0) -- not enough space for this
			{
				INT32 bumperx = x+19;
				V_DrawMappedPatch(bumperx-2, y-4, 0, kp_tinybumper[0], colormap);
				for (j = 1; j < players[tab[i].num].kartstuff[k_bumper]; j++)
				{
					bumperx += 5;
					V_DrawMappedPatch(bumperx, y-4, 0, kp_tinybumper[1], colormap);
				}
			}*/
		}

		if (tab[i].num == whiteplayer)
			V_DrawScaledPatch(x, y-4, 0, kp_facehighlight[(leveltime / 4) % 8]);

		if (G_BattleGametype() && players[tab[i].num].kartstuff[k_bumper] <= 0)
			V_DrawScaledPatch(x-4, y-7, 0, kp_ranknobumpers);
		else
		{
			INT32 pos = players[tab[i].num].kartstuff[k_position];
			if (pos < 0 || pos > MAXPLAYERS)
				pos = 0;
			// Draws the little number over the face
			V_DrawScaledPatch(x-5, y+6, 0, kp_facenum[pos]);
		}

		if (G_RaceGametype())
		{
#define timestring(time) va("%i'%02i\"%02i", G_TicsToMinutes(time, true), G_TicsToSeconds(time), G_TicsToCentiseconds(time))
			if (scorelines > 8)
			{
				if (players[tab[i].num].exiting)
					V_DrawRightAlignedThinString(x+rightoffset, y-1, hilicol|V_6WIDTHSPACE, timestring(players[tab[i].num].realtime));
				else if (players[tab[i].num].pflags & PF_TIMEOVER)
					V_DrawRightAlignedThinString(x+rightoffset, y-1, V_6WIDTHSPACE, "NO CONTEST.");
				else if (circuitmap)
					V_DrawRightAlignedThinString(x+rightoffset, y-1, V_6WIDTHSPACE, va("Lap %d", tab[i].count));
			}
			else
			{
				if (players[tab[i].num].exiting)
					V_DrawRightAlignedString(x+rightoffset, y, hilicol, timestring(players[tab[i].num].realtime));
				else if (players[tab[i].num].pflags & PF_TIMEOVER)
					V_DrawRightAlignedThinString(x+rightoffset, y-1, 0, "NO CONTEST.");
				else if (circuitmap)
					V_DrawRightAlignedString(x+rightoffset, y, 0, va("Lap %d", tab[i].count));
			}
#undef timestring
		}
		else
			V_DrawRightAlignedString(x+rightoffset, y, 0, va("%u", tab[i].count));

		y += 18;
		if (i == 7)
		{
			y = 33;
			x = (BASEVIDWIDTH/2) + 4;
		}
	}
}

static void K_drawKartLaps(void)
{
	INT32 splitflags = K_calcSplitFlags(V_SNAPTOBOTTOM|V_SNAPTOLEFT);
	INT32 fx = 0, fy = 0, fflags = 0;	// stuff for 3p / 4p splitscreen.
	boolean flipstring = false;	// used for 3p or 4p
	INT32 stringw = 0;	// used with the above

	if (splitscreen > 1)
	{

		// pain and suffering defined below
		if (splitscreen < 2)	// don't change shit for THIS splitscreen.
		{
			fx = LAPS_X;
			fy = LAPS_Y;
			fflags = splitflags;
		}
		else
		{
			if (stplyr == &players[displayplayers[0]] || stplyr == &players[displayplayers[2]])	// If we are P1 or P3...
			{
				fx = LAPS_X;
				fy = LAPS_Y;
				fflags = V_SNAPTOLEFT|((stplyr == &players[displayplayers[2]]) ? V_SPLITSCREEN|V_SNAPTOBOTTOM : 0);	// flip P3 to the bottom.
			}
			else // else, that means we're P2 or P4.
			{
				fx = LAPS2_X;
				fy = LAPS2_Y;
				fflags = V_SNAPTORIGHT|((stplyr == &players[displayplayers[3]]) ? V_SPLITSCREEN|V_SNAPTOBOTTOM : 0);	// flip P4 to the bottom
				flipstring = true;	// make the string right aligned and other shit
			}
		}



		if (stplyr->exiting)	// draw stuff as god intended.
		{
			V_DrawScaledPatch(fx, fy, V_HUDTRANS|fflags, kp_splitlapflag);
			V_DrawString(fx+13, fy+1, V_HUDTRANS|fflags, "FIN");
		}
		else					// take flipstring into account here since we may have more laps than just 10
			if (flipstring)
			{
				stringw = V_StringWidth(va("%d/%d", stplyr->laps+1, cv_numlaps.value), 0);

				V_DrawScaledPatch(BASEVIDWIDTH-stringw-16, fy, V_HUDTRANS|fflags, kp_splitlapflag);
				V_DrawRightAlignedString(BASEVIDWIDTH-3, fy+1, V_HUDTRANS|fflags, va("%d/%d", stplyr->laps+1, cv_numlaps.value));
			}
			else	// draw stuff NORMALLY.
			{
				V_DrawScaledPatch(fx, fy, V_HUDTRANS|fflags, kp_splitlapflag);
				V_DrawString(fx+13, fy+1, V_HUDTRANS|fflags, va("%d/%d", stplyr->laps+1, cv_numlaps.value));
			}
	}
	else
	{
		V_DrawScaledPatch(LAPS_X, LAPS_Y, V_HUDTRANS|splitflags, kp_lapsticker);

		if (stplyr->exiting)
			V_DrawKartString(LAPS_X+33, LAPS_Y+3, V_HUDTRANS|splitflags, "FIN");
		else
			V_DrawKartString(LAPS_X+33, LAPS_Y+3, V_HUDTRANS|splitflags, va("%d/%d", stplyr->laps+1, cv_numlaps.value));
	}
}

static void K_drawKartSpeedometer(void)
{
	fixed_t convSpeed;
	INT32 splitflags = K_calcSplitFlags(V_SNAPTOBOTTOM|V_SNAPTOLEFT);

	if (cv_kartspeedometer.value == 1) // Kilometers
	{
		convSpeed = FixedDiv(FixedMul(stplyr->speed, 142371), mapobjectscale)/FRACUNIT; // 2.172409058
		V_DrawKartString(SPDM_X, SPDM_Y, V_HUDTRANS|splitflags, va("%3d km/h", convSpeed));
	}
	else if (cv_kartspeedometer.value == 2) // Miles
	{
		convSpeed = FixedDiv(FixedMul(stplyr->speed, 88465), mapobjectscale)/FRACUNIT; // 1.349868774
		V_DrawKartString(SPDM_X, SPDM_Y, V_HUDTRANS|splitflags, va("%3d mph", convSpeed));
	}
	else if (cv_kartspeedometer.value == 3) // Fracunits
	{
		convSpeed = FixedDiv(stplyr->speed, mapobjectscale)/FRACUNIT;
		V_DrawKartString(SPDM_X, SPDM_Y, V_HUDTRANS|splitflags, va("%3d fu/t", convSpeed));
	}
}

static void K_drawKartBumpersOrKarma(void)
{
	UINT8 *colormap = R_GetTranslationColormap(TC_DEFAULT, stplyr->skincolor, GTC_CACHE);
	INT32 splitflags = K_calcSplitFlags(V_SNAPTOBOTTOM|V_SNAPTOLEFT);
	INT32 fx = 0, fy = 0, fflags = 0;
	boolean flipstring = false;	// same as laps, used for splitscreen
	INT32 stringw = 0;	// used with the above

	if (splitscreen > 1)
	{

		// we will reuse lap coords here since it's essentially the same shit.

		if (stplyr == &players[displayplayers[0]] || stplyr == &players[displayplayers[2]])	// If we are P1 or P3...
		{
			fx = LAPS_X;
			fy = LAPS_Y;
			fflags = V_SNAPTOLEFT|((stplyr == &players[displayplayers[2]]) ? V_SPLITSCREEN|V_SNAPTOBOTTOM : 0);	// flip P3 to the bottom.
		}
		else // else, that means we're P2 or P4.
		{
			fx = LAPS2_X;
			fy = LAPS2_Y;
			fflags = V_SNAPTORIGHT|((stplyr == &players[displayplayers[3]]) ? V_SPLITSCREEN|V_SNAPTOBOTTOM : 0);	// flip P4 to the bottom
			flipstring = true;
		}

		if (stplyr->kartstuff[k_bumper] <= 0)
		{
			V_DrawMappedPatch(fx, fy-1, V_HUDTRANS|fflags, kp_splitkarmabomb, colormap);
			V_DrawString(fx+13, fy+1, V_HUDTRANS|fflags, va("%d/2", stplyr->kartstuff[k_comebackpoints]));
		}
		else	// the above doesn't need to account for weird stuff since the max amount of karma necessary is always 2 ^^^^
		{
			if (flipstring)	// for p2 and p4, assume we can have more than 10 bumpers. It's retarded but who knows.
			{
				stringw = V_StringWidth(va("%d/%d", stplyr->kartstuff[k_bumper], cv_kartbumpers.value), 0);

				V_DrawMappedPatch(BASEVIDWIDTH-stringw-16, fy-1, V_HUDTRANS|fflags, kp_rankbumper, colormap);
				V_DrawRightAlignedString(BASEVIDWIDTH-3, fy+1, V_HUDTRANS|fflags, va("%d/%d", stplyr->kartstuff[k_bumper], cv_kartbumpers.value));
			}
			else	// draw bumpers normally.
			{
				V_DrawMappedPatch(fx, fy-1, V_HUDTRANS|fflags, kp_rankbumper, colormap);
				V_DrawString(fx+13, fy+1, V_HUDTRANS|fflags, va("%d/%d", stplyr->kartstuff[k_bumper], cv_kartbumpers.value));
			}
		}
	}
	else
	{
		if (stplyr->kartstuff[k_bumper] <= 0)
		{
			V_DrawMappedPatch(LAPS_X, LAPS_Y, V_HUDTRANS|splitflags, kp_karmasticker, colormap);
			V_DrawKartString(LAPS_X+47, LAPS_Y+3, V_HUDTRANS|splitflags, va("%d/2", stplyr->kartstuff[k_comebackpoints]));
		}
		else
		{
			if (stplyr->kartstuff[k_bumper] > 9 && cv_kartbumpers.value > 9)
				V_DrawMappedPatch(LAPS_X, LAPS_Y, V_HUDTRANS|splitflags, kp_bumperstickerwide, colormap);
			else
				V_DrawMappedPatch(LAPS_X, LAPS_Y, V_HUDTRANS|splitflags, kp_bumpersticker, colormap);
			V_DrawKartString(LAPS_X+47, LAPS_Y+3, V_HUDTRANS|splitflags, va("%d/%d", stplyr->kartstuff[k_bumper], cv_kartbumpers.value));
		}
	}
}

static fixed_t K_FindCheckX(fixed_t px, fixed_t py, angle_t ang, fixed_t mx, fixed_t my)
{
	fixed_t dist, x;
	fixed_t range = RING_DIST/3;
	angle_t diff;

	range *= gamespeed+1;

	dist = abs(R_PointToDist2(px, py, mx, my));
	if (dist > range)
		return -320;

	diff = R_PointToAngle2(px, py, mx, my) - ang;

	if (diff < ANGLE_90 || diff > ANGLE_270)
		return -320;
	else
		x = (FixedMul(FINETANGENT(((diff+ANGLE_90)>>ANGLETOFINESHIFT) & 4095), 160<<FRACBITS) + (160<<FRACBITS))>>FRACBITS;

	if (encoremode)
		x = 320-x;

	if (splitscreen > 1)
		x /= 2;

	return x;
}

static void K_drawKartWanted(void)
{
	UINT8 i, numwanted = 0;
	UINT8 *colormap = NULL;
	INT32 basex = 0, basey = 0;

	if (stplyr != &players[displayplayers[0]])
		return;

	for (i = 0; i < 4; i++)
	{
		if (battlewanted[i] == -1)
			break;
		numwanted++;
	}

	if (numwanted <= 0)
		return;

	// set X/Y coords depending on splitscreen.
	if (splitscreen < 3)		// 1P and 2P use the same code.
	{
		basex = WANT_X;
		basey = WANT_Y;
		if (splitscreen == 2)
		{
			basey += 16;	// slight adjust for 3P
			basex -= 6;
		}
	}
	else if (splitscreen == 3)	// 4P splitscreen...
	{
		basex = BASEVIDWIDTH/2 - (SHORT(kp_wantedsplit->width)/2);	// center on screen
		basey = BASEVIDHEIGHT - 55;
		//basey2 = 4;
	}

	if (battlewanted[0] != -1)
		colormap = R_GetTranslationColormap(0, players[battlewanted[0]].skincolor, GTC_CACHE);
	V_DrawFixedPatch(basex<<FRACBITS, basey<<FRACBITS, FRACUNIT, V_HUDTRANS|(splitscreen < 3 ? V_SNAPTORIGHT : 0)|V_SNAPTOBOTTOM, (splitscreen > 1 ? kp_wantedsplit : kp_wanted), colormap);
	/*if (basey2)
		V_DrawFixedPatch(basex<<FRACBITS, basey2<<FRACBITS, FRACUNIT, V_HUDTRANS|V_SNAPTOTOP, (splitscreen == 3 ? kp_wantedsplit : kp_wanted), colormap);	// < used for 4p splits.*/

	for (i = 0; i < numwanted; i++)
	{
		INT32 x = basex+(splitscreen > 1 ? 13 : 8), y = basey+(splitscreen > 1 ? 16 : 21);
		fixed_t scale = FRACUNIT/2;
		player_t *p = &players[battlewanted[i]];

		if (battlewanted[i] == -1)
			break;

		if (numwanted == 1)
			scale = FRACUNIT;
		else
		{
			if (i & 1)
				x += 16;
			if (i > 1)
				y += 16;
		}

		if (players[battlewanted[i]].skincolor)
		{
			colormap = R_GetTranslationColormap(TC_RAINBOW, p->skincolor, GTC_CACHE);
			V_DrawFixedPatch(x<<FRACBITS, y<<FRACBITS, FRACUNIT, V_HUDTRANS|(splitscreen < 3 ? V_SNAPTORIGHT : 0)|V_SNAPTOBOTTOM, (scale == FRACUNIT ? facewantprefix[p->skin] : facerankprefix[p->skin]), colormap);
			/*if (basey2)	// again with 4p stuff
				V_DrawFixedPatch(x<<FRACBITS, (y - (basey-basey2))<<FRACBITS, FRACUNIT, V_HUDTRANS|V_SNAPTOTOP, (scale == FRACUNIT ? facewantprefix[p->skin] : facerankprefix[p->skin]), colormap);*/
		}
	}
}

static void K_drawKartPlayerCheck(void)
{
	INT32 i;
	UINT8 *colormap;
	INT32 x;

	INT32 splitflags = K_calcSplitFlags(V_SNAPTOBOTTOM);

	if (!stplyr->mo || stplyr->spectator)
		return;

	if (stplyr->awayviewtics)
		return;

	if (camspin[0])
		return;

	for (i = 0; i < MAXPLAYERS; i++)
	{
		UINT8 pnum = 0;

		if (&players[i] == stplyr)
			continue;
		if (!playeringame[i] || players[i].spectator)
			continue;
		if (!players[i].mo)
			continue;

		if ((players[i].kartstuff[k_invincibilitytimer] <= 0) && (leveltime & 2))
			pnum++; // white frames

		if (players[i].kartstuff[k_itemtype] == KITEM_GROW || players[i].kartstuff[k_growshrinktimer] > 0)
			pnum += 4;
		else if (players[i].kartstuff[k_itemtype] == KITEM_INVINCIBILITY || players[i].kartstuff[k_invincibilitytimer])
			pnum += 2;

		x = K_FindCheckX(stplyr->mo->x, stplyr->mo->y, stplyr->mo->angle, players[i].mo->x, players[i].mo->y);
		if (x <= 320 && x >= 0)
		{
			if (x < 14)
				x = 14;
			else if (x > 306)
				x = 306;

			colormap = R_GetTranslationColormap(TC_DEFAULT, players[i].mo->color, GTC_CACHE);
			V_DrawMappedPatch(x, CHEK_Y, V_HUDTRANS|splitflags, kp_check[pnum], colormap);
		}
	}
}

static void K_drawKartMinimapHead(mobj_t *mo, INT32 x, INT32 y, INT32 flags, patch_t *AutomapPic)
{
	// amnum xpos & ypos are the icon's speed around the HUD.
	// The number being divided by is for how fast it moves.
	// The higher the number, the slower it moves.

	// am xpos & ypos are the icon's starting position. Withouht
	// it, they wouldn't 'spawn' on the top-right side of the HUD.

	UINT8 skin = 0;

	fixed_t amnumxpos, amnumypos;
	INT32 amxpos, amypos;

	node_t *bsp = &nodes[numnodes-1];
	fixed_t maxx, minx, maxy, miny;

	fixed_t mapwidth, mapheight;
	fixed_t xoffset, yoffset;
	fixed_t xscale, yscale, zoom;

	if (mo->skin)
		skin = ((skin_t*)mo->skin)-skins;

	maxx = maxy = INT32_MAX;
	minx = miny = INT32_MIN;
	minx = bsp->bbox[0][BOXLEFT];
	maxx = bsp->bbox[0][BOXRIGHT];
	miny = bsp->bbox[0][BOXBOTTOM];
	maxy = bsp->bbox[0][BOXTOP];

	if (bsp->bbox[1][BOXLEFT] < minx)
		minx = bsp->bbox[1][BOXLEFT];
	if (bsp->bbox[1][BOXRIGHT] > maxx)
		maxx = bsp->bbox[1][BOXRIGHT];
	if (bsp->bbox[1][BOXBOTTOM] < miny)
		miny = bsp->bbox[1][BOXBOTTOM];
	if (bsp->bbox[1][BOXTOP] > maxy)
		maxy = bsp->bbox[1][BOXTOP];

	// You might be wondering why these are being bitshift here
	// it's because mapwidth and height would otherwise overflow for maps larger than half the size possible...
	// map boundaries and sizes will ALWAYS be whole numbers thankfully
	// later calculations take into consideration that these are actually not in terms of FRACUNIT though
	minx >>= FRACBITS;
	maxx >>= FRACBITS;
	miny >>= FRACBITS;
	maxy >>= FRACBITS;

	mapwidth = maxx - minx;
	mapheight = maxy - miny;

	// These should always be small enough to be bitshift back right now
	xoffset = (minx + mapwidth/2)<<FRACBITS;
	yoffset = (miny + mapheight/2)<<FRACBITS;

	xscale = FixedDiv(AutomapPic->width, mapwidth);
	yscale = FixedDiv(AutomapPic->height, mapheight);
	zoom = FixedMul(min(xscale, yscale), FRACUNIT-FRACUNIT/20);

	amnumxpos = (FixedMul(mo->x, zoom) - FixedMul(xoffset, zoom));
	amnumypos = -(FixedMul(mo->y, zoom) - FixedMul(yoffset, zoom));

	if (encoremode)
		amnumxpos = -amnumxpos;

	amxpos = amnumxpos + ((x + AutomapPic->width/2 - (facemmapprefix[skin]->width/2))<<FRACBITS);
	amypos = amnumypos + ((y + AutomapPic->height/2 - (facemmapprefix[skin]->height/2))<<FRACBITS);

	// do we want this? it feels unnecessary. easier to just modify the amnumxpos?
	/*if (encoremode)
	{
		flags |= V_FLIP;
		amxpos = -amnumxpos + ((x + AutomapPic->width/2 + (facemmapprefix[skin]->width/2))<<FRACBITS);
	}*/

	if (!mo->color) // 'default' color
		V_DrawSciencePatch(amxpos, amypos, flags, facemmapprefix[skin], FRACUNIT);
	else
	{
		UINT8 *colormap;
		if (mo->colorized)
			colormap = R_GetTranslationColormap(TC_RAINBOW, mo->color, GTC_CACHE);
		else
			colormap = R_GetTranslationColormap(skin, mo->color, GTC_CACHE);
		V_DrawFixedPatch(amxpos, amypos, FRACUNIT, flags, facemmapprefix[skin], colormap);
		if (mo->player
			&& ((G_RaceGametype() && mo->player->kartstuff[k_position] == spbplace)
			|| (G_BattleGametype() && K_IsPlayerWanted(mo->player))))
		{
			V_DrawFixedPatch(amxpos - (4<<FRACBITS), amypos - (4<<FRACBITS), FRACUNIT, flags, kp_wantedreticle, NULL);
		}
	}
}

static void K_drawKartMinimap(void)
{
	INT32 lumpnum;
	patch_t *AutomapPic;
	INT32 i = 0;
	INT32 x, y;
	INT32 minimaptrans, splitflags = (splitscreen == 3 ? 0 : V_SNAPTORIGHT);	// flags should only be 0 when it's centered (4p split)
	SINT8 localplayers[4];
	SINT8 numlocalplayers = 0;

	// Draw the HUD only when playing in a level.
	// hu_stuff needs this, unlike st_stuff.
	if (gamestate != GS_LEVEL)
		return;

	if (stplyr != &players[displayplayers[0]])
		return;

	lumpnum = W_CheckNumForName(va("%sR", G_BuildMapName(gamemap)));

	if (lumpnum != -1)
		AutomapPic = W_CachePatchName(va("%sR", G_BuildMapName(gamemap)), PU_HUDGFX);
	else
		return; // no pic, just get outta here

	x = MINI_X - (AutomapPic->width/2);
	y = MINI_Y - (AutomapPic->height/2);

	if (timeinmap > 105)
	{
		minimaptrans = cv_kartminimap.value;
		if (timeinmap <= 113)
			minimaptrans = ((((INT32)timeinmap) - 105)*minimaptrans)/(113-105);
		if (!minimaptrans)
			return;
	}
	else
		return;

	minimaptrans = ((10-minimaptrans)<<FF_TRANSSHIFT);
	splitflags |= minimaptrans;

	if (encoremode)
		V_DrawScaledPatch(x+(AutomapPic->width), y, splitflags|V_FLIP, AutomapPic);
	else
		V_DrawScaledPatch(x, y, splitflags, AutomapPic);

	if (!(splitscreen == 2))
	{
		splitflags &= ~minimaptrans;
		splitflags |= V_HUDTRANSHALF;
	}

	// let offsets transfer to the heads, too!
	if (encoremode)
		x += SHORT(AutomapPic->leftoffset);
	else
		x -= SHORT(AutomapPic->leftoffset);
	y -= SHORT(AutomapPic->topoffset);

	// initialize
	for (i = 0; i < 4; i++)
		localplayers[i] = -1;

	// Player's tiny icons on the Automap. (drawn opposite direction so player 1 is drawn last in splitscreen)
	if (ghosts)
	{
		demoghost *g = ghosts;
		while (g)
		{
			K_drawKartMinimapHead(g->mo, x, y, splitflags, AutomapPic);
			g = g->next;
		}

		if (!stplyr->mo || stplyr->spectator) // do we need the latter..?
			return;

		localplayers[numlocalplayers] = stplyr-players;
		numlocalplayers++;
	}
	else
	{
		for (i = MAXPLAYERS-1; i >= 0; i--)
		{
			if (!playeringame[i])
				continue;
			if (!players[i].mo || players[i].spectator)
				continue;

			if (i != displayplayers[0] || splitscreen)
			{
				if (G_BattleGametype() && players[i].kartstuff[k_bumper] <= 0)
					continue;

				if (players[i].kartstuff[k_hyudorotimer] > 0)
				{
					if (!((players[i].kartstuff[k_hyudorotimer] < 1*TICRATE/2
						|| players[i].kartstuff[k_hyudorotimer] > hyudorotime-(1*TICRATE/2))
						&& !(leveltime & 1)))
						continue;
				}
			}

			if (P_IsDisplayPlayer(&players[i]))
			{
				// Draw display players on top of everything else
				localplayers[numlocalplayers] = i;
				numlocalplayers++;
				continue;
			}

			K_drawKartMinimapHead(players[i].mo, x, y, splitflags, AutomapPic);
		}
	}

	// draw our local players here, opaque.
	splitflags &= ~V_HUDTRANSHALF;
	splitflags |= V_HUDTRANS;

	for (i = 0; i < numlocalplayers; i++)
	{
		if (i == -1)
			continue; // this doesn't interest us
		K_drawKartMinimapHead(players[localplayers[i]].mo, x, y, splitflags, AutomapPic);
	}
}

static void K_drawKartStartCountdown(void)
{
	INT32 pnum = 0, splitflags = K_calcSplitFlags(0); // 3

	if (leveltime >= starttime-(2*TICRATE)) // 2
		pnum++;
	if (leveltime >= starttime-TICRATE) // 1
		pnum++;
	if (leveltime >= starttime) // GO!
		pnum++;
	if ((leveltime % (2*5)) / 5) // blink
		pnum += 4;
	if (splitscreen) // splitscreen
		pnum += 8;

	V_DrawScaledPatch(STCD_X - (SHORT(kp_startcountdown[pnum]->width)/2), STCD_Y - (SHORT(kp_startcountdown[pnum]->height)/2), splitflags, kp_startcountdown[pnum]);
}

static void K_drawKartFinish(void)
{
	INT32 pnum = 0, splitflags = K_calcSplitFlags(0);

	if (!stplyr->kartstuff[k_cardanimation] || stplyr->kartstuff[k_cardanimation] >= 2*TICRATE)
		return;

	if ((stplyr->kartstuff[k_cardanimation] % (2*5)) / 5) // blink
		pnum = 1;

	if (splitscreen > 1) // 3/4p, stationary FIN
	{
		pnum += 2;
		V_DrawScaledPatch(STCD_X - (SHORT(kp_racefinish[pnum]->width)/2), STCD_Y - (SHORT(kp_racefinish[pnum]->height)/2), splitflags, kp_racefinish[pnum]);
		return;
	}

	//else -- 1/2p, scrolling FINISH
	{
		INT32 x, xval;

		if (splitscreen) // wide splitscreen
			pnum += 4;

		x = ((vid.width<<FRACBITS)/vid.dupx);
		xval = (SHORT(kp_racefinish[pnum]->width)<<FRACBITS);
		x = ((TICRATE - stplyr->kartstuff[k_cardanimation])*(xval > x ? xval : x))/TICRATE;

		if (splitscreen && stplyr == &players[displayplayers[1]])
			x = -x;

		V_DrawFixedPatch(x + (STCD_X<<FRACBITS) - (xval>>1),
			(STCD_Y<<FRACBITS) - (SHORT(kp_racefinish[pnum]->height)<<(FRACBITS-1)),
			FRACUNIT,
			splitflags, kp_racefinish[pnum], NULL);
	}
}

static void K_drawBattleFullscreen(void)
{
	INT32 x = BASEVIDWIDTH/2;
	INT32 y = -64+(stplyr->kartstuff[k_cardanimation]); // card animation goes from 0 to 164, 164 is the middle of the screen
	INT32 splitflags = V_SNAPTOTOP; // I don't feel like properly supporting non-green resolutions, so you can have a misuse of SNAPTO instead
	fixed_t scale = FRACUNIT;
	boolean drawcomebacktimer = true;	// lazy hack because it's cleaner in the long run.
#ifdef HAVE_BLUA
	if (!LUA_HudEnabled(hud_battlecomebacktimer))
		drawcomebacktimer = false;
#endif

	if (splitscreen)
	{
		if ((splitscreen == 1 && stplyr == &players[displayplayers[1]])
			|| (splitscreen > 1 && (stplyr == &players[displayplayers[2]]
			|| (stplyr == &players[displayplayers[3]] && splitscreen > 2))))
		{
			y = 232-(stplyr->kartstuff[k_cardanimation]/2);
			splitflags = V_SNAPTOBOTTOM;
		}
		else
			y = -32+(stplyr->kartstuff[k_cardanimation]/2);

		if (splitscreen > 1)
		{
			scale /= 2;

			if (stplyr == &players[displayplayers[1]]
				|| (stplyr == &players[displayplayers[3]] && splitscreen > 2))
				x = 3*BASEVIDWIDTH/4;
			else
				x = BASEVIDWIDTH/4;
		}
		else
		{
			if (stplyr->exiting)
			{
				if (stplyr == &players[displayplayers[1]])
					x = BASEVIDWIDTH-96;
				else
					x = 96;
			}
			else
				scale /= 2;
		}
	}

	if (stplyr->exiting)
	{
		if (stplyr == &players[displayplayers[0]])
			V_DrawFadeScreen(0xFF00, 16);
		if (stplyr->exiting < 6*TICRATE && !stplyr->spectator)
		{
			if (stplyr->kartstuff[k_position] == 1)
				V_DrawFixedPatch(x<<FRACBITS, y<<FRACBITS, scale, splitflags, kp_battlewin, NULL);
			else
				V_DrawFixedPatch(x<<FRACBITS, y<<FRACBITS, scale, splitflags, (K_IsPlayerLosing(stplyr) ? kp_battlelose : kp_battlecool), NULL);
		}
		else
			K_drawKartFinish();
	}
	else if (stplyr->kartstuff[k_bumper] <= 0 && stplyr->kartstuff[k_comebacktimer] && comeback && !stplyr->spectator && drawcomebacktimer)
	{
		UINT16 t = stplyr->kartstuff[k_comebacktimer]/(10*TICRATE);
		INT32 txoff, adjust = (splitscreen > 1) ? 4 : 6; // normal string is 8, kart string is 12, half of that for ease
		INT32 ty = (BASEVIDHEIGHT/2)+66;

		txoff = adjust;

		while (t)
		{
			txoff += adjust;
			t /= 10;
		}

		if (splitscreen)
		{
			if (splitscreen > 1)
				ty = (BASEVIDHEIGHT/4)+33;
			if ((splitscreen == 1 && stplyr == &players[displayplayers[1]])
				|| (stplyr == &players[displayplayers[2]] && splitscreen > 1)
				|| (stplyr == &players[displayplayers[3]] && splitscreen > 2))
				ty += (BASEVIDHEIGHT/2);
		}
		else
			V_DrawFadeScreen(0xFF00, 16);

		if (!comebackshowninfo)
			V_DrawFixedPatch(x<<FRACBITS, y<<FRACBITS, scale, splitflags, kp_battleinfo, NULL);
		else
			V_DrawFixedPatch(x<<FRACBITS, y<<FRACBITS, scale, splitflags, kp_battlewait, NULL);

		if (splitscreen > 1)
			V_DrawString(x-txoff, ty, 0, va("%d", stplyr->kartstuff[k_comebacktimer]/TICRATE));
		else
		{
			V_DrawFixedPatch(x<<FRACBITS, ty<<FRACBITS, scale, 0, kp_timeoutsticker, NULL);
			V_DrawKartString(x-txoff, ty, 0, va("%d", stplyr->kartstuff[k_comebacktimer]/TICRATE));
		}
	}

	if (netgame && !stplyr->spectator && timeinmap > 113) // FREE PLAY?
	{
		UINT8 i;

		// check to see if there's anyone else at all
		for (i = 0; i < MAXPLAYERS; i++)
		{
			if (i == displayplayers[0])
				continue;
			if (playeringame[i] && !stplyr->spectator)
				return;
		}

#ifdef HAVE_BLUA
		if (LUA_HudEnabled(hud_freeplay))
#endif
			K_drawKartFreePlay(leveltime);
	}
}

static void K_drawKartFirstPerson(void)
{
	static INT32 pnum[4], turn[4], drift[4];
	INT32 pn = 0, tn = 0, dr = 0;
	INT32 target = 0, splitflags = K_calcSplitFlags(V_SNAPTOBOTTOM);
	INT32 x = BASEVIDWIDTH/2, y = BASEVIDHEIGHT;
	fixed_t scale;
	UINT8 *colmap = NULL;
	ticcmd_t *cmd = &stplyr->cmd;

	if (stplyr->spectator || !stplyr->mo || (stplyr->mo->flags2 & MF2_DONTDRAW))
		return;

	if (stplyr == &players[displayplayers[1]] && splitscreen)
		{ pn = pnum[1]; tn = turn[1]; dr = drift[1]; }
	else if (stplyr == &players[displayplayers[2]] && splitscreen > 1)
		{ pn = pnum[2]; tn = turn[2]; dr = drift[2]; }
	else if (stplyr == &players[displayplayers[3]] && splitscreen > 2)
		{ pn = pnum[3]; tn = turn[3]; dr = drift[3]; }
	else
		{ pn = pnum[0]; tn = turn[0]; dr = drift[0]; }

	if (splitscreen)
	{
		y >>= 1;
		if (splitscreen > 1)
			x >>= 1;
	}

	{
		if (stplyr->speed < (20*stplyr->mo->scale) && (leveltime & 1) && !splitscreen)
			y++;
		// the following isn't EXPLICITLY right, it just gets the result we want, but i'm too lazy to look up the right way to do it
		if (stplyr->mo->flags2 & MF2_SHADOW)
			splitflags |= FF_TRANS80;
		else if (stplyr->mo->frame & FF_TRANSMASK)
			splitflags |= (stplyr->mo->frame & FF_TRANSMASK);
	}

	if (cmd->driftturn > 400) // strong left turn
		target = 2;
	else if (cmd->driftturn < -400) // strong right turn
		target = -2;
	else if (cmd->driftturn > 0) // weak left turn
		target = 1;
	else if (cmd->driftturn < 0) // weak right turn
		target = -1;
	else // forward
		target = 0;

	if (encoremode)
		target = -target;

	if (pn < target)
		pn++;
	else if (pn > target)
		pn--;

	if (pn < 0)
		splitflags |= V_FLIP; // right turn

	target = abs(pn);
	if (target > 2)
		target = 2;

	x <<= FRACBITS;
	y <<= FRACBITS;

	if (tn != cmd->driftturn/50)
		tn -= (tn - (cmd->driftturn/50))/8;

	if (dr != stplyr->kartstuff[k_drift]*16)
		dr -= (dr - (stplyr->kartstuff[k_drift]*16))/8;

	if (splitscreen == 1)
	{
		scale = (2*FRACUNIT)/3;
		y += FRACUNIT/(vid.dupx < vid.dupy ? vid.dupx : vid.dupy); // correct a one-pixel gap on the screen view (not the basevid view)
	}
	else if (splitscreen)
		scale = FRACUNIT/2;
	else
		scale = FRACUNIT;

	if (stplyr->mo)
	{
		INT32 dsone = K_GetKartDriftSparkValue(stplyr);
		INT32 dstwo = dsone*2;
		INT32 dsthree = dstwo*2;

#ifndef DONTLIKETOASTERSFPTWEAKS
		{
			const angle_t ang = R_PointToAngle2(0, 0, stplyr->rmomx, stplyr->rmomy) - stplyr->frameangle;
			// yes, the following is correct. no, you do not need to swap the x and y.
			fixed_t xoffs = -P_ReturnThrustY(stplyr->mo, ang, (BASEVIDWIDTH<<(FRACBITS-2))/2);
			fixed_t yoffs = -(P_ReturnThrustX(stplyr->mo, ang, 4*FRACUNIT) - 4*FRACUNIT);

			if (splitscreen)
				xoffs = FixedMul(xoffs, scale);

			xoffs -= (tn)*scale;
			xoffs -= (dr)*scale;

			if (stplyr->frameangle == stplyr->mo->angle)
			{
				const fixed_t mag = FixedDiv(stplyr->speed, 10*stplyr->mo->scale);

				if (mag < FRACUNIT)
				{
					xoffs = FixedMul(xoffs, mag);
					if (!splitscreen)
						yoffs = FixedMul(yoffs, mag);
				}
			}

			if (stplyr->mo->momz > 0) // TO-DO: Draw more of the kart so we can remove this if!
				yoffs += stplyr->mo->momz/3;

			if (encoremode)
				x -= xoffs;
			else
				x += xoffs;
			if (!splitscreen)
				y += yoffs;
		}

		// drift sparks!
		if ((leveltime & 1) && (stplyr->kartstuff[k_driftcharge] >= dsthree))
			colmap = R_GetTranslationColormap(TC_RAINBOW, (UINT8)(1 + (leveltime % (MAXSKINCOLORS-1))), GTC_CACHE);
		else if ((leveltime & 1) && (stplyr->kartstuff[k_driftcharge] >= dstwo))
			colmap = R_GetTranslationColormap(TC_RAINBOW, SKINCOLOR_KETCHUP, GTC_CACHE);
		else if ((leveltime & 1) && (stplyr->kartstuff[k_driftcharge] >= dsone))
			colmap = R_GetTranslationColormap(TC_RAINBOW, SKINCOLOR_SAPPHIRE, GTC_CACHE);
		else
#endif
		// invincibility/grow/shrink!
		if (stplyr->mo->colorized && stplyr->mo->color)
			colmap = R_GetTranslationColormap(TC_RAINBOW, stplyr->mo->color, GTC_CACHE);
	}

	V_DrawFixedPatch(x, y, scale, splitflags, kp_fpview[target], colmap);

	if (stplyr == &players[displayplayers[1]] && splitscreen)
		{ pnum[1] = pn; turn[1] = tn; drift[1] = dr; }
	else if (stplyr == &players[displayplayers[2]] && splitscreen > 1)
		{ pnum[2] = pn; turn[2] = tn; drift[2] = dr; }
	else if (stplyr == &players[displayplayers[3]] && splitscreen > 2)
		{ pnum[3] = pn; turn[3] = tn; drift[3] = dr; }
	else
		{ pnum[0] = pn; turn[0] = tn; drift[0] = dr; }
}

// doesn't need to ever support 4p
static void K_drawInput(void)
{
	static INT32 pn = 0;
	INT32 target = 0, splitflags = (V_SNAPTOBOTTOM|V_SNAPTORIGHT);
	INT32 x = BASEVIDWIDTH - 32, y = BASEVIDHEIGHT-24, offs, col;
	const INT32 accent1 = splitflags|colortranslations[stplyr->skincolor][5];
	const INT32 accent2 = splitflags|colortranslations[stplyr->skincolor][9];
	ticcmd_t *cmd = &stplyr->cmd;

	if (timeinmap <= 105)
		return;

	if (timeinmap < 113)
	{
		INT32 count = ((INT32)(timeinmap) - 105);
		offs = 64;
		while (count-- > 0)
			offs >>= 1;
		x += offs;
	}

#define BUTTW 8
#define BUTTH 11

#define drawbutt(xoffs, butt, symb)\
	if (stplyr->cmd.buttons & butt)\
	{\
		offs = 2;\
		col = accent1;\
	}\
	else\
	{\
		offs = 0;\
		col = accent2;\
		V_DrawFill(x+(xoffs), y+BUTTH, BUTTW-1, 2, splitflags|31);\
	}\
	V_DrawFill(x+(xoffs), y+offs, BUTTW-1, BUTTH, col);\
	V_DrawFixedPatch((x+1+(xoffs))<<FRACBITS, (y+offs+1)<<FRACBITS, FRACUNIT, splitflags, tny_font[symb-HU_FONTSTART], NULL)

	drawbutt(-2*BUTTW, BT_ACCELERATE, 'A');
	drawbutt(  -BUTTW, BT_BRAKE,      'B');
	drawbutt(       0, BT_DRIFT,      'D');
	drawbutt(   BUTTW, BT_ATTACK,     'I');

#undef drawbutt

#undef BUTTW
#undef BUTTH

	y -= 1;

	if (!cmd->driftturn) // no turn
		target = 0;
	else // turning of multiple strengths!
	{
		target = ((abs(cmd->driftturn) - 1)/125)+1;
		if (target > 4)
			target = 4;
		if (cmd->driftturn < 0)
			target = -target;
	}

	if (pn != target)
	{
		if (abs(pn - target) == 1)
			pn = target;
		else if (pn < target)
			pn += 2;
		else //if (pn > target)
			pn -= 2;
	}

	if (pn < 0)
	{
		splitflags |= V_FLIP; // right turn
		x--;
	}

	target = abs(pn);
	if (target > 4)
		target = 4;

	if (!stplyr->skincolor)
		V_DrawFixedPatch(x<<FRACBITS, y<<FRACBITS, FRACUNIT, splitflags, kp_inputwheel[target], NULL);
	else
	{
		UINT8 *colormap;
		colormap = R_GetTranslationColormap(0, stplyr->skincolor, GTC_CACHE);
		V_DrawFixedPatch(x<<FRACBITS, y<<FRACBITS, FRACUNIT, splitflags, kp_inputwheel[target], colormap);
	}
}

static void K_drawChallengerScreen(void)
{
	// This is an insanely complicated animation.
	static UINT8 anim[52] = {
		0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13, // frame 1-14, 2 tics: HERE COMES A NEW slides in
		14,14,14,14,14,14, // frame 15, 6 tics: pause on the W
		15,16,17,18, // frame 16-19, 1 tic: CHALLENGER approaches screen
		19,20,19,20,19,20,19,20,19,20, // frame 20-21, 1 tic, 5 alternating: all text vibrates from impact
		21,22,23,24 // frame 22-25, 1 tic: CHALLENGER turns gold
	};
	const UINT8 offset = min(52-1, (3*TICRATE)-mapreset);

	V_DrawFadeScreen(0xFF00, 16); // Fade out
	V_DrawScaledPatch(0, 0, 0, kp_challenger[anim[offset]]);
}

static void K_drawLapStartAnim(void)
{
	// This is an EVEN MORE insanely complicated animation.
	const UINT8 progress = 80-stplyr->kartstuff[k_lapanimation];
	UINT8 *colormap = R_GetTranslationColormap(TC_DEFAULT, stplyr->skincolor, GTC_CACHE);

	V_DrawFixedPatch((BASEVIDWIDTH/2 + (32*max(0, stplyr->kartstuff[k_lapanimation]-76)))*FRACUNIT,
		(48 - (32*max(0, progress-76)))*FRACUNIT,
		FRACUNIT, V_SNAPTOTOP|V_HUDTRANS,
		(modeattacking ? kp_lapanim_emblem[1] : kp_lapanim_emblem[0]), colormap);

	if (stplyr->kartstuff[k_laphand] >= 1 && stplyr->kartstuff[k_laphand] <= 3)
	{
		V_DrawFixedPatch((BASEVIDWIDTH/2 + (32*max(0, stplyr->kartstuff[k_lapanimation]-76)))*FRACUNIT,
			(48 - (32*max(0, progress-76))
				+ 4 - abs((signed)((leveltime % 8) - 4)))*FRACUNIT,
			FRACUNIT, V_SNAPTOTOP|V_HUDTRANS,
			kp_lapanim_hand[stplyr->kartstuff[k_laphand]-1], NULL);
	}

	if (stplyr->laps == (UINT8)(cv_numlaps.value - 1))
	{
		V_DrawFixedPatch((62 - (32*max(0, progress-76)))*FRACUNIT, // 27
			30*FRACUNIT, // 24
			FRACUNIT, V_SNAPTOTOP|V_HUDTRANS,
			kp_lapanim_final[min(progress/2, 10)], NULL);

		if (progress/2-12 >= 0)
		{
			V_DrawFixedPatch((188 + (32*max(0, progress-76)))*FRACUNIT, // 194
				30*FRACUNIT, // 24
				FRACUNIT, V_SNAPTOTOP|V_HUDTRANS,
				kp_lapanim_lap[min(progress/2-12, 6)], NULL);
		}
	}
	else
	{
		V_DrawFixedPatch((82 - (32*max(0, progress-76)))*FRACUNIT, // 61
			30*FRACUNIT, // 24
			FRACUNIT, V_SNAPTOTOP|V_HUDTRANS,
			kp_lapanim_lap[min(progress/2, 6)], NULL);

		if (progress/2-8 >= 0)
		{
			V_DrawFixedPatch((188 + (32*max(0, progress-76)))*FRACUNIT, // 194
				30*FRACUNIT, // 24
				FRACUNIT, V_SNAPTOTOP|V_HUDTRANS,
				kp_lapanim_number[(((UINT32)stplyr->laps+1) / 10)][min(progress/2-8, 2)], NULL);

			if (progress/2-10 >= 0)
			{
				V_DrawFixedPatch((208 + (32*max(0, progress-76)))*FRACUNIT, // 221
					30*FRACUNIT, // 24
					FRACUNIT, V_SNAPTOTOP|V_HUDTRANS,
					kp_lapanim_number[(((UINT32)stplyr->laps+1) % 10)][min(progress/2-10, 2)], NULL);
			}
		}
	}
}

void K_drawKartFreePlay(UINT32 flashtime)
{
	// no splitscreen support because it's not FREE PLAY if you have more than one player in-game

	if ((flashtime % TICRATE) < TICRATE/2)
		return;

	V_DrawKartString((BASEVIDWIDTH - (LAPS_X+1)) - (12*9), // mirror the laps thingy
		LAPS_Y+3, V_SNAPTOBOTTOM|V_SNAPTORIGHT|V_HUDTRANS, "FREE PLAY");
}

static void K_drawDistributionDebugger(void)
{
	patch_t *items[NUMKARTRESULTS] = {
		kp_sadface[1],
		kp_sneaker[1],
		kp_rocketsneaker[1],
		kp_invincibility[7],
		kp_banana[1],
		kp_eggman[1],
		kp_orbinaut[4],
		kp_jawz[1],
		kp_mine[1],
		kp_ballhog[1],
		kp_selfpropelledbomb[1],
		kp_grow[1],
		kp_shrink[1],
		kp_thundershield[1],
		kp_hyudoro[1],
		kp_pogospring[1],
		kp_kitchensink[1],

		kp_sneaker[1],
		kp_banana[1],
		kp_banana[1],
		kp_orbinaut[4],
		kp_orbinaut[4],
		kp_jawz[1]
	};
	INT32 useodds = 0;
	INT32 pingame = 0, bestbumper = 0;
	INT32 i;
	INT32 x = -9, y = -9;
	boolean dontforcespb = false;
	boolean spbrush = false;

	if (stplyr != &players[displayplayers[0]]) // only for p1
		return;

	// The only code duplication from the Kart, just to avoid the actual item function from calculating pingame twice
	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (!playeringame[i] || players[i].spectator)
			continue;
		pingame++;
		if (players[i].exiting)
			dontforcespb = true;
		if (players[i].kartstuff[k_bumper] > bestbumper)
			bestbumper = players[i].kartstuff[k_bumper];
	}

	if (G_RaceGametype())
		spbrush = (spbplace != -1 && stplyr->kartstuff[k_position] == spbplace+1);

	useodds = K_FindUseodds(stplyr, 0, pingame, bestbumper, spbrush, dontforcespb);

	for (i = 1; i < NUMKARTRESULTS; i++)
	{
		const INT32 itemodds = K_KartGetItemOdds(useodds, i, 0, spbrush);
		if (itemodds <= 0)
			continue;

		V_DrawScaledPatch(x, y, V_HUDTRANS|V_SNAPTOTOP, items[i]);
		V_DrawThinString(x+11, y+31, V_HUDTRANS|V_SNAPTOTOP, va("%d", itemodds));

		// Display amount for multi-items
		if (i >= NUMKARTITEMS)
		{
			INT32 amount;
			switch (i)
			{
				case KRITEM_TENFOLDBANANA:
					amount = 10;
					break;
				case KRITEM_QUADORBINAUT:
					amount = 4;
					break;
				case KRITEM_DUALJAWZ:
					amount = 2;
					break;
				default:
					amount = 3;
					break;
			}
			V_DrawString(x+24, y+31, V_ALLOWLOWERCASE|V_HUDTRANS|V_SNAPTOTOP, va("x%d", amount));
		}

		x += 32;
		if (x >= 297)
		{
			x = -9;
			y += 32;
		}
	}

	V_DrawString(0, 0, V_HUDTRANS|V_SNAPTOTOP, va("USEODDS %d", useodds));
}

static void K_drawCheckpointDebugger(void)
{
	if (stplyr != &players[displayplayers[0]]) // only for p1
		return;

	if (stplyr->starpostnum >= (numstarposts - (numstarposts/2)))
		V_DrawString(8, 184, 0, va("Checkpoint: %d / %d (Can finish)", stplyr->starpostnum, numstarposts));
	else
		V_DrawString(8, 184, 0, va("Checkpoint: %d / %d (Skip: %d)", stplyr->starpostnum, numstarposts, ((numstarposts/2) + stplyr->starpostnum)));
	V_DrawString(8, 192, 0, va("Waypoint dist: Prev %d, Next %d", stplyr->kartstuff[k_prevcheck], stplyr->kartstuff[k_nextcheck]));
}

void K_drawKartHUD(void)
{
	boolean isfreeplay = false;
	boolean battlefullscreen = false;
	boolean freecam = demo.freecam;	//disable some hud elements w/ freecam
	UINT8 i;

	// Define the X and Y for each drawn object
	// This is handled by console/menu values
	K_initKartHUD();

	// Draw that fun first person HUD! Drawn ASAP so it looks more "real".
	for (i = 0; i <= splitscreen; i++)
	{
		if (stplyr == &players[displayplayers[i]] && !camera[i].chase && !freecam)
			K_drawKartFirstPerson();
	}

	// Draw full screen stuff that turns off the rest of the HUD
	if (mapreset && stplyr == &players[displayplayers[0]])
	{
		K_drawChallengerScreen();
		return;
	}

	battlefullscreen = ((G_BattleGametype())
		&& (stplyr->exiting
		|| (stplyr->kartstuff[k_bumper] <= 0
		&& stplyr->kartstuff[k_comebacktimer]
		&& comeback
		&& stplyr->playerstate == PST_LIVE)));

	if (!demo.title && (!battlefullscreen || splitscreen))
	{
		// Draw the CHECK indicator before the other items, so it's overlapped by everything else
#ifdef HAVE_BLUA
		if (LUA_HudEnabled(hud_check))	// delete lua when?
#endif
			if (cv_kartcheck.value && !splitscreen && !players[displayplayers[0]].exiting && !freecam)
				K_drawKartPlayerCheck();

		// Draw WANTED status
		if (G_BattleGametype())
		{
#ifdef HAVE_BLUA
			if (LUA_HudEnabled(hud_wanted))
#endif
				K_drawKartWanted();
		}

		if (cv_kartminimap.value)
		{
#ifdef HAVE_BLUA
			if (LUA_HudEnabled(hud_minimap))
#endif
				K_drawKartMinimap();
		}
	}

	if (battlefullscreen && !freecam)
	{
#ifdef HAVE_BLUA
		if (LUA_HudEnabled(hud_battlefullscreen))
#endif
			K_drawBattleFullscreen();
		return;
	}

	// Draw the item window
#ifdef HAVE_BLUA
	if (LUA_HudEnabled(hud_item) && !freecam)
#endif
		K_drawKartItem();

	// If not splitscreen, draw...
	if (!splitscreen && !demo.title)
	{
		// Draw the timestamp
#ifdef HAVE_BLUA
		if (LUA_HudEnabled(hud_time))
#endif
			K_drawKartTimestamp(stplyr->realtime, TIME_X, TIME_Y, gamemap, 0);

		if (!modeattacking)
		{
			// The top-four faces on the left
			/*#ifdef HAVE_BLUA
			if (LUA_HudEnabled(hud_minirankings))
			#endif*/
				isfreeplay = K_drawKartPositionFaces();
		}
	}

	if (!stplyr->spectator && !demo.freecam) // Bottom of the screen elements, don't need in spectate mode
	{
		if (demo.title) // Draw title logo instead in demo.titles
		{
			INT32 x = BASEVIDWIDTH - 32, y = 128, offs;

			if (splitscreen == 3)
			{
				x = BASEVIDWIDTH/2 + 10;
				y = BASEVIDHEIGHT/2 - 30;
			}

			if (timeinmap < 113)
			{
				INT32 count = ((INT32)(timeinmap) - 104);
				offs = 256;
				while (count-- > 0)
					offs >>= 1;
				x += offs;
			}

			V_DrawTinyScaledPatch(x-54, y, 0, W_CachePatchName("TTKBANNR", PU_CACHE));
			V_DrawTinyScaledPatch(x-54, y+25, 0, W_CachePatchName("TTKART", PU_CACHE));
		}
		else if (G_RaceGametype()) // Race-only elements
		{
			// Draw the lap counter
#ifdef HAVE_BLUA
			if (LUA_HudEnabled(hud_gametypeinfo))
#endif
				K_drawKartLaps();

			if (!splitscreen)
			{
				// Draw the speedometer
				// TODO: Make a better speedometer.
#ifdef HAVE_BLUA
			if (LUA_HudEnabled(hud_speedometer))
#endif
				K_drawKartSpeedometer();
			}

			if (isfreeplay)
				;
			else if (!modeattacking)
			{
				// Draw the numerical position
#ifdef HAVE_BLUA
				if (LUA_HudEnabled(hud_position))
#endif
					K_DrawKartPositionNum(stplyr->kartstuff[k_position]);
			}
			else //if (!(demo.playback && hu_showscores))
			{
				// Draw the input UI
#ifdef HAVE_BLUA
				if (LUA_HudEnabled(hud_position))
#endif
					K_drawInput();
			}
		}
		else if (G_BattleGametype()) // Battle-only
		{
			// Draw the hits left!
#ifdef HAVE_BLUA
			if (LUA_HudEnabled(hud_gametypeinfo))
#endif
				K_drawKartBumpersOrKarma();
		}
	}

	// Draw the countdowns after everything else.
	if (leveltime >= starttime-(3*TICRATE)
		&& leveltime < starttime+TICRATE)
		K_drawKartStartCountdown();
	else if (racecountdown && (!splitscreen || !stplyr->exiting))
	{
		char *countstr = va("%d", racecountdown/TICRATE);

		if (splitscreen > 1)
			V_DrawCenteredString(BASEVIDWIDTH/4, LAPS_Y+1, K_calcSplitFlags(0), countstr);
		else
		{
			INT32 karlen = strlen(countstr)*6; // half of 12
			V_DrawKartString((BASEVIDWIDTH/2)-karlen, LAPS_Y+3, K_calcSplitFlags(0), countstr);
		}
	}

	// Race overlays
	if (G_RaceGametype() && !freecam)
	{
		if (stplyr->exiting)
			K_drawKartFinish();
		else if (stplyr->kartstuff[k_lapanimation] && !splitscreen)
			K_drawLapStartAnim();
	}

	if (modeattacking || freecam) // everything after here is MP and debug only
		return;

	if (G_BattleGametype() && !splitscreen && (stplyr->kartstuff[k_yougotem] % 2)) // * YOU GOT EM *
		V_DrawScaledPatch(BASEVIDWIDTH/2 - (SHORT(kp_yougotem->width)/2), 32, V_HUDTRANS, kp_yougotem);

	// Draw FREE PLAY.
	if (isfreeplay && !stplyr->spectator && timeinmap > 113)
	{
#ifdef HAVE_BLUA
		if (LUA_HudEnabled(hud_freeplay))
#endif
			K_drawKartFreePlay(leveltime);
	}

	if (cv_kartdebugdistribution.value)
		K_drawDistributionDebugger();

	if (cv_kartdebugcheckpoint.value)
		K_drawCheckpointDebugger();

	if (cv_kartdebugnodes.value)
	{
		UINT8 p;
		for (p = 0; p < MAXPLAYERS; p++)
			V_DrawString(8, 64+(8*p), V_YELLOWMAP, va("%d - %d (%dl)", p, playernode[p], players[p].cmd.latency));
	}

	if (cv_kartdebugcolorize.value && stplyr->mo && stplyr->mo->skin)
	{
		INT32 x = 0, y = 0;
		UINT8 c;

		for (c = 1; c < MAXSKINCOLORS; c++)
		{
			UINT8 *cm = R_GetTranslationColormap(TC_RAINBOW, c, GTC_CACHE);
			V_DrawFixedPatch(x<<FRACBITS, y<<FRACBITS, FRACUNIT>>1, 0, facewantprefix[stplyr->skin], cm);

			x += 16;
			if (x > BASEVIDWIDTH-16)
			{
				x = 0;
				y += 16;
			}
		}
	}
}

//}

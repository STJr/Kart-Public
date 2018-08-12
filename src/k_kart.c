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

// SOME IMPORTANT VARIABLES DEFINED IN DOOMDEF.H:
// gamespeed is cc (0 for easy, 1 for normal, 2 for hard)
// franticitems is Frantic Mode items, bool
// mirrormode is Mirror Mode (duh), bool
// comeback is Battle Mode's karma comeback, also bool
// battlewanted is an array of the WANTED player nums, -1 for no player in that slot
// indirectitemcooldown is timer before anyone's allowed another Shrink/SPB
// spbincoming is the timer before k_deathsentence is cast on the player in 1st
// spbplayer is the last player who fired a SPB
// mapreset is set when enough players fill an empty server


//{ SRB2kart Color Code

#define SKIN_RAMP_LENGTH 16
#define DEFAULT_STARTTRANSCOLOR 160
#define NUM_PALETTE_ENTRIES 256

// These should be within 14 characters to fit on the character select screen
const char *KartColor_Names[MAXSKINCOLORS] =
{
	"None",           // 00 // SKINCOLOR_NONE
	"Ivory",          // 01 // SKINCOLOR_IVORY
	"White",          // 02 // SKINCOLOR_WHITE
	"Silver",         // 03 // SKINCOLOR_SILVER
	"Cloudy",         // 04 // SKINCOLOR_CLOUDY
	"Grey",           // 05 // SKINCOLOR_GREY
	"Nickel",         // 06 // SKINCOLOR_NICKEL
	"Black",          // 07 // SKINCOLOR_BLACK
	"Salmon",         // 08 // SKINCOLOR_SALMON
	"Pink",           // 09 // SKINCOLOR_PINK
	"Rose",           // 10 // SKINCOLOR_ROSE
	"Raspberry",      // 11 // SKINCOLOR_RASPBERRY
	"Red",            // 12 // SKINCOLOR_RED
	"Ruby",           // 13 // SKINCOLOR_RUBY
	"Crimson",        // 14 // SKINCOLOR_CRIMSON
	"Dawn",           // 15 // SKINCOLOR_DAWN
	"Creamsicle",     // 16 // SKINCOLOR_CREAMSICLE
	"Orange",         // 17 // SKINCOLOR_ORANGE
	"Pumpkin",        // 18 // SKINCOLOR_PUMPKIN
	"Rosewood",       // 19 // SKINCOLOR_ROSEWOOD
	"Burgundy",       // 20 // SKINCOLOR_BURGUNDY
	"Bronze",         // 21 // SKINCOLOR_BRONZE
	"Sepia",          // 22 // SKINCOLOR_SEPIA
	"Beige",          // 23 // SKINCOLOR_BEIGE
	"Brown",          // 24 // SKINCOLOR_BROWN
	"Leather",        // 25 // SKINCOLOR_LEATHER
	"Peach",          // 26 // SKINCOLOR_PEACH
	"Caramel",        // 27 // SKINCOLOR_CARAMEL
	"Tangerine",      // 28 // SKINCOLOR_TANGERINE
	"Gold",           // 29 // SKINCOLOR_GOLD
	"Vomit",          // 30 // SKINCOLOR_VOMIT
	"Yellow",         // 31 // SKINCOLOR_YELLOW
	"Canary",         // 32 // SKINCOLOR_CANARY
	"Olive",          // 33 // SKINCOLOR_OLIVE
	"Garden",         // 34 // SKINCOLOR_GARDEN
	"Lime",           // 35 // SKINCOLOR_LIME
	"Tea",            // 36 // SKINCOLOR_TEA
	"Army",           // 37 // SKINCOLOR_ARMY
	"Pistachio",      // 38 // SKINCOLOR_PISTACHIO
	"Moss",           // 39 // SKINCOLOR_MOSS
	"Mint",           // 40 // SKINCOLOR_MINT
	"Green",          // 41 // SKINCOLOR_GREEN
	"Robo-Hood",      // 42 // SKINCOLOR_ROBOHOOD
	"Pinetree",       // 43 // SKINCOLOR_PINETREE
	"Emerald",        // 44 // SKINCOLOR_EMERALD
	"Swamp",          // 45 // SKINCOLOR_SWAMP
	"Aqua",           // 46 // SKINCOLOR_AQUA
	"Teal",           // 47 // SKINCOLOR_TEAL
	"Cyan",           // 48 // SKINCOLOR_CYAN
	"Cerulean",       // 49 // SKINCOLOR_CERULEAN
	"Slate",          // 50 // SKINCOLOR_SLATE
	"Steel",          // 51 // SKINCOLOR_STEEL
	"Periwinkle",     // 52 // SKINCOLOR_PERIWINKLE
	"Blue",           // 53 // SKINCOLOR_BLUE
	"Sapphire",       // 54 // SKINCOLOR_SAPPHIRE
	"Blueberry",      // 55 // SKINCOLOR_BLUEBERRY
	"Navy",           // 56 // SKINCOLOR_NAVY
	"Jet",            // 57 // SKINCOLOR_JET
	"Dusk",           // 58 // SKINCOLOR_DUSK
	"Purple",         // 59 // SKINCOLOR_PURPLE
	"Lavender",       // 60 // SKINCOLOR_LAVENDER
	"Indigo",         // 61 // SKINCOLOR_INDIGO
	"Byzantium",      // 62 // SKINCOLOR_BYZANTIUM
	"Lilac"           // 63 // SKINCOLOR_LILAC
};

// Color_Opposite replacement; frame setting has not been changed from 8 for most, should be done later
const UINT8 KartColor_Opposite[MAXSKINCOLORS*2] =
{
	SKINCOLOR_NONE,8,        // 00 // SKINCOLOR_NONE
	SKINCOLOR_BLACK,8,       // 01 // SKINCOLOR_IVORY
	SKINCOLOR_NICKEL,8,      // 02 // SKINCOLOR_WHITE
	SKINCOLOR_GREY,8,        // 03 // SKINCOLOR_SILVER
	SKINCOLOR_CLOUDY,8,      // 04 // SKINCOLOR_CLOUDY
	SKINCOLOR_SILVER,8,      // 05 // SKINCOLOR_GREY
	SKINCOLOR_WHITE,8,       // 06 // SKINCOLOR_NICKEL
	SKINCOLOR_IVORY,8,       // 07 // SKINCOLOR_BLACK
	SKINCOLOR_TEA,8,         // 08 // SKINCOLOR_SALMON
	SKINCOLOR_ARMY,8,        // 09 // SKINCOLOR_PINK
	SKINCOLOR_MOSS,8,        // 10 // SKINCOLOR_ROSE
	SKINCOLOR_MINT,10,       // 11 // SKINCOLOR_RASPBERRY
	SKINCOLOR_GREEN,8,       // 12 // SKINCOLOR_RED
	SKINCOLOR_EMERALD,6,     // 13 // SKINCOLOR_RUBY
	SKINCOLOR_PINETREE,6,    // 14 // SKINCOLOR_CRIMSON
	SKINCOLOR_DUSK,8,        // 15 // SKINCOLOR_DAWN
	SKINCOLOR_PERIWINKLE,8,  // 16 // SKINCOLOR_CREAMSICLE
	SKINCOLOR_BLUE,8,        // 17 // SKINCOLOR_ORANGE
	SKINCOLOR_BLUEBERRY,8,   // 18 // SKINCOLOR_PUMPKIN
	SKINCOLOR_NAVY,8,        // 19 // SKINCOLOR_ROSEWOOD
	SKINCOLOR_JET,8,         // 20 // SKINCOLOR_BURGUNDY
	SKINCOLOR_STEEL,8,       // 21 // SKINCOLOR_BRONZE
	SKINCOLOR_LEATHER,6,     // 22 // SKINCOLOR_SEPIA
	SKINCOLOR_BROWN,2,       // 23 // SKINCOLOR_BEIGE
	SKINCOLOR_BEIGE,8,       // 24 // SKINCOLOR_BROWN
	SKINCOLOR_SEPIA,8,       // 25 // SKINCOLOR_LEATHER
	SKINCOLOR_SLATE,8,       // 26 // SKINCOLOR_PEACH
	SKINCOLOR_TEAL,8,        // 27 // SKINCOLOR_CARAMEL
	SKINCOLOR_LIME,8,        // 28 // SKINCOLOR_TANGERINE
	SKINCOLOR_LAVENDER,6,    // 29 // SKINCOLOR_GOLD
	SKINCOLOR_ROBOHOOD,8,    // 30 // SKINCOLOR_VOMIT
	SKINCOLOR_BYZANTIUM,8,   // 31 // SKINCOLOR_YELLOW
	SKINCOLOR_PURPLE,8,      // 32 // SKINCOLOR_CANARY
	SKINCOLOR_INDIGO,8,      // 33 // SKINCOLOR_OLIVE
	SKINCOLOR_AQUA,8,        // 34 // SKINCOLOR_GARDEN
	SKINCOLOR_TANGERINE,8,   // 35 // SKINCOLOR_LIME
	SKINCOLOR_SALMON,8,      // 36 // SKINCOLOR_TEA
	SKINCOLOR_PINK,6,        // 37 // SKINCOLOR_ARMY
	SKINCOLOR_CYAN,8,        // 38 // SKINCOLOR_PISTACHIO
	SKINCOLOR_ROSE,8,        // 39 // SKINCOLOR_MOSS
	SKINCOLOR_RASPBERRY,6,   // 40 // SKINCOLOR_MINT
	SKINCOLOR_RED,8,         // 41 // SKINCOLOR_GREEN
	SKINCOLOR_VOMIT,8,       // 42 // SKINCOLOR_ROBOHOOD
	SKINCOLOR_CRIMSON,8,     // 43 // SKINCOLOR_PINETREE
	SKINCOLOR_RUBY,8,        // 44 // SKINCOLOR_EMERALD
	SKINCOLOR_SAPPHIRE,8,    // 45 // SKINCOLOR_SWAMP
	SKINCOLOR_GARDEN,10,     // 46 // SKINCOLOR_AQUA
	SKINCOLOR_CARAMEL,8,     // 47 // SKINCOLOR_TEAL
	SKINCOLOR_PISTACHIO,6,   // 48 // SKINCOLOR_CYAN
	SKINCOLOR_LILAC,8,       // 49 // SKINCOLOR_CERULEAN
	SKINCOLOR_PEACH,8,       // 50 // SKINCOLOR_SLATE
	SKINCOLOR_BRONZE,10,     // 51 // SKINCOLOR_STEEL
	SKINCOLOR_CREAMSICLE,8,  // 52 // SKINCOLOR_PERIWINKLE
	SKINCOLOR_ORANGE,8,      // 53 // SKINCOLOR_BLUE
	SKINCOLOR_SWAMP,8,       // 54 // SKINCOLOR_SAPPHIRE
	SKINCOLOR_PUMPKIN,8,     // 55 // SKINCOLOR_BLUEBERRY
	SKINCOLOR_ROSEWOOD,8,    // 56 // SKINCOLOR_NAVY
	SKINCOLOR_BURGUNDY,6,    // 57 // SKINCOLOR_JET
	SKINCOLOR_DAWN,8,        // 58 // SKINCOLOR_DUSK
	SKINCOLOR_CANARY,8,      // 59 // SKINCOLOR_PURPLE
	SKINCOLOR_GOLD,10,       // 60 // SKINCOLOR_LAVENDER
	SKINCOLOR_OLIVE,8,       // 61 // SKINCOLOR_INDIGO
	SKINCOLOR_YELLOW,8,      // 62 // SKINCOLOR_BYZANTIUM
	SKINCOLOR_CERULEAN,8     // 63 // SKINCOLOR_LILAC
};

UINT8 colortranslations[MAXSKINCOLORS][16] = {
	{  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0}, // SKINCOLOR_NONE
	{120, 120, 120, 120,   0,   1,   3,   4,   6,   7,  10,  14,  18,  22,  25,  28}, // SKINCOLOR_IVORY
	{120, 120,   0,   1,   3,   4,   6,   7,   9,  11,  13,  16,  19,  23,  26,  29}, // SKINCOLOR_WHITE
	{  0,   1,   3,   5,   6,   8,  10,  11,  13,  15,  16,  18,  20,  24,  27,  30}, // SKINCOLOR_SILVER
	{  1,   3,   5,   7,   9,  11,  13,  15,  17,  19,  21,  23,  25,  27,  29,  31}, // SKINCOLOR_CLOUDY
	{  8,   9,  10,  12,  13,  15,  16,  19,  19,  20,  21,  23,  25,  27,  29,  31}, // SKINCOLOR_GREY
	{ 16,  16,  17,  18,  19,  20,  21,  23,  24,  25,  26,  27,  28,  29,  30,  31}, // SKINCOLOR_NICKEL
	{ 16,  17,  19,  21,  22,  24,  26,  27,  27,  28,  28,  29,  29,  30,  30,  31}, // SKINCOLOR_BLACK
	{120, 120, 120, 121, 121, 122, 122, 123, 124, 125, 126, 128, 129, 131, 133, 135}, // SKINCOLOR_SALMON
	{121, 121, 121, 121, 121, 122, 144, 145, 146, 147, 148, 149, 150, 150, 150, 151}, // SKINCOLOR_PINK
	{144, 145, 146, 147, 148, 149, 150, 151, 134, 135, 136, 137, 138, 139, 140, 141}, // SKINCOLOR_ROSE
	{120, 121, 122, 123, 124, 125, 126, 127, 128, 130, 131, 133, 134, 136, 137, 139}, // SKINCOLOR_RASPBERRY
	{125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140}, // SKINCOLOR_RED
	{120, 121, 144, 146, 149, 132, 132, 133, 134, 135, 197, 197, 198, 198, 199, 255}, // SKINCOLOR_RUBY
	{130, 131, 132, 133, 134, 136, 137, 138, 139, 139, 140, 140, 141, 141, 142, 143}, // SKINCOLOR_CRIMSON
	{120, 121, 122, 123, 124, 147, 147, 148,  90,  91,  92,  93,  94,  95, 152, 154}, // SKINCOLOR_DAWN
	{120, 120,  80,  80,  81,  82,  83,  83,  84,  85,  86,  88,  89,  91,  93,  95}, // SKINCOLOR_CREAMSICLE
	{ 80,  81,  82,  83,  84,  85,  86,  88,  89,  91,  94,  95, 154, 156, 158, 159}, // SKINCOLOR_ORANGE
	{ 84,  85,  86,  87,  88,  90,  92,  93,  94,  95, 152, 153, 154, 156, 157, 159}, // SKINCOLOR_PUMPKIN
	{ 90,  91,  92,  93,  94, 152, 153, 154, 155, 156, 157, 158, 159, 139, 141, 143}, // SKINCOLOR_ROSEWOOD
	{ 94,  95, 152, 153, 154, 156, 157, 159, 141, 141, 141, 142, 142, 143, 143,  31}, // SKINCOLOR_BURGUNDY
	{112, 113, 114, 115, 116, 117, 118, 119, 156, 157, 158, 159, 141, 141, 142, 143}, // SKINCOLOR_BRONZE
	{  0,   1,   3,   5,   7,   9,  34,  36,  38,  40,  42,  44,  60,  61,  62,  63}, // SKINCOLOR_SEPIA
	{ 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47}, // SKINCOLOR_BEIGE
	{ 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63}, // SKINCOLOR_BROWN
	{ 51,  52,  53,  55,  56,  57,  58,  60,  61,  63,  28,  28,  29,  29,  30,  31}, // SKINCOLOR_LEATHER
	{ 64,  65,  67,  68,  70,  71,  73,  74,  76,  77,  79,  48,  50,  53,  56,  59}, // SKINCOLOR_PEACH
	{ 64,  66,  68,  70,  72,  74,  76,  78,  48,  50,  52,  54,  56,  58,  60,  62}, // SKINCOLOR_CARAMEL
	{ 98,  98, 112, 112, 113, 113,  84,  85,  87,  89,  91,  93,  95, 153, 156, 159}, // SKINCOLOR_TANGERINE
	{112, 112, 112, 113, 113, 114, 114, 115, 115, 116, 116, 117, 117, 118, 118, 119}, // SKINCOLOR_GOLD
	{121, 144, 145,  72,  73,  84, 114, 115, 107, 108, 109, 183, 223, 207,  30, 246}, // SKINCOLOR_VOMIT
	{ 96,  97,  98, 100, 101, 102, 104, 113, 114, 115, 116, 117, 118, 119, 156, 159}, // SKINCOLOR_YELLOW
	{ 96,  97,  99, 100, 102, 104, 105, 105, 106, 107, 107, 108, 109, 109, 110, 111}, // SKINCOLOR_CANARY
	{105, 105, 105, 106, 106, 107, 107, 108, 108, 109, 109, 110, 110, 111, 111,  31}, // SKINCOLOR_OLIVE
	{ 98,  99, 112, 101, 113, 114, 106, 179, 180, 180, 181, 182, 183, 173, 174, 175}, // SKINCOLOR_GARDEN
	{ 96,  97,  99, 100, 102, 104, 160, 162, 164, 166, 168, 171, 223, 223, 207,  31}, // SKINCOLOR_LIME
	{120, 120, 176, 176, 176, 177, 177, 178, 178, 179, 180, 180, 181, 181, 182, 183}, // SKINCOLOR_TEA
	{176, 176, 176, 177, 177, 178, 178, 179, 179, 180, 180, 181, 181, 182, 182, 183}, // SKINCOLOR_ARMY
	{120, 120, 176, 176, 177, 177, 178, 179, 165, 166, 167, 168, 169, 170, 171, 172}, // SKINCOLOR_PISTACHIO
	{178, 178, 178, 179, 179, 180, 181, 182, 183, 172, 172, 173, 173, 174, 174, 175}, // SKINCOLOR_MOSS
	{120, 176, 176, 176, 177, 163, 164, 165, 167, 221, 221, 222, 223, 207, 207,  31}, // SKINCOLOR_MINT
	{160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175}, // SKINCOLOR_GREEN
	{176, 176, 177, 178, 165, 166, 167, 167, 168, 169, 182, 182, 182, 183, 183, 183}, // SKINCOLOR_ROBOHOOD
	{160, 161, 162, 164, 165, 167, 169, 170, 171, 171, 172, 173, 174, 175,  30,  31}, // SKINCOLOR_PINETREE
	{160, 184, 184, 185, 185, 186, 186, 187, 187, 188, 188, 189, 189, 190, 191, 175}, // SKINCOLOR_EMERALD
	{186, 187, 188, 188, 188, 189, 189, 190, 190, 191, 175, 175,  30,  30,  31,  31}, // SKINCOLOR_SWAMP
	{120, 208, 208, 210, 212, 214, 220, 220, 220, 221, 221, 222, 222, 223, 223, 191}, // SKINCOLOR_AQUA
	{210, 213, 220, 220, 220, 221, 221, 221, 221, 222, 222, 222, 223, 223, 191,  31}, // SKINCOLOR_TEAL
	{120, 208, 209, 210, 211, 212, 213, 215, 216, 216, 216, 217, 217, 218, 218, 219}, // SKINCOLOR_CYAN
	{208, 209, 211, 213, 215, 216, 216, 217, 217, 218, 218, 219, 205, 206, 207, 207}, // SKINCOLOR_CERULEAN
	{120, 120, 200, 200, 200, 201, 201, 201, 202, 202, 202, 203, 204, 205, 206, 207}, // SKINCOLOR_SLATE
	{120, 200, 200, 201, 201, 202, 202, 203, 203, 204, 204, 205, 205, 206, 207,  31}, // SKINCOLOR_STEEL
	{120, 224, 225, 226, 226, 227, 228, 228, 229, 230, 231, 234, 235, 237, 239, 241}, // SKINCOLOR_PERIWINKLE
	{224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239}, // SKINCOLOR_BLUE
	{208, 209, 211, 213, 215, 217, 229, 230, 232, 234, 236, 238, 240, 242, 244, 246}, // SKINCOLOR_SAPPHIRE
	{228, 229, 230, 231, 232, 233, 234, 235, 237, 238, 239, 240, 242, 243, 244, 245}, // SKINCOLOR_BLUEBERRY
	{215, 216, 217, 218, 204, 205, 206, 237, 238, 239, 240, 241, 242, 243, 244, 245}, // SKINCOLOR_NAVY
	{200, 201, 202, 203, 204, 205, 206, 207,  28,  28,  29,  29,  30,  30,  31,  31}, // SKINCOLOR_JET
	{192, 192, 248, 249, 250, 251, 204, 204, 205, 205, 206, 206, 207,  29,  30,  31}, // SKINCOLOR_DUSK
	{192, 192, 192, 193, 193, 194, 194, 195, 195, 196, 196, 197, 197, 198, 198, 199}, // SKINCOLOR_PURPLE
	{248, 248, 248, 249, 249, 250, 250, 251, 251, 252, 252, 253, 253, 254, 254, 255}, // SKINCOLOR_LAVENDER
	{192, 193, 194, 195, 196, 197, 198, 199, 255, 255,  29,  29,  30,  30,  31,  31}, // SKINCOLOR_INDIGO
	{192, 248, 249, 250, 251, 252, 253, 254, 255, 255,  29,  29,  30,  30,  31,  31}, // SKINCOLOR_BYZANTIUM
	{120, 120, 120, 121, 121, 122, 122, 123, 192, 248, 249, 250, 251, 252, 253, 254}, // SKINCOLOR_LILAC
	/* Removed Colours
		{120, 121, 123, 124, 126, 127, 129, 130, 132, 133, 135, 136, 138, 139, 141, 143}, // SKINCOLOR_RUBY, removed for other colors
		{ 80,  81,  83,  85,  86,  88,  90,  91,  93,  95, 152, 153, 154, 156, 157, 159}, // SKINCOLOR_AMBER, removed for other colors
		{224, 225, 226, 228, 229, 231, 232, 234, 235, 237, 238, 240, 241, 243, 244, 246}, // SKINCOLOR_SAPPHIRE, removed for other colors
		{160, 160, 160, 184, 184, 184, 185, 185, 185, 186, 187, 187, 188, 188, 189, 190}, // SKINCOLOR_JADE, removed for other colors
		{224, 225, 226, 212, 213, 213, 214, 215, 220, 221, 172, 222, 173, 223, 174, 175}, // SKINCOLOR_FROST, merged into Aqua
		{ 72,  73,  74,  75,  76,  77,  78,  79,  48,  49,  50,  51,  52,  53,  54,  55}, // SKINCOLOR_CARAMEL, new Caramel was previously Shiny Caramel
		{  1, 145, 125,  73,  83, 114, 106, 180, 187, 168, 219, 205, 236, 206, 199, 255}, // SKINCOLOR_RAINBOW, is Vomit 2.0
	*/
};

/** \brief	Generates the rainbow colourmaps that are used when a player has the invincibility power

	\param	dest_colormap	colormap to populate
	\param	skincolor		translation color
*/
void K_RainbowColormap(UINT8 *dest_colormap, UINT8 skincolor)
{
	INT32 i, j;
	RGBA_t color;
	UINT8 colorbrightnesses[16];
	UINT8 brightness;
	UINT16 brightdif;
	INT32 temp;

	// first generate the brightness of all the colours of that skincolour
	for (i = 0; i < 16; i++)
	{
		color = V_GetColor(colortranslations[skincolor][i]);
		colorbrightnesses[i] = (UINT8)(((UINT16)color.s.red + (UINT16)color.s.green + (UINT16)color.s.blue)/3);
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
		brightness = (UINT8)(((UINT16)color.s.red + (UINT16)color.s.green + (UINT16)color.s.blue)/3);
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
	if (skinnum == TC_BOSS || skinnum == TC_ALLWHITE || skinnum == TC_METALSONIC || color == SKINCOLOR_NONE)
	{
		for (i = 0; i < NUM_PALETTE_ENTRIES; i++)
		{
			if (skinnum == TC_ALLWHITE) dest_colormap[i] = 0;
			else dest_colormap[i] = (UINT8)i;
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

	CV_RegisterVar(&cv_triplesneaker);
	CV_RegisterVar(&cv_triplebanana);
	CV_RegisterVar(&cv_tripleorbinaut);
	CV_RegisterVar(&cv_dualjawz);

	CV_RegisterVar(&cv_kartminimap);
	CV_RegisterVar(&cv_kartcheck);
	CV_RegisterVar(&cv_kartinvinsfx);
	CV_RegisterVar(&cv_kartspeed);
	CV_RegisterVar(&cv_kartbumpers);
	CV_RegisterVar(&cv_kartfrantic);
	CV_RegisterVar(&cv_kartcomeback);
	CV_RegisterVar(&cv_kartmirror);
	CV_RegisterVar(&cv_kartspeedometer);
	CV_RegisterVar(&cv_kartvoices);
	CV_RegisterVar(&cv_karteliminatelast);
	CV_RegisterVar(&cv_votetime);

	CV_RegisterVar(&cv_kartdebugitem);
	CV_RegisterVar(&cv_kartdebugamount);
	CV_RegisterVar(&cv_kartdebugcheckpoint);
	CV_RegisterVar(&cv_kartdebugshrink);
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
static INT32 K_KartItemOddsRace[NUMKARTRESULTS][9] =
{
				//P-Odds	 0  1  2  3  4  5  6  7  8
			   /*Sneaker*/ {20, 0, 0, 3, 6, 6, 0, 0, 0 }, // Sneaker
		/*Rocket Sneaker*/ { 0, 0, 0, 0, 0, 2, 5, 4, 0 }, // Rocket Sneaker
		 /*Invincibility*/ { 0, 0, 0, 0, 0, 1, 5, 6,16 }, // Invincibility
				/*Banana*/ { 0, 9, 4, 2, 1, 0, 0, 0, 0 }, // Banana
		/*Eggman Monitor*/ { 0, 4, 3, 2, 0, 0, 0, 0, 0 }, // Eggman Monitor
			  /*Orbinaut*/ { 0, 6, 5, 4, 2, 0, 0, 0, 0 }, // Orbinaut
				  /*Jawz*/ { 0, 0, 3, 2, 2, 1, 0, 0, 0 }, // Jawz
				  /*Mine*/ { 0, 0, 1, 2, 1, 0, 0, 0, 0 }, // Mine
			   /*Ballhog*/ { 0, 0, 1, 2, 1, 0, 0, 0, 0 }, // Ballhog
   /*Self-Propelled Bomb*/ { 0, 0, 1, 1, 1, 2, 2, 3, 2 }, // Self-Propelled Bomb
				  /*Grow*/ { 0, 0, 0, 0, 0, 1, 3, 6, 4 }, // Grow
				/*Shrink*/ { 0, 0, 0, 0, 0, 0, 0, 1, 0 }, // Shrink
		/*Thunder Shield*/ { 0, 1, 2, 0, 0, 0, 0, 0, 0 }, // Thunder Shield
			   /*Hyudoro*/ { 0, 0, 0, 0, 1, 2, 1, 0, 0 }, // Hyudoro
		   /*Pogo Spring*/ { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // Pogo Spring
		  /*Kitchen Sink*/ { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // Kitchen Sink
			/*Sneaker x3*/ { 0, 0, 0, 0, 3, 6, 5, 3, 0 }, // Sneaker x3
			 /*Banana x3*/ { 0, 0, 1, 1, 0, 0, 0, 0, 0 }, // Banana x3
			/*Banana x10*/ { 0, 0, 0, 0, 1, 0, 0, 0, 0 }, // Banana x10
		   /*Orbinaut x3*/ { 0, 0, 0, 1, 1, 0, 0, 0, 0 }, // Orbinaut x3
		   /*Orbinaut x4*/ { 0, 0, 0, 0, 0, 1, 1, 0, 0 }, // Orbinaut x4
			   /*Jawz x2*/ { 0, 0, 0, 1, 1, 0, 0, 0, 0 }  // Jawz x2
};

static INT32 K_KartItemOddsBattle[NUMKARTRESULTS][6] =
{
				//P-Odds	 0  1  2  3
			   /*Sneaker*/ { 2, 2, 2, 2 }, // Sneaker
		/*Rocket Sneaker*/ { 0, 0, 0, 0 }, // Rocket Sneaker
		 /*Invincibility*/ { 4, 2, 1, 2 }, // Invincibility
				/*Banana*/ { 0, 0, 2, 0 }, // Banana
		/*Eggman Monitor*/ { 0, 0, 1, 0 }, // Eggman Monitor
			  /*Orbinaut*/ { 0, 1, 5, 0 }, // Orbinaut
				  /*Jawz*/ { 1, 3, 2, 2 }, // Jawz
				  /*Mine*/ { 1, 3, 2, 2 }, // Mine
			   /*Ballhog*/ { 1, 2, 1, 2 }, // Ballhog
   /*Self-Propelled Bomb*/ { 0, 0, 0, 0 }, // Self-Propelled Bomb
				  /*Grow*/ { 4, 2, 0, 2 }, // Grow
				/*Shrink*/ { 0, 0, 0, 0 }, // Shrink
		/*Thunder Shield*/ { 0, 0, 0, 0 }, // Thunder Shield
			   /*Hyudoro*/ { 0, 0, 1, 0 }, // Hyudoro
		   /*Pogo Spring*/ { 0, 0, 1, 0 }, // Pogo Spring
		  /*Kitchen Sink*/ { 0, 0, 0, 0 }, // Kitchen Sink
			/*Sneaker x3*/ { 2, 0, 0, 2 }, // Sneaker x3
			 /*Banana x3*/ { 0, 1, 1, 1 }, // Banana x3
			/*Banana x10*/ { 1, 0, 0, 1 }, // Banana x10
		   /*Orbinaut x3*/ { 0, 1, 1, 1 }, // Orbinaut x3
		   /*Orbinaut x4*/ { 1, 1, 0, 1 }, // Orbinaut x4
			   /*Jawz x2*/ { 3, 2, 0, 2 }  // Jawz x2
};

/**	\brief	Item Roulette for Kart

	\param	player		player
	\param	getitem		what item we're looking for

	\return	void
*/
static void K_KartGetItemResult(player_t *player, SINT8 getitem)
{
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

static INT32 K_KartGetItemOdds(UINT8 pos, SINT8 item, fixed_t mashed)
{
	const INT32 distvar = (64*14);
	INT32 newodds;
	INT32 i;
	UINT8 pingame = 0, pexiting = 0, pinvin = 0;
	SINT8 first = -1, second = -1;
	INT32 secondist = 0;

	if (G_BattleGametype())
		newodds = K_KartItemOddsBattle[item-1][pos];
	else
		newodds = K_KartItemOddsRace[item-1][pos];

	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (!playeringame[i] || players[i].spectator)
			continue;

		pingame++;
		if (players[i].exiting)
			pexiting++;
		if (players[i].mo)
		{
			if (players[i].kartstuff[k_position] == 1 && first == -1)
				first = i;
			if (players[i].kartstuff[k_position] == 2 && second == -1)
				second = i;
			if (players[i].kartstuff[k_itemtype] == KITEM_INVINCIBILITY
				|| players[i].kartstuff[k_itemtype] == KITEM_GROW
				|| players[i].kartstuff[k_invincibilitytimer]
				|| players[i].kartstuff[k_growshrinktimer] > 0)
				pinvin++;
		}
	}

	if (first != -1 && second != -1) // calculate 2nd's distance from 1st, for SPB
	{
		secondist = P_AproxDistance(P_AproxDistance(players[first].mo->x - players[second].mo->x,
													players[first].mo->y - players[second].mo->y),
													players[first].mo->z - players[second].mo->z) / mapheaderinfo[gamemap-1]->mobj_scale;
		if (franticitems)
			secondist = (15*secondist/14);
		if (pingame < 6 && !G_BattleGametype())
			secondist = ((28+(6-pingame))*secondist/28);
	}

	// POWERITEMODDS handles all of the "frantic item" related functionality, for all of our powerful items.
	// First, it multiplies it by 2 if franticitems is true; easy-peasy.
	// Then, it multiplies it further if there's less than 5 players in game.
	// This is done to make low player count races more fair & interesting. (1v1s are basically the same as franticitems false in a normal race)
	// Lastly, it *divides* it by your mashed value, which was determined in K_KartItemRoulette, to punish those who are impatient.
	// The last two are very fractional and complicated, very sorry!
#define POWERITEMODDS(odds) \
	if (franticitems) \
		odds *= 2; \
	if (pingame < 6 && !G_BattleGametype()) \
		odds = FixedMul(odds*FRACUNIT, FRACUNIT+min((6-pingame)*(FRACUNIT/25), FRACUNIT))/FRACUNIT; \
	if (mashed > 0) \
		odds = FixedDiv(odds*FRACUNIT, mashed+FRACUNIT)/FRACUNIT \

	switch (item)
	{
		case KITEM_SNEAKER:
			if ((!cv_sneaker.value) && (!modeattacking)) newodds = 0;
			break;
		case KITEM_ROCKETSNEAKER:
			POWERITEMODDS(newodds);
			if (!cv_rocketsneaker.value) newodds = 0;
			break;
		case KITEM_INVINCIBILITY:
			POWERITEMODDS(newodds);
			if ((!cv_invincibility.value) || (pinvin > 2)) newodds = 0;
			break;
		case KITEM_BANANA:
			if (!cv_banana.value) newodds = 0;
			break;
		case KITEM_EGGMAN:
			if (!cv_eggmanmonitor.value) newodds = 0;
			break;
		case KITEM_ORBINAUT:
			if (!cv_orbinaut.value) newodds = 0;
			break;
		case KITEM_JAWZ:
			POWERITEMODDS(newodds);
			if (!cv_jawz.value) newodds = 0;
			break;
		case KITEM_MINE:
			POWERITEMODDS(newodds);
			if (!cv_mine.value) newodds = 0;
			break;
		case KITEM_BALLHOG:
			POWERITEMODDS(newodds);
			if (!cv_ballhog.value) newodds = 0;
			break;
		case KITEM_SPB:
			POWERITEMODDS(newodds);
			if ((!cv_selfpropelledbomb.value)
				|| (indirectitemcooldown > 0)
				|| (pexiting > 0)
				|| (secondist/distvar < (4+gamespeed)))
				newodds = 0;
			newodds *= min((secondist/distvar)-(3+gamespeed), 3);
			break;
		case KITEM_GROW:
			POWERITEMODDS(newodds);
			if ((!cv_grow.value) || (pinvin > 2)) newodds = 0;
			break;
		case KITEM_SHRINK:
			POWERITEMODDS(newodds);
			if ((!cv_shrink.value)
				|| (indirectitemcooldown > 0)
				|| (pingame-1 <= pexiting)) newodds = 0;
			break;
		case KITEM_THUNDERSHIELD:
			POWERITEMODDS(newodds);
			if (!cv_thundershield.value) newodds = 0;
			break;
		case KITEM_HYUDORO:
			if (!cv_hyudoro.value) newodds = 0;
			break;
		case KITEM_POGOSPRING:
			if (!cv_pogospring.value) newodds = 0;
			break;
		case KITEM_KITCHENSINK:
			newodds = 0; // Not obtained via normal means.
			break;
		case KRITEM_TRIPLESNEAKER:
			POWERITEMODDS(newodds);
			if (!cv_triplesneaker.value) newodds = 0;
			break;
		case KRITEM_TRIPLEBANANA:
			POWERITEMODDS(newodds);
			if (!cv_triplebanana.value) newodds = 0;
			break;
		case KRITEM_TENFOLDBANANA:
			POWERITEMODDS(newodds);
			if (!cv_triplebanana.value) newodds = 0;
			break;
		case KRITEM_TRIPLEORBINAUT:
			POWERITEMODDS(newodds);
			if (!cv_tripleorbinaut.value) newodds = 0;
			break;
		case KRITEM_QUADORBINAUT:
			POWERITEMODDS(newodds);
			if (!cv_tripleorbinaut.value) newodds = 0;
			break;
		case KRITEM_DUALJAWZ:
			POWERITEMODDS(newodds);
			if (!cv_dualjawz.value) newodds = 0;
			break;
		default:
			break;
	}
#undef POWERITEMODDS

	return newodds;
}

//{ SRB2kart Roulette Code - Distance Based, no waypoints

static void K_KartItemRoulette(player_t *player, ticcmd_t *cmd)
{
	const INT32 distvar = (64*14);
	INT32 i;
	UINT8 pingame = 0;
	UINT8 roulettestop;
	INT32 pdis = 0, useodds = 0;
	INT32 spawnchance[NUMKARTRESULTS * NUMKARTODDS];
	INT32 chance = 0, numchoices = 0;
	INT32 bestbumper = 0;
	boolean oddsvalid[9];
	UINT8 disttable[14];
	UINT8 distlen = 0;
	fixed_t mashed = 0;

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
		if (players[i].kartstuff[k_bumper] > bestbumper)
			bestbumper = players[i].kartstuff[k_bumper];
	}

	// This makes the roulette produce the random noises.
	if ((player->kartstuff[k_itemroulette] % 3) == 1 && P_IsLocalPlayer(player))
		S_StartSound(NULL, sfx_mkitm1 + ((player->kartstuff[k_itemroulette] / 3) % 8));

	roulettestop = (TICRATE*1) + (3*(pingame - player->kartstuff[k_position]));

	// If the roulette finishes or the player presses BT_ATTACK, stop the roulette and calculate the item.
	// I'm returning via the exact opposite, however, to forgo having another bracket embed. Same result either way, I think.
	// Finally, if you get past this check, now you can actually start calculating what item you get.
	if ((cmd->buttons & BT_ATTACK) && !(player->kartstuff[k_eggmanheld] || player->kartstuff[k_itemheld]) && player->kartstuff[k_itemroulette] >= roulettestop)
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
		player->kartstuff[k_itemroulette] = 0;
		player->kartstuff[k_roulettetype] = 0;
		if (P_IsLocalPlayer(player))
			S_StartSound(NULL, sfx_mkitmF);
		return;
	}

	if (cv_kartdebugitem.value != 0)
	{
		K_KartGetItemResult(player, cv_kartdebugitem.value);
		player->kartstuff[k_itemamount] = cv_kartdebugamount.value;
		player->kartstuff[k_itemroulette] = 0;
		player->kartstuff[k_roulettetype] = 0;
		if (P_IsLocalPlayer(player))
			S_StartSound(NULL, sfx_mkitmF);
		return;
	}

	// Initializes existing spawnchance values
	for (i = 0; i < (NUMKARTRESULTS * NUMKARTODDS); i++)
		spawnchance[i] = 0;

	for (i = 0; i < 9; i++)
	{
		INT32 j;
		boolean available = false;

		if (G_BattleGametype() && i > 5)
		{
			oddsvalid[i] = false;
			break;
		}

		for (j = 0; j < NUMKARTRESULTS; j++)
		{
			if (K_KartGetItemOdds(i, j, mashed) > 0)
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
													players[i].mo->z - player->mo->z) / mapheaderinfo[gamemap-1]->mobj_scale
													* (pingame - players[i].kartstuff[k_position])
													/ ((pingame - 1) * (pingame + 1) / 3);
	}

#define SETUPDISTTABLE(odds, num) \
	for (i = num; i; --i) disttable[distlen++] = odds

	if (G_BattleGametype()) // Battle Mode
	{
		if (oddsvalid[0]) SETUPDISTTABLE(0,1);
		if (oddsvalid[1]) SETUPDISTTABLE(1,1);
		if (oddsvalid[2]) SETUPDISTTABLE(2,1);

		if (player->kartstuff[k_roulettetype] == 1 && oddsvalid[3]) // Player-controlled "Karma" items
			useodds = 3;
		else
		{
			SINT8 wantedpos = (player->kartstuff[k_bumper]-bestbumper)+2; // 0 is two bumpers below best player's bumper count, 2 is best player's bumper count
			if (K_IsPlayerWanted(player))
				wantedpos--;
			if (wantedpos > 2)
				wantedpos = 2;
			if (wantedpos < 0)
				wantedpos = 0;
			useodds = disttable[(wantedpos * distlen) / 3];
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
			pdis = (15*pdis/14);
		if (pingame < 6 && !G_BattleGametype())
			pdis = ((28+(6-pingame))*pdis/28);

		if (pingame == 1 && oddsvalid[0])					// Record Attack, or just alone
			useodds = 0;
		else if (pdis <= 0)								// (64*14) *  0 =     0
			useodds = disttable[0];
		else if (pdis > distvar * ((12 * distlen) / 14))	// (64*14) * 12 = 10752
			useodds = disttable[distlen-1];
		else
		{
			for (i = 1; i < 12; i++)
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

#define SETITEMRESULT(pos, itemnum) \
	for (chance = 0; chance < K_KartGetItemOdds(pos, itemnum, mashed); chance++) \
		spawnchance[numchoices++] = itemnum

	SETITEMRESULT(useodds, KITEM_SNEAKER);			// Sneaker
	SETITEMRESULT(useodds, KITEM_ROCKETSNEAKER);	// Rocket Sneaker
	SETITEMRESULT(useodds, KITEM_INVINCIBILITY);	// Invincibility
	SETITEMRESULT(useodds, KITEM_BANANA);			// Banana
	SETITEMRESULT(useodds, KITEM_EGGMAN);			// Eggman Monitor
	SETITEMRESULT(useodds, KITEM_ORBINAUT);			// Orbinaut
	SETITEMRESULT(useodds, KITEM_JAWZ);				// Jawz
	SETITEMRESULT(useodds, KITEM_MINE);				// Mine
	SETITEMRESULT(useodds, KITEM_BALLHOG);			// Ballhog
	SETITEMRESULT(useodds, KITEM_SPB);				// Self-Propelled Bomb
	SETITEMRESULT(useodds, KITEM_GROW);				// Grow
	SETITEMRESULT(useodds, KITEM_SHRINK);			// Shrink
	SETITEMRESULT(useodds, KITEM_THUNDERSHIELD);	// Thunder Shield
	SETITEMRESULT(useodds, KITEM_HYUDORO);			// Hyudoro
	SETITEMRESULT(useodds, KITEM_POGOSPRING);		// Pogo Spring
	//SETITEMRESULT(useodds, KITEM_KITCHENSINK);	// Kitchen Sink

	SETITEMRESULT(useodds, KRITEM_TRIPLESNEAKER);	// Sneaker x3
	SETITEMRESULT(useodds, KRITEM_TRIPLEBANANA);	// Banana x3
	SETITEMRESULT(useodds, KRITEM_TENFOLDBANANA);	// Banana x10
	SETITEMRESULT(useodds, KRITEM_TRIPLEORBINAUT);	// Orbinaut x3
	SETITEMRESULT(useodds, KRITEM_QUADORBINAUT);	// Orbinaut x4
	SETITEMRESULT(useodds, KRITEM_DUALJAWZ);		// Jawz x2

#undef SETITEMRESULT

	// Award the player whatever power is rolled
	if (numchoices > 0)
		K_KartGetItemResult(player, spawnchance[P_RandomKey(numchoices)]);
	else
	{
		player->kartstuff[k_itemtype] = KITEM_SAD;
		player->kartstuff[k_itemamount] = 1;
	}

	player->kartstuff[k_itemroulette] = 0; // Since we're done, clear the roulette number
	player->kartstuff[k_roulettetype] = 0; // This too

	if (P_IsLocalPlayer(player))
		S_StartSound(NULL, sfx_mkitmF);
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
				weight = (mobj->player->kartweight)<<FRACBITS;
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
	//fixed_t nobumpx = 0, nobumpy = 0;
	fixed_t dot, p;
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

	// if the speed difference is less than this let's assume they're going proportionately faster from each other
	if (P_AproxDistance(momdifx, momdify) < (25*mapheaderinfo[gamemap-1]->mobj_scale))
	{
		fixed_t momdiflength = P_AproxDistance(momdifx, momdify);
		fixed_t normalisedx = FixedDiv(momdifx, momdiflength);
		fixed_t normalisedy = FixedDiv(momdify, momdiflength);
		momdifx = FixedMul((25*mapheaderinfo[gamemap-1]->mobj_scale), normalisedx);
		momdify = FixedMul((25*mapheaderinfo[gamemap-1]->mobj_scale), normalisedy);
	}

	/*if (mass1 == 0 && mass2 > 0)
	{
		nobumpx = mobj2->momx;
		nobumpy = mobj2->momy;
	}
	else if (mass2 == 0 && mass1 > 0)
	{
		nobumpx = mobj1->momx;
		nobumpy = mobj1->momy;
	}*/

	distx = (mobj1->x + mobj2->momx) - (mobj2->x + mobj1->momx);
	disty = (mobj1->y + mobj2->momy) - (mobj2->y + mobj1->momy);

	if (distx == 0 && disty == 0)
	{
		// if there's no distance between the 2, they're directly on top of each other, don't run this
		return;
	}

	dot = FixedMul(momdifx, distx) + FixedMul(momdify, disty);

	if (dot >= 0)
	{
		// They're moving away from each other
		return;
	}

	p = FixedDiv(dot, FixedMul(distx, distx)+FixedMul(disty, disty));

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
		mobj1->momx = mobj1->momx - FixedMul(FixedMul(FixedDiv(2*mass2, mass1 + mass2), p), distx);
		mobj1->momy = mobj1->momy - FixedMul(FixedMul(FixedDiv(2*mass2, mass1 + mass2), p), disty);
	}

	if (mass1 > 0 && solid == false)
	{
		mobj2->momx = mobj2->momx - FixedMul(FixedMul(FixedDiv(2*mass1, mass1 + mass2), p), -distx);
		mobj2->momy = mobj2->momy - FixedMul(FixedMul(FixedDiv(2*mass1, mass1 + mass2), p), -disty);
	}

	// Do the bump fx when we've CONFIRMED we can bump.
	S_StartSound(mobj1, sfx_s3k49);

	fx = P_SpawnMobj(mobj1->x/2 + mobj2->x/2, mobj1->y/2 + mobj2->y/2, mobj1->z/2 + mobj2->z/2, MT_BUMP);
	if (mobj1->eflags & MFE_VERTICALFLIP)
		fx->eflags |= MFE_VERTICALFLIP;
	else
		fx->eflags &= ~MFE_VERTICALFLIP;
	fx->scale = mobj1->scale;

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
			mobj1->player->kartstuff[k_wipeoutslow] += wipeoutslowtime+1;
			mobj1->player->kartstuff[k_spinouttimer] += wipeoutslowtime+1;
		}
	}

	if (mobj2->player)
	{
		mobj2->player->rmomx = mobj2->momx - mobj2->player->cmomx;
		mobj2->player->rmomy = mobj2->momy - mobj2->player->cmomy;
		mobj2->player->kartstuff[k_justbumped] = bumptime;
		if (mobj2->player->kartstuff[k_spinouttimer])
		{
			mobj2->player->kartstuff[k_wipeoutslow] += wipeoutslowtime+1;
			mobj2->player->kartstuff[k_spinouttimer] += wipeoutslowtime+1;
		}
	}
}

/**	\brief	Checks that the player is on an offroad subsector for realsies

	\param	mo	player mobj object

	\return	boolean
*/
static UINT8 K_CheckOffroadCollide(mobj_t *mo, sector_t *sec)
{
	UINT8 i;
	sector_t *sec2;

	I_Assert(mo != NULL);
	I_Assert(!P_MobjWasRemoved(mo));

	sec2 = P_ThingOnSpecial3DFloor(mo);

	for (i = 2; i < 5; i++)
	{
		if ((sec2 && GETSECSPECIAL(sec2->special, 1) == i)
			|| (P_IsObjectOnRealGround(mo, sec)
			&& GETSECSPECIAL(sec->special, 1) == i))
			return i;
	}

	return 0;
}

/**	\brief	Updates the Player's offroad value once per frame

	\param	player	player object passed from K_KartPlayerThink

	\return	void
*/
static void K_UpdateOffroad(player_t *player)
{
	fixed_t kartweight = player->kartweight;
	fixed_t offroad;
	sector_t *nextsector = R_PointInSubsector(
		player->mo->x + player->mo->momx*2, player->mo->y + player->mo->momy*2)->sector;

	fixed_t offroadstrength = 0;

	if (K_CheckOffroadCollide(player->mo, nextsector) == 2)	// Weak Offroad
		offroadstrength = 1;
	else if (K_CheckOffroadCollide(player->mo, nextsector) == 3)	// Mid Offroad
		offroadstrength = 2;
	else if (K_CheckOffroadCollide(player->mo, nextsector) == 4)	// Strong Offroad
		offroadstrength = 3;

	// If you are offroad, a timer starts. Depending on your weight value, the timer increments differently.
	//if ((nextsector->special & 256) && nextsector->special != 768
	//	&& nextsector->special != 1024 && nextsector->special != 4864)
	if (offroadstrength)
	{
		if (K_CheckOffroadCollide(player->mo, player->mo->subsector->sector) && player->kartstuff[k_offroad] == 0)
			player->kartstuff[k_offroad] = 16;

		if (player->kartstuff[k_offroad] > 0)
		{
			if (kartweight < 1) { kartweight = 1; } if (kartweight > 9) { kartweight = 9; } // Safety Net

			// 1872 is the magic number - 35 frames adds up to approximately 65536. 1872/4 = 468/3 = 156
			// A higher kart weight means you can stay offroad for longer without losing speed
			offroad = (1872 + 5*156 - kartweight*156)*offroadstrength;

			//if (player->kartstuff[k_growshrinktimer] > 1) // grow slows down half as fast
			//	offroad /= 2;

			player->kartstuff[k_offroad] += offroad;
		}

		if (player->kartstuff[k_offroad] > FRACUNIT*offroadstrength)
			player->kartstuff[k_offroad] = FRACUNIT*offroadstrength;
	}
	else
		player->kartstuff[k_offroad] = 0;
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

	if (player->kartstuff[k_respawn] > 3)
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
				newx = player->mo->x + P_ReturnThrustX(player->mo, newangle, 31*player->mo->scale);
				newy = player->mo->y + P_ReturnThrustY(player->mo, newangle, 31*player->mo->scale);
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
					mo->momz = (8*FRACUNIT)*P_MobjFlip(player->mo);
				}
			}
		}
	}

	if (player->kartstuff[k_respawn] > 0 && player->kartstuff[k_respawn] <= 3)
	{
		if (!P_IsObjectOnGround(player->mo))
		{
			player->powers[pw_flashing] = 2;
			// If you tried to boost while in the air,
			// you lose your chance of boosting at all.
			if (cmd->buttons & BT_ACCELERATE)
			{
				player->powers[pw_flashing] = 0;
				player->kartstuff[k_respawn] = 0;
			}
		}
		else
		{
			player->kartstuff[k_respawn]--;
			// Quick! You only have three tics to boost!
			if (cmd->buttons & BT_ACCELERATE)
				K_DoSneaker(player, true);
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
	else if (player->speed > FixedMul(player->runspeed, player->mo->scale))
	{
		if (cmd->driftturn < 0 && !(player->mo->state >= &states[S_KART_RUN1_R] && player->mo->state <= &states[S_KART_RUN2_R]))
			P_SetPlayerMobjState(player->mo, S_KART_RUN1_R);
		else if (cmd->driftturn > 0 && !(player->mo->state >= &states[S_KART_RUN1_L] && player->mo->state <= &states[S_KART_RUN2_L]))
			P_SetPlayerMobjState(player->mo, S_KART_RUN1_L);
		else if (cmd->driftturn == 0 && !(player->mo->state >= &states[S_KART_RUN1] && player->mo->state <= &states[S_KART_RUN2]))
			P_SetPlayerMobjState(player->mo, S_KART_RUN1);
	}
	// Walk frames - S_KART_WALK1   S_KART_WALK1_L   S_KART_WALK1_R
	else if (player->speed <= FixedMul(player->runspeed, player->mo->scale))
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

static void K_PlayTauntSound(mobj_t *source)
{
#if 1
	sfxenum_t pick = P_RandomKey(4); // Gotta roll the RNG every time this is called for sync reasons
	boolean tasteful = (!source->player || !source->player->kartstuff[k_tauntvoices]);

	if (cv_kartvoices.value && (tasteful || cv_kartvoices.value == 2))
		S_StartSound(source, sfx_taunt1+pick);

	if (!tasteful)
		return;

	K_TauntVoiceTimers(source->player);
#else
	if (source->player && source->player->kartstuff[k_tauntvoices]) // Prevents taunt sounds from playing every time the button is pressed
		return;

	S_StartSound(source, sfx_taunt1+P_RandomKey(4));

	K_TauntVoiceTimers(source->player);
#endif
}

static void K_PlayOvertakeSound(mobj_t *source)
{
#if 1
	boolean tasteful = (!source->player || !source->player->kartstuff[k_voices]);

	if (!G_RaceGametype()) // Only in race
		return;

	// 4 seconds from before race begins, 10 seconds afterwards
	if (leveltime < starttime+(10*TICRATE))
		return;

	if (cv_kartvoices.value && (tasteful || cv_kartvoices.value == 2))
		S_StartSound(source, sfx_slow);

	if (!tasteful)
		return;

	K_RegularVoiceTimers(source->player);
#else
	if (source->player && source->player->kartstuff[k_voices]) // Prevents taunt sounds from playing every time the button is pressed
		return;

	if (!G_RaceGametype()) // Only in race
		return;

	// 4 seconds from before race begins, 10 seconds afterwards
	if (leveltime < starttime+(10*TICRATE))
		return;

	S_StartSound(source, sfx_slow);

	K_RegularVoiceTimers(source->player);
#endif
}

static void K_PlayHitEmSound(mobj_t *source)
{
	if (cv_kartvoices.value)
		S_StartSound(source, sfx_hitem);

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

// if speed is true it gets the speed boost power, otherwise it gets the acceleration
static fixed_t K_GetKartBoostPower(player_t *player, boolean speed)
{
	fixed_t boostpower = FRACUNIT;
	fixed_t boostvalue = 0;

	if (player->kartstuff[k_spinouttimer] && player->kartstuff[k_wipeoutslow] == 1) // Slow down after you've been bumped
		return 0;

	// Offroad is separate, it's difficult to factor it in with a variable value anyway.
	if (!(player->kartstuff[k_invincibilitytimer] || player->kartstuff[k_hyudorotimer] || player->kartstuff[k_sneakertimer])
		&& player->kartstuff[k_offroad] >= 0 && speed)
		boostpower = FixedDiv(boostpower, player->kartstuff[k_offroad] + FRACUNIT);

	if (player->kartstuff[k_bananadrag] > TICRATE)
		boostpower = 4*boostpower/5;

	if (player->kartstuff[k_growshrinktimer] > 0)
	{												// Grow
		if (speed)
		{
			boostvalue = max(boostvalue, FRACUNIT/5); // + 20%
		}
	}
	if (player->kartstuff[k_invincibilitytimer])
	{												// Invincibility
		if (speed)
			boostvalue = max(boostvalue, 3*(FRACUNIT/8)); // + 37.5%
		else
			boostvalue = max(boostvalue, 3*FRACUNIT); // + 600%
	}
	if (player->kartstuff[k_driftboost])
	{												// Drift Boost
		if (speed)
			boostvalue = max(boostvalue, FRACUNIT/4); // + 25%
		else
			boostvalue = max(boostvalue, 4*FRACUNIT); // + 400%
	}
	if (player->kartstuff[k_sneakertimer])
	{												// Sneaker
		if (speed)
		{
			switch (gamespeed)
			{
				case 0:
					boostvalue = max(boostvalue, 53740+768);
					break;
				case 2:
					boostvalue = max(boostvalue, 17294+768);
					break;
				default:
					boostvalue = max(boostvalue, 32768);
					break;
			}
		}
		else
			boostvalue = max(boostvalue, 8*FRACUNIT); // + 800%
	}
	// don't average them anymore, this would make a small boost and a high boost less useful
	// just take the highest we want instead

	return boostpower + boostvalue;
}

fixed_t K_GetKartSpeed(player_t *player, boolean doboostpower)
{
	fixed_t k_speed = 150;
	fixed_t g_cc = FRACUNIT;
	fixed_t xspd = 3072;		// 4.6875 aka 3/64
	UINT8 kartspeed = player->kartspeed;
	fixed_t finalspeed;

	if (doboostpower && !player->kartstuff[k_pogospring] && !P_IsObjectOnGround(player->mo))
		return (75*mapheaderinfo[gamemap-1]->mobj_scale); // air speed cap

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
		return FixedMul(finalspeed, K_GetKartBoostPower(player, true));
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

	return FixedMul(k_accel, K_GetKartBoostPower(player, false));
}

UINT16 K_GetKartFlashing(player_t *player)
{
    UINT16 tics = flashingtics;
    if (G_BattleGametype())
        tics *= 2;
    tics += (flashingtics/6) * (player->kartspeed-5); // when weight is buffed in battle, use this instead: (player->kartspeed - player->kartweight)
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
		const fixed_t scale = mapheaderinfo[gamemap-1]->mobj_scale + abs(player->mo->scale - mapheaderinfo[gamemap-1]->mobj_scale);
		const fixed_t minspeed = 24*scale;
		const fixed_t maxspeed = 36*scale;

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

void K_SpinPlayer(player_t *player, mobj_t *source, INT32 type, boolean trapitem)
{
	UINT8 scoremultiply = 1;
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
		K_DoInstashield(player);
		return;
	}

	if (source && source != player->mo && source->player)
		K_PlayHitEmSound(source);

	//player->kartstuff[k_sneakertimer] = 0;
	player->kartstuff[k_driftboost] = 0;

	if (G_BattleGametype())
	{
		if (source && source->player && player != source->player)
		{
			P_AddPlayerScore(source->player, scoremultiply);
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

		K_CheckBumpers();
	}

	player->kartstuff[k_comebacktimer] = comebacktime;

	player->kartstuff[k_spinouttype] = type;

	if (player->kartstuff[k_spinouttype] <= 0)
	{
		player->kartstuff[k_spinouttimer] = 3*TICRATE/2; // Banana Spinout

		// At spinout, player speed is increased to 1/4 their regular speed, moving them forward
		if (player->speed < K_GetKartSpeed(player, true)/4)
			P_InstaThrust(player->mo, player->mo->angle, FixedMul(K_GetKartSpeed(player, true)/4, player->mo->scale));

		S_StartSound(player->mo, sfx_slip);
	}
	else
		player->kartstuff[k_spinouttimer] = TICRATE+20; // Wipeout

	player->powers[pw_flashing] = K_GetKartFlashing(player);

	if (player->mo->state != &states[S_KART_SPIN])
		P_SetPlayerMobjState(player->mo, S_KART_SPIN);

	player->kartstuff[k_instashield] = 15;
	return;
}

void K_SquishPlayer(player_t *player, mobj_t *source)
{
	UINT8 scoremultiply = 1;
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
		K_DoInstashield(player);
		return;
	}

	player->kartstuff[k_sneakertimer] = 0;
	player->kartstuff[k_driftboost] = 0;

	if (G_BattleGametype())
	{
		if (source && source->player && player != source->player)
		{
			P_AddPlayerScore(source->player, scoremultiply);
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

		K_CheckBumpers();
	}

	player->kartstuff[k_comebacktimer] = comebacktime;

	player->kartstuff[k_squishedtimer] = 2*TICRATE;

	player->powers[pw_flashing] = K_GetKartFlashing(player);

	player->mo->flags |= MF_NOCLIP;

	if (player->mo->state != &states[S_KART_SQUISH]) // Squash
		P_SetPlayerMobjState(player->mo, S_KART_SQUISH);

	P_PlayRinglossSound(player->mo);

	player->kartstuff[k_instashield] = 15;
	return;
}

void K_ExplodePlayer(player_t *player, mobj_t *source) // A bit of a hack, we just throw the player up higher here and extend their spinout timer
{
	UINT8 scoremultiply = 1;
	if (G_BattleGametype())
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
		K_DoInstashield(player);
		return;
	}

	player->mo->momz = 18*(mapheaderinfo[gamemap-1]->mobj_scale);
	player->mo->momx = player->mo->momy = 0;

	player->kartstuff[k_sneakertimer] = 0;
	player->kartstuff[k_driftboost] = 0;

	if (G_BattleGametype())
	{
		if (source && source->player && player != source->player)
		{
			P_AddPlayerScore(source->player, scoremultiply);
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

		K_CheckBumpers();
	}

	player->kartstuff[k_comebacktimer] = comebacktime;

	player->kartstuff[k_spinouttype] = 1;
	player->kartstuff[k_spinouttimer] = 2*TICRATE+(TICRATE/2);

	player->powers[pw_flashing] = K_GetKartFlashing(player);

	if (player->mo->state != &states[S_KART_SPIN])
		P_SetPlayerMobjState(player->mo, S_KART_SPIN);

	P_PlayRinglossSound(player->mo);

	if (P_IsLocalPlayer(player))
	{
		quake.intensity = 64*FRACUNIT;
		quake.time = 5;
	}

	player->kartstuff[k_instashield] = 15;
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

	if (netgame)
	{
		if (player->kartstuff[k_bumper] <= 0)
			CONS_Printf(M_GetText("%s is back in the game!\n"), player_names[player-players]);
		else if (cv_hazardlog.value)
			CONS_Printf(M_GetText("%s stole a bumper from %s!\n"), player_names[player-players], player_names[victim-players]);
	}

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

		mobj->flags |= MF_NOCLIPTHING;
		mobj->flags &= ~MF_SPECIAL;

		P_SetTarget(&mobj->target, source);
	}
}

// Spawns the purely visual explosion
void K_SpawnMineExplosion(mobj_t *source, UINT8 color)
{
	INT32 i, radius, height;
	mobj_t *smoldering = P_SpawnMobj(source->x, source->y, source->z, MT_SMOLDERING);
	mobj_t *dust;
	mobj_t *truc;
	INT32 speed, speed2;

	smoldering->tics = TICRATE*3;
	radius = source->radius>>FRACBITS;
	height = source->height>>FRACBITS;

	if (!color)
		color = SKINCOLOR_RED;

	for (i = 0; i < 32; i++)
	{
		dust = P_SpawnMobj(source->x, source->y, source->z, MT_SMOKE);
		dust->angle = (ANGLE_180/16) * i;
		dust->scale = source->scale;
		dust->destscale = source->scale*10;
		P_InstaThrust(dust, dust->angle, FixedMul(20*FRACUNIT, source->scale));

		truc = P_SpawnMobj(source->x + P_RandomRange(-radius, radius)*FRACUNIT,
			source->y + P_RandomRange(-radius, radius)*FRACUNIT,
			source->z + P_RandomRange(0, height)*FRACUNIT, MT_BOOMEXPLODE);
		truc->scale = source->scale*2;
		truc->destscale = source->scale*6;
		speed = FixedMul(10*FRACUNIT, source->scale)>>FRACBITS;
		truc->momx = P_RandomRange(-speed, speed)*FRACUNIT;
		truc->momy = P_RandomRange(-speed, speed)*FRACUNIT;
		speed = FixedMul(20*FRACUNIT, source->scale)>>FRACBITS;
		truc->momz = P_RandomRange(-speed, speed)*FRACUNIT;
		truc->color = color;
	}

	for (i = 0; i < 16; i++)
	{
		dust = P_SpawnMobj(source->x + P_RandomRange(-radius, radius)*FRACUNIT,
			source->y + P_RandomRange(-radius, radius)*FRACUNIT,
			source->z + P_RandomRange(0, height)*FRACUNIT, MT_SMOKE);
		dust->scale = source->scale;
		dust->destscale = source->scale*10;
		dust->tics = 30;
		dust->momz = P_RandomRange(FixedMul(3*FRACUNIT, source->scale)>>FRACBITS, FixedMul(7*FRACUNIT, source->scale)>>FRACBITS)*FRACUNIT;

		truc = P_SpawnMobj(source->x + P_RandomRange(-radius, radius)*FRACUNIT,
			source->y + P_RandomRange(-radius, radius)*FRACUNIT,
			source->z + P_RandomRange(0, height)*FRACUNIT, MT_BOOMPARTICLE);
		truc->scale = source->scale;
		truc->destscale = source->scale*5;
		speed = FixedMul(20*FRACUNIT, source->scale)>>FRACBITS;
		truc->momx = P_RandomRange(-speed, speed)*FRACUNIT;
		truc->momy = P_RandomRange(-speed, speed)*FRACUNIT;
		speed = FixedMul(15*FRACUNIT, source->scale)>>FRACBITS;
		speed2 = FixedMul(45*FRACUNIT, source->scale)>>FRACBITS;
		truc->momz = P_RandomRange(speed, speed2)*FRACUNIT;
		if (P_RandomChance(FRACUNIT/2))
			truc->momz = -truc->momz;
		truc->tics = TICRATE*2;
		truc->color = color;
	}
}

static mobj_t *K_SpawnKartMissile(mobj_t *source, mobjtype_t type, angle_t angle, INT32 flags2, fixed_t speed)
{
	mobj_t *th;
	angle_t an;
	fixed_t x, y, z;
	fixed_t finalspeed = speed;
	mobj_t *throwmo;
	//INT32 dir;

	// angle at which you fire, is player angle
	an = angle;

	//if (source->player->kartstuff[k_throwdir] != 0)
	//	dir = source->player->kartstuff[k_throwdir];
	//else
	//	dir = 1;

	if (source->player && source->player->speed > K_GetKartSpeed(source->player, false))
		finalspeed = FixedMul(speed, FixedDiv(source->player->speed, K_GetKartSpeed(source->player, false)));

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

#ifdef WEAPON_SFX
	//Since rail and bounce have no thrown objects, this hack is necessary.
	//Is creating thrown objects for rail and bounce more or less desirable than this?
	if (th->info->seesound && !(th->flags2 & MF2_RAILRING) && !(th->flags2 & MF2_SCATTER))
		S_StartSound(source, th->info->seesound);
#else
	if (th->info->seesound)
		S_StartSound(source, th->info->seesound);
#endif

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

	if (type == MT_ORBINAUT)
	{
		if (source && source->player)
			th->color = source->player->skincolor;
		else
			th->color = SKINCOLOR_CLOUDY;
	}
	else if (type == MT_JAWZ || type == MT_JAWZ_DUD)
	{
		S_StartSound(th, th->info->activesound);
	}

	x = x + P_ReturnThrustX(source, an, source->radius + th->radius);
	y = y + P_ReturnThrustY(source, an, source->radius + th->radius);
	throwmo = P_SpawnMobj(x, y, z, MT_FIREDITEM);
	throwmo->movecount = 1;
	throwmo->movedir = source->angle - an;
	P_SetTarget(&throwmo->target, source);

	return NULL;
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
#ifdef ESLOPE
		if (player->mo->standingslope)
		{
			ground = P_GetZAt(player->mo->standingslope, newx, newy);
			if (player->mo->eflags & MFE_VERTICALFLIP)
				ground -= FixedMul(mobjinfo[MT_SNEAKERTRAIL].height, player->mo->scale);
		}
#endif
		flame = P_SpawnMobj(newx, newy, ground, MT_SNEAKERTRAIL);

		P_SetTarget(&flame->target, player->mo);
		flame->angle = travelangle;
		flame->fuse = TICRATE*2;
		flame->destscale = player->mo->scale;
		P_SetScale(flame, player->mo->scale);
		flame->eflags = (flame->eflags & ~MFE_VERTICALFLIP)|(player->mo->eflags & MFE_VERTICALFLIP);

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

		//if (i == 0)
			//P_SetMobjState(sparkle, S_KARTINVULN_LARGE1);

		P_SetTarget(&sparkle->target, mo);
		sparkle->destscale = mo->destscale;
		P_SetScale(sparkle, mo->scale);
		sparkle->eflags = (sparkle->eflags & ~MFE_VERTICALFLIP)|(mo->eflags & MFE_VERTICALFLIP);
		sparkle->color = mo->color;
		//sparkle->colorized = mo->colorized;
	}

	P_SetMobjState(sparkle, S_KARTINVULN_LARGE1);
}

void K_SpawnWipeoutTrail(mobj_t *mo, boolean translucent)
{
	mobj_t *dust;

	I_Assert(mo != NULL);
	I_Assert(!P_MobjWasRemoved(mo));

	dust = P_SpawnMobj(mo->x + (P_RandomRange(-25,25)<<FRACBITS), mo->y + (P_RandomRange(-25,25)<<FRACBITS), mo->z, MT_WIPEOUTTRAIL);

	P_SetTarget(&dust->target, mo);
	dust->angle = R_PointToAngle2(0,0,mo->momx,mo->momy);
	dust->destscale = mo->scale;
	P_SetScale(dust, mo->scale);
	dust->eflags = (dust->eflags & ~MFE_VERTICALFLIP)|(mo->eflags & MFE_VERTICALFLIP);

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
			anglediff = abs(spawner->angle - spawner->player->frameangle);
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

			anglediff = abs(playerangle - R_PointToAngle2(0, 0, spawner->player->rmomx, spawner->player->rmomy));
		}
	}
	else
	{
		if (P_AproxDistance(spawner->momx, spawner->momy) < 5<<FRACBITS)
			return;

		anglediff = abs(spawner->angle - R_PointToAngle2(0, 0, spawner->momx, spawner->momy));
	}

	if (anglediff > ANGLE_180)
		anglediff = InvAngle(anglediff);

	if (anglediff > ANG10*4) // Trying to turn further than 40 degrees
	{
		fixed_t spawnx = P_RandomRange(-spawnrange, spawnrange)<<FRACBITS;
		fixed_t spawny = P_RandomRange(-spawnrange, spawnrange)<<FRACBITS;
		INT32 speedrange = 2;
		mobj_t *dust = P_SpawnMobj(spawner->x + spawnx, spawner->y + spawny, spawner->z, MT_DRIFTDUST);
		if (spawner->eflags & MFE_VERTICALFLIP)
		{
			dust->z += spawner->height - dust->height;
		}
		dust->momx = FixedMul(spawner->momx + (P_RandomRange(-speedrange, speedrange)<<FRACBITS), 3*FRACUNIT/4);
		dust->momy = FixedMul(spawner->momy + (P_RandomRange(-speedrange, speedrange)<<FRACBITS), 3*FRACUNIT/4);
		dust->momz = P_MobjFlip(spawner) * P_RandomRange(1, 4)<<FRACBITS;
		dust->scale = spawner->scale/2;
		dust->destscale = spawner->scale * 3;

		if (leveltime % 6 == 0)
			S_StartSound(spawner, sfx_screec);

		// Now time for a bunch of flag shit, groooooaann...
		if (spawner->flags2 & MF2_DONTDRAW)
			dust->flags2 |= MF2_DONTDRAW;
		else
			dust->flags2 &= ~MF2_DONTDRAW;

		if (spawner->eflags & MFE_VERTICALFLIP)
			dust->eflags |= MFE_VERTICALFLIP;
		else
			dust->eflags &= ~MFE_VERTICALFLIP;

		if (spawner->eflags & MFE_DRAWONLYFORP1)
			dust->eflags |= MFE_DRAWONLYFORP1;
		else
			dust->eflags &= ~MFE_DRAWONLYFORP1;

		if (spawner->eflags & MFE_DRAWONLYFORP2)
			dust->eflags |= MFE_DRAWONLYFORP2;
		else
			dust->eflags &= ~MFE_DRAWONLYFORP2;

		if (spawner->eflags & MFE_DRAWONLYFORP3)
			dust->eflags |= MFE_DRAWONLYFORP3;
		else
			dust->eflags &= ~MFE_DRAWONLYFORP3;

		if (spawner->eflags & MFE_DRAWONLYFORP4)
			dust->eflags |= MFE_DRAWONLYFORP4;
		else
			dust->eflags &= ~MFE_DRAWONLYFORP4;
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

static mobj_t *K_ThrowKartItem(player_t *player, boolean missile, mobjtype_t mapthing, INT32 defaultDir, boolean minethrow)
{
	mobj_t *mo;
	INT32 dir, PROJSPEED;
	angle_t newangle;
	fixed_t newx, newy, newz;
	mobj_t *throwmo;

	if (!player)
		return NULL;

	// Figure out projectile speed by game speed
	switch (gamespeed)
	{
		case 0:
			PROJSPEED = 68*(mapheaderinfo[gamemap-1]->mobj_scale); // Avg Speed is 34
			break;
		case 2:
			PROJSPEED = 96*(mapheaderinfo[gamemap-1]->mobj_scale); // Avg Speed is 48
			break;
		default:
			PROJSPEED = 82*(mapheaderinfo[gamemap-1]->mobj_scale); // Avg Speed is 41
			break;
	}

	if (minethrow)
	{
		if (player->kartstuff[k_throwdir] == 1)
			dir = 2;
		else if (player->kartstuff[k_throwdir] == -1 && mapthing != MT_SINK)
			dir = -1;
		else
			dir = 1;
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
				mo = K_SpawnKartMissile(player->mo, mapthing, player->mo->angle + ANGLE_180 - 0x06000000, 0, PROJSPEED/2);
				K_SpawnKartMissile(player->mo, mapthing, player->mo->angle + ANGLE_180 - 0x03000000, 0, PROJSPEED/2);
				K_SpawnKartMissile(player->mo, mapthing, player->mo->angle + ANGLE_180, 0, PROJSPEED/2);
				K_SpawnKartMissile(player->mo, mapthing, player->mo->angle + ANGLE_180 + 0x03000000, 0, PROJSPEED/2);
				K_SpawnKartMissile(player->mo, mapthing, player->mo->angle + ANGLE_180 + 0x06000000, 0, PROJSPEED/2);
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
			if (dir == -1)
			{
				// Shoot backward
				mo = K_SpawnKartMissile(player->mo, mapthing, player->mo->angle + ANGLE_180, 0, PROJSPEED/2);
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

		if (dir == 1 || dir == 2)
		{
			// Shoot forward
			mo = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z + player->mo->height/2, mapthing);

			mo->threshold = 10;
			P_SetTarget(&mo->target, player->mo);

			S_StartSound(player->mo, mo->info->seesound);

			if (mo)
			{
				angle_t fa = player->mo->angle>>ANGLETOFINESHIFT;
				INT32 HEIGHT;

				if (dir == 2)
					HEIGHT = 40*(mapheaderinfo[gamemap-1]->mobj_scale) + player->mo->momz;
				else
					HEIGHT = 30*(mapheaderinfo[gamemap-1]->mobj_scale) + player->mo->momz;

				mo->momx = player->mo->momx + FixedMul(FINECOSINE(fa), PROJSPEED);
				mo->momy = player->mo->momy + FixedMul(FINESINE(fa), PROJSPEED);
				mo->momz = P_MobjFlip(player->mo) * HEIGHT;

				if (player->mo->eflags & MFE_VERTICALFLIP)
					mo->eflags |= MFE_VERTICALFLIP;
			}

			throwmo = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z + player->mo->height/2, MT_FIREDITEM);
			P_SetTarget(&throwmo->target, player->mo);
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

			mo = P_SpawnMobj(newx, newy, newz, mapthing);

			if (P_MobjFlip(player->mo) < 0)
				mo->z = player->mo->z + player->mo->height - mo->height;

			mo->threshold = 10;
			P_SetTarget(&mo->target, player->mo);

			if (P_IsObjectOnGround(player->mo))
			{
				// floorz and ceilingz aren't properly set to account for FOFs and Polyobjects on spawn
				// This should set it for FOFs
				P_TeleportMove(mo, mo->x, mo->y, mo->z);

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

			if (mo)
			{
				if (player->mo->eflags & MFE_VERTICALFLIP)
					mo->eflags |= MFE_VERTICALFLIP;
			}
		}
	}

	return mo;
}

static void K_DoThunderShield(player_t *player)
{
	S_StartSound(player->mo, sfx_s3k45);
	//player->kartstuff[k_thunderanim] = 35;
	P_NukeEnemies(player->mo, player->mo, RING_DIST/4);
}

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
			&& !players[i].kartstuff[k_itemheld]))
		{
			playerswappable[numplayers] = i;
			numplayers++;
		}
	}

	prandom = P_RandomFixed();
	S_StartSound(player->mo, sfx_s3k92);

	if (sink && numplayers > 0) // BEHOLD THE KITCHEN SINK
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

void K_DoSneaker(player_t *player, boolean doPFlag)
{
	if (!player->kartstuff[k_floorboost] || player->kartstuff[k_floorboost] == 3)
		S_StartSound(player->mo, sfx_cdfm01);

	player->kartstuff[k_sneakertimer] = sneakertime;

	if (doPFlag)
		player->pflags |= PF_ATTACKDOWN;

	K_PlayTauntSound(player->mo);
}

static void K_DoShrink(player_t *player)
{
	INT32 i;

	S_StartSound(player->mo, sfx_kc46); // Sound the BANG!
	player->pflags |= PF_ATTACKDOWN;

	for (i = 0; i < MAXPLAYERS; i++)
	{
		/*if (playeringame[i])
			P_FlashPal(&players[i], PAL_NUKE, 10);*/

		if (playeringame[i] && players[i].mo && !player->spectator
			&& players[i].kartstuff[k_position] < player->kartstuff[k_position])
			P_DamageMobj(players[i].mo, player->mo, player->mo, 64);
	}

	K_PlayTauntSound(player->mo);
}

static void K_DoSPB(player_t *victim, player_t *source)
{
	//INT32 i;
	S_StartSound(victim->mo, sfx_bkpoof); // Sound the BANG!

	/*for (i = 0; i < MAXPLAYERS; i++)
	{
		if (playeringame[i])
			P_FlashPal(&players[i], PAL_NUKE, 10);
	}*/

	if (victim->mo && !victim->spectator)
		P_DamageMobj(victim->mo, source->mo, source->mo, 65);
}

void K_DoPogoSpring(mobj_t *mo, fixed_t vertispeed, boolean mute)
{
	fixed_t scale = mapheaderinfo[gamemap-1]->mobj_scale + abs(mo->scale - mapheaderinfo[gamemap-1]->mobj_scale);

	if (mo->player && mo->player->spectator)
		return;

	if (mo->eflags & MFE_SPRUNG)
		return;

#ifdef ESLOPE
	mo->standingslope = NULL;
#endif

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

		mo->momz = FixedMul(FINESINE(ANGLE_22h>>ANGLETOFINESHIFT), FixedMul(thrust, scale));
	}
	else
		mo->momz = FixedMul(vertispeed, scale);

	if (!mute)
		S_StartSound(mo, sfx_kc2f);
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

static void K_MoveHeldObjects(player_t *player)
{
	if (!player->mo)
		return;

	if (!player->mo->hnext)
	{
		player->kartstuff[k_bananadrag] = 0;
		return;
	}

	if (P_MobjWasRemoved(player->mo->hnext))
	{
		// we need this here too because this is done in afterthink - pointers are cleaned up at the START of each tic...
		P_SetTarget(&player->mo->hnext, NULL);
		player->kartstuff[k_bananadrag] = 0;
		return;
	}

	switch (player->mo->hnext->type)
	{
		case MT_ORBINAUT_SHIELD: // Kart orbit items
		case MT_JAWZ_SHIELD:
			{
				mobj_t *cur = player->mo->hnext;

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
					cur->angle += FixedAngle(cur->info->speed);

					// If the player is on the ceiling, then flip your items as well.
					if (player && player->mo->eflags & MFE_VERTICALFLIP)
						cur->eflags |= MFE_VERTICALFLIP;
					else
						cur->eflags &= ~MFE_VERTICALFLIP;

					// Shrink your items if the player shrunk too.
					P_SetScale(cur, (cur->destscale = player->mo->scale));

					if (P_MobjFlip(cur) > 0)
						z = player->mo->z;
					else
						z = player->mo->z + player->mo->height - cur->height;

					cur->flags |= MF_NOCLIPTHING; // temporarily make them noclip other objects so they can't hit anyone while in the player
					P_TeleportMove(cur, player->mo->x, player->mo->y, z);
					cur->momx = FixedMul(FINECOSINE(cur->angle>>ANGLETOFINESHIFT), radius);
					cur->momy = FixedMul(FINESINE(cur->angle>>ANGLETOFINESHIFT), radius);
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
					cur->z = z;
					cur->momx = cur->momy = 0;
					cur->angle += ANGLE_90;

					cur = cur->hnext;
				}
			}
			break;
		case MT_BANANA_SHIELD: // Kart trailing items
		case MT_SSMINE_SHIELD:
		case MT_FAKESHIELD:
			{
				mobj_t *cur = player->mo->hnext;
				mobj_t *targ = player->mo;

				if (P_IsObjectOnGround(player->mo) && player->speed > 0)
				{
					player->kartstuff[k_bananadrag]++;
					if (player->kartstuff[k_bananadrag] > TICRATE)
					{
						K_SpawnWipeoutTrail(player->mo, true);
						if (leveltime % 6 == 0)
							S_StartSound(player->mo, sfx_cdfm70);
					}
				}

				while (cur && !P_MobjWasRemoved(cur))
				{
					const fixed_t radius = FixedHypot(targ->radius, targ->radius) + FixedHypot(cur->radius, cur->radius);
					angle_t ang;
					fixed_t targx;
					fixed_t targy;
					fixed_t targz;
					fixed_t speed;
					fixed_t dist = radius/2;

					if (!cur->health)
					{
						cur = cur->hnext;
						continue;
					}

					if (cur != player->mo->hnext)
					{
						targ = cur->hprev;
						dist = radius/4;
					}

					if (!targ || P_MobjWasRemoved(targ))
						continue;

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
		default:
			break;
	}
}

/**	\brief	Decreases various kart timers and powers per frame. Called in P_PlayerThink in p_user.c

	\param	player	player object passed from P_PlayerThink
	\param	cmd		control input from player

	\return	void
*/
void K_KartPlayerThink(player_t *player, ticcmd_t *cmd)
{
	K_UpdateOffroad(player);

	// setting players to use the star colormap and spawning afterimages
	if (player->kartstuff[k_invincibilitytimer])
	{
		mobj_t *ghost;
		player->mo->colorized = true;
		ghost = P_SpawnGhostMobj(player->mo);
		ghost->fuse = 4;
		ghost->frame |= FF_FULLBRIGHT;
	}
	else if (player->kartstuff[k_growshrinktimer] != 0)
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

	if (player->kartstuff[k_spinouttimer] == 0 && player->powers[pw_flashing] == K_GetKartFlashing(player))
		player->powers[pw_flashing]--;

	/*if (player->kartstuff[k_thunderanim])
		player->kartstuff[k_thunderanim]--;*/

	if (player->kartstuff[k_sneakertimer])
		player->kartstuff[k_sneakertimer]--;

	if (player->kartstuff[k_floorboost])
		player->kartstuff[k_floorboost]--;

	if (player->kartstuff[k_driftboost])
		player->kartstuff[k_driftboost]--;

	if (player->kartstuff[k_invincibilitytimer])
		player->kartstuff[k_invincibilitytimer]--;

	if (player->kartstuff[k_growshrinktimer] > 0)
		player->kartstuff[k_growshrinktimer]--;

	if (player->kartstuff[k_growshrinktimer] < 0)
		player->kartstuff[k_growshrinktimer]++;

	if (player->kartstuff[k_growshrinktimer] == 1 || player->kartstuff[k_growshrinktimer] == -1)
	{
		if (player->kartstuff[k_invincibilitytimer] == 0)
			player->mo->color = player->skincolor;
		player->mo->destscale = mapheaderinfo[gamemap-1]->mobj_scale;
		if (cv_kartdebugshrink.value && !player->bot)
			player->mo->destscale = 6*player->mo->destscale/8;
		P_RestoreMusic(player);
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

	if (player->kartstuff[k_deathsentence])
	{
		if (player->kartstuff[k_deathsentence] == 1)
			K_DoSPB(player, &players[spbplayer]);
		player->kartstuff[k_deathsentence]--;
	}

	if (player->kartstuff[k_lapanimation])
		player->kartstuff[k_lapanimation]--;

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
				player->powers[pw_flashing] = 0;
				eggsexplode = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_BLUEEXPLOSION);
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

	if (P_IsObjectOnGround(player->mo) && player->mo->momz <= 0 && player->kartstuff[k_pogospring])
		player->kartstuff[k_pogospring] = 0;

	if (cmd->buttons & BT_DRIFT)
		player->kartstuff[k_jmp] = 1;
	else
		player->kartstuff[k_jmp] = 0;

	// Respawn Checker
	if (player->kartstuff[k_respawn])
		K_RespawnChecker(player);

	// Roulette Code
	K_KartItemRoulette(player, cmd);

	// Stopping of the horrible invincibility SFX
	if (player->mo->health <= 0 || player->mo->player->kartstuff[k_invincibilitytimer] <= 0
		|| player->mo->player->kartstuff[k_growshrinktimer] > 0) 	// If you don't have invincibility (or grow is active too)
	{
		if (S_SoundPlaying(player->mo, sfx_kinvnc)) 					// But the sound is playing
			S_StopSoundByID(player->mo, sfx_kinvnc); 					// Stop it
	}

	// And the same for the grow SFX
	if (!(player->mo->health > 0 && player->mo->player->kartstuff[k_growshrinktimer] > 0)) // If you aren't big
	{
		if (S_SoundPlaying(player->mo, sfx_kgrow)) // But the sound is playing
			S_StopSoundByID(player->mo, sfx_kgrow); // Stop it
	}

	// AAAAAAND handle the invincibility alarm
	if (player->mo->health > 0 && (player->mo->player->kartstuff[k_invincibilitytimer] > 0
		|| player->mo->player->kartstuff[k_growshrinktimer] > 0))
	{
		if (leveltime % 13 == 0 && cv_kartinvinsfx.value && !P_IsLocalPlayer(player))
			S_StartSound(player->mo, sfx_smkinv);
	}
	else if (S_SoundPlaying(player->mo, sfx_smkinv))
		S_StopSoundByID(player->mo, sfx_smkinv);

	// Plays the music after the starting countdown.
	if (P_IsLocalPlayer(player) && leveltime == (starttime + (TICRATE/2)))
		S_ChangeMusicInternal(mapmusname, true);
}

void K_KartPlayerAfterThink(player_t *player)
{
	if (player->kartstuff[k_invincibilitytimer]
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

fixed_t K_GetKartDriftSparkValue(player_t *player)
{
	UINT8 kartspeed = (G_BattleGametype() && player->kartstuff[k_bumper] <= 0)
		? 1
		: player->kartspeed;
	return (26*4 + kartspeed*2 + (9 - player->kartweight))*8;
}

static void K_KartDrift(player_t *player, boolean onground)
{
	fixed_t dsone = K_GetKartDriftSparkValue(player);
	fixed_t dstwo = dsone*2;

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
		player->kartstuff[k_driftboost] = 20;
		S_StartSound(player->mo, sfx_s23c);
		player->kartstuff[k_driftcharge] = 0;
	}
	else if ((player->kartstuff[k_drift] != -5 && player->kartstuff[k_drift] != 5)
		// || (player->kartstuff[k_drift] >= 1 && player->kartstuff[k_turndir] != 1) || (player->kartstuff[k_drift] <= -1 && player->kartstuff[k_turndir] != -1))
		&& player->kartstuff[k_driftcharge] >= dstwo
		&& onground)
	{
		player->kartstuff[k_driftboost] = 50;
		S_StartSound(player->mo, sfx_s23c);
		player->kartstuff[k_driftcharge] = 0;
	}

	// Drifting: left or right?
	if ((player->cmd.driftturn > 0) && player->speed > FixedMul(10<<16, player->mo->scale) && player->kartstuff[k_jmp] == 1
		&& (player->kartstuff[k_drift] == 0 || player->kartstuff[k_driftend] == 1)) // && player->kartstuff[k_drift] != 1)
	{
		// Starting left drift
		player->kartstuff[k_drift] = 1;
		player->kartstuff[k_driftend] = 0;
		player->kartstuff[k_driftcharge] = 0;
	}
	else if ((player->cmd.driftturn < 0) && player->speed > FixedMul(10<<16, player->mo->scale) && player->kartstuff[k_jmp] == 1
		&& (player->kartstuff[k_drift] == 0 || player->kartstuff[k_driftend] == 1)) // && player->kartstuff[k_drift] != -1)
	{
		// Starting right drift
		player->kartstuff[k_drift] = -1;
		player->kartstuff[k_driftend] = 0;
		player->kartstuff[k_driftcharge] = 0;
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
				driftadditive += (player->cmd.driftturn/800)/8;
			if (player->cmd.driftturn < 0) // Outward
				driftadditive -= (player->cmd.driftturn/800)/8;
		}
		else if (player->kartstuff[k_drift] <= -1) // Drifting to the right
		{
			player->kartstuff[k_drift]--;
			if (player->kartstuff[k_drift] < -5)
				player->kartstuff[k_drift] = -5;

			if (player->cmd.driftturn < 0) // Inward
				driftadditive += (player->cmd.driftturn/800)/4;
			if (player->cmd.driftturn > 0) // Outward
				driftadditive -= (player->cmd.driftturn/800)/4;
		}

		// This spawns the drift sparks
		if (player->kartstuff[k_driftcharge] < dsone && player->kartstuff[k_driftcharge] + driftadditive >= dsone)
			P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_DRIFT)->target = player->mo;

		player->kartstuff[k_driftcharge] += driftadditive;
		player->kartstuff[k_driftend] = 0;
	}

	// Stop drifting
	if (player->kartstuff[k_spinouttimer] > 0 // banana peel
		|| player->speed < FixedMul(10<<16, player->mo->scale)) // you're too slow!
	{
		player->kartstuff[k_drift] = 0;
		player->kartstuff[k_driftcharge] = 0;
	}
}
//
// K_KartUpdatePosition
//
static void K_KartUpdatePosition(player_t *player)
{
	fixed_t position = 1;
	fixed_t oldposition = player->kartstuff[k_position];
	fixed_t i, ppcd, pncd, ipcd, incd;
	fixed_t pmo, imo;
	thinker_t *th;
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
				for (th = thinkercap.next; th != &thinkercap; th = th->next)
				{
					if (th->function.acp1 != (actionf_p1)P_MobjThinker)	// Not a mobj at all, shoo
						continue;

					mo = (mobj_t *)th;

					if (mo->type != MT_BOSS3WAYPOINT) // TODO: Change to 'MT_WAYPOINT'?
						continue;

					pmo = P_AproxDistance(P_AproxDistance(	mo->x - player->mo->x,
															mo->y - player->mo->y),
															mo->z - player->mo->z) / FRACUNIT;
					imo = P_AproxDistance(P_AproxDistance(	mo->x - players[i].mo->x,
															mo->y - players[i].mo->y),
															mo->z - players[i].mo->z) / FRACUNIT;

					if (mo->health == player->starpostnum)
					{
						player->kartstuff[k_prevcheck] += pmo;
						ppcd++;
					}
					if (mo->health == (player->starpostnum + 1))
					{
						player->kartstuff[k_nextcheck] += pmo;
						pncd++;
					}
					if (mo->health == players[i].starpostnum)
					{
						players[i].kartstuff[k_prevcheck] += imo;
						ipcd++;
					}
					if (mo->health == (players[i].starpostnum + 1))
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
	player->kartstuff[k_itemtype] = 0;
	player->kartstuff[k_itemamount] = 0;
	player->kartstuff[k_itemheld] = 0;

	player->kartstuff[k_itemroulette] = 0;
	player->kartstuff[k_roulettetype] = 0;

	player->kartstuff[k_rocketsneakertimer] = 0;
	player->kartstuff[k_invincibilitytimer] = 0;
	player->kartstuff[k_growshrinktimer] = 0;

	player->kartstuff[k_eggmanheld] = 0;
	player->kartstuff[k_eggmanexplode] = 0;
	player->kartstuff[k_eggmanblame] = 0;

	player->kartstuff[k_hyudorotimer] = 0;
	player->kartstuff[k_stealingtimer] = 0;
	player->kartstuff[k_stolentimer] = 0;

	player->kartstuff[k_curshield] = 0;
	//player->kartstuff[k_thunderanim] = 0;
	player->kartstuff[k_bananadrag] = 0;

	player->kartstuff[k_sadtimer] = 0;
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

	if (player && player->mo && player->mo->health > 0 && !player->spectator && !(player->exiting || mapreset) && player->kartstuff[k_spinouttimer] == 0)
	{
		// First, the really specific, finicky items that function without the item being directly in your item slot.
		// Karma item dropping
		if (ATTACK_IS_DOWN && player->kartstuff[k_comebackmode] && !player->kartstuff[k_comebacktimer])
		{
			mobj_t *newitem;

			player->kartstuff[k_comebackmode] = 0;
			player->kartstuff[k_comebacktimer] = comebacktime;
			S_StartSound(player->mo, sfx_s254);

			newitem = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_RANDOMITEM);
			newitem->flags2 = (player->mo->flags2 & MF2_OBJECTFLIP);
			newitem->fuse = 15*TICRATE; // selected randomly.
			newitem->threshold = 69; // selected "randomly".
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
			K_ThrowKartItem(player, false, MT_FAKEITEM, -1, false);
			K_PlayTauntSound(player->mo);
			player->kartstuff[k_eggmanheld] = 0;
		}
		// Rocket Sneaker
		else if (ATTACK_IS_DOWN && !HOLDING_ITEM && onground && NO_HYUDORO
			&& player->kartstuff[k_rocketsneakertimer] > 1)
		{
			K_DoSneaker(player, true);
			player->kartstuff[k_rocketsneakertimer] -= 5;
			if (player->kartstuff[k_rocketsneakertimer] < 1)
				player->kartstuff[k_rocketsneakertimer] = 1;
		}
		else
		{
			switch (player->kartstuff[k_itemtype])
			{
				case KITEM_SNEAKER:
					if (ATTACK_IS_DOWN && !HOLDING_ITEM && onground && NO_HYUDORO)
					{
						K_DoSneaker(player, true);
						player->kartstuff[k_itemamount]--;
					}
					break;
				case KITEM_ROCKETSNEAKER:
					if (ATTACK_IS_DOWN && !HOLDING_ITEM && onground && NO_HYUDORO
						&& player->kartstuff[k_rocketsneakertimer] == 0)
					{
						K_DoSneaker(player, true);
						player->kartstuff[k_rocketsneakertimer] = itemtime;
						player->kartstuff[k_itemamount]--;
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
						if (!cv_kartinvinsfx.value && !P_IsLocalPlayer(player))
							S_StartSound(player->mo, sfx_kinvnc);
						K_PlayTauntSound(player->mo);
						player->kartstuff[k_itemamount]--;
					}
					break;
				case KITEM_BANANA:
					if (ATTACK_IS_DOWN && !HOLDING_ITEM && NO_HYUDORO)
					{
						INT32 moloop;
						mobj_t *mo;
						mobj_t *prev = player->mo;

						//K_PlayTauntSound(player->mo);
						player->kartstuff[k_itemheld] = 1;
						S_StartSound(player->mo, sfx_s254);

						for (moloop = 0; moloop < player->kartstuff[k_itemamount]; moloop++)
						{
							mo = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_BANANA_SHIELD);
							mo->threshold = 10;
							mo->movecount = player->kartstuff[k_itemamount];
							mo->lastlook = moloop+1;
							P_SetTarget(&mo->target, player->mo);
							P_SetTarget(&mo->hprev, prev);
							P_SetTarget(&prev->hnext, mo);
							prev = mo;
						}
					}
					else if (ATTACK_IS_DOWN && player->kartstuff[k_itemheld]) // Banana x3 thrown
					{
						K_ThrowKartItem(player, false, MT_BANANA, -1, false);
						K_PlayTauntSound(player->mo);
						player->kartstuff[k_itemamount]--;
						if (!player->kartstuff[k_itemamount])
							player->kartstuff[k_itemheld] = 0;
					}
					break;
				case KITEM_EGGMAN:
					if (ATTACK_IS_DOWN && !HOLDING_ITEM && NO_HYUDORO)
					{
						mobj_t *mo;
						player->kartstuff[k_itemamount]--;
						player->kartstuff[k_eggmanheld] = 1;
						S_StartSound(player->mo, sfx_s254);
						mo = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_FAKESHIELD);
						mo->threshold = 10;
						mo->movecount = 1;
						mo->lastlook = 1;
						if (mo)
						{
							P_SetTarget(&mo->target, player->mo);
							P_SetTarget(&player->mo->hnext, mo);
						}
					}
					break;
				case KITEM_ORBINAUT:
					if (ATTACK_IS_DOWN && !HOLDING_ITEM && NO_HYUDORO)
					{
						angle_t newangle;
						fixed_t newx;
						fixed_t newy;
						INT32 moloop;
						mobj_t *mo = NULL;
						mobj_t *prev = player->mo;

						//K_PlayTauntSound(player->mo);
						player->kartstuff[k_itemheld] = 1;
						S_StartSound(player->mo, sfx_s3k3a);

						for (moloop = 0; moloop < player->kartstuff[k_itemamount]; moloop++)
						{
							newangle = FixedAngle(((360/player->kartstuff[k_itemamount])*moloop)*FRACUNIT) + ANGLE_90;
							newx = player->mo->x + P_ReturnThrustX(player->mo, newangle, 64*FRACUNIT);
							newy = player->mo->y + P_ReturnThrustY(player->mo, newangle, 64*FRACUNIT);
							mo = P_SpawnMobj(newx, newy, player->mo->z, MT_ORBINAUT_SHIELD);
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
						K_ThrowKartItem(player, true, MT_ORBINAUT, 1, false);
						K_PlayTauntSound(player->mo);

						player->kartstuff[k_itemamount]--;
						if (!player->kartstuff[k_itemamount])
							player->kartstuff[k_itemheld] = 0;
					}
					break;
				case KITEM_JAWZ:
					if (ATTACK_IS_DOWN && !HOLDING_ITEM && NO_HYUDORO)
					{
						angle_t newangle;
						fixed_t newx;
						fixed_t newy;
						INT32 moloop;
						mobj_t *mo = NULL;
						mobj_t *prev = player->mo;

						//K_PlayTauntSound(player->mo);
						player->kartstuff[k_itemheld] = 1;
						S_StartSound(player->mo, sfx_s3k3a);

						for (moloop = 0; moloop < player->kartstuff[k_itemamount]; moloop++)
						{
							newangle = FixedAngle(((360/player->kartstuff[k_itemamount])*moloop)*FRACUNIT) + ANGLE_90;
							newx = player->mo->x + P_ReturnThrustX(player->mo, newangle, 64*FRACUNIT);
							newy = player->mo->y + P_ReturnThrustY(player->mo, newangle, 64*FRACUNIT);
							mo = P_SpawnMobj(newx, newy, player->mo->z, MT_JAWZ_SHIELD);
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
							K_ThrowKartItem(player, true, MT_JAWZ, 1, false);
						else if (player->kartstuff[k_throwdir] == -1) // Throwing backward gives you a dud that doesn't home in
							K_ThrowKartItem(player, true, MT_JAWZ_DUD, -1, false);
						K_PlayTauntSound(player->mo);

						player->kartstuff[k_itemamount]--;
						if (!player->kartstuff[k_itemamount])
							player->kartstuff[k_itemheld] = 0;
					}
					break;
				case KITEM_MINE:
					if (ATTACK_IS_DOWN && !HOLDING_ITEM && NO_HYUDORO)
					{
						mobj_t *mo;
						player->kartstuff[k_itemheld] = 1;
						S_StartSound(player->mo, sfx_s254);
						mo = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_SSMINE_SHIELD);
						mo->threshold = 10;
						mo->movecount = 1;
						mo->lastlook = 1;
						if (mo)
						{
							P_SetTarget(&mo->target, player->mo);
							P_SetTarget(&player->mo->hnext, mo);
						}
					}
					else if (ATTACK_IS_DOWN && player->kartstuff[k_itemheld])
					{
						K_ThrowKartItem(player, false, MT_SSMINE, 1, true);
						K_PlayTauntSound(player->mo);
						player->kartstuff[k_itemamount]--;
						player->kartstuff[k_itemheld] = 0;
					}
					break;
				case KITEM_BALLHOG:
					if (ATTACK_IS_DOWN && !HOLDING_ITEM && NO_HYUDORO)
					{
						player->kartstuff[k_itemamount]--;
						K_ThrowKartItem(player, true, MT_BALLHOG, 1, false);
						S_StartSound(player->mo, sfx_mario7);
						K_PlayTauntSound(player->mo);
					}
					break;
				case KITEM_SPB:
					if (ATTACK_IS_DOWN && !HOLDING_ITEM && NO_HYUDORO)
					{
						UINT8 ploop;
						UINT8 bestrank = 0;
						fixed_t dist = 0;

						for (ploop = 0; ploop < MAXPLAYERS; ploop++)
						{
							fixed_t thisdist;
							if (!playeringame[ploop] || players[ploop].spectator)
								continue;
							if (&players[ploop] == player)
								continue;
							if (!players[ploop].mo)
								continue;
							if (players[ploop].exiting)
								continue;
							thisdist = R_PointToDist2(player->mo->x, player->mo->y, players[ploop].mo->x, players[ploop].mo->y);
							if (bestrank == 0 || players[ploop].kartstuff[k_position] < bestrank)
							{
								bestrank = players[ploop].kartstuff[k_position];
								dist = thisdist;
							}
						}

						if (dist == 0)
							spbincoming = 6*TICRATE; // If you couldn't find anyone, just set an abritary timer
						else
							spbincoming = (tic_t)max(1, FixedDiv(dist, 64*FRACUNIT)/FRACUNIT);

						spbplayer = player-players;

						player->kartstuff[k_itemamount]--;

						K_PlayTauntSound(player->mo);
					}
					break;
				case KITEM_GROW:
					if (ATTACK_IS_DOWN && !HOLDING_ITEM && NO_HYUDORO
						&& player->kartstuff[k_growshrinktimer] <= 0) // Grow holds the item box hostage
					{
						K_PlayTauntSound(player->mo);
						player->mo->scalespeed = FRACUNIT/TICRATE;
						player->mo->destscale = 3*(mapheaderinfo[gamemap-1]->mobj_scale)/2;
						if (cv_kartdebugshrink.value && !player->bot)
							player->mo->destscale = 6*player->mo->destscale/8;
						player->kartstuff[k_growshrinktimer] = itemtime+(4*TICRATE); // 12 seconds
						P_RestoreMusic(player);
						if (!cv_kartinvinsfx.value && !P_IsLocalPlayer(player))
							S_StartSound(player->mo, sfx_kgrow);
						S_StartSound(player->mo, sfx_kc5a);
						player->kartstuff[k_itemamount]--;
					}
					break;
				case KITEM_SHRINK:
					if (ATTACK_IS_DOWN && !HOLDING_ITEM && NO_HYUDORO)
					{
						K_DoShrink(player);
						player->kartstuff[k_itemamount]--;
					}
					break;
				case KITEM_THUNDERSHIELD:
					if (player->kartstuff[k_curshield] <= 0)
					{
						mobj_t *shield = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_THUNDERSHIELD);
						P_SetTarget(&shield->target, player->mo);
						player->kartstuff[k_curshield] = 1;
					}
					if (ATTACK_IS_DOWN && !HOLDING_ITEM && NO_HYUDORO)
					{
						K_DoThunderShield(player);
						player->kartstuff[k_itemamount]--;
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
						K_PlayTauntSound(player->mo);
						K_DoPogoSpring(player->mo, 32<<FRACBITS, false);
						player->kartstuff[k_pogospring] = 1;
						player->kartstuff[k_itemamount]--;
					}
					break;
				case KITEM_KITCHENSINK:
					if (ATTACK_IS_DOWN && !HOLDING_ITEM && NO_HYUDORO)
					{
						K_ThrowKartItem(player, false, MT_SINK, 1, true);
						K_PlayTauntSound(player->mo);
						player->kartstuff[k_itemamount]--;
						player->kartstuff[k_itemheld] = 0;
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
		if (!player->kartstuff[k_itemamount] && !player->kartstuff[k_itemheld])
			player->kartstuff[k_itemtype] = KITEM_NONE;

		if (player->kartstuff[k_itemtype] != KITEM_THUNDERSHIELD)
			player->kartstuff[k_curshield] = 0;

		if (player->kartstuff[k_itemtype] == KITEM_SPB
			|| player->kartstuff[k_itemtype] == KITEM_SHRINK
			|| player->kartstuff[k_growshrinktimer] < 0
			|| spbincoming)
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
					if (player == &players[secondarydisplayplayer])
						player->mo->eflags |= MFE_DRAWONLYFORP2;
					else if (player == &players[thirddisplayplayer] && splitscreen > 1)
						player->mo->eflags |= MFE_DRAWONLYFORP3;
					else if (player == &players[fourthdisplayplayer] && splitscreen > 2)
						player->mo->eflags |= MFE_DRAWONLYFORP4;
					else
						player->mo->eflags |= MFE_DRAWONLYFORP1;
				}
				else
					player->mo->eflags &= ~(MFE_DRAWONLYFORP1|MFE_DRAWONLYFORP2|MFE_DRAWONLYFORP3|MFE_DRAWONLYFORP4);
			}
			else
			{
				if (player == &players[displayplayer]
					|| (player != &players[displayplayer] && (player->kartstuff[k_hyudorotimer] < (1*TICRATE/2) || player->kartstuff[k_hyudorotimer] > hyudorotime-(1*TICRATE/2))))
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
			K_StripItems(player);
			player->mo->flags2 |= MF2_SHADOW;
			player->powers[pw_flashing] = player->kartstuff[k_comebacktimer];
		}
		else if (G_RaceGametype() || player->kartstuff[k_bumper] > 0)
		{
			player->mo->flags2 &= ~MF2_SHADOW;
		}
	}

	if (player->kartstuff[k_growshrinktimer] > 1)
		player->powers[pw_flashing] = 2;

	// Friction
	if (player->speed > 0 && cmd->forwardmove == 0 && player->mo->friction == 59392)
		player->mo->friction += 4608;
	if (player->speed > 0 && cmd->forwardmove < 0 && player->mo->friction == 59392)
		player->mo->friction += 1608;
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
	if (player->kartstuff[k_spinouttimer] && player->kartstuff[k_wipeoutslow])
	{
		player->mo->friction -= FixedMul(1228, player->kartstuff[k_offroad]);
		if (player->kartstuff[k_wipeoutslow] == 1)
			player->mo->friction -= 4912;
	}

	K_KartDrift(player, onground);

	// Quick Turning
	// You can't turn your kart when you're not moving.
	// So now it's time to burn some rubber!
	if (player->speed < 2 && leveltime > starttime && cmd->buttons & BT_ACCELERATE && cmd->buttons & BT_BRAKE && cmd->driftturn != 0)
	{
		if (leveltime % 20 == 0)
			S_StartSound(player->mo, sfx_mkslid);
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
	if (player == &players[displayplayer]) // Don't play louder in splitscreen
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
			player->kartstuff[k_boostcharge]++;
		else
			player->kartstuff[k_boostcharge] = 0;
	}

	// Increase your size while charging your engine.
	if (leveltime < starttime+10)
	{
		player->mo->destscale = (mapheaderinfo[gamemap-1]->mobj_scale) + (player->kartstuff[k_boostcharge]*131);
		if (cv_kartdebugshrink.value && !player->bot)
			player->mo->destscale = 6*player->mo->destscale/8;
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
			player->kartstuff[k_sneakertimer] = -((21*(player->kartstuff[k_boostcharge]*player->kartstuff[k_boostcharge]))/425)+131; // max time is 70, min time is 7; yay parabooolas
			if (!player->kartstuff[k_floorboost] || player->kartstuff[k_floorboost] == 3)
			{
				if (player->kartstuff[k_sneakertimer] >= 70)
					S_StartSound(player->mo, sfx_s25f); // Special sound for the perfect start boost!
				else if (player->kartstuff[k_sneakertimer] >= sneakertime)
					S_StartSound(player->mo, sfx_cdfm01); // Sneaker boost sound for big boost
				else
					S_StartSound(player->mo, sfx_s23c); // Drift boost sound for small boost
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
	UINT8 numingame = 0, numwanted = 0;
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

	if (numingame <= 2)
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
	UINT8 i, numingame = 0, numjoiners = 0;

	// Get the number of players in game, and the players to be de-spectated.
	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (!playeringame[i])
			continue;

		if (!players[i].spectator)
		{
			numingame++;
			if (gamestate != GS_LEVEL) // Allow if you're not in a level
                continue;
			if (players[i].exiting) // DON'T allow if anyone's exiting
				return;
			if (numingame < 2 || leveltime < starttime || mapreset) // Allow if the match hasn't started yet
                continue;
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

	// Reset the match if you're in an empty server
	if (!mapreset && gamestate == GS_LEVEL && leveltime >= starttime && (numingame < 2 && numingame+numjoiners >= 2))
	{
		S_ChangeMusicInternal("chalng", false); // COME ON
		mapreset = 3*TICRATE; // Even though only the server uses this for game logic, set for everyone for HUD in the future
	}

	// Finally, we can de-spectate everyone!
	for (i = 0; i < numjoiners; i++)
		P_SpectatorJoinGame(&players[respawnlist[i]]);
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
static patch_t *kp_lapstickernarrow;
static patch_t *kp_splitlapflag;
static patch_t *kp_bumpersticker;
static patch_t *kp_bumperstickerwide;
static patch_t *kp_karmasticker;
static patch_t *kp_splitkarmabomb;
static patch_t *kp_timeoutsticker;

static patch_t *kp_startcountdown[8];
static patch_t *kp_racefinish[2];

static patch_t *kp_positionnum[NUMPOSNUMS][NUMPOSFRAMES];
static patch_t *kp_winnernum[NUMPOSFRAMES];

static patch_t *kp_facenull;
static patch_t *kp_facefirst;
static patch_t *kp_facesecond;
static patch_t *kp_facethird;
static patch_t *kp_facefourth;

static patch_t *kp_rankbumper;
static patch_t *kp_ranknobumpers;

static patch_t *kp_battlewin;
static patch_t *kp_battlecool;
static patch_t *kp_battlelose;
static patch_t *kp_battlewait;
static patch_t *kp_battleinfo;
static patch_t *kp_wanted;

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

static patch_t *kp_spbwarning[2];
static patch_t *kp_eggnum[4];

static patch_t *kp_fpview[3];
static patch_t *kp_inputwheel[5];

static patch_t *kp_challenger[25];

static patch_t *kp_lapanim_lap[7];
static patch_t *kp_lapanim_final[11];
static patch_t *kp_lapanim_number[10][3];
static patch_t *kp_lapanim_emblem;

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

	kp_racefinish[0] = 			W_CachePatchName("K_FINA", PU_HUDGFX);
	kp_racefinish[1] = 			W_CachePatchName("K_FINB", PU_HUDGFX);

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

	kp_facenull = 				W_CachePatchName("K_PFACE0", PU_HUDGFX);
	kp_facefirst = 				W_CachePatchName("K_PFACE1", PU_HUDGFX);
	kp_facesecond = 			W_CachePatchName("K_PFACE2", PU_HUDGFX);
	kp_facethird = 				W_CachePatchName("K_PFACE3", PU_HUDGFX);
	kp_facefourth = 			W_CachePatchName("K_PFACE4", PU_HUDGFX);

	// Extra ranking icons
	kp_rankbumper =				W_CachePatchName("K_BLNICO", PU_HUDGFX);
	kp_ranknobumpers =			W_CachePatchName("K_NOBLNS", PU_HUDGFX);

	// Battle graphics
	kp_battlewin = 				W_CachePatchName("K_BWIN", PU_HUDGFX);
	kp_battlecool = 			W_CachePatchName("K_BCOOL", PU_HUDGFX);
	kp_battlelose = 			W_CachePatchName("K_BLOSE", PU_HUDGFX);
	kp_battlewait = 			W_CachePatchName("K_BWAIT", PU_HUDGFX);
	kp_battleinfo = 			W_CachePatchName("K_BINFO", PU_HUDGFX);
	kp_wanted = 				W_CachePatchName("K_WANTED", PU_HUDGFX);

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

	// SPB warning
	kp_spbwarning[0] = 			W_CachePatchName("K_SPBW1", PU_HUDGFX);
	kp_spbwarning[1] = 			W_CachePatchName("K_SPBW2", PU_HUDGFX);

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

	kp_lapanim_emblem = (patch_t *) W_CachePatchName("K_LAPE00", PU_HUDGFX);
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
INT32 SPBW_X, SPBW_Y;	// SPB warning
INT32 WANT_X, WANT_Y;	// Battle WANTED poster

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
	// Blue Shell warning
	SPBW_X = BASEVIDWIDTH/2;		// 270
	SPBW_Y = BASEVIDHEIGHT- 24;		// 176
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

		SPBW_Y = (BASEVIDHEIGHT/2)-8;

		WANT_X = BASEVIDWIDTH-8;
		WANT_Y = (BASEVIDHEIGHT/2)-12;

		if (splitscreen > 1)	// 3P/4P Small Splitscreen
		{
			ITEM_X = -9;
			ITEM_Y = -8;

			LAPS_X = 3;
			LAPS_Y = (BASEVIDHEIGHT/2)-13;

			POSI_X = (BASEVIDWIDTH/2)-3;

			STCD_X = BASEVIDWIDTH/4;

			MINI_X = (3*BASEVIDWIDTH/4);
			MINI_Y = (3*BASEVIDHEIGHT/4);

			SPBW_X = BASEVIDWIDTH/4;

			WANT_X = (BASEVIDWIDTH/2)-8;

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

	if (stplyr != &players[displayplayer])
	{
		if (splitscreen == 1 && stplyr == &players[secondarydisplayplayer])
		{
			splitflags |= V_SPLITSCREEN;
		}
		else if (splitscreen > 1)
		{
			if (stplyr == &players[thirddisplayplayer] || stplyr == &players[fourthdisplayplayer])
				splitflags |= V_SPLITSCREEN;
			if (stplyr == &players[secondarydisplayplayer] || stplyr == &players[fourthdisplayplayer])
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
	patch_t *localbg = ((splitscreen > 1) ? kp_itembg[2] : kp_itembg[0]);
	patch_t *localinv = ((splitscreen > 1) ? kp_invincibility[((leveltime % (6*3)) / 3) + 7] : kp_invincibility[(leveltime % (7*3)) / 3]);
	INT32 splitflags = K_calcSplitFlags(V_SNAPTOTOP|V_SNAPTOLEFT);

	if (stplyr->kartstuff[k_itemroulette])
	{
		switch((stplyr->kartstuff[k_itemroulette] % (13*3)) / 3)
		{
			// Each case is handled in threes, to give three frames of in-game time to see the item on the roulette
			case  0: localpatch = kp_sneaker[offset]; break;					// Sneaker
			case  1: localpatch = kp_banana[offset]; break;					// Banana
			case  2: localpatch = kp_orbinaut[3+offset]; break;				// Orbinaut
			case  3: localpatch = kp_mine[offset]; break;						// Mine
			case  4: localpatch = kp_grow[offset]; break;						// Grow
			case  5: localpatch = kp_hyudoro[offset]; break;					// Hyudoro
			case  6: localpatch = kp_rocketsneaker[offset]; break;			// Rocket Sneaker
			case  7: localpatch = kp_jawz[offset]; break;						// Jawz
			case  8: localpatch = kp_selfpropelledbomb[offset]; break;		// Self-Propelled Bomb
			case  9: localpatch = kp_shrink[offset]; break;					// Shrink
			case 10: localpatch = localinv; break;								// Invincibility
			case 11: localpatch = kp_eggman[offset]; break;					// Eggman Monitor
			case 12: localpatch = kp_ballhog[offset]; break;					// Ballhog
			case 13: localpatch = kp_thundershield[offset]; break;			// Thunder Shield
			//case 14: localpatch = kp_pogospring[offset]; break;				// Pogo Spring
			//case 15: localpatch = kp_kitchensink[offset]; break;				// Kitchen Sink
			default: break;
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
			else if (!(leveltime & 2))
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
			else if (!(leveltime & 1))
				localpatch = kp_nodraw;
		}
		else if (stplyr->kartstuff[k_rocketsneakertimer] > 1)
		{
			if (leveltime & 1)
				localpatch = kp_rocketsneaker[offset];
			else if (!(leveltime & 1))
				localpatch = kp_nodraw;
		}
		else if (stplyr->kartstuff[k_growshrinktimer] > 1)
		{
			if (leveltime & 1)
				localpatch = kp_grow[offset];
			else if (!(leveltime & 1))
				localpatch = kp_nodraw;
		}
		else if (stplyr->kartstuff[k_sadtimer] > 0)
		{
			if (leveltime & 2)
				localpatch = kp_sadface[offset];
			else if (!(leveltime & 2))
				localpatch = kp_nodraw;
		}
		else
		{
			if (!(stplyr->kartstuff[k_itemamount] || stplyr->kartstuff[k_itemheld]))
				return;

			switch(stplyr->kartstuff[k_itemtype])
			{
				case KITEM_SNEAKER:				localpatch = kp_sneaker[offset]; break;
				case KITEM_ROCKETSNEAKER:		localpatch = kp_rocketsneaker[offset]; break;
				case KITEM_INVINCIBILITY:		localpatch = localinv; localbg = kp_itembg[offset+1]; break;
				case KITEM_BANANA:				localpatch = kp_banana[offset]; break;
				case KITEM_EGGMAN:				localpatch = kp_eggman[offset]; break;
				case KITEM_ORBINAUT:
					localpatch = kp_orbinaut[(splitscreen > 1 ? 4
						: min(stplyr->kartstuff[k_itemamount]-1, 3))];
					break;
				case KITEM_JAWZ:				localpatch = kp_jawz[offset]; break;
				case KITEM_MINE:				localpatch = kp_mine[offset]; break;
				case KITEM_BALLHOG:				localpatch = kp_ballhog[offset]; break;
				case KITEM_SPB:					localpatch = kp_selfpropelledbomb[offset]; break;
				case KITEM_GROW:				localpatch = kp_grow[offset]; break;
				case KITEM_SHRINK:				localpatch = kp_shrink[offset]; break;
				case KITEM_THUNDERSHIELD:		localpatch = kp_thundershield[offset]; break;
				case KITEM_HYUDORO:				localpatch = kp_hyudoro[offset]; break;
				case KITEM_POGOSPRING:			localpatch = kp_pogospring[offset]; break;
				case KITEM_KITCHENSINK:			localpatch = kp_kitchensink[offset]; break;
				case KITEM_SAD:					localpatch = kp_sadface[offset]; break;
				default: return;
			}
		}
	}

	V_DrawScaledPatch(ITEM_X, ITEM_Y, V_HUDTRANS|splitflags, localbg);

	// Then, the numbers:
	if (stplyr->kartstuff[k_itemamount] > 1 && !stplyr->kartstuff[k_itemroulette])
	{
		V_DrawScaledPatch(ITEM_X, ITEM_Y, V_HUDTRANS|splitflags, kp_itemmulsticker[offset]);
		V_DrawScaledPatch(ITEM_X, ITEM_Y, V_HUDTRANS|splitflags, localpatch);
		if (splitscreen > 1)
			V_DrawString(ITEM_X+24, ITEM_Y+31, V_ALLOWLOWERCASE|V_HUDTRANS|splitflags, va("x%d", stplyr->kartstuff[k_itemamount]));
		else
		{
			V_DrawScaledPatch(ITEM_X+28, ITEM_Y+41, V_HUDTRANS|splitflags, kp_itemx);
			V_DrawKartString(ITEM_X+38, ITEM_Y+36, V_HUDTRANS|splitflags, va("%d", stplyr->kartstuff[k_itemamount]));
		}
	}
	else
		V_DrawScaledPatch(ITEM_X, ITEM_Y, V_HUDTRANS|splitflags, localpatch);

	// Meter for rocket sneaker, could be extended to work for any other timer item...
	if (stplyr->kartstuff[k_rocketsneakertimer])
	{
		const INT32 barlength = (splitscreen > 1 ? 12 : 2);
		const INT32 timer = stplyr->kartstuff[k_rocketsneakertimer]; // item's timer
		const INT32 max = itemtime; // timer's normal highest value
		INT32 length = min(barlength, (timer * barlength) / max);
		INT32 height = (splitscreen > 1 ? 1 : 2);
		INT32 x = (splitscreen > 1 ? 17 : 11), y = (splitscreen > 1 ? 27 : 35);

		V_DrawScaledPatch(ITEM_X+x, ITEM_Y+y, V_HUDTRANS|splitflags, kp_itemtimer[offset]);
		// The dark "AA" edges on the sides
		V_DrawFill(ITEM_X+x+1, ITEM_Y+y+1, length, height, 12);
		// The bar itself
		if (length >= 3)
		{
			if (height == 1)
				V_DrawFill(ITEM_X+x+2, ITEM_Y+y+1, length-1, height, 120);
			else
			{
				V_DrawFill(ITEM_X+x+2, ITEM_Y+y+1, length-1, height, 8);
				V_DrawFill(ITEM_X+x+2, ITEM_Y+y+1, length-1, height/2, 120);
			}
		}
	}

	// Quick Eggman numbers
	if (stplyr->kartstuff[k_eggmanexplode] > 1 /*&& stplyr->kartstuff[k_eggmanexplode] <= 3*TICRATE*/)
		V_DrawScaledPatch(ITEM_X+17, ITEM_Y+13, V_HUDTRANS|splitflags, kp_eggnum[min(3, G_TicsToSeconds(stplyr->kartstuff[k_eggmanexplode]))]);
}

static void K_drawKartTimestamp(void)
{
	// TIME_X = BASEVIDWIDTH-124;	// 196
	// TIME_Y = 6;					//   6

	INT32 TIME_XB, splitflags = V_HUDTRANS|K_calcSplitFlags(V_SNAPTOTOP|V_SNAPTORIGHT);
	tic_t drawtime = stplyr->realtime;

	if (cv_timelimit.value && timelimitintics > 0)
	{
		if (drawtime >= timelimitintics)
			drawtime = 0;
		else
			drawtime = timelimitintics - drawtime;
	}

	V_DrawScaledPatch(TIME_X, TIME_Y, splitflags, kp_timestickerwide);

	TIME_XB = TIME_X+33;

	if (drawtime/(60*TICRATE) < 100) // 99:99:99 only
	{
		// zero minute
		if (drawtime/(60*TICRATE) < 10)
		{
			V_DrawKartString(TIME_XB, TIME_Y+3, splitflags, va("0"));
			// minutes time       0 __ __
			V_DrawKartString(TIME_XB+12, TIME_Y+3, splitflags, va("%d", drawtime/(60*TICRATE)));
		}
		// minutes time       0 __ __
		else
			V_DrawKartString(TIME_XB, TIME_Y+3, splitflags, va("%d", drawtime/(60*TICRATE)));

		// apostrophe location     _'__ __
		V_DrawKartString(TIME_XB+24, TIME_Y+3, splitflags, va("'"));

		// zero second        _ 0_ __
		if ((drawtime/TICRATE % 60) < 10)
		{
			V_DrawKartString(TIME_XB+36, TIME_Y+3, splitflags, va("0"));
		// seconds time       _ _0 __
			V_DrawKartString(TIME_XB+48, TIME_Y+3, splitflags, va("%d", drawtime/TICRATE % 60));
		}
		// zero second        _ 00 __
		else
			V_DrawKartString(TIME_XB+36, TIME_Y+3, splitflags, va("%d", drawtime/TICRATE % 60));

		// quotation mark location    _ __"__
		V_DrawKartString(TIME_XB+60, TIME_Y+3, splitflags, va("\""));

		// zero tick          _ __ 0_
		if (G_TicsToCentiseconds(drawtime) < 10)
		{
			V_DrawKartString(TIME_XB+72, TIME_Y+3, splitflags, va("0"));
		// tics               _ __ _0
			V_DrawKartString(TIME_XB+84, TIME_Y+3, splitflags, va("%d", G_TicsToCentiseconds(drawtime)));
		}
		// zero tick          _ __ 00
		if (G_TicsToCentiseconds(drawtime) >= 10)
			V_DrawKartString(TIME_XB+72, TIME_Y+3, splitflags, va("%d", G_TicsToCentiseconds(drawtime)));
	}
	else if ((drawtime/TICRATE) & 1)
		V_DrawKartString(TIME_XB, TIME_Y+3, splitflags, va("99'59\"99"));

	if (modeattacking) // emblem time!
	{
		INT32 workx = TIME_XB + 96, worky = TIME_Y+18;
		SINT8 curemb = 0;
		patch_t *emblempic[3] = {NULL, NULL, NULL};
		UINT8 *emblemcol[3] = {NULL, NULL, NULL};

		emblem_t *emblem = M_GetLevelEmblems(gamemap);
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

						snprintf(targettext, 9, "%i:%02i.%02i",
							G_TicsToMinutes(timetoreach, false),
							G_TicsToSeconds(timetoreach),
							G_TicsToCentiseconds(timetoreach));

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

						targettext[8] = 0;
					}
					break;
				default:
					goto bademblem;
			}

			V_DrawRightAlignedString(workx, worky, splitflags, targettext);
			workx -= 69; // i SWEAR i wasn't aiming for this
			V_DrawSmallScaledPatch(workx + 4, worky, splitflags, W_CachePatchName("NEEDIT", PU_CACHE));

			break;

			bademblem:
			emblem = M_GetLevelEmblems(-1);
		}

		while (curemb--)
		{
			workx -= 16;
			V_DrawSmallMappedPatch(workx + 4, worky, splitflags, emblempic[curemb], emblemcol[curemb]);
		}
	}
}

static void K_DrawKartPositionNum(INT32 num)
{
	// POSI_X = BASEVIDWIDTH - 51;	// 269
	// POSI_Y = BASEVIDHEIGHT- 64;	// 136

	INT32 X = POSI_X;
	INT32 W = SHORT(kp_positionnum[0][0]->width);
	fixed_t scale = FRACUNIT;
	patch_t *localpatch = kp_positionnum[0][0];
	INT32 splitflags = K_calcSplitFlags(V_SNAPTOBOTTOM|V_SNAPTORIGHT);

	if (stplyr->kartstuff[k_positiondelay] || stplyr->exiting)
		scale *= 2;
	if (splitscreen)
		scale /= 2;

	W = FixedMul(W<<FRACBITS, scale)>>FRACBITS;

	// Special case for 0
	if (!num)
	{
		V_DrawFixedPatch(X<<FRACBITS, POSI_Y<<FRACBITS, scale, V_HUDTRANSHALF|splitflags, kp_positionnum[0][0], NULL);
		return;
	}

	I_Assert(num >= 0); // This function does not draw negative numbers

	// Draw the number
	while (num)
	{
		if (stplyr->exiting && num == 1) // 1st place winner? You get rainbows!!
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

		V_DrawFixedPatch(X<<FRACBITS, POSI_Y<<FRACBITS, scale, V_HUDTRANSHALF|splitflags, localpatch, NULL);

		X -= W;
		num /= 10;
	}
}

static boolean K_drawKartPositionFaces(void)
{
	// FACE_X = 15;				//  15
	// FACE_Y = 72;				//  72

	INT32 Y = FACE_Y+9; // +9 to offset where it's being drawn if there are more than one
	INT32 i, j, ranklines;
	boolean completed[MAXPLAYERS];
	INT32 rankplayer[MAXPLAYERS];
	INT32 bumperx, numplayersingame = 0;
	UINT8 *colormap;
	patch_t *localpatch = kp_facenull;

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

	for (j = 0; j < numplayersingame; j++)
	{
		for (i = 0; i < MAXPLAYERS; i++)
		{
			if (playeringame[i] && completed[i] == false && players[i].mo && !players[i].spectator
				&& (rankplayer[ranklines] < 0 || players[i].kartstuff[k_position] < players[rankplayer[ranklines]].kartstuff[k_position]))
			{
				rankplayer[ranklines] = i;
			}
		}
		i = rankplayer[ranklines];

		completed[i] = true;

		if (ranklines == 4)
			break; // Only draw the top 4 players

		ranklines++;
	}

	Y -= (9*ranklines);

	for (i = 0; i < ranklines; i++)
	{
		if (players[rankplayer[i]].spectator) continue; // Spectators are ignored
		if (!players[rankplayer[i]].mo) continue;

		bumperx = FACE_X+18;

		if (players[rankplayer[i]].mo->color)
		{
			colormap = R_GetTranslationColormap(players[rankplayer[i]].skin, players[rankplayer[i]].mo->color, GTC_CACHE);
			if (players[rankplayer[i]].mo->colorized)
				colormap = R_GetTranslationColormap(TC_RAINBOW, players[rankplayer[i]].mo->color, GTC_CACHE);
			else
				colormap = R_GetTranslationColormap(players[rankplayer[i]].skin, players[rankplayer[i]].mo->color, GTC_CACHE);

			V_DrawSmallMappedPatch(FACE_X, Y, V_HUDTRANS|V_SNAPTOLEFT, faceprefix[players[rankplayer[i]].skin], colormap);
			if (G_BattleGametype() && players[rankplayer[i]].kartstuff[k_bumper] > 0)
			{
				for (j = 0; j < players[rankplayer[i]].kartstuff[k_bumper]; j++)
				{
					V_DrawSmallMappedPatch(bumperx, Y+10, V_HUDTRANS|V_SNAPTOLEFT, kp_rankbumper, colormap);
					bumperx += 3;
				}
			}
		}

		// Draws the little number over the face
		switch (players[rankplayer[i]].kartstuff[k_position])
		{
			case 1: localpatch = kp_facefirst; break;
			case 2: localpatch = kp_facesecond; break;
			case 3: localpatch = kp_facethird; break;
			case 4: localpatch = kp_facefourth; break;
			default: break;
		}

		if (G_BattleGametype() && players[rankplayer[i]].kartstuff[k_bumper] <= 0)
			V_DrawSmallScaledPatch(FACE_X-2, Y, V_HUDTRANS|V_SNAPTOLEFT, kp_ranknobumpers);
		else
			V_DrawSmallScaledPatch(FACE_X, Y, V_HUDTRANS|V_SNAPTOLEFT, localpatch);

		Y += 18;
	}

	return false;
}

static void K_drawKartLaps(void)
{
	INT32 splitflags = K_calcSplitFlags(V_SNAPTOBOTTOM|V_SNAPTOLEFT);

	if (splitscreen > 1)
	{
		V_DrawScaledPatch(LAPS_X, LAPS_Y, V_HUDTRANS|splitflags, kp_splitlapflag);

		if (stplyr->exiting)
			V_DrawString(LAPS_X+13, LAPS_Y+1, V_HUDTRANS|splitflags, "FIN");
		else
			V_DrawString(LAPS_X+13, LAPS_Y+1, V_HUDTRANS|splitflags, va("%d/%d", stplyr->laps+1, cv_numlaps.value));
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
		convSpeed = FixedDiv(FixedMul(stplyr->speed, 142371), mapheaderinfo[gamemap-1]->mobj_scale)/FRACUNIT; // 2.172409058
		V_DrawKartString(SPDM_X, SPDM_Y, V_HUDTRANS|splitflags, va("%3d km/h", convSpeed));
	}
	else if (cv_kartspeedometer.value == 2) // Miles
	{
		convSpeed = FixedDiv(FixedMul(stplyr->speed, 88465), mapheaderinfo[gamemap-1]->mobj_scale)/FRACUNIT; // 1.349868774
		V_DrawKartString(SPDM_X, SPDM_Y, V_HUDTRANS|splitflags, va("%3d mph", convSpeed));
	}
	else if (cv_kartspeedometer.value == 3) // Fracunits
	{
		convSpeed = FixedDiv(stplyr->speed, mapheaderinfo[gamemap-1]->mobj_scale)/FRACUNIT;
		V_DrawKartString(SPDM_X, SPDM_Y, V_HUDTRANS|splitflags, va("%3d fu/t", convSpeed));
	}
}

static void K_drawKartBumpersOrKarma(void)
{
	UINT8 *colormap = R_GetTranslationColormap(TC_DEFAULT, stplyr->skincolor, 0);
	INT32 splitflags = K_calcSplitFlags(V_SNAPTOBOTTOM|V_SNAPTOLEFT);

	if (splitscreen > 1)
	{
		if (stplyr->kartstuff[k_bumper] <= 0)
		{
			V_DrawMappedPatch(LAPS_X, LAPS_Y-1, V_HUDTRANS|splitflags, kp_splitkarmabomb, colormap);
			V_DrawString(LAPS_X+13, LAPS_Y+1, V_HUDTRANS|splitflags, va("%d/3", stplyr->kartstuff[k_comebackpoints]));
		}
		else
		{
			V_DrawMappedPatch(LAPS_X, LAPS_Y-1, V_HUDTRANS|splitflags, kp_rankbumper, colormap);
			V_DrawString(LAPS_X+13, LAPS_Y+1, V_HUDTRANS|splitflags, va("%d/%d", stplyr->kartstuff[k_bumper], cv_kartbumpers.value));
		}
	}
	else
	{
		if (stplyr->kartstuff[k_bumper] <= 0)
		{
			V_DrawMappedPatch(LAPS_X, LAPS_Y, V_HUDTRANS|splitflags, kp_karmasticker, colormap);
			V_DrawKartString(LAPS_X+59, LAPS_Y+3, V_HUDTRANS|splitflags, va("%d/3", stplyr->kartstuff[k_comebackpoints]));
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

static void K_drawSPBWarning(void)
{
	patch_t *localpatch = kp_nodraw;
	INT32 splitflags = K_calcSplitFlags(V_SNAPTOBOTTOM);

	if (!(stplyr->kartstuff[k_deathsentence] > 0
		|| (spbincoming > 0 && spbincoming < 2*TICRATE && stplyr->kartstuff[k_position] == 1)))
		return;

	if (leveltime % 8 > 3)
		localpatch = kp_spbwarning[1];
	else
		localpatch = kp_spbwarning[0];

	V_DrawScaledPatch(SPBW_X, SPBW_Y, splitflags, localpatch);
}

fixed_t K_FindCheckX(fixed_t px, fixed_t py, angle_t ang, fixed_t mx, fixed_t my)
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

	if (mirrormode)
		x = 320-x;

	if (splitscreen > 1)
		x /= 2;

	return x;
}

static void K_drawKartWanted(void)
{
	UINT8 i, numwanted = 0;
	UINT8 *colormap = NULL;

	if (splitscreen) // Can't fit the poster on screen, sadly
	{
		if (K_IsPlayerWanted(stplyr) && leveltime % 10 > 3)
			V_DrawRightAlignedString(WANT_X, WANT_Y, K_calcSplitFlags(V_SNAPTOBOTTOM|V_SNAPTORIGHT|V_HUDTRANS|V_REDMAP), "WANTED");
		return;
	}

	for (i = 0; i < 4; i++)
	{
		if (battlewanted[i] == -1)
			break;
		numwanted++;
	}

	if (numwanted <= 0)
		return;

	if (battlewanted[0] != -1)
		colormap = R_GetTranslationColormap(0, players[battlewanted[0]].skincolor, GTC_CACHE);
	V_DrawFixedPatch(WANT_X<<FRACBITS, WANT_Y<<FRACBITS, FRACUNIT, V_HUDTRANS|V_SNAPTORIGHT|V_SNAPTOBOTTOM, kp_wanted, colormap);

	for (i = 0; i < numwanted; i++)
	{
		INT32 x = WANT_X+7, y = WANT_Y+20;
		fixed_t scale = FRACUNIT/2;
		player_t *p = &players[battlewanted[i]];

		if (battlewanted[i] == -1)
			break;

		if (numwanted == 1)
		{
			x++; //y++;
			scale = FRACUNIT;
		}
		else
		{
			if (i & 1)
				x += 18;
			if (i > 1)
				y += 17;
		}

		if (players[battlewanted[i]].skincolor == 0)
			V_DrawFixedPatch(x<<FRACBITS, y<<FRACBITS, scale, V_HUDTRANS|V_SNAPTORIGHT|V_SNAPTOBOTTOM, faceprefix[p->skin], NULL);
		else
		{
			colormap = R_GetTranslationColormap(TC_RAINBOW, p->skincolor, GTC_CACHE);
			V_DrawFixedPatch(x<<FRACBITS, y<<FRACBITS, scale, V_HUDTRANS|V_SNAPTORIGHT|V_SNAPTOBOTTOM, faceprefix[p->skin], colormap);
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

	if (camspin)
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

			colormap = R_GetTranslationColormap(TC_DEFAULT, players[i].mo->color, 0);
			V_DrawMappedPatch(x, CHEK_Y, V_HUDTRANS|splitflags, kp_check[pnum], colormap);
		}
	}
}

void K_LoadIconGraphics(char *facestr, INT32 skinnum)
{
	char namelump[9];

	// hack: make sure base face name is no more than 8 chars
	if (strlen(facestr) > 8)
		facestr[8] = '\0';
	strcpy(namelump, facestr); // copy base name

	iconprefix[skinnum] = W_CachePatchName(namelump, PU_HUDGFX);
	iconfreed[skinnum] = false;
}

#if 0 //unused
static void K_UnLoadIconGraphics(INT32 skinnum)
{
	Z_Free(iconprefix[skinnum]);
	iconfreed[skinnum] = true;
}
#endif

void K_ReloadSkinIconGraphics(void)
{
	INT32 i;

	for (i = 0; i < numskins; i++)
		K_LoadIconGraphics(skins[i].iconprefix, i);
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

	if (mirrormode)
		amnumxpos = -amnumxpos;

	amxpos = amnumxpos + ((x + AutomapPic->width/2 - (iconprefix[skin]->width/2))<<FRACBITS);
	amypos = amnumypos + ((y + AutomapPic->height/2 - (iconprefix[skin]->height/2))<<FRACBITS);

	// do we want this? it feels unnecessary. easier to just modify the amnumxpos?
	/*if (mirrormode)
	{
		flags |= V_FLIP;
		amxpos = -amnumxpos + ((x + AutomapPic->width/2 + (iconprefix[skin]->width/2))<<FRACBITS);
	}*/

	if (!mo->color) // 'default' color
		V_DrawSciencePatch(amxpos, amypos, flags, iconprefix[skin], FRACUNIT);
	else
	{
		UINT8 *colormap;
		if (mo->colorized)
			colormap = R_GetTranslationColormap(TC_RAINBOW, mo->color, 0);
		else
			colormap = R_GetTranslationColormap(skin, mo->color, 0);
		V_DrawFixedPatch(amxpos, amypos, FRACUNIT, flags, iconprefix[skin], colormap);
	}
}

static void K_drawKartMinimap(void)
{
	INT32 lumpnum;
	patch_t *AutomapPic;
	INT32 i = 0;
	INT32 x, y;
	INT32 minimaptrans, splitflags = (splitscreen ? 0 : V_SNAPTORIGHT);
	boolean dop1later = false;

	// Draw the HUD only when playing in a level.
	// hu_stuff needs this, unlike st_stuff.
	if (!(Playing() && gamestate == GS_LEVEL))
		return;

	if (stplyr != &players[displayplayer])
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
		minimaptrans = (splitscreen ? 10 : cv_kartminimap.value);
		if (timeinmap <= 113)
			minimaptrans = ((((INT32)timeinmap) - 105)*minimaptrans)/(113-105);
		if (!minimaptrans)
			return;
	}
	else
		return;

	minimaptrans = ((10-minimaptrans)<<FF_TRANSSHIFT);
	splitflags |= minimaptrans;

	if (mirrormode)
		V_DrawScaledPatch(x+(AutomapPic->width), y, splitflags|V_FLIP, AutomapPic);
	else
		V_DrawScaledPatch(x, y, splitflags, AutomapPic);

	if (!splitscreen)
	{
		splitflags &= ~minimaptrans;
		splitflags |= V_HUDTRANSHALF;
	}

	// let offsets transfer to the heads, too!
	if (mirrormode)
		x += SHORT(AutomapPic->leftoffset);
	else
		x -= SHORT(AutomapPic->leftoffset);
	y -= SHORT(AutomapPic->topoffset);

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
		dop1later = true;
	}
	else
	{
		for (i = MAXPLAYERS-1; i >= 0; i--)
		{
			if (!playeringame[i])
				continue;
			if (!players[i].mo || players[i].spectator)
				continue;

			if (!splitscreen && i == displayplayer)
			{
				dop1later = true; // Do displayplayer later
				continue;
			}

			if (G_BattleGametype() && players[i].kartstuff[k_bumper] <= 0)
				continue;
			if (players[i].kartstuff[k_hyudorotimer] > 0)
			{
				if (!((players[i].kartstuff[k_hyudorotimer] < 1*TICRATE/2
					|| players[i].kartstuff[k_hyudorotimer] > hyudorotime-(1*TICRATE/2))
					&& !(leveltime & 1)))
					continue;
			}

			K_drawKartMinimapHead(players[i].mo, x, y, splitflags, AutomapPic);
		}
	}

	if (!dop1later)
		return; // Don't need this

	splitflags &= ~V_HUDTRANSHALF;
	splitflags |= V_HUDTRANS;
	K_drawKartMinimapHead(stplyr->mo, x, y, splitflags, AutomapPic);
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

	if (splitscreen)
		V_DrawSmallScaledPatch(STCD_X - (SHORT(kp_startcountdown[pnum]->width)/4), STCD_Y - (SHORT(kp_startcountdown[pnum]->height)/4), splitflags, kp_startcountdown[pnum]);
	else
		V_DrawScaledPatch(STCD_X - (SHORT(kp_startcountdown[pnum]->width)/2), STCD_Y - (SHORT(kp_startcountdown[pnum]->height)/2), splitflags, kp_startcountdown[pnum]);
}

static void K_drawKartFinish(void)
{
	INT32 pnum = 0, splitflags = K_calcSplitFlags(0);

	if (!stplyr->kartstuff[k_cardanimation] || stplyr->kartstuff[k_cardanimation] >= 2*TICRATE)
		return;

	if ((stplyr->kartstuff[k_cardanimation] % (2*5)) / 5) // blink
		pnum = 1;

	if (splitscreen > 1)
	{
		V_DrawTinyScaledPatch(STCD_X - (SHORT(kp_racefinish[pnum]->width)/8), STCD_Y - (SHORT(kp_racefinish[pnum]->height)/8), splitflags, kp_racefinish[pnum]);
		return;
	}

	{
		INT32 scaleshift = (FRACBITS - splitscreen); // FRACUNIT or FRACUNIT/2
		INT32 x = ((vid.width<<FRACBITS)/vid.dupx), xval = (SHORT(kp_racefinish[pnum]->width)<<scaleshift);
		x = ((TICRATE - stplyr->kartstuff[k_cardanimation])*(xval > x ? xval : x))/TICRATE;

		if (splitscreen && stplyr == &players[secondarydisplayplayer])
			x = -x;

		V_DrawFixedPatch(x + (STCD_X<<FRACBITS) - (SHORT(kp_racefinish[pnum]->width)<<(scaleshift-1)),
			(STCD_Y<<FRACBITS) - (SHORT(kp_racefinish[pnum]->height)<<(scaleshift-1)),
			(1<<scaleshift),
			splitflags, kp_racefinish[pnum], NULL);
	}
}

static void K_drawBattleFullscreen(void)
{
	INT32 x = BASEVIDWIDTH/2;
	INT32 y = -64+(stplyr->kartstuff[k_cardanimation]); // card animation goes from 0 to 164, 164 is the middle of the screen
	INT32 splitflags = V_SNAPTOTOP; // I don't feel like properly supporting non-green resolutions, so you can have a misuse of SNAPTO instead
	fixed_t scale = FRACUNIT;

	if (splitscreen)
	{
		if ((splitscreen == 1 && stplyr == &players[secondarydisplayplayer])
			|| (splitscreen > 1 && (stplyr == &players[thirddisplayplayer]
			|| (stplyr == &players[fourthdisplayplayer] && splitscreen > 2))))
		{
			y = 232-(stplyr->kartstuff[k_cardanimation]/2);
			splitflags = V_SNAPTOBOTTOM;
		}
		else
			y = -32+(stplyr->kartstuff[k_cardanimation]/2);

		if (splitscreen > 1)
		{
			scale /= 2;

			if (stplyr == &players[secondarydisplayplayer]
				|| (stplyr == &players[fourthdisplayplayer] && splitscreen > 2))
				x = 3*BASEVIDWIDTH/4;
			else
				x = BASEVIDWIDTH/4;
		}
		else
		{
			if (stplyr == &players[secondarydisplayplayer])
				x = BASEVIDWIDTH-96;
			else
				x = 96;
		}
	}

	if (stplyr->exiting)
	{
		if (stplyr == &players[displayplayer])
			V_DrawFadeScreen(0xFF00, 16);
		if (stplyr->exiting < 6*TICRATE)
		{
			if (stplyr->kartstuff[k_position] == 1)
				V_DrawFixedPatch(x<<FRACBITS, y<<FRACBITS, scale, splitflags, kp_battlewin, NULL);
			else if (splitscreen < 2)
				V_DrawFixedPatch(x<<FRACBITS, y<<FRACBITS, scale, splitflags, (K_IsPlayerLosing(stplyr) ? kp_battlelose : kp_battlecool), NULL);
		}
		else
			K_drawKartFinish();
	}
	else if (stplyr->kartstuff[k_bumper] <= 0 && stplyr->kartstuff[k_comebacktimer] && comeback)
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
			if ((splitscreen == 1 && stplyr == &players[secondarydisplayplayer])
				|| (stplyr == &players[thirddisplayplayer] && splitscreen > 1)
				|| (stplyr == &players[fourthdisplayplayer] && splitscreen > 2))
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
			if (i == displayplayer)
				continue;
			if (playeringame[i] && !stplyr->spectator)
				return;
		}

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

	if (stplyr == &players[secondarydisplayplayer] && splitscreen)
		{ pn = pnum[1]; tn = turn[1]; dr = drift[1]; }
	else if (stplyr == &players[thirddisplayplayer] && splitscreen > 1)
		{ pn = pnum[2]; tn = turn[2]; dr = drift[2]; }
	else if (stplyr == &players[fourthdisplayplayer] && splitscreen > 2)
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
		if (stplyr->speed < FixedMul(stplyr->runspeed, stplyr->mo->scale) && (leveltime & 1) && !splitscreen)
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

	if (mirrormode)
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
		fixed_t dsone = K_GetKartDriftSparkValue(stplyr);
		fixed_t dstwo = dsone*2;

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

			if (mirrormode)
				x -= xoffs;
			else
				x += xoffs;
			if (!splitscreen)
				y += yoffs;
		}

		// drift sparks!
		if ((leveltime & 1) && (stplyr->kartstuff[k_driftcharge] >= dstwo))
			colmap = R_GetTranslationColormap(TC_RAINBOW, SKINCOLOR_TANGERINE, 0);
		else if ((leveltime & 1) && (stplyr->kartstuff[k_driftcharge] >= dsone))
			colmap = R_GetTranslationColormap(TC_RAINBOW, SKINCOLOR_SAPPHIRE, 0);
		else
#endif
		// invincibility/grow/shrink!
		if (stplyr->mo->colorized && stplyr->mo->color)
			colmap = R_GetTranslationColormap(TC_RAINBOW, stplyr->mo->color, 0);
	}

	V_DrawFixedPatch(x, y, scale, splitflags, kp_fpview[target], colmap);

	if (stplyr == &players[secondarydisplayplayer] && splitscreen)
		{ pnum[1] = pn; turn[1] = tn; drift[1] = dr; }
	else if (stplyr == &players[thirddisplayplayer] && splitscreen > 1)
		{ pnum[2] = pn; turn[2] = tn; drift[2] = dr; }
	else if (stplyr == &players[fourthdisplayplayer] && splitscreen > 2)
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
		colormap = R_GetTranslationColormap(0, stplyr->skincolor, 0);
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

	V_DrawScaledPatch(BASEVIDWIDTH/2 + (32*max(0, stplyr->kartstuff[k_lapanimation]-76)),
		64 - (32*max(0, progress-76)),
		0, kp_lapanim_emblem);

	if (stplyr->laps == (UINT8)(cv_numlaps.value - 1))
	{
		V_DrawScaledPatch(27 - (32*max(0, progress-76)),
			40,
			0, kp_lapanim_final[min(progress/2, 10)]);

		if (progress/2-12 >= 0)
		{
			V_DrawScaledPatch(194 + (32*max(0, progress-76)),
				40,
				0, kp_lapanim_lap[min(progress/2-12, 6)]);
		}
	}
	else
	{
		V_DrawScaledPatch(61 - (32*max(0, progress-76)),
			40,
			0, kp_lapanim_lap[min(progress/2, 6)]);

		if (progress/2-8 >= 0)
		{
			V_DrawScaledPatch(194 + (32*max(0, progress-76)),
				40,
				0, kp_lapanim_number[(((UINT32)stplyr->laps+1) / 10)][min(progress/2-8, 2)]);

			if (progress/2-10 >= 0)
			{
				V_DrawScaledPatch(221 + (32*max(0, progress-76)),
					40,
					0, kp_lapanim_number[(((UINT32)stplyr->laps+1) % 10)][min(progress/2-10, 2)]);
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
		LAPS_Y+3, V_SNAPTOBOTTOM|V_SNAPTORIGHT, "FREE PLAY");
}

static void K_drawCheckpointDebugger(void)
{
	if ((numstarposts/2 + stplyr->starpostnum) >= numstarposts)
		V_DrawString(8, 184, 0, va("Checkpoint: %d / %d (Can finish)", stplyr->starpostnum, numstarposts));
	else
		V_DrawString(8, 184, 0, va("Checkpoint: %d / %d (Skip: %d)", stplyr->starpostnum, numstarposts, (numstarposts/2 + stplyr->starpostnum)));
	V_DrawString(8, 192, 0, va("Waypoint dist: Prev %d, Next %d", stplyr->kartstuff[k_prevcheck], stplyr->kartstuff[k_nextcheck]));
}

void K_drawKartHUD(void)
{
	boolean isfreeplay = false;

	// Define the X and Y for each drawn object
	// This is handled by console/menu values
	K_initKartHUD();

	// Draw that fun first person HUD! Drawn ASAP so it looks more "real".
	if ((stplyr == &players[displayplayer] && !camera.chase)
		|| ((splitscreen && stplyr == &players[secondarydisplayplayer]) && !camera2.chase)
		|| ((splitscreen > 1 && stplyr == &players[thirddisplayplayer]) && !camera3.chase)
		|| ((splitscreen > 2 && stplyr == &players[fourthdisplayplayer]) && !camera4.chase))
		K_drawKartFirstPerson();

	// Draw a white fade on level opening
	if (leveltime < 15 && stplyr == &players[displayplayer])
	{
		if (leveltime <= 5)
			V_DrawFill(0,0,BASEVIDWIDTH,BASEVIDHEIGHT,120); // Pure white on first few frames, to hide SRB2's awful level load artifacts
		else
			V_DrawFadeScreen(120, 15-leveltime); // Then gradually fade out from there
	}

	if (splitscreen == 2) // Player 4 in 3P is the minimap :p
		K_drawKartMinimap();

	// Draw full screen stuff that turns off the rest of the HUD
	if (mapreset)
	{
		K_drawChallengerScreen();
		return;
	}

	if ((G_BattleGametype())
		&& (stplyr->exiting
		|| (stplyr->kartstuff[k_bumper] <= 0
		&& stplyr->kartstuff[k_comebacktimer]
		&& comeback
		&& stplyr->playerstate == PST_LIVE)))
	{
		K_drawBattleFullscreen();
		return;
	}

	// Draw the CHECK indicator before the other items, so it's overlapped by everything else
	if (cv_kartcheck.value && !splitscreen && !players[displayplayer].exiting)
		K_drawKartPlayerCheck();

	if (splitscreen == 0 && cv_kartminimap.value)
		K_drawKartMinimap(); // 3P splitscreen is handled above

	// Draw the item window
	K_drawKartItem();

	// Draw WANTED status
	if (G_BattleGametype())
		K_drawKartWanted();

	// If not splitscreen, draw...
	if (!splitscreen)
	{
		// Draw the timestamp
		K_drawKartTimestamp();

		if (!modeattacking)
		{
			// The top-four faces on the left
			isfreeplay = K_drawKartPositionFaces();
		}
	}

	if (!stplyr->spectator) // Bottom of the screen elements, don't need in spectate mode
	{
		if (G_RaceGametype()) // Race-only elements
		{
			// Draw the lap counter
			K_drawKartLaps();

			if (!splitscreen)
			{
				// Draw the speedometer
				// TODO: Make a better speedometer.
				K_drawKartSpeedometer();
			}

			if (isfreeplay)
				;
			else if (!modeattacking)
			{
				// Draw the numerical position
				K_DrawKartPositionNum(stplyr->kartstuff[k_position]);
			}
			else //if (!(demoplayback && hu_showscores))
			{
				// Draw the input UI
				K_drawInput();
			}

			// You're about to DIEEEEE
			K_drawSPBWarning();
		}
		else if (G_BattleGametype()) // Battle-only
		{
			// Draw the hits left!
			K_drawKartBumpersOrKarma();
		}
	}

	// Draw the countdowns after everything else.
	if (leveltime >= starttime-(3*TICRATE)
		&& leveltime < starttime+TICRATE)
		K_drawKartStartCountdown();
	else if (countdown && (!splitscreen || !stplyr->exiting))
	{
		char *countstr = va("%d", countdown/TICRATE);

		if (splitscreen > 1)
			V_DrawCenteredString(BASEVIDWIDTH/4, LAPS_Y+1, K_calcSplitFlags(0), countstr);
		else
		{
			INT32 karlen = strlen(countstr)*6; // half of 12
			V_DrawKartString((BASEVIDWIDTH/2)-karlen, LAPS_Y+3, K_calcSplitFlags(0), countstr);
		}
	}

	// Race overlays
	if (G_RaceGametype())
	{
		if (stplyr->exiting)
			K_drawKartFinish();
		else if (stplyr->kartstuff[k_lapanimation] && !splitscreen)
			K_drawLapStartAnim();
	}

	// Draw FREE PLAY.
	if (isfreeplay && !stplyr->spectator && timeinmap > 113)
		K_drawKartFreePlay(leveltime);

	if (cv_kartdebugcheckpoint.value)
		K_drawCheckpointDebugger();
}

//}

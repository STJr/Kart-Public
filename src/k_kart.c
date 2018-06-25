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
#include "k_kart.h"
#include "f_finale.h"

// SOME IMPORTANT VARIABLES DEFINED IN DOOMDEF.H:
// gamespeed is cc (0 for easy, 1 for normal, 2 for hard)
// franticitems is Frantic Mode items, bool
// mirrormode is Mirror Mode (duh), bool
// comeback is Battle Mode's karma comeback, also bool
// indirectitemcooldown is timer before anyone's allowed another Shrink/SPB
// spbincoming is the timer before k_deathsentence is cast on the player in 1st
// spbplayer is the last player who fired a SPB


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
	"Dark Grey",      // 06 // SKINCOLOR_DARKGREY
	"Black",          // 07 // SKINCOLOR_BLACK
	"Salmon",         // 08 // SKINCOLOR_SALMON
	"Pink",           // 09 // SKINCOLOR_PINK
	"Light Red",      // 10 // SKINCOLOR_LIGHTRED
	"Shiny Red",      // 11 // SKINCOLOR_SHINYRED
	"Red",            // 12 // SKINCOLOR_RED
	"Dark Pink",      // 13 // SKINCOLOR_DARKPINK
	"Dark Red",       // 14 // SKINCOLOR_DARKRED
	"Dawn",           // 15 // SKINCOLOR_DAWN
	"Orange",         // 16 // SKINCOLOR_ORANGE
	"Shiny Orange",   // 17 // SKINCOLOR_SHINYORANGE
	"Dark Orange",    // 18 // SKINCOLOR_DARKORANGE
	"Golden Brown",   // 19 // SKINCOLOR_GOLDENBROWN
	"Rosewood",       // 20 // SKINCOLOR_ROSEWOOD
	"Dark Rosewood",  // 21 // SKINCOLOR_DARKROSEWOOD
	"Sepia",          // 22 // SKINCOLOR_SEPIA
	"Beige",          // 23 // SKINCOLOR_BEIGE
	"Brown",          // 24 // SKINCOLOR_BROWN
	"Leather",        // 25 // SKINCOLOR_LEATHER
	"Yellow",         // 26 // SKINCOLOR_YELLOW
	"Peach",          // 27 // SKINCOLOR_PEACH
	"Light Orange",   // 28 // SKINCOLOR_LIGHTORANGE
	"Caramel",        // 29 // SKINCOLOR_CARAMEL
	"Gold",           // 30 // SKINCOLOR_GOLD
	"Shiny Caramel",  // 31 // SKINCOLOR_SHINYCARAMEL
	"Vomit",          // 32 // SKINCOLOR_VOMIT
	"Garden",         // 33 // SKINCOLOR_GARDEN
	"Light Army",     // 34 // SKINCOLOR_LIGHTARMY
	"Army",           // 35 // SKINCOLOR_ARMY
	"Pistachio",      // 36 // SKINCOLOR_PISTACHIO
	"Robo-Hood",      // 37 // SKINCOLOR_ROBOHOOD
	"Olive",          // 38 // SKINCOLOR_OLIVE
	"Dark Army",      // 39 // SKINCOLOR_DARKARMY
	"Light Green",    // 40 // SKINCOLOR_LIGHTGREEN
	"Ugly Green",     // 41 // SKINCOLOR_UGLYGREEN
	"Neon Green",     // 42 // SKINCOLOR_NEONGREEN
	"Green",          // 43 // SKINCOLOR_GREEN
	"Dark Green",     // 44 // SKINCOLOR_DARKGREEN
	"Swamp",          // 45 // SKINCOLOR_SWAMP
	"Frost",          // 46 // SKINCOLOR_FROST
	"Slate",          // 47 // SKINCOLOR_SLATE
	"Light Blue",     // 48 // SKINCOLOR_LIGHTBLUE
	"Cyan",           // 49 // SKINCOLOR_CYAN
	"Cerulean",       // 50 // SKINCOLOR_CERULEAN
	"Turquoise",      // 51 // SKINCOLOR_TURQUOISE
	"Teal",           // 52 // SKINCOLOR_TEAL
	"Steel Blue",     // 53 // SKINCOLOR_STEELBLUE
	"Blue",           // 54 // SKINCOLOR_BLUE
	"Shiny Blue",     // 55 // SKINCOLOR_SHINYBLUE
	"Navy",           // 56 // SKINCOLOR_NAVY
	"Dark Blue",      // 57 // SKINCOLOR_DARKBLUE
	"Jet Black",      // 58 // SKINCOLOR_JETBLACK
	"Lilac",          // 59 // SKINCOLOR_LILAC
	"Purple",         // 60 // SKINCOLOR_PURPLE
	"Lavender",       // 61 // SKINCOLOR_LAVENDER
	"Byzantium",      // 62 // SKINCOLOR_BYZANTIUM
	"Indigo"          // 63 // SKINCOLOR_INDIGO
};

// Color_Opposite replacement; frame setting has not been changed from 8 for most, should be done later
const UINT8 KartColor_Opposite[MAXSKINCOLORS*2] =
{
	SKINCOLOR_NONE,8,        // 00 // SKINCOLOR_NONE
	SKINCOLOR_BLACK,8,       // 01 // SKINCOLOR_IVORY
	SKINCOLOR_BLACK,8,       // 02 // SKINCOLOR_WHITE
	SKINCOLOR_DARKGREY,8,    // 03 // SKINCOLOR_SILVER
	SKINCOLOR_CLOUDY,8,      // 04 // SKINCOLOR_CLOUDY
	SKINCOLOR_GREY,8,        // 05 // SKINCOLOR_GREY
	SKINCOLOR_SILVER,8,      // 06 // SKINCOLOR_DARKGREY
	SKINCOLOR_WHITE,8,       // 07 // SKINCOLOR_BLACK
	SKINCOLOR_SWAMP,8,       // 08 // SKINCOLOR_SALMON
	SKINCOLOR_ARMY,8,        // 09 // SKINCOLOR_PINK
	SKINCOLOR_DARKGREEN,8,   // 10 // SKINCOLOR_LIGHTRED
	SKINCOLOR_UGLYGREEN,8,   // 11 // SKINCOLOR_SHINYRED
	SKINCOLOR_GREEN,8,       // 12 // SKINCOLOR_RED
	SKINCOLOR_DARKARMY,8,    // 13 // SKINCOLOR_DARKPINK
	SKINCOLOR_LIGHTGREEN,8,  // 14 // SKINCOLOR_DARKRED
	SKINCOLOR_NAVY,8,        // 15 // SKINCOLOR_DAWN
	SKINCOLOR_BLUE,8,        // 16 // SKINCOLOR_ORANGE
	SKINCOLOR_SHINYBLUE,8,   // 17 // SKINCOLOR_SHINYORANGE
	SKINCOLOR_LIGHTBLUE,8,   // 18 // SKINCOLOR_DARKORANGE
	SKINCOLOR_STEELBLUE,8,   // 19 // SKINCOLOR_GOLDENBROWN
	SKINCOLOR_STEELBLUE,8,   // 20 // SKINCOLOR_ROSEWOOD
	SKINCOLOR_SLATE,8,       // 21 // SKINCOLOR_DARKROSEWOOD
	SKINCOLOR_LEATHER,8,     // 22 // SKINCOLOR_SEPIA
	SKINCOLOR_BROWN,8,       // 23 // SKINCOLOR_BEIGE
	SKINCOLOR_BEIGE,8,       // 24 // SKINCOLOR_BROWN
	SKINCOLOR_SEPIA,8,       // 25 // SKINCOLOR_LEATHER
	SKINCOLOR_INDIGO,8,      // 26 // SKINCOLOR_YELLOW
	SKINCOLOR_BROWN,8,       // 27 // SKINCOLOR_PEACH
	SKINCOLOR_DARKBLUE,8,    // 28 // SKINCOLOR_LIGHTORANGE
	SKINCOLOR_SEPIA,8,       // 29 // SKINCOLOR_CARAMEL
	SKINCOLOR_LAVENDER,8,    // 30 // SKINCOLOR_GOLD
	SKINCOLOR_BEIGE,8,       // 31 // SKINCOLOR_SHINYCARAMEL
	SKINCOLOR_PURPLE,8,      // 32 // SKINCOLOR_VOMIT
	SKINCOLOR_BYZANTIUM,8,   // 33 // SKINCOLOR_GARDEN
	SKINCOLOR_LAVENDER,8,    // 34 // SKINCOLOR_LIGHTARMY
	SKINCOLOR_LAVENDER,8,    // 35 // SKINCOLOR_ARMY
	SKINCOLOR_PURPLE,8,      // 36 // SKINCOLOR_PISTACHIO
	SKINCOLOR_LILAC,8,       // 37 // SKINCOLOR_ROBOHOOD
	SKINCOLOR_LILAC,8,       // 38 // SKINCOLOR_OLIVE
	SKINCOLOR_LAVENDER,8,    // 39 // SKINCOLOR_DARKARMY
	SKINCOLOR_DARKRED,8,     // 40 // SKINCOLOR_LIGHTGREEN
	SKINCOLOR_SHINYRED,8,    // 41 // SKINCOLOR_UGLYGREEN
	SKINCOLOR_SHINYRED,8,    // 42 // SKINCOLOR_NEONGREEN
	SKINCOLOR_RED,8,         // 43 // SKINCOLOR_GREEN
	SKINCOLOR_LIGHTRED,8,    // 44 // SKINCOLOR_DARKGREEN
	SKINCOLOR_SALMON,8,      // 45 // SKINCOLOR_SWAMP
	SKINCOLOR_DARKRED,8,     // 46 // SKINCOLOR_FROST
	SKINCOLOR_PINK,8,        // 47 // SKINCOLOR_SLATE
	SKINCOLOR_LIGHTORANGE,8, // 48 // SKINCOLOR_LIGHTBLUE
	SKINCOLOR_CARAMEL,8,     // 49 // SKINCOLOR_CYAN
	SKINCOLOR_GOLD,8,        // 50 // SKINCOLOR_CERULEAN
	SKINCOLOR_SHINYCARAMEL,8,// 51 // SKINCOLOR_TURQUOISE
	SKINCOLOR_RED,8,         // 52 // SKINCOLOR_TEAL
	SKINCOLOR_PEACH,8,       // 53 // SKINCOLOR_STEELBLUE
	SKINCOLOR_ORANGE,8,      // 54 // SKINCOLOR_BLUE
	SKINCOLOR_SHINYORANGE,8, // 55 // SKINCOLOR_SHINYBLUE
	SKINCOLOR_DAWN,8,        // 56 // SKINCOLOR_NAVY
	SKINCOLOR_LIGHTORANGE,8, // 57 // SKINCOLOR_DARKBLUE
	SKINCOLOR_SLATE,8,       // 58 // SKINCOLOR_JETBLACK
	SKINCOLOR_YELLOW,4,      // 59 // SKINCOLOR_LILAC
	SKINCOLOR_YELLOW,8,      // 60 // SKINCOLOR_PURPLE
	SKINCOLOR_GOLD,8,        // 61 // SKINCOLOR_LAVENDER
	SKINCOLOR_GARDEN,8,      // 62 // SKINCOLOR_BYZANTIUM
	SKINCOLOR_YELLOW,8       // 63 // SKINCOLOR_INDIGO
};

UINT8 colortranslations[MAXSKINCOLORS][16] = {
	{  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0}, // SKINCOLOR_NONE
	{120, 120, 120, 120,   0,   0,   0,   0,   1,   1,   2,   2,   4,   6,   8,  10}, // SKINCOLOR_IVORY
	{  0,   0,   1,   1,   2,   2,   3,   3,   4,   4,   5,   5,   6,   6,   7,   7}, // SKINCOLOR_WHITE
	{  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15}, // SKINCOLOR_SILVER
	{  1,   3,   5,   7,   9,  11,  13,  15,  17,  19,  21,  23,  25,  27,  29,  31}, // SKINCOLOR_CLOUDY
	{  8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23}, // SKINCOLOR_GREY
	{ 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31}, // SKINCOLOR_DARKGREY
	{ 24,  24,  25,  25,  26,  26,  27,  27,  28,  28,  29,  29,  30,  30,  31,  31}, // SKINCOLOR_BLACK
	{120, 120, 121, 121, 122, 122, 123, 123, 124, 124, 125, 125, 126, 126, 127, 127}, // SKINCOLOR_SALMON
	{144, 144, 145, 145, 146, 146, 147, 147, 148, 148, 149, 149, 150, 150, 151, 151}, // SKINCOLOR_PINK
	{120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135}, // SKINCOLOR_LIGHTRED
	{120, 121, 123, 124, 126, 127, 129, 130, 132, 133, 135, 136, 138, 139, 141, 143}, // SKINCOLOR_SHINYRED
	{125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140}, // SKINCOLOR_RED
	{144, 145, 146, 147, 148, 149, 150, 151, 134, 135, 136, 137, 138, 139, 140, 141}, // SKINCOLOR_DARKPINK
	{136, 136, 137, 137, 138, 138, 139, 139, 140, 140, 141, 141, 142, 142, 143, 143}, // SKINCOLOR_DARKRED
	{120, 121, 122, 123, 124, 147,  88,  89, 149,  91,  92, 151,  94,  95, 152, 153}, // SKINCOLOR_DAWN
	{ 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95}, // SKINCOLOR_ORANGE
	{ 80,  81,  83,  85,  86,  88,  90,  91,  93,  95, 152, 153, 154, 156, 157, 159}, // SKINCOLOR_SHINYORANGE
	{ 88,  89,  90,  91,  92,  93,  94,  95, 152, 153, 154, 155, 156, 157, 158, 159}, // SKINCOLOR_DARKORANGE
	{112, 113, 114, 115, 116, 117, 118, 119, 156, 156, 157, 157, 158, 158, 159, 159}, // SKINCOLOR_GOLDENBROWN
	{152, 152, 153, 153, 154, 154, 155, 155, 156, 156, 157, 157, 158, 158, 159, 159}, // SKINCOLOR_ROSEWOOD
	{152, 153, 154, 155, 156, 157, 158, 159, 139, 140, 141, 142, 143,  31,  31,  31}, // SKINCOLOR_DARKROSEWOOD
	{  3,   5,   7,  32,   9,  34,  36,  37,  39,  42,  45,  59,  60,  61,  62,  63}, // SKINCOLOR_SEPIA
	{ 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47}, // SKINCOLOR_BEIGE
	{ 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63}, // SKINCOLOR_BROWN
	{ 57,  58,  59,  59,  60,  60,  61,  61,  62,  62,  63,  63,  28,  29,  30,  31}, // SKINCOLOR_LEATHER
	{ 97,  98,  99, 100, 101, 102, 103, 104, 113, 113, 114, 115, 115, 115, 116, 117}, // SKINCOLOR_YELLOW
	{ 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79}, // SKINCOLOR_PEACH
	{ 80,  80,  81,  81,  82,  82,  83,  83,  84,  84,  85,  85,  86,  86,  87,  87}, // SKINCOLOR_LIGHTORANGE
	{ 72,  73,  74,  75,  76,  77,  78,  79,  48,  49,  50,  51,  52,  53,  54,  55}, // SKINCOLOR_CARAMEL
	{112, 112, 113, 113, 114, 114, 115, 115, 116, 116, 117, 117, 118, 118, 119, 119}, // SKINCOLOR_GOLD
	{ 64,  66,  68,  70,  72,  74,  76,  78,  48,  50,  52,  54,  56,  58,  60,  62}, // SKINCOLOR_SHINYCARAMEL
	{121, 144, 145,  72,  73,  84, 114, 115, 107, 108, 109, 183, 223, 207,  30, 246}, // SKINCOLOR_VOMIT
	{ 98,  99, 112, 101, 113, 114, 106, 179, 180, 181, 182, 172, 183, 173, 174, 175}, // SKINCOLOR_GARDEN
	{176, 176, 176, 176, 177, 177, 177, 177, 178, 178, 178, 178, 179, 179, 179, 179}, // SKINCOLOR_LIGHTARMY
	{176, 176, 177, 177, 178, 178, 179, 179, 180, 180, 181, 181, 182, 182, 183, 183}, // SKINCOLOR_ARMY
	{176, 176, 177, 177, 178, 178, 179, 179, 166, 167, 168, 169, 170, 171, 172, 173}, // SKINCOLOR_PISTACHIO
	{177, 177, 178, 178, 165, 165, 167, 167, 182, 182, 171, 171, 183, 183, 173, 173}, // SKINCOLOR_ROBOHOOD
	{105, 105, 106, 106, 107, 107, 108, 108, 109, 109, 110, 110, 111, 111,  31,  31}, // SKINCOLOR_OLIVE
	{176, 177, 178, 179, 170, 181, 182, 183, 173, 173, 174, 174, 175, 175,  31,  31}, // SKINCOLOR_DARKARMY
	{160, 160, 161, 161, 162, 162, 163, 163, 164, 164, 165, 165, 166, 166, 167, 167}, // SKINCOLOR_LIGHTGREEN
	{184, 184, 184, 184, 185, 185, 185, 185, 186, 186, 186, 186, 187, 187, 187, 187}, // SKINCOLOR_UGLYGREEN
	{184, 184, 185, 185, 186, 186, 187, 187, 188, 188, 189, 189, 190, 190, 191, 191}, // SKINCOLOR_NEONGREEN
	{160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175}, // SKINCOLOR_GREEN
	{168, 168, 169, 169, 170, 170, 171, 171, 172, 172, 173, 173, 174, 174, 175, 175}, // SKINCOLOR_DARKGREEN
	{187, 187, 188, 188, 189, 189, 190, 190, 191, 191, 175, 175,  30,  30,  31,  31}, // SKINCOLOR_SWAMP
	{224, 225, 226, 212, 213, 213, 214, 215, 220, 221, 172, 222, 173, 223, 174, 175}, // SKINCOLOR_FROST
	{200, 200, 200, 200, 201, 201, 201, 201, 202, 202, 202, 202, 203, 203, 203, 203}, // SKINCOLOR_SLATE
	{224, 224, 225, 225, 226, 226, 227, 227, 228, 228, 229, 229, 230, 230, 231, 231}, // SKINCOLOR_LIGHTBLUE
	{208, 208, 209, 210, 210, 211, 212, 213, 213, 214, 215, 216, 216, 217, 218, 219}, // SKINCOLOR_CYAN
	{216, 216, 216, 216, 217, 217, 217, 217, 218, 218, 218, 218, 219, 219, 219, 219}, // SKINCOLOR_CERULEAN
	{208, 208, 209, 210, 210, 211, 212, 213, 213, 214, 215, 220, 220, 221, 222, 223}, // SKINCOLOR_TURQOISE
	{220, 220, 220, 220, 221, 221, 221, 221, 222, 222, 222, 222, 223, 223, 223, 223}, // SKINCOLOR_TEAL
	{200, 200, 201, 201, 202, 202, 203, 203, 204, 204, 205, 205, 206, 206, 207, 207}, // SKINCOLOR_STEELBLUE
	{224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239}, // SKINCOLOR_BLUE
	{224, 225, 226, 228, 229, 231, 232, 234, 235, 237, 238, 240, 241, 243, 244, 246}, // SKINCOLOR_SHINYBLUE
	{200, 201, 202, 203, 204, 205, 206, 238, 239, 240, 241, 242, 243, 244, 245, 246}, // SKINCOLOR_NAVY
	{231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246}, // SKINCOLOR_DARKBLUE
	{200, 201, 202, 203, 204, 205, 206, 207,  28,  28,  29,  29,  30,  30,  31,  31}, // SKINCOLOR_JETBLACK
	{120, 120, 121, 121, 122, 122, 123, 123, 192, 192, 248, 248, 249, 249, 250, 250}, // SKINCOLOR_LILAC
	{192, 192, 193, 193, 194, 194, 195, 195, 196, 196, 197, 197, 198, 198, 199, 199}, // SKINCOLOR_PURPLE
	{248, 248, 249, 249, 250, 250, 251, 251, 252, 252, 253, 253, 254, 254, 255, 255}, // SKINCOLOR_LAVENDER
	{192, 248, 249, 250, 251, 252, 253, 254, 255, 255,  29,  29,  30,  30,  31,  31}, // SKINCOLOR_BYZANTIUM
	{192, 193, 194, 195, 196, 197, 198, 199, 255, 255,  29,  29,  30,  30,  31,  31}, // SKINCOLOR_INDIGO
	/* Removed Colours
		{192, 192, 248, 249, 250, 251, 229, 204, 230, 205, 206, 239, 240, 241, 242, 243}, // SKINCOLOR_DUSK
		{  1, 145, 125,  73,  83, 114, 106, 180, 187, 168, 219, 205, 236, 206, 199, 255}, // SKINCOLOR_RAINBOW
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
	CV_RegisterVar(&cv_lightningshield);
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
	CV_RegisterVar(&cv_kartballoons);
	CV_RegisterVar(&cv_kartfrantic);
	CV_RegisterVar(&cv_kartcomeback);
	CV_RegisterVar(&cv_kartmirror);
	CV_RegisterVar(&cv_kartspeedometer);
	CV_RegisterVar(&cv_votetime);

	CV_RegisterVar(&cv_kartdebugitem);
	CV_RegisterVar(&cv_kartdebugamount);
	CV_RegisterVar(&cv_kartdebugcheckpoint);
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
		if (playeringame[i] && !players[i].spectator)
			pcount++;
	}

	if (pcount <= 1)
		return false;

	winningpos = pcount/2;
	if (pcount % 2) // any remainder?
		winningpos++;

	return (player->kartstuff[k_position] > winningpos);
}

//{ SRB2kart Roulette Code - Position Based

#define NUMKARTODDS 	40

// Less ugly 2D arrays
static INT32 K_KartItemOddsRace[NUMKARTRESULTS][9] =
{
				//P-Odds	 0  1  2  3  4  5  6  7  8
			   /*Sneaker*/ {20, 0, 0, 3, 6, 5, 0, 0, 0 }, // Sneaker
		/*Rocket Sneaker*/ { 0, 0, 0, 0, 0, 3, 5, 5, 0 }, // Rocket Sneaker
		 /*Invincibility*/ { 0, 0, 0, 0, 0, 1, 6, 9,16 }, // Invincibility
				/*Banana*/ { 0, 8, 4, 2, 1, 0, 0, 0, 0 }, // Banana
		/*Eggman Monitor*/ { 0, 4, 3, 2, 0, 0, 0, 0, 0 }, // Eggman Monitor
			  /*Orbinaut*/ { 0, 6, 5, 4, 2, 0, 0, 0, 0 }, // Orbinaut
				  /*Jawz*/ { 0, 0, 3, 2, 2, 1, 0, 0, 0 }, // Jawz
				  /*Mine*/ { 0, 1, 1, 2, 1, 0, 0, 0, 0 }, // Mine
			   /*Ballhog*/ { 0, 0, 1, 2, 1, 0, 0, 0, 0 }, // Ballhog
   /*Self-Propelled Bomb*/ { 0, 0, 1, 1, 1, 1, 2, 2, 2 }, // Self-Propelled Bomb
				  /*Grow*/ { 0, 0, 0, 0, 0, 0, 1, 1, 2 }, // Grow
				/*Shrink*/ { 0, 0, 0, 0, 0, 0, 1, 1, 2 }, // Shrink
	  /*Lightning Shield*/ { 0, 1, 2, 0, 0, 0, 0, 0, 0 }, // Lightning Shield
			   /*Hyudoro*/ { 0, 0, 0, 0, 1, 2, 1, 0, 0 }, // Hyudoro
		   /*Pogo Spring*/ { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // Pogo Spring
		  /*Kitchen Sink*/ { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // Kitchen Sink
			/*Sneaker x3*/ { 0, 0, 0, 0, 3, 7, 6, 4, 0 }, // Sneaker x3
			 /*Banana x3*/ { 0, 0, 1, 1, 0, 0, 0, 0, 0 }, // Banana x3
			/*Banana x10*/ { 0, 0, 0, 0, 1, 0, 0, 0, 0 }, // Banana x10
		   /*Orbinaut x3*/ { 0, 0, 0, 1, 1, 1, 0, 0, 0 }, // Orbinaut x3
			   /*Jawz x2*/ { 0, 0, 0, 1, 1, 0, 0, 0, 0 }  // Jawz x2
};

static INT32 K_KartItemOddsBattle[NUMKARTRESULTS][6] =
{
				//P-Odds	 0  1  2  3  4  5
			   /*Sneaker*/ { 3, 1, 2, 2, 0, 2 }, // Sneaker
		/*Rocket Sneaker*/ { 0, 0, 0, 0, 0, 0 }, // Rocket Sneaker
		 /*Invincibility*/ { 4, 2, 1, 0, 0, 2 }, // Invincibility
				/*Banana*/ { 0, 0, 2, 3, 6, 0 }, // Banana
		/*Eggman Monitor*/ { 0, 0, 2, 2, 3, 0 }, // Eggman Monitor
			  /*Orbinaut*/ { 0, 0, 3, 5,10, 0 }, // Orbinaut
				  /*Jawz*/ { 3, 3, 2, 1, 0, 2 }, // Jawz
				  /*Mine*/ { 0, 3, 2, 1, 0, 2 }, // Mine
			   /*Ballhog*/ { 0, 2, 1, 1, 0, 2 }, // Ballhog
   /*Self-Propelled Bomb*/ { 0, 0, 0, 0, 0, 0 }, // Self-Propelled Bomb
				  /*Grow*/ { 4, 2, 0, 0, 0, 2 }, // Grow
				/*Shrink*/ { 0, 0, 0, 0, 0, 0 }, // Shrink
	  /*Lightning Shield*/ { 0, 0, 0, 0, 0, 0 }, // Lightning Shield
			   /*Hyudoro*/ { 0, 0, 1, 1, 0, 0 }, // Hyudoro
		   /*Pogo Spring*/ { 0, 0, 1, 2, 0, 0 }, // Pogo Spring
		  /*Kitchen Sink*/ { 0, 0, 0, 0, 0, 0 }, // Kitchen Sink
			/*Sneaker x3*/ { 2, 0, 0, 0, 0, 2 }, // Sneaker x3
			 /*Banana x3*/ { 0, 2, 2, 1, 1, 2 }, // Banana x3
			/*Banana x10*/ { 1, 0, 0, 0, 0, 0 }, // Banana x10
		   /*Orbinaut x3*/ { 0, 3, 1, 1, 0, 2 }, // Orbinaut x3
			   /*Jawz x2*/ { 3, 2, 0, 0, 0, 2 }  // Jawz x2
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

static INT32 K_KartGetItemOdds(UINT8 pos, SINT8 item, player_t *player, boolean mashed)
{
	const INT32 distvar = (64*14);
	INT32 newodds;
	INT32 i;
	UINT8 pingame = 0, pexiting = 0, pinvin = 0;
	SINT8 first = -1;
	SINT8 second = -1;
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
													players[first].mo->z - players[second].mo->z) / mapheaderinfo[gamemap-1]->mobj_scale
													* (pingame - 1)
													/ ((pingame - 1) * (pingame + 1) / 3);
		if (franticitems)
			secondist = (15*secondist/14);
	}

	switch (item)
	{
		case KITEM_SNEAKER:
			if ((!cv_sneaker.value) && (!modeattacking)) newodds = 0;
			break;
		case KITEM_ROCKETSNEAKER:
			if (franticitems) newodds *= 2;
			if (mashed) newodds /= 2;
			if (!cv_rocketsneaker.value) newodds = 0;
			break;
		case KITEM_INVINCIBILITY:
			if (franticitems) newodds *= 2;
			if (mashed) newodds /= 2;
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
			if (franticitems) newodds *= 2;
			if (mashed) newodds /= 2;
			if (!cv_jawz.value) newodds = 0;
			break;
		case KITEM_MINE:
			if (franticitems) newodds *= 2;
			if (mashed) newodds /= 2;
			if (!cv_mine.value) newodds = 0;
			break;
		case KITEM_BALLHOG:
			if (franticitems) newodds *= 2;
			if (mashed) newodds /= 2;
			if (!cv_ballhog.value) newodds = 0;
			break;
		case KITEM_SPB:
			if (franticitems) newodds *= 2;
			if (mashed) newodds /= 2;
			if ((!cv_selfpropelledbomb.value)
				|| (indirectitemcooldown > 0)
				|| (pexiting > 0)
				|| (secondist/distvar < 4))
				newodds = 0;
			newodds *= min((secondist/distvar)-3, 3);
			break;
		case KITEM_GROW:
			if (franticitems) newodds *= 2;
			if (mashed) newodds /= 2;
			if ((!cv_grow.value) || (pinvin > 2)) newodds = 0;
			break;
		case KITEM_SHRINK:
			if (franticitems) newodds *= 2;
			if (mashed) newodds /= 2;
			if ((!cv_shrink.value)
				|| (indirectitemcooldown > 0)
				|| (pingame-1 <= pexiting)) newodds = 0;
			break;
		case KITEM_LIGHTNINGSHIELD:
			if (franticitems) newodds *= 2;
			if (mashed) newodds /= 2;
			if (!cv_lightningshield.value) newodds = 0;
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
			if (franticitems) newodds *= 2;
			if (mashed) newodds /= 2;
			if (!cv_triplesneaker.value) newodds = 0;
			break;
		case KRITEM_TRIPLEBANANA:
			if (franticitems) newodds *= 2;
			if (mashed) newodds /= 2;
			if (!cv_triplebanana.value) newodds = 0;
			break;
		case KRITEM_TENFOLDBANANA:
			if (franticitems) newodds *= 2;
			if (mashed) newodds /= 2;
			if (!cv_triplebanana.value) newodds = 0;
			break;
		case KRITEM_TRIPLEORBINAUT:
			if (franticitems) newodds *= 2;
			if (mashed) newodds /= 2;
			if (!cv_tripleorbinaut.value) newodds = 0;
			break;
		case KRITEM_DUALJAWZ:
			if (franticitems) newodds *= 2;
			if (mashed) newodds /= 2;
			if (!cv_dualjawz.value) newodds = 0;
			break;
		default:
			break;
	}

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
	INT32 avgballoon = 0;
	UINT8 oddsvalid[9];
	UINT8 disttable[14];
	UINT8 distlen = 0;
	boolean mashed = false;

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
		if (players[i].kartstuff[k_balloon] > 0)
			avgballoon += players[i].kartstuff[k_balloon];
	}

	if (pingame)
		avgballoon /= pingame;

	// This makes the roulette produce the random noises.
	if ((player->kartstuff[k_itemroulette] % 3) == 1 && P_IsLocalPlayer(player))
		S_StartSound(NULL,sfx_mkitm1 + ((player->kartstuff[k_itemroulette] / 3) % 8));

	roulettestop = (TICRATE*1) + (3*(pingame - player->kartstuff[k_position]));

	// If the roulette finishes or the player presses BT_ATTACK, stop the roulette and calculate the item.
	// I'm returning via the exact opposite, however, to forgo having another bracket embed. Same result either way, I think.
	// Finally, if you get past this check, now you can actually start calculating what item you get.
	if ((cmd->buttons & BT_ATTACK) && !(player->kartstuff[k_eggmanheld] || player->kartstuff[k_itemheld]) && player->kartstuff[k_itemroulette] >= roulettestop)
		mashed = true; // Mashing halves your chances for the good items
	else if (!(player->kartstuff[k_itemroulette] >= (TICRATE*3)))
		return;

	if (cmd->buttons & BT_ATTACK)
		player->pflags |= PF_ATTACKDOWN;

	if (cv_kartdebugitem.value != 0)
	{
		K_KartGetItemResult(player, cv_kartdebugitem.value);
		player->kartstuff[k_itemamount] = cv_kartdebugamount.value;
		player->kartstuff[k_itemroulette] = 0;
		return;
	}

	// Initializes existing spawnchance values
	for (i = 0; i < (NUMKARTRESULTS * NUMKARTODDS); i++)
		spawnchance[i] = 0;

	for (i = 0; i < 9; i++)
	{
		INT32 j;
		UINT8 available = 0;

		if (G_BattleGametype() && i > 5)
		{
			oddsvalid[i] = 0;
			break;
		}

		for (j = 0; j < NUMKARTRESULTS; j++)
		{
			if (K_KartGetItemOdds(i, j, player, mashed) > 0)
			{
				available = 1;
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
		if (oddsvalid[3]) SETUPDISTTABLE(3,1);
		if (oddsvalid[4]) SETUPDISTTABLE(4,1);

		if (player->kartstuff[k_roulettetype] == 1 && oddsvalid[5]) // Player-controlled "Karma" items
			useodds = 5;
		else
		{
			UINT8 wantedpos = (player->kartstuff[k_balloon]-avgballoon)+2; // 0 is two balloons below average, 2 is average, 4 is two balloons above average
			if (wantedpos > 4)
				wantedpos = 4;
			if (wantedpos < 0)
				wantedpos = 0;
			useodds = disttable[(wantedpos * distlen) / 5];
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
	for (chance = 0; chance < K_KartGetItemOdds(pos, itemnum, player, mashed); chance++) \
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
	SETITEMRESULT(useodds, KITEM_LIGHTNINGSHIELD);	// Lightning Shield
	SETITEMRESULT(useodds, KITEM_HYUDORO);			// Hyudoro
	SETITEMRESULT(useodds, KITEM_POGOSPRING);		// Pogo Spring
	//SETITEMRESULT(useodds, KITEM_KITCHENSINK);	// Kitchen Sink

	SETITEMRESULT(useodds, KRITEM_TRIPLESNEAKER);	// Sneaker x3
	SETITEMRESULT(useodds, KRITEM_TRIPLEBANANA);	// Banana x3
	SETITEMRESULT(useodds, KRITEM_TENFOLDBANANA);	// Banana x10
	SETITEMRESULT(useodds, KRITEM_TRIPLEORBINAUT);	// Orbinaut x3
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

void K_KartBouncing(mobj_t *mobj1, mobj_t *mobj2, boolean bounce, boolean solid)
{
	mobj_t *fx;
	fixed_t momdifx, momdify;
	fixed_t distx, disty;
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
	if ((mobj1->player && mobj1->player->kartstuff[k_justbumped])
		|| (mobj2->player && mobj1->player->kartstuff[k_justbumped]))
	{
		if (mobj1->player)
			mobj1->player->kartstuff[k_justbumped] = bumptime;
		if (mobj2->player)
			mobj2->player->kartstuff[k_justbumped] = bumptime;
		return;
	}

	S_StartSound(mobj1, sfx_s3k49);

	fx = P_SpawnMobj(mobj1->x/2 + mobj2->x/2, mobj1->y/2 + mobj2->y/2, mobj1->z/2 + mobj2->z/2, MT_BUMP);
	if (mobj1->eflags & MFE_VERTICALFLIP)
		fx->eflags |= MFE_VERTICALFLIP;
	else
		fx->eflags &= ~MFE_VERTICALFLIP;
	fx->scale = mobj1->scale;

	if (bounce == true) // Perform a Goomba Bounce.
		mobj1->momz = -mobj1->momz;
	else
	{
		fixed_t newz = mobj1->momz;
		mobj1->momz = mobj2->momz;
		if (solid == false)
			mobj2->momz = newz;
	}

	mass1 = mass2 = 5*FRACUNIT;

	if (mobj1->player)
		mass1 = (mobj1->player->kartweight)*FRACUNIT;

	if (mobj2->player)
		mass2 = (mobj2->player->kartweight)*FRACUNIT;
	else if (solid == true)
		mass2 = mass1;

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

	distx = mobj1->x - mobj2->x;
	disty = mobj1->y - mobj2->y;

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

	mobj1->momx = mobj1->momx - FixedMul(FixedMul(FixedDiv(2*mass2, mass1 + mass2), p), distx);
	mobj1->momy = mobj1->momy - FixedMul(FixedMul(FixedDiv(2*mass2, mass1 + mass2), p), disty);

	if (solid == false)
	{
		mobj2->momx = mobj2->momx - FixedMul(FixedMul(FixedDiv(2*mass1, mass1 + mass2), p), -distx);
		mobj2->momy = mobj2->momy - FixedMul(FixedMul(FixedDiv(2*mass1, mass1 + mass2), p), -disty);
	}

	// Because this is done during collision now, rmomx and rmomy need to be recalculated
	// so that friction doesn't immediately decide to stop the player if they're at a standstill
	// Also set justbumped here
	if (mobj1->player)
	{
		mobj1->player->rmomx = mobj1->momx - mobj1->player->cmomx;
		mobj1->player->rmomy = mobj1->momy - mobj1->player->cmomy;
		mobj1->player->kartstuff[k_justbumped] = bumptime;
	}

	if (mobj2->player)
	{
		mobj2->player->rmomx = mobj2->momx - mobj2->player->cmomx;
		mobj2->player->rmomy = mobj2->momy - mobj2->player->cmomy;
		mobj2->player->kartstuff[k_justbumped] = bumptime;
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

	if (player->kartstuff[k_respawn] > 3)
	{
		player->kartstuff[k_respawn]--;
		player->mo->momz = 0;
		player->powers[pw_flashing] = 2;
		player->powers[pw_nocontrol] = 2;
		if (leveltime % 8 == 0)
		{
			mobj_t *mo;
			fixed_t newz;

			S_StartSound(player->mo, sfx_s3kcas);

			if (player->mo->eflags & MFE_VERTICALFLIP)
				newz = player->mo->z + player->mo->height;
			else
				newz = player->mo->z;
			mo = P_SpawnMobj(player->mo->x, player->mo->y, newz, MT_DEZLASER);
			if (mo)
			{
				if (player->mo->eflags & MFE_VERTICALFLIP)
					mo->eflags |= MFE_VERTICALFLIP;
				P_SetTarget(&mo->target, player->mo);
				mo->momz = (8*FRACUNIT)*P_MobjFlip(player->mo);
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

static void K_PlayTauntSound(mobj_t *source)
{
	if (source->player && source->player->kartstuff[k_tauntvoices]) // Prevents taunt sounds from playing every time the button is pressed
		return;

	S_StartSound(source, sfx_taunt1+P_RandomKey(4));

	if (source->player)
	{
		source->player->kartstuff[k_tauntvoices] = 6*TICRATE;
		source->player->kartstuff[k_voices] = 3*TICRATE;
	}
}

static void K_PlayOvertakeSound(mobj_t *source)
{
	if (source->player && source->player->kartstuff[k_voices]) // Prevents taunt sounds from playing every time the button is pressed
		return;

	// 4 seconds from before race begins, 10 seconds afterwards
	if (leveltime < starttime+(10*TICRATE))
		return;

	S_StartSound(source, sfx_slow);

	if (source->player)
	{
		source->player->kartstuff[k_voices] = 3*TICRATE;

		if (source->player->kartstuff[k_tauntvoices] < 3*TICRATE)
			source->player->kartstuff[k_tauntvoices] = 3*TICRATE;
	}
}

static void K_PlayHitEmSound(mobj_t *source)
{
	S_StartSound(source, sfx_hitem);

	if (source->player)
	{
		source->player->kartstuff[k_voices] = 3*TICRATE;

		if (source->player->kartstuff[k_tauntvoices] < 3*TICRATE)
			source->player->kartstuff[k_tauntvoices] = 3*TICRATE;
	}
}

void K_MomentumToFacing(player_t *player)
{
	angle_t dangle = player->mo->angle - R_PointToAngle2(0, 0, player->mo->momx, player->mo->momy);

	if (dangle > ANGLE_180)
		dangle = InvAngle(dangle);

	// If you aren't on the ground or are moving in too different of a direction don't do this
	if (!P_IsObjectOnGround(player->mo) || dangle > ANGLE_90)
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

	// Offroad is separate, it's difficult to factor it in with a variable value anyway.
	if (!(player->kartstuff[k_invincibilitytimer] || player->kartstuff[k_hyudorotimer] || player->kartstuff[k_sneakertimer])
		&& player->kartstuff[k_offroad] >= 0 && speed)
		boostpower = FixedDiv(boostpower, player->kartstuff[k_offroad] + FRACUNIT);

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

	if (G_BattleGametype() && player->kartstuff[k_balloon] <= 0)
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

	if (G_BattleGametype() && player->kartstuff[k_balloon] <= 0)
		kartspeed = 1;

	//k_accel += 3 * (9 - kartspeed); // 36 - 60
	k_accel += 4 * (9 - kartspeed); // 32 - 64

	return FixedMul(k_accel, K_GetKartBoostPower(player, false));
}

UINT16 K_GetKartFlashing(void)
{
	UINT16 tics = flashingtics;
	if (G_BattleGametype())
	{
		tics *= 2;
		//tics += (3*TICRATE/8) * (player->kartspeed-1);
	}
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

void K_SpinPlayer(player_t *player, mobj_t *source, INT32 type)
{
	if (player->health <= 0)
		return;

	if (player->powers[pw_flashing] > 0 || player->kartstuff[k_squishedtimer] > 0 || player->kartstuff[k_spinouttimer] > 0
		|| player->kartstuff[k_invincibilitytimer] > 0 || player->kartstuff[k_growshrinktimer] > 0 || player->kartstuff[k_hyudorotimer] > 0
		|| (G_BattleGametype() && ((player->kartstuff[k_balloon] <= 0 && player->kartstuff[k_comebacktimer]) || player->kartstuff[k_comebackmode] == 1)))
		return;

	if (source && source != player->mo && source->player)
		K_PlayHitEmSound(source);

	//player->kartstuff[k_sneakertimer] = 0;
	player->kartstuff[k_driftboost] = 0;

	if (G_BattleGametype())
	{
		if (source && source->player && player != source->player)
			P_AddPlayerScore(source->player, 1);

		if (player->kartstuff[k_balloon] > 0)
		{
			if (player->kartstuff[k_balloon] == 1)
			{
				mobj_t *karmahitbox = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_KARMAHITBOX); // Player hitbox is too small!!
				P_SetTarget(&karmahitbox->target, player->mo);
				karmahitbox->destscale = player->mo->scale;
				P_SetScale(karmahitbox, player->mo->scale);
				CONS_Printf(M_GetText("%s lost all of their balloons!\n"), player_names[player-players]);
			}
			player->kartstuff[k_balloon]--;
		}

		K_CheckBalloons();
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
		player->kartstuff[k_spinouttimer] = 1*TICRATE; // Wipeout

	player->powers[pw_flashing] = K_GetKartFlashing();

	if (player->mo->state != &states[S_KART_SPIN])
		P_SetPlayerMobjState(player->mo, S_KART_SPIN);

	return;
}

void K_SquishPlayer(player_t *player, mobj_t *source)
{
	if (player->health <= 0)
		return;

	if (player->powers[pw_flashing] > 0 || player->kartstuff[k_squishedtimer] > 0 || player->kartstuff[k_invincibilitytimer] > 0
		|| player->kartstuff[k_growshrinktimer] > 0 || player->kartstuff[k_hyudorotimer] > 0
		|| (G_BattleGametype() && ((player->kartstuff[k_balloon] <= 0 && player->kartstuff[k_comebacktimer]) || player->kartstuff[k_comebackmode] == 1)))
		return;

	player->kartstuff[k_sneakertimer] = 0;
	player->kartstuff[k_driftboost] = 0;

	if (G_BattleGametype())
	{
		if (source && source->player && player != source->player)
			P_AddPlayerScore(source->player, 1);

		if (player->kartstuff[k_balloon] > 0)
		{
			if (player->kartstuff[k_balloon] == 1)
			{
				mobj_t *karmahitbox = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_KARMAHITBOX); // Player hitbox is too small!!
				P_SetTarget(&karmahitbox->target, player->mo);
				karmahitbox->destscale = player->mo->scale;
				P_SetScale(karmahitbox, player->mo->scale);
				CONS_Printf(M_GetText("%s lost all of their balloons!\n"), player_names[player-players]);
			}
			player->kartstuff[k_balloon]--;
		}

		K_CheckBalloons();
	}

	player->kartstuff[k_comebacktimer] = comebacktime;

	player->kartstuff[k_squishedtimer] = 2*TICRATE;

	player->powers[pw_flashing] = K_GetKartFlashing();

	player->mo->flags |= MF_NOCLIP;

	if (player->mo->state != &states[S_KART_SQUISH]) // Squash
		P_SetPlayerMobjState(player->mo, S_KART_SQUISH);

	P_PlayRinglossSound(player->mo);

	return;
}

void K_ExplodePlayer(player_t *player, mobj_t *source) // A bit of a hack, we just throw the player up higher here and extend their spinout timer
{
	if (player->health <= 0)
		return;

	if (player->powers[pw_flashing] > 0 || player->kartstuff[k_squishedtimer] > 0 || player->kartstuff[k_spinouttimer] > 0
		|| player->kartstuff[k_invincibilitytimer] > 0 || player->kartstuff[k_growshrinktimer] > 0 || player->kartstuff[k_hyudorotimer] > 0
		|| (G_BattleGametype() && ((player->kartstuff[k_balloon] <= 0 && player->kartstuff[k_comebacktimer]) || player->kartstuff[k_comebackmode] == 1)))
		return;

	player->mo->momz = 18*(mapheaderinfo[gamemap-1]->mobj_scale);
	player->mo->momx = player->mo->momy = 0;

	player->kartstuff[k_sneakertimer] = 0;
	player->kartstuff[k_driftboost] = 0;

	if (G_BattleGametype())
	{
		if (source && source->player && player != source->player)
			P_AddPlayerScore(source->player, 1);

		if (player->kartstuff[k_balloon] > 0)
		{
			if (player->kartstuff[k_balloon] == 1)
			{
				mobj_t *karmahitbox = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_KARMAHITBOX); // Player hitbox is too small!!
				P_SetTarget(&karmahitbox->target, player->mo);
				karmahitbox->destscale = player->mo->scale;
				P_SetScale(karmahitbox, player->mo->scale);
				CONS_Printf(M_GetText("%s lost all of their balloons!\n"), player_names[player-players]);
			}
			player->kartstuff[k_balloon]--;
		}

		K_CheckBalloons();
	}

	player->kartstuff[k_comebacktimer] = comebacktime;

	player->kartstuff[k_spinouttype] = 1;
	player->kartstuff[k_spinouttimer] = 2*TICRATE+(TICRATE/2);

	player->powers[pw_flashing] = K_GetKartFlashing();

	if (player->mo->state != &states[S_KART_SPIN])
		P_SetPlayerMobjState(player->mo, S_KART_SPIN);

	P_PlayRinglossSound(player->mo);

	if (P_IsLocalPlayer(player))
	{
		quake.intensity = 64*FRACUNIT;
		quake.time = 5;
	}

	return;
}

void K_StealBalloon(player_t *player, player_t *victim, boolean force)
{
	INT32 newballoon;
	angle_t newangle, diff;
	fixed_t newx, newy;
	mobj_t *newmo;

	if (!G_BattleGametype())
		return;

	if (player->health <= 0 || victim->health <= 0)
		return;

	if (!force)
	{
		if (victim->kartstuff[k_balloon] <= 0) // || player->kartstuff[k_balloon] >= cv_kartballoons.value+2
			return;

		if ((player->powers[pw_flashing] > 0 || player->kartstuff[k_squishedtimer] > 0 || player->kartstuff[k_spinouttimer] > 0
			|| player->kartstuff[k_invincibilitytimer] > 0 || player->kartstuff[k_growshrinktimer] > 0 || player->kartstuff[k_hyudorotimer] > 0
			|| (player->kartstuff[k_balloon] <= 0 && player->kartstuff[k_comebacktimer]))
			|| (victim->powers[pw_flashing] > 0 || victim->kartstuff[k_squishedtimer] > 0 || victim->kartstuff[k_spinouttimer] > 0
			|| victim->kartstuff[k_invincibilitytimer] > 0 || victim->kartstuff[k_growshrinktimer] > 0 || victim->kartstuff[k_hyudorotimer] > 0))
			return;
	}

	if (netgame)
	{
		if (player->kartstuff[k_balloon] <= 0)
			CONS_Printf(M_GetText("%s is back in the game!\n"), player_names[player-players]);
		else if (cv_hazardlog.value)
			CONS_Printf(M_GetText("%s stole a balloon from %s!\n"), player_names[player-players], player_names[victim-players]);
	}

	newballoon = player->kartstuff[k_balloon];
	if (newballoon <= 1)
		diff = 0;
	else
		diff = FixedAngle(360*FRACUNIT/newballoon);

	newangle = player->mo->angle;
	newx = player->mo->x + P_ReturnThrustX(player->mo, newangle + ANGLE_180, 64*FRACUNIT);
	newy = player->mo->y + P_ReturnThrustY(player->mo, newangle + ANGLE_180, 64*FRACUNIT);

	newmo = P_SpawnMobj(newx, newy, player->mo->z, MT_BATTLEBALLOON);
	newmo->threshold = newballoon;
	P_SetTarget(&newmo->tracer, victim->mo);
	P_SetTarget(&newmo->target, player->mo);
	newmo->angle = (diff * (newballoon-1));
	newmo->color = victim->skincolor;

	if (newballoon+1 < 2)
		P_SetMobjState(newmo, S_BATTLEBALLOON3);
	else if (newballoon+1 < 3)
		P_SetMobjState(newmo, S_BATTLEBALLOON2);
	else
		P_SetMobjState(newmo, S_BATTLEBALLOON1);

	player->kartstuff[k_balloon]++;
	player->kartstuff[k_comebackpoints] = 0;
	player->powers[pw_flashing] = K_GetKartFlashing();
	player->kartstuff[k_comebacktimer] = comebacktime;

	/*victim->powers[pw_flashing] = K_GetKartFlashing();
	victim->kartstuff[k_comebacktimer] = comebacktime;*/

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
	smoldering->tics = TICRATE*3;
	INT32 speed, speed2;

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
		|| (G_BattleGametype() && player->kartstuff[k_balloon] <= 0 && player->kartstuff[k_comebacktimer]))
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

		if (i == 0)
			P_SetMobjState(sparkle, S_KARTINVULN_LARGE1);

		P_SetTarget(&sparkle->target, mo);
		sparkle->destscale = mo->destscale;
		P_SetScale(sparkle, mo->scale);
		sparkle->eflags = (sparkle->eflags & ~MFE_VERTICALFLIP)|(mo->eflags & MFE_VERTICALFLIP);
		sparkle->color = mo->color;
		//sparkle->colorized = mo->colorized;
	}
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
		angle_t playerangle;

		if (spawner->player->speed < 5<<FRACBITS)
			return;

		if (spawner->player->cmd.forwardmove < 0)
		{
			playerangle = spawner->angle+ANGLE_180;
		}
		else
		{
			playerangle = spawner->angle;
		}
		anglediff = abs(playerangle - R_PointToAngle2(0, 0, spawner->player->rmomx, spawner->player->rmomy));
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

static mobj_t *K_FindLastTrailMobj(player_t *player) // YUCK, i hate this!
{
	mobj_t *trail = player->mo->hnext;

	if (!player || !trail)
		return NULL;
	
	while (trail->hnext && !P_MobjWasRemoved(trail->hnext))
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
	fixed_t newx;
	fixed_t newy;
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
		else if (player->kartstuff[k_throwdir] == -1)
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
		if (mapthing == MT_FIREBALL) // Messy
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
			}
			else
			{
				// Drop it directly behind you.
				fixed_t dropradius = FixedHypot(player->mo->radius, player->mo->radius) + FixedHypot(mobjinfo[mapthing].radius, mobjinfo[mapthing].radius);

				newangle = player->mo->angle;

				newx = player->mo->x + P_ReturnThrustX(player->mo, newangle + ANGLE_180, dropradius);
				newy = player->mo->y + P_ReturnThrustY(player->mo, newangle + ANGLE_180, dropradius);
			}

			mo = P_SpawnMobj(newx, newy, player->mo->z, mapthing);

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

static void K_DoLightningShield(player_t *player)
{
	S_StartSound(player->mo, sfx_s3k45);
	player->kartstuff[k_attractiontimer] = 35;
	P_NukeEnemies(player->mo, player->mo, RING_DIST/4);
}

static void K_DoHyudoroSteal(player_t *player)
{
	INT32 i, numplayers = 0;
	INT32 playerswappable[MAXPLAYERS];
	INT32 stealplayer = -1; // The player that's getting stolen from
	INT32 prandom = 0;

	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (playeringame[i] && players[i].mo && players[i].mo->health > 0 && players[i].playerstate == PST_LIVE
			&& player != &players[i] && !players[i].exiting && !players[i].spectator // Player in-game

			// Can steal from this player
			&& (G_RaceGametype() //&& players[i].kartstuff[k_position] < player->kartstuff[k_position])
			|| (G_BattleGametype() && players[i].kartstuff[k_balloon] > 0))

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

	if (P_RandomChance(FRACUNIT/256)) // BEHOLD THE KITCHEN SINK
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
		S_StartSound(player->mo, sfx_s23c);

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

void K_DoPogoSpring(mobj_t *mo, fixed_t vertispeed)
{
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
			if (mo->player->kartstuff[k_sneakertimer])
				thrust = FixedMul(thrust, 5*FRACUNIT/4);
			else if (mo->player->kartstuff[k_invincibilitytimer])
				thrust = FixedMul(thrust, 9*FRACUNIT/8);
		}
		else
		{
			thrust = FixedDiv(3*P_AproxDistance(mo->momx, mo->momy)/2, 5*FRACUNIT/2);
			if (thrust < 16<<FRACBITS)
				thrust = 16<<FRACBITS;
			if (thrust > 32<<FRACBITS)
				thrust = 32<<FRACBITS;
		}

		mo->momz = FixedMul(FINESINE(ANGLE_22h>>ANGLETOFINESHIFT), FixedMul(thrust, mo->scale));
	}
	else
		mo->momz = FixedMul(vertispeed, mo->scale);

	S_StartSound(mo, sfx_kc2f);
}

void K_KillBananaChain(mobj_t *banana, mobj_t *inflictor, mobj_t *source)
{
    if (banana->hnext)
        K_KillBananaChain(banana->hnext, inflictor, source);

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
	else
	{
		player->mo->colorized = false;
	}

	if (player->kartstuff[k_spinouttimer])
	{
		if (P_IsObjectOnGround(player->mo) || player->kartstuff[k_spinouttype] == 1)
			player->kartstuff[k_spinouttimer]--;
		if (player->kartstuff[k_spinouttimer] == 0)
			player->kartstuff[k_spinouttype] = 0; // Reset type
	}
	else if (!comeback)
		player->kartstuff[k_comebacktimer] = comebacktime;
	else if (player->kartstuff[k_comebacktimer])
	{
		player->kartstuff[k_comebacktimer]--;
		if (P_IsLocalPlayer(player) && player->kartstuff[k_balloon] <= 0 && player->kartstuff[k_comebacktimer] <= 0)
			comebackshowninfo = true; // client has already seen the message
	}

	if (player->kartstuff[k_spinouttimer] == 0 && player->powers[pw_flashing] == K_GetKartFlashing())
		player->powers[pw_flashing]--;

	if (player->kartstuff[k_attractiontimer])
		player->kartstuff[k_attractiontimer]--;

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
		player->mo->destscale = mapheaderinfo[gamemap-1]->mobj_scale;
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

	/*if (player->kartstuff[k_lapanimation])
		player->kartstuff[k_lapanimation]--;*/

	if (G_BattleGametype() && (player->exiting || player->kartstuff[k_comebacktimer]))
	{
		if (player->exiting)
		{
			if (player->exiting < 6*TICRATE)
				player->kartstuff[k_cardanimation] += ((164-player->kartstuff[k_cardanimation])/8)+1;
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
	else
		player->kartstuff[k_cardanimation] = 0;

	if (player->kartstuff[k_voices])
		player->kartstuff[k_voices]--;

	if (player->kartstuff[k_tauntvoices])
		player->kartstuff[k_tauntvoices]--;

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
	if (player->kartstuff[k_invincibilitytimer])
	{
		player->mo->frame |= FF_FULLBRIGHT;
	}
	else
	{
		if (!(player->mo->state->frame & FF_FULLBRIGHT))
			player->mo->frame &= ~FF_FULLBRIGHT;
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

static void K_KartDrift(player_t *player, boolean onground)
{
	UINT8 kartspeed = player->kartspeed;
	fixed_t dsone, dstwo;

	if (G_BattleGametype() && player->kartstuff[k_balloon] <= 0)
		kartspeed = 1;

	// IF YOU CHANGE THESE: MAKE SURE YOU UPDATE THE SAME VALUES IN p_mobjc, "case MT_DRIFT:"
	dsone = (26*4 + kartspeed*2 + (9 - player->kartweight))*8;
	dstwo = dsone*2;

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
			if (player->exiting)
				return;
			if (players[i].kartstuff[k_balloon] == player->kartstuff[k_balloon] && players[i].score > player->score)
				position++;
			else if (players[i].kartstuff[k_balloon] > player->kartstuff[k_balloon])
				position++;
		}
	}

	if (leveltime < starttime || oldposition == 0)
		oldposition = position;

	if (oldposition != position) // Changed places?
		player->kartstuff[k_positiondelay] = 10; // and set up the timer.

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

	player->kartstuff[k_hyudorotimer] = 0;
	player->kartstuff[k_stealingtimer] = 0;
	player->kartstuff[k_stolentimer] = 0;

	player->kartstuff[k_attractiontimer] = 0;

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
		if (player->kartstuff[k_oldposition] <= player->kartstuff[k_position]) // But first, if you lost a place,
			player->kartstuff[k_oldposition] = player->kartstuff[k_position]; // then the other player taunts.
		else if (player->kartstuff[k_oldposition] > player->kartstuff[k_position]) // Otherwise,
		{
			K_PlayOvertakeSound(player->mo); // Say "YOU'RE TOO SLOW!"
			player->kartstuff[k_oldposition] = player->kartstuff[k_position]; // Restore the old position,
		}
	}

	if (player->kartstuff[k_positiondelay])
		player->kartstuff[k_positiondelay]--;

	// Race Spectator
	if (netgame && player->jointime < 1
	&& G_RaceGametype() && countdown)
	{
		player->spectator = true;
		player->powers[pw_nocontrol] = 5;
	}

	if ((player->pflags & PF_ATTACKDOWN) && !(cmd->buttons & BT_ATTACK))
		player->pflags &= ~PF_ATTACKDOWN;
	else if (cmd->buttons & BT_ATTACK)
		player->pflags |= PF_ATTACKDOWN;

	if (player && player->mo && player->mo->health > 0 && !player->spectator && !player->exiting && player->kartstuff[k_spinouttimer] == 0)
	{
		// First, the really specific, finicky items that function without the item being directly in your item slot.
		// Eggman Monitor throwing
		if (ATTACK_IS_DOWN && player->kartstuff[k_eggmanheld])
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
						if (P_IsLocalPlayer(player) && !player->exiting)
							S_ChangeMusicInternal("kinvnc", true);
						if (!cv_kartinvinsfx.value && !P_IsLocalPlayer(player))
							S_StartSound(player->mo, sfx_kinvnc);
						if (!player->kartstuff[k_invincibilitytimer])
						{
							mobj_t *overlay = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_INVULNFLASH);
							P_SetTarget(&overlay->target, player->mo);
							overlay->destscale = player->mo->scale;
							P_SetScale(overlay, player->mo->scale);
						}
						player->kartstuff[k_invincibilitytimer] = itemtime+(2*TICRATE); // 10 seconds
						K_PlayTauntSound(player->mo);
						player->kartstuff[k_itemamount]--;
						player->pflags |= PF_ATTACKDOWN;
					}
					break;
				case KITEM_BANANA:
					if (ATTACK_IS_DOWN && !HOLDING_ITEM && NO_HYUDORO)
					{
						angle_t newangle;
						fixed_t newx;
						fixed_t newy;
						INT32 moloop;
						mobj_t *mo;
						mobj_t *prev = NULL;

						//K_PlayTauntSound(player->mo);
						player->kartstuff[k_itemheld] = 1;
						player->pflags |= PF_ATTACKDOWN;

						for (moloop = 0; moloop < player->kartstuff[k_itemamount]; moloop++)
						{
							newangle = player->mo->angle;
							newx = player->mo->x + P_ReturnThrustX(player->mo, newangle + ANGLE_180, mobjinfo[MT_BANANA_SHIELD].radius);
							newy = player->mo->y + P_ReturnThrustY(player->mo, newangle + ANGLE_180, mobjinfo[MT_BANANA_SHIELD].radius);
							mo = P_SpawnMobj(newx, newy, player->mo->z, MT_BANANA_SHIELD);
							mo->threshold = 10;
							mo->movecount = player->kartstuff[k_itemamount];
							mo->lastlook = moloop+1;
							P_SetTarget(&mo->target, player->mo);
							if (moloop > 0)
							{
								P_SetTarget(&mo->hprev, prev);
								if (prev != NULL)
									P_SetTarget(&prev->hnext, mo);
							}
							else
								P_SetTarget(&player->mo->hnext, mo);
							prev = mo;
						}
					}
					else if (ATTACK_IS_DOWN && player->kartstuff[k_itemheld]) // Banana x3 thrown
					{
						K_ThrowKartItem(player, false, MT_BANANA, -1, false);
						K_PlayTauntSound(player->mo);
						player->pflags |= PF_ATTACKDOWN;
						player->kartstuff[k_itemamount]--;
						if (!player->kartstuff[k_itemamount])
							player->kartstuff[k_itemheld] = 0;
					}
					break;
				case KITEM_EGGMAN:
					if (ATTACK_IS_DOWN && !HOLDING_ITEM && NO_HYUDORO)
					{
						angle_t newangle;
						fixed_t newx;
						fixed_t newy;
						mobj_t *mo;
						player->kartstuff[k_itemamount]--;
						player->kartstuff[k_eggmanheld] = 1;
						player->pflags |= PF_ATTACKDOWN;
						newangle = player->mo->angle;
						newx = player->mo->x + P_ReturnThrustX(player->mo, newangle + ANGLE_180, mobjinfo[MT_FAKESHIELD].radius);
						newy = player->mo->y + P_ReturnThrustY(player->mo, newangle + ANGLE_180, mobjinfo[MT_FAKESHIELD].radius);
						mo = P_SpawnMobj(newx, newy, player->mo->z, MT_FAKESHIELD);
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
						INT32 moloop;

						player->kartstuff[k_itemheld] = 1;
						player->pflags |= PF_ATTACKDOWN;
						//K_PlayTauntSound(player->mo);
						S_StartSound(player->mo, sfx_s3k3a);

						for (moloop = 0; moloop < player->kartstuff[k_itemamount]; moloop++)
						{
							angle_t newangle;
							fixed_t newx;
							fixed_t newy;
							mobj_t *mo;

							newangle = player->mo->angle;
							newx = player->mo->x + P_ReturnThrustX(player->mo, newangle + ANGLE_180, 64*FRACUNIT);
							newy = player->mo->y + P_ReturnThrustY(player->mo, newangle + ANGLE_180, 64*FRACUNIT);
							mo = P_SpawnMobj(newx, newy, player->mo->z, MT_GREENSHIELD);
							if (mo)
							{
								mo->threshold = 10;
								mo->lastlook = moloop+1;
								P_SetTarget(&mo->target, player->mo);
								mo->angle = FixedAngle(((360/player->kartstuff[k_itemamount])*moloop)*FRACUNIT) + ANGLE_90;
							}
						}
					}
					else if (ATTACK_IS_DOWN && player->kartstuff[k_itemheld]) // Orbinaut x3 thrown
					{
						K_ThrowKartItem(player, true, MT_GREENITEM, 1, false);
						K_PlayTauntSound(player->mo);
						player->pflags |= PF_ATTACKDOWN;

						player->kartstuff[k_itemamount]--;
						if (!player->kartstuff[k_itemamount])
							player->kartstuff[k_itemheld] = 0;
					}
					break;
				case KITEM_JAWZ:
					if (ATTACK_IS_DOWN && !HOLDING_ITEM && NO_HYUDORO)
					{
						INT32 moloop;

						player->kartstuff[k_itemheld] = 1;
						player->pflags |= PF_ATTACKDOWN;
						//K_PlayTauntSound(player->mo);
						S_StartSound(player->mo, sfx_s3k3a);

						for (moloop = 0; moloop < player->kartstuff[k_itemamount]; moloop++)
						{
							angle_t newangle;
							fixed_t newx;
							fixed_t newy;
							mobj_t *mo;

							newangle = player->mo->angle;
							newx = player->mo->x + P_ReturnThrustX(player->mo, newangle + ANGLE_180, 64*FRACUNIT);
							newy = player->mo->y + P_ReturnThrustY(player->mo, newangle + ANGLE_180, 64*FRACUNIT);
							mo = P_SpawnMobj(newx, newy, player->mo->z, MT_JAWZ_SHIELD);
							if (mo)
							{
								mo->threshold = 10;
								mo->lastlook = moloop+1;
								P_SetTarget(&mo->target, player->mo);
								mo->angle = FixedAngle(((360/player->kartstuff[k_itemamount])*moloop)*FRACUNIT) + ANGLE_90;
							}
						}
					}
					else if (ATTACK_IS_DOWN && HOLDING_ITEM && player->kartstuff[k_itemheld]) // Jawz thrown
					{
						if (player->kartstuff[k_throwdir] == 1 || player->kartstuff[k_throwdir] == 0)
							K_ThrowKartItem(player, true, MT_JAWZ, 1, false);
						else if (player->kartstuff[k_throwdir] == -1) // Throwing backward gives you a dud that doesn't home in
							K_ThrowKartItem(player, true, MT_JAWZ_DUD, -1, false);
						K_PlayTauntSound(player->mo);
						player->pflags |= PF_ATTACKDOWN;

						player->kartstuff[k_itemamount]--;
						if (!player->kartstuff[k_itemamount])
							player->kartstuff[k_itemheld] = 0;
					}
					break;
				case KITEM_MINE:
					if (ATTACK_IS_DOWN && !HOLDING_ITEM && NO_HYUDORO)
					{
						angle_t newangle;
						fixed_t newx;
						fixed_t newy;
						mobj_t *mo;
						player->kartstuff[k_itemheld] = 1;
						player->pflags |= PF_ATTACKDOWN;
						newangle = player->mo->angle;
						newx = player->mo->x + P_ReturnThrustX(player->mo, newangle + ANGLE_180, mobjinfo[MT_SSMINE_SHIELD].radius);
						newy = player->mo->y + P_ReturnThrustY(player->mo, newangle + ANGLE_180, mobjinfo[MT_SSMINE_SHIELD].radius);
						mo = P_SpawnMobj(newx, newy, player->mo->z, MT_SSMINE_SHIELD);
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
						K_ThrowKartItem(player, true, MT_FIREBALL, 1, false);
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

						player->pflags |= PF_ATTACKDOWN;
						player->kartstuff[k_itemamount]--;

						K_PlayTauntSound(player->mo);
					}
					break;
				case KITEM_GROW:
					if (ATTACK_IS_DOWN && !HOLDING_ITEM && NO_HYUDORO
						&& player->kartstuff[k_growshrinktimer] <= 0) // Grow holds the item box hostage
					{
						if (P_IsLocalPlayer(player) && !player->exiting)
							S_ChangeMusicInternal("kgrow", true);
						if (!cv_kartinvinsfx.value && !P_IsLocalPlayer(player))
							S_StartSound(player->mo, sfx_kgrow);
						K_PlayTauntSound(player->mo);
						player->mo->scalespeed = FRACUNIT/TICRATE;
						player->mo->destscale = 3*(mapheaderinfo[gamemap-1]->mobj_scale)/2;
						player->kartstuff[k_growshrinktimer] = itemtime+(4*TICRATE); // 12 seconds
						S_StartSound(player->mo, sfx_kc5a);
						player->pflags |= PF_ATTACKDOWN;
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
				case KITEM_LIGHTNINGSHIELD:
					if (ATTACK_IS_DOWN && !HOLDING_ITEM && NO_HYUDORO)
					{
						K_DoLightningShield(player);
						player->pflags |= PF_ATTACKDOWN;
						player->kartstuff[k_itemamount]--;
					}
					break;
				case KITEM_HYUDORO:
					if (ATTACK_IS_DOWN && !HOLDING_ITEM && NO_HYUDORO)
					{
						player->kartstuff[k_itemamount]--;
						player->pflags |= PF_ATTACKDOWN;
						K_DoHyudoroSteal(player);
					}
					break;
				case KITEM_POGOSPRING:
					if (ATTACK_IS_DOWN && !HOLDING_ITEM && onground && NO_HYUDORO
						&& !player->kartstuff[k_pogospring])
					{
						K_PlayTauntSound(player->mo);
						K_DoPogoSpring(player->mo, 32<<FRACBITS);
						player->pflags |= PF_ATTACKDOWN;
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

		// Grow - Make the player grow!
		/*if (player->kartstuff[k_growshrinktimer] > 1)
			player->mo->destscale = (mapheaderinfo[gamemap-1]->mobj_scale)*3/2;*/

		if (player->kartstuff[k_itemtype] == KITEM_SPB
			|| player->kartstuff[k_itemtype] == KITEM_SHRINK
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

		if (G_BattleGametype() && player->kartstuff[k_balloon] <= 0) // dead in match? you da bomb
		{
			K_StripItems(player);
			player->mo->flags2 |= MF2_SHADOW;
			player->powers[pw_flashing] = player->kartstuff[k_comebacktimer];
		}
		else if (G_RaceGametype() || player->kartstuff[k_balloon] > 0)
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
	if (G_BattleGametype() && player->kartstuff[k_balloon] <= 0)
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
	if ((leveltime == starttime-(3*TICRATE)) || (leveltime == starttime-(2*TICRATE)) || (leveltime == starttime-TICRATE))
		S_StartSound(NULL, sfx_s3ka7);
	if (leveltime == starttime)
		S_StartSound(NULL, sfx_s3kad);

	// Start charging once you're given the opportunity.
	if (leveltime >= starttime-(2*TICRATE) && leveltime <= starttime && cmd->buttons & BT_ACCELERATE)
		player->kartstuff[k_boostcharge]++;
	if (leveltime >= starttime-(2*TICRATE) && leveltime <= starttime && !(cmd->buttons & BT_ACCELERATE))
		player->kartstuff[k_boostcharge] = 0;

	// Increase your size while charging your engine.
	if (leveltime < starttime+10)
		player->mo->destscale = (mapheaderinfo[gamemap-1]->mobj_scale) + (player->kartstuff[k_boostcharge]*131);

	// Determine the outcome of your charge.
	if (leveltime > starttime && player->kartstuff[k_boostcharge])
	{
		// Get an instant boost!
		if (player->kartstuff[k_boostcharge] >= 35 && player->kartstuff[k_boostcharge] <= 50)
		{
			if (!player->kartstuff[k_floorboost] || player->kartstuff[k_floorboost] == 3)
				S_StartSound(player->mo, sfx_s23c);

			player->kartstuff[k_sneakertimer] = -((21*(player->kartstuff[k_boostcharge]*player->kartstuff[k_boostcharge]))/425)+131; // max time is 70, min time is 7; yay parabooolas
		}
		// You overcharged your engine? Those things are expensive!!!
		else if (player->kartstuff[k_boostcharge] > 50)
		{
			player->powers[pw_nocontrol] = 40;
			S_StartSound(player->mo, sfx_slip);
		}

		player->kartstuff[k_boostcharge] = 0;
	}
}

void K_CheckBalloons(void)
{
	UINT8 i;
	UINT8 numingame = 0;
	SINT8 winnernum = -1;

#if 0
	return; // set to 1 to test comeback mechanics while alone
#endif

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

		if (players[i].kartstuff[k_balloon] <= 0) // if you don't have any balloons, you're probably not a winner
			continue;
		else if (winnernum > -1) // TWO winners? that's dumb :V
			return;
		winnernum = i;
	}

	if (numingame <= 1)
		return;

	if (playeringame[winnernum])
	{
		players[winnernum].score += 1;
		CONS_Printf(M_GetText("%s recieved a point for winning!\n"), player_names[winnernum]);
	}

	for (i = 0; i < MAXPLAYERS; i++)
		P_DoPlayerExit(&players[i]);
}

//}

//{ SRB2kart HUD Code

#define NUMPOSNUMS 10
#define NUMPOSFRAMES 7 // White, three blues, three reds

//{ 	Patch Definitions
static patch_t *kp_nodraw;

static patch_t *kp_timesticker;
static patch_t *kp_timestickerwide;
static patch_t *kp_lapsticker;
static patch_t *kp_lapstickernarrow;
static patch_t *kp_splitlapflag;
static patch_t *kp_balloonsticker;
static patch_t *kp_balloonstickerwide;
static patch_t *kp_karmasticker;
static patch_t *kp_splitkarmabomb;
static patch_t *kp_timeoutsticker;

static patch_t *kp_startcountdown[8];

static patch_t *kp_positionnum[NUMPOSNUMS][NUMPOSFRAMES];
static patch_t *kp_winnernum[NUMPOSFRAMES];

static patch_t *kp_facenull;
static patch_t *kp_facefirst;
static patch_t *kp_facesecond;
static patch_t *kp_facethird;
static patch_t *kp_facefourth;

static patch_t *kp_rankballoon;
static patch_t *kp_ranknoballoons;

static patch_t *kp_battlewin;
static patch_t *kp_battlelose;
static patch_t *kp_battlewait;
static patch_t *kp_battleinfo;

static patch_t *kp_itembg;
static patch_t *kp_itembgdark;
static patch_t *kp_itemmulsticker;
static patch_t *kp_itemx;

static patch_t *kp_sneaker;
static patch_t *kp_rocketsneaker;
static patch_t *kp_invincibility[7];
static patch_t *kp_banana;
static patch_t *kp_eggman;
static patch_t *kp_orbinaut;
static patch_t *kp_jawz;
static patch_t *kp_mine;
static patch_t *kp_ballhog;
static patch_t *kp_selfpropelledbomb;
static patch_t *kp_grow;
static patch_t *kp_shrink;
static patch_t *kp_lightningshield;
static patch_t *kp_hyudoro;
static patch_t *kp_pogospring;
static patch_t *kp_kitchensink;
static patch_t *kp_sadface;

static patch_t *kp_check[6];

static patch_t *kp_spbwarning[2];

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
	kp_balloonsticker = 		W_CachePatchName("K_STBALN", PU_HUDGFX);
	kp_balloonstickerwide = 	W_CachePatchName("K_STBALW", PU_HUDGFX);
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

	// Position numbers
	for (i = 0; i < NUMPOSNUMS; i++)
	{
		for (j = 0; j < NUMPOSFRAMES; j++)
		{
			//if (i > 4 && j < 4 && j != 0) continue;	// We don't need blue numbers for ranks past 4th
			sprintf(buffer, "K_POSN%d%d", i, j);
			kp_positionnum[i][j] = (patch_t *) W_CachePatchName(buffer, PU_HUDGFX);
		}
	}

	for (i = 0; i < NUMPOSFRAMES; i++)
	{
		sprintf(buffer, "K_POSNW%d", i);
		kp_winnernum[i] = (patch_t *) W_CachePatchName(buffer, PU_HUDGFX);
	}

	kp_facenull = 				W_CachePatchName("K_PFACE0", PU_HUDGFX);
	kp_facefirst = 				W_CachePatchName("K_PFACE1", PU_HUDGFX);
	kp_facesecond = 			W_CachePatchName("K_PFACE2", PU_HUDGFX);
	kp_facethird = 				W_CachePatchName("K_PFACE3", PU_HUDGFX);
	kp_facefourth = 			W_CachePatchName("K_PFACE4", PU_HUDGFX);

	// Extra ranking icons
	kp_rankballoon =			W_CachePatchName("K_BLNICO", PU_HUDGFX);
	kp_ranknoballoons =			W_CachePatchName("K_NOBLNS", PU_HUDGFX);

	// Battle graphics
	kp_battlewin = 				W_CachePatchName("K_BWIN", PU_HUDGFX);
	kp_battlelose = 			W_CachePatchName("K_BLOSE", PU_HUDGFX);
	kp_battlewait = 			W_CachePatchName("K_BWAIT", PU_HUDGFX);
	kp_battleinfo = 			W_CachePatchName("K_BINFO", PU_HUDGFX);

	// Kart Item Windows
	kp_itembg = 				W_CachePatchName("K_ITBG", PU_HUDGFX);
	kp_itembgdark = 			W_CachePatchName("K_ITBGD", PU_HUDGFX);
	kp_itemmulsticker = 		W_CachePatchName("K_ITMUL", PU_HUDGFX);
	kp_itemx = 					W_CachePatchName("K_ITX", PU_HUDGFX);

	kp_sneaker =				W_CachePatchName("K_ITSHOE", PU_HUDGFX);
	kp_rocketsneaker =			W_CachePatchName("K_ITRSHE", PU_HUDGFX);
	for (i = 0; i < 7; i++)
	{
		sprintf(buffer, "K_ITINV%d", i+1);
		kp_invincibility[i] = (patch_t *) W_CachePatchName(buffer, PU_HUDGFX);
	}
	kp_banana =					W_CachePatchName("K_ITBANA", PU_HUDGFX);
	kp_eggman =					W_CachePatchName("K_ITEGGM", PU_HUDGFX);
	kp_orbinaut =				W_CachePatchName("K_ITORBN", PU_HUDGFX);
	kp_jawz =					W_CachePatchName("K_ITJAWZ", PU_HUDGFX);
	kp_mine =					W_CachePatchName("K_ITMINE", PU_HUDGFX);
	kp_ballhog =				W_CachePatchName("K_ITBHOG", PU_HUDGFX);
	kp_selfpropelledbomb =		W_CachePatchName("K_ITSPB", PU_HUDGFX);
	kp_grow =					W_CachePatchName("K_ITGROW", PU_HUDGFX);
	kp_shrink =					W_CachePatchName("K_ITSHRK", PU_HUDGFX);
	kp_lightningshield =		W_CachePatchName("K_ITLITS", PU_HUDGFX);
	kp_hyudoro = 				W_CachePatchName("K_ITHYUD", PU_HUDGFX);
	kp_pogospring = 			W_CachePatchName("K_ITPOGO", PU_HUDGFX);
	kp_kitchensink = 			W_CachePatchName("K_ITSINK", PU_HUDGFX);
	kp_sadface = 				W_CachePatchName("K_ITSAD", PU_HUDGFX);

	// CHECK indicators
	for (i = 0; i < 6; i++)
	{
		sprintf(buffer, "K_CHECK%d", i+1);
		kp_check[i] = (patch_t *) W_CachePatchName(buffer, PU_HUDGFX);
	}

	// SPB warning
	kp_spbwarning[0] = 			W_CachePatchName("K_SPBW1", PU_HUDGFX);
	kp_spbwarning[1] = 			W_CachePatchName("K_SPBW2", PU_HUDGFX);
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
	SPBW_X = BASEVIDWIDTH/2;	// 270
	SPBW_Y = BASEVIDHEIGHT- 24;	// 176

	if (splitscreen)	// Splitscreen
	{
		ITEM_X = 5;
		ITEM_Y = 0;

		LAPS_Y = (BASEVIDHEIGHT/2)-24;

		POSI_Y = (BASEVIDHEIGHT/2)- 2;

		STCD_Y = BASEVIDHEIGHT/4;

		MINI_Y = (BASEVIDHEIGHT/2);

		SPBW_Y = (BASEVIDHEIGHT/2)-8;

		if (splitscreen > 1)	// 3P/4P Small Splitscreen
		{
			ITEM_X = 0;
			ITEM_Y = 0;

			LAPS_X = 3;
			LAPS_Y = (BASEVIDHEIGHT/2)-13;

			POSI_X = (BASEVIDWIDTH/2)-3;

			STCD_X = BASEVIDWIDTH/4;

			MINI_X = (3*BASEVIDWIDTH/4);
			MINI_Y = (3*BASEVIDHEIGHT/4);

			SPBW_X = BASEVIDWIDTH/4;

			if (splitscreen > 2) // 4P-only
			{
				MINI_X = (BASEVIDWIDTH/2);
				MINI_Y = (BASEVIDHEIGHT/2);
			}
		}
	}
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
	patch_t *localpatch = kp_nodraw;
	patch_t *localbg = kp_itembg;
	patch_t *localinv = kp_invincibility[(leveltime % (7*3)) / 3];
	INT32 splitflags = K_calcSplitFlags(V_SNAPTOTOP|V_SNAPTOLEFT);

	if (stplyr->kartstuff[k_itemroulette])
	{
		switch((stplyr->kartstuff[k_itemroulette] % (13*3)) / 3)
		{
			// Each case is handled in threes, to give three frames of in-game time to see the item on the roulette
			case  0: localpatch = kp_sneaker; break;			// Sneaker
			case  1: localpatch = kp_banana; break;			// Banana
			case  2: localpatch = kp_orbinaut; break;			// Orbinaut
			case  3: localpatch = kp_mine; break;				// Mine
			case  4: localpatch = kp_grow; break;				// Grow
			case  5: localpatch = kp_hyudoro; break;			// Hyudoro
			case  6: localpatch = kp_rocketsneaker; break;		// Rocket Sneaker
			case  7: localpatch = kp_jawz; break;				// Jawz
			case  8: localpatch = kp_selfpropelledbomb; break;	// Self-Propelled Bomb
			case  9: localpatch = kp_shrink; break;			// Shrink
			case 10: localpatch = localinv; break;				// Invincibility
			case 11: localpatch = kp_eggman; break;			// Eggman Monitor
			case 12: localpatch = kp_ballhog; break;			// Ballhog
			case 13: localpatch = kp_lightningshield; break;	// Lightning Shield
			//case 14: localpatch = kp_pogospring; break;		// Pogo Spring
			//case 15: localpatch = kp_kitchensink; break;		// Kitchen Sink
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
				localpatch = kp_hyudoro;
			else if (!(leveltime & 2))
				localpatch = kp_nodraw;
		}
		else if ((stplyr->kartstuff[k_stealingtimer] > 0) && (leveltime & 2))
		{
			localpatch = kp_hyudoro;
		}
		else if (stplyr->kartstuff[k_rocketsneakertimer] > 1)
		{
			if (leveltime & 1)
				localpatch = kp_rocketsneaker;
			else if (!(leveltime & 1))
				localpatch = kp_nodraw;
		}
		else if (stplyr->kartstuff[k_growshrinktimer] > 1)
		{
			if (leveltime & 1)
				localpatch = kp_grow;
			else if (!(leveltime & 1))
				localpatch = kp_nodraw;
		}
		else if (stplyr->kartstuff[k_sadtimer] > 0)
		{
			if (leveltime & 2)
				localpatch = kp_sadface;
			else if (!(leveltime & 2))
				localpatch = kp_nodraw;
		}
		else
		{
			if (!(stplyr->kartstuff[k_itemamount] || stplyr->kartstuff[k_itemheld]))
				return;

			switch(stplyr->kartstuff[k_itemtype])
			{
				case KITEM_SNEAKER:				localpatch = kp_sneaker; break;
				case KITEM_ROCKETSNEAKER:		localpatch = kp_rocketsneaker; break;
				case KITEM_INVINCIBILITY:		localpatch = localinv; localbg = kp_itembgdark; break;
				case KITEM_BANANA:				localpatch = kp_banana; break;
				case KITEM_EGGMAN:				localpatch = kp_eggman; break;
				case KITEM_ORBINAUT:			localpatch = kp_orbinaut; break;
				case KITEM_JAWZ:				localpatch = kp_jawz; break;
				case KITEM_MINE:				localpatch = kp_mine; break;
				case KITEM_BALLHOG:				localpatch = kp_ballhog; break;
				case KITEM_SPB:					localpatch = kp_selfpropelledbomb; break;
				case KITEM_GROW:				localpatch = kp_grow; break;
				case KITEM_SHRINK:				localpatch = kp_shrink; break;
				case KITEM_LIGHTNINGSHIELD:		localpatch = kp_lightningshield; break;
				case KITEM_HYUDORO:				localpatch = kp_hyudoro; break;
				case KITEM_POGOSPRING:			localpatch = kp_pogospring; break;
				case KITEM_KITCHENSINK:			localpatch = kp_kitchensink; break;
				case KITEM_SAD:					localpatch = kp_sadface; break;
				default: return;
			}
		}
	}

	V_DrawScaledPatch(ITEM_X, ITEM_Y, V_HUDTRANS|splitflags, localbg);

	// Then, the numbers:
	if (stplyr->kartstuff[k_itemamount] > 1 && !stplyr->kartstuff[k_itemroulette])
	{
		V_DrawScaledPatch(ITEM_X+9, ITEM_Y+24, V_HUDTRANS|splitflags, kp_itemmulsticker);
		V_DrawScaledPatch(ITEM_X, ITEM_Y, V_HUDTRANS|splitflags, localpatch);
		V_DrawScaledPatch(ITEM_X+28, ITEM_Y+41, V_HUDTRANS|splitflags, kp_itemx);
		V_DrawKartString(ITEM_X+38, ITEM_Y+36, V_HUDTRANS|splitflags, va("%d", stplyr->kartstuff[k_itemamount]));
	}
	else
		V_DrawScaledPatch(ITEM_X, ITEM_Y, V_HUDTRANS|splitflags, localpatch);
}

static void K_drawKartTimestamp(void)
{
	// TIME_X = BASEVIDWIDTH-124;	// 196
	// TIME_Y = 6;					//   6

	INT32 TIME_XB;
	INT32 splitflags = K_calcSplitFlags(V_SNAPTOTOP|V_SNAPTORIGHT);

	V_DrawScaledPatch(TIME_X, TIME_Y, V_HUDTRANS|splitflags, kp_timestickerwide);

	TIME_XB = TIME_X+33;

	if (stplyr->realtime/(60*TICRATE) < 100) // 99:99:99 only
	{
		// zero minute
		if (stplyr->realtime/(60*TICRATE) < 10)
		{
			V_DrawKartString(TIME_XB, TIME_Y+3, V_HUDTRANS|splitflags, va("0"));
			// minutes time       0 __ __
			V_DrawKartString(TIME_XB+12, TIME_Y+3, V_HUDTRANS|splitflags, va("%d", stplyr->realtime/(60*TICRATE)));
		}
		// minutes time       0 __ __
		else
			V_DrawKartString(TIME_XB, TIME_Y+3, V_HUDTRANS|splitflags, va("%d", stplyr->realtime/(60*TICRATE)));

		// apostrophe location     _'__ __
		V_DrawKartString(TIME_XB+24, TIME_Y+3, V_HUDTRANS|splitflags, va("'"));

		// zero second        _ 0_ __
		if ((stplyr->realtime/TICRATE % 60) < 10)
		{
			V_DrawKartString(TIME_XB+36, TIME_Y+3, V_HUDTRANS|splitflags, va("0"));
		// seconds time       _ _0 __
			V_DrawKartString(TIME_XB+48, TIME_Y+3, V_HUDTRANS|splitflags, va("%d", stplyr->realtime/TICRATE % 60));
		}
		// zero second        _ 00 __
		else
			V_DrawKartString(TIME_XB+36, TIME_Y+3, V_HUDTRANS|splitflags, va("%d", stplyr->realtime/TICRATE % 60));

		// quotation mark location    _ __"__
		V_DrawKartString(TIME_XB+60, TIME_Y+3, V_HUDTRANS|splitflags, va("\""));

		// zero tick          _ __ 0_
		if (G_TicsToCentiseconds(stplyr->realtime) < 10)
		{
			V_DrawKartString(TIME_XB+72, TIME_Y+3, V_HUDTRANS|splitflags, va("0"));
		// tics               _ __ _0
			V_DrawKartString(TIME_XB+84, TIME_Y+3, V_HUDTRANS|splitflags, va("%d", G_TicsToCentiseconds(stplyr->realtime)));
		}
		// zero tick          _ __ 00
		if (G_TicsToCentiseconds(stplyr->realtime) >= 10)
			V_DrawKartString(TIME_XB+72, TIME_Y+3, V_HUDTRANS|splitflags, va("%d", G_TicsToCentiseconds(stplyr->realtime)));
	}
	else
		V_DrawKartString(TIME_XB, TIME_Y+3, V_HUDTRANS|splitflags, va("99'59\"99"));
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
		scale = FixedMul(scale, 3*FRACUNIT/2);
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
		{
			// Alternate frame every three frames
			switch (leveltime % 21)
			{
				case 1: case 2: case 3:
					localpatch = kp_winnernum[0];
					break;
				case 4: case 5: case 6:
					localpatch = kp_winnernum[1];
					break;
				case 7: case 8: case 9:
					localpatch = kp_winnernum[2];
					break;
				case 10: case 11: case 12:
					localpatch = kp_winnernum[3];
					break;
				case 13: case 14: case 15:
					localpatch = kp_winnernum[4];
					break;
				case 16: case 17: case 18:
					localpatch = kp_winnernum[5];
					break;
				case 19: case 20: case 21:
					localpatch = kp_winnernum[6];
					break;
				default:
					localpatch = kp_positionnum[1][0];
					break;
			}
		}
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

static void K_drawKartPositionFaces(void)
{
	// FACE_X = 15;				//  15
	// FACE_Y = 72;				//  72

	INT32 Y = FACE_Y+9; // +9 to offset where it's being drawn if there are more than one
	INT32 i, j, ranklines;
	boolean completed[MAXPLAYERS];
	INT32 rankplayer[MAXPLAYERS];
	INT32 rankcolor[MAXPLAYERS];
	INT32 myplayer;
	INT32 balloonx;
	UINT8 *colormap;
	patch_t *localpatch = kp_facenull;

	ranklines = 0;
	memset(completed, 0, sizeof (completed));
	memset(rankplayer, 0, sizeof (rankplayer));
	memset(rankcolor, 0, sizeof (rankcolor));
	myplayer = demoplayback ? displayplayer : consoleplayer;

	for (i = 0; i < MAXPLAYERS; i++)
		rankplayer[i] = -1;

	for (j = 0; j < MAXPLAYERS; j++)
	{
		if (!playeringame[j] || players[j].spectator || !players[j].mo)
			continue;

		for (i = 0; i < MAXPLAYERS; i++)
		{
			if (playeringame[i] && completed[i] == false && players[i].mo && !players[i].spectator
				&& (rankplayer[ranklines] < 0 || players[i].kartstuff[k_position] < players[rankplayer[ranklines]].kartstuff[k_position]))
			{
				rankplayer[ranklines] = i;
				rankcolor[ranklines] = players[i].skincolor;
			}
		}
		completed[rankplayer[ranklines]] = true;
		ranklines++;
	}

	if (ranklines > 4) ranklines = 4; // Only draw the top 4 players

	Y -= (9*ranklines);

	for (i = 0; i < ranklines; i++)
	{
		if (players[rankplayer[i]].spectator) continue; // Spectators are ignored
		if (!players[rankplayer[i]].mo) continue;

		balloonx = FACE_X+18;

		if (rankcolor[i] == 0)
		{
			colormap = colormaps;
			if (rankplayer[i] != myplayer)
			{
				V_DrawSmallTranslucentPatch(FACE_X, Y, V_HUDTRANS|V_SNAPTOLEFT, faceprefix[players[rankplayer[i]].skin]);
				if (G_BattleGametype() && players[rankplayer[i]].kartstuff[k_balloon] > 0)
				{
					for (j = 0; j < players[rankplayer[i]].kartstuff[k_balloon]; j++)
					{
						V_DrawSmallTranslucentPatch(balloonx, Y+10, V_HUDTRANS|V_SNAPTOLEFT, kp_rankballoon);
						balloonx += 3;
					}
				}
			}
			else
			{
				V_DrawSmallScaledPatch(FACE_X, Y, V_HUDTRANS|V_SNAPTOLEFT, faceprefix[players[rankplayer[i]].skin]);
				if (G_BattleGametype() && players[rankplayer[i]].kartstuff[k_balloon] > 0)
				{
					for (j = 0; j < players[rankplayer[i]].kartstuff[k_balloon]; j++)
					{
						V_DrawSmallScaledPatch(balloonx, Y+10, V_HUDTRANS|V_SNAPTOLEFT, kp_rankballoon);
						balloonx += 3;
					}
				}
			}
		}
		else
		{
			colormap = R_GetTranslationColormap(players[rankplayer[i]].skin, players[rankplayer[i]].mo->color, GTC_CACHE);
			if (players[rankplayer[i]].mo->colorized)
			{
				colormap = R_GetTranslationColormap(TC_RAINBOW, players[rankplayer[i]].mo->color, GTC_CACHE);
			}
			else
			{
				colormap = R_GetTranslationColormap(players[rankplayer[i]].skin, players[rankplayer[i]].mo->color, GTC_CACHE);
			}

			if (rankplayer[i] != myplayer)
			{
				V_DrawSmallTranslucentMappedPatch(FACE_X, Y, V_HUDTRANS|V_SNAPTOLEFT, faceprefix[players[rankplayer[i]].skin], colormap);
				if (G_BattleGametype() && players[rankplayer[i]].kartstuff[k_balloon] > 0)
				{
					for (j = 0; j < players[rankplayer[i]].kartstuff[k_balloon]; j++)
					{
						V_DrawSmallTranslucentMappedPatch(balloonx, Y+10, V_HUDTRANS|V_SNAPTOLEFT, kp_rankballoon, colormap);
						balloonx += 3;
					}
				}
			}
			else
			{
				V_DrawSmallMappedPatch(FACE_X, Y, V_HUDTRANS|V_SNAPTOLEFT, faceprefix[players[rankplayer[i]].skin], colormap);
				if (G_BattleGametype() && players[rankplayer[i]].kartstuff[k_balloon] > 0)
				{
					for (j = 0; j < players[rankplayer[i]].kartstuff[k_balloon]; j++)
					{
						V_DrawSmallMappedPatch(balloonx, Y+10, V_HUDTRANS|V_SNAPTOLEFT, kp_rankballoon, colormap);
						balloonx += 3;
					}
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

		if (rankplayer[i] != myplayer)
		{
			if (G_BattleGametype() && players[rankplayer[i]].kartstuff[k_balloon] <= 0)
				V_DrawSmallTranslucentPatch(FACE_X-2, Y, V_HUDTRANS|V_SNAPTOLEFT, kp_ranknoballoons);
			else
				V_DrawSmallTranslucentPatch(FACE_X, Y, V_HUDTRANS|V_SNAPTOLEFT, localpatch);
		}
		else
		{
			if (G_BattleGametype() && players[rankplayer[i]].kartstuff[k_balloon] <= 0)
				V_DrawSmallScaledPatch(FACE_X-2, Y, V_HUDTRANS|V_SNAPTOLEFT, kp_ranknoballoons);
			else
				V_DrawSmallScaledPatch(FACE_X, Y, V_HUDTRANS|V_SNAPTOLEFT, localpatch);
		}

		Y += 18;
	}
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

static void K_drawKartBalloonsOrKarma(void)
{
	UINT8 *colormap = R_GetTranslationColormap(TC_DEFAULT, stplyr->skincolor, 0);
	INT32 splitflags = K_calcSplitFlags(V_SNAPTOBOTTOM|V_SNAPTOLEFT);

	if (splitscreen > 1)
	{
		if (stplyr->kartstuff[k_balloon] <= 0)
		{
			V_DrawMappedPatch(LAPS_X, LAPS_Y-1, V_HUDTRANS|splitflags, kp_splitkarmabomb, colormap);
			V_DrawString(LAPS_X+13, LAPS_Y+1, V_HUDTRANS|splitflags, va("%d/3", stplyr->kartstuff[k_comebackpoints]));
		}
		else
		{
			V_DrawMappedPatch(LAPS_X, LAPS_Y-1, V_HUDTRANS|splitflags, kp_rankballoon, colormap);
			V_DrawString(LAPS_X+13, LAPS_Y+1, V_HUDTRANS|splitflags, va("%d/%d", stplyr->kartstuff[k_balloon], cv_kartballoons.value));
		}
	}
	else
	{
		if (stplyr->kartstuff[k_balloon] <= 0)
		{
			V_DrawMappedPatch(LAPS_X, LAPS_Y, V_HUDTRANS|splitflags, kp_karmasticker, colormap);
			V_DrawKartString(LAPS_X+59, LAPS_Y+3, V_HUDTRANS|splitflags, va("%d/3", stplyr->kartstuff[k_comebackpoints]));
		}
		else
		{
			if (stplyr->kartstuff[k_balloon] > 9 && cv_kartballoons.value > 9)
				V_DrawMappedPatch(LAPS_X, LAPS_Y, V_HUDTRANS|splitflags, kp_balloonstickerwide, colormap);
			else
				V_DrawMappedPatch(LAPS_X, LAPS_Y, V_HUDTRANS|splitflags, kp_balloonsticker, colormap);
			V_DrawKartString(LAPS_X+47, LAPS_Y+3, V_HUDTRANS|splitflags, va("%d/%d", stplyr->kartstuff[k_balloon], cv_kartballoons.value));
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

		if (!playeringame[i] || players[i].spectator)
			continue;
		if (!players[i].mo)
			continue;
		if (&players[i] == stplyr)
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

static void K_drawKartMinimapHead(player_t *player, INT32 x, INT32 y, INT32 flags, patch_t *AutomapPic)
{
	// amnum xpos & ypos are the icon's speed around the HUD.
	// The number being divided by is for how fast it moves.
	// The higher the number, the slower it moves.

	// am xpos & ypos are the icon's starting position. Withouht
	// it, they wouldn't 'spawn' on the top-right side of the HUD.

	fixed_t amnumxpos;
	fixed_t amnumypos;
	INT32 amxpos;
	INT32 amypos;

	node_t *bsp = &nodes[numnodes-1];
	fixed_t maxx, minx, maxy, miny;
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

	fixed_t mapwidth = maxx - minx;
	fixed_t mapheight = maxy - miny;

	// These should always be small enough to be bitshift back right now
	fixed_t xoffset = (minx + mapwidth/2)<<FRACBITS;
	fixed_t yoffset = (miny + mapheight/2)<<FRACBITS;

	fixed_t xscale = FixedDiv(AutomapPic->width, mapwidth);
	fixed_t yscale = FixedDiv(AutomapPic->height, mapheight);
	fixed_t zoom = FixedMul(min(xscale, yscale), FRACUNIT-FRACUNIT/20);

	amnumxpos = (FixedMul(player->mo->x, zoom) - FixedMul(xoffset, zoom));
	amnumypos = -(FixedMul(player->mo->y, zoom) - FixedMul(yoffset, zoom));

	amxpos = amnumxpos + ((x + AutomapPic->width/2 - (iconprefix[player->skin]->width/2))<<FRACBITS);
	amypos = amnumypos + ((y + AutomapPic->height/2 - (iconprefix[player->skin]->height/2))<<FRACBITS);

	if (mirrormode)
	{
		flags |= V_FLIP;
		amxpos = -amnumxpos + ((x + AutomapPic->width/2 + (iconprefix[player->skin]->width/2))<<FRACBITS);
	}

	if (!player->skincolor) // 'default' color
		V_DrawSciencePatch(amxpos, amypos, flags, iconprefix[player->skin], FRACUNIT);
	else
	{
		UINT8 *colormap;
		if (player->mo->colorized)
		{
			colormap = R_GetTranslationColormap(TC_RAINBOW, player->mo->color, 0);
		}
		else
		{
			colormap = R_GetTranslationColormap(player->skin, player->mo->color, 0);
		}
		V_DrawFixedPatch(amxpos, amypos, FRACUNIT, flags, iconprefix[player->skin], colormap);
	}
}

static void K_drawKartMinimap(void)
{
	INT32 lumpnum;
	patch_t *AutomapPic;
	INT32 i = 0;
	INT32 x, y;
	const INT32 minimaptrans = ((10-cv_kartminimap.value)<<FF_TRANSSHIFT);
	INT32 splitflags = V_SNAPTORIGHT|minimaptrans;

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

	if (splitscreen)
		splitflags = 0;

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

	// Player's tiny icons on the Automap.
	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (i == displayplayer && !splitscreen)
			continue; // Do displayplayer later
		if (players[i].mo && !players[i].spectator)
		{
			if (G_BattleGametype() && players[i].kartstuff[k_balloon] <= 0)
				continue;

			if (players[i].kartstuff[k_hyudorotimer] > 0)
			{
				if ((players[i].kartstuff[k_hyudorotimer] < 1*TICRATE/2
					|| players[i].kartstuff[k_hyudorotimer] > hyudorotime-(1*TICRATE/2))
					&& !(leveltime & 1))
					;
				else
					continue;
			}

			K_drawKartMinimapHead(&players[i], x, y, splitflags, AutomapPic);
		}
	}

	if (splitscreen)
		return; // Don't need this for splits

	splitflags &= ~V_HUDTRANSHALF;
	splitflags |= V_HUDTRANS;
	if (stplyr->mo && !stplyr->spectator)
		K_drawKartMinimapHead(stplyr, x, y, splitflags, AutomapPic);
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
			V_DrawFadeScreen();
		if (stplyr->kartstuff[k_balloon])
			V_DrawFixedPatch(x<<FRACBITS, y<<FRACBITS, scale, splitflags, kp_battlewin, NULL);
		else if (splitscreen < 2)
			V_DrawFixedPatch(x<<FRACBITS, y<<FRACBITS, scale, splitflags, kp_battlelose, NULL);
	}
	else if (stplyr->kartstuff[k_balloon] <= 0 && stplyr->kartstuff[k_comebacktimer] && comeback)
	{
		INT32 t = stplyr->kartstuff[k_comebacktimer]/TICRATE;
		INT32 txoff = 0;
		INT32 ty = (BASEVIDHEIGHT/2)+66;

		if (t == 0)
			txoff = 8;
		else
		{
			while (t)
			{
				txoff += 8;
				t /= 10;
			}
		}

		if (splitscreen)
		{
			if (splitscreen > 2)
				ty = (BASEVIDHEIGHT/4)+33;
			if ((splitscreen == 1 && stplyr == &players[secondarydisplayplayer])
				|| (stplyr == &players[thirddisplayplayer] && splitscreen > 1)
				|| (stplyr == &players[fourthdisplayplayer] && splitscreen > 2))
				ty += (BASEVIDHEIGHT/2);
		}
		else
			V_DrawFadeScreen();

		if (!comebackshowninfo)
			V_DrawFixedPatch(x<<FRACBITS, y<<FRACBITS, scale, splitflags, kp_battleinfo, NULL);
		else
			V_DrawFixedPatch(x<<FRACBITS, y<<FRACBITS, scale, splitflags, kp_battlewait, NULL);

		if (splitscreen > 1)
			V_DrawString(x-(txoff/2), ty, 0, va("%d", stplyr->kartstuff[k_comebacktimer]/TICRATE));
		else
		{
			V_DrawFixedPatch(x<<FRACBITS, ty<<FRACBITS, scale, 0, kp_timeoutsticker, NULL);
			V_DrawKartString(x-txoff, ty, 0, va("%d", stplyr->kartstuff[k_comebacktimer]/TICRATE));
		}
	}
}

static void K_drawStartCountdown(void)
{
	INT32 pnum = 0; // 3

	if (leveltime >= starttime-(2*TICRATE)) // 2
		pnum++;
	if (leveltime >= starttime-TICRATE) // 1
		pnum++;
	if (leveltime >= starttime) // GO!
		pnum++;
	if ((leveltime % (2*5)) / 5) // blink
		pnum += 4;

	V_DrawScaledPatch(STCD_X - (SHORT(kp_startcountdown[pnum]->width)/2), STCD_Y - (SHORT(kp_startcountdown[pnum]->height)/2), 0, kp_startcountdown[pnum]);
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
	// Define the X and Y for each drawn object
	// This is handled by console/menu values
	K_initKartHUD();

	if (splitscreen == 2) // Player 4 in 3P is basically the minimap :p
		K_drawKartMinimap();

	// Draw full screen stuff that turns off the rest of the HUD
	if ((G_BattleGametype())
		&& (stplyr->exiting
		|| (stplyr->kartstuff[k_balloon] <= 0
		&& stplyr->kartstuff[k_comebacktimer]
		&& comeback
		&& stplyr->playerstate == PST_LIVE)))
	{
		K_drawBattleFullscreen();
		return;
	}

	// Draw the CHECK indicator before the other items, so it's overlapped by everything else
	if (cv_kartcheck.value && !splitscreen)
		K_drawKartPlayerCheck();

	if (splitscreen == 0 && cv_kartminimap.value)
		K_drawKartMinimap(); // 3P splitscreen is handled above

	// Draw the item window
	K_drawKartItem();

	// If not splitscreen, draw...
	if (!splitscreen)
	{
		// Draw the timestamp
		K_drawKartTimestamp();

		if (!modeattacking)
		{
			// The little triple-item icons at the bottom
			// The top-four faces on the left
			K_drawKartPositionFaces();
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

			if (!modeattacking)
			{
				// Draw the numerical position
				K_DrawKartPositionNum(stplyr->kartstuff[k_position]);
			}

			// You're about to DIEEEEE
			K_drawSPBWarning();
		}
		else if (G_BattleGametype()) // Battle-only
		{
			// Draw the hits left!
			K_drawKartBalloonsOrKarma();
		}
	}

	// Draw the starting countdown after everything else.
	if (leveltime >= starttime-(3*TICRATE)
		&& leveltime < starttime+TICRATE)
		K_drawStartCountdown();

	if (cv_kartdebugcheckpoint.value)
		K_drawCheckpointDebugger();
}

//}

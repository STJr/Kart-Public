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

//{ SRB2kart Color Code

#define SKIN_RAMP_LENGTH 16
#define DEFAULT_STARTTRANSCOLOR 160
#define NUM_PALETTE_ENTRIES 256

const char *KartColor_Names[MAXSKINCOLORS] =
{
	"None",                   // 00 // SKINCOLOR_NONE
	"Ivory",                  // 01 // SKINCOLOR_IVORY
	"White",                  // 02 // SKINCOLOR_WHITE
	"Silver",                 // 03 // SKINCOLOR_SILVER
	"Cloudy",                 // 04 // SKINCOLOR_CLOUDY
	"Grey",                   // 05 // SKINCOLOR_GREY
	"Dark Grey",              // 06 // SKINCOLOR_DARKGREY
	"Black",                  // 07 // SKINCOLOR_BLACK
	"Salmon",                 // 08 // SKINCOLOR_SALMON
	"Pink",                   // 09 // SKINCOLOR_PINK
	"Light Red",              // 10 // SKINCOLOR_LIGHTRED
	"Full-Range Red",         // 11 // SKINCOLOR_FULLRANGERED
	"Red",                    // 12 // SKINCOLOR_RED
	"Dark Pink",              // 13 // SKINCOLOR_DARKPINK
	"Dark Red",               // 14 // SKINCOLOR_DARKRED
	"Dawn",                   // 15 // SKINCOLOR_DAWN
	"Orange",                 // 16 // SKINCOLOR_ORANGE
	"Full-Range Orange",      // 17 // SKINCOLOR_FULLRANGEORANGE
	"Dark Orange",            // 18 // SKINCOLOR_DARKORANGE
	"Golden Brown",           // 19 // SKINCOLOR_GOLDENBROWN
	"Rosewood",               // 20 // SKINCOLOR_ROSEWOOD
	"Dark Rosewood",          // 21 // SKINCOLOR_DARKROSEWOOD
	"Sepia",                  // 22 // SKINCOLOR_SEPIA
	"Beige",                  // 23 // SKINCOLOR_BEIGE
	"Brown",                  // 24 // SKINCOLOR_BROWN
	"Leather",                // 25 // SKINCOLOR_LEATHER
	"Yellow",                 // 26 // SKINCOLOR_YELLOW
	"Peach",                  // 27 // SKINCOLOR_PEACH
	"Light Orange",           // 28 // SKINCOLOR_LIGHTORANGE
	"Peach-Brown",            // 29 // SKINCOLOR_PEACHBROWN
	"Gold",                   // 30 // SKINCOLOR_GOLD
	"Full-Range Peach-Brown", // 31 // SKINCOLOR_FULLRANGEPEACHBROWN
	"Gypsy Vomit",            // 32 // SKINCOLOR_GYPSYVOMIT
	"Garden",                 // 33 // SKINCOLOR_GARDEN
	"Light Army",             // 34 // SKINCOLOR_LIGHTARMY
	"Army",                   // 35 // SKINCOLOR_ARMY
	"Pistachio",              // 36 // SKINCOLOR_PISTACHIO
	"Robo-Hood Green",        // 37 // SKINCOLOR_ROBOHOODGREEN
	"Olive",                  // 38 // SKINCOLOR_OLIVE
	"Dark Army",              // 39 // SKINCOLOR_DARKARMY
	"Light Green",            // 40 // SKINCOLOR_LIGHTGREEN
	"Ugly Green",             // 41 // SKINCOLOR_UGLYGREEN
	"Neon Green",             // 42 // SKINCOLOR_NEONGREEN
	"Green",                  // 43 // SKINCOLOR_GREEN
	"Dark Green",             // 44 // SKINCOLOR_DARKGREEN
	"Dark Neon Green",        // 45 // SKINCOLOR_DARKNEONGREEN
	"Frost",                  // 46 // SKINCOLOR_FROST
	"Light Steel Blue",       // 47 // SKINCOLOR_LIGHTSTEELBLUE
	"Light Blue",             // 48 // SKINCOLOR_LIGHTBLUE
	"Cyan",                   // 49 // SKINCOLOR_CYAN
	"Cerulean",               // 50 // SKINCOLOR_CERULEAN
	"Turquoise",              // 51 // SKINCOLOR_TURQUOISE
	"Teal",                   // 52 // SKINCOLOR_TEAL
	"Steel Blue",             // 53 // SKINCOLOR_STEELBLUE
	"Blue",                   // 54 // SKINCOLOR_BLUE
	"Full-Range Blue",        // 55 // SKINCOLOR_FULLRANGEBLUE
	"Dark Steel Blue",        // 56 // SKINCOLOR_DARKSTEELBLUE
	"Dark Blue",              // 57 // SKINCOLOR_DARKBLUE
	"Jet Black",              // 58 // SKINCOLOR_JETBLACK
	"Lilac",                  // 59 // SKINCOLOR_LILAC
	"Purple",                 // 60 // SKINCOLOR_PURPLE
	"Lavender",               // 61 // SKINCOLOR_LAVENDER
	"Byzantium",              // 62 // SKINCOLOR_BYZANTIUM
	"Indigo"                  // 63 // SKINCOLOR_INDIGO
};

/**	\brief	Generates a simple case table for given values. Not very optimal, but makes it easy to read in K_GenerateKartColormap.

	\param	i		loop iteration
	\param	cNumber	Numeric color value, from Zero to Fifteen

	\return	INT32	Returns the pulled value of the sixteen fed to it
*/
static INT32 R_KartColorSetter(UINT8 i,
	INT32 cZero,   INT32 cOne,      INT32 cTwo,      INT32 cThree,
	INT32 cFour,   INT32 cFive,     INT32 cSix,      INT32 cSeven,
	INT32 cEight,  INT32 cNine,     INT32 cTen,      INT32 cEleven,
	INT32 cTwelve, INT32 cThirteen, INT32 cFourteen, INT32 cFifteen)
{
	INT32 ThisColorIs = 0;

	switch (i)
	{
		case 0:  ThisColorIs = cZero;     break;
		case 1:  ThisColorIs = cOne;      break;
		case 2:  ThisColorIs = cTwo;      break;
		case 3:  ThisColorIs = cThree;    break;
		case 4:  ThisColorIs = cFour;     break;
		case 5:  ThisColorIs = cFive;     break;
		case 6:  ThisColorIs = cSix;      break;
		case 7:  ThisColorIs = cSeven;    break;
		case 8:  ThisColorIs = cEight;    break;
		case 9:  ThisColorIs = cNine;     break;
		case 10: ThisColorIs = cTen;      break;
		case 11: ThisColorIs = cEleven;   break;
		case 12: ThisColorIs = cTwelve;   break;
		case 13: ThisColorIs = cThirteen; break;
		case 14: ThisColorIs = cFourteen; break;
		case 15: ThisColorIs = cFifteen;  break;
	}

	return ThisColorIs;
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

	starttranscolor = (skinnum != TC_DEFAULT) ? skins[skinnum].starttranscolor : DEFAULT_STARTTRANSCOLOR;

	// Fill in the entries of the palette that are fixed
	for (i = 0; i < starttranscolor; i++)
		dest_colormap[i] = (UINT8)i;

	for (i = (UINT8)(starttranscolor + 16); i < NUM_PALETTE_ENTRIES; i++)
		dest_colormap[i] = (UINT8)i;

	// Build the translated ramp
	for (i = 0; i < SKIN_RAMP_LENGTH; i++)
		switch (color)
		{
			case SKINCOLOR_IVORY:				dest_colormap[starttranscolor + i] = R_KartColorSetter(i, 120, 120, 120, 120,   0,   0,   0,   0,   1,   1,   2,   2,   4,   6,   8,  10); break;
			case SKINCOLOR_WHITE:				dest_colormap[starttranscolor + i] = R_KartColorSetter(i,   0,   0,   1,   1,   2,   2,   3,   3,   4,   4,   5,   5,   6,   6,   7,   7); break;
			case SKINCOLOR_SILVER:				dest_colormap[starttranscolor + i] = R_KartColorSetter(i,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15); break;
			case SKINCOLOR_CLOUDY:				dest_colormap[starttranscolor + i] = R_KartColorSetter(i,   1,   3,   5,   7,   9,  11,  13,  15,  17,  19,  21,  23,  25,  27,  29,  31); break;
			case SKINCOLOR_GREY:				dest_colormap[starttranscolor + i] = R_KartColorSetter(i,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23); break;
			case SKINCOLOR_DARKGREY:			dest_colormap[starttranscolor + i] = R_KartColorSetter(i,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31); break;
			case SKINCOLOR_BLACK:				dest_colormap[starttranscolor + i] = R_KartColorSetter(i,  24,  24,  25,  25,  26,  26,  27,  27,  28,  28,  29,  29,  30,  30,  31,  31); break;
			case SKINCOLOR_SALMON:				dest_colormap[starttranscolor + i] = R_KartColorSetter(i, 120, 120, 121, 121, 122, 122, 123, 123, 124, 124, 125, 125, 126, 126, 127, 127); break;
			case SKINCOLOR_PINK:				dest_colormap[starttranscolor + i] = R_KartColorSetter(i, 144, 144, 145, 145, 146, 146, 147, 147, 148, 148, 149, 149, 150, 150, 151, 151); break;
			case SKINCOLOR_LIGHTRED:			dest_colormap[starttranscolor + i] = R_KartColorSetter(i, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135); break;
			case SKINCOLOR_FULLRANGERED:		dest_colormap[starttranscolor + i] = R_KartColorSetter(i, 120, 121, 123, 124, 126, 127, 129, 130, 132, 133, 135, 136, 138, 139, 141, 143); break;
			case SKINCOLOR_RED:					dest_colormap[starttranscolor + i] = R_KartColorSetter(i, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140); break;
			case SKINCOLOR_DARKPINK:			dest_colormap[starttranscolor + i] = R_KartColorSetter(i, 144, 145, 146, 147, 148, 149, 150, 151, 134, 135, 136, 137, 138, 139, 140, 141); break;
			case SKINCOLOR_DARKRED:				dest_colormap[starttranscolor + i] = R_KartColorSetter(i, 136, 136, 137, 137, 138, 138, 139, 139, 140, 140, 141, 141, 142, 142, 143, 143); break;
			case SKINCOLOR_DAWN:				dest_colormap[starttranscolor + i] = R_KartColorSetter(i, 120, 121, 122, 123, 124, 147,  88,  89, 149,  91,  92, 151,  94,  95, 152, 153); break;
			case SKINCOLOR_ORANGE:				dest_colormap[starttranscolor + i] = R_KartColorSetter(i,  80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95); break;
			case SKINCOLOR_FULLRANGEORANGE:		dest_colormap[starttranscolor + i] = R_KartColorSetter(i,  80,  81,  83,  85,  86,  88,  90,  91,  93,  95, 152, 153, 154, 156, 157, 159); break;
			case SKINCOLOR_DARKORANGE:			dest_colormap[starttranscolor + i] = R_KartColorSetter(i,  88,  89,  90,  91,  92,  93,  94,  95, 152, 153, 154, 155, 156, 157, 158, 159); break;
			case SKINCOLOR_GOLDENBROWN:			dest_colormap[starttranscolor + i] = R_KartColorSetter(i, 112, 113, 114, 115, 116, 117, 118, 119, 156, 156, 157, 157, 158, 158, 159, 159); break;
			case SKINCOLOR_ROSEWOOD:			dest_colormap[starttranscolor + i] = R_KartColorSetter(i, 152, 152, 153, 153, 154, 154, 155, 155, 156, 156, 157, 157, 158, 158, 159, 159); break;
			case SKINCOLOR_DARKROSEWOOD:		dest_colormap[starttranscolor + i] = R_KartColorSetter(i, 152, 153, 154, 155, 156, 157, 158, 159, 139, 140, 141, 142, 143,  31,  31,  31); break;
			case SKINCOLOR_SEPIA:				dest_colormap[starttranscolor + i] = R_KartColorSetter(i,   3,   5,   7,  32,   9,  34,  36,  37,  39,  42,  45,  59,  60,  61,  62,  63); break;
			case SKINCOLOR_BEIGE:				dest_colormap[starttranscolor + i] = R_KartColorSetter(i,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47); break;
			case SKINCOLOR_BROWN:				dest_colormap[starttranscolor + i] = R_KartColorSetter(i,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63); break;
			case SKINCOLOR_LEATHER:				dest_colormap[starttranscolor + i] = R_KartColorSetter(i,  57,  58,  59,  59,  60,  60,  61,  61,  62,  62,  63,  63,  28,  29,  30,  31); break;
			case SKINCOLOR_YELLOW:				dest_colormap[starttranscolor + i] = R_KartColorSetter(i,  97,  98,  99, 100, 101, 102, 103, 104, 113, 113, 114, 115, 115, 115, 116, 117); break;
			case SKINCOLOR_PEACH:				dest_colormap[starttranscolor + i] = R_KartColorSetter(i,  64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79); break;
			case SKINCOLOR_LIGHTORANGE:			dest_colormap[starttranscolor + i] = R_KartColorSetter(i,  80,  80,  81,  81,  82,  82,  83,  83,  84,  84,  85,  85,  86,  86,  87,  87); break;
			case SKINCOLOR_PEACHBROWN:			dest_colormap[starttranscolor + i] = R_KartColorSetter(i,  72,  73,  74,  75,  76,  77,  78,  79,  48,  49,  50,  51,  52,  53,  54,  55); break;
			case SKINCOLOR_GOLD:				dest_colormap[starttranscolor + i] = R_KartColorSetter(i, 112, 112, 113, 113, 114, 114, 115, 115, 116, 116, 117, 117, 118, 118, 119, 119); break;
			case SKINCOLOR_FULLRANGEPEACHBROWN:	dest_colormap[starttranscolor + i] = R_KartColorSetter(i,  64,  66,  68,  70,  72,  74,  76,  78,  48,  50,  52,  54,  56,  58,  60,  62); break;
			case SKINCOLOR_GYPSYVOMIT:			dest_colormap[starttranscolor + i] = R_KartColorSetter(i, 121, 144, 145,  72,  73,  84, 114, 115, 107, 108, 109, 183, 223, 207,  30, 246); break;
			case SKINCOLOR_GARDEN:				dest_colormap[starttranscolor + i] = R_KartColorSetter(i,  98,  99, 112, 101, 113, 114, 106, 179, 180, 181, 182, 172, 183, 173, 174, 175); break;
			case SKINCOLOR_LIGHTARMY:			dest_colormap[starttranscolor + i] = R_KartColorSetter(i, 176, 176, 176, 176, 177, 177, 177, 177, 178, 178, 178, 178, 179, 179, 179, 179); break;
			case SKINCOLOR_ARMY:				dest_colormap[starttranscolor + i] = R_KartColorSetter(i, 176, 176, 177, 177, 178, 178, 179, 179, 180, 180, 181, 181, 182, 182, 183, 183); break;
			case SKINCOLOR_PISTACHIO:			dest_colormap[starttranscolor + i] = R_KartColorSetter(i, 176, 176, 177, 177, 178, 178, 179, 179, 166, 167, 168, 169, 170, 171, 172, 173); break;
			case SKINCOLOR_ROBOHOODGREEN:		dest_colormap[starttranscolor + i] = R_KartColorSetter(i, 177, 177, 178, 178, 165, 165, 167, 167, 182, 182, 171, 171, 183, 183, 173, 173); break;
			case SKINCOLOR_OLIVE:				dest_colormap[starttranscolor + i] = R_KartColorSetter(i, 105, 105, 106, 106, 107, 107, 108, 108, 109, 109, 110, 110, 111, 111,  31,  31); break;
			case SKINCOLOR_DARKARMY:			dest_colormap[starttranscolor + i] = R_KartColorSetter(i, 176, 177, 178, 179, 170, 181, 182, 183, 173, 173, 174, 174, 175, 175,  31,  31); break;
			case SKINCOLOR_LIGHTGREEN:			dest_colormap[starttranscolor + i] = R_KartColorSetter(i, 160, 160, 161, 161, 162, 162, 163, 163, 164, 164, 165, 165, 166, 166, 167, 167); break;
			case SKINCOLOR_UGLYGREEN:			dest_colormap[starttranscolor + i] = R_KartColorSetter(i, 184, 184, 184, 184, 185, 185, 185, 185, 186, 186, 186, 186, 187, 187, 187, 187); break;
			case SKINCOLOR_NEONGREEN:			dest_colormap[starttranscolor + i] = R_KartColorSetter(i, 184, 184, 185, 185, 186, 186, 187, 187, 188, 188, 189, 189, 190, 190, 191, 191); break;
			case SKINCOLOR_GREEN:				dest_colormap[starttranscolor + i] = R_KartColorSetter(i, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175); break;
			case SKINCOLOR_DARKGREEN:			dest_colormap[starttranscolor + i] = R_KartColorSetter(i, 168, 168, 169, 169, 170, 170, 171, 171, 172, 172, 173, 173, 174, 174, 175, 175); break;
			case SKINCOLOR_DARKNEONGREEN:		dest_colormap[starttranscolor + i] = R_KartColorSetter(i, 187, 187, 188, 188, 189, 189, 190, 190, 191, 191, 175, 175,  30,  30,  31,  31); break;
			case SKINCOLOR_FROST:				dest_colormap[starttranscolor + i] = R_KartColorSetter(i, 224, 225, 226, 212, 213, 213, 214, 215, 220, 221, 172, 222, 173, 223, 174, 175); break;
			case SKINCOLOR_LIGHTSTEELBLUE:		dest_colormap[starttranscolor + i] = R_KartColorSetter(i, 200, 200, 200, 200, 201, 201, 201, 201, 202, 202, 202, 202, 203, 203, 203, 203); break;
			case SKINCOLOR_LIGHTBLUE:			dest_colormap[starttranscolor + i] = R_KartColorSetter(i, 224, 224, 225, 225, 226, 226, 227, 227, 228, 228, 229, 229, 230, 230, 231, 231); break;
			case SKINCOLOR_CYAN:				dest_colormap[starttranscolor + i] = R_KartColorSetter(i, 208, 208, 209, 210, 210, 211, 212, 213, 213, 214, 215, 216, 216, 217, 218, 219); break;
			case SKINCOLOR_CERULEAN:			dest_colormap[starttranscolor + i] = R_KartColorSetter(i, 216, 216, 216, 216, 217, 217, 217, 217, 218, 218, 218, 218, 219, 219, 219, 219); break;
			case SKINCOLOR_TURQUOISE:			dest_colormap[starttranscolor + i] = R_KartColorSetter(i, 208, 208, 209, 210, 210, 211, 212, 213, 213, 214, 215, 220, 220, 221, 222, 223); break;
			case SKINCOLOR_TEAL:				dest_colormap[starttranscolor + i] = R_KartColorSetter(i, 220, 220, 220, 220, 221, 221, 221, 221, 222, 222, 222, 222, 223, 223, 223, 223); break;
			case SKINCOLOR_STEELBLUE:			dest_colormap[starttranscolor + i] = R_KartColorSetter(i, 200, 200, 201, 201, 202, 202, 203, 203, 204, 204, 205, 205, 206, 206, 207, 207); break;
			case SKINCOLOR_BLUE:				dest_colormap[starttranscolor + i] = R_KartColorSetter(i, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239); break;
			case SKINCOLOR_FULLRANGEBLUE:		dest_colormap[starttranscolor + i] = R_KartColorSetter(i, 224, 225, 226, 228, 229, 231, 232, 234, 235, 237, 238, 240, 241, 243, 244, 246); break;
			case SKINCOLOR_DARKSTEELBLUE:		dest_colormap[starttranscolor + i] = R_KartColorSetter(i, 200, 201, 202, 203, 204, 205, 206, 238, 239, 240, 241, 242, 243, 244, 245, 246); break;
			case SKINCOLOR_DARKBLUE:			dest_colormap[starttranscolor + i] = R_KartColorSetter(i, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246); break;
			case SKINCOLOR_JETBLACK:			dest_colormap[starttranscolor + i] = R_KartColorSetter(i, 200, 201, 202, 203, 204, 205, 206, 207,  28,  28,  29,  29,  30,  30,  31,  31); break;
			case SKINCOLOR_LILAC:				dest_colormap[starttranscolor + i] = R_KartColorSetter(i, 120, 120, 121, 121, 122, 122, 123, 123, 192, 192, 248, 248, 249, 249, 250, 250); break;
			case SKINCOLOR_PURPLE:				dest_colormap[starttranscolor + i] = R_KartColorSetter(i, 192, 192, 193, 193, 194, 194, 195, 195, 196, 196, 197, 197, 198, 198, 199, 199); break;
			case SKINCOLOR_LAVENDER:			dest_colormap[starttranscolor + i] = R_KartColorSetter(i, 248, 248, 249, 249, 250, 250, 251, 251, 252, 252, 253, 253, 254, 254, 255, 255); break;
			case SKINCOLOR_BYZANTIUM:			dest_colormap[starttranscolor + i] = R_KartColorSetter(i, 192, 248, 249, 250, 251, 252, 253, 254, 255, 255,  29,  29,  30,  30,  31,  31); break;
			case SKINCOLOR_INDIGO:				dest_colormap[starttranscolor + i] = R_KartColorSetter(i, 192, 193, 194, 195, 196, 197, 198, 199, 255, 255,  29,  29,  30,  30,  31,  31); break;
			/*
			 * Removed Colors:
			 * case SKINCOLOR_DUSK: 			dest_colormap[starttranscolor + i] = R_KartColorSetter(i, 192, 192, 248, 249, 250, 251, 229, 204, 230, 205, 206, 239, 240, 241, 242, 243); break;
			 * case SKINCOLOR_RAINBOW:			dest_colormap[starttranscolor + i] = R_KartColorSetter(i,   1, 145, 125,  73,  83, 114, 106, 180, 187, 168, 219, 205, 236, 206, 199, 255); break;
			 */
			default:
				I_Error("Invalid skin color #%hu.", (UINT16)color);
				return;
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
	CV_RegisterVar(&cv_magnet);
	CV_RegisterVar(&cv_boo);
	CV_RegisterVar(&cv_mushroom);
	CV_RegisterVar(&cv_triplemushroom);
	CV_RegisterVar(&cv_megashroom);
	CV_RegisterVar(&cv_goldshroom);
	CV_RegisterVar(&cv_star);
	CV_RegisterVar(&cv_triplebanana);
	CV_RegisterVar(&cv_fakeitem);
	CV_RegisterVar(&cv_banana);
	CV_RegisterVar(&cv_greenshell);
	CV_RegisterVar(&cv_redshell);
	CV_RegisterVar(&cv_laserwisp);
	CV_RegisterVar(&cv_triplegreenshell);
	CV_RegisterVar(&cv_bobomb);
	CV_RegisterVar(&cv_blueshell);
	CV_RegisterVar(&cv_jaws);
	CV_RegisterVar(&cv_fireflower);
	CV_RegisterVar(&cv_tripleredshell);
	CV_RegisterVar(&cv_lightning);

	CV_RegisterVar(&cv_kartcc);
	CV_RegisterVar(&cv_speedometer);
	CV_RegisterVar(&cv_collideminimum);
	CV_RegisterVar(&cv_collidesoundnum);
	CV_RegisterVar(&cv_collidesounds);
}

//}

//{ SRB2kart Roulette Code

#define NUMKARTITEMS 	18
#define NUMKARTODDS 	40

// Ugly ol' 3D array
static INT32 K_KartItemOdds_Retro[MAXPLAYERS][NUMKARTITEMS][MAXPLAYERS] =
{
	// 1 Active Player
	{  //1st //
		{  0 }, // Magnet
		{  0 }, // Boo
		{ 40 }, // Mushroom
		{  0 }, // Triple Mushroom
		{  0 }, // Mega Mushroom
		{  0 }, // Gold Mushroom
		{  0 }, // Star
		{  0 }, // Triple Banana
		{  0 }, // Fake Item
		{  0 }, // Banana
		{  0 }, // Green Shell
		{  0 }, // Red Shell
		{  0 }, // Triple Green Shell
		{  0 }, // Bob-omb
		{  0 }, // Blue Shell
		{  0 }, // Fire Flower
		{  0 }, // Triple Red Shell
		{  0 }  // Lightning
	}, //1st //

	// 2 Active Players
	{  //1st 2nd //
		{  1,  0 }, // Magnet
		{  0,  0 }, // Boo
		{  6, 11 }, // Mushroom
		{  0,  3 }, // Triple Mushroom
		{  0,  1 }, // Mega Mushroom
		{  0,  1 }, // Gold Mushroom
		{  0,  0 }, // Star
		{  4,  0 }, // Triple Banana
		{  7,  0 }, // Fake Item
		{ 12,  0 }, // Banana
		{  8,  6 }, // Green Shell
		{  2, 12 }, // Red Shell
		{  0,  4 }, // Triple Green Shell
		{  0,  0 }, // Bob-omb
		{  0,  2 }, // Blue Shell
		{  0,  0 }, // Fire Flower
		{  0,  0 }, // Triple Red Shell
		{  0,  0 }  // Lightning
	}, //1st 2nd //

	// 3 Active Players
	{  //1st 2nd 3rd //
		{  1,  0,  0 }, // Magnet
		{  0,  1,  0 }, // Boo
		{  4, 10,  0 }, // Mushroom
		{  0,  4, 11 }, // Triple Mushroom
		{  0,  0,  2 }, // Mega Mushroom
		{  0,  0, 16 }, // Gold Mushroom
		{  0,  0,  6 }, // Star
		{  4,  0,  0 }, // Triple Banana
		{  7,  2,  0 }, // Fake Item
		{ 13,  4,  0 }, // Banana
		{  9,  4,  0 }, // Green Shell
		{  2,  8,  0 }, // Red Shell
		{  0,  3,  0 }, // Triple Green Shell
		{  0,  2,  0 }, // Bob-omb
		{  0,  2,  0 }, // Blue Shell
		{  0,  0,  0 }, // Fire Flower
		{  0,  0,  0 }, // Triple Red Shell
		{  0,  0,  5 }  // Lightning
	}, //1st 2nd 3rd //

	// 4 Active Players
	{  //1st 2nd 3rd 4th //
		{  1,  1,  0,  0 }, // Magnet
		{  0,  3,  0,  0 }, // Boo
		{  2, 10,  0,  0 }, // Mushroom
		{  0,  3,  9, 10 }, // Triple Mushroom
		{  0,  0,  3,  0 }, // Mega Mushroom
		{  0,  0,  9, 16 }, // Gold Mushroom
		{  0,  0,  0,  8 }, // Star
		{  4,  0,  0,  0 }, // Triple Banana
		{  7,  2,  0,  0 }, // Fake Item
		{ 14,  4,  0,  0 }, // Banana
		{  9,  5,  0,  0 }, // Green Shell
		{  3,  8,  6,  0 }, // Red Shell
		{  0,  2,  4,  0 }, // Triple Green Shell
		{  0,  2,  0,  0 }, // Bob-omb
		{  0,  0,  3,  0 }, // Blue Shell
		{  0,  0,  3,  0 }, // Fire Flower
		{  0,  0,  3,  0 }, // Triple Red Shell
		{  0,  0,  0,  6 }  // Lightning
	}, //1st 2nd 3rd 4th //

	// 5 Active Players
	{  //1st 2nd 3rd 4th 5th //
		{  1,  2,  0,  0,  0 }, // Magnet
		{  0,  3,  0,  0,  0 }, // Boo
		{  0,  9,  7,  0,  0 }, // Mushroom
		{  0,  0,  8, 12, 12 }, // Triple Mushroom
		{  0,  0,  2,  2,  0 }, // Mega Mushroom
		{  0,  0,  0,  9, 20 }, // Gold Mushroom
		{  0,  0,  0,  3,  8 }, // Star
		{  4,  2,  0,  0,  0 }, // Triple Banana
		{  7,  3,  0,  0,  0 }, // Fake Item
		{ 15,  4,  0,  0,  0 }, // Banana
		{ 10,  6,  3,  0,  0 }, // Green Shell
		{  3,  9,  6,  0,  0 }, // Red Shell
		{  0,  2,  4,  0,  0 }, // Triple Green Shell
		{  0,  0,  3,  0,  0 }, // Bob-omb
		{  0,  0,  4,  0,  0 }, // Blue Shell
		{  0,  0,  3,  4,  0 }, // Fire Flower
		{  0,  0,  0,  3,  0 }, // Triple Red Shell
		{  0,  0,  0,  7,  0 }  // Lightning
	}, //1st 2nd 3rd 4th 5th //

	// 6 Active Players
	{  //1st 2nd 3rd 4th 5th 6th //
		{  1,  2,  0,  0,  0,  0 }, // Magnet
		{  0,  2,  2,  0,  0,  0 }, // Boo
		{  0,  9,  8,  0,  0,  0 }, // Mushroom
		{  0,  0,  5, 11, 14, 12 }, // Triple Mushroom
		{  0,  0,  0,  2,  2,  0 }, // Mega Mushroom
		{  0,  0,  0,  6, 11, 20 }, // Gold Mushroom
		{  0,  0,  0,  0,  3,  8 }, // Star
		{  4,  2,  0,  0,  0,  0 }, // Triple Banana
		{  7,  3,  0,  0,  0,  0 }, // Fake Item
		{ 15,  5,  3,  0,  0,  0 }, // Banana
		{ 10,  6,  4,  0,  0,  0 }, // Green Shell
		{  3,  9,  8,  6,  0,  0 }, // Red Shell
		{  0,  2,  4,  5,  0,  0 }, // Triple Green Shell
		{  0,  0,  3,  0,  0,  0 }, // Bob-omb
		{  0,  0,  3,  3,  0,  0 }, // Blue Shell
		{  0,  0,  0,  4,  0,  0 }, // Fire Flower
		{  0,  0,  0,  3,  3,  0 }, // Triple Red Shell
		{  0,  0,  0,  0,  7,  0 }  // Lightning
	}, //1st 2nd 3rd 4th 5th 6th //

	// 7 Active Players
	{  //1st 2nd 3rd 4th 5th 6th 7th //
		{  1,  2,  0,  0,  0,  0,  0 }, // Magnet
		{  0,  2,  3,  0,  0,  0,  0 }, // Boo
		{  0,  8,  8,  8,  0,  0,  0 }, // Mushroom
		{  0,  0,  3,  8, 14, 15, 12 }, // Triple Mushroom
		{  0,  0,  0,  2,  4,  2,  0 }, // Mega Mushroom
		{  0,  0,  0,  0,  7, 12, 20 }, // Gold Mushroom
		{  0,  0,  0,  0,  0,  3,  8 }, // Star
		{  4,  2,  0,  0,  0,  0,  0 }, // Triple Banana
		{  7,  4,  2,  0,  0,  0,  0 }, // Fake Item
		{ 15,  5,  4,  0,  0,  0,  0 }, // Banana
		{ 10,  6,  4,  0,  0,  0,  0 }, // Green Shell
		{  3, 10,  8,  7,  0,  0,  0 }, // Red Shell
		{  0,  1,  4,  6,  4,  0,  0 }, // Triple Green Shell
		{  0,  0,  3,  0,  0,  0,  0 }, // Bob-omb
		{  0,  0,  1,  3,  3,  0,  0 }, // Blue Shell
		{  0,  0,  0,  3,  5,  0,  0 }, // Fire Flower
		{  0,  0,  0,  3,  3,  0,  0 }, // Triple Red Shell
		{  0,  0,  0,  0,  0,  8,  0 }  // Lightning
	}, //1st 2nd 3rd 4th 5th 6th 7th //

	// 8 Active Players
	{  //1st 2nd 3rd 4th 5th 6th 7th 8th //
		{  1,  2,  0,  0,  0,  0,  0,  0 }, // Magnet
		{  0,  2,  3,  0,  0,  0,  0,  0 }, // Boo
		{  0,  8,  8,  8,  0,  0,  0,  0 }, // Mushroom
		{  0,  0,  3,  7, 12, 15, 11,  7 }, // Triple Mushroom
		{  0,  0,  0,  2,  4,  4,  2,  0 }, // Mega Mushroom
		{  0,  0,  0,  0,  4,  8, 15, 24 }, // Gold Mushroom
		{  0,  0,  0,  0,  0,  2,  5,  9 }, // Star
		{  4,  2,  0,  0,  0,  0,  0,  0 }, // Triple Banana
		{  7,  4,  2,  0,  0,  0,  0,  0 }, // Fake Item
		{ 15,  5,  4,  0,  0,  0,  0,  0 }, // Banana
		{ 10,  7,  5,  2,  0,  0,  0,  0 }, // Green Shell
		{  3,  9,  8,  7,  5,  0,  0,  0 }, // Red Shell
		{  0,  1,  4,  6,  4,  0,  0,  0 }, // Triple Green Shell
		{  0,  0,  2,  3,  0,  0,  0,  0 }, // Bob-omb
		{  0,  0,  1,  3,  3,  0,  0,  0 }, // Blue Shell
		{  0,  0,  0,  2,  5,  3,  0,  0 }, // Fire Flower
		{  0,  0,  0,  0,  3,  3,  0,  0 }, // Triple Red Shell
		{  0,  0,  0,  0,  0,  5,  7,  0 }  // Lightning
	}, //1st 2nd 3rd 4th 5th 6th 7th 8th //

	// 9 Active Players
	{  //1st 2nd 3rd 4th 5th 6th 7th 8th 9th //
		{  1,  3,  2,  0,  0,  0,  0,  0,  0 }, // Magnet
		{  0,  2,  2,  3,  0,  0,  0,  0,  0 }, // Boo
		{  0,  7,  9, 10,  8,  0,  0,  0,  0 }, // Mushroom
		{  0,  0,  3,  7, 10, 17, 16, 12,  0 }, // Triple Mushroom
		{  0,  0,  0,  0,  2,  4,  6,  0,  0 }, // Mega Mushroom
		{  0,  0,  0,  0,  0,  4,  8, 15, 29 }, // Gold Mushroom
		{  0,  0,  0,  0,  0,  0,  3,  6, 11 }, // Star
		{  4,  2,  0,  0,  0,  0,  0,  0,  0 }, // Triple Banana
		{  7,  4,  2,  0,  0,  0,  0,  0,  0 }, // Fake Item
		{ 15,  6,  5,  2,  0,  0,  0,  0,  0 }, // Banana
		{ 10,  7,  5,  2,  0,  0,  0,  0,  0 }, // Green Shell
		{  3,  9,  8,  7,  6,  0,  0,  0,  0 }, // Red Shell
		{  0,  0,  2,  4,  6,  4,  0,  0,  0 }, // Triple Green Shell
		{  0,  0,  2,  3,  0,  0,  0,  0,  0 }, // Bob-omb
		{  0,  0,  0,  2,  3,  3,  0,  0,  0 }, // Blue Shell
		{  0,  0,  0,  0,  3,  5,  0,  0,  0 }, // Fire Flower
		{  0,  0,  0,  0,  2,  3,  2,  0,  0 }, // Triple Red Shell
		{  0,  0,  0,  0,  0,  0,  5,  7,  0 }  // Lightning
	}, //1st 2nd 3rd 4th 5th 6th 7th 8th 9th //

	// 10 Active Players
	{  //1st 2nd 3rd 4th 5th 6th 7th 8th 9th 10t //
		{  1,  3,  2,  0,  0,  0,  0,  0,  0,  0 }, // Magnet
		{  0,  2,  3,  3,  0,  0,  0,  0,  0,  0 }, // Boo
		{  0,  7,  8, 10,  8,  0,  0,  0,  0,  0 }, // Mushroom
		{  0,  0,  2,  5,  9, 14, 16, 14,  9,  0 }, // Triple Mushroom
		{  0,  0,  0,  0,  2,  4,  6,  5,  0,  0 }, // Mega Mushroom
		{  0,  0,  0,  0,  0,  4,  5, 10, 16, 28 }, // Gold Mushroom
		{  0,  0,  0,  0,  0,  0,  2,  4,  7, 12 }, // Star
		{  4,  2,  0,  0,  0,  0,  0,  0,  0,  0 }, // Triple Banana
		{  7,  4,  2,  0,  0,  0,  0,  0,  0,  0 }, // Fake Item
		{ 15,  6,  5,  2,  0,  0,  0,  0,  0,  0 }, // Banana
		{ 10,  7,  6,  4,  2,  0,  0,  0,  0,  0 }, // Green Shell
		{  3,  9,  8,  7,  6,  3,  0,  0,  0,  0 }, // Red Shell
		{  0,  0,  2,  4,  6,  4,  0,  0,  0,  0 }, // Triple Green Shell
		{  0,  0,  2,  3,  1,  0,  0,  0,  0,  0 }, // Bob-omb
		{  0,  0,  0,  2,  3,  3,  0,  0,  0,  0 }, // Blue Shell
		{  0,  0,  0,  0,  3,  6,  4,  0,  0,  0 }, // Fire Flower
		{  0,  0,  0,  0,  0,  2,  3,  1,  0,  0 }, // Triple Red Shell
		{  0,  0,  0,  0,  0,  0,  4,  6,  8,  0 }  // Lightning
	}, //1st 2nd 3rd 4th 5th 6th 7th 8th 9th 10t //

	// 11 Active Players
	{  //1st 2nd 3rd 4th 5th 6th 7th 8th 9th 10t 11t //
		{  1,  3,  2,  0,  0,  0,  0,  0,  0,  0,  0 }, // Magnet
		{  0,  2,  3,  3,  0,  0,  0,  0,  0,  0,  0 }, // Boo
		{  0,  6,  8,  9,  8,  6,  0,  0,  0,  0,  0 }, // Mushroom
		{  0,  0,  2,  5,  9, 10, 15, 17, 14, 10,  0 }, // Triple Mushroom
		{  0,  0,  0,  0,  2,  3,  6,  6,  4,  0,  0 }, // Mega Mushroom
		{  0,  0,  0,  0,  0,  0,  4,  5, 11, 15, 18 }, // Gold Mushroom
		{  0,  0,  0,  0,  0,  0,  0,  3,  5,  7, 12 }, // Star
		{  4,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, // Triple Banana
		{  7,  3,  2,  1,  0,  0,  0,  0,  0,  0,  0 }, // Fake Item
		{ 15,  7,  5,  2,  0,  0,  0,  0,  0,  0,  0 }, // Banana
		{ 10,  8,  6,  4,  2,  0,  0,  0,  0,  0,  0 }, // Green Shell
		{  3,  9,  8,  7,  6,  4,  0,  0,  0,  0,  0 }, // Red Shell
		{  0,  0,  2,  4,  6,  6,  5,  0,  0,  0,  0 }, // Triple Green Shell
		{  0,  0,  2,  3,  1,  0,  0,  0,  0,  0,  0 }, // Bob-omb
		{  0,  0,  0,  2,  3,  3,  2,  0,  0,  0,  0 }, // Blue Shell
		{  0,  0,  0,  0,  3,  6,  5,  3,  0,  0,  0 }, // Fire Flower
		{  0,  0,  0,  0,  0,  2,  3,  2,  0,  0,  0 }, // Triple Red Shell
		{  0,  0,  0,  0,  0,  0,  0,  4,  6,  8,  0 }  // Lightning
	}, //1st 2nd 3rd 4th 5th 6th 7th 8th 9th 10t 11t //

	// 12 Active Players
	{  //1st 2nd 3rd 4th 5th 6th 7th 8th 9th 10t 11t 12t //
		{  1,  3,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, // Magnet
		{  0,  2,  3,  3,  2,  0,  0,  0,  0,  0,  0,  0 }, // Boo
		{  0,  6,  8,  9, 10,  8,  0,  0,  0,  0,  0,  0 }, // Mushroom
		{  0,  0,  1,  4,  9, 11, 15, 17, 15, 11,  6,  0 }, // Triple Mushroom
		{  0,  0,  0,  0,  0,  2,  4,  6,  7,  5,  0,  0 }, // Mega Mushroom
		{  0,  0,  0,  0,  0,  0,  3,  5,  9, 13, 18, 28 }, // Gold Mushroom
		{  0,  0,  0,  0,  0,  0,  0,  0,  3,  5,  8, 12 }, // Star
		{  4,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, // Triple Banana
		{  7,  4,  2,  1,  0,  0,  0,  0,  0,  0,  0,  0 }, // Fake Item
		{ 15,  7,  6,  3,  2,  0,  0,  0,  0,  0,  0,  0 }, // Banana
		{ 10,  7,  6,  4,  2,  0,  0,  0,  0,  0,  0,  0 }, // Green Shell
		{  3,  9,  8,  7,  6,  5,  2,  0,  0,  0,  0,  0 }, // Red Shell
		{  0,  0,  2,  4,  5,  7,  5,  3,  0,  0,  0,  0 }, // Triple Green Shell
		{  0,  0,  2,  3,  1,  0,  0,  0,  0,  0,  0,  0 }, // Bob-omb
		{  0,  0,  0,  2,  3,  3,  3,  2,  0,  0,  0,  0 }, // Blue Shell
		{  0,  0,  0,  0,  0,  2,  5,  4,  0,  0,  0,  0 }, // Fire Flower
		{  0,  0,  0,  0,  0,  2,  3,  3,  2,  0,  0,  0 }, // Triple Red Shell
		{  0,  0,  0,  0,  0,  0,  0,  0,  4,  6,  8,  0 }  // Lightning
	}, //1st 2nd 3rd 4th 5th 6th 7th 8th 9th 10t 11t 12t //

	// 13 Active Players
	{  //1st 2nd 3rd 4th 5th 6th 7th 8th 9th 10t 11t 12t 13t //
		{  1,  3,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, // Magnet
		{  0,  2,  4,  4,  2,  0,  0,  0,  0,  0,  0,  0,  0 }, // Boo
		{  0,  5,  8,  9, 10,  8,  5,  0,  0,  0,  0,  0,  0 }, // Mushroom
		{  0,  0,  0,  2,  6,  9, 11, 15, 16, 14, 11, 10,  0 }, // Triple Mushroom
		{  0,  0,  0,  0,  0,  2,  5,  6,  8,  6,  3,  0,  0 }, // Mega Mushroom
		{  0,  0,  0,  0,  0,  0,  0,  4,  5,  8, 11, 20, 27 }, // Gold Mushroom
		{  0,  0,  0,  0,  0,  0,  0,  0,  2,  3,  5, 10, 13 }, // Star
		{  4,  2,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, // Triple Banana
		{  7,  4,  3,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, // Fake Item
		{ 15,  7,  6,  4,  2,  0,  0,  0,  0,  0,  0,  0,  0 }, // Banana
		{ 10,  8,  6,  4,  3,  2,  0,  0,  0,  0,  0,  0,  0 }, // Green Shell
		{  3,  9,  8,  7,  6,  5,  2,  0,  0,  0,  0,  0,  0 }, // Red Shell
		{  0,  0,  2,  4,  5,  7,  6,  5,  0,  0,  0,  0,  0 }, // Triple Green Shell
		{  0,  0,  0,  2,  3,  1,  0,  0,  0,  0,  0,  0,  0 }, // Bob-omb
		{  0,  0,  0,  2,  3,  3,  3,  2,  0,  0,  0,  0,  0 }, // Blue Shell
		{  0,  0,  0,  0,  0,  3,  6,  5,  3,  0,  0,  0,  0 }, // Fire Flower
		{  0,  0,  0,  0,  0,  0,  2,  3,  3,  2,  0,  0,  0 }, // Triple Red Shell
		{  0,  0,  0,  0,  0,  0,  0,  0,  3,  7, 10,  0,  0 }  // Lightning
	}, //1st 2nd 3rd 4th 5th 6th 7th 8th 9th 10t 11t 12t 13t //

	// 14 Active Players
	{  //1st 2nd 3rd 4th 5th 6th 7th 8th 9th 10t 11t 12t 13t 14t //
		{  1,  2,  3,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, // Magnet
		{  0,  2,  4,  4,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, // Boo
		{  0,  5,  7,  9, 11,  8,  5,  0,  0,  0,  0,  0,  0,  0 }, // Mushroom
		{  0,  0,  0,  2,  6,  9, 10, 12, 17, 15, 14, 12,  8,  0 }, // Triple Mushroom
		{  0,  0,  0,  0,  0,  2,  5,  6,  8,  7,  3,  0,  0,  0 }, // Mega Mushroom
		{  0,  0,  0,  0,  0,  0,  0,  3,  5,  8, 11, 13, 22, 27 }, // Gold Mushroom
		{  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,  3,  5, 10, 13 }, // Star
		{  4,  2,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, // Triple Banana
		{  7,  4,  3,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, // Fake Item
		{ 15,  7,  6,  4,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, // Banana
		{ 10,  8,  6,  4,  3,  2,  0,  0,  0,  0,  0,  0,  0,  0 }, // Green Shell
		{  3, 10,  8,  7,  6,  5,  3,  2,  0,  0,  0,  0,  0,  0 }, // Red Shell
		{  0,  0,  2,  4,  5,  7,  6,  5,  2,  0,  0,  0,  0,  0 }, // Triple Green Shell
		{  0,  0,  0,  2,  3,  1,  0,  0,  0,  0,  0,  0,  0,  0 }, // Bob-omb
		{  0,  0,  0,  0,  2,  3,  4,  3,  1,  0,  0,  0,  0,  0 }, // Blue Shell
		{  0,  0,  0,  0,  0,  3,  5,  6,  4,  2,  0,  0,  0,  0 }, // Fire Flower
		{  0,  0,  0,  0,  0,  0,  2,  3,  3,  3,  2,  0,  0,  0 }, // Triple Red Shell
		{  0,  0,  0,  0,  0,  0,  0,  0,  0,  3,  7, 10,  0,  0 }  // Lightning
	}, //1st 2nd 3rd 4th 5th 6th 7th 8th 9th 10t 11t 12t 13t 14t //

	// 15 Active Players
	{  //1st 2nd 3rd 4th 5th 6th 7th 8th 9th 10t 11t 12t 13t 14t 15t //
		{  1,  2,  3,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, // Magnet
		{  0,  2,  4,  4,  3,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, // Boo
		{  0,  4,  6,  8, 10,  9,  8,  4,  0,  0,  0,  0,  0,  0,  0 }, // Mushroom
		{  0,  0,  0,  2,  4,  8, 11, 13, 17, 18, 15, 13,  9,  8,  0 }, // Triple Mushroom
		{  0,  0,  0,  0,  0,  0,  2,  5,  6,  8,  7,  3,  0,  0,  0 }, // Mega Mushroom
		{  0,  0,  0,  0,  0,  0,  0,  0,  3,  5,  8, 11, 13, 22, 27 }, // Gold Mushroom
		{  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,  3,  5,  7, 10, 13 }, // Star
		{  4,  2,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, // Triple Banana
		{  7,  4,  3,  2,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, // Fake Item
		{ 15,  8,  6,  4,  2,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, // Banana
		{ 10,  8,  6,  4,  3,  2,  1,  0,  0,  0,  0,  0,  0,  0,  0 }, // Green Shell
		{  3, 10,  9,  8,  7,  6,  4,  3,  0,  0,  0,  0,  0,  0,  0 }, // Red Shell
		{  0,  0,  2,  4,  5,  7,  6,  5,  3,  0,  0,  0,  0,  0,  0 }, // Triple Green Shell
		{  0,  0,  0,  2,  3,  2,  1,  0,  0,  0,  0,  0,  0,  0,  0 }, // Bob-omb
		{  0,  0,  0,  0,  2,  3,  4,  3,  2,  0,  0,  0,  0,  0,  0 }, // Blue Shell
		{  0,  0,  0,  0,  0,  0,  3,  5,  6,  2,  0,  0,  0,  0,  0 }, // Fire Flower
		{  0,  0,  0,  0,  0,  0,  0,  2,  3,  3,  2,  0,  0,  0,  0 }, // Triple Red Shell
		{  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,  5,  8, 11,  0,  0 }  // Lightning
	}, //1st 2nd 3rd 4th 5th 6th 7th 8th 9th 10t 11t 12t 13t 14t 15t //

	// 16 Active Players
	{  //1st 2nd 3rd 4th 5th 6th 7th 8th 9th 10t 11t 12t 13t 14t 15t 16t //
		{  1,  2,  3,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, // Magnet
		{  0,  2,  4,  4,  3,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, // Boo
		{  0,  4,  6,  8, 10,  9,  7,  4,  0,  0,  0,  0,  0,  0,  0,  0 }, // Mushroom
		{  0,  0,  0,  2,  4,  7, 10, 13, 16, 18, 15, 14, 12,  8,  7,  0 }, // Triple Mushroom
		{  0,  0,  0,  0,  0,  0,  2,  5,  6,  8,  8,  5,  2,  0,  0,  0 }, // Mega Mushroom
		{  0,  0,  0,  0,  0,  0,  0,  0,  2,  4,  8, 11, 13, 14, 23, 27 }, // Gold Mushroom
		{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,  3,  5,  7, 10, 13 }, // Star
		{  4,  2,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, // Triple Banana
		{  7,  4,  3,  2,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, // Fake Item
		{ 15,  8,  6,  4,  2,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, // Banana
		{ 10,  8,  6,  4,  3,  2,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, // Green Shell
		{  3, 10,  9,  8,  7,  6,  5,  3,  2,  0,  0,  0,  0,  0,  0,  0 }, // Red Shell
		{  0,  0,  2,  4,  5,  8,  7,  5,  3,  2,  0,  0,  0,  0,  0,  0 }, // Triple Green Shell
		{  0,  0,  0,  2,  3,  2,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, // Bob-omb
		{  0,  0,  0,  0,  2,  3,  4,  3,  2,  1,  0,  0,  0,  0,  0,  0 }, // Blue Shell
		{  0,  0,  0,  0,  0,  0,  3,  5,  6,  4,  2,  0,  0,  0,  0,  0 }, // Fire Flower
		{  0,  0,  0,  0,  0,  0,  0,  2,  3,  3,  3,  2,  0,  0,  0,  0 }, // Triple Red Shell
		{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,  5,  8, 11,  0,  0 }  // Lightning
	}  //1st 2nd 3rd 4th 5th 6th 7th 8th 9th 10t 11t 12t 13t 14t 15t 16t //
};

/**	\brief	Item Roulette for Kart

	\param	player		player
	\param	getitem		what item we're looking for
	\param	retrokart	whether or not we're getting old or new item odds

	\return	void
*/
static void K_KartGetItemResult(player_t *player, fixed_t getitem, boolean retrokart)
{
	switch (getitem)
	{
		case  1:	// Magnet
			player->kartstuff[k_magnet] = 1;
			break;
		case  2:	// Boo
			player->kartstuff[k_boo] = 1;
			break;
		case  4:	// Triple Mushroom
			player->kartstuff[k_mushroom] = 4;
			break;
		case  5:	// Mega Mushroom
			player->kartstuff[k_megashroom] = 1;
			break;
		case  6:	// Gold Mushroom
			player->kartstuff[k_goldshroom] = 1;
			break;
		case  7:	// Star
			player->kartstuff[k_star] = 1;
			break;
		case  8:	// Triple Banana
			player->kartstuff[k_triplebanana] |= 8;
			break;
		case  9:	// Fake Item
			player->kartstuff[k_fakeitem] |= 2;
			break;
		case 10:	// Banana
			player->kartstuff[k_banana] |= 2;
			break;
		case 11:	// Green Shell
			player->kartstuff[k_greenshell] |= 2;
			break;
		case 12:	// Red Shell
			player->kartstuff[k_redshell] |= 2;
			break;
		case 13:	// Triple Green Shell	- or -	Laser Wisp
			if (retrokart)
				player->kartstuff[k_triplegreenshell] |= 8;
			else
				player->kartstuff[k_laserwisp] = 1;
			break;
		case 14:	// Bob-omb				- or -	3x Orbinaut (Triple Green Shell)
			if (retrokart)
				player->kartstuff[k_bobomb] |= 2;
			else
				player->kartstuff[k_triplegreenshell] |= 8;
			break;
		case 15:	// Blue Shell			- or -	Specialstage Mines (Bob-omb)
			if (retrokart)
				player->kartstuff[k_blueshell] = 1;
			else
				player->kartstuff[k_bobomb] |= 2;
			break;
		case 16:	// Fire Flower			- or -	Deton (Blue Shell)
			if (retrokart)
				player->kartstuff[k_fireflower] = 1;
			else
				player->kartstuff[k_blueshell] |= 4;
			break;
		case 17:	// Triple Red Shell		- or -	2x Jaws
			if (retrokart)
				player->kartstuff[k_tripleredshell] |= 8;
			else
				player->kartstuff[k_jaws] = 1;
			break;
		case 18:	// Lightning
			player->kartstuff[k_lightning] = 1;
			break;
		default:	// Mushroom - Doing it here as a fail-safe
			if (getitem != 3)
				CONS_Printf("ERROR: P_KartGetItemResult - Item roulette gave bad item (%d), giving Mushroom instead.\n", getitem);
			player->kartstuff[k_mushroom] = 1;
			break;
	}
}

/**	\brief	Item Roulette for Kart

	\param	player	player object passed from P_KartPlayerThink

	\return	void
*/
static void K_KartItemRoulette(player_t *player, ticcmd_t *cmd)
{
	INT32 i;
	INT32 pingame = 0, pexiting = 0;
	INT32 roulettestop;
	INT32 prandom;
	INT32 ppos = player->kartstuff[k_position] - 1;
	INT32 spawnchance[NUMKARTITEMS * NUMKARTODDS];
	INT32 chance = 0, numchoices = 0;


	// This makes the roulette cycle through items - if this is 0, you shouldn't be here.
	if (player->kartstuff[k_itemroulette])
		player->kartstuff[k_itemroulette]++;
	else
		return;

	// This makes the roulette produce the random noises.
	if ((player->kartstuff[k_itemroulette] % 3) == 1 && P_IsLocalPlayer(player))
		S_StartSound(NULL,sfx_mkitm1 + ((player->kartstuff[k_itemroulette] / 3) % 8));

	// Initializes existing spawnchance values
	for (i = 0; i < (NUMKARTITEMS * NUMKARTODDS); i++)
		spawnchance[i] = 0;

	// Gotta check how many players are active at this moment.
	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (playeringame[i] && !players[i].spectator)
			pingame++;
		if (players[i].exiting)
			pexiting++;
	}

	roulettestop = (TICRATE*1) + (3*(pingame - player->kartstuff[k_position]));

	// If the roulette finishes or the player presses BT_ATTACK, stop the roulette and calculate the item.
	// I'm returning via the exact opposite, however, to forgo having another bracket embed. Same result either way, I think.
	// Finally, if you get past this check, now you can actually start calculating what item you get.
	if (!(player->kartstuff[k_itemroulette] >= (TICRATE*3)
		|| ((cmd->buttons & BT_ATTACK) && player->kartstuff[k_itemroulette] >= roulettestop)))
		return;

	if (cmd->buttons & BT_ATTACK)
		player->pflags |= PF_ATTACKDOWN;

	player->kartstuff[k_itemclose] = 0;	// Reset the item window closer.

	// Tiny catcher in case player position is unset.
	if (ppos < 0) ppos = 0;

#define SETITEMRESULT(pos, numplayers, itemnum) \
	for (chance = 0; chance < K_KartItemOdds_Retro[numplayers-1][itemnum-1][pos]; chance++) spawnchance[numchoices++] = itemnum

	// Check the game type to differentiate odds.
	//if (gametype == GT_RETRO)
	//{
		if (cv_magnet.value) 							SETITEMRESULT(ppos, pingame, 1);	// Magnet
		if (cv_boo.value)								SETITEMRESULT(ppos, pingame, 2);	// Boo
		if (cv_mushroom.value)							SETITEMRESULT(ppos, pingame, 3);	// Mushroom
		if (cv_mushroom.value)							SETITEMRESULT(ppos, pingame, 4);	// Triple Mushroom
		if (cv_megashroom.value)						SETITEMRESULT(ppos, pingame, 5);	// Mega Mushroom
		if (cv_goldshroom.value)						SETITEMRESULT(ppos, pingame, 6);	// Gold Mushroom
		if (cv_star.value)								SETITEMRESULT(ppos, pingame, 7);	// Star
		if (cv_triplebanana.value)						SETITEMRESULT(ppos, pingame, 8);	// Triple Banana
		if (cv_fakeitem.value)							SETITEMRESULT(ppos, pingame, 9);	// Fake Item
		if (cv_banana.value)							SETITEMRESULT(ppos, pingame, 10);	// Banana
		if (cv_greenshell.value)						SETITEMRESULT(ppos, pingame, 11);	// Green Shell
		if (cv_redshell.value)							SETITEMRESULT(ppos, pingame, 12);	// Red Shell
		if (cv_triplegreenshell.value)					SETITEMRESULT(ppos, pingame, 13);	// Triple Green Shell
		if (cv_bobomb.value)							SETITEMRESULT(ppos, pingame, 14);	// Bob-omb
		if (cv_blueshell.value && pexiting == 0)		SETITEMRESULT(ppos, pingame, 15);	// Blue Shell
		//if (cv_fireflower.value)						SETITEMRESULT(ppos, pingame, 16);	// Fire Flower
		if (cv_tripleredshell.value)					SETITEMRESULT(ppos, pingame, 17);	// Triple Red Shell
		if (cv_lightning.value && pingame > pexiting)	SETITEMRESULT(ppos, pingame, 18);	// Lightning

		prandom = P_RandomKey(numchoices);

		// Award the player whatever power is rolled
		if (numchoices > 0)
			K_KartGetItemResult(player, spawnchance[prandom], true);
		else
			CONS_Printf("ERROR: P_KartItemRoulette - There were no choices given by the roulette (ppos = %d).\n", ppos);
	//}
	/*else if (gametype == GT_NEO)
	{
		if (cv_magnet.value) 							SETITEMRESULT(ppos, pingame, 1)	// Electro-Shield
		if (cv_boo.value)								SETITEMRESULT(ppos, pingame, 2)	// S3K Ghost
		if (cv_mushroom.value)							SETITEMRESULT(ppos, pingame, 3)	// Speed Shoe
		if (cv_mushroom.value)							SETITEMRESULT(ppos, pingame, 4)	// Triple Speed Shoe
		if (cv_megashroom.value)						SETITEMRESULT(ppos, pingame, 5)	// Size-Up Monitor
		if (cv_goldshroom.value)						SETITEMRESULT(ppos, pingame, 6)	// Rocket Shoe
		if (cv_star.value)								SETITEMRESULT(ppos, pingame, 7)	// Invincibility
		if (cv_triplebanana.value)						SETITEMRESULT(ppos, pingame, 8)	// Triple Banana
		if (cv_fakeitem.value)							SETITEMRESULT(ppos, pingame, 9)	// Eggman Monitor
		if (cv_banana.value)							SETITEMRESULT(ppos, pingame, 10)	// Banana
		if (cv_greenshell.value)						SETITEMRESULT(ppos, pingame, 11)	// 1x Orbinaut
		if (cv_redshell.value)							SETITEMRESULT(ppos, pingame, 12)	// 1x Jaws
		if (cv_laserwisp.value)							SETITEMRESULT(ppos, pingame, 13)	// Laser Wisp
		if (cv_triplegreenshell.value)					SETITEMRESULT(ppos, pingame, 14)	// 3x Orbinaut
		if (cv_bobomb.value)							SETITEMRESULT(ppos, pingame, 15)	// Specialstage Mines
		if (cv_blueshell.value && pexiting == 0)		SETITEMRESULT(ppos, pingame, 16)	// Deton
		if (cv_jaws.value)								SETITEMRESULT(ppos, pingame, 17)	// 2x Jaws
		if (cv_lightning.value && pingame > pexiting)	SETITEMRESULT(ppos, pingame, 18)	// Size-Down Monitor

		prandom = P_RandomKey(numchoices);

		// Award the player whatever power is rolled
		if (numchoices > 0)
			K_KartGetItemResult(player, spawnchance[prandom], false)
		else
			CONS_Printf("ERROR: P_KartItemRoulette - There were no choices given by the roulette (ppos = %d).\n", ppos);
	}
	else
		CONS_Printf("ERROR: P_KartItemRoulette - There's no applicable game type!\n");
	*/

#undef SETITEMRESULT

	player->kartstuff[k_itemroulette] = 0; // Since we're done, clear the roulette number

	if (P_IsLocalPlayer(player))
		S_StartSound(NULL, sfx_mkitmF);
}

//}

//{ SRB2kart p_user.c Stuff

boolean K_IsTouching(mobj_t *mobj1, mobj_t *mobj2)
{
	if (mobj1 == NULL || mobj2 == NULL)
		return false;
	fixed_t absx = abs(mobj1->x - mobj2->x);
	fixed_t absy = abs(mobj1->y - mobj2->y);
	fixed_t absz = abs(mobj1->z - mobj2->z);

	if (absx < 32*FRACUNIT && absy < 32*FRACUNIT && absz < 32*FRACUNIT)
		return true;
	else
		return false;
}

void K_SwapMomentum(mobj_t *mobj1, mobj_t *mobj2, boolean bounce)
{
	fixed_t newx;
	fixed_t newy;

	if (mobj1 == NULL || mobj2 == NULL)
		return;

	fixed_t meanX = (mobj1->momx + mobj2->momx) / 2;
	fixed_t meanY = (mobj1->momy + mobj2->momy) / 2;
	fixed_t deltaV1 = P_AproxDistance((mobj1->momx - meanX), (mobj1->momy - meanY));
	fixed_t deltaV2 = P_AproxDistance((mobj2->momx - meanX), (mobj2->momy - meanY));
	//fixed_t clashvolume = (deltaV1 / FRACUNIT) * 8; // In case you want to do a scaling bump sound volume.
	if (cv_collidesounds.value == 1)
	{
		S_StartSound(mobj1, cv_collidesoundnum.value);
		//S_StartSound(mobj2, cv_collidesoundnum.value);
	}
	if (deltaV1 < (cv_collideminimum.value * FRACUNIT / 2))
	{
		fixed_t a = 0;
		if (deltaV1 != 0)
			a = FixedDiv((cv_collideminimum.value * FRACUNIT / 2), deltaV1);
		else if (deltaV2 != 0)
			a = FixedDiv((cv_collideminimum.value * FRACUNIT / 2), deltaV2);
		else
			a = 0;
		fixed_t deltax1 = (mobj1->momx - meanX);
		fixed_t deltax2 = (mobj2->momx - meanX);
		fixed_t deltay1 = (mobj1->momy - meanY);
		fixed_t deltay2 = (mobj2->momy - meanY);
		mobj1->momx = meanX + FixedMul(deltax1, a);
		mobj1->momy = meanY + FixedMul(deltay1, a);
		mobj2->momx = meanX + FixedMul(deltax2, a);
		mobj2->momy = meanY + FixedMul(deltay2, a);
	}
	if (mobj1->player && mobj2->player) // Weight is applicable if both are players
	{
		fixed_t m1w = 15 + mobj1->player->kartweight;
		fixed_t m2w = 15 + mobj2->player->kartweight;

		newx = FixedMul(mobj1->momx, FixedDiv(m1w*FRACUNIT, m2w*FRACUNIT));
		newy = FixedMul(mobj1->momy, FixedDiv(m1w*FRACUNIT, m2w*FRACUNIT));
		mobj1->momx = FixedMul(mobj2->momx, FixedDiv(m2w*FRACUNIT, m1w*FRACUNIT));
		mobj1->momy = FixedMul(mobj2->momy, FixedDiv(m2w*FRACUNIT, m1w*FRACUNIT));
	}
	else
	{
		newx = mobj1->momx;
		newy = mobj1->momy;
		mobj1->momx = mobj2->momx;
		mobj1->momy = mobj2->momy;
	}
	mobj2->momx = newx;
	mobj2->momy = newy;
	if (bounce == true) // Perform a Goomba Bounce.
		mobj1->momz = -mobj1->momz;
	else
	{
		fixed_t newz = mobj1->momz;
		mobj1->momz = mobj2->momz;
		mobj2->momz = newz;
	}
}

void K_KartBouncer(void)
{
	fixed_t i, j;
	for (i = 0; i < MAXPLAYERS; i++)
		if (playeringame[i] && players[i].mo && !P_MobjWasRemoved(players[i].mo))
			for (j = 0; j < MAXPLAYERS; j++)
				players[i].collide[j] = false;
	for (i = 0; i < MAXPLAYERS; i++)
		if (playeringame[i] && players[i].mo && !P_MobjWasRemoved(players[i].mo))
		{
			for (j = i+1; j < MAXPLAYERS; j++)
				if (playeringame[j] && players[j].mo && !P_MobjWasRemoved(players[j].mo))
				{
					if (players[j].mo == players[i].mo)
						break;
					if (K_IsTouching(players[i].mo, players[j].mo))
					{
						if (!players[i].collide[j] && !players[j].collide[i])
						{
							if (P_IsObjectOnGround(players[j].mo) && players[i].mo->momz < 0)
								K_SwapMomentum(players[i].mo, players[j].mo, true);
							else if (P_IsObjectOnGround(players[i].mo) && players[j].mo->momz < 0)
								K_SwapMomentum(players[j].mo, players[i].mo, true);
							else
								K_SwapMomentum(players[i].mo, players[j].mo, false);
							players[i].collide[j] = true;
							players[j].collide[i] = true;
						}
					}
					else
					{
						players[i].collide[j] = false;
						players[j].collide[i] = false;
					}
				}
		}
}

/**	\brief	Checks that the player is on an offroad subsector for realsies

	\param	mo	player mobj object

	\return	boolean
*/
static boolean K_CheckOffroadCollide(mobj_t *mo)
{
	I_Assert(mo != NULL);
	I_Assert(!P_MobjWasRemoved(mo));

	if (((mo->z <= mo->subsector->sector->floorheight
		&& !(mo->eflags & MFE_VERTICALFLIP) && (mo->subsector->sector->flags & SF_FLIPSPECIAL_FLOOR))
	|| (mo->z + mo->height >= mo->subsector->sector->ceilingheight
		&& (mo->eflags & MFE_VERTICALFLIP) && (mo->subsector->sector->flags & SF_FLIPSPECIAL_CEILING)))
	&& (GETSECSPECIAL(mo->subsector->sector->special, 1) == 2
	|| GETSECSPECIAL(mo->subsector->sector->special, 1) == 3
	|| GETSECSPECIAL(mo->subsector->sector->special, 1) == 4))
		return true;

	return false;
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
	
	if (GETSECSPECIAL(nextsector->special, 1) == 2)		// Weak Offroad
		offroadstrength = 1;
	else if (GETSECSPECIAL(nextsector->special, 1) == 3)	// Mid Offroad
		offroadstrength = 2;
	else if (GETSECSPECIAL(nextsector->special, 1) == 4)	// Strong Offroad
		offroadstrength = 3;

	// If you are offroad, a timer starts. Depending on your weight value, the timer increments differently.
	//if ((nextsector->special & 256) && nextsector->special != 768 
	//	&& nextsector->special != 1024 && nextsector->special != 4864)
	if (offroadstrength)
	{
		if (K_CheckOffroadCollide(player->mo) && player->kartstuff[k_offroad] == 0)
			player->kartstuff[k_offroad] = 16;
		if (player->kartstuff[k_offroad] > 0)
		{
			if (kartweight < 1) { kartweight = 1; } if (kartweight > 9) { kartweight = 9; } // Safety Net

			// 1872 is the magic number - 35 frames adds up to approximately 65536. 1872/4 = 468/3 = 156
			// A higher kart weight means you can stay offroad for longer without losing speed
			offroad = (1872 + 5*156 - kartweight*156)*offroadstrength;

			if (player->kartstuff[k_growshrinktimer] > 1) // megashroom slows down half as fast
				offroad /= 2;

			player->kartstuff[k_offroad] += offroad;
		}

		if (player->kartstuff[k_offroad] > FRACUNIT*offroadstrength)
			player->kartstuff[k_offroad] = FRACUNIT*offroadstrength;
	}
	else
		player->kartstuff[k_offroad] = 0;
}

/**	\brief	Decreases various kart timers and powers per frame. Called in P_PlayerThink in p_user.c

	\param	player	player object passed from P_PlayerThink
	\param	cmd		control input from player

	\return	void
*/
void K_KartPlayerThink(player_t *player, ticcmd_t *cmd)
{
	K_UpdateOffroad(player);

	if (player->kartstuff[k_itemclose])
		player->kartstuff[k_itemclose]--;

	if (player->kartstuff[k_spinout])
		player->kartstuff[k_spinout]--;

	if (player->kartstuff[k_spinouttimer])
		player->kartstuff[k_spinouttimer]--;

	if (player->kartstuff[k_spinout] == 0 && player->kartstuff[k_spinouttimer] == 0 && player->powers[pw_flashing] == flashingtics)
		player->powers[pw_flashing]--;

	if (player->kartstuff[k_magnettimer])
		player->kartstuff[k_magnettimer]--;

	if (player->kartstuff[k_mushroomtimer])
		player->kartstuff[k_mushroomtimer]--;

	if (player->kartstuff[k_floorboost])
		player->kartstuff[k_floorboost]--;

	if (player->kartstuff[k_driftboost])
		player->kartstuff[k_driftboost]--;

	if (player->kartstuff[k_startimer])
		player->kartstuff[k_startimer]--;

	if (player->kartstuff[k_growshrinktimer] > 0)
		player->kartstuff[k_growshrinktimer]--;

	if (player->kartstuff[k_growshrinktimer] < 0)
		player->kartstuff[k_growshrinktimer]++;

	if (player->kartstuff[k_growshrinktimer] == 1 || player->kartstuff[k_growshrinktimer] == -1)
	{
		player->mo->destscale = FRACUNIT;
		P_RestoreMusic(player);
	}

	if (player->kartstuff[k_bootaketimer] == 0 && player->kartstuff[k_boostolentimer] == 0
		&& player->kartstuff[k_goldshroomtimer])
		player->kartstuff[k_goldshroomtimer]--;

	if (player->kartstuff[k_bootaketimer] == 0 && player->kartstuff[k_boostolentimer] == 0
		&& player->kartstuff[k_fireflowertimer])
		player->kartstuff[k_fireflowertimer]--;

	if (player->kartstuff[k_bootaketimer])
		player->kartstuff[k_bootaketimer]--;

	if (player->kartstuff[k_boostolentimer])
		player->kartstuff[k_boostolentimer]--;

	if (player->kartstuff[k_squishedtimer])
		player->kartstuff[k_squishedtimer]--;

	if (player->kartstuff[k_laserwisptimer])
		player->kartstuff[k_laserwisptimer]--;

	if (player->kartstuff[k_sounds])
		player->kartstuff[k_sounds]--;

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

	if (cmd->buttons & BT_JUMP)
		player->kartstuff[k_jmp] = 1;
	else
		player->kartstuff[k_jmp] = 0;

	K_KartItemRoulette(player, cmd); // Roulette Code

	// Stopping of the horrible star SFX
	if (player->mo->health <= 0 || player->mo->player->kartstuff[k_startimer] <= 0
		|| player->mo->player->kartstuff[k_growshrinktimer] > 0) 	// If you don't have invincibility (or mega is active too)
	{
		if (S_SoundPlaying(player->mo, sfx_star)) 					// But the sound is playing
			S_StopSoundByID(player->mo, sfx_star); 					// Stop it
	}

	// And the same for the mega mushroom SFX
	if (!(player->mo->health > 0 && player->mo->player->kartstuff[k_growshrinktimer] > 0)) // If you aren't big
	{
		if (S_SoundPlaying(player->mo, sfx_mega)) // But the sound is playing
			S_StopSoundByID(player->mo, sfx_mega); // Stop it
	}

	// Plays the music after the starting countdown.
	if (P_IsLocalPlayer(player) && leveltime == 158)
		S_ChangeMusicInternal(mapmusname, true);
}

static void K_PlayTauntSound(mobj_t *source)
{
	switch (P_RandomFixed() % 4)
	{
		case 1:
			S_StartSound(source, sfx_taunt1);
			return;
		case 2:
			S_StartSound(source, sfx_taunt2);
			return;
		case 3:
			S_StartSound(source, sfx_taunt3);
			return;
		case 4:
			S_StartSound(source, sfx_taunt4);
			return;
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
	if (!(player->kartstuff[k_startimer] || player->kartstuff[k_bootaketimer] || player->kartstuff[k_mushroomtimer])
		&& player->kartstuff[k_offroad] >= 0 && speed)
			boostpower = FixedDiv(boostpower, player->kartstuff[k_offroad] + FRACUNIT);

	if (player->kartstuff[k_growshrinktimer] > 1
		&& (player->kartstuff[k_growshrinktimer] > (itemtime - 25)
		|| player->kartstuff[k_growshrinktimer] <= 26))
	{												// Mega Mushroom - Mid-size
		if (speed)
		{
			boostvalue = max(boostvalue, FRACUNIT/8); // + 12.5%
		}
	}
	if (player->kartstuff[k_growshrinktimer] < (itemtime - 25)
		&& player->kartstuff[k_growshrinktimer] > 26)
	{												// Mega Mushroom
		if (speed)
		{
			boostvalue = max(boostvalue, FRACUNIT/4); // + 25%
		}
	}
	if (player->kartstuff[k_startimer])
	{												// Star
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
	if (player->kartstuff[k_mushroomtimer])
	{												// Mushroom
		if (speed)
			boostvalue = max(boostvalue, FRACUNIT/2); // + 50%
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
	fixed_t finalspeed;

	switch (cv_kartcc.value)
	{
		case 50:
			g_cc = 53248 + xspd; //  50cc =  81.25 + 4.69 =  85.94%
			break;
		case 150:
			g_cc = 77824 + xspd; // 150cc = 118.75 + 4.69 = 123.44%
			break;
		default:
			g_cc = 65536 + xspd; // 100cc = 100.00 + 4.69 = 104.69%
			break;
	}

	k_speed += player->kartspeed*3; // 153 - 177

	finalspeed = FixedMul(FixedMul(k_speed<<14, g_cc), player->mo->scale);

	if (doboostpower)
		return FixedMul(finalspeed, K_GetKartBoostPower(player, true));
	return finalspeed;
}

static fixed_t K_GetKartAccel(player_t *player)
{
	fixed_t k_accel = 32; // 36;

	//k_accel += 3 * (9 - player->kartspeed); // 36 - 60
	k_accel += 4 * (9 - player->kartspeed); // 32 - 64

	return FixedMul(k_accel, K_GetKartBoostPower(player, false));
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

void K_SpinPlayer(player_t *player, mobj_t *source)
{
	(void) source;
	if (player->health <= 0)
		return;

	if (player->powers[pw_flashing] > 0 || player->kartstuff[k_squishedtimer] > 0 || (player->kartstuff[k_spinouttimer] > 0 && player->kartstuff[k_spinout] > 0)
		|| player->kartstuff[k_startimer] > 0 || player->kartstuff[k_growshrinktimer] > 0 || player->kartstuff[k_bootaketimer] > 0)
		return;

	player->kartstuff[k_mushroomtimer] = 0;
	player->kartstuff[k_driftboost] = 0;

	if (player->kartstuff[k_spinouttype] <= 0)
	{
		if (player->kartstuff[k_spinouttype] == 0)
			player->kartstuff[k_spinouttimer] = 2*TICRATE;
		else
			player->kartstuff[k_spinouttimer] = 3*TICRATE/2;

		if (player->speed < K_GetKartSpeed(player, true)/4) // player->normalspeed/4)
			P_InstaThrust(player->mo, player->mo->angle, FixedMul(K_GetKartSpeed(player, true)/4, player->mo->scale)); // FixedMul(player->normalspeed/4, player->mo->scale));

		S_StartSound(player->mo, sfx_slip);
	}
	else
		player->kartstuff[k_spinouttimer] = 1*TICRATE;

	player->powers[pw_flashing] = flashingtics;

	player->kartstuff[k_spinout] = player->kartstuff[k_spinouttimer];

	if (!(player->mo->state >= &states[S_KART_SPIN1] && player->mo->state <= &states[S_KART_SPIN8]))
		P_SetPlayerMobjState(player->mo, S_KART_SPIN1);

	player->kartstuff[k_spinouttype] = 0;

	return;
}

void K_SquishPlayer(player_t *player, mobj_t *source)
{
	(void) source;
	if (player->health <= 0)
		return;

	if (player->powers[pw_flashing] > 0	|| player->kartstuff[k_squishedtimer] > 0
		|| player->kartstuff[k_startimer] > 0 || player->kartstuff[k_growshrinktimer] > 0 || player->kartstuff[k_bootaketimer] > 0)
		return;

	player->kartstuff[k_mushroomtimer] = 0;
	player->kartstuff[k_driftboost] = 0;

	player->kartstuff[k_squishedtimer] = 2*TICRATE;

	player->powers[pw_flashing] = flashingtics;

	player->mo->flags |= MF_NOCLIP;

	if (player->mo->state != &states[S_KART_SQUISH]) // Squash
		P_SetPlayerMobjState(player->mo, S_KART_SQUISH);

	P_PlayRinglossSound(player->mo);

	return;
}

void K_ExplodePlayer(player_t *player, mobj_t *source) // A bit of a hack, we just throw the player up higher here and extend their spinout timer
{
	(void) source;
	if (player->health <= 0)
		return;

	if (player->powers[pw_flashing] > 0 || player->kartstuff[k_squishedtimer] > 0 || (player->kartstuff[k_spinouttimer] > 0 && player->kartstuff[k_spinout] > 0)
		|| player->kartstuff[k_startimer] > 0 || player->kartstuff[k_growshrinktimer] > 0 || player->kartstuff[k_bootaketimer] > 0)
		return;

	player->mo->momz = 18*FRACUNIT;
	player->mo->momx = player->mo->momy = 0;

	player->kartstuff[k_mushroomtimer] = 0;
	player->kartstuff[k_driftboost] = 0;

	player->kartstuff[k_spinouttype] = 1;
	player->kartstuff[k_spinouttimer] = 2*TICRATE+(TICRATE/2);
	player->kartstuff[k_spinout] = player->kartstuff[k_spinouttimer];

	player->powers[pw_flashing] = flashingtics;

	if (!(player->mo->state >= &states[S_KART_SPIN1] && player->mo->state <= &states[S_KART_SPIN8]))
		P_SetPlayerMobjState(player->mo, S_KART_SPIN1);

	player->kartstuff[k_spinouttype] = 0;

	P_PlayRinglossSound(player->mo);

	return;
}

void K_SpawnKartExplosion(fixed_t x, fixed_t y, fixed_t z, fixed_t radius, INT32 number, mobjtype_t type, angle_t rotangle, boolean spawncenter, boolean ghostit)
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

	radius = FixedDiv(radius,5*(FRACUNIT/4));

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

		mobj->momx = FixedMul(FixedDiv(mobjx - x, dist), 2*FRACUNIT);
		mobj->momy = FixedMul(FixedDiv(mobjy - y, dist), 2*FRACUNIT);
		mobj->momz = FixedMul(FixedDiv(mobjz - z, dist), 2*FRACUNIT);

		if (spawncenter)
		{
			mobj->x = x;
			mobj->y = y;
			mobj->z = z;
		}

		mobj->flags |= MF_NOCLIPTHING;
		mobj->flags &= ~MF_SPECIAL;

	}
}

static mobj_t *K_SpawnKartMissile(mobj_t *source, mobjtype_t type, angle_t angle, INT32 flags2, fixed_t speed)
{
	mobj_t *th;
	angle_t an;
	fixed_t x, y, z;
	//INT32 dir;

	// angle at which you fire, is player angle
	an = angle;

	//if (source->player->kartstuff[k_throwdir] != 0)
	//	dir = source->player->kartstuff[k_throwdir];
	//else
	//	dir = 1;

	x = source->x + source->momx;
	y = source->y + source->momy;
	z = source->z + source->height/3;

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

	th->angle = an;
	th->momx = FixedMul(speed, FINECOSINE(an>>ANGLETOFINESHIFT));
	th->momy = FixedMul(speed, FINESINE(an>>ANGLETOFINESHIFT));

	return NULL;
}

void K_SpawnDriftTrail(player_t *player)
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

	if (player->mo->eflags & MFE_VERTICALFLIP)
		ground = player->mo->ceilingz - FixedMul(mobjinfo[MT_MUSHROOMTRAIL].height, player->mo->scale);
	else
		ground = player->mo->floorz;

	if (player->kartstuff[k_drift] != 0)
		travelangle = player->mo->angle;
	else
		travelangle = R_PointToAngle2(0, 0, player->rmomx, player->rmomy);

	for (i = 0; i < 2; i++)
	{
		if (player->kartstuff[k_bootaketimer] != 0)
			continue;

		newx = player->mo->x + P_ReturnThrustX(player->mo, travelangle + ((i&1) ? -1 : 1)*ANGLE_135, FixedMul(24*FRACUNIT, player->mo->scale));
		newy = player->mo->y + P_ReturnThrustY(player->mo, travelangle + ((i&1) ? -1 : 1)*ANGLE_135, FixedMul(24*FRACUNIT, player->mo->scale));
#ifdef ESLOPE
		if (player->mo->standingslope)
		{
			ground = P_GetZAt(player->mo->standingslope, newx, newy);
			if (player->mo->eflags & MFE_VERTICALFLIP)
				ground -= FixedMul(mobjinfo[MT_MUSHROOMTRAIL].height, player->mo->scale);
		}
#endif
		if (player->kartstuff[k_drift] != 0 && player->kartstuff[k_mushroomtimer] == 0)
			flame = P_SpawnMobj(newx, newy, ground, MT_DRIFTSMOKE);
		else
			flame = P_SpawnMobj(newx, newy, ground, MT_MUSHROOMTRAIL);

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

static mobj_t *P_ThrowKartItem(player_t *player, boolean missile, mobjtype_t mapthing, INT32 defaultDir, boolean bobombthrow)
{
	mobj_t *mo;
	INT32 dir;
	angle_t newangle;
	fixed_t newx;
	fixed_t newy;

	if (!player)
		return NULL;

	if (bobombthrow)
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

	if (missile)
	{
		if (dir == -1)
		{
			// Shoot backward
			mo = K_SpawnKartMissile(player->mo, mapthing, player->mo->angle + ANGLE_180, 0, 64*FRACUNIT);

			if (mo)
			{
				if (player->mo->eflags & MFE_VERTICALFLIP)
					mo->eflags |= MFE_VERTICALFLIP;
			}
		}
		else
		{
			// Shoot forward
			mo = K_SpawnKartMissile(player->mo, mapthing, player->mo->angle, 0, 64*FRACUNIT);

			if (mo)
			{
				if (player->mo->eflags & MFE_VERTICALFLIP)
					mo->eflags |= MFE_VERTICALFLIP;
			}
		}
	}
	else
	{
		if (dir == 1 || dir == 2)
		{
			// Shoot forward
			mo = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z + 80*FRACUNIT, mapthing);

			mo->threshold = 10;

			P_SetTarget(&mo->target, player->mo);

			if (mo)
			{
				angle_t fa = player->mo->angle>>ANGLETOFINESHIFT;
				INT32 DIST = 50*FRACUNIT + player->speed*FRACUNIT; // 6 when dropping CTF flag
				INT32 HEIGHT;
				if (DIST > 64*FRACUNIT)
					DIST = 64*FRACUNIT;

				if (dir == 2)
					HEIGHT = 16*FRACUNIT + player->mo->momz;
				else
					HEIGHT = 8*FRACUNIT + player->mo->momz;

				if (HEIGHT > 64*FRACUNIT)
					HEIGHT = 64*FRACUNIT;

				mo->momx = FixedMul(FINECOSINE(fa), DIST);
				mo->momy = FixedMul(FINESINE(fa), DIST);
				mo->momz = HEIGHT;

				if (player->mo->eflags & MFE_VERTICALFLIP)
					mo->eflags |= MFE_VERTICALFLIP;
			}
		}
		else
		{
			// Drop it directly behind you.
			newangle = player->mo->angle;

			mo = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, mapthing);

			mo->threshold = 10;

			P_SetTarget(&mo->target, player->mo);

			newx = player->mo->x + P_ReturnThrustX(player->mo, newangle + ANGLE_180, player->mo->radius*2 + mo->radius*3);
			newy = player->mo->y + P_ReturnThrustY(player->mo, newangle + ANGLE_180, player->mo->radius*2 + mo->radius*3);

			mo->x = newx;
			mo->y = newy;

			if (mo)
			{
				if (player->mo->eflags & MFE_VERTICALFLIP)
					mo->eflags |= MFE_VERTICALFLIP;
			}
		}
	}
	return mo;
}

static void K_DoMagnet(player_t *player)
{
	S_StartSound(player->mo, sfx_s3k45);
	player->kartstuff[k_magnettimer] = 35;
	P_NukeEnemies(player->mo, player->mo, 16*FRACUNIT);
}

static void K_DoBooSteal(player_t *player)
{
	INT32 i, numplayers = 0;
	INT32 playerswappable[MAXPLAYERS];
	INT32 stealplayer = 0; // The player that's getting stolen from
	INT32 prandom = 0;

	if (!multiplayer)
		return;

	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (playeringame[i] && players[i].mo && players[i].mo->health > 0 && players[i].playerstate == PST_LIVE
			&& !players[i].exiting && !players[i].powers[pw_super] && !((netgame || multiplayer) && players[i].spectator)
			&& players[i].kartstuff[k_position] < player->kartstuff[k_position] && player != &players[i]

			&& (players[i].kartstuff[k_star] || players[i].kartstuff[k_mushroom] || players[i].kartstuff[k_goldshroom]
			|| players[i].kartstuff[k_megashroom] || players[i].kartstuff[k_lightning] || players[i].kartstuff[k_blueshell]
			|| players[i].kartstuff[k_greenshell] & 2 || players[i].kartstuff[k_triplegreenshell] & 8
			|| players[i].kartstuff[k_redshell] & 2 || players[i].kartstuff[k_tripleredshell] & 8
			|| players[i].kartstuff[k_banana] & 2 || players[i].kartstuff[k_triplebanana] & 8
			|| players[i].kartstuff[k_fakeitem] & 2 || players[i].kartstuff[k_bobomb] & 2
			|| players[i].kartstuff[k_boo])) // Stealing boos with boos? sounds like fun
		{
			playerswappable[numplayers] = i+1;
			numplayers++;
		}
	}

	prandom = P_RandomFixed();

	if (player->kartstuff[k_position] == 1 || numplayers < 1) // No-one can be stolen from? Get longer invisibility for nothing
	{
		player->kartstuff[k_bootaketimer] = bootime;
		player->kartstuff[k_boo] = 0;
		return;
	}
	else if (numplayers == 1) // With just 2 players, we just need to set the other player to be the one to steal from
	{
		stealplayer = playerswappable[numplayers - 1];
	}
	else if (numplayers > 1) // We need to choose between the available candidates for the 2nd player
	{
		stealplayer = playerswappable[prandom%(numplayers-1)];
	}

	if (stealplayer) // Now here's where we do the stealing, has to be done here because we still know the player we're stealing from
	{
		stealplayer -= 1; // stealplayer is +1 so we know if it found there actually WAS a player

		player->kartstuff[k_bootaketimer] = bootime;
		player->kartstuff[k_boo] = 0;
		players[stealplayer].kartstuff[k_boostolentimer] = bootime;

		if (players[stealplayer].kartstuff[k_star])
		{
			player->kartstuff[k_star] = players[stealplayer].kartstuff[k_star];
			players[stealplayer].kartstuff[k_star] = 0;
		}
		else if (players[stealplayer].kartstuff[k_mushroom])
		{
			player->kartstuff[k_mushroom] = players[stealplayer].kartstuff[k_mushroom];
			players[stealplayer].kartstuff[k_mushroom] = 0;
		}
		else if (players[stealplayer].kartstuff[k_goldshroom])
		{
			player->kartstuff[k_goldshroom] = players[stealplayer].kartstuff[k_goldshroom];
			players[stealplayer].kartstuff[k_goldshroom] = 0;
		}
		else if (players[stealplayer].kartstuff[k_megashroom])
		{
			player->kartstuff[k_megashroom] = players[stealplayer].kartstuff[k_megashroom];
			players[stealplayer].kartstuff[k_megashroom] = 0;
		}
		else if (players[stealplayer].kartstuff[k_lightning])
		{
			player->kartstuff[k_lightning] = players[stealplayer].kartstuff[k_lightning];
			players[stealplayer].kartstuff[k_lightning] = 0;
		}
		else if (players[stealplayer].kartstuff[k_blueshell])
		{
			player->kartstuff[k_blueshell] = players[stealplayer].kartstuff[k_blueshell];
			players[stealplayer].kartstuff[k_blueshell] = 0;
		}
		else if (players[stealplayer].kartstuff[k_greenshell] & 2)
		{
			player->kartstuff[k_greenshell] |= 2;
			players[stealplayer].kartstuff[k_greenshell] &= ~2;
		}
		else if (players[stealplayer].kartstuff[k_triplegreenshell] & 8)
		{
			player->kartstuff[k_triplegreenshell] |= 8;
			players[stealplayer].kartstuff[k_triplegreenshell] &= ~8;
		}
		else if (players[stealplayer].kartstuff[k_redshell] & 2)
		{
			player->kartstuff[k_redshell] |= 2;
			players[stealplayer].kartstuff[k_redshell] &= ~2;
		}
		else if (players[stealplayer].kartstuff[k_tripleredshell] & 8)
		{
			player->kartstuff[k_tripleredshell] |= 8;
			players[stealplayer].kartstuff[k_tripleredshell] &= ~8;
		}
		else if (players[stealplayer].kartstuff[k_banana] & 2)
		{
			player->kartstuff[k_banana] |= 2;
			players[stealplayer].kartstuff[k_banana] &= ~2;
		}
		else if (players[stealplayer].kartstuff[k_triplebanana] & 8)
		{
			player->kartstuff[k_triplebanana] |= 8;
			players[stealplayer].kartstuff[k_triplebanana] &= ~8;
		}
		else if (players[stealplayer].kartstuff[k_fakeitem] & 2)
		{
			player->kartstuff[k_fakeitem] |= 2;
			players[stealplayer].kartstuff[k_fakeitem] &= ~2;
		}
		else if (players[stealplayer].kartstuff[k_bobomb] & 2)
		{
			player->kartstuff[k_bobomb] |= 2;
			players[stealplayer].kartstuff[k_bobomb] &= ~2;
		}
		if (players[stealplayer].kartstuff[k_boo])
		{
			player->kartstuff[k_boo] = players[stealplayer].kartstuff[k_boo];
			players[stealplayer].kartstuff[k_boo] = 0;
		}
	}
}

void K_DoMushroom(player_t *player, boolean doPFlag)
{
	if (!player->kartstuff[k_floorboost] || player->kartstuff[k_floorboost] == 3)
		S_StartSound(player->mo, sfx_mush);

	player->kartstuff[k_mushroomtimer] = mushroomtime;

	if (doPFlag)
		player->pflags |= PF_ATTACKDOWN;

	if (player->kartstuff[k_sounds]) // Prevents taunt sounds from playing every time the button is pressed
		return;

	K_PlayTauntSound(player->mo);
	player->kartstuff[k_sounds] = 50;
}

static void K_DoLightning(player_t *player, boolean bluelightning)
{
	mobj_t *mo;
	thinker_t *think;
	INT32 i;
	S_StartSound(player->mo, sfx_bkpoof); // Sound the BANG!
	player->pflags |= PF_ATTACKDOWN;

	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (playeringame[i])
			P_FlashPal(&players[i], PAL_NUKE, 10);
	}

	for (think = thinkercap.next; think != &thinkercap; think = think->next)
	{
		if (think->function.acp1 != (actionf_p1)P_MobjThinker)
			continue; // not a mobj thinker

		mo = (mobj_t *)think;

		if (mo->type == MT_PLAYER)
			P_DamageMobj(mo, player->mo, player->mo, bluelightning ? 65 : 64);
		else
			continue;
	}

	if (player->kartstuff[k_sounds]) // Prevents taunt sounds from playing every time the button is pressed
		return;

	K_PlayTauntSound(player->mo);
	player->kartstuff[k_sounds] = 50;
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

	if (player->kartstuff[k_startimer] || player->kartstuff[k_mushroomtimer] || player->kartstuff[k_growshrinktimer] > 0)
		turnvalue = FixedMul(turnvalue, FixedDiv(5*FRACUNIT, 4*FRACUNIT));

	return turnvalue;
}

static void K_KartDrift(player_t *player, boolean onground)
{
	// IF YOU CHANGE THESE: MAKE SURE YOU UPDATE THE SAME VALUES IN p_mobjc, "case MT_DRIFT:"
	fixed_t dsone = 26*4 + player->kartspeed*2 + (9 - player->kartweight);
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
		S_StartSound(player->mo, sfx_mush);
		player->kartstuff[k_driftcharge] = 0;
	}
	else if ((player->kartstuff[k_drift] != -5 && player->kartstuff[k_drift] != 5)
		// || (player->kartstuff[k_drift] >= 1 && player->kartstuff[k_turndir] != 1) || (player->kartstuff[k_drift] <= -1 && player->kartstuff[k_turndir] != -1))
		&& player->kartstuff[k_driftcharge] >= dstwo
		&& onground)
	{
		player->kartstuff[k_driftboost] = 50;
		S_StartSound(player->mo, sfx_mush);
		player->kartstuff[k_driftcharge] = 0;
	}

	// Drifting: left or right?
	if ((player->cmd.buttons & BT_DRIFTLEFT) && player->speed > (10<<16) && player->kartstuff[k_jmp] == 1
		&& (player->kartstuff[k_drift] == 0 || player->kartstuff[k_driftend] == 1)) // && player->kartstuff[k_drift] != 1)
	{
		// Starting left drift
		player->kartstuff[k_drift] = 1;
		player->kartstuff[k_driftend] = 0;
		player->kartstuff[k_driftcharge] = 0;
	}
	else if ((player->cmd.buttons & BT_DRIFTRIGHT) && player->speed > (10<<16) && player->kartstuff[k_jmp] == 1
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
	if (player->kartstuff[k_spinouttimer] == 0 && player->kartstuff[k_jmp] == 1 && onground
		&& player->kartstuff[k_drift] != 0)
	{
		fixed_t driftadditive = 3;

		if (player->kartstuff[k_drift] >= 1) // Drifting to the left
		{
			player->kartstuff[k_drift]++;
			if (player->kartstuff[k_drift] > 5)
				player->kartstuff[k_drift] = 5;

			if (player->cmd.buttons & BT_DRIFTLEFT) // Inward
				driftadditive++;
			if (player->cmd.buttons & BT_DRIFTRIGHT) // Outward
				driftadditive--;
		}
		else if (player->kartstuff[k_drift] <= -1) // Drifting to the right
		{
			player->kartstuff[k_drift]--;
			if (player->kartstuff[k_drift] < -5)
				player->kartstuff[k_drift] = -5;

			if (player->cmd.buttons & BT_DRIFTRIGHT) // Inward
				driftadditive++;
			if (player->cmd.buttons & BT_DRIFTLEFT) // Outward
				driftadditive--;
		}

		// This spawns the drift sparks
		if (player->kartstuff[k_driftcharge] < dsone && player->kartstuff[k_driftcharge] + driftadditive >= dsone)
			P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_DRIFT)->target = player->mo;

		player->kartstuff[k_driftcharge] += driftadditive;
		player->kartstuff[k_driftend] = 0;
	}

	// Stop drifting
	if (player->kartstuff[k_spinouttimer] > 0 // banana peel
		|| player->speed < (10<<16)) // you're too slow!
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
	fixed_t i, ppcd, pncd, ipcd, incd;
	fixed_t pmo, imo;
	thinker_t *th;
	mobj_t *mo;

	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (playeringame[i] && !players[i].spectator &&
			(((players[i].starpostnum) + (numstarposts + 1) * players[i].laps) >
			((player->starpostnum) + (numstarposts + 1) * player->laps)))
			position++;
		else if (playeringame[i] && !players[i].spectator
			&& (((players[i].starpostnum) + (numstarposts+1)*players[i].laps) ==
			((player->starpostnum) + (numstarposts+1)*player->laps)))
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

				pmo = P_AproxDistance(P_AproxDistance(	mo->x - player->mo->x,
														mo->y - player->mo->y),
														mo->z - player->mo->z) / FRACUNIT;
				imo = P_AproxDistance(P_AproxDistance(	mo->x - players[i].mo->x,
														mo->y - players[i].mo->y),
														mo->z - players[i].mo->z) / FRACUNIT;

				if (mo->type != MT_BOSS3WAYPOINT) // TODO: Change to 'MT_WAYPOINT'?
					continue;

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
	player->kartstuff[k_position] = position;
}

static boolean K_CheckForHoldItem(player_t *player)
{
	if (	player->kartstuff[k_greenshell] == 1
		|| 	player->kartstuff[k_redshell] == 1
		|| 	player->kartstuff[k_banana] == 1
		|| 	player->kartstuff[k_fakeitem] == 1
		|| 	player->kartstuff[k_bobomb] == 1
		|| 	player->kartstuff[k_triplegreenshell] & 1
		|| 	player->kartstuff[k_triplegreenshell] & 2
		|| 	player->kartstuff[k_triplegreenshell] & 4
		|| 	player->kartstuff[k_tripleredshell] & 1
		|| 	player->kartstuff[k_tripleredshell] & 2
		|| 	player->kartstuff[k_tripleredshell] & 4
		|| 	player->kartstuff[k_triplebanana] & 1
		|| 	player->kartstuff[k_triplebanana] & 2
		|| 	player->kartstuff[k_triplebanana] & 4
		) return true;
	return false;
}
//
// K_MoveKartPlayer
//
void K_MoveKartPlayer(player_t *player, boolean onground)
{
	ticcmd_t *cmd = &player->cmd;
	boolean ATTACK_IS_DOWN = ((cmd->buttons & BT_ATTACK) && !(player->pflags & PF_ATTACKDOWN));
	boolean HOLDING_ITEM = K_CheckForHoldItem(player);
	boolean NO_BOO = (player->kartstuff[k_boostolentimer] == 0 && player->kartstuff[k_bootaketimer] == 0);

	K_KartUpdatePosition(player);

	// Position Taunt
	// If you were behind someone but just passed them, taunt at them!
	if (!player->kartstuff[k_positiondelay] && !player->exiting)
	{
		if (player->kartstuff[k_oldposition] <= player->kartstuff[k_position]) // But first, if you lost a place,
			player->kartstuff[k_oldposition] = player->kartstuff[k_position]; // then the other player taunts.
		else if (player->kartstuff[k_oldposition] > player->kartstuff[k_position]) // Otherwise,
		{
			//S_StartSound(player->mo, sfx_slow); // Say "YOU'RE TOO SLOW!"
			player->kartstuff[k_oldposition] = player->kartstuff[k_position]; // Restore the old position,
			player->kartstuff[k_positiondelay] = 5*TICRATE; // and set up a timer.
		}
	}
	if (player->kartstuff[k_positiondelay])
		player->kartstuff[k_positiondelay]--;

	// Race Spectator
	if (netgame && player->jointime < 1
	&& gametype == GT_RACE && countdown)
	{
		player->spectator = true;
		player->powers[pw_nocontrol] = 5;
	}

	if ((player->pflags & PF_ATTACKDOWN) && !(cmd->buttons & BT_ATTACK))
		player->pflags &= ~PF_ATTACKDOWN;
	else if (cmd->buttons & BT_ATTACK)
		player->pflags |= PF_ATTACKDOWN;

	if (player && player->health > 0 && !player->spectator && !player->exiting && player->kartstuff[k_spinouttimer] == 0)
	{

// Magnet
// Boo
// Mushroom
// Triple Mushroom
// Mega Mushroom
// Gold Mushroom
// Star
// Triple Banana
// Fake Item
// Banana
// Green Shell
// Red Shell
// Triple Green Shell
// Bob-omb
// Blue Shell
// Fire Flower
// Triple Red Shell
// Lightning

		// GoldenMushroom power
		if (ATTACK_IS_DOWN && !HOLDING_ITEM && onground && player->kartstuff[k_goldshroom] == 1
			&& player->kartstuff[k_goldshroomtimer] == 0 && NO_BOO)
		{
			K_DoMushroom(player, true);
			player->kartstuff[k_goldshroomtimer] = itemtime;
			player->kartstuff[k_goldshroom] = 0;
		}
		// GoldenMushroom power
		else if (ATTACK_IS_DOWN && player->kartstuff[k_goldshroomtimer] > 1 && onground && NO_BOO)
		{
			K_DoMushroom(player, true);
			//player->kartstuff[k_goldshroomtimer] -= 10;
			//if (player->kartstuff[k_goldshroomtimer] < 1)
			//	player->kartstuff[k_goldshroomtimer] = 1;
		}
		// TripleMushroom power
		else if (ATTACK_IS_DOWN && !HOLDING_ITEM && player->kartstuff[k_mushroom] == 4 && onground && NO_BOO)
		{
			K_DoMushroom(player, true);
			player->kartstuff[k_mushroom] = 2;
		}
		// DoubleMushroom power
		else if (ATTACK_IS_DOWN && !HOLDING_ITEM && player->kartstuff[k_mushroom] == 2 && onground && NO_BOO)
		{
			K_DoMushroom(player, true);
			player->kartstuff[k_mushroom] = 1;
		}
		// Mushroom power
		else if (ATTACK_IS_DOWN && !HOLDING_ITEM && player->kartstuff[k_mushroom] == 1 && onground && NO_BOO)
		{
			K_DoMushroom(player, true);
			player->kartstuff[k_mushroom] = 0;
		}
		// Star power
		else if (ATTACK_IS_DOWN && !HOLDING_ITEM && player->kartstuff[k_star] == 1 && NO_BOO)
		{
			if (P_IsLocalPlayer(player) && !player->exiting)
				S_ChangeMusicInternal("minvnc", true);
			if (!P_IsLocalPlayer(player))
				S_StartSound(player->mo, sfx_star);
			player->kartstuff[k_startimer] = itemtime; // Activate it
			K_PlayTauntSound(player->mo);
			player->kartstuff[k_star] = 0;
			player->kartstuff[k_itemclose] = 10;
			player->pflags |= PF_ATTACKDOWN;
		}
		// Green Shell
		else if (ATTACK_IS_DOWN && !HOLDING_ITEM && player->kartstuff[k_greenshell] & 2 && NO_BOO)
		{
			angle_t newangle;
			fixed_t newx;
			fixed_t newy;
			mobj_t *mo;
			player->kartstuff[k_greenshell] &= ~2;
			player->kartstuff[k_greenshell] |= 1;
			player->pflags |= PF_ATTACKDOWN;
			newangle = player->mo->angle;
			newx = player->mo->x + P_ReturnThrustX(player->mo, newangle + ANGLE_180, 64*FRACUNIT);
			newy = player->mo->y + P_ReturnThrustY(player->mo, newangle + ANGLE_180, 64*FRACUNIT);
			mo = P_SpawnMobj(newx, newy, player->mo->z, MT_GREENSHIELD);
			mo->threshold = 10;
			if (mo)
				P_SetTarget(&mo->target, player->mo);
			player->kartstuff[k_itemclose] = 10;
		}
		else if (!(cmd->buttons & BT_ATTACK) && player->kartstuff[k_greenshell] & 1)
		{
			player->kartstuff[k_greenshell] &= ~1;

			P_ThrowKartItem(player, true, MT_GREENITEM, 1, false);
			K_PlayTauntSound(player->mo);

		}
		// Triple Green Shell
		else if (ATTACK_IS_DOWN && !HOLDING_ITEM && player->kartstuff[k_triplegreenshell] & 8 && NO_BOO)
		{
			angle_t newangle;
			fixed_t newx;
			fixed_t newy;
			mobj_t *mo, *mo2, *mo3;
			player->kartstuff[k_triplegreenshell] &= ~8;
			player->kartstuff[k_triplegreenshell] |= 7;
			player->pflags |= PF_ATTACKDOWN;
			newangle = player->mo->angle;
			K_PlayTauntSound(player->mo);
			newx = player->mo->x + P_ReturnThrustX(player->mo, newangle + ANGLE_180, 64*FRACUNIT);
			newy = player->mo->y + P_ReturnThrustY(player->mo, newangle + ANGLE_180, 64*FRACUNIT);
			mo = P_SpawnMobj(newx, newy, player->mo->z, MT_TRIPLEGREENSHIELD1);
			mo->threshold = 10;
			P_SetTarget(&mo->target, player->mo);
			mo->angle = 0;
			mo2 = P_SpawnMobj(newx, newy, player->mo->z, MT_TRIPLEGREENSHIELD2);
			mo2->threshold = 10;
			P_SetTarget(&mo2->target, player->mo);
			mo2->angle = ANGLE_120;
			mo3 = P_SpawnMobj(newx, newy, player->mo->z, MT_TRIPLEGREENSHIELD3);
			mo3->threshold = 10;
			P_SetTarget(&mo3->target, player->mo);
			mo3->angle = ANGLE_240;
			player->kartstuff[k_itemclose] = 10;
		}
		else if (ATTACK_IS_DOWN && (player->kartstuff[k_triplegreenshell] & 1 || player->kartstuff[k_triplegreenshell] & 2 || player->kartstuff[k_triplegreenshell] & 4))
		{
			P_ThrowKartItem(player, true, MT_GREENITEM, 1, false);
			K_PlayTauntSound(player->mo);
			player->pflags |= PF_ATTACKDOWN;

			if (player->kartstuff[k_triplegreenshell] & 4)
				player->kartstuff[k_triplegreenshell] &= ~4;
			else if (player->kartstuff[k_triplegreenshell] & 2)
				player->kartstuff[k_triplegreenshell] &= ~2;
			else if (player->kartstuff[k_triplegreenshell] & 1)
				player->kartstuff[k_triplegreenshell] &= ~1;

		}
		// Red Shell
		else if (ATTACK_IS_DOWN && !HOLDING_ITEM && player->kartstuff[k_redshell] & 2 && NO_BOO)
		{
			angle_t newangle;
			fixed_t newx;
			fixed_t newy;
			mobj_t *mo;
			player->kartstuff[k_redshell] &= ~2;
			player->kartstuff[k_redshell] |= 1;
			player->pflags |= PF_ATTACKDOWN;
			newangle = player->mo->angle;
			newx = player->mo->x + P_ReturnThrustX(player->mo, newangle + ANGLE_180, 64*FRACUNIT);
			newy = player->mo->y + P_ReturnThrustY(player->mo, newangle + ANGLE_180, 64*FRACUNIT);
			mo = P_SpawnMobj(newx, newy, player->mo->z, MT_REDSHIELD);
			mo->threshold = 10;
			if (mo)
				P_SetTarget(&mo->target, player->mo);
			player->kartstuff[k_itemclose] = 10;
		}
		else if (!(cmd->buttons & BT_ATTACK) && player->kartstuff[k_redshell] & 1 && (player->kartstuff[k_throwdir] == 1 || player->kartstuff[k_throwdir] == 0))
		{
			player->kartstuff[k_redshell] &= ~1;

			P_ThrowKartItem(player, true, MT_REDITEM, 1, false);
			K_PlayTauntSound(player->mo);
		}
		// Red Shell Dud
		else if (!(cmd->buttons & BT_ATTACK) && player->kartstuff[k_redshell] & 1 && player->kartstuff[k_throwdir] == -1)
		{
			player->kartstuff[k_redshell] &= ~1;

			P_ThrowKartItem(player, true, MT_REDITEMDUD, -1, false);
			K_PlayTauntSound(player->mo);
		}
		// Triple Red Shell
		else if (ATTACK_IS_DOWN && !HOLDING_ITEM && player->kartstuff[k_tripleredshell] & 8 && NO_BOO)
		{
			angle_t newangle;
			fixed_t newx;
			fixed_t newy;
			mobj_t *mo, *mo2, *mo3;
			player->kartstuff[k_tripleredshell] &= ~8;
			player->kartstuff[k_tripleredshell] |= 7;
			player->pflags |= PF_ATTACKDOWN;
			newangle = player->mo->angle;
			K_PlayTauntSound(player->mo);
			newx = player->mo->x + P_ReturnThrustX(player->mo, newangle + ANGLE_180, 64*FRACUNIT);
			newy = player->mo->y + P_ReturnThrustY(player->mo, newangle + ANGLE_180, 64*FRACUNIT);
			mo = P_SpawnMobj(newx, newy, player->mo->z, MT_TRIPLEREDSHIELD1);
			mo->threshold = 10;
			P_SetTarget(&mo->target, player->mo);
			mo->angle = 0;
			mo2 = P_SpawnMobj(newx, newy, player->mo->z, MT_TRIPLEREDSHIELD2);
			mo2->threshold = 10;
			P_SetTarget(&mo2->target, player->mo);
			mo2->angle = ANGLE_120;
			mo3 = P_SpawnMobj(newx, newy, player->mo->z, MT_TRIPLEREDSHIELD3);
			mo3->threshold = 10;
			P_SetTarget(&mo3->target, player->mo);
			mo3->angle = ANGLE_240;
			player->kartstuff[k_itemclose] = 10;
		}
		else if (ATTACK_IS_DOWN && (player->kartstuff[k_tripleredshell] & 1 || player->kartstuff[k_tripleredshell] & 2 || player->kartstuff[k_tripleredshell] & 4)
			&& (player->kartstuff[k_throwdir] == 1 || player->kartstuff[k_throwdir] == 0))
		{
			P_ThrowKartItem(player, true, MT_REDITEM, 1, false);
			K_PlayTauntSound(player->mo);
			player->pflags |= PF_ATTACKDOWN;
			if (player->kartstuff[k_tripleredshell] & 4)
				player->kartstuff[k_tripleredshell] &= ~4;
			else if (player->kartstuff[k_tripleredshell] & 2)
				player->kartstuff[k_tripleredshell] &= ~2;
			else if (player->kartstuff[k_tripleredshell] & 1)
				player->kartstuff[k_tripleredshell] &= ~1;
		}
		else if (ATTACK_IS_DOWN && (player->kartstuff[k_tripleredshell] & 1 || player->kartstuff[k_tripleredshell] & 2 || player->kartstuff[k_tripleredshell] & 4)
		&& player->kartstuff[k_throwdir] == -1)
		{
			P_ThrowKartItem(player, true, MT_REDITEMDUD, -1, false);
			K_PlayTauntSound(player->mo);
			player->pflags |= PF_ATTACKDOWN;
			if (player->kartstuff[k_tripleredshell] & 4)
				player->kartstuff[k_tripleredshell] &= ~4;
			else if (player->kartstuff[k_tripleredshell] & 2)
				player->kartstuff[k_tripleredshell] &= ~2;
			else if (player->kartstuff[k_tripleredshell] & 1)
				player->kartstuff[k_tripleredshell] &= ~1;
		}
		// Banana Peel
		else if (ATTACK_IS_DOWN && !HOLDING_ITEM && player->kartstuff[k_banana] & 2 && NO_BOO)
		{
			angle_t newangle;
			fixed_t newx;
			fixed_t newy;
			mobj_t *mo;
			player->kartstuff[k_banana] &= ~2;
			player->kartstuff[k_banana] |= 1;
			player->pflags |= PF_ATTACKDOWN;
			newangle = player->mo->angle;
			newx = player->mo->x + P_ReturnThrustX(player->mo, newangle + ANGLE_180, 64*FRACUNIT);
			newy = player->mo->y + P_ReturnThrustY(player->mo, newangle + ANGLE_180, 64*FRACUNIT);
			mo = P_SpawnMobj(newx, newy, player->mo->z, MT_BANANASHIELD);
			mo->threshold = 10;
			if (mo)
				P_SetTarget(&mo->target, player->mo);
			player->kartstuff[k_itemclose] = 10;
		}
		else if (!(cmd->buttons & BT_ATTACK) && player->kartstuff[k_banana] & 1)
		{
			P_ThrowKartItem(player, false, MT_BANANAITEM, -1, false);
			K_PlayTauntSound(player->mo);
			player->kartstuff[k_banana] &= ~1;
		}
		// Triple Banana Peel
		else if (ATTACK_IS_DOWN && !HOLDING_ITEM && player->kartstuff[k_triplebanana] & 8 && NO_BOO)
		{
			angle_t newangle;
			fixed_t newx;
			fixed_t newy;
			mobj_t *mo, *mo2, *mo3;
			K_PlayTauntSound(player->mo);
			player->kartstuff[k_triplebanana] &= ~8;
			player->kartstuff[k_triplebanana] |= 7;
			player->pflags |= PF_ATTACKDOWN;
			newangle = player->mo->angle;
			newx = player->mo->x + P_ReturnThrustX(player->mo, newangle + ANGLE_180, 64*FRACUNIT);
			newy = player->mo->y + P_ReturnThrustY(player->mo, newangle + ANGLE_180, 64*FRACUNIT);
			mo = P_SpawnMobj(newx, newy, player->mo->z, MT_TRIPLEBANANASHIELD1);
			mo->threshold = 10;
			if (mo) {
				P_SetTarget(&mo->target, player->mo);
				mo->angle = 0; }
			mo2 = P_SpawnMobj(newx, newy, player->mo->z, MT_TRIPLEBANANASHIELD2);
			mo2->threshold = 10;
			if (mo2) {
				P_SetTarget(&mo2->target, player->mo);
				mo2->angle = ANGLE_135; }
			mo3 = P_SpawnMobj(newx, newy, player->mo->z, MT_TRIPLEBANANASHIELD3);
			mo3->threshold = 10;
			if (mo3) {
				P_SetTarget(&mo3->target, player->mo);
				mo3->angle = ANGLE_225; }
			player->kartstuff[k_itemclose] = 10;
		}
		else if (ATTACK_IS_DOWN && (player->kartstuff[k_triplebanana] & 1 || player->kartstuff[k_triplebanana] & 2 || player->kartstuff[k_triplebanana] & 4))
		{
			P_ThrowKartItem(player, false, MT_BANANAITEM, -1,false );
			K_PlayTauntSound(player->mo);
			player->pflags |= PF_ATTACKDOWN;
			if (player->kartstuff[k_triplebanana] & 4)
				player->kartstuff[k_triplebanana] &= ~4;
			else if (player->kartstuff[k_triplebanana] & 2)
				player->kartstuff[k_triplebanana] &= ~2;
			else if (player->kartstuff[k_triplebanana] & 1)
				player->kartstuff[k_triplebanana] &= ~1;
		}
		// Fake Itembox
		else if (ATTACK_IS_DOWN && !HOLDING_ITEM && player->kartstuff[k_fakeitem] & 2 && NO_BOO)
		{
			angle_t newangle;
			fixed_t newx;
			fixed_t newy;
			mobj_t *mo;
			player->kartstuff[k_fakeitem] &= ~2;
			player->kartstuff[k_fakeitem] |= 1;
			player->pflags |= PF_ATTACKDOWN;
			newangle = player->mo->angle;
			newx = player->mo->x + P_ReturnThrustX(player->mo, newangle + ANGLE_180, 64*FRACUNIT);
			newy = player->mo->y + P_ReturnThrustY(player->mo, newangle + ANGLE_180, 64*FRACUNIT);
			mo = P_SpawnMobj(newx, newy, player->mo->z, MT_FAKESHIELD);
			mo->scale = FRACUNIT/2;
			mo->threshold = 10;
			if (mo)
				P_SetTarget(&mo->target, player->mo);
			player->kartstuff[k_itemclose] = 10;
		}
		else if (!(cmd->buttons & BT_ATTACK) && player->kartstuff[k_fakeitem] & 1)
		{
			P_ThrowKartItem(player, false, MT_FAKEITEM, -1, false);
			K_PlayTauntSound(player->mo);
			player->kartstuff[k_fakeitem] &= ~1;
		}
		// Bomb
		else if (ATTACK_IS_DOWN && !HOLDING_ITEM && player->kartstuff[k_bobomb] & 2 && NO_BOO)
		{
			angle_t newangle;
			fixed_t newx;
			fixed_t newy;
			mobj_t *mo;
			player->kartstuff[k_bobomb] &= ~2;
			player->kartstuff[k_bobomb] |= 1;
			player->pflags |= PF_ATTACKDOWN;
			newangle = player->mo->angle;
			newx = player->mo->x + P_ReturnThrustX(player->mo, newangle + ANGLE_180, 64*FRACUNIT);
			newy = player->mo->y + P_ReturnThrustY(player->mo, newangle + ANGLE_180, 64*FRACUNIT);
			mo = P_SpawnMobj(newx, newy, player->mo->z, MT_BOMBSHIELD);
			mo->threshold = 10;
			if (mo)
				P_SetTarget(&mo->target, player->mo);
			player->kartstuff[k_itemclose] = 10;
		}
		if (!(cmd->buttons & BT_ATTACK) && player->kartstuff[k_bobomb] & 1)
		{
			P_ThrowKartItem(player, false, MT_BOMBITEM, 1, true);
			K_PlayTauntSound(player->mo);
			player->kartstuff[k_bobomb] &= ~1;
		}
		// Thunder
		else if (ATTACK_IS_DOWN && !HOLDING_ITEM && player->kartstuff[k_lightning] == 1 && NO_BOO)
		{
			K_DoLightning(player, false);
			player->kartstuff[k_lightning] = 0;
			player->kartstuff[k_itemclose] = 10;
		}
		// Blue Shell
		else if (ATTACK_IS_DOWN && !HOLDING_ITEM && player->kartstuff[k_blueshell] == 1 && NO_BOO)
		{
			K_DoLightning(player, true);
			player->kartstuff[k_blueshell] = 0;
			player->kartstuff[k_itemclose] = 10;
		}
		// Mega Mushroom
		else if (ATTACK_IS_DOWN && !HOLDING_ITEM && player->kartstuff[k_megashroom] == 1 && NO_BOO)
		{
			if (P_IsLocalPlayer(player) && !player->exiting)
				S_ChangeMusicInternal("mega", true);
			if (!P_IsLocalPlayer(player))
				S_StartSound(player->mo, sfx_mega);
			K_PlayTauntSound(player->mo);
			player->kartstuff[k_growshrinktimer] = itemtime;
			S_StartSound(player->mo, sfx_mario3);
			player->pflags |= PF_ATTACKDOWN;
			player->kartstuff[k_megashroom] = 0;
			player->kartstuff[k_itemclose] = 10;
		}
		// Boo
		else if (ATTACK_IS_DOWN && !HOLDING_ITEM && player->kartstuff[k_boo] == 1 && NO_BOO)
		{
			K_DoBooSteal(player);
			player->pflags |= PF_ATTACKDOWN;
			player->kartstuff[k_boo] = 0;
		}
		// Magnet
		else if (ATTACK_IS_DOWN && !HOLDING_ITEM && player->kartstuff[k_magnet] == 1 && NO_BOO)
		{
			K_DoMagnet(player);
			player->pflags |= PF_ATTACKDOWN;
			player->kartstuff[k_magnet] = 0;
		}

		// Mushroom Boost
		if (((player->kartstuff[k_mushroomtimer] > 0 && player->kartstuff[k_boosting] == 0)
			|| (player->kartstuff[k_mushroomtimer] > 0 && ATTACK_IS_DOWN && NO_BOO)) && onground)
		{
			player->kartstuff[k_boosting] = 1;
		}
		else if (player->kartstuff[k_mushroomtimer] == 0 && player->kartstuff[k_boosting] == 1)
			player->kartstuff[k_boosting] = 0;

		// Megashroom - Make the player grow!
		if (player->kartstuff[k_growshrinktimer] > (itemtime - 25))
		{
			if (leveltime & 2)
				player->mo->destscale = FRACUNIT*3/2;
			else
				player->mo->destscale = FRACUNIT;
		}
		else if (player->kartstuff[k_growshrinktimer] > 26
			&& player->kartstuff[k_growshrinktimer] <= (itemtime - 25))
			player->mo->destscale = FRACUNIT*3/2;
		// Megashroom - Back to normal...
		else if (player->kartstuff[k_growshrinktimer] > 1
			&& player->kartstuff[k_growshrinktimer] <= 26)
		{
			if (leveltime & 2)
				player->mo->destscale = FRACUNIT;
			else
				player->mo->destscale = FRACUNIT*3/2;
		}
		if (player->kartstuff[k_growshrinktimer] == 26)
			S_StartSound(player->mo, sfx_mario8);

		if (player->kartstuff[k_bootaketimer] > 0)
		{
			if ((player == &players[displayplayer] || (splitscreen && player == &players[secondarydisplayplayer]))
				|| (!(player == &players[displayplayer] || (splitscreen && player == &players[secondarydisplayplayer]))
				&& (player->kartstuff[k_bootaketimer] < 1*TICRATE/2 || player->kartstuff[k_bootaketimer] > bootime-(1*TICRATE/2))))
			{
				if (leveltime & 1)
					player->mo->flags2 |= MF2_DONTDRAW;
				else
					player->mo->flags2 &= ~MF2_DONTDRAW;
			}
			else
				player->mo->flags2 |= MF2_DONTDRAW;

			player->powers[pw_flashing] = player->kartstuff[k_bootaketimer]; // We'll do this for now, let's people know about the invisible people through subtle hints
		}
		else if (player->kartstuff[k_bootaketimer] == 0)
		{
			player->mo->flags2 &= ~MF2_DONTDRAW;
		}
	}

	if (player->kartstuff[k_growshrinktimer] > 1)
		player->powers[pw_flashing] = 2;

	// Friction
	if (player->speed > 0 && cmd->forwardmove == 0 && player->mo->friction == 59392)
	player->mo->friction += 4608;
	if (player->speed > 0 && cmd->forwardmove < 0 && player->mo->friction == 59392)
	player->mo->friction += 1608;

	// Splitscreen camera
	if (splitscreen && player == &players[consoleplayer])
		CV_SetValue(&cv_cam_dist, 190);
	if (splitscreen && player == &players[secondarydisplayplayer])
		CV_SetValue(&cv_cam2_dist, 190);

	K_KartDrift(player, onground);

	// Quick Turning
	// You can't turn your kart when you're not moving.
	// So now it's time to burn some rubber!
	if (player->speed < 2 && leveltime > 140 && cmd->buttons & BT_ACCELERATE && cmd->buttons & BT_BRAKE
	&& ((cmd->buttons & BT_DRIFTLEFT) || (cmd->buttons & BT_DRIFTRIGHT)))
	{
		if (leveltime % 20 == 0)
			S_StartSound(player->mo, sfx_mkslid);
	}

	// Squishing
	// If a Mega Mushroom or a Thwomp crushes you, get flattened instead of being killed.

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

	// Play the stop light's sounds
	if ((leveltime == (TICRATE-4)*2) || (leveltime == (TICRATE-2)*3))
		S_StartSound(NULL, sfx_lkt1);
	if (leveltime == (TICRATE)*4)
		S_StartSound(NULL, sfx_lkt2);
	// Start charging once you're given the opportunity.
	if (leveltime >= 70 && leveltime <= 140 && cmd->buttons & BT_ACCELERATE && leveltime % 5 == 0)
		player->kartstuff[k_boostcharge]++;
	if (leveltime >= 70 && leveltime <= 140 && !(cmd->buttons & BT_ACCELERATE))
		player->kartstuff[k_boostcharge] = 0;
	// Increase your size while charging your engine.
	if (leveltime < 150)
		player->mo->destscale = FRACUNIT + (player->kartstuff[k_boostcharge]*655);

	// Determine the outcome of your charge.
	if (leveltime > 140 && player->kartstuff[k_boostcharge])
	{
		// Get an instant boost!
		if (player->kartstuff[k_boostcharge] >= 7 && player->kartstuff[k_boostcharge] <= 10)
		{
			K_DoMushroom(player, false);
		}
		// You overcharged your engine? Those things are expensive!!!
		if (player->kartstuff[k_boostcharge] > 10)
		{
			player->powers[pw_nocontrol] = 40;
			S_StartSound(player->mo, sfx_slip);
		}

		player->kartstuff[k_boostcharge] = 0;
	}
}

//}

//{ SRB2kart HUD Code

#define NUMPOSNUMS 10
#define NUMPOSFRAMES 7 // White, three blues, three reds

//{ 	Patch Definitions
static patch_t *kp_nodraw;
static patch_t *kp_noitem;
static patch_t *kp_timesticker;
static patch_t *kp_timestickerwide;
static patch_t *kp_lapsticker;
static patch_t *kp_lapstickernarrow;
static patch_t *kp_positionnum[NUMPOSNUMS][NUMPOSFRAMES];
static patch_t *kp_facenull;
static patch_t *kp_facefirst;
static patch_t *kp_facesecond;
static patch_t *kp_facethird;
static patch_t *kp_facefourth;
static patch_t *kp_magnet;
static patch_t *kp_boo;
static patch_t *kp_boosteal;
static patch_t *kp_mushroom;
static patch_t *kp_doublemushroom;
static patch_t *kp_triplemushroom;
static patch_t *kp_megashroom;
static patch_t *kp_goldshroom;
static patch_t *kp_star;
static patch_t *kp_triplebanana;
static patch_t *kp_fakeitem;
static patch_t *kp_banana;
static patch_t *kp_greenshell;
static patch_t *kp_redshell;
static patch_t *kp_triplegreenshell;
static patch_t *kp_bobomb;
static patch_t *kp_blueshell;
static patch_t *kp_fireflower;
static patch_t *kp_tripleredshell;
static patch_t *kp_lightning;
static patch_t *kp_kitchensink;
static patch_t *kp_itemused1;
static patch_t *kp_itemused2;
static patch_t *kp_itemused3;
static patch_t *kp_itemused4;
static patch_t *kp_itemused5;
static patch_t *kp_singlebananaicon;
static patch_t *kp_doublebananaicon;
static patch_t *kp_triplebananaicon;
static patch_t *kp_singlegreenshellicon;
static patch_t *kp_doublegreenshellicon;
static patch_t *kp_triplegreenshellicon;
static patch_t *kp_singleredshellicon;
static patch_t *kp_doubleredshellicon;
static patch_t *kp_tripleredshellicon;
/*
static patch_t *kp_neonoitem;
static patch_t *kp_electroshield;
static patch_t *kp_skghost;
static patch_t *kp_skghoststeal;
static patch_t *kp_speedshoe;
static patch_t *kp_doublespeedshoe;
static patch_t *kp_triplespeedshoe;
static patch_t *kp_sizeupmonitor;
static patch_t *kp_rocketshoe;
static patch_t *kp_invincibility;
static patch_t *kp_neotriplebanana;
static patch_t *kp_eggmanmonitor;
static patch_t *kp_neobanana;
static patch_t *kp_orbinaut;
static patch_t *kp_jaws;
static patch_t *kp_tripleorbinaut;
static patch_t *kp_specialstagemine;
static patch_t *kp_deton;
static patch_t *kp_laserwisp;
static patch_t *kp_doublejaws;
static patch_t *kp_sizedownmonitor;
static patch_t *kp_neoitemused1;
static patch_t *kp_neoitemused2;
static patch_t *kp_neoitemused3;
static patch_t *kp_neoitemused4;
static patch_t *kp_neoitemused5;
*/

void K_LoadKartHUDGraphics(void)
{
	INT32 i, j;
	char buffer[9];

	// Null Stuff
	kp_nodraw = 				W_CachePatchName("K_TRNULL", PU_HUDGFX);
	kp_noitem = 				W_CachePatchName("K_ITNULL", PU_HUDGFX);
	//kp_neonoitem = 				W_CachePatchName("KNITNULL", PU_HUDGFX);

	// Stickers
	kp_timesticker = 			W_CachePatchName("K_STTIME", PU_HUDGFX);
	kp_timestickerwide = 		W_CachePatchName("K_STTIMW", PU_HUDGFX);
	kp_lapsticker = 			W_CachePatchName("K_STLAPS", PU_HUDGFX);
	kp_lapstickernarrow = 		W_CachePatchName("K_STLAPN", PU_HUDGFX);

	// Position numbers
	for (i = 0; i < NUMPOSNUMS; i++)
	{
		for (j = 0; j < NUMPOSFRAMES; j++)
		{
			if (i > 4 && j < 4 && j != 0) continue;	// We don't need blue numbers for ranks past 4th
			sprintf(buffer, "K_POSN%d%d", i, j);
			kp_positionnum[i][j] = (patch_t *) W_CachePatchName(buffer, PU_HUDGFX);
		}
	}
	kp_facenull = 				W_CachePatchName("K_PFACE0", PU_HUDGFX);
	kp_facefirst = 				W_CachePatchName("K_PFACE1", PU_HUDGFX);
	kp_facesecond = 			W_CachePatchName("K_PFACE2", PU_HUDGFX);
	kp_facethird = 				W_CachePatchName("K_PFACE3", PU_HUDGFX);
	kp_facefourth = 			W_CachePatchName("K_PFACE4", PU_HUDGFX);

	// Kart Item Windows
	kp_magnet = 				W_CachePatchName("K_ITMAGN", PU_HUDGFX);
	kp_boo = 					W_CachePatchName("K_ITBOO1", PU_HUDGFX);
	kp_boosteal =				W_CachePatchName("K_ITBOO2", PU_HUDGFX);
	kp_mushroom = 				W_CachePatchName("K_ITMUSH", PU_HUDGFX);
	kp_doublemushroom = 		W_CachePatchName("K_ITDOUB", PU_HUDGFX);
	kp_triplemushroom = 		W_CachePatchName("K_ITTRIP", PU_HUDGFX);
	kp_megashroom = 			W_CachePatchName("K_ITMEGA", PU_HUDGFX);
	kp_goldshroom = 			W_CachePatchName("K_ITGOLD", PU_HUDGFX);
	kp_star = 					W_CachePatchName("K_ITSTAR", PU_HUDGFX);
	kp_triplebanana = 			W_CachePatchName("K_ITTBAN", PU_HUDGFX);
	kp_fakeitem = 				W_CachePatchName("K_ITFAKE", PU_HUDGFX);
	kp_banana = 				W_CachePatchName("K_ITBANA", PU_HUDGFX);
	kp_greenshell = 			W_CachePatchName("K_ITGREE", PU_HUDGFX);
	kp_redshell = 				W_CachePatchName("K_ITREDS", PU_HUDGFX);
	kp_triplegreenshell = 		W_CachePatchName("K_ITTGRE", PU_HUDGFX);
	kp_bobomb = 				W_CachePatchName("K_ITBOBO", PU_HUDGFX);
	kp_blueshell = 				W_CachePatchName("K_ITBLUE", PU_HUDGFX);
	kp_fireflower = 			W_CachePatchName("K_ITFIRE", PU_HUDGFX);
	kp_tripleredshell = 		W_CachePatchName("K_ITTRED", PU_HUDGFX);
	kp_lightning = 				W_CachePatchName("K_ITLIGH", PU_HUDGFX);
	kp_kitchensink = 			W_CachePatchName("K_ITSINK", PU_HUDGFX);

	// Item-used - Closing the item window after an item is used
	kp_itemused1 = 				W_CachePatchName("K_ITUSE1", PU_HUDGFX);
	kp_itemused2 = 				W_CachePatchName("K_ITUSE2", PU_HUDGFX);
	kp_itemused3 = 				W_CachePatchName("K_ITUSE3", PU_HUDGFX);
	kp_itemused4 = 				W_CachePatchName("K_ITUSE4", PU_HUDGFX);
	kp_itemused5 = 				W_CachePatchName("K_ITUSE5", PU_HUDGFX);

	// Triple-item HUD icons
	kp_singlebananaicon = 		W_CachePatchName("K_TRBAN1", PU_HUDGFX);
	kp_doublebananaicon = 		W_CachePatchName("K_TRBAN2", PU_HUDGFX);
	kp_triplebananaicon = 		W_CachePatchName("K_TRBAN3", PU_HUDGFX);
	kp_singlegreenshellicon = 	W_CachePatchName("K_TRGRE1", PU_HUDGFX);
	kp_doublegreenshellicon = 	W_CachePatchName("K_TRGRE2", PU_HUDGFX);
	kp_triplegreenshellicon = 	W_CachePatchName("K_TRGRE3", PU_HUDGFX);
	kp_singleredshellicon = 	W_CachePatchName("K_TRRED1", PU_HUDGFX);
	kp_doubleredshellicon = 	W_CachePatchName("K_TRRED2", PU_HUDGFX);
	kp_tripleredshellicon = 	W_CachePatchName("K_TRRED3", PU_HUDGFX);

	/*
	// Neo-Kart item windows
	kp_electroshield = 			W_CachePatchName("KNITELEC", PU_HUDGFX);
	kp_skghost = 				W_CachePatchName("KTITSKG1", PU_HUDGFX);
	kp_skghoststeal = 			W_CachePatchName("KTITSKG2", PU_HUDGFX);
	kp_speedshoe = 				W_CachePatchName("KTITSPEE", PU_HUDGFX);
	kp_triplespeedshoe = 		W_CachePatchName("KTITTSPE", PU_HUDGFX);
	kp_sizeupmonitor = 			W_CachePatchName("KTITSUPM", PU_HUDGFX);
	kp_rocketshoe = 			W_CachePatchName("KTITROCK", PU_HUDGFX);
	kp_invincibility = 			W_CachePatchName("KTITINVI", PU_HUDGFX);
	kp_neotriplebanana = 		W_CachePatchName("KTITTBAN", PU_HUDGFX);
	kp_eggmanmonitor = 			W_CachePatchName("KTITEGGM", PU_HUDGFX);
	kp_neobanana = 				W_CachePatchName("KTITBANA", PU_HUDGFX);
	kp_orbinaut = 				W_CachePatchName("KTITORBI", PU_HUDGFX);
	kp_jaws = 					W_CachePatchName("KTITJAWS", PU_HUDGFX);
	kp_tripleorbinaut = 		W_CachePatchName("KTITTORB", PU_HUDGFX);
	kp_specialstagemine = 		W_CachePatchName("KTITSPEC", PU_HUDGFX);
	kp_deton = 					W_CachePatchName("KTITDETO", PU_HUDGFX);
	kp_laserwisp = 				W_CachePatchName("KTITLASE", PU_HUDGFX);
	kp_doublejaws = 			W_CachePatchName("KTITDJAW", PU_HUDGFX);
	kp_sizedownmonitor = 		W_CachePatchName("KTITSDOW", PU_HUDGFX);

	// Item-used - Closing the item window after an item is used (Neo-Kart)
	kp_neoitemused1 = 			W_CachePatchName("KNITUSE1", PU_HUDGFX);
	kp_neoitemused2 = 			W_CachePatchName("KNITUSE2", PU_HUDGFX);
	kp_neoitemused3 = 			W_CachePatchName("KNITUSE3", PU_HUDGFX);
	kp_neoitemused4 = 			W_CachePatchName("KNITUSE4", PU_HUDGFX);
	kp_neoitemused5 = 			W_CachePatchName("KNITUSE5", PU_HUDGFX);
	*/
}

//}

static INT32 STRINGY(INT32 y)
{
	// Copied from st_stuff.c
	if (splitscreen)
	{
		y >>= 1;
		if (stplyr != &players[displayplayer])
			y += BASEVIDHEIGHT / 2;
	}
	return y;
}

static INT32 SCX(INT32 x)
{
	return FixedInt(FixedMul(x<<FRACBITS, vid.fdupx));
}

INT32 ITEM_X, ITEM_Y;	// Item Window
INT32 TRIP_X, TRIP_Y;	// Triple Item Icon
INT32 TIME_X, TIME_Y;	// Time Sticker
INT32 LAPS_X, LAPS_Y;	// Lap Sticker
INT32 POSI_X, POSI_Y;	// Position Number
INT32 FACE_X, FACE_Y;	// Top-four Faces
INT32 METE_X, METE_Y;	// Speed Meter

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

	if (!splitscreen)						// Local Single-Player
	{
		switch (cv_karthud.value)		// Item Window
		{
			default:
				// Item Window
				ITEM_X = BASEVIDWIDTH - 52;	// 268
				ITEM_Y = 34;				//  34
				// Triple Item Object
				TRIP_X = 143;				// 143
				TRIP_Y = BASEVIDHEIGHT- 34;	// 166
				// Level Timer
				TIME_X = BASEVIDWIDTH -148;	// 172
				TIME_Y = 9;					//   9
				// Level Laps
				LAPS_X = 9;					//   9
				LAPS_Y = BASEVIDHEIGHT- 29;	// 171
				// Position Number
				POSI_X = BASEVIDWIDTH - 52;	// 268
				POSI_Y = BASEVIDHEIGHT- 62;	// 138
				// Top-Four Faces
				FACE_X = 9;					//   9
				FACE_Y = 92;				//  92
				break;
		}
	}
	else							// Local Multi-Player
	{
		ITEM_X = 9;					//   9
		ITEM_Y = 48;				//  48

		TRIP_X = 143;				// 143
		TRIP_Y = BASEVIDHEIGHT- 34;	// 166

		TIME_X = BASEVIDWIDTH -114;	// 206  / Sticker is 196 (Base - 124) - Inside the boundry by 8px
		TIME_Y = 6;					//   6  / Sticker is  +2

		LAPS_X = 9;					//   9
		LAPS_Y = BASEVIDHEIGHT- 31;	// 169

		POSI_X = BASEVIDWIDTH - 51;	// 269
		POSI_Y = BASEVIDHEIGHT-128;	//  72

		FACE_X = 15;				//  15
		FACE_Y = 72;				//  72
	}

	// To correct the weird render location
	POSI_X = SCX(POSI_X);
	POSI_Y = SCX(POSI_Y);
}

static void K_drawKartItemClose(void)
{
	// ITEM_X = BASEVIDWIDTH-50;	// 270
	// ITEM_Y = 24;					//  24

	// Why write V_DrawScaledPatch calls over and over when they're all the same?
	// Set to 'no draw' just in case.
	patch_t *localpatch = kp_nodraw;

	/*if ()
		switch (stplyr->kartstuff[k_itemclose])
		{
			case  1: localpatch = kp_neoitemused5; break;
			case  3: localpatch = kp_neoitemused4; break;
			case  5: localpatch = kp_neoitemused3; break;
			case  7: localpatch = kp_neoitemused2; break;
			case  9: localpatch = kp_neoitemused1; break;
			default: break;
		}
	else*/
		switch (stplyr->kartstuff[k_itemclose])
		{
			case  1: localpatch = kp_itemused5; break;
			case  3: localpatch = kp_itemused4; break;
			case  5: localpatch = kp_itemused3; break;
			case  7: localpatch = kp_itemused2; break;
			case  9: localpatch = kp_itemused1; break;
			default: break;
		}

	if (localpatch != kp_nodraw)
		V_DrawScaledPatch(ITEM_X, STRINGY(ITEM_Y), V_SNAPTORIGHT|V_SNAPTOTOP, localpatch);
}

static void K_drawKartItemRoulette(void)
{
	// ITEM_X = BASEVIDWIDTH-50;	// 270
	// ITEM_Y = 24;					//  24

	// Why write V_DrawScaledPatch calls over and over when they're all the same?
	// Set to 'no item' just in case.
	patch_t *localpatch = kp_nodraw;

	/*if ()
				switch(stplyr->kartstuff[k_itemroulette] % 53)
		{
			// Each case is handled in threes, to give three frames of in-game time to see the item on the roulette
			// I'm also skipping by threes for the power order as to what item shows on the roulette
			case  0: case  1: case  2: localpatch = kp_electroshield; break;	// Electro-Shield
			case  3: case  4: case  5: localpatch = kp_triplespeedshoe; break;	// Triple Speed Shoe
			case  6: case  7: case  8: localpatch = kp_invincibility; break;	// Invincibility
			case  9: case 10: case 11: localpatch = kp_neobanana; break;		// Banana
			case 12: case 13: case 14: localpatch = kp_tripleorbinaut; break;	// 3x Orbinaut
			case 15: case 16: case 17: localpatch = kp_laserwisp; break;		// Laser Wisp
			case 18: case 19: case 20: localpatch = kp_skghost; break;			// S3K Ghost
			case 21: case 22: case 23: localpatch = kp_sizeupmonitor; break;	// Size-Up Monitor
			case 24: case 25: case 26: localpatch = kp_neotriplebanana; break;	// Triple Banana
			case 27: case 28: case 29: localpatch = kp_orbinaut; break;			// 1x Orbinaut
			case 30: case 31: case 32: localpatch = kp_specialstagemine; break;	// Specialstage Mines
			case 33: case 34: case 35: localpatch = kp_doublejaws; break;		// 2x Jaws
			case 36: case 37: case 38: localpatch = kp_speedshoe; break;		// Speed Shoe
			case 39: case 40: case 41: localpatch = kp_rocketshoe; break;		// Rocket Shoe
			case 42: case 43: case 44: localpatch = kp_eggmanmonitor; break;	// Eggman Monitor
			case 45: case 46: case 47: localpatch = kp_jaws; break;				// 1x Jaws
			case 48: case 49: case 50: localpatch = kp_deton; break;			// Deton
			case 51: case 52: case 53: localpatch = kp_sizedownmonitor; break;	// Size-Down Monitor
			default: break;
		}
	else*/
		switch(stplyr->kartstuff[k_itemroulette] % 53)
		{
			// Each case is handled in threes, to give three frames of in-game time to see the item on the roulette
			// I'm also skipping by threes for the power order as to what item shows on the roulette
			case  0: case  1: case  2: localpatch = kp_magnet; break;			// Magnet
			case  3: case  4: case  5: localpatch = kp_triplemushroom; break;	// Triple Mushroom
			case  6: case  7: case  8: localpatch = kp_star; break;				// Star
			case  9: case 10: case 11: localpatch = kp_banana; break;			// Banana
			case 12: case 13: case 14: localpatch = kp_triplegreenshell; break;	// Triple Green Shell
			case 15: case 16: case 17: localpatch = kp_fireflower; break;		// Fire Flower
			case 18: case 19: case 20: localpatch = kp_boo; break;				// Boo
			case 21: case 22: case 23: localpatch = kp_megashroom; break;		// Mega Mushroom
			case 24: case 25: case 26: localpatch = kp_triplebanana; break;		// Triple Banana
			case 27: case 28: case 29: localpatch = kp_greenshell; break;		// Green Shell
			case 30: case 31: case 32: localpatch = kp_bobomb; break;			// Bob-omb
			case 33: case 34: case 35: localpatch = kp_tripleredshell; break;	// Triple Red Shell
			case 36: case 37: case 38: localpatch = kp_mushroom; break;			// Mushroom
			case 39: case 40: case 41: localpatch = kp_goldshroom; break;		// Gold Mushroom
			case 42: case 43: case 44: localpatch = kp_fakeitem; break;			// Fake Item
			case 45: case 46: case 47: localpatch = kp_redshell; break;			// Red Shell
			case 48: case 49: case 50: localpatch = kp_blueshell; break;		// Blue Shell
			case 51: case 52: case 53: localpatch = kp_lightning; break;		// Lightning
			default: break;
		}

	V_DrawScaledPatch(ITEM_X, STRINGY(ITEM_Y), V_SNAPTORIGHT|V_SNAPTOTOP, localpatch);
}

static void K_drawKartRetroItem(void)
{
	// ITEM_X = BASEVIDWIDTH-50;	// 270
	// ITEM_Y = 24;					//  24

	// Why write V_DrawScaledPatch calls over and over when they're all the same?
	// Set to 'no item' just in case.
	patch_t *localpatch = kp_nodraw;

	// I'm doing this a little weird and drawing mostly in reverse order
	// The only actual reason is to make triple/double/single mushrooms line up this way in the code below
	// This shouldn't have any actual baring over how it functions
	// Boo is first, because we're drawing it on top of the player's current item
	if 		((stplyr->kartstuff[k_bootaketimer] > 0
		|| stplyr->kartstuff[k_boostolentimer] > 0) && (leveltime & 2)) 	localpatch = kp_boosteal;
	else if (stplyr->kartstuff[k_boostolentimer] > 0 && !(leveltime & 2))	localpatch = kp_noitem;
	else if (stplyr->kartstuff[k_kitchensink] == 1)							localpatch = kp_kitchensink;
	else if (stplyr->kartstuff[k_lightning] == 1)							localpatch = kp_lightning;
	else if (stplyr->kartstuff[k_tripleredshell] & 8)						localpatch = kp_tripleredshell;
	else if (stplyr->kartstuff[k_fireflower] == 1)							localpatch = kp_fireflower;
	else if (stplyr->kartstuff[k_blueshell] == 1)							localpatch = kp_blueshell;
	else if (stplyr->kartstuff[k_bobomb] & 2)								localpatch = kp_bobomb;
	else if (stplyr->kartstuff[k_triplegreenshell] & 8)						localpatch = kp_triplegreenshell;
	else if (stplyr->kartstuff[k_redshell] & 2)								localpatch = kp_redshell;
	else if (stplyr->kartstuff[k_greenshell] & 2)							localpatch = kp_greenshell;
	else if (stplyr->kartstuff[k_banana] & 2)								localpatch = kp_banana;
	else if (stplyr->kartstuff[k_fakeitem] & 2)								localpatch = kp_fakeitem;
	else if (stplyr->kartstuff[k_triplebanana] & 8)							localpatch = kp_triplebanana;
	else if (stplyr->kartstuff[k_star] == 1)								localpatch = kp_star;
	else if (stplyr->kartstuff[k_goldshroom] == 1
		|| (stplyr->kartstuff[k_goldshroomtimer] > 1 && (leveltime & 1)))	localpatch = kp_goldshroom;
	else if (stplyr->kartstuff[k_goldshroomtimer] > 1 && !(leveltime & 1))	localpatch = kp_noitem;
	else if (stplyr->kartstuff[k_megashroom] == 1
		|| (stplyr->kartstuff[k_growshrinktimer] > 1 && (leveltime & 1)))	localpatch = kp_megashroom;
	else if (stplyr->kartstuff[k_growshrinktimer] > 1 && !(leveltime & 1))	localpatch = kp_noitem;
	else if (stplyr->kartstuff[k_mushroom] & 4)								localpatch = kp_triplemushroom;
	else if (stplyr->kartstuff[k_mushroom] & 2)								localpatch = kp_doublemushroom;
	else if (stplyr->kartstuff[k_mushroom] == 1)							localpatch = kp_mushroom;
	else if (stplyr->kartstuff[k_boo] == 1)									localpatch = kp_boo;
	else if (stplyr->kartstuff[k_magnet] == 1)								localpatch = kp_magnet;

	V_DrawScaledPatch(ITEM_X, STRINGY(ITEM_Y), V_SNAPTORIGHT|V_SNAPTOTOP, localpatch);
}

/*
static void K_drawKartNeoItem(void)
{
	// ITEM_X = BASEVIDWIDTH-50;	// 270
	// ITEM_Y = 24;					//  24

	// Why write V_DrawScaledPatch calls over and over when they're all the same?
	// Set to 'no item' just in case.
	patch_t *localpatch = kp_noitem;

	// I'm doing this a little weird and drawing mostly in reverse order
	// The only actual reason is to make triple/double/single mushrooms line up this way in the code below
	// This shouldn't have any actual baring over how it functions
	// Boo is first, because we're drawing it on top of the player's current item
	if 		((stplyr->kartstuff[k_bootaketimer] > 0
		|| stplyr->kartstuff[k_boostolentimer] > 0) && (leveltime & 2)) 	localpatch = kp_skghoststeal;
	else if (stplyr->kartstuff[k_boostolentimer] > 0 && !(leveltime & 2))	localpatch = kp_neonoitem;
	else if (stplyr->kartstuff[k_lightning] == 1)							localpatch = kp_sizedownmonitor;
	else if (stplyr->kartstuff[k_jaws] & 4)									localpatch = kp_doublejaws;
	else if (stplyr->kartstuff[k_laserwisp] == 1)							localpatch = kp_laserwisp;
	else if (stplyr->kartstuff[k_blueshell] == 1)							localpatch = kp_deton;
	else if (stplyr->kartstuff[k_bobomb] & 2)								localpatch = kp_specialstagemine;
	else if (stplyr->kartstuff[k_triplegreenshell] & 8)						localpatch = kp_tripleorbinaut;
	else if (stplyr->kartstuff[k_redshell] & 2)								localpatch = kp_jaws;
	else if (stplyr->kartstuff[k_greenshell] & 2)							localpatch = kp_orbinaut;
	else if (stplyr->kartstuff[k_banana] & 2)								localpatch = kp_neobanana;
	else if (stplyr->kartstuff[k_fakeitem] & 2)								localpatch = kp_eggmanmonitor;
	else if (stplyr->kartstuff[k_triplebanana] & 8)							localpatch = kp_neotriplebanana;
	else if (stplyr->kartstuff[k_star] == 1)								localpatch = kp_invincibility;
	else if (stplyr->kartstuff[k_goldshroom] == 1
		|| (stplyr->kartstuff[k_goldshroomtimer] > 1 && (leveltime & 1)))	localpatch = kp_rocketshoe;
	else if (stplyr->kartstuff[k_goldshroomtimer] > 1 && !(leveltime & 1))	localpatch = kp_neonoitem;
	else if (stplyr->kartstuff[k_megashroom] == 1
		|| (stplyr->kartstuff[k_growshrinktimer] > 1 && (leveltime & 1)))	localpatch = kp_sizeupmonitor;
	else if (stplyr->kartstuff[k_growshrinktimer] > 1 && !(leveltime & 1))	localpatch = kp_neonoitem;
	else if (stplyr->kartstuff[k_mushroom] & 4)								localpatch = kp_triplespeedshoe;
	else if (stplyr->kartstuff[k_mushroom] & 2)								localpatch = kp_doublespeedshoe;
	else if (stplyr->kartstuff[k_mushroom] == 1)							localpatch = kp_speedshoe;
	else if (stplyr->kartstuff[k_boo] & 8)									localpatch = kp_skghost;
	else if (stplyr->kartstuff[k_magnet] & 8)								localpatch = kp_electroshield;

	V_DrawScaledPatch(ITEM_X, STRINGY(ITEM_Y), V_SNAPTORIGHT|V_TRANSLUCENT, localpatch);
}
*/

static void K_DrawKartTripleItem(void)
{
	// TRIP_X = 143;				// 143
	// TRIP_Y = BASEVIDHEIGHT-34;	// 166

	// Why write V_DrawScaledPatch calls over and over when they're all the same?
	// Set to 'no draw' just in case.
	patch_t *localpatch = kp_nodraw;
	INT32 thisitem;

	/*if ()
	{
		thisitem = stplyr->kartstuff[k_triplebanana];
		if 		(thisitem & 1) localpatch = kp_singleneobananaicon;
		else if (thisitem & 2) localpatch = kp_doubleneobananaicon;
		else if (thisitem & 4) localpatch = kp_tripleneobananaicon;

		thisitem = stplyr->kartstuff[k_triplegreenshell];
		if 		(thisitem & 1) localpatch = kp_singleorbitauricon;
		else if (thisitem & 2) localpatch = kp_doubleorbitauricon;
		else if (thisitem & 4) localpatch = kp_tripleorbitauricon;

		thisitem = stplyr->kartstuff[k_jaws];
		if 		(thisitem & 1) localpatch = kp_singlejawsicon;
		else if (thisitem & 2) localpatch = kp_doublejawsicon;
	}
	else*/
	//{
		thisitem = stplyr->kartstuff[k_triplebanana];
		if 		(thisitem & 1) localpatch = kp_singlebananaicon;
		else if (thisitem & 2) localpatch = kp_doublebananaicon;
		else if (thisitem & 4) localpatch = kp_triplebananaicon;

		thisitem = stplyr->kartstuff[k_triplegreenshell];
		if 		(thisitem & 1) localpatch = kp_singlegreenshellicon;
		else if (thisitem & 2) localpatch = kp_doublegreenshellicon;
		else if (thisitem & 4) localpatch = kp_triplegreenshellicon;

		thisitem = stplyr->kartstuff[k_tripleredshell];
		if 		(thisitem & 1) localpatch = kp_singleredshellicon;
		else if (thisitem & 2) localpatch = kp_doubleredshellicon;
		else if (thisitem & 4) localpatch = kp_tripleredshellicon;
	//}

	if (localpatch != kp_nodraw)
		V_DrawScaledPatch(TRIP_X, STRINGY(TRIP_Y), V_SNAPTORIGHT|V_TRANSLUCENT, localpatch);
}

static void K_drawKartTimestamp(void)
{
	// TIME_X = BASEVIDWIDTH-124;	// 196
	// TIME_Y = 6;					//   6

	INT32 TIME_XB;

	V_DrawScaledPatch(TIME_X, STRINGY(TIME_Y), 0, kp_timestickerwide);

	TIME_XB = TIME_X+33;

	if (stplyr->realtime/(60*TICRATE) < 100) // 99:99:99 only
	{
		// zero minute
		if (stplyr->realtime/(60*TICRATE) < 10)
		{
			V_DrawKartString(TIME_XB, STRINGY(TIME_Y+3), 0, va("0"));
			// minutes time       0 __ __
			V_DrawKartString(TIME_XB+12, STRINGY(TIME_Y+3), 0, va("%d", stplyr->realtime/(60*TICRATE)));
		}
		// minutes time       0 __ __
		else
			V_DrawKartString(TIME_XB, STRINGY(TIME_Y+3), 0, va("%d", stplyr->realtime/(60*TICRATE)));

		// apostrophe location     _'__ __
		V_DrawKartString(TIME_XB+24, STRINGY(TIME_Y+3), 0, va("'"));

		// zero second        _ 0_ __
		if ((stplyr->realtime/TICRATE % 60) < 10)
		{
			V_DrawKartString(TIME_XB+36, STRINGY(TIME_Y+3), 0, va("0"));
		// seconds time       _ _0 __
			V_DrawKartString(TIME_XB+48, STRINGY(TIME_Y+3), 0, va("%d", stplyr->realtime/TICRATE % 60));
		}
		// zero second        _ 00 __
		else
			V_DrawKartString(TIME_XB+36, STRINGY(TIME_Y+3), 0, va("%d", stplyr->realtime/TICRATE % 60));

		// quotation mark location    _ __"__
		V_DrawKartString(TIME_XB+60, STRINGY(TIME_Y+3), 0, va("\""));

		// zero tick          _ __ 0_
		if (G_TicsToCentiseconds(stplyr->realtime) < 10)
		{
			V_DrawKartString(TIME_XB+72, STRINGY(TIME_Y+3), 0, va("0"));
		// tics               _ __ _0
			V_DrawKartString(TIME_XB+84, STRINGY(TIME_Y+3), 0, va("%d", G_TicsToCentiseconds(stplyr->realtime)));
		}
		// zero tick          _ __ 00
		if (G_TicsToCentiseconds(stplyr->realtime) >= 10)
			V_DrawKartString(TIME_XB+72, STRINGY(TIME_Y+3), 0, va("%d", G_TicsToCentiseconds(stplyr->realtime)));
	}
	else
		V_DrawKartString(TIME_XB, STRINGY(TIME_Y+3), 0, va("99'59\"99"));
}

static void K_DrawKartPositionNum(INT32 num)
{
	// POSI_X = BASEVIDWIDTH - 51;	// 269
	// POSI_Y = BASEVIDHEIGHT- 64;	// 136

	INT32 X = POSI_X+SCX(43); // +43 to offset where it's being drawn if there are more than one
	INT32 W = SHORT(kp_positionnum[0][0]->width);
	patch_t *localpatch = kp_positionnum[0][0];

	// Special case for 0
	if (!num)
	{
		V_DrawTranslucentPatch(X-(W*vid.dupx), POSI_Y, V_NOSCALESTART|V_TRANSLUCENT|V_SNAPTORIGHT|V_SNAPTOBOTTOM, kp_positionnum[0][0]);
		return;
	}

	I_Assert(num >= 0); // This function does not draw negative numbers

	// Draw the number
	while (num)
	{
		X -= (W*vid.dupx);

		// Check for the final lap
		if (stplyr->laps+1 == cv_numlaps.value)
		{
			// Alternate frame every three frames
			switch (leveltime % 9)
			{
				case 1: case 2: case 3:
					if (stplyr->kartstuff[k_position] >= 4)
						localpatch = kp_positionnum[num % 10][4];
					else
						localpatch = kp_positionnum[num % 10][1];
					break;
				case 4: case 5: case 6:
					if (stplyr->kartstuff[k_position] >= 4)
						localpatch = kp_positionnum[num % 10][5];
					else
						localpatch = kp_positionnum[num % 10][2];
					break;
				case 7: case 8: case 9:
					if (stplyr->kartstuff[k_position] >= 4)
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

		V_DrawTranslucentPatch(X, POSI_Y, V_NOSCALESTART|V_TRANSLUCENT|V_SNAPTORIGHT|V_SNAPTOBOTTOM, localpatch);
		num /= 10;
	}
}

static void K_DrawKartPositionFaces(void)
{
	// FACE_X = 15;				//  15
	// FACE_Y = 72;				//  72

	INT32 Y = FACE_Y+9; // +9 to offset where it's being drawn if there are more than one
	INT32 i, j, ranklines;
	boolean completed[MAXPLAYERS];
	INT32 rankplayer[MAXPLAYERS];
	INT32 rankcolor[MAXPLAYERS];
	INT32 myplayer;
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
		if (!playeringame[j])
			continue;

		for (i = 0; i < MAXPLAYERS; i++)
		{
			if (playeringame[i] && completed[i] == false
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

		if (rankcolor[i] == 0)
		{
			colormap = colormaps;
			if (rankplayer[i] != myplayer)
				V_DrawSmallTranslucentPatch(FACE_X, Y, 0, faceprefix[players[rankplayer[i]].skin]);
			else
				V_DrawSmallScaledPatch(FACE_X, Y, 0, faceprefix[players[rankplayer[i]].skin]);
		}
		else
		{
			colormap = R_GetTranslationColormap(players[rankplayer[i]].skin, players[rankplayer[i]].mo->color, GTC_CACHE);
			if (rankplayer[i] != myplayer)
				V_DrawSmallTranslucentMappedPatch(FACE_X, Y, 0, faceprefix[players[rankplayer[i]].skin], colormap);
			else
				V_DrawSmallMappedPatch(FACE_X, Y, 0, faceprefix[players[rankplayer[i]].skin], colormap);
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
			V_DrawSmallTranslucentPatch(FACE_X, Y, 0, localpatch);
		else
			V_DrawSmallScaledPatch(FACE_X, Y, 0, localpatch);
		Y += 18;
	}
}

static void K_drawKartSpeedometer(void)
{
	fixed_t convSpeed;
	if (cv_speedometer.value == 1)
	{
		convSpeed = FixedMul(stplyr->speed, 142371)/FRACUNIT; // 2.172409058
		V_DrawKartString(9, STRINGY(155), 0, va("%3d km/h", convSpeed));
	}
	else if (cv_speedometer.value == 2)
	{
		convSpeed = FixedMul(stplyr->speed, 88465)/FRACUNIT; // 1.349868774
		V_DrawKartString(9, STRINGY(155), 0, va("%3d mph", convSpeed));
	}
	else if (cv_speedometer.value == 3)
	{
		convSpeed = stplyr->speed/FRACUNIT;
		V_DrawKartString(9, STRINGY(155), 0, va("%3d Fracunits/s", convSpeed));
	}
}

void K_drawKartHUD(void)
{
	// Define the X and Y for each drawn object
	// This is handled by console/menu values
	K_initKartHUD();

	// If the item window is closing, draw it closing!
	if (stplyr->kartstuff[k_itemclose])
		K_drawKartItemClose();

	// If the item-roulette is going, draw that
	// Otherwise, draw the item window normally (separated for retro/neo, to prevent this block from becoming a mess
	if (stplyr->kartstuff[k_itemroulette])
		K_drawKartItemRoulette();
	// else if ()
	//	K_drawKartNeoItem();
	else
		K_drawKartRetroItem();

	// Draw the little triple-item icons at the bottom
	if (!splitscreen)
	{
		K_DrawKartTripleItem();
		K_DrawKartPositionFaces();
	}

	// Draw the timestamp
	K_drawKartTimestamp();

	// Draw the lap counter
	V_DrawScaledPatch(LAPS_X, STRINGY(LAPS_Y), 0, kp_lapsticker);
	if (stplyr->exiting)
		V_DrawKartString(LAPS_X+33, STRINGY(LAPS_Y+3), 0, "FIN");
	else
		V_DrawKartString(LAPS_X+33, STRINGY(LAPS_Y+3), 0, va("%d/%d", stplyr->laps+1, cv_numlaps.value));

	// Draw the numerical position
	K_DrawKartPositionNum(stplyr->kartstuff[k_position]);

	K_drawKartSpeedometer();
}

//}






// SONIC ROBO BLAST 2 KART ~ ZarroTsu
//-----------------------------------------------------------------------------
/// \file  k_kart.c
/// \brief SRB2kart general.
///        All of the SRB2kart-unique stuff.

#include "doomdef.h"
#include "r_draw.h"
#include "r_local.h"

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


//{ P_KartPlayerThink - for p_user.c
	
/**	\brief	Decreases various kart timers and powers per frame. Called in P_PlayerThink in p_user.c

	\param	player	player object passed from P_PlayerThink
	\param	cmd		control input from player

	\return	void
*/
void P_KartPlayerThink(player_t *player, ticcmd_t *cmd)
{
	// This spawns the drift sparks when k_driftcharge hits 30. Its own AI handles life/death and color
	//if ((player->kartstuff[k_drift] == 1 || player->kartstuff[k_drift] == -1) 
	//	&& player->kartstuff[k_driftcharge] == 30)
	//	P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_DRIFT)->target = player->mo;
	
	if (player->kartstuff[k_itemclose])
		player->kartstuff[k_itemclose]--;
	
	if (player->kartstuff[k_spinout])
		player->kartstuff[k_spinout]--;
	
	if (player->kartstuff[k_spinouttimer])
		player->kartstuff[k_spinouttimer]--;
	
	if (player->kartstuff[k_magnettimer])
		player->kartstuff[k_magnettimer]--;
	
	if (player->kartstuff[k_mushroomtimer])
		player->kartstuff[k_mushroomtimer]--;
	
	if (player->kartstuff[k_startimer])
		player->kartstuff[k_startimer]--;
	
	if (player->kartstuff[k_growshrinktimer] > 0)
		player->kartstuff[k_growshrinktimer]--;
	
	if (player->kartstuff[k_growshrinktimer] < 0)
		player->kartstuff[k_growshrinktimer]++;
	
	if (player->kartstuff[k_growshrinktimer] == 1 || player->kartstuff[k_growshrinktimer] == -1)
	{
		player->mo->destscale = 100;
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
	
	// Restores music if too many sounds are playing (?)
	if (player->kartstuff[k_sounds] >= 1 && player->kartstuff[k_sounds] < 120)
		player->kartstuff[k_sounds] += 1;
	if ((player->kartstuff[k_sounds] < 120 && player->kartstuff[k_sounds] > 116) 
		&& P_IsLocalPlayer(player))
		P_RestoreMusic(player);
	
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
		
	P_KartItemRoulette(player); // Roulette Code
	
	// Looping and stopping of the horrible horrible star SFX ~Sryder
	if (player->mo->health > 0 && player->mo->player->kartstuff[k_startimer])// If you have invincibility
	{
		if (!P_IsLocalPlayer(player)) // If it isn't the current player
		{
			if (!S_SoundPlaying(NULL, sfx_star)) // and it isn't still playing
				S_StartSound(player->mo, sfx_star); // play it again
		}
	}
	else if (player->mo->health <= 0 || player->mo->player->kartstuff[k_startimer] <= 0 || player->mo->player->kartstuff[k_growshrinktimer] > 0) // If you don't have invincibility (or mega is active too)
	{
		if (S_SoundPlaying(player->mo, sfx_star)) // But the sound is playing
			S_StopSoundByID(player->mo, sfx_star); // Stop it
	}

	// And now the same for the mega mushroom SFX ~Sryder
	if (player->mo->health > 0 && player->mo->player->kartstuff[k_growshrinktimer] > 0) // If you are big
	{
		if (!P_IsLocalPlayer(player)) // If it isn't the current player
		{
			if (!S_SoundPlaying(NULL, sfx_mega)) // and it isn't still playing
				S_StartSound(player->mo, sfx_mega); // play it again
		}
	}
	else // If you aren't big
	{
		if (S_SoundPlaying(player->mo, sfx_mega)) // But the sound is playing
			S_StopSoundByID(player->mo, sfx_mega); // Stop it
	}
}

//}








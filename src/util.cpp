
/**
 *
 * @file util.cpp
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 23rd August 2005: Created main.c
 * 22nd July 2008: Created util.c from parts of main.c
 * 3rd February 2009: Renamed util.c to util.cpp
 * 3rd February 2009: Created file.cpp from parts of util.cpp
 * 4th February 2009: Created palette.cpp from parts of main.cpp and util.cpp
 * 13th July 2009: Created graphics.cpp from parts of util.cpp
 *
 * @section Licence
 * Copyright (c) 2005-2012 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * @section Description
 * Contains core utility functions.
 *
 */

#include "OpenJazz.h"
#include "util.h"

#include "io/file.h"

#include <string.h>
#ifdef CASIO
	#include <fxcg/file.h>
	#include "platforms/casio.h"
	#include <alloca.h>
#endif
const fixed sinLut[]={0,6,12,18,25,31,37,43,50,56,62,69,75,81,87,94,100,106,112,119,125,131,137,144,150,156,162,168,175,181,187,193,199,205,212,218,224,230,236,242,248,254,260,267,273,279,285,291,297,303,309,315,321,327,333,339,344,350,356,362,368,374,380,386,391,397,403,409,414,420,426,432,437,443,449,454,460,466,471,477,482,488,493,499,504,510,515,521,526,531,537,542,547,553,558,563,568,574,579,584,589,594,599,604,609,615,620,625,629,634,639,644,649,654,659,664,668,673,678,683,687,692,696,701,706,710,715,719,724,728,732,737,741,745,750,754,758,762,767,771,775,779,783,787,791,795,799,803,807,811,814,818,822,826,829,833,837,840,844,847,851,854,858,861,865,868,871,875,878,881,884,887,890,894,897,900,903,906,908,911,914,917,920,922,925,928,930,933,936,938,941,943,946,948,950,953,955,957,959,962,964,966,968,970,972,974,976,978,979,981,983,985,986,988,990,991,993,994,996,997,999,1000,1001,1003,1004,1005,1006,1007,1008,1009,1010,1011,1012,1013,1014,1015,1016,1017,1017,1018,1019,1019,1020,1020,1021,1021,1022,1022,1022,1023,1023,1023,1023,1023,1023,1023,1024,1023,1023,1023,1023,1023,1023,1023,1022,1022,1022,1021,1021,1020,1020,1019,1019,1018,1017,1017,1016,1015,1014,1013,1012,1011,1010,1009,1008,1007,1006,1005,1004,1003,1001,1000,999,997,996,994,993,991,990,988,986,985,983,981,979,978,976,974,972,970,968,966,964,962,959,957,955,953,950,948,946,943,941,938,936,933,930,928,925,922,920,917,914,911,908,906,903,900,897,894,890,887,884,881,878,875,871,868,865,861,858,854,851,847,844,840,837,833,829,826,822,818,814,811,807,803,799,795,791,787,783,779,775,771,767,762,758,754,750,745,741,737,732,728,724,719,715,710,706,701,696,692,687,683,678,673,668,664,659,654,649,644,639,634,629,625,620,615,609,604,599,594,589,584,579,574,568,563,558,553,547,542,537,531,526,521,515,510,504,499,493,488,482,477,471,466,460,454,449,443,437,432,426,420,414,409,403,397,391,386,380,374,368,362,356,350,344,339,333,327,321,315,309,303,297,291,285,279,273,267,260,254,248,242,236,230,224,218,212,205,199,193,187,181,175,168,162,156,150,144,137,131,125,119,112,106,100,94,87,81,75,69,62,56,50,43,37,31,25,18,12,6,0,-6,-12,-18,-25,-31,-37,-43,-50,-56,-62,-69,-75,-81,-87,-94,-100,-106,-112,-119,-125,-131,-137,-144,-150,-156,-162,-168,-175,-181,-187,-193,-199,-205,-212,-218,-224,-230,-236,-242,-248,-254,-260,-267,-273,-279,-285,-291,-297,-303,-309,-315,-321,-327,-333,-339,-344,-350,-356,-362,-368,-374,-380,-386,-391,-397,-403,-409,-414,-420,-426,-432,-437,-443,-449,-454,-460,-466,-471,-477,-482,-488,-493,-499,-504,-510,-515,-521,-526,-531,-537,-542,-547,-553,-558,-563,-568,-574,-579,-584,-589,-594,-599,-604,-609,-615,-620,-625,-629,-634,-639,-644,-649,-654,-659,-664,-668,-673,-678,-683,-687,-692,-696,-701,-706,-710,-715,-719,-724,-728,-732,-737,-741,-745,-750,-754,-758,-762,-767,-771,-775,-779,-783,-787,-791,-795,-799,-803,-807,-811,-814,-818,-822,-826,-829,-833,-837,-840,-844,-847,-851,-854,-858,-861,-865,-868,-871,-875,-878,-881,-884,-887,-890,-894,-897,-900,-903,-906,-908,-911,-914,-917,-920,-922,-925,-928,-930,-933,-936,-938,-941,-943,-946,-948,-950,-953,-955,-957,-959,-962,-964,-966,-968,-970,-972,-974,-976,-978,-979,-981,-983,-985,-986,-988,-990,-991,-993,-994,-996,-997,-999,-1000,-1001,-1003,-1004,-1005,-1006,-1007,-1008,-1009,-1010,-1011,-1012,-1013,-1014,-1015,-1016,-1017,-1017,-1018,-1019,-1019,-1020,-1020,-1021,-1021,-1022,-1022,-1022,-1023,-1023,-1023,-1023,-1023,-1023,-1023,-1024,-1023,-1023,-1023,-1023,-1023,-1023,-1023,-1022,-1022,-1022,-1021,-1021,-1020,-1020,-1019,-1019,-1018,-1017,-1017,-1016,-1015,-1014,-1013,-1012,-1011,-1010,-1009,-1008,-1007,-1006,-1005,-1004,-1003,-1001,-1000,-999,-997,-996,-994,-993,-991,-990,-988,-986,-985,-983,-981,-979,-978,-976,-974,-972,-970,-968,-966,-964,-962,-959,-957,-955,-953,-950,-948,-946,-943,-941,-938,-936,-933,-930,-928,-925,-922,-920,-917,-914,-911,-908,-906,-903,-900,-897,-894,-890,-887,-884,-881,-878,-875,-871,-868,-865,-861,-858,-854,-851,-847,-844,-840,-837,-833,-829,-826,-822,-818,-814,-811,-807,-803,-799,-795,-791,-787,-783,-779,-775,-771,-767,-762,-758,-754,-750,-745,-741,-737,-732,-728,-724,-719,-715,-710,-706,-701,-696,-692,-687,-683,-678,-673,-668,-664,-659,-654,-649,-644,-639,-634,-629,-625,-620,-615,-609,-604,-599,-594,-589,-584,-579,-574,-568,-563,-558,-553,-547,-542,-537,-531,-526,-521,-515,-510,-504,-499,-493,-488,-482,-477,-471,-466,-460,-454,-449,-443,-437,-432,-426,-420,-414,-409,-403,-397,-391,-386,-380,-374,-368,-362,-356,-350,-344,-339,-333,-327,-321,-315,-309,-303,-297,-291,-285,-279,-273,-267,-260,-254,-248,-242,-236,-230,-224,-218,-212,-205,-199,-193,-187,-181,-175,-168,-162,-156,-150,-144,-137,-131,-125,-119,-112,-106,-100,-94,-87,-81,-75,-69,-62,-56,-50,-43,-37,-31,-25,-18,-12,-6};
static inline unsigned int sq(unsigned int x){
	return x*x;
}

unsigned char nearestIndex(unsigned char ri,unsigned char gi,unsigned char bi,unsigned char * pal,unsigned short amt){
	unsigned short c;
	unsigned char bestindex=0;
	unsigned int min=(63*63) * 3 + 1;
	amt *= 3;
	for(c=0;c<amt;c += 3){
		unsigned char ro = pal[c];
		unsigned char go = pal[c + 1];
		unsigned char bo = pal[c + 2];
		unsigned int dist=sq(ro-ri)+sq(go-gi)+sq(bo-bi);
		if(dist<=min){
			min=dist;
			bestindex=c / 3;
		}
	}
	return bestindex;
}
/**
 * Check if a file exists.
 *
 * @param fileName The file to check
 *
 * @return Whether or not the file exists
 */
bool fileExists (const char * fileName) {
	File *file;
#ifdef VERBOSE
	printf("Check: ");
#endif
	try {
		file = new File(fileName, false);
	} catch (int e) {
		return false;
	}
	delete file;
	return true;
}


/**
 * Create a short based on the little-endian contents of the first two bytes in
 * the given memory location.
 *
 * @param data Pointer to the memory location
 *
 * @return The generated short
 */
unsigned short int createShort (unsigned char* data) {

	unsigned short int val;
	#ifdef CASIO
		val=(data[0]<<8)|(data[1]);
	#else
		val = data[0]|(data[1] << 8);
	#endif
	return val;
}


/**
 * Create an int based on the little-endian contents of the first two bytes in
 * the given memory location.
 *
 * @param data Pointer to the memory location
 *
 * @return The generated int
 */
int createInt (unsigned char* data) {
	unsigned int val;
	#ifdef CASIO
		val=(data[0]<<24)|(data[1]<<16)|(data[2]<<8)|(data[3]);
	#else
		val = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
	#endif
	return *((int *)&val);
}


/**
 * Create a new string from the contents of an existing string.
 *
 * @param string The existing string
 *
 * @return The new string
 */
char * createString (const char *string) {

	char *cloned;

	cloned = new char[strlen(string) + 1];
	strcpy(cloned, string);

	return cloned;

}


/**
 * Create a new string from the concatenation of two existing strings.
 *
 * @param first The existing string to form the start of the new string
 * @param second The exisitng string to form the end of the new string
 *
 * @return The new string
 */
char * createString (const char *first, const char *second) {

	char *concatenated;

	concatenated = new char[strlen(first) + strlen(second)+1];
	strcpy(concatenated, first);
	strcat(concatenated, second);

	return concatenated;

}


/**
 * Create a new file name string with a 3-digit numerical extension.
 *
 * @param type The pre-dot file name
 * @param extension The number to constitute the extension
 *
 * @return The new file name string
 */
char * createFileName (const char *type, int extension) {

	char *fileName;
	int pos;

	pos = strlen(type);
	fileName = new char[pos + 5];
	strcpy(fileName, type);
	fileName[pos++] = '.';
	fileName[pos++] = '0' + ((extension / 100) % 10);
	fileName[pos++] = '0' + ((extension / 10) % 10);
	fileName[pos++] = '0' + (extension % 10);
	fileName[pos] = 0;

	return fileName;

}


/**
 * Create a new file name string with the given extension.
 *
 * @param type The pre-dot file name
 * @param extension The extension
 *
 * @return The new file name string
 */
char * createFileName (const char *type, const char *extension) {

	char *fileName;
	int pos;

	pos = strlen(type);
	fileName = new char[pos + strlen(extension) + 2];
	strcpy(fileName, type);
	fileName[pos++] = '.';
	strcpy(fileName + pos, extension);

	return fileName;

}


/**
 * Create a new file name string with a 1-digit numerical suffix and a 3-digit
 * numerical extension.
 *
 * @param type The pre-dot file name
 * @param level The number to constitute the suffix
 * @param extension The number to constitute the extension
 *
 * @return The new file name string
 */
char * createFileName (const char *type, int level, int extension) {

	char *fileName;
	int pos;

	pos = strlen(type);
	fileName = new char[pos + 6];
	strcpy(fileName, type);
	fileName[pos++] = '0' + (level % 10);
	fileName[pos++] = '.';
	fileName[pos++] = '0' + ((extension / 100) % 10);
	fileName[pos++] = '0' + ((extension / 10) % 10);
	fileName[pos++] = '0' + (extension % 10);
	fileName[pos] = 0;

	return fileName;

}


/**
 * Create a new variable-length string from the contents of an existing string.
 *
 * @param string The existing string
 *
 * @return The new string
 */
char * createEditableString (const char *string) {

	char *cloned;

	cloned = new char[STRING_LENGTH + 1];
	strcpy(cloned, string);

	return cloned;

}


/**
 * Add a message to the log.
 *
 * @param message The log message
 */
void log (const char *message) {

	printf("%s\n", message);

	return;

}

#ifndef CASIO
/**
 * Add a message with a detail message to the log.
 *
 * @param message The log message
 * @param detail The detail message
 */
void log (const char *message, const char *detail) {

	printf("%s: %s\n", message, detail);

	return;

}


/**
 * Add a message with a detail number to the log.
 *
 * @param message The log message
 * @param number The detail number
 */
void log (const char *message, int number) {

	printf("%s: %d\n", message, number);

	return;

}


/**
 * Add a message with a detail message to the error log.
 *
 * @param message The log message
 * @param detail The detail message
 */
void logError (const char *message, const char *detail) {

	fprintf(stderr, "%s: %s\n", message, detail);

	return;

}
#endif

/**
 * Get the sine of the given angle
 *
 * @param angle The given angle (where 1024 represents a full circle)
 *
 * @return The sine of the angle
 */
/*fixed fSin (fixed angle) {

	return sinLut[angle & 1023];

}*/


/**
 * Get the cosine of the given angle
 *
 * @param angle The given angle (where 1024 represetns a full circle)
 *
 * @return The cosine of the angle
 */
/*fixed fCos (fixed angle) {

	return sinLut[(angle + 256) & 1023];

}*/


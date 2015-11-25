
/**
 *
 * @file jj1levelload.cpp
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 23rd August 2005: Created level.c
 * 22nd July 2008: Created levelload.c from parts of level.c
 * 3rd February 2009: Renamed levelload.c to levelload.cpp
 * 18th July 2009: Created demolevel.cpp from parts of level.cpp and
 *                 levelload.cpp
 * 19th July 2009: Added parts of levelload.cpp to level.cpp
 * 28th June 2010: Created levelloadjj2.cpp from parts of levelload.cpp
 * 1st August 2012: Renamed levelload.cpp to jj1levelload.cpp
 *
 * @section Licence
 * Copyright (c) 2005-2013 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * @section Description
 * Deals with the loading of level data.
 *
 */


#include "jj1bullet.h"
#include "jj1event/jj1event.h"
#include "jj1level.h"
#include "jj1levelplayer/jj1levelplayer.h"

#include "game/game.h"
#include "io/file.h"
#include "io/gfx/font.h"
#include "io/gfx/sprite.h"
#include "io/gfx/video.h"
//#include "io/sound.h"
#include "loop.h"
#include "util.h"
#include "MAINCHAR.h"
#include <string.h>
#ifdef CASIO
	#include <fxcg/keyboard.h>
	#include <fxcg/display.h>
	#include <fxcg/file.h>
	#include <fxcg/misc.h>
	#include "platforms/casio.h"
	#include <alloca.h>
	#include <stdlib.h>
#endif

#define SKEY 254 /* Sprite colour key */


/**
 * Load the HUD graphical data.
 *
 * @return Error code
 */
int JJ1Level::loadPanel () {

	File* file;
	//unsigned char* pixels;
	unsigned char* sorted;
	int type, x, y;


	try {

		file = new File(F_PANEL, false);

	} catch (int e) {

		return e;

	}
	rle_panel=(unsigned char*)malloc(46272);
	if(!rle_panel){
		#ifdef CASIO
			casioQuitM("rle_panel");
		#else
			puts("Malloc rle_panel");
		#endif
	}
	file->loadRLE(46272,rle_panel);
	delete file;


	// Create the panel background
	initMiniSurface(&panel,(unsigned char *)rle_panel,SW,32);

	// De-scramble the panel's ammo graphics

	for (type = 0; type < 6; type++) {
		addobj(64*26,&panelAmmoramid[type]);
		sorted=(unsigned char *)objs[panelAmmoramid[type]].ptr;

		for (y = 0; y < 26; y++) {

			for (x = 0; x < 64; x++)
				sorted[(y * 64) + x] = rle_panel[(type * 64 * 32) + (y * 64) + (x >> 2) + ((x & 3) << 4) + (55 * 320)];

		}

		initMiniSurface(&panelAmmo[type],sorted,64,26);

	}


	rle_panel=(unsigned char*)realloc(rle_panel,SW*32);
	return E_NONE;
}


/**
 * Load a sprite.
 *
 * @param file File from which to load the sprite data
 * @param sprite Sprite that will receive the loaded data
 */
void JJ1Level::loadSprite(File* file, Sprite* sprite) {

	//unsigned char* pixels;
	int pos, maskOffset;
	int width, height;

	// Load dimensions
	width = file->loadShort() << 2;
	height = file->loadShort();
	#ifdef CASIO
		{char sI[16];
			itoa(width,(unsigned char *)sI);
			drawStrL(4,sI);
		}
	#else
		printf("Width: %d\n",width);
	#endif
	file->seek(2, false);

	maskOffset = file->loadShort();

	pos = file->loadShort() << 2;

	// Sprites can be either masked or not masked.
	if(maskOffset)
		++height;
	unsigned char * pixels=(unsigned char *)alloca(width*height);
	if (maskOffset) {

		// Masked

		//height++;

		// Skip to mask
		file->seek(maskOffset, false);

		// Find the end of the data
		pos += file->tell() + ((width >> 2) * height);

		// Read scrambled, masked pixel data
		//pixels = file->loadPixels(width * height, SKEY);
		//unsigned char * pixels=(unsigned char *)alloca(width*height);
		file->loadPixels(width * height, SKEY,pixels);
		sprite->setPixels(pixels, width, height, SKEY);

		//delete[] pixels;

		file->seek(pos, true);

	} else if (width) {

		// Not masked

		// Read scrambled pixel data
		//unsigned char * pixels=(unsigned char *)alloca(width*height);
		//pixels = file->loadPixels(width * height);
		file->loadPixels(width * height,pixels);
		sprite->setPixels(pixels, width, height, SKEY);

		//delete[] pixels;

	}
				/*#ifdef CASIO
				//Draw the sprite on screen to which one is causing issue
				//start at y=96
				{
				unsigned short *o=(unsigned short *)0xA8000000;
				o+=96*384;
				unsigned short x,y;
				for(y=0;y<height;++y){
					for(x=0;x<width;++x){
						*o++=video.currentPalette[*pixels++];
					}
					o+=384-width;
				}
				Bdisp_PutDisp_DD_stripe(96,96+height);
				//casioDelay(1000);//See which sprite is causing issue
				}
			#endif*/

	return;

}


/**
 * Load sprites.
 *
 * @param fileName Name of the file containing the level-specific sprites
 *
 * @return Error code
 */
int JJ1Level::loadSprites (char * fileName){
	//File* mainFile = NULL;
	//File* specFile = NULL;
try{
	unsigned char* buffer;
	int count;
	bool loaded;
	// Open fileName
	File specFile(fileName, false);
	int specSize=specFile.getSize();
	// This function loads all the sprites, not just those in fileName
	/*try {
		mainFile = new File(F_MAINCHAR, false);
	} catch (int e) {
		delete specFile;
		return e;
	}*/
	sprites = specFile.loadShort(256);

	// Include space in the sprite set for the blank sprite at the end
	spriteSet = new Sprite[sprites + 1];


	// Read offsets
	{
	//buffer = specFile.loadBlock(sprites * 2);
	buffer=(unsigned char *)alloca(sprites*2);
	specFile.loadBlock(sprites * 2,buffer);
	for (count = 0; count < sprites; count++)
		spriteSet[count].setOffset(buffer[count] << 2, buffer[sprites + count]);
	}
	//delete[] buffer;


	// Skip to where the sprites start in mainchar.000
	//mainFile->seek(2, true);


	// Loop through all the sprites to be loaded
	for (count = 0; count < sprites; count++) {

		loaded = false;

		/*if (mainFile->loadChar() == 0xFF) {

			// Go to the next sprite/file indicator
			mainFile->seek(1, false);

		} else {
			// Return to the start of the sprite
			mainFile->seek(-1, false);

			// Load the individual sprite data
			loadSprite(mainFile, spriteSet + count);

			loaded = true;

		}*/
		if(mainchar_spriteTab[count]!=0){
			loaded=true;
			initMiniSurface(&spriteSet[count].pixels,(void *)mainchar_spriteTab[count],width_sprites[count],height_sprites[count]);
			setColKey(&spriteSet[count].pixels,SKEY);
		}

		if (specFile.loadChar() == 0xFF) {

			// Go to the next sprite/file indicator
			specFile.seek(1, false);

		} else {

			// Return to the start of the sprite
			specFile.seek(-1, false);
			#ifdef CASIO
			{
				char tMp[8];
				itoa(count,(unsigned char *)tMp);
				drawStrL(3,tMp);
			}
			#endif
			// Load the individual sprite data
			loadSprite(&specFile, spriteSet + count);
			/*#ifdef CASIO
				//Draw the sprite on screen to which one is causing issue
				//start at y=96
				{
				unsigned char *i=spriteSet[count].pixels.pix;
				unsigned short *o=(unsigned short *)0xA8000000;
				o+=(96+spriteSet[count].pixels.h)*384;
				unsigned short x,y;
				for(y=0;y<spriteSet[count].pixels.h;++y){
					for(x=0;x<spriteSet[count].pixels.w;++x){
						*o++=video.currentPalette[*i++];
					}
					o+=384-spriteSet[count].pixels.w;
				}
				Bdisp_PutDisp_DD_stripe(96+spriteSet[count].pixels.h,96+(spriteSet[count].pixels.h*2));
				casioDelay(1000);//See which sprite is causing issue
				}
			#endif*/
			loaded = true;

		}

		/* If both fileName and mainchar.000 have file indicators, create a
		blank sprite */
		if (!loaded) spriteSet[count].clearPixels();


		// Check if the next sprite exists
		// If not, create blank sprites for the remainder
		if (specFile.tell() >= specSize) {

			for (count++; count < sprites; count++) {

				spriteSet[count].clearPixels();

			}
		}
	}
	//delete mainFile;
	//delete specFile;
	// Include a blank sprite at the end
	spriteSet[sprites].clearPixels();
}catch (int e){
	#ifdef CASIO
		casioQuit("Error loading sprites");
	#else
		puts("Sprites error");
	#endif
}
	return E_NONE;

}


/**
 * Load the tileset.
 *
 * @param fileName Name of the file containing the tileset
 *
 * @return The number of tiles loaded
 */
/*#ifdef CASIO
static void drawTileC(unsigned char * i,unsigned short xoff,unsigned short yoff){
	unsigned short *o=(unsigned short *)0xA8000000;
	o+=yoff*384;
	o+=xoff;
	unsigned short x,y;
	for(y=0;y<32;++y){
		for(x=0;x<32;++x){
			*o++=video.currentPalette[*i++];
		}
		o+=384-32;
	} 
}
#endif*/
int JJ1Level::loadTiles(char* fileName) {
try{
	unsigned char* buffer;
	int rle, pos, index, count, fileSize;
	int tiles;
	File file(fileName, false);

	// Load the palette
	file.loadPalette(palette);
	video.setPalette(palette);

	// Load the background palette
	file.loadPalette(skyPalette);


	// Skip the second, identical, background palette
	file.skipRLE();


	// Load the tile pixel indices

	tiles = 240; // Never more than 240 tiles
	addobj(tiles<<10,&tileSetramid);//240kb
	//buffer = new unsigned char[tiles << 10];
	buffer=(unsigned char *)objs[tileSetramid].ptr;
	file.seek(4, false);

	pos = 0;
	fileSize = file.getSize();

	// Read the RLE pixels
	// file::loadRLE() cannot be used, for reasons that will become clear
	#ifdef CASIO
		unsigned short U=0;
		unsigned int Tl=0;
	#endif
	while ((pos < (tiles << 10)) && (file.tell() < fileSize)) {
		#ifdef CASIO
			if(U==2048){
				U=0;
				{char P[16];
				itoa(pos,(unsigned char *)P);
				drawStrL(7,P);}
			}else
				++U;
		#endif
		rle = file.loadChar();

		if (rle & 128) {

			index = file.loadChar();

			for (count = 0; count < (rle & 127); count++) buffer[pos++] = index;

		} else if (rle) {
			#ifdef CASIO
				Bfile_ReadFile_OS(file.file,&buffer[pos],rle,-1);
				pos+=rle;
			#else
				for (count = 0; count < rle; count++)
					buffer[pos++] = file.loadChar();
			#endif
		} else { // This happens at the end of each tile

			// 0 pixels means 1 pixel, apparently
			buffer[pos++] = file.loadChar();

			file.seek(2, false); /* I assume this is the length of the next
				tile block */

			if (pos == (60 << 10)) file.seek(2, false);
			else if (pos == (120 << 10)) file.seek(2, false);
			else if (pos == (180 << 10)) file.seek(2, false);

		}

	}
//Draw the tiles so far loaded
	//delete file;

	// Work out how many tiles were actually loaded
	// Should be a multiple of 60
	tiles = pos >> 10;
	//tileSet = createSurface(buffer, TTOI(1), TTOI(tiles));
	initMiniSurface(&tileSet,buffer, TTOI(1), TTOI(tiles));
	//SDL_SetColorKey(tileSet, SDL_SRCCOLORKEY, TKEY);
	setColKey(&tileSet,TKEY);
	//delete[] buffer;
	resizeobj(tileSetramid, TTOI(1)*TTOI(tiles));
	return tiles;
}catch(int e){
	#ifdef CASIO
		casioQuit("Error loading tiles");
	#endif
	throw e;
}
}


/**
 * Load the level.
 *
 * @param fileName Name of the file containing the level data
 * @param checkpoint Whether or not the player(s) will start at a checkpoint
 *
 * @return Error code
 */

/**
 * Load the level.
 *
 * @param fileName Name of the file containing the level data
 * @param checkpoint Whether or not the player(s) will start at a checkpoint
 *
 * @return Error code
 */
int JJ1Level::load (char* fileName, bool checkpoint) {
	#ifdef CASIO
		drawStrL(1,"Loading...");
	#endif
	Anim* pAnims[JJ1PANIMS];
	File* file;
	unsigned char* buffer;
	const char* ext;
	char* string = NULL;
	int tiles;
	int count, x, y, type;
	unsigned char startX, startY;


	// Load font
	#ifdef CASIO
		drawStrL(2,"Fonts");
	#endif
	try {

		font = new Font(false);

	} catch (int e) {

		return e;

	}


	// Load panel
	#ifdef CASIO
		drawStrL(2,"Panel");
	#endif
	count = loadPanel();

	if (count < 0) {

		delete font;

		return count;

	}
	#ifdef CASIO
		drawStrL(2,"Loading Screen");
	#endif

	// Show loading screen

	// Open planet.### file

	if (!strcmp(fileName, LEVEL_FILE)) {

		// Using the downloaded level file

		string = createString("DOWNLOADED");

	} else {

		// Load the planet's name from the planet.### file

		string = createFileName(F_PLANET, fileName + strlen(fileName) - 3);

		try {

			file = new File(string, false);

		} catch (int e) {

			file = NULL;

		}

		delete[] string;

		if (file) {

			file->seek(2, true);
			string = file->loadString();

			delete file;

		} else {

			string = createString("CUSTOM");

		}

	}

	switch (fileName[5]) {

		case '0':

			ext = " LEVEL ONE";

			break;

		case '1':

			ext = " LEVEL TWO";

			break;

		case '2':

			string[0] = 0;
			ext = "SECRET LEVEL";

			break;

		default:

			ext = " LEVEL";

			break;

	}

	video.setPalette(menuPalette);

	video.clearScreen(0);

	x = (canvasW >> 1) - ((strlen(string) + strlen(ext)) << 2);
	x = fontmn2->showString("LOADING ", x - 60, (canvasH >> 1) - 16);
	x = fontmn2->showString(string, x, (canvasH >> 1) - 16);
	fontmn2->showString(ext, x, (canvasH >> 1) - 16);

	delete[] string;

	if (::loop(NORMAL_LOOP) == E_QUIT) return E_QUIT;

	// Open level file

	try {

		file = new File(fileName, false);

	} catch (int e) {

		deletePanel();
		delete font;

		return e;

	}

	// Load the blocks.### extension
	#ifdef CASIO
		drawStrL(2,"Layout");
	#endif
	// Skip past all level data
	file->seek(39, true);
	file->skipRLE();
	file->skipRLE();
	file->skipRLE();
	file->skipRLE();
	file->skipRLE();
	file->skipRLE();
	file->skipRLE();
	file->skipRLE();
	file->seek(598, false);
	file->skipRLE();
	file->seek(4, false);
	file->skipRLE();
	file->skipRLE();
	file->seek(25, false);
	file->skipRLE();
	file->seek(3, false);

	// Load the level number
	levelNum = file->loadChar() ^ 210;

	// Load the world number
	worldNum = file->loadChar() ^ 4;


	// Load tile set from appropriate blocks.###

	// Load tile set extension
	file->seek(8, false);
	ext = file->loadString();

	// Create tile set file name
	if (!strcmp(ext, "999")) string = createFileName(F_BLOCKS, worldNum);
	else string = createFileName(F_BLOCKS, ext);

	delete[] ext;
	#ifdef CASIO
		drawStrL(2,"Tiles");
	#endif
	tiles = loadTiles(string);

	delete[] string;

	if (tiles < 0) {

		delete file;
		deletePanel();
		delete font;

		return tiles;

	}


	// Load sprite set from corresponding Sprites.###

	string = createFileName(F_SPRITES, worldNum);
	#ifdef CASIO
		drawStrL(2,"Sprites");
	#endif
	count = loadSprites(string);

	delete[] string;

	if (count < 0) {

		//SDL_FreeSurface(tileSet);
		if(tileSetramid!=INVALID_OBJ)
			freeobj(tileSetramid);
		delete file;
		deletePanel();
		delete font;

		return count;

	}


	// Skip to tile and event reference data
	file->seek(39, true);

	// Load tile and event references
	#ifdef CASIO
		drawStrL(2,"Layout");
	#endif
	buffer=(unsigned char *)malloc(LW * LH * 2);
	if(!buffer){
		#ifdef CASIO
			casioQuitM("grid");
		#else
			puts("Malloc error grid");
		#endif
	}
	file->loadRLE(LW * LH * 2,buffer);

	// Create grid from data
	for (x = 0; x < LW;++x){

		for (y = 0; y < LH;++y) {

			grid[y][x].tile = buffer[(y + (x * LH)) << 1];
			grid[y][x].bg = buffer[((y + (x * LH)) << 1) + 1] >> 7;
			grid[y][x].event = buffer[((y + (x * LH)) << 1) + 1] & 127;
			grid[y][x].hits = 0;
			grid[y][x].time = 0;

		}

	}
	free(buffer);
	//delete[] buffer;

	// A mysterious block of mystery
	file->skipRLE();


	// Load mask data
	buffer=(unsigned char *)malloc(tiles*8);
	file->loadRLE(tiles * 8,buffer);
	if(!buffer){
		#ifdef CASIO
			casioQuitM("mask");
		#else
			puts("Malloc error mask");
		#endif
	}
	// Unpack bits
	for (count = 0; count < tiles; count++) {
		for (y = 0; y < 8; y++) {
			for (x = 0; x < 8; x++)
				mask[count][(y << 3) + x] = (buffer[(count << 3) + y] >> x) & 1;
		}
	}
	//delete[] buffer;
	free(buffer);
	/* Uncomment the code below if you want to see the mask instead of the tile
	graphics during gameplay */

	/*if (SDL_MUSTLOCK(tileSet)) SDL_LockSurface(tileSet);

	for (count = 0; count < tiles; count++) {

		for (y = 0; y < 32; y++) {

			for (x = 0; x < 32; x++) {

				if (mask[count][((y >> 2) << 3) + (x >> 2)] == 1)
					((char *)(tileSet->pixels))
						[(count * 1024) + (y * 32) + x] = 88;

			}

		}

	}

	if (SDL_MUSTLOCK(tileSet)) SDL_UnlockSurface(tileSet);*/


	// Load special event path
	buffer=(unsigned char *)malloc(PATHS << 9);
	file->loadRLE(PATHS << 9,buffer);

	for (type = 0; type < PATHS; type++) {

		path[type].length = buffer[type << 9] + (buffer[(type << 9) + 1] << 8);
		if (path[type].length < 1) path[type].length = 1;
		path[type].x = new short int[path[type].length];
		path[type].y = new short int[path[type].length];

		for (count = 0; count < path[type].length; count++) {

			path[type].x[count] = ((signed char *)buffer)[(type << 9) + (count << 1) + 3] << 2;
			path[type].y[count] = ((signed char *)buffer)[(type << 9) + (count << 1) + 2];

		}

	}
	free(buffer);
	//delete[] buffer;


	// Load event set
	buffer=(unsigned char *)malloc(EVENTS * ELENGTH);
	if(!buffer){
		#ifdef CASIO
			casioQuitM("Event set");
		#else
			puts("Malloc error event set");
		#endif
	}
	file->loadRLE(EVENTS * ELENGTH,buffer);

	// Fill event set with data
	for (count = 0; count < EVENTS; count++) {

		eventSet[count].difficulty           = buffer[count * ELENGTH];
		eventSet[count].reflection           = buffer[(count * ELENGTH) + 2];
		eventSet[count].movement             = buffer[(count * ELENGTH) + 4];
		eventSet[count].anims[E_LEFTANIM]    = buffer[(count * ELENGTH) + 5];
		eventSet[count].anims[E_RIGHTANIM]   = buffer[(count * ELENGTH) + 6];
		eventSet[count].magnitude            = buffer[(count * ELENGTH) + 8];
		eventSet[count].strength             = buffer[(count * ELENGTH) + 9];
		eventSet[count].modifier             = buffer[(count * ELENGTH) + 10];
		eventSet[count].points               = buffer[(count * ELENGTH) + 11];
		eventSet[count].bullet               = buffer[(count * ELENGTH) + 12];
		eventSet[count].bulletPeriod         = buffer[(count * ELENGTH) + 13];
		eventSet[count].speed                = buffer[(count * ELENGTH) + 15] + 1;
		eventSet[count].animSpeed            = buffer[(count * ELENGTH) + 17] + 1;
		eventSet[count].sound                = buffer[(count * ELENGTH) + 21];
		eventSet[count].multiA               = buffer[(count * ELENGTH) + 22];
		eventSet[count].multiB               = buffer[(count * ELENGTH) + 23];
		eventSet[count].pieceSize            = buffer[(count * ELENGTH) + 24];
		eventSet[count].pieces               = buffer[(count * ELENGTH) + 25];
		eventSet[count].angle                = buffer[(count * ELENGTH) + 26];
		eventSet[count].anims[E_LFINISHANIM] = buffer[(count * ELENGTH) + 28];
		eventSet[count].anims[E_RFINISHANIM] = buffer[(count * ELENGTH) + 29];
		eventSet[count].anims[E_LSHOOTANIM]  = buffer[(count * ELENGTH) + 30];
		eventSet[count].anims[E_RSHOOTANIM]  = buffer[(count * ELENGTH) + 31];

	}

	// Process grid

	enemies = items = 0;

	for (x = 0; x < LW; x++) {

		for (y = 0; y < LH; y++) {

			type = grid[y][x].event;

			if (type) {

				// If the event hurts and can be killed, it is an enemy
				// Anything else that scores is an item
				if ((eventSet[type].modifier == 0) && eventSet[type].strength) enemies++;
				else if (eventSet[type].points) items++;
			}
		}
	}
	//delete[] buffer;
	free(buffer);

	// Yet more doubtless essential data
	file->skipRLE();


	// Load animation set
	#ifdef CASIO
		drawStrL(2,"Animations");
	#endif
	buffer=(unsigned char *)malloc(ANIMS<<6);
	if(!buffer){
		#ifdef CASIO
			casioQuitM("mask");
		#else
			puts("Malloc error mask");
		#endif
	}
	file->loadRLE(ANIMS << 6,buffer);
	
	// Create animation set based on that data
	for (count = 0; count < ANIMS; count++) {

		animSet[count].setData(buffer[(count << 6) + 6],
			buffer[count << 6], buffer[(count << 6) + 1],
			buffer[(count << 6) + 3], buffer[(count << 6) + 4],
			buffer[(count << 6) + 2], buffer[(count << 6) + 5]);

		for (y = 0; y < buffer[(count << 6) + 6]; y++) {

			// Get frame
			x = buffer[(count << 6) + 7 + y];
			if (x > sprites) x = sprites;

			// Assign sprite and vertical offset
			animSet[count].setFrame(y, true);
			animSet[count].setFrameData(spriteSet + x,
				buffer[(count << 6) + 26 + y], buffer[(count << 6) + 45 + y]);

		}

	}
	//delete[] buffer;
	free(buffer);
	#ifdef CASIO
		drawStrL(2,"Misc");
	#endif

	// At general data

	// There's a a whole load of unknown data around here

	// Like another one of those pesky RLE blocks
	file->skipRLE();

	// And 217 bytes of DOOM
	file->seek(217, false);


	// Load sound map

	x = file->tell();


	//file->seek(x + 288, true);

	// Music file
	//musicFile = file->loadString();

	// 26 bytes of undiscovered usefulness, less the music file name
	file->seek(x + 314, true);

	// End of episode cutscene
	sceneFile = file->loadString();

	// 52 bytes of undiscovered usefulness, less the cutscene file name
	file->seek(x + 366, true);


	// The players' initial coordinates
	startX = file->loadShort(LW);
	startY = file->loadShort(LH) + 1;


	// Next level
	x = file->loadChar();
	y = file->loadChar();
	setNext(x, y);


	// Thanks to Doubble Dutch for the water level bytes
	file->seek(4, false);
	waterLevelTarget = ITOF(file->loadShort() + 17);
	waterLevel = waterLevelTarget - F8;
	waterLevelSpeed = -80000;


	// Thanks to Feline and the JCS94 team for the next bits:

	file->seek(3, false);

	// Now at "Section 15"


	// Load player's animation set references
	buffer=(unsigned char *)malloc(JJ1PANIMS*2);
	if(!buffer){
		#ifdef CASIO
			casioQuitM("JJ1PANIMS");
		#else
			puts("Malloc error JJ1PANIMS");
		#endif
	}
	file->loadRLE(JJ1PANIMS * 2,buffer);

	for (x = 0; x < JJ1PANIMS; x++) {

		playerAnims[x] = buffer[x << 1];
		pAnims[x] = animSet + playerAnims[x];
	}

	free(buffer);

	createLevelPlayers(LT_JJ1, pAnims, NULL, checkpoint, startX, startY);

	// Load miscellaneous animations
	miscAnims[0] = file->loadChar();
	miscAnims[1] = file->loadChar();
	miscAnims[2] = file->loadChar();
	miscAnims[3] = file->loadChar();

	// Load bullet set
	buffer=(unsigned char *)malloc(BULLETS * BLENGTH);
	if(!buffer){
		#ifdef CASIO
			casioQuit("Malloc error bullet set");
		#else
			puts("Malloc error bullet set");
		#endif
	}
	file->loadRLE(BULLETS * BLENGTH,buffer);

	for (count = 0; count < BULLETS; count++) {

		memcpy(bulletSet[count], buffer + (count * BLENGTH), BLENGTH);
	}

	free(buffer);

	// Now at "Section 18." More skippability.
	file->skipRLE();


	// Now at "Section 19," THE MAGIC SECTION

	// First byte is the background palette effect type
	type = file->loadChar();

	sky = false;

	switch (type) {

		case 2:

			sky = true;

			// Sky background effect
			paletteEffects = new SkyPaletteEffect(156, 100, FH, skyPalette, NULL);

			break;

		case 8:

			// Parallaxing background effect
			paletteEffects = new P2DPaletteEffect(128, 64, FE, NULL);

			break;

		case 9:

			// Diagonal stripes "parallaxing" background effect
			paletteEffects = new P1DPaletteEffect(128, 32, FH, NULL);

			break;

		case 11:

			// The deeper below water, the darker it gets
			paletteEffects = new WaterPaletteEffect(TTOF(32), NULL);

			break;

		default:

			// No effect
			paletteEffects = NULL;

			break;

	}

	// Palette animations
	// These are applied to every level without a conflicting background effect
	// As a result, there are a few levels with things animated that shouldn't
	// be

	// In Diamondus: The red/yellow palette animation
	paletteEffects = new RotatePaletteEffect(112, 4, F32, paletteEffects);

	// In Diamondus: The waterfall palette animation
	paletteEffects = new RotatePaletteEffect(116, 8, F16, paletteEffects);

	// The following were discoverd by Unknown/Violet

	paletteEffects = new RotatePaletteEffect(124, 3, F16, paletteEffects);

	if ((type != PE_1D) && (type != PE_2D))
		paletteEffects = new RotatePaletteEffect(132, 8, F16, paletteEffects);

	if ((type != PE_SKY) && (type != PE_2D))
		paletteEffects = new RotatePaletteEffect(160, 32, -F16, paletteEffects);

	if (type != PE_SKY) {

		paletteEffects = new RotatePaletteEffect(192, 32, -F32, paletteEffects);
		paletteEffects = new RotatePaletteEffect(224, 16, F16, paletteEffects);

	}

	// Level fade-in/white-in effect
	if (checkpoint) paletteEffects = new FadeInPaletteEffect(T_START, paletteEffects);
	else paletteEffects = new WhiteInPaletteEffect(T_START, paletteEffects);


	// Check if a sun/star/distant planet, etc. is visible
	skyOrb = file->loadChar();

	// If so, find out which tile it uses
	if (skyOrb) skyOrb = file->loadChar();
	else file->loadChar();


	file->seek(15, false);

	// Board animations
	miscAnims[MA_LBOARD] = file->loadChar();
	miscAnims[MA_RBOARD] = file->loadChar();


	// And that's us done!

	delete file;

	// Set the tick at which the level will end
	endTime = (5 - game->getDifficulty()) * 2 * 60 * 1000;

	events = NULL;
	bullets = NULL;
	energyBar = 0;
	ammoType = 0;
	ammoOffset = -1;

	#ifdef CASIO
		drawStrL(2,"Done");
	#endif
	return E_NONE;
}


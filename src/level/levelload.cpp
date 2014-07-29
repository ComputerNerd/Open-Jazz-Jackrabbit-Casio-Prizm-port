
/*
 *
 * levelload.cpp
 *
 * 22nd July 2008: Created levelload.c from parts of level.c
 * 3rd February 2009: Renamed levelload.c to levelload.cpp
 * 18th July 2009: Created demolevel.cpp from parts of level.cpp and
 *                 levelload.cpp
 * 19th July 2009: Added parts of levelload.cpp to level.cpp
 *
 * Part of the OpenJazz project
 *
 *
 * Copyright (c) 2005-2010 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

/*
 * Deals with the loading of level data.
 *
 */


#include "bullet.h"
#include "event/event.h"
#include "level.h"

#include "game/game.h"
#include "game/gamemode.h"
#include "io/file.h"
#include "io/gfx/font.h"
#include "io/gfx/paletteeffects.h"
#include "io/gfx/sprite.h"
#include "io/gfx/video.h"
#include "io/sound.h"
#include "menu/menu.h"
#include "player/player.h"
#include "loop.h"
#include "util.h"

#include <string.h>


#define SKEY 254 /* Sprite colour key */


void Level::loadSprite (File* file, Sprite* sprite) {

	unsigned char* pixels;
	int pos, maskOffset;
	int width, height;

	// Load dimensions
	width = file->loadShort() << 2;
	height = file->loadShort();

	file->seek(2, false);

	maskOffset = file->loadShort();

	pos = file->loadShort() << 2;

	// Sprites can be either masked or not masked.
	if (maskOffset) {

		// Masked

		height++;

		// Skip to mask
		file->seek(maskOffset, false);

		// Find the end of the data
		pos += file->tell() + ((width >> 2) * height);

		// Read scrambled, masked pixel data
		pixels = file->loadPixels(width * height, SKEY);
		sprite->setPixels(pixels, width, height, SKEY);

		delete[] pixels;

		file->seek(pos, true);

	} else if (width) {

		// Not masked

		// Read scrambled pixel data
		pixels = file->loadPixels(width * height);
		sprite->setPixels(pixels, width, height, SKEY);

		delete[] pixels;

	}


	return;

}


int Level::loadSprites (char * fileName) {

	File* mainFile = NULL;
	File* specFile = NULL;
	int count;


	// Open fileName
	try {

		specFile = new File(fileName, false);

	} catch (int e) {

		return e;

	}


	// This function loads all the sprites, not fust those in fileName
	try {

		mainFile = new File(F_MAINCHAR, false);

	} catch (int e) {

		delete specFile;

		return e;

	}


	sprites = specFile->loadShort();

	// Include space in the sprite set for the blank sprite at the end
	spriteSet = new Sprite[sprites + 1];

	// Read horizontal offsets
	for (count = 0; count < sprites; count++)
		spriteSet[count].xOffset = specFile->loadChar() << 2;

	// Read vertical offsets
	for (count = 0; count < sprites; count++)
		spriteSet[count].yOffset = specFile->loadChar();


	// Skip to where the sprites start in mainchar.000
	mainFile->seek(2, true);


	// Loop through all the sprites to be loaded
	for (count = 0; count < sprites; count++) {

		if (specFile->loadChar() == 0xFF) {

			// Go to the next sprite/file indicator
			specFile->seek(1, false);

			if (mainFile->loadChar() == 0xFF) {

				// Go to the next sprite/file indicator
				mainFile->seek(1, false);

				/* Both fileName and mainchar.000 have file indicators, so
				create a blank sprite */
				spriteSet[count].clearPixels();

				continue;

			} else {

				// Return to the start of the sprite
				mainFile->seek(-1, false);

				// Load the individual sprite data
				loadSprite(mainFile, spriteSet + count);

			}

		} else {

			// Return to the start of the sprite
			specFile->seek(-1, false);

			// Go to the main file's next sprite/file indicator
			mainFile->seek(2, false);

			// Load the individual sprite data
			loadSprite(specFile, spriteSet + count);

		}


		// Check if the next sprite exists
		// If not, create blank sprites for the remainder
		if (specFile->tell() >= specFile->getSize()) {

			for (count++; count < sprites; count++) {

				spriteSet[count].clearPixels();

			}

		}

	}

	delete mainFile;
	delete specFile;


	// Include a blank sprite at the end
	spriteSet[sprites].clearPixels();
	spriteSet[sprites].xOffset = 0;
	spriteSet[sprites].yOffset = 0;

	return E_NONE;

}


int Level::loadTiles (char * fileName) {

	File *file;
	unsigned char *buffer;
	int rle, pos, index, count, fileSize;
	int tiles;


	try {

		file = new File(fileName, false);

	} catch (int e) {

		return e;

	}


	// Load the palette
	file->loadPalette(palette);


	// Load the background palette
	file->loadPalette(skyPalette);


	// Skip the second, identical, background palette
	file->skipRLE();


	// Load the tile pixel indices

	tiles = 240; // Never more than 240 tiles

	buffer = new unsigned char[tiles * 1024];

	file->seek(4, false);

	pos = 0;
	fileSize = file->getSize();

	// Read the RLE pixels
	// file::loadRLE() cannot be used, for reasons that will become clear
	while ((pos < 1024 * tiles) && (file->tell() < fileSize)) {

		rle = file->loadChar();

		if (rle & 128) {

			index = file->loadChar();

			for (count = 0; count < (rle & 127); count++) buffer[pos++] = index;

		} else if (rle) {

			for (count = 0; count < rle; count++)
				buffer[pos++] = file->loadChar();

		} else { // This happens at the end of each tile

			// 0 pixels means 1 pixel, apparently
			buffer[pos++] = file->loadChar();

			file->seek(2, false); /* I assume this is the length of the next
				tile block */

			if (pos == 1024 * 60) file->seek(2, false);
			if (pos == 1024 * 120) file->seek(2, false);
			if (pos == 1024 * 180) file->seek(2, false);

		}

	}

	delete file;

	// Work out how many tiles were actually loaded
	// Should be a multiple of 60
	tiles = pos / 1024;

	tileSet = createSurface(buffer, TTOI(1), TTOI(tiles));
	SDL_SetColorKey(tileSet, SDL_SRCCOLORKEY, TKEY);

	delete[] buffer;

	return tiles;

}


int Level::load (char *fileName, unsigned char diff, bool checkpoint) {

	File *file;
	unsigned char *buffer;
	const char *ext;
	char *string;
	int tiles;
	int count, x, y, type;


	difficulty = diff;


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

			delete[] string;

			return e;

		}

		delete[] string;

		file->seek(2, true);
		string = file->loadString();

		delete file;

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

	video.setPalette(menu->palettes[1]);

	clearScreen(0);

	x = (canvasW >> 1) - ((strlen(string) + strlen(ext)) << 2);
	x = fontmn2->showString("LOADING ", x - 60, (canvasH >> 1) - 16);
	x = fontmn2->showString(string, x, (canvasH >> 1) - 16);
	fontmn2->showString(ext, x, (canvasH >> 1) - 16);

	delete[] string;

	if (loop(NORMAL_LOOP) == E_QUIT) return E_QUIT;



	// Open level file

	try {

		file = new File(fileName, false);

	} catch (int e) {

		return e;

	}


	// Load level data from a Level#.### file

	// Load the blocks.### extension

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

	if (!strcmp(ext, "999")) {

		// Use the level file's extension instead
		delete[] ext;
		ext = createString(fileName + strlen(fileName) - 3);

	}

	// Allocate space for file names
	string = createFileName(F_BLOCKS, ext);

	delete[] ext;

	tiles = loadTiles(string);

	delete[] string;

	if (tiles < 0) {

		delete file;

		return tiles;

	}


	// Load sprite set from corresponding Sprites.###

	string = createFileName(F_SPRITES, worldNum);

	count = loadSprites(string);

	delete[] string;

	if (count < 0) {

		SDL_FreeSurface(tileSet);
		delete file;

		return count;

	}


	// Skip to tile and event reference data
	file->seek(39, true);

	// Load tile and event references

	buffer = file->loadRLE(LW * LH * 2);

	// Create grid from data
	for (x = 0; x < LW; x++) {

		for (y = 0; y < LH; y++) {

			grid[y][x].tile = buffer[(y + (x * LH)) << 1];
			grid[y][x].bg = buffer[((y + (x * LH)) << 1) + 1] >> 7;
			grid[y][x].event = buffer[((y + (x * LH)) << 1) + 1] & 127;
			grid[y][x].hits = 0;
			grid[y][x].time = 0;

		}

	}

	delete[] buffer;

	// A mysterious block of mystery
	file->skipRLE();


	// Load mask data

	buffer = file->loadRLE(tiles * 8);

	// Unpack bits
	for (count = 0; count < tiles; count++) {

		for (y = 0; y < 8; y++) {

			for (x = 0; x < 8; x++)
				mask[count][(y << 3) + x] = (buffer[(count << 3) + y] >> x) & 1;

		}

	}

	delete[] buffer;

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

	buffer = file->loadRLE(PATHS << 9);

	for (type = 0; type < PATHS; type++) {

		path[type].length = buffer[type << 9] + (buffer[(type << 9) + 1] << 8);
		path[type].node = 0;
		if (path[type].length < 1) path[type].length = 1;
		path[type].x = new short int[path[type].length];
		path[type].y = new short int[path[type].length];

		for (count = 0; count < path[type].length; count++) {

			path[type].x[count] = ((signed char *)buffer)[(type << 9) + (count << 1) + 3] << 2;
			path[type].y[count] = ((signed char *)buffer)[(type << 9) + (count << 1) + 2] << 2;

		}

	}

	delete[] buffer;


	// Load event set

	buffer = file->loadRLE(EVENTS * ELENGTH);

	// Fill event set with data
	for (count = 0; count < EVENTS; count++) {

		memcpy(eventSet[count], buffer + (count * ELENGTH), ELENGTH);
		eventSet[count][E_MOVEMENTSP]++;

	}

	delete[] buffer;


	// Process grid

	enemies = items = 0;

	for (x = 0; x < LW; x++) {

		for (y = 0; y < LH; y++) {

			type = grid[y][x].event;

			if (type) {

				// Eliminate event references for events of too high a difficulty
				if (eventSet[type][E_DIFFICULTY] > difficulty) grid[y][x].event = 0;

				// If the event hurts and can be killed, it is an enemy
				// Anything else that scores is an item
				if ((eventSet[type][E_MODIFIER] == 0) && eventSet[type][E_HITSTOKILL]) enemies++;
				else if (eventSet[type][E_ADDEDSCORE]) items++;

			}

		}

	}


	// Yet more doubtless essential data
	file->skipRLE();


	// Load animation set

	buffer = file->loadRLE(ANIMS << 6);

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

	delete[] buffer;


	// At general data

	// There's a a whole load of unknown data around here

	// Like another one of those pesky RLE blocks
	file->skipRLE();

	// And 217 bytes of DOOM
	file->seek(217, false);


	// Load sound map

	x = file->tell();

	for (count = 0; count < 32; count++) {

		file->seek(x + (count * 9), true);

		string = file->loadString();

		soundMap[count] = -1;

		// Search for matching sound

		for (y = 0; (y < nSounds) && (soundMap[count] == -1); y++) {

			if (!strcmp(string, sounds[y].name)) soundMap[count] = y;

		}

		delete[] string;

	}

	file->seek(x + 288, true);

	// Music file
	musicFile = file->loadString();

	// 26 bytes of undiscovered usefulness, less the music file name
	file->seek(x + 314, true);

	// End of episode cutscene
	sceneFile = file->loadString();

	// 52 bytes of undiscovered usefulness, less the cutscene file name
	file->seek(x + 366, true);


	// The players' coordinates
	x = file->loadShort();
	y = file->loadShort() + 1;

	if (!checkpoint && game) game->setCheckpoint(x, y);


	// Set the players' initial values
	if (game) {

		for (count = 0; count < nPlayers; count++)
			game->resetPlayer(players + count, false);

    } else {

		localPlayer->reset();
		localPlayer->setPosition(TTOF(x), TTOF(y));

    }


	// Next level
	x = file->loadChar();
	y = file->loadChar();
	setNext(x, y);


	// Thanks to Doubble Dutch for the water level bytes
	file->seek(4, false);
	waterLevelTarget = ITOF(file->loadShort());
	waterLevel = waterLevelTarget - F8;
	waterLevelSpeed = 0;


	// Thanks to Feline and the JCS94 team for the next bits:

	file->seek(3, false);

	// Now at "Section 15"


	// Load player's animation set references

	buffer = file->loadRLE(PANIMS * 2);
	string = new char[PANIMS + 3];

	for (x = 0; x < PANIMS; x++) string[x + 3] = buffer[x << 1];

	for (x = 0; x < nPlayers; x++) players[x].setAnims(string + 3);

	if (gameMode) {

		string[0] = MTL_P_ANIMS;
		string[1] = MT_P_ANIMS;
		string[2] = 0;
		game->send((unsigned char *)string);

	}

	delete[] string;
	delete[] buffer;


	// Load Skip to bullet set
	miscAnims[0] = file->loadChar();
	miscAnims[1] = file->loadChar();
	miscAnims[2] = file->loadChar();
	miscAnims[3] = file->loadChar();


	// Load bullet set
	buffer = file->loadRLE(BULLETS * BLENGTH);

	for (count = 0; count < BULLETS; count++) {

		memcpy(bulletSet[count], buffer + (count * BLENGTH), BLENGTH);

		// Make sure bullets go in the right direction

		if (bulletSet[count][B_XSPEED] > 0)
			bulletSet[count][B_XSPEED] = -bulletSet[count][B_XSPEED];

		if (bulletSet[count][B_XSPEED | 1] < 0)
			bulletSet[count][B_XSPEED | 1] = -bulletSet[count][B_XSPEED | 1];

		if (bulletSet[count][B_XSPEED | 2] > 0)
			bulletSet[count][B_XSPEED | 2] = -bulletSet[count][B_XSPEED | 2];

		if (bulletSet[count][B_XSPEED | 3] < 0)
			bulletSet[count][B_XSPEED | 3] = -bulletSet[count][B_XSPEED | 3];

	}

	delete[] buffer;


	// Now at "Section 18." More skippability.
	file->skipRLE();


	// Now at "Section 19," THE MAGIC SECTION

	// First byte is the background palette effect type
	type = file->loadChar();

	sky = false;

	// Free any existing palette effects
	if (paletteEffects) delete paletteEffects;

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


	file->seek(1, false);

	skyOrb = file->loadChar(); /* A.k.a the sun, the moon, the brightest star,
		that red planet with blue veins... */


	// And that's us done!

	delete file;


	// Set the tick at which the level will end
	endTime = (5 - difficulty) * 2 * 60 * 1000;


	events = NULL;
	bullets = NULL;

	energyBar = 0;

	return E_NONE;

}



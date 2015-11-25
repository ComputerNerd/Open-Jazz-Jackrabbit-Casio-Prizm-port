
/**
 *
 * @file level.cpp
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 23rd August 2005: Created level.c
 * 3rd February 2009: Renamed level.c to level.cpp
 * 19th July 2009: Created levelframe.cpp from parts of level.cpp
 * 19th July 2009: Added parts of levelload.cpp to level.cpp
 * 30th March 2010: Created baselevel.cpp from parts of level.cpp and
 *                  levelframe.cpp
 * 1st August 2012: Renamed baselevel.cpp to level.cpp
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
 * Deals with functionality common to ordinary levels and bonus levels.
 *
 */


#include "level.h"

#include "game/game.h"
#include "io/controls.h"
#include "io/gfx/font.h"
#include "io/gfx/sprite.h"
#include "io/gfx/video.h"
//#include "io/sound.h"
#include "player/player.h"
#include "jj1scene/jj1scene.h"
#include "loop.h"
#include "setup.h"
#ifdef CASIO
	#include <fxcg/keyboard.h>
	#include <fxcg/display.h>
	#include "platforms/casio.h"
#endif

/**
 * Create a new base level
 */
Level::Level (Game* owner) {
	//This is called by JJ1Level
	game = owner;

	menuOptions[0] = "continue game";
	menuOptions[2] = "save game";
	menuOptions[3] = "load game";
	menuOptions[4] = "setup options";
	menuOptions[5] = "quit game";
	// Arbitrary initial value
	smoothfps = 60;
	elapsed=0;

	paletteEffects = NULL;

	paused = false;

	// Set the level stage
	stage = LS_NORMAL;

	stats = 0;
	return;

}


/**
 * Destroy base level
 */
Level::~Level () {

	//stopMusic();

	if (paletteEffects) delete paletteEffects;

	return;

}


/**
 * Set the players' initial values.
 *
 * @param levelType The type of level for which to create a level player
 * @param anims New level player animations
 * @param flippedAnims New level player flipped animations
 * @param checkpoint Whether or not a checkpoint is in use
 * @param x The level players' new grid x-coordinate
 * @param y The level players' new grid y-coordinate
 */
void Level::createLevelPlayers (LevelType levelType, Anim** anims,
	Anim** flippedAnims, bool checkpoint, unsigned char x, unsigned char y) {
	#ifdef CASIO
		drawStrL(2,"L1");
	#endif
	int count;

	if (!checkpoint) game->setCheckpoint(x, y);//Crashes somewhere here on casio
	#ifdef CASIO
		drawStrL(2,"L2");
	#endif
	for (count = 0; count < nPlayers; count++) {

		players[count].createLevelPlayer(levelType, anims, flippedAnims, x, y);
		#ifdef CASIO
			drawStrL(2,"L3");
		#endif
		game->resetPlayer(players + count);

	}
	#ifdef CASIO
		drawStrL(2,"L4");
	#endif
	return;

}


/**
 * Play a cutscene.
 *
 * @param file File name of the cutscene to be played
 *
 * @return Error code
 */
int Level::playScene (char* file) {

	JJ1Scene* scene;
	int ret;

	delete paletteEffects;
	paletteEffects = NULL;

	try {

		scene = new JJ1Scene(file);

	} catch (int e) {

		return e;

	}

	ret = scene->play();

	delete scene;

	return ret;

}


/**
 * Perform timing calculations.
 */
void Level::timeCalcs(){
	// Calculate smoothed fps
	elapsed+=(ticks - prevTicks);
	++elapsedcnt;
	if(elapsed>=1000){
		smoothfps=elapsedcnt;
		elapsed-=1000;
		elapsedcnt=0;
	}
	/* This equation is a simplified version of
	(fps * c) + (smoothfps * (1 - c))
	where c = (1 / fps)
	and fps = 1000 / (ticks - prevTicks)
	In other words, the response of smoothFPS to changes in FPS decreases as the
	framerate increases
	The following version is for c = (1 / smoothfps)
	*/
	// smoothfps = (fps / smoothfps) + smoothfps - 1;

	// Track number of ticks of gameplay since the level started

	if (paused) {

		tickOffset = globalTicks - ticks;

	} else if (globalTicks - tickOffset > ticks + 100) {

		prevTicks = ticks;
		ticks += 100;

		tickOffset = globalTicks - ticks;

	} else {

		prevTicks = ticks;
		ticks = globalTicks - tickOffset;

	}

	return;

}


/**
 * Calculate the amount of time since the last completed step.
 *
 * @return Time since last step
 */
int Level::getTimeChange () {

	return paused? 0: ticks - ((steps * (setup.slowMotion? 100: 50)) / 3);

}


/**
 * Display menu (if visible) and statistics.
 *
 * @param bg Palette index of the box(es)
 * @param menu Whether or not the level menu should be displayed
 * @param option Selected menu uption
 * @param textPalIndex The first palette index for unseleceted text
 * @param selectedTextPalIndex The first palette index for selected text
 * @param textPalSpan The number of palette indices for text
 */
void Level::drawOverlay (unsigned char bg, bool menu, int option,
	unsigned char textPalIndex, unsigned char selectedTextPalIndex,
	int textPalSpan) {

	char* difficultyOptions[4] = {"easy", "medium", "hard", "turbo"};
	int count, width;

	// Draw graphics statistics

	if (stats & S_SCREEN) {

#ifdef SCALE
		if (video.getScaleFactor() > 1)
			drawRect(canvasW - 84, 11, 80, 37, bg);
		else
#endif
			drawRect(canvasW - 84, 11, 80, 25, bg);

		panelBigFont->showNumber(384, canvasW - 52, 14);
		panelBigFont->showString("x", canvasW - 48, 14);
		panelBigFont->showNumber(216, canvasW - 12, 14);
		panelBigFont->showString("fps", canvasW - 76, 26);
		panelBigFont->showNumber(smoothfps, canvasW - 12, 26);

#ifdef SCALE
		if (video.getScaleFactor() > 1) {

			panelBigFont->showNumber(canvasW, canvasW - 52, 38);
			panelBigFont->showString("x", canvasW - 48, 39);
			panelBigFont->showNumber(canvasH, canvasW - 12, 38);

		}
#endif

	}

	// Draw player list

	if (stats & S_PLAYERS) {

		width = 39;

		for (count = 0; count < nPlayers; count++)
			if (panelBigFont->getStringWidth(players[count].getName()) > width)
				width = panelBigFont->getStringWidth(players[count].getName());

		drawRect((canvasW >> 1) - 48, 11, width + 57, (nPlayers * 12) + 1, bg);

		for (count = 0; count < nPlayers; count++) {

			panelBigFont->showNumber(count + 1,
				(canvasW >> 1) - 24, 14 + (count * 12));
			panelBigFont->showString(players[count].getName(),
				(canvasW >> 1) - 16, 14 + (count * 12));
			panelBigFont->showNumber(players[count].teamScore,
				(canvasW >> 1) + width + 1, 14 + (count * 12));

		}

	}


	if (menu) {

		// Draw the menu

		drawRect((canvasW >> 2) - 8, (canvasH >> 1) - 54, 144, 108, bg);

		menuOptions[1] = difficultyOptions[game->getDifficulty()];

		for (count = 0; count < 6; count++) {

			if (count == option) fontmn2->mapPalette(240, 8, selectedTextPalIndex, textPalSpan);
			else fontmn2->mapPalette(240, 8, textPalIndex, textPalSpan);

			fontmn2->showString(menuOptions[count], canvasW >> 2, (canvasH >> 1) + (count << 4) - 46);

		}

		fontmn2->restorePalette();

	}

	return;

}


/**
 * Process in-game menu selection.
 *
 * @param menu Whether or not the level menu should be displayed
 * @param option Chosen menu option
 *
 * @return Error code
 */
int Level::select (bool& menu, int option) {

	bool wasSlow;

	switch (option) {

		case 0: // Continue

			menu = false;

		case 1: // Change difficulty

			game->setDifficulty((game->getDifficulty() + 1) & 3);

			break;

		case 2: // Save

			break;

		case 3: // Load

			break;

		case 4: // Setup
				wasSlow = setup.slowMotion;

				if (setupMenu.setupMain() == E_QUIT) return E_QUIT;

				if (wasSlow && !setup.slowMotion) steps <<= 1;
				else if (!wasSlow && setup.slowMotion) steps >>= 1;

				// Restore level palette
				video.setPalette(palette);
			break;

		case 5: // Quit game

			return E_RETURN;

	}

	return E_NONE;

}

/**
 * Process iteration.
 *
 * @param menu Whether or not the level menu should be displayed
 * @param option Selected menu uption
 * @param message Whether or not the "paused" message is being displayed
 *
 * @return Error code
 */
int Level::loop (bool& menu, int& option, bool& message) {

	int ret, x, y;


	// Main loop
	if (::loop(NORMAL_LOOP, paletteEffects) == E_QUIT) return E_QUIT;


	if (controls.release(C_ESCAPE)) {

		menu = !menu;
		option = 0;

	}

	if (controls.release(C_PAUSE)) message = !message;

	if (controls.release(C_STATS)) {

		stats ^= S_SCREEN;

	}

	if (menu) {

		// Deal with menu controls

		if (controls.release(C_UP)) option = (option + 5) % 6;

		if (controls.release(C_DOWN)) option = (option + 1) % 6;

		if (controls.release(C_ENTER)) {

			ret = select(menu, option);

			if (ret < 0) return ret;

		}

		/*if (controls.getCursor(x, y)) {

			x -= canvasW >> 2;
			y -= (canvasH >> 1) - 46;

			if ((x >= 0) && (x < 128) && (y >= 0) && (y < 96)) {

				option = y >> 4;

				if (controls.wasCursorReleased()) {

					ret = select(menu, option);

					if (ret < 0) return ret;

				}

			} else if (controls.wasCursorReleased()) menu = false;

		}*/

	}
#if !(ANDROID)
	else {

		//if (controls.wasCursorReleased()) menu = true;

	}
#endif

	paused = message || menu;

	timeCalcs();

	return E_NONE;

}



/**
 * Add extra time.
 *
 * @param seconds Number of seconds to add
 */
void Level::addTimer (int seconds) {

	if (stage != LS_NORMAL) return;

	endTime += seconds * 1000;

	if (endTime >= ticks + (10 * 60 * 1000))
		endTime = ticks + (10 * 60 * 1000) - 1;


}


/**
 * Set the level stage.
 *
 * @param newStage New level stage
 */
void Level::setStage (LevelStage newStage) {

	if (stage == newStage) return;

	stage = newStage;

}


/**
 * Determine the current level stage.
 *
 * @return The current level stage.
 */
LevelStage Level::getStage () {

	return stage;

}

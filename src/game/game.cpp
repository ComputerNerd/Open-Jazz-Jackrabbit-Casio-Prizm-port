
/**
 *
 * @file game.cpp
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 23rd August 2005: Created level.c and menu.c
 * 3rd of February 2009: Renamed level.c to level.cpp and menu.c to menu.cpp
 * 9th March 2009: Created game.cpp from parts of menu.cpp and level.cpp
 * 3rd June 2009: Created network.cpp from parts of game.cpp
 * 18th July 2009: Created servergame.cpp from parts of game.cpp
 * 18th July 2009: Created clientgame.cpp from parts of game.cpp
 * 3rd October 2010: Created localgame.cpp from parts of game.cpp
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
 */


#include "game.h"
#include "gamemode.h"

#include "io/gfx/video.h"
//#include "io/sound.h"
#include "jj1bonuslevel/jj1bonuslevel.h"
#include "jj1level/jj1level.h"
#include "jj1planet/jj1planet.h"
//#include "jj2level/jj2level.h"
#include "player/player.h"
#include "util.h"

#include <string.h>
#ifdef CASIO
#include "strcasecmp.h"
#include "strncasecmp.h"
#include <fxcg/display.h>
#include <fxcg/keyboard.h>
#include "platforms/casio.h"
#include <fxcg/heap.h>
#define free sys_free
#define malloc sys_malloc
#endif
inline void *operator new(size_t, void *p) throw ()
    {
    return p;
    }
/**
 * Create base game
 */
Game::Game () {

	levelFile = NULL;
	levelType = LT_JJ1;

	players = NULL;
	baseLevel = NULL;

	return;

}


/**
 * Destroy game
 */
Game::~Game () {

	if (levelFile) delete[] levelFile;

	if (players) delete[] players;
	localPlayer = NULL;

	return;

}


/**
 * Create a new game mode
 *
 * @param modeType The mode to create
 *
 * @return The new game mode (NULL on failure)
 */
GameMode* Game::createMode (GameModeType modeType) {

	switch (modeType) {

		case M_SINGLE:

			return new SingleGameMode();

		case M_COOP:

			return new CoopGameMode();

		case M_BATTLE:

			return new BattleGameMode();

		case M_TEAMBATTLE:

			return new TeamBattleGameMode();

		case M_RACE:

			return new RaceGameMode();

	}

	return NULL;

}


/**
 * Get the game's mode
 *
 * @return The game's mode
 */
GameMode* Game::getMode () {

	return mode;

}


/**
 * Get the game's difficulty
 *
 * @return The game's difficulty
 */
int Game::getDifficulty () {

	return difficulty;

}


/**
 * Set the game's difficulty
 */
void Game::setDifficulty (int diff) {

	difficulty = diff;

	return;

}
#ifdef CASIO
extern unsigned char * SaveVramAddr;
#define uintptr_t unsigned
#else
extern unsigned char SaveVramAddr[];
#endif
/**
 * Play a level.
 *
 * @return Error code
 */
unsigned char * alignPtr(uintptr_t p) {
	if (p & (sizeof(uintptr_t) - 1))
		p += sizeof(uintptr_t) - (p & (sizeof(uintptr_t) - 1));
	return (unsigned char *)p;
}
int Game::playLevel (char* fileName, bool intro, bool checkpoint) {
	int ret;
	if (!strncasecmp(fileName, "MACRO", 5)) {
		#ifdef CASIO
			DmaWaitNext();
			clearLine(2);
			PrintXY(1,2,"Game::playLevel M"-2,0x20,TEXT_COLOR_WHITE);
			Bdisp_PutDisp_DD();
			{int tKEY;
			GetKey(&tKEY);}
		#endif
		// Load and play the level
/*
		try {

			baseLevel = level = new JJ1DemoLevel(this, fileName);

		} catch (int e) {

			return e;

		}

		ret = level->play();

		delete level;
		baseLevel = level = NULL;
*/
	} else if (levelType == LT_JJ1BONUS) {
		#ifdef CASIO
			drawStrL(2,"Game::playLevel 2");
		#endif
		JJ1BonusLevel *bonus;
		baseLevel = bonus = new(SaveVramAddr) JJ1BonusLevel(this, fileName);
		ret = bonus->play();
		bonus->~JJ1BonusLevel();

		baseLevel = NULL;

	} /*else if (levelType == LT_JJ2) {

		try {

			baseLevel = jj2Level = new JJ2Level(this, fileName, checkpoint, multiplayer);

		} catch (int e) {

			return e;

		}

		ret = jj2Level->play();

		delete jj2Level;
		baseLevel = jj2Level = NULL;

	}*/else {
		#ifdef CASIO
			drawStrL(2,"Game::playLevel 3");
		#endif
		baseLevel = level = new(SaveVramAddr) JJ1Level(this, fileName, checkpoint);
		if (intro) {

			JJ1Planet *planet;
			char *planetFileName = NULL;

			planetFileName = createFileName(F_PLANET, level->getWorld());
			void * PlanetMem=malloc(sizeof(JJ1Planet));
			if(!PlanetMem){
				#ifdef CASIO
					casioQuitM("JJ1Planet");
				#else
					puts("Malloc error JJ1Planet");
				#endif
			}
			try {

				planet = new(PlanetMem) JJ1Planet(planetFileName, planetId);

			} catch (int e) {
				free(PlanetMem);
				planet = NULL;

			}

			delete[] planetFileName;

			if (planet) {
				if (planet->play() == E_QUIT){
					delete planet;
					//delete level;
					level->~JJ1Level();//allocated differently
					return E_QUIT;
				}
				planetId = planet->getId();
				planet->~JJ1Planet();
				free(PlanetMem);
			}
		}
		ret = level->play();
		level->~JJ1Level();

		baseLevel = level = NULL;
		if (ret == WON_WITH_BONUS) {
			char * bonusFile = createFileName(F_BONUSMAP, 0);
			LevelType oldLevelType = levelType; 
			playLevel(bonusFile);
			levelType = oldLevelType;
			delete[] bonusFile;
			ret = WON;
		}

	}

	return ret;

}


/**
 * Determine the type of the specified level file.
 *
 * @return Level type
 */
LevelType Game::getLevelType (char* fileName) {

	int length;

	length = strlen(fileName);

	if ((length > 4) && !strcasecmp(fileName + length - 4, ".j2l")) return LT_JJ2;
	if (!strncasecmp(fileName, F_BONUSMAP, 8)) return LT_JJ1BONUS;
	return LT_JJ1;

}


/**
 * Play a level.
 *
 * @return Error code
 */
int Game::playLevel (char* fileName) {

	levelType = getLevelType(fileName);

	return playLevel(fileName, false, false);

}


/**
 * Play the game
 *
 * @return Error code
 */
int Game::play () {

	bool multiplayer;
	bool checkpoint;
	int ret;
	multiplayer = (mode->getMode() != M_SINGLE);
	checkpoint = false;
	planetId = -1;
	// Play the level(s)
	while (true) {

		if (!levelFile) return E_NONE;

		sendTime = checkTime = 0;

		// Load and play the level

		ret = playLevel(levelFile, !multiplayer, checkpoint);

		if (ret <= 0) return ret;

		if (levelFile && !strncasecmp(levelFile, F_BONUSMAP, 8)) {

			if (ret == WON) {

				char *fileName;

				// Go to next level
				fileName = createFileName(F_BONUSMAP, (levelFile[10] * 10) + levelFile[11] - 527);
				setLevel(fileName);
				delete[] fileName;

			}

		} else {

			if (ret == WON) {

				// Won the level

				// Do not use old level's checkpoint coordinates
				checkpoint = false;

			} else {

				// Lost the level

				if (!localPlayer->getLives()) return E_NONE;

				// Use checkpoint coordinates
				checkpoint = true;

			}

		}

	}

	return E_NONE;

}


/**
 * Move the viewport towards the exit sign
 *
 * @param change Distance to move
 */
void Game::view (int change) {

	if (TTOF(checkX) > viewX + (canvasW << 9) + change) viewX += change;
	else if (TTOF(checkX) < viewX + (canvasW << 9) - change) viewX -= change;

	if (TTOF(checkY) > viewY + (canvasH << 9) + change) viewY += change;
	else if (TTOF(checkY) < viewY + (canvasH << 9) - change) viewY -= change;

	return;

}


/**
 * Make a player restart the level from the beginning/last checkpoint
 *
 * @param player Player to reset
 */
void Game::resetPlayer (Player *player) {

	player->reset(checkX, checkY);

	return;

}


/**
 * Create a level player
 *
 * @param player Player for which to create the level player
 */
void Game::addLevelPlayer (Player *player) {

	int count;

	if (level) {

		Anim* pAnims[JJ1PANIMS];

		for (count = 0; count < JJ1PANIMS; count++) pAnims[count] = level->getPlayerAnim(count);

		player->createLevelPlayer(levelType, pAnims, NULL, checkX, checkY);

	}/* else if (jj2Level) {

		Anim* pAnims[JJ2PANIMS];
		Anim* pFlippedAnims[JJ2PANIMS];

		for (count = 0; count < JJ2PANIMS; count++) {

			pAnims[count] = jj2Level->getPlayerAnim(0, count, false);
			pFlippedAnims[count] = jj2Level->getPlayerAnim(0, count, true);

		}

		player->createLevelPlayer(levelType, pAnims, pFlippedAnims, checkX, checkY);

	}*/
}

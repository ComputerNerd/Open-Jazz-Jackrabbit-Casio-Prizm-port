
/**
 *
 * @file gamemenu.cpp
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 23rd of August 2005: Created menu.c
 * 3rd of February 2009: Renamed menu.c to menu.cpp
 * 18th July 2009: Created menugame.cpp from parts of menu.cpp
 * 26th July 2009: Renamed menugame.cpp to gamemenu.cpp
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
 * Deals with the running of the menus used to create a new game.
 *
 */


#include "menu.h"

#include "game/game.h"
#include "game/gamemode.h"
#include "io/controls.h"
#include "io/gfx/font.h"
#include "io/gfx/video.h"
//#include "io/sound.h"
#include "loop.h"
#include "util.h"
#include "mem.h"
#include "surface.h"
#ifdef CASIO
	#include "platforms/casio.h"
	#include <fxcg/keyboard.h>
	#include <fxcg/display.h>
	#include <fxcg/misc.h>
#endif
/**
 * Create the game menu.
 *
 * @param file File containing menu graphics
 */
void GameMenu::loadDifficulty(File *file){
	// Load the difficulty graphics
	#ifdef CASIO
		drawStrL(1,"Loading difficulty");
	#endif
	file->loadPalette(menuPalette);
	//difficultyScreen = file->loadSurface(SW, SH);
	addobj(SW*SH,&difficultyScreenid);
	file->loadMiniSurface(SW,SH,(unsigned char *)objs[difficultyScreenid].ptr,&difficultyScreen);
	//SDL_SetColorKey(difficultyScreen, SDL_SRCCOLORKEY, 0);
	setColKey(&difficultyScreen,0);
	// Default difficulty setting
}
void GameMenu::loadEpisodes(File *file){
	int count, col;
	// Load the episode pictures (max. 10 episodes + bonus level)
	// Load their palette
	#ifdef CASIO
		drawStrL(1,"Loading thumbnails");
	#endif
	{
		unsigned char palette6[768];
		file->loadPalette6(palette6);
		file->convertPalette(palette, palette6);
		// Generate a greyscale mapping
		count=768;
		while(count -= 3){

			col = (palette6[count] + palette6[count + 1] + palette6[count + 2]) / 3;

			greyPalette[count / 3]=nearestIndex(col,col,col, palette6, 256);

		}
	}
	episodes = 11;

	for (count = 0; count < 11; count++){
		//episodeScreens[count] = file->loadSurface(134, 110);
		#ifdef CASIO
			char tMp[8];
			itoa(count,(unsigned char *)tMp);
			drawStrL(2,(const char *)tMp);
		#endif
		addobj(134*110,&episodeScreensid[count]);
		file->loadMiniSurface(134,110,(unsigned char *)objs[episodeScreensid[count]].ptr,&episodeScreens[count]);
		if (file->tell() >= file->getSize()) {
			episodes = ++count;
			for (; count < 11; count++) {
				episodeScreensid[count]=INVALID_OBJ;
				initMiniSurface(&episodeScreens[count],NULL,1,1);
				//pixel = 0;
				//episodeScreens[count] = createSurface(&pixel, 1, 1);
			}
		}
	}
}
GameMenu::GameMenu (File *file) {
	//unsigned char pixel;
	
	loadDifficulty(file);
	difficulty = 1;
	loadEpisodes(file);
}


/**
 * Delete the game menu.
 */
GameMenu::~GameMenu () {
	int count=11;
	while(count--){
		if(episodeScreensid[count]!=INVALID_OBJ)
			freeobj(episodeScreensid[count]);
	}

	if(difficultyScreenid!=INVALID_OBJ)
		freeobj(difficultyScreenid);
	//SDL_FreeSurface(difficultyScreen);

	return;

}


/**
 * Create and play a new game.
 *
 * @param mode Game mode
 * @param firstLevel First level's file name
 *
 * @return Error code
 */
int GameMenu::playNewGame (GameModeType mode, char* firstLevel) {

	Game* game;
	int ret;

	//playSound(S_ORB);
	if (mode == M_SINGLE) {

		try {

			game = new LocalGame(firstLevel, difficulty);

		} catch (int e) {

			if (message("COULD NOT START GAME") == E_QUIT) return E_QUIT;

			return e;

		}

	}else
		if (message("Not supported") == E_QUIT) return E_QUIT;


	// Play the level(s)

	ret = game->play();

	delete game;

	//if (ret != E_QUIT) playMusic("menusng.psm");

	switch (ret) {

		case E_QUIT:

			return E_QUIT;

		case E_FILE:

			return message("FILE NOT FOUND");

	}

	return E_NONE;

}


/**
 * Run the new game difficulty menu.
 *
 * @param mode Game mode
 * @param firstLevel First level's file name
 *
 * @return Error code
 */
static const char *const optionsDifficulty[4] = {"easy", "medium", "hard", "turbo"};
int GameMenu::newGameDifficulty (GameModeType mode, char* firstLevel) {

	
	//SDL_Rect src, dst;
	short src[4]; //x y w h
	int x, y, count;

	if(difficultyScreenid==INVALID_OBJ){
		File * file=skipLogos();
		loadDifficulty(file);
		delete file;
	}
	video.setPalette(menuPalette);
	while (true) {

		if (loop(NORMAL_LOOP) == E_QUIT) return E_QUIT;

		if (controls.release(C_ESCAPE)) return E_NONE;

		if (controls.release(C_UP)) difficulty = (difficulty + 3) % 4;

		if (controls.release(C_DOWN)) difficulty = (difficulty + 1) % 4;

		if (controls.release(C_ENTER)){
			int objCount=11;
			while(objCount--){
				if(episodeScreensid[objCount]!=INVALID_OBJ) {
					freeobj(episodeScreensid[objCount]);
					episodeScreensid[objCount] = INVALID_OBJ;
				}
			}
			if(difficultyScreenid!=INVALID_OBJ){
				freeobj(difficultyScreenid);
				difficultyScreenid=INVALID_OBJ;
			}
			return playNewGame(mode, firstLevel);
		}

		/*if (controls.getCursor(x, y)) {
			if ((x < 100) && (y >= canvasH - 12) && controls.wasCursorReleased()) return E_NONE;
			x -= canvasW >> 2;
			y -= (canvasH >> 1) - 32;
			if ((x >= 0) && (x < 256) && (y >= 0) && (y < 64)) {
				difficulty = y >> 4;
				if (controls.wasCursorReleased()) return playNewGame(mode, firstLevel);
			}
		}*/
		#ifdef CASIO
			//casioDelay(T_FRAME);
		#else
			SDL_Delay(T_FRAME);
		#endif
		video.clearScreen(0);

		for (count = 0; count < 4; count++) {

			if (count == difficulty) fontmn2->mapPalette(240, 8, 114, 16);

			fontmn2->showString(optionsDifficulty[count], canvasW >> 2,
				(canvasH >> 1) + (count << 4) - 32);

			if (count == difficulty) fontmn2->restorePalette();

		}

		src[0] = (difficulty & 1) * 160;
		src[1] = (difficulty & 2) * 50;
		src[2] = 160;
		src[3] = 100;
		//dst.x = (canvasW >> 1) - 40;
		//dst.y = (canvasH >> 1) - 50;
		//SDL_BlitSurface(difficultyScreen, &src, canvas, &dst);
		blitPartToCanvas(&difficultyScreen,(canvasW >> 1) - 40,(canvasH >> 1) - 50,src);
		showEscString();
	}
	return E_NONE;
}


/**
 * Run the new game difficulty menu.
 *
 * @param mode Game mode
 * @param levelNum First level's number
 * @param levelNum First level's world number
 *
 * @return Error code
 */
int GameMenu::newGameDifficulty(GameModeType mode, int levelNum, int worldNum){
	char* firstLevel;
	int ret;
	if (levelNum == -1) firstLevel = createFileName(F_BONUSMAP, worldNum);
	else firstLevel = createFileName(F_LEVEL, levelNum, worldNum);
	ret = newGameDifficulty(mode, firstLevel);
	delete[] firstLevel;
	return ret;
}
/**
 * Run the game loading menu.
 *
 * @return Error code
 */
int GameMenu::loadGame () {

	/// @todo Actual loading of saved games

	int option, worldNum, levelNum, x, y;

	worldNum = levelNum = option = 0;

	video.setPalette(menuPalette);

	while (true) {

		if (loop(NORMAL_LOOP) == E_QUIT) return E_QUIT;

		if (controls.release(C_ESCAPE)) return E_NONE;

		if (controls.release(C_UP)) option ^= 1;

		if (controls.release(C_DOWN)) option ^= 1;

		if (controls.release(C_LEFT)) {

			if (option) levelNum = ((levelNum + 11) % 11) - 1;
			else worldNum = (worldNum + 999) % 1000;

		}

		if (controls.release(C_RIGHT)) {

			if (option) levelNum = ((levelNum + 2) % 11) - 1;
			else worldNum = (worldNum + 1) % 1000;

		}

		if (controls.release(C_ENTER)) {

			//playSound(S_ORB);

			if (newGameDifficulty(M_SINGLE, levelNum, worldNum) == E_QUIT) return E_QUIT;

			video.setPalette(menuPalette);

		}

		/*if (controls.getCursor(x, y)) {
			if ((x < 100) && (y >= canvasH - 12) && controls.wasCursorReleased()) return E_NONE;
			if (y < (canvasH >> 1)) option = 0;
			else option = 1;
		}*/

		#ifdef CASIO
			//casioDelay(T_FRAME);
		#else
			SDL_Delay(T_FRAME);
		#endif
		video.clearScreen(15);

		if (option == 0) fontmn2->mapPalette(240, 8, 114, 16);
		fontmn2->showString("choose world:", 32, canvasH / 3);
		fontmn2->showNumber(worldNum, 208, canvasH / 3);

		if (option == 0) fontmn2->restorePalette();
		else fontmn2->mapPalette(240, 8, 114, 16);

		fontmn2->showString("choose level:", 32, (canvasH << 1) / 3);
		if (levelNum >= 0) fontmn2->showNumber(levelNum, 208, (canvasH << 1) / 3);
		else fontmn2->showString("bonus", 172, (canvasH << 1) / 3);

		if (option != 0) fontmn2->restorePalette();

		showEscString();

	}

	return E_NONE;

}


/**
 * Run the new game level selection menu.
 *
 * @param mode Game mode
 *
 * @return Error code
 */
int GameMenu::newGameLevel (GameModeType mode) {

	char* fileName;
	int ret;

	fileName = createString("level0.000");

	ret = E_NONE;

	while (true) {

		ret = textInput("level file name:", fileName);

		if (ret < 0) break;

		ret = newGameDifficulty(mode, fileName);

		if (ret < 0) break;

	}

	delete[] fileName;

	return ret;

}

File * GameMenu::skipLogos(){
	File *file;
	// Load the menu graphics
	try {
		file = new File(F_MENU, false);
	} catch (int e) {
		//SDL_FreeSurface(logo);
		throw e;
	}
	if (file->getSize() > 200000){
		file->skipRLE();
		file->skipRLE();
		file->skipRLE();
		file->skipRLE();
		file->skipRLE();
		file->skipRLE();
	} else{
		file->skipRLE();
		file->skipRLE();
		file->skipRLE();
	}
	return file;
}

/**
 * Run the appropriate menu for the given episode selection.
 *
 * @param mode Game mode
 * @param episode Episode number
 *
 * @return Error code
 */
int GameMenu::selectEpisode (GameModeType mode, int episode) {
	int worldNum;
	int count=11;
	while(count--){
		if(episodeScreensid[count]!=INVALID_OBJ) {
			freeobj(episodeScreensid[count]);
			episodeScreensid[count] = INVALID_OBJ;
		}
	}
	//playSound(S_ORB);

	if (episode < 10) {

		if (episode < 6) worldNum = episode * 3;
		else if ((episode >= 6) && (episode < 9)) worldNum = (episode + 4) * 3;
		else worldNum = 50;

		if (newGameDifficulty(mode, 0, worldNum) == E_QUIT) return E_QUIT;

	} else if (episode == 10) {

		if (newGameDifficulty(mode, -1, 0) == E_QUIT) return E_QUIT;

	} else {
		freeobj(difficultyScreenid);
		difficultyScreenid = INVALID_OBJ;
		if (newGameLevel(mode) == E_QUIT) return E_QUIT;

	}
	video.setPalette(palette);
	File * file=skipLogos();
	if(difficultyScreenid==INVALID_OBJ)
		loadDifficulty(file);
	else{
		file->skipRLE();
		file->skipRLE();
	}
	loadEpisodes(file);
	delete file;
	return E_NONE;

}


/**
 * Run the new game episode menu.
 *
 * @param mode Game mode
 *
 * @return Error code
 */
static const char *const optionsEpisode[12] = {"episode 1", "episode 2", "episode 3",
		"episode 4", "episode 5", "episode 6", "episode a", "episode b",
		"episode c", "episode x", "bonus stage", "specific level"};
int GameMenu::newGameEpisode (GameModeType mode) {

	
	bool exists[12];
	char *check;
	//SDL_Rect dst;
	int episode, count, x, y;

	video.setPalette(palette);

	for (count = 0; count < 10; count++) {

		if (count < 6) x = count * 3;
		else if ((count >= 6) && (count < 9)) x = (count + 4) * 3;
		else x = 50;

		check = createFileName(F_LEVEL, 0, x);
		exists[count] = fileExists(check);
		delete[] check;

		/*if (exists[count]) episodeScreens[count].flags&=~(miniS_useMap);
		else episodeScreens[count].flags|=miniS_useMap;*/

	}

	if (mode == M_SINGLE) {

		check = createFileName(F_BONUSMAP, 0);
		exists[10] = fileExists(check);
		delete[] check;

	} else exists[10] = false;

	exists[11] = true;

	episode = 0;

	while (true) {

		if (loop(NORMAL_LOOP) == E_QUIT) return E_QUIT;

		if (controls.release(C_ESCAPE)) return E_NONE;

		if (controls.release(C_UP)) episode = (episode + 11) % 12;

		if (controls.release(C_DOWN)) episode = (episode + 1) % 12;

		if (controls.release(C_ENTER) && exists[episode]) {

			count = selectEpisode(mode, episode);

			if (count < 0) return count;

		}

		/*if (controls.getCursor(x, y)) {
			if ((x >= canvasW - 100) && (y >= canvasH - 12) && controls.wasCursorReleased()) return E_NONE;
			x -= canvasW >> 3;
			y -= (canvasH >> 1) - 92;
			if ((x >= 0) && (x < 256) && (y >= 0) && (y < 192)) {
				episode = y >> 4;
				if (controls.wasCursorReleased() && exists[episode]) {
					count = selectEpisode(mode, episode);
					if (count < 0) return count;
				}
			}
		}*/

		#ifdef CASIO
			//casioDelay(T_FRAME);
		#else
			SDL_Delay(T_FRAME);
		#endif
		video.clearScreen(0);

		//dst.x = canvasW - 144;
		//dst.y = (canvasH - 110) >> 1;

		int episodeIdx;

		if ((episode < episodes - 1) || (episode < 6)) {
			episodeIdx = episode;
		} else if ((episode == 10) && (episodes > 6)) {
			episodeIdx = episode - 1;
		}

		if (exists[episodeIdx])
			blitToCanvas(&episodeScreens[episodeIdx],canvasW - 144,(canvasH - 110) >> 1);
		else
			blitToCanvasRemap(&episodeScreens[episodeIdx],canvasW - 144,(canvasH - 110) >> 1, greyPalette);

		for (count = 0; count < 12; count++) {

			if (count == episode) {

				fontmn2->mapPalette(240, 8, 79, -80);
				drawRect((canvasW >> 3) - 4, (canvasH >> 1) + (count << 4) - 94,
					136, 15, 79);

			} else if (!exists[count])
				fontmn2->mapPalette(240, 8, 94, -16);

			fontmn2->showString(optionsEpisode[count], canvasW >> 3,
				(canvasH >> 1) + (count << 4) - 92);

			if ((count == episode) || (!exists[count]))
				fontmn2->mapPalette(240, 8, 9, 80);

		}

		fontbig->showString(ESCAPE_STRING, canvasW - 100, canvasH - 12);

	}

	return E_NONE;

}



/**
 * Run the new game menu.
 *
 * @return Error code
 */
int GameMenu::newGame() {
	difficultyScreen.pix=(unsigned char *)objs[difficultyScreenid].ptr;
	int esc=11;
	while(esc--){
		if(episodeScreensid[esc]!=INVALID_OBJ){
			episodeScreens[esc].pix=(unsigned char *)objs[episodeScreensid[esc]].ptr;
		}
	}
	return newGameEpisode(M_SINGLE);

}


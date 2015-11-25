
/**
 *
 * @file setupmenu.cpp
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 23rd of August 2005: Created menu.c
 * 3rd of February 2009: Renamed menu.c to menu.cpp
 * 18th July 2009: Created menusetup.cpp from parts of menu.cpp
 * 26th July 2009: Renamed menusetup.cpp to setupmenu.cpp
 * 21st July 2013: Created setup.cpp from parts of main.cpp and setupmenu.cpp
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
 * Deals with the running of setup menus.
 *
 */


#include "menu.h"

#include "io/controls.h"
#include "io/gfx/font.h"
#include "io/gfx/video.h"
//#include "io/sound.h"
#include "player/player.h"
#include "loop.h"
#include "setup.h"
#include "util.h"
#ifdef CASIO
	#include "platforms/casio.h"
#endif

/**
 * Run the keyboard setup menu.
 *
 * @return Error code
 */
static const char *options[PCONTROLS] = {"up", "down", "left", "right", "jump", "swim up", "fire", "weapon"};
int SetupMenu::setupKeyboard () {

	int progress, character, x, y, count;

	progress = 0;

	while (true) {

		character = loop(SET_KEY_LOOP);

		if (character == E_QUIT) return E_QUIT;

		if (character == controls.getKey(C_ESCAPE)) return E_NONE;

		/*if (controls.getCursor(x, y) &&
			(x < 100) && (y >= canvasH - 12) &&
			controls.wasCursorReleased()) return E_NONE;*/

		if (character > 0) {

			// If this is a navigation controls (up, down, or enter),
			// make sure it's not the same as other navigation controls

			if (((progress != C_UP) &&
				(progress != C_DOWN) &&
				(progress != C_ENTER)) ||
				(controls.getKey(progress) == character) ||
				((controls.getKey(C_UP) != character) &&
				(controls.getKey(C_DOWN) != character) &&
				(controls.getKey(C_ENTER) != character))) {

				controls.setKey(progress, character);
				progress++;

				if (progress == PCONTROLS) {

					// If all controls have been assigned, return

					//playSound(S_ORB);

					return E_NONE;

				}

			}

		}


		#ifdef CASIO
			//casioDelay(T_FRAME);
		#else
			SDL_Delay(T_FRAME);
		#endif

		video.clearScreen(0);

		for (count = 0; count < PCONTROLS; count++) {

			if (count < progress)
				fontmn2->showString("okay", (canvasW >> 2) + 176,
					(canvasH >> 1) + (count << 4) - 56);

			else if (count == progress) fontmn2->mapPalette(240, 8, 114, 16);

			fontmn2->showString(options[count], canvasW >> 2,
				(canvasH >> 1) + (count << 4) - 56);

			if (count == progress) {

				fontmn2->showString("press key", (canvasW >> 2) + 112,
					(canvasH >> 1) + (count << 4) - 56);

				fontmn2->restorePalette();

			}

		}

		showEscString();

	}

	return E_NONE;

}




#ifdef SCALE
/**
 * Run the scaling setup menu.
 *
 * @return Error code
 */
int SetupMenu::setupScaling () {

	int scaleFactor, x, y;

	scaleFactor = video.getScaleFactor();

	if ( scaleFactor < MIN_SCALE || scaleFactor > MAX_SCALE )
		scaleFactor = 1;

	while (true) {

		if (loop(NORMAL_LOOP) == E_QUIT) return E_QUIT;

		if (controls.release(C_ESCAPE)) return E_NONE;

		if (controls.release(C_ENTER)) return E_NONE;

		/*if (controls.getCursor(x, y) &&
			(x >= 32) && (x < 132) && (y >= canvasH - 12) &&
			controls.wasCursorReleased()) return E_NONE;*/

		SDL_Delay(T_FRAME);

		video.clearScreen(0);


		// Show screen corners
		drawRect(0, 0, 32, 32, 79);
		drawRect(canvasW - 32, 0, 32, 32, 79);
		drawRect(canvasW - 32, canvasH - 32, 32, 32, 79);
		drawRect(0, canvasH - 32, 32, 32, 79);



		fontmn2->mapPalette(240, 8, 114, 16);

		// Scale
		fontmn2->showNumber(video.getScaleFactor(), (canvasW >> 2) + 32, canvasH >> 1);

		// X
		fontmn2->showString("x", (canvasW >> 2) + 40, canvasH >> 1);

		fontmn2->restorePalette();


		if ((controls.release(C_DOWN) || controls.release(C_LEFT)) && (scaleFactor > MIN_SCALE)) scaleFactor--;

		if ((controls.release(C_UP) || controls.release(C_RIGHT)) && (scaleFactor < MAX_SCALE)) scaleFactor++;

		// Check for a scaling change
		if (scaleFactor != video.getScaleFactor()) {

			//playSound(S_ORB);
			scaleFactor = video.setScaleFactor(scaleFactor);

		}

		fontbig->showString(ESCAPE_STRING, 35, canvasH - 12);

	}

	return E_NONE;

}
#endif



/**
 * Run the setup menu.
 *
 * @return Error code
 */
static const char*const setupOptions[7] = {"character", "keyboard", "joystick", "resolution", "scaling", "sound", "gameplay"};
static const char*const setupCharacterOptions[5] = {"name", "fur", "bandana", "gun", "wristband"};
static const char*const setupCharacterColOptions[8] = {"white", "red", "orange", "yellow", "green", "blue", "animation 1", "animation 2"};
static const unsigned char setupCharacterCols[8] = {PC_GREY, PC_RED, PC_ORANGE, PC_YELLOW, PC_LGREEN, PC_BLUE, PC_SANIM, PC_LANIM};
static const char*const setupModsOff[3] = {"slow motion off", "take extra items", "one-bird limit"};
static const char*const setupModsOn[3] = {"slow motion on", "leave extra items", "unlimited birds"};
int SetupMenu::setupMain(){
	const char* setupMods[3];
	int ret;
	int option, suboption, subsuboption;

	option = 0;

	setupMods[0] = (setup.slowMotion? setupModsOn[0]: setupModsOff[0]);
	setupMods[1] = (setup.leaveUnneeded? setupModsOn[1]: setupModsOff[1]);
	setupMods[2] = (setup.manyBirds? setupModsOn[2]: setupModsOff[2]);

	video.setPalette(menuPalette);

	while (true) {

		ret = generic(setupOptions, 7, option);

		if (ret == E_RETURN) return E_NONE;
		if (ret < 0) return ret;

		switch (option) {

			case 0:

				suboption = 0;

				while (true) {

					ret = generic(setupCharacterOptions, 5, suboption);

					if (ret == E_QUIT) return E_QUIT;
					if (ret < 0) break;

					switch (suboption) {

						case 0: // Character name

							if (textInput("character name:", setup.characterName) == E_QUIT) return E_QUIT;

							break;

						default: // Character colour

							subsuboption = 0;
							ret = generic(setupCharacterColOptions, 8, subsuboption);

							if (ret == E_QUIT) return E_QUIT;

							if (ret == E_NONE)
								setup.characterCols[suboption - 1] = setupCharacterCols[subsuboption];

							break;

					}

				}

				break;

			case 1:

#if !defined(CAANOO) && !defined(WIZ) && !defined(GP2X)
				if (setupKeyboard() == E_QUIT) return E_QUIT;
#else
				if (message("FEATURE NOT AVAILABLE") == E_QUIT) return E_QUIT;
#endif

				break;

			case 2:


				if (message("FEATURE NOT AVAILABLE") == E_QUIT) return E_QUIT;
				break;

			case 3:
				if (message("FEATURE NOT AVAILABLE") == E_QUIT) return E_QUIT;
				break;
			case 4:

#ifdef SCALE
				if (setupScaling() == E_QUIT) return E_QUIT;
#else
				if (message("FEATURE NOT AVAILABLE") == E_QUIT) return E_QUIT;
#endif

				break;

			case 5:

				//if (setupSound() == E_QUIT) return E_QUIT;
				if (message("FEATURE NOT AVAILABLE") == E_QUIT) return E_QUIT;
				break;

			case 6:

				suboption = 0;

				while (true) {

					ret = generic(setupMods, 3, suboption);

					if (ret == E_QUIT) return E_QUIT;
					if (ret < 0) break;

					if (setupMods[suboption] == setupModsOff[suboption])
						setupMods[suboption] = setupModsOn[suboption];
					else
						setupMods[suboption] = setupModsOff[suboption];

					setup.slowMotion = (setupMods[0] == setupModsOn[0]);
					setup.leaveUnneeded = (setupMods[1] == setupModsOn[1]);
					setup.manyBirds = (setupMods[2] == setupModsOn[2]);

				}

				break;

		}

	}

	return E_NONE;

}




/**
 *
 * @file setup.cpp
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 23rd of August 2005: Created main.c and menu.c
 * 3rd of February 2009: Renamed main.c to main.cpp and menu.c to menu.cpp
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


#include "io/controls.h"
#include "io/file.h"
#include "io/gfx/video.h"
#include "player/player.h"
#include "setup.h"
#include "util.h"


/**
 * Create default setup
 */
Setup::Setup () {

	// Create the player's name
	characterName = createEditableString(CHAR_NAME);

	// Assign the player's colour
	characterCols[0] = CHAR_FUR;
	characterCols[1] = CHAR_BAND;
	characterCols[2] = CHAR_GUN;
	characterCols[3] = CHAR_WBAND;

	return;

}


/**
 * Delete the setup data
 */
Setup::~Setup () {

	delete[] characterName;

}


/**
 * Load settings from config file.
 */
void Setup::load (int* videoW, int* videoH, bool* fullscreen, int* videoScale) {

	File* file;
	int count;

	// Open config file

	try {

		file = new File(CONFIG_FILE, false);

	} catch (int e) {

		log("Configuration file not found.");

		return;

	}

	// Check that the config file has the correct version
	if (file->loadChar() != 4) {

		log("Valid configuration file not found.");

		return;

	}


	// Read video settings
	*videoW = file->loadShort(7680);
	*videoH = file->loadShort(4800);
	count = file->loadChar();

	// Read the player's name
	for (count = 0; count < STRING_LENGTH; count++)
		setup.characterName[count] = file->loadChar();

	setup.characterName[STRING_LENGTH] = 0;

	// Read the player's colours
	setup.characterCols[0] = file->loadChar();
	setup.characterCols[1] = file->loadChar();
	setup.characterCols[2] = file->loadChar();
	setup.characterCols[3] = file->loadChar();


	// Read gameplay options
	count = file->loadChar();
	setup.slowMotion = ((count & 4) != 0);
	setup.manyBirds = ((count & 1) != 0);
	setup.leaveUnneeded = ((count & 2) != 0);


	delete file;


	return;

}


/**
 * Save settings to config file.
 */
void Setup::save () {

	File *file;
	int count;

	// Open config file
	try {

		file = new File(CONFIG_FILE, true);

	} catch (int e) {

		file = NULL;

	}

	// Check that the config file was opened
	if (!file) {

		logError("Could not write configuration file",
			"File could not be opened.");

		return;

	}


	// Write the version number
	file->storeChar(4);

	// Write the player's name
	for (count = 0; count < STRING_LENGTH; count++)
		file->storeChar(setup.characterName[count]);

	// Write the player's colour
	file->storeChar(setup.characterCols[0]);
	file->storeChar(setup.characterCols[1]);
	file->storeChar(setup.characterCols[2]);
	file->storeChar(setup.characterCols[3]);

	// Write gameplay options

	count = 0;

	if (setup.slowMotion) count |= 4;
	if (setup.manyBirds) count |= 1;
	if (setup.leaveUnneeded) count |= 2;

	file->storeChar(count);


	delete file;


	return;

}


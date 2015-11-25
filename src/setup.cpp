
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
//#include "io/sound.h"
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




/**
 *
 * @file menu.h
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 23rd August 2005: Created OpenJazz.h
 * 3rd February 2009: Created menu.h from parts of OpenJazz.h
 * 21st July 2013: Created setup.h from parts of menu.h
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
 */


#ifndef _MENU_H
#define _MENU_H


#include "game/gamemode.h"
#include "io/file.h"

#include "OpenJazz.h"
#ifndef CASIO
#include <SDL/SDL.h>
#endif
#include "surface.h"
#include "io/gfx/video.h"
#include "mem.h"
// Constants

#define ESCAPE_STRING "(esc) quits"

// Demo timeout
#define T_DEMO 20000


// Classes

/// Menu base class, providing generic menu screens
class Menu {

	protected:
		void showEscString ();
		int  message       (const char* text);
		int  generic       (const char* const* optionNames, int options, int& chosen);
		int  textInput     (const char* request, char*& text);

};

/// Setup menus
class SetupMenu : public Menu {

	private:
		int setupKeyboard   ();
#ifdef SCALE
		int setupScaling    ();
#endif
		int setupSound      ();

	public:
		int setupMain ();

};

/// New game menus
class GameMenu : public Menu {

	private:
		struct miniSurface		episodeScreens[11]; ///< Episode images
		struct miniSurface		difficultyScreen; ///< 4 difficulty images
		unsigned short			palette[256]; ///< Episode selection palette
		unsigned char			greyPalette[256]; ///< Greyed-out episode selection palette
		int				episodes; ///< Number of episodes
		unsigned 			difficulty; ///< Difficulty setting (0 = easy, 1 = medium, 2 = hard, 3 = turbo (hard in JJ2 levels))
		int playNewGame			(GameModeType mode, char* firstLevel);
		int newGameDifficulty	(GameModeType mode, char* firstLevel);
		int newGameDifficulty	(GameModeType mode, int levelNum, int worldNum);
		int newGameLevel		(GameModeType mode);
		int selectEpisode		(GameModeType mode, int episode);
		int newGameEpisode		(GameModeType mode);
		int joinGame			();
		void loadDifficulty		(File *file);
		void loadEpisodes		(File *file);
	public:
		objid_t					episodeScreensid[11];
		objid_t					difficultyScreenid=INVALID_OBJ;
		GameMenu  (File* file);
		~GameMenu ();
		File * skipLogos();
		int newGame  ();
		int loadGame ();

};
/// Main menu
class MainMenu : public Menu {

	private:
		struct miniSurface	background; ///< Menu image
		struct miniSurface	highlight; ///< Menu image with highlighted text
		struct miniSurface	logo; ///< OJ logo image
		//unsigned char		background_pixels[SW*SH];//SW and SH are constants they are SW=320 SH=200 62.5KB each
		//unsigned char		highlight_pixels[SW*SH];
		objid_t				background_id;
		objid_t				highlight_id;
		GameMenu*			gameMenu; ///< New game menu
		unsigned short		palette[256]; ///< Menu palette

		int select (int option);

	public:
		MainMenu  ();
		~MainMenu ();
		File *loadLogos();
		File *skipLogos();
		int main ();

};


// Variables

EXTERN unsigned short menuPalette[256]; /// Palette used by most menu screens

#endif


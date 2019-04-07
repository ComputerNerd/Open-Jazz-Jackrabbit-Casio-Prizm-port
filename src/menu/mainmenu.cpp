
/**
 *
 * @file mainmenu.cpp
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 23rd of August 2005: Created menu.c
 * 3rd of February 2009: Renamed menu.c to menu.cpp
 * 19th July 2009: Created menumain.cpp from parts of menu.cpp
 * 26th July 2009: Renamed menumain.cpp to mainmenu.cpp
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
 * Deals with the running of the main menu and its generic sub-menus.
 *
 */


#include "menu.h"

#include "game/game.h"
#include "io/controls.h"
#include "io/gfx/video.h"
//#include "io/sound.h"
#include "jj1scene/jj1scene.h"
#include "loop.h"
#include "util.h"
#include "mem.h"
#include <time.h>
#include "jj1level/jj1level.h"
#ifdef CASIO
	#include "platforms/casio.h"
	#include <fxcg/keyboard.h>
	#include <fxcg/display.h>
#endif
#include "menu/OpenJazzLogo.h"

/**
 * Create the main menu.
 */
 
File * MainMenu::skipLogos(){
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
#ifdef CASIO
extern unsigned char * SaveVramAddr;
#endif
File * MainMenu::loadLogos(){
	File *file;
	#ifndef CASIO
	time_t currentTime;
	#endif

	// Load the OpenJazz logo
	initMiniSurface(&logo, (void*)OpenJazzLogoPixels, 64, 40);

	// Load the menu graphics

	try {

		file = new File(F_MENU, false);

	} catch (int e) {

		//SDL_FreeSurface(logo);

		throw e;

	}
	#ifdef CASIO
		drawStrL(2,"Menu");
	#endif
	addobj(SW*SH,&background_id);
	addobj(SW*SH,&highlight_id);
	if (file->getSize() > 200000) {
		#ifndef CASIO
		time(&currentTime);
		#endif
		// In December, load the Christmas menu graphics
		#ifdef CASIO
		if((((*(unsigned char *)0xA413FECC & (1<<4))>>4)*10 + (*(unsigned char *)0xA413FECC & 15))==12){
		#else
		if (localtime(&currentTime)->tm_mon == 11) {
		#endif
			file->skipRLE();
			file->skipRLE();
			file->skipRLE();

			file->loadPalette(palette);
			//background = file->loadSurface(SW, SH);//loadSurface calls loadRLE and createSurface
			file->loadMiniSurface(SW,SH,(unsigned char *)objs[background_id].ptr,&background);
			//highlight = file->loadSurface(SW, SH);
			file->loadMiniSurface(SW,SH,(unsigned char *)objs[highlight_id].ptr,&highlight);
		} else {
			file->loadPalette(palette);
			//background = file->loadSurface(SW, SH);
			file->loadMiniSurface(SW,SH,(unsigned char *)objs[background_id].ptr,&background);
			//highlight = file->loadSurface(SW, SH);
			file->loadMiniSurface(SW,SH,(unsigned char *)objs[highlight_id].ptr,&highlight);
			file->skipRLE();
			file->skipRLE();
			file->skipRLE();
		}
	} else {
		file->loadPalette(palette);
		//background = file->loadSurface(SW, SH);
		file->loadMiniSurface(SW,SH,(unsigned char *)objs[background_id].ptr,&background);//Loaded on Casio
		//highlight = file->loadSurface(SW, SH);
		file->loadMiniSurface(SW,SH,(unsigned char *)objs[highlight_id].ptr,&highlight);
	}
	//SDL_SetColorKey(background, SDL_SRCCOLORKEY, 0);
	setColKey(&background,0);
	//SDL_SetColorKey(highlight, SDL_SRCCOLORKEY, 0);
	setColKey(&highlight,0);
	//SDL_SetColorKey(logo, SDL_SRCCOLORKEY, 28);
	setColKey(&logo,28);
	return file;
}

MainMenu::MainMenu(){
	File * file=loadLogos();
	//gameMenu = new GameMenu(file);
	delete file;
}


/**
 * Delete the main menu.
 */
MainMenu::~MainMenu(){
	if(highlight_id!=INVALID_OBJ)
		freeobj(highlight_id);
	if(background_id!=INVALID_OBJ)
		freeobj(background_id);
	//SDL_FreeSurface(background);
	//SDL_FreeSurface(highlight);
	//SDL_FreeSurface(logo);

	//delete gameMenu;
}


/**
 * Process a main menu selection.
 *
 * @param option Chosen menu option
 *
 * @return Error code
 */
int MainMenu::select (int option) {
	freeobj(highlight_id);
	freeobj(background_id);
	highlight_id=INVALID_OBJ;
	background_id=INVALID_OBJ;
	JJ1Scene *scene;
	{
		// Ensure that the menu palette is loaded first.
		File * file=skipLogos();
		file->loadPalette(menuPalette);
		delete file;
	}

	int ret;
	switch (option) {

		case 0: // New game
			{File * file=skipLogos();
			gameMenu = new GameMenu(file);
			delete file;}
			ret=gameMenu->newGame();
			delete gameMenu;
			if (ret == E_QUIT){
				return E_QUIT;
			}else
				loadLogos();
			break;

		case 1: // Load game
			{File * file=skipLogos();
			gameMenu = new GameMenu(file);
			delete file;}
			ret=gameMenu->loadGame();
			delete gameMenu;
			if (ret == E_QUIT)
				return E_QUIT;
			else
				loadLogos();

			break;

		case 2: // Instructions

			try {

				scene = new JJ1Scene(F_INSTRUCT_0SC);

			} catch (int e) {

				if (message("COULD NOT LOAD INSTRUCTIONS") == E_QUIT) return E_QUIT;

				break;

			}

			if (scene->play() == E_QUIT) {

				delete scene;

				return E_QUIT;

			}

			delete scene;
			loadLogos();
			break;

		case 3: // Setup options

			{SetupMenu setupMenu;
			if (setupMenu.setupMain() == E_QUIT)
				return E_QUIT;
			else
				loadLogos();
			}
			break;

		case 4: // Order info

			try {

				scene = new JJ1Scene(F_ORDER_0SC);

			} catch (int e) {

				if (message("COULD NOT LOAD ORDER INFO") == E_QUIT) return E_QUIT;

				break;

			}

			if (scene->play() == E_QUIT) {

				delete scene;

				return E_QUIT;

			}

			delete scene;
			loadLogos();
			break;

		case 5: // Exit

			return E_RETURN;

	}

	// Restore the main menu palette
	video.setPalette(palette);

	return E_NONE;

}

static void plasmaDraw(void){
	static fixed p0,p1,p2,p3;
	int x,y;

	int t1,t2,t3,t4;
	//int w,h;
	unsigned char *px=canvas.pix;
	unsigned char colour;
	unsigned int colb;

	// draw plasma

    t1 = p0;
    t2 = p1;
    y=canvasH;
    while(y--){
        t3 = p2;
        t4 = p3;
		colb = (fCos(t1*4)<<3)+(fCos(t2*4)<<3)+(32<<10);
		x=canvasW;
        while(x--){

			colour = ((colb+(fCos(t3*4)<<3)+(fCos(t4*4)<<3))>>10) & 0xF;

            t3 += 3;
            t4 += 2;

            *px++=colour;
		}
		// go to next row
		//px += canvasW;
        t1 += 2;
        ++t2;
	}

	p0 = p0 < 256 ? p0+1 : 1;
	p1 = p1 < 256 ? p1+2 : 2;
	p2 = p2 < 256 ? p2+3 : 3;
	p3 = p3 < 256 ? p3+4 : 4;

}
/**
 * Run the main menu.
 *
 * @return Error code
 */
static const short optionsPos[6][4] = {//X Y W H
		{92, 35, 136, 22},
		{92, 57, 140, 22},
		{88, 83, 144, 22},
		{86, 109, 150, 23},
		{82, 137, 156, 26},
		{78, 166, 166, 29}};
int MainMenu::main () {

	
	//int macroType[4];
	File* file;
	char* fileName;
	SDL_Rect dst;
	int option/*, macro*/, x, y, ret;
	unsigned int idleTime;

	option = 0;

	video.setPalette(palette);

	//playMusic("menusng.psm");


	// Demo timeout
	idleTime = globalTicks + T_DEMO;

	// Check for demo macros

	/*fileName = createString(F_MACRO);

	for (macro = 0; macro < 4; macro++)
	{

		try {

			file = new File(fileName, false);
			macroType[macro] = file->loadChar();
			delete file;

		} catch (int e) {

			macroType[macro] = -1;

		}

		fileName[6]++;

	}

	macro = 3;

	delete[] fileName;*/

	while (true) {

		if (loop(NORMAL_LOOP) == E_QUIT) return E_QUIT;

		if (controls.release(C_ESCAPE)) option = 5;

		if (controls.release(C_UP)) option = (option + 5) % 6;

		if (controls.release(C_DOWN)) option = (option + 1) % 6;

		if (controls.release(C_ENTER)) {

			ret = select(option);

			if (ret < 0) return ret;

			// New demo timeout
			idleTime = globalTicks + T_DEMO;

		}

		/*if (controls.getCursor(x, y)){

			int count;

			x -= (canvasW - SW) >> 1;
			y -= (canvasH - SH) >> 1;

			for (count = 0; count < 6; count++) {

				if ((x >= options[count][0]) &&
					(x < options[count][0] + options[count][2]) &&
					(y >= options[count][1]) &&
					(y < options[count][1] + options[count][3])) {
					option = count;
					if (controls.wasCursorReleased()) {
						ret = select(option);
						if (ret < 0) return ret;
					}
					// New demo timeout
					idleTime = globalTicks + T_DEMO;
					break;
				}
			}
		}*/
		/*if (idleTime <= globalTicks) {

			Game* game = NULL;

			try {
				game = new LocalGame("", 0);
			} catch (int e) {
				// Do nothing
			}
			if (game) {

				// Load the macro

				x = macro;
				macro = (macro + 1) & 3;

				while ((macroType[macro] != 0xFF) && (macro != x))
					macro = (macro + 1) & 3;

				if (macro != x) {

					fileName = createString(F_MACRO);
					fileName[6] += macro;

					if (game->playLevel(fileName) == E_QUIT) {

						delete[] fileName;
						delete game;

						return E_QUIT;

					}

					delete[] fileName;
				}
				delete game;

				//playMusic("menusng.psm");

				// Restore the main menu palette
				video.setPalette(palette);
			}
			idleTime = globalTicks + T_DEMO;
		}*/
		#ifdef CASIO
			//casioDelay(T_FRAME);
		#else
			SDL_Delay(T_FRAME);
		#endif
		//as long as we're drawing plasma, we don't need to clear the screen.
		//video.clearScreen(28);
		plasmaDraw();
		//dst.x = (canvasW >> 2) - 72;
		//dst.y = canvasH - (canvasH >> 2);
		//SDL_BlitSurface(logo, NULL, canvas, &dst);
		blitToCanvas(&logo,(canvasW >> 2) - 72,canvasH - (canvasH >> 2));
		//dst.x = (canvasW - SW) >> 1;
		//dst.y = (canvasH - SH) >> 1;
		//SDL_BlitSurface(background, NULL, canvas, &dst);
		blitToCanvas(&background,(canvasW - SW) >> 1,(canvasH - SH) >> 1);
		//dst.x = ((canvasW - SW) >> 1) + options[option].x;
		//dst.y = ((canvasH - SH) >> 1) + options[option].y;
		//SDL_BlitSurface(highlight, options + option, canvas, &dst);
		blitPartToCanvas(&highlight,((canvasW - SW) >> 1) + optionsPos[option][0],((canvasH - SH) >> 1) + optionsPos[option][1],optionsPos[option]);
	}

	return E_NONE;

}



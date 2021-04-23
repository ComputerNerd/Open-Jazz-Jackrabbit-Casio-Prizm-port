/**
 *
 * @file main.cpp
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 23rd August 2005: Created main.c
 * 22nd July 2008: Created util.c from parts of main.c
 * 3rd February 2009: Renamed main.c to main.cpp
 * 4th February 2009: Created palette.cpp from parts of main.cpp and util.cpp
 * 13th July 2009: Created controls.cpp from parts of main.cpp
 * 21st July 2013: Created setup.cpp from parts of main.cpp and setupmenu.cpp
 *
 * @section License
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
 * Contains the main function.
 *
 */


#define EXTERN
#include <string.h>
#include <stdint.h>

#include "game/game.h"
#include "io/controls.h"
#include "io/file.h"
#include "io/gfx/font.h"
#include "io/gfx/video.h"
//#include "io/network.h"
//#include "io/sound.h"
//#include "jj2level/jj2level.h"
#include "jj1level/jj1level.h"
#include "menu/menu.h"
#include "player/player.h"
#include "jj1scene/jj1scene.h"
#include "loop.h"
#include "setup.h"
#include "util.h"
#include "mem.h"
#include "jj1planet/jj1planet.h"
#include "io/gfx/sprite.h"
#if defined(CAANOO) || defined(WIZ) || defined(GP2X)
	#include "platforms/wiz.h"
#elif defined(CASIO)
	#include <fxcg/rtc.h>
	#include <fxcg/display.h>
	#include <fxcg/keyboard.h>
	#include <fxcg/misc.h>
	#include <alloca.h>
	#include <fxcg/heap.h>
	#define free sys_free
	#define malloc sys_malloc
	#include "platforms/casio.h"
#endif
inline void *operator new(size_t, void *p) throw ()
    {
    return p;
    }

class Main {

	public:
		Main  (void);
		~Main ();

		int play ();

};
static void * FontsBlock;
/**
 * Initializes OpenJazz.
 *
 * Establishes the paths from which to read files, loads configuration, sets up
 * the game window and loads required data.
 *
 * @param argc Number of arguments, as passed to main function
 * @param argv Array of argument strings, as apsse to main function
 */
Main::Main (void){
	File* file;
	//unsigned char* pixels = NULL;
	int count;
	int scaleFactor = 1;
#ifndef CASIO
	#ifdef FULLSCREEN_ONLY
		bool fullscreen = true;
	#else
		bool fullscreen = false;
	#endif
#endif

	// Use the current working directory
	#ifdef CASIO
		firstPath = new Path(NULL, "\\\\fls0\\jazz\\");
	#else
		firstPath = new Path(NULL, "jazz/");
	#endif


	// Default settings






	// Create the game's window



#ifdef CASIO
	if (!video.init()) {
#else
	if (!video.init(fullscreen)) {
#endif

		delete firstPath;

		throw E_VIDEO;

	}

#ifdef SCALE
	video.setScaleFactor(scaleFactor);
#endif

	#ifndef CASIO
	if (SDL_NumJoysticks() > 0) SDL_JoystickOpen(0);
	#endif

	// Set up audio
	//openAudio();



	// Load fonts

	// Open the panel, which contains two fonts

	/*try {

		file = new File(F_PANEL, false);

	} catch (int e) {

		//closeAudio();

		delete firstPath;
		#ifdef CASIO
		casioQuit("F_PANEL");
		#endif
		throw e;

	}*/
	//pixels=(unsigned char *)alloca(46272);
	//file->loadRLE(46272,pixels);//Store this in flash to save ram (it is used by fonts)

	//delete file;

	panelBigFont = NULL;
	panelSmallFont = NULL;
	font2 = NULL;
	fontbig = NULL;
	fontiny = NULL;
	fontmn1 = NULL;
	FontsBlock=malloc(sizeof(Font)*7);
	if(!FontsBlock){
		#ifdef CASIO
			casioQuit("Malloc error Fonts");
		#else
			puts("Malloc error fonts");
			throw -1;
		#endif
	}
	#ifdef CASIO
		clearLine(2);
		PrintXY(1,2,"Fonts"-2,0x20,TEXT_COLOR_WHITE);
		PrintXY(1,3,"0%"-2,0x20,TEXT_COLOR_WHITE);
		Bdisp_PutDisp_DD_stripe(2*24,4*24);
	#endif
	try {

		file = new File(F_PANEL, false);
	} catch (int e) {
		#ifdef CASIO
			casioQuitF(F_PANEL);
		#else
			puts(F_PANEL);
			throw -1;
		#endif
	}
	unsigned char * rle_panel=(unsigned char*)malloc(46272);
	if(!rle_panel){
		#ifdef CASIO
			casioQuitM("rle_panel");
		#else
			puts("Malloc rle_panel");
		#endif
	}
	file->loadRLE(46272,rle_panel);
	delete file;
	try {
		panelBigFont = new(FontsBlock) Font((unsigned char *)rle_panel + (40 * 320), true);
		#ifdef CASIO
			drawStrL(3,"14%");
		#endif
		panelSmallFont = new(FontsBlock+sizeof(Font)) Font((unsigned char *)rle_panel + (48 * 320), false);
		free(rle_panel);
		#ifdef CASIO
			drawStrL(3,"29%");
		#endif
		font2 = new(FontsBlock+(sizeof(Font)*2))Font(F_FONT2_0FN);
		#ifdef CASIO
			drawStrL(3,"43%");
		#endif
		fontbig = new(FontsBlock+(sizeof(Font)*3)) Font(F_FONTBIG_0FN);
		#ifdef CASIO
			drawStrL(3,"57%");
		#endif
		fontiny = new(FontsBlock+(sizeof(Font)*4)) Font(F_FONTINY_0FN);
		#ifdef CASIO
			drawStrL(3,"71%");
		#endif
		fontmn1 = new(FontsBlock+(sizeof(Font)*5)) Font(F_FONTMN1_0FN);
		#ifdef CASIO
			drawStrL(3,"86%");
		#endif
		fontmn2 = new(FontsBlock+(sizeof(Font)*6)) Font(F_FONTMN2_0FN);
		
		#ifdef CASIO
			drawStrL(3,"100%");
		#endif
	} catch (int e) {

		if (panelBigFont) panelBigFont->~Font();
		if (panelSmallFont) panelSmallFont->~Font();
		if (font2) font2->~Font();
		if (fontbig) fontbig->~Font();
		if (fontiny) fontiny->~Font();
		if (fontmn1) fontmn1->~Font();
		if (fontmn2) fontmn2->~Font();
		free(FontsBlock);
		//delete[] pixels;

		//closeAudio();

		delete firstPath;
		#ifdef CASIO
			casioQuit("Error loading fonts");
		#endif
		throw e;

	}

	//delete[] pixels;

	// Establish arbitrary timing
	#ifdef CASIO
		globalTicks = (RTC_GetTicks()*125/16) - 20;//originally *1000/128 but it has GCF of 8 this reduces overflow
	#else
		globalTicks = SDL_GetTicks() - 20;
	#endif

	// Fill trigonometric function look-up tables
	//for (count = 0; count < 1024; count++)//This has been replaced with constant table
	//	sinLut[count] = fixed(sinf(2 * PI * float(count) / 1024.0f) * 1024.0f);
	// Initiate networking
	//net = new Network();
	level = NULL;
	//jj2Level = NULL;
}


/**
 * De-initializes OpenJazz.
 *
 * Frees data, writes configuration, and shuts down SDL.
 */
Main::~Main () {

	//delete net;
	#ifndef CASIO
	puts("Main object gone");
	#endif
	fontmn2->~Font();
	fontmn1->~Font();
	fontiny->~Font();
	fontbig->~Font();
	font2->~Font();
	panelSmallFont->~Font();
	panelBigFont->~Font();
	free(FontsBlock);
#ifdef SCALE
	if (video.getScaleFactor() > 1) SDL_FreeSurface(canvas);
#endif

	//closeAudio();


	delete firstPath;

}


/**
 * Run the cutscenes and the main menu.
 *
 * @return Error code
 */
int Main::play() {

	// Load and play the startup cutscene
	/*#ifdef CASIO
	PrintXY(1,4,"  Scene",0x20,TEXT_COLOR_WHITE);
	Bdisp_PutDisp_DD();
	#endif
	try {

		scene = new JJ1Scene(F_STARTUP_0SC);

	} catch (int e) {
		#ifdef CASIO
		casioQuit("Error opening scene");
		#endif
		return e;

	}
	#ifdef CASIO
	__builtin_memset(vramAddress,0,384*216*2);
	PrintXY(1,1,"Playing!"-2,0x20,TEXT_COLOR_WHITE);
	Bdisp_PutDisp_DD();
	{int key;
	GetKey(&key);}
	#endif
	if (scene->play() == E_QUIT) {

		delete scene;

		return E_NONE;

	}

	delete scene;*/

	// Load and run the menu

	/*try {

		mainMenu = new MainMenu();

	} catch (int e) {
		#ifdef CASIO
		casioQuit("new MainMenu error");
		#endif
		return e;

	}*/
	{MainMenu mainMenu;
	if (mainMenu.main() == E_QUIT) {
		//delete mainMenu;
		return E_NONE;
	}}

	//delete mainMenu;//This is now on stack


	// Load and play the ending cutscene

	try {
		JJ1Scene scene(F_END_0SC);
		scene.play();
	} catch (int e) {
		#ifdef CASIO
		casioQuit("error new JJ1Scene (end)");
		#endif
		return e;

	}


	return E_NONE;

}


/**
 * Process iteration.
 *
 * Called once per game iteration. Updates timing, video, and input
 *
 * @param type Type of loop. Normal, typing, or input configuration
 * @param paletteEffects Palette effects to apply to video output
 *
 * @return Error code
 */
int loop (LoopType type, PaletteEffect* paletteEffects) {
	#ifndef CASIO
	SDL_Event event;
	#endif
	int prevTicks, ret;


	// Update tick count
	prevTicks = globalTicks;
	#ifdef CASIO
		globalTicks = RTC_GetTicks()*125/16;
	#else
		globalTicks = SDL_GetTicks();
	#endif

	#ifndef CASIO
	if (globalTicks - prevTicks < T_ACTIVE_FRAME) {

		// Limit framerate
		SDL_Delay(T_ACTIVE_FRAME + prevTicks - globalTicks);
		globalTicks = SDL_GetTicks();

	}
	#endif

	// Show what has been drawn
	video.flip(globalTicks - prevTicks, paletteEffects);
#ifdef CASIO
	ret = controls.update(type);
#else
	// Process system events
	ret=E_NONE;
	while (SDL_PollEvent(&event)){
		if (event.type == SDL_QUIT) return E_QUIT;
		ret = controls.update(&event, type);
		video.update(&event);
	}
#endif
	if (ret != E_NONE) return ret;
	controls.loop();
	return E_NONE;

}
#ifdef CASIO
unsigned char * SaveVramAddr;
static unsigned HackRET(unsigned char*x){
	return (unsigned)x;
}
#else
unsigned char __attribute__((aligned(sizeof(uintptr_t)))) SaveVramAddr[384 * 216 * 2];
#endif
/**
 * Main.
 *
 * Initializes SDL and launches game.
 */
uint16_t* vramAddress;
int main(void){
	//Main* mainObj;
	#ifdef CASIO
		Bdisp_EnableColor(1);
		FrameColor(1, 0);
		vramAddress=(uint16_t *)GetVRAMAddress();
		memset(vramAddress,0,384*216*2);
		PrintXY(1,1,"Loading..."-2,0x20,TEXT_COLOR_WHITE);
		Bdisp_PutDisp_DD();
		SaveVramAddr=(unsigned char*)getSecondaryVramAddress();
		if(HackRET(SaveVramAddr)&3)
			SaveVramAddr+=4-(HackRET(SaveVramAddr)&3);//Align address
	#else
		// Initialize SDL
		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK | SDL_INIT_TIMER) < 0) {
			logError("Could not start SDL", SDL_GetError());
			return -1;
		}
	#endif
	// Load configuration and establish a window
	controls.SetKeys();
	//JJ1BonusLevel=136320 JJ1LevelPlayer=760 Sprite=24 sizeof(JJ1Level)=154588 Font=2440 RotatePaletteEffect=24 JJ1Planet=544
	{
	Main mainObj;

	// Play the opening cutscene, run the main menu, etc.
	mainObj.play();
	}
	// Save configuration and shut down

	//delete mainObj;
	#ifndef CASIO
	SDL_Quit();
	#endif
	//return ret;
}

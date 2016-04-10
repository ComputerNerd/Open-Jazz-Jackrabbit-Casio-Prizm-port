
/**
 *
 * @file video.cpp
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 23rd August 2005: Created main.c
 * 22nd July 2008: Created util.c from parts of main.c
 * 3rd February 2009: Renamed util.c to util.cpp
 * 13th July 2009: Created graphics.cpp from parts of util.cpp
 * 26th July 2009: Renamed graphics.cpp to video.cpp
 *
 * @section Licence
 * Copyright (c) 2005-2011 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * @section Description
 * Contains graphics utility functions.
 *
 */


#include "paletteeffects.h"
#include "video.h"


#include "util.h"

#include <string.h>
#ifdef CASIO
	#include <fxcg/display.h>
	#include "platforms/casio.h"
#endif
struct miniSurface canvas;

/**
 * Create the video output object.
 */
Video::Video(){
	int count;
#ifdef SCALE
	scaleFactor = 1;
#endif

	// Generate the logical palette
	for (count = 0; count < 256; count++)
		currentPalette[count]=((count&248)<<8)|((count&252)<<3)|((count&248)>>3);
	canvas.pix=0;
	return;

}
Video::~Video(){
	freeobj(canvasID);
}




/**
 * Initialise video output.
 *
 * @param width Width of the window or screen
 * @param height Height of the window or screen
 * @param startFullscreen Whether or not to start in full-screen mode
 *
 * @return Success
 */
#ifdef CASIO
bool Video::init(void) {
#else
bool Video::init (bool startFullscreen) {
#endif
	int count;
	for (count = 0; count < 256;++count)
		currentPalette[count]=((count&248)<<8)|((count&252)<<3)|((count&248)>>3);
	#ifndef CASIO
		fullscreen = startFullscreen;
		if (fullscreen) SDL_ShowCursor(SDL_DISABLE);
	#endif
	if (!resize()) {
		#ifndef CASIO
		logError("Could not set video mode", SDL_GetError());
		#endif
		return false;

	}
	#ifndef CASIO
		SDL_WM_SetCaption("OpenJazz", NULL);
	#endif
	addobj(384*216,&canvasID);
	initMiniSurface(&canvas,objs[canvasID].ptr,384,216);
	return true;
}


/**
 * Sets the size of the video window or the resolution of the screen.
 *
 * @param width New width of the window or screen
 * @param height New height of the window or screen
 *
 * @return Success
 */
bool Video::resize (void) {


#ifdef SCALE
	if (canvas != screen) SDL_FreeSurface(canvas);
#endif

#if defined(CAANOO) || defined(WIZ) || defined(GP2X) || defined(DINGOO)
	screen = SDL_SetVideoMode(320, 240, 8, FULLSCREEN_FLAGS);
#elif defined(CASIO)
	//Do nothing
#else
	screen = SDL_SetVideoMode(384, 216, 16, fullscreen? FULLSCREEN_FLAGS: WINDOWED_FLAGS);
#endif
	#ifndef CASIO
	if (!screen) return false;
	#endif
		//canvasW = screenW;
		//canvasH = screenH;
#if !defined(WIZ) && !defined(GP2X) && !defined(CASIO)
	expose();
#endif


	/* A real 8-bit display is quite likely if the user has the right video
	card, the right video drivers, the right version of DirectX/whatever, and
	the right version of SDL. In other words, it's not likely enough. If a real
	palette is assumed when
	a) there really is a real palette, there will be an extremely small speed
		gain.
	b) the palette is emulated, there will be a HUGE speed loss.
	Therefore, assume the palette is emulated. */
	/// @todo Find a better way to determine if palette is emulated
	fakePalette = true;

	return true;

}


/**
 * Sets the display palette.
 *
 * @param palette The new palette
 */
void Video::setPalette (unsigned short *palette) {
	memcpy(currentPalette,palette,256*sizeof(unsigned short));
}
/**
 * Returns the current display palette.
 *
 * @return The current display palette
 */
unsigned short * Video::getPalette () {
	return currentPalette;
}


/**
 * Sets some colours of the display palette.
 *
 * @param palette The palette containing the new colours
 * @param first The index of the first colour in both the display palette and the specified palette
 * @param amount The number of colours
 */
void Video::changePalette (unsigned short *palette, unsigned char first, unsigned int amount) {
	//SDL_SetPalette(screen, SDL_PHYSPAL, palette, first, amount);
	//return;
	//puts("WOOPS!");//This function should not be called ever
	memcpy(finalPalette+first,palette+first,amount*sizeof(unsigned short));
}


#ifndef FULLSCREEN_ONLY
/**
 * Determines whether or not full-screen mode is being used.
 *
 * @return Whether or not full-screen mode is being used
 */
bool Video::isFullscreen () {

	return fullscreen;

}
#endif


/**
 * Refresh display palette.
 */
void Video::expose () {

	//SDL_SetPalette(screen, SDL_LOGPAL, logicalPalette, 0, 256);
	//SDL_SetPalette(screen, SDL_PHYSPAL, currentPalette, 0, 256);

	return;

}
#ifndef CASIO

/**
 * Update video based on a system event.
 *
 * @param event The system event. Events not affecting video will be ignored
 */
void Video::update (SDL_Event *event) {

#ifndef FULLSCREEN_ONLY
	switch (event->type) {

		case SDL_KEYDOWN:

			// If Alt + Enter has been pressed, switch between windowed and full-screen mode.
			if ((event->key.keysym.sym == SDLK_RETURN) &&
				(event->key.keysym.mod & KMOD_ALT)) {

				fullscreen = !fullscreen;

				if (fullscreen) SDL_ShowCursor(SDL_DISABLE);

				resize();

				if (!fullscreen) SDL_ShowCursor(SDL_ENABLE);

			}

			break;

		case SDL_VIDEORESIZE:

			resize();

			break;

		case SDL_VIDEOEXPOSE:

			expose();

			break;

	}
#endif

	return;

}
#endif
#ifdef CASIO
#define LCD_GRAM 0x202
#define LCD_BASE	0xB4000000
#define VRAM_ADDR 0xA8000000
// Module Stop Register 0
#define MSTPCR0	(volatile unsigned *)0xA4150030
// DMA0 operation register
#define DMA0_DMAOR	(volatile unsigned short*)0xFE008060
#define DMA0_SAR_0	(volatile unsigned *)0xFE008020
#define DMA0_DAR_0  (volatile unsigned *)0xFE008024
#define DMA0_TCR_0	(volatile unsigned *)0xFE008028
#define DMA0_CHCR_0	(volatile unsigned *)0xFE00802C

static void DoDMAlcdNonblock(void){
	Bdisp_WriteDDRegister3_bit7(1);
	Bdisp_DefineDMARange(6,389,0,215);
	Bdisp_DDRegisterSelect(LCD_GRAM);

	*MSTPCR0&=~(1<<21);//Clear bit 21
	*DMA0_CHCR_0&=~1;//Disable DMA on channel 0
	*DMA0_DMAOR=0;//Disable all DMA
	*DMA0_SAR_0=VRAM_ADDR&0x1FFFFFFF;//Source address is VRAM
	*DMA0_DAR_0=LCD_BASE&0x1FFFFFFF;//Desination is LCD
	*DMA0_TCR_0=(216*384)/16;//Transfer count bytes/32
	*DMA0_CHCR_0=0x00101400;
	*DMA0_DMAOR|=1;//Enable DMA on all channels
	*DMA0_DMAOR&=~6;//Clear flags
	*DMA0_CHCR_0|=1;//Enable channel0 DMA
}
#endif
/**
 * Draw graphics to screen.
 *
 * @param mspf Ticks per frame
 * @param paletteEffects Palette effects to use
 */
void Video::flip (int mspf, PaletteEffect* paletteEffects) {
	#ifndef CASIO
	if(SDL_MUSTLOCK(screen)) SDL_LockSurface(screen);
	#endif
	unsigned int x,y;
	#ifdef CASIO
		unsigned short *o=(unsigned short *)0xA8000000;
		DmaWaitNext();
	#else
		unsigned short * o=(unsigned short *)screen->pixels;
	#endif
	unsigned char * i=canvas.pix;
	y=canvasH;
	// Apply palette effects
	if (paletteEffects) {
		unsigned short shownPalette[256];
		/* If the palette is being emulated, compile all palette changes and
		apply them all at once.
		If the palette is being used directly, apply all palette effects
		directly. */
		memcpy(shownPalette, currentPalette, sizeof(unsigned short) * 256);
		paletteEffects->apply(shownPalette, !fakePalette, mspf);
		while(y--){
			x=canvasW;
			while(x--)
				*o++=shownPalette[*i++];
		}
	}else{
		while(y--){
			x=canvasW;
			while(x--)
				*o++=currentPalette[*i++];
		}
	}
	// Show what has been drawn
	#ifdef CASIO
		//Bdisp_PutDisp_DD();
		DoDMAlcdNonblock();
	#else
		if(SDL_MUSTLOCK(screen)) SDL_UnlockSurface(screen);
		SDL_Flip(screen);
	#endif
}


/**
 * Fill the screen with a colour.
 *
 * @param index Index of the colour to use
 */
void Video::clearScreen (int index) {

#if defined(CAANOO) || defined(WIZ) || defined(GP2X)
	// always 240 lines cleared to black
	memset(video.screen->pixels, index, 320*240);
#else
	memset(canvas.pix,index,canvasW*canvasH);
#endif

	return;

}


/**
 * Fill a specified rectangle of the screen with a colour.
 *
 * @param x X-coordinate of the left side of the rectangle
 * @param y Y-coordinate of the top of the rectangle
 * @param width Width of the rectangle
 * @param height Height of the rectangle
 * @param index Index of the colour to use
 */
void drawRect (int x, int y, int width, int height, int index) {
	if(x<0){
		width+=x;
		x=0;
	}
	if(y<0){
		height+=y;
		y=0;
	}
	if(x>canvasW)
		return;
	if(y>canvasH)
		return;
	if(width<1)
		return;
	if(height<1)
		return;
	if(width>(canvasW-x))
		width=canvasW-x;
	if(height>(canvasH-y))
		height=canvasH-y;
	unsigned char * p=canvas.pix+x+(y*canvasW);
	while(height--){
		memset(p,index,width);
		p+=canvasW;
	}
}


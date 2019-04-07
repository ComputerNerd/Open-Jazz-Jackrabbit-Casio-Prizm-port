
/**
 *
 * @file controls.cpp
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 23rd August 2005: Created main.c
 * 22nd July 2008: Created util.c from parts of main.c
 * 3rd February 2009: Renamed main.c to main.cpp
 * 13th July 2009: Created controls.cpp from parts of main.cpp
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
 * Deals with input.
 *
 */


#include "controls.h"
#include "gfx/video.h"

#include "loop.h"

#if defined(CAANOO) || defined(WIZ) || defined(GP2X)
#include "platforms/wiz.h"
#endif
#ifdef CASIO
	#include <fxcg/keyboard.h>
	#include <fxcg/display.h>
	#include <fxcg/misc.h>
	#include "platforms/casio.h"
#endif

/**
 * Set up the default controls.
 */
void Controls::SetKeys(){
	int count;
#ifdef CASIO
	keys[C_UP].key		= KEY_PRGM_UP;
	keys[C_DOWN].key	= KEY_PRGM_DOWN;
	keys[C_LEFT].key	= KEY_PRGM_LEFT;
	keys[C_RIGHT].key	= KEY_PRGM_RIGHT;
	keys[C_JUMP].key	= KEY_PRGM_ALPHA;
	keys[C_SWIM].key	= keys[C_JUMP].key;
	keys[C_FIRE].key	= KEY_PRGM_SHIFT;
	keys[C_CHANGE].key	= KEY_PRGM_OPTN;//change weapon
	keys[C_ENTER].key	= KEY_PRGM_RETURN;
	keys[C_ESCAPE].key	= KEY_PRGM_EXIT;
	keys[C_STATS].key	= KEY_PRGM_F1;
	keys[C_PAUSE].key	= KEY_PRGM_F2;
	keys[C_YES].key		= KEY_PRGM_F3;
	keys[C_NO].key		= KEY_PRGM_F4;

#else
	keys[C_UP].key     = SDLK_UP;
	keys[C_DOWN].key   = SDLK_DOWN;
	keys[C_LEFT].key   = SDLK_LEFT;
	keys[C_RIGHT].key  = SDLK_RIGHT;
	keys[C_JUMP].key   = SDLK_SPACE;
	keys[C_FIRE].key   = SDLK_LALT;
	keys[C_CHANGE].key = SDLK_RCTRL;
	keys[C_ENTER].key  = SDLK_RETURN;
	keys[C_ESCAPE].key = SDLK_ESCAPE;
	keys[C_STATS].key  = SDLK_F9;
	keys[C_PAUSE].key  = SDLK_p;
	keys[C_YES].key    = SDLK_y;
	keys[C_NO].key     = SDLK_n;
	keys[C_SWIM].key   = keys[C_JUMP].key;
#endif

	for (count = 0; count < CONTROLS; count++) {
		keys[count].state = false;
		//keys[count].time = 0;
	}

}
Controls::Controls () {
	SetKeys();
}



/**
 * Set the key to use for the specified control.
 *
 * @param control The control
 * @param key The key to use
 */
void Controls::setKey (int control, int key) {

	keys[control].key = key;
	keys[control].state = false;

	return;

}


/**
 * Get the key being used for the specified control.
 *
 * @param control The control
 *
 * @return The key being used
 */
int Controls::getKey (int control) {

	return keys[control].key;

}

#ifdef CASIO
static const volatile unsigned short* keyboard_register = (unsigned short*)0xA44B0000;
static int keydownlast(int basic_keycode) {
    int row, col, word, bit; 
    row = basic_keycode%10; 
    col = basic_keycode/10-1; 
    word = row>>1; 
    bit = col + 8*(row&1); 
    return (0 != (keyboard_register[word] & 1<<bit)); 
}

#endif

/**
 * Update controls based on a system event.
 *
 * @param event The system event. Non-input events will be ignored
 * @param type Type of loop. Normal, typing, or input configuration
 *
 * @return Error code
 */
#ifdef CASIO
int Controls::update (LoopType type)
#else
int Controls::update (SDL_Event *event, LoopType type)
#endif
{
	int count;
	count = CONTROLS;
	#ifdef CASIO
		for(count=0;count<CONTROLS;++count){
			keys[count].state=keydownlast(keys[count].key);
		}
	#else
	switch (event->type) {

		case SDL_KEYDOWN:

			if (type == SET_KEY_LOOP) return event->key.keysym.sym;

			for (count = 0; count < CONTROLS; count++)
				if (event->key.keysym.sym == keys[count].key)
					keys[count].state = true;

			if (type == TYPING_LOOP) return event->key.keysym.sym;

			break;

		case SDL_KEYUP:

			for (count = 0; count < CONTROLS; count++)
				if (event->key.keysym.sym == keys[count].key)
					keys[count].state = false;

			break;
		}
	#endif
	return E_NONE;
}



/**
 * Process input iteration.
 *
 * Called once per game iteration. Updates input.
 */
void Controls::loop () {

	//int count;

	// Apply controls to universal control tracking
	//for (count = 0; count < CONTROLS; count++)
	//	keys[count].state = (keys[count].time < globalTicks) && (keys[count].state);

}

/**
 * Determine whether or not the specified control is being used.
 *
 * @param control The control
 *
 * @return True if the control is being used
 */
bool Controls::getState (int control) {

	return keys[control].state;

}


/**
 * If it's being used, release the specified control.
 *
 * @param control The control
 *
 * @return True if the control was being used
 */
bool Controls::release (int control) {

	if (!keys[control].state) return false;

	//keys[control].time = globalTicks + T_KEY;
	keys[control].state = false;

	return true;

}

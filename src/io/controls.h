
/**
 *
 * @file controls.h
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 23rd August 2005: Created OpenJazz.h
 * 13th July 2009: Created controls.h from parts of OpenJazz.h
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

#ifndef _INPUT_H
#define _INPUT_H


#include "loop.h"
#include "OpenJazz.h"

#ifndef CASIO
#include <SDL/SDL.h>
#endif

// Constants

// Indexes for the keys / buttons / axes player controls arrays
#define C_UP      0
#define C_DOWN    1
#define C_LEFT    2
#define C_RIGHT   3
#define C_JUMP    4
#define C_SWIM    5
#define C_FIRE    6
#define C_CHANGE  7 /* Change weapon */
#define C_ENTER   8
#define C_ESCAPE  9
#define C_STATS  10
#define C_PAUSE  11
#define C_YES    12
#define C_NO     13
// Size of those arrays
#define CONTROLS 14

// Time interval
#define T_KEY   200


// Class

/// Keeps track of all control input
class Controls {

	private:
		struct {
			int  key; ///< Keyboard key
			bool state; ///< Whether or not the key is pressed

		} keys[CONTROLS];
	public:
		Controls ();
		void SetKeys();
		void setKey           (int control, int key);
		int  getKey           (int control);
		#ifdef CASIO
			int  update           (LoopType type);
		#else
			int  update           (SDL_Event *event, LoopType type);
		#endif
		void loop             ();

		bool getState          (int control);
		bool release           (int control);
		//bool getCursor         (int& x, int& y);
		//bool wasCursorReleased ();

};


// Variable

EXTERN Controls controls;

#endif

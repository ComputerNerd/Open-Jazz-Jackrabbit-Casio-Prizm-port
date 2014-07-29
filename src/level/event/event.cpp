
/*
 *
 * event.cpp
 *
 * 1st January 2006: Created events.c from parts of level.c
 * 3rd February 2009: Renamed events.c to events.cpp
 * 5th February 2009: Added parts of events.cpp and level.cpp to player.cpp
 * 11th February 2009: Created bullet.cpp from parts of events.cpp
 * 1st March 2009: Created bird.cpp from parts of events.cpp
 * 19th March 2009: Created sprite.cpp from parts of event.cpp and player.cpp
 * 19th July 2009: Created eventframe.cpp from parts of events.cpp
 * 19th July 2009: Renamed events.cpp to event.cpp
 *
 * Part of the OpenJazz project
 *
 *
 * Copyright (c) 2005-2009 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

/*
 * Deals with events in ordinary levels.
 *
 */


#include "../level.h"
#include "event.h"

#include "io/sound.h"
#include "player/player.h"

#include <math.h>


Event::Event (unsigned char gX, unsigned char gY, Event *nextEvent) {

	next = nextEvent;
	gridX = gX;
	gridY = gY;
	x = TTOF(gX);
	y = TTOF(gY + 1);
	flashTime = 0;

	// Choose initial settings

	switch (getProperty(E_BEHAVIOUR)) {

		case 21: // Destructible block
		case 25: // Float up / Belt
		case 38: // Sucker tubes
		case 40: // Monochrome
		case 57: // Bubbles

			animType = 0;

			break;

		case 26: // Flip animation

			animType = E_RIGHTANIM;

			break;

		case 28:

			animType = E_LEFTANIM;
			x -= F2;
			y += ITOF(getProperty(E_YAXIS)) - F40;

			// dx and dy used to store leftmost and rightmost player on bridge
			// Start with minimum values
			dx = getProperty(E_MULTIPURPOSE) * F8;
			dy = 0;

			break;

		default:

			animType = E_LEFTANIM;

			break;

	}

	return;

}


Event::~Event () {

	return;

}


Event * Event::getNext () {

	return next;

}


void Event::removeNext () {

	Event *newNext;

	if (next) {

		newNext = next->getNext();
		delete next;
		next = newNext;

	}

	return;

}


void Event::destroy (unsigned int ticks) {

	level->setEventTime(gridX, gridY, ticks + T_FINISH);

	animType = ((animType == E_RIGHTANIM) || (animType == E_RSHOOTANIM)) ?
			E_RFINISHANIM: E_LFINISHANIM;

	level->playSound(getProperty(E_SOUND));

	return;

}


bool Event::hit (Player *source, unsigned int ticks) {

	int hitsRemaining;

	// Deal with bullet collisions
	if ((animType == E_LFINISHANIM) || (animType == E_RFINISHANIM) ||
		(ticks < flashTime)) return false;

	hitsRemaining = level->hitEvent(source, gridX, gridY);

	// If the event cannot be hit, do not register hit
	if (hitsRemaining < 0) return false;

	// Check if the hit has destroyed the event
	if (hitsRemaining == 0) destroy(ticks);

	// The event has been hit, so it should flash
	flashTime = ticks + T_FLASH;

	// Register hit
	return true;

}


bool Event::isFrom (unsigned char gX, unsigned char gY) {

	return (gX == gridX) && (gY == gridY);

}


fixed Event::getWidth () {

	fixed width;

	if (animType && (getProperty(animType) >= 0)) {

		width = ITOF(level->getAnim(getProperty(animType))->getWidth());

		// Blank sprites for e.g. invisible springs
		if ((width == F1) && (getHeight() == F1)) return F32;

		return width;

	}

	return F32;

}


fixed Event::getHeight () {

	if (animType && (getProperty(animType) >= 0))
		return ITOF(level->getAnim(getProperty(animType))->getHeight());

	return F32;

}


bool Event::overlap (fixed left, fixed top, fixed width, fixed height) {

	return (x + getWidth() >= left) && (x < left + width) &&
		(y >= top) && (y - getHeight() < top + height);

}


signed char Event::getProperty (unsigned char property) {

	signed char *set;

	set = level->getEvent(gridX, gridY);

	if (set) return set[property];

	return 0;

}



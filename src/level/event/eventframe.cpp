
/*
 *
 * eventframe.cpp
 *
 * 19th July 2009: Created eventframe.cpp from parts of events.cpp
 * 2nd March 2010: Created guardians.cpp from parts of event.cpp and eventframe.cpp
 * 2nd March 2010: Created bridge.cpp from parts of event.cpp and eventframe.cpp
 *
 * Part of the OpenJazz project
 *
 *
 * Copyright (c) 2005-2010 Alister Thomson
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
 * Provides the once-per-frame functions of ordinary events.
 *
 */


#include "../bullet.h"
#include "../level.h"
#include "event.h"

#include "io/gfx/video.h"
#include "io/sound.h"
#include "player/player.h"

#include <math.h>


signed char * Event::prepareStep (unsigned int ticks, int msps) {

	signed char *set;

	// Process the next event
	if (next) {

		if (next->step(ticks, msps)) removeNext();

	}


	// Get the event properties
	set = level->getEvent(gridX, gridY);

	// If the event has been removed from the grid, destroy it
	if (!set) return NULL;


	// If the event and its origin are off-screen, the event is not in the
	// process of self-destruction, remove it
	if ((animType != E_LFINISHANIM) && (animType != E_RFINISHANIM) &&
		((x < viewX - F160) || (x > viewX + ITOF(viewW) + F160) ||
		(y < viewY - F160) || (y > viewY + ITOF(viewH) + F160)) &&
		((gridX < FTOT(viewX) - 1) ||
		(gridX > ITOT(FTOI(viewX) + viewW) + 1) ||
		(gridY < FTOT(viewY) - 1) ||
		(gridY > ITOT(FTOI(viewY) + viewH) + 1))) return NULL;


	// Find frame
	if (animType && (set[animType] >= 0)) {

		if ((animType != E_LEFTANIM) && (animType != E_RIGHTANIM))
			frame = (ticks + T_FINISH - level->getEventTime(gridX, gridY)) / 40;
		else if (set[E_ANIMSP])
			frame = ticks / (set[E_ANIMSP] * 40);
		else
			frame = ticks / 20;

	}

	return set;

}


bool Event::step (unsigned int ticks, int msps) {

	fixed width, height;
	signed char *set;
	int count;
	fixed offset;
	float angle;


	set = prepareStep(ticks, msps);

	if (!set) return true;


	// Find dimensions
	width = getWidth();
	height = getHeight();


	// Handle behaviour

	switch (set[E_BEHAVIOUR]) {

		case 1:

			// Sink down
			dy = ES_FAST;

			break;

		case 2:

			// Walk from side to side
			if (animType == E_LEFTANIM) dx = -ES_FAST;
			else if (animType == E_RIGHTANIM) dx = ES_FAST;
			else dx = 0;

			break;

		case 3:

			// Seek jazz
			if (localPlayer->getX() + PXO_R < x) dx = -ES_FAST;
			else if (localPlayer->getX() + PXO_L > x + width) dx = ES_FAST;
			else dx = 0;

			break;

		case 4:

			// Walk from side to side and down hills

			if (!level->checkMaskDown(x + (width >> 1), y)) {

				// Fall downwards
				dx = 0;
				dy = ES_FAST;

			} else {

				// Walk from side to side
				if (animType == E_LEFTANIM) dx = -ES_FAST;
				else if (animType == E_RIGHTANIM) dx = ES_FAST;

				dy = 0;

			}

			break;

		case 5:

			// TODO: Find out what this is

			break;

		case 6:

			// Use the path from the level file
			dx = TTOF(gridX) + F16 + (level->pathX[level->pathNode] << 9) - x;
			dy = TTOF(gridY) + (level->pathY[level->pathNode] << 9) - y;
			dx = ((dx << 10) / msps) * set[E_MOVEMENTSP];
			dy = ((dy << 10) / msps) * set[E_MOVEMENTSP];

			break;

		case 7:

			// Move back and forth horizontally with tail
			if (animType == E_LEFTANIM) dx = -ES_SLOW;
			else if (animType == E_RIGHTANIM) dx = ES_SLOW;

			break;

		case 8:

			// TODO: Bird-esque following

			break;

		case 9:

			// TODO: Find out what this is

			break;

		case 10:

			// TODO: Find out what this is

			break;

		case 11:

			// Sink to ground
			if (!level->checkMaskDown(x + (width >> 1), y)) dy = ES_FAST;
			else dy = 0;

			break;

		case 12:

			// Move back and forth horizontally
			if (animType == E_LEFTANIM) dx = -ES_SLOW;
			else if (animType == E_RIGHTANIM) dx = ES_SLOW;
			else dx = 0;

			break;

		case 13:

			// Move up and down
			if (animType == E_LEFTANIM) dy = -ES_SLOW;
			else if (animType == E_RIGHTANIM) dy = ES_SLOW;
			else dy = 0;

			break;

		case 14:

			// TODO: Move back and forth rapidly

			break;

		case 15:

			// TODO: Rise or lower to meet jazz

			break;

		case 16:

			// Move across level to the left or right
			if (set[E_MAGNITUDE] == 0) dx = -ES_SLOW;
			else dx = set[E_MAGNITUDE] * ES_SLOW;

			break;

		case 17:

			// TODO: Find out what this is

			break;

		case 18:

			// TODO: Find out what this is

			break;

		case 19:

			// TODO: Find out what this is

			break;

		case 20:

			// TODO: Find out what this is

			break;

		case 21:

			// Destructible block
			if (level->getEventHits(gridX, gridY) >= set[E_HITSTOKILL])
				level->setTile(gridX, gridY, set[E_MULTIPURPOSE]);

			break;

		case 22:

			// TODO: Fall down in random spot and repeat

			break;

		case 23:

			// TODO: Find out what this is

			break;

		case 24:

			// TODO: Crawl along ground and go downstairs

			break;

		case 26:

			// TODO: Find out what this is

			break;

		case 27:

			// TODO: Face jazz

			break;

		case 29:

			// Rotate

			offset = ITOF(set[E_BRIDGELENGTH] * set[E_CHAINLENGTH]);
			angle = set[E_MAGNITUDE] * ticks / 2048.0f;

			dx = TTOF(gridX) + (int)(sin(angle) * offset) - x;
			dy = TTOF(gridY) + (int)((cos(angle) + 1.0f) * offset) - y;
			dx = ((dx << 10) / msps) * set[E_MOVEMENTSP];
			dy = ((dy << 10) / msps) * set[E_MOVEMENTSP];

			break;

		case 30:

			// Swing

			offset = ITOF(set[E_BRIDGELENGTH] * set[E_CHAINLENGTH]);
			angle = (set[E_CHAINANGLE] * 3.141592f / 128.0f) +
				(set[E_MAGNITUDE] * ticks / 2048.0f);

			dx = TTOF(gridX) + (int)(sin(angle) * offset) - x;
			dy = TTOF(gridY) + (int)((fabs(cos(angle)) + 1.0f) * offset) - y;
			dx = ((dx << 10) / msps) * set[E_MOVEMENTSP];
			dy = ((dy << 10) / msps) * set[E_MOVEMENTSP];

			break;

		case 31:

			// Move horizontally
			if (animType == E_LEFTANIM) dx = -ES_FAST;
			else dx = ES_FAST;

			break;

		case 32:

			// Move horizontally
			if (animType == E_LEFTANIM) dx = -ES_FAST;
			else dx = ES_FAST;

			break;

		case 33:

			// Sparks-esque following

			if (localPlayer->getFacing() && (x + width < localPlayer->getX())) {

				dx = ES_FAST;

				if (y + height < localPlayer->getY() + PYO_TOP) dy = ES_SLOW;
				else if (y > localPlayer->getY()) dy = -ES_SLOW;
				else dy = 0;

			} else if (!localPlayer->getFacing() &&
				(x > localPlayer->getX() + F32)) {

				dx = -ES_FAST;

				if (y + height < localPlayer->getY() + PYO_TOP) dy = ES_SLOW;
				else if (y > localPlayer->getY()) dy = -ES_SLOW;
				else dy = 0;

			} else {

				dx = 0;
				dy = 0;

			}

			break;

		case 34:

			// Launching platform

			if (ticks > level->getEventTime(gridX, gridY)) {

				if (y <= TTOF(gridY) + F16 - (set[E_YAXIS] * F2)) {

					level->setEventTime(gridX, gridY, ticks + (set[E_MOVEMENTSP] * 1000));
					dy = 0;

				} else dy = -(y + (set[E_YAXIS] * F2) - TTOF(gridY)) / 100;

			} else {

				if (y < TTOF(gridY) + F16) dy = (y + (set[E_YAXIS] * F2) - TTOF(gridY)) / 100;
				else {

					y = TTOF(gridY) + F16;
					dy = 0;

				}

			}

			break;

		case 35:

			// Non-floating Sparks-esque following

			if (localPlayer->getFacing() && (x + width < localPlayer->getX() + PXO_L - F4)) {

				if (level->checkMaskDown(x + width, y + F4) &&
					!level->checkMaskDown(x + width + F4, y - (height >> 1)))
					dx = ES_FAST;
				else
					dx = 0;

			} else if (!localPlayer->getFacing() && (x > localPlayer->getX() + PXO_R + F4)) {

				if (level->checkMaskDown(x, y + F4) &&
				    !level->checkMaskDown(x - F4, y - (height >> 1)))
			    	dx = -ES_FAST;
			    else
			    	dx = 0;

			} else dx = 0;

			break;

		case 36:

			// Walk from side to side and down hills, staying on-screen

			if (!level->checkMaskDown(x + (width >> 1), y)) {

				// Fall downwards
				dx = 0;
				dy = ES_FAST;

			} else {

				// Walk from side to side, staying on-screen
				if (animType == E_LEFTANIM) dx = -ES_FAST;
				else if (animType == E_RIGHTANIM) dx = ES_FAST;
				else dx = 0;

				dy = 0;

			}

			break;

		case 37:
		case 38:

			// Sucker tubes

			for (count = 0; count < nPlayers; count++) {

				if (players[count].overlap(x + F8, y + F4 - height, width - F16,
					height - F8)) {

					players[count].setSpeed(set[E_YAXIS]? set[E_MAGNITUDE] * F4: set[E_MAGNITUDE] * F40,
						set[E_YAXIS]? set[E_MULTIPURPOSE] * -F20: 0);

				}

			}

			break;

		case 39:

			// TODO: Collapsing floor

			break;

		case 40:

			// TODO: Find out what this is

			break;

		case 41:

			// TODO: Switch left & right anim periodically

			break;

		case 42:

			// TODO: Find out what this is

			break;

		case 43:

			// TODO: Find out what this is

			break;

		case 44:

			// TODO: Leap to greet Jazz very quickly

			break;

		case 45:

			// TODO: Find out what this is

			break;

		case 46:

			// TODO: "Final" boss

			break;

		case 53:

			// Dreempipes turtles

			if (y > level->getWaterLevel()) {

				if (animType == E_LEFTANIM) dx = -ES_SLOW;
				else if (animType == E_RIGHTANIM) dx = ES_SLOW;
				else dx = 0;

			} else dx = 0;

			break;

		default:

			// Do nothing for the following:
			// 0: Static
			// 25: Float up / Belt

			// TODO: Remaining event behaviours

			break;

	}

	dx /= set[E_MOVEMENTSP];
	dy /= set[E_MOVEMENTSP];
	x += (dx * msps) >> 10;
	y += (dy * msps) >> 10;


	// Choose animation and direction

	if ((animType == E_LEFTANIM) || (animType == E_RIGHTANIM)) {

		switch (set[E_BEHAVIOUR]) {

			case 2:

				// Walk from side to side
				if (animType == E_LEFTANIM) {

					if (!level->checkMaskDown(x, y + F4) ||
					    level->checkMaskDown(x - F4, y - (height >> 1)))
					    animType = E_RIGHTANIM;

				} else if (animType == E_RIGHTANIM) {

					if (!level->checkMaskDown(x + width, y + F4) ||
					    level->checkMaskDown(x + width + F4, y - (height >> 1)))
					    animType = E_LEFTANIM;

				}

				break;

			case 3:

				// Seek jazz
				if (localPlayer->getX() + PXO_R < x)
					animType = E_LEFTANIM;
				else if (localPlayer->getX() + PXO_L > x + width)
					animType = E_RIGHTANIM;

				break;

			case 4:

				// Walk from side to side and down hills

				if (level->checkMaskDown(x + (width >> 1), y)) {

					// Walk from side to side
					if (animType == E_LEFTANIM) {

						if (level->checkMaskDown(x - F4, y - (height >> 1) - F12))
							animType = E_RIGHTANIM;

					} else if (animType == E_RIGHTANIM) {

						if (level->checkMaskDown(x + width + F4, y - (height >> 1) - F12))
							animType = E_LEFTANIM;

					}

				}

				break;

			case 6:

				// Use the path from the level file

				// Check movement direction
				if ((level->pathNode < 3) ||
					(level->pathX[level->pathNode] <= level->pathX[level->pathNode - 3]))
					animType = E_LEFTANIM;
				else
					animType = E_RIGHTANIM;

				break;

			case 7:

				// Move back and forth horizontally with tail

				if (animType == E_LEFTANIM) {

					if (x < TTOF(gridX)) animType = E_RIGHTANIM;

				} else if (animType == E_RIGHTANIM) {

					if (x > TTOF(gridX) + F100) animType = E_LEFTANIM;

				}

				break;

			case 12:

				// Move back and forth horizontally

				if (animType == E_LEFTANIM) {

					if (level->checkMaskDown(x - F4, y - (height >> 1)))
						animType = E_RIGHTANIM;

				} else if (animType == E_RIGHTANIM) {

					if (level->checkMaskDown(x + width + F4, y - (height >> 1)))
						animType = E_LEFTANIM;

				}

				break;

			case 13:

				// Move up and down

				if (animType == E_LEFTANIM) {

					if (level->checkMaskDown(x + (width >> 1), y - height - F4))
						animType = E_RIGHTANIM;

				} else if (animType == E_RIGHTANIM) {

					if (level->checkMaskDown(x + (width >> 1), y + F4))
						animType = E_LEFTANIM;

				}

				break;

			case 26:

				// Flip animation

				if (localPlayer->overlap(x, y - height, width, height))
					animType = E_LEFTANIM;
				else
					animType = E_RIGHTANIM;

				break;

			case 31:

				// Moving platform

				if (animType == E_LEFTANIM) {

					if (level->checkMaskDown(x, y - (height >> 1)))
					    animType = E_RIGHTANIM;

				} else if (animType == E_RIGHTANIM) {

					if (level->checkMaskDown(x + width, y - (height >> 1)))
					    animType = E_LEFTANIM;

				}
 
				break;

			case 32:

				// Moving platform

				if (x < TTOF(gridX) - (set[E_BRIDGELENGTH] << 14))
					animType = E_RIGHTANIM;
				else if (!animType || (x > TTOF(gridX + set[E_BRIDGELENGTH])))
					animType = E_LEFTANIM;

				break;

			case 33:

				// Sparks-esque following

				if (localPlayer->getFacing() &&
					(x + width < localPlayer->getX())) {

					animType = E_RIGHTANIM;

				} else if (!localPlayer->getFacing() &&
					(x > localPlayer->getX() + F32)) {

					animType = E_LEFTANIM;

				}

				break;

			case 36:

				// Walk from side to side and down hills, staying on-screen

				if (level->checkMaskDown(x + (width >> 1), y)) {

					// Walk from side to side, staying on-screen
					if (animType == E_LEFTANIM) {

						if (level->checkMaskDown(x - F4, y - (height >> 1)) ||
							(x - F4 < viewX))
							animType = E_RIGHTANIM;

					} else if (animType == E_RIGHTANIM) {

						if (level->checkMaskDown(x + width + F4, y - (height >> 1)) ||
							(x + width + F4 > viewX + ITOF(viewW)))
							animType = E_LEFTANIM;

					}

				}

				break;

			case 53:

				// Dreempipes turtles

				if (y > level->getWaterLevel()) {

					if (animType == E_LEFTANIM) {

						if (level->checkMaskDown(x - F4, y - (height >> 1)))
							animType = E_RIGHTANIM;

					} else if (animType == E_RIGHTANIM) {

						if (level->checkMaskDown(x + width + F4, y - (height >> 1)))
							animType = E_LEFTANIM;

					} else animType = E_LEFTANIM;

				}

				break;

			default:

				if (localPlayer->getX() + PXO_MID < x + (width >> 1))
					animType = E_LEFTANIM;
				else
					animType = E_RIGHTANIM;

				break;

		}

	}


	// If the event has been destroyed, play its finishing animation and set its
	// reaction time
	if (set[E_HITSTOKILL] &&
		(level->getEventHits(gridX, gridY) >= set[E_HITSTOKILL]) &&
		(animType != E_LFINISHANIM) && (animType != E_RFINISHANIM)) {

		destroy(ticks);

	}


	// Generate bullet
	if (set[E_BULLETSP]) {

		if ((ticks % (set[E_BULLETSP] * 25) >
			(unsigned int)(set[E_BULLETSP] * 25) - T_SHOOT) &&
			((animType == E_LEFTANIM) || (animType == E_RIGHTANIM))) {

			if (animType == E_LEFTANIM) animType = E_LSHOOTANIM;
			else animType = E_RSHOOTANIM;

			level->setEventTime(gridX, gridY, ticks + T_SHOOT);

		}

	}


	// If the reaction time has expired
	if (level->getEventTime(gridX, gridY) &&
		(ticks > level->getEventTime(gridX, gridY))) {

		if ((animType == E_LFINISHANIM) || (animType == E_RFINISHANIM)) {

			// The event has been destroyed, so remove it
			level->clearEvent(gridX, gridY);

			return true;

		} else if (animType == E_LSHOOTANIM) {

			if ((set[E_BULLET] < 32) &&
				(level->getBullet(set[E_BULLET])[B_SPRITE] != 0))
				level->firstBullet = new Bullet(this, false, ticks);

			animType = E_LEFTANIM;

		} else if (animType == E_RSHOOTANIM) {

			if ((set[E_BULLET] < 32) &&
				(level->getBullet(set[E_BULLET])[B_SPRITE + 1] != 0))
				level->firstBullet = new Bullet(this, true, ticks);

			animType = E_RIGHTANIM;

		} else {

			level->setEventTime(gridX, gridY, 0);

		}

	}


	if (level->getStage() == LS_END) return false;


	// Handle contact with player

	if ((animType != E_LFINISHANIM) && (animType != E_RFINISHANIM)) {

		for (count = 0; count < nPlayers; count++) {

			// Check if the player is touching the event
			if (set[E_MODIFIER] == 6) {

				if (width && height &&
					players[count].overlap(x, y - height, width - F8, height) &&
					(players[count].getY() <= y + (PYS_FALL / msps) - height) &&
					!level->checkMaskDown(players[count].getX() + PXO_MID, PYO_TOP + y - height)) {

					// Player is on a platform

					players[count].setEvent(gridX, gridY);
					players[count].setPosition(players[count].getX() + ((dx * msps) >> 10), y - height);

				} else players[count].clearEvent(gridX, gridY);

			} else {

				// Check if the player is touching the event
				if (width && height &&
					players[count].overlap(x, y - height, width, height)) {

					// If the player picks up the event, destroy it
					if (players[count].touchEvent(gridX, gridY, ticks, msps))
						destroy(ticks);

				}

			}

		}

	}


	return false;

}


void Event::draw (unsigned int ticks, int change) {

	Anim *anim;
	signed char *set;
	int count;


	// Uncomment the following to see the area of the event
	/*drawRect(FTOI(getDrawX(change) - viewX),
		FTOI(getDrawY(change) - (viewY + getHeight())), FTOI(getWidth()),
		FTOI(getHeight()), 88);*/


	// Get the event properties
	set = level->getEvent(gridX, gridY);

	// If the event has been removed from the grid, do not show it
	if (!set) return;


	// Check if the event has anything to draw
	if (!animType || (set[animType] < 0)) return;


	if ((animType == E_LFINISHANIM) || (animType == E_RFINISHANIM))
		frame = (ticks + T_FINISH - level->getEventTime(gridX, gridY)) / 40;
	else if (set[E_ANIMSP])
		frame = ticks / (set[E_ANIMSP] * 40);
	else
		frame = ticks / 20;

	anim = level->getAnim(set[animType]);
	anim->setFrame(frame + gridX + gridY, true);


	// Draw the event

	if (ticks < flashTime) anim->flashPalette(0);

	anim->draw(getDrawX(change), getDrawY(change));

	if (ticks < flashTime) anim->restorePalette();


	// If the event has been destroyed, draw an explosion
	if (set[E_HITSTOKILL] &&
		((animType == E_LFINISHANIM) || (animType == E_RFINISHANIM))) {

		anim = level->getMiscAnim(2);
		anim->setFrame(frame, false);
		anim->draw(getDrawX(change), getDrawY(change));

	}

	if ((set[E_MODIFIER] == 8) && set[E_HITSTOKILL]) {

		// Draw boss energy bar

		count = level->getEventHits(gridX, gridY) * 100 / set[E_HITSTOKILL];


		// Devan head

		anim = level->getMiscAnim(1);
		anim->setFrame(0, true);

		if (ticks < flashTime) anim->flashPalette(0);

		anim->draw(viewX + ITOF(viewW - 44), viewY + ITOF(count + 48));

		if (ticks < flashTime) anim->restorePalette();


		// Bar
		drawRect(viewW - 40, count + 40, 12, 100 - count,
			(ticks < flashTime)? 0: 32);

	}


	return;

}



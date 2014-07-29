
/*
 *
 * bird.cpp
 *
 * 1st March 2009: Created bird.cpp from parts of events.cpp
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


#include "bird.h"
#include "levelplayer.h"

#include "io/gfx/video.h"
#include "level/bullet.h"
#include "level/event/event.h"
#include "level/level.h"


Bird::Bird (Player *rescuer, unsigned char gX, unsigned char gY) {

	player = rescuer;
	x = TTOF(gX);
	y = TTOF(gY);
	dx = 0;
	dy = 0;
	fleeing = false;
	fireTime = 0;

	return;

}


Bird::~Bird () {

	return;

}


void Bird::reset () {

	x = player->getLevelPlayer()->getX();
	y = player->getLevelPlayer()->getY() - F64;
	fireTime = 0;

	return;

}


Player * Bird::getPlayer () {

	return player;

}


void Bird::hit () {

	fleeing = true;

	return;

}


bool Bird::step (unsigned int ticks, int msps) {

	LevelPlayer* levelPlayer;
	Event* event;
	bool target;

	if (fleeing) {

		// Trajectory for flying away
		dx = F80;
		dy = -F80;

		// If the bird has flown off-screen, remove it
		if (y < viewY - F160) return true;

	} else {

		// Trajectory for flying towards the player

		levelPlayer = player->getLevelPlayer();

		if ((x < levelPlayer->getX() - F160) || (x > levelPlayer->getX() + F160)) {

			// Far away from the player
			// Approach the player at a speed proportional to the distance

			dx = levelPlayer->getX() - x;

		} else if (x < levelPlayer->getX()) {

			// To the left of the player, so move right

			if (dx < F80) dx += 400 * msps;

		} else {

			// To the right of the player, so move left

			if (dx > -F80) dx -= 400 * msps;

		}

		if (y > level->getWaterLevel() - F24) {

			// Always stay above water

			y = level->getWaterLevel() - F24;
			dy = 0;

		} else {

			if ((y < levelPlayer->getY() - F100) || (y > levelPlayer->getY() + F100)) {

				// Far away from the player
				// Approach the player at a speed proportional to the distance

				dy = (levelPlayer->getY() - F64) - y;

			} else if (y < levelPlayer->getY() - F64) {

				// Above the player, so move downwards

				if (dy < F80) dy += 400 * msps;

			} else {

				// Below the player, so move upwards

				if (dy > -F80) dy -= 400 * msps;

			}


		}


		if (ticks > fireTime) {

			// Check for nearby targets

			target = false;
			event = level->events;

			if (levelPlayer->getFacing()) {

				while (event && !target) {

					target = event->isEnemy() && event->overlap(x, y, F160, F100);

					event = event->getNext();

				}

			} else {

				while (event && !target) {

					target = event->isEnemy() && event->overlap(x - F160, y, F160, F100);

					event = event->getNext();

				}

			}

			// If there is a target in the vicinity, generate bullets
			if (target) {

				level->bullets = new Bullet(this, false, ticks);

				fireTime = ticks + T_BIRD_FIRE;

			}

		}

	}

	// Apply trajectory
	x += (dx * msps) >> 10;
	y += (dy * msps) >> 10;

	return false;

}


void Bird::draw (unsigned int ticks, int change) {

	Anim *anim;

	anim = level->getAnim((player->getLevelPlayer()->getFacing() || fleeing)? BIRD_RIGHTANIM: BIRD_LEFTANIM);
	anim->setFrame(ticks / 80, true);

	anim->draw(getDrawX(change), getDrawY(change));

	return;

}



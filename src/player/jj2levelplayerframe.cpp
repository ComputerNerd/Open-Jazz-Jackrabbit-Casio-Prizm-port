
/*
 *
 * jj2levelplayerframe.cpp
 *
 * 29th June 2010: Created jj2levelplayerframe.cpp from parts of
 *                 levelplayerframe.cpp
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
 * Provides the once-per-frame functions of players in levels.
 *
 */


#include "jj2levelplayer.h"

#include "game/gamemode.h"
#include "io/controls.h"
#include "io/gfx/font.h"
#include "io/gfx/video.h"
#include "io/sound.h"
#include "jj2level/jj2level.h"
#include "util.h"


void JJ2LevelPlayer::control (unsigned int ticks, int msps) {

	bool platform;

	// Respond to controls, unless the player has been killed

	// If the player has been killed, drop but otherwise do not move
	if (!energy) {

		dx = 0;

		if (floating) dy = 0;
		else {

			dy += PYA_GRAVITY * msps;
			if (dy > PYS_FALL) dy = PYS_FALL;

		}

		animType = facing? PA_RDIE: PA_LDIE;

		return;

	}

	if (player->pcontrols[C_RIGHT]) {

		// Walk/run right

		if (dx < 0) dx += PXA_REVERSE * msps;
		else if (dx < PXS_WALK) dx += PXA_WALK * msps;
		else if (dx < PXS_RUN) dx += PXA_RUN * msps;

		facing = true;

	} else if (player->pcontrols[C_LEFT]) {

		// Walk/run left

		if (dx > 0) dx -= PXA_REVERSE * msps;
		else if (dx > -PXS_WALK) dx -= PXA_WALK * msps;
		else if (dx > -PXS_RUN) dx -= PXA_RUN * msps;

		facing = false;

	} else {

		// Slow down

		if (dx > 0) {

			if (dx < PXA_STOP * msps) dx = 0;
			else dx -= PXA_STOP * msps;

		}

		if (dx < 0) {

			if (dx > -PXA_STOP * msps) dx = 0;
			else dx += PXA_STOP * msps;

		}

	}

	if (dx < -PXS_RUN) dx = -PXS_RUN;
	if (dx > PXS_RUN) dx = PXS_RUN;


	// Check for platform event, bridge or level mask below player
	platform = /*(event >= 3) ||*/
		jj2Level->checkMaskDown(x + PXO_ML, y + 1) ||
		jj2Level->checkMaskDown(x + PXO_MID, y + 1) ||
		jj2Level->checkMaskDown(x + PXO_MR, y + 1) ||
		((dx > 0) && jj2Level->checkMaskDown(x + PXO_ML, y + F8)) ||
		((dx < 0) && jj2Level->checkMaskDown(x + PXO_MR, y + F8));


	if (floating) {

		if (player->pcontrols[C_UP]) {

			// Fly upwards

			if (dy > 0) dy -= PXA_REVERSE * msps;
			else if (dy > -PXS_WALK) dy -= PXA_WALK * msps;
			else if (dy > -PXS_RUN) dy -= PXA_RUN * msps;

		} else if (player->pcontrols[C_DOWN]) {

			// Fly downwards

			if (dy < 0) dy += PXA_REVERSE * msps;
			else if (dy < PXS_WALK) dy += PXA_WALK * msps;
			else if (dy < PXS_RUN) dy += PXA_RUN * msps;

		} else {

			// Slow down

			if (dy > 0) {

				if (dy < PXA_STOP * msps) dy = 0;
				else dy -= PXA_STOP * msps;

			}

			if (dy < 0) {

				if (dy > -PXA_STOP * msps) dy = 0;
				else dy += PXA_STOP * msps;

			}

		}

		/*if (event) {

			if (event == 1) dy = jj2Level->getEvent(eventX, eventY)[E_MULTIPURPOSE] * -F20;
			else if (event == 2) dy = PYS_JUMP;

		}*/

		if (dy < -PXS_RUN) dy = -PXS_RUN;
		if (dy > PXS_RUN) dy = PXS_RUN;

	} else if (y + PYO_MID > jj2Level->getWaterLevel()) {

		if (player->pcontrols[C_SWIM]) {

			// Swim upwards

			if (dy > 0) dy -= PXA_REVERSE * msps;
			else if (dy > -PXS_WALK) dy -= PXA_WALK * msps;
			else if (dy > -PXS_RUN) dy -= PXA_RUN * msps;

			// Prepare to jump upon leaving the water

			if (!jj2Level->checkMaskUp(x + PXO_MID, y - F36)) {

				jumpY = y - jumpHeight;

				if (dx < 0) jumpY += dx >> 4;
				else if (dx > 0) jumpY -= dx >> 4;

				//event = 0;

			}

		} else if (player->pcontrols[C_DOWN]) {

			// Swim downwards

			if (dy < 0) dy += PXA_REVERSE * msps;
			else if (dy < PXS_WALK) dy += PXA_WALK * msps;
			else if (dy < PXS_RUN) dy += PXA_RUN * msps;

		} else {

			// Sink

			dy += PYA_SINK * msps;
			if (dy > PYS_SINK) dy = PYS_SINK;

		}

		if (dy < -PXS_RUN) dy = -PXS_RUN;
		if (dy > PXS_RUN) dy = PXS_RUN;

	} else {

		if (platform && player->pcontrols[C_JUMP] &&
			!jj2Level->checkMaskUp(x + PXO_MID, y - F36)) {

			// Jump

			jumpY = y - jumpHeight;

			// Increase jump height if walking/running
			if (dx < 0) jumpY += dx >> 3;
			else if (dx > 0) jumpY -= dx >> 3;

			//event = 0;

			playSound(S_JUMPA);

		}

		// Stop jumping
		if (!player->pcontrols[C_JUMP] /*&& (event != 1) && (event != 2)*/)
			jumpY = TTOF(256);

		if (y >= jumpY) {

			// If jumping, rise

			dy = (jumpY - y - F64) * 4;

			// Spring/float up speed limit
			/*if ((event == 1) || (event == 2)) {

				speed = jj2Level->getEvent(eventX, eventY)[E_MULTIPURPOSE] * -F20;

				if (speed >= 0) speed = PYS_JUMP;

				if (dy < speed) dy = speed;

			}*/

			// Avoid jumping too fast, unless caused by an event
			if (/* !event &&*/ (dy < PYS_JUMP)) dy = PYS_JUMP;

		} else if (!platform) {

			// Fall under gravity
			dy += PYA_GRAVITY * msps;
			if (dy > PYS_FALL) dy = PYS_FALL;

		}

		// Don't descend through platforms
		//if ((dy > 0) && (event >= 3)) dy = 0;

		if (platform && !lookTime) {

			// If requested, look up or down
			if (player->pcontrols[C_UP]) lookTime = -ticks;
			else if (player->pcontrols[C_DOWN]) lookTime = ticks;

		}

		// Stop looking if there is no platform or the control has been released
		if (!platform ||
			(!player->pcontrols[C_UP] && (lookTime < 0)) ||
			(!player->pcontrols[C_DOWN] && (lookTime > 0))) lookTime = 0;

	}

	// If there is an obstacle above and the player is not floating up, stop
	// rising
	if (jj2Level->checkMaskUp(x + PXO_MID, y + PYO_TOP - F4) && (jumpY < y) /*&& (event != 2)*/) {

		jumpY = TTOF(256);
		if (dy < 0) dy = 0;

		//if ((event != 3) && (event != 4)) event = 0;

	}

	// If jump completed, stop rising
	if (y <= jumpY) {

		jumpY = TTOF(256);

		//if ((event != 3) && (event != 4)) event = 0;

	}


	// Handle firing
	if (player->pcontrols[C_FIRE]) {

		if (ticks > fireTime) {

			// Make sure bullet position is taken from correct animation
			if (platform) animType = facing? PA_RSHOOT: PA_LSHOOT;

			// TODO: Create new bullet

			// Set when the next bullet can be fired
			if (player->fireSpeed) fireTime = ticks + (1000 / player->fireSpeed);
			else fireTime = 0x7FFFFFFF;

			// Remove the bullet from the arsenal
			if (player->ammoType != -1) player->ammo[player->ammoType]--;

			/* If the current ammo type has been exhausted, use the previous
			non-exhausted ammo type */
			while ((player->ammoType > -1) && !player->ammo[player->ammoType]) player->ammoType--;

		}

	} else fireTime = 0;


	// Check for a change in ammo
	if (player->pcontrols[C_CHANGE]) {

		if (player == localPlayer) controls.release(C_CHANGE);

		player->ammoType = ((player->ammoType + 2) % 5) - 1;

		// If there is no ammo of this type, go to the next type that has ammo
		while ((player->ammoType > -1) && !player->ammo[player->ammoType])
			player->ammoType = ((player->ammoType + 2) % 5) - 1;

	}


	// Deal with the bird

	/*if (bird) {

		if (bird->step(ticks, msps)) {

			delete bird;
			bird = NULL;

		}

	}*/


	// Choose animation

	if ((reaction == JJ2PR_HURT) && (reactionTime - ticks > PRT_HURT - PRT_HURTANIM))
		animType = facing? PA_RHURT: PA_LHURT;

	else if (y + PYO_MID > jj2Level->getWaterLevel())
		animType = facing? PA_RSWIM: PA_LSWIM;

	else if (floating) animType = facing? PA_RBOARD: PA_LBOARD;

	else if (dy < 0) {

		/*if (event == 1) animType = facing? PA_RSPRING: PA_LSPRING;
		else*/ animType = facing? PA_RJUMP: PA_LJUMP;

	} else if (platform) {

		if (dx) {

			if (dx <= -PXS_RUN) animType = PA_LRUN;
			else if (dx >= PXS_RUN) animType = PA_RRUN;
			else if ((dx < 0) && facing) animType = PA_LSTOP;
			else if ((dx > 0) && !facing) animType = PA_RSTOP;
			else animType = facing? PA_RWALK: PA_LWALK;

		} else if (!jj2Level->checkMaskDown(x + PXO_ML, y + F12) &&
			!jj2Level->checkMaskDown(x + PXO_L, y + F2) /*&&
			(event != 3) && (event != 4)*/)
			animType = PA_LEDGE;

		else if (!jj2Level->checkMaskDown(x + PXO_MR, y + F12) &&
			!jj2Level->checkMaskDown(x + PXO_R, y + F2) /*&&
			(event != 3) && (event != 4)*/)
			animType = PA_REDGE;

		else if ((lookTime < 0) && ((int)ticks > 1000 - lookTime))
			animType = PA_LOOKUP;

		else if (lookTime > 0) {

			if ((int)ticks < 1000 + lookTime) animType = facing? PA_RCROUCH: PA_LCROUCH;
			else animType = PA_LOOKDOWN;

		}

		else if (player->pcontrols[C_FIRE])
			animType = facing? PA_RSHOOT: PA_LSHOOT;

		else
			animType = facing? PA_RSTAND: PA_LSTAND;

	} else animType = facing? PA_RFALL: PA_LFALL;


	return;

}


void JJ2LevelPlayer::move (unsigned int ticks, int msps) {

	fixed pdx, pdy;
	int count;

	if (warpTime && (ticks > warpTime)) {

		x = TTOF(warpX);
		y = TTOF(warpY + 1);
		warpTime = 0;

	}

	// Apply as much of the trajectory as possible, without going into the
	// scenery

	if (fastFeetTime > ticks) {

		pdx = (dx * msps * 3) >> 11;
		pdy = (dy * msps * 3) >> 11;

	} else {

		pdx = (dx * msps) >> 10;
		pdy = (dy * msps) >> 10;

	}

	// First for the vertical component of the trajectory

	if (pdy < 0) {

		// Moving up

		count = (-pdy) >> 12;

		while (count > 0) {

			if (jj2Level->checkMaskUp(x + PXO_MID, y + PYO_TOP - F4)) {

				y &= ~4095;
				dy = 0;

				break;

			}

			y -= F4;
			count--;

		}

		pdy = (-pdy) & 4095;

		if (!jj2Level->checkMaskUp(x + PXO_MID, y + PYO_TOP - pdy))
			y -= pdy;
		else {

			y &= ~4095;
			dy = 0;

		}

	} else if (pdy > 0) {

		// Moving down

		count = pdy >> 12;

		while (count > 0) {

			if (jj2Level->checkMaskDown(x + PXO_ML, y + F4) ||
				jj2Level->checkMaskDown(x + PXO_MID, y + F4) ||
				jj2Level->checkMaskDown(x + PXO_MR, y + F4)) {

				y |= 4095;
				dy = 0;

				break;

			}

			y += F4;
			count--;

		}

		pdy &= 4095;

		if (!(jj2Level->checkMaskDown(x + PXO_ML, y + pdy) ||
			jj2Level->checkMaskDown(x + PXO_MID, y + pdy) ||
			jj2Level->checkMaskDown(x + PXO_MR, y + pdy)))
			y += pdy;
		else {

			y |= 4095;
			dy = 0;

		}

	}



	// Then for the horizontal component of the trajectory

	if (pdx < 0) {

		// Moving left

		count = (-pdx) >> 12;

		while (count > 0) {

			// If there is an obstacle, stop
			if (jj2Level->checkMaskUp(x + PXO_L - F4, y + PYO_MID)) {

				x &= ~4095;

				break;

			}

			x -= F4;
			count--;

			// If on an uphill slope, push the player upwards
			if (jj2Level->checkMaskUp(x + PXO_ML, y) &&
				!jj2Level->checkMaskUp(x + PXO_ML, y - F4)) y -= F4;

		}

		pdx = (-pdx) & 4095;

		if (!jj2Level->checkMaskUp(x + PXO_L - pdx, y + PYO_MID)) x -= pdx;
		else x &= ~4095;

		// If on an uphill slope, push the player upwards
		while (jj2Level->checkMaskUp(x + PXO_ML, y) &&
			!jj2Level->checkMaskUp(x + PXO_ML, y - F4)) y -= F1;

	} else if (pdx > 0) {

		// Moving right

		count = pdx >> 12;

		while (count > 0) {

			// If there is an obstacle, stop
			if (jj2Level->checkMaskUp(x + PXO_R + F4, y + PYO_MID)) {

				x |= 4095;

				break;

			}

			x += F4;
			count--;

			// If on an uphill slope, push the player upwards
			if (jj2Level->checkMaskUp(x + PXO_MR, y) &&
				!jj2Level->checkMaskUp(x + PXO_MR, y - F4)) y -= F4;

		}

		pdx &= 4095;

		if (!jj2Level->checkMaskUp(x + PXO_R + pdx, y + PYO_MID)) x += pdx;
		else x |= 4095;

		// If on an uphill slope, push the player upwards
		while (jj2Level->checkMaskUp(x + PXO_MR, y) &&
			!jj2Level->checkMaskUp(x + PXO_MR, y - F4)) y -= F1;

	}


	// If using a float up event and have hit a ceiling, ignore event
	/*if ((event == 2) && jj2Level->checkMaskUp(x + PXO_MID, y + PYO_TOP - F4)) {

		jumpY = TTOF(256);
		event = 0;

	}*/


	if (jj2Level->getStage() == LS_END) return;


	// Handle spikes
	if (jj2Level->checkSpikes(x + PXO_MID, y + PYO_TOP - F4) ||
		jj2Level->checkSpikes(x + PXO_MID, y + F4) ||
		jj2Level->checkSpikes(x + PXO_L - F4, y + PYO_MID) ||
		jj2Level->checkSpikes(x + PXO_R + F4, y + PYO_MID)) hit(NULL, ticks);


	return;

}


void JJ2LevelPlayer::view (unsigned int ticks, int mspf) {

	int oldViewX, oldViewY, speed;

	// Calculate viewport

	// Record old viewport position for applying lag
	oldViewX = viewX;
	oldViewY = viewY;

	// There is no panel, so use the whole height
	viewH = canvasH;

	// Find new position

	viewX = x + F8 - (viewW << 9);
	viewY = y - F24 - (viewH << 9);

	if ((lookTime > 0) && ((int)ticks > 1000 + lookTime)) {

		// Look down
		if ((int)ticks < 2000 + lookTime) viewY += 64 * (ticks - (1000 + lookTime));
		else viewY += F64;

	} else if ((lookTime < 0) && ((int)ticks > 1000 - lookTime)) {

		// Look up
		if ((int)ticks < 2000 - lookTime) viewY -= 64 * (ticks - (1000 - lookTime));
		else viewY -= F64;

	}


	// Apply lag proportional to player "speed"
	speed = ((dx >= 0? dx: -dx) + (dy >= 0? dy: -dy)) >> 14;

	if (speed && (mspf < speed)) {

		viewX = ((oldViewX * (speed - mspf)) + (viewX * mspf)) / speed;
		viewY = ((oldViewY * (speed - mspf)) + (viewY * mspf)) / speed;

	}


	return;

}

void JJ2LevelPlayer::draw (unsigned int ticks, int change) {

	Anim *an;
	int frame;
	fixed drawX, drawY;

	// The current frame for animations
	if (reaction == JJ2PR_KILLED) frame = (ticks + PRT_KILLED - reactionTime) / 75;
	else frame = ticks / 75;


	// Get position

	drawX = getDrawX(change);
	drawY = getDrawY(change);


	// Choose sprite

	an = jj2Level->getAnim(getAnim());
	an->setFrame(frame, reaction != JJ2PR_KILLED);


	// Show the player

	// Flash red if hurt, otherwise use player colour
	if ((reaction == JJ2PR_HURT) && (!((ticks / 30) & 3)))
		an->flashPalette(36);

	else {

		an->setPalette(palette, 23, 41);
		an->setPalette(palette, 88, 8);

	}


	// Draw "motion blur"
	if (fastFeetTime > ticks) an->draw(drawX - (dx >> 6), drawY);

	// Draw player
	an->draw(drawX, drawY);


	// Remove red flash or player colour from sprite
	an->restorePalette();


	// Uncomment the following to see the area of the player
	/*drawRect(FTOI(drawX + PXO_L - viewX),
		FTOI(drawY + PYO_TOP - viewY),
		FTOI(PXO_R - PXO_L),
		FTOI(-PYO_TOP), 89);
	drawRect(FTOI(drawX + PXO_ML - viewX),
		FTOI(drawY + PYO_TOP - viewY),
		FTOI(PXO_MR - PXO_ML),
		FTOI(-PYO_TOP), 88);*/


	if (reaction == JJ2PR_INVINCIBLE) {

		// TODO: Show invincibility effect

	}

	switch (shield) {

		case JJ2S_NONE:

			// Do nothing

			break;

		case JJ2S_FLAME:

			// TODO: Show shield effect

			break;

		case JJ2S_BUBBLE:

			// TODO: Show shield effect

			break;

		case JJ2S_PLASMA:

			// TODO: Show shield effect

			break;

		case JJ2S_LASER:

			// TODO: Show shield effect

			break;

	}


	// Show the bird
	//if (bird) bird->draw(ticks, change);


	// Show the player's name
	if (gameMode)
		panelBigFont->showString(player->name,
			FTOI(drawX + PXO_MID) - (panelBigFont->getStringWidth(player->name) >> 1),
			FTOI(drawY - F32 - F16));

	return;

}



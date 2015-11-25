
/**
 *
 * @file jj1levelframe.cpp
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 23rd August 2005: Created level.c
 * 3rd February 2009: Renamed level.c to level.cpp
 * 19th July 2009: Created levelframe.cpp from parts of level.cpp
 * 30th March 2010: Created baselevel.cpp from parts of level.cpp and
 *                  levelframe.cpp
 * 29th June 2010: Created jj2levelframe.cpp from parts of levelframe.cpp
 * 1st August 2012: Renamed levelframe.cpp to jj1levelframe.cpp
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
 * @section Description
 * Provides the once-per-frame functions for levels.
 *
 */


#include "jj1bullet.h"
#include "jj1event/jj1event.h"
#include "jj1event/jj1guardians.h"
#include "jj1level.h"
#include "jj1levelplayer/jj1levelplayer.h"

#include "game/game.h"
#include "game/gamemode.h"
#include "io/controls.h"
#include "io/gfx/font.h"
#include "io/gfx/video.h"
#include "util.h"
#include "surface.h"

/**
 * Level iteration.
 *
 * @return Error code
 */
int JJ1Level::step () {

	JJ1Event *event;
	int viewH;
	int x, y;


	// Can we see below the panel?
	if (canvasW > SW) viewH = canvasH;
	else viewH = canvasH - 33;

	// Search for active events
	for (y = FTOT(viewY) - 5; y < ITOT(FTOI(viewY) + viewH) + 5; y++) {

		for (x = FTOT(viewX) - 5; x < ITOT(FTOI(viewX) + canvasW) + 5; x++) {

			if ((x >= 0) && (y >= 0) && (x < LW) && (y < LH) &&
				grid[y][x].event && (grid[y][x].event < 121) &&
				(eventSet[grid[y][x].event].difficulty <= game->getDifficulty())) {

				event = events;

				while (event) {

					// If the event has been found, stop searching
					if (event->isFrom(x, y)) break;

					event = event->getNext();

				}

				// If the event wasn't found, create it
				if (!event) {

					switch (getEvent(x, y)->movement) {

						case 28:

							events = new JJ1Bridge(x, y);

							break;

						case 41:

							events = new MedGuardian(x, y);

							break;

						case 60:

							events = new DeckGuardian(x, y);

							break;

						default:

							events = new JJ1StandardEvent(eventSet + grid[y][x].event, x, y, TTOF(x), TTOF(y + 1));

							break;

					}

				}

			}

		}

	}


	// Process bullets
	if (bullets) bullets = bullets->step(ticks);

	// Determine the players' trajectories
	for (x = 0; x < nPlayers; x++) players[x].getJJ1LevelPlayer()->control(ticks);

	// Process active events
	if (events) events = events->step(ticks);

	// Apply as much of those trajectories as possible, without going into the
	// scenery
	for (x = 0; x < nPlayers; x++) players[x].getJJ1LevelPlayer()->move(ticks);


	// Check if time has run out
	if (ticks > endTime) {

			if ((game->getDifficulty() >= 2) && (stage == LS_NORMAL))
				localPlayer->getJJ1LevelPlayer()->kill(NULL, endTime);

	}


	// Handle change in ammo selection

	x = localPlayer->getAmmo(false) + 1;

	if (x != ammoType) {

		// Change the ammo type display on the panel
		ammoType = x;
		ammoOffset = ITOF(26);

	}

	if (ammoOffset > 0) {

		// Descending
		ammoOffset -= F1;

		// Avoid an offset of 0, which prevents changes
		if (ammoOffset == 0) ammoOffset = -1;

	}


	// Handle change in water level
	if (waterLevel < waterLevelTarget) waterLevelSpeed += 3200;
	else waterLevelSpeed -= 3200;
	if (waterLevelSpeed > 80000) waterLevelSpeed = 80000;
	if (waterLevelSpeed < -80000) waterLevelSpeed = -80000;

	waterLevel += waterLevelSpeed >> 6;


	// Handle player reactions
	for (x = 0; x < nPlayers; x++) {

		if (players[x].getJJ1LevelPlayer()->reacted(ticks) == PR_KILLED) {

			players[x].clearAmmo();

			return LOST;

			game->resetPlayer(players + x);

		}

	}


	return E_NONE;

}



/**
 * Draw the level.
 */
void JJ1Level::draw () {

	GridElement *ge;
	//SDL_Rect dst;
	short src[4];//x y w h
	int viewH;
	int vX, vY;
	int x, y, bgScale;
	unsigned int change;

	// Calculate change since last step
	change = getTimeChange();

	// Calculate viewport
	if (game && (stage == LS_END)) game->view(paused? 0: ((ticks - prevTicks) * 160));
	else localPlayer->getJJ1LevelPlayer()->view(ticks, paused? 0: (ticks - prevTicks), change);

	// Can we see below the panel?
	if (canvasW > SW) viewH = canvasH;
	else viewH = canvasH - 33;

	// Ensure the new viewport is within the level
	if (FTOI(viewX) + canvasW >= TTOI(LW)) viewX = ITOF(TTOI(LW) - canvasW);
	if (viewX < 0) viewX = 0;
	if (FTOI(viewY) + viewH >= TTOI(LH)) viewY = ITOF(TTOI(LH) - viewH);
	if (viewY < 0) viewY = 0;

	// Use the viewport
	/*dst.x = 0;
	dst.y = 0;
	
	dst.w = canvasW;
	dst.h = viewH;*/
	//SDL_SetClipRect(canvas, &dst);
	vX = FTOI(viewX);
	vY = FTOI(viewY);

	// Set tile drawing dimensions
	src[2] = TTOI(1);//each tile is 32 pixels
	src[3] = TTOI(1);
	src[0] = 0;


	// If there is a sky, draw it
	if (sky) {

		// Background scale
		if (canvasW > 320) bgScale = ((canvasH - 1) / 100) + 1;
		else bgScale = ((canvasH - 34) / 100) + 1;

		for (y = 0; y < viewH; y += bgScale)
			drawRect(0, y, canvasW, bgScale, 156 + (y / bgScale));


		// Show sun / moon / etc.
		if (skyOrb) {

			src[1] = TTOI(skyOrb);
			blitPartToCanvas(&tileSet,((canvasW * 4) / 5) - (vX & 3),((canvasH - 33) * 3) / 25,src);
		}

	} else {

		// If there is no sky, draw a blank background
		// This is only very occasionally actually visible
		video.clearScreen(127);

	}



	// Show background tiles

	for (y = 0; y <= ITOT(viewH - 1) + 1; y++) {

		for (x = 0; x <= ITOT(canvasW - 1) + 1; x++) {

			if ((x + ITOT(vX) >= 256) || (y + ITOT(vY) >= 64)) {

				drawRect(TTOI(x) - (vX & 31), TTOI(y) - (vY & 31), 32, 32, LEVEL_BLACK);

				continue;

			}

			// Get the grid element from the given coordinates
			ge = grid[y + ITOT(vY)] + x + ITOT(vX);

			// If this tile uses a black background, draw it
			if (ge->bg)
				drawRect(TTOI(x) - (vX & 31), TTOI(y) - (vY & 31), 32, 32, LEVEL_BLACK);


			// If this is not a foreground tile, draw it
			if ((ge->event != 124) &&
				(ge->event != 125) &&
				(eventSet[ge->event].movement != 37) &&
				(eventSet[ge->event].movement != 38)) {

				//dst.x = TTOI(x) - (vX & 31);
				//dst.y = TTOI(y) - (vY & 31);
				src[1] = TTOI(ge->tile);
				//SDL_BlitSurface(tileSet, &src, canvas, &dst);
				blitPartToCanvas(&tileSet,TTOI(x) - (vX & 31),TTOI(y) - (vY & 31),src);
			}

		}

	}


	// Show active events
	if (events) events->draw(ticks, change);


	// Show the players
	for (x = 0; x < nPlayers; x++) players[x].getJJ1LevelPlayer()->draw(ticks, change);


	// Show bullets
	if (bullets) bullets->draw(change);



	// Show foreground tiles

	for (y = 0; y <= ITOT(viewH - 1) + 1; y++) {

		for (x = 0; x <= ITOT(canvasW - 1) + 1; x++) {

			if ((x + ITOT(vX) >= 256) || (y + ITOT(vY) >= 64)) continue;

			// Get the grid element from the given coordinates
			ge = grid[y + ITOT(vY)] + x + ITOT(vX);

			// If this is an "animated" foreground tile, draw it
			if (ge->event == 123) {

				//dst.x = TTOI(x) - (vX & 31);
				//dst.y = TTOI(y) - (vY & 31);
				if (ticks & 64) src[1] = TTOI(eventSet[ge->event].multiB);
				else src[1] = TTOI(eventSet[ge->event].multiA);
				//SDL_BlitSurface(tileSet, &src, canvas, &dst);
				blitPartToCanvas(&tileSet,TTOI(x) - (vX & 31),TTOI(y) - (vY & 31),src);
			}

			// If this is a foreground tile, draw it
			if ((ge->event == 124) ||
				(ge->event == 125) ||
				(eventSet[ge->event].movement == 37) ||
				(eventSet[ge->event].movement == 38)) {

				//dst.x = TTOI(x) - (vX & 31);
				//dst.y = TTOI(y) - (vY & 31);
				src[1] = TTOI(ge->tile);
				//SDL_BlitSurface(tileSet, &src, canvas, &dst);
				blitPartToCanvas(&tileSet,TTOI(x) - (vX & 31),TTOI(y) - (vY & 31),src);
			}

		}

	}

	// Temporary lines showing the water level
	drawRect(0, FTOI(waterLevel - viewY), canvasW, 2, 24);
	drawRect(0, FTOI(waterLevel - viewY) + 3, canvasW, 1, 24);
	drawRect(0, FTOI(waterLevel - viewY) + 6, canvasW, 1, 24);
	drawRect(0, FTOI(waterLevel - viewY) + 10, canvasW, 1, 24);

	// Show active guardian's energy bar
	if (events) events->drawEnergy(ticks);


	// If this is a competitive game, draw the score

	// Show panel

	blitToCanvas(&panel,0,canvasH-33);

	if (ammoOffset) {

		if (ammoOffset < 0) {

			// Finished descending
			ammoOffset = 0;

		}

		src[0] = 0;
		src[1] = FTOI(ammoOffset);
		src[2] = 64;
		src[3] = 26 - src[1];
		blitPartToCanvas(&panelAmmo[ammoType],248,canvasH-(33-3),src);
	}else
		blitToCanvas(&panelAmmo[ammoType],248,canvasH-(33-3));

	drawRect(0, canvasH - 1, SW, 1, LEVEL_BLACK);


	// Show panel data

	// Show score
	panelSmallFont->showNumber(localPlayer->getScore(), 84, canvasH - 27);

	// Show time remaining
	if (endTime > ticks) x = endTime - ticks;
	else x = 0;
	y = x / (60 * 1000);
	panelSmallFont->showNumber(y, 116, canvasH - 27);
	x -= (y * 60 * 1000);
	y = x / 1000;
	panelSmallFont->showNumber(y, 136, canvasH - 27);
	x -= (y * 1000);
	y = x / 100;
	panelSmallFont->showNumber(y, 148, canvasH - 27);

	// Show lives
	panelSmallFont->showNumber(localPlayer->getLives(), 124, canvasH - 13);

	// Show planet number


	if (worldNum <= 41) // Main game levels
		panelSmallFont->showNumber((worldNum % 3) + 1, 184, canvasH - 13);
	else if ((worldNum >= 50) && (worldNum <= 52)) // Christmas levels
		panelSmallFont->showNumber(worldNum - 49, 184, canvasH - 13);
	else panelSmallFont->showNumber(worldNum, 184, canvasH - 13);

	// Show level number
	panelSmallFont->showNumber(levelNum + 1, 196, canvasH - 13);

	// Show ammo
	if (localPlayer->getAmmo(false) == -1) {

		panelSmallFont->showString(":", 225, canvasH - 13);
		panelSmallFont->showString(";", 233, canvasH - 13);

	} else panelSmallFont->showNumber(localPlayer->getAmmo(true), 245, canvasH - 13);


	// Draw the health bar

	//dst.x = 20;
	int dstx=20;
	x = localPlayer->getJJ1LevelPlayer()->getEnergy();
	y = (ticks - prevTicks) * 40;

	if (FTOI(energyBar) < (x << 4)) {

		if ((x << 14) - energyBar < y) energyBar = x << 14;
		else energyBar += y;

	} else if (FTOI(energyBar) > (x << 4)) {

		if (energyBar - (x << 14) < y) energyBar = x << 14;
		else energyBar -= y;

	}
	int dstw=canvasW;
	if (energyBar > F1) {

		dstw = FTOI(energyBar) - 1;

		// Choose energy bar colour
		if (x == 4) x = 24;
		else if (x == 3) x = 17;
		else if (x == 2) x = 80;
		else if (x <= 1) x = 32 + (((ticks / 75) * 4) & 15);

		// Draw energy bar
		drawRect(20, canvasH - 13, dstw, 7, x);

		dstx += dstw;
		dstw = 64 - dstw;

	} else dstw = 64;


	// Fill in remaining energy bar space with black
	drawRect(dstx, canvasH - 13, dstw, 7, LEVEL_BLACK);


	return;

}




/**
 *
 * @file sprite.cpp
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 19th March 2009: Created sprite.cpp from parts of event.cpp and player.cpp
 * 26th July 2009: Created anim.cpp from parts of sprite.cpp
 *
 * @section Licence
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


#include "video.h"
#include "sprite.h"


Sprite::Sprite () {

	pixels = NULL;

	return;

}


Sprite::~Sprite () {

	if (pixels) SDL_FreeSurface(pixels);

	return;

}


void Sprite::clearPixels () {

	unsigned char data;

	if (pixels) SDL_FreeSurface(pixels);

	data = 0;
	pixels = createSurface(&data, 1, 1);
	SDL_SetColorKey(pixels, SDL_SRCCOLORKEY, 0);

	return;

}


void Sprite::setPixels (unsigned char *data, int width, int height, unsigned char key) {

	if (pixels) SDL_FreeSurface(pixels);

	pixels = createSurface(data, width, height);
	SDL_SetColorKey(pixels, SDL_SRCCOLORKEY, key);

	return;

}


int Sprite::getWidth () {

	return pixels->w;

}


int Sprite::getHeight() {

	return pixels->h;

}


void Sprite::setPalette (SDL_Color *palette, int start, int amount) {

	SDL_SetPalette(pixels, SDL_LOGPAL, palette + start, start, amount);

	return;

}


void Sprite::flashPalette (int index) {

	SDL_Color palette[256];
	int count;

	// Map the whole palette to one index

	for (count = 0; count < 256; count++)
		palette[count].r = palette[count].g = palette[count].b = index;

	SDL_SetPalette(pixels, SDL_LOGPAL, palette, 0, 256);

	return;

}


void Sprite::restorePalette () {

	video.restoreSurfacePalette(pixels);

	return;

}


void Sprite::draw (int x, int y, bool includeOffsets) {

	SDL_Rect dst;

	if (includeOffsets) {

		dst.x = x + xOffset;
		dst.y = y + yOffset;

	}
	else {

		dst.x = x;
		dst.y = y;

	}

	SDL_BlitSurface(pixels, NULL, canvas, &dst);

	return;

}


void Sprite::drawScaled (int x, int y, fixed scale) {

	unsigned char* srcRow;
	unsigned char* dstRow;
	unsigned char pixel, key;
	int width, height, fullWidth, fullHeight;
	int dstX, dstY;
	int srcX, srcY;

	key = pixels->format->colorkey;

	fullWidth = FTOI(pixels->w * scale);
	if (x < -(fullWidth >> 1)) return; // Off-screen
	if (x + (fullWidth >> 1) > canvasW) width = canvasW + (fullWidth >> 1) - x;
	else width = fullWidth;

	fullHeight = FTOI(pixels->h * scale);
	if (y < -(fullHeight >> 1)) return; // Off-screen
	if (y + (fullHeight >> 1) > canvasH) height = canvasH + (fullHeight >> 1) - y;
	else height = fullHeight;

	if (SDL_MUSTLOCK(canvas)) SDL_LockSurface(canvas);

	if (y < (fullHeight >> 1)) {

		srcY = (fullHeight >> 1) - y;
		dstY = 0;

	} else {

		srcY = 0;
		dstY = y - (fullHeight >> 1);

	}

	while (srcY < height) {

		srcRow = ((unsigned char *)(pixels->pixels)) + (pixels->pitch * DIV(srcY, scale));
		dstRow = ((unsigned char *)(canvas->pixels)) + (canvas->pitch * dstY);

		if (x < (fullWidth >> 1)) {

			srcX = (fullWidth >> 1) - x;
			dstX = 0;

		} else {

			srcX = 0;
			dstX = x - (fullWidth >> 1);

		}

		while (srcX < width) {

			pixel = srcRow[DIV(srcX, scale)];
			if (pixel != key) dstRow[dstX] = pixel;

			srcX++;
			dstX++;

		}

		srcY++;
		dstY++;

	}

	if (SDL_MUSTLOCK(canvas)) SDL_UnlockSurface(canvas);

	return;

}


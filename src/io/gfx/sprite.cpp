
/**
 *
 * @file sprite.cpp
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 23rd August 2005: Created level.c
 * 1st January 2006: Created events.c from parts of level.c
 * 3rd February 2009: Renamed events.c to events.cpp and level.c to level.cpp,
 *                    created player.cpp
 * 5th February 2009: Added parts of events.cpp and level.cpp to player.cpp
 * 19th March 2009: Created sprite.cpp from parts of event.cpp and player.cpp
 * 26th July 2009: Created anim.cpp from parts of sprite.cpp
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
 */


#include "video.h"
#include "sprite.h"
#include "surface.h"
#include "mem.h"
#include <string.h>
/**
 * Create a sprite.
 */
Sprite::Sprite(){
	//pixels = NULL;
	xOffset = 0;
	yOffset = 0;
	pixelsid=INVALID_OBJ;
	//palid=INVALID_OBJ;
}


/**
 * Delete the sprite.
 */
Sprite::~Sprite(){
	if (pixelsid!=INVALID_OBJ) freeobj(pixelsid);
	//if(palid!=INVALID_OBJ) freeobj(palid);
}

/**
 * Make the sprite blank.
 */
void Sprite::clearPixels () {
	if (pixelsid!=INVALID_OBJ) freeobj(pixelsid);
	//if (palid!=INVALID_OBJ) freeobj(palid);
	pixelsid=INVALID_OBJ;
	initMiniSurface(&pixels,NULL,1,1);
	//SDL_SetColorKey(pixels, SDL_SRCCOLORKEY, 0);
	setColKey(&pixels,0);
}


void Sprite::setOffset (short int x, short int y) {

	xOffset = x;
	yOffset = y;

	return;

}


/**
 * Set new pixel data for the sprite.
 *
 * @param data The new pixel data
 * @param width The width of the sprite image
 * @param height The height of the sprite image
 * @param key The transparent pixel value
 */
void Sprite::setPixels(unsigned char *data, int width, int height, unsigned char key){
	if((width!=0)&&(height!=0)){
		if (pixelsid==INVALID_OBJ)
			addobj(width*height,&pixelsid);
		else
			resizeobj(pixelsid,width*height);
		memcpy(objs[pixelsid].ptr,data,width*height);
		initMiniSurface(&pixels,objs[pixelsid].ptr,width,height);
		//pixels = createSurface(data, width, height);
		//SDL_SetColorKey(pixels, SDL_SRCCOLORKEY, key);
		setColKey(&pixels,key);
	}else{
		#ifndef CASIO
			printf("W: %d H: %d\n",width,height);
		#endif
		clearPixels();
	}
	return;

}

/**
 * Set the sprite's palette, or a portion thereof.
 *
 * @param palette New palette
 * @param start First colour to change
 * @param amount Number of colours to change
 */
void Sprite::setPalette (unsigned short *palette, int start, int amount){
	/*if(palid==INVALID_OBJ) addobj(256,&palid);
	pixels.start=start;
	pixels.length=amount;
	pixels.palette=(unsigned char *)objs[palid].ptr;
	pixels.flags|=miniS_useMap;
	unsigned int x;
	for(x=start;x<start+amount;++x){
		pixels.palette[x]=palette[x].r;
		//palMap[x*3+1]=palette[x].g;
		//palMap[x*3+2]=palette[x].b;
	}*/
	//SDL_SetPalette(pixels, SDL_LOGPAL, palette + start, start, amount);


}


/**
 * Map the whole of the sprite's palette to one index.
 *
 * @param index The index to use
 */
void Sprite::flashPalette (int index) {
	/*if(palid==INVALID_OBJ) addobj(256,&palid);
	pixels.start=0;
	pixels.length=256;
	pixels.palette=(unsigned char *)objs[palid].ptr;
	pixels.flags|=miniS_useMap;
	//SDL_Color palette[256];
	int count;

	for (count = 0; count < 256; count++)
		pixels.palette[count] *//*= palMap[count*3+1] = palMap[count*3+2]*/ // = index;

	//SDL_SetPalette(pixels, SDL_LOGPAL, palette, 0, 256);
	

}


/**
 * Restore the sprite's palette to its original state.
 */
void Sprite::restorePalette () {
	//if(palid!=INVALID_OBJ) freeobj(palid);
	//video.restoreSurfacePalette(pixels);
	//pixels.flags&=~(miniS_useMap);

}
/**
 * Draw the sprite
 *
 * @param x The x-coordinate at which to draw the sprite
 * @param y The y-coordinate at which to draw the sprite
 * @param includeOffsets Whether or not to include the sprite's offsets
 */
void Sprite::draw (int x, int y, bool includeOffsets){
	//SDL_Rect dst;
	int dx,dy;
	dx = x;
	dy = y;
	if (includeOffsets){
		dx += xOffset;
		dy += yOffset;
	}
	//SDL_BlitSurface(pixels, NULL, canvas, &dst);
	blitToCanvas(&pixels,dx,dy);
}


/**
 * Draw the sprite scaled
 *
 * @param x The x-coordinate at which to draw the sprite
 * @param y The y-coordinate at which to draw the sprite
 * @param scale The amount by which to scale the sprite
 */
void Sprite::drawScaled (int x, int y, fixed scale) {

	unsigned char* srcRow;
	unsigned char* dstRow;
	unsigned char pixel, key;
	int width, height, fullWidth, fullHeight;
	int dstX, dstY;
	int srcX, srcY;

	key = pixels.colkey;

	fullWidth = FTOI(pixels.w * scale);
	if (x < -(fullWidth >> 1)) return; // Off-screen
	if (x + (fullWidth >> 1) > canvasW) width = canvasW + (fullWidth >> 1) - x;
	else width = fullWidth;

	fullHeight = FTOI(pixels.h * scale);
	if (y < -(fullHeight >> 1)) return; // Off-screen
	if (y + (fullHeight >> 1) > canvasH) height = canvasH + (fullHeight >> 1) - y;
	else height = fullHeight;
	if (y < (fullHeight >> 1)) {

		srcY = (fullHeight >> 1) - y;
		dstY = 0;

	} else {

		srcY = 0;
		dstY = y - (fullHeight >> 1);

	}

	while (srcY < height) {

		srcRow = ((unsigned char *)(pixels.pix)) + (pixels.w * DIV(srcY, scale));
		dstRow = ((unsigned char *)(canvas.pix)) + (canvasW * dstY);

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
	return;

}


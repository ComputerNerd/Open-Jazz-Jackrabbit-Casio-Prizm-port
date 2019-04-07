#pragma once
#ifndef CASIO
	#include <SDL/SDL.h>
#endif
#define miniS_COLKEY 1
struct miniSurface {
	unsigned char * pix;
	int w,h;
	unsigned flags;//flags can be OR'd together
	unsigned char colkey;
};
void setColKey(struct miniSurface * in,unsigned char entry);
void initMiniSurface(struct miniSurface * surf,void * pix,unsigned int w,unsigned int h);
void MiniSurfToMiniSurf(const struct miniSurface * src,const struct miniSurface * dst,unsigned int xo,unsigned int yo);
void blitToCanvas(const struct miniSurface * __restrict__ over,int xo,int yo);
void blitToCanvasRemap(const struct miniSurface * __restrict__ over,int xo,int yo, const unsigned char * remap);
void blitPartToCanvas(const struct miniSurface * __restrict__ over,int xo,int yo,const short * part);

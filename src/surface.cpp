#include "surface.h"
#include "io/gfx/video.h"
#include <string.h>

#ifndef CASIO
#include <SDL/SDL.h>
#endif
void setColKey(struct miniSurface * in,unsigned char entry){
	in->flags|=miniS_COLKEY;
	in->colkey=entry;
}
void initMiniSurface(struct miniSurface * surf,void * pix,unsigned int w,unsigned int h){
	surf->pix=(unsigned char *)pix;
	surf->flags=0;
	surf->w=w;
	surf->h=h;
}
void MiniSurfToMiniSurf(const struct miniSurface * src,const struct miniSurface * dst,unsigned int xo,unsigned int yo){
	int maxX=((src->w)<(dst->w+xo))?(src->w):(dst->w+xo);//Prevent out of bounds copying
	int maxY=((src->h)<(dst->h+yo))?(src->h):(dst->h+yo);
	const unsigned char * sp=src->pix;
	unsigned char * dp=dst->pix+xo+(yo*dst->w);
	while(maxY--){
		if((src->flags&miniS_COLKEY)){
			unsigned int x=maxX;
			while(x--){
				if(*sp!=src->colkey)
					*dp=*sp;
				++dp;
				++sp;
			}
			dp+=dst->w-src->w;
		}else{
			memcpy(dp,sp,maxX);
			sp+=src->w;
			dp+=dst->w;
		}
	}
}

void blitToCanvasRemap(const struct miniSurface * __restrict__ over,int xo,int yo, const unsigned char * remap){
	const unsigned char * src=over->pix;
	unsigned char * dst=canvas.pix+xo+(yo*canvasW);
	int maxX=over->w;
	int maxY=over->h;
	while(maxY--){
		unsigned int x=maxX;
		if((over->flags&miniS_COLKEY)){
			while(x--){
				if(*src!=over->colkey)
					*dst=remap[*src];
				++dst;
				++src;
			}
		} else {
			while(x--){
				*dst++=remap[*src++];
			}
		}
		dst+=canvasW-maxX;
	}
}

void blitToCanvas(const struct miniSurface * __restrict__ over,int xo,int yo){
	if(!over->pix){
		#ifndef CASIO
			printf("Null pointer blit attempt: %d %d %d %d %d %d\n",xo,yo,over->w,over->h,over->flags,over->colkey);
		#endif
		return;
	}
	const unsigned char * src=over->pix;
	int maxX=over->w;
	int maxY=over->h;
	if(xo<0){
		if(xo<=(over->w*-1)){
			//puts("X over");
			return;
		}
		maxX+=xo;
		src-=xo;
		xo=0;
	}
	if(yo<0){
		if(yo<=(over->h*-1)){
			//puts("Y over");
			return;
		}
		maxY+=yo;
		src-=yo*(int)over->w;
		yo=0;
	}
	if(maxX+xo>(canvasW))
		maxX=(int)canvasW-xo;
	if(maxY+yo>(canvasH))
		maxY=(int)canvasH-yo;
	if(maxX<1)
		return;
	if(maxY<1)
		return;
	if(xo>canvasW){
		return;
	}
	if(yo>canvasH){
		return;
	}
	unsigned char * dst=canvas.pix+xo+(yo*canvasW);
	while(maxY--){
		if((over->flags&miniS_COLKEY)){
			unsigned int x=maxX;
			while(x--){
				if(*src!=over->colkey)
					*dst=*src;
				++dst;
				++src;
			}
			dst+=canvasW-maxX;
			src+=over->w-maxX;
		}else{
			memcpy(dst,src,maxX);
			src+=over->w;
			dst+=canvasW;
		}
	}
}
void blitPartToCanvas(const struct miniSurface * __restrict__ over,int xo,int yo,const short * __restrict__ part){
	//part is in x y w h format
	const unsigned char * sp=over->pix+(part[1]*over->w)+part[0];
	int maxX=part[2];
	if(maxX>(canvasW-xo))
		maxX=canvasW-xo;
	int maxY=part[3];
	if(maxY>(canvasH-yo))
		maxY=canvasH-yo;
	if(xo<0){
		maxX+=xo;
		sp-=xo;
		xo=0;
	}
	if(yo<0){
		maxY+=yo;
		sp-=yo*(int)over->w;
		yo=0;
	}
	if(maxX<1)
		return;
	if(maxY<1)
		return;
	unsigned char * dp=canvas.pix+xo+(yo*canvasW);
	while(maxY--){
		if((over->flags&miniS_COLKEY)){
			unsigned int x=maxX;
			while(x--){
				if(*sp!=over->colkey)
					*dp=*sp;
				++dp;
				++sp;
			}
			sp+=over->w-maxX;
			dp+=canvasW-maxX;
		}else{
			memcpy(dp,sp,maxX);
			sp+=over->w;
			dp+=canvasW;
		}
		
	}
}

#include "mem.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef CASIO
#include "platforms/casio.h"
#endif
struct memobj objs[MAXOBJ];

static unsigned cursize[2];
static unsigned objamt=0;

static unsigned char heapdat[MAXMEM];
objid_t * idptrs[MAXOBJ];//When freeing object it is going to change the "id" of other objects so these will need to be updated pointer appears to be the best solution

unsigned int allowUseSecondaryVramAsHeap;
#ifdef CASIO
extern unsigned char * SaveVramAddr;
#else
extern unsigned char SaveVramAddr[];
#endif

static unsigned char * heapPtrs[2];
static const unsigned memoryLimits[2] = {MAXMEM, MAXMEM2};

void initMemHeap(void) {
	heapPtrs[0] = heapdat;
	heapPtrs[1] = SaveVramAddr;
}

void addobj(unsigned size,objid_t * id){
	unsigned useHeap = 0;
	if((cursize[0]+size)>MAXMEM){
		unsigned needShowError = !allowUseSecondaryVramAsHeap;
		if (allowUseSecondaryVramAsHeap) {
			useHeap = 1;
			needShowError = (cursize[1] + size) > MAXMEM2;
		}

		if (needShowError) {
			#ifdef CASIO
				casioQuit("Out of stack for addojb allocator");
			#else
				printf("Out of stack for addojb allocator while adding object %d needs %d more bytes\n",objamt,(cursize[useHeap]+size)-memoryLimits[useHeap]);
				exit(-1);
			#endif
			return;
		}
	}
	if(objamt>=MAXOBJ){
		#ifdef CASIO
			casioQuit("Out of objects");
		#else
			puts("Out of objects");
			exit(-1);
		#endif
		return;
	}
	objs[objamt].size = size;
	objs[objamt].ptr = heapPtrs[useHeap] + cursize[useHeap];
	objs[objamt].memb = cursize[useHeap];
	objs[objamt].heapIdx = useHeap;

	cursize[useHeap] += size;

	*id=objamt;
	idptrs[objamt]=id;
	#ifndef CASIO
		printf("Size: %d memb %d id %d\n",objs[objamt].size,objs[objamt].memb,*id);
	#endif
	++objamt;
}
void freeobj(objid_t obj){
	//avoid fragmentation by moving data past free'd data into it's place
	//if this is the last object freeing will be much faster try to free last object as much as possible or free towards the end
	#ifndef CASIO
		printf("Freeing object %d that had a size of %d and a memb value of %d\n",obj,objs[obj].size,objs[obj].memb);
	#endif
	unsigned x;
	unsigned useHeap = objs[obj].heapIdx;
	unsigned char * heapPtr = heapPtrs[useHeap];

	if(obj!=(objamt-1)){
		unsigned removedbytes=objs[obj].size;
		--objamt;
		idptrs[obj][0]=INVALID_OBJ;
		memmove(heapPtr+(objs[obj].memb),heapPtr+(objs[obj+1].memb),cursize[useHeap]-(objs[obj+1].memb));
		memmove(&idptrs[obj],&idptrs[obj+1],(objamt-obj)*sizeof(objid_t *));
		cursize[useHeap]-=objs[obj].size;
		memmove(&objs[obj],&objs[obj+1],(objamt-obj)*sizeof(struct memobj));
		for(x=obj;x<objamt;++x){
			idptrs[x][0]--;
			if (objs[x].heapIdx == useHeap) {
				objs[x].memb-=removedbytes;
				objs[x].ptr=(void *)((unsigned char *)objs[x].ptr-removedbytes);
			}
		}
	}else{
		cursize[useHeap]-=objs[obj].size;
		--objamt;
	}
}
void resizeobj(objid_t obj, int newamt){
	int change=newamt-(objs[obj].size);
	if(change==0)
		return;
	if(newamt<=0){
		#ifdef CASIO
			casioQuit("Cannot resize object to x<=0");
		#else
			printf("Cannot resize object to %d\n",newamt);
			exit(-1);
		#endif
	}

	unsigned useHeap = objs[obj].heapIdx;
	unsigned char * heapPtr = heapPtrs[useHeap];
	
	if((cursize[useHeap]+change)>memoryLimits[useHeap]){
		#ifdef CASIO
			casioQuit("Not enough stack to resize object");
		#else
			printf("Not enough stack to resize object need %d more bytes\n",(cursize[useHeap]+change)-memoryLimits[useHeap]);
			exit(-1);
		#endif
		return;
	}
	if(obj!=(objamt-1)){
		#ifndef CASIO
			printf("Resizing and moving object %d from %d bytes to %d bytes\n",obj,objs[obj].size,newamt);
		#endif
		int x;
		memmove(heapPtr+(objs[obj+1].memb)+change,heapPtr+(objs[obj+1].memb),cursize[useHeap]-(objs[obj+1].memb));
		cursize[useHeap]-=objs[obj].size;
		objs[obj].size=newamt;
		cursize[useHeap]+=newamt;
		for(x=obj+1;x<objamt;++x){
			if (objs[x].heapIdx == useHeap) {
				objs[x].ptr+=change;
				objs[x].memb+=change;
			}
		}
	}else{
		cursize[useHeap]-=objs[obj].size;
		objs[obj].size=newamt;
		cursize[useHeap]+=newamt;
	}
}

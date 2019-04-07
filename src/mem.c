#include "mem.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef CASIO
#include "platforms/casio.h"
#endif
struct memobj objs[MAXOBJ];
static unsigned cursize=0;
static unsigned objamt=0;
unsigned char heapdat[MAXMEM];
objid_t * idptrs[MAXOBJ];//When freeing object it is going to change the "id" of other objects so these will need to be updated pointer appears to be the best solution
void addobj(unsigned size,objid_t * id){
	if((cursize+size)>MAXMEM){
		#ifdef CASIO
			casioQuit("Out of stack for addojb allocator");
		#else
			printf("Out of stack for addojb allocator while adding object %d needs %d more bytes\n",objamt,(cursize+size)-MAXMEM);
			exit(-1);
		#endif
		return;
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
	objs[objamt].size=size;
	objs[objamt].ptr=heapdat+cursize;
	objs[objamt].memb=cursize;
	cursize+=size;
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
	if(obj!=(objamt-1)){
		unsigned removedbytes=objs[obj].size;
		--objamt;
		idptrs[obj][0]=INVALID_OBJ;
		memmove(heapdat+(objs[obj].memb),heapdat+(objs[obj+1].memb),cursize-(objs[obj+1].memb));
		memmove(&idptrs[obj],&idptrs[obj+1],(objamt-obj)*sizeof(objid_t *));
		cursize-=objs[obj].size;
		memmove(&objs[obj],&objs[obj+1],(objamt-obj)*sizeof(struct memobj));
		for(x=obj;x<objamt;++x){
			idptrs[x][0]--;
			objs[x].memb-=removedbytes;
			objs[x].ptr=(void *)((unsigned char *)objs[x].ptr-removedbytes);
		}
	}else{
		cursize-=objs[obj].size;
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
	if((cursize+change)>MAXMEM){
		#ifdef CASIO
			casioQuit("Not enough stack to resize object");
		#else
			printf("Not enough stack to resize object need %d more bytes\n",(cursize+change)-MAXMEM);
			exit(-1);
		#endif
		return;
	}
	if(obj!=(objamt-1)){
		#ifndef CASIO
			printf("Resizing and moving object %d from %d bytes to %d bytes\n",obj,objs[obj].size,newamt);
		#endif
		int x;
		memmove(heapdat+(objs[obj+1].memb)+change,heapdat+(objs[obj+1].memb),cursize-(objs[obj+1].memb));
		cursize-=objs[obj].size;
		objs[obj].size=newamt;
		cursize+=newamt;
		for(x=obj+1;x<objamt;++x){
			objs[x].ptr+=change;
			objs[x].memb+=change;
		}
	}else{
		cursize-=objs[obj].size;
		objs[obj].size=newamt;
		cursize+=newamt;
	}
}

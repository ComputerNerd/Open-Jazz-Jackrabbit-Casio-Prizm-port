#pragma once
#ifdef __cplusplus
extern "C" {
#endif
struct memobj{
	unsigned int size;
	unsigned int memb;//Stores how many bytes are "behind" this chunck of memory
	void * ptr;
	unsigned heapIdx;
};

#define MAXOBJ 64
#define MAXMEM (384*1024)
#define MAXMEM2 (384 * 216 * 2)
#define INVALID_OBJ (MAXOBJ+1)

extern struct memobj objs[MAXOBJ];
typedef unsigned char objid_t;
extern objid_t * idptrs[MAXOBJ];

extern unsigned int allowUseSecondaryVramAsHeap;

void addobj(unsigned int size,objid_t * id);
void freeobj(objid_t obj);
void resizeobj(objid_t obj, int newamt);
void initMemHeap(void);
#ifdef __cplusplus
}
#endif

/**********************************************************************/
/* hit_map.h - Functions to keep track of read file locations.        */
/**********************************************************************/
#ifndef HIT_MAP_H
#define HIT_MAP_H


/***********/
/* Defines */
/***********/
typedef struct fmapType fmapType;

struct fmapType{
    unsigned int startOffset;
    unsigned int endOffset;
    fmapType* pNext;
};


/***********************/
/* Function Prototypes */
/***********************/
int initHitMap();
int destroyHitMap();
void setFileSize(unsigned int size);
int setFileHit(unsigned int start, unsigned int end);
void printFileSummary(char* outName);



#endif

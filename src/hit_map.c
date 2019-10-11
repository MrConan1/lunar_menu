/**********************************************************************/
/* hit_map.c - Functions to keep track of read file locations.        */
/**********************************************************************/
#ifdef _MSC_VER
#pragma warning(disable:4996)
#endif


/************/
/* Includes */
/************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hit_map.h"




/***********************/
/* Function Prototypes */
/***********************/
int initHitMap();
int destroyHitMap();
void setFileSize(unsigned int size);
int setFileHit(unsigned int start, unsigned int end);
void printFileSummary(char* outName);
void gapDetect();


/* Globals */
static fmapType *pFileMap = NULL;  /* List Ptr  */
static unsigned int G_fsize = 0;     /* File Size */

/*******************************************************************/
/* initHitMap                                                      */
/* Creates an empty list to hold the file read hit information.    */
/*******************************************************************/
int initHitMap()
{
    if(pFileMap != NULL)
        destroyHitMap();
    pFileMap = NULL;

    return 0;
}



/*******************************************************************/
/* destroyHitMap                                                   */
/* Destroys all items in a file read hit map.                      */
/*******************************************************************/
int destroyHitMap()
{
    fmapType *pCurrent = NULL;
    fmapType *pMapItem = pFileMap;

    while(pMapItem != NULL){
        pCurrent = pMapItem;
        pMapItem = pMapItem->pNext;
        free(pCurrent);
    }

    return 0;
}




/*******************************************************************/
/* setFileSize                                                     */
/* Sets the size of the file being studied.                        */
/*******************************************************************/
void setFileSize(unsigned int size){
    G_fsize = size;
    return;
}




/*******************************************************************/
/* setFileHit                                                      */
/* Inserts an element in the hit list.                             */
/* Returns 0 on success, -1 on failure, 1 on a collision.          */
/*         Failures can be due to out of bounds or fctn issues.    */
/*******************************************************************/
int setFileHit(unsigned int start, unsigned int end){

	fmapType* newItem, *pCurrent, *pNext;

    /* Create a new item */
    newItem = (fmapType*)malloc(sizeof(fmapType));
    if(newItem == NULL){
        printf("Error allocing memory for new item in setFileHit\n");
        return -1;
    }
    newItem->startOffset = start;
    newItem->endOffset = end;
    newItem->pNext = NULL;

    /* Head is Empty */
    if(pFileMap == NULL){
        pFileMap = newItem;
        return 0;
    }

    /* Head != NULL, Insert at Head ? */
    if(start <= pFileMap->startOffset){
        newItem->pNext = pFileMap;
        pFileMap = newItem;
        return 0;
    }

    /* First item is not empty and insertion will not take place at head */
    pCurrent = pFileMap;
    while(pCurrent != NULL){
        pNext = pCurrent->pNext;

        /* Current exists, but next does not */
        if(pNext == NULL){
            pCurrent->pNext = newItem;
            break;
        }

        /* Check to insert */
        if(pNext->startOffset > start){
            pCurrent->pNext = newItem;
            newItem->pNext = pNext;
			break;
        }

        pCurrent = pCurrent->pNext;
    }


    return 0;
}




/*******************************************************************/
/* printFileSummary                                                */
/* Prints a summary of the unread locations in the file            */
/* followed by the detailed hexadecimal contents.                  */
/*******************************************************************/
void printFileSummary(char* outName){

	unsigned int start, end, gapStart, gapEnd, lastEnd, nextStart, nextEnd;
    fmapType* pCurrent;
    FILE* summary = NULL;
	int final_test = 0;
	int numgaps = 0;
	lastEnd = -1;

    /* Open file for output */
    summary = fopen(outName,"w");
    if(outName == NULL){
        printf("Error opening file hit summary file %s\n",outName);
        return;
    }

    pCurrent = pFileMap;
    while(pCurrent != NULL){
        start = (unsigned int)pCurrent->startOffset;
        end = (unsigned int)pCurrent->endOffset;

        /* Check for Gap */
        if( (start != 0) && (start != (lastEnd + 1)) ){
            gapStart = lastEnd + 1;
            gapEnd = start-1;

            /* Print Gap Contents */
            printf("GAP: 0x%X to 0x%X\n", gapStart, gapEnd);
            fprintf(summary,"GAP: 0x%X to 0x%X\n", gapStart, gapEnd);
			numgaps++;
        }

		/* Print Start/End */
		printf("Start: 0x%X \t\t End: 0x%X\n", start, end);
		fprintf(summary, "Start: 0x%X \t\t End: 0x%X\n", start, end);

        /* Check for overlap */
        if(pCurrent->pNext == NULL){
            nextStart = G_fsize-1;
            nextEnd = G_fsize-1;
			final_test = 1;
        }
        else{
            nextStart = pCurrent->pNext->startOffset;
            nextEnd = pCurrent->pNext->endOffset;
        }
        if((start >= nextStart) || (end >= nextStart)){
			if(final_test && (end == nextStart))
				;  //nothing to do
			else{
				printf("Overlap Detected: Start: 0x%X  End: 0x%X   NextStart: 0x%X  NextEnd: 0x%X\n",
					start, end, nextStart, nextEnd);
				fprintf(summary, "Overlap Detected: Start: 0x%X  End: 0x%X   NextStart: 0x%X  NextEnd: 0x%X\n",
					start, end, nextStart, nextEnd);
				end = nextStart - 1;
			}
        }

        lastEnd = end;
        pCurrent = pCurrent->pNext;
    }

    /* Print out final gap, if one exists.  Also print if somehow overflow occurred */
    if(lastEnd != (G_fsize-1)){
        if(lastEnd < (G_fsize-1)){
            gapStart = lastEnd+1;
            gapEnd = (G_fsize-1);
            printf("GAP: 0x%X to 0x%X\n", gapStart, gapEnd);
            fprintf(summary,"GAP: 0x%X to 0x%X\n", gapStart, gapEnd);
			numgaps++;
        }
        else if(lastEnd > (G_fsize-1)){
            printf("Overflow Detected: 0x%X to 0x%X\n", G_fsize, lastEnd);
            fprintf(summary,"Overflow Detected: 0x%X to 0x%X\n", G_fsize, lastEnd);
        }
    }

    /* Print out final offset */
    printf("Last file offset = 0x%X, %d Gaps identified.\n",G_fsize-1,numgaps);
    fprintf(summary,"Last file offset = 0x%X, %d Gaps identified.\n",G_fsize-1,numgaps);
    fclose(summary);

	//Alert
	if (numgaps > 0)
		gapDetect();

    return;
}


void gapDetect(){
	char a = 0;

	printf("Warning, Gap Detected. Hit 'y' and Enter to continue.\n");
	while (a != 'y'){
		fflush(stdin);
		a = getchar();
	}

	return;
}

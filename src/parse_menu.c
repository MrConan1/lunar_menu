/*****************************************************************************/
/* parse_menu.c : Code to parse lunar SSS Menus                              */
/*****************************************************************************/

/* Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "node_types.h"
#include "llst_gen.h"
#include "util.h"
#include "hit_map.h"

/* Defines */
#define DBUF_SIZE      (128*1024)     /* 128kB */
#define PTR_ARRAY_SIZE (2*1792)       /* 0xE00 bytes, or 1792 16-bit LWs */

/* Globals */
static char* pdata = NULL;
extern unsigned int G_iFileSizeBytes;


/* Function Prototypes */
int decodeMenu(FILE* inFile, FILE* outFile);
int copyText(scriptNode* pParent, int offset, FILE** ptr_inFile, FILE** ptr_outFile);



/*****************************************************************************/
/* Function: decodeMenu                                                      */
/* Purpose: Parses the menu, starting at the top level pointers.  A tree     */
/*          containing all script-related data will be constructed.          */
/* Inputs:  Pointers to input/output files.                                  */
/* Outputs: 0 on Pass, -1 on Fail.                                           */
/*****************************************************************************/
int decodeMenu(FILE* inFile, FILE* outFile){

	char summaryName[300];
	int x;
    unsigned short* pIndexPtrs = NULL;
    scriptNode* pTopNode = NULL;
    scriptNode* pNewTopNode = NULL;
    scriptNode* prevNode = NULL;

    /* Allocate 128kB buffer, much bigger than the input file */
    if (pdata != NULL){
        free(pdata);
        pdata = NULL;
    }
    pdata = (char*)malloc(DBUF_SIZE); 
    if(pdata == NULL){
        printf("Error allocating space for file data buffer.\n");
        return -1;
    }


    //Hit Map for statistics 
    initHitMap();
    setFileSize(G_iFileSizeBytes);
    setFileHit((unsigned int)0, (unsigned int)0xE00 - 1);  //Take credit for header

    /**************************************************************/
    /* Step 1: Read in Index Pointers for the current script file */
    /**************************************************************/
    
    /* Allocate memory for the array */
    pIndexPtrs = (unsigned short*)malloc(PTR_ARRAY_SIZE);
    if(pIndexPtrs == NULL){
        printf("Error allocing memory for Index Ptr Array\n");
        return -1;
    }

    /* Read in first 0xE00 bytes, or 1792 16-bit LWs */
    if( fread(pIndexPtrs,2,1792,inFile) != 1792){
        printf("Error, 1792 16-bit words not read");
    }
    /* Convert Values to Little Endian so Pointer Table can be read */
    for(x=0; x < 1792; x++){
        swap16(&(pIndexPtrs[x]));
    }


    /*******************************************************************/
    /* Step 2: Construct a linked list with each unique script pointer */
    /*******************************************************************/
    initScriptList();
    for(x=0; x < 1792; x++){
        
        /* Not a pointer, just empty space */
        if (pIndexPtrs[x] == 0x0)
            continue;

        /* Search to see if it exists, otherwise add it to the list */
        if(searchScriptList(pIndexPtrs[x]) == NULL){
            pNewTopNode = createTopNode(prevNode, pIndexPtrs[x]);
            if (getScriptHead() == NULL)
                setScriptHead(pNewTopNode);
            else{
                prevNode->pNextB = pNewTopNode;
            }    
            prevNode = pNewTopNode;
        }
    }


    /************************************************************************************/
    /* Step 3: For each unique script pointer decode the menu text and output to a file */
	/*         Output Format: CSV - Offset, JP Text                                     */
    /************************************************************************************/
	fprintf(outFile, "Offset, JP Text\n");
	pTopNode = getScriptHead(); /* Reset to head */
    while(pTopNode != NULL){

        unsigned int shortOffset, byteOffset;

        /* Calculate byte offset into the data to start reading from */
        /* Based on the Top Level Script Index Pointer */
        shortOffset = *((unsigned short*)(pTopNode->data));
        byteOffset = shortOffset * 2;
        pTopNode->childAStartOffset = byteOffset;

        /* Print out Offset Location of Text */
        printf("Decoding Menu Text Offset 0x%X\n",byteOffset);
        
        if ((unsigned int)byteOffset >= G_iFileSizeBytes){
            printf("Warning, top level byte offset 0x%X is beyond EOF. Skipping Parse\n", byteOffset);
        }
        else if(copyText(pTopNode, byteOffset, &inFile, &outFile) != 0){
            printf("Error detected while parsing at byte offset=0x%X.\n",byteOffset);
            return -1;
        }

        /* Move to the next high-level pointer */
        /* pNextB is always the next pointer for top level nodes */
        pTopNode = pTopNode->pNextB;
    }

    /* Free memory */
    if(pdata != NULL)
        free(pdata);


    //Print out hit summary
	strcpy(summaryName,getBaseName());
	strcat(summaryName, "_file_coverage_stats.txt");
	printFileSummary(summaryName);

    /**********************************/
    /* Step 5: Release Tree Resources */
    /**********************************/
    initScriptList();

    return 0;
}




/*****************************************************************************/
/* Function: copyText                                                        */
/* Purpose: Parses a sequence of script commands into a tree structure.      */
/* Inputs:  Pointer to current node in the script.                           */
/*          Byte offset into file to read from.                              */
/* Outputs: None.                                                            */
/*****************************************************************************/
int copyText(scriptNode* pParent, int offset, FILE** ptr_inFile, FILE** ptr_outFile){

	int rval, index, text_detect, offsetFlg;
	FILE* inFile, *outFile;
	unsigned short* pShort = (unsigned short*)pdata;
	unsigned short short_data;
	index = 0;
	rval = -1;

	/* Update File Pointers */
	inFile = *ptr_inFile;
	outFile = *ptr_outFile;

	/* Jump to text */
	if (fseek(inFile, offset, SEEK_SET) != 0){
		printf("Error seeking in input file.\n");
		return -1;
	}

	fprintf(outFile, "0x%X ", offset);
	offsetFlg = 1;

	/* Read from the file */
	text_detect = 0;
	while (!feof(inFile)){

		rval = fread(&pShort[index], 2, 1, inFile);
		if (rval != 1){
			printf("Error encountered while reading MENU TEXT.\n");
			break;
		}
		swap16(&pShort[index]);
		short_data = pShort[index];

		/* Decode Text and output to file */
		if ((short_data & 0xF000) == 0xF000){

			/* Output Hex Code if you get here */
			if (text_detect == 1){
				text_detect = 0;
				fprintf(outFile, "\n");
				fprintf(outFile, "-,  0x%04X\n", (unsigned int)short_data);
			}
			else{
				if (!offsetFlg)
					fprintf(outFile, "-");
				fprintf(outFile, ",  0x%04X\n", (unsigned int)short_data);
			}

			/* Text Termination Detection */
			if (pShort[index] == 0xFFFF){
				rval = 0;
				setFileHit((unsigned int)offset, (unsigned int)(ftell(inFile) - 1));
				break;
			}
		}
		else{
			/* Text */
			char temp[32];
			unsigned int index = (unsigned int)short_data;
			if (text_detect == 0){
				if (!offsetFlg)
					fprintf(outFile, "-");
				fprintf(outFile, ", ");
				text_detect = 1;
			}

			memset(temp, 0, 32);
			getUTF8character((int)index, temp);
			fprintf(outFile, "%s", temp);
		}

		offsetFlg = 0;
	}

	return rval;
}

/*****************************************************************************/
/* llst_gen.h                                                                */
/*****************************************************************************/
#ifndef LLST_GEN_H
#define LLST_GEN_H

/************/
/* Includes */
/************/
#include "node_types.h"


/***********/
/* Defines */
/***********/
#define SEL_CHILD_NONE 0
#define SEL_CHILD_A    1
#define SEL_CHILD_B    2

#define TEXT_DECODE_TWO_BYTES_PER_CHAR 0
#define TEXT_DECODE_ONE_BYTE_PER_CHAR  1
#define TEXT_DECODE_UTF8               2


/***********************/
/* Function Prototypes */
/***********************/

/* List creation/search/upkeep routines */
int initScriptList();
int destroyScriptList(scriptNode *node);
scriptNode* getScriptHead();
void setScriptHead(scriptNode* newHead);
int linkNewNode(scriptNode* pParent,int nodeSelection, scriptNode* newNode,
                int newChildAOffset, int newChildBOffset);
int outputTreeDataToFile(FILE** pOutfile);
int reconstructTree(char* fname);
scriptNode* searchScriptList(short value);
scriptNode* searchForNodeByStartOffset(int offset);

/* Text Decode Functions */
int getTextDecodeMethod();
void setTextDecodeMethod(int method);
void setDecodeOptions(int numOptArgs, int* argList);

/* Unique node-creation routines */
scriptNode* createTopNode(scriptNode* prev, unsigned short jmpOffset);
scriptNode* createNoArgNode(scriptNode* prev, int type, int startOffset);
scriptNode* createGenShortWdArgNode(scriptNode* prev, int type, int numArgs, short* pArgs, int startOffset);
scriptNode* createTextNode(scriptNode* prev, char* textData, int dataSizeBytes, int startOffset);
scriptNode* createCondJumpNode(scriptNode* prev, int type,  unsigned short bitOffset, 
                               unsigned short jmpOffset, int startOffset, 
                               char* pdata, unsigned int pdataBytes);
scriptNode* create2OptionNode(scriptNode* prev, char* opt1Text, int dataSizeBytes1, 
                              char* opt2Text, int dataSizeBytes2, unsigned short opt2JmpOffset, int startOffset);

#endif

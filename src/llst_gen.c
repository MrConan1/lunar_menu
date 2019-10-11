/********************************************************************************/
/* llst_gen.c : Code to maintain script linkage                                 */
/********************************************************************************/
#ifdef _MSC_VER
#pragma warning(disable:4996)
#endif


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "node_types.h"
#include "llst_gen.h"
#include "util.h"


/* Globals */
static scriptNode *ptr_Head = NULL;
static unsigned int G_ID = 0;
static int textDecodeMode = TEXT_DECODE_TWO_BYTES_PER_CHAR;
static int numDecodeOptArgs = 0;
static int* decodeOptArgList = NULL;


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
void copyTreeData(FILE** pOutfile, scriptNode* node, int optlen, int* opt);
int reconstructTree(char* fname);
void buildTreeData(FILE** pInfile, scriptNode* node);
scriptNode* searchScriptList(short value); 
scriptNode* searchForNodeByStartOffset(int offset);


/* Decode Related Functions */
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


/* createTopNode - Creates a top level index node */
scriptNode* createTopNode(scriptNode* prev, unsigned short jmpOffset){

    scriptNode* pNode = NULL;

    /* Alloc a script node */
    pNode = (scriptNode*)malloc(sizeof(scriptNode));
    if(pNode == NULL){
        printf("Error creating new top level node.\n");
        return NULL;
    }

    /* Set type, # args will be fixed at 1 for top index nodes */
	pNode->originalStartOffset = -1;// jmpOffset * 2;
    pNode->childAStartOffset = -1;
    pNode->childBStartOffset = -1;
    pNode->id = G_ID++;
    pNode->type = NODE_INDEX;
    pNode->numWdArgs = 1;

    /* Data will contain the ushort offset */
    pNode->data = NULL;
    pNode->data = (char*)malloc(2);
    if(pNode->data == NULL){
        free(pNode);
        return NULL;
    }
    memcpy(pNode->data, &jmpOffset, 2);
    pNode->dataSizeBytes = 2;
    pNode->data2 = NULL;
    pNode->dataSizeBytes2 = 0;

    pNode->offsetElements = 0;

    /* Set Pointers */
    pNode->pNextA = NULL;
    pNode->pNextB = NULL;
    pNode->pPrev = prev;
    return pNode;
}




scriptNode* createNoArgNode(scriptNode* prev, int type, int startOffset){

    scriptNode* pNode = NULL;

    /* Alloc a script node */
    pNode = (scriptNode*)malloc(sizeof(scriptNode));
    if(pNode == NULL){
        printf("Error creating new No Arg node.\n");
        return NULL;
    }

    /* Set type, # args will be 0 */
    pNode->originalStartOffset = startOffset;
    pNode->childAStartOffset = -1;
    pNode->childBStartOffset = -1;
    pNode->id = G_ID++;
    pNode->type = type;
    pNode->numWdArgs = 0;

    /* No Data */
    pNode->data = NULL;
    pNode->dataSizeBytes = 0;
    pNode->data2 = NULL;
    pNode->dataSizeBytes2 = 0;

    pNode->offsetElements = 0;

    /* Set Pointers */
    pNode->pNextA = NULL;
    pNode->pNextB = NULL;
    pNode->pPrev = prev;
    return pNode;
}


scriptNode* createGenShortWdArgNode(scriptNode* prev, int type, int numArgs, short* pArgs, int startOffset){

    scriptNode* pNode = NULL;

    /* Alloc a script node */
    pNode = (scriptNode*)malloc(sizeof(scriptNode));
    if(pNode == NULL){
        printf("Error creating new Generic Short Wd node.\n");
        return NULL;
    }

    /* Set type, # args */
    pNode->originalStartOffset = startOffset;
    pNode->childAStartOffset = -1;
    pNode->childBStartOffset = -1;
    pNode->id = G_ID++;
    pNode->type = type;
    pNode->numWdArgs = numArgs;

    /* Data */
    pNode->data = (char*)malloc(numArgs*2);
    if(pNode->data == NULL){
        free(pNode);
        return NULL;
    }
    memcpy(pNode->data,pArgs,numArgs*2);
    pNode->dataSizeBytes = numArgs*2;
    pNode->data2 = NULL;
    pNode->dataSizeBytes2 = 0;

    pNode->offsetElements = 0;

    /* Set Pointers */
    pNode->pNextA = NULL;
    pNode->pNextB = NULL;
    pNode->pPrev = prev;
    return pNode;
}


scriptNode* createTextNode(scriptNode* prev, char* textData, int dataSizeBytes, int startOffset){

    scriptNode* pNode = NULL;

    /* Alloc a script node */
    pNode = (scriptNode*)malloc(sizeof(scriptNode));
    if(pNode == NULL){
        printf("Error creating new Text node.\n");
        return NULL;
    }

    /* 0 arguments, just text data */
    pNode->originalStartOffset = startOffset;
    pNode->childAStartOffset = -1;
    pNode->childBStartOffset = -1;
    pNode->id = G_ID++;
    pNode->type = NODE_TEXT;
    pNode->numWdArgs = 0;

    /* Data contains the text to be printed and terminator */
    pNode->data = NULL;
    pNode->data = (char*)malloc(dataSizeBytes+1);
    if(pNode->data == NULL){
        free(pNode);
        return NULL;
    }
    memcpy(pNode->data,textData,dataSizeBytes);
    pNode->dataSizeBytes = dataSizeBytes;
    pNode->data[dataSizeBytes] = '\0';
    pNode->data2 = NULL;
    pNode->dataSizeBytes2 = 0;

    pNode->offsetElements = 0;

    /* Set Pointers */
    pNode->pNextA = NULL;
    pNode->pNextB = NULL;
    pNode->pPrev = prev;
    return pNode;
}



scriptNode* createCondJumpNode(scriptNode* prev, int type,  unsigned short bitOffset, 
                               unsigned short jmpOffset, int startOffset, 
                               char* pdata, unsigned int pdataBytes){

    scriptNode* pNode = NULL;

    /* Alloc a script node */
    pNode = (scriptNode*)malloc(sizeof(scriptNode));
    if(pNode == NULL){
        printf("Error creating new Conditional Jump node.\n");
        return NULL;
    }

    /* 1 (Unconditional) or 2 (Conditional) Arguments */
    pNode->originalStartOffset = startOffset;
    pNode->childAStartOffset = -1;
    pNode->childBStartOffset = -1;
    pNode->id = G_ID++;
    pNode->type = type;
    pNode->numWdArgs = 2;

    /* Data will contain the parameters as they appeared in the file */
    pNode->data = NULL;
    pNode->data = (char*)malloc(pdataBytes);
    if(pNode->data == NULL){
        free(pNode);
        return NULL;
    }
    memcpy(pNode->data, pdata, pdataBytes);
    pNode->dataSizeBytes = pdataBytes;

    pNode->data2 = NULL;
    pNode->dataSizeBytes2 = 0;

    /* JMP Offsets */
    pNode->offsetElements = 0;

    /* Set Pointers */
    pNode->pNextA = NULL;
    pNode->pNextB = NULL;
    pNode->pPrev = prev;
    return pNode;
}


scriptNode* create2OptionNode(scriptNode* prev, char* opt1Text, int dataSizeBytes1, 
                              char* opt2Text, int dataSizeBytes2, unsigned short opt2JmpOffset,
                              int startOffset){

    scriptNode* pNode = NULL;

    /* Alloc a script node */
    pNode = (scriptNode*)malloc(sizeof(scriptNode));
    if(pNode == NULL){
        printf("Error creating new 2 Option node.\n");
        return NULL;
    }

    /* 0 arguments, just text data */
    pNode->originalStartOffset = startOffset;
    pNode->childAStartOffset = -1;
    pNode->childBStartOffset = -1;
    pNode->id = G_ID++;
    pNode->type = NODE_SEL_OPT;
    pNode->numWdArgs = 0;

    /* First 2 short words are jump address and 2nd parameter, then */
    /* Data should contain the text to be printed and terminator */
    pNode->data = NULL;  /* First String */
    pNode->dataSizeBytes = dataSizeBytes1;
    pNode->data = (char*)malloc(pNode->dataSizeBytes+1);
    if(pNode->data == NULL){
        free(pNode);
        return NULL;
    }
    memcpy(pNode->data,opt1Text,pNode->dataSizeBytes);
    pNode->data[pNode->dataSizeBytes] = '\0';

    pNode->data2 = NULL;  /* Second String */
    pNode->dataSizeBytes2 = dataSizeBytes2;
    pNode->data2 = (char*)malloc(dataSizeBytes2+1);
    if(pNode->data2 == NULL){
        free(pNode);
        return NULL;
    }
    memcpy(pNode->data2,opt2Text,dataSizeBytes2);
    pNode->data2[dataSizeBytes2] = '\0';

    pNode->offsetArray[0] = opt2JmpOffset; /* 2nd (Bottom) Option JMP Location */
    pNode->offsetElements = 1;

    /* Set Pointers */
    pNode->pNextA = NULL;
    pNode->pNextB = NULL;
    pNode->pPrev = prev;
    return pNode;
}




/****************************************************************************/
/* Start of Functions for Tree Operations                                   */
/* (The top level of each tree is a linked list of initial script pointers) */
/****************************************************************************/


/***************************************************/
/* initScriptList - Reinitializes the linked list. */
/***************************************************/
int initScriptList()
{
    if (ptr_Head != NULL)
        destroyScriptList(ptr_Head);
    ptr_Head = NULL;

    return 0;
}



/******************************************************************************/
/* destroyScriptList - Destroys the linked list, and any associated tree data */
/*                     Accomplished by postorder traversal.                   */
/******************************************************************************/
int destroyScriptList(scriptNode *node)
{
    if (node == NULL) 
        return 0;
  
    /* Recurse left */ 
    destroyScriptList(node->pNextA); 
  
    /* Recurse right */
    destroyScriptList(node->pNextB); 
  
    /* Remove the node */
    if(node->data != NULL)
        free(node->data);
    if(node != NULL)
        free(node);
    node = NULL;

    return 0;
}




/*********************************************************************/
/* getScriptHead - Returns a pointer to the head of the linked list. */
/*********************************************************************/
scriptNode* getScriptHead(){
    return ptr_Head;
}




/*****************************************************************************/
/* setScriptHead - Sets the head of the linked list to the input scriptNode* */
/*****************************************************************************/
void setScriptHead(scriptNode* newHead){
    ptr_Head = newHead;	
}




/*****************************************************************************/
/* searchScriptList - Searches the top level nodes to see if a matching      */
/*                    pointer value can be found.                            */
/* Returns: Pointer to node on success, NULL on failure.                     */
/*****************************************************************************/
scriptNode* searchScriptList(short value){

    scriptNode* ptr;

    ptr = getScriptHead();
    while(ptr != NULL){
        if( (short)(*(ptr->data)) == value)
            return ptr;
        ptr = ptr->pNextB;
    }
    return NULL;
}




/*****************************************************************************/
/* searchScriptList - Searches all tree nodes for a node with the matching   */
/*                    file start offset (byte offset).                       */
/* Returns: Pointer to node on success, NULL on failure.                     */
/*****************************************************************************/
void helper_searchForNodeByStartOffset(scriptNode* node, 
                                              scriptNode** pFoundNode, 
                                              int offset);

scriptNode* searchForNodeByStartOffset(int offset){

    scriptNode* ptrHead, *ptrTarget;
    ptrHead = getScriptHead();
    ptrTarget = NULL;
    helper_searchForNodeByStartOffset(ptrHead, &ptrTarget, offset);

    return ptrTarget;
}

void helper_searchForNodeByStartOffset(scriptNode* node, scriptNode** pFoundNode, int offset){

    /* Base Case - node does not exist, or node of interest */
    if(node == NULL)
        return;

    /* Test for value */
    if((node->type != 0) && (node->originalStartOffset == offset)){
        *pFoundNode = node;        
        return;
    }

    /* Recurse on left subtree */
    helper_searchForNodeByStartOffset(node->pNextA,pFoundNode,offset);  
  
    /* Recurse on right subtree */
    helper_searchForNodeByStartOffset(node->pNextB,pFoundNode,offset); 
}




/*****************************************************************************/
/* linkNewNode - Adds a new node to an existing tree structure.              */
/*****************************************************************************/
int linkNewNode(scriptNode* pParent,int nodeSelection, scriptNode* newNode,
                int newChildAOffset, int newChildBOffset){

    if(nodeSelection == SEL_CHILD_A){
        pParent->pNextA = newNode;
        newNode->childAStartOffset = newChildAOffset;
        newNode->childBStartOffset = newChildBOffset;
        return 0;
    }
    else if(nodeSelection == SEL_CHILD_B){
        pParent->pNextB = newNode;
        newNode->childAStartOffset = newChildAOffset;
        newNode->childBStartOffset = newChildBOffset;
        return 0;
    }
 
    /* Logic Error if you get here */
    printf("Error, child target did not match ptr A or B\n");   
    return -1;
}




/***********************************************/
/* Returns the current method of text decoding */
/* Either 1-byte encoding, or 2-byte           */
/***********************************************/
int getTextDecodeMethod(){
    return textDecodeMode;
}




/********************************************/
/* Sets the current method of text decoding */
/* Either 1-byte encoding, or 2-byte        */
/********************************************/
void setTextDecodeMethod(int method){
    textDecodeMode = method;
    return;
}



/**************************************************/
/* setDecodeOptions                               */
/* Sets up the optional decode to only decode     */
/* certain subroutines, as specified by the user. */
/**************************************************/
void setDecodeOptions(int numOptArgs, int* argList){

    numDecodeOptArgs = numOptArgs;
    if(decodeOptArgList != NULL)
        free(decodeOptArgList);
    decodeOptArgList = NULL;
    if (numOptArgs > 0){
        decodeOptArgList = (int*)malloc(sizeof(int)*numOptArgs);
        if (decodeOptArgList == NULL){
            printf("Error allocating space for decodeOptArgList\n");
            return;
        }
        memcpy(decodeOptArgList, argList, sizeof(int)*numOptArgs);
    }

    return;
}



/*****************************************************/
/* Copies a Tree's data to a file for manual editing */
/* ================================================= */
/* This program is being designed to read it back in */
/* and then reconstruct the script file.             */
/* opt array allows several limited types of script  */
/* cmds to be output if it is desired.               */
/*****************************************************/
int outputTreeDataToFile(FILE** pOutfile){

    /* Dump the tree in memory to disk */
    copyTreeData(pOutfile, getScriptHead(), numDecodeOptArgs, decodeOptArgList); 

    return 0;
}




/*****************************************************/
/* Helper function for the above.                    */
/* Copies a Tree's data to a file for manual editing */
/* This program is being designed to read it back in */
/* and then reconstruct the script file.             */
/*****************************************************/
void copyTreeData(FILE** pOutfile, scriptNode* node, int optlen, int* opt) 
{ 
    char* pdata;
    unsigned short short_data;
    unsigned char byte_data;
    int x;
    FILE* outfile = *pOutfile;
    int textDecodeMethod = 0;
    int outputData = 0;

    if (node == NULL) 
        return; 
  
    /* Determine if the Node's Info Should be Output */
    if(optlen == 0){
        outputData = 1;
    }
    else if(optlen > 0)
    {
        for(x=0; x < optlen; x++){
            if(opt[x] == node->type){
                outputData = 1;
                break;
            }
        }
    }


    /* Output the Node's Information to the file, if possible */
    if(outputData != 0){
        fprintf(outfile, "\r\n");

        /* Basic Info */
        fprintf(outfile, "ID: %u\r\n", node->id);
        fprintf(outfile, "Type: 0x%X\r\n",node->type);
        fprintf(outfile, "Num_Short_Word_Args: %d\r\n",node->numWdArgs);
        fprintf(outfile, "Data_Size_Bytes: %d\r\n",node->dataSizeBytes);
        fprintf(outfile, "Data_Size_Bytes2: %d\r\n",node->dataSizeBytes2);

        /* Original Location Data */        
        fprintf(outfile, "Offset: 0x%X\r\n",node->originalStartOffset);
        fprintf(outfile, "Child A Offset: 0x%X\r\n",node->childAStartOffset);
        fprintf(outfile, "Child B Offset: 0x%X\r\n",node->childBStartOffset);

        /* Output Offset Array (If Type has one) */
        for(x=0; x < node->offsetElements; x++){
            fprintf(outfile, "Short Offset[%d]: 0x%04X\r\n",(x+1),((unsigned int)(node->offsetArray[x]) & 0xFFFF));
        }

        /*****************************************/
        /* Output Data1 (Format depends on Type) */
        /*****************************************/
        fprintf(outfile, "\r\nData1:\r\n");
        
        if(node->dataSizeBytes > 0){

            if( (node->type == NODE_TEXT) || (node->type == NODE_SEL_OPT) ){

				/************************************************************/
				/* Text Output - Output a Mix of Hex & Text, (Hex for FFxx) */
				/************************************************************/
				pdata = node->data;
				textDecodeMethod = getTextDecodeMethod();
				
				if(node->type == NODE_SEL_OPT){
                    short_data = (*((unsigned short*)pdata) & 0xFFFF);
                    swap16(&short_data);
                    fprintf(outfile, "\r\nJMP: %04X\r\n",(unsigned int)short_data);
                    pdata += 2;
                    short_data = (*((unsigned short*)pdata) & 0xFFFF);
                    swap16(&short_data);
                    fprintf(outfile, "\r\nPARAM: %04X\r\n",(unsigned int)short_data);
                    pdata += 2;
                    x = 4;
                }
                else{
                    x = 0;
                }

                switch(textDecodeMethod){

                    case TEXT_DECODE_TWO_BYTES_PER_CHAR:
                    {
                        for(; x < node->dataSizeBytes; ){
                            if((node->dataSizeBytes-x) >= 2){
                                short_data = (*((unsigned short*)pdata) & 0xFFFF);
                                swap16(&short_data);

                                if( ((short_data & 0xF000) == 0xF000) && (short_data != 0xF90A) ){
                                     /* Hex */
                                    fprintf(outfile, "\r\n%04X\r\n",(unsigned int)short_data);
                                }
                                else{
                                    /* Text */
                                    char temp[32];
                                    unsigned int index = (unsigned int)short_data;
                                    memset(temp,0,32);
                                    if(short_data == 0xF90A)
                                        strcpy(temp," ");
                                    else
                                        getUTF8character((int)index, temp);
                                    fprintf(outfile, "%s", temp);
                                }
                                pdata+=2;
                                x+=2;
                            }
                            else if((node->dataSizeBytes-x) == 1){
                                fprintf(outfile, "\r\nError, leftover byte: 0x%02X \r\n",(unsigned int)(*pdata & 0xFF));
                                pdata++;
                                x++;
                            }
                        }
                        break;
                    }

                    case TEXT_DECODE_ONE_BYTE_PER_CHAR:
                    {
                        for(; x < node->dataSizeBytes; ){
                            char temp[32];
                            byte_data = (((unsigned char)*pdata) & 0xFF);

                            if(byte_data != 0xFF){
                                /* 1-Byte Text */
                                unsigned int index = (unsigned int)byte_data;
                                memset(temp,0,32);
                                getUTF8character((int)index, temp);
                                fprintf(outfile, "%s", temp);
                                pdata++;
                                x++;
                            }
                            else if((byte_data == 0xFF) && ((node->dataSizeBytes-x) > 1) ){
                                /* 2-byte Code or Space */
                                pdata++;
                                x++;
                                byte_data = (((unsigned char)*pdata) & 0xFF);
                                short_data = 0xFF00 | byte_data;

                                /* Could be a space or Hex */
                                if (short_data == 0xF90A){
                                    strcpy(temp," ");
                                    fprintf(outfile, "%s", temp);
                                }
                                else{
                                    fprintf(outfile, "\r\n%04X\r\n",(unsigned int)short_data);
                                }
                                pdata++;
                                x++;
                            }
                            else{
                                /* Junk Leftover Encoded Data (1-byte), Expected for this encoding, Can Ignore */
                                pdata++;
                                x++;
                            }

                        }
                        break;
                    }

					case TEXT_DECODE_UTF8:
					{
						unsigned char utf8data[5];
						int numBytes, y;

						for (; x < node->dataSizeBytes;){
							memset(utf8data, 0, 5);
							short_data = 0;
							if ((node->dataSizeBytes - x) >= 2){

								/* Grab the first byte of the utf8 character & get # bytes */
								if ((unsigned char)*pdata >= 0xF0)
									numBytes = 1;
								else
									numBytes = numBytesInUtf8Char((unsigned char)*pdata);
								utf8data[0] = (unsigned char)(*pdata);

								/* Read the rest of the utf8 character */
								for (y = 1; y < numBytes; y++){
									pdata++;
									x++;
									utf8data[y] = (unsigned char)*pdata;
								}

								/* Check for Control Code */
								if ((numBytes == 1) && (utf8data[0] >= 0xF0)){
										pdata++;
										x++;
										short_data = (utf8data[0] << 8) | (unsigned char)(*pdata);
								}


								if (short_data != 0x0000){
									if (short_data != 0xF90A){
										/* Hex */
										fprintf(outfile, "\r\n%04X\r\n", (unsigned int)short_data);
									}
									else
										fprintf(outfile, " ");
								}
								else{
									/* Text */
									fprintf(outfile, "%s", utf8data);
								}
								pdata++;
								x++;
							}
							else if ((node->dataSizeBytes - x) == 1){
								fprintf(outfile, "\r\nError, leftover byte: 0x%02X \r\n", (unsigned int)(*pdata & 0xFF));
								pdata++;
								x++;
							}
						}
						break;
					}

                    default: {
                        fprintf(outfile, "\r\nError, INVALID TEXT DECODE Method \r\n");
                        break;
                    }

                }  /* End Switch */
            }

            else {

                /**************************/
                /* HEX Output (Short Wds) */
                /**************************/
                pdata = node->data;
                for(x = 0; x < node->dataSizeBytes; ){

                    short_data = (*((unsigned short*)pdata) & 0xFFFF);
                    swap16(&short_data);

                    if((node->dataSizeBytes-x) >= 2){
                        fprintf(outfile, "%04X ",(unsigned int)short_data);
                        pdata+=2;
                        x+=2;
                    }
                    else if((node->dataSizeBytes-x) == 1){
                        fprintf(outfile, "%02X ",(unsigned int)(*pdata & 0xFF) );
                        pdata++;
                        x++;
                    }
                }

            }
        }


        /***********************************/
        /* Output Data2 (Text, 2 Opt only) */
        /***********************************/
        if(node->type == NODE_SEL_OPT){
            fprintf(outfile, "\r\nData2:\r\n");
            if(node->dataSizeBytes2 > 0){

                /************************************************************/
                /* Text Output - Output a Mix of Hex & Text, (Hex for FFxx) */
                /************************************************************/
                pdata = node->data2;
                textDecodeMethod = getTextDecodeMethod();
                
                switch(textDecodeMethod){

                    case TEXT_DECODE_TWO_BYTES_PER_CHAR:
                    {
                        for(x = 0; x < node->dataSizeBytes2; ){
                            if((node->dataSizeBytes2-x) >= 2){
                                short_data = (*((unsigned short*)pdata) & 0xFFFF);
                                swap16(&short_data);

                                if (((short_data & 0xF000) == 0xF000) && (short_data != 0xF90A)){
                                     /* Hex */
                                    fprintf(outfile, "\r\n%04X\r\n",(unsigned int)short_data);
                                }
                                else{
                                    /* Text */
                                    char temp[32];
                                    unsigned int index = (unsigned int)short_data;
                                    memset(temp,0,32);
                                    if (short_data == 0xF90A)
                                        strcpy(temp," ");
                                    else
                                        getUTF8character((int)index, temp);
                                    fprintf(outfile, "%s", temp);
                                }
                                pdata+=2;
                                x+=2;
                            }
                            else if((node->dataSizeBytes2-x) == 1){
                                fprintf(outfile, "\r\nError, leftover byte: 0x%02X \r\n",(unsigned int)(*pdata & 0xFF));
                                pdata++;
                                x++;
                            }
                        }
                        break;
                    }

                    case TEXT_DECODE_ONE_BYTE_PER_CHAR:
                    {
                        for(x = 0; x < node->dataSizeBytes; ){
                            char temp[32];
                            byte_data = ((unsigned char)*pdata) & 0xFF;

                            if(byte_data != 0xFF){
                                /* 1-Byte Text */
                                unsigned int index = (unsigned int)byte_data;
                                memset(temp,0,32);
                                getUTF8character((int)index, temp);
                                fprintf(outfile, "%s", temp);
                                pdata++;
                                x++;
                            }
                            else if((byte_data == 0xFF) && ((node->dataSizeBytes-x) > 1) ){
                                /* 2-byte Code or Space */
                                pdata++;
                                x++;
                                byte_data = (((unsigned char)*pdata) & 0xFF);
                                short_data = 0xFF00 | byte_data;

                                /* Could be a space or Hex */
                                if (short_data == 0xF90A){
                                    strcpy(temp," ");
                                    fprintf(outfile, "%s", temp);
                                }
                                else{
                                    fprintf(outfile, "\r\n%04X\r\n",(unsigned int)short_data);
                                }
                                pdata++;
                                x++;
                            }
                            else{
                                /* Junk Leftover Encoded Data (1-byte), Expected for this encoding, Can Ignore */
                                pdata++;
                                x++;
                            }

                        }
                        break;
                    }

					case TEXT_DECODE_UTF8:
					{
						unsigned char utf8data[5];
						int numBytes, y;

						for (x = 0; x < node->dataSizeBytes2;){
							memset(utf8data, 0, 5);
							short_data = 0;
							if ((node->dataSizeBytes2 - x) >= 2){

								/* Grab the first byte of the utf8 character & get # bytes */
								if ((unsigned char)*pdata >= 0xF0)
									numBytes = 1;
								else
									numBytes = numBytesInUtf8Char((unsigned char)*pdata);
								utf8data[0] = (unsigned char)(*pdata);

								/* Read the rest of the utf8 character */
								for (y = 1; y < numBytes; y++){
									pdata++;
									x++;
									utf8data[y] = (unsigned char)*pdata;
								}

								/* Check for Control Code */
								if ((numBytes == 1) && (utf8data[0] >= 0xF0)){
									pdata++;
									x++;
									short_data = (utf8data[0] << 8) | (unsigned char)(*pdata);
								}

								if (short_data != 0x0000){
									if (short_data != 0xF90A){
										/* Hex */
										fprintf(outfile, "\r\n%04X\r\n", (unsigned int)short_data);
									}
									else
										fprintf(outfile, " ");
								}
								else{
									/* Text */
									fprintf(outfile, "%s", utf8data);
								}
								pdata++;
								x++;
							}
							else if ((node->dataSizeBytes2 - x) == 1){
								fprintf(outfile, "\r\nError, leftover byte: 0x%02X \r\n", (unsigned int)(*pdata & 0xFF));
								pdata++;
								x++;
							}
						}
						break;
					}

                    default: {
                        fprintf(outfile, "\r\nError, INVALID TEXT DECODE Method \r\n");
                        break;
                    }

                }  /* End Switch */
            }
            else{
                fprintf(outfile, "\r\nError, 2-Option Node Missing 2nd Text \r\n");
            }
        }


        fprintf(outfile, "\r\n\r\n");
    }
  
    /* Recurse on left subtree */
    copyTreeData(pOutfile,node->pNextA,optlen,opt);  
  
    /* Recurse on right subtree */
    copyTreeData(pOutfile,node->pNextB,optlen,opt); 
}  




/*****************************************************/
/* Copies a Tree's data to a file for manual editing */
/* Copies a Tree's data to a file for manual editing */
/* This program is being designed to read it back in */
/* and then reconstruct the script file.             */
/* opt array allows several limited types of script  */
/* cmds to be output if it is desired.               */
/*****************************************************/
int reconstructTree(char* fname){

    FILE* ifile;
    ifile = fopen(fname,"r");
    if(ifile == NULL){
        printf("Error opening %s for reading\n",fname);
        return -1;
    }

    /* Delete the tree if it exists */
    initScriptList();

    /* Build the tree from disk to memory */
    buildTreeData(&ifile, getScriptHead()); 

    return 0;
}




/*****************************************************/
/* Helper function for the above.                    */
/* Copies a Tree's data to a file for manual editing */
/* This program is being designed to read it back in */
/* and then reconstruct the script file.             */
/*****************************************************/
void buildTreeData(FILE** pInfile, scriptNode* node) 
{ 
#if 0
    FILE* outfile = *pOutfile;

    if (node == NULL) 
        return; 
  
    /* Output the Node's Information to the file */
    if(optlen == 0)
    {
        fprintf("\r\n");
        fprintf("ID: %u\r\n", node->id);
        fprintf("Type: %d\r\n",node->type);
        fprintf("Num_Short_Word_Args: %d\r\n",node->type);
        fprintf("Data: %d\r\n",node->type);
        fprintf("Data_Size_Bytes: %d\r\n",node->type);
        fprintf("\r\n");
    }
    else if(optlen > 0)
    {
        for(x=0; x < optlen; x++){
            if(opt[x] == node->type){
                fprintf("\r\n");
                fprintf("ID: %u\r\n", node->id);
                fprintf("Type: %d\r\n",node->type);
                fprintf("Num_Short_Word_Args: %d\r\n",node->type);
                fprintf("Data: %d\r\n",node->type);
                fprintf("Data_Size_Bytes: %d\r\n",node->type);
                fprintf("\r\n");
                break;
            }
        }
    }
  
    /* Recurse on left subtree */
    buildTreeData(node->left);  
  
    /* Recurse on right subtree */
    buildTreeData(node->right); 
#endif
}  


/*****************************************************/
/* END of Functions for Tree / Linked List Operation */
/*****************************************************/

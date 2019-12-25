/*****************************************************************************/
/* encode_menu.c : Code to encode lunar SSS Menus                            */
/*****************************************************************************/

#ifdef _MSC_VER
#pragma warning(disable:4996)
#endif

/* Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

/* Defines */
#define MAX_MENU_SIZE  (32*1024)      /* 32kB  */
#define DBUF_SIZE      (128*1024)     /* 128kB */
#define MAX_MENU_ITEMS 1792
#define MENU_DELIM     "\t\r\n"
#define MENU_DELIM2    "\t\r\n "
#define CTRL_CODE_SPACE 0xF905
#define CTRL_CODE_SPACEA 0xF9
#define CTRL_CODE_SPACEB 0x05

typedef struct subRecord subRecord;
struct subRecord{
	unsigned short* convertedString;
	unsigned int lenConvertedStringBytes;
	unsigned short ctrlCode;
};

/* Menu Structure */
typedef struct menuRecord menuRecord;
struct menuRecord{
	unsigned short oldOffset;
	unsigned short newOffset;
	subRecord subRecords[32];
	int numSubRecords;
};


/***********/
/* Globals */
/***********/
static menuRecord menuRecordArray[MAX_MENU_ITEMS];
static int numMenuItems;

/***********************/
/* Function Prototypes */
/***********************/
int encodeMenu(FILE* infile, FILE* csvfile, FILE* outfile);
static void freeMenuData();




/*****************************************************************************/
/* Function: encodeMenu                                                      */
/* Purpose: Reads 1st Column of CSV file to store old offset,                */
/*          Assumes 2nd Column is Japanese and skips is,                     */
/*          Assumes 3rd column is Google Translate or scratch space.         */
/*          Uses 4th column to read and store Updated English Menu Item.     */
/* Inputs:  Pointers to input/output files.                                  */
/* Outputs: 0 on Pass, -1 on Fail.                                           */
/*****************************************************************************/
int encodeMenu(FILE* infile, FILE* csvfile, FILE* outfile)
{

	int x, y, rval, entryEndLocated, line;
	unsigned int fsize, outputSizeBytes;
	unsigned char* pBuffer = NULL;
	char* pInput = NULL;
	unsigned short* outBuffer = NULL;
	unsigned char* pMenuData = NULL;
	entryEndLocated = 0;
	line = 1;

	/* Init Menu Items */
	memset(menuRecordArray, 0, sizeof(menuRecord)*MAX_MENU_ITEMS);
	numMenuItems = 0;

	/**********************************************/
	/* Put Tab Delimited CSV Data into Memory     */
	/* Col 1: Old Offset                          */
	/* Col 2: JP Text (SKIP) or Ctrl Code (Use)   */
	/* Col 3: Google Text (SKIP)                  */
	/* Col 4: Corrected Text                      */
	/**********************************************/

	/* Determine CSV Input File Size */
	if (fseek(csvfile, 0, SEEK_END) != 0){
		printf("Error seeking in csv input file.\n");
		return -1;
	}
	fsize = ftell(csvfile);
	if (fseek(csvfile, 0, SEEK_SET) != 0){
		printf("Error seeking in csv input file.\n");
		return -1;
	}

	/* Read the entire CSV file into memory */
	pBuffer = (unsigned char*)malloc(fsize+1);
	if (pBuffer == NULL){
		printf("Error allocating to put csv input file in memory.\n");
		return -1;
	}
	memset(pBuffer, 0, fsize + 1);
	rval = fread(pBuffer, 1, fsize, csvfile);
	if (rval != fsize){
		printf("Error, reading csv file into memory\n");
		free(pBuffer);
		return -1;
	}

	/**************************************************/
	/* Parse the CSV input file to populate the array */
	/**************************************************/

	/* Skip Headers */
	pInput = (char*)strtok((char *)pBuffer, MENU_DELIM);
	pInput = (char*)strtok(NULL, MENU_DELIM);
	pInput = (char*)strtok(NULL, MENU_DELIM);
	pInput = (char*)strtok(NULL, MENU_DELIM);

	while (1){

		line++;

		/* Column 1 - Old Offset */
		pInput = ( char*)strtok(NULL, MENU_DELIM);
		if (pInput == NULL) {
			printf("Reached End of CSV Input Data.\n");
			break;
		}
		else if ((*pInput == '-') && (entryEndLocated)){
			/* Ignore any further data for this entry */
			/* Ignore rest of row */
			pInput = (char*)strtok(NULL, MENU_DELIM);
			pInput = (char*)strtok(NULL, MENU_DELIM);
			pInput = (char*)strtok(NULL, MENU_DELIM);
			continue;
		}
		else if (*pInput == '-'){
			/* Continuation */
			;
		}
		else if (*pInput != '\0'){
			unsigned int offset;
			sscanf((char*)pInput, "%X", &offset);
			menuRecordArray[numMenuItems].oldOffset = ((unsigned short)offset & 0xFFFF)/2;
			entryEndLocated = 0;
		}

		/* Columns 2 - Original Ctrl Code (Skip) */
		pInput = (char*)strtok(NULL, MENU_DELIM2);
		if (pInput == NULL) {
			printf("Reached Unexpected End of CSV Input Data.\n");
			break;
		}

		/* Skip Column 3 */
		pInput = (char*)strtok(NULL, MENU_DELIM);
		if (pInput == NULL) {
			printf("Error, unexpected end to CSV File.\n");
			break;
		}

		/* Column 4 - English Menu Translation  */
		/* (If Column 2 was not a control code) */
		pInput = (char*)strtok(NULL, MENU_DELIM);
		if (pInput == NULL) {
			printf("Error, unexpected end to CSV File.\n");
			break;
		}
		else{
			int index,z,numCharacters;
			unsigned char* ptr;
			unsigned char* pOut;

			/* Count # of characters */
			ptr = (unsigned char*)pInput;
			numCharacters = 0;
			while (*ptr != '\0'){
				numCharacters++;
				ptr += numBytesInUtf8Char(*ptr);
			}
			
			/* Control Code Entry */
			if ((pInput[0] == '0') && (pInput[1] == 'x')){
				unsigned int ctrlCode;
				int index;
				sscanf(pInput, "%X", &ctrlCode);
				index = menuRecordArray[numMenuItems].numSubRecords;
				menuRecordArray[numMenuItems].subRecords[index].ctrlCode = (unsigned short)ctrlCode & 0xFFFF;
				swap16(&(menuRecordArray[numMenuItems].subRecords[index].ctrlCode));
				menuRecordArray[numMenuItems].subRecords[index].convertedString = NULL;
				menuRecordArray[numMenuItems].numSubRecords++;

				/* Terminator Found */
				if (ctrlCode == 0xFFFF){
					numMenuItems++;
					entryEndLocated = 1;
				}

			}	
			
			/* Text Entry */
			else{
				index = menuRecordArray[numMenuItems].numSubRecords;
				menuRecordArray[numMenuItems].subRecords[index].convertedString = (unsigned short*)malloc(numCharacters * 2);
				if (menuRecordArray[numMenuItems].subRecords[index].convertedString == NULL){
					printf("Error creating converted string.\n");
					free(pBuffer);
					freeMenuData();
					return -1;
				}
				pOut = (unsigned char*)menuRecordArray[numMenuItems].subRecords[index].convertedString;
				memset(pOut, 0xFF, numCharacters * 2);

				ptr = (unsigned char*)pInput;
				z = 0;
				while (*ptr != '\0'){
					unsigned char tmpChar;
					if (*ptr == ' '){
						pOut[z++] = ' '; //Control Code replacement performed by draw routine
					}
					else{
						int rval;
						rval = getUTF8code_Byte((char*)ptr, &tmpChar);
						if (rval < 0){
							int numBytes, i;
							printf("WARNING: No table mapping for unicode character, inserting space!\n\tLine: %d\n\tHex Data: 0x",line);
							numBytes = numBytesInUtf8Char((unsigned char)*ptr);
							for (i = 0; i < numBytes; i++){
								printf("%X", ptr[i]);
							}
							printf("\n\n");
							tmpChar = 0;
						}
						pOut[z++] = tmpChar;
					}				
					ptr += numBytesInUtf8Char(*ptr);
				}
				//Align End of Text
	//			if ((numCharacters & 0x1) != 0){
	//				pOut[z] = 0xFF;
	//				numCharacters++;
	//			}
				menuRecordArray[numMenuItems].subRecords[index].lenConvertedStringBytes = numCharacters;
				menuRecordArray[numMenuItems].numSubRecords++;
			}
		}
	}
	free(pBuffer);


	/********************************************/
	/* Copy Old Short Pointers to Output Buffer */
	/********************************************/
	outBuffer = (unsigned short*)malloc(MAX_MENU_SIZE*8*1024 / 2);
	if (outBuffer == NULL){
		printf("Error allocating output buffer size.\n");
		return -1;
	}
	for (x = 0; x < MAX_MENU_ITEMS; x++){
		if (fread(&outBuffer[x], 2, 1, infile) != 1){
			printf("Error reading menu offset at index %d.\n", x);
			free(outBuffer);
			freeMenuData();
			return -1;
		}
		swap16(&outBuffer[x]);
	}
	outputSizeBytes = x * 2;


	/*********************************************/
	/* Copy Converted Menu Text to Output Buffer */
	/*   Update menuRecordArray[x].newOffset     */
	/*********************************************/
	pMenuData = (unsigned char*)&outBuffer[x];
	for (x = 0; x < numMenuItems; x++){

		menuRecordArray[x].newOffset = (outputSizeBytes / 2);

		for (y = 0; y < menuRecordArray[x].numSubRecords; y++){
			if (menuRecordArray[x].subRecords[y].convertedString == NULL){
				memcpy(pMenuData, &menuRecordArray[x].subRecords[y].ctrlCode, 2);
				outputSizeBytes += 2;
				pMenuData += 2;
			}
			else{
				memcpy(pMenuData, menuRecordArray[x].subRecords[y].convertedString, menuRecordArray[x].subRecords[y].lenConvertedStringBytes);
				outputSizeBytes += menuRecordArray[x].subRecords[y].lenConvertedStringBytes;
				pMenuData += menuRecordArray[x].subRecords[y].lenConvertedStringBytes;
			}
		}
		
		//16-bit align the end of each record
		if (((unsigned int)pMenuData & 0x1) != 0x0){
			*pMenuData = 0xFF;
			pMenuData++;
			outputSizeBytes++;
		}

		/* Check for 32kB overflow */
		if (outputSizeBytes > MAX_MENU_SIZE){
			printf("Error, Menu will NOT fit in memory.\n");
			free(outBuffer);
			freeMenuData();
			return -1;
		}
	}


	/******************************************/
	/* Update Short Pointers in Output Buffer */
	/******************************************/
	for (x = 0; x < MAX_MENU_ITEMS; x++){
		int found = 0;
		unsigned short oldPointer;
		oldPointer = outBuffer[x];
		if (oldPointer != 0x0000){
			for (y = 0; y < MAX_MENU_ITEMS; y++){
				if (oldPointer == menuRecordArray[y].oldOffset){
					outBuffer[x] = menuRecordArray[y].newOffset;
					swap16(&outBuffer[x]); /* Fix Endian */
					found = 1;
				}
			}
			if (!found){
				printf("Error locating updated record for 0x%X\n", (unsigned int)oldPointer);
				swap16(&outBuffer[x]);
				free(outBuffer);
				freeMenuData();
				return -1;
			}
		}
	}
	freeMenuData();

	/**************************************/
	/* Write Output Buffer to Output File */
	/**************************************/
	fwrite(outBuffer, 1, outputSizeBytes, outfile);
	free(outBuffer);


	return 0;
}




/******************/
/* Free Menu Data */
/******************/
static void freeMenuData(){
	int x, y, numSubItems;

	for (x = 0; x < numMenuItems; x++){
		numSubItems = menuRecordArray[x].numSubRecords;
		for (y = 0; y < numSubItems; y++){
			if (menuRecordArray[x].subRecords[y].convertedString != NULL){
				free(menuRecordArray[x].subRecords[y].convertedString);
			}
		}
	}
	return;
}

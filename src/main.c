/******************************************************************************/
/* main.c - Main execution file for lunar Menu Editor                         */
/******************************************************************************/

/***********************************************************************/
/* Lunar Menu Editor (lunar_menu.exe) Usage                            */
/* ========================================                            */
/* lunar_menu.exe decodemenu InputMenuFname [sss]                      */
/* lunar_menu.exe encodemenu InputMenuFname InputCsvFname [sss]        */
/*     sss will interpret SSSC JP table as the SSS JP table.           */
/*     encoding assumes UTF8 characters.                               */
/*     Output will encode text with 8 bits per byte.  Control Codes    */
/*     are 16-bit, and records of control codes/text will start and    */
/*     end on 16-bit aligned boundaries.  Not completing on a boundary */
/*     will result in a filler byte of 0xFF appended at the end.       */
/***********************************************************************/
#ifdef _MSC_VER
#pragma warning(disable:4996)
#endif

/* Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse_menu.h"
#include "encode_menu.h"
#include "util.h"

/* Defines */
#define MODE_DECODE_MENU 1
#define MODE_ENCODE_MENU 2


/* Globals */
unsigned int G_iFileSizeBytes = 0;


/******************************************************************************/
/* printUsage() - Display command line usage of this program.                 */
/******************************************************************************/
void printUsage(){
    printf("Error in input arguments.\n");
    printf("Usage:\n=========\n");
	printf("lunar_menu.exe decodemenu InputMenuFname [sss]\n");
	printf("lunar_menu.exe encodemenu InputMenuFname InputCsvFname [sss]\n");
	printf("\tsss will interpret SSSC JP table as the SSS JP table.\n");
	printf("\n\n");
    return;
}


/******************************************************************************/
/* main()                                                                     */
/******************************************************************************/
int main(int argc, char** argv){

    FILE *inFile, *outFile, *inCSVFile;
    static char inFileName[300];
	static char csvFileName[300];
    static char outFileName[300];
    int rval, mode, minarg;

    /**************************/
    /* Check input parameters */
    /**************************/

	/* So it doesnt crash */
	if (argc < 3){
		printUsage();
		return -1;
	}
	minarg = 3;

    /* Parameter #1 - Verify decode or encode */
    if ((strcmp(argv[1], "decodemenu") == 0)){
		mode = MODE_DECODE_MENU;

		/* Parameter #2 */
		/* Copy input/output file name parameters to local variables */
		memset(inFileName, 0, 300);
		strncpy(inFileName, argv[2], 299);
		memset(outFileName, 0, 300);
	}
    else if( (strcmp(argv[1],"encodemenu") == 0) ){
        mode = MODE_ENCODE_MENU;
		minarg++;

		/* Parameter #2 */
		/* Copy input/output file name parameters to local variables */
		memset(inFileName, 0, 300);
		strncpy(inFileName, argv[2], 299);
		memset(outFileName, 0, 300);

		/* Parameter #3 */
		/* Copy csv input file name parameter to local variable */
		memset(csvFileName, 0, 300);
		strncpy(csvFileName, argv[3], 299);
	}
    else{
        printUsage();
        return -1;
    }

	/* Not enough args */
	if (argc < minarg){
		printUsage();
		return -1;
	}


    /* Last Parameter - Identify SSS or SSSC */
	if (argc >= (minarg+1)){
		if (strcmp(argv[minarg], "sss") == 0){
			setSSSEncode();
		}
	}

    
    /***********************************************/
    /* Finished Checking the Input File Parameters */
    /***********************************************/
	setBaseName(inFileName);
	strncpy(outFileName, getBaseName(), 200);
	if (mode == MODE_DECODE_MENU){
		strcat(outFileName, "_menu_out.csv");
	}
	else{
		strcat(outFileName, "_menu_out.dat");
	}


    /********************************************/
    /* Perform Decoding or Encoding Accordingly */
    /********************************************/
    if(mode == MODE_DECODE_MENU){

        printf("\n\nDECODE MENU Mode Entered.\n\n");

		/* Load in the Table File for Decoding Text */
		if (loadUTF8Table("text_table.txt") < 0){
			printf("Error loading UTF8 Table for Text Decoding.\n");
			return -1;
		}

        /* Open the input/output files */
        inFile = outFile = NULL;
        inFile = fopen(inFileName,"rb");
        if(inFile == NULL){
			printf("Error occurred while opening %s for reading\n", inFileName);
            return -1;
        }
		if (fseek(inFile, 0, SEEK_END) != 0){
			printf("Error seeking in input file.\n");
			return -1;
		}
		G_iFileSizeBytes = ftell(inFile);
		if (fseek(inFile, 0, SEEK_SET) != 0){
			printf("Error seeking in input file.\n");
			return -1;
		}

        outFile = fopen(outFileName,"w");
        if(outFile == NULL){
            printf("Error occurred while opening %s for writing\n",outFileName);
            fclose(inFile);
            return -1;
        }

        /* Parse the Input File for Decoding */
		rval = decodeMenu(inFile, outFile);

		/* Check return value */
		if (rval == 0){
			printf("Input File Decoded Successfully\n");
		}
		else{
			printf("Input File Decoding FAILED\n");
		}
    }
    else{

        printf("\n\nENCODE MENU Mode Entered.\n\n");

        /* Open the input/output files */
		inFile = outFile = inCSVFile = NULL;
        inFile = fopen(inFileName,"rb");
        if(inFile == NULL){
			printf("Error occurred while opening %s for reading\n", inFileName);
            return -1;
        }
		inCSVFile = fopen(csvFileName, "rb");
		if (inCSVFile == NULL){
			printf("Error occurred while opening %s for reading\n", csvFileName);
			return -1;
		}
        outFile = fopen(outFileName,"wb");
        if(outFile == NULL){
			printf("Error occurred while opening %s for writing\n", outFileName);
            fclose(inFile);
            return -1;
        }
#if 0
		// Load Table
		if (loadBPETable("bpe.table", "8bit_table.txt") < 0){
			printf("Error loading BPE Tables for Text Encoding/Decoding.\n");
			return -1;
		}
#else
		/* Load in the Table File for Decoding Text */
		if (loadUTF8Table("8bit_menu_table.txt") < 0){
			printf("Error loading UTF8 Table for Menu Decoding.\n");
			return -1;
		}
#endif
        /* Parse the Input File for Encoding */    
		rval = encodeMenu(inFile,inCSVFile,outFile);
		rval = 0;
        if(rval == 0){
            printf("Input File Encoded Successfully\n");
        }
        else{
            printf("Input File Encoding FAILED\n");
        }
		fclose(inCSVFile);
    }

    /* Close files */
    fclose(inFile);
    fclose(outFile);

    return 0;
}

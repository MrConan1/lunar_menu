#ifndef UTIL_H
#define UTIL_H

/* Function Prototypes */
void setSSSEncode();
int getSSSEncode();
void swap16(void* pWd);
void swap32(void* pWd);
int numBytesInUtf8Char(unsigned char val);
int loadUTF8Table(char* fname);
int getUTF8character(int index, char* utf8Value);
int getUTF8code_Byte(char* utf8Value, unsigned char* utf8Code);
int getUTF8code_Short(char* utf8Value, unsigned short* utf8Code);
void setBaseName(char* basename);
char* getBaseName();

#endif

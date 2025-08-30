#pragma once
#ifndef __MAIN_H
#define __MAIN_H

#include <stdint.h>

typedef struct _BFLHeader {
	char ident[4];
	int32_t size;
} BFLHeader;

typedef struct _BFLFileList {
	int32_t fileSize;
	int32_t fileOffset;
	int32_t fileNameLength;
	//char fileName[12];
} BFLFileList;

#endif

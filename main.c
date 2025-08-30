#include "main.h"
#include "zlib/zlib.h"
#include <stdio.h>
#include <stdbool.h>
#include <malloc.h>
#include <string.h>
#include <Windows.h>

char* bfl_data = NULL; /* Our data buffer */
int32_t bfl_length; /* Size of data buffer */

bool bfl_createdirectory(const char* path) {
    uint32_t fileAttributes = GetFileAttributes(path);

    if (fileAttributes == INVALID_FILE_ATTRIBUTES) {
        if (!CreateDirectory(path, 0)) {
            return false;
        }
    }
    else if ((fileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
        return false;
    }

    return true;
}

bool bfl_writeoutputfile(const char* fileName, char* buffer, int length) {
    FILE* fp = fopen(fileName, "wb");
    if (fp != NULL) {
        fwrite(buffer, 1, length, fp);
        fclose(fp);
        return true;
    }

    return false;
}

void bfl_extractfile(const char* fileName, int32_t offset, int32_t size) {
    char file[32];
    /* Hardcode this for now */
    bfl_createdirectory("extracted");
    sprintf(file, "extracted/%s", fileName);

    offset += sizeof(BFLHeader);

    if (bfl_writeoutputfile(file, bfl_data + offset, size))
        printf("Extracting %s...\n", fileName);
}

void bfl_readfilelist() {
    if (bfl_data == NULL)
        return;

    int32_t fileListOffset = bfl_length - 4;
    int32_t fileListPos = *(int32_t*)(bfl_data + fileListOffset);
    fileListPos += sizeof(BFLHeader);

    char fileName[14];

    int32_t offset = fileListPos;
    while (offset < fileListOffset) {
        /* Read the file list (minus file names) */
        BFLFileList* fileList = (BFLFileList*)(bfl_data + offset);
        offset += sizeof(BFLFileList);

        /* Read file names seperately as they're not all the same length */
        memcpy(fileName, bfl_data + offset, fileList->fileNameLength);
        offset += fileList->fileNameLength;

        /* null terminator */
        fileName[fileList->fileNameLength] = '\0';

        /* Extract the damn file */
        bfl_extractfile(fileName, fileList->fileOffset, fileList->fileSize);
        
        /* Skip the empty space after fileName strings */
        uint8_t i = *(uint8_t*)(bfl_data + offset);
        while (i == 0) {
            offset++;
            i = *(uint8_t*)(bfl_data + offset);
        }
    }
}

bool bfl_uncompress(const char* fileName) {
    int32_t size = 0x4000000; /* 4MB */

    gzFile fp = gzopen(fileName, "r");
    if (fp != NULL) {
        printf("Decompressing %s...\n", fileName);
        bfl_data = malloc(size);
        if (bfl_data != NULL) {
            bfl_length = gzread(fp, bfl_data, size); /* Read actual data length */
        }

        gzclose(fp);

        return true;
    }

    return false;
}

void stringfromcommandline(const char* cmdLine, char* str, int32_t length)
{
    int32_t len = length - 1;

    while (*cmdLine == ' ') cmdLine++;

    int32_t i = 0;
    while (i < len &&
        *cmdLine &&
        *cmdLine != ' ' &&
        *cmdLine != '-' &&
        *cmdLine != '/')
    {
        if (*cmdLine == '"') {
            cmdLine++;
            continue;
        }

        *str = *cmdLine;
        str++;
        cmdLine++;
        i++;
    }
    *str = '\0';
}

int main() {
    char fileName[MAX_PATH];
    char* cmdLine = GetCommandLine();

    while (*cmdLine) {
        if (*cmdLine == '-' || *cmdLine == '/') {
            cmdLine++;
            switch (*cmdLine) {
            case 'x':
            case 'X':
                cmdLine++;
                stringfromcommandline(cmdLine, fileName, sizeof(fileName));
                break;
            }
        }
        
        cmdLine++;
    }

    if (!fileName || !*fileName)
        return 1;

    if (bfl_uncompress(fileName))
        bfl_readfilelist();

    if (bfl_data != NULL)
        free(bfl_data);

	return 0;
}

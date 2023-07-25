/*
READ BITS by bathtaters
*/

#ifndef READ_BITS_H
#define READ_BITS_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define BYTE_LEN 8
#define CEIL_DIV(x,y) (1 + (((x) - 1) / (y)))

/* Object to be passed to BitReader functions (Interacting directly with this will cause undefined behavior!) */
typedef struct BitReader {
    /* File object to read from */
    FILE* file;
    /* Current byte being read */
    uint8_t byte;
    /* Current bit bitOffset within this.byte */
    int bitOffset;
    /* Non-zero if this.file is currently open */
    char canRead;
    /* Read starting from left (1) or right (0) */
    char msbFirst;
} BitReader;

/* Creates aa new BitReader object (result.canRead == 1 if successful) */
BitReader newBitReader(char *filename, char msbFirst);
/* Closes file & frees BitReader memory */
void freeBitReader(BitReader br);
/* Get the next [bitCount] bits as byte array (Array size = ceil of bitCount/8) */
uint8_t* getBits(BitReader* br, int bitCount);
/* Seek to the given byte/bit offset (whence = SEEK_[CUR|END|SET]), returns 0 on success */
int seekBits(BitReader* br, long int byteOffset, int bitOffset, int whence);

/* Utility to print Binary data for testing */
void printbin(uint8_t* bindata, char bitWidth);
/* Utility to flip array between endianess */
void swap_bytes(uint8_t* val, int bitWidth);

#endif
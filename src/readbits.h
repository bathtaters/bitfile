/*
READ BITS by bathtaters
*/

#ifndef READ_BITS_H
#define READ_BITS_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define BYTE_LEN 8
#define BIT_LIMIT (int)(sizeof(uint64_t) * BYTE_LEN)

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
} BitReader;

/* Creates a new BitReader object (result.canRead == 1 if successful) */
BitReader* newBitReader(char *filename);
/* Closes file & frees BitReader memory */
void freeBitReader(BitReader* br);
/* Get the next [bitCount] bits as a uint64 (Result must fit into uint64) */
uint64_t getBits(BitReader* br, char bitCount);
/* Seek to the given byte/bit offset (whence = SEEK_[CUR|END|SET]) */
BitReader* seekBits(BitReader* br, long int byteOffset, int bitOffset, int whence);

/* Utility to print Binary data for testing */
void printbin(uint64_t bindata, char bitWidth);

#endif
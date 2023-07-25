#include "readbits.h"

uint8_t getBit(BitReader* br);
int getByte(BitReader* br);
int alignByte(BitReader* br);


uint8_t* getBits(BitReader* br, int bitCount)
{
    const int byteCount = CEIL_DIV(bitCount, BYTE_LEN);
    uint8_t* result = calloc(byteCount, sizeof(uint8_t));

    if (!br->canRead)
    {
        fprintf(stderr, "Error: Attempting to read a closed file.\n");
        return result;
    }

    /* Result array is always big-endian */
    int byteOffset = -1;
    for (char bitOffset = 0; bitOffset < bitCount; bitOffset++)
    {
        /* Increment byte reader & writer */
        if (alignByte(br)) return result;
        if (bitOffset % BYTE_LEN == 0) byteOffset++;

        /* Read bit by bit */
        uint8_t newBit = getBit(br);
        if (br->msbFirst) result[byteOffset] = (result[byteOffset] << 1) | newBit;
        else result[byteOffset] |= newBit << (bitOffset % BYTE_LEN);
    }

    return result;
}


int seekBits(BitReader* br, long int byteOffset, int bitOffset, int whence)
{
    if (br->file == NULL)
    {
        fprintf(stderr, "Error: Attempting to seek a closed file.\n");
        return -1;
    }

    /* Include current bitOffset if seeking from current position */
    if (whence == SEEK_CUR) bitOffset += br->bitOffset;

    /* Allow bitCount to overflow */
    while (bitOffset >= BYTE_LEN)
    {
        bitOffset -= BYTE_LEN;
        byteOffset++;
    }
    while (bitOffset < 0)
    {
        bitOffset += BYTE_LEN;
        byteOffset--;
    }
    
    fseek(br->file, byteOffset, whence);

    /* Update BitReader parameters */
    if (getByte(br)) return 1;
    br->bitOffset = bitOffset;
    br->canRead = true;
    return 0;
}


BitReader newBitReader(char *filename, bool msbFirst)
{
    BitReader br;

    br.file = fopen(filename, "rb");
    br.byte = 0;
    br.bitOffset = 0;
    br.canRead = br.file != NULL;
    br.msbFirst = msbFirst;

    if (br.canRead) getByte(&br); /* Ingest first byte */
    return br;
}


void freeBitReader(BitReader br)
{
    if (br.file != NULL) fclose(br.file);
    br.file = NULL;
}







/* --- BR UTILITIES --- */

/* Align memory byte to bitOffset */
int alignByte(BitReader* br)
{
    while (br->bitOffset >= BYTE_LEN)
    {
        if (getByte(br)) return 1; /* Return if EOF reached. */
        br->bitOffset -= BYTE_LEN;
    }
    return 0;
}

/* Reads next byte of br.file into br.byte.
    - OnSuccess: return 0
    - OnFailure: return 1 and br.canRead = false */
int getByte(BitReader* br)
{
    int nextByte = getc(br->file);
    if (nextByte == EOF)
    {
        fprintf(stderr, "Warning: Reached end of file.\n");
        br->canRead = false;
        return 1;
    }

    br->byte = (uint8_t)nextByte;
    return 0;
}

/* Gets the value of a bit offset by br.bitOffset into br.byte */
uint8_t getBit(BitReader* br)
{
    char offset = br->bitOffset++;
    if (br->msbFirst) offset = BYTE_LEN - 1 - offset;

    return !!(br->byte & (1 << offset));
}

/* Print value as binary - For debugging/testing */
#define INT64BYTES 8
void printbin(uint8_t* bindata, int bitWidth)
{
    const int byteCount = CEIL_DIV(bitWidth, BYTE_LEN);

    /* Print numeric value (w/ ">" if overflowing 64-bits) */
    uint64_t num = 0;
    for (int i = 0; i < byteCount && i < INT64BYTES; i++)
    {
        num |= bindata[i] << (i * BYTE_LEN);
    }
    printf("%c%8llu,", byteCount > 8 ? '>' : ' ', num); /* Add a > if > 64-bit */

    /* Print hex value */
    for (int i = 0; i < byteCount; i++) printf(" %02X", bindata[i]);
    printf(", ");

    /* Print binary value */
    for (int j = 0; j < bitWidth; j++)
    {
        if (j && j % BYTE_LEN == 0) printf(" ");
        printf("%c", bindata[j / BYTE_LEN] & (0x1 << (j % BYTE_LEN)) ? '1' : '0');
    }
}

/* Convert array between endianess using bitWidth */
void swapbytes(uint8_t* bindata, int bitWidth)
{
    int offsetFactor = CEIL_DIV(bitWidth, BYTE_LEN);
    int half = offsetFactor / 2;
    offsetFactor--; /* 0-index for offset math */
    
    /* Go through 50% of array and swap all mirrored bytes (a & b) */
    for (int a = 0; a < half; a++)
    {
        int b = offsetFactor - a;

        uint8_t temp = bindata[a];
        bindata[a] = bindata[b];
        bindata[b] = temp;
    }
}
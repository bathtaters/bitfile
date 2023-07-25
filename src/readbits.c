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
        printf("Error: Attempting to read a closed file.\n");
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

        // printf("\t> rOffset: %03d / %03d, bOffset: %d / %d, byteOffset: %02d / %02d, bitVal: %d, byteVal: %03u, result: ",bitOffset,bitCount,br->bitOffset,BYTE_LEN,byteOffset,byteCount,newBit,br->byte);printbin(result,bitCount);printf("\n");
    }

    return result;
}


int seekBits(BitReader* br, long int byteOffset, int bitOffset, int whence)
{
    if (br->file == NULL)
    {
        printf("Error: Attempting to seek a closed file.\n");
        return -1;
    }

    /* Include current bitOffset if seeking from current position */
    if (whence == SEEK_CUR)
    {
        bitOffset += br->bitOffset;
    }

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
    
    // printf("\t> SEEK FROM: %ld [%d]; (%ld, %d, %d)",ftell(br->file),br->bitOffset,byteOffset,bitOffset,whence);
    
    fseek(br->file, byteOffset, whence);

    // printf("; SEEK TO: %ld [%d]\n",ftell(br->file),bitOffset);

    /* Update BitReader parameters */
    if (getByte(br)) return 1;
    br->bitOffset = bitOffset;
    br->canRead = 1;
    return 0;
}


BitReader newBitReader(char *filename, char msbFirst)
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
        if (getByte(br)) { return 1; } /* Return if EOF reached. */
        br->bitOffset -= BYTE_LEN;
    }
    return 0;
}

/* Reads next byte of br.file into br.byte.
    - OnSuccess: return 0
    - OnFailure: return 1 and br.canRead = 0 */
int getByte(BitReader* br)
{
    int nextByte = getc(br->file);
    if (nextByte == EOF)
    {
        printf("Warning: Reached end of file.\n");
        br->canRead = 0;
        return 1;
    }

    br->byte = (uint8_t)nextByte;
    return 0;
}

/* Gets the value of a bit offset by br.bitOffset into br.byte */
uint8_t getBit(BitReader* br)
{
    char offset = br->bitOffset++;
    if (br->msbFirst)
    {
        offset = BYTE_LEN - 1 - offset;
    }
    return !!(br->byte & (1 << offset));
}

/* Print value as binary - For debugging/testing */
#define INT64BYTES 8
void printbin(uint8_t* bindata, char bitWidth)
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
void swap_bytes(uint8_t* val, int bitWidth)
{
    int offsetFactor = CEIL_DIV(bitWidth, BYTE_LEN);
    int half = offsetFactor / 2;
    offsetFactor--; /* 0-index for offset math */
    
    /* Go through 50% of array and swap all mirrored bytes (a & b) */
    for (int a = 0; a < half; a++)
    {
        int b = offsetFactor - a;

        uint8_t temp = val[a];
        val[a] = val[b];
        val[b] = temp;
    }
}
#include "readbits.h"

uint8_t getBit(BitReader* br);
int getByte(BitReader* br);
int alignByte(BitReader* br);

uint64_t getBits(BitReader* br, char bitCount)
{
    uint64_t result = 0;

    if (!br->canRead)
    {
        printf("Error: Attempting to read a closed file.\n");
        return result;
    }
    if (bitCount > BIT_LIMIT)
    {
        printf("Error: Attempting to read too many bits: %d. Returning last %d bits.\n", bitCount, BIT_LIMIT);
        seekBits(br, 0, bitCount - BIT_LIMIT, SEEK_CUR);
        return getBits(br, BIT_LIMIT);
    }
    
    for (char resOffset = 0; resOffset < bitCount; resOffset++)
    {
        if (alignByte(br)) { return result; }

        /* Read bit by bit */
        uint64_t newBit = getBit(br);
        if (br->msbFirst) { result = (result << 1) | newBit; }
        else { result |= newBit << resOffset; }

        // printf("\t> rCount: %03d, rOffset: %03d, bOffset: %d, byteVal: %03u, result: ",bitCount,resOffset,br->bitOffset,br->byte);printbin(result,bitCount);printf("\n");
    }

    alignByte(br);
    return result;
}


BitReader* seekBits(BitReader* br, long int byteOffset, int bitOffset, int whence)
{
    if (br->file == NULL)
    {
        printf("Error: Attempting to seek a closed file.\n");
        return br;
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
    if (getByte(br)) { return br; }
    br->bitOffset = bitOffset;
    br->canRead = 1;
    return br;
}


BitReader* newBitReader(char *filename, char msbFirst)
{
    BitReader* br = (BitReader*)calloc(1, sizeof(BitReader));

    br->file = fopen(filename, "rb");
    br->canRead = br->file != NULL;
    if (br->canRead) { getByte(br); } /* Ingest first byte */
    br->msbFirst = msbFirst;
    return br;
}


void freeBitReader(BitReader* br)
{
    if (br->file != NULL) { fclose(br->file); }
    free(br);
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

/* Print value as binary (Big-Endian) - For debugging */
void printbin(uint64_t bindata, char bitWidth)
{
    printf("%-8llu ", bindata);

    const int spaces = bitWidth % BYTE_LEN;
    for (int i = BYTE_LEN; spaces && i > spaces; i--) { printf(" "); }

    for (int j = bitWidth; j > 0; j--)
    {
        if (j != bitWidth && j % BYTE_LEN == 0) { printf(" "); }
        printf("%c", bindata & (0x1 << (j - 1)) ? '1' : '0');
    }
}
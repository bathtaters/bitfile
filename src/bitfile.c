#include "bitfile.h"

uint8_t getBit(BITFILE* bitfile);
int getByte(BITFILE* bitfile);
int alignByte(BITFILE* bitfile);
int copyByteAccessMode(const char* basic_access, char* byte_access);


/* --- OPEN/CLOSE FUNCTIONS --- */

BITFILE* bfopen(const char* filename, const char* access_mode, bool msb_first)
{
    char access[ACCESS_MODE_LEN];
    if (copyByteAccessMode(access_mode, access))
    {
        fprintf(stderr, "ERROR: Invalid file access mode: %s\n", access_mode);
        return NULL;
    }

    FILE* fileobj = fopen(filename, access);
    if (fileobj == NULL)
    {
        fprintf(stderr, "ERROR: File unable to be opened for '%s': %s\n", access, filename);
        return NULL;
    }

    BITFILE* bitfile = malloc(sizeof(BITFILE));

    bitfile->_fileobj = fileobj;
    bitfile->_currbyte = 0x0;
    bitfile->_bitoffset = BYTE_LEN; /* Force read on next operation */
    bitfile->_msb = msb_first;

    return bitfile;
}

int bfclose(BITFILE* bitfile)
{
    int result = fclose(bitfile->_fileobj);
    free(bitfile);
    return result;
}


/* --- READ/WRITE FUNCTIONS --- */

uint64_t bfread(void* save_to_ptr, uint64_t number_of_bits, BITFILE* bitfile)
{
    uint64_t readCount = 0;
    byte_t* result = save_to_ptr;

    for (int byteIdx = -1; readCount < number_of_bits; readCount++)
    {
        /* Increment byte reader & writer */
        if (alignByte(bitfile)) return readCount;
        if (readCount % BYTE_LEN == 0) byteIdx++;

        /* Read bit by bit */
        byte_t newBit = getBit(bitfile);
        if (bitfile->_msb) result[byteIdx] = (result[byteIdx] << 1) | newBit;
        else result[byteIdx] |= newBit << (readCount % BYTE_LEN);
    }

    return readCount;
}


/* --- POSITION FUNCTIONS --- */

int bfseek(BITFILE* bitfile, long int byte_offset, int64_t bit_offset, int whence)
{
    if (whence == SEEK_CUR)
    {
        /* Catch unset _bitoffset */
        if (bitfile->_bitoffset == -1)
        {
            fprintf(stderr, "ERROR: Cannot seek from current file position with no current position.\n");
            return -1;
        }

        /* Include current bitOffset if seeking from current position */
        bit_offset += bitfile->_bitoffset;
    }

    /* Allow bitCount to overflow */
    while (bit_offset >= BYTE_LEN)
    {
        bit_offset -= BYTE_LEN;
        byte_offset++;
    }
    while (bit_offset < 0)
    {
        bit_offset += BYTE_LEN;
        byte_offset--;
    }
    
    fseek(bitfile->_fileobj, byte_offset, whence);

    /* Update BITFILE parameters */
    bitfile->_bitoffset = bit_offset;
    if (!getByte(bitfile)) return 0;
    bitfile->_bitoffset = 0;
    return 1;
}



/* --- UTILITIES --- */

void swapendian(void* bin_data, uint64_t number_of_bits)
{
    byte_t* data_ptr = (byte_t *)bin_data;

    int offsetFactor = CEIL_DIV(number_of_bits, BYTE_LEN);
    int half = offsetFactor / 2 - 1;
    
    /* Go through 50% of array and swap all mirrored bytes (a & b) */
    for (int a = 0; a < half; a++)
    {
        int b = offsetFactor - a;

        byte_t temp = data_ptr[a];
        data_ptr[a] = data_ptr[b];
        data_ptr[b] = temp;
    }
}

void printbin(const void* bin_data, uint64_t number_of_bits)
{
    byte_t* data_ptr = (byte_t *)bin_data;

    for (int j = 0; j < number_of_bits; j++)
    {
        if (PRINT_BYTE_SPACES && j && j % BYTE_LEN == 0) printf(" ");
        printf("%c", data_ptr[j / BYTE_LEN] & (0x1 << (j % BYTE_LEN)) ? '1' : '0');
    }
}


/* --- INTERNAL HELPERS --- */

/* Size of valid_access_chars array */
#define VALID_ACCESS_CHARS_OUTER_SZ (ACCESS_MODE_LEN - 1)
/* Size of valid_access_chars elements */
#define VALID_ACCESS_CHARS_INNER_SZ 3
/* Valid letters for each char index of access_mode */
const char valid_access_chars[VALID_ACCESS_CHARS_OUTER_SZ][VALID_ACCESS_CHARS_INNER_SZ] = {
    { 'r', 'w', 'a' },
    { '+', 'b',  0  },
    { '+', 'b',  0  },
};

/* Check if val is in arr */
bool isIn(char val, const char* arr, int arr_size)
{
    for (int i = 0; i < arr_size; i++)
    {
        if (arr[i] == val) return true;
    }
    return false;
}

/* Align memory byte to bitoffset */
int alignByte(BITFILE* bitfile)
{
    if (bitfile->_bitoffset < 0 || bitfile->_currbyte == EOF) return 2; /* Cannot read */

    while (bitfile->_bitoffset >= BYTE_LEN)
    {
        if (getByte(bitfile)) return 1; /* EOF */
        bitfile->_bitoffset -= BYTE_LEN;
    }
    return 0;
}

/* Reads next byte of bitfile._fileobj into bitfile._currbyte.
    - Success: return 0
    - EOF: return 1 and bitfile._currbyte = EOF */
int getByte(BITFILE* bitfile)
{
    int nextByte = getc(bitfile->_fileobj);
    bitfile->_currbyte = nextByte;
    return nextByte == EOF;
}

/* Gets the value of a bit offset by bitfile._bitoffset into bitfile._currbyte */
byte_t getBit(BITFILE* bitfile)
{
    int8_t offset = bitfile->_bitoffset++;
    if (bitfile->_msb) offset = BYTE_LEN - 1 - offset;

    return !!(bitfile->_currbyte & (1 << offset));
}

/* Store access mode w/ appended 'b' in byte_access
    - Expects: r,w,a,r+,w+,a+,rb,wb,ab,rb+,wb+,ab+,r+b,w+b,a+b
    - Success: return 0
    - Invalid string: return 1 */
int copyByteAccessMode(const char* basic_access, char* byte_access)
{
    int len = strlen(basic_access);
    if (len < 1 || len > VALID_ACCESS_CHARS_OUTER_SZ) return 1;

    bool has_b = false;
    for (int i = 0; i < len; i++)
    {
        if (!isIn(basic_access[i], valid_access_chars[i], VALID_ACCESS_CHARS_INNER_SZ))

        if (basic_access[i] == 'b') has_b = true;
        byte_access[i] = basic_access[i];
    }

    if (!has_b) byte_access[len++] = 'b';
    byte_access[len] = '\0';
    return 0;
}
#include "bitfile.h"

/* --- INTERNAL MACROS & FUNCTION DEFINITIONS --- */

/* Calculate bit offset within byte based on if counting most significant byte first or not */
#define MSB_OFFSET(bit, flags) (flags & BF_FLAG_MSB ? BYTE_LEN - 1 - (bit) : bit)
/* Calculate bit shift for partial byte writes if counting most significant byte first */
#define MSB_SHIFT(n) ((BYTE_LEN - (n)) % BYTE_LEN)

int8_t incBitOffset(BITFILE* bitfile);
void bfgetbit(byte_t* dst, bsize_t offset, BITFILE* bitfile);
void bfputbit(byte_t src, bsize_t offset, BITFILE* bitfile);
int getByte(BITFILE* bitfile);
int writeByte(BITFILE* bitfile, bool inc);
int alignByte(BITFILE* bitfile);
void bfreset(BITFILE* bitfile, bool msb_first);
uint8_t copyByteAccessMode(const char* basic_access, char* byte_access);


/* --- OPEN/CLOSE FUNCTIONS --- */

BITFILE* bfopen(const char* filename, const char* access_mode, bool msb_first)
{
    char access[ACCESS_MODE_LEN];
    uint8_t flags = copyByteAccessMode(access_mode, access);
    if (flags & BF_FLAG_ERR) return NULL;

    FILE* fileobj = fopen(filename, access);
    if (fileobj == NULL) return NULL;

    BITFILE* bitfile = malloc(sizeof(BITFILE));
    bitfile->_flags = flags;
    bitfile->_fileobj = fileobj;
    bfreset(bitfile, msb_first);
    return bitfile;
}

int bfclose(BITFILE* bitfile)
{
    int result = fclose(bitfile->_fileobj);
    free(bitfile);
    return result;
}

BITFILE* bfreopen(const char* filename, const char* access_mode, bool msb_first, BITFILE* bitfile)
{
    char access[ACCESS_MODE_LEN];
    bitfile->_flags = copyByteAccessMode(access_mode, access);
    if (bitfile->_flags & BF_FLAG_ERR) return NULL;

    bitfile->_fileobj = freopen(filename, access, bitfile->_fileobj);
    if (bitfile->_fileobj == NULL) return NULL;

    bfreset(bitfile, msb_first);
    return bitfile;
}

BITFILE* tmpbitfile(char* nametemplate, bool msb_first)
{
    if (!mkstemp(nametemplate)) return NULL;
    return bfopen(nametemplate, TMP_FILE_ACCESS, msb_first);
}


/* --- READ/WRITE FUNCTIONS --- */

bsize_t bfread(void* ptr, bsize_t number_of_bits, BITFILE* bitfile)
{
    if (!(bitfile->_flags & BF_FLAG_READ))
    {
        fread(ptr, 1, 1, bitfile->_fileobj);
        return 0;
    }

    bsize_t readCount = 0;
    byte_t* output = ptr;
    byte_t* endptr = output + CEIL_DIV(number_of_bits, BYTE_LEN);

    output--; /* Fix for initial cycle */

    while (readCount < number_of_bits)
    {
        /* Increment reader & zero-out new output byte -- break on EOF */
        if (alignByte(bitfile)) break;
        if (readCount % BYTE_LEN == 0) *(++output) = 0x0;

        /* Read bit by bit into output array */
        bfgetbit(output, readCount++, bitfile);
    }

    /* Shift or Zero-out remaining bits */
    for (int8_t b = readCount % BYTE_LEN; b < BYTE_LEN && b; b++)
    {
        if (bitfile->_flags & BF_FLAG_MSB) *output >>= 1;
        else *output &= ~(0x1 << b);
    }
    /* Zero-out remaining bytes */
    while (++output < endptr) *output = 0x0;

    return readCount;
}

bsize_t bfwrite(void* ptr, bsize_t number_of_bits, BITFILE* bitfile)
{
    if (!(bitfile->_flags & BF_FLAG_WRITE))
    {
        fwrite(ptr, 1, 1, bitfile->_fileobj);
        return 0;
    }

    bsize_t writeCount = 0;
    byte_t* input = ptr;
    byte_t* endptr = input + CEIL_DIV(number_of_bits, BYTE_LEN) - 1;

    int8_t shift = 0; /* Fix for final MSB byte */

    /* Update buffer */
    if (alignByte(bitfile) == 2) return writeCount; /* Actual error */

    /* Align file byte cursor */
    if (bfeof(bitfile)) clearbferr(bitfile);
    else if (fseek(bitfile->_fileobj, -1, SEEK_CUR)) return writeCount;

    while (writeCount < number_of_bits)
    {
        /* Flush byte and increment writer */
        if (bitfile->_bitoffset >= BYTE_LEN)
        {
            if (writeByte(bitfile, true)) return writeCount;
            bitfile->_bitoffset = 0;
        }

        /* Increment input byte & Add in bit shift to final byte */
        if (writeCount % BYTE_LEN == 0)
        {
            if (writeCount) input++;
            if (bitfile->_flags & BF_FLAG_MSB && input == endptr)
            {
                shift = MSB_SHIFT(number_of_bits - writeCount);
            }
        }

        /* Write bit by bit from input array */
        bfputbit(*input, shift + writeCount++, bitfile);
    }

    /* Flush final write buffer */
    if (writeCount && writeByte(bitfile, false)) writeCount -= bitfile->_bitoffset;

    return writeCount;
}

int bfflush(BITFILE* bitfile)
{
    return fflush(bitfile->_fileobj);
}

int setbfbuf(BITFILE* bitfile, char* buffer, int mode, size_t size)
{
    if (mode == _IOLBF) return -1; /* Cannot do line buffering of bits */
    return setvbuf(bitfile->_fileobj, buffer, mode, size);
}

/* --- POSITION FUNCTIONS --- */

int bfseek(BITFILE* bitfile, bpos_t offset, int whence)
{
    fpos_t byte_offset = 0;

    /* Include current bitOffset and fix byte offset if seeking from current position */
    if (whence == SEEK_CUR)
    {
        offset += bitfile->_bitoffset;
        byte_offset--;
    }

    /* Allow bitCount to overflow */
    while (offset >= BYTE_LEN)
    {
        offset -= BYTE_LEN;
        byte_offset++;
    }
    while (offset < 0)
    {
        offset += BYTE_LEN;
        byte_offset--;
    }
    
    fseek(bitfile->_fileobj, byte_offset, whence);

    /* Update BITFILE parameters */
    bitfile->_bitoffset = offset;
    if (!getByte(bitfile)) return 0;
    bitfile->_bitoffset = 0;
    return 1;
}

bpos_t bftell(BITFILE* bitfile)
{
    return (bpos_t)ftell(bitfile->_fileobj) * (bpos_t)BYTE_LEN + bitfile->_bitoffset - BYTE_LEN;
}

void bfrewind(BITFILE* bitfile)
{
    rewind(bitfile->_fileobj);
    bfreset(bitfile, bitfile->_flags & BF_FLAG_MSB);
}

int bfgetpos(BITFILE* bitfile, bfpos_t* pos)
{
    int result = fgetpos(bitfile->_fileobj, &pos->byte);

    pos->byte--; /* File cursor is always +1 from bit cursor */
    pos->bit = bitfile->_bitoffset % BYTE_LEN;

    /* Wrap overflown bits */
    if (!result && bitfile->_bitoffset >= BYTE_LEN) pos->byte += bitfile->_bitoffset / BYTE_LEN;

    return bitfile->_bitoffset < 0 ? -1 : result;
}

int bfsetpos(BITFILE* bitfile, const bfpos_t* pos)
{
    if (pos->bit < 0 || pos->bit >= BYTE_LEN)
    {
        errno = EINVAL;
        return 1;
    }
    
    int result = fsetpos(bitfile->_fileobj, &pos->byte);
    if (result) return result;

    bitfile->_bitoffset = (int8_t)pos->bit;
    return getByte(bitfile);
}


/* --- ERROR FUNCTIONS --- */

int bferror(BITFILE* bitfile)
{
    int err = ferror(bitfile->_fileobj);
    if (err) return err;
    if (bitfile->_flags & BF_FLAG_ERR) return 1;
    if (bitfile->_bitoffset >= 0) return err;
    errno = EFAULT;
    return -1;
}

int bfeof(BITFILE* bitfile)
{
    int eof = feof(bitfile->_fileobj);
    if (eof) return eof;
    return bitfile->_currbyte == EOF;
}

void clearbferr(BITFILE* bitfile)
{
    if (bitfile->_bitoffset < 0) bitfile->_bitoffset = 0;
    if (bitfile->_currbyte == EOF) bitfile->_currbyte = 0;
    bitfile->_flags &= ~BF_FLAG_ERR;
    clearerr(bitfile->_fileobj);
}


/* --- UTILITIES --- */

void swapendian(void* bin_data, bsize_t number_of_bits)
{
    byte_t* data_ptr = (byte_t *)bin_data;

    int offsetFactor = CEIL_DIV(number_of_bits, BYTE_LEN);
    int half = offsetFactor / 2;
    offsetFactor--; /* 0-index offsetter */
    
    /* Go through 50% of array and swap all mirrored bytes (a & b) */
    for (int a = 0; a < half; a++)
    {
        int b = offsetFactor - a;

        byte_t temp = data_ptr[a];
        data_ptr[a] = data_ptr[b];
        data_ptr[b] = temp;
    }
}

void fprintbin(FILE* stream, const void* bin_data, bsize_t number_of_bits)
{
    byte_t* data_ptr = (byte_t *)bin_data;

    for (int j = 0; j < number_of_bits; j++)
    {
        if (PRINT_BYTE_SPACES && j % BYTE_LEN == 0 && j) fprintf(stream, " ");
        int offset = (number_of_bits - 1 - j) % BYTE_LEN;
        fprintf(stream, "%c", data_ptr[j / BYTE_LEN] & (0x1 << offset) ? '1' : '0');
    }
}

void printbin(const void* bin_data, bsize_t number_of_bits)
{
    fprintbin(stdout, bin_data, number_of_bits);
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

/* Copies the value of bit at bitfile to given offset of dst (Incrementing bit cursor) */
void bfgetbit(byte_t* dst, bsize_t offset, BITFILE* bitfile)
{
    byte_t dstbit = 0x1 << MSB_OFFSET(offset % BYTE_LEN,     bitfile->_flags);
    byte_t srcbit = 0x1 << MSB_OFFSET(bitfile->_bitoffset++, bitfile->_flags);

    /* Set dstbit to 1 or 0 to match srcbit */
    if (bitfile->_currbyte & srcbit) *dst |=  dstbit;
    else                             *dst &= ~dstbit;
}


/* Copies the value of bit at given offset of src to bitfile (Incrementing bit cursor) */
void bfputbit(byte_t src, bsize_t offset, BITFILE* bitfile)
{
    byte_t srcbit = 0x1 << MSB_OFFSET(offset % BYTE_LEN,     bitfile->_flags);
    byte_t dstbit = 0x1 << MSB_OFFSET(bitfile->_bitoffset++, bitfile->_flags);

    /* Set dstbit to 1 or 0 to match srcbit */
    if (src & srcbit) bitfile->_currbyte |=  dstbit;
    else              bitfile->_currbyte &= ~dstbit;
}

/* Set bitfile parameters to initial values (Doesn't modify _fileobj) */
void bfreset(BITFILE* bitfile, bool msb_first)
{
    bitfile->_currbyte = 0x0;
    bitfile->_bitoffset = BYTE_LEN; /* Force read on next operation */
    if (msb_first) bitfile->_flags |= BF_FLAG_MSB;
    else bitfile->_flags &= ~(uint8_t)BF_FLAG_MSB;
}

/* Return the current bit offset with while incrementing */
int8_t incBitOffset(BITFILE* bitfile)
{
    bitfile->_bitoffset += 1;
    if (bitfile->_flags & BF_FLAG_MSB) return BYTE_LEN - bitfile->_bitoffset;
    return bitfile->_bitoffset - 1;
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

/* Write byte in buffer to file
    - 0 on success
    - EOF on failure to write/inc
    - 1 on failure after write */
int writeByte(BITFILE* bitfile, bool inc)
{
    /* Write buffer to file */
    bitfile->_currbyte = putc(bitfile->_currbyte, bitfile->_fileobj);
    if (bfeof(bitfile)) return EOF;

    /* Read next byte into buffer */
    if (inc && getByte(bitfile))
    {
        clearerr(bitfile->_fileobj);
        bitfile->_currbyte = 0x0;
    }
    return bfeof(bitfile);
}

/* Align memory byte to bitoffset */
int alignByte(BITFILE* bitfile)
{
    if (bitfile->_bitoffset < 0)
    {
        if (errno == 0) errno = ESPIPE;
        return 2; /* Cannot read */
    }

    if (bfeof(bitfile)) return 1;

    while (bitfile->_bitoffset >= BYTE_LEN)
    {
        bitfile->_bitoffset -= BYTE_LEN;
        if (getByte(bitfile)) return 1; /* EOF */
    }
    return 0;
}

/* Store access mode w/ appended 'b' in byte_access
    - Expects: r,w,a,r+,w+,a+,rb,wb,ab,rb+,wb+,ab+,r+b,w+b,a+b
    - Success: return appropriate read/write flags
    - Invalid string: return 1 */
uint8_t copyByteAccessMode(const char* basic_access, char* byte_access)
{
    int len = strlen(basic_access);
    if (len < 1 || len > VALID_ACCESS_CHARS_OUTER_SZ)
    {
        errno = EINVAL;
        return BF_FLAG_ERR;
    }

    uint8_t flags = 0;
    bool has_b = false;
    for (int i = 0; i < len; i++)
    {
        if (!isIn(basic_access[i], valid_access_chars[i], VALID_ACCESS_CHARS_INNER_SZ))
        {
            errno = EINVAL;
            return BF_FLAG_ERR | flags;
        }

        switch (basic_access[i])
        {
            case '+':
                flags |= BF_FLAG_WRITE;
            case 'r':
                flags |= BF_FLAG_READ;
                break;

            case 'a':
            case 'w':
                flags |= BF_FLAG_WRITE;
                break;

            case 'b':
                has_b = true;
                break;
        }

        byte_access[i] = basic_access[i];
    }

    if (!has_b) byte_access[len++] = 'b';
    byte_access[len] = '\0';
    return flags;
}
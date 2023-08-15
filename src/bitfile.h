/*
READ BITS by bathtaters
*/

#ifndef READ_BITS_H
#define READ_BITS_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


/* -- LIBRARY OPTIONS -- */

/* Add spacing between bytes for printbin() */
#define PRINT_BYTE_SPACES true
/* Access mode for temp file */
#define TMP_FILE_ACCESS "wb+"

/* -- CONSTANTS & MACROS -- */

/* Max size of access_mode (including NUL) */
#define ACCESS_MODE_LEN 4
/* Length of 1 byte in bits */
#define BYTE_LEN 8
/* x / y rounded up to nearest whole number (Expects integer x & y) */
#define CEIL_DIV(x,y) (1 + (((x) - 1) / (y)))

/* Flag bit representing if file is readable */
#define BF_FLAG_READ  0x1
/* Flag bit representing if file is writable */
#define BF_FLAG_WRITE 0x2
/* Flag bit representing if bits are read from left to right */
#define BF_FLAG_MSB 0x4
/* Flag bit representing error */
#define BF_FLAG_ERR 0x80

/* -- DATA TYPES -- */

/* Type used to store raw byte */
typedef uint8_t byte_t;
/* Format string for raw byte */
#define BYTE_T_STR "d"

/* Size in bits of a bit file */
typedef uint64_t bsize_t;
/* Format string for size in bits of a bit file */
#define BSIZE_T_STR "llu"

/* Bit position within a bit file */
typedef int64_t bpos_t;
/* Format string for bit position within a bit file */
#define BPOS_T_STR "lld"

/* Full position within a file */
typedef struct bfpos_t {
    fpos_t byte;
    uint8_t bit;
} bfpos_t;

/* Data object for bitfile functions
   (DO NOT modify this directly!) */
typedef struct BITFILE {
    /* File object to read from */
    FILE* _fileobj;
    /* Current byte being read */
    int _currbyte;
    /* Current bit offset within this.byte */
    int8_t _bitoffset;
    /* Flag descriptors for bit file */
    uint8_t _flags;
} BITFILE;


/* -- OPEN/CLOSE FUNCTIONS -- */

/* Opens the file pointed to by filename using the given mode & bit order
    - Access Modes: r, w, a, r+, w+, a+
    - MSB_First = true: Most-Sig Bit First (Left-to-right)
    - MSB_First = false: Least-Sig Bit First (Right-to-left) */
BITFILE* bfopen(const char* filename, const char* access_mode, bool msb_first);
/* Flushes all buffers and closes the file */
int bfclose(BITFILE* bitfile);
/* Associates a new filename with the given bitfile while closing the old file in stream. */
BITFILE* bfreopen(const char* filename, const char* access_mode, bool msb_first, BITFILE *bitfile);
/* Creates a temporary file in update mode (wb+).
    - nametemplate must end with "XXXXXX" */
BITFILE* tmpbitfile(char* nametemplate, bool msb_first);


/* --- READ/WRITE FUNCTIONS --- */

/* Reads data from the given 'bitfile' into the array pointed to by 'ptr'
    - ptr must be able to store the number_of_bits */
bsize_t bfread(void* ptr, bsize_t number_of_bits, BITFILE* bitfile);
/* Writes data from the array pointed to by 'ptr' to the given 'bitfile.' */
bsize_t bfwrite(void* ptr, bsize_t number_of_bits, BITFILE* bitfile);
/* Flushes output buffer of the bitfile to file. */
int bfflush(BITFILE* bitfile);
/* Define how the bitfile should be buffered.
    - buffer should be array of size 'size' (or NULL to use internal buffer)
    - mode should be _IOFBF (Full buffer) or _IONBF (No buffer) */
int setbfbuf(BITFILE* bitfile, char* buffer, int mode, size_t size);

/* --- POSITION FUNCTIONS --- */

/* Sets the file position of the stream to the offsets from the whence position
    - Accepts negative offsets and bit_offset > 8
    - Whence: SEEK_CUR, SEEK_SET, SEEK_END */
int bfseek(BITFILE* bitfile, bpos_t offset, int whence);
/* Returns the current bit position of the given bit file. */
bpos_t bftell(BITFILE *bitfile);
/* Sets the position to the beginning of the bit file. */
void bfrewind(BITFILE *bitfile);
/* Gets the current position of the bit file and writes it to pos. */
int bfgetpos(BITFILE *bitfile, bfpos_t* pos);
/* Sets the file position of the given bit file to the given position. */
int bfsetpos(BITFILE *bitfile, const bfpos_t* pos);

/* --- ERROR FUNCTIONS --- */

/* Clears the end-of-file and error indicators for the given bitfile. */
void clearbferr(BITFILE* bitfile);
/* Returns non-zero if the error indicator is set for the given bitfile. */
int bferror(BITFILE* bitfile);
/* Returns non-zero if the end-of-file indicator is set for the given bitfile. */
int bfeof(BITFILE* bitfile);

/* -- UTILITY FUNCTIONS -- */

/* Swap endianess of bin_data of length number_of_bits */
void swapendian(void* bin_data, bsize_t number_of_bits);
/* Print binary value of bin_data of length number_of_bits to given stream */
void fprintbin(FILE* stream, const void* bin_data, bsize_t number_of_bits);
/* Print binary value of bin_data of length number_of_bits */
void printbin(const void* bin_data, bsize_t number_of_bits);


#endif
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


/* -- LIBRARY OPTIONS -- */

/* Add spacing between bytes for printbin() */
#define PRINT_BYTE_SPACES true

/* -- CONSTANTS & MACROS -- */

/* Max size of access_mode (including NUL) */
#define ACCESS_MODE_LEN 4
/* Length of 1 byte in bits */
#define BYTE_LEN 8
/* x / y rounded up to nearest whole number (Expects integer x & y) */
#define CEIL_DIV(x,y) (1 + (((x) - 1) / (y)))

/* -- DATA TYPES -- */

/* Type used to store raw byte */
typedef uint8_t byte_t;

/* Data object for bitfile functions
   (DO NOT modify this directly!) */
typedef struct BITFILE {
    /* File object to read from */
    FILE* _fileobj;
    /* Current byte being read */
    int _currbyte;
    /* Current bit offset within this.byte */
    int8_t _bitoffset;
    /* Read starting from left (true) or right (false) */
    bool _msb;
} BITFILE;


/* -- OPEN/CLOSE FUNCTIONS -- */

/* Opens the file pointed to by filename using the given mode & bit order
    - Access Modes: r, w, a, r+, w+, a+
    - MSB_First = true: Most-Sig Bit First (Left-to-right)
    - MSB_First = false: Least-Sig Bit First (Right-to-left) */
BITFILE* bfopen(const char* filename, const char* access_mode, bool msb_first);
/* Flushes all buffers and closes the file */
int bfclose(BITFILE* bitfile);

/* --- READ/WRITE FUNCTIONS --- */

/* Reads data from the given file into the array pointed to by ptr
    - save_to_ptr size must be at least bitCount/8 */
uint64_t bfread(void* save_to_ptr, uint64_t number_of_bits, BITFILE* bitfile);

/* --- POSITION FUNCTIONS --- */

/* Sets the file position of the stream to the offsets from the whence position
    - Accepts negative offsets and bit_offset > 8
    - Whence: SEEK_CUR, SEEK_SET, SEEK_END */
int bfseek(BITFILE* bitfile, long int byte_offset, int64_t bit_offset, int whence);

/* -- UTILITY FUNCTIONS -- */

/* Swap endianess of bin_data of length number_of_bits */
void swapendian(void* bin_data, uint64_t number_of_bits);
/* Print binary value of bin_data of length number_of_bits */
void printbin(const void* bin_data, uint64_t number_of_bits);


#endif
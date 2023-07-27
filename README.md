# bitfile v1.0
C Library to help read & write files at the bit level.
Extends the binary file read/write operations

---

## Functions

### *BITFILE\** **bfopen**(filename, access_mode, msb_first)
Opens the file pointed to by filename using the given mode & bit order.
#### Parameters
 - ***const char\**** **filename**: name of the file when present in the same directory as the source file. Otherwise, full path.
 - ***const char\**** **access_mode**: Specifies for what operation the file is being opened *(Accepted: r,w,a,r+,w+,a+)*.
 - ***bool*** **msb_first**:
   - **True** Read/write bits from left to right (Most significant bit first).
   - **False** Read/write bits right to left (Least significant bit first).
#### Return Value
 - If the file is opened successfully, returns a pointer to it.
 - If the file is not opened, then returns NULL.

---

### *int* **bfclose**(bitfile)
Flushes all buffers and closes the file.
#### Parameters
 - ***BITFILE\**** **bitfile**: pointer to file returned from bfopen.
#### Return Value
 - If the file is closed successfully, returns 0.
 - If there is an error closing the file, returns EOF.

---

### *uint64_t* **bfread**(save_to_ptr, number_of_bits, bitfile)
Reads data from the given file into the array pointed to by ptr (Array size must be at least bitCount/8).
#### Parameters
 - ***void\**** **save_to_ptr**: Pointer to block of memory to store read bits.
 - ***uint64_t*** **number_of_bits**: The number of bits to read.
 - ***BITFILE\**** **bitfile**: Pointer to the *BITFILE* containing the input stream.
#### Return Value
 - The number of bits successfully read from **bitfile**. This should equal **number_of_bits** unless an error or EOF was encountered.

---

### *int* **bfseek**(bitfile, byte_offset, bit_offset, whence)
Sets the file position of the stream to the offsets from the whence position (Accepts negative offsets and bit_offset > 8).
#### Parameters
 - *BITFILE\** **bitfile** - Pointer to the *BITFILE* to seek within.
 - *long int* **byteOffset** - Byte offset from **whence** to seek to.
 - *uint64_t* **bitOffset** - Bit offset within byte to seek to.
 - *int* **whence**:
    - ***SEEK_CUR*** Start from current byte/bit position.
    - ***SEEK_SET*** Start from start of file.
    - ***SEEK_END*** Start from end of file (Expects negative offsets).
#### Return Code
 -  **0**: Success
 -  **Other**: Failed to seek to requested position

---

### *void* **swapendian**(**bin_data**, **number_of_bits**)
Swap endianess of **bin_data** of length **number_of_bits**.
#### Parameters
 - ***void\**** **bin_data**: Pointer to block of binary memory to modify.
 - ***uint64_t*** **number_of_bits**: The size in bits of bin_data.

---

### *void* **printbin**(**bin_data**, **number_of_bits**)
Print binary value of **bin_data** of length **number_of_bits**.
 - ***void\**** **bin_data**: Pointer to block of binary memory to print.
 - ***uint64_t*** **number_of_bits**: The size in bits of bin_data.

---

## Constants & Macros

 - **CEIL_DIV**(*int* ***x***, *int* ***y***): Result of *x* / *y* rounded up to the nearest *int*.
 - **PRINT_BYTE_SPACES**: True will add spaces between bytes when printing binary w/ printbin
 - **BYTE_LEN**: Length of 1 byte in bits (8).
 - **ACCESS_MODE_LEN**: Max size of file access_mode (+ 1 for terminating char)

---

## Data Types

### *struct* **BITFILE**
Object representing bit stream
(Should not be modified directly!)

|Type|Name|Description|
|--|--|--|
|***FILE***|**_fileobj**|File object to read from|
|***int***|**_curbyte**|Current byte being read (or EOF)|
|***int8_t***|**_bitoffset**|Offset of current bit within byte|
|***bool***|**_msb**|True if bits will be read left-to-right|

### **byte_t**
Type used to store raw bytes

---

## For developers

Compile & run tests:

```bash
make
make clean # Optional: Remove temp files
./bitfile-test
```

---

## To Do

### Add Bit-Version of Functions

 -	size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
Writes data from the array pointed to by ptr to the given stream.

 -	void clearerr(FILE *stream)
Clears the end-of-file and error indicators for the given stream.

 -	int feof(FILE *stream)
Tests the end-of-file indicator for the given stream.

 -	int ferror(FILE *stream)
Tests the error indicator for the given stream.

 -	int fflush(FILE *stream)
Flushes the output buffer of a stream.

 -	int fgetpos(FILE *stream, fpos_t *pos)
Gets the current file position of the stream and writes it to pos.

 -	FILE *freopen(const char *filename, const char *mode, FILE *stream)
Associates a new filename with the given open stream and same time closing the old file in stream.

 -	int fsetpos(FILE *stream, const fpos_t *pos)
Sets the file position of the given stream to the given position. The argument pos is a position given by the function fgetpos.

 -	long int ftell(FILE *stream)
Returns the current file position of the given stream.

 -	void rewind(FILE *stream)
Sets the file position to the beginning of the file of the given stream.

 -	void setbuf(FILE *stream, char *buffer)
Defines how a stream should be buffered.

 -	int setvbuf(FILE *stream, char *buffer, int mode, size_t size)
Another function to define how a stream should be buffered.

 -	FILE *tmpfile(void)
Creates a temporary file in binary update mode (wb+).

---
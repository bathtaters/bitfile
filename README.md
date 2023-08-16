# bitfile v0.5
C Library to help read & write files at the bit level.
Extends the binary file read/write operations

For simple usage, copy `bitfile.c` & `bitfile.h` from the `src` directory into your project.

---

## Documentation

 1. [Open/Close Functions](#openclose-functions)
    - [`bfopen`](#bitfile-bfopenfilename-access_mode-msb_first)
    - [`bfclose`](#int-bfclosebitfile)
    - [`bfreopen`](#bitfile-bfreopenfilename-access_mode-msb_first-bitfile)
    - [`tmpbitfile`](#bitfile-tmpbitfilenametemplate-msb_first)

 1. [Read/Write Functions](#readwrite-functions)
    - [`bfread`](#bsize_t-bfreadptr-number_of_bits-bitfile)
    - [`bfwrite`](#bsize_t-bfwriteptr-number_of_bits-bitfile)
    - [`bfflush`](#int-bfflushbitfile)
    - [`setbfbuf`](#int-setbfbufbitfile-buffer-mode-size)

 1. [Position Functions](#position-functions)
    - [`bfseek`](#int-bfseekbitfile-offset-whence)
    - [`bftell`](#bpos_t-bftellbitfile)
    - [`bfrewind`](#void-bfrewindbitfile)
    - [`bfgetpos`](#int-bfgetposbitfile-pos)
    - [`bfsetpos`](#int-bfsetposbitfile-bitfile-const-bfpos_t-pos)

 1. [Error Functions](#error-functions)
    - [`clearbferr`](#void-clearbferrbitfile)
    - [`bferror`](#int-bferrorbitfile)
    - [`bfeof`](#int-bfeofbitfile)

 1. [Utility Functions](#utility-functions)
    - [`swapendian`](#void-swapendianbin_data-number_of_bits)
    - [`printbin`](#void-printbinbin_data-number_of_bits)
    - [`fprintbin`](#void-fprintbinf-bin_data-number_of_bits)

 1. [Constants & Macros](#constants--macros)

 1. [Data Types](#data-types)
    - [`BITFILE`](#struct-bitfile)
    - [`bfpos_t`](#struct-bfpos_t)
    - [`bsize_t`](#bsize_t)
    - [`bpos_t`](#bpos_t)
    - [`byte_t`](#byte_t)

 1. [For Developers](#for-developers)

---

## Open/Close Functions

### *BITFILE\** **bfopen**(filename, access_mode, msb_first)
Opens the file pointed to by filename using the given mode & bit order.
#### Parameters
 - ***const char\**** **filename**: Path/name of file.
 - ***const char\**** **access_mode**: Specifies for what operation the file is being opened *(Accepted: r,w,a,r+,w+,a+)*.
 - ***bool*** **msb_first**:
   - **True** Read/write bits from left to right (Most significant bit first).
   - **False** Read/write bits right to left (Least significant bit first).
#### Return Value
 - ***Pointer to BITFILE struct***: File was opened successfully.
 - **NULL**: File was unable to be opened.


### *int* **bfclose**(bitfile)
Flushes all buffers and closes the file.
#### Parameters
 - ***BITFILE\**** **bitfile**: pointer to file returned from bfopen.
#### Return Code
 - **0**: File was closed successfully.
 - **EOF**: Error occured while closing the file.


### BITFILE* bfreopen(filename, access_mode, msb_first, bitfile);
Associates a new filename with the given bitfile while closing the old file in stream.
#### Parameters
 - ***const char\**** **filename**: Path/name of new file.
 - ***const char\**** **access_mode**: Specifies for what operation the file is being opened *(Accepted: r,w,a,r+,w+,a+)*.
 - ***bool*** **msb_first**:
   - **True** Read/write bits from left to right (Most significant bit first).
   - **False** Read/write bits right to left (Least significant bit first).
 - ***BITFILE\**** **bitfile**: Pointer to exisiting (non-closed) *BITFILE*.
#### Return Value
 - ***Pointer to BITFILE struct***: New file was opened successfully.
 - **NULL**: New file was unable to be opened.


### BITFILE* tmpbitfile(nametemplate, msb_first);
Creates a temporary file in update mode (wb+).
#### Parameters
 - ***char\**** **nametemplate**: Array to set filename, last 6 characters must be "XXXXXX".
 - ***bool*** **msb_first**: True = prefer reading left-to-right, False = right-to-left.
#### Return Value
 - ***Pointer to BITFILE struct***: Temp file was created successfully.
 - **NULL**: Temp file was unable to be created.

<div align="right"><h6><a href="#documentation">Index ^</a></h6></div>

---

## Read/Write Functions

### *bsize_t* **bfread**(ptr, number_of_bits, bitfile)
Reads data from the given **bitfile** into the array pointed to by **ptr** (Must be able to store the number_of_bits).
#### Parameters
 - ***void\**** **ptr**: Pointer to block of memory to store read bits.
 - ***uint64_t*** **number_of_bits**: The number of bits to read.
 - ***BITFILE\**** **bitfile**: Pointer to the *BITFILE* containing the input stream.
#### Return Value
 - The number of bits successfully read from **bitfile**.
 - This should equal **number_of_bits** unless an error was encountered.


### *bsize_t* **bfwrite**(ptr, number_of_bits, bitfile)
Writes data from the array pointed to by **ptr** to the given **bitfile**.
#### Parameters
 - ***void\**** **ptr**: Pointer to block of memory to write to file.
 - ***uint64_t*** **number_of_bits**: The number of bits to write.
 - ***BITFILE\**** **bitfile**: Pointer to the *BITFILE* containing the output stream.
#### Return Value
 - The number of bits successfully written to **bitfile**.
 - This should equal **number_of_bits** unless an error was encountered.


### *int* **bfflush**(bitfile)
Flushes output buffer of the **bitfile** to file
#### Parameters
 - ***BITFILE\**** **bitfile**: Pointer to the *BITFILE* containing the output stream.
#### Return Code
 -  **0**: Success.
 -  ***Non-zero***: Failed to flush buffer.


### *int* **setbfbuf**(bitfile, buffer, mode, size);
Define how the **bitfile** should be buffered.
#### Parameters
 - ***BITFILE\**** **bitfile**: Pointer to the *BITFILE* to modify.
 - ***char\**** **buffer**: Pointer to new buffer, or NULL to use internal buffer.
 - ***int*** **mode**: Buffer type (*_IOFBF* for full buffer or *_IONBF* for no buffer).
 - ***size_t*** **size**: Size of buffer (Can use *BUFSIZ* for recommended size).
#### Return Code
 -  **0**: Success.
 -  ***Non-zero***: Failed to seek to requested position.

<div align="right"><h6><a href="#documentation">Index ^</a></h6></div>

---

## Position Functions

### *int* **bfseek**(bitfile, offset, whence)
Sets the file position of the stream to the offsets from the whence position (Accepts negative offsets and bit_offset > 8).
#### Parameters
 - ***BITFILE\**** **bitfile**: Pointer to the *BITFILE*.
 - ***bpos_t*** **offset**: Bit offset from **whence** to seek to.
 - ***int*** **whence**:
    - **SEEK_CUR** Start from current byte/bit position.
    - **SEEK_SET** Start from start of file.
    - **SEEK_END** Start from end of file (Expects negative offsets).
#### Return Code
 -  **0**: Success.
 -  ***Non-zero***: Failed to seek to requested position.


### *bpos_t* **bftell**(bitfile);
Returns the current bit position of the given bit file.
#### Parameters
 - ***BITFILE\**** **bitfile**: Pointer to the *BITFILE*.
#### Return Value
 - Current offset in bits from the start of the given bit file.


### *void* **bfrewind**(bitfile);
Sets the position to the beginning of the bit file.
#### Parameters
 - ***BITFILE\**** **bitfile**: Pointer to the *BITFILE*.


### *int* **bfgetpos**(bitfile, pos);
Gets the current position of the bit file and writes it to pos.
#### Parameters
 - ***BITFILE\**** **bitfile**: Pointer to the *BITFILE*.
 - ***bfpos_t\**** **pos**: Pointer to position object to set to current bitfile position.
#### Return Code
 - **0**: Success.
 - ***Non-zero***: Error.


### *int* **bfsetpos**(BITFILE *bitfile, const bfpos_t *pos)
Sets the file position of the given bit file to the given position.
(Use with output of bfgetpos)
#### Parameters
 - ***BITFILE\**** **bitfile**: Pointer to the *BITFILE*.
 - ***const bfpos_t\**** **pos**: Pointer to position object to update current bitfile to.
#### Return Code
 - **0**: Success.
 - ***Non-zero***: Error.

<div align="right"><h6><a href="#documentation">Index ^</a></h6></div>

---

## Error Functions

### *void* **clearbferr**(bitfile);
Clears the end-of-file and error indicators for the given **bitfile**.
#### Parameters
 - ***BITFILE\**** **bitfile**: Pointer to the *BITFILE*.


### *int* **bferror**(bitfile);
Returns non-zero if the error indicator is set for the given **bitfile**.
Check errno for error code or perror for error description.
#### Parameters
 - ***BITFILE\**** **bitfile**: Pointer to the *BITFILE*.
#### Return Code
 -  **0**: Success.
 -  ***Non-zero***: Error exists on *BITFILE*.


### *int* **bfeof**(bitfile);
Returns non-zero if the end-of-file indicator is set for the given **bitfile**.
#### Parameters
 - ***BITFILE\**** **bitfile**: Pointer to the *BITFILE*.
#### Return Code
 -  **0**: Success.
 -  ***Non-zero***: *BITFILE* has reached end of file.

<div align="right"><h6><a href="#documentation">Index ^</a></h6></div>

---

## Utility Functions

### *void* **swapendian**(**bin_data**, **number_of_bits**)
Swap endianess of **bin_data** of length **number_of_bits**.
#### Parameters
 - ***void\**** **bin_data**: Pointer to block of binary memory to modify.
 - ***uint64_t*** **number_of_bits**: The size in bits of bin_data.


### *void* **printbin**(**bin_data**, **number_of_bits**)
Print binary value of **bin_data** of length **number_of_bits**.
 - ***void\**** **bin_data**: Pointer to block of binary memory to print.
 - ***uint64_t*** **number_of_bits**: The size in bits of bin_data.

### *void* **fprintbin**(**stream**, **bin_data**, **number_of_bits**)
Print binary value of **bin_data** of length **number_of_bits** to given **stream**.
 - ***FILE\**** **stream**: Stream to print to (ie. stdout).
 - ***void\**** **bin_data**: Pointer to block of binary memory to print.
 - ***uint64_t*** **number_of_bits**: The size in bits of bin_data.

<div align="right"><h6><a href="#documentation">Index ^</a></h6></div>

---

## Constants & Macros

 - **CEIL_DIV**(*int* ***x***, *int* ***y***): Result of *x* / *y* rounded up to the nearest *int*.
 - **PRINT_BYTE_SPACES**: True will add spaces between bytes when printing binary w/ printbin.
 - **BYTE_LEN**: Length of 1 byte in bits (8).
 - **ACCESS_MODE_LEN**: Max size of file access_mode (+ 1 for terminating char).
 - **TMP_FILE_ACCESS**: Access mode for temp file (*"wb+"*).
 - **BF_FLAG_READ**: Flag bit representing if file is readable
 - **BF_FLAG_WRITE**: Flag bit representing if file is writable
 - **BF_FLAG_MSB**: Flag bit representing if bits are read from left to right
 - **BF_FLAG_ERR**: Flag bit representing error

<div align="right"><h6><a href="#documentation">Index ^</a></h6></div>

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
|***uint8_t***|**_flags**|Flags describing bit stream|

### *struct* **bfpos_t**
Bit cursor position within a file

|Type|Name|Description|
|--|--|--|
|***fpos_t***|**byte**|Byte offset from start of file|
|***bpos_t***|**bit**|Bit offset from start of byte|

### **bsize_t**
Size in bits of a bit file
 - Format String: "%"**BSIZE_T_STR**

### **bpos_t**
Bit position within a bit file
 - Format String: "%"**BPOS_T_STR**

### **byte_t**
Type used to store raw bytes
 - Format String: "%"**BYTE_T_STR**

<div align="right"><h6><a href="#documentation">Index ^</a></h6></div>

---

## For Developers

Compile & run tests:

```bash
make
make clean      # Optional: Remove temp files
./bitfile-test
./testclean.sh  # Optional: Clean up temp test files
```

<div align="right"><h6><a href="#documentation">Index ^</a></h6></div>

---

## To Do

#### Add testing for...

- [ ] bferror
- [ ] bfeof
- [ ] clearbferr

<div align="right"><h6><a href="#documentation">Index ^</a></h6></div>

---

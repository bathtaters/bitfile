# readbits v1.0
C Library to help read files bit-by-bit.

To use, copy src/readbits.h & src/readbits.c into your project.

---

## Data Types

### *struct* **BitReader**
|Type|Name|Description|
|--|--|--|
|***FILE***|**file**|File object to read from|
|***uint8_t***|**byte**|Current byte being read|
|***int***|**bitOffset**|Current bit bitOffset within byte|
|***bool***|**canRead**|True if data is available to be read from file|
|***bool***|**msbFirst**|True if bits will be read left-to-right|

---

## Functions

---
### *BitReader* **newBitReader**(filename, msbFirst)
Creates and returns a new BitReader struct
 - *char\** **filename** - Name of file to read bits from.
 - *bool* **msbFirst** - If false start reading from least-significant bit (right), otherwise read from most-significant (left)
##### If there is an error, bitReader.canRead will be set to false.

---
### *void* **freeBitReader**(bitReader)
Close file & free any memory allocated by **bitReader** struct.
 - *BitReader* **bitReader** - Object to free.
##### Must be called to avoid memory leak.

---
### *uint8_t\** **getBits**(bitReader, bitCount)
Return the next **bitCount** bits as an array of *uint8*s.
 - *BitReader\** **bitReader** - Object to retrieve bits from.
 - *int* **bitCount** - Number of bits to retrieve.
#### NOTE: Return pointer must be free'd.
##### Size of return array will always be bitCount / 8 rounded up.
##### Endianess of array will reflect endianess of file.
##### If **bitReader.canRead** is false, will return array of 0s and display a message in stderr.
##### If end of file reached, will pad out remainder of array with 0s and display a message in stderr.

---
### *int* **seekBits**(bitReader, byteOffset, bitOffset, whence)
Seek **bitReader** to the given **byteOffset** and **bitOffest** in the file, returning a status code.
 - *BitReader\** **bitReader** - Object to seek within.
 - *long int* **byteOffset** - Byte offset from **whence** to seek to.
 - *int* **bitOffset** - Bit offset within byte to seek to.
 - *int* **whence** - Must be equal to one of:
    - ***SEEK_CUR***: Offset from current byte/bit position.
    - ***SEEK_SET***: Offset from start of file.
    - ***SEEK_END***: Offset from end of file (Expects negative offsets).
 - ***Return Code***
    - **0** – Success
    - **1** – EOF reached
    - **-1** – File not open
##### If end of file reached/passed, will set **bitReader.canRead** = 0 and display a message.
##### Accepts negative offsets, and will carry to bytes if bitOffset > 8
---

## Misc. Functions

Also includes:
### *void* **swapbytes**(*uint8_t\** **bindata**, *int* **bitWidth**)
Utility to swap endianess of array (**bindata**) based on its **bitWidth**.

### *void* **printbin**(*uint8_t\** **bindata**, *int* **bitWidth**)
Utility to print **bindata** as binary number **bitWidth** digits long.

---
### For developers

Compile & run tests:

```bash
make && make clean && ./readbits-test
```
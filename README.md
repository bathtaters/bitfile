# readbits
C Library to help read files bit-by-bit.

To use, copy src/readbits.h & src/readbits.c into your project.

---

## Data Types

---
### *struct* **BitReader**
|Type|Name|Description|
|--|--|--|
|***FILE***|**file**|File object to read from|
|***uint8_t***|**byte**|Current byte being read|
|***int***|**bitOffset**|Current bit bitOffset within byte|
|***char***|**canRead**|Non-zero if data is able to be read|

---

## Functions

---
### *BitReader\** **newBitReader**(filename)
Creates and returns a pointer to a new BitReader object
 - *char\** **filename** - Name of file to read bits from.
##### It is recommended you test bitReader.canRead == 1 before using.

---
### *void* **freeBitReader**(bitReader)
Close file & free **bitReader** memory.
 - *BitReader\** **bitReader** - Object to free.
##### ***Must be called to avoid memory leak.***

---
### *uint64_t* **getBits**(bitReader, bitCount)
Return the next **bitCount** bits as a *uint64*.
 - *BitReader\** **bitReader** - Object to retrieve bits from.
 - *char* **bitCount** - Number of bits to retrieve.
##### If **bitCount** is > 64, only last 64 bits will be returned and a warning displayed.
##### If **bitReader.canRead** == 0, will return 0 and display a message.
##### If end of file reached, will return current result and display a message.

---
### *BitReader\** **seekBits**(bitReader, byteOffset, bitOffset, whence)
Seek **bitReader** to the given **byteOffset** and **bitOffest** in the file.
 - *BitReader\** **bitReader** - Object to seek within.
 - *long int* **byteOffset** - Byte offset from **whence** to seek to.
 - *int* **bitOffset** - Bit offset within byte to seek to.
 - *int* **whence** - Must be equal to one of:
    - ***SEEK_CUR***: Offset from current byte/bit position.
    - ***SEEK_SET***: Offset from start of file.
    - ***SEEK_END***: Offset from end of file (Expects negative offsets).
##### If end of file reached/passed, will set **bitReader.canRead** = 0 and display a message.
##### Accepts negative offsets, and allows > 8 for bitOffset
---

## Misc.

Also includes:
### *void* **printbin**(*uint64_t* **bindata**, *char* **bitWidth**)
Utility to print **bindata** as binary number **bitWidth** digits long.

---
### For developers

Compile & run tests:

```bash
make && make clean && ./readbits-test
```
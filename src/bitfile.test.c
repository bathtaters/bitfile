/* TESTs for BITFILE */

#include "bitfile.h"

/* Print all test details */
#define VERBOSE false

/* Test files */
#define TEST_FILE_R "test.txt"
#define TEST_FILE_W "write.txt"

byte_t testtext[] = { 't', 'u', 'v', 'w', 'x', 'y', 'z', 'a', '\2' };

int testCount = 1;

int  readTest(const char* test, const char* filename, bool msbFirst, bsize_t counts[], int size, int width, byte_t expected[size][width]);
int writeTest(const char* test, const char* filename, bool msbFirst, bsize_t counts[], int size, int width, byte_t data[size][width]);
int checkRead(char* name, BITFILE *bf, bsize_t bitcount, byte_t* expected);
int checkPosition(const char* name, BITFILE* bitfile, fpos_t expectedByte, uint8_t expectedBit, int priorReturnVal);
int printTest(byte_t* bin, bsize_t bitcount, const char* expected);
int swapTest(int size, byte_t* expected);
int freeprint();
int expectError(int code);


/* Run all tests */
int main()
{
    /* READ TESTS */

    bsize_t a[] = {8,8,8,8};
    byte_t arm[][1] = {{testtext[0]}, {testtext[1]}, {testtext[2]}, {testtext[3]}};
    byte_t arl[][1] = {{testtext[0]}, {testtext[1]}, {testtext[2]}, {testtext[3]}};
    if (readTest("Byte", TEST_FILE_R, false, a, 4, 1, arl)) return 1;
    if (readTest("Byte", TEST_FILE_R,  true, a, 4, 1, arm)) return 1;

    bsize_t b[] = {6,4,3,6,3,4,5,1};
    byte_t brl[][1] = {{52},{5},{5},{51},{6},{13},{29},{0}};
    byte_t brm[][1] = {{29},{1},{6},{43},{5},{ 9},{27},{1}};
    if (readTest("Partial Byte", TEST_FILE_R, false, b, 8, 1, brl)) return 1;
    if (readTest("Partial Byte", TEST_FILE_R,  true, b, 8, 1, brm)) return 1;

    bsize_t c[] = {12,17,3};
    byte_t crl[][3] = {{116, 5, 0}, {103, 119, 1}, {3, 0, 0}};
    byte_t crm[][3] = {{116, 7, 0}, { 87, 103, 0}, {7, 0, 0}};
    if (readTest("Multi-Byte", TEST_FILE_R, false, c, 3, 3, crl)) return 1;
    if (readTest("Multi-Byte", TEST_FILE_R,  true, c, 3, 3, crm)) return 1;

    bsize_t e[] = {66};
    byte_t erl[][9] = {{ testtext[0], testtext[1], testtext[2], testtext[3], 0, 0, 0, 0, 0 }};
    if (readTest(">64 bit", TEST_FILE_R, false, e, 1, 9, erl)) return 1;
    if (VERBOSE)
    {
        printf("    w/ Error: '01 [bits: 32/66]'\n");
    }
    else
    {
        printf("    w/ bit-count mismatch 32 of 66.\n");
    }

    bsize_t f[] = {8,8,64,2};
    byte_t frm[][8] = {{testtext[0]}, {testtext[1]}, {testtext[2], testtext[3], 0, 0, 0, 0, 0, 0}, {0}};
    if (readTest("Read After EOF", TEST_FILE_R, true, f, 4, 8, frm)) return 1;
    if (VERBOSE)
    {
        printf("    w/ Errors: '03 [bits: 16/64]'\n");
        printf("               '04 [bits: 00/02]'\n");
    }
    else
    {
        printf("    w/ bit-count mismatches 16 of 64, 0 of 2.\n");
    }

    bsize_t g[] = {8,8};
    byte_t grl[][1] = {{0},{0}};
    if (readTest("Missing File", "cancel.txt", false, g, 2, 1, grl) != -1) return 1;
    if (expectError(2)) return 1;


    /* WRITE TESTS */

    bsize_t h[] = {8,8,8,8};
    byte_t hrm[][1] = {{testtext[0]}, {testtext[1]}, {testtext[2]}, {testtext[3]}};
    byte_t hrl[][1] = {{testtext[0]}, {testtext[1]}, {testtext[2]}, {testtext[3]}};
    if (writeTest("Byte", TEST_FILE_W, false, h, 4, 1, hrl)) return 1;
    if (writeTest("Byte", TEST_FILE_W,  true, h, 4, 1, hrm)) return 1;

    bsize_t i[] = {6,4,3,6,3,4,5,1};
    byte_t irl[][1] = {{52},{5},{5},{51},{6},{13},{29},{0}};
    byte_t irm[][1] = {{29},{1},{6},{43},{5},{ 9},{27},{1}};
    if (writeTest("Partial Byte", TEST_FILE_W, false, i, 8, 1, irl)) return 1;
    if (writeTest("Partial Byte", TEST_FILE_W,  true, i, 8, 1, irm)) return 1;

    bsize_t j[] = {12,17,3};
    byte_t jrl[][3] = {{116, 5, 0}, {103, 119, 1}, {3, 0, 0}};
    byte_t jrm[][3] = {{116, 7, 0}, { 87, 103, 0}, {7, 0, 0}};
    if (writeTest("Multi-Byte", TEST_FILE_W, false, j, 3, 3, jrl)) return 1;
    if (writeTest("Multi-Byte", TEST_FILE_W,  true, j, 3, 3, jrm)) return 1;

    bsize_t k[] = {66};
    byte_t krl[][9] = {{ testtext[0], testtext[1], testtext[2], testtext[3], testtext[4], testtext[5], testtext[6], testtext[7], testtext[8] }};
    byte_t krm[][9] = {{ testtext[0], testtext[1], testtext[2], testtext[3], testtext[4], testtext[5], testtext[6], testtext[7], testtext[8] }};
    if (writeTest(">64 bit", TEST_FILE_W, false, k, 1, 9, krl)) return 1;
    if (writeTest(">64 bit", TEST_FILE_W,  true, k, 1, 9, krm)) return 1;


    /* FILE OPS */

    printf("%02d) File Operations tests\n", testCount++);

    BITFILE* bf = bfopen(TEST_FILE_R, "r", false);
    if (checkRead("- File Open", bf, 8, testtext)) return 1;
    else if (VERBOSE) printf("    SUCCESS: File Open subtest.\n");

    int res = bfclose(bf);
    if (res || VERBOSE) printf("  - File Close subtest.\n");
    if (res)
    {
        printf("  FAILED: Failed closing test [%d]\n", res ? res : errno);
        return 1;
    }
    else if (VERBOSE) printf("    SUCCESS: File Close subtest.\n");

    char name[15] = ".testXXXXXX";
    char namesuff[25];
    bf = tmpbitfile(name, false);
    if (VERBOSE) printf("  - Temp file subtest.\n");
    if (sscanf(name, ".test%s", &namesuff[0]) != 1 || namesuff[6] != '\0')
    {
        printf("  FAILED: Temp file test failed to create temp name '%s'.\n", name);
        return 1;
    }
    res = (int)bfwrite(&testtext[0], 8, bf);
    if (res != 8)
    {
        printf("  FAILED: To write to temp file: %d of %d bits", res, 8);
        perror("");
        return 1;
    }
    else if (VERBOSE) printf("    SUCCESS: Temp file subtest.\n");

    if (VERBOSE) printf("  - Re-open file subtest.\n");
    bf = bfreopen(TEST_FILE_R, "r", false, bf);
    remove(name);
    if (checkRead("- File Re-Open", bf, 8, &testtext[0])) return 1;
    else if (VERBOSE) printf("    SUCCESS: Re-open file subtest.\n");

    res = bfclose(bf);
    if (res)
    {
        printf("  FAILED: Failed closing re-opened file [%d]", res);
        perror("");
        return 1;
    }
    printf("  SUCCESS: File Op subtests passed.\n");


    /* FILE POSITION */

    printf("%02d) Position tests\n", testCount++);

    bf = bfopen(TEST_FILE_R, "r", false);

    if (VERBOSE) printf("  - bftell subtest.\n");
    bpos_t offset = bftell(bf);
    bpos_t exp = 0;
    if (offset != exp)
    {
        printf("    Initial    -- EXPECTED: %03"BPOS_STR",  FAILED: %03"BPOS_STR"\n",exp,offset);
        return 1;
    }
    else if (VERBOSE) printf("    Initial    -- EXPECTED: %03"BPOS_STR", SUCCESS: %03"BPOS_STR"\n",exp,offset);

    bfseek(bf, 12, SEEK_CUR);
    offset = bftell(bf);
    exp = 12;
    if (offset != exp)
    {
        printf("    Offset     -- EXPECTED: %03"BPOS_STR",  FAILED: %03"BPOS_STR"\n",exp,offset);
        return 1;
    }
    else if (VERBOSE) printf("    Offset     -- EXPECTED: %03"BPOS_STR", SUCCESS: %03"BPOS_STR"\n",exp,offset);


    if (VERBOSE) printf("  - bfgetpos subtest.\n");
    if (checkPosition("Offset: 12", bf, 1, 4, 0)) return 1;
    res = bfseek(bf, 6, SEEK_CUR);
    if (checkPosition("Offset: 18", bf, 2, 2, res)) return 1;


    if (VERBOSE) printf("  - bfsetpos subtest.\n");
    bfpos_t pos = {0, 3};
    res = bfsetpos(bf, &pos);
    if (checkPosition("Bit only", bf, 0, 3, res)) return 1;

    pos.byte = 3;
    pos.bit = 6;
    res = bfsetpos(bf, &pos);
    if (checkPosition("Bit+Byte", bf, 3, 6, res)) return 1;

    pos.byte = 12;
    pos.bit = 23;
    res = bfsetpos(bf, &pos);
    if (checkPosition("Past EOF", bf, 3, 6, res != 1)) return 1;


    if (VERBOSE) printf("  - bfseek subtest.\n");
    res = bfseek(bf, 10, SEEK_SET);
    if (checkPosition("Seek Set", bf, 1, 2, res)) return 1;

    res = bfseek(bf, 6, SEEK_CUR);
    if (checkPosition("Seek Cur", bf, 2, 0, res)) return 1;

    res = bfseek(bf, -2, SEEK_END);
    if (checkPosition("Seek End", bf, 3, 6, res)) return 1;

    res = bfseek(bf, 22, SEEK_CUR);
    if (checkPosition("Seek EOF", bf, 5, 0, res != 1)) return 1;


    if (VERBOSE) printf("  - bfrewind subtest.\n");
    bfrewind(bf);
    if (checkPosition("Rewind", bf, 0, 0, 0)) return 1;
    
    res = bfclose(bf);
    if (res)
    {
        printf("  FAILED: Failed closing position test file [%d]", res);
        perror("");
        return 1;
    }
    printf("  SUCCESS: Position subtests passed.\n");



    /* ERROR HANDLING */

    /* ... TO DO ... */



    /* UTILITIES */

    printf("%02d) Utility tests\n", testCount++);

    if (VERBOSE) printf("  - Print Binary subtest.\n");

    if (printTest((void *)testtext,  4, "0100")) return 1;
    if (printTest((void *)testtext,  8, "01110100")) return 1;
    if (printTest((void *)testtext, 12, "01000111 0101")) return 1;
    if (printTest((void *)testtext, 66, "00011101 01011101 10011101 11011101 00011110 01011110 10011110 01011000 10")) return 1;

    freeprint();
    if (VERBOSE) printf("    SUCCESS: Print Binary subtest.\n");


    if (VERBOSE) printf("  - Swap Endian subtest.\n");
    byte_t expectarr[8] = { 8, 7, 6, 5, 4, 3, 2, 1 };

    if (swapTest(1, &expectarr[7])) return 1;
    if (swapTest(2, &expectarr[6])) return 1;
    if (swapTest(3, &expectarr[5])) return 1;
    if (swapTest(4, &expectarr[4])) return 1;
    if (swapTest(8, &expectarr[0])) return 1;
    if (VERBOSE) printf("    SUCCESS: Swap Endian subtest.\n");

    printf("  SUCCESS: Utility subtests passed.\n");

    printf("\nSUCCESS: All tests passed!\n");
    return 0;
}


/* -- TEST IMPLEMENTATION -- */


void printNumber(byte_t* bin_data, bsize_t bit_width);
int resetprint();
int checkprint(const char* str);
void printspaces(int count);
int arrcmp(byte_t* a, byte_t* b, size_t size);

FILE* printstream = NULL;
char printname[20] = ".printXXXXXX";


/* Run a read test on the given file, outputting result */
int readTest(const char* test, const char* filename, bool msbFirst, bsize_t counts[], int size, int width, byte_t expected[size][width])
{
    printf("%02d) %s Test (%s first) - Read File: '%s'\n", testCount++, test, msbFirst ? "MSB" : "LSB", filename);
    BITFILE* bitfile = bfopen(filename, "r", msbFirst);

    if (bitfile == NULL)
    {
        if (VERBOSE) perror("  Test cancelled");
        return -1;
    }

    for (int i = 0; i < size; i++)
    {
        char name[4];
        snprintf(name, sizeof(name), "%02d", i + 1);

        if (checkRead(name, bitfile, counts[i], expected[i])) return 1;
    }
    printf("  SUCCESS: %d of %d subtests passed.\n", size, size);

    bfclose(bitfile);
    return 0;
}

/* Run a read test on the given file, outputting result */
int writeTest(const char* test, const char* filename, bool msbFirst, bsize_t counts[], int size, int width, byte_t data[size][width])
{
    printf("%02d) %s Test (%s first) - Write File: '%s'\n", testCount++, test, msbFirst ? "MSB" : "LSB", filename);
    BITFILE* bitfile = bfopen(filename, "w+", msbFirst);

    if (bitfile == NULL)
    {
        if (VERBOSE) printf("  Test cancelled, failed to create file.\n");
        return -1;
    }

    for (int i = 0; i < size; i++)
    {
        bsize_t count = bfwrite(data[i], counts[i], bitfile);

        if (count != counts[i])
        {
            printf("  %02dW [fail: %02"BSIZE_STR"/%02"BSIZE_STR"]: ", i+1, count, counts[i]);
            printNumber(data[i], counts[i]);
        }
        else if (VERBOSE)
        {
            printf("  %02dW [bits: %02"BSIZE_STR"/%02"BSIZE_STR"]: ", i+1, count, counts[i]);
            printNumber(data[i], counts[i]);
        }
    }

    bfrewind(bitfile);

    for (int i = 0; i < size; i++)
    {
        char name[5];
        snprintf(name, sizeof(name), "%02dR", i + 1);

        if (checkRead(name, bitfile, counts[i], data[i])) return 1;
    }
    printf("  SUCCESS: %d of %d subtests passed.\n", size, size);

    bfclose(bitfile);
    return 0;
}

/* Tests if file is at expected position */
int checkPosition(const char* name, BITFILE* bitfile, fpos_t expectedByte, uint8_t expectedBit, int priorReturnVal)
{
    bfpos_t position;
    int result = bfgetpos(bitfile, &position);
    bool failed = priorReturnVal || result || expectedByte != position.byte || expectedBit != position.bit;

    if (failed || VERBOSE) printf("    %-10s -- EXPECTED: %03lld:%u, ", name, expectedByte, expectedBit);

    if (failed)
    {
        printf("FAILED: %03lld:%u [%d]", position.byte, position.bit, result);
        if (priorReturnVal) printf(" -- Operation Error <%d>",priorReturnVal);
        printf("\n  FAILED: Position subtests\n");

        result = bfclose(bitfile);
        if (result) perror("    Error Closing File");
        return 1;
    }

    if (VERBOSE) printf("SUCCESS: %03lld:%u\n", position.byte, position.bit);
    return 0;
}

/* Tests if printing "bin" matches "expected" */
int printTest(byte_t* bin, bsize_t bitcount, const char* expected)
{
    if (resetprint() || printstream == NULL)
    {
        freeprint();
        printf("Error attempting to reset file");
        return -1;
    }

    fprintbin(printstream, bin, bitcount);

    if (checkprint(expected))
    {
        freeprint();

        printf("      EXPECTED: %s\n", expected);
        printf("        FAILED: ");
        fprintbin(stdout, bin, bitcount);
        printf("\n");

        return 1;
    }
    else if (VERBOSE)
    {
        printf("      EXPECTED: %s\n", expected);
        printf("       PRINTED: ");
        fprintbin(stdout, bin, bitcount);
        printf("\n");
    }
    
    return 0;
}

/* Check if swapping array of number from 1-size returns expected array */
int swapTest(int size, byte_t* expected)
{
    byte_t data[size];
    for (int b = 0; b < size; b++) data[b] = b + 1;

    swapendian(data, size * BYTE_LEN);

    if (arrcmp(data, expected, size))
    {
        printf("      EXPECTED [%03d]: ", size);
        for (int b = 0; b < size; b++) printf("%s%"BYTE_STR, b ? ", " : "", expected[b]);
        printf("\n");
        printf("        FAILED [%03d]: ", size);
        for (int b = 0; b < size; b++) printf("%s%"BYTE_STR, b ? ", " : "", data[b]);
        printf("\n");
        return 1;
    }
    else if (VERBOSE)
    {
        printf("      EXPECTED [%03d]: ", size);
        for (int b = 0; b < size; b++) printf("%s%"BYTE_STR, b ? ", " : "", expected[b]);
        printf("\n");
        printf("       SUCCESS [%03d]: ", size);
        for (int b = 0; b < size; b++) printf("%s%"BYTE_STR, b ? ", " : "", data[b]);
        printf("\n");
    }

    return 0;
}

/* Returns 0 if error code == code, else returns 1 */
int expectError(int code)
{
    if (VERBOSE || errno != code) printf("  Expected: ERRx%02d (%s)\n", code, sys_errlist[code]);

    if (errno != code)
    {
        printf("  FAILED:   ERRx%02X (%s)\n", errno, sys_errlist[errno]);
        return 1;
    }
    
    printf("  SUCCESS:  ERRx%02X (%s)\n", errno, sys_errlist[errno]);
    return 0;
}

/* Do a single read on the file, 0 on success, close file on error */
int checkRead(char* name, BITFILE *bf, bsize_t bitcount, byte_t* expected)
{
    size_t bytecount = CEIL_DIV(bitcount, BYTE_LEN);
    byte_t result[bytecount];
    for (int i = 0; i < bytecount; i++) result[i] = 0;

    bsize_t rescount = bfread(result, bitcount, bf);

    int fails = arrcmp(result, expected, bytecount);

    if (VERBOSE || fails)
    {
        printf("  %s [bits: %06"BSIZE_STR"]: ", name, rescount);
        printNumber(result, bitcount);

        printspaces(strlen(name));
        printf("         Expected: ");
        printNumber(expected, bitcount);
    }
    else if (rescount != bitcount)
    {
        printf("  Bit-count mismatch: %"BSIZE_STR" of %"BSIZE_STR" read.\n", rescount, bitcount);
    }

    if (fails)
    {
        printf("  FAILED: %s read test.\n", name);
        bfclose(bf);
        return 1;
    }

    return 0;
}


/* Resets print buffer (WARNING: This breaks stdout until checkprint/freeprint is called.) */
int resetprint()
{
    if (printstream == NULL)
    {
        if (mkstemp(printname) == -1) return -2;

        printstream = fopen(printname, "w+");
        return printstream == NULL ? -1 : 0;
    }

    rewind(printstream);
    return ftell(printstream) != 0L;
}


/* Check if stdout (Since resetprint was called) matches str */
int checkprint(const char* str)
{
    int size = strlen(str);

    char buff[size + 1];
    fflush(printstream);
    rewind(printstream);

    fread(&buff[0], sizeof(char), size, printstream);
    buff[size] = '\0';

    return strcmp(str, &buff[0]);
}

/* Free print buffer (Redirect output to stdout) */
int freeprint()
{
    int res = printstream == NULL ? 0 : fclose(printstream);

    if (res)
    {
        perror("  ERROR closing temp capture file");
        remove(printname);
        printname[0] = '\0';
        return res;
    }

    res = printname[0] == '\0' ? 0 : remove(printname);
    if (res)
    {
        printf("  ERROR removing temp capture file (%s)", printname);
        perror("");
        return res;
    }

    printstream = NULL;
    printname[0] = '\0';
    return 0;
}


#define BIT64 8
/* Print number as full int, hex:int bytes, binary */
void printNumber(byte_t* bin_data, bsize_t bit_width)
{
    const int byte_width = CEIL_DIV(bit_width, BYTE_LEN);

    /* Print numeric value (w/ ">" if overflowing 64-bits) */
    uint64_t num = 0;
    for (int i = 0; i < byte_width && i < BIT64; i++)
    {
        num |= bin_data[i] << (i * BYTE_LEN);
    }
    printf("%c%8llu,", byte_width > 8 ? '>' : ' ', num);

    /* Print hex value */
    for (int i = 0; i < byte_width; i++) printf(" 0x%02X:%03"BYTE_STR, bin_data[i], bin_data[i]);
    printf(", ");

    fprintbin(stdout, bin_data, bit_width);
    printf("\n");
}

/* Compare two arrays, returns 0 if equal */
int arrcmp(byte_t* a, byte_t* b, size_t size)
{
    for (int i = 0; i < size; i++)
    {
        if (a[i] < b[i]) return -1;
        if (a[i] > b[i]) return 1;
    }
    return 0;
}

/* Print <count> spaces */
void printspaces(int count)
{
    while (count--) printf(" ");
}
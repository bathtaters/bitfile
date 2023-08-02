/* TESTs for BITFILE */

#include "bitfile.h"

/* Print all test details */
#define VERBOSE false

/* Test files */
#define TEST_FILE_R "test.txt"
#define TEST_FILE_W "write.txt"

int  readTest(const char* test, const char* filename, bool msbFirst, bsize_t counts[], int size, int width, byte_t expected[size][width], bpos_t offset, int whence);
int writeTest(const char* test, const char* filename, bool msbFirst, bsize_t counts[], int size, int width, byte_t data[size][width], bpos_t offset, int whence);
int expectError(int code);


/* Run all tests */
int main()
{
    /* READ TESTS */

    bsize_t a[] = {8,8,8,8};
    byte_t arm[][1] = {{116}, {117}, {118}, {119}};
    byte_t arl[][1] = {{116}, {117}, {118}, {119}};
    if (readTest("Byte", TEST_FILE_R, false, a, 4, 1, arl, 0, 0)) return 1;
    if (readTest("Byte", TEST_FILE_R,  true, a, 4, 1, arm, 0, 0)) return 1;

    bsize_t b[] = {6,4,3,6,3,4,5,1};
    byte_t brl[][1] = {{52},{5},{5},{51},{6},{13},{29},{0}};
    byte_t brm[][1] = {{29},{1},{6},{43},{5},{ 9},{27},{1}};
    if (readTest("Partial Byte", TEST_FILE_R, false, b, 8, 1, brl, 0, 0)) return 1;
    if (readTest("Partial Byte", TEST_FILE_R,  true, b, 8, 1, brm, 0, 0)) return 1;

    bsize_t c[] = {12,17,3};
    byte_t crl[][3] = {{116, 5, 0}, {103, 119, 1}, {3, 0, 0}};
    byte_t crm[][3] = {{116, 7, 0}, { 87, 103, 0}, {7, 0, 0}};
    if (readTest("Multi-Byte", TEST_FILE_R, false, c, 3, 3, crl, 0, 0)) return 1;
    if (readTest("Multi-Byte", TEST_FILE_R,  true, c, 3, 3, crm, 0, 0)) return 1;

    bsize_t d[] = {16};
    byte_t drs[][2] = {{118, 119}};
    byte_t drc[][2] = {{116, 117}};
    byte_t dre[][2] = {{117, 118}};
    if (readTest("Seek Set", TEST_FILE_R, false, d, 1, 2, drs, 8 *  2, SEEK_SET)) return 1;
    if (readTest("Seek Cur", TEST_FILE_R, false, d, 1, 2, drc, 8 * -1, SEEK_CUR)) return 1;
    if (readTest("Seek End", TEST_FILE_R, false, d, 1, 2, dre, 8 * -3, SEEK_END)) return 1;

    bsize_t e[] = {66};
    byte_t erl[][9] = {{ 116, 117, 118, 119, 0, 0, 0, 0, 0 }};
    if (readTest(">64 bit", TEST_FILE_R, false, e, 1, 9, erl, 0, 0)) return 1;
    if (VERBOSE)
    {
        printf("    w/ Error: '01 [bits: 32/66]'\n");
    }
    else
    {
        printf("    w/ bit-count mismatch 32 of 66.\n");
    }

    bsize_t f[] = {8,8,64,2};
    byte_t frm[][8] = {{116}, {117}, {118, 119, 0, 0, 0, 0, 0, 0}, {0}};
    if (readTest("Read After EOF", TEST_FILE_R, true, f, 4, 8, frm, 0, 0)) return 1;
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
    if (readTest("Missing File", "cancel.txt", false, g, 2, 1, grl, 0, 0) != -1) return 1;
    if (expectError(2)) return 1;

    /* WRITE TESTS */

    bsize_t h[] = {8,8,8,8};
    byte_t hrm[][1] = {{116}, {117}, {118}, {119}};
    byte_t hrl[][1] = {{116}, {117}, {118}, {119}};
    if (writeTest("Byte", TEST_FILE_W, false, h, 4, 1, hrl, 0, 0)) return 1;
    if (writeTest("Byte", TEST_FILE_W,  true, h, 4, 1, hrm, 0, 0)) return 1;

    bsize_t i[] = {6,4,3,6,3,4,5,1};
    byte_t irl[][1] = {{52},{5},{5},{51},{6},{13},{29},{0}};
    byte_t irm[][1] = {{29},{1},{6},{43},{5},{ 9},{27},{1}};
    if (writeTest("Partial Byte", TEST_FILE_W, false, i, 8, 1, irl, 0, 0)) return 1;
    if (writeTest("Partial Byte", TEST_FILE_W,  true, i, 8, 1, irm, 0, 0)) return 1;

    bsize_t j[] = {12,17,3};
    byte_t jrl[][3] = {{116, 5, 0}, {103, 119, 1}, {3, 0, 0}};
    byte_t jrm[][3] = {{116, 7, 0}, { 87, 103, 0}, {7, 0, 0}};
    if (writeTest("Multi-Byte", TEST_FILE_W, false, j, 3, 3, jrl, 0, 0)) return 1;
    if (writeTest("Multi-Byte", TEST_FILE_W,  true, j, 3, 3, jrm, 0, 0)) return 1;

    bsize_t k[] = {66};
    byte_t krl[][9] = {{ 116, 117, 118, 119, 120, 121, 122, 123, 2 }};
    byte_t krm[][9] = {{ 116, 117, 118, 119, 120, 121, 122, 123, 2 }};
    if (writeTest(">64 bit", TEST_FILE_W, false, k, 1, 9, krl, 0, 0)) return 1;
    if (writeTest(">64 bit", TEST_FILE_W,  true, k, 1, 9, krm, 0, 0)) return 1;

    printf("\nSUCCESS: All tests passed!\n");
    return 0;
}



/* -- TEST IMPLEMENTATION -- */


void printNumber(byte_t* bin_data, bsize_t bit_width);
int arrcmp(byte_t* a, byte_t* b, size_t size);

int testCount = 1;

/* Run a read test on the given file, outputting result */
int readTest(const char* test, const char* filename, bool msbFirst, bsize_t counts[], int size, int width, byte_t expected[size][width], bpos_t offset, int whence)
{
    printf("%02d) %s Test (%s first) - Read File: '%s'\n", testCount++, test, msbFirst ? "MSB" : "LSB", filename);
    BITFILE* bitfile = bfopen(filename, "r", msbFirst);

    if (bitfile == NULL)
    {
        if (VERBOSE) perror("  Test cancelled");
        return -1;
    }

    if (offset) bfseek(bitfile, offset, whence);

    for (int i = 0; i < size; i++)
    {
        size_t resultsize = CEIL_DIV(counts[i], BYTE_LEN);
        byte_t result[resultsize];

        int count = (int)bfread(result, counts[i], bitfile);

        int fails = arrcmp(result, &expected[i][0], resultsize);

        if (VERBOSE || fails)
        {
            printf("  %02d [bits: %02d/%02"BSIZE_STR"]: ", i+1, count, counts[i]);
            printNumber(result, counts[i]);

            printf("          Expected: ");
            printNumber(expected[i], counts[i]);
        }
        else if (count != counts[i])
        {
            printf("  Bit-count mismatch: %d of %"BSIZE_STR" read.\n", count, counts[i]);
        }

        if (fails)
        {
            printf("  FAILED: Subtest %d of %d.\n", i+1, size);
            bfclose(bitfile);
            return 1;
        }
    }
    printf("  SUCCESS: %d of %d subtests passed.\n", size, size);

    bfclose(bitfile);
    return 0;
}

/* Run a read test on the given file, outputting result */
int writeTest(const char* test, const char* filename, bool msbFirst, bsize_t counts[], int size, int width, byte_t data[size][width], bpos_t offset, int whence)
{
    printf("%02d) %s Test (%s first) - Write File: '%s'\n", testCount++, test, msbFirst ? "MSB" : "LSB", filename);
    BITFILE* bitfile = bfopen(filename, "w+", msbFirst);

    if (bitfile == NULL)
    {
        if (VERBOSE) printf("  Test cancelled, failed to create file.\n");
        return -1;
    }

    if (offset) bfseek(bitfile, offset, whence);

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
    if (offset) bfseek(bitfile, offset, whence);

    for (int i = 0; i < size; i++)
    {
        size_t resultsize = CEIL_DIV(counts[i], BYTE_LEN);
        byte_t result[resultsize];

        bsize_t count = bfread(result, counts[i], bitfile);

        int fails = arrcmp(result, &data[i][0], resultsize);

        if (VERBOSE || fails)
        {
            printf("  %02dR [bits: %02"BSIZE_STR"/%02"BSIZE_STR"]: ", i+1, count, counts[i]);
            printNumber(result, counts[i]);

            printf("           Expected: ");
            printNumber(data[i], counts[i]);
        }
        else if (count != counts[i])
        {
            printf("  Bit-count mismatch: %"BSIZE_STR" of %"BSIZE_STR" read.\n", count, counts[i]);
        }

        if (fails)
        {
            printf("  FAILED: Subtest %d of %d.\n", i+1, size);
            bfclose(bitfile);
            return 1;
        }
    }
    printf("  SUCCESS: %d of %d subtests passed.\n", size, size);

    bfclose(bitfile);
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

/* Print number as full int, hex:int bytes, binary */
#define BIT64 8
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

    printbin(bin_data, bit_width);
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
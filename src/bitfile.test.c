/* TESTs for BITFILE */

#include "bitfile.h"

/* Print all test details */
#define VERBOSE false

/* Test files */
#define TEST_FILE_1 "test.txt"

int readTest(char* test, char* filename, bool msbFirst, int counts[], int size, int width, uint8_t expected[size][width], int offset, int whence);

/* Run all tests */
int main()
{
    int a[] = {8,8,8,8};
    uint8_t arm[][1] = {{116}, {117}, {118}, {119}};
    uint8_t arl[][1] = {{116}, {117}, {118}, {119}};
    if (readTest("Byte", TEST_FILE_1, false, a, 4, 1, arl, 0, 0)) return 1;
    if (readTest("Byte", TEST_FILE_1,  true, a, 4, 1, arm, 0, 0)) return 1;

    int b[] = {6,4,3,6,3,4,5,1};
    uint8_t brl[][1] = {{52},{5},{5},{51},{6},{13},{29},{0}};
    uint8_t brm[][1] = {{29},{1},{6},{43},{5},{ 9},{27},{1}};
    if (readTest("Partial Byte", TEST_FILE_1, false, b, 8, 1, brl, 0, 0)) return 1;
    if (readTest("Partial Byte", TEST_FILE_1,  true, b, 8, 1, brm, 0, 0)) return 1;

    int c[] = {12,17,3};
    uint8_t crl[][3] = {{116, 5, 0}, {103, 119, 1}, {3, 0, 0}};
    uint8_t crm[][3] = {{116, 7, 0}, { 87, 103, 0}, {7, 0, 0}};
    if (readTest("Multi-Byte", TEST_FILE_1, false, c, 3, 3, crl, 0, 0)) return 1;
    if (readTest("Multi-Byte", TEST_FILE_1,  true, c, 3, 3, crm, 0, 0)) return 1;

    int d[] = {16};
    uint8_t drs[][2] = {{118, 119}};
    uint8_t drc[][2] = {{116, 117}};
    uint8_t dre[][2] = {{117, 118}};
    if (readTest("Seek Set", TEST_FILE_1, false, d, 1, 2, drs, 8 *  2, SEEK_SET)) return 1;
    if (readTest("Seek Cur", TEST_FILE_1, false, d, 1, 2, drc, 8 * -1, SEEK_CUR)) return 1;
    if (readTest("Seek End", TEST_FILE_1, false, d, 1, 2, dre, 8 * -3, SEEK_END)) return 1;

    int e[] = {66};
    uint8_t erl[][9] = {{ 116, 117, 118, 119, 0, 0, 0, 0, 0 }};
    if (readTest(">64 bit", TEST_FILE_1, false, e, 1, 9, erl, 0, 0)) return 1;
    if (VERBOSE)
    {
        printf("    w/ Error: '01 [bits: 32/66]'\n");
    }
    else
    {
        printf("    w/ bit-count mismatch 32 of 66.\n");
    }

    int f[] = {8,8,64,2};
    uint8_t frm[][8] = {{116}, {117}, {118, 119, 0, 0, 0, 0, 0, 0}, {0}};
    if (readTest("Read After EOF", TEST_FILE_1, true, f, 4, 8, frm, 0, 0)) return 1;
    if (VERBOSE)
    {
        printf("    w/ Errors: '03 [bits: 16/64]'\n");
        printf("               '04 [bits: 00/02]'\n");
    }
    else
    {
        printf("    w/ bit-count mismatches 16 of 64, 0 of 2.\n");
    }

    int g[] = {8,8};
    uint8_t grl[][1] = {{0},{0}};
    if (readTest("Missing File", "cancel.txt", false, g, 2, 1, grl, 0, 0) != -1) return 1;
    if (VERBOSE)
    {
        printf("    w/ Errors: 'File unable to be opened for 'rb': cancel.txt'\n");
        printf("               'Test cancelled due to missing file.'\n");
    }
    else
    {
        printf("  SUCCESS if file open error is present.\n");
    }

    printf("\nSUCCESS: All tests passed!\n");
    return 0;
}






/* -- TEST IMPLEMENTATION -- */


void printNumber(byte_t* bin_data, int bit_width);
int arrcmp(uint8_t* a, uint8_t* b, int size);

int testCount = 1;

/* Run a read test on the given file, outputting result */
int readTest(char* test, char* filename, bool msbFirst, int counts[], int size, int width, uint8_t expected[size][width], int offset, int whence)
{
    printf("%02d) %s Test (%s first) - File: '%s'\n", testCount++, test, msbFirst ? "MSB" : "LSB", filename);
    BITFILE* bitfile = bfopen(filename, "r", msbFirst);

    if (bitfile == NULL)
    {
        if (VERBOSE) printf("  Test cancelled due to missing file.\n");
        return -1;
    }

    for (int i = 0; i < size; i++)
    {
        int resultsize = CEIL_DIV(counts[i], BYTE_LEN);
        byte_t result[resultsize];
        for (int j = 0; j < resultsize; j++) result[j] = 0;

        if (offset) bfseek(bitfile, 0, offset, whence);

        int count = (int)bfread(result, counts[i], bitfile);

        int fails = arrcmp(result, &expected[i][0], resultsize);

        if (VERBOSE || fails)
        {
            printf("  %02d [bits: %02d/%02d]: ", i+1, count, counts[i]);
            printNumber(result, counts[i]);

            printf("          Expected: ");
            printNumber(expected[i], counts[i]);
        }
        else if (count != counts[i])
        {
            printf("  Bit-count mismatch: %d of %d read.\n", count, counts[i]);
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

/* Print number as full int, hex:int bytes, binary */
void printNumber(byte_t* bin_data, int bit_width)
{
    const int byte_width = CEIL_DIV(bit_width, BYTE_LEN);

    /* Print numeric value (w/ ">" if overflowing 64-bits) */
    uint64_t num = 0;
    for (int i = 0; i < byte_width && i < 8; i++)
    {
        num |= bin_data[i] << (i * BYTE_LEN);
    }
    printf("%c%8llu,", byte_width > 8 ? '>' : ' ', num);

    /* Print hex value */
    for (int i = 0; i < byte_width; i++) printf(" 0x%02X:%03d", bin_data[i], bin_data[i]);
    printf(", ");

    printbin(bin_data, bit_width);
    printf("\n");
}

/* Compare two arrays, returns 0 if equal */
int arrcmp(uint8_t* a, uint8_t* b, int size)
{
    for (int i = 0; i < size; i++)
    {
        if (a[i] < b[i]) return -1;
        if (a[i] > b[i]) return 1;
    }
    return 0;
}
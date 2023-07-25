/* TESTs for BitReader */

#include "readbits.h"

#define VERBOSE false

int runTest(char* test, char* filename, bool msbFirst, int counts[], int size, int width, uint8_t results[size][width]);

int main()
{
    int a[] = {8,8,8,8};
    uint8_t arm[][1] = {{116}, {117}, {118}, {119}};
    uint8_t arl[][1] = {{116}, {117}, {118}, {119}};
    if (runTest("Byte", "test.txt", false, a, 4, 1, arl)) return 1;
    if (runTest("Byte", "test.txt",  true, a, 4, 1, arm)) return 1;

    int b[] = {6,4,3,6,3,4,5,1};
    uint8_t brl[][1] = {{52},{5},{5},{51},{6},{13},{29},{0}};
    uint8_t brm[][1] = {{29},{1},{6},{43},{5},{ 9},{27},{1}};
    if (runTest("Partial Byte", "test.txt", false, b, 8, 1, brl)) return 1;
    if (runTest("Partial Byte", "test.txt",  true, b, 8, 1, brm)) return 1;

    int c[] = {12,17,3};
    uint8_t crl[][3] = {{116, 5, 0}, {103, 119, 1}, {3, 0, 0}};
    uint8_t crm[][3] = {{116, 7, 0}, { 87, 103, 0}, {7, 0, 0}};
    if (runTest("Multi-Byte", "test.txt", false, c, 3, 3, crl)) return 1;
    if (runTest("Multi-Byte", "test.txt",  true, c, 3, 3, crm)) return 1;

    int d[] = {66};
    uint8_t drl[][9] = {{ 116, 117, 118, 119, 0, 0, 0, 0, 0 }};
    if (runTest(">64 bit", "test.txt", false, d, 1, 9, drl)) return 1;
    if (VERBOSE)
    {
        printf("  Expected: 'Warning: Reached end of file.'\n");
    }
    
    int e[] = {8,8,64,2};
    uint8_t erm[][8] = {{116}, {117}, {118, 119, 0, 0, 0, 0, 0, 0}, {0}};
    if (runTest("Read After EOF", "test.txt", true, e, 4, 8, erm)) return 1;
    if (VERBOSE)
    {
        printf("  Expected: 'Warning: Reached end of file.'\n");
        printf("            'Error: Attempting to read a closed file.'\n");
    }

    int f[] = {8,8};
    uint8_t frl[][1] = {{0},{0}};
    if (runTest("Missing File", "missing.txt", false, f, 2, 1, frl)) return 1;
    if (VERBOSE)
    {
        printf("  Expected: 'Error: Attempting to read a closed file.'\n");
        printf("            'Error: Attempting to read a closed file.'\n");
    }

    printf("\nSUCCESS: All tests passed!\n");
    return 0;
}






int arrcmp(uint8_t* a, uint8_t* b, int size);

int testCount = 1;

/* Run BitReader on the given file, outputting result */
int runTest(char* test, char* filename, bool msbFirst, int counts[], int size, int width, uint8_t expected[size][width])
{
    BitReader br = newBitReader(filename, msbFirst);

    printf("%02d) %s Test (%s first) - File: '%s'\n", testCount++, test, msbFirst ? "MSB" : "LSB", filename);
    for (int i = 0; i < size; i++)
    {
        uint8_t* result = getBits(&br, counts[i]);
        int fails = arrcmp(result, &expected[i][0], CEIL_DIV(counts[i], BYTE_LEN));

        if (VERBOSE || fails)
        {
            printf("  %02d [bits: %02d]: ", i+1, counts[i]);
            printbin(result, counts[i]);
            printf("\n");

            printf("       Expected: ");
            printbin(expected[i], counts[i]);
            printf("\n");
        }

        if (fails)
        {
            printf("  FAILED: Subtest %d of %d.\n", i+1, size);
            free(result);
            return 1;
        }
        free(result);
    }
    printf("  SUCCESS: %d of %d subtests passed.\n", size, size);

    freeBitReader(br);
    return 0;
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
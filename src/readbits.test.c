/* TESTs for BitReader */

#include "readbits.h"

#define VERBOSE 0

int runTest(char* test, char* filename, char msbFirst, int counts[], uint64_t results[], int size);


int main()
{
    int a[] = {8,8,8,8};
    uint64_t arm[] = {116, 117, 118, 119};
    uint64_t arl[] = {116, 117, 118, 119};
    if (runTest("Byte", "test.txt", 0, a, arl, 4)) { return 1; }
    if (runTest("Byte", "test.txt", 1, a, arm, 4)) { return 1; }

    int b[] = {6,4,3,6,3,4,5,1};
    uint64_t brl[] = {52,5,5,51,6,13,29,0};
    uint64_t brm[] = {29,1,6,43,5, 9,27,1};
    if (runTest("Partial Byte", "test.txt", 0, b, brl, 8)) { return 1; }
    if (runTest("Partial Byte", "test.txt", 1, b, brm, 8)) { return 1; }

    int c[] = {12,17,3};
    uint64_t crl[] = {1396, 96103, 3};
    uint64_t crm[] = {1863, 44750, 7};
    if (runTest("Multi-Byte", "test.txt", 0, c, crl, 3)) { return 1; }
    if (runTest("Multi-Byte", "test.txt", 1, c, crm, 3)) { return 1; }

    int d[] = {65};
    uint64_t drl[] = {3914554};
    if (runTest("Overflow", "test.txt", 0, d, drl, 1)) { return 1; }
    if (VERBOSE)
    {
        printf("  Expected: 'Error: Attempting to read too many bits.'\n");
        printf("            'Warning: Reached end of file.'\n");
    }
    
    int e[] = {8,8,64,2};
    uint64_t erm[] = {116, 117, 30327, 0};
    if (runTest("Read After EOF", "test.txt", 1, e, erm, 4)) { return 1; }
    if (VERBOSE)
    {
        printf("  Expected: 'Warning: Reached end of file.'\n");
        printf("            'Error: Attempting to read a closed file.'\n");
    }

    int f[] = {8,8};
    uint64_t frl[] = {0,0};
    if (runTest("Missing File", "missing.txt", 0, f, frl, 2)) { return 1; }
    if (VERBOSE)
    {
        printf("  Expected: 'Error: Attempting to read a closed file.'\n");
        printf("            'Error: Attempting to read a closed file.'\n");
    }

    printf("\nSUCCESS: All tests passed!\n");
    return 0;
}

int testCount = 1;

/* Run BitReader on the given file, outputting result */
int runTest(char* test, char* filename, char msbFirst, int counts[], uint64_t results[], int size)
{
    BitReader* br = newBitReader(filename, msbFirst);

    printf("%02d) %s Test (%s first) - File: '%s'\n", testCount++, test, msbFirst ? "MSB" : "LSB", filename);
    for (int i = 0; i < size; i++)
    {
        uint64_t result = getBits(br, counts[i]);

        if (VERBOSE || result != results[i])
        {
            printf("  %02d [bits: %02d]: ", i+1, counts[i]);
            printbin(result, counts[i]);
            printf("\n");

            printf("       Expected: ");
            printbin(results[i], counts[i]);
            printf("\n");
        }

        if (result != results[i])
        {
            printf("  FAILED: Subtest %d of %d.\n", i+1, size);
            return 1;
        }
    }
    printf("  SUCCESS: %d of %d subtests passed.\n", size, size);

    freeBitReader(br);
    return 0;
}
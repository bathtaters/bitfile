/* TESTs for BitReader */

#include "readbits.h"

#define VERBOSE 0

int runTest(char* filename, int counts[], uint64_t results[], int size);


int main()
{
    // TEST 01
    int a[] = {8,8,8,8};
    uint64_t ar[] = {116, 117, 118, 119};
    if (runTest("test.txt", a, ar, 4)) { return 1; }

    // TEST 02
    int b[] = {6,4,3,6,3,4,5,1};
    uint64_t br[] = {52,5,5,51,6,13,29,0};
    if (runTest("test.txt", b, br, 8)) { return 1; }

    // TEST 03
    int c[] = {12,17,3};
    uint64_t cr[] = {1396, 96103, 3};
    if (runTest("test.txt", c, cr, 3)) { return 1; }

    // TEST 04
    int d[] = {65};
    uint64_t dr[] = {3914554};
    if (runTest("test.txt", d, dr, 1)) { return 1; }
    if (VERBOSE)
    {
        printf("  Expected: 'Error: Attempting to read too many bits.'\n");
        printf("            'Warning: Reached end of file.'\n");
    }
    
    // TEST 05
    int e[] = {8,8,64,2};
    uint64_t er[] = {116, 117, 30582, 0};
    if (runTest("test.txt", e, er, 4)) { return 1; }
    if (VERBOSE)
    {
        printf("  Expected: 'Warning: Reached end of file.'\n");
        printf("            'Error: Attempting to read a closed file.'\n");
    }

    // TEST 06
    int f[] = {8,8};
    uint64_t fr[] = {0,0};
    if (runTest("does_not_exist.txt", f, fr, 2)) { return 1; }
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
int runTest(char* filename, int counts[], uint64_t results[], int size)
{
    BitReader* br = newBitReader(filename);

    printf("TEST %02d - File: '%s'\n", testCount++, filename);
    for (int i = 0; i < size; i++)
    {
        uint64_t result = getBits(br, counts[i]);

        if (VERBOSE)
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
            printf("  FAILED: Subtest %d of %d, Expecting %llu, Recieved: %llu.\n", i+1, size, results[i], result);
            return 1;
        }
    }
    printf("  SUCCESS: %d of %d subtests passed.\n", size, size);

    freeBitReader(br);
    return 0;
}
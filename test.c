#include <stdio.h>

#define TESTREGADDR     0
#define TESTREG     (uint32_t*)TESTREGADDR

int main()
{
    printf("%d\n", *TESTREG);


    return 0;
}
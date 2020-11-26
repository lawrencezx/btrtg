#include "compiler.h"
#include "nasmlib.h"

int nasm_random32(int bound)
{
    int rand32;
    rand32 = rand() % bound;
    return rand32;
}

long long nasm_random64(long long bound)
{
    long long rand0, rand1, rand2, rand64;
    rand0 = rand() % 2;
    rand1 = rand();
    rand2 = rand();
    rand64 = ((rand0 << 62) | (rand1 << 31) | rand2) % bound;
    return rand64;
}

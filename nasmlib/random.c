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

bool likely_happen_p(double p)
{
    if (p <= 0)
        return false;
    if (p >= 1)
        return true;
    int pivot;
    static const double precision_bound = 10000.0;
    pivot = (int)(precision_bound * p);
    return nasm_random32(precision_bound) < pivot;
}

bool likely_happen_w(int w, int total_w)
{
    if (w < 0 || w >= INT_MAX || total_w < 0 || total_w > INT_MAX) {
        return false;
    }
    return nasm_random32(total_w) < w;
}

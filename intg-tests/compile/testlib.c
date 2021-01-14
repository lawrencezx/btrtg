#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include "verification.h"

struct X86StandardRegisters output[] =
{
#include "output.h"
    {}
};

void compare_x86_state(struct X86StandardRegisters x86regs)
{
    static int i = 0;
    int diff = 0;
    if (x86regs.gs != output[i].gs) {
        printf("diff [gs]: %x, should be: 0x%x\n", x86regs.gs, output[i].gs);
        diff = 1;
    }
    if (x86regs.fs != output[i].fs) {
        printf("diff [fs]: %x, should be: 0x%x\n", x86regs.fs, output[i].fs);
        diff = 1;
    }
    if (x86regs.es != output[i].es) {
        printf("diff [es]: %x, should be: 0x%x\n", x86regs.es, output[i].es);
        diff = 1;
    }
    if (x86regs.ds != output[i].ds) {
        printf("diff [ds]: %x, should be: 0x%x\n", x86regs.ds, output[i].ds);
        diff = 1;
    }
    if (x86regs.ss != output[i].ss) {
        printf("diff [ss]: %x, should be: 0x%x\n", x86regs.ss, output[i].ss);
        diff = 1;
    }
    if (x86regs.cs != output[i].cs) {
        printf("diff [cs]: %x, should be: 0x%x\n", x86regs.cs, output[i].cs);
        diff = 1;
    }
    if (x86regs.eflags != output[i].eflags) {
        printf("diff [eflags]: %x, should be: 0x%x\n", x86regs.eflags, output[i].eflags);
        diff = 1;
    }
    if (x86regs.edi != output[i].edi) {
        printf("diff [edi]: %x, should be: 0x%x\n", x86regs.edi, output[i].edi);
        diff = 1;
    }
    if (x86regs.esi != output[i].esi) {
        printf("diff [esi]: %x, should be: 0x%x\n", x86regs.esi, output[i].esi);
        diff = 1;
    }
    if (x86regs.ebx != output[i].ebx) {
        printf("diff [ebx]: %x, should be: 0x%x\n", x86regs.ebx, output[i].ebx);
        diff = 1;
    }
    if (x86regs.edx != output[i].edx) {
        printf("diff [edx]: %x, should be: 0x%x\n", x86regs.edx, output[i].edx);
        diff = 1;
    }
    if (x86regs.ecx != output[i].ecx) {
        printf("diff [ecx]: %x, should be: 0x%x\n", x86regs.ecx, output[i].ecx);
        diff = 1;
    }
    if (x86regs.eax != output[i].eax) {
        printf("diff [eax]: %x, should be: 0x%x\n", x86regs.eax, output[i].eax);
        diff = 1;
    }
    if (diff) {
        printf("diff test position: %d\n", i + 1);
        exit(0);
    }
    i++;
}

void print_all_state(struct X87LegacyXSaveArea x87fpstate, struct X86StandardRegisters x86regs)
{
    compare_x86_state(x86regs);
}

void finish_test(void)
{
    printf("PASS!\n");
    fclose(stdout);
}

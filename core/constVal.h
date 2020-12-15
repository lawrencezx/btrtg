#ifndef NASM_GENINIT_H
#define NASM_GENINIT_H

typedef struct constVal {
    opflags_t type;
    union {
        int8_t imm8;
        int8_t unity;
        int16_t imm16;
        int32_t imm32;
        char *instName;
    };
} constVal;

#endif
